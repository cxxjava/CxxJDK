/*
 * ESequenceInputStream.cpp
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#include "ESequenceInputStream.hh"
#include "EVector.hh"
#include "ENullPointerException.hh"

namespace efc {

ESequenceInputStream::~ESequenceInputStream() {
	if (_eOwned) {
		delete _e;
	}
	delete _v;
}

ESequenceInputStream::ESequenceInputStream(EEnumeration<EInputStream*>* e) :
	_e(e), _eOwned(false), _in(null), _v(null) {
	try {
		nextStream();
	} catch (EIOException& ex) {
		// This should never happen
		throw EException(__FILE__, __LINE__, "panic");
	}
}

ESequenceInputStream::ESequenceInputStream(EInputStream* s1, EInputStream* s2) :
	_e(null), _eOwned(true), _in(null), _v(null) {
	_v = new EVector<EInputStream*>(false, 2);
	_v->setThreadSafe(false);

	_v->add(s1);
	_v->add(s2);
	_e = _v->elements().dismiss();
	try {
		nextStream();
	} catch (EIOException& ex) {
		// This should never happen
		throw EException(__FILE__, __LINE__, "panic");
	}
}

long ESequenceInputStream::available() {
	if (_in == null) {
		return 0; // no way to signal EOF from available()
	}
	return _in->available();
}

int ESequenceInputStream::read() {
	if (_in == null) {
		return -1;
	}
	int c = _in->read();
	if (c == -1) {
		nextStream();
		return read();
	}
	return c;
}

int ESequenceInputStream::read(void* b, int len) {
	ES_ASSERT(b);

	if (_in == null) {
		return -1;
	} else if (len == 0) {
		return 0;
	}

	int n = _in->read(b, len);
	if (n <= 0) {
		nextStream();
		return read(b, len);
	}
	return n;
}

void ESequenceInputStream::close() {
	do {
		nextStream();
	} while (_in != null);
}

void ESequenceInputStream::nextStream() {
	if (_in != null) {
		_in->close();
	}

	if (_e->hasMoreElements()) {
		_in = (EInputStream*) _e->nextElement();
		if (_in == null)
			throw ENullPointerException(__FILE__, __LINE__);
	} else {
		_in = null;
	}
}

} /* namespace efc */
