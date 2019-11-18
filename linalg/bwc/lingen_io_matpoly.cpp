#include "cado.h"
#include "lingen_io_matpoly.hpp"
#include "gmp-hacks.h"
#include <unistd.h>
#ifdef HAVE_OPENMP
#include <omp.h>
#endif


// constexpr const unsigned int simd = matpoly::over_gf2 ? ULONG_BITS : 1;
constexpr const unsigned int splitwidth = matpoly::over_gf2 ? 64 : 1;

/* {{{ I/O helpers */

/* This is an indication of the number of bytes we read at a time for A
 * (input) and F (output) */
unsigned int io_matpoly_block_size = 1 << 20;

void lingen_io_matpoly_decl_usage(cxx_param_list & pl)
{
    param_list_decl_usage(pl, "io-block-size",
            "chunk size for reading the input or writing the output");
}

void lingen_io_matpoly_lookup_parameters(cxx_param_list & pl)
{
    param_list_lookup_string(pl, "io-block-size");
}

void lingen_io_matpoly_interpret_parameters(cxx_param_list & pl)
{
    param_list_parse_uint(pl, "io-block-size", &(io_matpoly_block_size));
}

/* {{{ matpoly_write
 * writes some of the matpoly data to f, either in ascii or binary
 * format. This can be used to write only part of the data (degrees
 * [k0..k1[). Returns the number of coefficients (i.e., matrices, so at
 * most k1-k0) successfully written, or
 * -1 on error (e.g. when some matrix was only partially written).
 */

#ifndef SELECT_MPFQ_LAYER_u64k1
int matpoly_write(abdst_field ab, std::ostream& os, matpoly const & M, unsigned int k0, unsigned int k1, int ascii, int transpose)
{
    unsigned int m = transpose ? M.n : M.m;
    unsigned int n = transpose ? M.m : M.n;
    ASSERT_ALWAYS(k0 == k1 || (k0 < M.get_size() && k1 <= M.get_size()));
    for(unsigned int k = k0 ; k < k1 ; k++) {
        int err = 0;
        int matnb = 0;
        for(unsigned int i = 0 ; !err && i < m ; i++) {
            for(unsigned int j = 0 ; !err && j < n ; j++) {
                absrc_elt x;
                x = transpose ? M.coeff(j, i, k) : M.coeff(i, j, k);
                if (ascii) {
                    if (j) err = !(os << " ");
                    if (!err) err = !(abcxx_out(ab, os, x));
                } else {
                    err = !(os.write((const char *) x, (size_t) abvec_elt_stride(ab, 1)));
                }
                if (!err) matnb++;
            }
            if (!err && ascii) err = !(os << "\n");
        }
        if (ascii) err = err || !(os << "\n");
        if (err) {
            return (matnb == 0) ? (int) (k - k0) : -1;
        }
    }
    return k1 - k0;
}
#else
int matpoly_write(abdst_field, std::ostream& os, matpoly const & M, unsigned int k0, unsigned int k1, int ascii, int transpose)
{
    unsigned int m = M.m;
    unsigned int n = M.n;
    ASSERT_ALWAYS(k0 == k1 || (k0 < M.get_size() && k1 <= M.get_size()));
    ASSERT_ALWAYS(m % ULONG_BITS == 0);
    ASSERT_ALWAYS(n % ULONG_BITS == 0);
    size_t ulongs_per_mat = m * n / ULONG_BITS;
    std::vector<unsigned long> buf(ulongs_per_mat);
    for(unsigned int k = k0 ; k < k1 ; k++) {
        buf.assign(ulongs_per_mat, 0);
        bool err = false;
        size_t kq = k / ULONG_BITS;
        size_t kr = k % ULONG_BITS;
        unsigned long km = 1UL << kr;
        if (!transpose) {
            for(unsigned int i = 0 ; i < m ; i++) {
                unsigned long * v = &(buf[i * (n / ULONG_BITS)]);
                for(unsigned int j = 0 ; j < n ; j++) {
                    unsigned int jq = j / ULONG_BITS;
                    unsigned int jr = j % ULONG_BITS;
                    unsigned long bit = (M.part(i, j)[kq] & km) != 0;
                    v[jq] |= bit << jr;
                }
            }
        } else {
            for(unsigned int j = 0 ; j < n ; j++) {
                unsigned long * v = &(buf[j * (m / ULONG_BITS)]);
                for(unsigned int i = 0 ; i < m ; i++) {
                    unsigned int iq = i / ULONG_BITS;
                    unsigned int ir = i % ULONG_BITS;
                    unsigned long bit = (M.part(i, j)[kq] & km) != 0;
                    v[iq] |= bit << ir;
                }
            }
        }
        if (ascii) {
            /* do we have an endian-robust wordsize-robust convention for
             * printing bitstrings in hex ?
             *
             * it's not even clear that we should care -- after all, as
             * long as mksol follows a consistent convention too, we
             * should be fine.
             */
            abort();
        } else {
            err = !(os.write((const char*) &buf[0], ulongs_per_mat * sizeof(unsigned long)));
        }
        if (err) return -1;
    }
    return k1 - k0;
}
#endif
/* }}} */

#define VOID_POINTER_ADD(x, k) (((char*)(x))+(k))

/* fw must be an array of ofstreams of exactly the same size as the
 * matrix to be written.
 */
int matpoly_write_split(abdst_field ab MAYBE_UNUSED, std::vector<std::ofstream> & fw, matpoly const & M, unsigned int k0, unsigned int k1, int ascii)
{
    ASSERT_ALWAYS(k0 == k1 || (k0 < M.get_size() && k1 <= M.get_size()));
#ifdef SELECT_MPFQ_LAYER_u64k1
    size_t ulongs_per_mat = splitwidth * splitwidth / ULONG_BITS;
    std::vector<unsigned long> buf(ulongs_per_mat);
#endif
    for(unsigned int k = k0 ; k < k1 ; k++) {
        int err = 0;
        int matnb = 0;
        for(unsigned int i = 0 ; !err && i < M.m ; i += splitwidth) {
            for(unsigned int j = 0 ; !err && j < M.n ; j += splitwidth) {
                std::ostream& os = fw[i/splitwidth*M.n/splitwidth+j/splitwidth];
#ifndef SELECT_MPFQ_LAYER_u64k1
                absrc_elt x = M.coeff(i, j, k);
                if (ascii) {
                    err = !(abcxx_out(ab, os, x));
                    if (!err) err = !(os << "\n");
                } else {
                    err = !(os.write((const char *) x, (size_t) abvec_elt_stride(ab, 1)));
                }
#else
                if (ascii)
                    abort();
                buf.assign(ulongs_per_mat, 0);
                size_t kq = k / ULONG_BITS;
                size_t kr = k % ULONG_BITS;
                for(unsigned int di = 0 ; di < splitwidth ; di++) {
                    unsigned int ii = i + di;
                    unsigned int ulongs_per_row = splitwidth / ULONG_BITS;
                    for(unsigned int dj0 = 0 ; dj0 < ulongs_per_row ; dj0++) {
                        for(unsigned int dj = 0 ; dj < ULONG_BITS ; dj++) {
                            unsigned int jj = j + dj0 * ULONG_BITS + dj;
                            const unsigned long * mij = M.part(ii, jj);
                            unsigned long bit = (mij[kq] >> kr) & 1;
                            buf[di * ulongs_per_row + dj0] ^= bit << dj;
                        }
                    }
                }
                err = !(os.write((const char *) &buf[0], ulongs_per_mat * sizeof(unsigned long)));
#endif
                if (!err) matnb++;
            }
        }
        if (err) {
            return (matnb == 0) ? (int) (k - k0) : -1;
        }
    }
    return k1 - k0;
}
/* }}} */

/* {{{ matpoly_read
 * reads some of the matpoly data from f, either in ascii or binary
 * format. This can be used to parse only part of the data (degrees
 * [k0..k1[, k1 being an upper bound). Returns the number of coefficients
 * (i.e., matrices, so at most k1-k0) successfully read, or
 * -1 on error (e.g. when some matrix was only partially read).
 *
 * Note that the matrix must *not* be in pre-init state. It must have
 * been already allocated.
 */

#ifndef SELECT_MPFQ_LAYER_u64k1
int matpoly_read(abdst_field ab, FILE * f, matpoly & M, unsigned int k0, unsigned int k1, int ascii, int transpose)
{
    ASSERT_ALWAYS(!M.check_pre_init());
    unsigned int m = transpose ? M.n : M.m;
    unsigned int n = transpose ? M.m : M.n;
    ASSERT_ALWAYS(k0 == k1 || (k0 < M.get_size() && k1 <= M.get_size()));
    mpz_srcptr pz = abfield_characteristic_srcptr(ab);
    std::vector<mp_limb_t> vbuf(SIZ(pz) + 1);
    mp_limb_t * buf = &vbuf[0];
    for(unsigned int k = k0 ; k < k1 ; k++) {
        int err = 0;
        int matnb = 0;
        for(unsigned int i = 0 ; !err && i < m ; i++) {
            for(unsigned int j = 0 ; !err && j < n ; j++) {
                abdst_elt x;
                x = transpose ? M.coeff(j, i, k)
                              : M.coeff(i, j, k);
                if (ascii) {
                    err = abfscan(ab, f, x) == 0;
                } else {
                    err = fread(x, abvec_elt_stride(ab, 1), 1, f) < 1;
                    if (mpn_cmp(x, PTR(pz), SIZ(pz) >= 0)) {
                        mpn_tdiv_qr(buf + SIZ(pz), buf, 0, x, SIZ(pz), PTR(pz), SIZ(pz));
                        mpn_copyi(x, buf, SIZ(pz));
                    }
                }
                if (!err) matnb++;
            }
        }
        if (err) {
            if (matnb == 0) 
                return (int) (k - k0);
            fprintf(stderr, "I/O error after %d,%d coefficients (matrix size %u*%u)\n",
                    (int) (k-k0), matnb, m, n);
            return -1;
        }
    }
    return k1 - k0;
}
#else
int matpoly_read_inner(abdst_field, FILE * f, matpoly & M, unsigned int k0, unsigned int k1, int ascii, int transpose, off_t base, unsigned int batch = 1)
{
    unsigned int m = M.m;
    unsigned int n = M.n;
    ASSERT_ALWAYS(k0 == k1 || (k0 < M.get_size() && k1 <= M.get_size()));
    ASSERT_ALWAYS(m % ULONG_BITS == 0);
    ASSERT_ALWAYS(n % ULONG_BITS == 0);
    size_t ulongs_per_mat = m * n / ULONG_BITS;
    batch = MIN(batch, k1 - k0);
    std::vector<unsigned long> buf(ulongs_per_mat * batch);
    for(unsigned int k = k0 ; k < k1 ; k+=batch) {
        if (k + batch > k1) batch = k1 - k;
        unsigned int good;
        if (ascii) {
            /* do we have an endian-robust wordsize-robust convention for
             * printing bitstrings in hex ?
             *
             * it's not even clear that we should care -- after all, as long as
             * mksol follows a consistent convention too, we should be fine.
             */
            abort();
        } else {
            int rc;
            if (base < 0) {
                rc = fread(&buf[0], sizeof(unsigned long), ulongs_per_mat * batch, f);
                if (rc != (int) (ulongs_per_mat * batch) && ferror(f))
                    return k - k0;
            } else {
                /* use pread -- good for multithreading */
                size_t one = ulongs_per_mat * sizeof(unsigned long);
                off_t off = base + (k - k0) * one;
                ssize_t r = pread(fileno(f), &buf[0], one * batch, off);
                if (r < 0) rc = 0;
                else rc = r / sizeof(unsigned long);
            }
            good = rc / ulongs_per_mat;
        }
        for(unsigned int b = 0 ; b < good ; b++) {
            size_t kq = (k + b) / ULONG_BITS;
            size_t kr = (k + b) % ULONG_BITS;
            if (!transpose) {
                for(unsigned int i = 0 ; i < m ; i++) {
                    unsigned long * v = &(buf[b * ulongs_per_mat + i * (n / ULONG_BITS)]);
                    for(unsigned int j = 0 ; j < n ; j++) {
                        unsigned int jq = j / ULONG_BITS;
                        unsigned int jr = j % ULONG_BITS;
                        unsigned long bit = (v[jq] >> jr) & 1;
                        M.part(i, j)[kq] &= ~(1UL << kr);
                        M.part(i, j)[kq] |= bit << kr;
                    }
                }
            } else {
                for(unsigned int j = 0 ; j < n ; j++) {
                    unsigned long * v = &(buf[b * ulongs_per_mat + j * (m / ULONG_BITS)]);
                    for(unsigned int i = 0 ; i < m ; i++) {
                        unsigned int iq = i / ULONG_BITS;
                        unsigned int ir = i % ULONG_BITS;
                        unsigned long bit = (v[iq] >> ir) & 1;
                        M.part(i, j)[kq] &= ~(1UL << kr);
                        M.part(i, j)[kq] |= bit << kr;
                    }
                }
            }
        }
        if (good < batch) {
            return k - k0 + good;
        }
    }
    return k1 - k0;
}
int matpoly_read(abdst_field ab, FILE * f, matpoly & M, unsigned int k0, unsigned int k1, int ascii, int transpose)
{
    int rc = 0;
    if (k0 % ULONG_BITS) {
        unsigned int fk0 = MIN(k1, k0 + ULONG_BITS - (k0 % ULONG_BITS));
        rc += matpoly_read_inner(ab, f, M, k0, fk0, ascii, transpose, -1);
        if (rc < (int) (fk0 - k0) || fk0 == k1)
            return rc;
        return rc + matpoly_read(ab, f, M, fk0, k1, ascii, transpose);
    }
    if (k1 % ULONG_BITS) {
        unsigned int fk1 = MAX(k0, k1 - (k1 % ULONG_BITS));
        if (k0 < fk1) {
            /* recurse and to the bulk of the processing on aligned
             * values */
            rc += matpoly_read(ab, f, M, k0, fk1, ascii, transpose);
            if (rc < (int) (fk1 - k0))
                return rc;
        }
        return rc + matpoly_read_inner(ab, f, M, fk1, k1, ascii, transpose, -1);
    }

    ASSERT_ALWAYS(!(k0 % ULONG_BITS));
    ASSERT_ALWAYS(!(k1 % ULONG_BITS));

    off_t pos0 = ftell(f);
    size_t ulongs_per_mat = M.m * M.n / ULONG_BITS;
    size_t one = ulongs_per_mat * sizeof(unsigned long);

    unsigned int nth = 1;
#ifdef HAVE_OPENMP
    nth = omp_get_max_threads();
#endif
    unsigned int dk = ((k1 - k0)/ULONG_BITS) / nth;
    unsigned int mk = ((k1 - k0)/ULONG_BITS) % nth;
#ifdef HAVE_OPENMP
#pragma omp parallel num_threads(nth)
#endif
    {
        unsigned int idx = 0;
#ifdef HAVE_OPENMP
        idx = omp_get_thread_num();
#endif
        unsigned int lk0 = k0 / ULONG_BITS + idx * dk + MIN(idx, mk);
        unsigned int lk1 = lk0 + dk + (idx < mk);
        lk0 *= ULONG_BITS;
        lk1 *= ULONG_BITS;

        off_t base = pos0 + (lk0 - k0) * one;
        int my_rc = matpoly_read_inner(ab, f, M, lk0, lk1, ascii, transpose, base, UINT_MAX);

#ifdef HAVE_OPENMP
#pragma omp critical
#endif
        rc += my_rc;
    }

    fseek(f, pos0 + rc * one, SEEK_SET);
    return rc;
}

#endif
/* }}} */

