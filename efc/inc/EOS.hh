/*
 * EOS.hh
 *
 *  Created on: 2015-2-11
 *      Author: cxxjava@163.com
 */

#ifndef EOS_HH_
#define EOS_HH_

#include "EBase.hh"

namespace efc {

class EOS {
public:
	DECLARE_STATIC_INITZZ;

public:
	// Interface for detecting multiprocessor system
	static inline boolean is_MP() {
		ES_ASSERT(_processor_count > 0);
		return _processor_count > 1;
	}
	static ullong available_memory();
	static ullong physical_memory();

	// number of CPUs
	static int processor_count() {
		return _processor_count;
	}

	// size of Page.
	static int page_size(void) {
		return _page_size;
	}

	// Returns the number of CPUs this process is currently allowed to run on.
	// Note that on some OSes this can change dynamically.
	static int active_processor_count();

	// Bind processes to processors.
	//     This is a two step procedure:
	//     first you generate a distribution of processes to processors,
	//     then you bind processes according to that distribution.
	// Compute a distribution for number of processes to processors.
	//    Stores the processor id's into the distribution array argument.
	//    Returns true if it worked, false if it didn't.
	static boolean distribute_processes(uint length, uint* distribution);
	// Binds the current process to a processor.
	//    Returns true if it worked, false if it didn't.
	static boolean bind_to_processor(uint processor_id);

protected:
	static int _processor_count;              // number of processors
	static int _page_size;                    // size of page
	static ullong _physical_memory;           // number of memorys
};

} /* namespace efc */
#endif /* EOS_HH_ */
