/*
 * EMath.hh
 *
 *  Created on: 2013-3-19
 *      Author: Administrator
 */

#ifndef EMath_HH_
#define EMath_HH_

#include "EObject.hh"

#include <math.h>

namespace efc {

class ERandom;

/**
 *
 * @class Math
 * @version $Id: Math.h,v 1.16 2003-08-26 19:50:16 florian Exp $
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class EMath : public EObject {
public:
	DECLARE_STATIC_INITZZ;

public:
	/**
	 * @brief The mathematical constant <em>e</em>.
	 * Used in natural log and exp.
	 */
	static const double E;// = 2.7182818284590452354;

	/**
	 * @brief The mathematical constant <em>pi</em>.
	 * This is the ratio of a circle's diameter to its circumference.
	 */
	static const double PI;// = 3.14159265358979323846;

	/**
	 * @brief The trigonometric function <em>cos</em>.
	 * @param a the angle (in radians).
	 * @return cos(a).
	 */
	static inline double cos(double a) {
		return ::cos(a);
	}

	/**
	 * @brief The trigonometric function <em>sin</em>.
	 * @param a the angle (in radians).
	 * @return sin(a).
	 */
	static inline double sin(double a) {
		return ::sin(a);
	}

	/**
	 * @brief The trigonometric function <em>tan</em>.
	 * @param a the angle (in radians).
	 * @return tan(a).
	 */
	static inline double tan(double a) {
		return ::tan(a);
	}

	/**
	 * @brief The trigonometric function <em>arccos</em>.
	 * The range of angles you will get are from 0 to pi radians (0 to 180 degrees).
	 * @param a the cos to turn back into an angle.
	 * @return arccos(a).
	 */
	static inline double acos(double a) {
		return ::acos(a);
	}

	/**
	 * @brief The trigonometric function <em>arcsin</em>.
	 * The range of angles you will get are from -pi/2 to pi/2 radians (-90 to 90 degrees)
	 * @param a the sin to turn back into an angle.
	 * @return arcsin(a).
	 */
	static inline double asin(double a) {
		return ::asin(a);
	}

	/**
	 * @brief The trigonometric function <em>arctan</em>.
	 * The range of angles you will get are from -pi/2 to pi/2 radians (-90 to 90 degrees)
	 * @param a the sin to turn back into an angle.
	 * @return arcsin(a).
	 */
	static inline double atan(double a) {
		return ::atan(a);
	}

	/**
	 * @brief A special version of the trigonometric function <em>arctan</em>.
	 * Given a position (x,y), this function will give you the angle of
	 * that position.
	 * The range of angles you will get are from -pi to pi radians (-180 to 180 degrees),
	 * the whole spectrum of angles.  That is what makes this function so
	 * much more useful than the other <code>atan()</code>.
	 * @param y the y position
	 * @param x the x position
	 * @return arctan2(y, x).
	 */
	static inline double atan2(double y, double x) {
		return ::atan2(y, x);
	}

	/**
	 * @brief Take <em>e</em><sup>a</sup>.  The opposite of <code>log()</code>.
	 * @param a the number to raise to the power.
	 * @return the number raised to the power of <em>e</em>.
	 */
	static inline double exp(double a) {
		return ::exp(a);
	}

	/**
	 * @brief Take ln(a) (the natural log).  The opposite of <code>exp()</code>.
	 * Note that the way to get log<sub>b</sub>(a) is to do this:
	 * <code>log(a) / log(b)</code>.
	 * @param a the number to take the natural log of.
	 * @return the natural log of <code>a</code>.
	 */
	static inline double log(double a) {
		return ::log(a);
	}

	/**
	 * @brief Take a square root.
	 * For other roots, to pow(a,1/rootNumber).
	 * @param a the numeric argument
	 * @return the square root of the argument.
	 */
	static inline double sqrt(double a) {
		return ::sqrt(a);
	}

	/**
	 * @brief Take a number to a power.
	 * @param a the number to raise.
	 * @param b the power to raise it to.
	 * @return a<sup>b</sup>.
	 */
	static inline double pow(double a, double b) {
		return ::pow(a, b);
	}

	/**
	 * @brief Take the nearest integer that is that is greater than or equal to the
	 * argument.
	 * @param a the value to act upon.
	 * @return the nearest integer >= <code>a</code>.
	 */
	static inline double ceil(double a) {
		return ::ceil(a);
	}

	/**
	 * @brief Take the nearest integer that is that is less than or equal to the
	 * argument.
	 * @param a the value to act upon.
	 * @return the nearest integer <= <code>a</code>.
	 */
	static inline double floor(double a) {
		return ::floor(a);
	}

	/**
	 * @brief Take the nearest integer to the argument.  If it is exactly between
	 * two integers, the even integer is taken.
	 * @param a the value to act upon.
	 * @return the nearest integer to <code>a</code>.
	 */
	//static double rint(double a) { return ::rint(a); }
	/**
	 * @brief Take the nearest integer to the argument.  If it is exactly between
	 * two integers, then the lower of the two (-10 lower than -9) is taken.
	 * If the argument is less than \c Integer.MIN_VALUE or negative infinity,
	 * \c Integer.MIN_VALUE will be returned.  If the argument is greater than
	 * \c Integer.MAX_VALUE, \c Integer.MAX_VALUE will be returned.
	 *
	 * @param a the argument to round.
	 * @return the nearest integer to the argument.
	 */
	static inline int round(float a) {
		return (int) ::floor(a + 0.5f);
	}

	/**
	 * @brief Take the nearest integer to the argument.  If it is exactly between
	 * two integers, then the lower of the two (-10 lower than -9) is taken.
	 * If the argument is less than \c Long.MIN_VALUE or negative infinity,
	 * \c Long.MIN_VALUE will be returned.  If the argument is greater than
	 * \c Long.MAX_VALUE, \c Long.MAX_VALUE will be returned.
	 *
	 * @param a the argument to round.
	 * @return the nearest integer to the argument.
	 */
	static inline long round(double a) {
		return (long) floor(a + 0.5F);
	}

	/**
	 * @brief Convert from degrees to radians.
	 * The formula for this is radians = degrees * (pi/180).
	 * @param degrees an angle in degrees
	 * @return the angle in radians
	 */
	static double toRadians(double degrees);

	/**
	 * @brief Convert from radians to degrees.
	 * The formula for this is degrees = radians * (180/pi).
	 * @param rads an angle in radians
	 * @return the angle in degrees
	 */
	static double toDegrees(double rads);

	/**
	 * Returns the absolute value of an {@code int} value.
	 * If the argument is not negative, the argument is returned.
	 * If the argument is negative, the negation of the argument is returned.
	 *
	 * <p>Note that if the argument is equal to the value of
	 * {@link Integer#MIN_VALUE}, the most negative representable
	 * {@code int} value, the result is that same value, which is
	 * negative.
	 *
	 * @param   a   the argument whose absolute value is to be determined
	 * @return  the absolute value of the argument.
	 */
	static inline int abs(int a) {
		return (a < 0) ? -a : a;
	}

	/**
	 * Returns the absolute value of a {@code long} value.
	 * If the argument is not negative, the argument is returned.
	 * If the argument is negative, the negation of the argument is returned.
	 *
	 * <p>Note that if the argument is equal to the value of
	 * {@link Long#MIN_VALUE}, the most negative representable
	 * {@code long} value, the result is that same value, which
	 * is negative.
	 *
	 * @param   a   the argument whose absolute value is to be determined
	 * @return  the absolute value of the argument.
	 */
	static inline llong abs(llong a) {
		return (a < 0) ? -a : a;
	}
#if (SIZEOF_LONG != 8)
	static inline long abs(long a) {
		return (a < 0) ? -a : a;
	}
#endif

	/**
	 * Returns the absolute value of a {@code float} value.
	 * If the argument is not negative, the argument is returned.
	 * If the argument is negative, the negation of the argument is returned.
	 * Special cases:
	 * <ul><li>If the argument is positive zero or negative zero, the
	 * result is positive zero.
	 * <li>If the argument is infinite, the result is positive infinity.
	 * <li>If the argument is NaN, the result is NaN.</ul>
	 * In other words, the result is the same as the value of the expression:
	 * <p>{@code Float.intBitsToFloat(0x7fffffff & Float.floatToIntBits(a))}
	 *
	 * @param   a   the argument whose absolute value is to be determined
	 * @return  the absolute value of the argument.
	 */
	static inline float abs(float a) {
		return (a <= 0.0F) ? 0.0F - a : a;
	}

	/**
	 * Returns the absolute value of a {@code double} value.
	 * If the argument is not negative, the argument is returned.
	 * If the argument is negative, the negation of the argument is returned.
	 * Special cases:
	 * <ul><li>If the argument is positive zero or negative zero, the result
	 * is positive zero.
	 * <li>If the argument is infinite, the result is positive infinity.
	 * <li>If the argument is NaN, the result is NaN.</ul>
	 * In other words, the result is the same as the value of the expression:
	 * <p>{@code Double.longBitsToDouble((Double.doubleToLongBits(a)<<1)>>>1)}
	 *
	 * @param   a   the argument whose absolute value is to be determined
	 * @return  the absolute value of the argument.
	 */
	static inline double abs(double a) {
		return (a < 0.0 ? -a : a);
	}

	/**
	 * @brief Get the floating point remainder on two numbers,
	 * which really does the following:
	 * <P>
	 *
	 * <OL>
	 *   <LI>
	 *       Takes x/y and finds the nearest integer <em>n</em> to the
	 *       quotient.  (Uses the <code>rint()</code> function to do this.
	 *   </LI>
	 *   <LI>
	 *       Takes x - y*<em>n</em>.
	 *   </LI>
	 *   <LI>
	 *       If x = y*n, then the result is 0 if x is positive and -0 if x
	 *       is negative.
	 *   </LI>
	 * </OL>
	 *
	 * @param x the dividend (the top half)
	 * @param y the divisor (the bottom half)
	 * @return the IEEE 754-defined floating point remainder of x/y.
	 */
	//static double IEEEremainder(double x, double y);
	/**
	 * @brief Returns the greater of the two given \c long values.
	 */
	static inline llong max(llong a, llong b) {
		return (a > b ? a : b);
	}
#if (SIZEOF_LONG != 8)
	static inline long max(long a, long b) {
		return (a > b ? a : b);
	}
#endif

	/**
	 * @brief Returns the smaller of the two given \c long values.
	 */
	static inline llong min(llong a, llong b) {
		return (a < b ? a : b);
	}
#if (SIZEOF_LONG != 8)
	static inline long min(long a, long b) {
		return (a < b ? a : b);
	}
#endif

	/**
	 * @brief Returns the greater of the two given \c int values.
	 */
	static inline int max(int a, int b) {
		return (a > b ? a : b);
	}

	/**
	 * @brief Returns the smaller of the two given \c int values.
	 */
	static inline int min(int a, int b) {
		return (a < b ? a : b);
	}

	/**
	 * @brief Returns the greater of the two given \c double values.
	 */
	static inline double max(double a, double b) {
		return (a > b ? a : b);
	}

	/**
	 * @brief Returns the smaller of the two given \c double values.
	 */
	static inline double min(double a, double b) {
		return (a < b ? a : b);
	}

	/**
	 * Returns a {@code double} value with a positive sign, greater
	 * than or equal to {@code 0.0} and less than {@code 1.0}.
	 * Returned values are chosen pseudorandomly with (approximately)
	 * uniform distribution from that range.
	 *
	 * <p>When this method is first called, it creates a single new
	 * pseudorandom-number generator, exactly as if by the expression
	 * <blockquote>{@code new java.util.Random}</blockquote> This
	 * new pseudorandom-number generator is used thereafter for all
	 * calls to this method and is used nowhere else.
	 *
	 * <p>This method is properly synchronized to allow correct use by
	 * more than one thread. However, if many threads need to generate
	 * pseudorandom numbers at a great rate, it may reduce contention
	 * for each thread to have its own pseudorandom-number generator.
	 *
	 * @return  a pseudorandom {@code double} greater than or equal
	 * to {@code 0.0} and less than {@code 1.0}.
	 * @see     java.util.Random#nextDouble()
	 */
	static double random();

private:
	static ERandom* randomNumberGenerator;
	static ERandom* getRandom();
};

} /* namespace efc */
#endif //!EMath_HH_
