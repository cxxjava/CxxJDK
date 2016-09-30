/*
 * EScopeGuard.hh
 *
 *  Created on: 2015-9-18
 *      Author: cxxjava@163.com
 */

#ifndef ESCOPEGUARD_HH_
#define ESCOPEGUARD_HH_

#include "EBase.hh"

#ifdef CPP11_SUPPORT

namespace efc {

//@see: http://the-witness.net/news/2012/11/scopeexit-in-c11/

/** How to use it:
	// 1. MakeScopeGuard
	{
		auto onFailureRollback = MakeScopeGuard([&] {
			LOG("onFailureRollback...");
		});

		...

		onFailureRollback.dismiss();
	}

	// 2. ON_SCOPE_EXIT
	{
		ON_SCOPE_EXIT( ... );
		...
	}

	// 3. ON_FINALLY_NOTHROW
	{
		ON_FINALLY_NOTHROW(
			...
		) {
			...
		}}
	}
 */

template <typename F>
class EScopeGuard {
public:
	EScopeGuard(F f) : f(f), d(false) {}
    ~EScopeGuard() { if(!d) { f();} }
    void dismiss() { d = true; }
private:
    F f;
    boolean d;
};

template <typename F>
EScopeGuard<F> MakeScopeGuard(F f) {
    return EScopeGuard<F>(f);
};

#define DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define STRING_JOIN2(arg1, arg2) DO_STRING_JOIN2(arg1, arg2)

#define ON_SCOPE_EXIT(code) \
    auto STRING_JOIN2(scope_exit_, __LINE__) = MakeScopeGuard([&](){code;})

#define ON_FINALLY_NOTHROW(code) { \
    auto __scope_exit__ = MakeScopeGuard([&](){ try{ code; } catch(...){} });

} /* namespace efc */

#endif //!CPP11_SUPPORT

#endif /* ESCOPEGUARD_HH_ */
