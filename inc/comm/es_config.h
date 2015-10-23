/**
 * @file  es_config.h
 * @brief ES Prefix Definitions
 */

#ifndef ES_CONFIG_H_
#define ES_CONFIG_H_

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN //@see: http://www.cppblog.com/tx7do/archive/2008/04/20/47654.html

#elif defined(__sun) //solaris

#elif defined(__linux__) //linux

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) //bsd

#else //other.

#endif


/**
 * The accept4 system call which exists from kernel 2.6.28
 * and from glibc 2.10 (both conditions must be satisfied).
 */
//#define HAVE_ACCEPT4

//#define HAVE_IPV6

#define HAVE_THREADS

/**
 *
 */
#define AQS_SUPPORT_THREAD_DAEMON 0

#endif /* ES_CONFIG_H_ */
