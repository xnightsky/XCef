#include <stdafx.h>
#include "XCefInfo.h"



static void _list_to_dict(
	CefRefPtr<CefListValue> &		src,
	CefRefPtr<CefDictionaryValue> & dest,
	CefDictionaryValue::KeyList &	keys
	)
{
	if (!src.get())
		return;
	if (!dest.get())
		dest = CefDictionaryValue::Create();

	bool has_keys = keys.size() == dest->GetSize();
	for (int i = 0, nsize = static_cast<int>(src->GetSize()); i < nsize; ++i)
	{
		std::string key = has_keys ? keys[i] : std::to_string(i);
		switch (src->GetType(i))
		{
		case VTYPE_NULL:
			dest->SetNull(key);
			break;
		case VTYPE_BOOL:
			dest->SetBool(key, src->GetBool(i));
			break;
		case VTYPE_INT:
			dest->SetInt(key, src->GetInt(i));
			break;
		case VTYPE_DOUBLE:
			dest->SetDouble(key, src->GetDouble(i));
			break;
		case VTYPE_STRING:
			dest->SetString(key, src->GetString(i));
			break;
		case VTYPE_BINARY:
			dest->SetBinary(key, src->GetBinary(i));
			break;
		case VTYPE_DICTIONARY:
			dest->SetDictionary(key, src->GetDictionary(i));
			break;
		case VTYPE_LIST:
			dest->SetList(key, src->GetList(i));
			break;
		default:
			assert(0);
			break;
		}
	}
}
static void _list_to_dict(
	CefRefPtr<CefListValue> &		src,
	CefRefPtr<CefDictionaryValue> & dest
	)
{
	CefDictionaryValue::KeyList		keys;
	_list_to_dict(src, dest, keys);
}

// 丢弃key
static void _dict_to_list(CefRefPtr<CefDictionaryValue> & src, CefRefPtr<CefListValue> & dest)
{
	if (!src.get())
		return;
	if (!dest.get())
		dest = CefListValue::Create();

	CefDictionaryValue::KeyList keys;
	src->GetKeys(keys);

	int idx = 0;
	for (CefString & i : keys)
	{
		switch (src->GetType(i))
		{
		case VTYPE_NULL:
			dest->SetNull(idx);
			break;
		case VTYPE_BOOL:
			dest->SetBool(idx, src->GetBool(i));
			break;
		case VTYPE_INT:
			dest->SetInt(idx, src->GetInt(i));
			break;
		case VTYPE_DOUBLE:
			dest->SetDouble(idx, src->GetDouble(i));
			break;
		case VTYPE_STRING:
			dest->SetString(idx, src->GetString(i));
			break;
		case VTYPE_BINARY:
			dest->SetBinary(idx, src->GetBinary(i));
			break;
		case VTYPE_DICTIONARY:
			dest->SetDictionary(idx, src->GetDictionary(i));
			break;
		case VTYPE_LIST:
			dest->SetList(idx, src->GetList(i));
			break;
		default:
			assert(0);
			break;
		}
		++idx;
	}
}



XCefValue::XCefValue()
	: ktype_(VTYPE_INVALID)
{
}

/*virtual*/ bool					XCefValue::IsValid()
{
	return VTYPE_INVALID != ktype_
		&& (!list_.get() || !dict_.get())
		;
}
/*virtual*/ bool					XCefValue::IsList()
{
	return VTYPE_LIST == ktype_;
}
/*virtual*/ bool					XCefValue::IsDictionary()
{
	return VTYPE_DICTIONARY == ktype_;
}
/*virtual*/ CefValueType			XCefValue::GetType()
{
	return ktype_;
}

/*virtual*/ void					XCefValue::Clear()
{
	list_ = NULL, dict_ = NULL, ktype_ = VTYPE_INVALID;
}

// set and get，会强制转换
/*virtual*/ void								XCefValue::SetList(CefRefPtr<CefListValue> & lst)
{
	dict_ = NULL;
	list_ = lst;
	ktype_ = VTYPE_LIST;
}
/*virtual*/ CefRefPtr<CefListValue>			XCefValue::GetList()
{
	if (VTYPE_DICTIONARY == ktype_)
	{
		_dict_to_list(dict_, list_);
	}
	return list_;
}
/*virtual*/ void								XCefValue::SetDictionary(CefRefPtr<CefDictionaryValue> & dict)
{
	list_ = NULL;
	dict_ = dict;
	ktype_ = VTYPE_DICTIONARY;
}
/*virtual*/ CefRefPtr<CefDictionaryValue>		XCefValue::GetDictionary()
{
	if (VTYPE_LIST == ktype_)
	{
		_list_to_dict(list_, dict_);
	}
	return dict_;
}

XCefValue& XCefValue::operator=(CefRefPtr<CefListValue> & lst)
{
	if (reinterpret_cast<XCefValue*>(&lst) == this)
		return *this;
	SetList(lst);
	return *this;
}
XCefValue& XCefValue::operator=(CefRefPtr<CefDictionaryValue> & dict)
{
	if (reinterpret_cast<XCefValue*>(&dict) == this)
		return *this;
	SetDictionary(dict);
	return *this;
}