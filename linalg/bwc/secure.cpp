#include "cado.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bwc_config.h"
#include "parallelizing_info.h"
#include "matmul_top.h"
#include "select_mpi.h"
#include "params.h"
#include "xvectors.h"
#include "bw-common.h"
#include "mpfq/mpfq.h"
#include "mpfq/mpfq_vbase.h"
#include "async.h"
#include <algorithm>
#include "cheating_vec_init.h"
#include "fmt/printf.h"
#include "fmt/format.h"
using namespace fmt::literals;

int legacy_check_mode = 0;

/* We create the check data based on:
 *
 *  - the random seed
 *  - the X vector that was created by the prep program.
 *
 * The check data is made of four distinct sets of files.
 *
 * The first two are singletons, and depend on the random seed only.
 *
 * Ct0-$nchecks.0-$m (also referred to as T): a random matrix of size bw->m*nchecks (yes it's named like this because the data is written row-major, and the first interval in the name customarily denotes the number of items in a major division).
 *
 * Cr0-$nchecks.0-$nchecks (also referred to as R): a sequence of random matrices of size nchecks * nchecks
 *
 * Multiple instances of the other two can exist, depending on various
 * check distances.
 *
 * Cv0-$nchecks.$s (also referred to as C) : check vector for distance $s. Depends on X.
 * Cd0-$nchecks.$s (also referred to as D) : check vector for distance $s. Depends on X, T, and R.
 */

void * sec_prog(parallelizing_info_ptr pi, param_list pl, void * arg MAYBE_UNUSED)
{

    int fake = param_list_lookup_string(pl, "random_matrix") != NULL;

    ASSERT_ALWAYS(!pi->interleaved);

    int tcan_print = bw->can_print && pi->m->trank == 0;
    matmul_top_data mmt;

    int withcoeffs = mpz_cmp_ui(bw->p, 2) > 0;
    int nchecks = withcoeffs ? NCHECKS_CHECK_VECTOR_GFp : NCHECKS_CHECK_VECTOR_GF2;
    mpfq_vbase A;
    mpfq_vbase_oo_field_init_byfeatures(A, 
            MPFQ_PRIME_MPZ, bw->p,
            MPFQ_SIMD_GROUPSIZE, nchecks,
            MPFQ_DONE);

    /* We need that in order to do matrix products */
    mpfq_vbase_tmpl AxA;
    mpfq_vbase_oo_init_templates(AxA, A, A);


    matmul_top_init(mmt, A, pi, pl, bw->dir);
    pi_datatype_ptr A_pi = mmt->pitype;

    mmt_vec myy[2];
    mmt_vec_ptr my = myy[0];
    mmt_vec_ptr y = myy[1];
    mmt_vec dvec;

    /* we work in the opposite direction compared to other programs */
    mmt_vec_init(mmt,0,0, y,   bw->dir,                0, mmt->n[bw->dir]);
    mmt_vec_init(mmt,0,0, my, !bw->dir, /* shared ! */ 1, mmt->n[!bw->dir]);
    mmt_vec_init(mmt,0,0, dvec,!bw->dir, /* shared ! */ 1, mmt->n[!bw->dir]);

    unsigned int unpadded = MAX(mmt->n0[0], mmt->n0[1]);

    /* Because we're a special case, we _expect_ to work opposite to
     * optimized direction. So we pass bw->dir even though _we_ are going
     * to call mmt_mul with !bw->dir.
     */

    serialize(pi->m);

    int rc;

    /* To fill Cr and Ct, we need a random state */
    gmp_randstate_t rstate;
    gmp_randinit_default(rstate);
#if 0
    /* After all, a zero seed is fine, too */
    if (pi->m->trank == 0 && !bw->seed) {
        /* note that bw is shared between threads, thus only thread 0 should
         * test and update it here.
         * at pi->m->jrank > 0, we don't care about the seed anyway
         */
        bw->seed = time(NULL);
        MPI_Bcast(&bw->seed, 1, MPI_INT, 0, pi->m->pals);
    }
#endif
    gmp_randseed_ui(rstate, bw->seed);
    if (tcan_print) {
        printf("// Random generator seeded with %d\n", bw->seed);
    }

    /* Ct is a constant projection matrix of size bw->m * nchecks */
    /* It depends only on the random seed. We create it if start==0, or
     * reload it otherwise. */
    std::string Tfilename = "Ct0-{}.0-{}"_format(nchecks, bw->m);
    size_t T_coeff_size = A->vec_elt_stride(A, bw->m);
    void * Tdata;
    cheating_vec_init(A, &Tdata, bw->m);

    /* Cr is a list of matrices of size nchecks * nchecks */
    /* It depends only on the random seed */
    std::string Rfilename = "Cr0-{}.0-{}"_format(nchecks, nchecks);
    FILE * Rfile = NULL;
    size_t R_coeff_size = A->vec_elt_stride(A, nchecks);


    /* {{{ First check consistency of existing files with the bw->start
     * value. We wish to abort early (and not touch any existing file!)
     * if an inconsistency is detected.
     */
    int consistency = 1;
    if (!legacy_check_mode && pi->m->jrank == 0 && pi->m->trank == 0) {
        struct stat sbuf[1];
        rc = stat(Rfilename.c_str(), sbuf);
        if (bw->start == 0) {
            if ((rc == 0 && sbuf->st_size) || errno != ENOENT) {
                fmt::fprintf(stderr, "Refusing to overwrite %s with new random data\n", Rfilename);
                consistency = 0;
            }
        } else {
            if (rc != 0) {
                fmt::fprintf(stderr, "Cannot expand non-existing %s with new random data\n", Rfilename);
                consistency = 0;
            } else if ((size_t) sbuf->st_size != (size_t) bw->start * R_coeff_size) {
                fmt::fprintf(stderr, "Cannot expand %s (%u entries) starting at position %u\n", Rfilename, (unsigned int) (sbuf->st_size / R_coeff_size), bw->start);
                consistency = 0;
            }
        }
        rc = stat(Tfilename.c_str(), sbuf);
        if (bw->start == 0) {
            if ((rc == 0 && sbuf->st_size) || errno != ENOENT) {
                fmt::fprintf(stderr, "Refusing to overwrite %s with new random data\n", Tfilename);
                consistency = 0;
            }
        } else {
            if (rc != 0) {
                fmt::fprintf(stderr, "File %s not found, cannot expand check data\n", Tfilename);
                consistency = 0;
            } else if ((size_t) sbuf->st_size != T_coeff_size) {
                fmt::fprintf(stderr, "File %s has wrong size (%zu != %zu), cannot expand check data\n", Tfilename, (size_t) sbuf->st_size, T_coeff_size);
                consistency = 0;
            }
        }
    }
    pi_bcast(&consistency, 1, BWC_PI_INT, 0, 0, pi->m);
    if (!consistency) pi_abort(EXIT_FAILURE, pi->m);
    /* }}} */

    /* {{{ create or load T, based on the random seed. */
    if (!legacy_check_mode) {
        /* When start > 0, the call below does not care about the data it
         * generates, it only cares about the side effect to the random
         * state. We do it just in order to keep the random state
         * synchronized compared to what would have happened if we
         * started with start=0. It's cheap enough anyway.
         *
         * (also, random generation matters only at the leader node)
         */
        A->vec_random(A, Tdata, bw->m, rstate);
        if (bw->start == 0) {
            if (pi->m->jrank == 0 && pi->m->trank == 0) {
                FILE * Tfile = fopen(Tfilename.c_str(), "wb");
                rc = fwrite(Tdata, A->vec_elt_stride(A, bw->m), 1, Tfile);
                ASSERT_ALWAYS(rc == 1);
                fclose(Tfile);
                if (tcan_print) fmt::printf("Saved %s\n", Tfilename);
            }
        } else {
            if (pi->m->jrank == 0 && pi->m->trank == 0) {
                FILE * Tfile = fopen(Tfilename.c_str(), "rb");
                rc = fread(Tdata, A->vec_elt_stride(A, bw->m), 1, Tfile);
                ASSERT_ALWAYS(rc == 1);
                fclose(Tfile);
                if (tcan_print) fmt::printf("loaded %s\n", Tfilename);
            }
        }
        pi_bcast(Tdata, bw->m, A_pi, 0, 0, pi->m);
    }
    /* }}} */
    if (legacy_check_mode) {
        void * Rdata;
        cheating_vec_init(A, &Rdata, nchecks);
        for(int k = 0 ; k < bw->start ; k++) {
            /* same remark as above */
            A->vec_random(A, Rdata, nchecks, rstate);
        }
        cheating_vec_clear(A, &Rdata, nchecks);
    }

    /* {{{ Set file pointer for R (append-only) */
    if (!legacy_check_mode && pi->m->jrank == 0 && pi->m->trank == 0) {
        Rfile = fopen(Rfilename.c_str(), "ab");
    }
    /* }}} */

    /* {{{ create initial Cv and Cd, or load them if start>0 */
    if (bw->start == 0) {
        if (tcan_print)
            printf("We have start=0: creating Cv0-%u.0 as an expanded copy of X*T\n", nchecks);
        uint32_t * gxvecs = NULL;
        unsigned int nx = 0;

        if (!fake) {
            load_x(&gxvecs, bw->m, &nx, pi);
        } else {
            set_x_fake(&gxvecs, bw->m, &nx, pi);
        }

        mmt_full_vec_set_zero(my);
        ASSERT_ALWAYS(bw->m % nchecks == 0);

        if (legacy_check_mode) {
            mmt_vec_set_x_indices(my, gxvecs, MIN(nchecks, bw->m), nx);
            mmt_vec_save(my, "C%u-%u.0", unpadded, 0);
        } else {
            for(int c = 0 ; c < bw->m ; c += nchecks) {
                mmt_vec_set_x_indices(dvec, gxvecs + c * nx, nchecks, nx);
                AxA->addmul_tiny(A, A,
                        mmt_my_own_subvec(my),
                        mmt_my_own_subvec(dvec),
                        A->vec_subvec(A, Tdata, c),
                        mmt_my_own_size_in_items(my));
            }
            /* addmul_tiny degrades consistency ! */
            my->consistency = 1;
            mmt_vec_broadcast(my);
            mmt_vec_save(my, "Cv%u-%u.0", unpadded, 0);
        }

        free(gxvecs);

        mmt_full_vec_set_zero(dvec);
    } else {
        int ok;

        if (legacy_check_mode) {
            ok = mmt_vec_load(my,   "C%u-%u.{}"_format(bw->start), unpadded, 0);
            ASSERT_ALWAYS(ok);
        } else {
            ok = mmt_vec_load(my,   "Cv%u-%u.{}"_format(bw->start), unpadded, 0);
            ASSERT_ALWAYS(ok);
            ok = mmt_vec_load(dvec, "Cd%u-%u.{}"_format(bw->start), unpadded, 0);
            ASSERT_ALWAYS(ok);
        }
    }
    /* }}} */

    /* {{{ adjust the list of check stops according to the check_stops
     * and interval parameters
     */
    serialize_threads(pi->m);
    if (pi->m->trank == 0) {
        /* the bw object is global ! */
        bw_set_length_and_interval_krylov(bw, mmt->n0);
    }
    serialize_threads(pi->m);
    ASSERT_ALWAYS(bw->end % bw->interval == 0);

    int interval_already_in_check_stops = 0;
    for(int i = 0 ; i < bw->number_of_check_stops ; i++) {
        if (bw->check_stops[i] == bw->interval) {
            interval_already_in_check_stops = 1;
            break;
        }
    }
    if (serialize_threads(pi->m)) {
        if (!interval_already_in_check_stops) {
            ASSERT_ALWAYS(bw->number_of_check_stops < MAX_NUMBER_OF_CHECK_STOPS - 1);
            bw->check_stops[bw->number_of_check_stops++] = bw->interval;
        }
        std::sort(bw->check_stops, bw->check_stops + bw->number_of_check_stops);
    }
    serialize_threads(pi->m);

    if (tcan_print) {
        printf("Computing trsp(x)*M^k for check stops k=");
        for(int s = 0 ; s < bw->number_of_check_stops ; s++) {
            int next = bw->check_stops[s];
            if (s) printf(",");
            printf("%d", next);
        }
        printf("\n");
    }
    serialize(pi->m);
    /* }}} */

    // {{{ kill the warning about wrong spmv direction
    for(int i = 0 ; i < mmt->nmatrices ; i++) {
        mmt->matrices[i]->mm->iteration[!bw->dir] = INT_MIN;
    }
    // }}}

    int k = bw->start;
    for(int s = 0 ; s < bw->number_of_check_stops ; s++) {
        serialize(pi->m);
        int next = bw->check_stops[s];
        if (next < k) {
            /* This may happen when start is passed and is beyond the
             * first check stop.
             */
            continue;
        }
        mmt_vec_twist(mmt, my);
        mmt_vec_twist(mmt, dvec);

        /* Allocate an area in memory where we store the random
         * coefficient. We do so in order to avoid the situation where an
         * interrupted execution of the binary has already committed to
         * disk the random coefficients (possibly cut halfway because of
         * stdio buffering), while no Cd file has been written yet.
         * Therefore this is mostly a matter of consistency.
         */
        void * Rdata_stream = NULL;
        int k0 = k;
        if (!legacy_check_mode && (next - k0)) {
            cheating_vec_init(A, &Rdata_stream, nchecks * (next - k0));
            A->vec_set_zero(A, Rdata_stream, nchecks * (next - k0));
            A->vec_random(A, Rdata_stream, nchecks * (next - k0), rstate);
            pi_bcast(Rdata_stream, nchecks * (next - k0), A_pi, 0, 0, pi->m);
        }
    
        for( ; k < next ; k++) {
            /* new random coefficient in R */
            if (!legacy_check_mode) {
                void * Rdata = A->vec_subvec(A, Rdata_stream, (k-k0) * nchecks);
                /* At this point Rdata should be consistent across all
                 * threads */
                AxA->addmul_tiny(A, A,
                        mmt_my_own_subvec(dvec),
                        mmt_my_own_subvec(my),
                        Rdata,
                        mmt_my_own_size_in_items(my));
            }
            /* addmul_tiny degrades consistency ! */
            dvec->consistency = 1;
            mmt_vec_broadcast(dvec);
            pi_log_op(mmt->pi->m, "iteration %d", k);
            matmul_top_mul(mmt, myy, NULL);

            if (tcan_print) {
                putchar('.');
                fflush(stdout);
            }
        }
        serialize(pi->m);
        serialize_threads(mmt->pi->m);
        mmt_vec_untwist(mmt, my);
        mmt_vec_untwist(mmt, dvec);

        if (legacy_check_mode) {
            mmt_vec_save(my,   "C%u-%u.{}"_format(k), unpadded, 0);
        } else {
            mmt_vec_save(my,   "Cv%u-%u.{}"_format(k), unpadded, 0);
            mmt_vec_save(dvec, "Cd%u-%u.{}"_format(k), unpadded, 0);
            if (pi->m->trank == 0 && pi->m->jrank == 0 && (next - k0)) {
                rc = fwrite(Rdata_stream, A->vec_elt_stride(A, nchecks), next - k0, Rfile);
                ASSERT_ALWAYS(rc == (next - k0));
                rc = fflush(Rfile);
                ASSERT_ALWAYS(rc == 0);
            }
            cheating_vec_clear(A, &Rdata_stream, nchecks * (next - k0));
        }
    }

    cheating_vec_clear(A, &Tdata, bw->m);

    gmp_randclear(rstate);

    mmt_vec_clear(mmt, dvec);
    mmt_vec_clear(mmt, y);
    mmt_vec_clear(mmt, my);
    matmul_top_clear(mmt);

    A->oo_field_clear(A);

    return NULL;
}


int main(int argc, char * argv[])
{
    param_list pl;

    bw_common_init(bw, &argc, &argv);
    param_list_init(pl);
    parallelizing_info_init();

    bw_common_decl_usage(pl);
    parallelizing_info_decl_usage(pl);
    matmul_top_decl_usage(pl);
    /* declare local parameters and switches: none here (so far). */
    param_list_decl_usage(pl, "legacy_check_mode", "generate check data for legacy mode");

    bw_common_parse_cmdline(bw, pl, &argc, &argv);

    param_list_remove_key(pl, "interleaving");

    bw_common_interpret_parameters(bw, pl);
    parallelizing_info_lookup_parameters(pl);
    matmul_top_lookup_parameters(pl);
    /* interpret our parameters */
    param_list_parse_int(pl, "legacy_check_mode", &legacy_check_mode);

    catch_control_signals();

    if (param_list_warn_unused(pl)) {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (!rank) param_list_print_usage(pl, bw->original_argv[0], stderr);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    pi_go(sec_prog, pl, 0);

    parallelizing_info_finish();
    param_list_clear(pl);
    bw_common_clear(bw);
    return 0;
}

