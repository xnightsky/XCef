#include "stdafx.h"
#include "XWinCallback.h"



BEGIN_XV8(XWinCallback)
	HANDLE_XV8_CB("title", XWinCallback::title)
	HANDLE_XV8_CB("cache_hwnd", XWinCallback::cache_hwnd)
//	HANDLE_XV8_CB("drag_window", XWinCallback::drag_window)
	HANDLE_XV8_CB("native_thread_callback", XWinCallback::native_thread_callback)

	HANDLE_XV8_CB("set_title_areas", XWinCallback::set_title_areas)

	
	
	FORWARD_XV8_PROTOCOL_APP()
		HANDLE_XV8_CB_ASYNC("async_title", XWinCallback::async_title)
		HANDLE_XV8_CB_ASYNC("async_get_hwnd", XWinCallback::async_get_hwnd)
//		HANDLE_XV8_CB_ASYNC("async_drag_window", XWinCallback::async_drag_window)
END_XV8()


/*static*/ bool		XWinCallback::title(
	XCefV8Handler *				pthis_handle,
	CefRefPtr<CefV8Value>		object,
	const CefV8ValueList &		arguments,
	CefRefPtr<CefV8Value> &		retval,
	CefString &					exception
	)
{
	// 	DCHECK(pthis_handle);
	// 	CEF_REQUIRE_RENDERER_THREAD();
	if (arguments.size() < 1){
		//retval = CefV8Value::CreateBool(false);

// 		retval = CefV8Value::CreateObject(NULL);
// 		retval->SetValue("result", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
// 		retval->SetValue("message", CefV8Value::CreateString("Invalid function arguements!"), V8_PROPERTY_ATTRIBUTE_NONE);
		
		retval = XCefV8Handler::CreateResultValue(false, "Invalid function arguements!");
 		return false;
	}

	CefWindowHandle hwnd = pthis_handle->GetCCWindowHandle();
	CefString str_title = arguments[0]->GetStringValue();

	if (NULL == hwnd)
	{
		retval = XCefV8Handler::CreateResultValue(false, "Can not get hwnd!");
		return false;
	}
 
	SetWindowTextW(hwnd, str_title.ToWString().c_str());
 
	retval = CefV8Value::CreateBool(true);
	return true;
}

/*static*/ bool		XWinCallback::async_title(
	XAsyncBrowserHandler *			pthis_handle,
	CefRefPtr<CefBrowser>			browser,
	CefRefPtr<CefFrame>				frame,
	int64							query_id,
	const CefString &				request,
	bool							persistent,
	CefRefPtr<BrowserCallback>		callback,
	XRPC_INFO &						rpc_info
	)
{
	CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
	CefRefPtr<CefListValue>	args;
	CefString str_title;
	do
	{
		if (!rpc_info.arguments_.IsList()
			|| !(args = rpc_info.arguments_.GetList())
			|| (args->GetSize() <= 0) 
			|| VTYPE_STRING != args->GetType(0)
			)
		{
			callback->Failure(0, XCefRpc::GetErrorByInfo(rpc_info.protocol_, XCefRpc::ERRINFO_INVALID_PARAMS));
			return false;
		}
		str_title = args->GetString(0);
		
	} while (0);
	
	SetWindowTextW(hwnd, str_title.c_str());
	callback->Success(XCefRpc::GetResult(XPROT_WIN(), true, rpc_info.id));
	
	return false;
}

struct WorkContext
{
private:
	WorkContext() : ref_count_(1){}
public:
	static WorkContext *	Create()
	{
		return new WorkContext;
	}

	void	V8AddRef()
	{
// 		context_->AddRef();
// 		callback_->AddRef();
	}
	void	V8Release()
	{
// 		context_->Release();
// 		callback_->Release();
	}

	// 执行 C++ 回调，必须在render thread中
	void	DispathCallback(double progress)
	{
		this->V8Release();

		{
			// eg: cefclient/binding_test.cpp
			
			// ERROR: window == NULL
			//CefRefPtr<CefV8Value>		window = CefV8Context::GetCurrentContext()->GetGlobal();
			CefV8ValueList				argList;	
			CefRefPtr<CefV8Value>		retval;
			CefRefPtr<CefV8Exception>	exceptionPtr;

			argList.push_back(CefV8Value::CreateDouble(progress));

			// 成员函数才需要进出上下文，还有 ExecuteFunction 改版了
// 			window->Enter();
// 			/*result =*/ window->ExecuteFunction(pWork->callback_, argList, retval, exceptionPtr, false);
// 			if (exceptionPtr.get())
// 				LOG(ERROR) << exceptionPtr->GetMessage();
// 			window->Exit();

			// http://stackoverflow.com/questions/19088594/c-create-cef-object-in-custom-thread
			// context init is "context = CefV8Context::GetEnteredContext();" 
			if (context_.get() && context_->Enter())
			{
				// call ExecuteFunctionWithContext and perform other actions
				/*result =*/ this->callback_->ExecuteFunction(NULL, argList);
				context_->Exit();
			}
		}
		this->Release();
		//this = NULL;
		
		return;
	}

	// 自定义线程函数
	static DWORD WINAPI Proc(LPVOID lpThreadParameter)
	{
		WorkContext * pWork = reinterpret_cast<WorkContext*>(lpThreadParameter);
		if (NULL == pWork)
			return 0;

		pWork->V8Release();

		bool result = false;

		for (double i = 0; i <= 100; i+=2)
		{
			if (std::abs(i - 100) > DBL_EPSILON)
			{
				pWork->AddRef();
			}
			else
			{
				// 让pWork自我删除
			}
			pWork->V8AddRef();
			result = CefPostTask(TID_RENDERER, NewCefRunnableMethod(pWork, &WorkContext::DispathCallback, i/100));
			DCHECK(result);

			Sleep(100);
		}


		pWork = NULL;

		return 0;
	}

public:
	CefRefPtr<CefV8Context>		context_;		// 回调所在上下文
	CefRefPtr<CefV8Value>		callback_;		// 回调

	void AddRef() const {
		InterlockedIncrement((LONG*)&ref_count_);
	}
	bool Release() const {
		if (0 == InterlockedDecrement((LONG*)&ref_count_))
		{
			delete this;
			return true;
		}
		return false;
	}
	volatile	LONG			ref_count_;
};

/*static*/ bool		XWinCallback::native_thread_callback(
	XCefV8Handler *				pthis_handle,
	CefRefPtr<CefV8Value>		object,
	const CefV8ValueList &		arguments,
	CefRefPtr<CefV8Value> &		retval,
	CefString &					exception
	)
{
	do
	{
		if (arguments.size() < 1 
			|| !arguments[0]->IsFunction())
		{
			break;
		}
		CefRefPtr<CefV8Value>		callback;

		callback = arguments[0];

		// 创建自定义线程
		WorkContext * pWork = WorkContext::Create();
		pWork->callback_ = callback;
		pWork->context_ = CefV8Context::GetEnteredContext();

		pWork->V8AddRef();
		//CloseHandle((HANDLE)_beginthreadex(...));
		QueueUserWorkItem(WorkContext::Proc, pWork, WT_EXECUTEINIOTHREAD);

		retval = CefV8Value::CreateBool(true);
		return true;
	} while (0);

	retval = CefV8Value::CreateBool(false);
	return true;
}




/*static*/ bool   XWinCallback::async_get_hwnd(
	XAsyncBrowserHandler *			pthis_handle,
	CefRefPtr<CefBrowser>			browser,
	CefRefPtr<CefFrame>				frame,
	int64							query_id,
	const CefString &				request,
	bool							persistent,
	CefRefPtr<BrowserCallback>		callback,
	XRPC_INFO &						rpc_info
	)
{
	CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
	
	callback->Success(XCefRpc::GetResult(XPROT_WIN(), std::to_string(reinterpret_cast<std::size_t>(hwnd)), rpc_info.id));
	return true;
}

/*static*/ bool   XWinCallback::cache_hwnd(
	XCefV8Handler *				pthis_handle,
	CefRefPtr<CefV8Value>		object,
	const CefV8ValueList &		arguments,
	CefRefPtr<CefV8Value> &		retval,
	CefString &					exception
	)
{
	CefRefPtr<CefListValue>	args;
	do
	{
		if ((arguments.size() <= 0)
			|| !arguments[0]->IsString()
			)
		{
			retval = XCefV8Handler::CreateResultValue(false, "Invalid function arguements!");
			return true;
		}
		XCefAppManage::Instance()->cache_browser_hwnd_ = (HWND)xstd::parse_string<size_t>(arguments[0]->GetStringValue().ToString());

	} while (0);

	retval = CefV8Value::CreateBool(true);
	return true;
}

/*static*/ bool XWinCallback::set_title_areas(
	XCefV8Handler *				pthis_handle,
	CefRefPtr<CefV8Value>		object,
	const CefV8ValueList &		arguments,
	CefRefPtr<CefV8Value> &		retval,
	CefString &					exception
	)
{

	CefWindowHandle hwnd = pthis_handle->GetCCWindowHandle();

	do 
	{
		if (NULL == hwnd)
		{
			retval = XCefV8Handler::CreateResultValue(false, "Can not get hwnd!");
			break;
		}
		if (arguments.size() < 2){
			retval = XCefV8Handler::CreateResultValue(false, "Invalid function arguements!");
			break;
		}
		int x = arguments[0]->GetIntValue();
		int y = arguments[1]->GetIntValue();

		XWinUtil::PostTitilAreas(hwnd, x, y);

		retval = XCefV8Handler::CreateResultValue(true);
	} while (0);
	return true;
}
