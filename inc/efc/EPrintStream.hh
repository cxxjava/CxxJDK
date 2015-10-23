/*
 * EPrintStream.hh
 *
 *  Created on: 2013-3-16
 *      Author: Administrator
 */

#ifndef EPRINTSTREAM_HH_
#define EPRINTSTREAM_HH_

#include "EFilterOutputStream.hh"
#include "ESynchronizeable.hh"
#include "ESimpleLock.hh"

namespace efc {

/**
 * A <code>PrintStream</code> adds functionality to another output stream,
 * namely the ability to print representations of various data values
 * conveniently.  Two other features are provided as well.  Unlike other output
 * streams, a <code>PrintStream</code> never throws an
 * <code>IOException</code>; instead, exceptional situations merely set an
 * internal flag that can be tested via the <code>checkError</code> method.
 * Optionally, a <code>PrintStream</code> can be created so as to flush
 * automatically; this means that the <code>flush</code> method is
 * automatically invoked after a byte array is written, one of the
 * <code>println</code> methods is invoked, or a newline character or byte
 * (<code>'\n'</code>) is written.
 *
 * <p> All characters printed by a <code>PrintStream</code> are converted into
 * bytes using the platform's default character encoding.  The <code>{@link
 * PrintWriter}</code> class should be used in situations that require writing
 * characters rather than bytes.
 *
 * @author     Frank Yellin
 * @author     Mark Reinhold
 * @since      JDK1.0
 */

class EPrintStream: public EFilterOutputStream {
public:
	virtual ~EPrintStream();

	/**
	 * Creates a new print stream.
	 *
	 * @param  out        The output stream to which values and objects will be
	 *                    printed
	 * @param  autoFlush  A boolean; if true, the output buffer will be flushed
	 *                    whenever a byte array is written, one of the
	 *                    <code>println</code> methods is invoked, or a newline
	 *                    character or byte (<code>'\n'</code>) is written
	 *
	 * @see java.io.PrintWriter#PrintWriter(java.io.OutputStream, boolean)
	 */
	EPrintStream(EOutputStream *out, boolean autoFlush=false);

	/**
	 * Writes <code>len</code> bytes from the specified byte array starting at
	 * offset <code>off</code> to this stream.  If automatic flushing is
	 * enabled then the <code>flush</code> method will be invoked.
	 *
	 * <p> Note that the bytes will be written as given; to write characters
	 * that will be translated according to the platform's default character
	 * encoding, use the <code>print(char)</code> or <code>println(char)</code>
	 * methods.
	 *
	 * @param  b   A byte array
	 * @param  len   Number of bytes to write
	 */
	void write(const void *b, int len) THROWS(EIOException);

	/**
	 * Flushes the stream.  This is done by writing any buffered output bytes to
	 * the underlying output stream and then flushing that stream.
	 *
	 * @see        java.io.OutputStream#flush()
	 */
	void flush() THROWS(EIOException);

	/**
	 * Closes the stream.  This is done by flushing the stream and then closing
	 * the underlying output stream.
	 *
	 * @see        java.io.OutputStream#close()
	 */
	void close() THROWS(EIOException);

	/**
	 * Flushes the stream and checks its error state. The internal error state
	 * is set to <code>true</code> when the underlying output stream throws an
	 * <code>IOException</code> other than <code>InterruptedIOException</code>,
	 * and when the <code>setError</code> method is invoked.  If an operation
	 * on the underlying output stream throws an
	 * <code>InterruptedIOException</code>, then the <code>PrintStream</code>
	 * converts the exception back into an interrupt by doing:
	 * <pre>
	 *     Thread.currentThread().interrupt();
	 * </pre>
	 * or the equivalent.
	 *
	 * @return <code>true</code> if and only if this stream has encountered an
	 *         <code>IOException</code> other than
	 *         <code>InterruptedIOException</code>, or the
	 *         <code>setError</code> method has been invoked
	 */
	boolean checkError();

	/* Methods that do not terminate lines */

	/**
	 * A convenience method to write a formatted string to this output stream
	 * using the specified format string and arguments.
	 *
	 * <p> An invocation of this method of the form <tt>out.printf(format,
	 * args)</tt> behaves in exactly the same way as the invocation
	 *
	 * <pre>
	 *     out.format(format, args) </pre>
	 *
	 * @param  format
	 *         A format string as described in <a
	 *         href="../util/Formatter.html#syntax">Format string syntax</a>
	 *
	 * @param  args
	 *         Arguments referenced by the format specifiers in the format
	 *         string.  If there are more arguments than format specifiers, the
	 *         extra arguments are ignored.  The number of arguments is
	 *         variable and may be zero.  The maximum number of arguments is
	 *         limited by the maximum dimension of a Java array as defined by
	 *         the <a href="http://java.sun.com/docs/books/vmspec/">Java
	 *         Virtual Machine Specification</a>.  The behaviour on a
	 *         <tt>null</tt> argument depends on the <a
	 *         href="../util/Formatter.html#syntax">conversion</a>.
	 *
	 * @throws  IllegalFormatException
	 *          If a format string contains an illegal syntax, a format
	 *          specifier that is incompatible with the given arguments,
	 *          insufficient arguments given the format string, or other
	 *          illegal conditions.  For specification of all possible
	 *          formatting errors, see the <a
	 *          href="../util/Formatter.html#detail">Details</a> section of the
	 *          formatter class specification.
	 *
	 * @throws  NullPointerException
	 *          If the <tt>format</tt> is <tt>null</tt>
	 *
	 * @return  This output stream
	 *
	 * @since  1.5
	 */
	EPrintStream& print(const char* fmt, ...) THROWS(EIOException);
	EPrintStream& println() THROWS(EIOException);
	EPrintStream& println(const char* fmt, ...) THROWS(EIOException);

private:
	EOutputStream *m_Out;
	boolean m_autoFlush;// = false;
	boolean m_trouble;// = false;
	boolean m_closing;// = false; /* To avoid recursive closing */

	ESimpleLock m_lock;

	/** Check to make sure that the stream has not been closed */
	void ensureOpen() THROWS(EIOException);
};

} /* namespace efc */
#endif /* EPRINTSTREAM_HH_ */
