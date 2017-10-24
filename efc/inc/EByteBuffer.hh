#ifndef __EByteBuffer_HH__
#define __EByteBuffer_HH__

#include "EA.hh"
#include "EString.hh"
#include "ESharedPtr.hh"
#include "EIndexOutOfBoundsException.hh"

namespace efc {

// Dynamic length buffer.

class EByteBuffer : public EObject
{
public:
	virtual ~EByteBuffer();
	EByteBuffer(uint initialCapacity=10, uint growSize=32);
	
	EByteBuffer(const EByteBuffer& that);
	EByteBuffer& operator= (const EByteBuffer& that);

public:
	void*             data();                      //Get address of this data.
	int               size();                      //Get data length.
	void              clear();                     //Clear this data.
	boolean           isEmpty();                   //Check data is or not empty.
	uint              capacity();                  //Get capacity of this buffer.
	void              resize(int newCapacity);     //Resize this buffer capacity.
	boolean           hasRemaining();              //Check has or not free memory from data end to limited.
	uint              remaining();                 //Get length from data end to limited.
	uint              limit();                     //Get the limited size.
	void              setLimit(uint limit);        //Set limited size, if limit==0 then unlimited.
	void              setGrowSize(uint size);      //Set grow size, when reached capacity size then buffer need grow.
	int               zero(int offset, int size);  //Fill zero from offset to  (offset+size).
	
	/**
	 * Append data to buffer.
	 * @return appended data length.
	 */
	int              append(const void *data, int size);
	int              append(const char *cstr);
	int              append(const EString& estr);
	int              appendFormat(const char *fmt, ...);
	int              appendFormatV(const char *fmt, va_list args);
	
	/**
	 * Insert data to buffer.
	 * @return inserted data length.
	 */
	int              insert(uint pos, const void *data, int size);
	int              insert(uint pos, const char *cstr);
	int              insert(uint pos, const EString& estr);
	int              insertFormat(uint pos, const char *fmt, ...);
	int              insertFormatV(uint pos, const char *fmt, va_list args);
	
	/**
	 * Delete data.
	 * @return deleted data length.
	 */
	int              erase(uint pos, int size);
	
	/**
	 * Update data.
	 * @return updated data length.
	 */
	int              update(uint pos, const void *data, int size);
	
	/**
	 * Get byte at index.
	 * @return the byte at index.
	 */
	byte             operator[](int index) THROWS(EIndexOutOfBoundsException);
	
	/**
	 * Reset and get the old data;
	 */
	sp<EA<byte> >    reset();

private:
	byte*            m_data;
	int              m_length;
	int              m_capacity;
	uint             m_limit;
	uint             initialCapacity;
	uint             growSize;

	void ensureCapacity(int minCapacity);
};

} /* namespace efc */
#endif //!__EByteBuffer_HH__
