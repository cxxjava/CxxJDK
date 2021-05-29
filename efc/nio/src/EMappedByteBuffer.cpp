/*
 * EMappedByteBuffer.cpp
 *
 *  Created on: 2014-2-15
 *      Author: cxxjava@163.com
 */

#include "../inc/EMappedByteBuffer.hh"
#include "../../inc/EBits.hh"
#include "../../inc/EInteger.hh"
#include "../../inc/EIOStatus.hh"
#include "../../inc/EOutOfMemoryError.hh"
#include "../../inc/EIllegalArgumentException.hh"
#include "../../inc/EIOException.hh"

#ifdef WIN32
#include <io.h>
#else
#include <sys/mman.h>
#if !defined(__APPLE__)
#if SIZEOF_VOID_P == 8
	#define mmap mmap64
#endif
#endif
#endif

namespace efc {
namespace nio {

//@see: openjdk-6/jdk/src/share/classes/java/nio/MappedByteBuffer.java
//@see: openjdk-6/jdk/src/solaris/native/java/nio/MappedByteBuffer.c
//@see: openjdk-6/jdk/src/windows/native/java/nio/MappedByteBuffer.c

#ifdef WIN32
static long allocationGranularity() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwAllocationGranularity;
}

static void* map0(EMapMode mode, int fd, long off, long len) {
	void *mapAddress = 0;
	int lowOffset = (int)off;
	int highOffset = 0;
	long maxSize = off + len;
	int lowLen = (int)(maxSize);
	int highLen = 0;
	HANDLE fileHandle = (HANDLE)_get_osfhandle(fd);
	HANDLE mapping;
	DWORD mapAccess = FILE_MAP_READ;
	DWORD fileProtect = PAGE_READONLY;
	DWORD mapError;
	BOOL result;

	#if SIZEOF_VOID_P == 8
	highOffset = (int)(off >> 32);
	highLen = (int)(maxSize >> 32);
	#endif

	if (mode == READ_ONLY) {
		fileProtect = PAGE_READONLY;
		mapAccess = FILE_MAP_READ;
	} else if (mode == READ_WRITE) {
		fileProtect = PAGE_READWRITE;
		mapAccess = FILE_MAP_WRITE;
	} else if (mode == PRIVATE) {
		fileProtect = PAGE_WRITECOPY;
		mapAccess = FILE_MAP_COPY;
	}

	mapping = CreateFileMapping(
		fileHandle,      /* Handle of file */
		NULL,            /* Not inheritable */
		fileProtect,     /* Read and write */
		highLen,         /* High word of max size */
		lowLen,          /* Low word of max size */
		NULL);           /* No name for object */

	if (mapping == NULL) {
		throw EIOException(__FILE__, __LINE__, "Map failed");
	}

	mapAddress = MapViewOfFile(
		mapping,             /* Handle of file mapping object */
		mapAccess,           /* Read and write access */
		highOffset,          /* High word of offset */
		lowOffset,           /* Low word of offset */
		(DWORD)len);         /* Number of bytes to map */
	mapError = GetLastError();

	result = CloseHandle(mapping);
	if (result == 0) {
		throw EIOException(__FILE__, __LINE__, "Map failed");
	}

	if (mapAddress == NULL) {
		if (mapError == ERROR_NOT_ENOUGH_MEMORY)
			throw EOutOfMemoryError(__FILE__, __LINE__, "Map failed");
		else
			throw EIOException(__FILE__, __LINE__, "Map failed");
	}

	return mapAddress;
}

static int unmap0(void* address, long length) {
	BOOL result;

	result = UnmapViewOfFile(address);
	if (result == 0) {
		throw EIOException(__FILE__, __LINE__, "Unmap failed");
		return EIOStatus::THROWN;
	}
	return 0;
}

static void force0(void* address, long length) {
	int result;
	int retry;

	/*
	 * FlushViewOfFile can fail with ERROR_LOCK_VIOLATION if the memory
	 * system is writing dirty pages to disk. As there is no way to
	 * synchronize the flushing then we retry a limited number of times.
	 */
	retry = 0;
	do {
		result = FlushViewOfFile(address, (DWORD)length);
		if ((result != 0) || (GetLastError() != ERROR_LOCK_VIOLATION))
			break;
		retry++;
	} while (retry < 3);

	if (result == 0) {
		throw EIOException(__FILE__, __LINE__, "Flush failed");
	}
}

#else //linux

#define allocationGranularity() sysconf(_SC_PAGESIZE)

static void* map0(EMapMode mode, int fd, long off, long len) {
	void *mapAddress = 0;
	int protections = 0;
	int flags = 0;

	if (mode == READ_ONLY) {
		protections = PROT_READ;
		flags = MAP_SHARED;
	} else if (mode == READ_WRITE) {
		protections = PROT_WRITE | PROT_READ;
		flags = MAP_SHARED;
	} else if (mode == PRIVATE) {
		protections =  PROT_WRITE | PROT_READ;
		flags = MAP_PRIVATE;
	}

	mapAddress = mmap(
		0,                    /* Let OS decide location */
		len,                  /* Number of bytes to map */
		protections,          /* File permissions */
		flags,                /* Changes are shared */
		fd,                   /* File descriptor of mapped file */
		off);                 /* Offset into file */

	if (mapAddress == MAP_FAILED) {
		if (errno == ENOMEM) {
			throw EOutOfMemoryError(__FILE__, __LINE__, "Map failed");
		}
		throw EIOException(__FILE__, __LINE__, "Map failed");
	}
	return mapAddress;
}

static int unmap0(void* address, long length) {
	int rv;
	RESTARTABLE(munmap(address, length), rv);
	if (rv >= 0)
		return rv;
	throw EIOException(__FILE__, __LINE__, "Unmap failed");
}

static void force0(void* address, long length) {
	long pageSize = EBits::pageSize();
	unsigned long lAddress = (unsigned long)address;

	long offset = lAddress % pageSize;
	void *a = (void *)(lAddress - offset);
	int result = msync(a, (size_t)(length + offset), MS_SYNC);
	if (result != 0) {
		throw EIOException(__FILE__, __LINE__, "msync failed");
	}
}
#endif //!WIN32

//=============================================================================

EMappedByteBuffer::~EMappedByteBuffer() {
	if (_address) {
		unmap0(_address, mapSize_);
	}
}

EMappedByteBuffer::EMappedByteBuffer(EMapMode mode, int fd, long position, long size) : EIOByteBuffer() {
	if (position < 0L)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative position");
	if (size < 0L)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative size");
	if (position + size < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Position + size overflow");
	if (size > EInteger::MAX_VALUE)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Size exceeds Integer.MAX_VALUE");

	if (size > 0) {
		long _allocationGranularity = allocationGranularity();
		int pagePosition = (int) (position % _allocationGranularity);
		long mapPosition = position - pagePosition;
		mapSize_ = size + pagePosition;
		// If no exception was thrown from map0, the address is valid
		_address = map0(mode, fd, mapPosition, mapSize_);
		_limit = _capacity = size;

		ES_ASSERT(EIOStatus::checkAll(_address ? 0 : EIOStatus::THROWN));
		ES_ASSERT((llong)_address % _allocationGranularity == 0);
	}
}

boolean EMappedByteBuffer::isLoaded() {
	if ((_address == 0) || (_capacity == 0))
		return true;

	//return isLoaded0(((DirectByteBuffer)this).address(), capacity());

#ifdef WIN32
	boolean loaded = false;
	/* Information not available?
	MEMORY_BASIC_INFORMATION info;
	int result = VirtualQuery(_address, &info, (DWORD)len);
	*/
	return loaded;
#else
	boolean loaded = true;
	int pageSize = sysconf(_SC_PAGESIZE);
	int numPages = (_capacity + pageSize - 1) / pageSize;
	int result = 0;
	int i = 0;
	char * vec = (char *)eso_malloc(numPages * sizeof(char));

	if (vec == NULL) {
		throw EOUTOFMEMORYERROR;
		return false;
	}

#ifdef __APPLE__
	result = mincore(_address, (size_t)_capacity, vec);
#else
	result = mincore(_address, (size_t)_capacity, (unsigned char*)vec);
#endif
	if (result != 0) {
		eso_free(vec);
		throw EIOException(__FILE__, __LINE__, "mincore failed");
		return false;
	}

	for (i=0; i<numPages; i++) {
		if (vec[i] == 0) {
			loaded = false;
			break;
		}
	}
	eso_free(vec);
	return loaded;
#endif
}

EMappedByteBuffer& EMappedByteBuffer::load() {
	if ((_address == 0) || (_capacity == 0))
		return *this;

	//load0(((DirectByteBuffer)this).address(), capacity(), Bits.pageSize());

	long pageSize = EBits::pageSize();
#ifdef WIN32
	int *ptr = (int *)_address;
	int pageIncrement = pageSize / sizeof(int);
	long numPages = (_capacity + pageSize - 1) / pageSize;
	int i = 0;
	int j = 0;

	/* touch every page */
	for (i=0; i<numPages; i++) {
		j += *((volatile int *)ptr);
		ptr += pageIncrement;
	}
	//return j;
#else
	int pageIncrement = pageSize / sizeof(int);
	int numPages = (_capacity + pageSize - 1) / pageSize;
	int *ptr = (int *)_address;
	int i = 0;
	int j = 0;
	int result = madvise((caddr_t)ptr, _capacity, MADV_WILLNEED);
	ES_UNUSED(result);

	/* touch every page */
	for (i=0; i<numPages; i++) {
		j += *((volatile int *)ptr);
		ptr += pageIncrement;
	}
	//return j;
#endif
	return *this;
}

EMappedByteBuffer& EMappedByteBuffer::force() {
	if ((_address == 0) || (_capacity == 0))
		return *this;
	force0(_address, _capacity);
	return *this;
}

void EMappedByteBuffer::checkMapped() {
}

} /* namespace nio */
} /* namespace efc */
