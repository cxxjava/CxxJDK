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