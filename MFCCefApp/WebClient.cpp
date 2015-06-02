#include "stdafx.h"
#include "WebClient.h"



// Custom menu command Ids.
enum client_menu_ids {
	CLIENT_ID_SHOW_DEVTOOLS = MENU_ID_USER_FIRST,
	CLIENT_ID_CLOSE_DEVTOOLS,
	CLIENT_ID_INSPECT_ELEMENT,
	//
	CLIENT_ID_RELOAD
};
void CWebClient::OnBeforeContextMenu(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model) {
	CEF_REQUIRE_UI_THREAD();

	if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
		// Add a separator if the menu already has items.
		if (model->GetCount() > 0)
			model->AddSeparator();

		// Add DevTools items to all context menus.
		model->AddItem(CLIENT_ID_SHOW_DEVTOOLS, "&Show DevTools");
		model->AddItem(CLIENT_ID_CLOSE_DEVTOOLS, "Close DevTools");
		model->AddSeparator();
		model->AddItem(CLIENT_ID_INSPECT_ELEMENT, "Inspect Element");

		// Test context menu features.
		//BuildTestMenu(model);

		model->AddSeparator();
		model->AddItem(CLIENT_ID_RELOAD, "&Reload");
	}
}

inline void ShowDevTools(CWebClient * pthis, CefRefPtr<CefBrowser> browser,
	const CefPoint& inspect_element_at) {
	CefWindowInfo windowInfo;
	CefBrowserSettings settings;

#if defined(OS_WIN)
	windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

	browser->GetHost()->ShowDevTools(windowInfo, pthis, settings,
		inspect_element_at);
}
inline void CloseDevTools(CefRefPtr<CefBrowser> browser) {
	browser->GetHost()->CloseDevTools();
}
inline void RefreshPage(CefRefPtr<CefBrowser> browser, bool no_cache)
{
	std::stringstream ss;
	ss << "location.reload(" << (no_cache ? "true" : "false") << ");";
	browser->GetMainFrame()->ExecuteJavaScript(ss.str(), "", 0);
}
bool CWebClient::OnContextMenuCommand(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	int command_id,
	EventFlags event_flags) {
	CEF_REQUIRE_UI_THREAD();

	switch (command_id) {
	case CLIENT_ID_SHOW_DEVTOOLS:
		ShowDevTools(this, browser, CefPoint());
		return true;
	case CLIENT_ID_CLOSE_DEVTOOLS:
		CloseDevTools(browser);
		return true;
	case CLIENT_ID_INSPECT_ELEMENT:
		ShowDevTools(this, browser, CefPoint(params->GetXCoord(), params->GetYCoord()));
		return true;
	case CLIENT_ID_RELOAD:
		RefreshPage(browser, true);
		return true;
	default:  // Allow default handling, if any.
		return false;
	}
}


/*virtual*/ void CWebClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) /*OVERRIDE*/
{
	CEF_REQUIRE_UI_THREAD();
	base::AutoLock lock_scope(lock_);
	if (!main_browser_.get())
	{
		
		main_browser_ = browser;
	}
	else
	{
		browsers_list_.push_back(browser);
	}
}

/*virtual*/ void CWebClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) /*OVERRIDE*/
{
	CEF_REQUIRE_UI_THREAD();
	base::AutoLock lock_scope(lock_);
	if (main_browser_.get() && main_browser_->GetIdentifier() == browser->GetIdentifier())
	{
		main_browser_ = NULL;

		for (auto bit = browsers_list_.begin(); bit != browsers_list_.end(); ++bit) {
			if ((*bit)->IsLoading())
				(*bit)->StopLoad();
			(*bit)->GetHost()->CloseBrowser(true);
		}
	}
	else
	{
		browsers_list_.erase(
			std::remove_if(browsers_list_.begin(), browsers_list_.end(), [&](BrowserList::value_type & it){
				return it->GetIdentifier() == browser->GetIdentifier();
			}),
			browsers_list_.end()
			);
	}

// 	if (!main_browser_ && browsers_list_.empty())
// 	{
// 		HWND hwnd = AfxGetMainWnd()->GetSafeHwnd();
// 		PostMessage(hwnd, WM_CLOSE, 0, 0);
// 	}
	return;
}

/*virtual*/ bool CWebClient::DoClose(CefRefPtr<CefBrowser> browser)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////