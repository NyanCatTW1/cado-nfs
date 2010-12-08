#include "cado.h"

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include "bwc_config.h"
#include "parallelizing_info.h"
#include "matmul_top.h"
#include "abase.h"
#include "select_mpi.h"

#include "params.h"
#include "xvectors.h"
#include "bw-common-mpi.h"
#include "filenames.h"
#include "balancing.h"

struct sfile_info {
    unsigned int n0,n1;
    unsigned int iter;
};

struct sfiles_list {
    int sfiles_alloc;
    int nsfiles;
    struct sfile_info * sfiles;
};


static void prelude(parallelizing_info_ptr pi, struct sfiles_list * s, balancing_ptr bal)
{
    s->sfiles_alloc=0;
    s->sfiles = NULL;
    s->nsfiles=0;
    serialize_threads(pi->m);
    int rc;
    char * spat;
    rc = asprintf(&spat, S_FILE_PATTERN "%%n", bal->h->checksum);
    ASSERT_ALWAYS(rc >= 0);
    if (pi->m->jrank == 0 && pi->m->trank == 0) {
        /* It's our job to collect the directory data.
         */
        DIR * dir;
        dir = opendir(".");
        struct dirent * de;
        for( ; (de = readdir(dir)) != NULL ; ) {
            int k;
            int rc;
            unsigned int n0,n1;
            unsigned int iter;
            rc = sscanf(de->d_name, spat, &n0, &n1, &iter, &k);
            if (rc < 3 || k != (int) strlen(de->d_name)) {
                continue;
            }
            if (s->nsfiles >= s->sfiles_alloc) {
                for( ; s->nsfiles >= s->sfiles_alloc ; s->sfiles_alloc += 8 + s->sfiles_alloc/2);
                s->sfiles = realloc(s->sfiles, s->sfiles_alloc * sizeof(struct sfile_info));
            }
            s->sfiles[s->nsfiles].n0 = n0;
            s->sfiles[s->nsfiles].n1 = n1;
            s->sfiles[s->nsfiles].iter = iter;
            if (bw->interval && iter % bw->interval != 0) {
                fprintf(stderr,
                        "Warning: %s is not a checkpoint at a multiple of "
                        "the interval value %d -- this might indicate a "
                        "severe bug with leftover data, likely to corrupt "
                        "the final computation\n", de->d_name, bw->interval);
            }
            s->nsfiles++;
        }
        closedir(dir);
    }
    free(spat);
    /* Note that it's not necessary to care about the file names -- in
     * practice, the file name is only relevant to the job/thread doing
     * actual I/O.
     */
    serialize(pi->m);
    global_broadcast(pi->m, &s->sfiles_alloc, sizeof(int), 0, 0);
    serialize(pi->m);
    global_broadcast(pi->m, &s->nsfiles, sizeof(int), 0, 0);
    serialize(pi->m);
    if (!(pi->m->jrank == 0 && pi->m->trank == 0)) {
        s->sfiles = malloc(s->sfiles_alloc * sizeof(struct sfile_info));
    }
    global_broadcast(pi->m, s->sfiles, s->nsfiles * sizeof(struct sfile_info), 0, 0);
    serialize(pi->m);
}

int agree_on_flag(pi_wiring_ptr w, int v)
{
    int * ptr = &v;
    thread_broadcast(w, (void**) &ptr, 0);
    for(unsigned int i = 0 ; i < w->ncores ; i++) {
        serialize_threads(w);
        * ptr &= v;
    }
    serialize_threads(w);

    if (w->trank == 0) {
        int err = MPI_Allreduce(MPI_IN_PLACE, &v, 1, MPI_INT, MPI_LAND, w->pals);
        ASSERT_ALWAYS(!err);
    }
    serialize_threads(w);
    v = *ptr;
    /* serialize again because the leader must not release its stack
     * before all pals have read it ! */
    serialize_threads(w);
    return v;
}

void * gather_prog(parallelizing_info_ptr pi, param_list pl, void * arg MAYBE_UNUSED)
{
    /* Interleaving does not make sense for this program. So the second
     * block of threads just leave immediately */
    if (pi->interleaved && pi->interleaved->idx)
        return NULL;

    struct sfiles_list sf[1];

    abobj_t abase;
    abobj_init(abase);
    abobj_set_nbys(abase, bw->n);

    int tcan_print = bw->can_print && pi->m->trank == 0;
    matmul_top_data mmt;

    int flags[2];
    flags[bw->dir] = THREAD_SHARED_VECTOR;
    flags[!bw->dir] = 0;

    matmul_top_init(mmt, abase, pi, flags, pl, bw->dir);

    prelude(pi, sf, mmt->bal);

    mmt_wiring_ptr mcol = mmt->wr[bw->dir];
    mmt_wiring_ptr mrow = mmt->wr[!bw->dir];

    pi_wiring_ptr picol = mmt->pi->wr[bw->dir];

    abzero(abase, mrow->v->v, mrow->i1 - mrow->i0);

    unsigned int ii0, ii1;
    unsigned int di = mcol->i1 - mcol->i0;

    ii0 = mcol->i0 + di * (picol->jrank * picol->ncores + picol->trank) /
        (picol->njobs * picol->ncores);
    ii1 = mcol->i0 + di * (picol->jrank * picol->ncores + picol->trank + 1) /
        (picol->njobs * picol->ncores);

    size_t stride =  abbytes(abase, 1);

    mmt_generic_vec svec;
    vec_init_generic(mmt->pi->m, stride, svec, 0, ii1-ii0);
    mmt_generic_vec tvec;
    vec_init_generic(mmt->pi->m, stride, tvec, 0, ii1-ii0);

    abase_generic_zero(stride, tvec->v, ii1 - ii0);

    int rc;
    char * tmp;
    char * spat;
    rc = asprintf(&spat, S_FILE_PATTERN, mmt->bal->h->checksum);
    ASSERT_ALWAYS(rc >= 0);

    for(int i = 0 ; i < sf->nsfiles ; i++) {
        rc = asprintf(&tmp, spat,
                sf->sfiles[i].n0,
                sf->sfiles[i].n1,
                sf->sfiles[i].iter);

        if (tcan_print) {
            printf("loading %s\n", tmp);
        }
        pi_load_file_2d(pi, bw->dir, tmp, svec->v, (ii1 - ii0) * stride);
        free(tmp);
        for(unsigned int j = ii0 ; j < ii1 ; j++) {
            abadd(abase,
                    tvec->v + abbytes(abase, j - ii0),
                    svec->v + abbytes(abase, j - ii0));
        }
    }
    free(spat);

    /* We're simply using file I/O as a means of broadcast. It's much
     * easier, since we'll be doing I/O anyway...
     */
    rc = asprintf(&tmp, COMMON_VECTOR_ITERATE_PATTERN, K_FILE_BASE_PATTERN, 0, mmt->bal->h->checksum);
    ASSERT_ALWAYS(rc >= 0);
    pi_save_file_2d(pi, bw->dir, tmp, tvec->v, (ii1 - ii0) * stride);
    free(tmp);
    
    int is_zero = 0;

    serialize(pi->m);
    matmul_top_load_vector(mmt, K_FILE_BASE_PATTERN, bw->dir, 0);

    unsigned int how_many;
    unsigned int offset_c;
    unsigned int offset_v;
    how_many = intersect_two_intervals(&offset_c, &offset_v,
                mrow->i0, mrow->i1,
                mcol->i0, mcol->i1);

    abt * check_area = mcol->v->v + aboffset(abase, offset_v);
    is_zero = abis_zero(abase, check_area, how_many);

    if (agree_on_flag(pi->m, is_zero)) {
        fprintf(stderr, "Found zero vector. Most certainly a bug. "
                "No solution found.\n");
        exit(1);
    }
    serialize(pi->m);

    for(int i = 1 ; i < 10 ; i++) {
        serialize(pi->m);
        matmul_top_mul(mmt, bw->dir);
        is_zero = abis_zero(abase, check_area, how_many);
        serialize(pi->m);
        if (agree_on_flag(pi->m, is_zero)) {
            if (tcan_print) {
                printf("M^%u * V is zero !\n", i);
            }
            if (pi->m->jrank == 0 && pi->m->trank == 0) {
                int rc;
                rc = asprintf(&tmp, K_FILE_PATTERN, i-1, mmt->bal->h->checksum);
                ASSERT_ALWAYS(rc != -1);
                char * tmp2;
                rc = asprintf(&tmp2, W_FILE, mmt->bal->h->checksum);
                ASSERT_ALWAYS(rc >= 0);
                unlink(tmp2);
                rc = link(tmp, tmp2);
                if (rc < 0) {
                    fprintf(stderr, "Cannot hard link %s to %s: %s\n",
                            tmp2, tmp, strerror(errno));
                }
                free(tmp2);
                free(tmp);
            }
            break;
        }

        matmul_top_save_vector(mmt, K_FILE_BASE_PATTERN, bw->dir, i);
    }
    if (!is_zero) {
        if (tcan_print) {
            printf("No solution found ; most probably a bug\n");
        }
    }

    serialize(pi->m);

    vec_clear_generic(mmt->pi->m, stride, svec, ii1-ii0);
    vec_clear_generic(mmt->pi->m, stride, tvec, ii1-ii0);

    matmul_top_clear(mmt, abase);
    return NULL;
}


void usage()
{
    fprintf(stderr, "Usage: ./gather <options>\n");
    fprintf(stderr, "%s", bw_common_usage_string());
    fprintf(stderr, "Relevant options here: wdir cfg m n mpi thr matrix interval\n");
    fprintf(stderr, "Note: data files must be found in wdir !\n");
    fprintf(stderr, "All S* files are taken in wdir. Spurious leftover data may corrupt result !\n");
    exit(1);
}

int main(int argc, char * argv[])
{
    param_list pl;
    param_list_init(pl);
    bw_common_init_mpi(bw, pl, &argc, &argv);
    if (param_list_warn_unused(pl)) usage();

    if (bw->nx == 0) { fprintf(stderr, "no nx value set\n"); exit(1); } 

    setvbuf(stdout,NULL,_IONBF,0);
    setvbuf(stderr,NULL,_IONBF,0);

    pi_go(gather_prog, pl, 0);

    param_list_clear(pl);
    bw_common_clear_mpi(bw);
    return 0;
}

