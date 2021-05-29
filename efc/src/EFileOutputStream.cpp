/*
 * EFileOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EFileOutputStream.hh"
#include "../nio/inc/EFileChannel.hh"

#ifdef WIN32
#include <io.h>
#endif

namespace efc {

#define DEFAULT_STAT_BLKSIZE 4096

inline size_t restartable_write(es_file_t* pfile, const void *buf, unsigned int nBytes) {
	size_t res;
	int ret;
	do {
		res = ::fwrite(buf, 1, nBytes, pfile);
		ret = eso_ferror(pfile);
	} while (res != nBytes && ret && (errno == EINTR));
	return ret ? -1 : res;
}

EFileOutputStream::~EFileOutputStream()
{
	try {
		close();
	} catch (...) {
	}

	delete channel;
}

EFileOutputStream::EFileOutputStream(const char *name) :
		needClose(true),
		closed(false),
		append(false),
		channel(null)
{
	ES_ASSERT(name);
	mFile = eso_fopen(name, "wb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__, name);
	}
	mBuffered = false;
	if (!mBuffered) {
		eso_file_setbuffer(mFile, NULL, 0);
	}
}

EFileOutputStream::EFileOutputStream(const char *name, boolean append) :
		needClose(true),
		closed(false),
		append(append),
		channel(null)
{
	ES_ASSERT(name);
	mFile = eso_fopen(name, append ? "ab" : "wb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__, name);
	}
	mBuffered = false;
	if (!mBuffered) {
		eso_file_setbuffer(mFile, NULL, 0);
	}
}

EFileOutputStream::EFileOutputStream(EFile *file) :
		needClose(true),
		closed(false),
		append(false),
		channel(null)
{
	ES_ASSERT(file);
	mFile = eso_fopen(file->getPath().c_str(), "wb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__, file->getPath().c_str());
	}
	mBuffered = false;
	if (!mBuffered) {
		eso_file_setbuffer(mFile, NULL, 0);
	}
}

EFileOutputStream::EFileOutputStream(EFile *file, boolean append) :
			needClose(true),
			closed(false),
			append(append),
			channel(null)
{
	ES_ASSERT(file);
	mFile = eso_fopen(file->getPath().c_str(), append ? "ab" : "wb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__, file->getPath().c_str());
	}
	mBuffered = false;
	if (!mBuffered) {
		eso_file_setbuffer(mFile, NULL, 0);
	}
}

EFileOutputStream::EFileOutputStream(es_file_t* file) :
			needClose(false),
			closed(false),
			append(false),
			channel(null)
{
	ES_ASSERT(file);
	mFile = file;
	mBuffered = false;
	if (!mBuffered) {
		eso_fflush(mFile);
		eso_file_setbuffer(mFile, NULL, 0);
	}
}

EFileOutputStream::EFileOutputStream(es_os_file_t hfile) :
			needClose(true),
			closed(false),
			append(false),
			channel(null)
{
	mFile = eso_fdopen(hfile, "wb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__);
	}
	mBuffered = false;
	if (!mBuffered) {
		eso_file_setbuffer(mFile, NULL, 0);
	}
}

void EFileOutputStream::write(const void *b, int len)
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	ES_ASSERT(b);
	
	if (len <= 0) {
		return;
	}
	
	//@see: openjdk-7/jdk/src/share/native/java/io/io_util.c :: writeBytes()
	int off = 0;
	while (len > 0) {
		if (mFile == null) {
			throw EIOException(__FILE__, __LINE__, "Stream Closed");
			break;
		}
		int n = (int)restartable_write(mFile, (byte*)b+off, len);
		if (n == -1) {
			throw EIOException(__FILE__, __LINE__, "Write error");
			break;
		}
		off += n;
		len -= n;
	}
}

void EFileOutputStream::write(const char *s)
{
	EOutputStream::write(s);
}

void EFileOutputStream::write(int b)
{
	EOutputStream::write(b);
}

//@see: openjdk-8/src/share/classes/java/io/FileOutputStream.java
/*
void EFileOutputStream::flush()
{
	if (!mBuffered) return;

	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}

	if (eso_fflush(mFile) != 0) {
		throw EIOException(__FILE__, __LINE__);
	}
}
*/

void EFileOutputStream::close()
{
	SYNCBLOCK(&closeLock) {
		if (closed) {
			return;
		}
		closed = true;
	}}
	
	flush();

	if (channel != null) {
		channel->close();
	}

	if (needClose && mFile && eso_fclose(mFile) != 0) {
		throw EIOException(__FILE__, __LINE__);
	}
}

es_file_t* EFileOutputStream::getFD() {
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	return (mFile);
}

nio::EFileChannel* EFileOutputStream::getChannel() {
	SYNCHRONIZED(this) {
		if (channel == null) {
			channel = nio::EFileChannel::open(eso_fileno(mFile), false, true, append);
		}
		return channel;
	}}
}

boolean EFileOutputStream::isIOBuffered() {
	return mBuffered;
}

void EFileOutputStream::setIOBuffered(boolean onoff) {
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}

	mBuffered = onoff;
	if (mBuffered) {
		eso_file_setbuffer(mFile, NULL, DEFAULT_STAT_BLKSIZE);
	}
	else {
		flush();
		eso_file_setbuffer(mFile, NULL, 0);
	}
}

} /* namespace efc */
