/**
 * @file eso_mmap.c
 * @brief MMAP (Memory Map) routines
 */

#include "eso_mmap.h"
#include "eso_ring.h"

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <sys/mman.h>
#include <fcntl.h> /* O_RDWR... */
#include <unistd.h> /* close */
#include <sys/stat.h> /* for mac */
#endif

/**
 * @remark
 * As far as I can tell the only really sane way to store an MMAP is as a
 * void * and a length.  BeOS requires this area_id, but that's just a little
 * something extra.  I am exposing this type, because it doesn't make much
 * sense to keep it private, and opening it up makes some stuff easier in
 * Apache.
 */
/** The MMAP structure */
struct es_mmap_t {
#ifdef WIN32
    /** The handle of the file mapping */
    HANDLE mhandle;
    /** The start of the real memory page area (mapped view) */
    void *mv;
    /** The physical start, size and offset */
    es_off_t  pstart;
    es_size_t psize;
    es_off_t  poffset;
#endif
    /** The start of the memory mapped area */
    void *mm;
    /** The amount of data in the mmap */
    es_size_t size;
    /** ring of ez_mmap_t's that reference the same
     * mmap'ed region; acts in place of a reference count */
    ES_RING_ENTRY(es_mmap_t) link;
};

#ifdef WIN32
es_mmap_t* eso_mmap_create(const char* filename, int flag,
		                   es_off_t offset, es_size_t size)
{
	static DWORD memblock = 0;
	es_mmap_t *new_mmap;
	DWORD foaccess = 0;
	DWORD fmaccess = 0;
	DWORD mvaccess = 0;
	DWORD offlo;
	DWORD offhi;
	HANDLE filehand = INVALID_HANDLE_VALUE;

	if (flag & ES_MMAP_WRITE) {
		fmaccess |= PAGE_READWRITE;
		foaccess = GENERIC_WRITE | GENERIC_READ;
	}
	else if (flag & ES_MMAP_READ) {
		fmaccess |= PAGE_READONLY;
		foaccess = GENERIC_READ;
	}

	if (flag & ES_MMAP_READ)
		mvaccess |= FILE_MAP_READ;
	if (flag & ES_MMAP_WRITE)
		mvaccess |= FILE_MAP_WRITE;

	if (filename) {
		DWORD filesize;
		filehand = CreateFile(filename,
				foaccess,
				FILE_SHARE_READ,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
				NULL);
		if (filehand == INVALID_HANDLE_VALUE) {
			return NULL;
		}
		filesize = GetFileSize(filehand, NULL);
		if ((flag & ES_MMAP_WRITE) && size > filesize) {
			SetFilePointer(filehand, offset+size, NULL, FILE_BEGIN);
			SetEndOfFile(filehand);
		}
		if (size == 0) {
			size = filesize;
		}
	}

	if (!memblock) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		memblock = si.dwAllocationGranularity;
	}

	new_mmap = eso_calloc(sizeof(es_mmap_t));
	new_mmap->pstart = (offset / memblock) * memblock;
	new_mmap->poffset = offset - new_mmap->pstart;
	new_mmap->psize = (es_size_t)(new_mmap->poffset) + size;
	/* The size of the CreateFileMapping object is the current size
	 * of the size of the mmap object (e.g. file size), not the size
	 * of the mapped region!
	 */
	new_mmap->mhandle = CreateFileMapping(filehand, NULL, fmaccess,
			0, 0, NULL);
	if (!new_mmap->mhandle || new_mmap->mhandle == INVALID_HANDLE_VALUE) {
		if (filehand != INVALID_HANDLE_VALUE) {
			CloseHandle(filehand);
		}
		eso_free(new_mmap);
		return NULL;
	}

	offlo = (DWORD)new_mmap->pstart;
	offhi = 0;
	new_mmap->mv = MapViewOfFile(new_mmap->mhandle, mvaccess, offhi,
			offlo, new_mmap->psize);
	if (!new_mmap->mv) {
		if (filehand != INVALID_HANDLE_VALUE) {
			CloseHandle(filehand);
		}
		CloseHandle(new_mmap->mhandle);
		eso_free(new_mmap);
		return NULL;
	}

	if (filehand != INVALID_HANDLE_VALUE) {
		CloseHandle(filehand);
	}

	new_mmap->mm = (char*)(new_mmap->mv) + new_mmap->poffset;
	new_mmap->size = size;
	ES_RING_ELEM_INIT(new_mmap, link);

	return new_mmap;
}

void eso_mmap_delete(es_mmap_t **mm)
{
	es_mmap_t *themm;
	es_mmap_t *next;

	if (!mm || !*mm)
		return;

	themm = (*mm);
	next = ES_RING_NEXT(themm, link);

	/* we no longer refer to the mmaped region */
	ES_RING_REMOVE(themm, link);
	ES_RING_NEXT(themm, link) = NULL;
	ES_RING_PREV(themm, link) = NULL;

	if (next != themm) {
		/* more references exist, so we're done */
		ESO_FREE(mm);
		return;
	}

	if (themm->mv) {
		UnmapViewOfFile(themm->mv);
		themm->mv = NULL;
	}
	if (themm->mhandle) {
		CloseHandle(themm->mhandle);
		themm->mhandle = NULL;
	}

	ESO_FREE(mm);
}

void eso_mmap_flush(es_mmap_t *mm, int flag)
{
	ES_UNUSED(flag);
	FlushViewOfFile(mm->mm, mm->size);
}

#else //linux

es_mmap_t* eso_mmap_create(const char* filename, int flag,
		                   es_off_t offset, es_size_t size)
{
	es_mmap_t *new_mmap;
	void *mm;
	int open_flags = 0;
	int mmap_flags = 0;
	mode_t perm_flags;
	int fd = -1; //if -1 then flag need use MAP_ANON.

#if 0
	if ((flag & ES_MMAP_READ) && (flag & ES_MMAP_WRITE)) {
		open_flags = O_RDWR | O_CREAT;
		perm_flags = S_IWUSR | S_IRUSR;
	} else if (flag & ES_MMAP_READ) {
		open_flags = O_RDONLY;
		perm_flags = S_IRUSR;
	} else if (flag & ES_MMAP_WRITE) {
		open_flags = O_RDWR | O_CREAT; //if only use O_WRONLY then errno=13
		perm_flags = S_IWUSR;
	}
#else
	if (flag & ES_MMAP_WRITE) {
		open_flags = O_RDWR;
		perm_flags = S_IWUSR | S_IRUSR;//if only use O_WRONLY then errno=13
	}
	else {
		open_flags = O_RDONLY;
		perm_flags = S_IRUSR;
	}
#endif

	ES_ASSERT(open_flags != 0);

	if (flag & ES_MMAP_WRITE) {
		mmap_flags |= PROT_WRITE;
	}
	if (flag & ES_MMAP_READ) {
		mmap_flags |= PROT_READ;
	}

	if (filename) {
		struct stat buf;
		fd = open(filename, open_flags, perm_flags);
		if (fd == -1) {
			return NULL;
		}
		fstat(fd, &buf);
		if ((flag & ES_MMAP_WRITE) && size > buf.st_size) {
			ftruncate(fd, offset + size);
		}
		if (size == 0) {
			size = buf.st_size;
		}
	}

	mm = mmap(NULL, size, mmap_flags,
			(fd == -1) ? (MAP_ANON | MAP_SHARED) : MAP_SHARED, fd, offset);
	if (mm == (void *) -1) {
		/* we failed to get an mmap'd file... */
		if (fd >= 0) {
			close(fd);
		}
		return NULL;
	}

	if (fd >= 0) {
		close(fd);
	}

	new_mmap = (es_mmap_t *) eso_calloc(sizeof(es_mmap_t));
	new_mmap->mm = mm;
	new_mmap->size = size;
	ES_RING_ELEM_INIT(new_mmap, link);

	return new_mmap;
}

void eso_mmap_delete(es_mmap_t **mm)
{
	es_mmap_t *themm;
	es_mmap_t *next;

	if (!mm || !*mm)
		return;

	themm = (*mm);
	next = ES_RING_NEXT(themm, link);

	/* we no longer refer to the mmaped region */
	ES_RING_REMOVE(themm, link);
	ES_RING_NEXT(themm, link) = NULL;
	ES_RING_PREV(themm, link) = NULL;

	if (next != themm) {
		/* more references exist, so we're done */
		ESO_FREE(mm);
		return;
	}

	munmap(themm->mm, themm->size);
	themm->mm = (void *) -1;
	ESO_FREE(mm);
}

void eso_mmap_flush(es_mmap_t *mm, int flag)
{
	int native_flags = 0;

	if (flag == ES_MMAP_SYNC) {
		native_flags = MS_SYNC;
	}
	else if (flag == ES_MMAP_ASYNC) {
		native_flags = MS_ASYNC;
	}

	msync(mm->mm, mm->size, native_flags);
}

#endif //!WIN32

es_mmap_t* eso_mmap_dup(es_mmap_t *mm)
 {
	es_mmap_t *new_mmap;

	new_mmap = (es_mmap_t *) eso_malloc(sizeof(es_mmap_t));
	if (!new_mmap)
		return NULL;
	eso_memcpy(new_mmap, (void *) mm, sizeof(es_mmap_t));
	ES_RING_INSERT_AFTER(mm, new_mmap, link);

	return new_mmap;
}

void* eso_mmap_offset(es_mmap_t *mm, es_off_t offset)
{
	if (offset < 0 || offset > mm->size)
		return NULL;

	return (char *) mm->mm + offset;
}
