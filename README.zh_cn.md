# CxxJDK
##山寨JDK实现的C++强大类库.
<br/>
*英文版本: [English](README.md)*

### 目录
  - [特点](#特点)
  - [示例](#示例)
  - [性能](#性能)
  - [特性](#特性)
  - [TODO](#todo)
  - [Support](#support)

####特点
* 跨平台：同时支持Linux32/64、OSX64、Win64等平台；
* 高性能：同时具备C/C++和Java并发库的优点，性能爆棚；
* 类丰富：实现JDK丰富的类库，从此摆脱C++贫类库的困境；
* 易开发：先java开发，再使用`CxxJDK`进行翻译，So easy!

####示例
`java:`

```
public static void main(String[] args) {
	class Worker implements Runnable {

		@Override
		public void run() {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			System.out.println(String.format("thread %d finished", 
				this.hashCode()));
		}
	}

    ThreadPoolExecutor executor = new ThreadPoolExecutor(100, 200, 10, 
			TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>());   
	executor.allowCoreThreadTimeOut(true);
	
	for (int i = 0; i < 200; i++) {   
		executor.execute(new Worker());   
	}
	
	executor.shutdown();
}
```

`c++:`

```
int main(int argc, const char **argv) {
	ESystem::init(argc, argv); //cxxjdk初始化!

	class Worker : public ERunnable {
	public:
		virtual void run() {
			try {   
				EThread::sleep(1000);   
			} catch (EInterruptedException& e) {   
				e.printStackTrace();   
			}   
			ESystem::out->println(EString::formatOf("thread %d finished", 
				this->hashCode()).c_str());
		}
	};

	EThreadPoolExecutor* executor = new EThreadPoolExecutor(100, 200, 10,
			ETimeUnit::SECONDS, new ELinkedBlockingQueue<ERunnable>());
	executor->allowCoreThreadTimeOut(true);
	
	for (int i = 0; i < 200; i++) {
		executor->execute(new Worker());
	}
	
	executor->shutdown();
	
	delete executor; //对象资源释放!
}
```

以上示例是不是够吸引人？

更多示例：  
[testefc.cpp](test/testefc.cpp)  
[testnio.cpp](test/testnio.cpp)  
[testbon.cpp](test/testbon.cpp)  
[testc11.cpp](test/testc11.cpp)

####性能
EConcurrentHashmap测试: 

`软件环境:`

```
put 20线程 && get 20线程
```

`硬件环境：`

```
型号名称：	MacBook Pro
型号标识符：	MacBookPro10,2
处理器名称：	Intel Core i5
处理器速度：	2.6 GHz
处理器数目：	1
核总数：	2
```
`测试结果：`

![test_concurrentHashmap](img/test_concurrentHashmap.gif)

####特性

######base:

| jdk类 | cxxjdk类 |
| ----- | ----- |
|	Object[]                        |  EA                              |
|	AbstractCollection              |  EAbstractCollection             |
|	AbstractList                    |  EAbstractList                   |
|	AbstractMap                     |  EAbstractMap                    |
|	AbstractQueue                   |  EAbstractQueue                  |
|	AbstractSet                     |  EAbstractSet                    |
|	AlreadyBoundException           |  EAlreadyBoundException          |
|	Array                           |  EArray                          |
|	ArrayDeque                      |  EArrayDeque                     |
|	ArrayList                       |  EArrayList                      |
|	Arrays                          |  EArrays                         |
|	AuthenticationException         |  EAuthenticationException        |
|	Base64                          |  EBase64                         |
|	BindException                   |  EBindException                  |
|	BitSet                          |  EBitSet                         |
|	Bits                            |  EBits                           |
|	Boolean                         |  EBoolean                        |
|	BufferedInputStream             |  EBufferedInputStream            |
|	BufferedOutputStream            |  EBufferedOutputStream           |
|	Byte                            |  EByte                           |
|	ByteArrayInputStream            |  EByteArrayInputStream           |
|	ByteArrayOutputStream           |  EByteArrayOutputStream          |
|	CRC32                           |  ECRC32                          |
|	Calendar                        |  ECalendar                       |
|	Character                       |  ECharacter                      |
|	CheckedInputStream              |  ECheckedInputStream             |
|	Checksum                        |  EChecksum                       |
|	ClassCastException              |  EClassCastException             |
|	Collection                      |  ECollection                     |
|	Collections                     |  ECollections                    |
|	Comparable                      |  EComparable                     |
|	Comparator                      |  EComparator                     |
|	Condition                       |  ECondition                      |
|	ConnectException                |  EConnectException               |
|	DataFormatException             |  EDataFormatException            |
|	DataInput                       |  EDataInput                      |
|	DataInputStream                 |  EDataInputStream                |
|	DataOutput                      |  EDataOutput                     |
|	DataOutputStream                |  EDataOutputStream               |
|	Date                            |  EDate                           |
|	Deflater                        |  EDeflater                       |
|	DeflaterOutputStream            |  EDeflaterOutputStream           |
|	Deque                           |  EDeque                          |
|	Dictionary                      |  EDictionary                     |
|	Double                          |  EDouble                         |
|	EOFException                    |  EEOFException                   |
|	EmptyStackException             |  EEmptyStackException            |
|	Enumeration                     |  EEnumeration                    |
|	EventListener                   |  EEventListener                  |
|	EventListenerProxy              |  EEventListenerProxy             |
|	EventObject                     |  EEventObject                    |
|	Exception                       |  EException                      |
|	File                            |  EFile                           |
|	FileInputStream                 |  EFileInputStream                |
|	FileNotFoundException           |  EFileNotFoundException          |
|	FileOutputStream                |  EFileOutputStream               |
|	FilenameFilter                  |  EFilenameFilter                 |
|	FilterInputStream               |  EFilterInputStream              |
|	FilterOutputStream              |  EFilterOutputStream             |
|	Float                           |  EFloat                          |
|	GZIPInputStream                 |  EGZIPInputStream                |
|	GZIPOutputStream                |  EGZIPOutputStream               |
|	HashMap                         |  EHashMap                        |
|	HashSet                         |  EHashSet                        |
|	IOException                     |  EIOException                    |
|	IPAddressUtil                   |  EIPAddressUtil                  |
|	IdentityHashMap                 |  EIdentityHashMap                |
|	IllegalArgumentException        |  EIllegalArgumentException       |
|	IllegalStateException           |  EIllegalStateException          |
|	IllegalThreadStateException     |  EIllegalThreadStateException    |
|	IndexOutOfBoundsException       |  EIndexOutOfBoundsException      |
|	InetAddress                     |  EInetAddress                    |
|	InetSocketAddress               |  EInetSocketAddress              |
|	Inflater                        |  EInflater                       |
|	InflaterInputStream             |  EInflaterInputStream            |
|	InputStream                     |  EInputStream                    |
|	Integer                         |  EInteger                        |
|	InterruptedException            |  EInterruptedException           |
|	InterruptedIOException          |  EInterruptedIOException         |
|	Interruptible                   |  EInterruptible                  |
|	Iterable                        |  EIterable                       |
|	Iterator                        |  EIterator                       |
|	LLong                           |  ELLong                          |
|	LinkedList                      |  ELinkedList                     |
|	List                            |  EList                           |
|	ListIterator                    |  EListIterator                   |
|	Lock                            |  ELock                           |
|	MalformedURLException           |  EMalformedURLException          |
|	Map                             |  EMap                            |
|	MatchResult                     |  EMatchResult                    |
|	Matcher                         |  EMatcher                        |
|	Math                            |  EMath                           |
|	MathFP                          |  EMathFP                         |
|	NavigableMap                    |  ENavigableMap                   |
|	NavigableSet                    |  ENavigableSet                   |
|	NoRouteToHostException          |  ENoRouteToHostException         |
|	NoSuchElementException          |  ENoSuchElementException         |
|	NullPointerException            |  ENullPointerException           |
|	NumberFormatException           |  ENumberFormatException          |
|	Object                          |  EObject                         |
|	Observable                      |  EObservable                     |
|	Observer                        |  EObserver                       |
|	OutOfMemoryError                |  EOutOfMemoryError               |
|	OutputStream                    |  EOutputStream                   |
|	Pattern                         |  EPattern                        |
|	PatternSyntaxException          |  EPatternSyntaxException         |
|	PipedInputStream                |  EPipedInputStream               |
|	PipedOutputStream               |  EPipedOutputStream              |
|	PortUnreachableException        |  EPortUnreachableException       |
|	PrintStream                     |  EPrintStream                    |
|	PriorityQueue                   |  EPriorityQueue                  |
|	Process                         |  EProcess                        |
|	ProtocolException               |  EProtocolException              |
|	Queue                           |  EQueue                          |
|	Random                          |  ERandom                         |
|	RandomAccessFile                |  ERandomAccessFile               |
|	Reference                       |  EReference                      |
|	Runnable                        |  ERunnable                       |
|	Runtime                         |  ERuntime                        |
|	RuntimeException                |  ERuntimeException               |
|	SaslException                   |  ESaslException                  |
|	SecureRandom                    |  ESecureRandom                   |
|	SecurityException               |  ESecurityException              |
|	Sentry                          |  ESentry                         |
|	SequenceInputStream             |  ESequenceInputStream            |
|	ServerSocket                    |  EServerSocket                   |
|	Set                             |  ESet                            |
|	Socket                          |  ESocket                         |
|	SocketException                 |  ESocketException                |
|	SocketOptions                   |  ESocketOptions                  |
|	SocketTimeoutException          |  ESocketTimeoutException         |
|	SortedMap                       |  ESortedMap                      |
|	SortedSet                       |  ESortedSet                      |
|   Stack                           |  EStack                          |
|	String                          |  EString                         |
|	StringTokenizer                 |  EStringTokenizer                |
|	System                          |  ESystem                         |
|	Thread                          |  EThread                         |
|	ThreadGroup                     |  EThreadGroup                    |
|	ThreadLocal                     |  EThreadLocal                    |
|	ThreadLocalStorage              |  EThreadLocalStorage             |
|	Throwable                       |  EThrowable                      |
|	TimeUnit                        |  ETimeUnit                       |
|	Timer                           |  ETimer                          |
|	TimerTask                       |  ETimerTask                      |
|	TreeMap                         |  ETreeMap                        |
|	TreeSet                         |  ETreeSet                        |
|	URI                             |  EURI                            |
|	URISyntaxException              |  EURISyntaxException             |
|	URLDecoder                      |  EURLDecoder                     |
|	URLEncoder                      |  EURLEncoder                     |
|	URLString                       |  EURLString                      |
|	UnknownHostException            |  EUnknownHostException           |
|	UnknownServiceException         |  EUnknownServiceException        |
|	UnsupportedEncodingException    |  EUnsupportedEncodingException   |
|	UnsupportedOperationException   |  EUnsupportedOperationException  |
|	Vector                          |  EVector                         |
|   try {xxx} finally {yyy}         |  ON_FINALLY_NOTHROW(yyy) {xxx}   |

                    
######concurrent:

| jdk类 | cxxjdk类 |
| ----- | ----- |
| synchronized                              |  SYNCHRONIZED                    |
| AbstractConcurrentCollection              |   EAbstractConcurrentCollection        |
| AbstractConcurrentQueue                   |   EAbstractConcurrentQueue             |
| AbstractExecutorService                   |   EAbstractExecutorService             |
| AbstractOwnableSynchronizer               |   EAbstractOwnableSynchronizer         |
| AbstractQueuedSynchronizer                |   EAbstractQueuedSynchronizer          |
| AtomicBoolean                             |   EAtomicBoolean                       |
| AtomicCounter                             |   EAtomicCounter                       |
| AtomicInteger                             |   EAtomicInteger                       |
| AtomicLLong                               |   EAtomicLLong                         |
| AtomicReference                           |   EAtomicReference                     |
| BlockingQueue                             |   EBlockingQueue                       |
| BrokenBarrierException                    |   EBrokenBarrierException              |
| Callable                                  |   ECallable                            |
| CancellationException                     |   ECancellationException               |
| CompletionService                         |   ECompletionService                   |
| ConcurrentHashMap                         |   EConcurrentHashMap                   |
| ConcurrentLinkedQueue                     |   EConcurrentLinkedQueue               |
| ConcurrentModificationException           |   EConcurrentModificationException     |
| ConcurrentNavigableMap                    |   EConcurrentNavigableMap              |
| ConcurrentSkipListMap                     |   EConcurrentSkipListMap               |
| CopyOnWriteArrayList                      |   ECopyOnWriteArrayList                |
| CountDownLatch                            |   ECountDownLatch                      |
| CyclicBarrier                             |   ECyclicBarrier                       |
| Exchanger                                 |   EExchanger                           |
| ExecutionException                        |   EExecutionException                  |
| Executor                                  |   EExecutor                            |
| ExecutorCompletionService                 |   EExecutorCompletionService           |
| ExecutorService                           |   EExecutorService                     |
| Executors                                 |   EExecutors                           |
| Future                                    |   EFuture                              |
| FutureTask                                |   EFutureTask                          |
| LinkedBlockingQueue                       |   ELinkedBlockingQueue                 |
| LinkedTransferQueue                       |   ELinkedTransferQueue                 |
| LockSupport                               |   ELockSupport                         |
| ReadWriteLock                             |   EReadWriteLock                       |
| ReentrantLock                             |   EReentrantLock                       |
| ReentrantReadWriteLock                    |   EReentrantReadWriteLock              |
| RejectedExecutionException                |   ERejectedExecutionException          |
| RejectedExecutionHandler                  |   ERejectedExecutionHandler            |
| RunnableFuture                            |   ERunnableFuture                      |
| Semaphore                                 |   ESemaphore                           |
| SynchronousQueue                          |   ESynchronousQueue                    |
| ThreadFactory                             |   EThreadFactory                       |
| ThreadLocalRandom                         |   EThreadLocalRandom                   |
| ThreadPoolExecutor                        |   EThreadPoolExecutor                  |
| TimeoutException                          |   ETimeoutException                    |
| TransferQueue                             |   ETransferQueue                       |
| Unsafe                                    |   EUnsafe                              |
| Collection*                               |   EConcurrentCollection                |
| Enumeration*                              |   EConcurrentEnumeration               |
| Iterator*                                 |   EConcurrentIterator                  |
| List*                                     |   EConcurrentList                      |
| Map*                                      |   EConcurrentMap                       |
| ListIterator*                             |   EConcurrentListIterator              |
| NavigableSet*                             |   EConcurrentNavigableSet              |
| Queue*                                    |   EConcurrentQueue                     |
| Set*                                      |   EConcurrentSet                       |
| SortedMap*                                |   EConcurrentSortedMap                 |
| SortedSet*                                |   EConcurrentSortedSet                 |
| Object[]*                                 |   ESharedArr                           |
| ArrayList*                                |   ESharedArrLst                        |


######nio:

| jdk类 | cxxjdk类 |
| ----- | ----- |
| AlreadyConnectedException       |  EAlreadyConnectedException         |
| AsynchronousCloseException      |  EAsynchronousCloseException        |
| BufferOverflowException         |  EBufferOverflowException           |
| BufferUnderflowException        |  EBufferUnderflowException          |
| ByteChannel                     |  EByteChannel                       |
| CancelledKeyException           |  ECancelledKeyException             |
| Channel                         |  EChannel                           |
| ChannelInputStream              |  EChannelInputStream                |
| ClosedByInterruptException      |  EClosedByInterruptException        |
| ClosedChannelException          |  EClosedChannelException            |
| ClosedSelectorException         |  EClosedSelectorException           |
| ConnectionPendingException      |  EConnectionPendingException        |
| FileChannel                     |  EFileChannel                       |
| FileLock                        |  EFileLock                          |
| xxByteBuffer                    |  EIOByteBuffer                      |
| IllegalBlockingModeException    |  EIllegalBlockingModeException      |
| InterruptibleChannel            |  EInterruptibleChannel              |
| InvalidMarkException            |  EInvalidMarkException              |
| MappedByteBuffer                |  EMappedByteBuffer                  |
| NativeThreadSet                 |  ENativeThreadSet                   |
| NoConnectionPendingException    |  ENoConnectionPendingException      |
| NonReadableChannelException     |  ENonReadableChannelException       |
| NonWritableChannelException     |  ENonWritableChannelException       |
| NotYetConnectedException        |  ENotYetConnectedException          |
| OverlappingFileLockException    |  EOverlappingFileLockException      |
| ReadOnlyBufferException         |  EReadOnlyBufferException           |
| SelectableChannel               |  ESelectableChannel                 |
| SelectionKey                    |  ESelectionKey                      |
| Selector                        |  ESelector                          |
| ServerSocketChannel             |  EServerSocketChannel               |
| SocketChannel                   |  ESocketChannel                     |
| UnresolvedAddressException      |  EUnresolvedAddressException        |


######other:

| cxxjdk类 | 类描述 |
| ----- | ----- |
| EConfig               | Foundation for application configuration |
| EByteBuffer           | Byte array buffer |
| ESharedPtr            | Smart Pointer |
| ELockPool             | SpinLock Pool |
| EOrderAccess          | Memory Access Ordering Model |
| EFork                 | Process system call, only for Unix/Linux system |
| ESimpleCondition      | C++ style condition |
| ESimpleLock           | C++ style mutex lock      |
| ESpinLock             | C++ wrapper for spin lock |
| ESimpleMap            | C++ wrapper for map table |
| ESimpleStack          | C++ wrapper for stack     |
| ESimpleVector         | C++ wrapper for vector    |
| ESimpleEnumeration    | Enumeration for ESimpleXXX|
| EStream               | C++ wrapper for io stream |
| EOS                   | Utils for OS              |
| EAtomic               | C++ wrapper for atomic operation |
   


####TODO
    java.net.DatagramSocket；
    java.nio.channels.DatagramChannel；
    java.security.*;

####Support
Email: [cxxjava@163.com](mailto:cxxjava@163.com)