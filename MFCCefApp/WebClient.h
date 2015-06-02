#pragma once



class CWebClient 
	:	public CefClient,
		public CefContextMenuHandler,
		public CefLifeSpanHandler
{
public:
	typedef std::vector<CefRefPtr<CefBrowser> > BrowserList;
public:
	CWebClient(void){};
	virtual ~CWebClient(void){};

	// CefClient methods
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
	{ return this; }

	// CefContextMenuHandler methods
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model) OVERRIDE;
	virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		int command_id,
		EventFlags event_flags) OVERRIDE;

	// CefLifeSpanHandler methods
	// virtual void OnBeforePopup...
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

public:
	CefRefPtr<CefBrowser> GetMainBrowser() { base::AutoLock lock_scope(lock_); return main_browser_; }

private:
	CefRefPtr<CefBrowser>				main_browser_;
	BrowserList							browsers_list_;

	// Ìí¼ÓCEFµÄSPÐéº¯Êý
	IMPLEMENT_REFCOUNTING(CWebClient);
	IMPLEMENT_LOCKING(CWebClient);
};


class TestHandler : public CefMessageRouterBrowserSide::Handler {
public:
	TestHandler() {}

	virtual bool OnQuery(
		CefRefPtr<CefBrowser>	browser,
		CefRefPtr<CefFrame>		frame,
		int64					query_id,
		const CefString&		request,
		bool					persistent,
		CefRefPtr<Callback>		callback
		) OVERRIDE{
		const std::string& message_name = request;
		if (message_name == "myfunc") {
			// Return my string value.
			callback->Success("My Value!");
			return true;
		}

		// Function does not exist.
		return false;
	}

	// Provide the reference counting implementation for this class.
	IMPLEMENT_REFCOUNTING(TestHandler);
};