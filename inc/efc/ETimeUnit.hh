/*
 * ETimeUnit.hh
 *
 *  Created on: 2014-6-27
 *      Author: cxxjava@163.com
 */

#ifndef ETIMEUNIT_HH_
#define ETIMEUNIT_HH_

#include "ELLong.hh"
#include "EUnsupportedOperationException.hh"
#include "EInterruptedException.hh"

namespace efc {

/**
 * A <tt>TimeUnit</tt> represents time durations at a given unit of
 * granularity and provides utility methods to convert across units,
 * and to perform timing and delay operations in these units.  A
 * <tt>TimeUnit</tt> does not maintain time information, but only
 * helps organize and use time representations that may be maintained
 * separately across various contexts.  A nanosecond is defined as one
 * thousandth of a microsecond, a microsecond as one thousandth of a
 * millisecond, a millisecond as one thousandth of a second, a minute
 * as sixty seconds, an hour as sixty minutes, and a day as twenty four
 * hours.
 *
 * <p>A <tt>TimeUnit</tt> is mainly used to inform time-based methods
 * how a given timing parameter should be interpreted. For example,
 * the following code will timeout in 50 milliseconds if the {@link
 * java.util.concurrent.locks.Lock lock} is not available:
 *
 * <pre>  Lock lock = ...;
 *  if ( lock.tryLock(50L, TimeUnit.MILLISECONDS) ) ...
 * </pre>
 * while this code will timeout in 50 seconds:
 * <pre>
 *  Lock lock = ...;
 *  if ( lock.tryLock(50L, TimeUnit.SECONDS) ) ...
 * </pre>
 *
 * Note however, that there is no guarantee that a particular timeout
 * implementation will be able to notice the passage of time at the
 * same granularity as the given <tt>TimeUnit</tt>.
 *
 * @since 1.5
 * @author Doug Lea
 */

class EThread;
class ESynchronizeable;

class _NANOSECONDS;
class _MICROSECONDS;
class _MILLISECONDS;
class _SECONDS;
class _MINUTES;
class _HOURS;
class _DAYS;

class ETimeUnit {
public:
	DECLARE_STATIC_INITZZ;

public:
    static _NANOSECONDS* NANOSECONDS;
    static _MICROSECONDS* MICROSECONDS;
    static _MILLISECONDS* MILLISECONDS;
    static _SECONDS* SECONDS;
    static _MINUTES* MINUTES;
    static _HOURS* HOURS;
    static _DAYS* DAYS;

public:
    virtual ~ETimeUnit(){}

    // To maintain full signature compatibility with 1.5, and to improve the
    // clarity of the generated javadoc (see 6287639: Abstract methods in
    // enum classes should not be listed as abstract), method convert
    // etc. are not declared abstract but otherwise act as abstract methods.

    /**
     * Convert the given time duration in the given unit to this
     * unit.  Conversions from finer to coarser granularities
     * truncate, so lose precision. For example converting
     * <tt>999</tt> milliseconds to seconds results in
     * <tt>0</tt>. Conversions from coarser to finer granularities
     * with arguments that would numerically overflow saturate to
     * <tt>Long.MIN_VALUE</tt> if negative or <tt>Long.MAX_VALUE</tt>
     * if positive.
     *
     * <p>For example, to convert 10 minutes to milliseconds, use:
     * <tt>TimeUnit.MILLISECONDS.convert(10L, TimeUnit.MINUTES)</tt>
     *
     * @param sourceDuration the time duration in the given <tt>sourceUnit</tt>
     * @param sourceUnit the unit of the <tt>sourceDuration</tt> argument
     * @return the converted duration in this unit,
     * or <tt>Long.MIN_VALUE</tt> if conversion would negatively
     * overflow, or <tt>Long.MAX_VALUE</tt> if it would positively overflow.
     */
    virtual llong convert(llong sourceDuration, ETimeUnit* sourceUnit) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Equivalent to <tt>NANOSECONDS.convert(duration, this)</tt>.
     * @param duration the duration
     * @return the converted duration,
     * or <tt>Long.MIN_VALUE</tt> if conversion would negatively
     * overflow, or <tt>Long.MAX_VALUE</tt> if it would positively overflow.
     * @see #convert
     */
    virtual llong toNanos(llong duration) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Equivalent to <tt>MICROSECONDS.convert(duration, this)</tt>.
     * @param duration the duration
     * @return the converted duration,
     * or <tt>Long.MIN_VALUE</tt> if conversion would negatively
     * overflow, or <tt>Long.MAX_VALUE</tt> if it would positively overflow.
     * @see #convert
     */
    virtual llong toMicros(llong duration) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Equivalent to <tt>MILLISECONDS.convert(duration, this)</tt>.
     * @param duration the duration
     * @return the converted duration,
     * or <tt>Long.MIN_VALUE</tt> if conversion would negatively
     * overflow, or <tt>Long.MAX_VALUE</tt> if it would positively overflow.
     * @see #convert
     */
    virtual llong toMillis(llong duration) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Equivalent to <tt>SECONDS.convert(duration, this)</tt>.
     * @param duration the duration
     * @return the converted duration,
     * or <tt>Long.MIN_VALUE</tt> if conversion would negatively
     * overflow, or <tt>Long.MAX_VALUE</tt> if it would positively overflow.
     * @see #convert
     */
    virtual llong toSeconds(llong duration) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Equivalent to <tt>MINUTES.convert(duration, this)</tt>.
     * @param duration the duration
     * @return the converted duration,
     * or <tt>Long.MIN_VALUE</tt> if conversion would negatively
     * overflow, or <tt>Long.MAX_VALUE</tt> if it would positively overflow.
     * @see #convert
     * @since 1.6
     */
    virtual llong toMinutes(llong duration) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Equivalent to <tt>HOURS.convert(duration, this)</tt>.
     * @param duration the duration
     * @return the converted duration,
     * or <tt>Long.MIN_VALUE</tt> if conversion would negatively
     * overflow, or <tt>Long.MAX_VALUE</tt> if it would positively overflow.
     * @see #convert
     * @since 1.6
     */
    virtual llong toHours(llong duration) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Equivalent to <tt>DAYS.convert(duration, this)</tt>.
     * @param duration the duration
     * @return the converted duration
     * @see #convert
     * @since 1.6
     */
    virtual llong toDays(llong duration) {
        throw EUnsupportedOperationException(__FILE__, __LINE__);
    }

    /**
     * Utility to compute the excess-nanosecond argument to wait,
     * sleep, join.
     * @param d the duration
     * @param m the number of milliseconds
     * @return the number of nanoseconds
     */
    virtual int excessNanos(llong d, llong m) = 0;

    /**
     * Performs a timed <tt>Object.wait</tt> using this time unit.
     * This is a convenience method that converts timeout arguments
     * into the form required by the <tt>Object.wait</tt> method.
     *
     * <p>For example, you could implement a blocking <tt>poll</tt>
     * method (see {@link BlockingQueue#poll BlockingQueue.poll})
     * using:
     *
     * <pre>  public synchronized Object poll(long timeout, TimeUnit unit) throws InterruptedException {
     *    while (empty) {
     *      unit.timedWait(this, timeout);
     *      ...
     *    }
     *  }</pre>
     *
     * @param obj the object to wait on
     * @param timeout the maximum time to wait. If less than
     * or equal to zero, do not wait at all.
     * @throws InterruptedException if interrupted while waiting.
     * @see Object#wait(long, int)
     */
    void timedWait(ESynchronizeable* obj, llong timeout) THROWS(EInterruptedException);

    /**
     * Performs a timed <tt>Thread.join</tt> using this time unit.
     * This is a convenience method that converts time arguments into the
     * form required by the <tt>Thread.join</tt> method.
     * @param thread the thread to wait for
     * @param timeout the maximum time to wait. If less than
     * or equal to zero, do not wait at all.
     * @throws InterruptedException if interrupted while waiting.
     * @see Thread#join(long, int)
     */
    void timedJoin(EThread* thread, llong timeout) THROWS(EInterruptedException);

    /**
     * Performs a <tt>Thread.sleep</tt> using this unit.
     * This is a convenience method that converts time arguments into the
     * form required by the <tt>Thread.sleep</tt> method.
     * @param timeout the minimum time to sleep. If less than
     * or equal to zero, do not sleep at all.
     * @throws InterruptedException if interrupted while sleeping.
     * @see Thread#sleep
     */
    void sleep(llong timeout) THROWS(EInterruptedException);

protected:
    // Handy constants for conversion methods
	static const llong C0 = 1L;
	static const llong C1 = 1000L;// = C0 * 1000L;
	static const llong C2 = 1000000L;// = C1 * 1000L;
	static const llong C3 = 1000000000L;// = C2 * 1000L;
	static const llong C4 = 60000000000L;// = C3 * 60L;
	static const llong C5 = 3600000000000L;// = C4 * 60L;
	static const llong C6 = 86400000000000L;// = C5 * 24L;

	static const llong MAX = LLONG(0x7fffffffffffffff);// = ELLong::MAX_VALUE;

    /**
     * Scale d by m, checking for overflow.
     * This has a short name to make above code more readable.
     */
    static llong x(llong d, llong m, llong over) {
        if (d >  over) return ELLong::MAX_VALUE;
        if (d < -over) return ELLong::MIN_VALUE;
        return d * m;
    }
};

class _NANOSECONDS: public ETimeUnit {
public:
	llong toNanos(llong d) {return d;}
	llong toMicros(llong d) {return d/(C1/C0);}
	llong toMillis(llong d) {return d/(C2/C0);}
	llong toSeconds(llong d) {return d/(C3/C0);}
	llong toMinutes(llong d) {return d/(C4/C0);}
	llong toHours(llong d) {return d/(C5/C0);}
	llong toDays(llong d) {return d/(C6/C0);}
	llong convert(llong d, ETimeUnit* u) {return u->toNanos(d);}
private:
	int excessNanos(llong d, llong m) {return (int)(d - (m*C2));}
};
class _MICROSECONDS: public ETimeUnit {
public:
	llong toNanos(llong d)   { return x(d, C1/C0, MAX/(C1/C0)); }
	llong toMicros(llong d)  { return d; }
	llong toMillis(llong d)  { return d/(C2/C1); }
	llong toSeconds(llong d) { return d/(C3/C1); }
	llong toMinutes(llong d) { return d/(C4/C1); }
	llong toHours(llong d)   { return d/(C5/C1); }
	llong toDays(llong d)    { return d/(C6/C1); }
	llong convert(llong d, ETimeUnit* u) { return u->toMicros(d); }
	int excessNanos(llong d, llong m) { return (int)((d*C1) - (m*C2)); }
};
class _MILLISECONDS: public ETimeUnit {
public:
	llong toNanos(llong d)   { return x(d, C2/C0, MAX/(C2/C0)); }
	llong toMicros(llong d)  { return x(d, C2/C1, MAX/(C2/C1)); }
	llong toMillis(llong d)  { return d; }
	llong toSeconds(llong d) { return d/(C3/C2); }
	llong toMinutes(llong d) { return d/(C4/C2); }
	llong toHours(llong d)   { return d/(C5/C2); }
	llong toDays(llong d)    { return d/(C6/C2); }
	llong convert(llong d, ETimeUnit* u) { return u->toMillis(d); }
	int excessNanos(llong d, llong m) { return 0; }
};
class _SECONDS: public ETimeUnit {
public:
	llong toNanos(llong d)   { return x(d, C3/C0, MAX/(C3/C0)); }
	llong toMicros(llong d)  { return x(d, C3/C1, MAX/(C3/C1)); }
	llong toMillis(llong d)  { return x(d, C3/C2, MAX/(C3/C2)); }
	llong toSeconds(llong d) { return d; }
	llong toMinutes(llong d) { return d/(C4/C3); }
	llong toHours(llong d)   { return d/(C5/C3); }
	llong toDays(llong d)    { return d/(C6/C3); }
	llong convert(llong d, ETimeUnit* u) { return u->toSeconds(d); }
	int excessNanos(llong d, llong m) { return 0; }
};
class _MINUTES: public ETimeUnit {
public:
	llong toNanos(llong d)   { return x(d, C4/C0, MAX/(C4/C0)); }
	llong toMicros(llong d)  { return x(d, C4/C1, MAX/(C4/C1)); }
	llong toMillis(llong d)  { return x(d, C4/C2, MAX/(C4/C2)); }
	llong toSeconds(llong d) { return x(d, C4/C3, MAX/(C4/C3)); }
	llong toMinutes(llong d) { return d; }
	llong toHours(llong d)   { return d/(C5/C4); }
	llong toDays(llong d)    { return d/(C6/C4); }
	llong convert(llong d, ETimeUnit* u) { return u->toMinutes(d); }
	int excessNanos(llong d, llong m) { return 0; }
};
class _HOURS: public ETimeUnit {
public:
	llong toNanos(llong d)   { return x(d, C5/C0, MAX/(C5/C0)); }
	llong toMicros(llong d)  { return x(d, C5/C1, MAX/(C5/C1)); }
	llong toMillis(llong d)  { return x(d, C5/C2, MAX/(C5/C2)); }
	llong toSeconds(llong d) { return x(d, C5/C3, MAX/(C5/C3)); }
	llong toMinutes(llong d) { return x(d, C5/C4, MAX/(C5/C4)); }
	llong toHours(llong d)   { return d; }
	llong toDays(llong d)    { return d/(C6/C5); }
	llong convert(llong d, ETimeUnit* u) { return u->toHours(d); }
	int excessNanos(llong d, llong m) { return 0; }
};
class _DAYS: public ETimeUnit {
public:
	llong toNanos(llong d)   { return x(d, C6/C0, MAX/(C6/C0)); }
	llong toMicros(llong d)  { return x(d, C6/C1, MAX/(C6/C1)); }
	llong toMillis(llong d)  { return x(d, C6/C2, MAX/(C6/C2)); }
	llong toSeconds(llong d) { return x(d, C6/C3, MAX/(C6/C3)); }
	llong toMinutes(llong d) { return x(d, C6/C4, MAX/(C6/C4)); }
	llong toHours(llong d)   { return x(d, C6/C5, MAX/(C6/C5)); }
	llong toDays(llong d)    { return d; }
	llong convert(llong d, ETimeUnit* u) { return u->toDays(d); }
	int excessNanos(llong d, llong m) { return 0; }
};

} /* namespace efc */
#endif /* ETIMEUNIT_HH_ */
