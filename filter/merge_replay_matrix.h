#ifndef SPARSE_MAT_H_
#define SPARSE_MAT_H_

#include <stddef.h>    // for NULL
#include <stdint.h>    // for uint64_t, uint32_t

#include "typedefs.h"
#include "filter_config.h"
#include "merge_types.h"  // typerow_t, col_weight_t, ...
#include "merge_heap.h"  // heapctx_t

#define TRACE_COL -1 // 253224 // 231 // put to -1 if not...!
#define TRACE_ROW -1 // 59496 // put to -1 if not...!

#ifdef FOR_DL

#define rowCell(row, k) (row)[k].id
#define rowFullCell(row, k) (row)[k]
#define setRawCell(row, k, v, c) (row)[k] = (ideal_merge_t) {.id = (v), .e = (c)}
#define setCell(row, k, v, c) (row)[k] = (ideal_merge_t) {.id = (v), .e = (c)}
#define compressRow(row, buf, n) memcpy(row,buf,(n+1)*sizeof (typerow_t))

#else

#define setRawCell(row, k, v, c) (row)[k] = (v)
#define rowFullCell(row, k) rowCell((row), (k))
#define rowCell(row, k) (row)[k]
#define setCell(row, k, v, c) (row)[k] = (v)
#define compressRow(row, buf, n) memcpy((row),buf,((n)+1)*sizeof (typerow_t))
#endif


/* rows correspond to relations, and columns to primes (or prime ideals) */
typedef struct {
  heapctx_t heap;
  int verbose;         /* verbose level */
  uint64_t nrows;
  uint64_t ncols;
  uint64_t rem_nrows;  /* number of remaining rows */
  uint64_t rem_ncols;  /* number of remaining columns, including the buried
                          columns */
  typerow_t **rows;    /* rows[i][k] contains indices of an ideal of row[i]
                          with 1 <= k <= rows[i][0] */
                       /* FOR_DL: struct containing also the exponent */
  col_weight_t *wt;    /* weight w of column j, if w <= cwmax,
                          else 0 for a deleted column */
                       /* 8 bits is sufficient as we only want precise weight
                          for column of low weight. If the weight exceeds
                          255, we saturate at 255 */
  uint64_t skip;       /* number of buried/skipped columns of smaller index */
  uint64_t weight;     /* number of non-zero coefficients in the active part */
  uint64_t tot_weight; /* Initial total number of non-zero coefficients */
  int cwmax;           /* bound on weight of j to enter the SWAR structure */
  index_t Rn;          /* number of rows in R (potential merges) */
  index_t *Rp, *Ri;    /* (part of) transposed matrix in CSR format:
                          Rp has size Rn + 1, and Ri has size nnz where nnz
                          is the number of elements in the transposed matrix.
                          Column j has Rp[j+1] - Rp[j] elements, which are
                          located from Ri[Rp[j]] to Ri[Rp[j+1]-1]. */
  index_t *Rq, *Rqinv; /* row i of R corresponds to column Rqinv[i] of [self];
                          column j of [self] corresponds to row Rq[j] of R. */
  index_t *p;          /* internal renumbering function: the original ideal
                          corresponding to column j is p[j] >= j */
  uint64_t initial_ncols; /* number of columns in the purge file */
} filter_matrix_t;

#ifdef __cplusplus
extern "C" {
#endif

#define compute_WN(mat) ((double) (mat)->rem_nrows * (double) (mat)->weight)
#define compute_WoverN(mat) (((double)(mat)->weight)/((double)(mat)->rem_nrows))

void initMat(filter_matrix_t *, uint32_t);
void clearMat (filter_matrix_t *mat);

void print_row(filter_matrix_t *mat, index_t i);

#define matCell(mat, i, k) rowCell(mat->rows[i], k)
#define rowLength(row, i) rowCell(row[i], 0)
#define matLengthRow(mat, i) matCell(mat, i, 0)
#define isRowNull(mat, i) ((mat)->rows[(i)] == NULL)

#define SPARSE_ITERATE(mat, i, k) for((k)=1; (k)<=lengthRow((mat),(i)); (k)++)

#ifdef __cplusplus
}
#endif

#endif	/* SPARSE_MAT_H_ */
