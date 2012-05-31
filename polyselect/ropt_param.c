/**
 * @file ropt_param.c
 * Holds extern vars. The customizable parameters 
 * are in ropt_param.h.
 */


#include "ropt_param.h"


/**
 * Default values for L1 cache size and tune sieve length. The code
 * will also try to auto-detect them.
 */
unsigned int L1_cachesize = 12288;
unsigned int size_tune_sievearray = 6144;


/* ------------------
   possible to change
   ------------------ */


/**
 * Total number of sublattices to be root-sieved in the final step.
 * If with the default parametes (SIZE_SIEVEARRAY_V_MAX 4194304),
 * each root sieve takes about 2-4 seconds. The left column is 
 * ranked by the digits of integers to be factored and the right
 * column is the number (actually number+1).
 *
 * Usually, there is one or more tunning steps before the final root
 * sieve. In that case, more sublattices are checked (e.g. double/quad
 * the following valeus).
 */
const unsigned int size_total_sublattices[7][2] = {
  // {digits, num_of_sublattices} 
  {80, 4},
  {100, 8},
  {140, 16},
  {180, 16},
  {220, 16},
  {260, 32},
  {300, 128}
};


/**
 * Number of top sublattice for individual primes[i] in stage 1,
 * where i < NUM_SUBLATTICE_PRIMES. The constrcution should depends
 * on the total num of primes in s1param->e_sl[]. 
 * 
 * The main purpose is to prevent too much crt computations in stage 1.
 */
const unsigned int
size_each_sublattice[NUM_SUBLATTICE_PRIMES][NUM_SUBLATTICE_PRIMES] = {
  { 64,  0,  0,  0,  0,  0,  0,  0,  0 }, // tlen_e_sl = 1
  { 64, 64,  0,  0,  0,  0,  0,  0,  0 }, // tlen_e_sl = 2
  { 64, 64, 64,  0,  0,  0,  0,  0,  0 }, // tlen_e_sl = 3
  { 64, 32, 32, 32,  0,  0,  0,  0,  0 }, // tlen_e_sl = 4
  { 32, 32, 16,  8,  8,  0,  0,  0,  0 }, // tlen_e_sl = 5
  { 32, 16, 16,  8,  8,  4,  0,  0,  0 }, // tlen_e_sl = 6 (current bound)
  { 32, 16,  8,  8,  4,  4,  4,  0,  0 }, // tlen_e_sl = 7
  { 32, 16,  8,  8,  4,  4,  4,  2,  0 }, // tlen_e_sl = 8
  { 32, 16,  8,  8,  4,  4,  4,  2,  2 }, // tlen_e_sl = 9
};


/**
 * As above, but it's only used for tunning good w in 'ropt_quadratic.c'.
 * Therefore, the values are much smaller than above.
 */
const unsigned int
size_each_sublattice_tune[NUM_SUBLATTICE_PRIMES] = {
  8,  8,  4,  4,  4,  2,  2,  2,  2
};


/* -------------------------
   perhaps no need to change
   ------------------------- */


/**
 * Default parameters for sublattice p_i^e^i.
 * Non-decreasing due to ropt_s1param_setup() in ropt_str.c
 */
const unsigned int 
default_sublattice_pe[NUM_DEFAULT_SUBLATTICE][NUM_SUBLATTICE_PRIMES] = {
  /* 2, 3, 5, 7, 11, 13, 17, 19, 23 */
  { 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // 2
  { 2, 0, 0, 0, 0, 0, 0, 0, 0 }, // 4
  { 2, 1, 0, 0, 0, 0, 0, 0, 0 }, // 12
  { 3, 1, 0, 0, 0, 0, 0, 0, 0 }, // 24
  { 4, 1, 0, 0, 0, 0, 0, 0, 0 }, // 48
  { 3, 2, 0, 0, 0, 0, 0, 0, 0 }, // 72
  { 3, 1, 1, 0, 0, 0, 0, 0, 0 }, // 120
  { 4, 2, 0, 0, 0, 0, 0, 0, 0 }, // 144
  { 4, 1, 1, 0, 0, 0, 0, 0, 0 }, // 240
  { 5, 2, 0, 0, 0, 0, 0, 0, 0 }, // 288
  { 3, 2, 1, 0, 0, 0, 0, 0, 0 }, // 360
  { 5, 1, 1, 0, 0, 0, 0, 0, 0 }, // 480
  { 4, 2, 1, 0, 0, 0, 0, 0, 0 }, // 720
  { 5, 2, 1, 0, 0, 0, 0, 0, 0 }, // 1440
  { 5, 3, 1, 0, 0, 0, 0, 0, 0 }, // 4320
  { 5, 2, 1, 1, 0, 0, 0, 0, 0 }, // 10080
  { 6, 2, 1, 1, 0, 0, 0, 0, 0 }, // 20160
  { 5, 3, 2, 0, 0, 0, 0, 0, 0 }, // 21600
  { 5, 2, 2, 1, 0, 0, 0, 0, 0 }, // 50400
  { 6, 2, 2, 1, 0, 0, 0, 0, 0 }, // 100800
  { 6, 3, 2, 1, 0, 0, 0, 0, 0 }, // 302400
  { 6, 3, 2, 1, 1, 0, 0, 0, 0 }, // 3326400
  { 7, 3, 2, 1, 1, 0, 0, 0, 0 }, // 6652800
  { 8, 3, 2, 1, 1, 0, 0, 0, 0 }, // 13305600
  { 7, 4, 2, 1, 1, 0, 0, 0, 0 }, // 19958400
  { 6, 3, 2, 1, 1, 1, 0, 0, 0 }, // 43243200
};


/**
 * Hard-coded product for above default_sublattice_pe.
 */
const unsigned long
default_sublattice_prod[NUM_DEFAULT_SUBLATTICE] = {
  2, 4, 12, 24, 48, 72, 120, 144, 240, 288, 360, 480,
  720, 1440, 4320, 10080, 20160, 21600, 50400, 100800,
  302400, 3326400, 6652800, 13305600, 19958400, 43243200,
};


/**
<<<<<<< HEAD
=======
 * Total number of top sublattices (ranked by the size of integers
 * to be factored).
 */
const unsigned int size_total_sublattices[7][2] = {
  // {digits, num_of_sublattices} 
  {80, 4},   /* for up to 79 digits */
  {100, 8},  /* up to 99 digits */
  {140, 32}, /* up to 139 digits */
  {180, 16},
  {220, 16},
  {260, 32},
  {300, 64}
};


/**
 * Number of top sublattice for each primes[i] 
 * where i < NUM_SUBLATTICE_PRIMES. This only
 * depends on the num of primes in sublattices.
 * Purposed to prevent too much crt computations.
 */
const unsigned int
size_each_sublattice[NUM_SUBLATTICE_PRIMES][NUM_SUBLATTICE_PRIMES] = {
  { 64,  0,  0,  0,  0,  0,  0,  0,  0 }, // tlen_e_sl = 1
  { 64, 64,  0,  0,  0,  0,  0,  0,  0 }, // tlen_e_sl = 2
  { 64, 64, 64,  0,  0,  0,  0,  0,  0 }, // tlen_e_sl = 3
  { 64, 32, 32, 32,  0,  0,  0,  0,  0 }, // tlen_e_sl = 4
  { 32, 32, 16,  8,  8,  0,  0,  0,  0 }, // tlen_e_sl = 5
  { 32, 16, 16,  8,  8,  4,  0,  0,  0 }, // tlen_e_sl = 6 (current bound)
  { 32, 16,  8,  8,  4,  4,  4,  0,  0 }, // tlen_e_sl = 7
  { 32, 16,  8,  8,  4,  4,  4,  2,  0 }, // tlen_e_sl = 8
  { 32, 16,  8,  8,  4,  4,  4,  2,  2 }, // tlen_e_sl = 9
};


/**
 * As above, but for tunning good w in 'ropt_quadratic.c'.
 */
const unsigned int
size_each_sublattice_tune[NUM_SUBLATTICE_PRIMES] = {
  8,  8,  4,  4,  4,  2,  2,  2,  2
};


/**
>>>>>>> origin/master
 * Primes.
 */
const unsigned int primes[NP] = {
  2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
  31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
  73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
  127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199
};


/**
 * next_prime_idx[3] = ind(5) = 2 in above table.
 */
const unsigned char next_prime_idx[] = {
  0, // 0
  0, 1, 2, 2, 3, 3, 4, 4, 4, 4, // 1 - 10
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
  43, 43, 44, 44, 44, 44, 45, 45 // 191 - 198
};


/**
 * Asymptotic estimate of minimum order statistics
 * for 2^i many rotations where i <= 150.
 */
const double exp_alpha[] = {
  0 ,
  -1.15365373215 ,-1.52232116258 ,-1.84599014685, -2.13527943823, -2.39830159659, // 2^2, ...
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
