#CxxJDK使用指南
版本：0.1.1  
更新：2016/08/01

##目录
  - [适用对象](#适用对象：)
  - [API说明](#API说明：)
  - [编译说明](#编译说明：)
  - [其他说明](#其他说明：)
  - [开发建议](#开发建议：)
  - [FAQ](#FAQ：)

###适用对象：
本指南适用于具备c++的基础知识的人员，知道什么是对象，什么是构造函数、析构函数，什么是继承，什么是覆盖、重载、隐藏，什么是虚函数等；特别是需要知道c++对象的生命周期，知道如何销毁对象！

如果您同时了解java的语法和开发过程，那么`CxxJDK`简直就是为您量身定做的，以后再也不用怕高深尖的c++开发了！

###API说明：

首先，`CxxJDK`几乎所有API都是高度山寨自JDK7&JDK8，除本手册未特别说明外，API的使用请自行参考JDK相关文档，如：[https://docs.oracle.com/javase/7/docs/api/](https://docs.oracle.com/javase/7/docs/api/)

但是，c++跟java毕竟是两种不同的编程语言，部分API和关键字的使用稍有差异，请牢记`CxxJDK`的使用规则!!!

#####1.关于对象
在`CxxJDK`里，除EThrowable异常对象外，其他所有对象的根对象都是EObject；
c++对象具有构造函数和析构函数，在栈内存里分配的对象在离开作用域后对象自动销毁，在堆内存里分配(new)的对象需要手动销毁(delete)；

  `java:`

  ```
  class ClassX {
  	public void print() {
  		System.out.println(">o<");
  	}  
  }
  
  ClassX x = new ClassX();
  x.print();
  
  ```
  
  `c++:`
  
  ```
  class ClassX: public EObject {
  public:
  	void print() {
  		ESystem::out->println(">o<");
  	}  
  };
  
  ClassX* x = new ClassX();
  x->print();
  delete x; //!
  ```

#####2.对象生命期
在`CxxJDK`里，对象清理时机、变量和函数名定义有一定的规律可寻，一般情况下请遵循以下原则：  

* 对象生命期由对象作用域决定，离开当前作用域时进行对象销毁；
  
  `c++:`
  
  ```
  {
    EString* s = new EString("x");
    // do something.
    delete s; //当前作用域结束时进行对象销毁
  }
  ```
* 函数名如果是`newXXX` or `cloneXXX` or `copyXXX`则表示返回了一个新创建的对象，该对象需要用户手动清理（智能指针除外）；
  
  `c++:`
  
  ```
  {
    EA<byte> b1(10);
    for (int i=0; i<b1.length(); i++) {
      b1[i] = i;
    }
    EA<byte>* b2 = b1->clone(); //返回新创建对象
    // do something.
    delete b2;
  }
  ```
* 函数参数如果是一个sp智能指针，则表示对象生命期交由智能指针进行管理，开发者无需手工销毁对象；而函数参数如果是一个普通指针，则表示对象仅被函数内部操作了一下，对象生命仍需在当前作用域结束时进行清理；

#####3.关于字符串
在`CxxJDK`里，字符串分为两类，一类是c字符串`char*`,另一类是c++字符串`EString`；  

  `c++:`
  
  ```
  //字符串转换：
  EString s("xxx");            //char* -> EString
  const char* ps = s.c_str();  //EString -> char*
  ```

在`CxxJDK`里，字符串统一使用`utf-8`编码；

另外，`CxxJDK`字符串EString跟java字符串String一个非常重要的区别是：  
java字符串String是不可变字符串，对象不可修改；但`CxxJDK`字符串EString却是可变字符串，允许修改；所以EString的功能实际约等同于java里的String & StringBuffer；  

  `c++:`
  
  ```
  EString s("xxx");
  s.append("yyy").append("zzz");
  ESystem::out->println(s.c_str()); //输出："xxxyyyzzz"
  ```

#####4.关于初始化
在java里，对象成员会自动初始化为零；但在c++里，对象成员则一般需要在构造函数里手动初始化，示例如：

  `c++:`
  
  ```
  class ClassX : public EObject {
  public:
    ClassX() : number(0), handler(null) { //!
    }
    
  private:
    int number;
    void* handler;
  };
  ```

c++里的静态对象初始化较为复杂，特别是c++里不同源码里的静态对象不保证像java里的先后顺序，所以一旦出错问题很难排查（静态对象仅本源码文件里引用则无此问题）；在`CxxJDK`里，对跨源码引用的静态对象初始化提供了相应的解决方案，具体可参见`EBase.hh`里`DECLARE_STATIC_XXX`宏定义组的使用。

#####5.关于容器
在`CxxJDK`里，容器有两种：一种是仅支持普通对象指针的容器（简称`NP容器`），另一种是仅支持智能智能的容器（简称`SP容器`）；
NP容器内的对象生命期由创建容器时指定的autoFree参数决定，该参数默认一般为true，即容器销毁时内部对象也一并销毁；
SP容器内的对象生命期由对象的智能指针决定；
在`CxxJDK`里，concurrent目录下的容器一般都是SP容器，其他则为NP容器；

  `NP容器：`

  ```
  EArrayList<EString*>* arrList = new EArrayList(true);
  arrList->add(new EString("x"));
  arrList->add(new EString("y"));
  arrList->add(new EString("z"));
  delete arrList; //!
  ```

  `SP容器：`

  ```
  eal<EString> arrList;
  arrList.add(new EString("x"));
  arrList.add(new EString("y"));
  arrList.add(new EString("z"));
  ```

#####6.关于数组
在Java里，数组是内置对象，但在`CxxJDK`里是EA<E>对象：

  `java:`

  ```
  byte[] arr = new byte[10];
  for (int i=0; i<arr.length; i++) {
  	arr[i] = i;
  }
  
  String[] strs = new String[10];
  for (int i=0; i<strs.length; i++) {
  	strs[i] = new String(i);
  }
  
  ```
  
  `c++:`
  
  ```
  EA<byte>* arr = new EA<byte>(10);
  for (int i=0; i<arr->length(); i++) {
  	(*arr)[i] = i;
  }
  delete arr;
  
  EA<EString*>* strs = new EA<EString*>(10);
  for (int i=0; i<strs->length(); i++) {
  	(*strs)[i] = new EString(i);
  }
  delete strs;
  ```
  
#####7.智能指针
`CxxJDK`里的智能指针是[sp对象](../inc/concurrent/ESharedPtr.hh)，concurrent类库里大量使用了sp对象，确切的说是没有sp就没有concurrent类库。

  `c++:`
  
  ```
  sp<EString> s(new EString("xxx"));
  ESystem::out->println(s->c_str());
  ```
  
`CxxJDK`里的智能指针sp对象还可以退化为原始指针：

  `c++:`
  
  ```
  sp<EString> s(new EString("xxx"));
  EString* ps = s.dismiss(); //退化操作!
  ESystem::out->println(ps->c_str());
  delete ps; //退化后需手动释放资源!
  ```

#####8.线程同步
在java里，线程同步一般使用`synchronized`关键字和`ReentrantLock`锁对象，示例如：

  `java:`
  
  ```
  class ClassX {
    private int n = 0;
  	public synchronized void syncAdd(int i) {
  		n += i;
  	}
  }
  
  class ClassY {
	private int n = 0;
	private ReentrantLock lock;
	public void syncAdd(int i) {
		lock.lock();
		try {
			n += i;
		} finally {
			lock.unlock();
		}
	}
  }
  ```

在`CxxJDK`里，线程同步则对应如下：

  `c++:`
  
  ```
  class ClassX : public ESynchronizeable { //!
  private:
  	int n;
  public:
    ClassX(): n(0) {
    }
    void syncAdd(int i) {
	  	SYNCHRONIZED(this) { //!
  			n += i;
  		}}
  	}
  };
  
  class ClassY : public EObject {
  private:
    int n;
	EReentrantLock lock;
  public:
    ClassY(): n(0) {
    }
    void syncAdd(int i) {
		SYNCBLOCK(&lock) { //!
			n += i;
		}}
	}
  };
  ```

#####9.关于异常

在`CxxJDK`里，异常对象统一是EThrowable及其子类；
异常抛出统一使用对象引用方式，同时异常对象构造参数需传入`__FILE__`、`__LINE__`参数，如：

  `c++:`
  
  ```
  try {
    // do something.
    throw EIoException(__FILE__, __LINE__, "message");
  } catch (EException& e) {
    e.printStackTrace();
  }
  ```
  
同时，c++里还有捕获所有异常的特有方法，如：

  `c++:`
  
  ```
  try {
    // do something.
    throw EIoException(__FILE__, __LINE__, "message");
  } catch (...) { //捕获所有异常
    // do something.
    throw; //重新抛出原异常
  }
  ```
  
####10.finally
在c++里，finally没有统一的处理标准，所以在`CxxJDK`里，我们使用了新的解决方案：  

* 如果编译器支持c++11，那么可以使用[ON_FINALLY_NOTHROW](../inc/EScopeGuard.hh)宏进行处理，对比示例如：

  `java:`
  
  ```
  FileInputStream fis = new FileInputStream("/tmp");
  try {
    // do something.
    throw new IoException("message");
  } catch (Exception e) {
    e.printStackTrace();
  } finally { //!
    try {
      fis.close();
    } catch (Throwable ingnore) {
    }
  }
  ```
  
  `c++:`
  
  ```
  EFileInputStream fis("/tmp");
  ON_FINALLY_NOTHROW （
    //throw EException(__FILE__, __LINE__, "another message"); //error!
    //return 0; //error!    
    fis.close();
  ）{
    try {
      // do something.
      throw EIoException(__FILE__, __LINE__, "message");
    } catch (EException& e) {
      e.printStackTrace();
    }
  }}
  ```
  
  **注意：**[ON_FINALLY_NOTHROW](../inc/EScopeGuard.hh)宏并不支持宏内异常抛出或return一个对象返回!!!

* 如果编译器不支持c++11或需在finally内抛出一个异常或return一个对象，那么在`CxxJDK`里，解决方案示例如下：
  
  `c++:`
  
  ```
  EFileInputStream fis("/tmp");
  try {
    // do something.
  } catch (EException& e) {
    e.printStackTrace();
  } catch (...) { //必须添加对其他所有异常的捕获!
    finally { //1.
      // do something.
      try {
        fis.close();
      } catch (...) {
      }
    }
    throw; //!必须重新抛出其他异常!
  } finally { //2.
    // do something.
    try {
      fis.close();
    } catch (...) {
    }
  }
  ```

####11.instanceof
在java里是这样使用：

  `java:`
  
  ```
	HashMap<String, String> hm = new HashMap<String, String>();
	if (hm instanceof HashMap<?, ?>) {
		System.out.println("hm instanceof HashMap: true");
	}
	if (hm instanceof Map<?, ?>) {
		System.out.println("hm instanceof Map: true");
	}
	if (!(hm instanceof List<?>)) {
		System.out.println("hm instanceof List: false");
	}
  ```

在`CxxJDK`里，则是这样使用：

  `c++:`
  
  ```
	EHashMap<EString*, EString*>* hm = new EHashMap<EString*, EString*>();
	if (instanceof<EHashMap<EString*, EString*> >(hm)) {
		ESystem::out->println("hm instanceof HashMap: true");
	}
	if (instanceof<EMap<EString*, EString*> >(hm)) {
		ESystem::out->println("hm instanceof Map: true");
	}
	if (!(instanceof<EList<EString*> >(hm))) {
		ESystem::out->println("hm instanceof List: false");
	}
	delete hm;
  ```

###编译说明：
* c++11支持：

| 操作系统 | 编译器版本 |
| ------ | ------- |
| Win64 | Visual C++ 2010 sp1 及以上 |
| Linux | Gcc 4.8 及以上 |
| OS X | XCode 最新版 |

* xcode编译提示："error: control may reach end of non-void function"  

```
@see:
template<class T> sp<T> atomic_load( sp<T> const * p )
{
	SCOPED_SLOCK0(p) {
    return *p;
	}}
}
```
解决方法：
Build Settings-->Mismatched Return Type-->Yes

* xcode c++11编译：  
设置 "C++ Language Dialect" 选项为 "c11"; 设置 "C++ Standard Library" 选项为 "libc++ (LLVM standard C++ library with C++11 support)"

* vs2010 sp1 make 编译：  
1) "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
2) bmake.exe -f Makefile_win

* win下编译需要添加链接Ws2_32.lib，否则会报winsock2相关函数链接错误。

###其他说明：
* 不同于JDK，ESocket/EServerSocket非线程安全；  
* 不同于JDK，ExxxStream::close()方法非线程安全；  
* 不同于JDK，EServerSocket/ESocket每次执行getOutputStream()返回的是唯一绑定的EOutputStream对象；  
* 不同于JDK，EFileOutputStream带有setIOBuffered()，write()需flush()后才等同于jdk的write()；  
* 不同于JDK，EGZipOutputStream/EGZipInputStream不支持内置同步锁；  
* 不同于JDK，ERandomAccessFile默认即带有缓冲功能，而jdk的没有；  

###开发建议：
如果老项目是java开发的，那么直接使用`CxxJDK`进行翻译即可；
如果是新项目，建议先java进行原型开发（开发中注意避免使用lamaba等`CxxJDK`不支持的语法），等调试稳定后再翻译为c++；

c++开发环境建议是MacOSX，代码编辑使用eclipse+cdt，代码编译和调试使用xcode，组合使用效率极高！

###FAQ：
* libc++abi.dylib: terminate called throwing an exception  
该错误是当子线程未捕获异常导致的，正确的做法是在子线程的run()方法内添加try-catch语句。

* winxp上运行时报错：“无法定位程序输入点 InterlockedCompareExchange64 于动态链接库 KERNEL32.dll 上。”  
由于类InterlockedExchangeAdd64等64原子操作api需要win7或以上64位操作系统才支持，所有winxp不再支持；
