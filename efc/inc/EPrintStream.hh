/*
 * EPrintStream.hh
 *
 *  Created on: 2013-3-16
 *      Author: cxxjava@163.com
 */

#ifndef EPRINTSTREAM_HH_
#define EPRINTSTREAM_HH_

#include "EFilterOutputStream.hh"
#include "ESynchronizeable.hh"
#include "ESimpleLock.hh"
#include "EFile.hh"
#include "EFileNotFoundException.hh"

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
	EPrintStream(EOutputStream *out, boolean autoFlush=false, boolean owned=false);

	/**
	 * Creates a new print stream, without automatic line flushing, with the
	 * specified file name.  This convenience constructor creates
	 * the necessary intermediate {@link java.io.OutputStreamWriter
	 * OutputStreamWriter}, which will encode characters using the
	 * {@linkplain java.nio.charset.Charset#defaultCharset() default charset}
	 * for this instance of the Java virtual machine.
	 *
	 * @param  fileName
	 *         The name of the file to use as the destination of this print
	 *         stream.  If the file exists, then it will be truncated to
	 *         zero size; otherwise, a new file will be created.  The output
	 *         will be written to the file and is buffered.
	 *
	 * @throws  FileNotFoundException
	 *          If the given file object does not denote an existing, writable
	 *          regular file and a new regular file of that name cannot be
	 *          created, or if some other error occurs while opening or
	 *          creating the file
	 *
	 * @throws  SecurityException
	 *          If a security manager is present and {@link
	 *          SecurityManager#checkWrite checkWrite(fileName)} denies write
	 *          access to the file
	 *
	 * @since  1.5
	 */
	explicit
	EPrintStream(const char* fileName) THROWS(EFileNotFoundException);

	/**
	 * Creates a new print stream, without automatic line flushing, with the
	 * specified file.  This convenience constructor creates the necessary
	 * intermediate {@link java.io.OutputStreamWriter OutputStreamWriter},
	 * which will encode characters using the {@linkplain
	 * java.nio.charset.Charset#defaultCharset() default charset} for this
	 * instance of the Java virtual machine.
	 *
	 * @param  file
	 *         The file to use as the destination of this print stream.  If the
	 *         file exists, then it will be truncated to zero size; otherwise,
	 *         a new file will be created.  The output will be written to the
	 *         file and is buffered.
	 *
	 * @throws  FileNotFoundException
	 *          If the given file object does not denote an existing, writable
	 *          regular file and a new regular file of that name cannot be
	 *          created, or if some other error occurs while opening or
	 *          creating the file
	 *
	 * @throws  SecurityException
	 *          If a security manager is present and {@link
	 *          SecurityManager#checkWrite checkWrite(file.getPath())}
	 *          denies write access to the file
	 *
	 * @since  1.5
	 */
	explicit
	EPrintStream(EFile* file) THROWS(EFileNotFoundException);

	// TODO:
	EPrintStream(const EPrintStream& that);
	EPrintStream& operator= (const EPrintStream& that);

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
	virtual void write(const void *b, int len) THROWS(EIOException);

	/**
	 * Flushes the stream.  This is done by writing any buffered output bytes to
	 * the underlying output stream and then flushing that stream.
	 *
	 * @see        java.io.OutputStream#flush()
	 */
	virtual void flush() THROWS(EIOException);

	/**
	 * Closes the stream.  This is done by flushing the stream and then closing
	 * the underlying output stream.
	 *
	 * @see        java.io.OutputStream#close()
	 */
	virtual void close() THROWS(EIOException);

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
	EPrintStream& printf(const char* fmt, ...) THROWS(EIOException);

	/**
	 * Prints a boolean value.  The string produced by <code>{@link
	 * java.lang.String#valueOf(boolean)}</code> is translated into bytes
	 * according to the platform's default character encoding, and these bytes
	 * are written in exactly the manner of the
	 * <code>{@link #write(int)}</code> method.
	 *
	 * @param      b   The <code>boolean</code> to be printed
	 */
	EPrintStream& print(boolean b);

	/**
	 * Prints a character.  The character is translated into one or more bytes
	 * according to the platform's default character encoding, and these bytes
	 * are written in exactly the manner of the
	 * <code>{@link #write(int)}</code> method.
	 *
	 * @param      c   The <code>char</code> to be printed
	 */
	EPrintStream& print(char c);

	/**
	 * Prints an integer.  The string produced by <code>{@link
	 * java.lang.String#valueOf(int)}</code> is translated into bytes
	 * according to the platform's default character encoding, and these bytes
	 * are written in exactly the manner of the
	 * <code>{@link #write(int)}</code> method.
	 *
	 * @param      i   The <code>int</code> to be printed
	 * @see        java.lang.Integer#toString(int)
	 */
	EPrintStream& print(int i);

	/**
	 * Prints a long integer.  The string produced by <code>{@link
	 * java.lang.String#valueOf(long)}</code> is translated into bytes
	 * according to the platform's default character encoding, and these bytes
	 * are written in exactly the manner of the
	 * <code>{@link #write(int)}</code> method.
	 *
	 * @param      l   The <code>long</code> to be printed
	 * @see        java.lang.Long#toString(long)
	 */
	EPrintStream& print(llong l);

	/**
	 * Prints a floating-point number.  The string produced by <code>{@link
	 * java.lang.String#valueOf(float)}</code> is translated into bytes
	 * according to the platform's default character encoding, and these bytes
	 * are written in exactly the manner of the
	 * <code>{@link #write(int)}</code> method.
	 *
	 * @param      f   The <code>float</code> to be printed
	 * @see        java.lang.Float#toString(float)
	 */
	EPrintStream& print(float f);

	/**
	 * Prints a double-precision floating-point number.  The string produced by
	 * <code>{@link java.lang.String#valueOf(double)}</code> is translated into
	 * bytes according to the platform's default character encoding, and these
	 * bytes are written in exactly the manner of the <code>{@link
	 * #write(int)}</code> method.
	 *
	 * @param      d   The <code>double</code> to be printed
	 * @see        java.lang.Double#toString(double)
	 */
	EPrintStream& print(double d);

	/**
	 * Prints an array of characters.  The characters are converted into bytes
	 * according to the platform's default character encoding, and these bytes
	 * are written in exactly the manner of the
	 * <code>{@link #write(int)}</code> method.
	 *
	 * @param      s   The array of chars to be printed
	 *
	 * @throws  NullPointerException  If <code>s</code> is <code>null</code>
	 */
	EPrintStream& print(const char* s, int len=-1);

	/* Methods that do terminate lines */

	/**
	 * Terminates the current line by writing the line separator string.  The
	 * line separator string is defined by the system property
	 * <code>line.separator</code>, and is not necessarily a single newline
	 * character (<code>'\n'</code>).
	 */
	EPrintStream& println() THROWS(EIOException);

	/**
	 *
	 */
	EPrintStream& printfln(const char* fmt, ...) THROWS(EIOException);

	/**
	 * Prints a boolean and then terminate the line.  This method behaves as
	 * though it invokes <code>{@link #print(boolean)}</code> and then
	 * <code>{@link #println()}</code>.
	 *
	 * @param x  The <code>boolean</code> to be printed
	 */
	EPrintStream& println(boolean x);

	/**
	 * Prints a character and then terminate the line.  This method behaves as
	 * though it invokes <code>{@link #print(char)}</code> and then
	 * <code>{@link #println()}</code>.
	 *
	 * @param x  The <code>char</code> to be printed.
	 */
	EPrintStream& println(char x);

	/**
	 * Prints an integer and then terminate the line.  This method behaves as
	 * though it invokes <code>{@link #print(int)}</code> and then
	 * <code>{@link #println()}</code>.
	 *
	 * @param x  The <code>int</code> to be printed.
	 */
	EPrintStream& println(int x);

	/**
	 * Prints a long and then terminate the line.  This method behaves as
	 * though it invokes <code>{@link #print(long)}</code> and then
	 * <code>{@link #println()}</code>.
	 *
	 * @param x  a The <code>long</code> to be printed.
	 */
	EPrintStream& println(llong x);

	/**
	 * Prints a float and then terminate the line.  This method behaves as
	 * though it invokes <code>{@link #print(float)}</code> and then
	 * <code>{@link #println()}</code>.
	 *
	 * @param x  The <code>float</code> to be printed.
	 */
	EPrintStream& println(float x);

	/**
	 * Prints a double and then terminate the line.  This method behaves as
	 * though it invokes <code>{@link #print(double)}</code> and then
	 * <code>{@link #println()}</code>.
	 *
	 * @param x  The <code>double</code> to be printed.
	 */
	EPrintStream& println(double x);

	/**
	 * Prints an array of characters and then terminate the line.  This method
	 * behaves as though it invokes <code>{@link #print(char[])}</code> and
	 * then <code>{@link #println()}</code>.
	 *
	 * @param x  an array of chars to print.
	 */
	EPrintStream& println(const char* x, int len=-1);

private:
	boolean m_autoFlush;// = false;
	boolean m_trouble;// = false;

	ESimpleLock m_lock;

	/** Check to make sure that the stream has not been closed */
	void ensureOpen() THROWS(EIOException);
};

} /* namespace efc */
#endif /* EPRINTSTREAM_HH_ */
