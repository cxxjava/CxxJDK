/*
 * EFile.hh
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#ifndef EFILE_HH_
#define EFILE_HH_

#include "EObject.hh"
#include "EString.hh"
#include "EFileFilter.hh"
#include "EFilenameFilter.hh"
#include "EArray.hh"
#include "EIOException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

/**
 * An abstract representation of file and directory pathnames.
 *
 * <p> User interfaces and operating systems use system-dependent <em>pathname
 * strings</em> to name files and directories.  This class presents an
 * abstract, system-independent view of hierarchical pathnames.  An
 * <em>abstract pathname</em> has two components:
 *
 * <ol>
 * <li> An optional system-dependent <em>prefix</em> string,
 *      such as a disk-drive specifier, <code>"/"</code>&nbsp;for the UNIX root
 *      directory, or <code>"\\\\"</code>&nbsp;for a Microsoft Windows UNC pathname, and
 * <li> A sequence of zero or more string <em>names</em>.
 * </ol>
 *
 * The first name in an abstract pathname may be a directory name or, in the
 * case of Microsoft Windows UNC pathnames, a hostname.  Each subsequent name
 * in an abstract pathname denotes a directory; the last name may denote
 * either a directory or a file.  The <em>empty</em> abstract pathname has no
 * prefix and an empty name sequence.
 *
 * <p> The conversion of a pathname string to or from an abstract pathname is
 * inherently system-dependent.  When an abstract pathname is converted into a
 * pathname string, each name is separated from the next by a single copy of
 * the default <em>separator character</em>.  The default name-separator
 * character is defined by the system property <code>file.separator</code>, and
 * is made available in the public static fields <code>{@link
 * #separator}</code> and <code>{@link #separatorChar}</code> of this class.
 * When a pathname string is converted into an abstract pathname, the names
 * within it may be separated by the default name-separator character or by any
 * other name-separator character that is supported by the underlying system.
 *
 * <p> A pathname, whether abstract or in string form, may be either
 * <em>absolute</em> or <em>relative</em>.  An absolute pathname is complete in
 * that no other information is required in order to locate the file that it
 * denotes.  A relative pathname, in contrast, must be interpreted in terms of
 * information taken from some other pathname.  By default the classes in the
 * <code>java.io</code> package always resolve relative pathnames against the
 * current user directory.  This directory is named by the system property
 * <code>user.dir</code>, and is typically the directory in which the Java
 * virtual machine was invoked.
 *
 * <p> The <em>parent</em> of an abstract pathname may be obtained by invoking
 * the {@link #getParent} method of this class and consists of the pathname's
 * prefix and each name in the pathname's name sequence except for the last.
 * Each directory's absolute pathname is an ancestor of any <tt>File</tt>
 * object with an absolute abstract pathname which begins with the directory's
 * absolute pathname.  For example, the directory denoted by the abstract
 * pathname <tt>"/usr"</tt> is an ancestor of the directory denoted by the
 * pathname <tt>"/usr/local/bin"</tt>.
 *
 * <p> The prefix concept is used to handle root directories on UNIX platforms,
 * and drive specifiers, root directories and UNC pathnames on Microsoft Windows platforms,
 * as follows:
 *
 * <ul>
 *
 * <li> For UNIX platforms, the prefix of an absolute pathname is always
 * <code>"/"</code>.  Relative pathnames have no prefix.  The abstract pathname
 * denoting the root directory has the prefix <code>"/"</code> and an empty
 * name sequence.
 *
 * <li> For Microsoft Windows platforms, the prefix of a pathname that contains a drive
 * specifier consists of the drive letter followed by <code>":"</code> and
 * possibly followed by <code>"\\"</code> if the pathname is absolute.  The
 * prefix of a UNC pathname is <code>"\\\\"</code>; the hostname and the share
 * name are the first two names in the name sequence.  A relative pathname that
 * does not specify a drive has no prefix.
 *
 * </ul>
 *
 * <p> Instances of this class may or may not denote an actual file-system
 * object such as a file or a directory.  If it does denote such an object
 * then that object resides in a <i>partition</i>.  A partition is an
 * operating system-specific portion of storage for a file system.  A single
 * storage device (e.g. a physical disk-drive, flash memory, CD-ROM) may
 * contain multiple partitions.  The object, if any, will reside on the
 * partition <a name="partName">named</a> by some ancestor of the absolute
 * form of this pathname.
 *
 * <p> A file system may implement restrictions to certain operations on the
 * actual file-system object, such as reading, writing, and executing.  These
 * restrictions are collectively known as <i>access permissions</i>.  The file
 * system may have multiple sets of access permissions on a single object.
 * For example, one set may apply to the object's <i>owner</i>, and another
 * may apply to all other users.  The access permissions on an object may
 * cause some methods in this class to fail.
 *
 * <p> Instances of the <code>File</code> class are immutable; that is, once
 * created, the abstract pathname represented by a <code>File</code> object
 * will never change.
 *
 * @version 1.142, 04/01/09
 * @since   JDK1.0
 */

class EFile: public EObject {
public:
	/**
	 * The system-dependent default name-separator character.  This field is
	 * initialized to contain the first character of the value of the system
	 * property <code>file.separator</code>.  On UNIX systems the value of this
	 * field is <code>'/'</code>; on Microsoft Windows systems it is <code>'\\'</code>.
	 *
	 * @see     java.lang.System#getProperty(java.lang.String)
	 */
	static const char separatorChar;

	/**
	 * The system-dependent default name-separator character, represented as a
	 * string for convenience.  This string contains a single character, namely
	 * <code>{@link #separatorChar}</code>.
	 */
	static const char *separator;

	/**
	 * The system-dependent path-separator character.  This field is
	 * initialized to contain the first character of the value of the system
	 * property <code>path.separator</code>.  This character is used to
	 * separate filenames in a sequence of files given as a <em>path list</em>.
	 * On UNIX systems, this character is <code>':'</code>; on Microsoft Windows systems it
	 * is <code>';'</code>.
	 *
	 * @see     java.lang.System#getProperty(java.lang.String)
	 */
	static const char pathSeparatorChar;

	/**
	 * The system-dependent path-separator character, represented as a string
	 * for convenience.  This string contains a single character, namely
	 * <code>{@link #pathSeparatorChar}</code>.
	 */
	static const char *pathSeparator;

public:
	virtual ~EFile(){}

	/* -- Constructors -- */

	/**
	 * Creates a new <code>File</code> instance by converting the given
	 * pathname string into an abstract pathname.  If the given string is
	 * the empty string, then the result is the empty abstract pathname.
	 *
	 * @param   pathname  A pathname string
	 * @throws  NullPointerException
	 *          If the <code>pathname</code> argument is <code>null</code>
	 */
	explicit
	EFile(const char *pathname);

	/**
	 *
	 */
	explicit
	EFile(EFile* file);

	/**
	 * Creates a new <code>File</code> instance from a parent pathname string
	 * and a child pathname string.
	 *
	 * <p> If <code>parent</code> is <code>null</code> then the new
	 * <code>File</code> instance is created as if by invoking the
	 * single-argument <code>File</code> constructor on the given
	 * <code>child</code> pathname string.
	 *
	 * <p> Otherwise the <code>parent</code> pathname string is taken to denote
	 * a directory, and the <code>child</code> pathname string is taken to
	 * denote either a directory or a file.  If the <code>child</code> pathname
	 * string is absolute then it is converted into a relative pathname in a
	 * system-dependent way.  If <code>parent</code> is the empty string then
	 * the new <code>File</code> instance is created by converting
	 * <code>child</code> into an abstract pathname and resolving the result
	 * against a system-dependent default directory.  Otherwise each pathname
	 * string is converted into an abstract pathname and the child abstract
	 * pathname is resolved against the parent.
	 *
	 * @param   parent  The parent pathname string
	 * @param   child   The child pathname string
	 * @throws  NullPointerException
	 *          If <code>child</code> is <code>null</code>
	 */
	EFile(const char *parent, const char *child);

	/**
	 * Creates a new <code>File</code> instance from a parent abstract
	 * pathname and a child pathname string.
	 *
	 * <p> If <code>parent</code> is <code>null</code> then the new
	 * <code>File</code> instance is created as if by invoking the
	 * single-argument <code>File</code> constructor on the given
	 * <code>child</code> pathname string.
	 *
	 * <p> Otherwise the <code>parent</code> abstract pathname is taken to
	 * denote a directory, and the <code>child</code> pathname string is taken
	 * to denote either a directory or a file.  If the <code>child</code>
	 * pathname string is absolute then it is converted into a relative
	 * pathname in a system-dependent way.  If <code>parent</code> is the empty
	 * abstract pathname then the new <code>File</code> instance is created by
	 * converting <code>child</code> into an abstract pathname and resolving
	 * the result against a system-dependent default directory.  Otherwise each
	 * pathname string is converted into an abstract pathname and the child
	 * abstract pathname is resolved against the parent.
	 *
	 * @param   parent  The parent abstract pathname
	 * @param   child   The child pathname string
	 * @throws  NullPointerException
	 *          If <code>child</code> is <code>null</code>
	 */
	EFile(EFile *parent, const char *child);

	/**
	 * Converts this abstract pathname into a pathname string.  The resulting
	 * string uses the {@link #separator default name-separator character} to
	 * separate the names in the name sequence.
	 *
	 * @return  The string form of this abstract pathname
	 */
	EString& getPath();

	/**
	 * Returns the directory part of the File's path including the trailing slash.
	 */
	EString& getDir();

	/* -- Path-component accessors -- */

	/**
	 * Returns the name of the file or directory denoted by this abstract
	 * pathname.  This is just the last name in the pathname's name
	 * sequence.  If the pathname's name sequence is empty, then the empty
	 * string is returned.
	 *
	 * @return  The name of the file or directory denoted by this abstract
	 *          pathname, or the empty string if this pathname's name sequence
	 *          is empty
	 */
	EString& getName();

	/**
	 * Returns the file name part of the File's path without trailing dot and extension.
	 */
	EString& getFName();

	/**
	 * Returns the extension part of the File's path including the dot.
	 * If there is not dot in the file name, the empty \c String is returned.
	 */
	EString& getExt();

	/**
	 * Returns the pathname string of this abstract pathname's parent, or
	 * <code>null</code> if this pathname does not name a parent directory.
	 *
	 * <p> The <em>parent</em> of an abstract pathname consists of the
	 * pathname's prefix, if any, and each name in the pathname's name
	 * sequence except for the last.  If the name sequence is empty then
	 * the pathname does not name a parent directory.
	 *
	 * @return  The pathname string of the parent directory named by this
	 *          abstract pathname, or <code>null</code> if this pathname
	 *          does not name a parent
	 */
	EString getParent();

	/**
	 * Returns the abstract pathname of this abstract pathname's parent,
	 * or <code>null</code> if this pathname does not name a parent
	 * directory.
	 *
	 * <p> The <em>parent</em> of an abstract pathname consists of the
	 * pathname's prefix, if any, and each name in the pathname's name
	 * sequence except for the last.  If the name sequence is empty then
	 * the pathname does not name a parent directory.
	 *
	 * @return  The abstract pathname of the parent directory named by this
	 *          abstract pathname, or <code>null</code> if this pathname
	 *          does not name a parent
	 *
	 * @since 1.2
	 */
	EFile getParentFile();

	/**
	 * Determines the path a symbolic link points to.
	 * Returns \c null if \c ::readlink returned an error or if called under Windows.
	 */
	EString readlink();


	/* -- Path operations -- */

	/**
	 * Tests whether this abstract pathname is absolute.  The definition of
	 * absolute pathname is system dependent.  On UNIX systems, a pathname is
	 * absolute if its prefix is <code>"/"</code>.  On Microsoft Windows systems, a
	 * pathname is absolute if its prefix is a drive specifier followed by
	 * <code>"\\"</code>, or if its prefix is <code>"\\\\"</code>.
	 *
	 * @return  <code>true</code> if this abstract pathname is absolute,
	 *          <code>false</code> otherwise
	 */
	boolean isAbsolute();

	/**
	 * Returns the absolute pathname string of this abstract pathname.
	 *
	 * <p> If this abstract pathname is already absolute, then the pathname
	 * string is simply returned as if by the <code>{@link #getPath}</code>
	 * method.  If this abstract pathname is the empty abstract pathname then
	 * the pathname string of the current user directory, which is named by the
	 * system property <code>user.dir</code>, is returned.  Otherwise this
	 * pathname is resolved in a system-dependent way.  On UNIX systems, a
	 * relative pathname is made absolute by resolving it against the current
	 * user directory.  On Microsoft Windows systems, a relative pathname is made absolute
	 * by resolving it against the current directory of the drive named by the
	 * pathname, if any; if not, it is resolved against the current user
	 * directory.
	 *
	 * @return  The absolute pathname string denoting the same file or
	 *          directory as this abstract pathname
	 *
	 * @throws  SecurityException
	 *          If a required system property value cannot be accessed.
	 *
	 * @see     java.io.File#isAbsolute()
	 */
	EString getAbsolutePath();

	/**
	 * Returns the canonical pathname string of this abstract pathname.
	 *
	 * <p> A canonical pathname is both absolute and unique.  The precise
	 * definition of canonical form is system-dependent.  This method first
	 * converts this pathname to absolute form if necessary, as if by invoking the
	 * {@link #getAbsolutePath} method, and then maps it to its unique form in a
	 * system-dependent way.  This typically involves removing redundant names
	 * such as <tt>"."</tt> and <tt>".."</tt> from the pathname, resolving
	 * symbolic links (on UNIX platforms), and converting drive letters to a
	 * standard case (on Microsoft Windows platforms).
	 *
	 * <p> Every pathname that denotes an existing file or directory has a
	 * unique canonical form.  Every pathname that denotes a nonexistent file
	 * or directory also has a unique canonical form.  The canonical form of
	 * the pathname of a nonexistent file or directory may be different from
	 * the canonical form of the same pathname after the file or directory is
	 * created.  Similarly, the canonical form of the pathname of an existing
	 * file or directory may be different from the canonical form of the same
	 * pathname after the file or directory is deleted.
	 *
	 * @return  The canonical pathname string denoting the same file or
	 *          directory as this abstract pathname
	 *
	 * @throws  IOException
	 *          If an I/O error occurs, which is possible because the
	 *          construction of the canonical pathname may require
	 *          filesystem queries
	 *
	 * @throws  SecurityException
	 *          If a required system property value cannot be accessed, or
	 *          if a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead}</code> method denies
	 *          read access to the file
	 *
	 * @since   JDK1.1
	 */
	EString& getCanonicalPath();

	//URL toURL() throws MalformedURLException;
	//URI toURI();

	/* -- Attribute accessors -- */

	/**
	 * Tests whether the file or directory denoted by this abstract pathname
	 * exists.
	 *
	 * @return  <code>true</code> if and only if the file or directory denoted
	 *          by this abstract pathname exists; <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the file or directory
	 */
	boolean exists( );

	/**
	 * Tests whether the file denoted by this abstract pathname is a
	 * directory.
	 *
	 * @return <code>true</code> if and only if the file denoted by this
	 *          abstract pathname exists <em>and</em> is a directory;
	 *          <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the file
	 */
	boolean isDirectory();

	/**
	 * Tests whether the file denoted by this abstract pathname is a normal
	 * file.  A file is <em>normal</em> if it is not a directory and, in
	 * addition, satisfies other system-dependent criteria.  Any non-directory
	 * file created by a Java application is guaranteed to be a normal file.
	 *
	 * @return  <code>true</code> if and only if the file denoted by this
	 *          abstract pathname exists <em>and</em> is a normal file;
	 *          <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the file
	 */
	boolean isFile();

	/*
	 * Determines whether the \c File represents a symbolic link.
	 */
	boolean isLink();

	/**
	 * Returns the time that the file denoted by this abstract pathname was
	 * last modified.
	 *
	 * @return  A <code>long</code> value representing the time the file was
	 *          last modified, measured in milliseconds since the epoch
	 *          (00:00:00 GMT, January 1, 1970), or <code>0L</code> if the
	 *          file does not exist or if an I/O error occurs
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the file
	 */
	llong lastModified();

	/**
	 * Returns the length of the file denoted by this abstract pathname.
	 * The return value is unspecified if this pathname denotes a directory.
	 *
	 * @return  The length, in bytes, of the file denoted by this abstract
	 *          pathname, or <code>0L</code> if the file does not exist.  Some
	 *          operating systems may return <code>0L</code> for pathnames
	 *          denoting system-dependent entities such as devices or pipes.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the file
	 */
	llong length();

	/**
	 * Returns the uniquely identifies of a file.
	 *
	 * @return a inode number of the given file
	 */
	llong inode();

	/**
	 * Tests whether the application can read the file denoted by this
	 * abstract pathname. On some platforms it may be possible to start the
	 * Java virtual machine with special privileges that allow it to read
	 * files that are marked as unreadable. Consequently this method may return
	 * {@code true} even though the file does not have read permissions.
	 *
	 * @return  <code>true</code> if and only if the file specified by this
	 *          abstract pathname exists <em>and</em> can be read by the
	 *          application; <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the file
	 */
	boolean canRead();

	/**
	 * Tests whether the application can modify the file denoted by this
	 * abstract pathname. On some platforms it may be possible to start the
	 * Java virtual machine with special privileges that allow it to modify
	 * files that are marked read-only. Consequently this method may return
	 * {@code true} even though the file is marked read-only.
	 *
	 * @return  <code>true</code> if and only if the file system actually
	 *          contains a file denoted by this abstract pathname <em>and</em>
	 *          the application is allowed to write to the file;
	 *          <code>false</code> otherwise.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to the file
	 */
	boolean canWrite();

	/**
	 * Tests whether the application can execute the file denoted by this
	 * abstract pathname. On some platforms it may be possible to start the
	 * Java virtual machine with special privileges that allow it to execute
	 * files that are not marked executable. Consequently this method may return
	 * {@code true} even though the file does not have execute permissions.
	 *
	 * @return  <code>true</code> if and only if the abstract pathname exists
	 *          <em>and</em> the application is allowed to execute the file
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkExec(java.lang.String)}</code>
	 *          method denies execute access to the file
	 *
	 * @since 1.6
	 */
	boolean canExecute();

	/* -- File operations -- */

	/**
	 * Atomically creates a new, empty file named by this abstract pathname if
	 * and only if a file with this name does not yet exist.  The check for the
	 * existence of the file and the creation of the file if it does not exist
	 * are a single operation that is atomic with respect to all other
	 * filesystem activities that might affect the file.
	 * <P>
	 * Note: this method should <i>not</i> be used for file-locking, as
	 * the resulting protocol cannot be made to work reliably. The
	 * {@link java.nio.channels.FileLock FileLock}
	 * facility should be used instead.
	 *
	 * @return  <code>true</code> if the named file does not exist and was
	 *          successfully created; <code>false</code> if the named file
	 *          already exists
	 *
	 * @throws  IOException
	 *          If an I/O error occurred
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to the file
	 *
	 * @since 1.2
	 */
	boolean createNewFile() THROWS(EIOException);

	/**
	 * Deletes the file or directory denoted by this abstract pathname.  If
	 * this pathname denotes a directory, then the directory must be empty in
	 * order to be deleted.
	 *
	 * @return  <code>true</code> if and only if the file or directory is
	 *          successfully deleted; <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkDelete}</code> method denies
	 *          delete access to the file
	 */
	boolean remove();

	/**
	 * Returns an array of strings naming the files and directories in the
	 * directory denoted by this abstract pathname that satisfy the specified
	 * filter.  The behavior of this method is the same as that of the
	 * <code>{@link #list()}</code> method, except that the strings in the
	 * returned array must satisfy the filter.  If the given
	 * <code>filter</code> is <code>null</code> then all names are accepted.
	 * Otherwise, a name satisfies the filter if and only if the value
	 * <code>true</code> results when the <code>{@link
	 * FilenameFilter#accept}</code> method of the filter is invoked on this
	 * abstract pathname and the name of a file or directory in the directory
	 * that it denotes.
	 *
	 * @param  filter  A filename filter
	 *
	 * @return  An array of strings naming the files and directories in the
	 *          directory denoted by this abstract pathname that were accepted
	 *          by the given <code>filter</code>.  The array will be empty if
	 *          the directory is empty or if no names were accepted by the
	 *          filter.  Returns <code>null</code> if this abstract pathname
	 *          does not denote a directory, or if an I/O error occurs.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the directory
	 */
	EArray<EString*> list(EFilenameFilter *filter = null);
	void list(EArray<EString*> *result, EFilenameFilter *filter = null);

	/**
	 * Returns an array of abstract pathnames denoting the files and
	 * directories in the directory denoted by this abstract pathname that
	 * satisfy the specified filter.  The behavior of this method is the
	 * same as that of the <code>{@link #listFiles()}</code> method, except
	 * that the pathnames in the returned array must satisfy the filter.
	 * If the given <code>filter</code> is <code>null</code> then all
	 * pathnames are accepted.  Otherwise, a pathname satisfies the filter
	 * if and only if the value <code>true</code> results when the
	 * <code>{@link FilenameFilter#accept}</code> method of the filter is
	 * invoked on this abstract pathname and the name of a file or
	 * directory in the directory that it denotes.
	 *
	 * @param  filter  A filename filter
	 *
	 * @return  An array of abstract pathnames denoting the files and
	 *          directories in the directory denoted by this abstract
	 *          pathname.  The array will be empty if the directory is
	 *          empty.  Returns <code>null</code> if this abstract pathname
	 *          does not denote a directory, or if an I/O error occurs.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method denies read access to the directory
	 *
	 * @since 1.2
	 */
	EArray<EFile*> listFiles(EFilenameFilter *filter = null);
	void listFiles(EArray<EFile*> *result, EFilenameFilter *filter = null);

	/**
	 * Returns an array of abstract pathnames denoting the files and
	 * directories in the directory denoted by this abstract pathname that
	 * satisfy the specified filter.  The behavior of this method is the same
	 * as that of the {@link #listFiles()} method, except that the pathnames in
	 * the returned array must satisfy the filter.  If the given {@code filter}
	 * is {@code null} then all pathnames are accepted.  Otherwise, a pathname
	 * satisfies the filter if and only if the value {@code true} results when
	 * the {@link FileFilter#accept FileFilter.accept(File)} method of the
	 * filter is invoked on the pathname.
	 *
	 * @param  filter
	 *         A file filter
	 *
	 * @return  An array of abstract pathnames denoting the files and
	 *          directories in the directory denoted by this abstract pathname.
	 *          The array will be empty if the directory is empty.  Returns
	 *          {@code null} if this abstract pathname does not denote a
	 *          directory, or if an I/O error occurs.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its {@link
	 *          SecurityManager#checkRead(String)} method denies read access to
	 *          the directory
	 *
	 * @since  1.2
	 * @see java.nio.file.Files#newDirectoryStream(Path,java.nio.file.DirectoryStream.Filter)
	 */
	EArray<EFile*> listFiles(EFileFilter *filter);
	void listFiles(EArray<EFile*> *result, EFileFilter *filter);

	/**
	 * Creates the directory named by this abstract pathname.
	 *
	 * @return  <code>true</code> if and only if the directory was
	 *          created; <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method does not permit the named directory to be created
	 */
	boolean mkdir();

	/**
	 * Creates the directory named by this abstract pathname, including any
	 * necessary but nonexistent parent directories.  Note that if this
	 * operation fails it may have succeeded in creating some of the necessary
	 * parent directories.
	 *
	 * @return  <code>true</code> if and only if the directory was created,
	 *          along with all necessary parent directories; <code>false</code>
	 *          otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkRead(java.lang.String)}</code>
	 *          method does not permit verification of the existence of the
	 *          named directory and all necessary parent directories; or if
	 *          the <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method does not permit the named directory and all necessary
	 *          parent directories to be created
	 */
	boolean mkdirs();

	/**
	 * Renames the file denoted by this abstract pathname.
	 *
	 * <p> Many aspects of the behavior of this method are inherently
	 * platform-dependent: The rename operation might not be able to move a
	 * file from one filesystem to another, it might not be atomic, and it
	 * might not succeed if a file with the destination abstract pathname
	 * already exists.  The return value should always be checked to make sure
	 * that the rename operation was successful.
	 *
	 * @param  dest  The new abstract pathname for the named file
	 *
	 * @return  <code>true</code> if and only if the renaming succeeded;
	 *          <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to either the old or new pathnames
	 *
	 * @throws  NullPointerException
	 *          If parameter <code>dest</code> is <code>null</code>
	 */
	boolean renameTo(EFile *dest);
	boolean renameTo(const char *newName);

	/**
	 * Sets the last-modified time of the file or directory named by this
	 * abstract pathname.
	 *
	 * <p> All platforms support file-modification times to the nearest second,
	 * but some provide more precision.  The argument will be truncated to fit
	 * the supported precision.  If the operation succeeds and no intervening
	 * operations on the file take place, then the next invocation of the
	 * <code>{@link #lastModified}</code> method will return the (possibly
	 * truncated) <code>time</code> argument that was passed to this method.
	 *
	 * @param  time  The new last-modified time, measured in milliseconds since
	 *               the epoch (00:00:00 GMT, January 1, 1970)
	 *
	 * @return <code>true</code> if and only if the operation succeeded;
	 *          <code>false</code> otherwise
	 *
	 * @throws  IllegalArgumentException  If the argument is negative
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to the named file
	 *
	 * @since 1.2
	 */
	boolean setLastModified(llong time);

	/**
	 * Marks the file or directory named by this abstract pathname so that
	 * only read operations are allowed. After invoking this method the file
	 * or directory will not change until it is either deleted or marked
	 * to allow write access. On some platforms it may be possible to start the
	 * Java virtual machine with special privileges that allow it to modify
	 * files that are marked read-only. Whether or not a read-only file or
	 * directory may be deleted depends upon the underlying system.
	 *
	 * @return <code>true</code> if and only if the operation succeeded;
	 *          <code>false</code> otherwise
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to the named file
	 *
	 * @since 1.2
	 */
	boolean setReadOnly();

	/**
	 * Sets the owner's or everybody's write permission for this abstract
	 * pathname. On some platforms it may be possible to start the Java virtual
	 * machine with special privileges that allow it to modify files that
	 * disallow write operations.
	 *
	 * <p> The {@link java.nio.file.Files} class defines methods that operate on
	 * file attributes including file permissions. This may be used when finer
	 * manipulation of file permissions is required.
	 *
	 * @param   writable
	 *          If <code>true</code>, sets the access permission to allow write
	 *          operations; if <code>false</code> to disallow write operations
	 *
	 * @param   ownerOnly
	 *          If <code>true</code>, the write permission applies only to the
	 *          owner's write permission; otherwise, it applies to everybody.  If
	 *          the underlying file system can not distinguish the owner's write
	 *          permission from that of others, then the permission will apply to
	 *          everybody, regardless of this value.
	 *
	 * @return  <code>true</code> if and only if the operation succeeded. The
	 *          operation will fail if the user does not have permission to change
	 *          the access permissions of this abstract pathname.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to the named file
	 *
	 * @since 1.6
	 */
	boolean setWritable(boolean writable, boolean ownerOnly=true);

	/**
	 * Sets the owner's or everybody's read permission for this abstract
	 * pathname. On some platforms it may be possible to start the Java virtual
	 * machine with special privileges that allow it to read files that are
	 * marked as unreadable.
	 *
	 * <p> The {@link java.nio.file.Files} class defines methods that operate on
	 * file attributes including file permissions. This may be used when finer
	 * manipulation of file permissions is required.
	 *
	 * @param   readable
	 *          If <code>true</code>, sets the access permission to allow read
	 *          operations; if <code>false</code> to disallow read operations
	 *
	 * @param   ownerOnly
	 *          If <code>true</code>, the read permission applies only to the
	 *          owner's read permission; otherwise, it applies to everybody.  If
	 *          the underlying file system can not distinguish the owner's read
	 *          permission from that of others, then the permission will apply to
	 *          everybody, regardless of this value.
	 *
	 * @return  <code>true</code> if and only if the operation succeeded.  The
	 *          operation will fail if the user does not have permission to
	 *          change the access permissions of this abstract pathname.  If
	 *          <code>readable</code> is <code>false</code> and the underlying
	 *          file system does not implement a read permission, then the
	 *          operation will fail.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to the file
	 *
	 * @since 1.6
	 */
	boolean setReadable(boolean readable, boolean ownerOnly=true);

	/**
	 * Sets the owner's or everybody's execute permission for this abstract
	 * pathname. On some platforms it may be possible to start the Java virtual
	 * machine with special privileges that allow it to execute files that are
	 * not marked executable.
	 *
	 * <p> The {@link java.nio.file.Files} class defines methods that operate on
	 * file attributes including file permissions. This may be used when finer
	 * manipulation of file permissions is required.
	 *
	 * @param   executable
	 *          If <code>true</code>, sets the access permission to allow execute
	 *          operations; if <code>false</code> to disallow execute operations
	 *
	 * @param   ownerOnly
	 *          If <code>true</code>, the execute permission applies only to the
	 *          owner's execute permission; otherwise, it applies to everybody.
	 *          If the underlying file system can not distinguish the owner's
	 *          execute permission from that of others, then the permission will
	 *          apply to everybody, regardless of this value.
	 *
	 * @return  <code>true</code> if and only if the operation succeeded.  The
	 *          operation will fail if the user does not have permission to
	 *          change the access permissions of this abstract pathname.  If
	 *          <code>executable</code> is <code>false</code> and the underlying
	 *          file system does not implement an execute permission, then the
	 *          operation will fail.
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method denies write access to the file
	 *
	 * @since 1.6
	 */
	boolean setExecutable(boolean executable, boolean ownerOnly=true);


	/* -- Temporary files -- */

	//get temp dir
	static const char* getTempDir();

	/**
	 * <p> Creates a new empty file in the specified directory, using the
	 * given prefix and suffix strings to generate its name.  If this method
	 * returns successfully then it is guaranteed that:
	 *
	 * <ol>
	 * <li> The file denoted by the returned abstract pathname did not exist
	 *      before this method was invoked, and
	 * <li> Neither this method nor any of its variants will return the same
	 *      abstract pathname again in the current invocation of the virtual
	 *      machine.
	 * </ol>
	 *
	 * This method provides only part of a temporary-file facility.  To arrange
	 * for a file created by this method to be deleted automatically, use the
	 * <code>{@link #deleteOnExit}</code> method.
	 *
	 * <p> The <code>prefix</code> argument must be at least three characters
	 * long.  It is recommended that the prefix be a short, meaningful string
	 * such as <code>"hjb"</code> or <code>"mail"</code>.  The
	 * <code>suffix</code> argument may be <code>null</code>, in which case the
	 * suffix <code>".tmp"</code> will be used.
	 *
	 * <p> To create the new file, the prefix and the suffix may first be
	 * adjusted to fit the limitations of the underlying platform.  If the
	 * prefix is too long then it will be truncated, but its first three
	 * characters will always be preserved.  If the suffix is too long then it
	 * too will be truncated, but if it begins with a period character
	 * (<code>'.'</code>) then the period and the first three characters
	 * following it will always be preserved.  Once these adjustments have been
	 * made the name of the new file will be generated by concatenating the
	 * prefix, five or more internally-generated characters, and the suffix.
	 *
	 * <p> If the <code>directory</code> argument is <code>null</code> then the
	 * system-dependent default temporary-file directory will be used.  The
	 * default temporary-file directory is specified by the system property
	 * <code>java.io.tmpdir</code>.  On UNIX systems the default value of this
	 * property is typically <code>"/tmp"</code> or <code>"/var/tmp"</code>; on
	 * Microsoft Windows systems it is typically <code>"C:\\WINNT\\TEMP"</code>.  A different
	 * value may be given to this system property when the Java virtual machine
	 * is invoked, but programmatic changes to this property are not guaranteed
	 * to have any effect upon the temporary directory used by this method.
	 *
	 * @param  prefix     The prefix string to be used in generating the file's
	 *                    name; must be at least three characters long
	 *
	 * @param  directory  The directory in which the file is to be created, or
	 *                    <code>null</code> if the default temporary-file
	 *                    directory is to be used
	 *
	 * @return  An abstract pathname denoting a newly-created empty file
	 *
	 * @throws  IllegalArgumentException
	 *          If the <code>prefix</code> argument contains fewer than three
	 *          characters
	 *
	 * @throws  IOException  If a file could not be created
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its <code>{@link
	 *          java.lang.SecurityManager#checkWrite(java.lang.String)}</code>
	 *          method does not allow a file to be created
	 *
	 * @since 1.2
	 */
	static EFile createTempFile(const char* prefix,
			const char* directory = NULL) THROWS2(EIllegalArgumentException, EIOException);

	/* -- Basic infrastructure -- */

	/**
	 * Compares two abstract pathnames lexicographically.  The ordering
	 * defined by this method depends upon the underlying system.  On UNIX
	 * systems, alphabetic case is significant in comparing pathnames; on Microsoft Windows
	 * systems it is not.
	 *
	 * @param   pathname  The abstract pathname to be compared to this abstract
	 *                    pathname
	 *
	 * @return  Zero if the argument is equal to this abstract pathname, a
	 *          value less than zero if this abstract pathname is
	 *          lexicographically less than the argument, or a value greater
	 *          than zero if this abstract pathname is lexicographically
	 *          greater than the argument
	 *
	 * @since   1.2
	 */
	virtual int compareTo(EFile* pathname);

	/**
	 * Tests this abstract pathname for equality with the given object.
	 * Returns <code>true</code> if and only if the argument is not
	 * <code>null</code> and is an abstract pathname that denotes the same file
	 * or directory as this abstract pathname.  Whether or not two abstract
	 * pathnames are equal depends upon the underlying system.  On UNIX
	 * systems, alphabetic case is significant in comparing pathnames; on Microsoft Windows
	 * systems it is not.
	 *
	 * @param   obj   The object to be compared with this abstract pathname
	 *
	 * @return  <code>true</code> if and only if the objects are the same;
	 *          <code>false</code> otherwise
	 */
	boolean equals(EFile* obj);
	virtual boolean equals(EObject* obj);

	/**
	 * Computes a hash code for this abstract pathname.  Because equality of
	 * abstract pathnames is inherently system-dependent, so is the computation
	 * of their hash codes.  On UNIX systems, the hash code of an abstract
	 * pathname is equal to the exclusive <em>or</em> of the hash code
	 * of its pathname string and the decimal value
	 * <code>1234321</code>.  On Microsoft Windows systems, the hash
	 * code is equal to the exclusive <em>or</em> of the hash code of
	 * its pathname string converted to lower case and the decimal
	 * value <code>1234321</code>.  Locale is not taken into account on
	 * lowercasing the pathname string.
	 *
	 * @return  A hash code for this abstract pathname
	 */
	virtual int hashCode();

	/**
	 * Returns the pathname string of this abstract pathname.  This is just the
	 * string returned by the <code>{@link #getPath}</code> method.
	 *
	 * @return  The string form of this abstract pathname
	 */
	virtual EString toString();

private:
	/**
	 * This abstract pathname's normalized pathname string.  A normalized
	 * pathname string uses the default name-separator character and does not
	 * contain any duplicate or redundant separators.
	 *
	 * @serial
	 */
	EString path;
	EString absolutePath;
	EString canonicalPath;

	EString dir;
	EString name;
	EString fname;
	EString ext;

	void init(const char *pathname);

	EString makeAbsolutePath();
	EString makeCanonicalPath();
};

} /* namespace efc */
#endif /* EFILE_HH_ */
