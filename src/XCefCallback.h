#pragma once
#include "XCefInfo.h"



// class FunctionHashMapContain
// {
// public:
// 	virtual ~FunctionHashMapContain(){ fn_map_.clear(); }
// 
// 	bool							AddFn(const CefString& name, CBINFO & fun_info)
// 	{
// 		if (name.empty())
// 			return false;
// 		fn_map_.insert(std::make_pair(name, fun_info));
// 		return true;
// 	}
// 	inline FunctionHashMap &		GetData(){ return fn_map_; }
// private:
// 	FunctionHashMap				fn_map_;
// };



class XCefV8Handler :	public CefV8Handler
{
public:
	XCefV8Handler();
	~XCefV8Handler();

	// get interface methods
	inline CefRefPtr<CefV8Context>		GetCC(){ return CefV8Context::GetCurrentContext(); }
	inline CefRefPtr<CefFrame>			GetCCFrame(){ return GetCC()->GetFrame(); }
	inline CefRefPtr<CefBrowser>		GetCCBrowser(){ return GetCC()->GetBrowser(); }
	CefWindowHandle						GetCCWindowHandle();


	// CefV8Handler methods:
	virtual bool	Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception);

	// about v8 methods:
	static CefRefPtr<CefV8Value>		CreateObject(CefRefPtr<CefV8Value> context, const CefString& name);
	// 返回错误对象
	static CefRefPtr<CefV8Value>		CreateResultValue(bool bretval, CefString message = L"");

public:
	bool							AddFn(const CefString& name, XCB_INFO & fun_info)
	{
		if (name.empty())
		{
			DCHECK(!name.empty());
			return false;
		}
		fn_map_.insert(std::make_pair(name, fun_info));
		return true;
	}
	inline FunctionHashMap &		GetData(){ return fn_map_; }
public:
	CefWindowHandle						cache_hwnd_;
private:
	FunctionHashMap				fn_map_;
	// Include the default reference counting implementation.  
	IMPLEMENT_REFCOUNTING(XCefV8Handler);
};



class XAsyncBrowserHandler :	public CefMessageRouterBrowserSide::Handler
{
public:
	XAsyncBrowserHandler() = default;
	virtual ~XAsyncBrowserHandler() = default;

	virtual bool OnQuery(
		CefRefPtr<CefBrowser>			browser,
		CefRefPtr<CefFrame>				frame,
		int64							query_id,
		const CefString&				request,
		bool							persistent,
		CefRefPtr<BrowserCallback>		callback
		) OVERRIDE;


	bool		AddFn(const CefString & protocol, const CefString & name, XCB_INFO & fun_info);
	inline ProtocolFunctionHashMap &		GetData(){ return fn_protocol_map_; }
public:
	ProtocolFunctionHashMap				fn_protocol_map_;
};



class XCefCallback
{
public:
	virtual void	Init() = 0;

	virtual void	CBLoad(CefRefPtr<CefV8Value> app, CefRefPtr<CefV8Context> context);
	virtual void    ACBLoad(MessageHandlerSet & msg_handle_set);

	CefRefPtr<XCefV8Handler>		GetCBHandle(bool to_create);
	XAsyncBrowserHandler *			GetACBHandle(bool to_create);

	void							ReleaseCB();
	void							ReleaseACB();
	XAsyncBrowserHandler *			DetachACB();
private:
	CefRefPtr<XCefV8Handler>		cb_handle_;
	XAsyncBrowserHandler *			acb_handle_;
};


#define xcef_v8handle_case(sptr)	static_cast<XCefV8Handler*>(sptr.get())


// 注册函数宏

#define DECLARE_XV8()	\
public:\
	virtual void	Init() OVERRIDE;

#define BEGIN_XV8(class)	\
/*virtual*/ void	class::Init() /*OVERRIDE*/{\
	std::string		protocol;

#define END_XV8()	}

#define _SET_XV8_CBINFO(is_async, name, cb, acb_query, acb_query_canceled)	\
		XCB_INFO info = { is_async, name, cb, acb_query, acb_query_canceled };

#define HANDLE_XV8_CB(name, cb)		\
	{\
		_SET_XV8_CBINFO(false, name, cb, NULL, NULL);\
		GetCBHandle(true)->AddFn(name, info);\
	}

#define HANDLE_XV8_CB_ASYNC(name, acb)		\
	{\
		_SET_XV8_CBINFO(true, name, NULL, acb, NULL);\
		DCHECK(!protocol.empty());\
		GetACBHandle(true)->AddFn(protocol, name, info);\
	}

// 转发到对应协议的PRC
#define FORWARD_XV8_PROTOCOL(name)		protocol = name;
// app://  render to browser
#define FORWARD_XV8_PROTOCOL_APP()		protocol = XPROT_APP();
// win://  browser to render
#define FORWARD_XV8_PROTOCOL_WIN()		protocol = XPROT_WIN();