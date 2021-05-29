/*
 * EFile.cpp
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#include "EFile.hh"
#include "ESystem.hh"
#include "ENullPointerException.hh"

#ifdef WIN32
# include <windows.h>
# include <direct.h>
# include <sys/stat.h>
# include <io.h>
#else
# include <dirent.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <utime.h>
# include <unistd.h>
#endif

namespace efc {

#ifdef WIN32
const char EFile::separatorChar = '\\';
const char* EFile::separator = "\\";
const char EFile::pathSeparatorChar = ';';
const char* EFile::pathSeparator = ";";
#else
const char EFile::separatorChar = '/';
const char* EFile::separator = "/";
const char EFile::pathSeparatorChar = ':';
const char* EFile::pathSeparator = ":";
#endif

#ifdef WIN32
#define FSTAT    _fstat
#define STAT     _stat
#define ACCESS   _access
#define MKDIR(a) _mkdir((a))
#else
#define FSTAT    fstat
#define STAT     stat
#define ACCESS   access
#define MKDIR(a) mkdir((a),0755)
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

class FileStat {
public:
	/**
	 * Constructs a new \c FileStat for the given \c File.
	 */
	FileStat(EFile* file) {
		fpath = file->getAbsolutePath();
		#ifdef WIN32
		fpath.rtrim('\\');
		#endif

		init();
	}

	/**
	 * Determines whether the \c File represents a directory.
	 */
	boolean isDirectory() {
		return _isDirectory;
	}

	/**
	 * Determines whether the \c File represents a regular file or a link to a regular file.
	 */
	boolean isFile() {
		return _isFile;
	}

	/**
	 * Determines whether the \c File represents a symbolic link.
	 */
	boolean isLink() {
#ifndef WIN32
		struct STAT info;
		if (lstat(fpath.c_str(), &info) == 0) {
			_isLink = S_ISLNK(info.st_mode);
		}
#endif
		return _isLink;
	}

	/**
	 * returns the file's size in bytes.
	 */
	llong length() {
		return _length;
	}

	/**
	 * Returns the file's last modification date as unix timestamp.
	 */
	llong lastModified() {
		return _mTime;
	}

	/**
	 * If this represents a symbolic link, returns the link destination path.
	 */
	const char* readlink() {
#ifndef WIN32
		if (this->isLink()) {
			char buf[ES_PATH_MAX];
			int l = ::readlink(fpath.c_str(), buf, ES_PATH_MAX - 1);

			if (l != -1) {
				buf[l] = '\0';
				_linkDest = buf;
			}
		}
#endif
		return _linkDest.c_str();
	}

	/**
	 * Returns a inode number of the given file.
	 */
	llong inode() {
		return _inode;
	}

private:
	EString fpath;
	llong _length;
	boolean _isFile;
	boolean _isDirectory;
	boolean _isLink;

	llong _aTime;
	llong _mTime;

	llong _inode;

	EString _linkDest;

	void init() {
		_isFile = false;
		_isDirectory = false;
		_length = -1;
		_aTime = 0;
		_mTime = 0;
		_isLink = false;

		struct STAT info;
		int v = STAT(fpath.c_str(), &info);
		if (v == 0) {
			_length = info.st_size;
			_isFile = S_ISREG(info.st_mode);
			_isDirectory = S_ISDIR(info.st_mode);

			_aTime = (llong) info.st_atime * LLONG(1000);
			_mTime = (llong) info.st_mtime * LLONG(1000);

			_inode = (llong) info.st_ino;
		}
	}
};

class FileAccess {
public:
	FileAccess(EFile* file) {
		fpath = file->getAbsolutePath();
#ifdef WIN32
		fpath.rtrim('\\');
#endif //!
	}

	boolean canRead() {
#ifdef WIN32
		return true;
#else
		return (ACCESS(fpath.c_str(), R_OK) == 0);
#endif
	}

	boolean canWrite() {
#ifdef WIN32
		return true;
#else
		return (ACCESS(fpath.c_str(), W_OK) == 0);
#endif
	}

	boolean canExecute() {
#ifdef WIN32
		return true;
#else
		return (ACCESS(fpath.c_str(), X_OK) == 0);
#endif
	}

	boolean setLastModified(llong time) {
		boolean rv = false;
#ifdef WIN32
		HANDLE h;
		h = CreateFile(fpath.c_str(),
						FILE_WRITE_ATTRIBUTES,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_BACKUP_SEMANTICS,
						0);
		if (h != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER modTime;
			FILETIME t;
			modTime.QuadPart = (time + 11644473600000L) * 10000L;
			t.dwLowDateTime = (DWORD)modTime.LowPart;
			t.dwHighDateTime = (DWORD)modTime.HighPart;
			if (SetFileTime(h, NULL, NULL, &t)) {
				rv = true;
			}
			CloseHandle(h);
		}
#else
		struct STAT sb;
		if (STAT(fpath.c_str(), &sb) == 0) {
			struct timeval tv[2];

			/* Preserve access time */
			tv[0].tv_sec = sb.st_atime;
			tv[0].tv_usec = 0;

			/* Change last-modified time */
			tv[1].tv_sec = time / 1000;
			tv[1].tv_usec = (time % 1000) * 1000;

			if (utimes(fpath.c_str(), tv) == 0)
				rv = true;
		}
#endif
		return rv;
	}

	boolean setReadOnly() {
		boolean rv = false;
#ifdef WIN32
		DWORD a = GetFileAttributes(fpath.c_str());
		if ((a != INVALID_FILE_ATTRIBUTES) &&
			((a & FILE_ATTRIBUTE_DIRECTORY) == 0)) {
			if (SetFileAttributes(fpath.c_str(), a | FILE_ATTRIBUTE_READONLY))
				rv = true;
		}
#else
		struct STAT sb;
		if (STAT(fpath.c_str(), &sb) == 0) {
			if (::chmod(fpath.c_str(), sb.st_mode & ~(S_IWUSR | S_IWGRP | S_IWOTH)) >= 0) {
				rv = true;
			}
		}
#endif
		return rv;
	}

	boolean setWritable(boolean writable, boolean ownerOnly) {
		return setPermission(ACCESS_READ, writable, ownerOnly);
	}

	boolean setReadable(boolean readable, boolean ownerOnly) {
		return setPermission(ACCESS_WRITE, readable, ownerOnly);
	}

	boolean setExecutable(boolean executable, boolean ownerOnly) {
		return setPermission(ACCESS_EXECUTE, executable, ownerOnly);
	}

private:
	EString fpath;
	enum {
		ACCESS_READ,
		ACCESS_WRITE,
		ACCESS_EXECUTE
	};

	boolean setPermission(int access, boolean enable, boolean owneronly) {
		boolean rv = false;

#ifdef WIN32
		if (access == ACCESS_READ ||
			access == ACCESS_EXECUTE) {
			return enable;
		}
		DWORD a = GetFileAttributes(fpath.c_str());
		if ((a != INVALID_FILE_ATTRIBUTES) &&
			((a & FILE_ATTRIBUTE_DIRECTORY) == 0))
		{
			if (enable)
				a =  a & ~FILE_ATTRIBUTE_READONLY;
			else
				a =  a | FILE_ATTRIBUTE_READONLY;
			if (SetFileAttributes(fpath.c_str(), a))
				rv = true;
		}
#else
		int amode = 0;
		switch (access) {
		case ACCESS_READ:
			if (owneronly)
				amode = S_IRUSR;
			else
				amode = S_IRUSR | S_IRGRP | S_IROTH;
			break;
		case ACCESS_WRITE:
			if (owneronly)
				amode = S_IWUSR;
			else
				amode = S_IWUSR | S_IWGRP | S_IWOTH;
			break;
		case ACCESS_EXECUTE:
			if (owneronly)
				amode = S_IXUSR;
			else
				amode = S_IXUSR | S_IXGRP | S_IXOTH;
			break;
		default:
			ES_ASSERT(0);
			break;
		}
		struct STAT sb;
		if (STAT(fpath.c_str(), &sb) == 0) {
			if (enable)
				sb.st_mode |= amode;
			else
				sb.st_mode &= ~amode;
			if (::chmod(fpath.c_str(), sb.st_mode) >= 0) {
				rv = true;
			}
		}
#endif

		return rv;
	}
};

//=============================================================================

EFile::EFile(const char *pathname) {
	init(pathname);
}

EFile::EFile(EFile* file) {
	if (file == null) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	this->path = file->path;
	this->absolutePath = file->absolutePath;
	this->canonicalPath = file->canonicalPath;
	this->dir = file->dir;
	this->name = file->name;
	this->fname = file->fname;
	this->ext = file->ext;
}

EFile::EFile(const char *parent, const char *child) {
	ES_ASSERT(child);

	if (parent && *parent) {
		es_string_t *new_path = NULL;
		if (parent[eso_strlen(parent) - 1] == separatorChar) {
			eso_msprintf(&new_path, "%s%s", parent, child);
		} else {
			eso_msprintf(&new_path, "%s%c%s", parent, separatorChar, child);
		}
		init(new_path);
		eso_mfree(new_path);
	} else {
		init(child);
	}
}

EFile::EFile(EFile *parent, const char *child) {
	ES_ASSERT(child);

	if (parent) {
		EString pparent = parent->getPath();
		if (!pparent.isEmpty()) {
			es_string_t *new_path = NULL;
			if (pparent.endsWith(separator)) {
				eso_msprintf(&new_path, "%s%s", pparent.c_str(), child);
			} else {
				eso_msprintf(&new_path, "%s%c%s", pparent.c_str(), separatorChar,
						child);
			}
			init(new_path);
			eso_mfree(new_path);
		} else {
			init(child);
		}
	} else {
		init(child);
	}
}

int EFile::compareTo(EFile* pathname) {
	if (!pathname) return -1;
	return this->canonicalPath.compareTo(&pathname->canonicalPath);
}

boolean EFile::equals(EFile* obj) {
	if (obj != null) {
		return compareTo(obj) == 0;
	}
	return false;
}

boolean EFile::equals(EObject* obj) {
	EFile* that = dynamic_cast<EFile*>(obj);
	if (that != null) {
		return compareTo(that) == 0;
	}
	return false;
}

int EFile::hashCode() {
	return canonicalPath.hashCode() ^ 1234321;
}

llong EFile::inode() {
	return FileStat(this).inode();
}

boolean EFile::canRead() {
	return FileAccess(this).canRead();
}

boolean EFile::canWrite() {
	return FileAccess(this).canWrite();
}

boolean EFile::canExecute() {
	return FileAccess(this).canExecute();
}

void EFile::init(const char *pathname) {
	ES_ASSERT(pathname);

	EString tempStr(pathname);
#ifdef WIN32
	tempStr.replace('/', '\\');
#else
	tempStr.replace('\\', '/');
#endif
	this->path = tempStr.c_str();

	absolutePath = makeAbsolutePath();
	canonicalPath = makeCanonicalPath();

	// Split directory and file name:
	if (canonicalPath.length() > 0) {
		int slashPos = canonicalPath.lastIndexOf(separatorChar);
		if (slashPos != -1) {
			name = canonicalPath.substring(slashPos + 1);
			dir = canonicalPath.substring(0, slashPos + 1);
		} else {
			name = canonicalPath;
		}

		// Extract extension:
		int dotPos = name.lastIndexOf((int)'.');
		if (dotPos != -1) {
			ext = name.substring(dotPos);
			fname = name.substring(0, dotPos);
		} else {
			fname = name.c_str();
		}
	}
}

EString& EFile::getPath()
{
	return path;
}

EString& EFile::getDir()
{
	return dir;
}

EString& EFile::getName()
{
	return name;
}

EString& EFile::getFName()
{
	return fname;
}

EString& EFile::getExt()
{
	return ext;
}

EString EFile::getParent() {
	int index = path.lastIndexOf(separatorChar);
	if (index > 0) {
		return path.substring(0, index);
	} else {
		return EString();
	}
}

EFile EFile::getParentFile() {
	return EFile(this->getParent().c_str());
}

EString EFile::readlink()
{
	return EString(FileStat(this).readlink());
}

boolean EFile::isDirectory() {
	return FileStat(this).isDirectory();
}

boolean EFile::isFile() {
	return FileStat(this).isFile();
}

boolean EFile::isLink() {
	return FileStat(this).isLink();
}

llong EFile::length() {
	return FileStat(this).length();
}

llong EFile::lastModified() {
	return FileStat(this).lastModified();
}

boolean EFile::remove() {
#if defined(_MSC_VER)
	if (isFile()) {
		return (DeleteFile(path.c_str()) != 0);
	}
	else if (isDirectory()) {
		return (RemoveDirectory(path.c_str()) != 0);
	}
#else
	if (isFile() || isLink()) {
		return (::remove(path.c_str()) == 0);
	} else {
		return (::rmdir(path.c_str()) == 0);
	}
#endif

	return false;
}

boolean EFile::mkdir() {
	if (!path.isEmpty() && ::ACCESS(path.c_str(), 0)) {
		if (::MKDIR(path.c_str()) == 0) {
			return true;
		}
	}
	return false;
}

boolean EFile::mkdirs() {
	return (eso_mkdir(path.c_str()) == 0);
}

EString EFile::toString() {
	return path;
}

EString EFile::getAbsolutePath() {
	return absolutePath;
}

EString EFile::makeAbsolutePath() {
	if (isAbsolute()) {
		return path;
	}

	char cwd[ES_PATH_MAX];

#ifdef WIN32
	if (GetCurrentDirectory(ES_PATH_MAX, cwd) == 0)
	return path;
#else
	if (getcwd(cwd, ES_PATH_MAX) == null)
		return path;
#endif

	EString dir(cwd);

	if (path.length() == 0)
		return dir;
	if (dir.endsWith(separator) || path.startsWith(separator))
		return dir.concat(path.c_str());
	else
		return dir.concat(separator).concat(path.c_str());
}

EString& EFile::getCanonicalPath() {
	return canonicalPath;
}

EString EFile::makeCanonicalPath() {
	char resolved_path[ES_PATH_MAX];
	EString path = getAbsolutePath();
	canonicalize((char*)path.c_str(), resolved_path, (int)sizeof(resolved_path));
	return resolved_path;
}

boolean EFile::isAbsolute() {
#ifdef WIN32
	return (path.length() >= 3
	          && path.charAt(1) == ':'
	          && path.charAt(2) == '\\');
#else
	return (path.startsWith(separator));
#endif
}

boolean EFile::renameTo(EFile *dest) {
	ES_ASSERT(dest);
	return (::rename(path.c_str(), dest->getCanonicalPath().c_str()) == 0);
}

boolean EFile::renameTo(const char *newName) {
	return (::rename(path.c_str(), newName) == 0);
}

boolean EFile::setLastModified(llong time) {
	return FileAccess(this).setLastModified(time);
}

boolean EFile::setReadOnly() {
	return FileAccess(this).setReadOnly();
}

boolean EFile::setWritable(boolean writable, boolean ownerOnly) {
	return FileAccess(this).setWritable(writable, ownerOnly);
}

boolean EFile::setReadable(boolean readable, boolean ownerOnly) {
	return FileAccess(this).setReadable(readable, ownerOnly);
}

boolean EFile::setExecutable(boolean executable, boolean ownerOnly) {
	return FileAccess(this).setExecutable(executable, ownerOnly);
}

boolean EFile::exists() {
	struct STAT buf;
	return (STAT(path.c_str(), &buf) == 0);
}

EArray<EString*> EFile::list(EFilenameFilter *filter) {
	EArray<EString*> result;
	list(&result, filter);
	return result;
}

void EFile::list(EArray<EString*> *result, EFilenameFilter *filter) {
	ES_ASSERT(result);

	if (!isDirectory())
		return;

	EString pattern(path.c_str());
	if (path.length() > 0 && !path.endsWith(separator))
		pattern.concat(separator);

#ifdef WIN32

	HANDLE dir;
	WIN32_FIND_DATA dirent;

	if ((dir = FindFirstFile(pattern.concat("*").c_str(), &dirent)) != INVALID_HANDLE_VALUE) {
		do {
			if (dirent.cFileName
					&& (eso_strcmp(dirent.cFileName, ".") == 0
							|| eso_strcmp(dirent.cFileName, "..") == 0)) {
				continue;
			}

			if (filter == null || filter->accept(null, dirent.cFileName)) {
				result->add(new EString(dirent.cFileName));
			}
		}
		while(FindNextFile(dir, &dirent));

		FindClose(dir);
	}
#else
	DIR* dir = opendir(pattern.c_str());

	if (dir == null) {
		return;
	} else {
		struct dirent* entry;

		while ((entry = readdir(dir)) != null) {
			if (entry->d_name
					&& (eso_strcmp(entry->d_name, ".") == 0
							|| eso_strcmp(entry->d_name, "..") == 0)) {
				continue;
			}
			if (filter == null || filter->accept(null, entry->d_name)) {
				result->add(new EString(entry->d_name));
			}
		}

		closedir(dir);
	}
#endif
}

EArray<EFile*> EFile::listFiles(EFilenameFilter *filter) {
	EArray<EFile*> result;
	listFiles(&result, filter);
	return result;
}

void EFile::listFiles(EArray<EFile*> *result, EFilenameFilter *filter) {
	ES_ASSERT(result);

	EArray<EString*> files;
	list(&files, filter);
	if (files.size() == 0) {
		return;
	}
	for (int i = 0; i < files.size(); i++) {
		const char *filename = files.getAt(i)->c_str();
		result->add(new EFile(this, filename));
	}
}

EArray<EFile*> EFile::listFiles(EFileFilter *filter) {
	EArray<EFile*> result;
	listFiles(&result, filter);
	return result;
}

void EFile::listFiles(EArray<EFile*> *result, EFileFilter *filter) {
	ES_ASSERT(result);

	EArray<EString*> files;
	list(&files, null);
	if (files.size() == 0) {
		return;
	}
	for (int i = 0; i < files.size(); i++) {
		const char *filename = files.getAt(i)->c_str();
		EFile* f = new EFile(this, filename);
		if (filter == null || filter->accept(f)) {
			result->add(f);
		} else {
			delete f;
		}
	}
}

boolean EFile::createNewFile() THROWS(EIOException) {
	es_int32_t r = eso_fcreate(path.c_str(), false);
	if (r == 0) {
		return true;
	}
	else {
		if (errno == EEXIST) {
			return false;
		}
		else {
			throw EIOException(__FILE__, __LINE__, errno);
		}
	}
}

const char* EFile::getTempDir() {
	return ESystem::getTempPath();
}

EFile EFile::createTempFile(const char* prefix, const char* directory)
		THROWS2(EIllegalArgumentException, EIOException)
{
	char *ppath = NULL;

	char resolved_path[ES_PATH_MAX] = {0};
	if (directory) {
		EString tempStr(directory);
#ifdef WIN32
		tempStr.replace('/', '\\');
#else
		tempStr.replace('\\', '/');
		tempStr.concat("/");
#endif
		ppath = eso_realpath(tempStr.c_str(), resolved_path,
				sizeof(resolved_path));
		if (!ppath) {
			throw EIllegalArgumentException(__FILE__, __LINE__, directory);
		}
	}
	if (!ppath) {
		ppath = (char*)getTempDir();
	}

#ifdef WIN32
	char outpath[ES_PATH_MAX];
	int r = GetTempFileName(ppath, prefix ? prefix : "", 0, outpath);
	if (r == 0) {
		throw EIOException(__FILE__, __LINE__, ppath);
	}
	else {
		return EFile(outpath);
	}
#else
	char *outpath = tempnam(ppath, prefix ? prefix : "");
	if (!outpath) {
		throw EIOException(__FILE__, __LINE__, ppath);
	}
	else {
		EString result(outpath);
		free(outpath);
		return EFile(result.c_str());
	}
#endif
}

} /* namespace efc */
