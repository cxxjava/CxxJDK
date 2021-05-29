/*
 * EFork.cpp
 *
 *  Created on: 2013-3-19
 *      Author: cxxjava@163.com
 */

#include "EFork.hh"

namespace efc {

#define _LOCK	if (m_Lock) { \
					eso_proc_mutex_lock(m_Lock); \
				}
#define _UNLOCK	if (m_Lock) { \
					eso_proc_mutex_unlock(m_Lock); \
				}

EFork::~EFork() {
	
	//FIXME: need to kill child process?
	
	if (m_Lock) {
		eso_proc_mutex_destroy(&m_Lock);
	}
}

void EFork::init(sp<ERunnable> target, const char *name, boolean needLock) {
	ES_PROC_INIT(&m_Process);
	
	m_Target = target;
	
	if (name) {
		m_Name = name;
	}
	
	if (needLock) {
		m_Lock = eso_proc_mutex_create();
	}
	else {
		m_Lock = null;
	}
	
	m_Daemon = false;
	m_ppid = -1;
}

EFork::EFork(boolean needLock)
{
	init(null, null, needLock);
}

EFork::EFork(const char *name, boolean needLock)
{
	init(null, name, needLock);
}

EFork::EFork(sp<ERunnable> target, boolean needLock)
{
	init(target, null, needLock);
}

EFork::EFork(sp<ERunnable> target, const char *name, boolean needLock)
{
	init(target, name, needLock);
}

void EFork::setDaemon(boolean on)
{
	_LOCK;
	
	m_Daemon = on;
	
	_UNLOCK;
}

boolean EFork::isDaemon()
{
	boolean daemon;

	_LOCK;
	
	daemon = m_Daemon;

	_UNLOCK;

	return daemon;
}

void EFork::start()
{
	_LOCK;
	
	m_ppid = eso_os_process_current();

	if (m_Process.pid == 0) {
		es_status_t stat = eso_proc_fork(&m_Process);
		if (stat == ES_INCHILD) {
			if (m_Daemon) {
				eso_proc_detach(m_Daemon, 0, 0, 0);
			}
			
			run();
		}
		else if (stat == ES_INPARENT) {
			//nothing to do.
		}
		else {
			_UNLOCK;
			throw ERuntimeException(__FILE__, __LINE__, "Fork failed.");
		}
	}
	else {
		_UNLOCK;
		throw EIllegalStateException(__FILE__, __LINE__, "Already started.");
	}
	
	_UNLOCK;
}

void EFork::run() {
	if (m_Target != null) {
		m_Target->run();
	}
}

void EFork::exit(int status)
{
	_exit(status);
}

void EFork::kill(int sig)
{
	_LOCK;

	if (m_Process.pid > 0) {
		if (eso_proc_kill(&m_Process, sig) != ES_SUCCESS) {
			_UNLOCK;
			throw ERuntimeException(__FILE__, __LINE__);
		}
	}

	_UNLOCK;
}

int EFork::waitFor(es_exit_why_e *exitwhy)
{
	int exitcode = 0;
	_LOCK;

	if (m_Process.pid > 0) {
		es_status_t stat = eso_proc_wait(&m_Process, &exitcode, exitwhy, ES_WAIT);
		if (stat == ES_CHILD_DONE) {
			m_Process.pid = 0;
			_UNLOCK;
			return exitcode;
		}
	}

	_UNLOCK;
	throw ERuntimeException(__FILE__, __LINE__);
}

int EFork::exitValue(es_exit_why_e *exitwhy)
{
	int exitcode = 0;
	_LOCK;

	es_status_t stat = eso_proc_wait(&m_Process, &exitcode, exitwhy, ES_NOWAIT);
	if (stat == ES_CHILD_DONE) {
		m_Process.pid = 0;
		_UNLOCK;
		return exitcode;
	}

	if (stat == ES_CHILD_NOTDONE) {
		_UNLOCK;
		throw EInterruptedException(__FILE__, __LINE__, "Child is still running.");
	}

	_UNLOCK;
	throw ERuntimeException(__FILE__, __LINE__);
}

boolean EFork::isAlive()
{
	boolean active;

	_LOCK;

	active = (m_Process.pid > 0) ? true : false;

	_UNLOCK;

	return active;
}

char* EFork::getName()
{
	return (char*)m_Name.c_str();
}

es_os_pid_t EFork::getPid() {
	return m_Process.pid;
}

es_os_pid_t EFork::getPPid() {
	return m_ppid;
}

EString EFork::toString()
{
	EString out;
	return out.format("Process[%s]", getName());
}

es_os_pid_t EFork::waitAny(int *exitcode, es_exit_why_e *exitwhy,
			                   es_wait_how_e waithow)
{
	es_proc_t proc = {0};
	es_status_t stat = eso_proc_wait_any_proc(&proc, exitcode, exitwhy, waithow);
	if (stat != ES_CHILD_DONE && stat != ES_CHILD_NOTDONE) {
		throw ERuntimeException(__FILE__, __LINE__);
	}
	return proc.pid;
}

} /* namespace efc */
