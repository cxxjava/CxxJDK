/*
 * EThreadX.hh
 *
 *  Created on: 2016-9-1
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADX_HH_
#define ETHREADX_HH_

#include "EThread.hh"

#ifdef CPP11_SUPPORT

#include <functional>

namespace efc {

class EThreadX: public EThread {
public:
	static EThreadX* execute(std::function<void()> f) {
		EThreadX* thread = new EThreadX(f);
		thread->start();
		return thread;
	}

protected:
	EThreadX(std::function<void()>& f) {
		this->f = f;
	}

	virtual void run() {
		f();
	}

private:
	std::function<void()> f;
};

} /* namespace efc */

#endif //!CPP11_SUPPORT

#endif /* ETHREADX_HH_ */
