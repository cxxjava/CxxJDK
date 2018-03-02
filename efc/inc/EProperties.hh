/*
 * EProperties.hh
 *
 *  Created on: 2017-11-23
 *      Author: cxxjava@163.com
 */

#ifndef EPROPERTIES_HH_
#define EPROPERTIES_HH_

#include "EHashMap.hh"
#include "EHashSet.hh"
#include "ESharedPtr.hh"
#include "EInputStream.hh"
#include "EOutputStream.hh"
#include "EPrintStream.hh"
#include "ESynchronizeable.hh"

namespace efc {

/**
 * The {@code Properties} class represents a persistent set of
 * properties. The {@code Properties} can be saved to a stream
 * or loaded from a stream. Each key and its corresponding value in
 * the property list is a string.
 * <p>
 * A property list can contain another property list as its
 * "defaults"; this second property list is searched if
 * the property key is not found in the original property list.
 * <p>
 * Because {@code Properties} inherits from {@code Hashtable}, the
 * {@code put} and {@code putAll} methods can be applied to a
 * {@code Properties} object.  Their use is strongly discouraged as they
 * allow the caller to insert entries whose keys or values are not
 * {@code Strings}.  The {@code setProperty} method should be used
 * instead.  If the {@code store} or {@code save} method is called
 * on a "compromised" {@code Properties} object that contains a
 * non-{@code String} key or value, the call will fail. Similarly,
 * the call to the {@code propertyNames} or {@code list} method
 * will fail if it is called on a "compromised" {@code Properties}
 * object that contains a non-{@code String} key.
 *
 * <p>
 * The {@link #load(java.io.Reader) load(Reader)} <tt>/</tt>
 * {@link #store(java.io.Writer, java.lang.String) store(Writer, String)}
 * methods load and store properties from and to a character based stream
 * in a simple line-oriented format specified below.
 *
 * The {@link #load(java.io.InputStream) load(InputStream)} <tt>/</tt>
 * {@link #store(java.io.OutputStream, java.lang.String) store(OutputStream, String)}
 * methods work the same way as the load(Reader)/store(Writer, String) pair, except
 * the input/output stream is encoded in ISO 8859-1 character encoding.
 * Characters that cannot be directly represented in this encoding can be written using
 * Unicode escapes as defined in section 3.3 of
 * <cite>The Java&trade; Language Specification</cite>;
 * only a single 'u' character is allowed in an escape
 * sequence. The native2ascii tool can be used to convert property files to and
 * from other character encodings.
 *
 * <p> The {@link #loadFromXML(InputStream)} and {@link
 * #storeToXML(OutputStream, String, String)} methods load and store properties
 * in a simple XML format.  By default the UTF-8 character encoding is used,
 * however a specific encoding may be specified if required. Implementations
 * are required to support UTF-8 and UTF-16 and may support other encodings.
 * An XML properties document has the following DOCTYPE declaration:
 *
 * <pre>
 * &lt;!DOCTYPE properties SYSTEM "http://java.sun.com/dtd/properties.dtd"&gt;
 * </pre>
 * Note that the system URI (http://java.sun.com/dtd/properties.dtd) is
 * <i>not</i> accessed when exporting or importing properties; it merely
 * serves as a string to uniquely identify the DTD, which is:
 * <pre>
 *    &lt;?xml version="1.0" encoding="UTF-8"?&gt;
 *
 *    &lt;!-- DTD for properties --&gt;
 *
 *    &lt;!ELEMENT properties ( comment?, entry* ) &gt;
 *
 *    &lt;!ATTLIST properties version CDATA #FIXED "1.0"&gt;
 *
 *    &lt;!ELEMENT comment (#PCDATA) &gt;
 *
 *    &lt;!ELEMENT entry (#PCDATA) &gt;
 *
 *    &lt;!ATTLIST entry key CDATA #REQUIRED&gt;
 * </pre>
 *
 * <p>This class is thread-safe: multiple threads can share a single
 * <tt>Properties</tt> object without the need for external synchronization.
 *
 * @see <a href="../../../technotes/tools/solaris/native2ascii.html">native2ascii tool for Solaris</a>
 * @see <a href="../../../technotes/tools/windows/native2ascii.html">native2ascii tool for Windows</a>
 *
 * @since   JDK1.0
 */

class LineReader;

class EProperties: public EHashMap<sp<EString>, sp<EString> >, public ESynchronizeable {
public:
	virtual ~EProperties();

	/**
	 * Creates an empty property list with the specified defaults.
	 *
	 * @param   defaults   the defaults.
	 */
	EProperties(EProperties* defaults=null, boolean owned=false);

	/**
	 * Calls the <tt>Hashtable</tt> method {@code put}. Provided for
	 * parallelism with the <tt>getProperty</tt> method. Enforces use of
	 * strings for property keys and values. The value returned is the
	 * result of the <tt>Hashtable</tt> call to {@code put}.
	 *
	 * @param key the key to be placed into this property list.
	 * @param value the value corresponding to <tt>key</tt>.
	 * @return     the previous value of the specified key in this property
	 *             list, or {@code null} if it did not have one.
	 * @see #getProperty
	 * @since    1.2
	 */
	synchronized sp<EString> setProperty(EString key, EString value);

	/**
	 * Reads a property list (key and element pairs) from the input
	 * character stream in a simple line-oriented format.
	 * <p>
	 * Properties are processed in terms of lines. There are two
	 * kinds of line, <i>natural lines</i> and <i>logical lines</i>.
	 * A natural line is defined as a line of
	 * characters that is terminated either by a set of line terminator
	 * characters ({@code \n} or {@code \r} or {@code \r\n})
	 * or by the end of the stream. A natural line may be either a blank line,
	 * a comment line, or hold all or some of a key-element pair. A logical
	 * line holds all the data of a key-element pair, which may be spread
	 * out across several adjacent natural lines by escaping
	 * the line terminator sequence with a backslash character
	 * {@code \}.  Note that a comment line cannot be extended
	 * in this manner; every natural line that is a comment must have
	 * its own comment indicator, as described below. Lines are read from
	 * input until the end of the stream is reached.
	 *
	 * <p>
	 * A natural line that contains only white space characters is
	 * considered blank and is ignored.  A comment line has an ASCII
	 * {@code '#'} or {@code '!'} as its first non-white
	 * space character; comment lines are also ignored and do not
	 * encode key-element information.  In addition to line
	 * terminators, this format considers the characters space
	 * ({@code ' '}, {@code '\u005Cu0020'}), tab
	 * ({@code '\t'}, {@code '\u005Cu0009'}), and form feed
	 * ({@code '\f'}, {@code '\u005Cu000C'}) to be white
	 * space.
	 *
	 * <p>
	 * If a logical line is spread across several natural lines, the
	 * backslash escaping the line terminator sequence, the line
	 * terminator sequence, and any white space at the start of the
	 * following line have no affect on the key or element values.
	 * The remainder of the discussion of key and element parsing
	 * (when loading) will assume all the characters constituting
	 * the key and element appear on a single natural line after
	 * line continuation characters have been removed.  Note that
	 * it is <i>not</i> sufficient to only examine the character
	 * preceding a line terminator sequence to decide if the line
	 * terminator is escaped; there must be an odd number of
	 * contiguous backslashes for the line terminator to be escaped.
	 * Since the input is processed from left to right, a
	 * non-zero even number of 2<i>n</i> contiguous backslashes
	 * before a line terminator (or elsewhere) encodes <i>n</i>
	 * backslashes after escape processing.
	 *
	 * <p>
	 * The key contains all of the characters in the line starting
	 * with the first non-white space character and up to, but not
	 * including, the first unescaped {@code '='},
	 * {@code ':'}, or white space character other than a line
	 * terminator. All of these key termination characters may be
	 * included in the key by escaping them with a preceding backslash
	 * character; for example,<p>
	 *
	 * {@code \:\=}<p>
	 *
	 * would be the two-character key {@code ":="}.  Line
	 * terminator characters can be included using {@code \r} and
	 * {@code \n} escape sequences.  Any white space after the
	 * key is skipped; if the first non-white space character after
	 * the key is {@code '='} or {@code ':'}, then it is
	 * ignored and any white space characters after it are also
	 * skipped.  All remaining characters on the line become part of
	 * the associated element string; if there are no remaining
	 * characters, the element is the empty string
	 * {@code ""}.  Once the raw character sequences
	 * constituting the key and element are identified, escape
	 * processing is performed as described above.
	 *
	 * <p>
	 * As an example, each of the following three lines specifies the key
	 * {@code "Truth"} and the associated element value
	 * {@code "Beauty"}:
	 * <pre>
	 * Truth = Beauty
	 *  Truth:Beauty
	 * Truth                    :Beauty
	 * </pre>
	 * As another example, the following three lines specify a single
	 * property:
	 * <pre>
	 * fruits                           apple, banana, pear, \
	 *                                  cantaloupe, watermelon, \
	 *                                  kiwi, mango
	 * </pre>
	 * The key is {@code "fruits"} and the associated element is:
	 * <pre>"apple, banana, pear, cantaloupe, watermelon, kiwi, mango"</pre>
	 * Note that a space appears before each {@code \} so that a space
	 * will appear after each comma in the final result; the {@code \},
	 * line terminator, and leading white space on the continuation line are
	 * merely discarded and are <i>not</i> replaced by one or more other
	 * characters.
	 * <p>
	 * As a third example, the line:
	 * <pre>cheeses
	 * </pre>
	 * specifies that the key is {@code "cheeses"} and the associated
	 * element is the empty string {@code ""}.
	 * <p>
	 * <a name="unicodeescapes"></a>
	 * Characters in keys and elements can be represented in escape
	 * sequences similar to those used for character and string literals
	 * (see sections 3.3 and 3.10.6 of
	 * <cite>The Java&trade; Language Specification</cite>).
	 *
	 * The differences from the character escape sequences and Unicode
	 * escapes used for characters and strings are:
	 *
	 * <ul>
	 * <li> Octal escapes are not recognized.
	 *
	 * <li> The character sequence {@code \b} does <i>not</i>
	 * represent a backspace character.
	 *
	 * <li> The method does not treat a backslash character,
	 * {@code \}, before a non-valid escape character as an
	 * error; the backslash is silently dropped.  For example, in a
	 * Java string the sequence {@code "\z"} would cause a
	 * compile time error.  In contrast, this method silently drops
	 * the backslash.  Therefore, this method treats the two character
	 * sequence {@code "\b"} as equivalent to the single
	 * character {@code 'b'}.
	 *
	 * <li> Escapes are not necessary for single and double quotes;
	 * however, by the rule above, single and double quote characters
	 * preceded by a backslash still yield single and double quote
	 * characters, respectively.
	 *
	 * <li> Only a single 'u' character is allowed in a Unicode escape
	 * sequence.
	 *
	 * </ul>
	 * <p>
	 * The specified stream remains open after this method returns.
	 *
	 * @param   inStream   the input stream.
	 * @throws  IOException  if an error occurred when reading from the
	 *          input stream.
	 * @throws  IllegalArgumentException if a malformed Unicode escape
	 *          appears in the input.
	 * @since   1.6
	 */
	synchronized void load(EInputStream* inStream) THROWS(EIOException);
	synchronized void load(const char* fileName) THROWS(EIOException);

	/**
	 * Writes this property list (key and element pairs) in this
	 * {@code Properties} table to the output stream in a format suitable
	 * for loading into a {@code Properties} table using the
	 * {@link #load(InputStream) load(InputStream)} method.
	 * <p>
	 * Properties from the defaults table of this {@code Properties}
	 * table (if any) are <i>not</i> written out by this method.
	 * <p>
	 * This method outputs the comments, properties keys and values in
	 * the same format as specified in
	 * {@link #store(java.io.Writer, java.lang.String) store(Writer)},
	 * with the following differences:
	 * <ul>
	 * <li>The stream is written using the ISO 8859-1 character encoding.
	 *
	 * <li>Characters not in Latin-1 in the comments are written as
	 * {@code \u005Cu}<i>xxxx</i> for their appropriate unicode
	 * hexadecimal value <i>xxxx</i>.
	 *
	 * <li>Characters less than {@code \u005Cu0020} and characters greater
	 * than {@code \u005Cu007E} in property keys or values are written
	 * as {@code \u005Cu}<i>xxxx</i> for the appropriate hexadecimal
	 * value <i>xxxx</i>.
	 * </ul>
	 * <p>
	 * After the entries have been written, the output stream is flushed.
	 * The output stream remains open after this method returns.
	 * <p>
	 * @param   out      an output stream.
	 * @param   comments   a description of the property list.
	 * @exception  IOException if writing this property list to the specified
	 *             output stream throws an <tt>IOException</tt>.
	 * @exception  ClassCastException  if this {@code Properties} object
	 *             contains any keys or values that are not {@code Strings}.
	 * @exception  NullPointerException  if {@code out} is null.
	 * @since 1.2
	 */
	void store(EOutputStream* out, EString comments) THROWS(EIOException);
	void store(const char* fileName, EString comments) THROWS(EIOException);

	/**
	 * Searches for the property with the specified key in this property list.
	 * If the key is not found in this property list, the default property list,
	 * and its defaults, recursively, are then checked. The method returns the
	 * default value argument if the property is not found.
	 *
	 * @param   key            the hashtable key.
	 * @param   defaultValue   a default value.
	 *
	 * @return  the value in this property list with the specified key value.
	 * @see     #setProperty
	 * @see     #defaults
	 */
	sp<EString> getProperty(EString key, const char* defaultValue=null);

	/**
	 * Returns a set of keys in this property list where
	 * the key and its corresponding value are strings,
	 * including distinct keys in the default property list if a key
	 * of the same name has not already been found from the main
	 * properties list.  Properties whose key or value is not
	 * of type <tt>String</tt> are omitted.
	 * <p>
	 * The returned set is not backed by the <tt>Properties</tt> object.
	 * Changes to this <tt>Properties</tt> are not reflected in the set,
	 * or vice versa.
	 *
	 * @return  a set of keys in this property list where
	 *          the key and its corresponding value are strings,
	 *          including the keys in the default property list.
	 * @see     java.util.Properties#defaults
	 * @since   1.6
	 */
	sp<ESet<sp<EString> > > propertyNames();

	/**
	 * Prints this property list out to the specified output stream.
	 * This method is useful for debugging.
	 *
	 * @param   out   an output stream.
	 * @throws  ClassCastException if any key in this property list
	 *          is not a string.
	 */
	void list(EPrintStream* out);

private:
	/**
	 * A property list that contains default values for any keys not
	 * found in this property list.
	 *
	 * @serial
	 */
	EProperties* defaults;

	boolean owned;

	void load0 (LineReader* lr) THROWS(EIOException);
	void store0(EPrintStream* out, EString& comments) THROWS(EIOException);

	/**
	 * Enumerates all key/value pairs in the specified hashtable.
	 * @param h the hashtable
	 * @throws ClassCastException if any of the property keys
	 *         is not of String type.
	 */
	synchronized void enumerate(EHashMap<sp<EString>, sp<EString> >& h);
	synchronized void enumerateKeys(EHashSet<sp<EString> >& h);

	/*
	 * Converts encoded &#92;uxxxx to unicode chars
	 * and changes special saved chars to their original forms
	 */
	sp<EString> loadConvert (const char* in, int off, int len);

	/*
	 * Converts unicodes to encoded &#92;uxxxx and escapes
	 * special characters with a preceding slash
	 */
	sp<EString> saveConvert(EString* theString, boolean escapeSpace);

	static void writeComments(EPrintStream* out, EString& comments) THROWS(EIOException);
};

} /* namespace efc */
#endif /* EPROPERTIES_HH_ */
