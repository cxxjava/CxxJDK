#include "EByteBuffer.hh"

namespace efc {

EByteBuffer::~EByteBuffer()
{
	if (m_data) {
		eso_free(m_data);
	}
}

EByteBuffer::EByteBuffer(uint capacity, uint grow)
{
	this->initialCapacity = capacity;
	this->growSize = grow;
	m_data = (byte*)eso_malloc(initialCapacity);
	m_length = 0;
	m_data[0] = '\0';
	m_capacity = initialCapacity;
	m_limit = 0;
}

EByteBuffer::EByteBuffer(const EByteBuffer& that) {
	EByteBuffer* t = (EByteBuffer*)&that;

	initialCapacity = t->initialCapacity;
	growSize = t->growSize;
	m_length = t->m_length;
	m_limit = t->m_limit;

	if (t->m_length > 0) {
		m_capacity = t->m_capacity;
		m_data = (byte*)eso_malloc(m_length);
		eso_memcpy(m_data, t->m_data, m_length);
	}
	else {
		m_capacity = initialCapacity;
		m_data = (byte*)eso_malloc(initialCapacity);
		m_data[0] = '\0';
	}
}

EByteBuffer& EByteBuffer::operator= (const EByteBuffer& that) {
	if (this == &that) return *this;

	EByteBuffer* t = (EByteBuffer*)&that;

	//1.
	eso_free(m_data);

	//2.
	initialCapacity = t->initialCapacity;
	growSize = t->growSize;
	m_length = t->m_length;
	m_capacity = t->m_capacity;
	m_limit = t->m_limit;

	if (t->m_length > 0) {
		m_data = (byte*)eso_malloc(m_length);
		eso_memcpy(m_data, t->m_data, m_length);
	}
	else {
		m_data = (byte*)eso_malloc(initialCapacity);
		m_data[0] = '\0';
	}

	return *this;
}

void* EByteBuffer::data()
{
	return m_data;
}

int EByteBuffer::size()
{
	return m_length;
}

void EByteBuffer::clear()
{
	m_length = 0;
	m_data[0] = '\0';
}

boolean EByteBuffer::isEmpty()
{
	return (m_length == 0) ? TRUE : FALSE;
}

uint EByteBuffer::capacity()
{
	return m_capacity;
}

void EByteBuffer::resize(int newCapacity)
{
	int shift_size;

	if (newCapacity <= m_length + growSize)
		return;

	shift_size = newCapacity - m_capacity;
	if (shift_size > 0) {
		m_capacity += shift_size;
		m_data = (byte*)eso_realloc(m_data, m_capacity);
	}
	else if (shift_size < 0) {
		void *old_data = m_data;
		m_capacity = newCapacity;
		m_data = (byte*)eso_malloc(newCapacity);
		eso_memcpy(m_data, old_data, m_length);
		eso_free(old_data);
	}
}

boolean EByteBuffer::hasRemaining()
{
	if (m_limit == 0) {
		return TRUE;
	}
	else {
		return (m_capacity > m_length) ? TRUE : FALSE;
	}
}

uint EByteBuffer::remaining()
{
	uint remaining = -1;
	if (m_limit > 0) {
		if (m_length >= m_limit)
			remaining = 0;
		else
			remaining = m_limit - m_length;
	}
	return remaining;
}

uint EByteBuffer::limit()
{
	return m_limit==0 ? -1 : m_limit;
}

void EByteBuffer::setLimit(uint limit)
{
	m_limit = limit;
	m_length = ES_MIN(m_length, m_capacity);
}

void EByteBuffer::setGrowSize(uint size) {
	growSize = size;
}

int EByteBuffer::append(byte b) {
	return append(&b, 1);
}

int EByteBuffer::append(const void *data, int size)
{
	int length = 0;
	
	if (m_limit == 0) {
		length = size;
	}
	else {
		if (m_length < m_limit) {
			length = ES_MIN(size, m_limit-m_length);
		}
	}

	if (length > 0) {
		ensureCapacity(m_length + length);
		eso_memcpy(m_data + m_length, data, length);
		m_length += length;
	}

	return length;
}

int EByteBuffer::append(const char *cstr)
{
	return append((const void *)cstr, eso_strlen(cstr));
}

int EByteBuffer::append(const EString& estr)
{
	return append((const char *)estr.c_str());
}

int EByteBuffer::appendFormat(const char *fmt, ...)
{
	va_list args;
	es_string_t *cstring = NULL;
	int ret;
	
	va_start(args, fmt);
	eso_mvsprintf(&cstring, fmt, args);
	va_end(args);
	ret = append((const char *)cstring);
	ESO_MFREE(&cstring);
	
	return ret;
}

int EByteBuffer::appendFormatV(const char *fmt, va_list args)
{
	va_list args2;
	es_string_t *cstring = NULL;
	int ret;
	
#ifdef va_copy
	va_copy(args2, args);
#else
	args2 = args;
#endif
	
	eso_mvsprintf(&cstring, fmt, args2);
	ret = append((const char *)cstring);
	ESO_MFREE(&cstring);
	
#ifdef va_copy
	va_end(args2);
#endif
	
	return ret;
}

int EByteBuffer::insert(uint pos, byte b) {
	return insert(pos, (const void *)&b, 1);
}

int EByteBuffer::insert(uint pos, const void *data, int size)
{
	if (pos >= m_length) {
		return 0;
	}

	int length = 0;
	int r_length = 0;
	if (m_limit == 0) {
		length = size;
		r_length = m_length - pos;
	}
	else {
		length = ES_MIN(size, m_limit-pos);
		r_length = ES_MIN(m_limit-pos-length, m_length-pos-length);
	}

	if (length > 0) {
		ensureCapacity(pos + length + r_length);
		eso_memmove(m_data+pos+length, m_data+pos, r_length);
		eso_memcpy(m_data+pos, data, length);
		m_length = pos + length + r_length;
	}
	
	return length;
}

int EByteBuffer::insert(uint pos, const char *cstr)
{
	return insert(pos, (const void *)cstr, eso_strlen(cstr));
}

int EByteBuffer::insert(uint pos, const EString& estr)
{
	return insert(pos, (const char *)estr.c_str());
}

int EByteBuffer::insertFormat(uint pos, const char *fmt, ...)
{
	va_list args;
	es_string_t *cstring = NULL;
	int ret;
	
	va_start(args, fmt);
	eso_mvsprintf(&cstring, fmt, args);
	va_end(args);
	ret = insert(pos, (const char *)cstring);
	ESO_MFREE(&cstring);
	
	return ret;
}

int EByteBuffer::insertFormatV(uint pos, const char *fmt, va_list args)
{
	va_list args2;
	es_string_t *cstring = NULL;
	int ret;
	
#ifdef va_copy
	va_copy(args2, args);
#else
	args2 = args;
#endif
	
	eso_mvsprintf(&cstring, fmt, args2);
	ret = insert(pos, (const char *)cstring);
	ESO_MFREE(&cstring);
	
#ifdef va_copy
	va_end(args2);
#endif
	
	return ret;
}

int EByteBuffer::erase(uint pos, int size)
{
	if (pos >= m_length) {
		return 0;
	}

	int old_len = m_length;

	if (size >= m_length-pos) {
		m_length = pos;
		return (old_len - m_length);
	}
	else {
		eso_memmove(m_data+pos, m_data+pos+size, m_length-pos-size);
		m_length -= size;
		return size;
	}
}

int EByteBuffer::update(uint pos, const void *data, int size)
{
	if (pos >= m_length || !data || size <= 0) {
		return 0;
	}

	int old_size = ES_MIN(size, m_length-pos);
	int new_size = 0;
	if (m_limit == 0) {
		new_size = size;
	}
	else {
		new_size = ES_MIN(size, m_limit-pos);
	}

	int more = new_size - old_size;
	if (more > 0) {
		ensureCapacity(m_length + more);
	}
	eso_memmove(m_data + pos + new_size,
			m_data + pos + old_size,
			m_length - pos - old_size);
	if (new_size > 0) {
		eso_memcpy(m_data + pos, data, new_size);
	}
	m_length += more;

	return old_size;
}

byte EByteBuffer::operator[](int index) THROWS(EIndexOutOfBoundsException)
{
	if (index < 0 || index >= size()) {
		throw EIndexOutOfBoundsException(__FILE__, __LINE__);
	}
	return (m_data)[index];
}

int EByteBuffer::zero(int offset, int size)
{
	if (offset > m_length) {
		return 0;
	}
	int more = (offset + size - m_length);
	if (more > 0) {
		ensureCapacity(m_length + more);
		m_length += more;
	}
	eso_memset(m_data+offset, 0, size);

	return size;
}

sp<EA<byte> > EByteBuffer::reset()
{
	EA<byte>* out = new EA<byte>(m_data, m_length, true, MEM_MALLOC);
	m_data = (byte*)eso_malloc(initialCapacity);
	m_length = 0;
	m_data[0] = '\0';
	m_capacity = initialCapacity;
	return out;
}

void EByteBuffer::ensureCapacity(int minCapacity)
{
	if (minCapacity > m_capacity) {
		m_capacity = minCapacity + growSize;
		m_data = (byte*)eso_realloc(m_data, m_capacity);
	}
}

} /* namespace efc */
