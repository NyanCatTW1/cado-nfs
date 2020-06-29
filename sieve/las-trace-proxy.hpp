#ifndef LAS_TRACE_PROXY_HPP_
#define LAS_TRACE_PROXY_HPP_

#include <memory>

/* Yes, where_am_I::impl is an _incomplete_ type, on purpose. We only
 * manipulate opaque pointers. The compilation units that actually _do_
 * something with that are:
 *  - separate functions that do allocation/free ; there's a trace and a
 *    notrace version
 *  - template instantiations that also call more specific code in the
 *    las-debug.cpp module.
 *
 */

#if defined(TRACE_K) && !defined(TRACK_CODE_PATH)
#define TRACK_CODE_PATH
#endif

struct where_am_I {
#ifdef TRACK_CODE_PATH
    where_am_I();
    where_am_I(where_am_I const &);
    where_am_I & operator=(where_am_I const &);
    ~where_am_I();
#else
    where_am_I() {}
    where_am_I(where_am_I const &) {}
    where_am_I & operator=(where_am_I const &) { return *this; }
    ~where_am_I() {}
#endif
    private:
    struct impl;  // forward declaration of the implementation class
    impl * pimpl;
    public:
    impl * operator->() { return pimpl; };
    impl const * operator->() const { return pimpl; };
};

/* Please, don't use this in tight loops. This just to avoid code bloat.
 * In cases where the GOT linking saves us a megabyte of duplicated code,
 * why not go for it ? */
int extern_trace_on_spot_ab(int64_t a, uint64_t b);

/*
struct where_am_I_deleter { void operator()(where_am_I*) const; };

typedef std::unique_ptr<where_am_I, where_am_I_deleter> opaque_where_am_I_ptr;
typedef const std::unique_ptr<where_am_I, where_am_I_deleter> opaque_where_am_I_srcptr;

opaque_where_am_I_ptr create_opaque_where_am_I_ptr();
*/

#endif	/* LAS_TRACE_PROXY_HPP_ */
