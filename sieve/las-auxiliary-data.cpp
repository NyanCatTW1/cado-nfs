#include "cado.h" // IWYU pragma: keep

#include <iomanip>             // for operator<<, setprecision, fixed
#include <map>                 // for map
#include <ostream>             // for ostringstream, basic_ostream, basic_os...
#include <string>              // for allocator, basic_string, string
#include <cstdio>              // IWYU pragma: keep
#include <cstdarg>             // IWYU pragma: keep

#include <gmp.h>               // for gmp_vfprintf, mpz_srcptr, mpz_import

#include "las-auxiliary-data.hpp"

#include "macros.h"            // for ASSERT_ALWAYS_NOTHROW

#include "las-info.hpp"        // for las_info
#include "las-todo-entry.hpp"  // for las_todo_entry
#include "tdict.hpp"           // for timetree_t, slot, global_enable, slot_...
#include "ularith.h"           // for ularith_addmod_ul_ul


void
sieve_checksum::update(const unsigned int other)
{
    unsigned long r;
    ularith_addmod_ul_ul(&r, checksum, other, checksum_prime);
    checksum = r;
}

void
sieve_checksum::update(const unsigned char *data, const size_t len)
{
    cxx_mpz mb;
    unsigned int new_checksum;

    mpz_import(mb, len, -1, sizeof(unsigned char), -1, 0, data);
    new_checksum = mpz_tdiv_ui(mb, checksum_prime);

    this->update(new_checksum);
}

nfs_aux::~nfs_aux()
{
    ASSERT_ALWAYS(!rt.timer.running());

    ASSERT_ALWAYS(dest_rt);

    if (!complete) {
        std::lock_guard<std::mutex> lock(rt.mm);
        dest_rt->rep.accumulate_and_clear(std::move(rt.rep));
        dest_rt->timer += rt.timer;
        return;
    }

    /* locking is not needed here, since rt.rep is our private timer and all
     * threads completed their subtasks.
     */
    for (auto & T : th) {
        rt.rep.accumulate_and_clear(std::move(T.rep));
        rt.timer += T.timer;
        for (int side = 0; side < 2; side++)
            checksum_post_sieve[side].update(T.checksum_post_sieve[side]);
    }

    verbose_output_start_batch();

    if (tdict::global_enable >= 2) {
        verbose_output_print (0, 1, "%s", rt.timer.display().c_str());

        timetree_t::timer_data_type t = 0;
        for(auto const &c : rt.timer.filter_by_category()) {
            std::ostringstream os;
            os << std::fixed << std::setprecision(2) << c.second;
            verbose_output_print (0, 1, "# %s: %s\n",
                    coarse_las_timers::explain(c.first).c_str(),
                    os.str().c_str());
            t += c.second;
        }
        std::ostringstream os;
        os << std::fixed << std::setprecision(2) << t;
        verbose_output_print (0, 1, "# total counted time: %s\n", os.str().c_str());
    }

    rt.rep.display_survivor_counters();

    verbose_output_print(0, 2,
            "# Checksums over sieve region: "
            "after all sieving: %u, %u\n",
            checksum_post_sieve[0].get_checksum(),
            checksum_post_sieve[1].get_checksum());

    verbose_output_vfprint(0, 1, gmp_vfprintf,
            "# %lu %s\n",
            rt.rep.reports,
            las.batch ? "survivor(s) saved" : "relation(s)"
            );

    if (las.suppress_duplicates)
        verbose_output_print(0, 1, "# number of eliminated duplicates: %lu\n", rt.rep.duplicates);

    wct_qt0 = wct_seconds() - wct_qt0;

    if (rt.rep.survivors.after_sieve != rt.rep.survivors.not_both_even) {
        verbose_output_print(0, 1, "# Warning: found %ld hits with i,j both even (not a bug, but should be very rare)\n", rt.rep.survivors.after_sieve - rt.rep.survivors.not_both_even);
    }

    auto D = rt.timer.filter_by_category();
    timetree_t::timer_data_type qtcpu = rt.timer.total_counted_time();

    verbose_output_vfprint (0, 1, gmp_vfprintf,
            "# Time for side-%d q=%Zd r=%Zd:"
            " %1.2fs",
            doing.side,
            (mpz_srcptr) doing.p, (mpz_srcptr) doing.r,
            qtcpu
            );
    verbose_output_print (0, 2,
            " [norm %1.2f+%1.1f, sieving %1.1f"
            " (%1.1f+%1.1f + %1.1f),"
            " factor %1.1f (%1.1f+%1.1f + %1.1f),"
            " rest %1.1f]",
            D[coarse_las_timers::norms(0)],
            D[coarse_las_timers::norms(1)],

            D[coarse_las_timers::sieving(0)]+
            D[coarse_las_timers::sieving(1)]+
            D[coarse_las_timers::search_survivors()]+
            D[coarse_las_timers::sieving_mixed()],

            D[coarse_las_timers::sieving(0)],
            D[coarse_las_timers::sieving(1)],
            D[coarse_las_timers::search_survivors()]+
            D[coarse_las_timers::sieving_mixed()],

            D[coarse_las_timers::cofactoring(0)]+
            D[coarse_las_timers::cofactoring(1)]+
            D[coarse_las_timers::cofactoring_mixed()],

            D[coarse_las_timers::cofactoring(0)],
            D[coarse_las_timers::cofactoring(1)],
            D[coarse_las_timers::cofactoring_mixed()],

            D[coarse_las_timers::bookkeeping()]
            );
    verbose_output_print (0, 1, "\n");

    verbose_output_end_batch();

    {
        std::lock_guard<std::mutex> lock(dest_rt->mm);
        dest_rt->rep.accumulate_and_clear(std::move(rt.rep));
        dest_rt->timer += rt.timer;
    }
}

#ifndef DISABLE_TIMINGS
/* we really wish to have a single timing slot for all the instantiations
 * of fill_in_buckets_toplevel_wrapper */
tdict::slot tdict_slot_for_fibt("fill_in_buckets_toplevel");
tdict::slot tdict_slot_for_alloc_buckets("allocate_buckets");
tdict::slot tdict_slot_for_threads("multithreaded tasks");
tdict::slot_parametric tdict_slot_for_side("side ", "");
#endif

