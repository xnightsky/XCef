# XCef
Chromiumembedded[CEF] demo

这是 CEF的一部分例子...

###  配置环境：  
1.   cef配置  
    1.1 CEF版本是[cef_binary_3.2171.1979_windows64](http://www.magpcss.net/cef_downloads/index.php?file=cef_binary_3.2171.1979_windows64.7z)  
    1.2 导入"\library\cef\include_cef.h"或者直接使用"src\include_XCef.h"包含全部xcef+cef内容  
    1.3 有时需要修改CEF_LIB_DIR宏来重新定位cef lib位置  
    1.4 cef dll和相关资源文件，请根据1.1中的链接自行下载和编译  
2.  需要VS2013以上版本IDE编译  
3.  如果需要编译wtl项目，wtl版本WTL90_4140_Final  
4.  cJson已经包含，导入"\library\cJSON\include_cJSON.h"  

include eg:  
```cpp  
    #include "..\\library\\cJSON\include_cJSON.h"  
    #define CEF_LIB_DIR "..\\library\\cef\\lib64\\"  
    #include "..\\library\\cef\\include_cef.h"  
    #include "..\src\include_XCef.h"  
```  

### 关于XCefCallback
XCefCallback 统一了同步回调和cefQuery  
cpp eg:  
```cpp
// h文件
class XXXCallback : public XCefCallback
{
    // 组件注册接口宏
    DECLARE_XV8()
public:
    // 同步回调
	static bool		sync_xx(
		XCefV8Handler *				pthis_handle,
		CefRefPtr<CefV8Value>		object,
		const CefV8ValueList &		arguments,
		CefRefPtr<CefV8Value> &		retval,
		CefString &					exception
		);
	// cefQuery回调
	static bool		async_xx(
		XAsyncBrowserHandler *			pthis_handle,
		CefRefPtr<CefBrowser>			browser,
		CefRefPtr<CefFrame>				frame,
		int64							query_id,
		const CefString &				request,
		bool							persistent,
		CefRefPtr<BrowserCallback>		callback,
		XRPC_INFO &						rpc_info
		);
}
```
```cpp
// cpp文件

// 注册接口宏
BEGIN_XV8(XXXCallback)
	// 同步回调注册
	HANDLE_XV8_CB("sync_xx", XXXCallback::sync_xx)
    // cefQuery回调注册
	FORWARD_XV8_PROTOCOL_APP()
		HANDLE_XV8_CB_ASYNC("async_xx", XXXCallback::async_xx)
END_XV8()
```
```cpp
// 在main函数中在cef之前把XXXCallback托管给管理类
XCefAppManagePtr mng = XCefAppManage::Instance();
// 注册事件
mng->RigisterCallback(new XXXCallback);
```
js eg:
```javascript
// \resources\JSHandle.html文件
// 同步调用
app.sync_xx(...)
// JSHandle.html文件封装了部分jsonrpc来处理cefQuery
var request_id = jsonrpcQuery("async_xx", str_xx);
var request_id = jsonrpcQuery("async_xx", str_xx, function(response) {});
var request_id = jsonrpcQuery("async_xx", str_xx, function(response) {});
var request_id = jsonrpcQuery("async_xx", str_xx, function(response) {}, function(error_code, error_message) {});
var request_id = jsonrpcQuery("async_xx", [1, "abc", 132], function(response) {}, function(error_code, error_message) {});
```

###### XWinCallback文件补充：  
drag系列函数没有实现，一时没有什么好的办法  
native_thread_callback和相关js{JSHandle.html文件}，实现一个开启cpp thread，分段处理数据并回调js的例子  

###### CefWin64补充:  
CefWin64调试默认开启了离屏渲染(off-screen),背景被opengl托管了。。。  



收工:)
