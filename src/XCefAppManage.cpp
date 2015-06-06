#include "stdafx.h"
#include "XCefAppManage.h"
#include "XWinUtil.h"

#include "client_switches.h"
#include "cefclient_osr_widget_win.h"

#ifndef _DWMAPI_H_
#include <dwmapi.h>
#pragma comment(lib, "dwmapi")
#endif

// 全局变量: 
HINSTANCE	s_hInst = NULL;
int			s_nCmdShow = 0;



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CreateHostWindow()
{
	HWND hWnd = NULL;
	LPCWSTR szWindowClass = XWinUtil::GetMainWindowClassName();

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = s_hInst;
	wcex.hIcon = LoadIcon(s_hInst, MAKEINTRESOURCE(IDI_CEFCLIENT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TIERTIME);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	if (!RegisterClassEx(&wcex))
		return FALSE;

	// 防止重入WM_CREATE
	XCefAppManage::Instance()->SetClientHostHandle((HWND)-1);
	hWnd = CreateWindow(
		szWindowClass,
		_T("YE!HOST"), 
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
		NULL, NULL, wcex.hInstance, NULL
		);
	if (!hWnd)
	{
		return FALSE;
	}
	XCefAppManage::Instance()->SetClientHostHandle(hWnd);

	ShowWindow(hWnd, s_nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}


class MainBrowserProvider : public OSRBrowserProvider {
	virtual CefRefPtr<CefBrowser> GetBrowser() {
		CefRefPtr<ClientHandler> handler = XCefAppManage::Instance()->GetClient();
		if (handler)
			return handler->GetBrowser();

		return NULL;
	}
} g_main_browser_provider;

/*static*/ char						XCefAppManage::working_dir_[MAX_PATH] = { 0 };

XCefAppManage::XCefAppManage()
	: message_wnd__(NULL), 
	cache_browser_hwnd_(NULL), 
	client_host_hwnd_(NULL)
{
}
XCefAppManage::~XCefAppManage(){
	//Shutdown();
}


/*static*/  CefRefPtr<XCefAppManage>	XCefAppManage::Instance()
{
	static std::once_flag oc;
	static CefRefPtr<XCefAppManage> G_HANDLE;

	std::call_once(oc, [&]{
		G_HANDLE = new XCefAppManage;
		});
	return G_HANDLE;
}


int XCefAppManage::RunSingle(HINSTANCE hInstance)
{
	int exit_code = Init(hInstance);
	if (exit_code >= 0)
		return exit_code;

	Loop(hInstance);

	Shutdown();
	return 0;
}

int XCefAppManage::Init(HINSTANCE hInstance, int nCmdShow /*= SW_SHOW*/)
{
	s_hInst = hInstance;
	s_nCmdShow = nCmdShow;
	XManifestUtil & cfg = XManifestUtil::Instance();

	void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
	// Manage the life span of the sandbox information object. This is necessary
	// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
#endif

	CefMainArgs main_args(hInstance);
	app_ = new ClientApp;

	// Execute the secondary process, if any.
	int exit_code = CefExecuteProcess(main_args, app_.get(), sandbox_info);
	if (exit_code >= 0)
		return exit_code;

	//// Retrieve the current working directory.
	//	if (_getcwd(szWorkingDir, MAX_PATH) == NULL)
	//		szWorkingDir[0] = 0;

	// Parse command line arguments. The passed in values are ignored on Windows.
	InitCommandLine(0, NULL);

	//CefSettings settings;
#if !defined(CEF_USE_SANDBOX)
	settings_.no_sandbox = true;
#endif

	// Populate the settings based on command line arguments.
	InitCefSettings(settings_);

	if (cfg.window.has_host_hwnd_)
		XCefAppManage::Instance()->SetClientHostHandle(reinterpret_cast<HWND>(-1));

	// Initialize CEF.
	CefInitialize(main_args, settings_, app_.get(), sandbox_info);
	
	return exit_code;
}



LRESULT CALLBACK MessageWndProc(HWND hWnd, UINT message, WPARAM wParam,
	LPARAM lParam) {
	switch (message) {
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		switch (wmId) {
		case XWM_QUIT_APP:
			XCefAppManage::Instance()->GetClient()->CloseAllBrowsers(true);
			break;
		case ID_QUIT:
			PostQuitMessage(0);
			return 0;
		}
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
HWND CreateMessageWindow(HINSTANCE hInstance) {
	//static const wchar_t kWndClass[] = L"ClientMessageWindow";
	const wchar_t * kWndClass = XWinUtil::GetMessageWindowClassName();

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = MessageWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = kWndClass;
	RegisterClassEx(&wc);

	//return CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
	return CreateWindowEx(0, kWndClass, L"XCEF_MESSAGE_WINDOW", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
}
static BOOL IsIdleMessage(MSG* pMsg)
{
	// These messages should NOT cause idle processing
	switch (pMsg->message)
	{
	case WM_MOUSEMOVE:
#ifndef _WIN32_WCE
	case WM_NCMOUSEMOVE:
#endif // !_WIN32_WCE
	case WM_PAINT:
	case 0x0118:	// WM_SYSTIMER (caret blink)
		return FALSE;
	}

	return TRUE;
}
BOOL	XCefAppManage::Loop(HINSTANCE hInstance)
{
	if (!settings_.multi_threaded_message_loop) {
		// Run the CEF message loop. This function will block until the application
		// recieves a WM_QUIT message.
		CefRunMessageLoop();
	}
	else {
		// Create a hidden window for message processing.
		message_wnd__ = CreateMessageWindow(hInstance);
		DCHECK(message_wnd__);

		MSG		msg = {0};

		//HACCEL hAccelTable = NULL;		// !!!!!!!!!!!!!

// 		while (GetMessage(&msg, NULL, 0, 0))
// 		{
// 			if (!PreTranslateMessage(&msg))
// 			{
// 				//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
// 				TranslateMessage(&msg);
// 				DispatchMessage(&msg);
// 			}
// 		}

		while (true)
		{
			if (IsIdleMessage(&msg) && !::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				CefDoMessageLoopWork();
				continue;
			}

			if (GetMessage(&msg, NULL, 0, 0))
			{
				if (!PreTranslateMessage(&msg))
				{
					//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else
			{
				break;
			}
		}


		DestroyWindow(message_wnd__);
		message_wnd__ = NULL;

		//result = static_cast<int>(msg.wParam);
	}

	return TRUE;
}

void XCefAppManage::Shutdown()
{
	CefShutdown();
	app_ = NULL;
	ClearCallback();
}

void    XCefAppManage::PreLoop(HINSTANCE hInstance)
{
	if (settings_.multi_threaded_message_loop)
	{
		// Create a hidden window for message processing.
		message_wnd__ = CreateMessageWindow(hInstance);
		DCHECK(message_wnd__);
	}
}

BOOL	XCefAppManage::PreTranslateMessage(MSG* pMsg)
{
	return FALSE;
}



#ifndef _INC_SHLWAPI
#	include <Shlwapi.h>
#	pragma comment(lib, "Shlwapi.lib")
#endif
/*static*/ CefString XCefAppManage::GetModuleCurrentDirectory()
{
	TCHAR szPath[MAX_PATH];
	::GetModuleFileNameW(NULL, szPath, MAX_PATH);
	::PathRemoveFileSpecW(szPath);
	std::wstring strRet = szPath;
	strRet += L"\\";
	return strRet;
}

#include <direct.h>
/*static*/ CefString XCefAppManage::GetWorkingDirectory()
{
	static std::once_flag oc;

	std::call_once(oc, [&] { 
		// Retrieve the current working directory.
		if (_getcwd(working_dir_, MAX_PATH) == NULL)
			working_dir_[0] = 0;
	});


	return working_dir_;
}

void XCefAppManage::InitCommandLine(int argc, const char* const* argv)
{
	command_line_ = CefCommandLine::CreateCommandLine();
#if defined(OS_WIN)
	command_line_->InitFromString(::GetCommandLineW());
#else
	command_line_->InitFromArgv(argc, argv);
#endif
}

void XCefAppManage::InitCefSettings(__out CefSettings& settings) {
	DCHECK(command_line_.get());
	if (!command_line_.get())
		return;

	CefString str;

#if defined(OS_WIN)
	settings.multi_threaded_message_loop = command_line_->HasSwitch(cefclient::kMultiThreadedMessageLoop);

	// [WARNING]:Cannot use V8 Proxy resolver in single process mode.
	//settings.single_process = true;
#endif

	// --cache-path
	CefString(&settings.cache_path) = command_line_->GetSwitchValue(cefclient::kCachePath);

	// --off-screen-rendering-enabled
	if (command_line_->HasSwitch(cefclient::kOffScreenRenderingEnabled) 
		|| XManifestUtil::Instance().browser.is_off_screen_rendering_
		)
		settings.windowless_rendering_enabled = true;

	//// 解决——[0418/193102:FATAL:resource_bundle.cc(507)] Check failed: false. unable to find resource: 28023
	// pak不兼容，已经解决


	//CefString(&settings.locales_dir_path) = GetModuleCurrentPath(L"locales\\");
	//CefString(&settings.resources_dir_path) = GetModuleCurrentPath(L"");
}

void							XCefAppManage::InitBrowserSettings(__out CefBrowserSettings & settings)
{
	// --allow-file-access-from-files
	//if (command_line_->HasSwitch("allow-file-access-from-files"))
		settings.file_access_from_file_urls = STATE_ENABLED;
}

void							XCefAppManage::CreateBrowser(HWND hwnd_parent)
{
	CefSettings & cef_setting = GetCefSettings();
	// 如果接管消息处理，必须外部处理父句柄
	DCHECK(!cef_setting.multi_threaded_message_loop || (cef_setting.multi_threaded_message_loop && hwnd_parent));


	CefRefPtr<ClientHandler> client = new ClientHandler;
	SetClient(client);


	// 如果宿主窗口不存在即创建
	if (NULL == hwnd_parent)
	{
		if (!CreateHostWindow())
		{
			DCHECK(0);
		}
		hwnd_parent = client_host_hwnd_;
	}
	CefWindowInfo window_info;
	RECT rt = { 0 };
	GetClientRect(hwnd_parent, &rt);
	
	
	if (IsOffScreenRenderingEnabled()) {
		CefRefPtr<CefCommandLine> cmd_line = GetCefCommandLine();
		const bool transparent =
			//cmd_line->HasSwitch(cefclient::kTransparentPaintingEnabled)
			true
			;
		const bool show_update_rect =
			//cmd_line->HasSwitch(cefclient::kShowUpdateRect)
			true
			;

		CefRefPtr<OSRWindow> osr_window =
			OSRWindow::Create(&g_main_browser_provider, transparent, show_update_rect);
		osr_window->CreateWidget(hwnd_parent, rt, s_hInst, _T("OSRWindow"));
		// implement CefRenderHandler
		window_info.SetAsWindowless(osr_window->hwnd(), transparent);
		client->SetOSRHandler(osr_window.get());
	}
	else {
		if (hwnd_parent)
		{
			// Initialize window info to the defaults for a child window.
			window_info.SetAsChild(hwnd_parent, rt);
		}
		else
		{
			// On Windows we need to specify certain flags that will be passed to CreateWindowEx().
			window_info.SetAsPopup(hwnd_parent, "OH!YE!");
		}
	}
	
	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;
	InitBrowserSettings(browser_settings);

	std::string url;
	// Check if a "--url=" value was provided via the command-line. If so, use
	// that instead of the default URL.
	CefRefPtr<CefCommandLine> command_line = GetCefCommandLine();
	url = command_line->GetSwitchValue("url");
	if (url.empty())
	{
		//url = "about:blank";
		//url = "http://www.baidu.com";
		url = XCefAppManage::GetModuleCurrentDirectory().ToString() + "..\\..\\resources\\JSHandle.html";
	}

	XManifestUtil &	cfg = XManifestUtil::Instance();
	bool				is_sync = false;
	bool				is_win32less = false;
	bool				is_maximize = false;
	CefString			str_html;
	if (!cfg.browser.is_url_or_html_)
	{
		is_sync = true;
		url = cfg.browser.str_url_;
		str_html = cfg.browser.str_html_;
	}
	else if (!cfg.browser.str_url_.empty())
	{
		url = cfg.browser.str_url_;
	}
 	if (cfg.window.win32less_)
 	{
 		is_sync = true;
 		is_win32less = true;
 	}
//  	switch (cfg.window.show_state_)
//  	{
//  	case SW_MAX:
//  		is_maximize = true;
//  		break;
//  	default:
// 		break;
//  	}



	if (!is_sync)
	{
		// Create the first browser window.
		CefBrowserHost::CreateBrowser(
			window_info,
			client.get(),
			url,
			browser_settings,
			NULL
			);
	}
	else
	{

		CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
			window_info,
			client.get(),
			url,
			browser_settings,
			NULL
			);
		HWND hwnd_browser = browser->GetHost()->GetWindowHandle();
		//HWND hwnd_host = cef_setting.multi_threaded_message_loop ? hwnd_parent : hwnd_browser;

		if (!str_html.empty())
		{
			browser->GetMainFrame()->LoadString(str_html, url);
		}
		if (is_win32less)
		{
			XWinUtil::SetWin32Less(/*hwnd_host*/hwnd_parent);
		}
 		if (is_maximize)
 		{
			XWinUtil::Maximize(/*hwnd_host*/hwnd_parent);
 		}
	}
}


void						XCefAppManage::QuitMessageLoop() {
	if (GetCefSettings().multi_threaded_message_loop)
	{
		// Running in multi-threaded message loop mode. Need to execute
		// PostQuitMessage on the main application thread.
		if (NULL == message_wnd__)
		{
			message_wnd__ = ::FindWindow(XWinUtil::GetMessageWindowClassName(XWinUtil::GetParentProcessID()), NULL);
		}
		DCHECK(message_wnd__);
		PostMessage(message_wnd__, WM_COMMAND, ID_QUIT, 0);
	}
	else {
		CefQuitMessageLoop();
	}
}
void							XCefAppManage::QuitMessageLoopByChildProcess()
{
	DWORD ppid = XWinUtil::GetParentProcessID();
	CefWindowHandle hmsg = ::FindWindow(XWinUtil::GetMessageWindowClassName(ppid), NULL);
	if (NULL != hmsg)
	{
		PostMessage(hmsg, WM_COMMAND, XWM_QUIT_APP, 0);
		return;
	}
	if (NULL == hmsg)
	{
		hmsg = ::FindWindow(XWinUtil::GetMainWindowClassName(ppid), NULL);
	}
	DCHECK(hmsg);
	PostMessage(hmsg, XWM_QUIT_APP, 0, 0);
}

bool							XCefAppManage::IsOffScreenRenderingEnabled()
{
	return XManifestUtil::Instance().browser.is_off_screen_rendering_;
}



void							XCefAppManage::RigisterCallback(XCefCallback * cb)
{
	cb->Init();
	cb_vector_.push_back(cb);
}
void							XCefAppManage::ClearCallback()
{
	for (auto pit : cb_vector_)
	{
		delete pit;
	}
	cb_vector_.clear();
}
void							XCefAppManage::CallbackLoad(CefRefPtr<CefV8Value> window, CefRefPtr<CefV8Context> context)
{
	DCHECK(window);
	CefRefPtr<CefV8Value> app;
	if (!window->HasValue("app"))
	{
		app = XCefV8Handler::CreateObject(window, "app");
		app->SetValue("name", CefV8Value::CreateString("{app}"), V8_PROPERTY_ATTRIBUTE_NONE);
	}
	else
	{
		app = window->GetValue("app");
	}

	for (auto pit : cb_vector_)
	{
		pit->CBLoad(app, context);
	}
}
void							XCefAppManage::AnsycCallbackLoad(MessageHandlerSet & msg_handle_set)
{
	for (auto pit : cb_vector_)
	{
		pit->ACBLoad(msg_handle_set);
	}
}

// WNDPROC							XCefAppManage::SetClientHostProc(WNDPROC proc, HWND hwnd /*= NULL*/){
// 	if (proc == client_host_old_proc_)
// 		return NULL;
// 	
// 	WNDPROC ret_proc = client_host_old_proc_;
// 	if (NULL == hwnd)
// 	{
// 		client_host_old_proc_ = proc;
// 	}
// 	else
// 	{
// 		client_host_old_proc_ = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
// 	}
// 	return ret_proc;
// }


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	XCefAppManagePtr				mng = XCefAppManage::Instance();
	CefRefPtr<ClientHandler>		client = mng->GetClient();


	PAINTSTRUCT ps;
	HDC			hdc;

	do
	{
		// Callback for the main window
		switch (message) {
		case WM_CREATE:
			// 宿主窗口才会有 WM_CREATE
			if (NULL == mng->GetClientHostHandle())
			{
				mng->CreateBrowser(hWnd);
				mng->SetClientHostHandle(hWnd);
			}
			return 0;
		case XWM_QUIT_APP:
			if (client){
				client->CloseAllBrowsers(true);
			}
			break;
		case WM_CLOSE:
			if (client && !client->IsClosing()) {
				CefRefPtr<CefBrowser> browser = client->GetBrowser();
				if (browser.get()) {
					// Notify the browser window that we would like to close it. This
					// will result in a call to ClientHandler::DoClose() if the
					// JavaScript 'onbeforeunload' event handler allows it.
					browser->GetHost()->CloseBrowser(false);

					// Cancel the close.
					return 0;
				}
			}

			// Allow the close.
			break;

		case WM_DESTROY:
			// Quitting CEF is handled in ClientHandler::OnBeforeClose().
			XWinHookHandle::UnHook(hWnd);
			return 0;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;
		case WM_SETFOCUS:
			if (client && client->GetBrowser()) {
				// Pass focus to the browser window
				CefWindowHandle hwnd = client->GetBrowser()->GetHost()->GetWindowHandle();
				if (hwnd)
					PostMessage(hwnd, WM_SETFOCUS, wParam, NULL);
			}
			return 0;

		case WM_SIZE: {
			if (!client)
				break;

			if (client->GetBrowser()) {
				// Retrieve the window handle (parent window with off-screen rendering).
				CefWindowHandle hwnd = client->GetBrowser()->GetHost()->GetWindowHandle();
				if (hwnd) {
					if (wParam == SIZE_MINIMIZED) {
						SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
					}
					else {
						// Resize the window and address bar to match the new frame size.
						RECT rect;
						GetClientRect(hWnd, &rect);
						SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
					}
				}
			}
		} break;

		case WM_MOVING:
		case WM_MOVE:
			// Notify the browser of move events so that popup windows are displayed
			// in the correct location and dismissed when the window moves.
			if (client && client->GetBrowser())
				client->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
			return 0;

		case WM_ERASEBKGND:
			if (client.get() && client->GetBrowser()) {
				CefWindowHandle hwnd = client->GetBrowser()->GetHost()->GetWindowHandle();
				if (hwnd) {
					// Dont erase the background if the browser window has been loaded
					// (this avoids flashing)
					//return 0;
					break;
				}
			}
			break;

		case WM_ENTERMENULOOP:
			if (!wParam) {
				// Entering the menu loop for the application menu.
				CefSetOSModalLoop(true);
			}
			break;

		case WM_EXITMENULOOP:
			if (!wParam) {
				// Exiting the menu loop for the application menu.
				CefSetOSModalLoop(false);
			}
			break;
		case XWM_SET_TITTLE_AREAS:
			//SetTitleAreas(wParam, lParam, XWinUtil::GetWinInfo());
			if(client->GetBrowser()) {
				// Retrieve the window handle (parent window with off-screen rendering).
				CefWindowHandle hwnd = client->GetBrowser()->GetHost()->GetWindowHandle();
				XWinHookHandle::SetHook(hwnd, wParam, lParam);
			}
			return 0;
		}
	} while (0);

	return DefWindowProc(hWnd, message, wParam, lParam);
}