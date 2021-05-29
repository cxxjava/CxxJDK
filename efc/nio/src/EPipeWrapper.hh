/*
 * EPipeWrapper.hh
 *
 *  Created on: 2014-1-26
 *      Author: cxxjava@163.com
 */

#ifndef EPIPEWRAPPER_HH_
#define EPIPEWRAPPER_HH_

#include "../inc/ESocketChannel.hh"

namespace efc {
namespace nio {

class EPipeWrapper {
public:
	EPipeWrapper();
	~EPipeWrapper();

	int inFD();
	int outFD();

private:
#ifdef WIN32
	ESocketChannel* source;
	ESocketChannel* sink;
#else
	es_pipe_t *pipe;
#endif
};

} /* namespace nio */
} /* namespace efc */
#endif /* EPIPEWRAPPER_HH_ */
