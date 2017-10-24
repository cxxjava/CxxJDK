#ifndef __EFILENAMEFILTER_HH__
#define __EFILENAMEFILTER_HH__

#include "EBase.hh"

namespace efc {

class EFile;

/**
 * This interface has one method which is used for filtering filenames
 * returned in a directory listing. It is currently used by the
 * \c File::list() method.
 *
 * The method in this interface determines if a particular file should
 * or should not be included in the file listing.
 *
 * @class FilenameFilter
 * @version $Id: FilenameFilter.h,v 1.4 2003-09-13 08:46:07 florian Exp $
 */

interface EFilenameFilter : virtual public EObject
{
	virtual ~EFilenameFilter(){}
	
	/**
	 * This method determines whether or not a given file should be included
	 * in a directory listing.
	 *
	 * @param dir  The \c File instance for the directory being read
	 * @param name The name of the file to test
	 *
	 * @return \c true if the file should be included in the list,
	 *         \c false otherwise.
	 */
	virtual boolean accept(EFile *dir, const char *name) = 0;
};

} /* namespace efc */
#endif //!__EFILENAMEFILTER_HH__
