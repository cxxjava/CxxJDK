/*
 * EFileChannel.cpp
 *
 *  Created on: 2014-2-15
 *      Author: cxxjava@163.com
 */

#include "../inc/EFileChannel.hh"
#include "../inc/EFileKey.hh"
#include "../inc/EFileDispatcher.hh"
#include "../inc/ESelectableChannel.hh"
#include "../../inc/ELLong.hh"
#include "../../inc/EIOStatus.hh"
#include "../../inc/EInteger.hh"
#include "../../inc/EThread.hh"
#include "../../inc/ENullPointerException.hh"
#include "../../inc/EFileNotFoundException.hh"
#include "../../inc/EIllegalArgumentException.hh"
#include "../inc/EClosedChannelException.hh"
#include "../inc/ENonWritableChannelException.hh"
#include "../inc/ENonReadableChannelException.hh"

namespace efc {
namespace nio {

volatile boolean EFileChannel::transferSupported_ = true;
//volatile boolean EFileChannel::pipeSupported_ = true;
volatile boolean EFileChannel::fileSupported_ = true;

EFileChannel::~EFileChannel() {
	delete threads_;
	if (pfile_) {
		eso_fclose(pfile_);
	}
}

EFileChannel* EFileChannel::open(int fd, boolean readable, boolean writable,
		boolean append) {
	return new EFileChannel(fd, readable, writable, append);
}

EFileChannel* EFileChannel::open(const char* filename, boolean readable,
		boolean writable, boolean append) {
	ES_ASSERT(filename);

	return new EFileChannel(filename, readable, writable, append);
}

int EFileChannel::read(EIOByteBuffer* dst) {
	ensureOpen();
	if (!readable_)
		throw ENonReadableChannelException(__FILE__, __LINE__);
	SYNCBLOCK(&positionLock_) {
		int n = 0;
		int ti = -1;
		int rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			ti = threads_->add();
			do {
				//@see: n = IOUtil.read(fd, dst, -1, nd, positionLock);
				n = EFileDispatcher::read(fd_, dst);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch (...) {
			finally {
				threads_->remove(ti);
				end(n > 0);
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(n > 0);
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv;
    }}
}

long EFileChannel::read(EA<EIOByteBuffer*>* dsts, int offset, int length) {
	if (dsts == null)
		throw ENULLPOINTEREXCEPTION;
	if ((offset < 0) || (length < 0) || (offset > dsts->length() - length))
		throw EINDEXOUTOFBOUNDSEXCEPTION;

	ensureOpen();
	if (!readable_)
		throw ENonReadableChannelException(__FILE__, __LINE__);
	SYNCBLOCK(&positionLock_) {
		long n = 0;
		int ti = -1;
		long rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			ti = threads_->add();
			do {
				//@see: n = IOUtil.read(fd, dsts, nd);
				n = EFileDispatcher::read(fd_, dsts, offset, length);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch(...) {
			finally {
				threads_->remove(ti);
				end(n > 0);
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(n > 0);
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv;
    }}
}

long EFileChannel::read(EA<EIOByteBuffer*>* dsts) {
	return read(dsts, 0, dsts->length());
}

int EFileChannel::write(EIOByteBuffer* src) {
	ensureOpen();
	if (!writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);
	SYNCBLOCK(&positionLock_) {
		int n = 0;
		int ti = -1;
		int rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			ti = threads_->add();
			if (appending_)
				EFileDispatcher::position(fd_, 0, ES_SEEK_END);
			do {
				//@see: n = IOUtil.write(fd, src, -1, nd, positionLock);
				n = EFileDispatcher::write(fd_, src);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch(...) {
			finally {
				threads_->remove(ti);
				end(n > 0);
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(n > 0);
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv;
    }}
}

long EFileChannel::write(EA<EIOByteBuffer*>* srcs, int offset, int length) {
	if (srcs == null)
		throw ENULLPOINTEREXCEPTION;
	if ((offset < 0) || (length < 0) || (offset > srcs->length() - length))
		throw EINDEXOUTOFBOUNDSEXCEPTION;

	ensureOpen();
	if (!writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);
	SYNCBLOCK(&positionLock_) {
		long n = 0;
		int ti = -1;
		long rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			ti = threads_->add();
			if (appending_)
				EFileDispatcher::position(fd_, 0, ES_SEEK_END);
			do {
				//@see: n = IOUtil.write(fd, srcs, nd);
				n = EFileDispatcher::write(fd_, srcs, offset, length);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch(...) {

			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(n > 0);
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv;
    }}
}

long EFileChannel::write(EA<EIOByteBuffer*>* srcs) {
	return write(srcs, 0, srcs->length());
}

long EFileChannel::position() {
	ensureOpen();
	SYNCBLOCK(&positionLock_) {
		long p = -1;
		int ti = -1;
		int rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			ti = threads_->add();
			do {
				p = EFileDispatcher::position(fd_, 0, ES_SEEK_CUR);
			} while ((p == EIOStatus::INTERRUPTED) && isOpen());
			rv =  EIOStatus::normalize(p);
			goto FINALLY;
		} catch(...) {
			finally {
				threads_->remove(ti);
				end(p > -1);
				ES_ASSERT(EIOStatus::check(p));
			}
			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(p > -1);
			ES_ASSERT(EIOStatus::check(p));
		}
		return rv; //!
    }}
}

EFileChannel* EFileChannel::position(long newPosition) {
	ensureOpen();
	if (newPosition < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	SYNCBLOCK(&positionLock_) {
		long p = -1;
		int ti = -1;
		EFileChannel* rv = null;
		try {
			begin();
			if (!isOpen()) {
				rv = null;
				goto FINALLY;
			}
			ti = threads_->add();
			do {
				p  = EFileDispatcher::position(fd_, newPosition, ES_SEEK_SET);
			} while ((p == EIOStatus::INTERRUPTED) && isOpen());
			rv = this;
			goto FINALLY;
		} catch(...) {
			finally {
				threads_->remove(ti);
				end(p > -1);
				ES_ASSERT(EIOStatus::check(p));
			}
			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(p > -1);
			ES_ASSERT(EIOStatus::check(p));
		}
		return rv; //!
    }}
}

long EFileChannel::size() {
	ensureOpen();
	SYNCBLOCK(&positionLock_) {
		long s = -1;
		int ti = -1;
		long rv = -1;
		try {
			begin();
			if (!isOpen()) {
				rv = -1;
				goto FINALLY;
			}
			ti = threads_->add();
			do {
				s = EFileDispatcher::size(fd_);
			} while ((s == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(s);
			goto FINALLY;
		} catch(...) {
			finally {
				threads_->remove(ti);
				end(s > -1);
				ES_ASSERT(EIOStatus::check(s));
			}
			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(s > -1);
			ES_ASSERT(EIOStatus::check(s));
		}
		return rv; //!
    }}
}

EFileChannel* EFileChannel::truncate(long _size) {
	ensureOpen();
	if (_size < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	if (_size > size())
		return this;
	if (!writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);
	SYNCBLOCK(&positionLock_) {
		int rv = -1;
		long p = -1;
		int ti = -1;
		EFileChannel* rc = null;
		try {
			begin();
			if (!isOpen()) {
				rc = null;
				goto FINALLY;
			}
			ti = threads_->add();

			// get current position
			do {
				p = EFileDispatcher::position(fd_, 0, ES_SEEK_CUR);
			} while ((p == EIOStatus::INTERRUPTED) && isOpen());
			if (!isOpen()) {
				rc = null;
				goto FINALLY;
			}
			ES_ASSERT(p >= 0);

			// truncate file
			do {
				rv = EFileDispatcher::truncate(fd_, _size);
			} while ((rv == EIOStatus::INTERRUPTED) && isOpen());
			if (!isOpen()) {
				rc = null;
				goto FINALLY;
			}

			// set position to size if greater than size
			if (p > _size)
				p = _size;
			do {
				rv = (int)EFileDispatcher::position(fd_, p, ES_SEEK_SET);
			} while ((rv == EIOStatus::INTERRUPTED) && isOpen());
			rc = this;
			goto FINALLY;
		} catch(...) {
			finally {
				threads_->remove(ti);
				end(rv > -1);
				ES_ASSERT(EIOStatus::check(rv));
			}
			throw; //!
		}
		FINALLY:
		finally {
			threads_->remove(ti);
			end(rv > -1);
			ES_ASSERT(EIOStatus::check(rv));
		}
		return rc; //!
    }}
}

void EFileChannel::force(boolean metaData) {
	ensureOpen();
	int rv = -1;
	int ti = -1;
	try {
		begin();
		if (!isOpen())
			goto FINALLY;
		ti = threads_->add();
		do {
			rv = EFileDispatcher::force(fd_, metaData);
		} while ((rv == EIOStatus::INTERRUPTED) && isOpen());
	} catch(...) {
		finally {
			threads_->remove(ti);
			end(rv > -1);
			ES_ASSERT(EIOStatus::check(rv));
		}
		throw; //!
	}
	FINALLY:
	finally {
		threads_->remove(ti);
		end(rv > -1);
		ES_ASSERT(EIOStatus::check(rv));
	}
}

long EFileChannel::transferTo(long position, long count, EByteChannel* target) {
	ensureOpen();
	if (!target->isOpen())
		throw EClosedChannelException(__FILE__, __LINE__);
	if (!readable_)
		throw ENonReadableChannelException(__FILE__, __LINE__);
	if (instanceof<EFileChannel>(target)) {
		if (!(dynamic_cast<EFileChannel*>(target))->writable_)
			throw ENonWritableChannelException(__FILE__, __LINE__);
	}
	if ((position < 0) || (count < 0))
		throw EIllegalArgumentException(__FILE__, __LINE__);
	long sz = size();
	if (count == 0 || position >= sz)
		return 0;
	int icount = (int)ES_MIN(count, EInteger::MAX_VALUE);
	if ((sz - position) < icount)
		icount = (int)(sz - position);

	long n;

	// Attempt a direct transfer, if the kernel supports it
	if ((n = transferToDirectly(position, icount, target)) >= 0)
		return n;

	// Attempt a mapped transfer, but only to trusted channel types
	if ((n = transferToTrustedChannel(position, icount, target)) >= 0)
		return n;

	// Slow path for untrusted targets
	return transferToArbitraryChannel(position, icount, target);
}

long EFileChannel::transferFrom(EByteChannel* src, long position, long count) {
	ensureOpen();
	if (!src->isOpen())
		throw EClosedChannelException(__FILE__, __LINE__);
	if (!writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);
	if ((position < 0) || (count < 0))
		throw EIllegalArgumentException(__FILE__, __LINE__);
	if (position > size())
		return 0;

	if (instanceof<EFileChannel>(src))
	   return transferFromFileChannel(dynamic_cast<EFileChannel*>(src), position, count);

	return transferFromArbitraryChannel(src, position, count);
}

int EFileChannel::read(EIOByteBuffer* dst, long position) {
	if (dst == null)
		throw ENullPointerException(__FILE__, __LINE__);
	if (position < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative position");
	if (!readable_)
		throw ENonReadableChannelException(__FILE__, __LINE__);
	ensureOpen();
	int n = 0;
	int ti = -1;
	int rv = -1;
	try {
		begin();
		if (!isOpen()) {
			rv = -1;
			goto FINALLY;
		}
		ti = threads_->add();
		do {
			//@see: n = IOUtil.read(fd, dst, position, nd, positionLock);
			n = EFileDispatcher::pread(fd_, position, dst, &positionLock_);
		} while ((n == EIOStatus::INTERRUPTED) && isOpen());
		rv = EIOStatus::normalize(n);
		goto FINALLY;
	} catch(...) {
		finally {
			threads_->remove(ti);
			end(n > 0);
			ES_ASSERT(EIOStatus::check(n));
		}
		throw; //!
	}
	FINALLY:
	finally {
		threads_->remove(ti);
		end(n > 0);
		ES_ASSERT(EIOStatus::check(n));
	}
	return rv;
}

int EFileChannel::write(EIOByteBuffer* src, long position) {
	if (src == null)
		throw ENullPointerException(__FILE__, __LINE__);
	if (position < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative position");
	if (!writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);;
	ensureOpen();
	int n = 0;
	int ti = -1;
	int rv = -1;
	try {
		begin();
		if (!isOpen()) {
			rv = -1;
			goto FINALLY;
		}
		ti = threads_->add();
		do {
			//@see: n = IOUtil.write(fd, src, position, nd, positionLock);
			n = EFileDispatcher::pwrite(fd_, position, src, &positionLock_);
		} while ((n == EIOStatus::INTERRUPTED) && isOpen());
		rv = EIOStatus::normalize(n);
		goto FINALLY;
	} catch(...) {

		throw; //!
	}
	FINALLY:
	finally {
		threads_->remove(ti);
		end(n > 0);
		ES_ASSERT(EIOStatus::check(n));
	}
	return rv;
}

int EFileChannel::getFDVal() {
	return fd_;
}

EMappedByteBuffer* EFileChannel::map(EMapMode mode, long position, long count) {
	ensureOpen();

	if (position < 0L)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative position");
	if (count < 0L)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative size");
	if (position + count < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Position + size overflow");
	if (count > EInteger::MAX_VALUE)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Size exceeds Integer.MAX_VALUE");

	if ((mode != READ_ONLY) && !writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);
	if (!readable_)
		throw ENonReadableChannelException(__FILE__, __LINE__);

	EMappedByteBuffer* mbb = null;
	int ti = -1;
	try {
		begin();
		if (!isOpen()) {
			mbb = null;
			goto FINALLY;
		}
		ti = threads_->add();

		if (size() < position + count) { // Extend file size
			if (!writable_) {
				throw EIOException(__FILE__, __LINE__, "Channel not open for writing - cannot extend file to required size");
			}
			int rv;
			do {
				rv = EFileDispatcher::truncate(fd_, position + count);
			} while ((rv == EIOStatus::INTERRUPTED) && isOpen());
		}
		mbb = new EMappedByteBuffer(mode, fd_, position, count);
	} catch(...) {
		finally {
			threads_->remove(ti);
			end(mbb ? true : false);
		}
		throw; //!
	}
	FINALLY:
	finally {
		threads_->remove(ti);
		end(mbb ? true : false);
	}
	return mbb;
}

EFileLock* EFileChannel::lock(long position, long size, boolean shared) {
	ensureOpen();
	if (shared && !readable_)
		throw ENonReadableChannelException(__FILE__, __LINE__);
	if (!shared && !writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);
	EFileLock* fli = new EFileLock(this, position, size, shared);
	boolean i = true;
	int ti = -1;
	try {
		begin();
		if (!isOpen()) {
			finally {
				delete fli;
				end(i);
			}
			return null;
		}
		ti = threads_->add();
		int result = EFileDispatcher::lock(fd_, true, position, size, shared);
//		if (result == RET_EX_LOCK) {
//			assert shared;
//			FileLockImpl fli2 = new FileLockImpl(this, position, size,
//												 false);
//			flt.replace(fli, fli2);
//			return fli2;
//		}
		if (result == EFileDispatcher::INTERRUPTED || result == EFileDispatcher::NO_LOCK) {
			i = false;
		}
	} catch (EIOException& e) {
		finally {
			delete fli;
			threads_->remove(ti);
			end(i);
		}
		throw e; //!
	} finally {
		threads_->remove(ti);
		end(i);
	}
	return fli;
}

EFileLock* EFileChannel::lock() {
	return lock(0L, ES_LONG_MAX_VALUE, false);
}

EFileLock* EFileChannel::tryLock(long position, long size, boolean shared) {
	ensureOpen();
	if (shared && !readable_)
		throw ENonReadableChannelException(__FILE__, __LINE__);
	if (!shared && !writable_)
		throw ENonWritableChannelException(__FILE__, __LINE__);
	EFileLock* fli = new EFileLock(this, position, size, shared);
	int result = EFileDispatcher::lock(fd_, false, position, size, shared);
	if (result == EFileDispatcher::NO_LOCK) {
		delete fli;
		return null;
	}
//	if (result == RET_EX_LOCK) {
//		assert shared;
//		FileLockImpl fli2 = new FileLockImpl(this, position, size,
//											 false);
//		flt.replace(fli, fli2);
//		return fli2;
//	}
	return fli;
}

EFileLock* EFileChannel::tryLock() {
	return tryLock(0L, ES_LONG_MAX_VALUE, false);
}

void EFileChannel::implCloseChannel() {
	/* preClose() removed in jdk8!
	 * @see: openjdk-8/src/share/classes/sun/nio/ch/FileChannelImpl.java#L128
	 */
	//EFileDispatcher::preClose(fd_);

	threads_->signal();

	/* when fd_ is from pfile_ then need close here, otherwise closed in parent!
	 */
	//EFileDispatcher::close(fd_);

	if (pfile_) {
		eso_fclose(pfile_);
		pfile_ = NULL;
	}
}

EFileChannel::EFileChannel(int fd, boolean readable, boolean writable,
		boolean append) :
		fd_(fd), pfile_(null), readable_(readable), writable_(writable), appending_(
				append) {
	threads_ = new ENativeThreadSet(2);
}

EFileChannel::EFileChannel(const char* filename, boolean readable,
		boolean writable, boolean append) :
		readable_(readable), writable_(writable), appending_(append) {
	pfile_ = eso_fopen(filename, "r+");
	if (!pfile_) {
		if (writable_ && (errno == ENOENT)) {
			pfile_ = eso_fopen(filename, "w+");
		}
		if (!pfile_) {
			throw EFileNotFoundException(__FILE__, __LINE__);
		}
	}
	fd_ = eso_fileno(pfile_);

	threads_ = new ENativeThreadSet(2);
}

void EFileChannel::release(EFileLock* fli) {
	ensureOpen();
	EFileDispatcher::release(fd_, fli->position(), fli->size());
}

void EFileChannel::ensureOpen() {
	if (!isOpen())
		throw EClosedChannelException(__FILE__, __LINE__);
}

long EFileChannel::transferToDirectly(long position, int icount, EByteChannel* target) {
#ifdef WIN32
	return EIOStatus::UNSUPPORTED;
#else
	if (!transferSupported_)
		return EIOStatus::UNSUPPORTED;

	int targetFD = -1;
	if (instanceof<EFileChannel>(target)) {
		if (!fileSupported_)
			return EIOStatus::UNSUPPORTED_CASE;
		EFileChannel* fc = dynamic_cast<EFileChannel*>(target);
		targetFD = fc->getFDVal();
		if (fc->appending_) {
			fc->position(fc->size());
		}
	} else if (instanceof<ESelectableChannel>(target)) {
		// Direct transfer to pipe causes EINVAL on some configurations
//		if ((target instanceof SinkChannelImpl) && !pipeSupported)
//			return IOStatus.UNSUPPORTED_CASE;
		targetFD = (dynamic_cast<ESelectableChannel*>(target))->getFDVal();
	}
	if (targetFD == -1)
		return EIOStatus::UNSUPPORTED;
	int thisFDVal = fd_;
	int targetFDVal = targetFD;
	if (thisFDVal == targetFDVal) // Not supported on some configurations
		return EIOStatus::UNSUPPORTED;

	long n = -1;
	int ti = -1;
	long rv = -1;
	try {
		begin();
		if (!isOpen()) {
			rv = -1;
			goto FINALLY;
		}
		ti = threads_->add();
		do {
			n = EFileDispatcher::transferTo(thisFDVal, position, icount, targetFDVal);
		} while ((n == EIOStatus::INTERRUPTED) && isOpen());
		if (n == EIOStatus::UNSUPPORTED_CASE) {
//			if (target instanceof SinkChannelImpl)
//				pipeSupported = false;
			if (instanceof<EFileChannel>(target))
				fileSupported_ = false;
			return EIOStatus::UNSUPPORTED_CASE;
		}
		if (n == EIOStatus::UNSUPPORTED) {
			// Don't bother trying again
			transferSupported_ = false;
			return EIOStatus::UNSUPPORTED;
		}
		rv = EIOStatus::normalize(n);
		goto FINALLY;
	} catch(...) {
		finally {
			threads_->remove(ti);
			end (n > -1);
		}
		throw; //!
	}
	FINALLY:
	finally {
		threads_->remove(ti);
		end (n > -1);
	}
	return rv;
#endif
}

long EFileChannel::transferToTrustedChannel(long position, int icount, EByteChannel* target) {
	if ( !(instanceof<EFileChannel>(target)
			|| instanceof<ESelectableChannel>(target)))
		return EIOStatus::UNSUPPORTED;

	// Trusted target: Use a mapped buffer
	EMappedByteBuffer* dbb = null;
	long rv;
	try {
		dbb = map(READ_ONLY, position, icount);
		// ## Bug: Closing this channel will not terminate the write
		rv = target->write(dbb);
	} catch(...) {
		finally {
			delete dbb;
		}
		throw; //!
	} finally {
		delete dbb;
	}
	return rv;
}

long EFileChannel::transferToArbitraryChannel(long position, int icount, EByteChannel* target) {
	//TODO...

	return EIOStatus::UNSUPPORTED_CASE;
}

long EFileChannel::transferFromFileChannel(EFileChannel* src, long position, long count) {
	// Note we could loop here to accumulate more at once
	SYNCBLOCK(&src->positionLock_) {
		long p = src->position();
		int icount = (int)ES_MIN(ES_MIN(count, EInteger::MAX_VALUE),
								   src->size() - p);
		// ## Bug: Closing this channel will not terminate the write
		EMappedByteBuffer* bb = src->map(READ_ONLY, p, icount);
		long n;
		try {
			n = write(bb, position);
			src->position(p + n);
		} catch(...) {
			finally {
				delete bb;
			}
			throw; //!
		} finally {
			delete bb;
		}
		return n;
    }}
}

long EFileChannel::transferFromArbitraryChannel(EByteChannel* src, long position, long count) {
	//TODO...

	return EIOStatus::UNSUPPORTED_CASE;
}

} /* namespace nio */
} /* namespace efc */
