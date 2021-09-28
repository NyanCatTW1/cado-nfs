/* Auxiliary routines for polynomial selection

Copyright 2008-2017 Emmanuel Thome, Paul Zimmermann

This file is part of CADO-NFS.

CADO-NFS is free software; you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2.1 of the License, or (at your option)
any later version.

CADO-NFS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License
along with CADO-NFS; see the file COPYING.  If not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "cado.h" // IWYU pragma: keep
#include <stdio.h>
#include <stdlib.h>
#include <float.h> // for DBL_MAX
#include <math.h>
#include <gmp.h>
#include "auxiliary.h"
#include "gmp_aux.h"    // ulong_isprime
#include "macros.h" /* for ASSERT_ALWAYS */
#include "murphyE.h"
#include "rootfinder.h" // mpz_poly_roots
#include "timing.h"             // for seconds
#include "usp.h"        // usp_root_data
#include "version_info.h"        // cado_revision_string
#include "double_poly.h"
#include "polyselect_norms.h"
#include "polyselect_alpha.h"

/**************************** rotation ***************************************/

/* replace f + k0 * x^t * (b*x + m) by f + k * x^t * (b*x + m), and return k */
long
rotate_aux (mpz_t *f, mpz_t b, mpz_t m, long k0, long k, unsigned int t)
{
  /* Warning: k - k0 might not be representable in a long! */
  unsigned long diff;
  if (k >= k0)
    {
      diff = k - k0; /* k - k0 always fits in an unsigned long */
      mpz_addmul_ui (f[t + 1], b, diff);
      mpz_addmul_ui (f[t], m, diff);
    }
  else
    {
      diff = k0 - k;
      mpz_submul_ui (f[t + 1], b, diff);
      mpz_submul_ui (f[t], m, diff);
    }
  return k;
}

/* replace f by f + k * x^t * (b*x + g0) */
void
rotate_auxg_z (mpz_t *f, const mpz_t b, const mpz_t g0, const mpz_t k, unsigned int t)
{
  mpz_addmul (f[t + 1], b, k);
  mpz_addmul (f[t], g0, k);
}

/* replace f by f - k * x^t * (b*x + g0) */
void
derotate_auxg_z (mpz_t *f, const mpz_t b, const mpz_t g0, const mpz_t k, unsigned int t)
{
  mpz_submul (f[t + 1], b, k);
  mpz_submul (f[t], g0, k);
}

/*
   Print f, g only.
   Note: it's a backend for print_cadopoly().
*/
void
print_cadopoly_fg (FILE *fp, mpz_t *f, int df, mpz_t *g, int dg, mpz_srcptr n)
{
   int i;

   /* n */
   gmp_fprintf (fp, "\nn: %Zd\n", n);

   /* Y[i] */
   for (i = dg; i >= 0; i--)
     gmp_fprintf (fp, "Y%d: %Zd\n", i, g[i]);

   /* c[i] */
   for (i = df; i >= 0; i--)
     gmp_fprintf (fp, "c%d: %Zd\n", i, f[i]);
}


/*
   Print f, g only, lognorm, skew, alpha, MurphyE.
   Note:  it's a backend for print_cadopoly_extra().
*/
double
print_cadopoly (FILE *fp, cado_poly_srcptr p)
{
   unsigned int nroots = 0;
   double alpha, alpha_proj, logmu, e;
   mpz_poly F, G;

   F->coeff = p->pols[ALG_SIDE]->coeff;
   F->deg = p->pols[ALG_SIDE]->deg;
   G->coeff = p->pols[RAT_SIDE]->coeff;
   G->deg = p->pols[RAT_SIDE]->deg;

   /* print f, g only*/
   print_cadopoly_fg (fp, F->coeff, F->deg, G->coeff, G->deg, p->n);

#ifdef DEBUG
   fprintf (fp, "# ");
   fprint_polynomial (fp, F->coeff, F->deg);
   fprintf (fp, "# ");
   fprint_polynomial (fp, G->coeff, G->deg);
#endif

   fprintf (fp, "skew: %1.3f\n", p->skew);

   if (G->deg > 1)
   {
    logmu = L2_lognorm (G, p->skew);
    alpha = get_alpha (G, get_alpha_bound ());
    alpha_proj = get_alpha_projective (G, get_alpha_bound ());
    nroots = numberOfRealRoots ((const mpz_t *) G->coeff, G->deg, 0, 0, NULL);
    fprintf (fp, "# lognorm: %1.2f, alpha: %1.2f (proj: %1.2f), E: %1.2f, "
                 "nr: %u\n", logmu, alpha, alpha_proj, logmu + alpha, nroots);
   }

   logmu = L2_lognorm (F, p->skew);
   alpha = get_alpha (F, get_alpha_bound ());
   alpha_proj = get_alpha_projective (F, get_alpha_bound ());
   nroots = numberOfRealRoots ((const mpz_t *) F->coeff, F->deg, 0, 0, NULL);
   fprintf (fp, "# lognorm: %1.2f, alpha: %1.2f (proj: %1.2f), E: %1.2f, "
                "nr: %u\n", logmu, alpha, alpha_proj, logmu + alpha, nroots);

   int alpha_bound = get_alpha_bound ();
   e = MurphyE (p, bound_f, bound_g, area, MURPHY_K, alpha_bound);
   cado_poly_fprintf_MurphyE (fp, e, bound_f, bound_g, area, "");

   return e;
}


/*
   Print f, g, lognorm, skew, alpha, MurphyE, REV, time duration.
*/
void
print_cadopoly_extra (FILE *fp, cado_poly cpoly, int argc, char *argv[], double st)
{
   int i;

   print_cadopoly (fp, cpoly);
   /* extra info */
   fprintf (fp, "# generated by %s: %s", cado_revision_string, argv[0]);
   for (i = 1; i < argc; i++)
      fprintf (fp, " %s", argv[i]);
   fprintf (fp, " in %.2fs\n", (seconds () - st));
}


/*
  Call print_cadopoly, given f, g and return MurphyE.
*/
double
print_poly_fg (mpz_poly_srcptr f, mpz_t *g, mpz_t N, int mode)
{
   double e;
   int i;
   int d = f->deg;

   cado_poly cpoly;
   cado_poly_init(cpoly);
   for (i = 0; i < (d + 1); i++)
      mpz_set(cpoly->pols[ALG_SIDE]->coeff[i], f->coeff[i]);
   for (i = 0; i < 2; i++)
      mpz_set(cpoly->pols[RAT_SIDE]->coeff[i], g[i]);
   mpz_set(cpoly->n, N);
   cpoly->skew = L2_skewness (f, SKEWNESS_DEFAULT_PREC);
   cpoly->pols[ALG_SIDE]->deg = d;
   cpoly->pols[RAT_SIDE]->deg = 1;

   if (mode == 1)
     {
       e = print_cadopoly (stdout, cpoly);
       fflush(stdout);
     }
   else
     e = MurphyE (cpoly, bound_f, bound_g, area, MURPHY_K, get_alpha_bound ());

   cado_poly_clear (cpoly);
   return e;
}

/* f <- f(x+k), g <- g(x+k) */
void
do_translate_z (mpz_poly_ptr f, mpz_t *g, const mpz_t k)
{
  int i, j;
  int d = f->deg;

  for (i = d - 1; i >= 0; i--)
    for (j = i; j < d; j++)
      mpz_addmul (f->coeff[j], f->coeff[j+1], k);
  mpz_addmul (g[0], g[1], k);
}

/* f <- f(x-k), g <- g(x-k) */
void
do_detranslate_z (mpz_poly_ptr f, mpz_t *g, const mpz_t k)
{
  int i, j;
  int d = f->deg;

  for (i = d - 1; i >= 0; i--)
    for (j = i; j < d; j++)
      mpz_submul (f->coeff[j], f->coeff[j+1], k);
  mpz_submul (g[0], g[1], k);
}


/* If final <> 0, print the real value of E (root-optimized polynomial),
   otherwise print the expected value of E. Return E or exp_E accordingly.
   TODO: adapt for more than 2 polynomials and two algebraic polynomials */
double
cado_poly_fprintf_with_info (FILE *fp, cado_poly_ptr poly, const char *prefix,
                             int final)
{
  unsigned int nrroots;
  double lognorm, alpha, alpha_proj, exp_E;

  nrroots = numberOfRealRoots ((const mpz_t *) poly->pols[ALG_SIDE]->coeff, poly->pols[ALG_SIDE]->deg, 0, 0, NULL);
  if (poly->skew <= 0.0) /* If skew is undefined, compute it. */
    poly->skew = L2_skewness (poly->pols[ALG_SIDE], SKEWNESS_DEFAULT_PREC);
  lognorm = L2_lognorm (poly->pols[ALG_SIDE], poly->skew);
  alpha = get_alpha (poly->pols[ALG_SIDE], get_alpha_bound ());
  alpha_proj = get_alpha_projective (poly->pols[ALG_SIDE], get_alpha_bound ());
  exp_E = (final) ? 0.0 : lognorm
    + expected_rotation_gain (poly->pols[ALG_SIDE], poly->pols[RAT_SIDE]);

  cado_poly_fprintf (stdout, poly, prefix);
  cado_poly_fprintf_info (fp, lognorm, exp_E, alpha, alpha_proj, nrroots,
                          prefix);
  return (final) ? lognorm + alpha : exp_E;
}

/* TODO: adapt for more than 2 polynomials and two algebraic polynomials */
double
cado_poly_fprintf_with_info_and_MurphyE (FILE *fp, cado_poly_ptr poly,
                                         double MurphyE, double bound_f,
                                         double bound_g, double area,
                                         const char *prefix)
{
  double exp_E;
  exp_E = cado_poly_fprintf_with_info (fp, poly, prefix, 1);
  cado_poly_fprintf_MurphyE (fp, MurphyE, bound_f, bound_g, area, prefix);
  return exp_E;
}

/* compute largest interval kmin <= k <= kmax such that when we add k*x^i*g(x)
   to f(x), the lognorm does not exceed maxlognorm (with skewness s) */
void
expected_growth (rotation_space *r, mpz_poly_srcptr f, mpz_poly_srcptr g, int i,
                 double maxlognorm, double s)
{
  mpz_t fi, fip1, kmin, kmax, k;
  double n2;

  mpz_init_set (fi, f->coeff[i]);
  mpz_init_set (fip1, f->coeff[i+1]);
  mpz_init (kmin);
  mpz_init (kmax);
  mpz_init (k);

  /* negative side */
  mpz_set_si (kmin, -1);
  for (;;)
    {
      mpz_set (f->coeff[i], fi);
      mpz_set (f->coeff[i+1], fip1);
      rotate_auxg_z (f->coeff, g->coeff[1], g->coeff[0], kmin, i);
      n2 = L2_lognorm (f, s);
      if (n2 > maxlognorm)
        break;
      mpz_mul_2exp (kmin, kmin, 1);
    }
  /* now kmin < k < kmin/2 */
  mpz_tdiv_q_2exp (kmax, kmin, 1);
  while (1)
    {
      mpz_add (k, kmin, kmax);
      mpz_div_2exp (k, k, 1);
      if (mpz_cmp (k, kmin) == 0 || mpz_cmp (k, kmax) == 0)
        break;
      mpz_set (f->coeff[i], fi);
      mpz_set (f->coeff[i+1], fip1);
      rotate_auxg_z (f->coeff, g->coeff[1], g->coeff[0], k, i);
      n2 = L2_lognorm (f, s);
      if (n2 > maxlognorm)
        mpz_set (kmin, k);
      else
        mpz_set (kmax, k);
    }
  r->kmin = mpz_get_d (kmax);

  /* positive side */
  mpz_set_ui (kmax, 1);
  for (;;)
    {
      mpz_set (f->coeff[i], fi);
      mpz_set (f->coeff[i+1], fip1);
      rotate_auxg_z (f->coeff, g->coeff[1], g->coeff[0], kmax, i);
      n2 = L2_lognorm (f, s);
      if (n2 > maxlognorm)
        break;
      mpz_mul_2exp (kmax, kmax, 1);
    }
  /* now kmax < k < kmax/2 */
  mpz_tdiv_q_2exp (kmin, kmax, 1);
  while (1)
    {
      mpz_add (k, kmin, kmax);
      mpz_div_2exp (k, k, 1);
      if (mpz_cmp (k, kmin) == 0 || mpz_cmp (k, kmax) == 0)
        break;
      mpz_set (f->coeff[i], fi);
      mpz_set (f->coeff[i+1], fip1);
      rotate_auxg_z (f->coeff, g->coeff[1], g->coeff[0], k, i);
      n2 = L2_lognorm (f, s);
      if (n2 > maxlognorm)
        mpz_set (kmax, k);
      else
        mpz_set (kmin, k);
    }
  r->kmax = mpz_get_d (kmin);

  /* reset f[i] and f[i+1] */
  mpz_set (f->coeff[i], fi);
  mpz_set (f->coeff[i+1], fip1);

  mpz_clear (fi);
  mpz_clear (fip1);
  mpz_clear (kmin);
  mpz_clear (kmax);
  mpz_clear (k);
}

/* for a given pair (f,g), tries to estimate the value of alpha one might
   expect from rotation (including the projective alpha) */
double
expected_rotation_gain (mpz_poly_srcptr f, mpz_poly_srcptr g)
{
  double S = 1.0, s, incr = 0.0;
  rotation_space r;
  double proj_alpha = get_alpha_projective (f, ALPHA_BOUND_SMALL);
  double skew = L2_skewness (f, SKEWNESS_DEFAULT_PREC);
  double n = L2_lognorm (f, skew);

  for (int i = 0; 2 * i < f->deg; i++)
    {
      expected_growth (&r, f, g, i, n + NORM_MARGIN, skew);
      s = r.kmax - r.kmin + 1.0;
      S *= s;
      /* assume each non-zero rotation increases on average by NORM_MARGIN/2 */
      if (s >= 2.0)
        incr += NORM_MARGIN / 2.0;
    }
  return proj_alpha + expected_alpha (log(S)) + incr;
}

