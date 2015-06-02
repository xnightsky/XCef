#pragma once
//#include "WebRenderProcessHandler.h"


class CWebApp : public CefApp//,
				//public CefBrowserProcessHandler,
				//public CefRenderProcessHandler
{
public:
	CWebApp();
	~CWebApp();

protected:
	IMPLEMENT_REFCOUNTING(CWebApp);
};

