#pragma once
/*
Syntax:

--> data sent to Server
<-- data sent to Client
带索引数组参数的rpc调用:

--> {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1}
<-- {"jsonrpc": "2.0", "result": 19, "id": 1}

--> {"jsonrpc": "2.0", "method": "subtract", "params": [23, 42], "id": 2}
<-- {"jsonrpc": "2.0", "result": -19, "id": 2}
带关联数组参数的rpc调用:

--> {"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3}
<-- {"jsonrpc": "2.0", "result": 19, "id": 3}

--> {"jsonrpc": "2.0", "method": "subtract", "params": {"minuend": 42, "subtrahend": 23}, "id": 4}
<-- {"jsonrpc": "2.0", "result": 19, "id": 4}
通知:

--> {"jsonrpc": "2.0", "method": "update", "params": [1,2,3,4,5]}
--> {"jsonrpc": "2.0", "method": "foobar"}
不包含调用方法的rpc调用:

--> {"jsonrpc": "2.0", "method": "foobar", "id": "1"}
<-- {"jsonrpc": "2.0", "error": {"code": -32601, "message": "Method not found"}, "id": "1"}
包含无效json的rpc调用:

--> {"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz]
<-- {"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null}
包含无效请求对象的rpc调用:

--> {"jsonrpc": "2.0", "method": 1, "params": "bar"}
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
包含无效json的rpc批量调用:

--> [
{"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
{"jsonrpc": "2.0", "method"
]
<-- {"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null}
包含空数组的rpc调用:

--> []
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
非空且无效的rpc批量调用:

--> [1]
<-- [
{"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
]
无效的rpc批量调用:

--> [1,2,3]
<-- [
{"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null},
{"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null},
{"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
]
rpc批量调用:

--> [
{"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
{"jsonrpc": "2.0", "method": "notify_hello", "params": [7]},
{"jsonrpc": "2.0", "method": "subtract", "params": [42,23], "id": "2"},
{"foo": "boo"},
{"jsonrpc": "2.0", "method": "foo.get", "params": {"name": "myself"}, "id": "5"},
{"jsonrpc": "2.0", "method": "get_data", "id": "9"}
]
<-- [
{"jsonrpc": "2.0", "result": 7, "id": "1"},
{"jsonrpc": "2.0", "result": 19, "id": "2"},
{"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null},
{"jsonrpc": "2.0", "error": {"code": -32601, "message": "Method not found"}, "id": "5"},
{"jsonrpc": "2.0", "result": ["hello", 5], "id": "9"}
]
所有都为通知的rpc批量调用:

--> [
{"jsonrpc": "2.0", "method": "notify_sum", "params": [1,2,4]},
{"jsonrpc": "2.0", "method": "notify_hello", "params": [7]}
]

<-- //Nothing is returned for all notification batches
*/
#include "XCefInfo.h"



/*
typedef enum {
VTYPE_INVALID = 0,
VTYPE_NULL,
VTYPE_BOOL,
VTYPE_INT,
VTYPE_DOUBLE,
VTYPE_STRING,
VTYPE_BINARY,
VTYPE_DICTIONARY,
VTYPE_LIST,
} cef_value_type_t;
*/

class XCefRpc
{
public:
	enum ERPC_ERROR
	{
		ERRINFO_PARSE_ERROR,
		ERRINFO_INVALID_REQUEST,
		ERRINFO_METHOD_NOT_FOUND,
		ERRINFO_INVALID_PARAMS,
		ERRINFO_INTERANL_ERROR,
		ERRINFO_SERVER_ERROR,
		// 自定义
		ERRINFO_INVALID_PROTOCOL,
		ERRINFO_NULL
	};
	enum ERPC_ERROR_ID
	{
		ERRID_PARSE_ERROR = - 32700,
		ERRID_INVALID_REQUEST = -32600,
		ERRID_METHOD_NOT_FOUND = -32601,
		ERRID_INVALID_PARAMS = -32602,
		ERRID_INTERANL_ERROR = -32603,
		ERRID_SERVER_ERROR = -32000,		// -32000 to -32099
		// 自定义
		ERRID_INVALID_PROTOCOL = -32001,
		ERRID_NULL = 0
	};
	struct RPC_ERROR_INFO
	{
		int				code_;
		std::string		message_;
	};
public:
	XCefRpc();
	~XCefRpc();

	//// @arguments - 参数值
	//// @argument_names - [opt]参数如果是字典，这里是key集合
	//static bool				ParseJSONString(
	//	__in		CefString &				message,
	//	__out		std::string	&			method_name,
	//	__out		CefV8ValueList &		arguments,
	//	__out_opt	CefV8ValueList &		argument_keys,
	//	__out		CefString &				result_message
 //		 );
	static bool				ParseJSONString(
		__in		const CefString &		message,
		__out		XRPC_INFO & 			rpc_info,
		__out		CefString &				exception_message
		);

// 	static std::string		GetResult(
// 		CefV8ValueList &			result_array,
// 		int							id
// 		);
// 	static std::string		GetResult(
// 		CefRefPtr<CefV8Value> &		result,
// 		int							id
// 		);
	static std::string		GetResult(
		std::string					protocol,
		cJSON *						json_params,
		int							rpc_id
		);
	static std::string		GetResult(
		std::string					protocol,
	 	XCefValue &					result_var,
	 	int							rpc_id
	 	);
	static std::string		GetResult(
		std::string					protocol,
		std::string					strresult,
		int							rpc_id
		);
	static std::string		GetResult(
		std::string					protocol,
		bool						bresult,
		int							rpc_id
		);
	static std::string		GetResult(
		std::string					protocol,
		int							bresult,
		int							rpc_id
		);


	static CefString		GetErrorByInfo(std::string protocol, ERPC_ERROR kErr);
	static CefString		GetError(std::string protocol, int code, std::string message);

	static inline RPC_ERROR_INFO &	GetErrorInfo(ERPC_ERROR kErr){ return RPC_ERRORS[kErr]; }
public:
	// name
	static std::string			NAME_PROTOCOL;

	// value
	// render process send message to browser process
	static std::string			VALUE_PROTOCOL_APP;
	// browser process send message to render process
	static std::string			VALUE_PROTOCOL_WIN;
	

	static RPC_ERROR_INFO		RPC_ERRORS[];
};

// 函数缩短宏
#define XPROT_NULL()	std::string()
#define XPROT_APP()		XCefRpc::VALUE_PROTOCOL_APP
#define XPROT_WIN()		XCefRpc::VALUE_PROTOCOL_WIN



namespace xcef_cvt
{
	// Transfer a V8 value to a List index.
	extern void			V8ObjectToListItem(__in CefRefPtr<CefV8Value> value, CefRefPtr<CefListValue> list, int index);
	// Transfer a V8 array to a List.
	extern void			V8ArrayToList(__in CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target);
	// Transfer a List value to a V8 array index.
	extern void			ListItemToV8ArrayItem(__in CefRefPtr<CefListValue> value, CefRefPtr<CefV8Value> list, int index);
	// Transfer a List to a V8 array.
	extern void			ListToV8Array(__in CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target);

	// Transfer a Json to a V8 object.
	extern void			JsonToV8Object(__in cJSON * object, __out CefRefPtr<CefV8Value> & value);
	// Transfer a Json to a V8 object.
	extern cJSON *		V8ObjectToJson(CefRefPtr<CefV8Value> & value);
	// Transfer a V8 object list to a Json Array.
	extern cJSON *		V8ObjectListToJsonArray(CefV8ValueList & value_list);

	// Transfer a Json conllection to XRpcValue
	extern void			JsonCollectionToXRpcValue(__in cJSON * object, __out XCefValue & rpc_value);
	// Transfer a Json to a List index.
	extern void			JsonToListItem(__in cJSON * object, __out CefRefPtr<CefListValue> & list, int index);
	// Transfer a Json to a Dict key.
	extern void			JsonToDictionaryItem(__in cJSON * object, __out CefRefPtr<CefDictionaryValue> & dict, std::string key);
	// Transfer a XRpcValue to Json
	extern cJSON *		XRpcValueToJson(__in XCefValue & rpc_value);
	// Transfer a List to Json
	extern cJSON *		ListToJson(__in CefRefPtr<CefListValue> & list);
	// Transfer a Json to a Dict key.
	extern cJSON *		DictionaryToJson(__in CefRefPtr<CefDictionaryValue> & dict);

}	/* xcef_cvt */
