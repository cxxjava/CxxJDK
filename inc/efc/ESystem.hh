#ifndef __ESystem_H__
#define __ESystem_H__

#include "EA.hh"
#include "EConfig.hh"
#include "EString.hh"
#include "EInputStream.hh"
#include "EPrintStream.hh"
#include "ESimpleMap.hh"
#include "EFileOutputStream.hh"
#include "ESharedPtr.hh"

namespace efc {

/**
 * Default configuration file name for this program.
 */
#define DEFAUT_PROPERTY_CONFIG_FILENAME "default.conf"

class ESystem : public EObject {
public:
	DECLARE_STATIC_INITZZ;

public:
	/**
	 * The "standard" input stream. This stream is already
	 * open and ready to supply input data. Typically this stream
	 * corresponds to keyboard input or another input source specified by
	 * the host environment or user.
	 */
	static EInputStream* in;

	/**
	 * The "standard" output stream. This stream is already
	 * open and ready to accept output data. Typically this stream
	 * corresponds to display output or another output destination
	 * specified by the host environment or user.
	 * <p>
	 * For simple stand-alone Java applications, a typical way to write
	 * a line of output data is:
	 * <blockquote><pre>
	 *     System.out.println(data)
	 * </pre></blockquote>
	 * <p>
	 * See the <code>println</code> methods in class <code>PrintStream</code>.
	 *
	 * @see     java.io.PrintStream#println()
	 * @see     java.io.PrintStream#println(boolean)
	 * @see     java.io.PrintStream#println(char)
	 * @see     java.io.PrintStream#println(char[])
	 * @see     java.io.PrintStream#println(int)
	 * @see     java.io.PrintStream#println(long)
	 * @see     java.io.PrintStream#println(java.lang.Object)
	 * @see     java.io.PrintStream#println(java.lang.String)
	 */
	static EPrintStream* out;

	/**
	 * The "standard" error output stream. This stream is already
	 * open and ready to accept output data.
	 * <p>
	 * Typically this stream corresponds to display output or another
	 * output destination specified by the host environment or user. By
	 * convention, this output stream is used to display error messages
	 * or other information that should come to the immediate attention
	 * of a user even if the principal output stream, the value of the
	 * variable <code>out</code>, has been redirected to a file or other
	 * destination that is typically not continuously monitored.
	 */
	static EPrintStream* err;

	/**
	 * Reassigns the "standard" input stream.
	 *
	 * <p>First, if there is a security manager, its <code>checkPermission</code>
	 * method is called with a <code>RuntimePermission("setIO")</code> permission
	 *  to see if it's ok to reassign the "standard" input stream.
	 * <p>
	 *
	 * @param in the new standard input stream.
	 *
	 * @throws SecurityException
	 *        if a security manager exists and its
	 *        <code>checkPermission</code> method doesn't allow
	 *        reassigning of the standard input stream.
	 *
	 * @see SecurityManager#checkPermission
	 * @see java.lang.RuntimePermission
	 *
	 * @since   JDK1.1
	 */
	static void setIn(EInputStream* in);

	/**
	 * Reassigns the "standard" output stream.
	 *
	 * <p>First, if there is a security manager, its <code>checkPermission</code>
	 * method is called with a <code>RuntimePermission("setIO")</code> permission
	 *  to see if it's ok to reassign the "standard" output stream.
	 *
	 * @param out the new standard output stream
	 *
	 * @throws SecurityException
	 *        if a security manager exists and its
	 *        <code>checkPermission</code> method doesn't allow
	 *        reassigning of the standard output stream.
	 *
	 * @see SecurityManager#checkPermission
	 * @see java.lang.RuntimePermission
	 *
	 * @since   JDK1.1
	 */
	static void setOut(EPrintStream* out);

	/**
	 * Reassigns the "standard" error output stream.
	 *
	 * <p>First, if there is a security manager, its <code>checkPermission</code>
	 * method is called with a <code>RuntimePermission("setIO")</code> permission
	 *  to see if it's ok to reassign the "standard" error output stream.
	 *
	 * @param err the new standard error output stream.
	 *
	 * @throws SecurityException
	 *        if a security manager exists and its
	 *        <code>checkPermission</code> method doesn't allow
	 *        reassigning of the standard error output stream.
	 *
	 * @see SecurityManager#checkPermission
	 * @see java.lang.RuntimePermission
	 *
	 * @since   JDK1.1
	 */
	static void setErr(EPrintStream* err);

	/**
	 * Returns the local time zone.
	 *
	 * @return the int value of timezone
	 */
	static int localTimeZone();

	/**
	 * Returns the current time in milliseconds.
	 *
	 * @return  the difference, measured in milliseconds, between the current
	 *          time and midnight, January 1, 1970 UTC.
	 */
	static llong currentTimeMillis();
	static uint  currentTimeSeconds();

	/**
	 * Returns the current value of the running Java Virtual Machine's
	 * high-resolution time source, in nanoseconds.
	 *
	 * <p>This method can only be used to measure elapsed time and is
	 * not related to any other notion of system or wall-clock time.
	 * The value returned represents nanoseconds since some fixed but
	 * arbitrary <i>origin</i> time (perhaps in the future, so values
	 * may be negative).  The same origin is used by all invocations of
	 * this method in an instance of a Java virtual machine; other
	 * virtual machine instances are likely to use a different origin.
	 *
	 * <p>This method provides nanosecond precision, but not necessarily
	 * nanosecond resolution (that is, how frequently the value changes)
	 * - no guarantees are made except that the resolution is at least as
	 * good as that of {@link #currentTimeMillis()}.
	 *
	 * <p>Differences in successive calls that span greater than
	 * approximately 292 years (2<sup>63</sup> nanoseconds) will not
	 * correctly compute elapsed time due to numerical overflow.
	 *
	 * <p>The values returned by this method become meaningful only when
	 * the difference between two such values, obtained within the same
	 * instance of a Java virtual machine, is computed.
	 *
	 * <p> For example, to measure how long some code takes to execute:
	 *  <pre> {@code
	 * long startTime = System.nanoTime();
	 * // ... the code being measured ...
	 * long estimatedTime = System.nanoTime() - startTime;}</pre>
	 *
	 * <p>To compare two nanoTime values
	 *  <pre> {@code
	 * long t0 = System.nanoTime();
	 * ...
	 * long t1 = System.nanoTime();}</pre>
	 *
	 * one should use {@code t1 - t0 < 0}, not {@code t1 < t0},
	 * because of the possibility of numerical overflow.
	 *
	 * @return the current value of the running Java Virtual Machine's
	 *         high-resolution time source, in nanoseconds
	 * @since 1.5
	 */
	static llong nanoTime();

	/**
	 * Returns the current value of the specified environment
	 * variable.
	 */
	static const char* getEnv(const char* name);

	/**
	 * Gets the system property indicated by the specified key.
	 *
	 * @param      key   the name of the system property.
	 * @return     the string value of the system property,
	 *             or <code>null</code> if there is no property with that key.
	 */
	static const char* getProperty(const char* key, const char* defaultValue=null);

	/**
	 * Get the program config full path file name.
	 */
	static const char* getConfigFilename();

	/**
	 * Gets the program config parameter indicated by the specified key.
	 *
	 * @param      key   the name of the program config parameter.
	 * @return     the string value of the program config parameter,
	 *             or <code>null</code> if there is no parameter with that key.
	 */
	static const char* getConfigParameter(const char* key, const char* defaultValue=null) ;

	/**
	 * Gets the program main argument indicated by the specified key.
	 *
	 * @param      key   the name of the program main argument.
	 * @return     the string value of the program main argument,
	 *             or <code>null</code> if there is no argument with that key.
	 */
	static const char* getProgramArgument(const char* key, const char* defaultValue=null);

	/**
	 * Get the program current work path.
	 */
	static const char* getCurrentWorkPath();

	/**
	 * Get the program execute's file name.
	*/
	static const char* getExecuteFilename();

	/**
	 * Get temp path.
	 */
	static const char* getTempPath();

	/**
	 * Initialize the currently running application. Called in main()
	 * function at first.
	 */
	static es_status_t init(char *config=null);
	static es_status_t init(int argc, const char **argv, const char *config=null);

	/**
	 * Detach the process from the controlling terminal.
	 * @param daemonize set to TRUE if the process should daemonize
	 *                  and become a background process, else it will
	 *                  stay in the foreground.
	 * @param in if 0 then close stdin
	 * @param out if 0 then close stdout
	 * @param err if 0 then close stderr
	 */
	static void detach(boolean daemonize, int in, int out, int err);

	/**
	 * Terminates the currently running application. The
	 * argument serves as a status code; by convention, a nonzero
	 * status code indicates abnormal termination.
	 *
	 * @param      status   exit status.
	 * @see exit(status)
	 */
	static void exit(int status);

	/**
	 * Copies an array from the specified source array, beginning at the
	 * specified position, to the specified position of the destination array.
	 * A subsequence of array components are copied from the source
	 * array referenced by <code>src</code> to the destination array
	 * referenced by <code>dest</code>. The number of components copied is
	 * equal to the <code>length</code> argument. The components at
	 * positions <code>srcPos</code> through
	 * <code>srcPos+length-1</code> in the source array are copied into
	 * positions <code>destPos</code> through
	 * <code>destPos+length-1</code>, respectively, of the destination
	 * array.
	 *
	 * @param      src      the source array.
	 * @param      srcPos   starting position in the source array.
	 * @param      dest     the destination array.
	 * @param      destPos  starting position in the destination data.
	 * @param      length   the number of array elements to be copied.
	 * @exception  NullPointerException if either <code>src</code> or
	 *               <code>dest</code> is <code>null</code>.
	 */
	static void arraycopy(void* src, int srcPos,
                          void* dest, int destPos,
                          int length);
	/**
	 *
	 */
	template<typename E>
	static void arraycopy(EA<E>& src, int srcPos,
			              EA<E>& dest, int destPos,
                          int length)
	{
		if ((&src == &dest) && (destPos == srcPos)) { //only the same one.
			return;
		}
		if (srcPos + length > src.length() || destPos + length > dest.length()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}
		int i;
		if (&src == &dest) { //the same one and overlapping.
			if (destPos < srcPos) {
				for (i = 0; i < length; i++) {
					dest[destPos + i] = src[srcPos + i];
				}
			} else {
				for (i = length; i > 0; i--) {
					dest[destPos + i - 1] = src[srcPos + i - 1];
				}
			}
		}
		else {
			for (i=0; i<length; i++) {
				dest[destPos + i] = src[srcPos + i];
			}
		}
	}

	template<typename E>
	static void arraycopy(EA<E*>& src, int srcPos,
						  EA<E*>& dest, int destPos,
						  int length)
	{
		if ((&src == &dest) && (destPos == srcPos)) { //only the same one.
			return;
		}
		if (srcPos + length > src.length() || destPos + length > dest.length()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}
		eso_memmove(dest.address() + destPos, src.address() + srcPos, length*sizeof(E*));
	}

	template<typename E>
	static void arraycopy(EA<sp<E> >& src, int srcPos,
						  EA<sp<E> >& dest, int destPos,
						  int length)
	{
		if ((&src == &dest) && (destPos == srcPos)) { //only the same one.
			return;
		}
		if (srcPos + length > src.length() || destPos + length > dest.length()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}
		int i;
		if (&src == &dest) { //the same one and overlapping.
			if (destPos < srcPos) {
				for (i = 0; i < length; i++) {
					dest[destPos + i] = src[srcPos + i];
				}
			} else {
				for (i = length; i > 0; i--) {
					dest[destPos + i - 1] = src[srcPos + i - 1];
				}
			}
		}
		else {
			for (i=0; i<length; i++) {
				dest[destPos + i] = src[srcPos + i];
			}
		}
	}

	template<typename E>
	static void arraycopy(EA<E>* src, int srcPos,
						  EA<E>* dest, int destPos,
						  int length)
	{
		ES_ASSERT(src && dest);
		if ((src == dest) && (destPos == srcPos)) { //only the same one.
			return;
		}
		if (srcPos + length > src->length() || destPos + length > dest->length()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}
		int i;
		if (src == dest) { //the same one and overlapping.
			if (destPos < srcPos) {
				for (i = 0; i < length; i++) {
					(*dest)[destPos + i] = (*src)[srcPos + i];
				}
			} else {
				for (i = length; i > 0; i--) {
					(*dest)[destPos + i - 1] = (*src)[srcPos + i - 1];
				}
			}
		}
		else {
			for (i=0; i<length; i++) {
				(*dest)[destPos + i] = (*src)[srcPos + i];
			}
		}
	}

	template<typename E>
	static void arraycopy(EA<E*>* src, int srcPos,
						  EA<E*>* dest, int destPos,
						  int length)
	{
		ES_ASSERT(src && dest);
		if ((src == dest) && (destPos == srcPos)) { //only the same one.
			return;
		}
		if (srcPos + length > src->length() || destPos + length > dest->length()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}
		eso_memmove(dest->address() + destPos, src->address() + srcPos, length*sizeof(E*));
	}

	template<typename E>
	static void arraycopy(EA<sp<E> >* src, int srcPos,
						  EA<sp<E> >* dest, int destPos,
						  int length)
	{
		ES_ASSERT(src && dest);
		if ((src == dest) && (destPos == srcPos)) { //only the same one.
			return;
		}
		if (srcPos + length > src->length() || destPos + length > dest->length()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}
		int i;
		if (src == dest) { //the same one and overlapping.
			if (destPos < srcPos) {
				for (i = 0; i < length; i++) {
					(*dest)[destPos + i] = (*src)[srcPos + i];
				}
			} else {
				for (i = length; i > 0; i--) {
					(*dest)[destPos + i - 1] = (*src)[srcPos + i - 1];
				}
			}
		}
		else {
			for (i=0; i<length; i++) {
				(*dest)[destPos + i] = (*src)[srcPos + i];
			}
		}
	}

	/**
	 * Returns the same hash code for the given object as
	 * would be returned by the default method hashCode(),
	 * whether or not the given object's class overrides
	 * hashCode().
	 * The hash code for the null reference is zero.
	 *
	 * @param x object for which the hashCode is to be calculated
	 * @return  the hashCode
	 * @since   JDK1.1
	 */
	static int identityHashCode(EObject* x);

private:
	ESystem(){}
	static void initSysProp();
	static EInputStream* getInput();
	static EPrintStream* getOutput();
	static EPrintStream* getErrput();
	static EInputStream *m_stdin;
	static EFileOutputStream *m_stdout0;
	static EPrintStream *m_stdout;
	static EFileOutputStream *m_stderr0;
	static EPrintStream *m_stderr;
	static EString *m_FileCfg;
	static EConfig *m_Cfg;
	static ESimpleMap *m_Arg;
	static ESimpleMap *m_Prop;
	static es_string_t *m_CurrPath;
	static es_string_t *m_ExecPath;
	static es_string_t *m_TempPath;
};

} /* namespace efc */
#endif //!__ESystem_H__
