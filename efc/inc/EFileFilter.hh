#ifndef __EFILEFILTER_HH__
#define __EFILEFILTER_HH__

#include "EBase.hh"

namespace efc {

class EFile;

/**
 * A filter for abstract pathnames.
 *
 * <p> Instances of this interface may be passed to the <code>{@link
 * File#listFiles(java.io.FileFilter) listFiles(FileFilter)}</code> method
 * of the <code>{@link java.io.File}</code> class.
 *
 * @since 1.2
 */

interface EFileFilter : virtual public EObject
{
	virtual ~EFileFilter(){}
	
	/**
	 * Tests whether or not the specified abstract pathname should be
	 * included in a pathname list.
	 *
	 * @param  pathname  The abstract pathname to be tested
	 * @return  <code>true</code> if and only if <code>pathname</code>
	 *          should be included
	 */
	virtual boolean accept(EFile *pathname) = 0;
};

} /* namespace efc */
#endif //!__EFILEFILTER_HH__
