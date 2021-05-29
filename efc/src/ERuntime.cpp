/*
 * ERuntime.cpp
 *
 *  Created on: 2014-9-18
 *      Author: cxxjava@163.com
 */

#include "ERuntime.hh"
#include "EOS.hh"
#include "EStringTokenizer.hh"
#include "EToDoException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

//=============================================================================

ERuntime* ERuntime::currentRuntime;

DEFINE_STATIC_INITZZ_BEGIN(ERuntime)
	EOS::_initzz_();
	currentRuntime = new ERuntime();
DEFINE_STATIC_INITZZ_END

//=============================================================================

ERuntime* ERuntime::getRuntime() {
	return currentRuntime;
}

EProcess* ERuntime::exec(const char* command) {
	return this->exec(command, (const char**)null, (const char*)null);
}

EProcess* ERuntime::exec(const char* command, const char* envp[]) {
	return this->exec(command, envp, (const char*)null);
}

EProcess* ERuntime::exec(const char* command, EArray<EString*>* envp) {
	return this->exec(command, envp, (const char*)null);
}

EProcess* ERuntime::exec(const char* command, const char* envp[], EFile* dir) {
	EString s;
	if (dir) {
		s = dir->getAbsolutePath();
	}
	return this->exec(command, envp, s.c_str());
}

EProcess* ERuntime::exec(const char* command, EArray<EString*>* envp,  EFile* dir) {
	EString s;
	if (dir) {
		s = dir->getAbsolutePath();
	}
	return this->exec(command, envp, s.c_str());
}

EProcess* ERuntime::exec(const char* command, const char* envp[], const char* dir) {
	if (!command || !*command)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Empty command");

	EProcess* process = new EProcess(command);
	process->environment(envp);
	process->directory(dir);
	process->start();
	return process;
}

EProcess* ERuntime::exec(const char* command, EArray<EString*>* envp, const char* dir) {
	if (!command || !*command)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Empty command");

	EProcess* process = new EProcess(command);
	process->environment(envp);
	process->directory(dir);
	process->start();
	return process;
}

int ERuntime::availableProcessors() {
	return EOS::active_processor_count();
}

ullong ERuntime::freeMemory() {
	return EOS::available_memory();
}

ullong ERuntime::totalMemory() {
	return EOS::physical_memory();
}

ullong ERuntime::maxMemory() {
	return EOS::physical_memory();
}

} /* namespace efc */
