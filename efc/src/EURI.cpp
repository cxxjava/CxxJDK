/*
 * EURI.cpp
 *
 *  Created on: 2016-6-14
 *      Author: cxxjava@163.com
 */

#include "EURI.hh"
//#include "EURL.hh"
#include "EA.hh"
#include "ESystem.hh"
#include "EInteger.hh"
#include "EPattern.hh"
#include "EHashMap.hh"
#include "EURLDecoder.hh"
#include "ESharedPtr.hh"
#include "ERuntimeException.hh"
#include "ENumberFormatException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

static EString c_normalize(EString& path);

// -- Utility methods for string-field comparison and hashing --

// These methods return appropriate values for null string arguments,
// thereby simplifying the equals, hashCode, and compareTo methods.
//
// The case-ignoring methods should only be applied to strings whose
// characters are all known to be US-ASCII.  Because of this restriction,
// these methods are faster than the similar methods in the String class.

// US-ASCII only
static int toLower(char c) {
	if ((c >= 'A') && (c <= 'Z'))
		return c + ('a' - 'A');
	return c;
}

// US-ASCII only
static int toUpper(char c) {
	if ((c >= 'a') && (c <= 'z'))
		return c - ('a' - 'A');
	return c;
}

static boolean equal(EString* s, EString* t) {
	if (s == t) return true;
	if ((s != null) && (t != null)) {
		if (s->length() != t->length())
			return false;
		if (s->indexOf('%') < 0)
			return s->equals(t);
		int n = s->length();
		for (int i = 0; i < n;) {
			char c = s->charAt(i);
			char d = t->charAt(i);
			if (c != '%') {
				if (c != d)
					return false;
				i++;
				continue;
			}
			if (d != '%')
				return false;
			i++;
			if (toLower(s->charAt(i)) != toLower(t->charAt(i)))
				return false;
			i++;
			if (toLower(s->charAt(i)) != toLower(t->charAt(i)))
				return false;
			i++;
		}
		return true;
	}
	return false;
}

// US-ASCII only
static boolean equalIgnoringCase(EString* s, EString* t) {
	if (s == t) return true;
	if ((s != null) && (t != null)) {
		int n = s->length();
		if (t->length() != n)
			return false;
		for (int i = 0; i < n; i++) {
			if (toLower(s->charAt(i)) != toLower(t->charAt(i)))
				return false;
		}
		return true;
	}
	return false;
}

static int normalizedHash(int hash, EString* s) {
	int h = 0;
	for (int index = 0; index < s->length(); index++) {
		char ch = s->charAt(index);
		h = 31 * h + ch;
		if (ch == '%') {
			/*
			 * Process the next two encoded characters
			 */
			for (int i = index + 1; i < index + 3; i++)
				h = 31 * h + toUpper(s->charAt(i));
			index += 2;
		}
	}
	return hash * 127 + h;
}

static int hashit(int hash, EString* s) {
	if (s == null) return hash;
	return s->indexOf('%') < 0 ? hash * 127 + s->hashCode()
							  : normalizedHash(hash, s);
}


// US-ASCII only
static int hashIgnoringCase(int hash, EString* s) {
	if (s == null) return hash;
	int h = hash;
	int n = s->length();
	for (int i = 0; i < n; i++)
		h = 31 * h + toLower(s->charAt(i));
	return h;
}

static int compare(EString* s, EString* t) {
	if (s == t) return 0;
	if (s != null) {
		if (t != null)
			return s->compareTo(t);
		else
			return +1;
	} else {
		return -1;
	}
}

// US-ASCII only
static int compareIgnoringCase(EString* s, EString* t) {
	if (s == t) return 0;
	if (s != null) {
		if (t != null) {
			int sn = s->length();
			int tn = t->length();
			int n = sn < tn ? sn : tn;
			for (int i = 0; i < n; i++) {
				int c = toLower(s->charAt(i)) - toLower(t->charAt(i));
				if (c != 0)
					return c;
			}
			return sn - tn;
		}
		return +1;
	} else {
		return -1;
	}
}


// -- String construction --

// If a scheme is given then the path, if given, must be absolute
//
static void checkPath(EString& s, const char* scheme, const char* path)
{
	if (scheme != null) {
		if ((path != null)
			&& ((eso_strlen(path) > 0) && (*path != '/')))
			throw EURISyntaxException(__FILE__, __LINE__, s.c_str(),
										 "Relative path in absolute URI");
	}
}

// -- Normalization, resolution, and relativization --

// RFC2396 5.2 (6)
static EString resolvePath(EString* base, EString* child,
								  boolean absolute)
{
	ES_ASSERT(base);
	ES_ASSERT(child);

	int i = base->lastIndexOf('/');
	int cn = child->length();
	EString path("");

	if (cn == 0) {
		// 5.2 (6a)
		if (i >= 0)
			path = base->substring(0, i + 1);
	} else {
		path.append((int)(base->length() + cn));
		// 5.2 (6a)
		if (i >= 0)
			path.append(base->substring(0, i + 1));
		// 5.2 (6b)
		path.append(*child);
	}

	// 5.2 (6c-f)
	EString np = c_normalize(path);

	// 5.2 (6g): If the result is absolute but the path begins with "../",
	// then we simply leave the path as-is

	return np;
}


// -- Path normalization --

// The following algorithm for path normalization avoids the creation of a
// string object for each segment, as well as the use of a string buffer to
// compute the final result, by using a single char array and editing it in
// place.  The array is first split into segments, replacing each slash
// with '\0' and creating a segment-index array, each element of which is
// the index of the first char in the corresponding segment.  We then walk
// through both arrays, removing ".", "..", and other segments as necessary
// by setting their entries in the index array to -1.  Finally, the two
// arrays are used to rejoin the segments and compute the final result.
//
// This code is based upon src/solaris/native/java/io/canonicalize_md.c


// Check the given path to see if it might need normalization.  A path
// might need normalization if it contains duplicate slashes, a "."
// segment, or a ".." segment.  Return -1 if no further normalization is
// possible, otherwise return the number of segments found.
//
// This method takes a string argument rather than a char array so that
// this test can be performed without invoking path.toCharArray().
//
static int needsNormalization(EString& path) {
	boolean normal = true;
	int ns = 0;                     // Number of segments
	int end = path.length() - 1;    // Index of last char in path
	int p = 0;                      // Index of next char in path

	// Skip initial slashes
	while (p <= end) {
		if (path.charAt(p) != '/') break;
		p++;
	}
	if (p > 1) normal = false;

	// Scan segments
	while (p <= end) {

		// Looking at "." or ".." ?
		if ((path.charAt(p) == '.')
			&& ((p == end)
				|| ((path.charAt(p + 1) == '/')
					|| ((path.charAt(p + 1) == '.')
						&& ((p + 1 == end)
							|| (path.charAt(p + 2) == '/')))))) {
			normal = false;
		}
		ns++;

		// Find beginning of next segment
		while (p <= end) {
			if (path.charAt(p++) != '/')
				continue;

			// Skip redundant slashes
			while (p <= end) {
				if (path.charAt(p) != '/') break;
				normal = false;
				p++;
			}

			break;
		}
	}

	return normal ? -1 : ns;
}


// Split the given path into segments, replacing slashes with nulls and
// filling in the given segment-index array.
//
// Preconditions:
//   segs.length == Number of segments in path
//
// Postconditions:
//   All slashes in path replaced by '\0'
//   segs[i] == Index of first char in segment i (0 <= i < segs.length)
//
static void split(EA<char>& path, EA<int>& segs) {
	int end = path.length() - 1;      // Index of last char in path
	int p = 0;                      // Index of next char in path
	int i = 0;                      // Index of current segment

	// Skip initial slashes
	while (p <= end) {
		if (path[p] != '/') break;
		path[p] = '\0';
		p++;
	}

	while (p <= end) {

		// Note start of segment
		segs[i++] = p++;

		// Find beginning of next segment
		while (p <= end) {
			if (path[p++] != '/')
				continue;
			path[p - 1] = '\0';

			// Skip redundant slashes
			while (p <= end) {
				if (path[p] != '/') break;
				path[p++] = '\0';
			}
			break;
		}
	}

	if (i != segs.length())
		throw ERuntimeException(__FILE__, __LINE__);  // ASSERT
}


// Join the segments in the given path according to the given segment-index
// array, ignoring those segments whose index entries have been set to -1,
// and inserting slashes as needed.  Return the length of the resulting
// path.
//
// Preconditions:
//   segs[i] == -1 implies segment i is to be ignored
//   path computed by split, as above, with '\0' having replaced '/'
//
// Postconditions:
//   path[0] .. path[return value] == Resulting path
//
static int join(EA<char>& path, EA<int>& segs) {
	int ns = segs.length();           // Number of segments
	int end = path.length() - 1;      // Index of last char in path
	int p = 0;                      // Index of next path char to write

	if (path[p] == '\0') {
		// Restore initial slash for absolute paths
		path[p++] = '/';
	}

	for (int i = 0; i < ns; i++) {
		int q = segs[i];            // Current segment
		if (q == -1)
			// Ignore this segment
			continue;

		if (p == q) {
			// We're already at this segment, so just skip to its end
			while ((p <= end) && (path[p] != '\0'))
				p++;
			if (p <= end) {
				// Preserve trailing slash
				path[p++] = '/';
			}
		} else if (p < q) {
			// Copy q down to p
			while ((q <= end) && (path[q] != '\0'))
				path[p++] = path[q++];
			if (q <= end) {
				// Preserve trailing slash
				path[p++] = '/';
			}
		} else
			throw ERuntimeException(__FILE__, __LINE__);  // ASSERT false
	}

	return p;
}


// Remove "." segments from the given path, and remove segment pairs
// consisting of a non-".." segment followed by a ".." segment.
//
static void removeDots(EA<char>& path, EA<int>& segs) {
	int ns = segs.length();
	int end = path.length() - 1;

	for (int i = 0; i < ns; i++) {
		int dots = 0;               // Number of dots found (0, 1, or 2)

		// Find next occurrence of "." or ".."
		do {
			int p = segs[i];
			if (path[p] == '.') {
				if (p == end) {
					dots = 1;
					break;
				} else if (path[p + 1] == '\0') {
					dots = 1;
					break;
				} else if ((path[p + 1] == '.')
						   && ((p + 1 == end)
							   || (path[p + 2] == '\0'))) {
					dots = 2;
					break;
				}
			}
			i++;
		} while (i < ns);
		if ((i > ns) || (dots == 0))
			break;

		if (dots == 1) {
			// Remove this occurrence of "."
			segs[i] = -1;
		} else {
			// If there is a preceding non-".." segment, remove both that
			// segment and this occurrence of ".."; otherwise, leave this
			// ".." segment as-is.
			int j;
			for (j = i - 1; j >= 0; j--) {
				if (segs[j] != -1) break;
			}
			if (j >= 0) {
				int q = segs[j];
				if (!((path[q] == '.')
					  && (path[q + 1] == '.')
					  && (path[q + 2] == '\0'))) {
					segs[i] = -1;
					segs[j] = -1;
				}
			}
		}
	}
}


// DEVIATION: If the normalized path is relative, and if the first
// segment could be parsed as a scheme name, then prepend a "." segment
//
static void maybeAddLeadingDot(EA<char>& path, EA<int>& segs) {

	if (path[0] == '\0')
		// The path is absolute
		return;

	int ns = segs.length();
	int f = 0;                      // Index of first segment
	while (f < ns) {
		if (segs[f] >= 0)
			break;
		f++;
	}
	if ((f >= ns) || (f == 0))
		// The path is empty, or else the original first segment survived,
		// in which case we already know that no leading "." is needed
		return;

	int p = segs[f];
	while ((p < path.length()) && (path[p] != ':') && (path[p] != '\0')) p++;
	if (p >= path.length() || path[p] == '\0')
		// No colon in first segment, so no "." needed
		return;

	// At this point we know that the first segment is unused,
	// hence we can insert a "." segment at that position
	path[0] = '.';
	path[1] = '\0';
	segs[0] = 0;
}


// Normalize the given path string.  A normal path string has no empty
// segments (i.e., occurrences of "//"), no segments equal to ".", and no
// segments equal to ".." that are preceded by a segment not equal to "..".
// In contrast to Unix-style pathname normalization, for URI paths we
// always retain trailing slashes.
//
static EString c_normalize(EString& ps) {
	// Does this path need normalization?
	int ns = needsNormalization(ps);        // Number of segments
	if (ns < 0)
		// Nope -- just return it
		return ps;

	int length = ps.length();
	EA<char> path(length);         // Path in char-array form
	ESystem::arraycopy((void*)ps.c_str(), 0, path.address(), 0, length);

	// Split path into segments
	EA<int> segs(ns);               // Segment-index array
	split(path, segs);

	// Remove dots
	removeDots(path, segs);

	// Prevent scheme-name confusion
	maybeAddLeadingDot(path, segs);

	// Join the remaining segments and return the result
	EString s(path.address(), 0, join(path, segs));
	if (s.equals(ps)) {
		// string was already normalized
		return ps;
	}
	return s;
}



// -- Character classes for parsing --

// RFC2396 precisely specifies which characters in the US-ASCII charset are
// permissible in the various components of a URI reference.  We here
// define a set of mask pairs to aid in enforcing these restrictions.  Each
// mask pair consists of two longs, a low mask and a high mask.  Taken
// together they represent a 128-bit mask, where bit i is set iff the
// character with value i is permitted.
//
// This approach is more efficient than sequentially searching arrays of
// permitted characters.  It could be made still more efficient by
// precompiling the mask information so that a character's presence in a
// given mask could be determined by a single table lookup.

// Compute the low-order mask for the characters in the given string
static llong lowMask(const char* s) {
	EString chars(s);
	int n = chars.length();
	llong m = 0;
	for (int i = 0; i < n; i++) {
		char c = chars.charAt(i);
		if (c < 64)
			m |= (1L << c);
	}
	return m;
}

// Compute the high-order mask for the characters in the given string
static llong highMask(const char* s) {
	EString chars(s);
	int n = chars.length();
	llong m = 0;
	for (int i = 0; i < n; i++) {
		char c = chars.charAt(i);
		if ((c >= 64) && (c < 128))
			m |= (1L << (c - 64));
	}
	return m;
}

// Compute a low-order mask for the characters
// between first and last, inclusive
static llong lowMask(char first, char last) {
	llong m = 0;
	int f = ES_MAX(ES_MIN(first, 63), 0);
	int l = ES_MAX(ES_MIN(last, 63), 0);
	for (int i = f; i <= l; i++)
		m |= 1L << i;
	return m;
}

// Compute a high-order mask for the characters
// between first and last, inclusive
static llong highMask(char first, char last) {
	llong m = 0;
	int f = ES_MAX(ES_MIN(first, 127), 64) - 64;
	int l = ES_MAX(ES_MIN(last, 127), 64) - 64;
	for (int i = f; i <= l; i++)
		m |= 1L << i;
	return m;
}

// Tell whether the given character is permitted by the given mask pair
static boolean match(char c, llong lowMask, llong highMask) {
	if (c == 0) // 0 doesn't have a slot in the mask. So, it never matches.
		return false;
	if (c < 64)
		return ((1L << c) & lowMask) != 0;
	if (c < 128)
		return ((1L << (c - 64)) & highMask) != 0;
	return false;
}

// Character-class masks, in reverse order from RFC2396 because
// initializers for static fields cannot make forward references.

// digit    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
//            "8" | "9"
static llong L_DIGIT = lowMask('0', '9');
static llong H_DIGIT = 0L;

// upalpha  = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
//            "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
//            "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
static llong L_UPALPHA = 0L;
static llong H_UPALPHA = highMask('A', 'Z');

// lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
//            "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
//            "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
static llong L_LOWALPHA = 0L;
static llong H_LOWALPHA = highMask('a', 'z');

// alpha         = lowalpha | upalpha
static llong L_ALPHA = L_LOWALPHA | L_UPALPHA;
static llong H_ALPHA = H_LOWALPHA | H_UPALPHA;

// alphanum      = alpha | digit
static llong L_ALPHANUM = L_DIGIT | L_ALPHA;
static llong H_ALPHANUM = H_DIGIT | H_ALPHA;

// hex           = digit | "A" | "B" | "C" | "D" | "E" | "F" |
//                         "a" | "b" | "c" | "d" | "e" | "f"
static llong L_HEX = L_DIGIT;
static llong H_HEX = highMask('A', 'F') | highMask('a', 'f');

// mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" |
//                 "(" | ")"
static llong L_MARK = lowMask("-_.!~*'()");
static llong H_MARK = highMask("-_.!~*'()");

// unreserved    = alphanum | mark
static llong L_UNRESERVED = L_ALPHANUM | L_MARK;
static llong H_UNRESERVED = H_ALPHANUM | H_MARK;

// reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
//                 "$" | "," | "[" | "]"
// Added per RFC2732: "[", "]"
static llong L_RESERVED = lowMask(";/?:@&=+$,[]");
static llong H_RESERVED = highMask(";/?:@&=+$,[]");

// The zero'th bit is used to indicate that escape pairs and non-US-ASCII
// characters are allowed; this is handled by the scanEscape method below.
static llong L_ESCAPED = 1L;
static llong H_ESCAPED = 0L;

// uric          = reserved | unreserved | escaped
static llong L_URIC = L_RESERVED | L_UNRESERVED | L_ESCAPED;
static llong H_URIC = H_RESERVED | H_UNRESERVED | H_ESCAPED;

// pchar         = unreserved | escaped |
//                 ":" | "@" | "&" | "=" | "+" | "$" | ","
static llong L_PCHAR
	= L_UNRESERVED | L_ESCAPED | lowMask(":@&=+$,");
static llong H_PCHAR
	= H_UNRESERVED | H_ESCAPED | highMask(":@&=+$,");

// All valid path characters
static llong L_PATH = L_PCHAR | lowMask(";/");
static llong H_PATH = H_PCHAR | highMask(";/");

// Dash, for use in domainlabel and toplabel
static llong L_DASH = lowMask("-");
static llong H_DASH = highMask("-");

// Dot, for use in hostnames
static llong L_DOT = lowMask(".");
static llong H_DOT = highMask(".");

// userinfo      = *( unreserved | escaped |
//                    ";" | ":" | "&" | "=" | "+" | "$" | "," )
static llong L_USERINFO
	= L_UNRESERVED | L_ESCAPED | lowMask(";:&=+$,");
static llong H_USERINFO
	= H_UNRESERVED | H_ESCAPED | highMask(";:&=+$,");

// reg_name      = 1*( unreserved | escaped | "$" | "," |
//                     ";" | ":" | "@" | "&" | "=" | "+" )
static llong L_REG_NAME
	= L_UNRESERVED | L_ESCAPED | lowMask("$,;:@&=+");
static llong H_REG_NAME
	= H_UNRESERVED | H_ESCAPED | highMask("$,;:@&=+");

// All valid characters for server-based authorities
static llong L_SERVER
	= L_USERINFO | L_ALPHANUM | L_DASH | lowMask(".:@[]");
static llong H_SERVER
	= H_USERINFO | H_ALPHANUM | H_DASH | highMask(".:@[]");

// Special case of server authority that represents an IPv6 address
// In this case, a % does not signify an escape sequence
static llong L_SERVER_PERCENT
	= L_SERVER | lowMask("%");
static llong H_SERVER_PERCENT
	= H_SERVER | highMask("%");
static llong L_LEFT_BRACKET = lowMask("[");
static llong H_LEFT_BRACKET = highMask("[");

// scheme        = alpha *( alpha | digit | "+" | "-" | "." )
static llong L_SCHEME = L_ALPHA | L_DIGIT | lowMask("+-.");
static llong H_SCHEME = H_ALPHA | H_DIGIT | highMask("+-.");

// uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" |
//                 "&" | "=" | "+" | "$" | ","
static llong L_URIC_NO_SLASH
	= L_UNRESERVED | L_ESCAPED | lowMask(";?:@&=+$,");
static llong H_URIC_NO_SLASH
	= H_UNRESERVED | H_ESCAPED | highMask(";?:@&=+$,");


// -- Escaping and encoding --

static char hexDigits[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static void appendEscape(EString& sb, byte b) {
	sb.append('%');
	sb.append(hexDigits[(b >> 4) & 0x0f]);
	sb.append(hexDigits[(b >> 0) & 0x0f]);
}

static void appendEncoded(EString& sb, char c) {
	int b = c & 0xff;
	if (b >= 0x80)
		appendEscape(sb, (byte)b);
	else
		sb.append((char)b);
}

// Quote any characters in s that are not permitted
// by the given mask pair
//
static EString quote(EString& s, llong lowMask, llong highMask) {
	int n = s.length();
	EString sb;
	boolean allowNonASCII = ((lowMask & L_ESCAPED) != 0);
	for (int i = 0; i < s.length(); i++) {
		char c = s.charAt(i);
		if (c < 0x80) {
			if (!match(c, lowMask, highMask)) {
				sb.append(s.substring(0, i));
				appendEscape(sb, (byte)c);
			} else {
				sb.append(c);
			}
		} else if (allowNonASCII
				   && (eso_isspace(c)
					   || eso_iscntrl(c))) {
			sb.append(s.substring(0, i));
			appendEncoded(sb, c);
		} else {
			sb.append(c);
		}
	}
	return (sb.isEmpty()) ? s : sb;
}

// Encodes all characters >= \u0080 into escaped, normalized UTF-8 octets,
// assuming that s is otherwise legal
//
static EString encode(EString& s) {
	int n = s.length();
	if (n == 0)
		return s;

	// First check whether we actually need to encode
	for (int i = 0;;) {
		if (s.charAt(i) >= 0x80)
			break;
		if (++i >= n)
			return s;
	}

	EString sb;
	for (int i=0; i<n; i++) {
		int b = s.charAt(i) & 0xff;
		if (b >= 0x80)
			appendEscape(sb, (byte)b);
		else
			sb.append((char)b);
	}
	return sb;
}

static int decode(char c) {
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	ES_ASSERT(false);
	return -1;
}

static byte decode(char c1, char c2) {
	return (byte)(  ((decode(c1) & 0xf) << 4)
				  | ((decode(c2) & 0xf) << 0));
}

// Evaluates all escapes in s, applying UTF-8 decoding if needed.  Assumes
// that escapes are well-formed syntactically, i.e., of the form %XX.  If a
// sequence of escaped octets is not valid UTF-8 then the erroneous octets
// are replaced with '\uFFFD'.
// Exception: any "%" found between "[]" is left alone. It is an IPv6 literal
//            with a scope_id
//
static EString decode(EString* s) {
	ES_ASSERT(s);
	int n = s->length();
	if (n == 0)
		return *s;
	if (s->indexOf('%') < 0)
		return *s;

	// This is not horribly efficient, but it will do for now
	char c = s->charAt(0);
	boolean betweenBrackets = false;

	EString sb;
	for (int i = 0; i < n;) {
		ES_ASSERT(c == s->charAt(i));    // Loop invariant
		if (c == '[') {
			betweenBrackets = true;
		} else if (betweenBrackets && c == ']') {
			betweenBrackets = false;
		}
		if (c != '%' || betweenBrackets) {
			sb.append(c);
			if (++i >= n)
				break;
			c = s->charAt(i);
			continue;
		}
		int ui = i;
		EString bb;
		for (;;) {
			ES_ASSERT (n - i >= 2);
			bb << decode(s->charAt(++i), s->charAt(++i));
			if (++i >= n)
				break;
			c = s->charAt(i);
			if (c != '%')
				break;
		}
		sb.append(bb);
	}

	return sb;
}


EURI::~EURI() {
	delete scheme;
	delete fragment;

	delete authority;

	delete userInfo;
	delete host;

	delete path;
	delete query;

	delete schemeSpecificPart;

	delete decodedUserInfo;
	delete decodedAuthority;
	delete decodedPath;
	delete decodedQuery;
	delete decodedFragment;
	delete decodedSchemeSpecificPart;

	delete string;
}

EURI::EURI(const EURI& that):
		scheme(null),
		fragment(null),
		authority(null),
		userInfo(null),
		host(null),
		port(-1),
		path(null),
		query(null),
		schemeSpecificPart(null),
		hash(0),
		decodedUserInfo(null),
		decodedAuthority(null),
		decodedPath(null),
		decodedQuery(null),
		decodedFragment(null),
		decodedSchemeSpecificPart(null),
		string(null) {
	EURI* t = (EURI*)&that;

	if (t->scheme) scheme = new EString(t->scheme);
	if (t->fragment) fragment = new EString(t->fragment);
	if (t->authority) authority = new EString(t->authority);
	if (t->userInfo) userInfo = new EString(t->userInfo);
	if (t->host) host = new EString(t->host);
	port = t->port;
	if (t->path) path = new EString(t->path);
	if (t->query) query = new EString(t->query);
	if (t->schemeSpecificPart) schemeSpecificPart = new EString(t->schemeSpecificPart);
	hash = t->hash;
	if (t->decodedUserInfo) decodedUserInfo = new EString(t->decodedUserInfo);
	if (t->decodedAuthority) decodedAuthority = new EString(t->decodedAuthority);
	if (t->decodedPath) decodedPath = new EString(t->decodedPath);
	if (t->decodedQuery) decodedQuery = new EString(t->decodedQuery);
	if (t->decodedFragment) decodedFragment = new EString(t->decodedFragment);
	if (t->decodedSchemeSpecificPart) decodedSchemeSpecificPart = new EString(t->decodedSchemeSpecificPart);
	if (t->string) string = new EString(t->string);
}

EURI& EURI::operator= (const EURI& that) {
	if (this == &that) return *this;

	EURI* t = (EURI*)&that;

	//free old
	delete scheme;
	delete fragment;
	delete authority;
	delete userInfo;
	delete host;
	delete path;
	delete query;
	delete schemeSpecificPart;
	delete decodedUserInfo;
	delete decodedAuthority;
	delete decodedPath;
	delete decodedQuery;
	delete decodedFragment;
	delete decodedSchemeSpecificPart;
	delete string;

	//clone new
	if (t->scheme) scheme = new EString(t->scheme);
	if (t->fragment) fragment = new EString(t->fragment);
	if (t->authority) authority = new EString(t->authority);
	if (t->userInfo) userInfo = new EString(t->userInfo);
	if (t->host) host = new EString(t->host);
	port = t->port;
	if (t->path) path = new EString(t->path);
	if (t->query) query = new EString(t->query);
	if (t->schemeSpecificPart) schemeSpecificPart = new EString(t->schemeSpecificPart);
	hash = t->hash;
	if (t->decodedUserInfo) decodedUserInfo = new EString(t->decodedUserInfo);
	if (t->decodedAuthority) decodedAuthority = new EString(t->decodedAuthority);
	if (t->decodedPath) decodedPath = new EString(t->decodedPath);
	if (t->decodedQuery) decodedQuery = new EString(t->decodedQuery);
	if (t->decodedFragment) decodedFragment = new EString(t->decodedFragment);
	if (t->decodedSchemeSpecificPart) decodedSchemeSpecificPart = new EString(t->decodedSchemeSpecificPart);
	if (t->string) string = new EString(t->string);

	return *this;
}

EURI::EURI():
		scheme(null),
		fragment(null),
		authority(null),
		userInfo(null),
		host(null),
		port(-1),
		path(null),
		query(null),
		schemeSpecificPart(null),
		hash(0),
		decodedUserInfo(null),
		decodedAuthority(null),
		decodedPath(null),
		decodedQuery(null),
		decodedFragment(null),
		decodedSchemeSpecificPart(null),
		string(null) {
	//
}

EURI::EURI(const char* str):
		scheme(null),
		fragment(null),
		authority(null),
		userInfo(null),
		host(null),
		port(-1),
		path(null),
		query(null),
		schemeSpecificPart(null),
		hash(0),
		decodedUserInfo(null),
		decodedAuthority(null),
		decodedPath(null),
		decodedQuery(null),
		decodedFragment(null),
		decodedSchemeSpecificPart(null),
		string(null) {
	EString s(str);
	this->parseURI(s, false);
}

EURI::EURI(const char* scheme,
			const char* userInfo, const char* host, int port,
			const char* path, const char* query, const char* fragment):
		scheme(null),
		fragment(null),
		authority(null),
		userInfo(null),
		host(null),
		port(-1),
		path(null),
		query(null),
		schemeSpecificPart(null),
		hash(0),
		decodedUserInfo(null),
		decodedAuthority(null),
		decodedPath(null),
		decodedQuery(null),
		decodedFragment(null),
		decodedSchemeSpecificPart(null),
		string(null) {
	EString s = toString(scheme, null,
						null, userInfo, host, port,
						path, query, fragment);
	checkPath(s, scheme, path);
	this->parseURI(s, true);
}

EURI::EURI(const char* scheme,
			const char* authority,
			const char* path, const char* query, const char* fragment):
		scheme(null),
		fragment(null),
		authority(null),
		userInfo(null),
		host(null),
		port(-1),
		path(null),
		query(null),
		schemeSpecificPart(null),
		hash(0),
		decodedUserInfo(null),
		decodedAuthority(null),
		decodedPath(null),
		decodedQuery(null),
		decodedFragment(null),
		decodedSchemeSpecificPart(null),
		string(null) {
	EString s = toString(scheme, null,
						authority, null, null, -1,
						path, query, fragment);
	checkPath(s, scheme, path);
	this->parseURI(s, false);
}

EURI::EURI(const char* scheme, const char* host,
		const char* path, const char* fragment):
		scheme(null),
		fragment(null),
		authority(null),
		userInfo(null),
		host(null),
		port(-1),
		path(null),
		query(null),
		schemeSpecificPart(null),
		hash(0),
		decodedUserInfo(null),
		decodedAuthority(null),
		decodedPath(null),
		decodedQuery(null),
		decodedFragment(null),
		decodedSchemeSpecificPart(null),
		string(null) {
	//@see: this(scheme, null, host, -1, path, null, fragment);
	EString s = toString(scheme, null,
						null, null, host, -1,
						path, null, fragment);
	checkPath(s, scheme, path);
	this->parseURI(s, true);
}

EURI::EURI(const char* scheme, const char* ssp, const char* fragment):
		scheme(null),
		fragment(null),
		authority(null),
		userInfo(null),
		host(null),
		port(-1),
		path(null),
		query(null),
		schemeSpecificPart(null),
		hash(0),
		decodedUserInfo(null),
		decodedAuthority(null),
		decodedPath(null),
		decodedQuery(null),
		decodedFragment(null),
		decodedSchemeSpecificPart(null),
		string(null) {
	EString s = toString(scheme, ssp,
			null, null, null, -1,
			null, null, fragment);
	this->parseURI(s, false);
}

EURI* EURI::create(const char* str) {
	try {
		return new EURI(str);
	} catch (EURISyntaxException& x) {
		throw EIllegalArgumentException(__FILE__, __LINE__, x.getMessage(), &x);
	}
}

EURI* EURI::parseServerAuthority() {
	// We could be clever and cache the error message and index from the
	// exception thrown during the original parse, but that would require
	// either more fields or a more-obscure representation.
	if ((host != null) || (authority == null))
		return this;
	defineString();
	this->parseURI(*string, true);
	return this;
}

sp<EURI> EURI::normalize() {
   return normalize(this);
}

sp<EURI> EURI::resolve(EURI* uri) {
	return resolve(this, uri);
}

sp<EURI> EURI::resolve(const char* str) {
	return resolve(EURI::create(str));
}

sp<EURI> EURI::relativize(EURI* uri) {
	return relativize(this, uri);
}

//EURL* EURI::toURL() {
//	if (!isAbsolute())
//		throw new IllegalArgumentException("URI is not absolute");
//	return new URL(toString());
//}

const char* EURI::getScheme() {
	return scheme ? scheme->c_str() : null;
}

boolean EURI::isAbsolute() {
	return scheme != null;
}

boolean EURI::isOpaque() {
	return path == null;
}

const char* EURI::getRawSchemeSpecificPart() {
	defineSchemeSpecificPart();
	EString* s = schemeSpecificPart;
	return s ? s->c_str() : null;
}

const char* EURI::getSchemeSpecificPart() {
	if (decodedSchemeSpecificPart == null) {
		defineSchemeSpecificPart();
		if (schemeSpecificPart) {
			decodedSchemeSpecificPart = new EString(decode(schemeSpecificPart));
		}
	}
	EString* s = decodedSchemeSpecificPart;
	return s ? s->c_str() : null;
}

const char* EURI::getRawAuthority() {
	return authority ? authority->c_str() : null;
}

const char* EURI::getAuthority() {
	if (decodedAuthority == null && authority != null) {
		decodedAuthority = new EString(decode(authority));
	}
	EString* s = decodedAuthority;
	return s ? s->c_str() : null;
}

const char* EURI::getRawUserInfo() {
	return userInfo ? userInfo->c_str() : null;
}

const char* EURI::getUserInfo() {
	if ((decodedUserInfo == null) && (userInfo != null)) {
		decodedUserInfo = new EString(decode(userInfo));
	}
	EString* s = decodedUserInfo;
	return s ? s->c_str() : null;
}

const char* EURI::getHost() {
	return host ? host->c_str() : null;
}

int EURI::getPort() {
	return port;
}

const char* EURI::getRawPath() {
	return path ? path->c_str() : null;
}

const char* EURI::getPath() {
	if ((decodedPath == null) && (path != null)) {
		decodedPath = new EString(decode(path));
	}
	EString* s = decodedPath;
	return s ? s->c_str() : null;
}

const char* EURI::getRawQuery() {
	return query ? query->c_str() : null;
}

const char* EURI::getQuery() {
	if ((decodedQuery == null) && (query != null)) {
		decodedQuery = new EString(decode(query));
	}
	EString* s = decodedQuery;
	return s ? s->c_str() : null;
}

const char* EURI::getRawFragment() {
	return fragment ? fragment->c_str() : null;
}

const char* EURI::getFragment() {
	if ((decodedFragment == null) && (fragment != null)) {
		decodedFragment = new EString(decode(fragment));
	}
	EString* s = decodedFragment;
	return s ? s->c_str() : null;
}

boolean EURI::equals(EObject* ob) {
	if (ob == this)
		return true;
	if (!(instanceof<EURI>(ob)))
		return false;
	EURI* that = dynamic_cast<EURI*>(ob);
	if (this->isOpaque() != that->isOpaque()) return false;
	if (!equalIgnoringCase(this->scheme, that->scheme)) return false;
	if (!equal(this->fragment, that->fragment)) return false;

	// Opaque
	if (this->isOpaque())
		return equal(this->schemeSpecificPart, that->schemeSpecificPart);

	// Hierarchical
	if (!equal(this->path, that->path)) return false;
	if (!equal(this->query, that->query)) return false;

	// Authorities
	if (this->authority == that->authority) return true;
	if (this->host != null) {
		// Server-based
		if (!equal(this->userInfo, that->userInfo)) return false;
		if (!equalIgnoringCase(this->host, that->host)) return false;
		if (this->port != that->port) return false;
	} else if (this->authority != null) {
		// Registry-based
		if (!equal(this->authority, that->authority)) return false;
	} else if (this->authority != that->authority) {
		return false;
	}

	return true;
}

int EURI::hashCode() {
	if (hash != 0)
		return hash;
	int h = hashIgnoringCase(0, scheme);
	h = hashit(h, fragment);
	if (isOpaque()) {
		h = hashit(h, schemeSpecificPart);
	} else {
		h = hashit(h, path);
		h = hashit(h, query);
		if (host != null) {
			h = hashit(h, userInfo);
			h = hashIgnoringCase(h, host);
			h += 1949 * port;
		} else {
			h = hashit(h, authority);
		}
	}
	hash = h;
	return h;
}

int EURI::compareTo(EURI* that) {
	int c;

	if ((c = compareIgnoringCase(this->scheme, that->scheme)) != 0)
		return c;

	if (this->isOpaque()) {
		if (that->isOpaque()) {
			// Both opaque
			if ((c = compare(this->schemeSpecificPart,
					that->schemeSpecificPart)) != 0)
				return c;
			return compare(this->fragment, that->fragment);
		}
		return +1;                  // Opaque > hierarchical
	} else if (that->isOpaque()) {
		return -1;                  // Hierarchical < opaque
	}

	// Hierarchical
	if ((this->host != null) && (that->host != null)) {
		// Both server-based
		if ((c = compare(this->userInfo, that->userInfo)) != 0)
			return c;
		if ((c = compareIgnoringCase(this->host, that->host)) != 0)
			return c;
		if ((c = this->port - that->port) != 0)
			return c;
	} else {
		// If one or both authorities are registry-based then we simply
		// compare them in the usual, case-sensitive way.  If one is
		// registry-based and one is server-based then the strings are
		// guaranteed to be unequal, hence the comparison will never return
		// zero and the compareTo and equals methods will remain
		// consistent.
		if ((c = compare(this->authority, that->authority)) != 0) return c;
	}

	if ((c = compare(this->path, that->path)) != 0) return c;
	if ((c = compare(this->query, that->query)) != 0) return c;
	return compare(this->fragment, that->fragment);
}

EString EURI::toString() {
	defineString();
	return *string;
}

EString EURI::toASCIIString() {
	defineString();
	return encode(*string);
}

sp<EMap<EString*, EString*> > EURI::getParameterMap() {
	if (paramsMap != null) {
		return paramsMap;
	}

	paramsMap = new EHashMap<EString*, EString*>();
	EString query = this->getRawQuery();
	if (!query.isEmpty()) {
		EArray<EString*> params = EPattern::split("&", query.c_str());
		for (int i=0; i<params.length(); i++) {
			EString* param = params.getAt(i);
			paramsMap->put(new EString(EURLDecoder::decode(param->splitAt("=", 1).c_str())),
					new EString(EURLDecoder::decode(param->splitAt("=", 2).c_str())));
		}
	}
	return paramsMap;
}

EString EURI::getParameter(const char* key, const char* defVal) {
	sp<EMap<EString*, EString*> > paramsMap = getParameterMap();
	EString key_(key);
	EString* v = paramsMap->get(&key_);
	if (!v || v->isEmpty()) {
		return defVal;
	}
	return v;
}

// RFC2396 5.2
sp<EURI> EURI::resolve(EURI* base, EURI* child) {
	// check if child if opaque first so that NPE is thrown
	// if child is null.
	if (child->isOpaque() || base->isOpaque())
		return new EURI(*child);

	// 5.2 (2): Reference to current document (lone fragment)
	if ((child->scheme == null) && (child->authority == null)
		&& child->path->equals("") && (child->fragment != null)
		&& (child->query == null)) {
		if ((base->fragment != null)
			&& child->fragment->equals(base->fragment)) {
			return new EURI(*base);
		}
		EURI* ru = new EURI();
		if (base->scheme) ru->scheme = new EString(base->scheme);
		if (base->authority) ru->authority = new EString(base->authority);
		if (base->userInfo) ru->userInfo =  new EString(base->userInfo);
		if (base->host) ru->host =  new EString(base->host);
		ru->port = base->port;
		if (base->path) ru->path = new EString(base->path);
		if (child->fragment) ru->fragment = new EString(child->fragment);
		if (base->query) ru->query = new EString(base->query);
		return ru;
	}

	// 5.2 (3): Child is absolute
	if (child->scheme != null)
		return new EURI(*child);

	EURI* ru = new EURI();             // Resolved URI
	if (base->scheme) ru->scheme = new EString(base->scheme);
	if (child->query) ru->query = new EString(child->query);
	if (child->fragment) ru->fragment = new EString(child->fragment);

	// 5.2 (4): Authority
	if (child->authority == null) {
		if (base->authority) ru->authority = new EString(base->authority);
		if (base->host) ru->host = new EString(base->host);
		if (base->userInfo) ru->userInfo = new EString(base->userInfo);
		ru->port = base->port;

		EString cp = (child->path == null) ? "" : *(child->path);
		if ((cp.length() > 0) && (cp.charAt(0) == '/')) {
			// 5.2 (5): Child path is absolute
			if (child->path) ru->path = new EString(child->path);
		} else {
			// 5.2 (6): Resolve relative path
			ru->path = new EString(resolvePath(base->path, &cp, base->isAbsolute()));
		}
	} else {
		if (child->authority) ru->authority = new EString(child->authority);
		if (child->host) ru->host = new EString(child->host);
		if (child->userInfo) ru->userInfo = new EString(child->userInfo);
		if (child->host) ru->host = new EString(child->host);
		ru->port = child->port;
		if (child->path) ru->path = new EString(child->path);
	}

	// 5.2 (7): Recombine (nothing to do here)
	return ru;
}

// If the given URI's path is normal then return the URI;
// o.w., return a new URI containing the normalized path.
//
sp<EURI> EURI::normalize(EURI* u) {
	if (u->isOpaque() || (u->path == null) || (u->path->length() == 0))
		return new EURI(*u);

	EString np = c_normalize(*(u->path));
	if (u->path->equals(np))
		return new EURI(*u);

	EURI* v = new EURI();
	if (u->scheme) v->scheme = new EString(u->scheme);
	if (u->fragment) v->fragment = new EString(u->fragment);
	if (u->authority) v->authority = new EString(u->authority);
	if (u->userInfo) v->userInfo = new EString(u->userInfo);
	if (u->host) v->host = new EString(u->host);
	v->port = u->port;
	v->path = new EString(np);
	if (u->query) v->query = new EString(u->query);
	return v;
}

// If both URIs are hierarchical, their scheme and authority components are
// identical, and the base path is a prefix of the child's path, then
// return a relative URI that, when resolved against the base, yields the
// child; otherwise, return the child.
//
sp<EURI> EURI::relativize(EURI* base, EURI* child) {
	// check if child if opaque first so that NPE is thrown
	// if child is null.
	if (child->isOpaque() || base->isOpaque())
		return new EURI(*child);
	if (!equalIgnoringCase(base->scheme, child->scheme)
		|| !equal(base->authority, child->authority))
		return new EURI(*child);

	EString bp = c_normalize(*(base->path));
	EString cp = c_normalize(*(child->path));
	if (!bp.equals(cp)) {
		if (!bp.endsWith("/"))
			bp = bp + "/";
		if (!cp.startsWith(bp))
			return new EURI(*child);
	}

	EURI* v = new EURI();
	v->path = new EString(cp.substring(bp.length()));
	if (child->query) v->query = new EString(child->query);
	if (child->fragment) v->fragment = new EString(child->fragment);
	return v;
}

void EURI::appendAuthority(EString& sb, EString* authority, EString* userInfo,
			EString* host, int port) {
	if (host != null) {
		sb.append("//");
		if (userInfo != null) {
			sb.append(quote(*userInfo, L_USERINFO, H_USERINFO));
			sb.append('@');
		}
		boolean needBrackets = ((host->indexOf(':') >= 0)
								&& !host->startsWith("[")
								&& !host->endsWith("]"));
		if (needBrackets) sb.append('[');
		sb.append(*host);
		if (needBrackets) sb.append(']');
		if (port != -1) {
			sb.append(':');
			sb.append(port);
		}
	} else if (authority != null) {
		sb.append("//");
		if (authority->startsWith("[")) {
			// authority should (but may not) contain an embedded IPv6 address
			int end = authority->indexOf("]");
			EString doquote = *authority, dontquote = "";
			if (end != -1 && authority->indexOf(":") != -1) {
				// the authority contains an IPv6 address
				if (end == authority->length()) {
					dontquote = *authority;
					doquote = "";
				} else {
					dontquote = authority->substring(0 , end + 1);
					doquote = authority->substring(end + 1);
				}
			}
			sb.append(dontquote);
			sb.append(quote(doquote,
						L_REG_NAME | L_SERVER,
						H_REG_NAME | H_SERVER));
		} else {
			sb.append(quote(*authority,
						L_REG_NAME | L_SERVER,
						H_REG_NAME | H_SERVER));
		}
	}
}

void EURI::appendSchemeSpecificPart(EString& sb, EString* opaquePart,
		EString* authority, EString* userInfo, EString* host, int port,
		EString* path, EString* query) {
	if (opaquePart != null) {
		/* check if SSP begins with an IPv6 address
		 * because we must not quote a literal IPv6 address
		 */
		if (opaquePart->startsWith("//[")) {
			int end =  opaquePart->indexOf("]");
			if (end != -1 && opaquePart->indexOf(":")!=-1) {
				EString doquote, dontquote;
				if (end == opaquePart->length()) {
					dontquote = *opaquePart;
					doquote = "";
				} else {
					dontquote = opaquePart->substring(0,end+1);
					doquote = opaquePart->substring(end+1);
				}
				sb.append (dontquote);
				sb.append(quote(doquote, L_URIC, H_URIC));
			}
		} else {
			sb.append(quote(*opaquePart, L_URIC, H_URIC));
		}
	} else {
		appendAuthority(sb, authority, userInfo, host, port);
		if (path != null)
			sb.append(quote(*path, L_PATH, H_PATH));
		if (query != null) {
			sb.append('?');
			sb.append(quote(*query, L_URIC, H_URIC));
		}
	}
}

void EURI::appendFragment(EString& sb, EString* fragment) {
	if (fragment != null) {
		sb.append('#');
		sb.append(quote(*fragment, L_URIC, H_URIC));
	}
}

EString EURI::toString(const char* scheme, const char* opaquePart,
		const char* authority, const char* userInfo, const char* host,
		int port, const char* path, const char* query,
		const char* fragment) {
	EString sb;
	if (scheme != null) {
		sb.append(scheme);
		sb.append(':');
	}
	EString opaquePart_(opaquePart);
	EString authority_(authority);
	EString userInfo_(userInfo);
	EString host_(host);
	EString path_(path);
	EString query_(query);
	appendSchemeSpecificPart(sb, &opaquePart_,
							 &authority_, &userInfo_, &host_, port,
							 &path_, &query_);
	EString fragment_(fragment);
	appendFragment(sb, &fragment_);
	return sb;
}

void EURI::defineSchemeSpecificPart() {
	if (schemeSpecificPart != null) return;
	EString sb;

	const char* a = getAuthority();
	EString authority(a);
	const char* u = getUserInfo();
	EString userInfo(u);
	const char* p = getPath();
	EString path(p);
	const char* q = getQuery();
	EString query(q);
	appendSchemeSpecificPart(sb,
			null,
			a ? &authority : null,
			u ? &userInfo : null,
			host, port,
			p ? &path : null,
			q ? &query : null);
	if (sb.length() == 0) return;
	schemeSpecificPart = new EString(sb);
}

void EURI::defineString() {
	if (string != null) return;

	EString sb;
	if (scheme != null) {
		sb.append(*scheme);
		sb.append(':');
	}
	if (isOpaque()) {
		sb.append(*schemeSpecificPart);
	} else {
		if (host != null) {
			sb.append("//");
			if (userInfo != null) {
				sb.append(*userInfo);
				sb.append('@');
			}
			boolean needBrackets = ((host->indexOf(':') >= 0)
								&& !host->startsWith("[")
								&& !host->endsWith("]"));
			if (needBrackets) sb.append('[');
			sb.append(*host);
			if (needBrackets) sb.append(']');
			if (port != -1) {
				sb.append(':');
				sb.append(port);
			}
		} else if (authority != null) {
			sb.append("//");
			sb.append(*authority);
		}
		if (path != null)
			sb.append(*path);
		if (query != null) {
			sb.append('?');
			sb.append(*query);
		}
	}
	if (fragment != null) {
		sb.append('#');
		sb.append(*fragment);
	}
	string = new EString(sb);
}

//=============================================================================

// Tells whether start < end and, if so, whether charAt(start) == c
//
static boolean at(EString& input, int start, int end, char c) {
	return (start < end) && (input.charAt(start) == c);
}

// Tells whether start + s.length() < end and, if so,
// whether the chars at the start position match s exactly
//
static boolean at(EString& input, int start, int end, const char* str) {
	EString s(str);

	int p = start;
	int sn = s.length();
	if (sn > end - p)
		return false;
	int i = 0;
	while (i < sn) {
		if (input.charAt(p++) != s.charAt(i)) {
			break;
		}
		i++;
	}
	return (i == sn);
}

// -- Scanning --

// The various scan and parse methods that follow use a uniform
// convention of taking the current start position and end index as
// their first two arguments.  The start is inclusive while the end is
// exclusive, just as in the String class, i.e., a start/end pair
// denotes the left-open interval [start, end) of the input string.
//
// These methods never proceed past the end position.  They may return
// -1 to indicate outright failure, but more often they simply return
// the position of the first char after the last char scanned.  Thus
// a typical idiom is
//
//     int p = start;
//     int q = scan(p, end, ...);
//     if (q > p)
//         // We scanned something
//         ...;
//     else if (q == p)
//         // We scanned nothing
//         ...;
//     else if (q == -1)
//         // Something went wrong
//         ...;


// Scan a specific char: If the char at the given start position is
// equal to c, return the index of the next char; otherwise, return the
// start position.
//
static int scan(EString& input, int start, int end, char c) {
	if ((start < end) && (input.charAt(start) == c))
		return start + 1;
	return start;
}

// Scan forward from the given start position.  Stop at the first char
// in the err string (in which case -1 is returned), or the first char
// in the stop string (in which case the index of the preceding char is
// returned), or the end of the input string (in which case the length
// of the input string is returned).  May return the start position if
// nothing matches.
//
static int scan(EString& input, int start, int end, const char* err, const char* stop) {
	EString e(err);
	EString s(stop);

	int p = start;
	while (p < end) {
		char c = input.charAt(p);
		if (e.indexOf(c) >= 0)
			return -1;
		if (s.indexOf(c) >= 0)
			break;
		p++;
	}
	return p;
}

// Scan a potential escape sequence, starting at the given position,
// with the given first char (i.e., charAt(start) == c).
//
// This method assumes that if escapes are allowed then visible
// non-US-ASCII chars are also allowed.
//
static int scanEscape(EString& input, int start, int n, char first)
{
	int p = start;
	char c = first;
	if (c == '%') {
		// Process escape pair
		if ((p + 3 <= n)
			&& match(input.charAt(p + 1), L_HEX, H_HEX)
			&& match(input.charAt(p + 2), L_HEX, H_HEX)) {
			return p + 3;
		}
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Malformed escape pair", p);
	} else if ((c > 128)
			   && !eso_isspace(c)
			   && !eso_iscntrl(c)) {
		// Allow unescaped but visible non-US-ASCII chars
		return p + 1;
	}
	return p;
}

// Scan chars that match the given mask pair
//
static int scan(EString& input, int start, int n, llong lowMask, llong highMask)
{
	int p = start;
	while (p < n) {
		char c = input.charAt(p);
		if (match(c, lowMask, highMask)) {
			p++;
			continue;
		}
		if ((lowMask & L_ESCAPED) != 0) {
			int q = scanEscape(input, p, n, c);
			if (q > p) {
				p = q;
				continue;
			}
		}
		break;
	}
	return p;
}

// Check that each of the chars in [start, end) matches the given mask
//
static void checkChars(EString& input, int start, int end,
		llong lowMask, llong highMask, const char* what)
{
	EString w("Illegal character in ");
	w << what;

	int p = scan(input, start, end, lowMask, highMask);
	if (p < end)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), w.c_str(), p);
}

// Check that the char at position p matches the given mask
//
static void checkChar(EString& input, int p,
		llong lowMask, llong highMask, const char* what)
{
	checkChars(input, p, p + 1, lowMask, highMask, what);
}


void EURI::parseURI(EString& input, boolean requireServerAuthority) {
	delete string; string = new EString(input);

	int ssp;                    // Start of scheme-specific part
	int n = input.length();
	int p = scan(input, 0, n, "/?#", ":");
	if ((p >= 0) && at(input, p, n, ':')) {
		if (p == 0)
			throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected scheme name", 0);
		checkChar(input, 0, L_ALPHA, H_ALPHA, "scheme name");
		checkChars(input, 1, p, L_SCHEME, H_SCHEME, "scheme name");
		delete scheme; scheme = new EString(input.substring(0, p));
		p++;                    // Skip ':'
		ssp = p;
		if (at(input, p, n, '/')) {
			p = parseHierarchical(input, requireServerAuthority, p, n);
		} else {
			int q = scan(input, p, n, "", "#");
			if (q <= p)
				throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected scheme-specific part", p);
			checkChars(input, p, q, L_URIC, H_URIC, "opaque part");
			p = q;
		}
	} else {
		ssp = 0;
		p = parseHierarchical(input, requireServerAuthority, 0, n);
	}
	delete schemeSpecificPart; schemeSpecificPart = new EString(input.substring(ssp, p));
	if (at(input, p, n, '#')) {
		checkChars(input, p + 1, n, L_URIC, H_URIC, "fragment");
		delete fragment; fragment = new EString(input.substring(p + 1, n));
		p = n;
	}
	if (p < n)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "end of URI", p);
}

int EURI::parseHierarchical(EString& input, boolean requireServerAuthority, int start, int n)
{
	int p = start;
	if (at(input, p, n, '/') && at(input, p + 1, n, '/')) {
		p += 2;
		int q = scan(input, p, n, "", "/?#");
		if (q > p) {
			p = parseAuthority(input, requireServerAuthority, p, q);
		} else if (q < n) {
			// DEVIATION: Allow empty authority prior to non-empty
			// path, query component or fragment identifier
		} else
			throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected authority", p);
	}
	int q = scan(input, p, n, "", "?#"); // DEVIATION: May be empty
	checkChars(input, p, q, L_PATH, H_PATH, "path");
	delete path; path = new EString(input.substring(p, q));
	p = q;
	if (at(input, p, n, '?')) {
		p++;
		q = scan(input, p, n, "", "#");
		checkChars(input, p, q, L_URIC, H_URIC, "query");
		delete query; query = new EString(input.substring(p, q));
		p = q;
	}
	return p;
}

int EURI::parseAuthority(EString& input, boolean requireServerAuthority, int start, int n) {
	int p = start;
	int q = p;
	sp<EURISyntaxException> ex;

	boolean serverChars;
	boolean regChars;

	if (scan(input, p, n, "", "]") > p) {
		// contains a literal IPv6 address, therefore % is allowed
		serverChars = (scan(input, p, n, L_SERVER_PERCENT, H_SERVER_PERCENT) == n);
	} else {
		serverChars = (scan(input, p, n, L_SERVER, H_SERVER) == n);
	}
	regChars = (scan(input, p, n, L_REG_NAME, H_REG_NAME) == n);

	if (regChars && !serverChars) {
		// Must be a registry-based authority
		delete authority; authority = new EString(input.substring(p, n));
		return n;
	}

	if (serverChars) {
		// Might be (probably is) a server-based authority, so attempt
		// to parse it as such.  If the attempt fails, try to treat it
		// as a registry-based authority.
		try {
			q = parseServer(input, p, n);
			if (q < n)
				throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected end of authority", q);
			delete authority; authority = new EString(input.substring(p, n));
		} catch (EURISyntaxException& x) {
			// Undo results of failed parse
			delete userInfo; userInfo = null;
			delete host;     host = null;
			port = -1;
			if (requireServerAuthority) {
				// If we're insisting upon a server-based authority,
				// then just re-throw the exception
				throw x;
			} else {
				// Save the exception in case it doesn't parse as a
				// registry either
				ex = new EURISyntaxException(x);
				q = p;
			}
		}
	}

	if (q < n) {
		if (regChars) {
			// Registry-based authority
			delete authority; authority = new EString(input.substring(p, n));
		} else if (ex != null) {
			// Re-throw exception; it was probably due to
			// a malformed IPv6 address
			throw EURISyntaxException(*ex);
		} else {
			throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Illegal character in authority", q);
		}
	}

	return n;
}

int EURI::parseServer(EString& input, int start, int n) {
	int p = start;
	int q;

	// userinfo
	q = scan(input, p, n, "/?#", "@");
	if ((q >= p) && at(input, q, n, '@')) {
		checkChars(input, p, q, L_USERINFO, H_USERINFO, "user info");
		delete userInfo; userInfo = new EString(input.substring(p, q));
		p = q + 1;              // Skip '@'
	}

	int ipv6byteCount = 0;

	// hostname, IPv4 address, or IPv6 address
	if (at(input, p, n, '[')) {
		// DEVIATION from RFC2396: Support IPv6 addresses, per RFC2732
		p++;
		q = scan(input, p, n, "/?#", "]");
		if ((q > p) && at(input, q, n, ']')) {
			// look for a "%" scope id
			int r = scan (input, p, q, "", "%");
			if (r > p) {
				parseIPv6Reference(input, &ipv6byteCount, p, r);
				if (r+1 == q) {
					throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected scope id expected");
				}
				checkChars (input, r+1, q, L_ALPHANUM, H_ALPHANUM,
										"scope id");
			} else {
				parseIPv6Reference(input, &ipv6byteCount, p, q);
			}
			delete host; host = new EString(input.substring(p-1, q+1));
			p = q + 1;
		} else {
			throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected closing bracket for IPv6 address", q);
		}
	} else {
		q = parseIPv4Address(input, p, n);
		if (q <= p)
			q = parseHostname(input, p, n);
		p = q;
	}

	// port
	if (at(input, p, n, ':')) {
		p++;
		q = scan(input, p, n, "", "/");
		if (q > p) {
			checkChars(input, p, q, L_DIGIT, H_DIGIT, "port number");
			try {
				port = EInteger::parseInt(input.substring(p, q).c_str());
			} catch (ENumberFormatException& x) {
				throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Malformed port number", p);
			}
			p = q;
		}
	}
	if (p < n)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected port number", p);

	return p;
}

int EURI::scanByte(EString& input, int start, int n) {
	int p = start;
	int q = scan(input, p, n, L_DIGIT, H_DIGIT);
	if (q <= p) return q;
	if (EInteger::parseInt(input.substring(p, q).c_str()) > 255) return p;
	return q;
}

int EURI::scanIPv4Address(EString& input, int start, int n, boolean strict) {
	int p = start;
	int q;
	int m = scan(input, p, n, L_DIGIT | L_DOT, H_DIGIT | H_DOT);
	if ((m <= p) || (strict && (m != n)))
		return -1;
	for (;;) {
		// Per RFC2732: At most three digits per byte
		// Further constraint: Each element fits in a byte
		if ((q = scanByte(input, p, m)) <= p) break;   p = q;
		if ((q = scan(input, p, m, '.')) <= p) break;  p = q;
		if ((q = scanByte(input, p, m)) <= p) break;   p = q;
		if ((q = scan(input, p, m, '.')) <= p) break;  p = q;
		if ((q = scanByte(input, p, m)) <= p) break;   p = q;
		if ((q = scan(input, p, m, '.')) <= p) break;  p = q;
		if ((q = scanByte(input, p, m)) <= p) break;   p = q;
		if (q < m) break;
		return q;
	}
	throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Malformed IPv4 address", q);
	return -1;
}

int EURI::takeIPv4Address(EString& input, int start, int n, const char* expected) {
	int p = scanIPv4Address(input, start, n, true);
	if (p <= start)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), expected, start);
	return p;
}

int EURI::parseIPv4Address(EString& input, int start, int n) {
	int p;

	try {
		p = scanIPv4Address(input, start, n, false);
	} catch (EURISyntaxException& x) {
		return -1;
	} catch (ENumberFormatException& nfe) {
		return -1;
	}

	if (p > start && p < n) {
		// IPv4 address is followed by something - check that
		// it's a ":" as this is the only valid character to
		// follow an address.
		if (input.charAt(p) != ':') {
			p = -1;
		}
	}

	if (p > start && (this->host == null)) {
		delete host; host = new EString(input.substring(start, p));
	}

	return p;
}

int EURI::parseHostname(EString& input, int start, int n) {
	int p = start;
	int q;
	int l = -1;                 // Start of last parsed label

	do {
		// domainlabel = alphanum [ *( alphanum | "-" ) alphanum ]
		q = scan(input, p, n, L_ALPHANUM, H_ALPHANUM);
		if (q <= p)
			break;
		l = p;
		if (q > p) {
			p = q;
			q = scan(input, p, n, L_ALPHANUM | L_DASH, H_ALPHANUM | H_DASH);
			if (q > p) {
				if (input.charAt(q - 1) == '-')
					throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Illegal character in hostname", q - 1);
				p = q;
			}
		}
		q = scan(input, p, n, '.');
		if (q <= p)
			break;
		p = q;
	} while (p < n);

	if ((p < n) && !at(input, p, n, ':'))
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Illegal character in hostname", p);

	if (l < 0)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected hostname", start);

	// for a fully qualified hostname check that the rightmost
	// label starts with an alpha character.
	if (l > start && !match(input.charAt(l), L_ALPHA, H_ALPHA)) {
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Illegal character in hostname", l);
	}

	delete host; host = new EString(input.substring(start, p));
	return p;
}

int EURI::parseIPv6Reference(EString& input, int* ipv6byteCount, int start, int n) {
	int p = start;
	int q;
	boolean compressedZeros = false;

	q = scanHexSeq(input, ipv6byteCount, p, n);

	if (q > p) {
		p = q;
		if (at(input, p, n, "::")) {
			compressedZeros = true;
			p = scanHexPost(input, ipv6byteCount, p + 2, n);
		} else if (at(input, p, n, ':')) {
			p = takeIPv4Address(input, p + 1,  n, "IPv4 address");
			*ipv6byteCount += 4;
		}
	} else if (at(input, p, n, "::")) {
		compressedZeros = true;
		p = scanHexPost(input, ipv6byteCount, p + 2, n);
	}
	if (p < n)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Malformed IPv6 address", start);
	if (*ipv6byteCount > 16)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "IPv6 address too long", start);
	if (!compressedZeros && *ipv6byteCount < 16)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "IPv6 address too short", start);
	if (compressedZeros && *ipv6byteCount == 16)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Malformed IPv6 address", start);

	return p;
}

int EURI::scanHexPost(EString& input, int* ipv6byteCount, int start, int n) {
	int p = start;
	int q;

	if (p == n)
		return p;

	q = scanHexSeq(input, ipv6byteCount, p, n);
	if (q > p) {
		p = q;
		if (at(input, p, n, ':')) {
			p++;
			p = takeIPv4Address(input, p, n, "hex digits or IPv4 address");
			*ipv6byteCount += 4;
		}
	} else {
		p = takeIPv4Address(input, p, n, "hex digits or IPv4 address");
		*ipv6byteCount += 4;
	}
	return p;
}

int EURI::scanHexSeq(EString& input, int* ipv6byteCount, int start, int n) {
	int p = start;
	int q;

	q = scan(input, p, n, L_HEX, H_HEX);
	if (q <= p)
		return -1;
	if (at(input, q, n, '.'))          // Beginning of IPv4 address
		return -1;
	if (q > p + 4)
		throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "IPv6 hexadecimal digit sequence too long", p);
	*ipv6byteCount += 2;
	p = q;
	while (p < n) {
		if (!at(input, p, n, ':'))
			break;
		if (at(input, p + 1, n, ':'))
			break;              // "::"
		p++;
		q = scan(input, p, n, L_HEX, H_HEX);
		if (q <= p)
			throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "Expected digits for an IPv6 address", p);
		if (at(input, q, n, '.')) {    // Beginning of IPv4 address
			p--;
			break;
		}
		if (q > p + 4)
			throw EURISyntaxException(__FILE__, __LINE__, input.c_str(), "IPv6 hexadecimal digit sequence too long", p);
		*ipv6byteCount += 2;
		p = q;
	}

	return p;
}

} /* namespace efc */
