#include "EStringTokenizer.hh"

namespace efc {

/*****************************************************************************\
 * StringTokenizer                                                            |
 *****************************************************************************/
EStringTokenizer::EStringTokenizer(const char* str) :
		currentPosition(0), newPosition(-1) {
  this->str = str;
  this->delimiters = " \t\n\r\f";
  this->maxPosition = eso_strlen(str);
  this->delimsChanged = false;
  this->retDelims = false;
}

EStringTokenizer::EStringTokenizer(const char* str, const char* delim, boolean returnDelims) :
		currentPosition(0), newPosition(-1) {
  this->str = str;
  this->delimiters = delim;
  this->maxPosition = eso_strlen(str);
  this->retDelims = returnDelims;
  this->delimsChanged = false;
}

boolean EStringTokenizer::isDelimiter(char codePoint) {
	for (uint i = 0; i < delimiters.length(); i++) {
		if (delimiters.charAt(i) == codePoint) {
			return true;
		}
	}
	return false;
}

int EStringTokenizer::skipDelimiters(int startPos) {
	int position = startPos;
	while (!retDelims && position < maxPosition && isDelimiter(str[position])) {
		position++;
	}
	return position;
}

int EStringTokenizer::scanToken(int startPos) {
	int position = startPos;

	while (retDelims && position < maxPosition && isDelimiter(str[position])) {
		return ++position;
	}

	while (position < maxPosition) {
		if (isDelimiter(str[++position])) {
			break;
		}
	}

	return position;
}

/*****************************************************************************\
 * hasMoreTokens                                                              |
 *****************************************************************************/
boolean EStringTokenizer::hasMoreTokens() {
	newPosition = currentPosition;
	newPosition = skipDelimiters(currentPosition);
	return (newPosition < maxPosition);
}


/*****************************************************************************\
 * nextToken                                                                  |
 *****************************************************************************/
EString EStringTokenizer::nextToken(const char* delim) THROWS(ENoSuchElementException)
{
  this->delimiters = delim;

  /* delimiter string specified, so set the appropriate flag. */
  delimsChanged = true;

  return nextToken();
}


/*****************************************************************************\
 * nextToken                                                                  |
 *****************************************************************************/
EString EStringTokenizer::nextToken() THROWS(ENoSuchElementException)
{
	/*
	 * If next position already computed in hasMoreElements() and
	 * delimiters have changed between the computation and this invocation,
	 * then use the computed value.
	 */

	currentPosition =
			(newPosition >= 0 && !delimsChanged) ?
					newPosition : skipDelimiters(currentPosition);

	/* Reset these anyway */
	delimsChanged = false;
	newPosition = -1;

	if (currentPosition >= maxPosition)
		throw ENoSuchElementException(__FILE__, __LINE__);
	int start = currentPosition;
	currentPosition = scanToken(currentPosition);
	return EString(str, start, currentPosition-start);
}

/*****************************************************************************\
 * countTokens                                                                |
 *****************************************************************************/
int EStringTokenizer::countTokens()
{
	int count = 0;
	int currpos = currentPosition;
	while (currpos < maxPosition) {
		currpos = skipDelimiters(currpos);
		if (currpos >= maxPosition)
			break;
		currpos = scanToken(currpos);
		count++;
	}
	return count;
}

} /* namespace efc */
