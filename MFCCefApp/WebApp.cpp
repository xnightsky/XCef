#include "stdafx.h"
#include "WebApp.h"



// 加载Exe中的资源
bool LoadBinaryResource(const wchar_t* type, CefString name, LPBYTE& pData, DWORD& dwSize)
{
	HINSTANCE hInst = ::GetModuleHandle(NULL);
	HRSRC hRes = FindResource(hInst, name.ToWString().c_str(), type);
	if (hRes)
	{
		HGLOBAL hGlob = LoadResource(hInst, hRes);
		if (hGlob)
		{
			dwSize = SizeofResource(hInst, hRes);
			pData = (LPBYTE)LockResource(hGlob);
			if (dwSize > 0 && pData != NULL)
			{
				return true;
			}
		}
	}
	return false;
}
bool LoadStringResource(const wchar_t* type, CefString name, CefString& strData)
{
	LPBYTE pData = NULL;
	DWORD dwSize = 0;
	if (LoadBinaryResource(type, name, pData, dwSize))
	{
		std::wstring str((const wchar_t*)pData, dwSize / 2);
		strData.FromWString(str.c_str());
		return true;
	}
	strData = "";
	return false;
}



CWebApp::CWebApp()
{
}


CWebApp::~CWebApp()
{
}

