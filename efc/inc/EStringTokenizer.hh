#ifndef __ESTRINGTOKENIZER_HH__
#define __ESTRINGTOKENIZER_HH__

#include "EObject.hh"
#include "ENoSuchElementException.hh"

namespace efc {

/**
 * The string tokenizer class allows an application to break a
 * string into tokens. The tokenization method is much simpler than
 * the one used by the <code>StreamTokenizer</code> class. The
 * <code>StringTokenizer</code> methods do not distinguish among
 * identifiers, numbers, and quoted strings, nor do they recognize
 * and skip comments.
 * <p>
 * The set of delimiters (the characters that separate tokens) may
 * be specified either at creation time or on a per-token basis.
 * <p>
 * An instance of <code>StringTokenizer</code> behaves in one of two
 * ways, depending on whether it was created with the
 * <code>returnDelims</code> flag having the value <code>true</code>
 * or <code>false</code>:
 * <ul>
 * <li>If the flag is <code>false</code>, delimiter characters serve to
 *     separate tokens. A token is a maximal sequence of consecutive
 *     characters that are not delimiters.
 * <li>If the flag is <code>true</code>, delimiter characters are themselves
 *     considered to be tokens. A token is thus either one delimiter
 *     character, or a maximal sequence of consecutive characters that are
 *     not delimiters.
 * </ul><p>
 * A <tt>StringTokenizer</tt> object internally maintains a current
 * position within the string to be tokenized. Some operations advance this
 * current position past the characters processed.<p>
 * A token is returned by taking a substring of the string that was used to
 * create the <tt>StringTokenizer</tt> object.
 * <p>
 * The following is one example of the use of the tokenizer. The code:
 * <blockquote><pre>
 *     StringTokenizer st = new StringTokenizer("this is a test");
 *     while (st.hasMoreTokens()) {
 *         System.out.println(st.nextToken());
 *     }
 * </pre></blockquote>
 * <p>
 * prints the following output:
 * <blockquote><pre>
 *     this
 *     is
 *     a
 *     test
 * </pre></blockquote>
 *
 * <p>
 * <tt>StringTokenizer</tt> is a legacy class that is retained for
 * compatibility reasons although its use is discouraged in new code. It is
 * recommended that anyone seeking this functionality use the <tt>split</tt>
 * method of <tt>String</tt> or the java.util.regex package instead.
 * <p>
 * The following example illustrates how the <tt>String.split</tt>
 * method can be used to break up a string into its basic tokens:
 * <blockquote><pre>
 *     String[] result = "this is a test".split("\\s");
 *     for (int x=0; x&lt;result.length; x++)
 *         System.out.println(result[x]);
 * </pre></blockquote>
 * <p>
 * prints the following output:
 * <blockquote><pre>
 *     this
 *     is
 *     a
 *     test
 * </pre></blockquote>
 *
 * @version 1.35, 11/17/05
 * @see     java.io.StreamTokenizer
 * @since   JDK1.0
 */
    
class EStringTokenizer : public EObject {
public:
	/**
	 * @brief Creates a new StringTokenizer for the string <code>str</code>,
	 * that should split on the default delimiter set (space, tap,
	 * newline, return and formfeed), and which doesn't return the
	 * delimiters.
	 * @param str The string to split.
	 */
	EStringTokenizer(const char* str);

	/**
	 * @brief Create a new StringTokenizer, that splits the given string on
	 * the given delimiter characters.  If you set
	 * <code>returnDelims</code> to <code>true</code>, the delimiter
	 * characters are returned as tokens of their own.  The delimiter
	 * tokens always consist of a single character.
	 *
	 * @param str The string to split.
	 * @param delim A string containing all delimiter characters.
	 * @param returnDelims Tells, if you want to get the delimiters.
	 */
	EStringTokenizer(const char* str, const char* delim, boolean returnDelims =
			false);

	/**
	 * @brief Tells if there are more tokens.
	 * @return True, if the next call of nextToken() succeeds, false otherwise.
	 */
	boolean hasMoreTokens();

	/**
	 * @brief Returns the nextToken, changing the delimiter set to the given
	 * <code>delim</code>.  The change of the delimiter set is
	 * permanent, ie. the next call of nextToken(), uses the same
	 * delimiter set.
	 *
	 * @param delim a string containing the new delimiter characters.
	 * @return the next token with respect to the new delimiter characters.
	 * @exception NoSuchElementException if there are no more tokens.
	 */
	EString nextToken(const char* delim) THROWS(ENoSuchElementException);

	/**
	 * @brief Returns the nextToken of the string.
	 *
	 * @return the next token with respect to the new delimiter characters.
	 * @exception NoSuchElementException if there are no more tokens.
	 */
	EString nextToken() THROWS(ENoSuchElementException);

	/**
	 * @brief This counts the number of remaining tokens in the string, with
	 * respect to the current delimiter set.
	 *
	 * @return the number of times <code>nextTokens()</code> will
	 * succeed.  
	 * @see #nextToken
	 */
	int countTokens();

private:
	/** @brief The position in the str, where we currently are. */
	int currentPosition;
	int newPosition;
	int maxPosition;

	/** @brief The string that should be split into tokens. */
	const char *str; //only reference to the origin str address.

	/** @brief The string containing the delimiter characters. */
	EString delimiters;

	/** @brief Tells, if we should return the delimiters. */
	boolean retDelims;

	boolean delimsChanged;

	/*
	 * Temporarily store this position and use it in the following
	 * nextToken() method only if the delimiters haven't been changed in
	 * that nextToken() invocation.
	 */
	int skipDelimiters(int startPos);

	/**
	 * Skips ahead from startPos and returns the index of the next delimiter
	 * character encountered, or maxPosition if no such delimiter is found.
	 */
	int scanToken(int startPos);

	boolean isDelimiter(char codePoint);
};

} /* namespace efc */
#endif //!__ESTRINGTOKENIZER_HH__
