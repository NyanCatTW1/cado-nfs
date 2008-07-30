/* header file for common routines to polyselect and kleinjung */

#include <float.h>  /* for DBL_MAX */
#include <string.h>

#if 1 /* use L2-norm with integral over whole sieving region */
#define LOGNORM  L2_lognorm
#define SKEWNESS L2_skewness
#else /* use 1-norm relative to coefficients */
#define LOGNORM  l1_lognorm
#define SKEWNESS l1_skewness
#endif

#define SKEWNESS_DEFAULT_PREC 10

#define MAX_ROTATE 65536 /* bound on rotation, to avoid overflow
                            (but this should not happen in practice) */

/* prime bounds for the computation of alpha */
#define ALPHA_BOUND_SMALL  100
#define ALPHA_BOUND       2000

#define mpz_add_si(a,b,c)                       \
  if (c >= 0) mpz_add_ui (a, b, c);             \
  else mpz_sub_ui (a, b, -(c))

#define mpz_submul_si(a,b,c)                    \
  if (c >= 0) mpz_submul_ui (a, b, c);          \
  else mpz_addmul_ui (a, b, -(c))
  
mpz_t* alloc_mpz_array   (int);
mpz_t* realloc_mpz_array (mpz_t *, int, int);
void   clear_mpz_array   (mpz_t *, int);
void mpz_ndiv_qr (mpz_t, mpz_t, mpz_t, mpz_t);
void generate_base_mb (cado_poly, mpz_t, mpz_t);
double l1_skewness (mpz_t*, int, int);
double l1_lognorm (mpz_t*, unsigned long, double);
double L2_lognorm (mpz_t*, unsigned long, double);
double L2_skewness (mpz_t*, int, int);
/* rotation */
double get_alpha (mpz_t*, const int, unsigned long);
void discriminant (mpz_t, mpz_t*, const int);
double rotate (mpz_t*, int, unsigned long, mpz_t, mpz_t, int);
void print_poly (FILE*, cado_poly, int, char**, double, int);

/********************* data structures for first phase ***********************/

typedef struct {
  /* the linear polynomial is b*x-m, with lognorm logmu */
  mpz_t b;
  mpz_t m;
  double logmu;
} m_logmu_t;

m_logmu_t* m_logmu_init (unsigned long);
void m_logmu_clear (m_logmu_t*, unsigned long);
int m_logmu_insert (m_logmu_t*, unsigned long, unsigned long*, mpz_t, mpz_t,
                    double, char*, int);

