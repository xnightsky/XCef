#include "stdafx.h"
#include "WtlMain.h"

#include <assert.h>

/*extern*/ CAppModule _Module;
/*extern*/ CGLMessageLoop  _glLoop;


/*extern*/ void WtlInit(HINSTANCE hInst)
{
	assert(hInst);
	GSInstance(hInst);

	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

#if (_WIN32_IE >= 0x0300)
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_BAR_CLASSES;	// change to support other controls
	BOOL bRet = ::InitCommonControlsEx(&iccx);
	bRet;
	ATLASSERT(bRet);
#else
	::InitCommonControls();
#endif

	hRes = _Module.Init(NULL, hInst);
	ATLASSERT(SUCCEEDED(hRes));
}

/*extern*/ void WtlUnit()
{
	_Module.Term();
	::CoUninitialize();
}

/*extern*/ CAppModule &	GetModule()
{
	return _Module;
}

/*extern*/ HINSTANCE	GSInstance(HINSTANCE saveInst /*= NULL*/)
{
	static HINSTANCE GINST = NULL;
	if (saveInst)
		GINST = saveInst;
	return GINST;
}