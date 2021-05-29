/*
 * ENativeThread.cpp
 *
 *  Created on: 2014-1-3
 *      Author: cxxjava@163.com
 */

#include "./ENativeThread.hh"
#include "../../inc/EIOException.hh"

namespace efc {
namespace nio {

int ENativeThread::inited = false;

static void
nullHandler(int sig)
{
}

void ENativeThread::init() {
	if (!inited) {
		if (eso_os_thread_sigaction_init(nullHandler) != 0) {
			throw EIOException(__FILE__, __LINE__, "sigaction");
		}
		inited = true;
	}
}

es_os_thread_t ENativeThread::current() {
	init();
	return eso_os_thread_current();
}

void ENativeThread::signal(es_os_thread_t nt) {
	init();
	eso_os_thread_kill(nt);
}

} /* namespace nio */
} /* namespace efc */
