#include "ERandom.hh"
#include "ESystem.hh"
#include "EInteger.hh"
#include "EByte.hh"
#include "EMath.hh"
#include "ENullPointerException.hh"
#include "../inc/concurrent/EAtomicLLong.hh"

namespace efc {

static volatile llong seedUniquifier = 8682522807148012L;

ERandom::~ERandom() {
	delete seed;
}

ERandom::ERandom() : nextNextGaussian(0.0), haveNextNextGaussian(false)
{
	this->seed = new EAtomicLLong(0L);
	setSeed(++seedUniquifier + ESystem::nanoTime() + (long)this);
}

ERandom::ERandom(llong seed) : nextNextGaussian(0.0), haveNextNextGaussian(false)
{
	this->seed = new EAtomicLLong(0L);
	setSeed(seed);
}

void ERandom::setSeed(llong seed)
{
	SYNCHRONIZED(this) {
		seed = (seed ^ multiplier) & mask;
		this->seed->set(seed);
		haveNextNextGaussian = false;
    }}
}

int ERandom::next(int bits) {
	ullong oldseed, nextseed;
	EAtomicLLong* seed = this->seed;
	do {
		oldseed = seed->get();
		nextseed = (oldseed * multiplier + addend) & mask;
	} while (!seed->compareAndSet(oldseed, nextseed));
	return (int)(nextseed >> (48 - bits));
}

void ERandom::nextBytes(EA<byte>* bytes) {
	if (!bytes) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	for (int i = 0, len = bytes->length(); i < len; )
		for (int rnd = nextInt(),
				 n = EMath::min(len - i, EInteger::SIZE/EByte::SIZE);
			 n-- > 0; rnd >>= EByte::SIZE)
			(*bytes)[i++] = (byte)rnd;
}

void ERandom::nextBytes(byte* bytes, int size) {
	if (!bytes) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	if (size < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "size must be positive");
	}
	for (int i = 0, len = size; i < len; )
		for (int rnd = nextInt(),
				 n = EMath::min(len - i, EInteger::SIZE/EByte::SIZE);
			 n-- > 0; rnd >>= EByte::SIZE)
			bytes[i++] = (byte)rnd;
}

int ERandom::nextInt()
{
	return next(32);
}

int ERandom::nextInt(int n) {
	if (n <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "n must be positive");

	if ((n & -n) == n)  // i.e., n is a power of 2
		return (int)((n * (llong)next(31)) >> 31);

	int bits, val;
	do {
		bits = next(31);
		val = bits % n;
	} while (bits - val + (n-1) < 0);
	return val;
}

llong ERandom::nextLLong() {
	// it's okay that the bottom word remains signed.
	return ((llong)(next(32)) << 32) + next(32);
}

boolean ERandom::nextBoolean() {
	return next(1) != 0;
}

float ERandom::nextFloat() {
	return next(24) / ((float)(1 << 24));
}

double ERandom::nextDouble() {
	return (((llong)(next(26)) << 27) + next(27))
			/ (double)(1L << 53);
}

double ERandom::nextGaussian() {
	// See Knuth, ACP, Section 3.4.1 Algorithm C.
	if (haveNextNextGaussian) {
		haveNextNextGaussian = false;
		return nextNextGaussian;
	} else {
		double v1, v2, s;
		do {
			v1 = 2 * nextDouble() - 1; // between -1 and 1
			v2 = 2 * nextDouble() - 1; // between -1 and 1
			s = v1 * v1 + v2 * v2;
		} while (s >= 1 || s == 0);
		double multiplier = ::sqrt(-2 * ::log(s)/s);
		nextNextGaussian = v2 * multiplier;
		haveNextNextGaussian = true;
		return v1 * multiplier;
	}
}

} /* namespace efc */
