#ifndef LAS_PROCESS_BUCKET_REGION_HPP_
#define LAS_PROCESS_BUCKET_REGION_HPP_

#include <stdint.h>
#include <memory>
#include <array>
#include "threadpool.hpp"
#include "las-threads-work-data.hpp"
#include "las-auxiliary-data.hpp"
#include "las-report-stats.hpp"

#if 0
struct process_bucket_region_parameters: public task_parameters {
    /* ws is used in the synchronous phase, so we don't need to retain
     * ownership via a shared pointer. For wc and aux, we do, because
     * they're used late */
    nfs_work & ws;
    std::shared_ptr<nfs_work_cofac> wc_p;
    std::shared_ptr<nfs_aux> aux_p;
    sieve_info & si;
    where_am_I w;

    /* For good-old process_bucket_region, which actually does a whole
     * congruence class of bucket regions, this is the smallest N that is
     * considered. For the one-bucket-region-at-a-time approach, this is
     * simply the bucket index itself.
     *
     * Note that in the former (good-old) case, when si.toplevel==1, we
     * have only one level of buckets, so we don't need this field */
    uint32_t first_region0_index=0;

    process_bucket_region_parameters(nfs_work & ws, std::shared_ptr<nfs_work_cofac> wc_p, std::shared_ptr<nfs_aux> aux_p, sieve_info & si, where_am_I const& w)
        : ws(ws), wc_p(wc_p), aux_p(aux_p), si(si), w(w)
    {}
};

/* This is in las.cpp */
extern task_result * process_bucket_regions_congruence_class(worker_thread * worker, task_parameters * _param, int);

#endif

extern void process_many_bucket_regions(nfs_work & ws, std::shared_ptr<nfs_work_cofac> wc_p, std::shared_ptr<nfs_aux> aux_p, thread_pool & pool, int first_region0_index, where_am_I const & w);

/* {{{ process_one_bucket_region */

struct process_bucket_region_spawn {
    nfs_work & ws;
    std::shared_ptr<nfs_work_cofac> wc_p;
    std::shared_ptr<nfs_aux> aux_p;
    where_am_I w_saved;
    
    /* These two indices are set from within process_many_bucket_regions,
     * prior to spawning all threads.
     *
     * first_region0_index is the bucket index of the first region for
     * which we filled the buckets.
     *
     * already done is, relative to first_region0_index, the index of the
     * first region for which the small sieve start position are
     * available in ssdpos_many.
     *
     * The i-th process_bucket_region task thus handles the bucket region
     * of index first_region0_index + already_done + i
     */
    int first_region0_index;
    int already_done;

    process_bucket_region_spawn(
            nfs_work & ws,
            std::shared_ptr<nfs_work_cofac> wc_p,
            std::shared_ptr<nfs_aux> aux_p,
            where_am_I w
            ) :
        ws(ws), wc_p(wc_p), aux_p(aux_p), w_saved(w) {}

    void operator()(worker_thread * worker, int id);
};

/*}}}*/

#endif	/* LAS_PROCESS_BUCKET_REGION_HPP_ */