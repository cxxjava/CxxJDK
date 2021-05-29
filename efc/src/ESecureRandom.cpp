/*
 * ESecureRandom.cpp
 *
 *  Created on: 2016-6-23
 *      Author: cxxjava@163.com
 */

#include "es_config.h" //for WIN32_LEAN_AND_MEAN
#ifdef WIN32
#include <windows.h>
#include <wincrypt.h>
#endif
#include "ESecureRandom.hh"
#include "ERandomAccessFile.hh"
#include "ENullPointerException.hh"

namespace efc {

#ifdef WIN32

void ESecureRandom::uninit() {
	if (provider != 0) {
		HCRYPTPROV context = (HCRYPTPROV)provider;
		CryptReleaseContext(context, 0);
	}
}

void ESecureRandom::init() {
	HCRYPTPROV context;
	int result = CryptAcquireContext(&context, NULL, NULL, PROV_DSS, CRYPT_VERIFYCONTEXT);
	if (result != 0) { //success
		provider = (llong)context;
	}
}

void ESecureRandom::nextBytes(byte* bytes, int size) {
	boolean succ = false;

	try {
		if (provider != 0) {
			HCRYPTPROV context = (HCRYPTPROV)provider;
			BYTE b;
			int result = CryptGenRandom(context, size, &b);
			if (result != 0) {
				if (bytes) *bytes = (byte)b;
				succ = true;
			}
		}
	} catch (...) {
	}

	if (!succ) {
		ERandom::nextBytes(bytes, size);
	}
}

#else

void ESecureRandom::uninit() {
	if (provider != 0) {
		ERandomAccessFile* raf = (ERandomAccessFile*)provider;
		delete raf;
	}
}

void ESecureRandom::init() {
	const char* files[] = { "/dev/urandom", "/dev/random" };
	int index = 0;
	ERandomAccessFile* raf = null;

	do {
		try {
			raf = new ERandomAccessFile(files[index++], "rb");
		} catch (...) {
		}
	} while( index < 2 && raf == null);

	if (raf != NULL) {
		provider = (llong)raf;
	}
}

void ESecureRandom::nextBytes(byte* bytes, int size) {
	boolean succ = false;

	try {
		if (provider != 0) {
			ERandomAccessFile* raf = (ERandomAccessFile*)provider;
			raf->readFully(bytes, size);
			succ = true;
		}
	} catch (...) {
	}

	if (!succ) {
		ERandom::nextBytes(bytes, size);
	}
}

#endif //!WIN32

//=============================================================================

ESecureRandom::~ESecureRandom() {
	this->uninit();
}

ESecureRandom::ESecureRandom() : ERandom(), provider(0) {
	this->init();
}

ESecureRandom::ESecureRandom(llong seed) : ERandom(seed), provider(0) {
	this->init();
}

void ESecureRandom::nextBytes(EA<byte>* bytes) {
	if (!bytes) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	this->nextBytes(bytes->address(), bytes->length());
}

int ESecureRandom::next(int numBits) {
	if (numBits < 0) {
		numBits = 0;
	} else if (numBits > 32) {
		numBits = 32;
	}

	int bytes = (numBits + 7) / 8;
	byte* next = new byte[bytes];
	uint ret = 0;

	this->nextBytes(next, bytes);
	for (int i = 0; i < bytes; i++) {
		ret = (next[i] & 0xFF) | (ret << 8);
	}

	ret = ret >> ((bytes * 8) - numBits);

	delete[] next;

	return ret;
}

} /* namespace efc */
