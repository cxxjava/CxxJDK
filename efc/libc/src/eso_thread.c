/**
 * @file  eso_thread.c
 * @brief OS thread
 */

#include "eso_thread.h"
#include "eso_libc.h"

#ifdef WIN32
#include <windows.h>
#else
#ifdef __APPLE__
#include <sched.h>
#include <unistd.h>
#endif//
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/syscall.h>
#endif

#ifdef WIN32
/* Chosen for us by eso_initialize */
DWORD g_tls_thread = 0;
#else
	#ifdef __APPLE__
	static pid_t gettid()
	{
		return (pid_t) syscall(SYS_thread_selfid);
	}
	#else
	static pid_t gettid()
	{
		return (pid_t) syscall(SYS_gettid);
	}
	#endif
#endif

#define INTERRUPT_SIGNAL SIGUSR1

/**
 * OS线程
 */

es_status_t eso_threadattr_init(es_threadattr_t *attr)
{
#ifdef WIN32

	attr->detach = 0;
    attr->stacksize = 0;
    return ES_SUCCESS;
    
#else

	es_status_t stat = pthread_attr_init(&attr->attr);
    if (stat == 0) {
        return ES_SUCCESS;
    }
#ifdef PTHREAD_SETS_ERRNO
    stat = errno;
#endif
    return stat;
    
#endif
}

void eso_threadattr_destroy(es_threadattr_t *attr)
{
#ifdef WIN32
	//
#else
	if (attr) {
		pthread_attr_destroy(attr);
	}
#endif
}

es_status_t eso_threadattr_detach_set(es_threadattr_t *attr, es_bool_t on)
{
#ifdef WIN32

	attr->detach = on;
    return ES_SUCCESS;
    
#else

	#define DETACH_ARG(v) ((v) ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE)
	es_status_t stat;
	if ((stat = pthread_attr_setdetachstate(&attr->attr, 
                                            DETACH_ARG(on))) == 0) {
		return ES_SUCCESS;                               	
	}
	else {
#ifdef PTHREAD_SETS_ERRNO
        stat = errno;
#endif
        return stat;
    }
    
#endif
}

es_bool_t eso_threadattr_detach_get(es_threadattr_t *attr)
{
#ifdef WIN32

	return attr->detach;
	
#else

	int state;
	pthread_attr_getdetachstate(&attr->attr, &state);
    if (state == 1)
        return TRUE;
    return FALSE;
    
#endif
}

/**
 * stacksize
 * 改变新创建线程的栈容量。这个值确定了栈的最小体积，单位为字节（byte）。
 * 如果属性值超过了系统最大栈容量，或者小于 PTHREAD_STACK_MIN，
 * pthread_attr_setstacksize 会失败并返回 EINVAL。
 */
es_status_t eso_threadattr_stacksize_set(es_threadattr_t *attr, es_size_t stacksize)
{
#ifdef WIN32

	attr->stacksize = stacksize;
    return ES_SUCCESS;
    
#else

	int stat = pthread_attr_setstacksize(&attr->attr, stacksize);
    if (stat == 0) {
        return ES_SUCCESS;
    }
#ifdef PTHREAD_SETS_ERRNO
    stat = errno;
#endif
    return stat;
    
#endif
}

/**
 * guardsize
 * 改变线程栈的警戒区域的最小容积。默认值为一个内存分页的大小。如果这个属性被设置，
 * 它的值将被向上靠拢到最接近的内存分页大小的整数倍。如果值为 0，则系统将不会为新
 * 线程设置警戒区域。被分配作为警戒区域的内存将被用于捕获栈溢出状况。因此，当尝试
 * 在栈上分配体积巨大的结构体时，可能需要一个较大的警戒区域以捕获栈溢出的可能。
 * 如果调用者自主管理栈区域（如果 stackaddr 属性被设置），则 guardsize 属性将被忽略。
 * 如果这个值的大小超过了 stacksize，pthread_attr_setguardsize 将会失败并返回 EINVAL。
 */
es_status_t eso_threadattr_guardsize_set(es_threadattr_t *attr, es_size_t guardsize)
{
#ifdef WIN32

	return ES_SUCCESS;
	
#else

	es_status_t rv = pthread_attr_setguardsize(&attr->attr, guardsize);
    if (rv == 0) {
        return ES_SUCCESS;
    }
#ifdef PTHREAD_SETS_ERRNO
    rv = errno;
#endif
    return rv;
    
#endif
}

static void *dummy_worker(void *opaque)
{
    es_thread_t *thread = (es_thread_t*)opaque;
#ifdef WIN32
	TlsSetValue(g_tls_thread, thread->td);
#endif
    return thread->func(thread);
}

es_thread_t* eso_thread_create(es_threadattr_t *attr,
                               es_thread_func_t *func, 
                               void *data)
{
#ifdef WIN32

	es_thread_t *thread;
    HANDLE handle;
	unsigned temp;
    
    thread = (es_thread_t*)eso_calloc(sizeof(es_thread_t));
	if (!thread) {
		return NULL;
	}
	thread->data = data;
    thread->func = func;
	
	if ((handle = CreateThread(NULL,
                        attr && attr->stacksize > 0 ? attr->stacksize : 0,
                        (LPTHREAD_START_ROUTINE)dummy_worker,
                        thread, 0, &temp)) == 0) {
        return NULL;
    }
    
    if (attr && attr->detach) {
        CloseHandle(handle);
    }
    else
        thread->td = handle;
    
    return thread;
    
#else //!

	es_thread_t *thread;
	pthread_attr_t *temp;
	
	thread = (es_thread_t*)eso_calloc(sizeof(es_thread_t));
	if (!thread) {
		return NULL;
	}
	thread->data = data;
    thread->func = func;
    
	if (attr)
        temp = &attr->attr;
    else
        temp = NULL;
    
	if (pthread_create(&thread->td, temp, dummy_worker, thread) == 0) {
        return thread;
    }
    else {
		eso_free(thread);
		return NULL;
    }
    
#endif
}

void eso_thread_destroy(es_thread_t **thd)
{
	if (!thd || !*thd)
		return;

	ESO_FREE(thd);
}

es_status_t eso_thread_exit(es_thread_t *thd, 
                            es_status_t retval)
{
#ifdef WIN32
	
	thd->exitval = retval;    
    ExitThread(0);
    
    return ES_SUCCESS;

#else

	thd->exitval = retval;
    pthread_exit(NULL);
	
    return ES_SUCCESS;

#endif
}

es_status_t eso_thread_join(es_status_t *retval, 
                            es_thread_t *thd)
{
#ifdef WIN32

	es_status_t rv = ES_SUCCESS;
    
    if (!thd->td) {
        /* Can not join on detached threads */
        return ES_FAILURE;
    }
    rv = WaitForSingleObject(thd->td, INFINITE);
    if ( rv == WAIT_OBJECT_0 || rv == WAIT_ABANDONED) {
        /* If the thread_exit has been called */
        if (retval) *retval = thd->exitval;
    }
    else
        rv = GetLastError();
    CloseHandle(thd->td);
    thd->td = NULL;

    return rv;
	
#else

	es_status_t stat;
    es_status_t *thread_stat;

    if ((stat = pthread_join(thd->td,(void *)&thread_stat)) == 0) {
    	if (retval) *retval = thd->exitval;
        return ES_SUCCESS;
    }
    else {
#ifdef PTHREAD_SETS_ERRNO
        stat = errno;
#endif
        return stat;
    }

#endif
}

es_status_t eso_thread_detach(es_thread_t *thd)
{
#ifdef WIN32

	if (thd->td && CloseHandle(thd->td)) {
        thd->td = NULL;
        return ES_SUCCESS;
    }
    else {
        return GetLastError();
    }

#else

	es_status_t stat;

    if ((stat = pthread_detach(thd->td)) == 0) {
        return ES_SUCCESS;
    }
    else {
#ifdef PTHREAD_SETS_ERRNO
        stat = errno;
#endif
        return stat;
    }

#endif
}

void eso_thread_yield(void)
{
#ifdef WIN32
	/**
	 * 两个都是放弃时间片，按照书中或者网上的一般说法，Sleep(0)将会在可调度的同等级或者高优先级的线程里面找，
	 * 而SwitchToThread则是全系统范围，低优先级的也可能会被调用。
	 * SwitchToThread() need _WIN32_WINNT >= 0x400
	 */
	Sleep(0);
#else
#ifdef __APPLE__
	sched_yield();
#else
	pthread_yield();
#endif

#endif
}

//==============================================================================

void eso_thread_sleep(long milliseconds)
{
#ifdef WIN32
	Sleep(milliseconds);
#else
	/* @see http://bbs.chinaunix.net/thread-790727-1-1.html */
	
	struct timespec remain;
	remain.tv_sec = milliseconds / 1000;
	remain.tv_nsec = (milliseconds % 1000) * ES_NANOS_PER_MILLISEC;
	
	do {
		if (nanosleep(&remain, &remain) == 0 || errno != EINTR) {
			break;
		}
	} while(1);
#endif
	errno = 0; //reset!
}

//==============================================================================

es_status_t eso_thread_once(void (*func)(void))
{
#ifdef WIN32
	long value = 0L;
	if (!InterlockedExchange(&value, 1)) {
        func();
    }
    return ES_SUCCESS;
#else
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	return pthread_once(&once, func);  //fixed on 20140326
#endif
}

//==============================================================================

es_os_thread_t eso_os_thread_current(void)
{
#ifdef WIN32
	HANDLE hthread = (HANDLE)TlsGetValue(g_tls_thread);
    HANDLE hproc;

    if (hthread) {
        return hthread;
    }
    
    hproc = GetCurrentProcess();
    hthread = GetCurrentThread();
    if (!DuplicateHandle(hproc, hthread, 
                         hproc, &hthread, 0, FALSE, 
                         DUPLICATE_SAME_ACCESS)) {
        return NULL;
    }
    TlsSetValue(g_tls_thread, hthread);
    return hthread;
#else
	return pthread_self();
#endif
}

es_os_thread_t eso_os_thread_get(es_thread_t *thethd)
{
	return thethd->td;
}

es_bool_t eso_os_thread_equal(es_os_thread_t tid1, es_os_thread_t tid2)
{
#ifdef WIN32
	/* Since the only tid's we support our are own, and
     * apr_os_thread_current returns the identical handle
     * to the one we created initially, the test is simple.
     */
    return (tid1 == tid2);
#else
	return pthread_equal(tid1, tid2);
#endif
}

int eso_os_thread_sigaction_init(void(*handler)(int sig))
{
#ifdef WIN32
	//not support.
#else
	/* Install the null handler for INTERRUPT_SIGNAL.  This might overwrite the
	 * handler previously installed by java/net/linux_close.c, but that's okay
	 * since neither handler actually does anything.  We install our own
	 * handler here simply out of paranoia; ultimately the two mechanisms
	 * should somehow be unified, perhaps within the VM.
	 */

	struct sigaction sa, osa;
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if (sigaction(INTERRUPT_SIGNAL, &sa, &osa) < 0)
		return -1;
#endif
	return 0;
}

void eso_os_thread_kill(es_os_thread_t thread)
{
#ifdef WIN32
	//not support.
#else
	pthread_kill((pthread_t)thread, INTERRUPT_SIGNAL);
#endif
}

es_ulong_t eso_os_thread_current_id(void)
{
#ifdef WIN32
	return GetCurrentThreadId();
#else //
	return gettid();
#endif //!
}

es_status_t eso_os_thread_priority_set(es_ulong_t tid, int priority)
{
#ifdef WIN32
	es_status_t rv = SetThreadPriority((HANDLE)tid, priority);
	return rv ? ES_SUCCESS : GetLastError();
#else
	es_status_t stat = setpriority(PRIO_PROCESS, tid, priority);
    if (stat == 0) {
        return ES_SUCCESS;
    }
#ifdef PTHREAD_SETS_ERRNO
    stat = errno;
#endif
    return stat;
#endif
}

es_status_t eso_os_thread_priority_get(es_ulong_t tid, int *ppriority)
{
#ifdef WIN32
	int os_prio = GetThreadPriority((HANDLE)tid);
	if (os_prio == THREAD_PRIORITY_ERROR_RETURN) {
		return GetLastError();
	}
	if (ppriority) *ppriority = os_prio;
	return ES_SUCCESS;
#else
	int os_prio = getpriority(PRIO_PROCESS, tid);
	if (os_prio != -1 || errno == 0) {
		if (ppriority) *ppriority = os_prio;
		return ES_SUCCESS;
	}
	else {
#ifdef PTHREAD_SETS_ERRNO
    return errno;
#else
    return os_prio;
#endif
	}
#endif
}
