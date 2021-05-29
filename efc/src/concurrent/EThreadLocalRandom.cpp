/*
 * EThreadLocalRandom.cpp
 *
 *  Created on: 2015-7-10
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EThreadLocalRandom.hh"
#include "../../inc/EThread.hh"
#include "../../inc/EInteger.hh"
#include "../../inc/EIllegalArgumentException.hh"
#include "../../inc/EUnsupportedOperationException.hh"

namespace efc {

EThreadLocalVariable<tlr::ThreadLocal, EThreadLocalRandom>* EThreadLocalRandom::localRandom = null;

namespace tlr {
	class ThreadLocal : public EThreadLocal {
	public:
		virtual EObject* initialValue() {
            return new EThreadLocalRandom();
        }
	};
}

DEFINE_STATIC_INITZZ_BEGIN(EThreadLocalRandom)
EThread::_initzz_();
localRandom = new EThreadLocalVariable<tlr::ThreadLocal, EThreadLocalRandom>();
DEFINE_STATIC_INITZZ_END

EThreadLocalRandom::EThreadLocalRandom() : ERandom() {
	initialized = true;
}

void EThreadLocalRandom::setSeed(llong seed) {
	if (initialized)
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	rnd = (seed ^ multiplier) & mask;
}

int EThreadLocalRandom::nextInt(int least, int bound) {
	if (least >= bound)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	return nextInt(bound - least) + least;
}

llong EThreadLocalRandom::nextLong(llong n) {
	if (n <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "n must be positive");
	// Divide n by two until small enough for nextInt. On each
	// iteration (at most 31 of them but usually much less),
	// randomly choose both whether to include high bit in result
	// (offset) and whether to continue with the lower vs upper
	// half (which makes a difference only if odd).
	llong offset = 0;
	while (n >= EInteger::MAX_VALUE) {
		int bits = next(2);
		llong half = ((ullong)n) >> 1;
		llong nextn = ((bits & 2) == 0) ? half : n - half;
		if ((bits & 1) == 0)
			offset += n - nextn;
		n = nextn;
	}
	return offset + nextInt((int) n);
}

llong EThreadLocalRandom::nextLong(llong least, llong bound) {
	if (least >= bound)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	return nextLong(bound - least) + least;
}

double EThreadLocalRandom::nextDouble(double n) {
	if (n <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "n must be positive");
	return nextDouble() * n;
}

double EThreadLocalRandom::nextDouble(double least, double bound) {
	if (least >= bound)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	return nextDouble() * (bound - least) + least;
}

EThreadLocalRandom* EThreadLocalRandom::current() {
	return localRandom->get();
}

int EThreadLocalRandom::next(int bits) {
	rnd = (rnd * multiplier + addend) & mask;
	return (int) ((unsigned)rnd >> (48-bits));
}

} /* namespace efc */
