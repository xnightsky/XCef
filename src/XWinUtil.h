#pragma once



enum
{
	XWM_SET_TITTLE_AREAS = WM_USER + 1
};

namespace XWinUtil
{
	struct WinInfo
	{
		int			title_x_;
		int			title_y_;

		WinInfo()
			: title_x_(-1), title_y_(-1)
		{}
	};
	static WinInfo & GetWinInfo(){ static WinInfo G_INFO;	return G_INFO; }



	extern DWORD			GetParentProcessID();
	
	// ERROR: ”–±º¿£Œ Ã‚
	extern void				ExitWithParentProcess();

	extern void				SetWin32Less(CefWindowHandle hwnd, BOOL no_border = TRUE);
	extern void				Maximize(CefWindowHandle hwnd);

	extern LPCWSTR			GetMainWindowClassName(int processId = _getpid());
	extern CefWindowHandle	GetMainWindowHwnd(int processId = _getpid());

	extern void				PostTitilAreas(CefWindowHandle hwnd, int x = -1, int y = -1);
	extern void				SetTitleAreas(WPARAM wParam, LPARAM lParam, __out WinInfo & info);
}


