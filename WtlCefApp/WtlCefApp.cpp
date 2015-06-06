// WtlCefApp.cpp : main source file for WtlCefApp.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "win/WtlMain.h"


#define MIXTURE_UI


//#include "aboutdlg.h"
//#include "MainDlg.h"

//CAppModule _Module;

// int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
// {
// 	CMessageLoop theLoop;
// 	_Module.AddMessageLoop(&theLoop);
// 
// 	CMainDlg dlgMain;
// 
// 	if(dlgMain.Create(NULL) == NULL)
// 	{
// 		ATLTRACE(_T("Main dialog creation failed!\n"));
// 		return 0;
// 	}
// 
// 	dlgMain.ShowWindow(nCmdShow);
// 
// 	int nRet = theLoop.Run();
// 
// 	_Module.RemoveMessageLoop();
// 	return nRet;
// }

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
// 	HRESULT hRes = ::CoInitialize(NULL);
// // If you are running on NT 4.0 or higher you can use the following call instead to 
// // make the EXE free threaded. This means that calls come in on a random RPC thread.
// //	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
// 	ATLASSERT(SUCCEEDED(hRes));
// 
// 	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
// 	::DefWindowProc(NULL, 0, 0, 0L);
// 
// 	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls
// 
// 	hRes = _Module.Init(NULL, hInstance);
// 	ATLASSERT(SUCCEEDED(hRes));
// 
// 	int nRet = Run(lpstrCmdLine, nCmdShow);
// 
// 	_Module.Term();
// 	::CoUninitialize();
// 
// 	return nRet;

	// Cef 不支持 CoInitializeEx Com多线程
	XCefAppManagePtr mng = XCefAppManage::Instance();
	mng->RigisterCallback(new XWinCallback);

	int exit_code = mng->Init(hInstance);
	if (exit_code >= 0)
		return exit_code;

	//////////////////////////////////////////////////////////////////////////
	// 接管消息处理（可选）
	mng->GetCefSettings().multi_threaded_message_loop = true;
	//////////////////////////////////////////////////////////////////////////

#ifndef MIXTURE_UI
	// 独立线程

	// 保存资源句柄
	GSInstance(hInstance);
	// 创建线程
	CGLThread  thr(&GetModule(), CREATE_SUSPENDED);
	thr.Resume();
#else
	// 混合在主线程
	GSInstance(hInstance);
	WtlInit(GSInstance());
	class CCefIdleHandler : public CIdleHandler {
		virtual BOOL OnIdle() {
			CefDoMessageLoopWork();
			return 0;
		}
	}idleCef;
	CGLMessageLoop  glLoop;
	glLoop.AddIdleHandler(&idleCef);
	GetModule().AddMessageLoop(&glLoop);
	CMainFrame wtlMain;
	if (wtlMain.CreateEx() == NULL)
	{
		assert(0);
	}
	wtlMain.ShowWindow(nCmdShow);
#endif

#ifndef MIXTURE_UI
	mng->Loop(hInstance);
	thr.PostQuitMessage();
	mng->Shutdown();
	::WaitForSingleObject(thr.GetHandle(), INFINITE);
#else
	{
 		mng->PreLoop(hInstance);
 		/*result =*/ glLoop.Run();
 		if (wtlMain.IsWindow())
 		{
 			wtlMain.DestroyWindow();
 		}
 
 		_Module.RemoveMessageLoop();
 		glLoop.RemoveIdleHandler(&idleCef);
	}
	mng->Shutdown();
	WtlUnit();
#endif

	return NULL;
}
