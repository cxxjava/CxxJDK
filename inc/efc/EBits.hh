/*
 * EBits.hh
 *
 *  Created on: 2013-3-19
 *      Author: Administrator
 */

#ifndef EBITS_HH_
#define EBITS_HH_

#include "EObject.hh"
#include "EByteBuffer.hh"
#include "EIndexOutOfBoundsException.hh"

namespace efc {

/**
 * Utility methods for packing/unpacking primitive values in/out of byte arrays
 * using big-endian byte ordering.
 */
class EBits : public EObject {
public:
	/*
     * Methods for unpacking primitive values from byte arrays starting at
     * given offsets.
     */

    static boolean getBoolean(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException);
    
    static char getChar(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException);
    
    static short getShort(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException);
    
    static int getInt(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException);
    
    static llong getLLong(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException);

    static float getFloat(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException);

    static double getDouble(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException);
    
    /*
     * Methods for packing primitive values into byte arrays starting at given
     * offsets.
     */

    static void putBoolean(EByteBuffer& b, int off, boolean val) THROWS(EIndexOutOfBoundsException);

    static void putChar(EByteBuffer& b, int off, char val) THROWS(EIndexOutOfBoundsException);

    static void putShort(EByteBuffer& b, int off, short val) THROWS(EIndexOutOfBoundsException);

    static void putInt(EByteBuffer& b, int off, int val) THROWS(EIndexOutOfBoundsException);

    static void putLLong(EByteBuffer& b, int off, long val) THROWS(EIndexOutOfBoundsException);

    static void putFloat(EByteBuffer& b, int off, float val) THROWS(EIndexOutOfBoundsException);

    static void putDouble(EByteBuffer& b, int off, double val) THROWS(EIndexOutOfBoundsException);

    // -- Processor and memory-system properties --

    static long pageSize();
	
private:
    static long pageSize_;// = -1;

    static void checkRange(EByteBuffer& b, int off, int size) THROWS(EIndexOutOfBoundsException);
};

} /* namespace efc */
#endif /* EBITS_HH_ */
