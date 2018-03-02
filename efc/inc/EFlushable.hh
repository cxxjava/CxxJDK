/*
 * EFlushable.hh
 *
 *  Created on: 2013-3-19
 *      Author: cxxjava@163.com
 */

#ifndef EFlushable_HH_
#define EFlushable_HH_

#include "EIOException.hh"

namespace efc {

/**
 * A <tt>Flushable</tt> is a destination of data that can be flushed.  The
 * flush method is invoked to write any buffered output to the underlying
 * stream.
 *
 * @since 1.5
 */
interface EFlushable : virtual public EObject {
	virtual ~EFlushable(){}

    /**
     * Flushes this stream by writing any buffered output to the underlying
     * stream.
     *
     * @throws IOException If an I/O error occurs
     */
    virtual void flush() THROWS(IOException) = 0;
};

} /* namespace efc */
#endif /* EFlushable_HH_ */

