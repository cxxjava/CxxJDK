2018/03/02

1. fixed a bug for efc::EInetAddress::getAllByName();
2. sync EConcurrentLinkedQueue.hh to java 9 and fixed bugs;
3. add EMutexLinkedQueue.hh;
4. rename all concurrent keySet()/values()/entrySet() apis to native pointer mode;
5. remove some EConcurrentXXX interface;
6. add a new api for efc:EArrays.
7. re-code efc::EString && remove efc::EStringBase;
8. fixed memory leaks for efc::nio;
9. EThread setDaemon(boolean) --> EThread::setDaemon(sp<EThread>, boolean) !!!
10. add api getChannel() for efc::EFileXXXStream;
11. add EArrayBlockingQueue for efc;
12. add Closable && Flushable interface for efc;
13. add owned param for efc::EFilterInputStream & EFilterOutputStream;
14. fixed bugs for efc::sha1;
15. add EFileFilter.hh for efc.
16. add ECheckOutputStream for efc.
17. add c++11 api support for efc::ETimer;
18. add getXXX & setXXX for efc::nio::EIOByteBuffer;
19. support EArrays::hashCode() for efc;
20. add efc::EProperties;
21. add EPushbackInputStream for efc;
22. add lock support for EByteArrayXXX for efc;
23. fixed a bug for efc::Fork::waitFor();



2017/09/30

1. 重新整合目录结构；
2. 添加utils扩展模块；
3. 其他少量适应性修改；

   ​

2017/08/30

1. 添加EShort&EBigDecimal&EBigInteger的支持；
2. 完善EBson的功能；
3. 添加更多的C++11接口；
4. 其他少量适应性修改；
   ​


2017/05/08

1. 添加对SSL的支持（ESSLServerSocket & ESSLSocket）；  
2. 添加EAtomicDouble的支持；  
3. 添加ESocket::sendfile(...)的支持；  
4. 添加所有普通容器对sp<x>智能指针的支持（同时移除ESharedArr.hh & ESharedArrLst& ESharedLnkLst.hh）；  
   ​


2016/09/30  

1. 添加对UDP的支持；  
2. 修正nullptr对C++98的兼容；  
3. ON_FINALLY_NOTHROW/SYNCBLOCK/SYNCHRONIZED 回退到 "xxx {...}}" 代码模式；  
4. 部分其他api&class完善；