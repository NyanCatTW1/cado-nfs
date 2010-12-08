#include "cado.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "bwc_config.h"
#include "macros.h"
#include "filenames.h"
#include "bw-common.h"
#include "params.h"
#include "balancing.h"

char * ifile;
char * ofile_fmt;

/* Maximum number of possible splits */
#define MAXSPLITS 16

/* splits for the different sites */
int splits[MAXSPLITS + 1];

int split_y = 0;
int split_f = 0;
int force = 0;

void usage()
{
    fprintf(stderr, "Usage: ./splits <options> [--split-y|--split-f] splits=0,<n1>,<n2>,...\n");
    fprintf(stderr, "%s", bw_common_usage_string());
    fprintf(stderr, "Relevant options here: wdir cfg n\n");
    fprintf(stderr, "Note: data files must be found in wdir !\n");
    exit(1);
}

int main(int argc, char * argv[])
{
    FILE ** files;
    balancing bal;

    param_list pl;
    param_list_init(pl);
    param_list_configure_knob(pl, "--split-y", &split_y);
    param_list_configure_knob(pl, "--split-f", &split_f);
    param_list_configure_knob(pl, "--force", &force);
    bw_common_init(bw, pl, &argc, &argv);
    int nsplits;
    nsplits = param_list_parse_int_list(pl, "splits", splits, MAXSPLITS, ",");

    const char * balancing_filename = param_list_lookup_string(pl, "balancing");
    if (!balancing_filename) {
        fprintf(stderr, "Required argument `balancing' is missing\n");
        usage();
    }
    balancing_read_header(bal, balancing_filename);

    if (param_list_warn_unused(pl)) usage();
    if (split_f + split_y != 1) {
        fprintf(stderr, "Please select one of --split-y or --split-f\n");
        usage();
    }
    if (nsplits == 0) {
        fprintf(stderr, "Please indicate the splitting points\n");
        usage();
    }

    files = malloc(nsplits * sizeof(FILE *));

    for(int i = 0 ; i < nsplits ; i++) {
        ASSERT_ALWAYS((i == 0) == (splits[i] == 0));
        ASSERT_ALWAYS((i == 0) || (splits[i-1] < splits[i]));
    }
    if (splits[nsplits-1] != bw->n) {
        fprintf(stderr, "last split does not coincide with configured n\n");
        exit(1);
    }

    for(int i = 0 ; i < nsplits ; i++) {
        ASSERT_ALWAYS(splits[i] % CHAR_BIT == 0);
        splits[i] /= CHAR_BIT;
    }

    nsplits--;

    int rc;

    /* prepare the file names */
    if (split_y) {
        rc = asprintf(&ifile, COMMON_VECTOR_ITERATE_PATTERN,
                Y_FILE_BASE, 0, bal->h->checksum);
        rc = asprintf(&ofile_fmt, COMMON_VECTOR_ITERATE_PATTERN,
                V_FILE_BASE_PATTERN, 0, bal->h->checksum);
    } else {
        ifile = strdup(LINGEN_F_FILE);
        ofile_fmt = strdup(F_FILE_SLICE_PATTERN);
    }

    struct stat sbuf[1];
    stat(ifile, sbuf);

    if ((sbuf->st_size) % splits[nsplits] != 0) {
        fprintf(stderr, 
                "Size of %s (%ld bytes) is not a multiple of %d bytes\n",
                ifile, (long) sbuf->st_size, splits[nsplits]);
    }

    FILE * f = fopen(ifile, "r");
    if (f == NULL) {
        fprintf(stderr,"%s: %s\n", ifile, strerror(errno));
        exit(1);
    }

    if (nsplits == 1) {
        char * fname;
        int i = 0;
        /* Special case ; a hard link is enough */
        rc = asprintf(&fname, ofile_fmt, CHAR_BIT * splits[i], CHAR_BIT * splits[i+1]);
        rc = stat(fname, sbuf);
        if (rc == 0 && !force) {
            fprintf(stderr,"%s already exists\n", fname);
            exit(1);
        }
        if (rc == 0 && force) { unlink(fname); }
        if (rc < 0 && errno != ENOENT) {
            fprintf(stderr,"%s: %s\n", fname, strerror(errno));
            exit(1);
        }
        rc = link(ifile, fname);
        if (rc < 0) {
            fprintf(stderr,"%s: %s\n", fname, strerror(errno));
            exit(1);
        }
        free(fname);
        free(ifile);
        free(ofile_fmt);
        return 0;
    }

    for(int i = 0 ; i < nsplits ; i++) {
        char * fname;
        rc = asprintf(&fname, ofile_fmt, CHAR_BIT * splits[i], CHAR_BIT * splits[i+1]);
        rc = stat(fname, sbuf);
        if (rc == 0 && !force) {
            fprintf(stderr,"%s already exists\n", fname);
            exit(1);
        }
        if (rc == 0 && force) { unlink(fname); }
        if (rc < 0 && errno != ENOENT) {
            fprintf(stderr,"%s: %s\n", fname, strerror(errno));
            exit(1);
        }
        files[i] = fopen(fname, "w");
        if (files[i] == NULL) {
            fprintf(stderr,"%s: %s\n", fname, strerror(errno));
            exit(1);
        }
        free(fname);
    }

    void * ptr = malloc(splits[nsplits]);

    for(;;) {
        rc = fread(ptr, 1, splits[nsplits], f);
        if (rc != splits[nsplits] && rc != 0) {
            fprintf(stderr, "Unexpected short read\n");
            exit(1);
        }
        if (rc == 0)
            break;

        char * q = ptr;

        for(int i = 0 ; i < nsplits ; i++) {
            rc = fwrite(q, 1, splits[i+1]-splits[i], files[i]);
            if (rc != splits[i+1]-splits[i]) {
                fprintf(stderr, "short write\n");
                exit(1);
            }
            q += splits[i+1]-splits[i];
        }
    }

    param_list_clear(pl);
    for(int i = 0 ; i < nsplits ; i++) {
        fclose(files[i]);
    }
    fclose(f);

    free(ifile);
    free(ofile_fmt);
}

