#include "stdafx.h"
#include "XWinUtil.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <tlhelp32.h>


namespace XWinUtil
{
	/*extern*/ DWORD			GetParentProcessID()
	{
		HANDLE hSnapshot;
		PROCESSENTRY32 pe32;
		DWORD ppid = 0, pid = GetCurrentProcessId();

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		__try{
			if (hSnapshot == INVALID_HANDLE_VALUE) __leave;

			ZeroMemory(&pe32, sizeof(pe32));
			pe32.dwSize = sizeof(pe32);
			if (!Process32First(hSnapshot, &pe32)) __leave;

			do{
				if (pe32.th32ProcessID == pid){
					ppid = pe32.th32ParentProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));

		}
		__finally{
			if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
		}
		return ppid;
	}

	HANDLE GetProcessHandleByPID(DWORD dwProcessId)
	{
		return OpenProcess(/*PROCESS_ALL_ACCESS*/PROCESS_QUERY_INFORMATION, false, dwProcessId);
	}
	/*extern*/ void				ExitWithParentProcess()
	{
		DWORD dwProcessId = GetParentProcessID();
		HANDLE hParentProcess = OpenProcess(SYNCHRONIZE, FALSE, dwProcessId);
		if (dwProcessId == DWORD(-1) || NULL == hParentProcess)
			return;
		CloseHandle(
			(HANDLE)_beginthreadex(
				NULL,
				0,
				[](void* pData) -> unsigned int{
					HANDLE handle = reinterpret_cast<HANDLE>(pData);
					WaitForSingleObject(handle, INFINITE);
					exit(0); // ±¼À£
				},
				hParentProcess,
				0,
				NULL
				)
			);
	}



	/*extern*/	void			SetWin32Less(CefWindowHandle hwnd, BOOL no_border /*= TRUE*/)
	{
		DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
		//	WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		dwStyle &= ~(WS_VSCROLL | WS_HSCROLL | WS_CAPTION);
		if (no_border)
			dwStyle &= ~(WS_THICKFRAME);
		SetWindowLong(hwnd, GWL_STYLE, dwStyle);
		if (no_border)
		{
			DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			dwExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
			SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);
		}
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}

	/*extern*/ void				Maximize(CefWindowHandle hwnd)
	{
		if (!(GetWindowLong(hwnd, GWL_STYLE) & WS_CAPTION) )
		{
 			RECT rt = { 0 };
 			SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rt, 0);
 			SetWindowPos(hwnd, NULL, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, SWP_NOZORDER /*| SWP_NOMOVE | SWP_NOSIZE*/ | SWP_FRAMECHANGED);
		}
		else
		{
			ShowWindow(hwnd, SW_SHOWMAXIMIZED);
		}
	}

	/*extern*/ LPCWSTR			GetMainWindowClassName(int processId /*= _getpid()*/)
	{
		static WCHAR className[MAX_PATH];
		ZeroMemory(className, MAX_PATH);
		wsprintf(className, _T("XCEF%d"), processId);
		return className;
	}

	/*extern*/ CefWindowHandle	GetMainWindowHwnd(int processId /*= _getpid()*/)
	{
		LPCWSTR className = GetMainWindowClassName(processId);
		return FindWindowW(className, NULL);
	}

	/*extern*/ LPCWSTR			GetMessageWindowClassName(int processId /*= _getpid()*/)
	{
		static WCHAR className[MAX_PATH];
		ZeroMemory(className, MAX_PATH);
		wsprintf(className, _T("XCEF_MESSAGE_WINDOW%d"), processId);
		return className;
	}

	/*extern*/ void				PostTitilAreas(CefWindowHandle hwnd, int x /*= -1*/, int y /*= -1*/)
	{
		PostMessage(hwnd, XWM_SET_TITTLE_AREAS, (WPARAM)(x), (LPARAM)(y));
	}
	/*extern*/ void				SetTitleAreas(WPARAM wParam, LPARAM lParam, __out WinInfo & info)
	{
		info.x_ = (int)(wParam);
		info.y_ = (int)(lParam);
	}
}
