/*
 * ESocketChannel.cpp
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#include "../inc/ESocketChannel.hh"
#include "./ESocketAdaptor.hh"
#include "./ENativeThread.hh"
#include "./ESocketDispatcher.hh"
#include "../../inc/ENetWrapper.hh"
#include "../../inc/ESocket.hh"

namespace efc {
namespace nio {

#ifdef WIN32
#define HAVE_SELECT
#else
#define HAVE_POLL
#endif //!WIN32

// State, increases monotonically
#define ST_UNINITIALIZED  -1
#define ST_UNCONNECTED     0
#define ST_PENDING         1
#define ST_CONNECTED       2
#define ST_KILLPENDING     3
#define ST_KILLED          4

ESocketChannel::ESocketChannel() :
		_fd(-1), _socket(null), _localAddress(null), _remoteAddress(
				null), _readerThread(0), _writerThread(0), _isInputOpen(true ), _isOutputOpen(
				true ), _readyToConnect(false ), _state(ST_UNCONNECTED) {
    _fd = ENetWrapper::socket(PF_INET, SOCK_STREAM, 0);
}

ESocketChannel::ESocketChannel(const int fd, EInetSocketAddress* isa) :
		_socket(null), _localAddress(null), _readerThread(
				0), _writerThread(0), _isInputOpen(true ), _isOutputOpen(true ), _readyToConnect(
				false ), _state(ST_CONNECTED) {
	_fd = fd;
	_remoteAddress = new EInetSocketAddress(isa->getAddress(), isa->getPort());
}

ESocketChannel::~ESocketChannel() {
	delete _localAddress;
	delete _remoteAddress;
	delete _socket;
}

ESocketChannel* ESocketChannel::open() {
	return new ESocketChannel();
}

ESocketChannel* ESocketChannel::open(EInetSocketAddress* remote) {
	ESocketChannel* sc = open();
	try {
		sc->connect(remote);
	} catch (...) {
		finally {
			if (!sc->isConnected()) {
				try {
					sc->close();
				} catch (...) {
				}
			}
		}
		throw;
	}
	finally {
		if (!sc->isConnected()) {
			try {
				sc->close();
			} catch (...) {
			}
		}
	}
	ES_ASSERT(sc->isConnected());
	return sc;
}

ESocketChannel* ESocketChannel::open(const char* hostname, int port) {
	EInetSocketAddress remote(hostname, port);
	return open(&remote);
}

int ESocketChannel::validOps() {
	return (ESelectionKey::OP_READ | ESelectionKey::OP_WRITE
			| ESelectionKey::OP_CONNECT);
}

ESocket* ESocketChannel::socket() {
	SYNCBLOCK(&_stateLock) {
		if (_socket == null)
			_socket = ESocketAdaptor::create(this);
		return _socket;
    }}
}

void ESocketChannel::bind(EInetSocketAddress* local) {
	SYNCBLOCK(&_readLock) {
		SYNCBLOCK(&_writeLock) {
			SYNCBLOCK(&_stateLock) {
				ensureOpenAndUnconnected();
				if (_localAddress != null)
					throw EALREADYBOUNDEXCEPTION;
				EInetSocketAddress* isa = ENetWrapper::checkAddress(local);
				ENetWrapper::bind(_fd, isa->getAddress(), isa->getPort());

				int laddr, lport;
				ENetWrapper::localAddress(_fd, &laddr, &lport);
				_localAddress = new EInetSocketAddress(laddr, lport);
            }}
        }}
    }}
}

void ESocketChannel::bind(const char* hostname, int port) {
	EInetSocketAddress local(hostname, port);
	this->bind(&local);
}

boolean ESocketChannel::isConnected() {
	SYNCBLOCK(&_stateLock) {
		return (_state == ST_CONNECTED);
    }}
}

boolean ESocketChannel::isConnectionPending() {
	SYNCBLOCK(&_stateLock) {
		return (_state == ST_PENDING);
    }}
}

boolean ESocketChannel::connect(EInetSocketAddress* remote) {
    int trafficClass = 0;           // ## Pick up from options

    SYNCBLOCK (&_readLock) {
    	SYNCBLOCK (&_writeLock) {
            ensureOpenAndUnconnected();
            EInetSocketAddress* isa = ENetWrapper::checkAddress(remote);
            //SecurityManager sm = System.getSecurityManager();
            //if (sm != null)
            //    sm.checkConnect(isa.getAddress().getHostAddress(), isa.getPort());
            SYNCBLOCK (blockingLock()) {
                int n = 0;
                boolean rv = true;
                try {
                    try {
                        begin();
                        SYNCBLOCK (&_stateLock) {
                            if (!isOpen()) {
                            	rv = false;
                            	goto FINALLY;
                            }
                            _readerThread = ENativeThread::current();
                        }}
                        for (;;) {
                            EInetAddress* ia = remote->getAddress();
                            n = ENetWrapper::connect(_fd,
                                            ia,
                                            isa->getPort(),
                                            trafficClass);
                            if ((n == EIOStatus::INTERRUPTED) && isOpen())
                                continue;
                            break;
                        }
                    } catch (...) {
                    	finally {
                    		readerCleanup();
                    		end((n > 0) || (n == EIOStatus::UNAVAILABLE));
                    	    ES_ASSERT(EIOStatus::check(n));
                    	}
                    	throw; //!
                    }
                    FINALLY:
                    finally {
                        readerCleanup();
                        end((n > 0) || (n == EIOStatus::UNAVAILABLE));
                        ES_ASSERT(EIOStatus::check(n));
                    }
                    if (rv == false) {
                    	return false;
                    }
                } catch (EIOException& x) {
                    // If an exception was thrown, close the channel after
                    // invoking end() so as to avoid bogus
                    // AsynchronousCloseExceptions
                    close();
                    throw x;
                }
                SYNCBLOCK (&_stateLock) {
                    _remoteAddress = new EInetSocketAddress(*isa);
                    if (n > 0) {

                        // Connection succeeded; disallow further
                        // invocation
                        _state = ST_CONNECTED;
                        return true;
                    }
                    // If nonblocking and no exception then connection
                    // pending; disallow another invocation
                    if (!isBlocking())
                        _state = ST_PENDING;
                    else
                    	ES_ASSERT(false);
                }}
            }}
            return false;
        }}
    }}
}

boolean ESocketChannel::connect(const char* hostname, int port) {
	EInetSocketAddress remote(hostname, port);
	return this->connect(&remote);
}

boolean ESocketChannel::finishConnect() {
	SYNCBLOCK (&_readLock) {
		SYNCBLOCK (&_writeLock) {
			SYNCBLOCK (&_stateLock) {
				if (!isOpen())
					throw ECLOSEDCHANNELEXCEPTION;
				if (_state == ST_CONNECTED)
					return true;
				if (_state != ST_PENDING)
					throw ENOCONNECTIONPENDINGEXCEPTION;
            }}
			int n = 0;
			boolean rv = true;
			try {
				try {
					begin();
					SYNCBLOCK(blockingLock()) {
						SYNCBLOCK(&_stateLock) {
							if (!isOpen()) {
								rv = false;
								goto FINALLY;
							}
							_readerThread = ENativeThread::current();
                        }}
						if (!isBlocking()) {
							for (;;) {
								n = ENetWrapper::checkConnect(_fd, false, _readyToConnect);
								if ((n == EIOStatus::INTERRUPTED) && isOpen())
									continue;
								break;
							}
						} else {
							for (;;) {
								n = ENetWrapper::checkConnect(_fd, true, _readyToConnect);
								if (n == 0) {
									// Loop in case of
									// spurious notifications
									continue;
								}
								if ((n == EIOStatus::INTERRUPTED) && isOpen())
									continue;
								break;
							}
						}
                    }}
				} catch (...) {
					finally {
						SYNCBLOCK (&_stateLock) {
							_readerThread = 0;
							if (_state == ST_KILLPENDING) {
								kill();
								// poll()/getsockopt() does not report
								// error (throws exception, with n = 0)
								// on Linux platform after dup2 and
								// signal-wakeup. Force n to 0 so the
								// end() can throw appropriate exception
								n = 0;
							}
                        }}
						end((n > 0) || (n == EIOStatus::UNAVAILABLE));
						ES_ASSERT(EIOStatus::check(n));
					}
					throw; //!
				}
				FINALLY:
				finally {
					SYNCBLOCK (&_stateLock) {
						_readerThread = 0;
						if (_state == ST_KILLPENDING) {
							kill();
							// poll()/getsockopt() does not report
							// error (throws exception, with n = 0)
							// on Linux platform after dup2 and
							// signal-wakeup. Force n to 0 so the
							// end() can throw appropriate exception
							n = 0;
						}
                    }}
					end((n > 0) || (n == EIOStatus::UNAVAILABLE));
					ES_ASSERT(EIOStatus::check(n));
				}
				if (rv == false) {
					return false;
				}
			} catch (EIOException& x) {
				// If an exception was thrown, close the channel after
				// invoking end() so as to avoid bogus
				// AsynchronousCloseExceptions
				close();
				throw x;
			}
			if (n > 0) {
				SYNCBLOCK (&_stateLock) {
					_state = ST_CONNECTED;
                }}
				return true;
			}
			return false;
        }}
    }}
}

int ESocketChannel::read(EIOByteBuffer* buf) {
	if (buf == null)
		throw ENULLPOINTEREXCEPTION;

	SYNCBLOCK (&_readLock) {
		if (!ensureReadOpen())
			return -1;
		int n = 0;
		int rv = -1;
		try {

			// Set up the interruption machinery; see
			// AbstractInterruptibleChannel for details
			//
			begin();

			SYNCBLOCK (&_stateLock) {
				if (!isOpen()) {

					// Either the current thread is already interrupted, so
					// begin() closed the channel, or another thread closed the
					// channel since we checked it a few bytecodes ago.  In
					// either case the value returned here is irrelevant since
					// the invocation of end() in the finally block will throw
					// an appropriate exception.
					//

					rv = 0;
					goto FINALLY;
				}

				// Save this thread so that it can be signalled on those
				// platforms that require it
				//
				_readerThread = ENativeThread::current();
            }}

			// Between the previous test of isOpen() and the return of the
			// IOUtil.read invocation below, this channel might be closed
			// or this thread might be interrupted.  We rely upon the
			// implicit synchronization point in the kernel read() call to
			// make sure that the right thing happens.  In either case the
			// implCloseSelectableChannel method is ultimately invoked in
			// some other thread, so there are three possibilities:
			//
			//   - implCloseSelectableChannel() invokes nd.preClose()
			//     before this thread invokes read(), in which case the
			//     read returns immediately with either EOF or an error,
			//     the latter of which will cause an IOException to be
			//     thrown.
			//
			//   - implCloseSelectableChannel() invokes nd.preClose() after
			//     this thread is blocked in read().  On some operating
			//     systems (e.g., Solaris and Windows) this causes the read
			//     to return immediately with either EOF or an error
			//     indication.
			//
			//   - implCloseSelectableChannel() invokes nd.preClose() after
			//     this thread is blocked in read() but the operating
			//     system (e.g., Linux) doesn't support preemptive close,
			//     so implCloseSelectableChannel() proceeds to signal this
			//     thread, thereby causing the read to return immediately
			//     with IOStatus.INTERRUPTED.
			//
			// In all three cases the invocation of end() in the finally
			// clause will notice that the channel has been closed and
			// throw an appropriate exception (AsynchronousCloseException
			// or ClosedByInterruptException) if necessary.
			//
			// *There is A fourth possibility. implCloseSelectableChannel()
			// invokes nd.preClose(), signals reader/writer thred and quickly
			// moves on to nd.close() in kill(), which does a real close.
			// Then a third thread accepts a new connection, opens file or
			// whatever that causes the released "fd" to be recycled. All
			// above happens just between our last isOpen() check and the
			// next kernel read reached, with the recycled "fd". The solution
			// is to postpone the real kill() if there is a reader or/and
			// writer thread(s) over there "waiting", leave the cleanup/kill
			// to the reader or writer thread. (the preClose() still happens
			// so the connection gets cut off as usual).
			//
			// For socket channels there is the additional wrinkle that
			// asynchronous shutdown works much like asynchronous close,
			// except that the channel is shutdown rather than completely
			// closed.  This is analogous to the first two cases above,
			// except that the shutdown operation plays the role of
			// nd.preClose().
			for (;;) {
				//@see: n = IOUtil.read(fd, buf, -1, nd, readLock);
				n = ESocketDispatcher::read(_fd, buf);
				if ((n == EIOStatus::INTERRUPTED) && isOpen()) {
					// The system call was interrupted but the channel
					// is still open, so retry
					continue;
				}
				rv = EIOStatus::normalize(n);
				goto FINALLY;
			}

		} catch (...) {
			finally {
				readerCleanup();
				end(n > 0 || (n == EIOStatus::UNAVAILABLE));
				SYNCBLOCK(&_stateLock) {
					if ((n <= 0) && (!_isInputOpen))
						return EIOStatus::EOF_;
                }}
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			readerCleanup();        // Clear reader thread
			// The end method, which is defined in our superclass
			// AbstractInterruptibleChannel, resets the interruption
			// machinery.  If its argument is true then it returns
			// normally; otherwise it checks the interrupt and open state
			// of this channel and throws an appropriate exception if
			// necessary.
			//
			// So, if we actually managed to do any I/O in the above try
			// block then we pass true to the end method.  We also pass
			// true if the channel was in non-blocking mode when the I/O
			// operation was initiated but no data could be transferred;
			// this prevents spurious exceptions from being thrown in the
			// rare event that a channel is closed or a thread is
			// interrupted at the exact moment that a non-blocking I/O
			// request is made.
			//
			end(n > 0 || (n == EIOStatus::UNAVAILABLE));

			// Extra case for socket channels: Asynchronous shutdown
			//
			SYNCBLOCK(&_stateLock) {
				if ((n <= 0) && (!_isInputOpen))
					return EIOStatus::EOF_;
            }}
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv; //!
    }}
}

int ESocketChannel::write(EIOByteBuffer* buf) {
	if (buf == null)
		throw ENULLPOINTEREXCEPTION;

	SYNCBLOCK (&_writeLock) {
		ensureWriteOpen();
		int n = 0;
		int rv = -1;
		try {
			begin();
			SYNCBLOCK (&_stateLock) {
				if (!isOpen()) {
					rv = 0;
					goto FINALLY;
				}
				_writerThread = ENativeThread::current();
            }}
			for (;;) {
				//@see: n = IOUtil.write(fd, buf, -1, nd, writeLock);
				n = ESocketDispatcher::write(_fd, buf);
				if ((n == EIOStatus::INTERRUPTED) && isOpen())
					continue;
				rv = EIOStatus::normalize(n);
				goto FINALLY;
			}
		} catch(...) {
			finally {
				writerCleanup();
				end(n > 0 || (n ==EIOStatus::UNAVAILABLE));
				SYNCBLOCK (&_stateLock) {
					if ((n <= 0) && (!_isOutputOpen))
						throw EASYNCHRONOUSCLOSEEXCEPTION;
                }}
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			writerCleanup();
			end(n > 0 || (n == EIOStatus::UNAVAILABLE));
			SYNCBLOCK (&_stateLock) {
				if ((n <= 0) && (!_isOutputOpen))
					throw EASYNCHRONOUSCLOSEEXCEPTION;
            }}
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv; //!
    }}
}

long ESocketChannel::read(EA<EIOByteBuffer*>* dsts) {
	return read(dsts, 0, dsts->length());
}

long ESocketChannel::read(EA<EIOByteBuffer*>* dsts, int offset, int length) {
	if (dsts == null)
		throw ENULLPOINTEREXCEPTION;
	if ((offset < 0) || (length < 0) || (offset > dsts->length() - length))
		throw EINDEXOUTOFBOUNDSEXCEPTION;

	SYNCBLOCK (&_readLock) {
		if (!ensureReadOpen())
			return -1;
		long n = 0;
		long rv = -1;
		try {
			begin();
			SYNCBLOCK (&_stateLock) {
				if (!isOpen()) {
					rv = 0;
					goto FINALLY;
				}
				_readerThread = ENativeThread::current();
            }}

			for (;;) {
				//@see: n = IOUtil.read(fd, bufs, nd);
				n = ESocketDispatcher::read(_fd, dsts, offset, length);
				if ((n == EIOStatus::INTERRUPTED) && isOpen())
					continue;

				rv = EIOStatus::normalize(n);
				goto FINALLY;
			}
		} catch(...) {
			finally {
				readerCleanup();
				end(n > 0 || (n == EIOStatus::UNAVAILABLE));
				SYNCBLOCK (&_stateLock) {
					if ((n <= 0) && (!_isInputOpen))
						return EIOStatus::EOF_;
                }}
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			readerCleanup();
			end(n > 0 || (n == EIOStatus::UNAVAILABLE));
			SYNCBLOCK (&_stateLock) {
				if ((n <= 0) && (!_isInputOpen))
					return EIOStatus::EOF_;
            }}
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv; //!
    }}
}

long ESocketChannel::write(EA<EIOByteBuffer*>* srcs) {
	return write(srcs, 0, srcs->length());
}

long ESocketChannel::write(EA<EIOByteBuffer*>* srcs, int offset, int length) {
	if (srcs == null)
		throw ENULLPOINTEREXCEPTION;
	if ((offset < 0) || (length < 0) || (offset > srcs->length() - length))
		throw EINDEXOUTOFBOUNDSEXCEPTION;

	SYNCBLOCK (&_writeLock) {
		ensureWriteOpen();
		long n = 0;
		long rv = -1;
		try {
			begin();
			SYNCBLOCK (&_stateLock) {
				if (!isOpen()) {
					rv = 0;
					goto FINALLY;
				}
				_writerThread = ENativeThread::current();
            }}
			for (;;) {
				//@see: n = IOUtil.write(fd, bufs, nd);
				n = ESocketDispatcher::write(_fd, srcs, offset, length);
				if ((n == EIOStatus::INTERRUPTED) && isOpen())
					continue;

				rv = EIOStatus::normalize(n);
				goto FINALLY;
			}
		} catch (...) {
			finally {
				writerCleanup();
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				SYNCBLOCK (&_stateLock) {
					if ((n <= 0) && (!_isOutputOpen))
						throw EASYNCHRONOUSCLOSEEXCEPTION;
                }}
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			writerCleanup();
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			SYNCBLOCK (&_stateLock) {
				if ((n <= 0) && (!_isOutputOpen))
					throw EASYNCHRONOUSCLOSEEXCEPTION;
            }}
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv; //!
    }}
}

boolean ESocketChannel::translateAndUpdateReadyOps(int ops, ESelectionKey* sk) {
	return translateReadyOps(ops, sk->nioReadyOps(), sk);
}

boolean ESocketChannel::translateAndSetReadyOps(int ops, ESelectionKey* sk) {
	return translateReadyOps(ops, 0, sk);
}

void ESocketChannel::translateAndSetInterestOps(int ops, ESelectionKey* sk) {
	int newOps = 0;
	if ((ops & ESelectionKey::OP_READ) != 0)
		newOps |= ENetWrapper::POLLIN_;
	if ((ops & ESelectionKey::OP_WRITE) != 0)
		newOps |= ENetWrapper::POLLOUT_;
	if ((ops & ESelectionKey::OP_CONNECT) != 0)
		newOps |= ENetWrapper::POLLCONN_;
	sk->selector()->putEventOps(sk, newOps);
}

void ESocketChannel::kill() {
	SYNCBLOCK(&_stateLock) {
		if (_state == ST_KILLED)
			return;
		if (_state == ST_UNINITIALIZED) {
			_state = ST_KILLED;
			return;
		}
		ES_ASSERT(!isOpen() && !isRegistered());

		// Postpone the kill if there is a waiting reader
		// or writer thread. See the comments in read() for
		// more detailed explanation.
		if (_readerThread == 0 && _writerThread == 0) {
			ESocketDispatcher::close(_fd);
			_state = ST_KILLED;
		} else {
			_state = ST_KILLPENDING;
		}
    }}
}

int ESocketChannel::sendOutOfBandData(byte b) {
	SYNCBLOCK(&_writeLock) {
		ensureWriteOpen();
		int n = 0;
		int rv = -1;
		try {
			begin();
			SYNCBLOCK(&_stateLock) {
				if (!isOpen()) {
					rv = 0;
					goto FINALLY;
				}
				_writerThread = ENativeThread::current();
            }}
			for (;;) {
				//@see: n = sendOutOfBandData(fd, b);
				{
					unsigned char d = b & 0xFF;
					int n = ::send(_fd, (const char*)&d, 1, MSG_OOB);
					if (n == -1) {
						throw ESocketException(__FILE__, __LINE__, errno);
					}
				}
				if ((n == EIOStatus::INTERRUPTED) && isOpen())
					continue;

				rv = EIOStatus::normalize(n);
				goto FINALLY;
			}
		} catch (...) {
			finally {
				writerCleanup();
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				SYNCBLOCK(&_stateLock) {
					if ((n <= 0) && (!_isOutputOpen))
						throw EAsynchronousCloseException(__FILE__, __LINE__);
                }}
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			writerCleanup();
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			SYNCBLOCK(&_stateLock) {
				if ((n <= 0) && (!_isOutputOpen))
					throw EAsynchronousCloseException(__FILE__, __LINE__);
            }}
			ES_ASSERT(EIOStatus::check(n));
		}
		return rv; //!
    }}
}

int ESocketChannel::getFDVal() {
	return _fd;
}

void ESocketChannel::implConfigureBlocking(boolean block) {
	ENetWrapper::configureBlocking(_fd, block);
}

void ESocketChannel::implCloseSelectableChannel() {
	SYNCBLOCK(&_stateLock) {
		_isInputOpen = false;
		_isOutputOpen = false;

		// Close the underlying file descriptor and dup it to a known fd
		// that's already closed.  This prevents other operations on this
		// channel from using the old fd, which might be recycled in the
		// meantime and allocated to an entirely different channel.
		//
		ESocketDispatcher::preClose(_fd);

		// Signal native threads, if needed.  If a target thread is not
		// currently blocked in an I/O operation then no harm is done since
		// the signal handler doesn't actually do anything.
		//
		es_os_thread_t th;
		if ((th = _readerThread) != 0)
			ENativeThread::signal(th);
		if ((th = _writerThread) != 0)
			ENativeThread::signal(th);

		// If this channel is not registered then it's safe to close the fd
		// immediately since we know at this point that no thread is
		// blocked in an I/O operation upon the channel and, since the
		// channel is marked closed, no thread will start another such
		// operation.  If this channel is registered then we don't close
		// the fd since it might be in use by a selector.  In that case
		// closing this channel caused its keys to be cancelled, so the
		// last selector to deregister a key for this channel will invoke
		// kill() to close the fd.
		//
		if (!isRegistered())
			kill();
    }}
}

void ESocketChannel::ensureOpenAndUnconnected() {
	SYNCBLOCK (&_stateLock) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		if (_state == ST_CONNECTED)
			throw EALREADYCONNECTEDEXCEPTION;
		if (_state == ST_PENDING)
			throw ECONNECTIONPENDINGEXCEPTION;
    }}
}

void ESocketChannel::readerCleanup() {
	SYNCBLOCK (&_stateLock) {
		_readerThread = 0;
		if (_state == ST_KILLPENDING)
			kill();
    }}
}

EInetSocketAddress* ESocketChannel::localAddress() {
	SYNCBLOCK (&_stateLock) {
		if (_state == ST_CONNECTED
				&& (_localAddress == null
						|| _localAddress->getAddress()->isAnyLocalAddress())) {
			if (_localAddress) {
				delete _localAddress;
				_localAddress = null;
			}
			// Socket was not bound before connecting or
			// Socket was bound with an "anyLocalAddress"
			int laddr, lport;
			ENetWrapper::localAddress(_fd, &laddr, &lport);
			_localAddress = new EInetSocketAddress(laddr, lport);
		}
		return _localAddress;
    }}
}

EInetSocketAddress* ESocketChannel::remoteAddress() {
	SYNCBLOCK (&_stateLock) {
		return _remoteAddress;
    }}
}

boolean ESocketChannel::isBound() {
	SYNCBLOCK (&_stateLock) {
		if (_state == ST_CONNECTED)
			return true;
		return _localAddress != null;
    }}
}

void ESocketChannel::shutdownInput() {
	SYNCBLOCK (&_stateLock) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
        if (_isInputOpen) {
            ENetWrapper::shutdown(_fd, SHUT_RD_);
            es_os_thread_t th = _readerThread;
            if (th != 0) {
                ENativeThread::signal(th);
            }
            _isInputOpen = false;
        }
    }}
}

void ESocketChannel::shutdownOutput() {
	SYNCBLOCK (&_stateLock) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		_isOutputOpen = false;
		ENetWrapper::shutdown(_fd, SHUT_WR_);
		es_os_thread_t th = _writerThread;
		if (th != 0) {
			ENativeThread::signal(th);
		}
    }}
}

boolean ESocketChannel::isInputOpen() {
	SYNCBLOCK (&_stateLock) {
		return _isInputOpen;
    }}
}

boolean ESocketChannel::isOutputOpen() {
	SYNCBLOCK (&_stateLock) {
		return _isOutputOpen;
    }}
}

boolean ESocketChannel::translateReadyOps(int ops, int initialOps,
		ESelectionKey* sk) {
	int intOps = sk->nioInterestOps(); // Do this just once, it synchronizes
	int oldOps = sk->nioReadyOps();
	int newOps = initialOps;

	if ((ops & ENetWrapper::POLLNVAL_) != 0) {
		// This should only happen if this channel is pre-closed while a
		// selection operation is in progress
		// ## Throw an error if this channel has not been pre-closed
		return false ;
	}

	if ((ops & (ENetWrapper::POLLERR_ | ENetWrapper::POLLHUP_)) != 0) {
		newOps = intOps;
		sk->nioReadyOps(newOps);
		// No need to poll again in checkConnect,
		// the error will be detected there
		_readyToConnect = true;
		return (newOps & ~oldOps) != 0;
	}

	if (((ops & ENetWrapper::POLLIN_) != 0)
			&& ((intOps & ESelectionKey::OP_READ) != 0)
			&& (_state == ST_CONNECTED))
		newOps |= ESelectionKey::OP_READ;

	if (((ops & ENetWrapper::POLLCONN_) != 0)
			&& ((intOps & ESelectionKey::OP_CONNECT) != 0)
			&& ((_state == ST_UNCONNECTED) || (_state == ST_PENDING))) {
		newOps |= ESelectionKey::OP_CONNECT;
		_readyToConnect = true;
	}

	if (((ops & ENetWrapper::POLLOUT_) != 0)
			&& ((intOps & ESelectionKey::OP_WRITE) != 0)
			&& (_state == ST_CONNECTED))
		newOps |= ESelectionKey::OP_WRITE;

	sk->nioReadyOps(newOps);
	return (newOps & ~oldOps) != 0;
}

boolean ESocketChannel::ensureReadOpen() {
	SYNCBLOCK (&_stateLock) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		if (!isConnected())
			throw ENOTYETCONNECTEDEXCEPTION;
		if (!_isInputOpen)
			return false ;
		else
			return true ;
    }}
}

EString ESocketChannel::toString() {
	EString sb;
	sb.append("ESocketChannel[");
	if (!isOpen())
		sb.append("closed");
	else {
		SYNCBLOCK (&_stateLock) {
			switch (_state) {
			case ST_UNCONNECTED:
				sb.append("unconnected");
				break;
			case ST_PENDING:
				sb.append("connection-pending");
				break;
			case ST_CONNECTED:
				sb.append("connected");
				if (!_isInputOpen)
					sb.append(" ishut");
				if (!_isOutputOpen)
					sb.append(" oshut");
				break;
			}
			if (localAddress() != null) {
				sb.append(" local=");
				sb.append(localAddress()->toString());
			}
			if (remoteAddress() != null) {
				sb.append(" remote=");
				sb.append(remoteAddress()->toString());
			}
        }}
	}
	sb.append(']');
	return sb;
}

void ESocketChannel::ensureWriteOpen() {
	SYNCBLOCK(&_stateLock) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		if (!_isOutputOpen)
			throw ECLOSEDCHANNELEXCEPTION;
		if (!isConnected())
			throw ENOTYETCONNECTEDEXCEPTION;
    }}
}

void ESocketChannel::writerCleanup() {
	SYNCBLOCK(&_stateLock) {
		_writerThread = 0;
		if (_state == ST_KILLPENDING)
			kill();
    }}
}

void ESocketChannel::setOption(int optID, const void* optval, int optlen) {
	ENetWrapper::setOption(_fd, optID, (char*)optval, optlen);
}

void ESocketChannel::getOption(int optID, void* optval, int* optlen) {
	ENetWrapper::getOption(_fd, optID, optval, optlen);
}

} /* namespace nio */
} /* namespace efc */
