#include "stdafx.h"
#include "XCefCallback.h"



XCefV8Handler::XCefV8Handler() : cache_hwnd_(NULL){}
/*virtual*/ XCefV8Handler::~XCefV8Handler(){ cache_hwnd_ = NULL; }

CefWindowHandle				XCefV8Handler::GetCCWindowHandle()
{
	CefWindowHandle hwnd = NULL;
	if (!CefCurrentlyOn(TID_RENDERER))
	{
		hwnd = GetCCBrowser()->GetHost()->GetWindowHandle();
	}
	else if (XCefAppManage::Instance())
	{
		if (NULL == XCefAppManage::Instance()->cache_browser_hwnd_)
		{
			XCefAppManage::Instance()->cache_browser_hwnd_ = XWinUtil::GetMainWindowHwnd(
				XWinUtil::GetParentProcessID()
				);
		}

		hwnd = XCefAppManage::Instance()->cache_browser_hwnd_;
	}
	return hwnd;
}


/*virtual*/ bool XCefV8Handler::Execute(
	const CefString&		name, 
	CefRefPtr<CefV8Value>	object, 
	const CefV8ValueList&	arguments, 
	CefRefPtr<CefV8Value>&	retval, 
	CefString&				exception
	)
{
	CEF_REQUIRE_RENDERER_THREAD();

	FunctionHashMap & fn_map = GetData();
	FunctionHashMap::iterator itor = fn_map.find(name);
	if (itor == fn_map.end()){
		exception = "Invalid method name";
		return false;
	}
	DCHECK_CB_INFO_IS_CB(itor->second);
	itor->second.cb_(this, object, arguments, retval, exception);
	return true;
}

/*static*/ CefRefPtr<CefV8Value>		XCefV8Handler::CreateObject(CefRefPtr<CefV8Value> context, const CefString& name)
{
	CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(NULL);
	obj->SetValue("name", CefV8Value::CreateString(name), V8_PROPERTY_ATTRIBUTE_NONE);
	context->SetValue(name, obj, V8_PROPERTY_ATTRIBUTE_NONE);
	return obj;
}

/*static*/ CefRefPtr<CefV8Value>		XCefV8Handler::CreateResultValue(bool bretval, CefString message /*= L""*/)
{
	CefRefPtr<CefV8Value> retval = CefV8Value::CreateObject(NULL);
	retval->SetValue("result", CefV8Value::CreateBool(bretval), V8_PROPERTY_ATTRIBUTE_NONE);
	retval->SetValue("message", CefV8Value::CreateString(message), V8_PROPERTY_ATTRIBUTE_NONE);
	return retval;
}



//bool	XCefV8Handler::PostCBToProcess(
//	CefProcessId			process_id,
//	XCefV8Handler *			pthis_handle, 
//	const CefV8ValueList&	arguments, 
//	CefRefPtr<CefV8Value>&	retval
//	)
//{
//	CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(STR_MSG_SENDPROCESSTASK);
//	CefRefPtr<CefListValue> message_args = message->GetArgumentList();
//	CefRefPtr<CefListValue> func_args = CefListValue::Create();
//	{
//		for (size_t i = 0; i < arguments.size(); ++i)
//			v8cvt::SetListValue(func_args, i, arguments[i]);
//	}
//	CefRefPtr<CefListValue> describe_args = CefListValue::Create();
//	{
//		int64 frame_id = pthis_handle->GetCCFrame()->GetIdentifier();
//		describe_args->SetBinary(0,
//			CefBinaryValue::Create(reinterpret_cast<void*>(frame_id), size_t(int64))
//			);
//	}
//	message_args->SetList(0, describe_args);
//	message_args->SetList(1, func_args);
//	return GetCCBrowser()->SendProcessMessage(process_id, message);
//}
//
//static bool	XCefV8Handler::ParseCBMessage(
//	__in	CefRefPtr<CefListValue>		message_args,
//	__out	CefRefPtr<CefV8Value>		object,
//	__out	CefV8ValueList				arguments
//	)
//{
//	CefRefPtr<CefListValue> describe_args = message_args->GetList(0);
//	CefRefPtr<CefListValue> func_args = message_args->GetList(1);
//
//	CefRefPtr<CefBinaryValue> bval = describe_args->GetBinary(0);
//	unsigned char buf_val[sizeof(int64)] = { 0 };
//	bval->GetData(buf_val, sizeof(int64));
//	frame_id  = reinterpret_cast<int64>()
//}



//////////////////////////////////////////////////////////////////////////
/*virtual*/ bool XAsyncBrowserHandler::OnQuery(
	CefRefPtr<CefBrowser>			browser,
	CefRefPtr<CefFrame>				frame,
	int64							query_id,
	const CefString&				request,
	bool							persistent,
	CefRefPtr<BrowserCallback>		callback
	) /*OVERRIDE*/
{
	//const std::string &		message = request;
	XRPC_INFO 				rpc_info;
	CefString				exception_message;

	if (!XCefRpc::ParseJSONString(request, rpc_info, exception_message))
	{
		callback->Failure(0, exception_message);

		return false;
	}

	ProtocolFunctionHashMap & fn_protocol_map = GetData();
// 	{
// 		OutputDebugStringA(
// 			fn_protocol_map.begin()->first.c_str()
// 			);
// 		OutputDebugStringA("<<===>>");
// 		OutputDebugStringA(rpc_info.protocol_.ToString().c_str());
// 	}
	auto it_protocol = fn_protocol_map.find(rpc_info.protocol_);
	if (it_protocol == fn_protocol_map.end())
	{
		callback->Failure(
			0,
			XCefRpc::GetErrorByInfo(rpc_info.protocol_, XCefRpc::ERRINFO_INVALID_PROTOCOL)
			);
		return false;
	}
	FunctionHashMap & fn_map = it_protocol->second;
	auto it_fn = fn_map.find(rpc_info.method_);
	if (it_fn == fn_map.end())
	{
		callback->Failure(
			0,
			XCefRpc::GetError(
				rpc_info.protocol_,
				XCefRpc::ERRID_SERVER_ERROR,
				CefString("[Server]:Invalid method name")
				)
			);
		return false;
	}
	DCHECK_CB_INFO_IS_ACB(it_fn->second);
	it_fn->second.acb_query_(
		this, browser, frame, query_id, request, persistent, callback, 
		rpc_info
		);
	return true;
}

bool		XAsyncBrowserHandler::AddFn(const CefString & protocol, const CefString & name, XCB_INFO & fun_info)
{
	if (protocol.empty() || name.empty())
	{
		DCHECK(!protocol.empty() && !name.empty());
		return false;
	}
// 	ProtocolFunctionHashMap::iterator it_map = fn_protocol_map_.find(protocol);
// 	FunctionHashMap * fn_map = NULL;
// 	if (fn_protocol_map_.end() == it_map)
// 	{
// 		auto it_inserted = fn_protocol_map_.insert(std::make_pair(name, FunctionHashMap()));
// 		DCHECK(it_inserted.second);
// 		fn_map = &it_inserted.first->second;
// 	}
// 	else
// 	{
// 		fn_map = &it_map->second;
// 	}
// 	fn_map->insert(std::make_pair(name, fun_info));
	fn_protocol_map_[protocol].insert(std::make_pair(name, fun_info));
	return true;
}



//////////////////////////////////////////////////////////////////////////
/*virtual*/ void	XCefCallback::CBLoad(CefRefPtr<CefV8Value> app, CefRefPtr<CefV8Context> context)
{
	CefRefPtr<XCefV8Handler>	v8handler = GetCBHandle(false);
	if (v8handler)
	{
		// cache CefWindowHandle
		if (CefCurrentlyOn(TID_UI))
		{
			v8handler->cache_hwnd_ = context->GetBrowser()->GetHost()->GetWindowHandle();
		}
		

		FunctionHashMap & fnmap = v8handler->GetData();
		for (FunctionHashMap::value_type & ipair : fnmap)
		{
			XCB_INFO & info = ipair.second;
			DCHECK_CB_INFO_IS_CB(info);
			CefRefPtr<CefV8Value> fun = CefV8Value::CreateFunction(info.name_, v8handler);
			app->SetValue(info.name_, fun, V8_PROPERTY_ATTRIBUTE_NONE);
		}
	}
}
/*virtual*/ void    XCefCallback::ACBLoad(MessageHandlerSet & msg_handle_set)
{
	XAsyncBrowserHandler *	handler = GetACBHandle(false);
	if (handler)
	{
		if (!handler->GetData().empty())
		{
			msg_handle_set.insert(handler);
			DetachACB();
		}
		else
		{
			ReleaseACB();
		}
	}
}


CefRefPtr<XCefV8Handler>		XCefCallback::GetCBHandle(bool to_create)
{
	if (!cb_handle_ && to_create)
		cb_handle_ = new XCefV8Handler;
	return cb_handle_;
}
XAsyncBrowserHandler *			XCefCallback::GetACBHandle(bool to_create)
{
	if (nullptr == acb_handle_ && to_create)
		acb_handle_ = new XAsyncBrowserHandler;
	return acb_handle_;
}

void							XCefCallback::ReleaseCB()
{
	cb_handle_ = NULL;
}
void							XCefCallback::ReleaseACB()
{
	if (acb_handle_)
		delete	acb_handle_;
	acb_handle_ = NULL;
}
XAsyncBrowserHandler *			XCefCallback::DetachACB()
{
	XAsyncBrowserHandler * pold = acb_handle_;
	acb_handle_ = NULL;
	return pold;
}