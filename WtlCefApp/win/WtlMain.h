/*
//this class relized the template of the WTL
// @LI BO,2015.5.3
// @XNightSky
*/
#include "WtlCommon.h"
#include "mainfrm.h"




// 
class CGLThread : public CGuiThreadImpl2<CGLThread, CGLMessageLoop>
{
	BEGIN_MSG_MAP(CGLThread)

	END_MSG_MAP()

	CMainFrame	wtlMain;
public:
	CGLThread(CAppModule* pModule, DWORD dwCreationFlags = 0)
		: CGuiThreadImpl2<CGLThread, CGLMessageLoop>(pModule, dwCreationFlags)
	{ }

	BOOL InitModule()
	{
		WtlInit(GSInstance());
		return TRUE;
	}
	BOOL InitializeThread()
	{
		if (wtlMain.CreateEx() == NULL)
		{
			assert(0);
			return FALSE;
		}
		wtlMain.ShowWindow(SW_SHOW);
		return TRUE;
	}

	void CleanupThread(DWORD)
	{
		if (wtlMain.IsWindow())
		{
			wtlMain.DestroyWindow();
		}

	}

	BOOL UnitModule()
	{
		WtlUnit();
		return TRUE;
	}
};
