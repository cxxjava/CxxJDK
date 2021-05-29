/*
 * EBitSet.cpp
 *
 *  Created on: 2013-3-16
 *      Author: cxxjava@163.com
 */

#include "EBitSet.hh"
#include "ESystem.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

#define BITSET_BITS CHAR_BIT

#define BITSET_MASK(pos) \
	( ((es_byte_t)1) << ((pos) - ES_ALIGN_DOWN(pos, BITSET_BITS)) )

#define BITSET_WORD(_bits, pos) \
	( _bits[(pos) / BITSET_BITS] )

void EBitSet::checkRange(int fromIndex, int toIndex)
{
	if (fromIndex < 0)
		throw EIndexOutOfBoundsException(__FILE__, __LINE__,
				EString::formatOf("fromIndex < 0: %d", fromIndex).c_str());
	if (toIndex < 0)
		throw EIndexOutOfBoundsException(__FILE__, __LINE__,
				EString::formatOf("toIndex < 0: %d", toIndex).c_str());
	if (fromIndex > toIndex)
		throw EIndexOutOfBoundsException(__FILE__, __LINE__,
				EString::formatOf("fromIndex: %d > toIndex: %d", fromIndex,
						toIndex).c_str());
}

void EBitSet::expandTo(int bitIndex)
{
	if (bitIndex < _nbits) {
		return;
	}

	_nbits = bitIndex + 1;
	int bytesRequired = ES_ALIGN_UP(_nbits, BITSET_BITS) / BITSET_BITS;
	if (_nbytes < bytesRequired) {
		_nbytes = ES_MAX(_nbytes*2, bytesRequired);
		_bits = (es_byte_t*)eso_mrealloc(_bits, _nbytes);
	}
}

EBitSet::~EBitSet() {
	eso_mfree(_bits);
}

EBitSet::EBitSet(const EBitSet& that)
{
	_bits = (es_byte_t*)eso_mmemdup(that._bits, that._nbytes);
	_nbytes = that._nbytes;
	_nbits = that._nbits;
}

EBitSet::EBitSet(int nbits) {
	_nbytes = ES_ALIGN_UP(nbits, BITSET_BITS) / BITSET_BITS;
	_bits = (es_byte_t*)eso_mcalloc(_nbytes);
	_nbits = nbits;
}

EBitSet::EBitSet(const char *bitset, int len) {
	ES_ASSERT(bitset);

	_nbits = (len < 0) ? eso_strlen(bitset) : len;
	_nbytes = ES_ALIGN_UP(_nbits, BITSET_BITS) / BITSET_BITS;
	_bits = (es_byte_t*)eso_mcalloc(_nbytes);

	int bits = ES_MIN(_nbits, eso_strlen(bitset));
	for (int i = bits - 1; i >= 0; i--) {
		set(i, (bitset[i] != '0'));
	}
}

EBitSet& EBitSet::operator= (const EBitSet& that) {
	if (this == &that) return *this;

	//1.
	eso_mfree(_bits);

	//2.
	_bits = (es_byte_t*)eso_mmemdup(that._bits, that._nbytes);
	_nbytes = that._nbytes;
	_nbits = that._nbits;

	return *this;
}

void EBitSet::flip(int bitIndex)
{
	ES_ASSERT(bitIndex >= 0);

	if (get(bitIndex)) {
		clear(bitIndex);
	}
	else {
		set(bitIndex);
	}
}

void EBitSet::flip(int fromIndex, int toIndex)
		THROWS(EIndexOutOfBoundsException)
{
	checkRange(fromIndex, toIndex);

	if (fromIndex == toIndex)
		return;

	for (int i = fromIndex; i < toIndex; i++) {
		flip(i);
	}
}

void EBitSet::set(int bitIndex, boolean value)
{
	ES_ASSERT(bitIndex >= 0);

	expandTo(bitIndex);
	if (value) {
		BITSET_WORD(_bits, bitIndex) |= BITSET_MASK(bitIndex);
	}
	else {
		BITSET_WORD(_bits, bitIndex) &= ~BITSET_MASK(bitIndex);
	}
}

void EBitSet::set(int fromIndex, int toIndex, boolean value)
		THROWS(EIndexOutOfBoundsException)
{
	checkRange(fromIndex, toIndex);

	if (fromIndex == toIndex)
		return;

	for (int i = fromIndex; i < toIndex; i++) {
		set(i, value);
	}
}

void EBitSet::clear(int bitIndex)
{
	ES_ASSERT(bitIndex >= 0);

	if (bitIndex >= _nbits) {
		return;
	}
	BITSET_WORD(_bits, bitIndex) &= ~BITSET_MASK(bitIndex);
}

void EBitSet::clear(int fromIndex, int toIndex)
		THROWS(EIndexOutOfBoundsException)
{
	checkRange(fromIndex, toIndex);

	if (fromIndex == toIndex)
		return;

	for (int i = fromIndex; i < toIndex; i++) {
		clear(i);
	}
}

void EBitSet::clear() {
	eso_mmemfill(_bits, 0);
}

boolean EBitSet::get(int bitIndex)
{
	ES_ASSERT(bitIndex >= 0);

	return (bitIndex < _nbits)
			&& (BITSET_WORD(_bits, bitIndex) & BITSET_MASK(bitIndex));
}

EBitSet EBitSet::get(int fromIndex, int toIndex)
		THROWS(EIndexOutOfBoundsException)
{
	checkRange(fromIndex, toIndex);

	if (fromIndex == toIndex || fromIndex >= _nbits)
		return EBitSet((int)0);

	EBitSet newBS(toIndex - fromIndex);
	for (int i = fromIndex; i < toIndex; i++) {
		BITSET_WORD(newBS._bits, i - fromIndex) |= (BITSET_WORD(_bits, i) & BITSET_MASK(i));
	}
	return newBS;
}

int EBitSet::nextSetBit(int fromIndex)
{
	ES_ASSERT(fromIndex >= 0);

	if (fromIndex >= _nbits) {
		return -1;
	}

	for (int i=fromIndex; i<_nbits; i++) {
		if (get(i)) {
			return i;
		}
	}

	return -1;
}

int EBitSet::nextClearBit(int fromIndex)
{
	ES_ASSERT(fromIndex >= 0);

	if (fromIndex >= _nbits) {
		return -1;
	}

	for (int i = fromIndex; i < _nbits; i++) {
		if (!get(i)) {
			return i;
		}
	}

	return -1;
}

int EBitSet::length() {
	return _nbits;
}

boolean EBitSet::isEmpty() {
	for (int i=0; i<_nbytes; i++) {
		if (*(_bits+i)) {
			return false;
		}
	}
	return true;
}

boolean EBitSet::intersects(EBitSet* set) {
	for (int i = _nbytes - 1; i >= 0; i--)
		if ((_bits[i] & set->_bits[i]) != 0)
			return true;
	return false;
}

int EBitSet::cardinality() {
	throw EUNSUPPORTEDOPERATIONEXCEPTION;
}

void EBitSet::and_(EBitSet* set)
{
	if (this == set)
		return;

	int n = _nbytes;
	while (_nbytes > set->_nbytes)
		_bits[--n] = 0;

	// Perform logical AND on words in common
	for (int i = 0; i < n; i++)
		_bits[i] &= set->_bits[i];
}

void EBitSet::or_(EBitSet* set)
{
	if (this == set)
		return;

	int bytesInCommon = ES_MIN(_nbytes, set->_nbytes);
	if (_nbits < set->_nbits) {
		expandTo(set->_nbits);
	}
	_bits = (es_byte_t*)eso_mrealloc(_bits, set->_nbytes);

	// Perform logical OR on words in common
	for (int i = 0; i < bytesInCommon; i++)
		_bits[i] |= set->_bits[i];

	// Copy any remaining words
	if (bytesInCommon < set->_nbytes)
		ESystem::arraycopy(set->_bits, bytesInCommon, _bits, bytesInCommon,
				_nbits - bytesInCommon);
}

void EBitSet::xor_(EBitSet* set)
{
	int bytesInCommon = ES_MIN(_nbytes, set->_nbytes);

	if (_nbits < set->_nbits) {
		expandTo(set->_nbits);
	}
	_bits = (es_byte_t*)eso_mrealloc(_bits, set->_nbytes);

	// Perform logical XOR on words in common
	for (int i = 0; i < bytesInCommon; i++)
		_bits[i] ^= set->_bits[i];

	// Copy any remaining words
	if (bytesInCommon < set->_nbytes)
		ESystem::arraycopy(set->_bits, bytesInCommon, _bits, bytesInCommon,
				_nbits - bytesInCommon);
}

void EBitSet::andNot(EBitSet* set) {
	// Perform logical (a & !b) on words in common
	for (int i = ES_MIN(_nbytes, set->_nbytes) - 1; i >= 0; i--)
		_bits[i] &= ~set->_bits[i];
}

int EBitSet::hashCode() {
	llong h = 1234;
	for (int i = _nbytes; --i >= 0;)
		h ^= _bits[i] * (i + 1);

	return (int) ((h >> 32) ^ h);
}

int EBitSet::size() {
	return _nbytes;
}

boolean EBitSet::equals(EBitSet *set) {
	if (this == set)
		return true;

	if (_nbits != set->_nbits)
		return false;

	// Check words in use by both BitSets
	for (int i = 0; i < _nbits; i++)
		if (_bits[i] != set->_bits[i])
			return false;

	return true;
}

boolean EBitSet::equals(EObject* obj) {
	if (this == obj)
		return true;

	EBitSet* set = dynamic_cast<EBitSet*>(obj);

	if (!set || _nbits != set->_nbits)
		return false;

	// Check words in use by both BitSets
	for (int i = 0; i < _nbits; i++)
		if (_bits[i] != set->_bits[i])
			return false;

	return true;
}


EBitSet* EBitSet::clone() {
	EBitSet *bs = new EBitSet(_nbits);
	eso_memcpy(bs->_bits, _bits, bs->size());
	return bs;
}

EString EBitSet::toString() {
	EString str;

	int pos = 0;
	for (int i = 0; i < _nbytes; i++) {
		for (int j = 0; j < BITSET_BITS; j++) {
			if (pos >= _nbits)
				break;
			if (_bits[i] & BITSET_MASK(pos))
				str.concat("1");
			else
				str.concat("0");
			pos++;
		}
	}

	return str;
}

} /* namespace efc */
