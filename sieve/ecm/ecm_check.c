/* Usage: ecm_check B1 B2 sigma < in > out

   'in' is a file with one prime number per line
   
   Puts in 'out' the primes which are not found with the curve (B1,B2,sigma).
 */

#include <stdlib.h>
#include "../../utils/modredc_ul_default.h"
#include "facul.h"
#include "ecm.h"

/* return non-zero iff p is found */
static int
tryecm (const unsigned long p, const ecm_plan_t *plan)
{
  modulus_t m;
  modint_t f;
  int r;
  
  mod_intset_ul (f, p);
  mod_initmod_uls (m, f);
  r = ecm_ul (f, m, plan);
  mod_clearmod (m);
  return r || mod_intequal_ul (f, p);
}

int
main (int argc, char *argv[])
{
  unsigned long B1, B2, sigma, p;
  ecm_plan_t plan[1];

  if (argc != 4)
    {
      fprintf (stderr, "Usage: ecm_check B1 B2 sigma\n");
      exit (1);
    }

  B1 = atoi (argv[1]);
  B2 = atoi (argv[2]);
  sigma = atoi (argv[3]);

  ecm_make_plan (plan, B1, B2, BRENT12, sigma, 0);
  while (!feof (stdin))
    {
      scanf ("%lu\n", &p);
      if (p != 2 && p != 3 && tryecm (p, plan) == 0)
        printf ("%lu\n", p);
    }
  return 0;
}

