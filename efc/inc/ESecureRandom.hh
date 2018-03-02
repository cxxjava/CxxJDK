/*
 * ESecureRandom.hh
 *
 *  Created on: 2016-6-23
 *      Author: cxxjava@163.com
 */

#ifndef ESECURERANDOM_HH_
#define ESECURERANDOM_HH_

#include "ERandom.hh"

namespace efc {

/**
 * This class provides a cryptographically strong random number
 * generator (RNG).
 *
 * <p>A cryptographically strong random number
 * minimally complies with the statistical random number generator tests
 * specified in <a href="http://csrc.nist.gov/cryptval/140-2.htm">
 * <i>FIPS 140-2, Security Requirements for Cryptographic Modules</i></a>,
 * section 4.9.1.
 * Additionally, SecureRandom must produce non-deterministic output.
 * Therefore any seed material passed to a SecureRandom object must be
 * unpredictable, and all SecureRandom output sequences must be
 * cryptographically strong, as described in
 * <a href="http://www.ietf.org/rfc/rfc1750.txt">
 * <i>RFC 1750: Randomness Recommendations for Security</i></a>.
 *
 * <p>A caller obtains a SecureRandom instance via the
 * no-argument constructor or one of the {@code getInstance} methods:
 *
 * <pre>
 *      SecureRandom random = new SecureRandom();
 * </pre>
 *
 * <p> Many SecureRandom implementations are in the form of a pseudo-random
 * number generator (PRNG), which means they use a deterministic algorithm
 * to produce a pseudo-random sequence from a true random seed.
 * Other implementations may produce true random numbers,
 * and yet others may use a combination of both techniques.
 *
 * <p> Typical callers of SecureRandom invoke the following methods
 * to retrieve random bytes:
 *
 * <pre>
 *      SecureRandom random = new SecureRandom();
 *      byte bytes[] = new byte[20];
 *      random.nextBytes(bytes);
 * </pre>
 *
 * <p> Callers may also invoke the {@code generateSeed} method
 * to generate a given number of seed bytes (to seed other random number
 * generators, for example):
 * <pre>
 *      byte seed[] = random.generateSeed(20);
 * </pre>
 *
 * Note: Depending on the implementation, the {@code generateSeed} and
 * {@code nextBytes} methods may block as entropy is being gathered,
 * for example, if they need to read from /dev/random on various Unix-like
 * operating systems.
 *
 * @see java.security.SecureRandomSpi
 * @see java.util.Random
 *
 */

class ESecureRandom: public ERandom {
public:
	virtual ~ESecureRandom();

	/**
	 * Constructs a secure random number generator (RNG) implementing the
	 * default random number algorithm.
	 *
	 * <p> This constructor traverses the list of registered security Providers,
	 * starting with the most preferred Provider.
	 * A new SecureRandom object encapsulating the
	 * SecureRandomSpi implementation from the first
	 * Provider that supports a SecureRandom (RNG) algorithm is returned.
	 * If none of the Providers support a RNG algorithm,
	 * then an implementation-specific default is returned.
	 *
	 * <p> Note that the list of registered providers may be retrieved via
	 * the {@link Security#getProviders() Security.getProviders()} method.
	 *
	 * <p> See the SecureRandom section in the <a href=
	 * "{@docRoot}/../technotes/guides/security/StandardNames.html#SecureRandom">
	 * Java Cryptography Architecture Standard Algorithm Name Documentation</a>
	 * for information about standard RNG algorithm names.
	 *
	 * <p> The returned SecureRandom object has not been seeded.  To seed the
	 * returned object, call the {@code setSeed} method.
	 * If {@code setSeed} is not called, the first call to
	 * {@code nextBytes} will force the SecureRandom object to seed itself.
	 * This self-seeding will not occur if {@code setSeed} was
	 * previously called.
	 */
	ESecureRandom();

	/**
	 * Constructs a secure random number generator (RNG) implementing the
	 * default random number algorithm.
	 * The SecureRandom instance is seeded with the specified seed bytes.
	 *
	 * <p> This constructor traverses the list of registered security Providers,
	 * starting with the most preferred Provider.
	 * A new SecureRandom object encapsulating the
	 * SecureRandomSpi implementation from the first
	 * Provider that supports a SecureRandom (RNG) algorithm is returned.
	 * If none of the Providers support a RNG algorithm,
	 * then an implementation-specific default is returned.
	 *
	 * <p> Note that the list of registered providers may be retrieved via
	 * the {@link Security#getProviders() Security.getProviders()} method.
	 *
	 * <p> See the SecureRandom section in the <a href=
	 * "{@docRoot}/../technotes/guides/security/StandardNames.html#SecureRandom">
	 * Java Cryptography Architecture Standard Algorithm Name Documentation</a>
	 * for information about standard RNG algorithm names.
	 *
	 * @param seed the initial seed
	 */
	ESecureRandom(llong seed);

	/**
	 * Generates a user-specified number of random bytes.
	 *
	 * <p> If a call to {@code setSeed} had not occurred previously,
	 * the first call to this method forces this SecureRandom object
	 * to seed itself.  This self-seeding will not occur if
	 * {@code setSeed} was previously called.
	 *
	 * @param bytes the array to be filled in with random bytes.
	 */
	virtual void nextBytes(EA<byte>* bytes);
	virtual void nextBytes(byte* bytes, int size);

protected:
	/**
	 * Generates an integer containing the user-specified number of
	 * pseudo-random bits (right justified, with leading zeros).  This
	 * method overrides a {@code java.util.Random} method, and serves
	 * to provide a source of random bits to all of the methods inherited
	 * from that class (for example, {@code nextInt},
	 * {@code nextLong}, and {@code nextFloat}).
	 *
	 * @param numBits number of pseudo-random bits to be generated, where
	 * {@code 0 <= numBits <= 32}.
	 *
	 * @return an {@code int} containing the user-specified number
	 * of pseudo-random bits (right justified, with leading zeros).
	 */
	virtual int next(int bits);

private:
	llong provider;

	ESecureRandom(const ESecureRandom& that); //unsupported
	ESecureRandom& operator= (const ESecureRandom& that); //unsupported

	void init();
	void uninit();
};

} /* namespace efc */
#endif /* ESECURERANDOM_HH_ */
