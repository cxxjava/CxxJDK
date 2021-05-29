#include "EStream.hh"
#include "EIOException.hh"

namespace efc {

#define FS_READ_ONLY		0x0000
#define FS_WRITE_ONLY       0x0001
#define FS_READ_WRITE       0x0002
#define FS_FILE_HANDLE      0x0004
#define FS_MEMORY_HANDLE    0x0008
#define FS_BUFFER_HANDLE    0x000C
#define FS_FILE_OPEN        0x0010
#define FS_BINARY_MODE      0x0080
#define FS_EOF              0x8000

#define FS_MODE_MASK        0x0003
#define FS_HANDLE_MASK      0x000C

static es_uint32_t get_open_mode(const char* mode)
{
	es_uint32_t flag = FS_READ_ONLY;
	
	if (mode == NULL || *mode == 0)
        flag = FS_READ_ONLY;
    else if (mode[0] == 'r' || mode[0] == 'R') {
        flag = FS_READ_ONLY;
        if (mode[1] == '+' || ( mode[1] != 0 && mode[2] == '+'))
            flag = FS_READ_WRITE;
    }
    else if (mode[0] == 'w' || mode[0] == 'W') {
        /* "w" and "w+" have the same effect */
        flag = FS_READ_WRITE;
    }else if (*mode == 'a') {
        flag = FS_READ_WRITE;
	}
	
	return flag;
}

EStream::EStream()
{
	m_opened = FALSE;
	m_flag = 0;
	m_file = NULL;
	m_maddr = NULL;
	m_msize = 0;
	m_array = NULL;
}

EStream::~EStream()
{
    try {
        close();
    } catch (...) {
    }
}

void EStream::open(const char* filename, const char* mode) THROWS2(EIOException,EFileNotFoundException)
{
	if (m_opened || !filename || !*filename) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	m_file = eso_fopen(filename, mode);
	if (!m_file) {
		if (errno == ENOENT) {
			throw EFileNotFoundException(__FILE__, __LINE__);
		}
		else {
			throw EIOException(__FILE__, __LINE__);
		}
	}
	m_flag = FS_FILE_HANDLE | FS_BINARY_MODE | FS_FILE_OPEN;
	m_flag |= get_open_mode(mode);
	if (mode && (*mode=='a' || *mode=='A')) {
		int v = eso_fseek(m_file, 0, ES_SEEK_END);
		if (v != 0) {
			v = errno;
			eso_fclose(m_file);
			throw EIOException(__FILE__, __LINE__, v);
		}
	}
	
	m_opened = TRUE;
}

void EStream::open(es_file_t* file, const char* mode) THROWS(EIOException)
{
	if (m_opened || !file) {
		throw EIOException(__FILE__, __LINE__);
	}

	m_file = file;

	m_flag = FS_FILE_HANDLE | FS_BINARY_MODE;
	m_flag |= get_open_mode(mode);

	if (mode && (*mode=='a' || *mode=='A') && eso_fseek(m_file, 0, ES_SEEK_END)!=0) {
		throw EIOException(__FILE__, __LINE__);
	}
	m_opened = TRUE;
}

void EStream::open(void* memory, es_size_t size, const char* mode) THROWS(EIOException)
{
	if (m_opened || !memory || size == 0) {
		throw EIOException(__FILE__, __LINE__);
	}

	m_maddr = (byte*)memory;
	m_msize = size;

	m_flag = FS_MEMORY_HANDLE | FS_BINARY_MODE;
	m_flag |= get_open_mode(mode);

	if (mode && (*mode=='a' || *mode=='A')) {
		char msg[100];
		sprintf(msg, "don't support this mode(%s) for memory stream!", mode);
		throw EIOException(__FILE__, __LINE__, msg);
	}
	m_position = 0;
	m_opened = TRUE;
}

void EStream::open(EByteBuffer* array, const char* mode) THROWS(EIOException)
{
	if (m_opened || !array) {
		throw EIOException(__FILE__, __LINE__);
	}

	m_array = array;

	m_flag = FS_BUFFER_HANDLE | FS_BINARY_MODE;
	m_flag |= get_open_mode(mode);

	m_position = 0;
	if (mode && (*mode=='a' || *mode=='A')) {
		m_position = m_array->size();
	}
	m_opened = TRUE;
}

void EStream::close()
{
	if (m_opened) {
		if (m_flag & FS_FILE_OPEN) {
			if (m_file) {
				eso_fclose(m_file);
			}
		}
		m_file = NULL;
		m_maddr = NULL;
		m_msize = 0;
		m_array = NULL;
		m_opened = FALSE;
	}
}

void EStream::seek(long offset, int whence) THROWS(EIOException)
{
	int ret = -1;
	
	if (!m_opened) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	m_flag &= ~FS_EOF;
	
	if ((m_flag & FS_HANDLE_MASK) == FS_FILE_HANDLE) {
		ret = eso_fseek(m_file, offset, whence);
	}
	else {
		int newpos = -1;
		
		switch(whence) {
			case ES_SEEK_SET:
				newpos = offset; 
				break;
			case ES_SEEK_CUR:
				newpos = m_position + offset; 
				break;
			case ES_SEEK_END:
				newpos = this->size() + offset;
		}
		
		if (newpos < 0 || newpos > this->size()) {
			throw EIOException(__FILE__, __LINE__, "OutOfBounds");
		}
		m_position = newpos;
		
		if (m_position == (es_uint32_t)this->size()) {
			m_flag &= ~FS_EOF;
		}
		
		ret = 0;
	}

	if (ret) {
		throw EIOException(__FILE__, __LINE__);
	}
}

bool EStream::eof() THROWS(EIOException)
{
	bool r = false;

	if (!m_opened) {
		throw EIOException(__FILE__, __LINE__);
	}

	if ((m_flag & FS_HANDLE_MASK) == FS_FILE_HANDLE) {
		r = (bool)eso_feof(m_file);
		if (errno != 0) {
			throw EIOException(__FILE__, __LINE__);
		}
	}
	else {
		r = ((m_flag & FS_EOF) == FS_EOF);
	}

	return r;
}

void EStream::flush() THROWS(EIOException)
{
	if (!m_opened) {
		throw EIOException(__FILE__, __LINE__);
	}

	if ((m_flag & FS_HANDLE_MASK) == FS_FILE_HANDLE) {
		if (eso_fflush(m_file) != 0) {
			throw EIOException(__FILE__, __LINE__);
		}
	}
}

es_ssize_t EStream::tell() THROWS(EIOException)
{
	if (!m_opened) {
		throw EIOException(__FILE__, __LINE__);
	}

	if ((m_flag & FS_HANDLE_MASK) == FS_FILE_HANDLE) {
		es_ssize_t position = eso_ftell(m_file);
		if (position == -1) {
			throw EIOException(__FILE__, __LINE__);
		}
		return position;
	}
	else {
		return m_position;
	}
}

es_ssize_t EStream::size() THROWS(EIOException)
{
	if (!m_opened) {
		throw EIOException(__FILE__, __LINE__);
	}

	if ((m_flag & FS_HANDLE_MASK) == FS_FILE_HANDLE) {
		es_ssize_t size = eso_fsize(m_file);
		if (size == -1) {
			throw EIOException(__FILE__, __LINE__);
		}
		return size;
	}
	else if ((m_flag & FS_HANDLE_MASK) == FS_MEMORY_HANDLE) {
		return m_msize;
	}
	else if ((m_flag & FS_HANDLE_MASK) == FS_BUFFER_HANDLE) {
		return m_array->size();
	}

	throw EIOException(__FILE__, __LINE__);
}

int EStream::read(void* buffer, es_size_t size) THROWS(EIOException)
{
	es_size_t bytesread = 0;
	
	if (!m_opened || !buffer) {
		throw EIOException(__FILE__, __LINE__);
	}
	if ((m_flag & FS_MODE_MASK) == FS_WRITE_ONLY) {
		throw EIOException(__FILE__, __LINE__, "WriteOnly");
	}
	eso_memset(buffer, 0, size);
	
	if (size == 0) return 0;
	
	if ((m_flag & FS_HANDLE_MASK) == FS_FILE_HANDLE) {
		bytesread = eso_fread(buffer, size, m_file);
		if (bytesread != size && eso_ferror(m_file)) {
			throw EIOException(__FILE__, __LINE__);
		}
	}
	else if ((m_flag & FS_HANDLE_MASK) == FS_MEMORY_HANDLE) {
		bytesread = ES_MIN(size, (es_size_t)(m_msize-m_position));
		eso_memcpy(buffer, m_maddr+m_position, bytesread);
		m_position += bytesread;
		if (m_position == m_msize) m_flag |= FS_EOF;
	}
	else if ((m_flag & FS_HANDLE_MASK) == FS_BUFFER_HANDLE) {
		bytesread = ES_MIN(size, m_array->size()-m_position);
		eso_memcpy(buffer, (char*)m_array->data()+m_position, bytesread);
		m_position += bytesread;
		if (m_position == (es_ssize_t)m_array->size()) m_flag |= FS_EOF;
	}
	
	return bytesread;
}

int EStream::readChar(char& c) THROWS(EIOException)
{
	return read(&c, 1);
}

int EStream::readBoolean(boolean& b) THROWS(EIOException)
{
	return read(&b, 1);
}

int EStream::readByte(byte& b) THROWS(EIOException)
{
	return read(&b, 1);
}

int EStream::readShort(short& s) THROWS(EIOException)
{
	es_byte_t v[2];
	int ret;

	ret = read(v, 2);
	if (ret == 2) {
		s = readShort(v);
	}
	return ret;
}

int EStream::readInt(int& n) THROWS(EIOException)
{
	es_byte_t v[4];
	int ret;
	
	ret = read(v, 4);
	if (ret == 4) {
		n = readInt(v);
	}
	return ret;
}

int EStream::readLLong(llong& l) THROWS(EIOException)
{
	es_byte_t v[8];
	int ret;

	ret = read(v, 8);
	if (ret == 8) {
		l = readLLong(v);
	}
	return ret;
}

int EStream::readFloat(float& f) THROWS(EIOException)
{
	es_byte_t v[4];
	int ret;

	ret = read(v, 4);
	if (ret == 4) {
		int i = readInt(v);
		f = eso_intBits2float(i);
	}
	return ret;
}

int EStream::readDouble(double& d) THROWS(EIOException)
{
	es_byte_t v[8];
	int ret;

	ret = read(v, 8);
	if (ret == 8) {
		llong l = readLLong(v);
		d = eso_llongBits2double(l);
	}
	return ret;
}

int EStream::write(void* buffer, es_size_t size) THROWS(EIOException)
{
	es_size_t byteswrite = 0;
	
	if (!m_opened || !buffer || ((m_flag & FS_MODE_MASK) == FS_READ_ONLY)) {
		throw EIOException(__FILE__, __LINE__);
	}

	if (size == 0) return 0;
	
	if ((m_flag & FS_HANDLE_MASK) == FS_FILE_HANDLE) {
		byteswrite = eso_fwrite(buffer, size, m_file);
	}
	else if ((m_flag & FS_HANDLE_MASK) == FS_MEMORY_HANDLE) {
		byteswrite = ES_MIN(size, (es_size_t)(m_msize-m_position));
		eso_memcpy(m_maddr+m_position, buffer, byteswrite);
		m_position += byteswrite;
		if (m_position == m_msize) m_flag |= FS_EOF;
	}
	else if ((m_flag & FS_HANDLE_MASK) == FS_BUFFER_HANDLE) {
		byteswrite = m_array->update(m_position, (const void *)buffer, size);
		m_position += byteswrite;
		if (m_position == (es_ssize_t)m_array->size()) m_flag |= FS_EOF;
	}
	
	return byteswrite;	
}

int EStream::writeChar(char c) THROWS(EIOException)
{
	return write(&c, 1);
}

int EStream::writeBoolean(boolean b) THROWS(EIOException)
{
	return write(&b, 1);
}

int EStream::writeByte(byte b) THROWS(EIOException)
{
	return write(&b, 1);
}

int EStream::writeShort(short s) THROWS(EIOException)
{
	if (!ES_BIG_ENDIAN) {
		s = ES_BSWAP_16(s);
	}
	return write(&s, 2);
}

int EStream::writeInt(int n) THROWS(EIOException)
{
	if (!ES_BIG_ENDIAN) {
		n = ES_BSWAP_32(n);
	}
	return write(&n, 4);
}

int EStream::writeLLong(llong l) THROWS(EIOException)
{
	if (!ES_BIG_ENDIAN) {
		l = ES_BSWAP_64(l);
	}
	return write(&l, 8);
}

int EStream::writeFloat(float f) THROWS(EIOException)
{
	es_int32_t i = eso_float2intBits(f);
	return write(&i, 4);
}

int EStream::writeDouble(double d) THROWS(EIOException)
{
	es_int64_t i = eso_double2llongBits(d);
	return write(&i, 8);
}

short EStream::readShort(void* src) {
	short s = 0;
	eso_memcpy(&s, src, sizeof(short));
	if (!ES_BIG_ENDIAN) {
		s = ES_BSWAP_16(s);
	}
	return s;
}

int EStream::readInt(void* src) {
	int i = 0;
	eso_memcpy(&i, src, sizeof(int));
	if (!ES_BIG_ENDIAN) {
		i = ES_BSWAP_32(i);
	}
	return i;
}

llong EStream::readLLong(void* src) {
	llong l = 0;
	eso_memcpy(&l, src, sizeof(llong));
	if (!ES_BIG_ENDIAN) {
		l = ES_BSWAP_64(l);
	}
	return l;
}

int EStream::writeShort(void* dst, short s) {
	if (!ES_BIG_ENDIAN) {
		s = ES_BSWAP_16(s);
	}
	int n = sizeof(short);
	eso_memcpy(dst, &s, n);
	return n;
}

int EStream::writeInt(void* dst, int i) {
	if (!ES_BIG_ENDIAN) {
		i = ES_BSWAP_32(i);
	}
	int n = sizeof(int);
	eso_memcpy(dst, &i, n);
	return n;
}

int EStream::writeLLong(void* dst, llong l) {
	if (!ES_BIG_ENDIAN) {
		l = ES_BSWAP_64(l);
	}
	int n = sizeof(llong);
	eso_memcpy(dst, &l, n);
	return n;
}

} /* namespace efc */
