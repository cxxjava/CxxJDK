/**
 * @file  eso_status.h
 * @brief ES Error Codes
 */

#ifndef __ESO_STATUS_H__
#define __ESO_STATUS_H__

#include "es_comm.h"
#include "es_types.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef int es_status_t;      /* c style status code */


/**
 * Type for specifying an error or status code.
 */
/** no error. */
#define ES_SUCCESS 0
/** normal error */
#define ES_FAILURE -1

/**
 * ES_OS_START_ERROR is where the ES specific error values
 * start.
 */
#define ES_OS_START_ERROR     20000
/**
 * ES_OS_ERRSPACE_SIZE is the maximum number of errors you can
 *    fit into one of the error/status ranges below -- except
 *    for ES_OS_START_USERERR, which see.
 */
#define ES_OS_ERRSPACE_SIZE 50000
/**
 * ES_OS_START_STATUS is where the ES specific status codes start.
 */
#define ES_OS_START_STATUS    (ES_OS_START_ERROR + ES_OS_ERRSPACE_SIZE)
/**
 * ES_OS_START_USERERR are reserved for applications that use ES that
 *     layer their own error codes along with ES's.  Note that the
 *     error immediately following this one is set ten times farther
 *     away than usual, so that users of ES have a lot of room in
 *     which to declare custom error codes.
 */
#define ES_OS_START_USERERR    (ES_OS_START_STATUS + ES_OS_ERRSPACE_SIZE)
/**
 * ES_OS_START_CANONERR is where ES versions of errno values are defined
 *     on systems which don't have the corresponding errno.
 */
#define ES_OS_START_CANONERR  (ES_OS_START_USERERR + (ES_OS_ERRSPACE_SIZE * 10))
/**
 * ES_OS_START_EAIERR folds EAI_ error codes from getaddrinfo() into 
 *     es_status_t values.
 */
#define ES_OS_START_EAIERR    (ES_OS_START_CANONERR + ES_OS_ERRSPACE_SIZE)
/**
 * ES_OS_START_SYSERR folds platform-specific system error values into 
 *     es_status_t values.
 */
#define ES_OS_START_SYSERR    (ES_OS_START_EAIERR + ES_OS_ERRSPACE_SIZE)

/** 
 * OS Error Values
 */
#define ES_ENOTIMPL       (ES_OS_START_STATUS + 1)
#define ES_INCOMPLETE     (ES_OS_START_STATUS + 2)
#define ES_IOERROR        (ES_OS_START_STATUS + 3)
#define ES_EOF            (ES_OS_START_STATUS + 4)
#define ES_TIMEUP         (ES_OS_START_STATUS + 5)
#define ES_EBUSY          (ES_OS_START_STATUS + 6)
#define ES_BADARG         (ES_OS_START_STATUS + 7)
#define ES_NOTFOUND       (ES_OS_START_STATUS + 8)
#define ES_INCHILD        (ES_OS_START_STATUS + 9)
#define ES_INPARENT       (ES_OS_START_STATUS + 10)
#define ES_DETACH         (ES_OS_START_STATUS + 11)
#define ES_NOTDETACH      (ES_OS_START_STATUS + 12)
#define ES_CHILD_DONE     (ES_OS_START_STATUS + 13)
#define ES_CHILD_NOTDONE  (ES_OS_START_STATUS + 14)

/**
 * Canonical Error Values
 */
/** @see ES_STATUS_IS_EACCES */
#ifdef EACCES
#define ES_EACCES EACCES
#else
#define ES_EACCES         (ES_OS_START_CANONERR + 1)
#endif

/** @see ES_STATUS_IS_EEXIST */
#ifdef EEXIST
#define ES_EEXIST EEXIST
#else
#define ES_EEXIST         (ES_OS_START_CANONERR + 2)
#endif

/** @see ES_STATUS_IS_ENAMETOOLONG */
#ifdef ENAMETOOLONG
#define ES_ENAMETOOLONG ENAMETOOLONG
#else
#define ES_ENAMETOOLONG   (ES_OS_START_CANONERR + 3)
#endif

/** @see ES_STATUS_IS_ENOENT */
#ifdef ENOENT
#define ES_ENOENT ENOENT
#else
#define ES_ENOENT         (ES_OS_START_CANONERR + 4)
#endif

/** @see ES_STATUS_IS_ENOTDIR */
#ifdef ENOTDIR
#define ES_ENOTDIR ENOTDIR
#else
#define ES_ENOTDIR        (ES_OS_START_CANONERR + 5)
#endif

/** @see ES_STATUS_IS_ENOSPC */
#ifdef ENOSPC
#define ES_ENOSPC ENOSPC
#else
#define ES_ENOSPC         (ES_OS_START_CANONERR + 6)
#endif

/** @see ES_STATUS_IS_ENOMEM */
#ifdef ENOMEM
#define ES_ENOMEM ENOMEM
#else
#define ES_ENOMEM         (ES_OS_START_CANONERR + 7)
#endif

/** @see ES_STATUS_IS_EMFILE */
#ifdef EMFILE
#define ES_EMFILE EMFILE
#else
#define ES_EMFILE         (ES_OS_START_CANONERR + 8)
#endif

/** @see ES_STATUS_IS_ENFILE */
#ifdef ENFILE
#define ES_ENFILE ENFILE
#else
#define ES_ENFILE         (ES_OS_START_CANONERR + 9)
#endif

/** @see ES_STATUS_IS_EBADF */
#ifdef EBADF
#define ES_EBADF EBADF
#else
#define ES_EBADF          (ES_OS_START_CANONERR + 10)
#endif

/** @see ES_STATUS_IS_EINVAL */
#ifdef EINVAL
#define ES_EINVAL EINVAL
#else
#define ES_EINVAL         (ES_OS_START_CANONERR + 11)
#endif

/** @see ES_STATUS_IS_ESPIPE */
#ifdef ESPIPE
#define ES_ESPIPE ESPIPE
#else
#define ES_ESPIPE         (ES_OS_START_CANONERR + 12)
#endif

/** 
 * @see ES_STATUS_IS_EAGAIN 
 * @warning use ES_STATUS_IS_EAGAIN instead of just testing this value
 */
#ifdef EAGAIN
#define ES_EAGAIN EAGAIN
#elif defined(EWOULDBLOCK)
#define ES_EAGAIN EWOULDBLOCK
#else
#define ES_EAGAIN         (ES_OS_START_CANONERR + 13)
#endif

/** @see ES_STATUS_IS_EINTR */
#ifdef EINTR
#define ES_EINTR EINTR
#else
#define ES_EINTR          (ES_OS_START_CANONERR + 14)
#endif

/** @see ES_STATUS_IS_ENOTSOCK */
#ifdef ENOTSOCK
#define ES_ENOTSOCK ENOTSOCK
#else
#define ES_ENOTSOCK       (ES_OS_START_CANONERR + 15)
#endif

/** @see ES_STATUS_IS_ECONNREFUSED */
#ifdef ECONNREFUSED
#define ES_ECONNREFUSED ECONNREFUSED
#else
#define ES_ECONNREFUSED   (ES_OS_START_CANONERR + 16)
#endif

/** @see ES_STATUS_IS_EINPROGRESS */
#ifdef EINPROGRESS
#define ES_EINPROGRESS EINPROGRESS
#else
#define ES_EINPROGRESS    (ES_OS_START_CANONERR + 17)
#endif

/** 
 * @see ES_STATUS_IS_ECONNABORTED
 * @warning use ES_STATUS_IS_ECONNABORTED instead of just testing this value
 */
#ifdef ECONNABORTED
#define ES_ECONNABORTED ECONNABORTED
#else
#define ES_ECONNABORTED   (ES_OS_START_CANONERR + 18)
#endif

/** @see ES_STATUS_IS_ECONNRESET */
#ifdef ECONNRESET
#define ES_ECONNRESET ECONNRESET
#else
#define ES_ECONNRESET     (ES_OS_START_CANONERR + 19)
#endif

/** @see ES_STATUS_IS_ETIMEDOUT 
 *  @deprecated */
#ifdef ETIMEDOUT
#define ES_ETIMEDOUT ETIMEDOUT
#else
#define ES_ETIMEDOUT      (ES_OS_START_CANONERR + 20)
#endif

/** @see ES_STATUS_IS_EHOSTUNREACH */
#ifdef EHOSTUNREACH
#define ES_EHOSTUNREACH EHOSTUNREACH
#else
#define ES_EHOSTUNREACH   (ES_OS_START_CANONERR + 21)
#endif

/** @see ES_STATUS_IS_ENETUNREACH */
#ifdef ENETUNREACH
#define ES_ENETUNREACH ENETUNREACH
#else
#define ES_ENETUNREACH    (ES_OS_START_CANONERR + 22)
#endif

/** @see ES_STATUS_IS_EFTYPE */
#ifdef EFTYPE
#define ES_EFTYPE EFTYPE
#else
#define ES_EFTYPE        (ES_OS_START_CANONERR + 23)
#endif

/** @see ES_STATUS_IS_EPIPE */
#ifdef EPIPE
#define ES_EPIPE EPIPE
#else
#define ES_EPIPE         (ES_OS_START_CANONERR + 24)
#endif

/** @see ES_STATUS_IS_EXDEV */
#ifdef EXDEV
#define ES_EXDEV EXDEV
#else
#define ES_EXDEV         (ES_OS_START_CANONERR + 25)
#endif

/** @see ES_STATUS_IS_ENOTEMPTY */
#ifdef ENOTEMPTY
#define ES_ENOTEMPTY ENOTEMPTY
#else
#define ES_ENOTEMPTY     (ES_OS_START_CANONERR + 26)
#endif


/** 
 * BON Error Values
 */
/** Invalid argument */
#define BON_EINVAL         (ES_OS_START_USERERR + 1)
/** System error */
#define BON_ESYSTEM        (ES_OS_START_USERERR + 2)
/** Invalid node name */
#define BON_EINVNODENAME   (ES_OS_START_USERERR + 3)
/** No such node */
#define BON_ENONODE        (ES_OS_START_USERERR + 4)
/** No such attribute */
#define BON_ENOATTR        (ES_OS_START_USERERR + 5)
/** Invalid data postion */
#define BON_EINVDATAPOS    (ES_OS_START_USERERR + 6)
/** String syntax error */
#define BON_ESYNTAX        (ES_OS_START_USERERR + 7)
/** Head data error */
#define BON_EHEAD          (ES_OS_START_USERERR + 8)
/** Read data error */
#define BON_EREAD          (ES_OS_START_USERERR + 9)


#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_STATUS_H__ */

