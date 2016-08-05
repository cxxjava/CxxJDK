/*
 * EMatcher.hh
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#ifndef EMATCHER_HH_
#define EMATCHER_HH_

#include "EObject.hh"
#include "EMatchResult.hh"
#include "EPattern.hh"
#include "EA.hh"

namespace efc {

class EMatcher : virtual public EMatchResult {
public:
	virtual ~EMatcher();

	/**
	 * No default constructor.
	 */
	EMatcher();

	/**
	 * All matchers have the state used by Pattern during a match.
	 */
	EMatcher(EPattern* parent, const char* text);

	//TODO:
	EMatcher(const EMatcher& that);
	EMatcher& operator= (const EMatcher& that);

	/**
	 * Returns the pattern that is interpreted by this matcher.
	 *
	 * @return  The pattern for which this matcher was created
	 */
	EPattern* pattern();

	/**
	 * Changes the <tt>Pattern</tt> that this <tt>Matcher</tt> uses to
	 * find matches with.
	 *
	 * <p> This method causes this matcher to lose information
	 * about the groups of the last match that occurred. The
	 * matcher's position in the input is maintained and its
	 * last append position is unaffected.</p>
	 *
	 * @param  newPattern
	 *         The new pattern used by this matcher
	 * @return  This matcher
	 * @throws  IllegalArgumentException
	 *          If newPattern is <tt>null</tt>
	 * @since 1.5
	 */
	EMatcher* usePattern(EPattern* newPattern);

	/**
	 * Resets this matcher.
	 *
	 * <p> Resetting a matcher discards all of its explicit state information
	 * and sets its append position to zero. The matcher's region is set to the
	 * default region, which is its entire character sequence. The anchoring
	 * and transparency of this matcher's region boundaries are unaffected.
	 *
	 * @return  This matcher
	 */
	EMatcher* reset();

	/**
	 * Resets this matcher with a new input sequence.
	 *
	 * <p> Resetting a matcher discards all of its explicit state information
	 * and sets its append position to zero.  The matcher's region is set to
	 * the default region, which is its entire character sequence.  The
	 * anchoring and transparency of this matcher's region boundaries are
	 * unaffected.
	 *
	 * @param  input
	 *         The new input character sequence
	 *
	 * @return  This matcher
	 */
	EMatcher* reset(const char* input);

	/**
	 * Returns the start index of the previous match.  </p>
	 *
	 * @return  The index of the first character matched
	 *
	 * @throws  IllegalStateException
	 *          If no match has yet been attempted,
	 *          or if the previous match operation failed
	 */
	int start();

	/**
	 * Returns the start index of the subsequence captured by the given group
	 * during the previous match operation.
	 *
	 * <p> <a href="Pattern.html#cg">Capturing groups</a> are indexed from left
	 * to right, starting at one.  Group zero denotes the entire pattern, so
	 * the expression <i>m.</i><tt>start(0)</tt> is equivalent to
	 * <i>m.</i><tt>start()</tt>.  </p>
	 *
	 * @param  group
	 *         The index of a capturing group in this matcher's pattern
	 *
	 * @return  The index of the first character captured by the group,
	 *          or <tt>-1</tt> if the match was successful but the group
	 *          itself did not match anything
	 *
	 * @throws  IllegalStateException
	 *          If no match has yet been attempted,
	 *          or if the previous match operation failed
	 *
	 * @throws  IndexOutOfBoundsException
	 *          If there is no capturing group in the pattern
	 *          with the given index
	 */
	int start(int group);

	/**
	 * Returns the offset after the last character matched.  </p>
	 *
	 * @return  The offset after the last character matched
	 *
	 * @throws  IllegalStateException
	 *          If no match has yet been attempted,
	 *          or if the previous match operation failed
	 */
	int end();

	/**
	 * Returns the offset after the last character of the subsequence
	 * captured by the given group during the previous match operation.
	 *
	 * <p> <a href="Pattern.html#cg">Capturing groups</a> are indexed from left
	 * to right, starting at one.  Group zero denotes the entire pattern, so
	 * the expression <i>m.</i><tt>end(0)</tt> is equivalent to
	 * <i>m.</i><tt>end()</tt>.  </p>
	 *
	 * @param  group
	 *         The index of a capturing group in this matcher's pattern
	 *
	 * @return  The offset after the last character captured by the group,
	 *          or <tt>-1</tt> if the match was successful
	 *          but the group itself did not match anything
	 *
	 * @throws  IllegalStateException
	 *          If no match has yet been attempted,
	 *          or if the previous match operation failed
	 *
	 * @throws  IndexOutOfBoundsException
	 *          If there is no capturing group in the pattern
	 *          with the given index
	 */
	int end(int group);

	/**
	 * Returns the input subsequence matched by the previous match.
	 *
	 * <p> For a matcher <i>m</i> with input sequence <i>s</i>,
	 * the expressions <i>m.</i><tt>group()</tt> and
	 * <i>s.</i><tt>substring(</tt><i>m.</i><tt>start(),</tt>&nbsp;<i>m.</i><tt>end())</tt>
	 * are equivalent.  </p>
	 *
	 * <p> Note that some patterns, for example <tt>a*</tt>, match the empty
	 * string.  This method will return the empty string when the pattern
	 * successfully matches the empty string in the input.  </p>
	 *
	 * @return The (possibly empty) subsequence matched by the previous match,
	 *         in string form
	 *
	 * @throws  IllegalStateException
	 *          If no match has yet been attempted,
	 *          or if the previous match operation failed
	 */
	EString group();

	/**
	 * Returns the input subsequence captured by the given group during the
	 * previous match operation.
	 *
	 * <p> For a matcher <i>m</i>, input sequence <i>s</i>, and group index
	 * <i>g</i>, the expressions <i>m.</i><tt>group(</tt><i>g</i><tt>)</tt> and
	 * <i>s.</i><tt>substring(</tt><i>m.</i><tt>start(</tt><i>g</i><tt>),</tt>&nbsp;<i>m.</i><tt>end(</tt><i>g</i><tt>))</tt>
	 * are equivalent.  </p>
	 *
	 * <p> <a href="Pattern.html#cg">Capturing groups</a> are indexed from left
	 * to right, starting at one.  Group zero denotes the entire pattern, so
	 * the expression <tt>m.group(0)</tt> is equivalent to <tt>m.group()</tt>.
	 * </p>
	 *
	 * <p> If the match was successful but the group specified failed to match
	 * any part of the input sequence, then <tt>null</tt> is returned. Note
	 * that some groups, for example <tt>(a*)</tt>, match the empty string.
	 * This method will return the empty string when such a group successfully
	 * matches the empty string in the input.  </p>
	 *
	 * @param  group
	 *         The index of a capturing group in this matcher's pattern
	 *
	 * @return  The (possibly empty) subsequence captured by the group
	 *          during the previous match, or <tt>null</tt> if the group
	 *          failed to match part of the input
	 *
	 * @throws  IllegalStateException
	 *          If no match has yet been attempted,
	 *          or if the previous match operation failed
	 *
	 * @throws  IndexOutOfBoundsException
	 *          If there is no capturing group in the pattern
	 *          with the given index
	 */
	EString group(int group);

	/**
	 * Returns the number of capturing groups in this matcher's pattern.
	 *
	 * <p> Group zero denotes the entire pattern by convention. It is not
	 * included in this count.
	 *
	 * <p> Any non-negative integer smaller than or equal to the value
	 * returned by this method is guaranteed to be a valid group index for
	 * this matcher.  </p>
	 *
	 * @return The number of capturing groups in this matcher's pattern
	 */
	int groupCount();

	/**
	 * Attempts to match the entire region against the pattern.
	 *
	 * <p> If the match succeeds then more information can be obtained via the
	 * <tt>start</tt>, <tt>end</tt>, and <tt>group</tt> methods.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if, the entire region sequence
	 *          matches this matcher's pattern
	 */
	boolean matches();

	/**
	 * Attempts to find the next subsequence of the input sequence that matches
	 * the pattern.
	 *
	 * <p> This method starts at the beginning of this matcher's region, or, if
	 * a previous invocation of the method was successful and the matcher has
	 * not since been reset, at the first character not matched by the previous
	 * match.
	 *
	 * <p> If the match succeeds then more information can be obtained via the
	 * <tt>start</tt>, <tt>end</tt>, and <tt>group</tt> methods.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if, a subsequence of the input
	 *          sequence matches this matcher's pattern
	 */
	boolean find();

private:
	/**
	 * The Pattern object that created this Matcher.
	 */
	EPattern* _parentPattern;

	/**
	 * The storage used by groups. They may contain invalid values if
	 * a group was skipped during the matching.
	 */
	EA<int>* _groups;

	/**
	 * The original string being matched.
	 */
	const char* _text;

	int _offset;
	int _gcount;
};

} /* namespace efc */
#endif /* EMATCHER_HH_ */
