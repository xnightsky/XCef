// CefWin64.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "CefWin64.h"



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	XCefAppManagePtr mng = XCefAppManage::Instance();
	{
		// Manage配置写在这里，每个进程都会加载
		//XManifestUtil::Instance().EnableOffScreenRendering();

		// 注册事件
		mng->RigisterCallback(new XWinCallback);
		{
			std::string strPath = XCefAppManage::GetModuleCurrentDirectory().ToString() + "..\\..\\resources\\JSHandle.html";
			XManifestUtil::Instance().LoadUrl(strPath.c_str());
		}
// 		XManifestUtil::Instance().LoadHtml(
//  			CefString("<script type='text/javascript'>alert('111');</script>")
//  			);
		XManifestUtil::Instance().SetWin32less();
		//XManifestUtil::Instance().Maximize();
	}
	int exit_code = mng->RunSingle(hInstance);
	if (exit_code >= 0)
		return exit_code;
	return NULL;
}
