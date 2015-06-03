
// MFCCefDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCCefDemo.h"
#include "MFCCefDemoDlg.h"
#include "afxdialogex.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCCefDemoDlg 对话框



CMFCCefDemoDlg::CMFCCefDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCCefDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCCefDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCCefDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_EXECUTE_JS, &CMFCCefDemoDlg::OnBnClickedBtnExecuteJs)
END_MESSAGE_MAP()


// CMFCCefDemoDlg 消息处理程序
CString		GetAppDir()
{
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos + 1);
	return path;
}
BOOL CMFCCefDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	{
		CWnd * pWnd = this->GetDlgItem(IDC_STATIC);
		if (pWnd)
		{
			// 			pWnd->GetWindowRect(&rt);
			// 			this->ScreenToClient(&rt);
			// 			pWnd->DestroyWindow();
			// 			pWnd->Detach();
			//delete pWnd;

			pWnd->ShowWindow(SW_HIDE);
			pWnd = NULL;
		}

		CWnd* pEditJs = GetDlgItem(IDC_EDIT_JS);
		if (pEditJs)
		{
			pEditJs->SetWindowText(
				//_T("alert('后台调用js')")
				_T("//redirect Baidu \r\n")\
				_T("(function(){ \r\nlocation = 'http://www.baidu.com'; \r\nwindow.open('http://www.douban.com/');\r\n})();")\
				);
		}
	}

	CRect	rc/*(0, 0, 800, 600)*/;
	this->GetClientRect(rc);
	_LayoutRectCefControl(rc);
	_LayoutOtherControl(rc);

	// Cef 嵌入
	m_cefClient = new CWebClient();

	CefWindowInfo info;
	info.SetAsChild(m_hWnd, rc);
	

	std::string strUIPath =
		//"http://www.baidu.com";
		//"http://cn.bing.com"
		GetAppDir() + "skin\\"
			// 测试网上的皮肤
			"bootstrap\\"
				"gebo_admin\\index.htm"
				//"arctic-sunset\\index.html"				
				//"musik-music-html\\index.html"
			// 测试C++和js交互
			//"testapp.html"
		;
	BOOL bRet = CefBrowserHost::CreateBrowser(
		info, 
		static_cast<CefRefPtr<CefClient> >(m_cefClient),
		strUIPath,
		CefBrowserSettings(),
		NULL
		);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCCefDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCCefDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCCefDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCCefDemoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
	CRect rc;
	GetClientRect(rc);
	_LayoutRectCefControl(rc);
	_LayoutOtherControl(rc);
}


void CMFCCefDemoDlg::_LayoutRectCefControl(CRect & rc)
{
	if (m_cefClient.get())
	{
		CefRefPtr<CefBrowser> browser = m_cefClient->GetMainBrowser();
		if (browser)
		{
			CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();

			// ::SetWindowPos(hwnd, HWND_TOP, 0, 0, cx, cy, SWP_NOZORDER);  
			::MoveWindow(hwnd, rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
		}
	}
}
void	CMFCCefDemoDlg::_LayoutOtherControl(CRect & rc)
{
	int nleft = 0;
	CRect rcControl = { 0 };
	CWnd * pEditJs = GetDlgItem(IDC_EDIT_JS);
	CWnd * pBtnJs = GetDlgItem(IDC_BTN_EXECUTE_JS);
	if (pEditJs)
	{
		rcControl.SetRect(0, 0, 300, 100);
		rcControl.MoveToXY(rc.left, rc.bottom - rcControl.Height());
		nleft += rcControl.Width();
		pEditJs->MoveWindow(&rcControl);
	}
	if (pBtnJs)
	{
		rcControl.SetRect(0, 0, 100, 20);
		rcControl.MoveToXY(nleft + 10, rc.bottom - rcControl.Height());
		pBtnJs->MoveWindow(&rcControl);
	}
}

void CMFCCefDemoDlg::OnBnClickedBtnExecuteJs()
{
	// TODO:  在此添加控件通知处理程序代码
	
	// https://code.google.com/p/chromiumembedded/wiki/JavaScriptIntegration
	if (m_cefClient.get())
	{
		CefRefPtr<CefBrowser> browser = m_cefClient->GetMainBrowser();
		if (browser)
		{
			CefRefPtr<CefFrame> frame = browser->GetMainFrame();
			if (frame)
			{
				std::wstring	wstrJS;
				CString		strJS;

				
				GetDlgItemText(IDC_EDIT_JS, strJS);
				// document.querySelector("#sb_form_q").value="C++";
				wstrJS = strJS;

				CefString cefstrJs;
				cefstrJs.FromWString(wstrJS);
				// wchar 没有乱码
				frame->ExecuteJavaScript(
					cefstrJs,
					frame->GetURL(),  
					0);
			}
		}
	}
}

