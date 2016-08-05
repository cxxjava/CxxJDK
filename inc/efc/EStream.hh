#ifndef __EStream_H__
#define __EStream_H__

#include "EObject.hh"
#include "EByteBuffer.hh"
#include "EIOException.hh"
#include "EFileNotFoundException.hh"

namespace efc {

class EStream : public EObject {
public:
	virtual ~EStream();
	EStream();

	//TODO:
	EStream(const EStream& that);
	EStream& operator= (const EStream& that);

public:
	void open(const char* filename, const char* mode)
			THROWS2(EIOException,EFileNotFoundException);
	void open(es_file_t* file, const char* mode) THROWS(EIOException);
	void open(void* memory, es_size_t size, const char* mode)
			THROWS(EIOException);
	void open(EByteBuffer* array, const char* mode) THROWS(EIOException);
	void close();

	void seek(long offset, int whence) THROWS(EIOException);bool eof()
			THROWS(EIOException);
	void flush() THROWS(EIOException);
	es_ssize_t tell() THROWS(EIOException);
	es_ssize_t size() THROWS(EIOException);

	int read(void* buffer, es_size_t size) THROWS(EIOException);
	int readChar(char& c) THROWS(EIOException);
	int readBoolean(boolean& b) THROWS(EIOException);
	int readByte(byte& b) THROWS(EIOException);
	int readShort(short& s) THROWS(EIOException);
	int readInt(int& n) THROWS(EIOException);
	int readLLong(llong& l) THROWS(EIOException);
	int readFloat(float& f) THROWS(EIOException);
	int readDouble(double& d) THROWS(EIOException);

	int write(void* buffer, es_size_t size) THROWS(EIOException);
	int writeChar(char c) THROWS(EIOException);
	int writeBoolean(boolean b) THROWS(EIOException);
	int writeByte(byte b) THROWS(EIOException);
	int writeShort(short s) THROWS(EIOException);
	int writeInt(int n) THROWS(EIOException);
	int writeLLong(llong l) THROWS(EIOException);
	int writeFloat(float f) THROWS(EIOException);
	int writeDouble(double d) THROWS(EIOException);

	static short readShort(void* src);
	static int readInt(void* src);
	static llong readLLong(void* src);

	static int writeShort(void* dst, short s);
	static int writeInt(void* dst, int i);
	static int writeLLong(void* dst, llong l);

private:
	boolean m_opened;
	es_uint32_t m_flag;

	es_file_t* m_file;

	byte* m_maddr;
	es_ssize_t m_msize;

	EByteBuffer* m_array;
	es_ssize_t m_position;
};

} /* namespace efc */
#endif //!__EStream_H__
