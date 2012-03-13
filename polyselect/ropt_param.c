/**
 * @file ropt_param.c
 * Setting parameters such as sieving bounds. The customizable parameters are in ropt_param.h.
 */

#include "ropt_param.h"

/* Define primes, exp_alpha. */
const unsigned int primes[NP] = {
  2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
  31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
  73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
  127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199
};

/* next_prime_idx[3] = ind(5) = 2 in above table */
const unsigned char next_prime_idx[] = {
  0, /* 0 */
  0, 1, 2, 2, 3, 3, 4, 4, 4, 4, /*1 - 10*/
  5, 5, 6, 6, 6, 6, 7, 7, 8, 8,
  8, 8, 9, 9, 9, 9, 9, 9, 10, 10,
  11, 11, 11, 11, 11, 11, 12, 12, 12, 12,
  13, 13, 14, 14, 14, 14, 15, 15, 15, 15,
  15, 15, 16, 16, 16, 16, 16, 16, 17, 17,
  18, 18, 18, 18, 18, 18, 19, 19, 19, 19,
  20, 20, 21, 21, 21, 21, 21, 21, 22, 22,
  22, 22, 23, 23, 23, 23, 23, 23, 24, 24,
  24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
  26, 26, 27, 27, 27, 27, 28, 28, 29, 29,
  29, 29, 30, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
  32, 32, 32, 32, 32, 32, 33, 33, 34, 34,
  34, 34, 34, 34, 34, 34, 34, 34, 35, 35,
  36, 36, 36, 36, 36, 36, 37, 37, 37, 37,
  37, 37, 38, 38, 38, 38, 39, 39, 39, 39,
  39, 39, 40, 40, 40, 40, 40, 40, 41, 41,
  42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
  43, 43, 44, 44, 44, 44, 45, 45 /* 191 - 198 */
};

/* 2^i for i <= 150. generated by the asymptotic expansion
   of the first order statistics as pointed by Emmanuel Thom\'e */
const double exp_alpha[] = {
  0 ,
  -1.15365373215 ,-1.52232116258 ,-1.84599014685, -2.13527943823, -2.39830159659, /* 2^2, ...*/
  -2.64067276005 ,-2.86635204676 ,-3.0782200169 ,-3.27843684076 ,-3.46866583492 ,
  -3.65021705343 ,-3.8241424823 ,-3.99130105332 ,-4.15240425248 ,-4.30804888816 ,
  -4.45874113786 ,-4.60491453048 ,-4.74694362117 ,-4.88515454833 ,-5.01983329465 ,
  -5.15123223117 ,-5.27957535905 ,-5.40506255088 ,-5.52787301451 ,-5.64816814584 ,
  -5.76609389702 ,-5.88178275645 ,-5.99535541547 ,-6.10692217992 ,-6.21658417274 ,
  -6.32443436393 ,-6.43055845718 ,-6.53503565678 ,-6.63793933391 ,-6.73933760789 ,
  -6.83929385532 ,-6.93786715768 ,-7.03511269625 ,-7.13108210169 ,-7.22582376443 ,
  -7.3193831112 ,-7.41180285201 ,-7.50312320134 ,-7.59338207681 ,-7.68261527801 ,
  -7.77085664785 ,-7.85813821855 ,-7.94449034388 ,-8.02994181933 ,-8.11451999143 ,
  -8.19825085747 ,-8.28115915656 ,-8.36326845294 ,-8.44460121241 ,-8.52517887245 ,
  -8.60502190671 ,-8.68414988441 ,-8.76258152512 ,-8.84033474938 ,-8.9174267255 ,
  -8.99387391288 ,-9.0696921022 ,-9.14489645276 ,-9.21950152716 ,-9.29352132356 ,
  -9.36696930575 ,-9.43985843123 ,-9.51220117738 ,-9.58400956592 ,-9.65529518581 ,
  -9.72606921473 ,-9.79634243911 ,-9.86612527306 ,-9.93542777601 ,-10.0042596694 ,
  -10.0726303522 ,-10.140548916 ,-10.2080241583 ,-10.2750645962 ,-10.3416784783 ,
  -10.4078737968 ,-10.4736582979 ,-10.5390394928 ,-10.6040246674 ,-10.6686208913 ,
  -10.7328350271 ,-10.7966737386 ,-10.8601434986 ,-10.9232505968 ,-10.9860011466 ,
  -11.0484010921 ,-11.1104562145 ,-11.1721721386 ,-11.233554338 ,-11.2946081414 ,
  -11.3553387375 ,-11.4157511801 ,-11.4758503931 ,-11.535641175 ,-11.5951282035 ,
  -11.6543160394 ,-11.713209131 ,-11.7718118176 ,-11.8301283334 ,-11.888162811 ,
  -11.9459192847 ,-12.0034016939 ,-12.0606138859 ,-12.1175596192 ,-12.1742425661 ,
  -12.2306663156 ,-12.2868343759 ,-12.342750177 ,-12.3984170731 ,-12.4538383449 ,
  -12.5090172017 ,-12.5639567839 ,-12.6186601648 ,-12.6731303524 ,-12.7273702918 ,
  -12.7813828666 ,-12.8351709011 ,-12.8887371614 ,-12.9420843577 ,-12.9952151453 ,
  -13.0481321268 ,-13.1008378527 ,-13.1533348236 ,-13.2056254911 ,-13.2577122596 ,
  -13.3095974869 ,-13.3612834859 ,-13.4127725259 ,-13.4640668332 ,-13.5151685928 ,
  -13.5660799493 ,-13.6168030075 ,-13.6673398341 ,-13.7176924584 ,-13.7678628729 ,
  -13.8178530347 ,-13.8676648661 ,-13.9173002556 ,-13.9667610588 ,-14.0160490987 ,
  -14.0651661673 ,-14.1141140255 ,-14.1628944044 ,-14.211509006 };

/*
  Init rsbound.
*/
void
rsbound_init ( rsbound_t rsbound )
{
  mpz_init_set_ui (rsbound->Umax, 0UL);
  mpz_init_set_ui (rsbound->Umin, 0UL);
  mpz_init_set_ui (rsbound->Vmax, 0UL);
  mpz_init_set_ui (rsbound->Vmin, 0UL);
  mpz_init_set_ui (rsbound->A, 0UL);
  mpz_init_set_ui (rsbound->B, 0UL);
  mpz_init_set_ui (rsbound->MOD, 0UL);

  rsbound->Amax = rsbound->Amin = 0;
  rsbound->Bmax = rsbound->Bmin = 0;
}

/*
  Free rsbound.
*/
void
rsbound_free ( rsbound_t rsbound )
{
  mpz_clear (rsbound->Umax);
  mpz_clear (rsbound->Umin);
  mpz_clear (rsbound->Vmax);
  mpz_clear (rsbound->Vmin);
  mpz_clear (rsbound->A);
  mpz_clear (rsbound->B);
  mpz_clear (rsbound->MOD);
}


/*
  Given rsparam->sizebound_ratio_rs and polynomial, set the size for
  sieving region. Set the Amax, Amin, Amax, Amin.
*/
void
rsbound_setup_AB_bound ( rsbound_t rsbound,
                         rsparam_t rsparam,
                         param_t param,
                         mpz_t mod,
                         int verbose )
{
  /* verbose == 1 means "tune mode";
     verbose == 0 means "polyselect2 mode"; */
  if (verbose == 0) {
    unsigned long len;
    mpz_t q;
    mpz_init (q);
    mpz_fdiv_q (q, rsparam->global_v_bound_rs, mod);
    len =  mpz_get_ui (q);
    rsbound->Bmax = ( (len > MAX_SIEVEARRAY_SIZE) ? MAX_SIEVEARRAY_SIZE : (long) len);
    mpz_set_ui (q, rsparam->global_u_bound_rs);
    mpz_fdiv_q (q, q, mod);
    len =  mpz_get_ui (q);
    rsbound->Amax = ( (len > 128) ? 128 : (long) len);
    mpz_clear (q);
  }
  else if (verbose == 2) {
    if (param->s2_Amax >= 0 && param->s2_Bmax > 0) {
      rsbound->Amax = param->s2_Amax;
      rsbound->Bmax = param->s2_Bmax;
    }
    else {
      unsigned long len;
      mpz_t q;
      mpz_init (q);
      mpz_fdiv_q (q, rsparam->global_v_bound_rs, mod);
      len =  mpz_get_ui (q);
      rsbound->Bmax = ( (len > MAX_SIEVEARRAY_SIZE) ? MAX_SIEVEARRAY_SIZE : (long) len);
      mpz_set_ui (q, rsparam->global_u_bound_rs);
      mpz_fdiv_q (q, q, mod);
      len =  mpz_get_ui (q);
      rsbound->Amax = ( (len > 128) ? 128 : (long) len);
      mpz_clear (q);
    }
  }
  /* Tune mode. For speed purpose, choose smaller sieving range (compared to TUNE_SIEVEARRAY_SIZE). */
  else {
    rsbound->Amax = 0;
    unsigned long len;
    mpz_t q, v;
    mpz_init (q);
    mpz_init (v);
    mpz_fdiv_q (q, rsparam->global_v_bound_rs, mod);
    len =  mpz_get_ui (q);
    rsbound->Bmax = ( (len > TUNE_SIEVEARRAY_SIZE) ? TUNE_SIEVEARRAY_SIZE : (long) len);
    mpz_clear (q);
    mpz_clear (v);
  }

  rsbound->Bmin = -rsbound->Bmax;
  rsbound->Amin = -rsbound->Amax;
}


/*
  Given Amax, Amin, Bmax, Bmin, set the Umax, Umin, Vmax, Vmin.
  Note that they should have similar size as rsparam->global_v_bound_rs.
*/
void
rsbound_setup_sublattice ( rsbound_t rsbound,
                           mpz_t sl_A,
                           mpz_t sl_B,
                           mpz_t mod )
{
  mpz_set (rsbound->A, sl_A);
  mpz_set (rsbound->B, sl_B);
  /* the rsparam->modulus might be not true since rsparam might be
     changed for different qudratic rotations. Instead, the true mod
     is recorded in the priority queue */
  mpz_set (rsbound->MOD, mod);
  ab2uv (rsbound->A, rsbound->MOD, rsbound->Amax, rsbound->Umax);
  ab2uv (rsbound->A, rsbound->MOD, rsbound->Amin, rsbound->Umin);
  ab2uv (rsbound->B, rsbound->MOD, rsbound->Bmax, rsbound->Vmax);
  ab2uv (rsbound->B, rsbound->MOD, rsbound->Bmin, rsbound->Vmin);
}


/*
  Print root sieve bound and sublattice.
*/
void
rsbound_print ( rsbound_t rsbound )
{
  gmp_fprintf ( stderr,
                "# Info: (u, v) = (%Zd + i * %Zd, %Zd + j * %Zd)\n",
                rsbound->A, rsbound->MOD, rsbound->B, rsbound->MOD );

  gmp_fprintf ( stderr,
                "# Info: (Amin: %4ld, Amax: %4ld) -> (Umin: %6Zd, Umax: %6Zd)\n",
                rsbound->Amin, rsbound->Amax,
                rsbound->Umin, rsbound->Umax );

  gmp_fprintf ( stderr,
                "# Info: (Bmin: %4ld, Bmax: %4ld) -> (Vmin: %6Zd, Vmax: %6Zd)\n",
                rsbound->Bmin, rsbound->Bmax,
                rsbound->Vmin, rsbound->Vmax );
}


/*
  Init rsstr_t with.
*/
void
rsstr_init ( rsstr_t rs )
{
  unsigned int i;
  /* init */
  mpz_init (rs->n);
  mpz_init (rs->m);
  rs->f = (mpz_t*) malloc ((MAX_DEGREE + 1) * sizeof (mpz_t));
  rs->g = (mpz_t*) malloc ((MAX_DEGREE + 1) * sizeof (mpz_t));
  /* pre-computing function values f(r), g(r) for 0 <= r < p*/
  (rs->fx) = (mpz_t *) malloc ( (primes[NP-1]+1) * sizeof (mpz_t) );
  (rs->gx) = (mpz_t *) malloc ( (primes[NP-1]+1) * sizeof (mpz_t) );
  (rs->numerator) = (mpz_t *) malloc ( (primes[NP-1]+1) * sizeof (mpz_t) );

  if ((rs->f == NULL) || (rs->g == NULL)) {
    fprintf (stderr, "Error, cannot allocate memory for polynomial coefficients in rsstr_init().\n");
    exit (1);
  }

  if (((rs->fx) == NULL) || ((rs->gx) == NULL) || ((rs->numerator) == NULL)) {
    fprintf (stderr, "Error, cannot allocate memory for polynomials values in rsstr_init().\n");
    exit (1);
  }

  for (i = 0; i <= MAX_DEGREE; i++)
  {
    mpz_init (rs->f[i]);
    mpz_init (rs->g[i]);
  }
  for (i = 0; i <= primes[NP-1]; i++)
  {
    mpz_init (rs->fx[i]);
    mpz_init (rs->gx[i]);
    mpz_init (rs->numerator[i]);
  }
}


/*
  Precompute fx, gx and numerator in rsstr_t.
  Note, rs->f, etc must be set already.
*/
void
rsstr_setup ( rsstr_t rs )
{
  int i;
  mpz_t t;
  mpz_init (t);

  /* polynomial degree */
  for ((rs->d) = MAX_DEGREE; (rs->d) > 0 && mpz_cmp_ui ((rs->f[rs->d]), 0) == 0; rs->d --);

  /* M = -Y0/Y1 mod N */
  mpz_invert (rs->m, rs->g[1], rs->n);
  mpz_neg (rs->m, rs->m);
  mpz_mul (rs->m, rs->m, rs->g[0]);
  mpz_mod (rs->m, rs->m, rs->n);

  /* check M ? a root of the algebraic polynomial mod N */
  mpz_set (t, rs->f[rs->d]);
  for (i = rs->d - 1; i >= 0; i --) {
    mpz_mul (t, t, rs->m);
    mpz_add (t, t, rs->f[i]);
    mpz_mod (t, t, rs->n);
  }
  if (mpz_cmp_ui (t, 0) != 0)
  {
    fprintf (stderr, "ERROR: The following polynomial have no common root. \n");
    print_poly_fg_bare (stderr, rs->f, rs->g, rs->d, rs->n);
    exit (1);
  }

  /* save f(l) to an array for all l < B
     can we save the differences? not necessary since anyway
     precomputation is only done for once for one polynomial. */
  eval_polys (rs->f, rs->g, rs->fx, rs->gx, rs->numerator, primes, rs->d);

  rs->alpha_proj = get_biased_alpha_projective (rs->f, rs->d, 2000);

  mpz_clear (t);
}


/*
  Free rsstr_t.
*/
void
rsstr_free ( rsstr_t rs )
{
  unsigned int i;
  /* free fl, gl */
  for (i = 0; i <= primes[NP-1]; i ++)
  {
    mpz_clear(rs->fx[i]);
    mpz_clear(rs->gx[i]);
    mpz_clear(rs->numerator[i]);
  }
  for (i = 0; i <= MAX_DEGREE; i++)
  {
    mpz_clear (rs->f[i]);
    mpz_clear (rs->g[i]);
  }
  mpz_clear (rs->n);
  mpz_clear (rs->m);
  free (rs->fx);
  free (rs->gx);
  free (rs->numerator);
  free (rs->f);
  free (rs->g);
}


/*
  Init root sieve parameters. Often, there is no need to change
  them. The real customisation happens in rsparam_setup() function.
*/
void
rsparam_init ( rsparam_t rsparam,
               rsstr_t rs,
               param_t param )
{
  /* will be set in rsparam_setup */
  rsparam->nbest_sl = 0;
  rsparam->ncrts_sl = 0;
  rsparam->len_e_sl = 0;
  rsparam->tlen_e_sl = 0;
  rsparam->exp_min_alpha_rs = 0.0;
  rsparam->global_w_bound_rs = 0;
  rsparam->global_u_bound_rs = 0;
  rsparam->init_lognorm = 0.0;

  /* "rsparam->sizebound_ratio_rs"
     the higher, the more margin in computing the sieving bound
     u and v, hence the larger the sieving bound, and hence
     larger e_sl[] in rsparam_setup(). */
  rsparam->sizebound_ratio_rs = 1.01;
  mpz_init (rsparam->modulus);
  mpz_init_set_ui (rsparam->global_v_bound_rs, 0UL);

  /* number of primes beside e_sl[] considered in second stage
     root sieve. Larger takes longer time, but more accurate. */
  rsparam->len_p_rs = NP - 1;
  if (rsparam->len_p_rs >= NP)
    rsparam->len_p_rs = NP - 1;

  /* decide the lognorm bound */
  if (param->lognorm_bound > 0)
    rsparam->lognorm_bound = param->lognorm_bound;
  else {
    double skewness = L2_skewness (rs->f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
    rsparam->init_lognorm = L2_lognorm (rs->f, rs->d, skewness, DEFAULT_L2_METHOD);
    rsparam->lognorm_bound = rsparam->init_lognorm * 1.07;
  }
}


/*
  init the best poly (for the return)
*/
void
bestpoly_init ( bestpoly_t bestpoly,
                int d )
{
  int i;
  bestpoly->f = (mpz_t*) malloc ((d + 1) * sizeof (mpz_t));
  bestpoly->g = (mpz_t*) malloc (2 * sizeof (mpz_t));
  if ((bestpoly->f == NULL) || (bestpoly->g == NULL)) {
    fprintf (stderr, "Error, cannot allocate memory for bestpoly_init()\n");
    exit (1);
  }
  for (i = 0; i <= d; i++)
  {
    mpz_init (bestpoly->f[i]);
  }

  mpz_init (bestpoly->g[0]);
  mpz_init (bestpoly->g[1]);
}


/*
  free the best poly (for the return)
*/
void
bestpoly_free ( bestpoly_t bestpoly,
                int d )
{
  int i;
  for (i = 0; i <= d; i++)
  {
    mpz_clear (bestpoly->f[i]);
  }

  mpz_clear (bestpoly->g[0]);
  mpz_clear (bestpoly->g[1]);
  free (bestpoly->f);
  free (bestpoly->g);
}
/*
  replace f + k0 * x^t * (b*x - m) by f + k * x^t * (b*x - m), and return k to k0
  (modified from auxiliary.c)
*/
static inline void
rotate_aux_mpz ( mpz_t *f,
                 mpz_t b,
                 mpz_t m,
                 mpz_t k0,
                 mpz_t k,
                 unsigned int t )
{
  mpz_t tmp;
  mpz_init (tmp);
  mpz_sub (tmp, k, k0);
  mpz_addmul (f[t + 1], b, tmp);
  mpz_submul (f[t], m, tmp);
  mpz_set (k0, k);
  mpz_clear (tmp);
}


/*
  Modifed from auxiliary.c using Emmanuel Thome and Paul Zimmermann's ideas.
  Assume lognorm(f + v*g) + E(alpha(f + v*g)) is first decreasing, then
  increasing, then the optimal v corresponds to the minimum of that function.
*/
static inline void
rotate_bounds_V_mpz ( rsstr_t rs,
                      rsparam_t rsparam )
{
  int i, j;
  double skewness, lognorm;
  mpz_t *f, *g, b, m, tmpv, V;

  f = (mpz_t *) malloc ( (rs->d + 1)* sizeof (mpz_t));
  g = (mpz_t *) malloc ( 2 * sizeof (mpz_t));
  for (j = 0; j <= rs->d; j ++)
    mpz_init_set (f[j], rs->f[j]);
  for (j = 0; j < 2; j ++)
    mpz_init_set (g[j], rs->g[j]);
  mpz_init_set (b, rs->g[1]);
  mpz_init_set (m, rs->g[0]);
  mpz_neg (m, m);
  mpz_init_set_si (V, 1);
  mpz_init_set_ui (tmpv, 0);

  /* look for positive V: 2, 4, 8, ... */
  for (i = 0; i < 150; i++, mpz_mul_si (V, V, 2) )
  {
    /* rotate by w*x */
    rotate_aux_mpz (rs->f, b, m, tmpv, V, 0);

    /* translation-optimize the rotated polynomial */
    for (j = 0; j <= rs->d; j ++)
      mpz_set (f[j], rs->f[j]);
    for (j = 0; j < 2; j ++)
      mpz_set (g[j], rs->g[j]);
    optimize_aux (f, rs->d, g, 0, 0, DEFAULT_L2_METHOD);

    /* get lognorm */
    skewness = L2_skewness (f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
    lognorm = L2_lognorm (f, rs->d, skewness, DEFAULT_L2_METHOD);

    //fprintf (stderr, "# DEBUG --- [%d-th] U: %ld, lognorm: %f, lognorm_bound: %f\n", i, w, lognorm,  rsparam->lognorm_bound);

    if (lognorm > rsparam->lognorm_bound)
      break;
  }

  mpz_set (rsparam->global_v_bound_rs, V);
  mpz_set_si (V, 0);
  /* rotate back */
  rotate_aux_mpz (rs->f, b, m, tmpv, V, 0);

  /* look for negative k: -2, -4, -8, ... */
  mpz_set_si (V, -1);
  for (i = 0; i < 150; i++, mpz_mul_si (V, V, 2))
  {
    /* rotate by w*x */
    rotate_aux_mpz (rs->f, b, m, tmpv, V, 0);

    /* translation-optimize the rotated polynomial */
    for (j = 0; j <= rs->d; j ++)
      mpz_set (f[j], rs->f[j]);
    for (j = 0; j < 2; j ++)
      mpz_set (g[j], rs->g[j]);
    optimize_aux (f, rs->d, g, 0, 0, DEFAULT_L2_METHOD);

    /* get lognorm */
    skewness = L2_skewness (f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
    lognorm = L2_lognorm (f, rs->d, skewness, DEFAULT_L2_METHOD);

    //fprintf (stderr, "# DEBUG --- [%d-th] U: %ld, lognorm: %f, lognorm_bound: %f\n", i, w, lognorm,  rsparam->lognorm_bound);

    if (lognorm > rsparam->lognorm_bound)
      break;
  }

  /* set bound v */
  if (mpz_cmpabs (rsparam->global_v_bound_rs, V) < 0)
    mpz_set (rsparam->global_v_bound_rs, V);
  mpz_abs (rsparam->global_v_bound_rs, rsparam->global_v_bound_rs);

  /* rotate back */
  mpz_set_ui (V, 0);
  rotate_aux_mpz (rs->f, b, m, tmpv, V, 0);
  for (j = 0; j <= rs->d; j ++)
    mpz_clear (f[j]);
  for (j = 0; j < 2; j ++)
    mpz_clear (g[j]);
  free (f);
  free (g);
  mpz_clear (b);
  mpz_clear (m);
  mpz_clear (V);
  mpz_clear (tmpv);
}


/* find bound U for linear rotation */
static inline void
rotate_bounds_U_lu ( rsstr_t rs,
                     rsparam_t rsparam )
{
  unsigned int i;
  int j;
  double skewness, lognorm;
  mpz_t *f, *g, b, m;
  f = (mpz_t *) malloc ( (rs->d + 1)* sizeof (mpz_t));
  g = (mpz_t *) malloc ( 2 * sizeof (mpz_t));
  for (j = 0; j <= rs->d; j ++)
    mpz_init_set (f[j], rs->f[j]);
  for (j = 0; j < 2; j ++)
    mpz_init_set (g[j], rs->g[j]);
  mpz_init_set (b, rs->g[1]);
  mpz_init_set (m, rs->g[0]);
  mpz_neg (m, m);

  /* look for positive w: 1, 2, 4, 8, ...
     Note (sizeof (long) * 8 - 2) to prevent overflow in the rotate_aux(). */
  long w0 = 0, w = 1;
  for (i = 0; i < (sizeof (long) * 8 - 2); i++, w *= 2)
  {
    /* rotate by w*x */
    w0 = rotate_aux (rs->f, b, m, w0, w, 1);

    /* translation-optimize the rotated polynomial */
    for (j = 0; j <= rs->d; j ++)
      mpz_set (f[j], rs->f[j]);
    for (j = 0; j < 2; j ++)
      mpz_set (g[j], rs->g[j]);
    optimize_aux (f, rs->d, g, 0, 0, CIRCULAR);

    skewness = L2_skewness (f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
    lognorm = L2_lognorm (f, rs->d, skewness, DEFAULT_L2_METHOD);

    //fprintf (stderr, "# DEBUG --- [%d-th] U: %ld, lognorm: %f, lognorm_bound: %f\n", i, w, lognorm,  rsparam->lognorm_bound);

    if (lognorm > rsparam->lognorm_bound)
      break;
  }

  /* go back to w=0 */
  rotate_aux (rs->f, b, m, w0, 0, 1);
  rsparam->global_u_bound_rs = (unsigned long) w;

  /* look for negative w: -1, -2, -4, -8, ...
     Note (sizeof (long) * 8 - 2) to prevent overflow in the rotate_aux(). */
  w0 = 0;
  w = -1;
  for (i = 0; i < (sizeof (long int) * 8 - 2); i++, w *= 2)
  {
    /* rotate by w*x */
    w0 = rotate_aux (rs->f, b, m, w0, w, 1);

    /* translation-optimize the rotated polynomial */
    for (j = 0; j <= rs->d; j ++)
      mpz_set (f[j], rs->f[j]);
    for (j = 0; j < 2; j ++)
      mpz_set (g[j], rs->g[j]);
    optimize_aux (f, rs->d, g, 0, 0, CIRCULAR);

    skewness = L2_skewness (f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
    lognorm = L2_lognorm (f, rs->d, skewness, DEFAULT_L2_METHOD);

    //fprintf (stderr, "# DEBUG --- [%d-th] U: %ld, lognorm: %f, lognorm_bound: %f\n", i, w, lognorm,  rsparam->lognorm_bound);

    if (lognorm > rsparam->lognorm_bound)
      break;
  }

  /* go back to w=0 */
  rotate_aux (rs->f, b, m, w0, 0, 1);

  if ( (unsigned long) labs(w) > rsparam->global_u_bound_rs )
    rsparam->global_u_bound_rs = (unsigned long) labs(w);

  for (j = 0; j <= rs->d; j ++)
    mpz_clear (f[j]);
  for (j = 0; j < 2; j ++)
    mpz_clear (g[j]);
  free (f);
  free (g);
  mpz_clear (b);
  mpz_clear (m);
}


/* find bound W for qudratic rotation*/
static inline void
rotate_bounds_W_lu ( rsstr_t rs,
                     rsparam_t rsparam )
{
  int i, j;
  double skewness, lognorm;
  mpz_t *f, *g, b, m;
  f = (mpz_t *) malloc ( (rs->d + 1)* sizeof (mpz_t));
  g = (mpz_t *) malloc ( 2 * sizeof (mpz_t));
  for (i = 0; i <= rs->d; i ++)
    mpz_init_set (f[i], rs->f[i]);
  for (i = 0; i < 2; i ++)
    mpz_init_set (g[i], rs->g[i]);
  mpz_init_set (b, rs->g[1]);
  mpz_init_set (m, rs->g[0]);
  mpz_neg (m, m);

  /* look for positive w: , ... 0, 1, 2 */
  long w0 = 0, w = 0;
  for (i = 0; i < 2048; i++, w += 1)
  {
    /* rotate by w*x */
    w0 = rotate_aux (rs->f, b, m, w0, w, 2);

    /* translation-optimize the rotated polynomial */
    for (j = 0; j <= rs->d; j ++)
      mpz_set (f[j], rs->f[j]);
    for (j = 0; j < 2; j ++)
      mpz_set (g[j], rs->g[j]);
    optimize_aux (f, rs->d, g, 0, 0, CIRCULAR);

    skewness = L2_skewness (f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
    lognorm = L2_lognorm (f, rs->d, skewness, DEFAULT_L2_METHOD);

    //fprintf (stderr, "# DEBUG --- [%d-th] W: %ld, lognorm: %f, lognorm_bound: %f\n", i, w, lognorm,  rsparam->lognorm_bound);

    if (lognorm > rsparam->lognorm_bound)
      break;
  }

  /* go back to w=0 */
  rotate_aux (rs->f, b, m, w0, 0, 2);
  rsparam->global_w_bound_rs = (unsigned long) w;

  /* look for positive w: , ... 0, -1, -2 */
  w0 = 0;
  w = 0;
  for (i = 0; i < 2048; i++, w -= 1)
  {
    /* rotate by w*x */
    w0 = rotate_aux (rs->f, b, m, w0, w, 2);

    /* translation-optimize the rotated polynomial */
    for (j = 0; j <= rs->d; j ++)
      mpz_set (f[j], rs->f[j]);
    for (j = 0; j < 2; j ++)
      mpz_set (g[j], rs->g[j]);
    optimize_aux (f, rs->d, g, 0, 0, CIRCULAR);

    skewness = L2_skewness (f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
    lognorm = L2_lognorm (f, rs->d, skewness, DEFAULT_L2_METHOD);

    //fprintf (stderr, "# DEBUG --- [%d-th] W: %ld, lognorm: %f, lognorm_bound: %f\n", i, w, lognorm,  rsparam->lognorm_bound);

    if (lognorm > rsparam->lognorm_bound)
      break;
  }

  /* go back to w=0 */
  rotate_aux (rs->f, b, m, w0, 0, 2);

  if ( (unsigned long) labs(w) > rsparam->global_w_bound_rs )
    rsparam->global_w_bound_rs = (unsigned long) labs(w);

  for (i = 0; i <= rs->d; i ++)
    mpz_clear (f[i]);
  for (i = 0; i < 2; i ++)
    mpz_clear (g[i]);
  free (f);
  free (g);
  mpz_clear (b);
  mpz_clear (m);
}


/*
  Note, this function should be called in the first instance,
  without doing any rotation on the origional polynomial,
  since the rsparam->init_lognorm parameter will be set to decide
  the rotation range in the follows.

  Given rsparam->sizebound_ratio_rs and polynomial information,
  compute rsparam->global_u_bound_rs and rsparam->global_v_bound_rs;
  Then it will set e_sl[];
*/
void
rsparam_setup ( rsparam_t rsparam,
                rsstr_t rs,
                param_t param,
                int verbose )
{
  mpz_t q;
  mpz_init (q);

  /* polyselect2 mode, pass v bounds from param */
  if (verbose == 0) {
    mpz_ui_pow_ui (q, 2UL, (unsigned long) param->s2_Vmax);
    mpz_set (rsparam->global_v_bound_rs, q);
  }
  /* compute bounds v */
  else {
    /* "global_w_bound_rs" */
    if (rs->d == 6)
      rotate_bounds_W_lu ( rs,
                           rsparam );
    else
      rsparam->global_w_bound_rs = 0;

    /* "global_v_bound_rs" */
    rotate_bounds_V_mpz ( rs,
                          rsparam );
  }

  /* "global_u_bound_rs" */
  rotate_bounds_U_lu ( rs,
                       rsparam );

  /* repair if u is too large:
     -- global_u_bound will be used to identify good sublattice and decide e[],
     -- global_v_bound will be used to identify sieving bound */
  double skewness = L2_skewness (rs->f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
  mpz_fdiv_q_ui (q, rsparam->global_v_bound_rs, lround (skewness));
  if (mpz_cmpabs_ui (q, rsparam->global_u_bound_rs) < 0)
    rsparam->global_u_bound_rs = mpz_get_ui (q);
  mpz_clear (q);

  /* "rsparam->exp_min_alpha_rs" */
  int size;
  size = mpz_sizeinbase (rsparam->global_v_bound_rs, 2);
  if (rsparam->global_u_bound_rs <= 0)
    rsparam->global_u_bound_rs = 1;
  size += (int) (log ( (double) rsparam->global_u_bound_rs ) * 1.442695);
  rsparam->exp_min_alpha_rs = exp_alpha[size-1];

  if (verbose == 0) {
    gmp_fprintf ( stderr, "# Info: Bounds (%lu, %lu, %Zd) gives:\n",
                  rsparam->global_w_bound_rs,
                  rsparam->global_u_bound_rs,
                  rsparam->global_v_bound_rs );
    gmp_fprintf ( stderr,
                  "# Info: exp_alpha: %.3f, bound: %.3f\n",
                  exp_alpha[size-1],
                  rsparam->lognorm_bound );
  }

  /* "rsparam->nbest_sl" and "rsparam->ncrts_sl" */
  /* there could be too much individual sublattices to do crts. We restrict the num.*/
  rsparam->nbest_sl = 128;
  rsparam->ncrts_sl = 64;

  /* "rsparam->len_e_sl" and "rsparam->e_sl" */
  rsparam->len_e_sl = LEN_SUBLATTICE_PRIMES;
  rsparam->tlen_e_sl = rsparam->len_e_sl;

  if (rsparam->len_p_rs < rsparam->len_e_sl) {
    fprintf (stderr, "# Warning: number of primes considered in the root sieve is smaller than that in find_sublattice(). This might not be accurate. \n");
  }

  rsparam->e_sl = (unsigned short*)
    malloc ( rsparam->len_e_sl * sizeof (unsigned short) );

  if (rsparam->e_sl == NULL) {
    fprintf (stderr, "Error, cannot allocate memory in rsparam_setup().\n");
    exit (1);
  }

  for (size = 0; size < rsparam->len_e_sl; size ++) {
    rsparam->e_sl[size] = 0;
  }

  /* Experimental. Note at least consider two primes. */
  if (rsparam->global_u_bound_rs <= 512) {
    rsparam->e_sl[0] = 2;
    rsparam->e_sl[1] = 1;
    rsparam->e_sl[2] = 0;
    rsparam->e_sl[3] = 0;
  }
  else if (rsparam->global_u_bound_rs <= 2048) {
    rsparam->e_sl[0] = 3;
    rsparam->e_sl[1] = 1;
    rsparam->e_sl[2] = 0;
    rsparam->e_sl[3] = 0;
  }
  else if (rsparam->global_u_bound_rs <= 4096) {
    rsparam->e_sl[0] = 4;
    rsparam->e_sl[1] = 1;
    rsparam->e_sl[2] = 1;
    rsparam->e_sl[3] = 0;
  }
  else if (rsparam->global_u_bound_rs <= 8192) {
    rsparam->e_sl[0] = 5;
    rsparam->e_sl[1] = 2;
    rsparam->e_sl[2] = 2;
    rsparam->e_sl[3] = 1;
  }
  else if (rsparam->global_u_bound_rs <= 16384) {
    rsparam->e_sl[0] = 5;
    rsparam->e_sl[1] = 2;
    rsparam->e_sl[2] = 2;
    rsparam->e_sl[3] = 1;
  }
  else if (rsparam->global_u_bound_rs <= 32768) {
    rsparam->e_sl[0] = 6;
    rsparam->e_sl[1] = 2;
    rsparam->e_sl[2] = 1;
    rsparam->e_sl[3] = 1;
  }
  else if (rsparam->global_u_bound_rs <= 327680) {
    rsparam->e_sl[0] = 6;
    rsparam->e_sl[1] = 3;
    rsparam->e_sl[2] = 1;
    rsparam->e_sl[3] = 0;
    rsparam->e_sl[4] = 0;
  }
  else if (rsparam->global_u_bound_rs <= 3276800) {
    rsparam->e_sl[0] = 6;
    rsparam->e_sl[1] = 3;
    rsparam->e_sl[2] = 2;
    rsparam->e_sl[3] = 0;
    rsparam->e_sl[4] = 0;
  }
  else {
    rsparam->e_sl[0] = 6;
    rsparam->e_sl[1] = 3;
    rsparam->e_sl[2] = 2;
    rsparam->e_sl[3] = 2;
    rsparam->e_sl[4] = 1;
  }
}


/*
  For existing rsparam and when rs is changed,
*/
void
rsparam_reset_bounds ( rsparam_t rsparam,
                       rsstr_t rs,
                       param_t param,
                       int verbose )
{
  mpz_t q;
  mpz_init (q);

  /* polyselect2 mode, pass v bounds from param */
  if (verbose == 0) {
    mpz_ui_pow_ui (q, 2UL, (unsigned long) param->s2_Vmax);
    mpz_set (rsparam->global_v_bound_rs, q);
  }
  /* compute bounds v */
  else {
    /* "global_w_bound_rs" */
    if (rs->d == 6)
      rotate_bounds_W_lu ( rs,
                           rsparam );
    else
      rsparam->global_w_bound_rs = 0;

    /* "global_v_bound_rs" */
    rotate_bounds_V_mpz ( rs,
                          rsparam );
  }

  /* "global_u_bound_rs" */
  rotate_bounds_U_lu ( rs,
                       rsparam );

  /* repair if u is too large:
     -- global_u_bound will be used to identify good sublattice and decide e[],
     -- global_v_bound will be used to identify sieving bound */
  double skewness = L2_skewness (rs->f, rs->d, SKEWNESS_DEFAULT_PREC, DEFAULT_L2_METHOD);
  mpz_fdiv_q_ui (q, rsparam->global_v_bound_rs, lround (skewness));
  if (mpz_cmpabs_ui (q, rsparam->global_u_bound_rs) < 0)
    rsparam->global_u_bound_rs = mpz_get_ui (q);
  mpz_clear (q);

  /* "rsparam->exp_min_alpha_rs" */
  int size;
  size = mpz_sizeinbase (rsparam->global_v_bound_rs, 2);
  if (rsparam->global_u_bound_rs <= 0)
    rsparam->global_u_bound_rs = 1;
  size += (int) (log ( (double) rsparam->global_u_bound_rs ) * 1.442695);
  rsparam->exp_min_alpha_rs = exp_alpha[size-1];

  if (verbose == 0) {
    gmp_fprintf ( stderr, "# Info: Reset (U, V) to (%lu, %Zd) gives:\n",
                  rsparam->global_u_bound_rs,
                  rsparam->global_v_bound_rs );
    gmp_fprintf ( stderr,
                  "# Info: exp_alpha: %.3f, bound: %.3f\n",
                  exp_alpha[size-1],
                  rsparam->lognorm_bound );
  }
}



/*
  Free root sieve parameters.
*/
void
rsparam_free ( rsparam_t rsparam )
{
  free(rsparam->e_sl);
  mpz_clear (rsparam->global_v_bound_rs);
  mpz_clear (rsparam->modulus);
}


/*
  Init param
*/
void
param_init ( param_t param )
{
  mpz_init (param->s2_u);
  mpz_init (param->s2_v);
  mpz_init (param->s2_mod);
  mpz_init (param->n);
  mpz_set_ui (param->s2_u, 0);
  mpz_set_ui (param->s2_v, 0);
  mpz_set_ui (param->s2_mod, 0);
  mpz_set_ui (param->n, 0);
  param->w_left_bound = 0;
  param->w_length = 0;
  param->s1_num_e_sl = 0;
  param->s2_Amax = 0;
  param->s2_Bmax = 0;
  param->s2_Vmax = 0;
  param->s2_w = 0;
  param->lognorm_bound = 0;
  param->s1_e_sl = (unsigned short*)
    malloc ( LEN_SUBLATTICE_PRIMES * sizeof (unsigned short) );
  int i;
  for (i = 0; i < LEN_SUBLATTICE_PRIMES; i ++)
    param->s1_e_sl[i] = 0;
  param->d = 0;
}


void
param_clear ( param_t param )
{
  mpz_clear (param->s2_u);
  mpz_clear (param->s2_v);
  mpz_clear (param->s2_mod);
  mpz_clear (param->n);
  free (param->s1_e_sl);
}
