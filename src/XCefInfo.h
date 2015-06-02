#pragma once
#include "XCefRpc.h"



class XCefValue
{
public:
	XCefValue();

	virtual bool					IsValid();
	virtual bool					IsList();
	virtual bool					IsDictionary();
	virtual CefValueType			GetType();

	virtual void					Clear();
	// set and get，会强制转换
	virtual	void								SetList(CefRefPtr<CefListValue> & lst);
	virtual CefRefPtr<CefListValue> 			GetList();
	virtual	void								SetDictionary(CefRefPtr<CefDictionaryValue> & dict);
	virtual CefRefPtr<CefDictionaryValue> 		GetDictionary();

	XCefValue& operator=(CefRefPtr<CefListValue> & lst);
	XCefValue& operator=(CefRefPtr<CefDictionaryValue> & dict);
public:
	CefValueType					ktype_;

	CefRefPtr<CefListValue>			list_;
	CefRefPtr<CefDictionaryValue>	dict_;
};


class XCefV8Handler;
class XAsyncBrowserHandler;

typedef CefMessageRouterBrowserSide::Callback BrowserCallback;

// request 中 jsonrpc 拆分出来的数据
struct XRPC_INFO
{
	CefString		protocol_;		// 协议
	CefString		method_;		// method name
	XCefValue		arguments_;		// arguments (dict or list)
	int				id;				// rpc_id
};

typedef bool(*PFN_CB)(
	XCefV8Handler *					pthis_handle,
	CefRefPtr<CefV8Value>			object,
	const CefV8ValueList&			arguments,
	CefRefPtr<CefV8Value> &			retval,
	CefString &						exception
	);
typedef bool(*PFN_ACB_QUERY)(
	XAsyncBrowserHandler *			pthis_handle,
	CefRefPtr<CefBrowser>			browser,
	CefRefPtr<CefFrame>				frame,
	int64							query_id,
	const CefString &				request,
	bool							persistent,
	CefRefPtr<BrowserCallback>		callback,
	// 拆分后数据
	XRPC_INFO &						rpc_info
	);
typedef bool(*PFN_ACB_QUERY_CANCELED)(
	XAsyncBrowserHandler *			pthis_handle,
	CefRefPtr<CefBrowser>			browser,
	CefRefPtr<CefFrame>				frame,
	int64							query_id
	);
struct XCB_INFO{
	bool							is_async_;
	std::string						name_;
	PFN_CB							cb_;
	PFN_ACB_QUERY					acb_query_;
	PFN_ACB_QUERY_CANCELED			acb_query_canceled_;
};
typedef std::unordered_map<std::string, XCB_INFO>				FunctionHashMap;
typedef std::unordered_map<std::string, FunctionHashMap>		ProtocolFunctionHashMap;		// <协议, <name, 回调描述> >

typedef std::set<CefMessageRouterBrowserSide::Handler*>			MessageHandlerSet;

#define DCHECK_CB_INFO_IS_CB(info)		DCHECK(!(info).is_async_ && (info).cb_);
#define DCHECK_CB_INFO_IS_ACB(info)		DCHECK((info).is_async_ && (info).acb_query_)