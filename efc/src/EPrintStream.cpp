/*
 * EPrintStream.cpp
 *
 *  Created on: 2013-3-16
 *      Author: cxxjava@163.com
 */

#include "EPrintStream.hh"
#include "EThread.hh"
#include "EFileOutputStream.hh"
#include "EInterruptedIOException.hh"

namespace efc {

EPrintStream::~EPrintStream() {
	/* 20180207: removed for destroy error when owned object freed before!
	try {
		close();
	} catch (...) {
	}
	*/
}

EPrintStream::EPrintStream(EOutputStream *out, boolean autoFlush, boolean owned) :
		EFilterOutputStream(out, owned), m_autoFlush(autoFlush),
		m_trouble(false) {
}

EPrintStream::EPrintStream(const char* fileName) :
		EFilterOutputStream(new EFileOutputStream(fileName), true),
		m_autoFlush(false), m_trouble(false) {
}

EPrintStream::EPrintStream(EFile* file) :
		EFilterOutputStream(new EFileOutputStream(file), true),
		m_autoFlush(false), m_trouble(false) {
}

void EPrintStream::write(const void *b, int len) {
	try {
		SYNCBLOCK(&m_lock) {
			ensureOpen();
			_out->write(b, len);
			if (m_autoFlush)
				_out->flush();
        }}
	} catch (EInterruptedIOException& x1) {
		EThread::currentThread()->interrupt();
	} catch (EIOException& x2) {
		m_trouble = true;
	}
}

void EPrintStream::flush() {
	try {
		SYNCBLOCK(&m_lock) {
			ensureOpen();
			_out->flush();
        }}
	} catch (EIOException& x2) {
		m_trouble = true;
	}
}

void EPrintStream::close() {
	SYNCBLOCK(&m_lock) {
		if (!closed) {
			closed = true;
			try {
				_out->close();
			} catch (EIOException& x) {
				m_trouble = true;
			}
			if (_owned) {
				delete _out; //!!!
			}
			_out = null;
		}
    }}
}

boolean EPrintStream::checkError() {
	if (_out != null)
		flush();
	EPrintStream* ps = dynamic_cast<EPrintStream*>(_out);
	if (ps) {
		return ps->checkError();
	}
	return m_trouble;
}

EPrintStream& EPrintStream::printf(const char* fmt, ...) THROWS(EIOException)
{
	EString s;

	va_list args;
	va_start(args, fmt);
	s.vformat(fmt, args);
	va_end(args);

	write(s.c_str(), s.length());

	return *this;
}

EPrintStream& EPrintStream::print(boolean b) {
	if (b) {
		write("true", 4);
	} else {
		write("false", 5);
	}
	return *this;
}

EPrintStream& EPrintStream::print(char c) {
	EString s = EString::valueOf(c);
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::print(int i) {
	EString s = EString::valueOf(i);
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::print(llong l) {
	EString s = EString::valueOf(l);
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::print(float f) {
	EString s = EString::valueOf(f);
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::print(double d) {
	EString s = EString::valueOf(d);
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::print(const char* s, int len) {
	if (s == null) {
		s = "null";
		len = 4;
	}
	if (len < 0) {
		len = eso_strlen(s);
	}
	write(s, len);
	return *this;
}

EPrintStream& EPrintStream::println() {
	write((void*)"\n", 1);
	return *this;
}

EPrintStream& EPrintStream::printfln(const char* fmt, ...)
{
	EString s;

	va_list args;
	va_start(args, fmt);
	s.vformat(fmt, args);
	va_end(args);

	s.append("\n");

	write(s.c_str(), s.length());

	return *this;
}

EPrintStream& EPrintStream::println(boolean x) {
	EString s = EString::valueOf(x);
	s.append("\n");
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::println(char x) {
	EString s = EString::valueOf(x);
	s.append("\n");
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::println(int x) {
	EString s = EString::valueOf(x);
	s.append("\n");
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::println(llong x) {
	EString s = EString::valueOf(x);
	s.append("\n");
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::println(float x) {
	EString s = EString::valueOf(x);
	s.append("\n");
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::println(double x) {
	EString s = EString::valueOf(x);
	s.append("\n");
	write(s.c_str(), s.length());
	return *this;
}

EPrintStream& EPrintStream::println(const char* x, int len) {
	if (x == null) {
		x = "null";
		len = 4;
	}
	if (len < 0) {
		len = eso_strlen(x);
	}
	EString s(x, 0, len);
	s.append("\n");
	write(s.c_str(), s.length());
	return *this;
}

void EPrintStream::ensureOpen() {
	if (_out == null)
		throw EIOException(__FILE__, __LINE__, "Stream closed");
}

} /* namespace efc */
