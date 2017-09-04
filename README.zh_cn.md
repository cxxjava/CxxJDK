# CxxJDK
## 山寨JDK实现的C++强大类库.



英文版本: [English](README.md)




### 目录
- [特点](#特点)
  - [示例](#示例)
  - [性能](#性能)
  - [特性](#特性)
  - [手册](#手册)
  - [TODO](#todo)
  - [Support](#support)

#### 特点
* 跨平台：同时支持Linux32/64、OSX64、Win64等平台；
* 高性能：同时具备C/C++和Java并发库的优点，性能爆棚；
* 类丰富：实现JDK丰富的类库，从此摆脱C++贫类库的困境；
* 易开发：先java开发，再使用`CxxJDK`进行翻译，So easy!

#### 示例
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
	executor->awaitTermination();
	
	delete executor; //对象资源释放!
}
```

以上示例是不是够吸引人？

更多示例：  
[testefc.cpp](test/testefc.cpp)  
[testnio.cpp](test/testnio.cpp)  
[testbson.cpp](test/testbson.cpp)  
[testc11.cpp](test/testc11.cpp)  
[testssl.cpp](test/testssl.cpp)  

#### 性能
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

#### 特性

###### base:

| jdk类                            | cxxjdk类                          |
| ------------------------------- | -------------------------------- |
| Object[]                        | EA                               |
| AbstractCollection              | EAbstractCollection              |
| AbstractList                    | EAbstractList                    |
| AbstractMap                     | EAbstractMap                     |
| AbstractQueue                   | EAbstractQueue                   |
| AbstractSet                     | EAbstractSet                     |
| AlreadyBoundException           | EAlreadyBoundException           |
| ArithmeticException             | EArithmeticException             |
| Array                           | EArray                           |
| ArrayDeque                      | EArrayDeque                      |
| ArrayList                       | EArrayList                       |
| Arrays                          | EArrays                          |
| AuthenticationException         | EAuthenticationException         |
| Base64                          | EBase64                          |
| BigDecimal                      | EBigDecimal                      |
| BigInteger                      | EBigInteger                      |
| BindException                   | EBindException                   |
| BitSet                          | EBitSet                          |
| Bits                            | EBits                            |
| Boolean                         | EBoolean                         |
| BufferedInputStream             | EBufferedInputStream             |
| BufferedOutputStream            | EBufferedOutputStream            |
| Byte                            | EByte                            |
| ByteArrayInputStream            | EByteArrayInputStream            |
| ByteArrayOutputStream           | EByteArrayOutputStream           |
| CRC32                           | ECRC32                           |
| Calendar                        | ECalendar                        |
| Character                       | ECharacter                       |
| CheckedInputStream              | ECheckedInputStream              |
| Checksum                        | EChecksum                        |
| ClassCastException              | EClassCastException              |
| Collection                      | ECollection                      |
| Collections                     | ECollections                     |
| Comparable                      | EComparable                      |
| Comparator                      | EComparator                      |
| Condition                       | ECondition                       |
| ConnectException                | EConnectException                |
| DataFormatException             | EDataFormatException             |
| DatagramPacket                  | EDatagramPacket                  |
| DatagramSocket                  | EDatagramSocket                  |
| DataInput                       | EDataInput                       |
| DataInputStream                 | EDataInputStream                 |
| DataOutput                      | EDataOutput                      |
| DataOutputStream                | EDataOutputStream                |
| Date                            | EDate                            |
| Deflater                        | EDeflater                        |
| DeflaterOutputStream            | EDeflaterOutputStream            |
| Deque                           | EDeque                           |
| Dictionary                      | EDictionary                      |
| Double                          | EDouble                          |
| EOFException                    | EEOFException                    |
| EmptyStackException             | EEmptyStackException             |
| Enumeration                     | EEnumeration                     |
| EventListener                   | EEventListener                   |
| EventListenerProxy              | EEventListenerProxy              |
| EventObject                     | EEventObject                     |
| Exception                       | EException                       |
| File                            | EFile                            |
| FileInputStream                 | EFileInputStream                 |
| FileNotFoundException           | EFileNotFoundException           |
| FileOutputStream                | EFileOutputStream                |
| FilenameFilter                  | EFilenameFilter                  |
| FilterInputStream               | EFilterInputStream               |
| FilterOutputStream              | EFilterOutputStream              |
| Float                           | EFloat                           |
| GZIPInputStream                 | EGZIPInputStream                 |
| GZIPOutputStream                | EGZIPOutputStream                |
| HashMap                         | EHashMap                         |
| HashSet                         | EHashSet                         |
| IOException                     | EIOException                     |
| IPAddressUtil                   | EIPAddressUtil                   |
| IdentityHashMap                 | EIdentityHashMap                 |
| IllegalArgumentException        | EIllegalArgumentException        |
| IllegalStateException           | EIllegalStateException           |
| IllegalThreadStateException     | EIllegalThreadStateException     |
| IndexOutOfBoundsException       | EIndexOutOfBoundsException       |
| InetAddress                     | EInetAddress                     |
| InetSocketAddress               | EInetSocketAddress               |
| Inflater                        | EInflater                        |
| InflaterInputStream             | EInflaterInputStream             |
| InputStream                     | EInputStream                     |
| Integer                         | EInteger                         |
| InterfaceAddress                | EInterfaceAddress                |
| InterruptedException            | EInterruptedException            |
| InterruptedIOException          | EInterruptedIOException          |
| Interruptible                   | EInterruptible                   |
| Iterable                        | EIterable                        |
| Iterator                        | EIterator                        |
| LLong                           | ELLong                           |
| LinkedList                      | ELinkedList                      |
| List                            | EList                            |
| ListIterator                    | EListIterator                    |
| Lock                            | ELock                            |
| MalformedURLException           | EMalformedURLException           |
| Map                             | EMap                             |
| MatchResult                     | EMatchResult                     |
| Matcher                         | EMatcher                         |
| Math                            | EMath                            |
| MathFP                          | EMathFP                          |
| MulticastSocket                 | EMulticastSocket                 |
| NavigableMap                    | ENavigableMap                    |
| NavigableSet                    | ENavigableSet                    |
| NetworkInterface                | ENetworkInterface                |
| NoRouteToHostException          | ENoRouteToHostException          |
| NoSuchElementException          | ENoSuchElementException          |
| NotYetBoundException            | ENotYetBoundException            |
| NullPointerException            | ENullPointerException            |
| Number                          | ENumber                          |
| NumberFormatException           | ENumberFormatException           |
| Object                          | EObject                          |
| Observable                      | EObservable                      |
| Observer                        | EObserver                        |
| OutOfMemoryError                | EOutOfMemoryError                |
| OutputStream                    | EOutputStream                    |
| Pattern                         | EPattern                         |
| PatternSyntaxException          | EPatternSyntaxException          |
| PipedInputStream                | EPipedInputStream                |
| PipedOutputStream               | EPipedOutputStream               |
| PortUnreachableException        | EPortUnreachableException        |
| PrintStream                     | EPrintStream                     |
| PriorityQueue                   | EPriorityQueue                   |
| Process                         | EProcess                         |
| ProtocolException               | EProtocolException               |
| Queue                           | EQueue                           |
| Random                          | ERandom                          |
| RandomAccessFile                | ERandomAccessFile                |
| Reference                       | EReference                       |
| Runnable                        | ERunnable                        |
| Runtime                         | ERuntime                         |
| RuntimeException                | ERuntimeException                |
| SaslException                   | ESaslException                   |
| SecureRandom                    | ESecureRandom                    |
| SecurityException               | ESecurityException               |
| Sentry                          | ESentry                          |
| SequenceInputStream             | ESequenceInputStream             |
| ServerSocket                    | EServerSocket                    |
| Set                             | ESet                             |
| Short                           | EShort                           |
| Socket                          | ESocket                          |
| SocketException                 | ESocketException                 |
| SocketOptions                   | ESocketOptions                   |
| SocketTimeoutException          | ESocketTimeoutException          |
| SortedMap                       | ESortedMap                       |
| SortedSet                       | ESortedSet                       |
| Stack                           | EStack                           |
| String                          | EString                          |
| StringTokenizer                 | EStringTokenizer                 |
| System                          | ESystem                          |
| Thread                          | EThread                          |
| ThreadGroup                     | EThreadGroup                     |
| ThreadLocal                     | EThreadLocal                     |
| ThreadLocalStorage              | EThreadLocalStorage              |
| Throwable                       | EThrowable                       |
| TimeUnit                        | ETimeUnit                        |
| Timer                           | ETimer                           |
| TimerTask                       | ETimerTask                       |
| TreeMap                         | ETreeMap                         |
| TreeSet                         | ETreeSet                         |
| URI                             | EURI                             |
| URISyntaxException              | EURISyntaxException              |
| URLDecoder                      | EURLDecoder                      |
| URLEncoder                      | EURLEncoder                      |
| URLString                       | EURLString                       |
| UnknownHostException            | EUnknownHostException            |
| UnknownServiceException         | EUnknownServiceException         |
| UnsupportedAddressTypeException | EUnsupportedAddressTypeException |
| UnsupportedEncodingException    | EUnsupportedEncodingException    |
| UnsupportedOperationException   | EUnsupportedOperationException   |
| Vector                          | EVector                          |
| try {xxx} finally {yyy}         | ON_FINALLY_NOTHROW(yyy) {xxx}}   |


###### concurrent:

| jdk类                            | cxxjdk类                          |
| ------------------------------- | -------------------------------- |
| synchronized                    | SYNCHRONIZED                     |
| AbstractConcurrentCollection    | EAbstractConcurrentCollection    |
| AbstractConcurrentQueue         | EAbstractConcurrentQueue         |
| AbstractExecutorService         | EAbstractExecutorService         |
| AbstractOwnableSynchronizer     | EAbstractOwnableSynchronizer     |
| AbstractQueuedSynchronizer      | EAbstractQueuedSynchronizer      |
| AtomicBoolean                   | EAtomicBoolean                   |
| AtomicCounter                   | EAtomicCounter                   |
| AtomicInteger                   | EAtomicInteger                   |
| AtomicLLong                     | EAtomicLLong                     |
| AtomicReference                 | EAtomicReference                 |
| BlockingQueue                   | EBlockingQueue                   |
| BrokenBarrierException          | EBrokenBarrierException          |
| Callable                        | ECallable                        |
| CancellationException           | ECancellationException           |
| CompletionService               | ECompletionService               |
| ConcurrentHashMap               | EConcurrentHashMap               |
| ConcurrentLinkedQueue           | EConcurrentLinkedQueue           |
| ConcurrentModificationException | EConcurrentModificationException |
| ConcurrentNavigableMap          | EConcurrentNavigableMap          |
| ConcurrentSkipListMap           | EConcurrentSkipListMap           |
| CopyOnWriteArrayList            | ECopyOnWriteArrayList            |
| CountDownLatch                  | ECountDownLatch                  |
| CyclicBarrier                   | ECyclicBarrier                   |
| Exchanger                       | EExchanger                       |
| ExecutionException              | EExecutionException              |
| Executor                        | EExecutor                        |
| ExecutorCompletionService       | EExecutorCompletionService       |
| ExecutorService                 | EExecutorService                 |
| Executors                       | EExecutors                       |
| Future                          | EFuture                          |
| FutureTask                      | EFutureTask                      |
| LinkedBlockingQueue             | ELinkedBlockingQueue             |
| LinkedTransferQueue             | ELinkedTransferQueue             |
| LockSupport                     | ELockSupport                     |
| ReadWriteLock                   | EReadWriteLock                   |
| ReentrantLock                   | EReentrantLock                   |
| ReentrantReadWriteLock          | EReentrantReadWriteLock          |
| RejectedExecutionException      | ERejectedExecutionException      |
| RejectedExecutionHandler        | ERejectedExecutionHandler        |
| RunnableFuture                  | ERunnableFuture                  |
| Semaphore                       | ESemaphore                       |
| SynchronousQueue                | ESynchronousQueue                |
| ThreadFactory                   | EThreadFactory                   |
| ThreadLocalRandom               | EThreadLocalRandom               |
| ThreadPoolExecutor              | EThreadPoolExecutor              |
| TimeoutException                | ETimeoutException                |
| TransferQueue                   | ETransferQueue                   |
| Unsafe                          | EUnsafe                          |
| Collection*                     | EConcurrentCollection            |
| Enumeration*                    | EConcurrentEnumeration           |
| Iterator*                       | EConcurrentIterator              |
| List*                           | EConcurrentList                  |
| Map*                            | EConcurrentMap                   |
| ListIterator*                   | EConcurrentListIterator          |
| NavigableSet*                   | EConcurrentNavigableSet          |
| Queue*                          | EConcurrentQueue                 |
| Set*                            | EConcurrentSet                   |
| SortedMap*                      | EConcurrentSortedMap             |
| SortedSet*                      | EConcurrentSortedSet             |


###### nio:

| jdk类                         | cxxjdk类                       |
| ---------------------------- | ----------------------------- |
| AlreadyConnectedException    | EAlreadyConnectedException    |
| AsynchronousCloseException   | EAsynchronousCloseException   |
| BufferOverflowException      | EBufferOverflowException      |
| BufferUnderflowException     | EBufferUnderflowException     |
| ByteChannel                  | EByteChannel                  |
| CancelledKeyException        | ECancelledKeyException        |
| Channel                      | EChannel                      |
| ChannelInputStream           | EChannelInputStream           |
| ClosedByInterruptException   | EClosedByInterruptException   |
| ClosedChannelException       | EClosedChannelException       |
| ClosedSelectorException      | EClosedSelectorException      |
| ConnectionPendingException   | EConnectionPendingException   |
| DatagramChannel              | EDatagramChannel              |
| FileChannel                  | EFileChannel                  |
| FileLock                     | EFileLock                     |
| xxByteBuffer                 | EIOByteBuffer                 |
| IllegalBlockingModeException | EIllegalBlockingModeException |
| InterruptibleChannel         | EInterruptibleChannel         |
| InvalidMarkException         | EInvalidMarkException         |
| MappedByteBuffer             | EMappedByteBuffer             |
| MembershipKey                | EMembershipKey                |
| MulticastChannel             | EMulticastChannel             |
| NativeThreadSet              | ENativeThreadSet              |
| NoConnectionPendingException | ENoConnectionPendingException |
| NonReadableChannelException  | ENonReadableChannelException  |
| NonWritableChannelException  | ENonWritableChannelException  |
| NotYetConnectedException     | ENotYetConnectedException     |
| OverlappingFileLockException | EOverlappingFileLockException |
| ReadOnlyBufferException      | EReadOnlyBufferException      |
| SelectableChannel            | ESelectableChannel            |
| SelectionKey                 | ESelectionKey                 |
| Selector                     | ESelector                     |
| ServerSocketChannel          | EServerSocketChannel          |
| SocketChannel                | ESocketChannel                |
| UnresolvedAddressException   | EUnresolvedAddressException   |


###### other:

| cxxjdk类            | 类描述                           |
| ------------------ | ----------------------------- |
| EConfig            | 应用配置管理                        |
| EByteBuffer        | 二进制数据缓存                       |
| ESharedPtr         | C++智能指针                       |
| ELockPool          | SpinLock池化管理类                 |
| EOrderAccess       | 内存访问模型辅助类                     |
| EFork              | Unix/Linux系统下的进程创建            |
| ESimpleCondition   | C++ style condition           |
| ESimpleLock        | C++ style mutex lock          |
| ESpinLock          | C++ wrapper for spin lock     |
| ESimpleMap         | C++ wrapper for map table     |
| ESimpleStack       | C++ wrapper for stack         |
| ESimpleVector      | C++ wrapper for vector        |
| ESimpleEnumeration | Enumeration for ESimpleXXX    |
| EStream            | IO数据流操作类                      |
| EOS                | 操作系统相关工具类                     |
| EAtomic            | 原子操作类                         |
| EScopeGuard        | C++11风格封装的通过局部变量析构函数来管理资源的辅助类 |
| EAtomicDouble      | double原子操作类                   |
| ENetWrapper        | socket操作辅助工具类                 |


#### 手册
详见[Guide文档](doc/Guide.md)

#### TODO
    java.security.*;

#### Support
Email: [cxxjava@163.com](mailto:cxxjava@163.com)