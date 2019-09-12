#include "cado.h"

#include "cxx_mpz.hpp"
#include "lingen_expected_pi_length.hpp"



std::tuple<unsigned int, unsigned int> get_minmax_delta(std::vector<unsigned int> const & delta)/*{{{*/
{
    unsigned int maxdelta = 0;
    unsigned int mindelta = UINT_MAX;
    for(auto x : delta) {
        if (x > maxdelta) maxdelta = x;
        if (x < mindelta) mindelta = x;
    }
    return std::make_tuple(mindelta, maxdelta);
}/*}}}*/
unsigned int get_min_delta(std::vector<unsigned int> const & delta)/*{{{*/
{
    unsigned int mindelta, maxdelta;
    std::tie(mindelta, maxdelta) = get_minmax_delta(delta);
    return mindelta;
}/*}}}*/
unsigned int get_max_delta(std::vector<unsigned int> const & delta)/*{{{*/
{
    unsigned int mindelta, maxdelta;
    std::tie(mindelta, maxdelta) = get_minmax_delta(delta);
    return maxdelta;
}/*}}}*/
std::tuple<unsigned int, unsigned int> get_minmax_delta_on_solutions(bmstatus & bm, std::vector<unsigned int> const & delta)/*{{{*/
{
    unsigned int maxdelta = 0;
    unsigned int mindelta = UINT_MAX;
    for(unsigned int j = 0 ; j < bm.d.m + bm.d.n ; j++) {
        if (bm.lucky[j] <= 0) continue;
        if (delta[j] > maxdelta) maxdelta = delta[j];
        if (delta[j] < mindelta) mindelta = delta[j];
    }
    return std::make_tuple(mindelta, maxdelta);
}/*}}}*/
unsigned int get_max_delta_on_solutions(bmstatus & bm, std::vector<unsigned int> const & delta)/*{{{*/
{
    unsigned int mindelta, maxdelta;
    std::tie(mindelta, maxdelta) = get_minmax_delta_on_solutions(bm, delta);
    return maxdelta;
}/*}}}*/



unsigned int expected_pi_length(bw_dimensions & d, unsigned int len)/*{{{*/
{
    /* The idea is that we want something which may account for something
     * exceptional, bounded by probability 2^-64. This corresponds to a
     * column in e (matrix of size m*b) to be spontaneously equal to
     * zero. This happens with probability (#K)^-m.
     * The solution to
     * (#K)^(-m*x) > 2^-64
     * is m*x*log_2(#K) < 64
     *
     * We thus need to get an idea of the value of log_2(#K).
     *
     * (Note that we know that #K^abgroupsize(ab) < 2^64, but that bound
     * might be very gross).
     *
     * The same esitmate can be used to appreciate what we mean by
     * ``luck'' in the end. If a column happens to be zero more than
     * expected_pi_length(d,0) times in a row, then the cause must be
     * more than sheer luck, and we use it to detect generating rows.
     */

    unsigned int m = d.m;
    unsigned int n = d.n;
    unsigned int b = m + n;
    abdst_field ab MAYBE_UNUSED = d.ab;
    unsigned int res = 1 + iceildiv(len * m, b);
#ifndef SELECT_MPFQ_LAYER_u64k1
    cxx_mpz p;
    abfield_characteristic(ab, (mpz_ptr) p);
    unsigned int l;
    if (mpz_cmp_ui(p, 1024) >= 0) {
        l = mpz_sizeinbase(p, 2);
        l *= abfield_degree(ab);    /* roughly log_2(#K) */
    } else {
        mpz_pow_ui(p, p, abfield_degree(ab));
        l = mpz_sizeinbase(p, 2);
    }
#else
    // K is GF(2), period.
    unsigned int l = 1;
#endif
    // unsigned int safety = iceildiv(abgroupsize(ab), m * sizeof(abelt));
    unsigned int safety = iceildiv(64, m * l);
    return res + safety;
}/*}}}*/

unsigned int expected_pi_length(bw_dimensions & d, std::vector<unsigned int> const & delta, unsigned int len)/*{{{*/
{
    // see comment above.

    unsigned int mi, ma;
    std::tie(mi, ma) = get_minmax_delta(delta);

    return expected_pi_length(d, len) + ma - mi;
}/*}}}*/

unsigned int expected_pi_length_lowerbound(bw_dimensions & d, unsigned int len)/*{{{*/
{
    /* generically we expect that len*m % (m+n) columns have length
     * 1+\lfloor(len*m/(m+n))\rfloor, and the others have length one more.
     * For one column to have a length less than \lfloor(len*m/(m+n))\rfloor,
     * it takes probability 2^-(m*l) using the notations above. Therefore
     * we can simply count 2^(64-m*l) accidental zero cancellations at
     * most below the bound.
     * In particular, it is sufficient to derive from the code above!
     */
    unsigned int m = d.m;
    unsigned int n = d.n;
    unsigned int b = m + n;
    abdst_field ab MAYBE_UNUSED = d.ab;
    unsigned int res = 1 + (len * m) / b;
#ifndef SELECT_MPFQ_LAYER_u64k1
    cxx_mpz p;
    abfield_characteristic(ab, p);
    unsigned int l;
    if (mpz_cmp_ui(p, 1024) >= 0) {
        l = mpz_sizeinbase(p, 2);
        l *= abfield_degree(ab);    /* roughly log_2(#K) */
    } else {
        mpz_pow_ui(p, p, abfield_degree(ab));
        l = mpz_sizeinbase(p, 2);
    }
#else
    unsigned int l = 1;
#endif
    unsigned int safety = iceildiv(64, m * l);
    return safety < res ? res - safety : 0;
}/*}}}*/

