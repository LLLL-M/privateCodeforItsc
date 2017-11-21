// sadpdlgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sadpdlg.h"
#include "sadpdlgDlg.h"
#include "DlgHikSadp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_DLG_HIKSADP };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSadpdlgDlg dialog

CSadpdlgDlg::CSadpdlgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSadpdlgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSadpdlgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSadpdlgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSadpdlgDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSadpdlgDlg, CDialog)
	//{{AFX_MSG_MAP(CSadpdlgDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_ENTRY, OnButtonEntry)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_DEV_INFO, OnCustomDrawList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSadpdlgDlg message handlers

BOOL CSadpdlgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSadpdlgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CSadpdlgDlg::OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	
    // Take the default processing unless we set this to something else below.
    *pResult = 0;
	
    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.
    if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
        *pResult = CDRF_NOTIFYITEMDRAW;
	}
    else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
        // This is the prepaint stage for an item. Here's where we set the
        // item's text color. Our return value will tell Windows to draw the
        // item itself, but it will use the new color we set here.
        // We'll cycle the colors through red, green, and light blue.	
        int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		int nLevel = 0;

		/*
		nLevel = m_listCurInfo.GetItemData(nItem);

		switch(nLevel) 
		{
		case 1:
			pLVCD->clrTextBk = RGB(255, 192, 203);//错误
			break;
		case 2:
			pLVCD->clrTextBk = RGB(191, 191, 242);//客户端连接or bye
			break;
		case 3:
			pLVCD->clrTextBk = RGB(175, 238, 238);//命令文字
			break;
		case 4:
			pLVCD->clrTextBk = RGB(152, 251, 152);//状态文字代理启动、停止。
			break;
		default:
			pLVCD->clrTextBk = RGB(255, 255, 255);
			break;
		}
		*/

        // Store the color back in the NMLVCUSTOMDRAW struct.
		
        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
	}

}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSadpdlgDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSadpdlgDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// void CSadpdlgDlg::OnButton1() 
// {
// 	CDlgHikSadp dlg;
// 
// 	SSADPRET sret;
// 
// 	dlg.fSetRetBuf(&sret);
// 
// 	dlg.fInsDevice("79.4.0.192");
// 	dlg.fInsDevice("22.1.168.192");
// 	dlg.DoModal();
// 
// 	CDialog::OnOK();
// }

void CSadpdlgDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
    // Judge the default lanuage ID of Windows System
    WORD wLangID=PRIMARYLANGID(::GetSystemDefaultLangID());

    switch(wLangID)
    {
		case LANG_CHINESE:
			break;
        case LANG_ENGLISH:
		default:
			GetDlgItem(IDC_BUTTON_ENTRY)->SetWindowText("Enter");
			GetDlgItem(IDC_STATIC_TIP)->SetWindowText("install WinPcap before use");
			break;
	}	
}

void CSadpdlgDlg::OnButtonEntry() 
{
	// TODO: Add your control notification handler code here
	CDlgHikSadp dlg;
	
	SSADPRET sret;
	
	dlg.fSetRetBuf(&sret);
	
	dlg.fInsDevice("79.4.0.192");
	dlg.fInsDevice("22.1.168.192");
	dlg.DoModal();
	
	CDialog::OnOK();
	
}


