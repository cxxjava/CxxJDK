/*
 * EOS.cpp
 *
 *  Created on: 2015-2-11
 *      Author: cxxjava@163.com
 */

#include "EOS.hh"
#ifdef WIN32
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#ifdef __linux__
#include <sys/sysinfo.h>
#endif
#endif

namespace efc {

const es_size_t K                  = 1024;
const es_size_t M                  = K*K;
const es_size_t G                  = M*K;

//=============================================================================

int EOS::_processor_count = 0;
int EOS::_page_size = 0;
ullong EOS::_physical_memory = 0L;

DEFINE_STATIC_INITZZ_BEGIN(EOS)
eso_initialize();
#ifdef WIN32
SYSTEM_INFO si;
GetSystemInfo(&si);
_page_size    = si.dwPageSize;
_processor_count = si.dwNumberOfProcessors;

MEMORYSTATUSEX ms;
ms.dwLength = sizeof(ms);
// also returns dwAvailPhys (free physical memory bytes), dwTotalVirtual, dwAvailVirtual,
// dwMemoryLoad (% of memory in use)
GlobalMemoryStatusEx(&ms);
_physical_memory = ms.ullTotalPhys;
#else
_processor_count = sysconf(_SC_NPROCESSORS_CONF);
_page_size = sysconf(_SC_PAGESIZE);
#ifdef __APPLE__
	/* So, darwin doesn't support _SC_PHYS_PAGES, but it does
	   support getting the raw memory size in bytes through
	   sysctlbyname(hw.memsize); */
	es_size_t len = sizeof(ullong);

	/* Note hw.memsize is in bytes, so no need to multiply by page size. */
	sysctlbyname("hw.memsize", &_physical_memory, &len, NULL, 0);
#else
	_physical_memory = (ullong)sysconf(_SC_PHYS_PAGES) * (ullong)sysconf(_SC_PAGESIZE);
#endif
#endif
DEFINE_STATIC_INITZZ_END

//=============================================================================

ullong EOS::available_memory() {
#ifdef WIN32
	// Use GlobalMemoryStatusEx() because GlobalMemoryStatus() may return incorrect
	// value if total memory is larger than 4GB
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatusEx(&ms);

	return (ullong)ms.ullAvailPhys;
#else
#ifdef __APPLE__
	es_size_t len = sizeof(ullong);
	ullong mem = 0;

	/* Note hw.memsize is in bytes, so no need to multiply by page size. */
	sysctlbyname("hw.physmem", &mem, &len, NULL, 0);
	return mem;
#else
	// values in struct sysinfo are "unsigned long"
	struct sysinfo si;
	sysinfo(&si);

	return (ullong)si.freeram * si.mem_unit;
#endif
#endif
}

ullong EOS::physical_memory() {
	return _physical_memory;
}

int EOS::active_processor_count() {
#ifdef WIN32
	#define BitsPerByte 8
	DWORD_PTR lpProcessAffinityMask = 0;
	DWORD_PTR lpSystemAffinityMask = 0;
	int proc_count = processor_count();
	if (proc_count <= sizeof(UINT_PTR) * BitsPerByte &&
			GetProcessAffinityMask(GetCurrentProcess(), &lpProcessAffinityMask, &lpSystemAffinityMask)) {
		// Nof active processors is number of bits in process affinity mask
		int bitcount = 0;
		while (lpProcessAffinityMask != 0) {
			lpProcessAffinityMask = lpProcessAffinityMask & (lpProcessAffinityMask-1);
			bitcount++;
		}
		return bitcount;
	} else {
		return proc_count;
	}
#else
	// Linux doesn't yet have a (official) notion of processor sets,
	// so just return the number of online processors.
	int online_cpus = ::sysconf(_SC_NPROCESSORS_ONLN);
	ES_ASSERT(online_cpus > 0 && online_cpus <= processor_count());
	return online_cpus;
#endif
}

boolean EOS::distribute_processes(uint length, uint* distribution) {
#ifdef WIN32
	// Not yet implemented.
	return false;
#else
	// Not yet implemented.
	return false;
#endif
}

boolean EOS::bind_to_processor(uint processor_id) {
#ifdef WIN32
	// Not yet implemented.
	return false;
#else
	// Not yet implemented.
	return false;
#endif
}

} /* namespace efc */
