// videodetectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "videodetect.h"
#include "videodetectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

STRU_DETECTOR_DATA		gData;
STRU_DETECTOR_STATUS	gStatus;

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CvideodetectDlg �Ի���




CvideodetectDlg::CvideodetectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CvideodetectDlg::IDD, pParent)
	, m_strEquid(_T(""))
	, m_strVehileCount(_T(""))
	, m_bDetectType(FALSE)
	, m_strIniFileName(_T(".\\TSCIP.ini"))
	, m_dwTSCip(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_nSocket = 0;
	m_bStopSendThread = FALSE;

	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested=MAKEWORD(2,2);

	if(WSAStartup(wVersionRequested,&wsaData) !=0)
	{
		return;
	}

	if (LOBYTE( wsaData.wVersion) != 2 ||
		HIBYTE( wsaData.wVersion) != 2)
	{
		WSACleanup();
	}
}

void CvideodetectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_EQUID, m_strEquid);
	DDX_Text(pDX, IDC_EDIT_VEHCOUNT, m_strVehileCount);
	DDX_Radio(pDX, IDC_RADIO1, m_bDetectType);
	//DDX_IPAddress(pDX, IDC_IPADDRESS1, m_dwTSCip);
	DDX_Control(pDX, IDC_IPADDRTSC, m_ctlTSCip);
	DDX_Control(pDX, IDC_EDIT1, m_data_total_flow);
	DDX_Control(pDX, IDC_EDIT2, m_data_large_flow);
	DDX_Control(pDX, IDC_EDIT3, m_data_small_flow);
	DDX_Control(pDX, IDC_EDIT4, m_data_percent);
	DDX_Control(pDX, IDC_EDIT5, m_data_speed);
	DDX_Control(pDX, IDC_EDIT6, m_data_vechile_length);
	DDX_Control(pDX, IDC_CHECK1, m_detector_unlive);
	DDX_Control(pDX, IDC_CHECK3, m_detector_unstable);
	DDX_Control(pDX, IDC_CHECK2, m_detector_livelong);
	DDX_Control(pDX, IDC_CHECK4, m_detector_communication_err);
	DDX_Control(pDX, IDC_CHECK5, m_detector_configure_err);
	DDX_Control(pDX, IDC_CHECK6, m_detector_unknow_err);
	DDX_Control(pDX, IDC_CHECK12, m_detector_noraml);
	DDX_Control(pDX, IDC_CHECK7, m_induction_unkown_err);
	DDX_Control(pDX, IDC_CHECK8, m_induction_watchdog_err);
	DDX_Control(pDX, IDC_CHECK9, m_induction_open);
	DDX_Control(pDX, IDC_CHECK10, m_induction_low);
	DDX_Control(pDX, IDC_CHECK11, m_induction_high);
	DDX_Control(pDX, IDC_CHECK13, m_induction_normal);
}

BEGIN_MESSAGE_MAP(CvideodetectDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CvideodetectDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_SEND, &CvideodetectDlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_STOP, &CvideodetectDlg::OnBnClickedBtnStop)
	ON_EN_KILLFOCUS(IDC_EDIT1, &CvideodetectDlg::OnEnKillfocusTotalFlow)
	ON_EN_KILLFOCUS(IDC_EDIT2, &CvideodetectDlg::OnEnKillfocusLargeVechileFlow)
	ON_EN_KILLFOCUS(IDC_EDIT3, &CvideodetectDlg::OnEnKillfocusSmallVechileFlow)
	ON_EN_KILLFOCUS(IDC_EDIT4, &CvideodetectDlg::OnEnKillfocusPercent)
	ON_EN_KILLFOCUS(IDC_EDIT5, &CvideodetectDlg::OnEnKillfocusSpeed)
	ON_EN_KILLFOCUS(IDC_EDIT6, &CvideodetectDlg::OnEnKillfocusVechileLength)
	ON_BN_CLICKED(IDC_CHECK1, &CvideodetectDlg::OnBnClickedDetectorNolive)
	ON_BN_CLICKED(IDC_CHECK2, &CvideodetectDlg::OnBnClickedDetectorLiveLong)
	ON_BN_CLICKED(IDC_CHECK3, &CvideodetectDlg::OnBnClickedDetectorUnstable)
	ON_BN_CLICKED(IDC_CHECK4, &CvideodetectDlg::OnBnClickedDetectorCommunicationErr)
	ON_BN_CLICKED(IDC_CHECK5, &CvideodetectDlg::OnBnClickedConfigureErr)
	ON_BN_CLICKED(IDC_CHECK6, &CvideodetectDlg::OnBnClickedDetectorUnkonwErr)
	ON_BN_CLICKED(IDC_CHECK12, &CvideodetectDlg::OnBnClickedDetectorNormal)
	ON_BN_CLICKED(IDC_CHECK7, &CvideodetectDlg::OnBnClickedInductionUnknow)
	ON_BN_CLICKED(IDC_CHECK8, &CvideodetectDlg::OnBnClickedInductionWatchdogErr)
	ON_BN_CLICKED(IDC_CHECK9, &CvideodetectDlg::OnBnClickedInductionOpen)
	ON_BN_CLICKED(IDC_CHECK10, &CvideodetectDlg::OnBnClickedInductionLow)
	ON_BN_CLICKED(IDC_CHECK11, &CvideodetectDlg::OnBnClickedInductionHiigh)
	ON_BN_CLICKED(IDC_CHECK13, &CvideodetectDlg::OnBnClickedInductionNormal)
	ON_EN_KILLFOCUS(IDC_EDIT_EQUID, &CvideodetectDlg::OnEnKillfocusEditEquid)
END_MESSAGE_MAP()


// CvideodetectDlg ��Ϣ�������

BOOL CvideodetectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	GetTSCip();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CvideodetectDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CvideodetectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CvideodetectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CvideodetectDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//OnOK();
}

BOOL CvideodetectDlg::GetTSCip()
{
	//�������ļ���ȡIP
	//1.
	CString strBuf;
	::GetPrivateProfileString(_T("TSCIP"), _T("IP"), _T("172.7.18.62"), \
		strBuf.GetBuffer(128), 128, m_strIniFileName);
	m_dwTSCip = inet_addr((LPSTR)(LPCTSTR)strBuf);
	m_ctlTSCip.SetAddress(ntohl(m_dwTSCip));
	m_ctlTSCip.SetWindowText(strBuf);
	//AfxMessageBox(strBuf);
	strBuf.ReleaseBuffer();
	UpdateData(FALSE);
	return TRUE;
}

BOOL CvideodetectDlg::SetTSCip()
{
	CString strip;
	BYTE byteIP[4] = "0";
	m_ctlTSCip.GetAddress(byteIP[0], byteIP[1], byteIP[2], byteIP[3]);
	strip.Format(_T("%u.%u.%u.%u"), byteIP[0], byteIP[1], byteIP[2], byteIP[3]);
	//AfxMessageBox(strip);
	if(!::WritePrivateProfileString(_T("TSCIP"), _T("IP"), strip, m_strIniFileName))
	{
		return FALSE;
	}
	return TRUE;
}

void CvideodetectDlg::OnBnClickedBtnSend()
{
	UpdateData();
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_strEquid.IsEmpty())
	{
		AfxMessageBox(_T("�豸��Ų���ȷ!"));
		return;
	}
	
	if (m_ctlTSCip.IsBlank())
	{
		AfxMessageBox(_T("�źŻ�IP����ȷ!"));
		return;
	}
	SetTSCip();
	GetDlgItem(IDC_EDIT_EQUID)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_VEHCOUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPADDRTSC)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow();
	if (m_bDetectType)
	{
		GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
	}
	m_bStopSendThread = FALSE;
	
	//���������߳�
	AfxBeginThread(CvideodetectDlg::fnSendThread, this);
}

void CvideodetectDlg::OnBnClickedBtnStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bStopSendThread = TRUE;
	Sleep(2000);
}

BOOL CvideodetectDlg::ConnectTSC()
{
	int iResult = -1;
	struct sockaddr_in clientService; 
	m_nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_nSocket == INVALID_SOCKET)
	{
		CString strErr;
		strErr.Format(_T("err:%d"), GetLastError());
		//AfxMessageBox(strErr);
		return FALSE;
	}
	m_ctlTSCip.GetAddress(m_dwTSCip);
	// IP address, and port of the server to be connected to.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = htonl(m_dwTSCip);//inet_addr("172.7.18.61");
	clientService.sin_port = htons(7200);
	// Connect to server.
	iResult = connect(m_nSocket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR)
	{
		CString strErr;
		strErr.Format(_T("�����źŻ�ʧ��! err:%d"), GetLastError());
		AfxMessageBox(strErr);
		CloseSocketDef();
		return FALSE;
	}
	return TRUE;
}

void CvideodetectDlg::CloseSocketDef()
{
	shutdown(m_nSocket, SD_BOTH);
	closesocket(m_nSocket);
}

UINT CvideodetectDlg::fnSendThread(PVOID p)
{
	CvideodetectDlg *pMain = (CvideodetectDlg*)p;
	INTER_DVR_REQUEST_HEAD_V30 m_stHead;
	memset(&m_stHead, 0, sizeof(INTER_DVR_REQUEST_HEAD_V30));
	NET_TPS_ALARM stTpsAlarm;
	memset(&stTpsAlarm, 0, sizeof(NET_TPS_ALARM));
	NET_TPS_PED_ALARM stPedAlarm;
	memset(&stPedAlarm, 0, sizeof(NET_TPS_PED_ALARM));

	stTpsAlarm.dwDeviceId = htonl(atoi((LPSTR)(LPCTSTR)pMain->m_strEquid));
	
	if (pMain->m_bDetectType)
	{
		m_stHead.byCommand = 0x3b; //����
		m_stHead.dwLength = sizeof(INTER_DVR_REQUEST_HEAD_V30) + sizeof(NET_TPS_PED_ALARM);
		stPedAlarm.dwDeviceId = atoi((LPSTR)(LPCTSTR)pMain->m_strEquid);
	}
	else
	{
		m_stHead.byCommand = 0x3a; //������
		m_stHead.dwLength = sizeof(INTER_DVR_REQUEST_HEAD_V30) + sizeof(NET_TPS_ALARM);
	}
	
	char buf[4000] = {0};
	UINT count = 0;
	while (1)
	{
		if (pMain->m_bStopSendThread)
		{
			pMain->GetDlgItem(IDC_EDIT_EQUID)->EnableWindow();
			pMain->GetDlgItem(IDC_EDIT_VEHCOUNT)->EnableWindow();
			pMain->GetDlgItem(IDC_BTN_SEND)->EnableWindow();
			pMain->GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
			pMain->GetDlgItem(IDC_RADIO1)->EnableWindow();
			pMain->GetDlgItem(IDC_RADIO2)->EnableWindow();
			pMain->GetDlgItem(IDC_IPADDRTSC)->EnableWindow();
			pMain->CloseSocketDef();

			char tmpBuf = 0xff;
			if (!pMain->ConnectTSC())
			{
				Sleep(5000);
				continue;
			}
			send(pMain->m_nSocket, (char *)&tmpBuf,1, 0);
			return 0;
		}

		if (!pMain->ConnectTSC())
		{
			Sleep(5000);
			continue;
		}

		for (int i=0; i<4; i++)
		{
			stTpsAlarm.struTPSInfo.struLaneParam[i].dwVaryType = htonl(ENUM_TRAFFIC_VARY_VEHICLE_ENTER);
		}
		memcpy(buf, &m_stHead, sizeof(INTER_DVR_REQUEST_HEAD_V30));
		memcpy(buf+sizeof(INTER_DVR_REQUEST_HEAD_V30), (const void*)&stTpsAlarm, sizeof(NET_TPS_ALARM));
	
		if (pMain->m_bDetectType) //����û�е�����뿪����һ��
		{
			memcpy(buf+sizeof(INTER_DVR_REQUEST_HEAD_V30), (const void*)&stPedAlarm, sizeof(NET_TPS_PED_ALARM));
			send(pMain->m_nSocket, buf, m_stHead.dwLength, 0);
			Sleep(1000);
			pMain->CloseSocketDef();
			pMain->m_strVehileCount.Format(_T("%d"), ++count);
			pMain->UpdateData(FALSE);
			continue;
		}
		if (send(pMain->m_nSocket, buf, m_stHead.dwLength, 0) <= 0)
		{
			pMain->CloseSocketDef();
			continue;
		}

		Sleep(500);
		pMain->CloseSocketDef();
		
		if (!pMain->ConnectTSC())
		{
			Sleep(5000);
			continue;
		}

		for (int i=0; i<4; i++)
		{
			stTpsAlarm.struTPSInfo.struLaneParam[i].dwVaryType = htonl(ENUM_TRAFFIC_VARY_VEHICLE_LEAVE);
		}
		memcpy(buf, &m_stHead, sizeof(INTER_DVR_REQUEST_HEAD_V30));
		memcpy(buf+sizeof(INTER_DVR_REQUEST_HEAD_V30), (const void*)&stTpsAlarm, sizeof(NET_TPS_ALARM));
		
		if (send(pMain->m_nSocket, buf, m_stHead.dwLength, 0) <= 0)
		{
			pMain->CloseSocketDef();
			continue;
		}

		if (send(pMain->m_nSocket, (char *)&gData, sizeof(gData), 0) <= 0)
		{
			pMain->CloseSocketDef();
			continue;
		}

		if (send(pMain->m_nSocket, (char *)&gStatus, sizeof(gStatus), 0) <= 0)
		{
			pMain->CloseSocketDef();
			continue;
		}

		Sleep(500);
		pMain->m_strVehileCount.Format(_T("%d"), ++count);
		pMain->UpdateData(FALSE);
		pMain->CloseSocketDef();
	}

	return 0;
}

void CvideodetectDlg::OnEnKillfocusTotalFlow()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//AfxMessageBox(_T("data changed "));
	CString str;
	m_data_total_flow.GetWindowText(str);
	gData.nTotalFlow = atoi(str.GetBuffer());
}


void CvideodetectDlg::OnEnKillfocusLargeVechileFlow()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	m_data_large_flow.GetWindowText(str);
	gData.nLargeFlow  = atoi(str.GetBuffer());
}


void CvideodetectDlg::OnEnKillfocusSmallVechileFlow()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	m_data_small_flow.GetWindowText(str);
	gData.nSmallFlow  = atoi(str.GetBuffer());
}


void CvideodetectDlg::OnEnKillfocusPercent()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	m_data_percent.GetWindowText(str);
	gData.nPercent   = atoi(str.GetBuffer());
}


void CvideodetectDlg::OnEnKillfocusSpeed()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	m_data_speed.GetWindowText(str);
	gData.nSpeed  = atoi(str.GetBuffer());
}


void CvideodetectDlg::OnEnKillfocusVechileLength()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	m_data_vechile_length.GetWindowText(str);
	gData.nLength  = atoi(str.GetBuffer());
}


void CvideodetectDlg::OnBnClickedDetectorNolive()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	AfxMessageBox(_T("asdf"));
	gStatus.noLive = m_detector_unlive.GetCheck();
}


void CvideodetectDlg::OnBnClickedDetectorLiveLong()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.liveLong = m_detector_livelong.GetCheck();
}


void CvideodetectDlg::OnBnClickedDetectorUnstable()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.unStable = m_detector_unstable.GetCheck();
}


void CvideodetectDlg::OnBnClickedDetectorCommunicationErr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.commErr = m_detector_communication_err.GetCheck();
}


void CvideodetectDlg::OnBnClickedConfigureErr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.cfgErr = m_detector_configure_err.GetCheck();
}


void CvideodetectDlg::OnBnClickedDetectorUnkonwErr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.unKown = m_detector_unknow_err.GetCheck();
}


void CvideodetectDlg::OnBnClickedDetectorNormal()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_detector_noraml.GetCheck() == 1)//���ѡ�������Ļ����Ͱ���������Ϊ�����ã���ֵ����Ϊ0
	{
		m_detector_unlive.SetCheck(FALSE);
		m_detector_livelong.SetCheck(FALSE);
		m_detector_unstable.SetCheck(FALSE);
		m_detector_communication_err.SetCheck(FALSE);
		m_detector_configure_err.SetCheck(FALSE);
		m_detector_unknow_err.SetCheck(FALSE);

		gStatus.noLive = 0;
		gStatus.liveLong = 0;
		gStatus.unStable = 0;
		gStatus.commErr = 0;
		gStatus.cfgErr = 0;
		gStatus.unKown = 0;
	}

}


void CvideodetectDlg::OnBnClickedInductionUnknow()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.nOther = m_induction_unkown_err.GetCheck();
}


void CvideodetectDlg::OnBnClickedInductionWatchdogErr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.nWatchdog = m_induction_watchdog_err.GetCheck();
}


void CvideodetectDlg::OnBnClickedInductionOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.nOpen = m_induction_open.GetCheck();
}


void CvideodetectDlg::OnBnClickedInductionLow()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.nLow = m_induction_low.GetCheck();
}


void CvideodetectDlg::OnBnClickedInductionHiigh()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	gStatus.nHigh = m_induction_high.GetCheck();
}


void CvideodetectDlg::OnBnClickedInductionNormal()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_induction_normal.GetCheck() == 1)
	{
		m_induction_unkown_err.SetCheck(FALSE);
		m_induction_watchdog_err.SetCheck(FALSE);
		m_induction_open.SetCheck(FALSE);
		m_induction_low.SetCheck(FALSE);
		m_induction_high.SetCheck(FALSE);

		gStatus.nOther = 0;
		gStatus.nWatchdog = 0;
		gStatus.nOpen = 0;
		gStatus.nLow = 0;
		gStatus.nHigh = 0;
	}
}

void CvideodetectDlg::OnEnKillfocusEditEquid()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString str;
	GetDlgItemText(IDC_EDIT_EQUID,str);

	gData.nDetectorId = atoi(str);
	gStatus.nDetectorId = atoi(str);
}
