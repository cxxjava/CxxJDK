/*
 * EURLDecoder.hh
 *
 *  Created on: 2013-3-16
 *      Author: Administrator
 */

#ifndef EURLDecoder_HH_
#define EURLDecoder_HH_

#include "EString.hh"

namespace efc {

/**
 * Utility class for HTML form decoding. This class contains static methods
 * for decoding a String from the <CODE>application/x-www-form-urlencoded</CODE>
 * MIME format.
 * <p>
 * To conversion process is the reverse of that used by the URLEncoder class. It is assumed
 * that all characters in the encoded string are one of the following: 
 * &quot;<code>a</code>&quot; through &quot;<code>z</code>&quot;,
 * &quot;<code>A</code>&quot; through &quot;<code>Z</code>&quot;,
 * &quot;<code>0</code>&quot; through &quot;<code>9</code>&quot;, and
 * &quot;<code>-</code>&quot;, &quot;<code>_</code>&quot;,
 * &quot;<code>.</code>&quot;, and &quot;<code>*</code>&quot;. The
 * character &quot;<code>%</code>&quot; is allowed but is interpreted
 * as the start of a special escaped sequence.
 * <p>
 * The following rules are applied in the conversion:
 * <p>
 * <ul>
 * <li>The alphanumeric characters &quot;<code>a</code>&quot; through
 *     &quot;<code>z</code>&quot;, &quot;<code>A</code>&quot; through
 *     &quot;<code>Z</code>&quot; and &quot;<code>0</code>&quot; 
 *     through &quot;<code>9</code>&quot; remain the same.
 * <li>The special characters &quot;<code>.</code>&quot;,
 *     &quot;<code>-</code>&quot;, &quot;<code>*</code>&quot;, and
 *     &quot;<code>_</code>&quot; remain the same. 
 * <li>The plus sign &quot;<code>+</code>&quot; is converted into a
 *     space character &quot;<code>&nbsp;</code>&quot; .
 * <li>A sequence of the form "<code>%<i>xy</i></code>" will be
 *     treated as representing a byte where <i>xy</i> is the two-digit
 *     hexadecimal representation of the 8 bits. Then, all substrings
 *     that contain one or more of these byte sequences consecutively
 *     will be replaced by the character(s) whose encoding would result
 *     in those consecutive bytes. 
 *     The encoding scheme used to decode these characters may be specified, 
 *     or if unspecified, the default encoding of the platform will be used.
 * </ul>
 * <p>
 * There are two possible ways in which this decoder could deal with
 * illegal strings.  It could either leave illegal characters alone or
 * it could throw an <tt>{@link java.lang.IllegalArgumentException}</tt>.
 * Which approach the decoder takes is left to the
 * implementation.
 *
 * @version 1.28, 11/17/05
 * @since   1.2
 */
class EURLDecoder {
public:
	
	/**
     * Decodes a <code>x-www-form-urlencoded</code> string.
     * The platform's default encoding is used to determine what characters 
     * are represented by any consecutive sequences of the form 
     * "<code>%<i>xy</i></code>".
     * @param s the <code>String</code> to decode
     * @deprecated The resulting string may vary depending on the platform's
     *          default encoding. Instead, use the decode(String,String) method
     *          to specify the encoding.
     * @return the newly decoded <code>String</code>
     */
	static EString decode(const char* s);
};

} /* namespace efc */
#endif //!EURLDecoder_HH_
