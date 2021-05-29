/*
 * EProcess.cpp
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EProcess.hh"
#include "EStringTokenizer.hh"
#include "ENullPointerException.hh"

namespace efc {

EProcess::~EProcess()
{
	eso_pipe_destroy(&m_Process.in);
	eso_pipe_destroy(&m_Process.out);
	eso_pipe_destroy(&m_Process.err);

	delete m_Stdin;
	delete m_Stdout;
	delete m_Stderr;

	destroy();
}

EProcess::EProcess() :
		m_Detached(false), m_Stdin(null), m_Stdout(null), m_Stderr(null)
{
	ES_PROC_INIT(&m_Process);
}

EProcess::EProcess(const char* command) :
						m_Detached(false), m_Stdin(null), m_Stdout(null), m_Stderr(null)
{
	ES_PROC_INIT(&m_Process);
	this->command(command);
}

EProcess::EProcess(const char* program, const char* args[]) :
						m_Detached(false), m_Stdin(null), m_Stdout(null), m_Stderr(null)
{
	ES_PROC_INIT(&m_Process);
	this->command(program, args);
}

EProcess& EProcess::command(const char* command) {
	EStringTokenizer st(command);
	for (int i = 0; st.hasMoreTokens(); i++) {
		if (i == 0) {
			m_Prog = st.nextToken().c_str();
			m_Commands.add(new EString(m_Prog.c_str()));
		}
		else {
			m_Commands.add(new EString(st.nextToken().c_str()));
		}
	}

	return *this;
}

EProcess& EProcess::command(const char* program, const char* args[]) {
	m_Prog = program;

	m_Commands.add(new EString(program));
	for (int i = 0; args && args[i]; ++i) {
		m_Commands.add(new EString(args[i]));
	}

	return *this;
}

EProcess& EProcess::commandFormat(const char* fmt, ...) {
	EString cmd;

	va_list args;
	va_start(args, fmt);
	cmd.vformat(fmt, args);
	va_end(args);

	return command(cmd.c_str());
}

EProcess& EProcess::commandArguments(const char* command, ...)
{
	if (!command || !*command)
		throw ENullPointerException(__FILE__, __LINE__, "Empty command");

	m_Prog = command;

	m_Commands.add(new EString(command));
	va_list args;
	va_start(args, command);
	char *v;
	while ((v = va_arg (args, char*))) {
		m_Commands.add(new EString(v));
	}
	va_end(args);

	return *this;
}

EProcess& EProcess::command(const char* program, EArray<EString*>* envp) {
	if (!program || !*program)
		throw ENullPointerException(__FILE__, __LINE__, "Empty program");

	m_Prog = program;

	if (envp) {
		for (int i=0; i<envp->length(); i++) {
			m_Environments.add(new EString(envp->getAt(i)->c_str()));
		}
	}

	return *this;
}

EArray<EString*>& EProcess::command() {
	return m_Commands;
}

EArray<EString*>& EProcess::environment() {
	return m_Environments;
}

EProcess& EProcess::environment(const char* envp[]) {
	for (int i = 0; envp && envp[i]; ++i) {
		m_Environments.add(new EString(envp[i]));
	}

	return *this;
}

EProcess& EProcess::environment(EArray<EString*>* envp) {
	if (envp) {
		int len = envp->length();
		for (int i = 0; i < len; i++) {
			m_Environments.add(new EString((*envp)[i]->c_str()));
		}
	}
	return *this;
}

EProcess& EProcess::environmentList(const char* envp0, ...)
{
	if (!envp0 || !*envp0)
		return *this;

	m_Environments.add(new EString(envp0));
	va_list args;
	va_start(args, envp0);
	char *v;
	while ((v = va_arg (args, char*))) {
		m_Environments.add(new EString(v));
	}
	va_end(args);

	return *this;
}

EProcess& EProcess::directory(EFile* directory) {
	if (directory)
		this->m_Dir = directory->getAbsolutePath();
	return *this;
}

EProcess& EProcess::directory(const char* directory)
{
	if (directory)
		this->m_Dir = directory;
	return *this;
}

const char* EProcess::directory()
{
	return m_Dir.c_str();
}

void EProcess::setDaemon(boolean on)
{
	m_Detached = on;
}

boolean EProcess::isDaemon()
{
	return m_Detached;
}

void EProcess::start() THROWS3(EIOException,EIllegalStateException,ERuntimeException)
{
	if (m_Process.pid == 0) {
		if (!m_Detached) {
			m_Process.in = eso_pipe_create();
			m_Process.out = eso_pipe_create();
			m_Process.err = eso_pipe_create();
		}

		int len = m_Commands.size();
		const char **args = (const char **) eso_calloc(sizeof(char*) * (len + 3));
		for (int i = 0; i < len; i++) {
			args[i] = m_Commands.getAt(i)->c_str();
		}

		len = m_Environments.size();
		const char **envp = (const char **) eso_calloc(sizeof(char*) * (len + 3));
		for (int i = 0; i < len; i++) {
			envp[i] = m_Environments.getAt(i)->c_str();
		}

		es_status_t stat = eso_proc_create(&m_Process, m_Prog.c_str(), args, envp,
				(m_Dir.length() > 0) ? m_Dir.c_str() : NULL, m_Detached);

		eso_free(envp);
		eso_free(args);

		if (m_Process.in) {
			eso_fclose(m_Process.in->in);
			m_Process.in->in = NULL;
		}
		if (m_Process.out) {
			eso_fclose(m_Process.out->out);
			m_Process.out->out = NULL;
		}
		if (m_Process.err) {
			eso_fclose(m_Process.err->out);
			m_Process.err->out = NULL;
		}

		if (stat == ES_NOTFOUND) {
			throw EIOException(__FILE__, __LINE__, "Program not found.");
		} else if (stat != ES_SUCCESS) {
			throw ERuntimeException(__FILE__, __LINE__);
		}
	} else {
		throw EIllegalStateException(__FILE__, __LINE__, "Already started.");
	}
}

EOutputStream* EProcess::getOutputStream()
{
	if (m_Process.in && !m_Stdin) {
		m_Stdin = new EFileOutputStream(m_Process.in->out);
	}
	return m_Stdin;
}

EInputStream* EProcess::getInputStream()
{
	if (m_Process.out && !m_Stdout) {
		m_Stdout = new EFileInputStream(m_Process.out->in);
	}
	return m_Stdout;
}

EInputStream* EProcess::getErrorStream()
{
	if (m_Process.err && !m_Stderr) {
		m_Stderr = new EFileInputStream(m_Process.err->in);
	}
	return m_Stderr;
}

int EProcess::waitFor()
{
	if (m_Process.pid > 0) {
		int exitcode = 0;
		es_status_t stat = eso_proc_wait(&m_Process, &exitcode, NULL, ES_WAIT);
		if (stat == ES_CHILD_DONE) {
			m_Process.pid = 0;
			return exitcode;
		}
	}
	throw ERuntimeException(__FILE__, __LINE__);
}

int EProcess::exitValue() THROWS(EInterruptedException)
{
	if (m_Process.pid > 0) {
		int exitcode = 0;
		es_status_t stat = eso_proc_wait(&m_Process, &exitcode, NULL,
				ES_NOWAIT);
		if (stat == ES_CHILD_DONE) {
			m_Process.pid = 0;
			return exitcode;
		}

		if (stat == ES_CHILD_NOTDONE) {
			throw EInterruptedException(__FILE__, __LINE__, "Child is still running.");
		}
	}
	throw ERuntimeException(__FILE__, __LINE__);
}

void EProcess::destroy()
{
	if (m_Process.pid > 0) {
		eso_proc_kill(&m_Process, SIGTERM);
	}
}

} /* namespace efc */
