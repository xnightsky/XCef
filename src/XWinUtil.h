#pragma once



enum
{
	XWM_SET_TITTLE_AREAS	=	WM_USER + 1,
	XWM_QUIT_APP
};

namespace XWinUtil
{
	struct WinInfo
	{
		int			x_;
		int			y_;

		WinInfo()
			: x_(-1), y_(-1)
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
	extern LPCWSTR			GetMessageWindowClassName(int processId = _getpid());

	extern void				PostTitilAreas(CefWindowHandle hwnd, int x = -1, int y = -1);
	extern void				SetTitleAreas(WPARAM wParam, LPARAM lParam, __out WinInfo & info);
}

struct XWinHookHandle
{
	typedef struct XHookDrap
	{
		DWORD		thread_id_;
		HHOOK		hhook_;
		HWND		hwnd_;
		HWND		hwnd_root_;

		bool		is_drap_;
		RECT		rt_;
		POINT		pt_;
		XWinUtil::WinInfo title_info_;
		RECT		rt_title_;

		XHookDrap()
		{
			thread_id_ = 0;
			hhook_ = NULL;
			hwnd_ = NULL;
			hwnd_root_ = NULL;

			is_drap_ = false;
			rt_ = { 0 };
			pt_ = { 0 };
			rt_title_ = { 0 };
		}

		void RecalcTitleRect()
		{
			POINT pt_title = { title_info_.x_, title_info_.y_ };
			ClientToScreen(hwnd_, &pt_title);
			rt_title_ = { 0, 0, pt_title.x, pt_title.y };
		}
	};
	typedef std::unordered_map<HWND, XHookDrap> Map;
	static inline Map &		GetMap()
	{
		static Map ls_hookdata;
		return ls_hookdata;
	}
	static XHookDrap * GetMapItemByPoint(POINT pt)
	{
		Map & data = GetMap();
		auto it = std::find_if(data.begin(), data.end(), [&](const Map::value_type& vt){ return PtInRect(&vt.second.rt_, pt); });
		return data.end() == it ? NULL : &it->second;
	}
	static XHookDrap * GetMapItem(HWND hwnd)
	{
		Map & data = GetMap();
		auto it = data.find(hwnd);
		return data.end() == it ? NULL : &it->second;
	}

	static LRESULT CALLBACK Proc(int code, WPARAM wp, LPARAM lp)
	{
		if (code >= 0) {
			static XHookDrap * pcurr = NULL;

			PMOUSEHOOKSTRUCT pStruct = (PMOUSEHOOKSTRUCT)lp;
			HWND hwnd_root = GetTop(pStruct->hwnd);
			XHookDrap * pdata = pcurr ? pcurr : /*GetMapItemByPoint(pStruct->pt)*/GetMapItem(hwnd_root);
			if (pdata){
				XHookDrap & data = *pdata;
				switch (wp)
				{
				case WM_LBUTTONDOWN:
					if (!PtInRect(&data.rt_title_, pStruct->pt))
						break;

					data.is_drap_ = true;
					{
						RECT rect = { 0 };
						SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, 0);
						ClipCursor(&rect);
					}
					::GetCursorPos(&data.pt_);
					::GetWindowRect(data.hwnd_root_, &data.rt_);
					pcurr = &data;
					break;
				case WM_LBUTTONUP:
					data.is_drap_ = false;
					ClipCursor(NULL);
					pcurr = NULL;
					data.RecalcTitleRect();
					//::OutputDebugStringA("WM_LBUTTONUP");
					break;
				case WM_MOUSEMOVE:
					if (data.is_drap_)
					{
						RECT rt = data.rt_;
						POINT pt = data.pt_;
						POINT pe = { 0 };
						::GetCursorPos(&pe);
						int x = rt.left + (pe.x - pt.x);
						int y = rt.top + (pe.y - pt.y);
// 						::OutputDebugStringA(xstd::format(
// 							"{ left:%d, top:%d, pe_x:%d, pe_y:%d, pt_x:%d, pt_y:%d, x:%d, y:%d }\r\n",
// 							rt.left, rt.top, pe.x, pe.y, pt.x, pt.y, x, y
// 							).c_str());
						SetWindowPos(data.hwnd_root_, NULL, x, y, 0, 0, SWP_NOREDRAW | SWP_NOSIZE);
					}
				default:
					break;
				}
			}
		}
		return CallNextHookEx(NULL, code, wp, lp);
	}


	static HWND GetTop(HWND hwnd)
	{
		return ::GetAncestor(hwnd, GA_ROOT);
	}

	static BOOL SetHook(HWND hwnd, WPARAM wp, LPARAM lp)
	{
		HWND hwnd_top = GetTop(hwnd);
		XHookDrap & data = XWinHookHandle::GetMap()[hwnd_top];
		XWinUtil::SetTitleAreas(wp, lp, data.title_info_);
		if (NULL != data.hhook_)
		{
			UnHook(hwnd);
		}
		if (data.title_info_.x_ >= 0 && data.title_info_.y_ >= 0)
		{
			data.thread_id_ = GetWindowThreadProcessId(hwnd, NULL);
			data.hhook_ = ::SetWindowsHookEx(WH_MOUSE, (HOOKPROC)XWinHookHandle::Proc, NULL, data.thread_id_);
			data.hwnd_ = hwnd;
			data.hwnd_root_ = hwnd_top;
			::GetWindowRect(hwnd_top, &data.rt_);
			data.RecalcTitleRect();
		}
		return TRUE;
	}
	static BOOL UnHook(HWND hwnd)
	{
		XHookDrap & data = XWinHookHandle::GetMap()[hwnd];

		BOOL bResult = UnhookWindowsHookEx(data.hhook_);//–∂‘ÿº¸≈Ãπ≥◊”°£
		if (bResult)
		{
			data.hhook_ = NULL;
		}
		else
		{
			::OutputDebugString(_T("Unhook Failed!"));
		}
		return bResult;
	}
};
