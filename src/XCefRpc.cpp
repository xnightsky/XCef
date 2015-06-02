#include "stdafx.h"
#include "XCefRpc.h"


 
/*static*/ std::string			XCefRpc::NAME_PROTOCOL				=			"protocol";
/*static*/ std::string			XCefRpc::VALUE_PROTOCOL_APP			=			"app://";
/*static*/ std::string			XCefRpc::VALUE_PROTOCOL_WIN			=			"win://";

/*
code				message						meaning
-32700				Parse error语法解析错误		服务端接收到无效的json。该错误发送于服务器尝试解析json文本
-32600				Invalid Request无效请求		发送的json不是一个有效的请求对象。
-32601				Method not found找不到方法	该方法不存在或无效
-32602				Invalid params无效的参数	无效的方法参数。
-32603				Internal error内部错误		JSON-RPC内部错误。
-32000 to -32099	Server error服务端错误		预留用于自定义的服务器错误。
*/
/*static*/ XCefRpc::RPC_ERROR_INFO		XCefRpc::RPC_ERRORS[] = {
	{ XCefRpc::ERRID_PARSE_ERROR, "Parse error" },
	{ XCefRpc::ERRID_INVALID_REQUEST, "Invalid Request" },
	{ XCefRpc::ERRID_METHOD_NOT_FOUND, "Method not found" },
	{ XCefRpc::ERRID_INVALID_PARAMS, "Invalid params" },
	{ XCefRpc::ERRID_INTERANL_ERROR, "Internal error" },
	// -32000 to -32099
	{ XCefRpc::ERRID_SERVER_ERROR, "Server error" },
	// 自定义
	{ XCefRpc::ERRID_INVALID_PROTOCOL, "Invalid Protocol" }
};

namespace{
	std::string				NAME_METHOD			=		"method";
	std::string				NAME_PARAMS			=		"params";
	std::string				NAME_ID				=		"id";
	std::string				NAME_RESULT			=		"result";

	std::string				LOG_PRE_JSON		=		"[app://json]:";

	void XJsonRpcInit(cJSON * msg, std::string protocol)
	{
		DCHECK(msg);
		cJSON_AddStringToObject(msg, "jsonrpc", "2.0");
		cJSON_AddItemToObject(
			msg,
			XCefRpc::NAME_PROTOCOL.c_str(),
			protocol.empty() ? cJSON_CreateNull() : cJSON_CreateString(protocol.c_str())
			);
	}
}






XCefRpc::XCefRpc()
{
}
XCefRpc::~XCefRpc()
{
}

///*static*/ bool		XCefRpc::ParseJSONString(
//	__in		CefString &				message,
//	__out		std::string	&			method_name,
//	__out		CefV8ValueList &		arguments,
//	__out_opt	CefV8ValueList &		argument_keys,
//	__out		CefString &				result_message
//	)
//{
//	method_name.clear();
//	arguments.clear();
//	argument_keys.clear();
//	result_message.clear();
//
//	bool	result = false;
//	cJSON * json = cJSON_Parse(message.ToString().c_str());
//	do
//	{
//		if (nullptr == json)
//		{
//			result_message = GetError(XJSON_ERR_PARSE_ERROR);
//			break;
//		}
// 		cJSON * json_protocol = cJSON_GetObjectItem(json, NAME_PROTOCOL.c_str());
// 		if (nullptr == json_protocol || 0 != VALUE_PROTOCOL.compare(json_protocol->valuestring))
// 		{
// 			result_message = GetError(XJSON_ERR_INVALID_PROTOCOL);
// 			break;
// 		}
//
//		cJSON * json_method = cJSON_GetObjectItem(json, NAME_METHOD.c_str());
//		method_name = json_method->valuestring ? json_method->valuestring : "";
//		if (nullptr == json_method || method_name.empty())
//		{
//			result_message = GetError(XJSON_ERR_METHOD_NOT_FOUND);
//			break;
//		}
//		cJSON * json_args = cJSON_GetObjectItem(json, NAME_PARAMS.c_str());
//		do
//		{
//			if (nullptr == json_args)
//				break;
//			int icurrent = 0;
//			cJSON_ForEach(it_param, json_args)
//			{
//				CefRefPtr<CefV8Value> key, value;
//
//				if (it_param->string)
//					key = CefV8Value::CreateString(it_param->string);
//				else
//					key = CefV8Value::CreateInt(icurrent);
//				++icurrent;
//
//				xcef_cvt::JsonToV8Object(it_param, value);
//
//				arguments.push_back(value);
//				argument_keys.push_back(key);
//			}
//
//		} while (0);
//
//		result = true;
//	} while (0);
//
//	if (nullptr != json)
//		cJSON_Delete(json), json = NULL;
//	return result;
//}

/*static*/ bool				XCefRpc::ParseJSONString(
	__in		const CefString &		message,
	__out		XRPC_INFO & 			rpc_info,
	__out		CefString &				exception_message
	)
{
	rpc_info.protocol_.clear();
	rpc_info.method_.clear();
	rpc_info.arguments_.Clear();
	exception_message.clear();

	bool	result = false;
	cJSON * json = cJSON_Parse(message.ToString().c_str());
	do
	{
		if (nullptr == json)
		{
			exception_message = GetErrorByInfo(XPROT_NULL(), ERRINFO_PARSE_ERROR);
			break;
		}
		cJSON * json_protocol = cJSON_GetObjectItem(json, NAME_PROTOCOL.c_str());
		if (nullptr == json_protocol)
		{
			exception_message = GetErrorByInfo(XPROT_NULL(), ERRINFO_INVALID_PROTOCOL);
			break;
		}
		rpc_info.protocol_ = cJSON_ValueToString(json_protocol);

		cJSON * json_method = cJSON_GetObjectItem(json, NAME_METHOD.c_str());
		rpc_info.method_ = json_method->valuestring ? json_method->valuestring : "";
		if (nullptr == json_method || rpc_info.method_.empty())
		{
			exception_message = GetErrorByInfo(rpc_info.protocol_, ERRINFO_METHOD_NOT_FOUND);
			break;
		}
		cJSON * json_args = cJSON_GetObjectItem(json, NAME_PARAMS.c_str());
		do
		{
			if (nullptr == json_args)
				break;
			xcef_cvt::JsonCollectionToXRpcValue(json_args, rpc_info.arguments_);

		} while (0);

		cJSON * json_rpc_id = cJSON_GetObjectItem(json, NAME_ID.c_str());
		if (cJSON_TypeIsInt(json_rpc_id))
			rpc_info.id = json_rpc_id->valueint;
		else
			rpc_info.id = 0;

		result = true;
	} while (0);

	if (nullptr != json)
		cJSON_Delete(json), json = NULL;
	return result;
}

// /*static*/ std::string		XCefRpc::GetResult(
// 	CefV8ValueList &			result_array,
// 	int							id
// 	)
// {
// 	cJSON * msg = cJSON_CreateObject();
// 	XJsonRpcInit(msg);
// 
// 	size_t num_result = result_array.size();
// 	if (num_result > 0)
// 	{
// 		cJSON * arr = xcef_cvt::V8ObjectListToJsonArray(result_array);
// 		cJSON_AddItemToObject(msg, NAME_RESULT.c_str(), arr);
// 	}
// 	else
// 	{
// 		cJSON_AddItemToObject(msg, NAME_RESULT.c_str(), cJSON_CreateNull());
// 	}
// 	cJSON_AddNumberToObject(msg, NAME_ID.c_str(), id);
// 
// 	cJSONStringPtr sp_result = msg;
// 	cJSON_Delete(msg), msg = NULL;
// 	return sp_result;
// }
// /*static*/ std::string		XCefRpc::GetResult(
// 	CefRefPtr<CefV8Value> &		result,
// 	int							id
// 	)
// {
// 	cJSON * msg = cJSON_CreateObject();
// 	XJsonRpcInit(msg);
// 	cJSON_AddItemToObject(msg, NAME_RESULT.c_str(), xcef_cvt::V8ObjectToJson(result));
// 
// 	cJSON_AddNumberToObject(msg, NAME_ID.c_str(), id);
// 
// 	cJSONStringPtr sp_result = msg;
// 	cJSON_Delete(msg), msg = NULL;
// 	return sp_result;
// }



/*static*/ std::string		XCefRpc::GetResult(
	std::string					protocol,
	cJSON *						json_params,
	int							rpc_id
	)
{
	DCHECK(json_params);

	cJSON * msg = cJSON_CreateObject();
	XJsonRpcInit(msg, protocol);
	cJSON_AddItemToObject(msg, NAME_RESULT.c_str(), json_params);
	cJSON_AddNumberToObject(msg, NAME_ID.c_str(), rpc_id);

	cJSONPrintStringPtr sp_result = msg;
	cJSON_Delete(msg), msg = NULL;
	return sp_result;
}
/*static*/ std::string		XCefRpc::GetResult(
	std::string					protocol,
	XCefValue &					result_var,
	int							rpc_id
	)
{
	return GetResult(protocol, xcef_cvt::XRpcValueToJson(result_var), rpc_id);
}
/*static*/ std::string		XCefRpc::GetResult(
	std::string					protocol,
	std::string					strresult,
	int							rpc_id
	)
{
	return GetResult(protocol, cJSON_CreateString(strresult.c_str()), rpc_id);
}
/*static*/ std::string		XCefRpc::GetResult(
	std::string					protocol,
	bool						bresult,
	int							rpc_id
	)
{
	return GetResult(protocol, bresult ? cJSON_CreateTrue() : cJSON_CreateFalse(), rpc_id);
}
/*static*/ std::string		XCefRpc::GetResult(
	std::string					protocol,
	int							nresult,
	int							rpc_id
	)
{
	return GetResult(protocol, cJSON_CreateNumber(nresult), rpc_id);
}

/*static*/ CefString	XCefRpc::GetErrorByInfo(std::string protocol, XCefRpc::ERPC_ERROR kErr)
{
	RPC_ERROR_INFO &	info = RPC_ERRORS[kErr];
	return GetError(protocol, info.code_, info.message_);
}
/*static*/ CefString		XCefRpc::GetError(std::string protocol, int code, std::string message)
{
	const char * errstr = cJSON_GetErrorPtr();

	if (nullptr != errstr)
	{
		LOG(ERROR) << LOG_PRE_JSON << message << " - " << errstr;
	}
	else
	{
		LOG(ERROR) << LOG_PRE_JSON << message;
	}

	cJSON * msg = cJSON_CreateObject();
	XJsonRpcInit(msg, protocol);
	//{ protocol: "app://" ,"jsonrpc": "2.0", "error": {"code": -ddd, "message": "xxx"}, "id": null}
	{
		cJSON * error = cJSON_CreateObject();
		cJSON_AddNumberToObject(error, "code", code);
		cJSON_AddStringToObject(error, "message", message.c_str());

		cJSON_AddItemToObject(msg, "error", error);
	}
	cJSON_AddNullToObject(msg, NAME_ID.c_str());

	CefString result = cJSONPrintStringPtr(msg);
	cJSON_Delete(msg), msg = NULL;
	return result;
}




namespace xcef_cvt
{
	// Transfer a V8 value to a List index.
	/*extern*/ void V8ObjectToListItem(__in CefRefPtr<CefV8Value> value, CefRefPtr<CefListValue> list, int index)
	{
		if (value->IsArray()) {
			CefRefPtr<CefListValue> new_list = CefListValue::Create();
			V8ArrayToList(value, new_list);
			list->SetList(index, new_list);
		}
		else if (value->IsString()) {
			list->SetString(index, value->GetStringValue());
		}
		else if (value->IsBool()) {
			list->SetBool(index, value->GetBoolValue());
		}
		else if (value->IsInt()) {
			list->SetInt(index, value->GetIntValue());
		}
		else if (value->IsDouble()) {
			list->SetDouble(index, value->GetDoubleValue());
		}
	}

	// Transfer a V8 array to a List.
	/*extern*/ void V8ArrayToList(__in CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target)
	{
		assert(source->IsArray());

		int arg_length = source->GetArrayLength();
		if (arg_length == 0)
			return;

		// Start with null types in all spaces.
		target->SetSize(arg_length);

		for (int i = 0; i < arg_length; ++i)
			V8ObjectToListItem(source->GetValue(i), target, i);
	}

	// Transfer a List value to a V8 array index.
	/*extern*/ void ListItemToV8ArrayItem(__in CefRefPtr<CefListValue> value, CefRefPtr<CefV8Value> list, int index)
	{
		CefRefPtr<CefV8Value> new_value;

		CefValueType type = value->GetType(index);
		switch (type) {
		case VTYPE_LIST: {
			CefRefPtr<CefListValue> list = value->GetList(index);
			new_value = CefV8Value::CreateArray(static_cast<int>(list->GetSize()));
			V8ArrayToList(new_value, list);
		} break;
		case VTYPE_BOOL:
			new_value = CefV8Value::CreateBool(value->GetBool(index));
			break;
		case VTYPE_DOUBLE:
			new_value = CefV8Value::CreateDouble(value->GetDouble(index));
			break;
		case VTYPE_INT:
			new_value = CefV8Value::CreateInt(value->GetInt(index));
			break;
		case VTYPE_STRING:
			new_value = CefV8Value::CreateString(value->GetString(index));
			break;
		default:
			break;
		}

		if (new_value.get()) {
			list->SetValue(index, new_value);
		}
		else {
			list->SetValue(index, CefV8Value::CreateNull());
		}
	}

	// Transfer a List to a V8 array.
	/*extern*/ void ListToV8Array(__in CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target)
	{
		assert(target->IsArray());

		int arg_length = static_cast<int>(source->GetSize());
		if (arg_length == 0)
			return;

		for (int i = 0; i < arg_length; ++i)
			ListItemToV8ArrayItem(source, target, i);
	}

	// Transfer a Json to a V8 object.
	/*extern*/ void	JsonToV8Object(__in cJSON * object, __out CefRefPtr<CefV8Value> & value)
	{
		if (nullptr == object)
			return;
		if (!value)
			value = CefV8Value::CreateNull();

		switch (cJSON_GetSimpleType(object))
		{
		case cJSON_Number:
			if (cJSON_TypeIsInt(object))
				value->CreateInt(object->valueint);
			else
				value->CreateDouble(object->valuedouble);
			break;
		case cJSON_True:
		case cJSON_False:
			value->CreateBool(object->valueint ? true : false);
			break;
		case cJSON_String:
			value->CreateString(object->valuestring);
			break;
		case cJSON_NULL:
			value->CreateInt(0);
			break;
		case cJSON_Array:
		case cJSON_Object:
		{
			CefString str = cJSONPrintStringPtr(object);
			value->CreateString(str);
		}
			break;
		default:
			return;
		}
		return;
	}

	// Transfer a Json to a V8 object.
	/*extern*/ cJSON * V8ObjectToJson(CefRefPtr<CefV8Value> & value)
	{
		cJSON * result = nullptr;

		do
		{
			if (!value || value->IsValid())
				break;

			if (value->IsArray()) {
				int arg_length = value->GetArrayLength();
				if (arg_length == 0)
					break;;

				result = cJSON_CreateArray();
				cJSON * curr = NULL, *prev = NULL;
				for (int i = 0; i < arg_length; ++i)
				{
					curr = V8ObjectToJson(value->GetValue(i));
					if (nullptr == prev)
					{
						result->child = curr;
					}
					else
					{
						prev->next = curr;
					}
					prev = curr;
				}
			}
			else if (value->IsString()) {
				result = cJSON_CreateString(value->GetStringValue().ToString().c_str());
			}
			else if (value->IsBool()) {
				result = value->GetBoolValue() ? cJSON_CreateTrue() : cJSON_CreateFalse();
			}
			else if (value->IsInt()) {
				result = cJSON_CreateNumber(value->GetIntValue());
			}
			else if (value->IsDouble()) {
				result = cJSON_CreateNumber(value->GetDoubleValue());
			}
		} while (0);

		return nullptr != result ? result : cJSON_CreateNull();
	}

	// Transfer a V8 object list to a Json Array.
	/*extern*/ cJSON * V8ObjectListToJsonArray(CefV8ValueList & value_list)
	{
		cJSON * result = nullptr;

		do
		{
			if (value_list.empty())
				break;

			result = cJSON_CreateArray();
			cJSON * curr = NULL, *prev = NULL;
			for (CefRefPtr<CefV8Value> & itsp : value_list)
			{
				curr = V8ObjectToJson(itsp);
				if (nullptr == prev)
				{
					result->child = curr;
				}
				else
				{
					prev->next = curr;
				}
				prev = curr;
			}
		} while (0);

		return nullptr != result ? result : cJSON_CreateNull();
	}

	// Transfer a Json conllection to XRpcValue
	/*extern*/ void			JsonCollectionToXRpcValue(__in cJSON * object, XCefValue & rpc_value)
	{
		if (nullptr == object)
			return;

		int icurrent = 0;
		bool is_list_or_dict = true;
		CefRefPtr<CefListValue>			list = CefListValue::Create();
		CefRefPtr<CefDictionaryValue>	dict = CefDictionaryValue::Create();
		CefString		key;
		cJSON_ForEach(it_param, object)
		{
			if (it_param->string)
			{
				key = it_param->string;
				is_list_or_dict = false;
			}
			else
			{
				key = std::to_string(icurrent);
			}

			xcef_cvt::JsonToListItem(it_param, list, icurrent++);
			xcef_cvt::JsonToDictionaryItem(it_param, dict, key);
		}
		if (is_list_or_dict)
			rpc_value = list;
		else
			rpc_value = dict;
	}
	// Transfer a Json to a List index.
	/*extern*/ void			JsonToListItem(__in cJSON * object, __out CefRefPtr<CefListValue> & list, int index)
	{
		if (nullptr == object)
			return;
		if (!list)
			list = CefListValue::Create();

		switch (cJSON_GetSimpleType(object))
		{
		case cJSON_Number:
			if (cJSON_TypeIsInt(object))
				list->SetInt(index, object->valueint);
			else
				list->SetDouble(index, object->valuedouble);
			break;
		case cJSON_True:
		case cJSON_False:
			list->SetBool(index, object->valueint ? true : false);
			break;
		case cJSON_String:
			list->SetString(index, object->valuestring);
			break;
		case cJSON_NULL:
			list->SetNull(index);
			break;
		case cJSON_Array:
		case cJSON_Object:
			{
				XCefValue rpc_value;
				JsonCollectionToXRpcValue(object, rpc_value);
				if (rpc_value.IsList())
				{
					list->SetList(index, rpc_value.GetList());
				}
				else if (rpc_value.IsDictionary())
				{
					list->SetDictionary(index, rpc_value.GetDictionary());
				}
			}
			break;
		default:
			return;
		}
		return;
	}
	// Transfer a Json to a Dict key.
	/*extern*/ void			JsonToDictionaryItem(__in cJSON * object, __out CefRefPtr<CefDictionaryValue> & dict, std::string key)
	{
		if (nullptr == object)
			return;
		if (!dict)
			dict = CefDictionaryValue::Create();

		switch (cJSON_GetSimpleType(object))
		{
		case cJSON_Number:
			if (cJSON_TypeIsInt(object))
				dict->SetInt(key, object->valueint);
			else
				dict->SetDouble(key, object->valuedouble);
			break;
		case cJSON_True:
		case cJSON_False:
			dict->SetBool(key, object->valueint ? true : false);
			break;
		case cJSON_String:
			dict->SetString(key, object->valuestring);
			break;
		case cJSON_NULL:
			dict->SetNull(key);
			break;
		case cJSON_Array:
		case cJSON_Object:
			{
				XCefValue rpc_value;
				JsonCollectionToXRpcValue(object, rpc_value);
				if (rpc_value.IsList())
				{
					dict->SetList(key, rpc_value.GetList());
				}
				else if (rpc_value.IsDictionary())
				{
					dict->SetDictionary(key, rpc_value.GetDictionary());
				}
			}
			break;
		default:
			return;
		}
		return;
	}

	template<class Collect> class Iterator{};
	template<> class Iterator<CefListValue>
	{
	public:
		typedef int				KeyType;

		Iterator<CefListValue>(CefRefPtr<CefListValue> & list){}
		KeyType GetKey(int i){ return i; }
	};
	template<> class Iterator<CefDictionaryValue>
	{
	public:
		typedef std::string		KeyType;

		Iterator<CefDictionaryValue>(CefRefPtr<CefDictionaryValue> & dict){ dict->GetKeys(keys_); }
		KeyType GetKey(int i){ return keys_[i]; }

		CefDictionaryValue::KeyList		keys_;
	};
	template<class CollectPtr>
	static cJSON * CollectToJson(CollectPtr & collect)
	{
		typedef typename Iterator<CollectPtr::element_type>		Iter;

		if (!collect.get())
			return cJSON_CreateNull();
		Iter	it(collect);
		cJSON * result = cJSON_CreateArray();
		cJSON * curr = NULL, *prev = NULL;
		for (int i = 0, nsize = static_cast<int>(collect->GetSize()); i < nsize; ++i)
		{
			Iter::KeyType key = it.GetKey(i);
			switch (collect->GetType(key))
			{
			case VTYPE_NULL:
				curr = cJSON_CreateNull();
				break;
			case VTYPE_BOOL:
				curr = collect->GetBool(key) ? cJSON_CreateTrue() : cJSON_CreateFalse();
				break;
			case VTYPE_INT:
				curr = cJSON_CreateNumber(collect->GetInt(key));
				break;
			case VTYPE_DOUBLE:
				curr = cJSON_CreateNumber(collect->GetDouble(key));
				break;
			case VTYPE_STRING:
				curr = cJSON_CreateString(collect->GetString(key).ToString().c_str());
				break;
			case VTYPE_BINARY:
				LOG(ERROR) << "XRpcValue no implement Binary Type";
				curr = NULL;
				continue;
			case VTYPE_DICTIONARY:
				curr = CollectToJson(collect->GetDictionary(key));
				break;
			case VTYPE_LIST:
				curr = CollectToJson(collect->GetList(key));
				break;
			default:
				curr = NULL;
				assert(0);
				continue;
			}

			if (nullptr == prev)
			{
				result->child = curr;
			}
			else
			{
				prev->next = curr;
			}
			prev = curr;
		}
		if (result && 0 == cJSON_GetArraySize(result))
		{
			cJSON_Delete(result);
			result = cJSON_CreateNull();
		}
		return result;
	}

	// Transfer a XRpcValue to Json
	/*extern*/ cJSON *		XRpcValueToJson(__in XCefValue & rpc_value)
	{
		cJSON * result = NULL;
		CefRefPtr<CefListValue>			list;
		CefRefPtr<CefDictionaryValue>	dict;

		if (rpc_value.IsList())
		{
			list = rpc_value.GetList(); 
			if (list)
			{
				result = CollectToJson(list);
			}
		}
		else if (rpc_value.IsDictionary())
		{
			dict = rpc_value.GetDictionary();
			if (dict)
			{
				result = CollectToJson(dict);
			}
		}

		return nullptr != result ? result : cJSON_CreateNull();
	}
	// Transfer a List to Json
	/*extern*/ cJSON *		ListToJson(__in CefRefPtr<CefListValue> & list)
	{
		return CollectToJson(list);
	}
	// Transfer a Json to a Dict key.
	/*extern*/ cJSON *		DictionaryToJson(__in CefRefPtr<CefDictionaryValue> & dict)
	{
		return CollectToJson(dict);
	}
} /* xcef_cvt */