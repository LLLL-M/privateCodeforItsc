// DlgHikSadp.cpp : implementation file
//

#include "stdafx.h"
#include "DlgHikSadp.h"


#define MAX_LEN 128

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define Single 1

CString g_csPort;
CString g_csMask;
CString g_csIP;

SSADPINFO * ginfo;

//排序回调函数
int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortList* pV=(CSortList*)lParamSort;
	//根据IP地址进行排列
	CString lp1 = pV->GetItemText(lParam1, pV->m_nSortedCol);   
	CString lp2 = pV->GetItemText(lParam2, pV->m_nSortedCol);   

	if (pV->m_fAsc)
	{
		//生序排列
		return lp1.CompareNoCase(lp2);
	}
	else
	{
		//倒序排列
		return lp2.CompareNoCase(lp1);
	}
}
////////////////////////////////////////////
//多语言支持

typedef struct TXT_SRC
{
    int		index;
    char	*text;
}TXT_SRC_S;

TXT_SRC_S TEXT_chinese[]=
{
	{0,		"   在线设备侦测...."},
	{1,		"设备编号"},
	{2,		"软件版本"},
	{3,		"子网掩码"},
	{4,		"IP地址"},
	{5,		"设备PORT"},
	{6,		"设备MAC"},
	{7,		"---- 请输入管理员口令 ----"},
	{8,		"修改"},
	{9,		"取消"},
	{10,	"保存"},
	{11,	"Start"},
	{12,	"返回"},
	{13,	"请输入PORT号!"},
	{14,	"请输入管理员口令!"},
	{15,	"请选择一台设备!"},
	{16,	"确定添加设备(%s)吗?"},
	{17,    "确定"},
	{18,	"错误:参数不是有效值!"},
	{19,	"启动sadp失败!\n"},
	{20,	"保存失败!"},
	{21,	"保存成功!"},
	{22,	"请选择要修改的设备!"},
	{23,	"请输入要修改的子网掩码!"},
	{24,	"请输入要修改的IP!"},
	{25,	"请输入验证码!"},
	{26,	"更新设备，修改成功!"},
	{27,	"修改失败!"},
	{28,	" ----恢复设备缺省密码----"},
	{29,	"PORT号必须在2000-65535之间"},
	{30,	"恢复成功!"},
	{31,	"恢复失败"},
	{32,	"设备类型"},
	{33,	"IP地址"},
	{34,	"端口号"},
	{35,	"设备序列号"},
	{36,	"子网掩码"},
	{37,	"物理地址"},
	{38,	"模拟通道数"},
	{39,	"软件版本"},
	{40,	"DSP版本"},
	{41,	"启动时间"},
	{42,    "设备类型描述"},
	{43,    "OEM厂商信息"},
	{44,    "IPv4网关"},
	{45,    "IPv6地址"},
	{46,    "IPv6网关"},
	{47,    "IPv6子网前缀长度"},
	{48,    "是否支持IPv6"},
	{49,    "IPv4 DHCP状态"},
	{50,    "是否支持修改IPv6"},
	{51,    "是否支持DHCP"},
	{52,    "CMS IPv4地址"},
	{53,    "CMS 端口"},
	{54,    "数字通道数"},
	{55,    "Http 端口"},
	{56,	"OEM 标识"},
	{57,    "OEM 信息"},
	{58,	"版本信息"},
	{59,    "设备硬盘数"}
};

TXT_SRC_S TEXT_english[]=
{
	{0,		"   SADP"},
	{1,		"Device Serial Number"},
	{2,		"Soft Version"},
	{3,		"Subnet Mask"},
	{4,		"IP Address"},
	{5,		"Device Port"},
	{6,		"MAC Address"},
	{7,		"---please input password---"},
	{8,		"modify"},
	{9,		"cancel"},
	{10,	"save"},
	{11,	"start"},
	{12,	"Exit"},
	{13,	"please input device port!"},
	{14,	"please input password!"},
	{15,	"please select a device!"},
	{16,	"are you sure to insert device(%s)?"},
	{17,    "OK"},
	{18,	"err: params invalid!"},
	{19,	"start sadp fail!\n"},
	{20,	"save failed!"},
	{21,	"save successfully!"},
	{22,	"select a device to modify,please!"},
	{23,	"input the subnet,please!"},
	{24,	"input the device IP,please!"},
	{25,	"input the confirm-word!"},
	{26,	"update device,modify device successfully!"},
	{27,	"modify failed!"},
	{28,	"-Resume default password-"},
	{29,	"The port number must between 2000-65535"},
	{30,	"success for Resumption"},
	{31,	"failed to Resump"},
	{32,	"Device Type"},
	{33,	"IP Address"},
	{34,	"Port Number"},
	{35,	"Device Serial No."},
	{36,	"Mask"},
	{37,	"Mac"},
	{38,	"Analog Channels"},
	{39,	"Soft Version"},
	{40,	"DSP Version"},
	{41,	"Start Time"},
	{42,    "Device Type Description"},
	{43,    "OEM Info"},
	{44,    "IPv4 Gateway"},
	{45,    "IPv6 Address"},
	{46,    "IPv6 Gateway"},
	{47,    "IPv6 Subnetmask Len"},
	{48,    "IPv6 support or not"},
	{49,    "IPv4 DHCP state"},
	{50,    "IPv6 modify or not"},
	{51,    "DHCP support or not"},
	{52,    "CMS IPv4"},
	{53,    "CMS Port"},
	{54,    "Digital Channels"},
	{55,    "Http Port"},
	{56,    "OEM Code"},
	{57,    "OEM Info"},
	{58,	"Version Info"},
	{59,    "Harddisk Number"}
};

TXT_SRC_S TEXT_chineseT[]=
{
	{0,		"   絬砞称盎代...."},
	{1,		"砞称絪腹"},
	{2,		"硁ンセ"},
	{3,		"呼被絏"},
	{4,		"IP"},
	{5,		"砞称PORT"},
	{6,		"砞称MAC"},
	{7,		"----叫块恨瞶----"},
	{8,		"э"},
	{9,		""},
	{10,	"玂"},
	{11,	"匡拒砞称"},
	{12,	""},
	{13,	"叫块PORT腹"},
	{14,	"叫块恨瞶"},
	{15,	"叫匡拒籓砞称"},
	{16,	"絋﹚睰砞称(%s)盾"},
	{17,    "絋﹚"},
	{18,	"岿粇把计ぃ琌Τ"},
	{19,	"币笆sadpア毖\n"},
	{20,	"玂ア毖"},
	{21,	"玂Θ"},
	{22,	"叫匡拒璶э砞称"},
	{23,	"叫块璶э呼被絏"},
	{24,	"叫块璶эIP"},
	{25,	"叫块喷靡絏"},
	{26,	"穝砞称эΘ"},
	{27,	"эア毖"},
	{28,	" ----確砞称盞絏----"},
	{29,	"PORT腹ゲ斗2000-65535ぇ丁"},
	{30,	"確Θ"},
	{31,	"確ア毖"},
	{32,	"砞称摸"},
	{33,	"IP"},
	{34,	"狠腹"},
	{35,	"砞称腹"},
	{36,	"呼被絏"},
	{37,	"瞶"},
	{38,	"硄笵腹"},
	{39,	"硁ンセ"},
	{40,	"DSPセ"},
	{41,	"币笆丁"}
};

TXT_SRC_S *TEXT_index[]=
{
	TEXT_chinese,		//0
	TEXT_english,		//1
	TEXT_chineseT		//2
};

char * CDlgHikSadp::GetText(int id)
{
	TXT_SRC_S * TEXT_tmp;

	TEXT_tmp = TEXT_index[m_ilanguage];

	return TEXT_tmp[id].text;
}

//多语言支持
////////////////////////////////////////////


HWND gHwndSADP;

BOOL strlenchk(char *sstr)
{
	if(sstr==NULL)
		return TRUE;

	if(strlen(sstr)>=64)
	{
		return FALSE;
	}else
	{
		return TRUE;
	}
}


void __stdcall SadpDataCallBack(const SADP_DEVICE_INFO *_pDevInfo, void* _pUserData)
{
	//check!!
// 	if((strlenchk(_pDevInfo->szSeries) ||
// 		strlenchk(_pDevInfo->szSerialNO) || 
// 		strlenchk(_pDevInfo->szMAC) ||
// 		strlenchk(_pDevInfo->szIPv4Address) ||
// 		strlenchk(_pDevInfo->szIPv4SubnetMask) ||
// 		strlenchk(_pDevInfo->szDeviceSoftwareVersion) || 
// 		strlenchk(_pDevInfo->szDSPVersion) ||
// 		strlenchk(_pDevInfo->szBootTime)) == FALSE)
// 	{
// 		AfxMessageBox("err : params invalid!");
// 		return;
// 	}

// 	TRACE("  %d  > series=%s,deviceid=%s,hwaddr=%s,praddr=%s,subnetmask=%s,result=%d,sfwversion=%s,dspversion=%s,starttime=%s\n",
// 		result,series,deviceid,hwaddr,praddr,subnetmask,result,sfwversion,dspversion,starttime);

	SSADPINFO * pinfo;

	pinfo = (SSADPINFO*)malloc(sizeof(SSADPINFO));

	memset(pinfo, 0, sizeof(SSADPINFO));


	strcpy(pinfo->sseries,	_pDevInfo->szSeries);
	strcpy(pinfo->szSerialNO, _pDevInfo->szSerialNO);
	strcpy(pinfo->shwaddr,	_pDevInfo->szMAC);
	strcpy(pinfo->spraddr,	_pDevInfo->szIPv4Address);
	strcpy(pinfo->ssubnetmask,	_pDevInfo->szIPv4SubnetMask);
	strcpy(pinfo->ssfwversion,	_pDevInfo->szDeviceSoftwareVersion);
	strcpy(pinfo->sdspversion, _pDevInfo->szDSPVersion);
	strcpy(pinfo->sstarttime, _pDevInfo->szBootTime);

	strcpy(pinfo->szDevDesc, _pDevInfo->szDevDesc);
	strcpy(pinfo->szOEMinfo, _pDevInfo->szOEMinfo);
	strcpy(pinfo->szIPv4Gateway, _pDevInfo->szIPv4Gateway);
	strcpy(pinfo->szIPv6Address, _pDevInfo->szIPv6Address);
	strcpy(pinfo->szIPv6Gateway, _pDevInfo->szIPv6Gateway);
	pinfo->result	=	_pDevInfo->iResult;
	pinfo->dev_type	=	_pDevInfo->dwDeviceType;
	pinfo->port		=	_pDevInfo->dwPort;
	pinfo->enccnt	=	_pDevInfo->dwNumberOfEncoders; 
	pinfo->hdiskcnt	=	_pDevInfo->dwNumberOfHardDisk;

	pinfo->byIPv6MaskLen  =	_pDevInfo->byIPv6MaskLen;
	pinfo->bySupport	  =	_pDevInfo->bySupport; 
	pinfo->byDhcpEnabled  =	_pDevInfo->byDhcpEnabled;
	strcpy(pinfo->szCmsIPv4, _pDevInfo->szCmsIPv4);
	pinfo->wCmsPort = _pDevInfo->wCmsPort;
	pinfo->wHttpPort = _pDevInfo->wHttpPort;
	pinfo->wDigitalChannelNum = _pDevInfo->wDigitalChannelNum;
	//pinfo->byOEMCode = _pDevInfo->byOEMCode;


	if(gHwndSADP)   //gHwndSADP全局变量
	{
		::SendMessage(gHwndSADP, MSG_FIND_DEVICE, (int)pinfo, 0);
	}
	
	free(pinfo);

	return;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgHikSadp dialog


CDlgHikSadp::CDlgHikSadp(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgHikSadp::IDD, pParent)
	, m_origipasswd(_T(""))
{
	//{{AFX_DATA_INIT(CDlgHikSadp)
	m_sdevid = _T("");
	m_sversion = _T("");
	m_uport = 0;
	m_smac = _T("");
	m_spsw = _T("");
	m_strSoftVersion = _T("");
	m_strPUID = _T("");
	m_strCmsPasswd = _T("");
	m_strCmsIPv6 = _T("");
	m_wCmsPort = 0;
	m_wHttpPort = 0;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_retbuf = NULL;
}


void CDlgHikSadp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgHikSadp)
	DDX_Control(pDX, IDC_IPADDRESS_CMS, m_addrCmsIPv4);
	DDX_Control(pDX, IDC_IPADDRESS_ADDR, m_sip);
	DDX_Control(pDX, IDC_IPADDRESS_MASK, m_smask);
	DDX_Control(pDX, IDC_IPADDRESS_GATEWAY, m_addrIPv4Gateway);
	DDX_Control(pDX, IDC_LIST_DEV_INFO, m_slist);
	DDX_Text(pDX, IDC_EDIT_DEV_SN, m_sdevid);
	DDX_Text(pDX, IDC_EDIT_IPv6_ADDRESS, m_strIPv6Adress);
	DDX_Text(pDX, IDC_EDIT_IPv6_GATEWAY, m_strIPv6Gateway);
	DDX_Text(pDX, IDC_EDIT_MASK_LEN, m_strIPv6MaskLen);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, m_strDevDescription);
	DDX_Text(pDX, IDC_EDIT_PORT, m_uport);
	DDX_Text(pDX, IDC_EDIT_MAC, m_smac);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_spsw);
	DDX_Text(pDX, IDC_EDIT_SOFT_VERSION, m_strSoftVersion);
	DDX_Text(pDX, IDC_EDIT_PUID, m_strPUID);
	DDX_Text(pDX, IDC_EDIT_PW, m_strCmsPasswd);
	DDX_Text(pDX, IDC_EDIT_IPv6, m_strCmsIPv6);
	DDX_Text(pDX, IDC_EDIT_CMS_PORT, m_wCmsPort);
	DDX_Text(pDX, IDC_EDIT_DEFAULT_PW, m_origipasswd);
	DDX_Text(pDX, IDC_EDIT_HTTP_PORT, m_wHttpPort);
	//}}AFX_DATA_MAP
	
}


BEGIN_MESSAGE_MAP(CDlgHikSadp, CDialog)
	ON_MESSAGE(MSG_FIND_DEVICE, OnTabDraw)
	//{{AFX_MSG_MAP(CDlgHikSadp)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_BACK, OnButBack)
	ON_NOTIFY(HDN_ITEMCHANGED, IDC_LIST_DEV_INFO, OnItemchangedList)
	ON_NOTIFY(NM_CLICK, IDC_LIST_DEV_INFO, OnClickList)
	ON_BN_CLICKED(IDC_BUTTON_MODIFY, OnButtonModify)
	ON_BN_CLICKED(IDC_BUTTON_CANSEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_SAFE, OnButtonSafe)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_DEV, OnButtonSelectDev)
	ON_BN_CLICKED(IDC_BUTTON_CMS_SET, OnButtonCmsSet)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_PW, OnBnClickedButtonDefaultPW)
	ON_BN_CLICKED(IDC_CHECK_DHCP, OnCheckDhcp)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_DEV_INFO, OnColumnclickListDevInfo)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_DEV_INFO, OnCustomDrawList)
//	ON_STN_CLICKED(IDC_STATIC_PSWINPUT, OnStnClickedStaticPswinput)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgHikSadp message handlers

void CDlgHikSadp::OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		int nold = 0;

		nold = m_slist.GetItemData(nItem);

		switch(nold) 
		{
		case 1:
			pLVCD->clrTextBk = RGB(255, 192, 203);//错误
			break;
		case 0:
			pLVCD->clrTextBk = RGB(255, 192, 203);//错误
			//pLVCD->clrTextBk = RGB(191, 191, 242);//客户端连接or bye
			break;
		default:
			pLVCD->clrTextBk = RGB(255, 192, 203);//错误
			//pLVCD->clrTextBk = RGB(255, 255, 255);
			break;
		}
        // Store the color back in the NMLVCUSTOMDRAW struct.
			
        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
	}
}

BOOL CDlgHikSadp::OnInitDialog() 
{
	CDialog::OnInitDialog();

	DWORD dwSadpVersion = SADP_GetSadpVersion();

	CString csTemp = _T("");
	csTemp.Format("V%d.%d.%d.%d", (0xff000000 & dwSadpVersion)>>24, (0x00ff0000 & dwSadpVersion)>>16,\
		(0x0000ff00 & dwSadpVersion)>>8, (0x000000ff & dwSadpVersion));

	GetDlgItem(IDC_STATIC_SADP_VERSION)->SetWindowText(csTemp);
	GetDlgItem(IDC_BUTTON_SELECT_DEV)->SetWindowText("Start") ;



	SADP_SetLogToFile(3);
	

	// TODO: Add extra initialization here
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

	
#ifdef Single
	GetDlgItem(IDC_BUTTON_SELECT_DEV)->ShowWindow(SW_HIDE);
#else
	GetDlgItem(IDC_BUTTON_SELECT_DEV)->ShowWindow(SW_SHOW);
#endif

	m_brun	 = FALSE;
	m_bedit	 = FALSE;
	m_icount = 1;


	m_sip.SetAddress(0,0,0,0);
	m_smask.SetAddress(0,0,0,0);
	m_dwsip = 0;
	m_dwsmask = 0;

	// Judge the default lanuage ID of Windows System
    WORD wLangID=PRIMARYLANGID(::GetSystemDefaultLangID());

    switch(wLangID)
    {
		case LANG_CHINESE:
			m_ilanguage = 0;
			break;
        case LANG_ENGLISH:
		default:
			m_ilanguage = 1;
			break;
	}
	//set text!
//	m_ilanguage = g_nLanguageType;
	
	gHwndSADP = this->GetSafeHwnd();
	GetDlgItem(IDC_IPADDRESS_MASK)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPADDRESS_ADDR)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPADDRESS_GATEWAY)->EnableWindow(FALSE);

	
	m_btnDhcp = (CButton*)GetDlgItem(IDC_CHECK_DHCP);
	m_btnDhcp->SetCheck(FALSE);
	m_btnDhcp->EnableWindow(FALSE);

	((CEdit *)GetDlgItem(IDC_EDIT_PORT))->SetLimitText(5);
	((CEdit *)GetDlgItem(IDC_EDIT_PASSWORD))->SetLimitText(32);
	((CEdit *)GetDlgItem(IDC_EDIT_DEFAULT_PW))->SetLimitText(32);

	CString strTemp = _T("");
	m_slist.InsertColumn(0, "序号", LVCFMT_CENTER, 30);
	strTemp.Format("%s",GetText(42));
	m_slist.InsertColumn(1, strTemp, LVCFMT_LEFT, 100);
	strTemp.Format("%s",GetText(33));
	m_slist.InsertColumn(2, strTemp, LVCFMT_LEFT, 100);
	strTemp.Format("%s",GetText(34));
	m_slist.InsertColumn(3, strTemp, LVCFMT_LEFT, 80);
	strTemp.Format("%s",GetText(35));
	m_slist.InsertColumn(4, strTemp, LVCFMT_LEFT, 240);
	strTemp.Format("%s",GetText(36));
	m_slist.InsertColumn(5, strTemp, LVCFMT_LEFT, 100);
	strTemp.Format("%s",GetText(37));
	m_slist.InsertColumn(6, strTemp, LVCFMT_LEFT, 140);
	strTemp.Format("%s",GetText(38));
	m_slist.InsertColumn(7, strTemp, LVCFMT_LEFT, 100);
	strTemp.Format("%s",GetText(39));
	m_slist.InsertColumn(8, strTemp, LVCFMT_LEFT, 140);
	strTemp.Format("%s",GetText(40));
	m_slist.InsertColumn(9, strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(41));
	m_slist.InsertColumn(10,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(44));
	m_slist.InsertColumn(11,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(45));
	m_slist.InsertColumn(12,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(46));
	m_slist.InsertColumn(13,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(47));
	m_slist.InsertColumn(14,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(48));
	m_slist.InsertColumn(15,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(49));
	m_slist.InsertColumn(16,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(50));
	m_slist.InsertColumn(17,strTemp,LVCFMT_LEFT,140);
	strTemp.Format("%s",GetText(51));
	m_slist.InsertColumn(18,strTemp,LVCFMT_LEFT,140);

	//CMS IPv4
	strTemp.Format("%s",GetText(52));
	m_slist.InsertColumn(19,strTemp,LVCFMT_LEFT,140);

	//CMS Port
	strTemp.Format("%s",GetText(53));
	m_slist.InsertColumn(20,strTemp,LVCFMT_LEFT,140);

	//Digital Channel Number
	strTemp.Format("%s",GetText(54));
	m_slist.InsertColumn(21,strTemp,LVCFMT_LEFT,140);
	
	//Http port
	strTemp.Format("%s",GetText(55));
	m_slist.InsertColumn(22,strTemp,LVCFMT_LEFT,100);

	//OEM Code
	strTemp.Format("%s",GetText(56));
	m_slist.InsertColumn(23,strTemp,LVCFMT_LEFT,100);

	//OEM info
	strTemp.Format("%s",GetText(57));
	m_slist.InsertColumn(24,strTemp,LVCFMT_LEFT,100);

	//HardDisk Number
	strTemp.Format("%s",GetText(59));
	m_slist.InsertColumn(25,strTemp,LVCFMT_LEFT,100);


	((CEdit *)GetDlgItem(IDC_EDIT_PASSWORD))->SetLimitText(32);

	m_slist.SetExtendedStyle(m_slist.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgHikSadp::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if(m_brun)
	{
		TRACE("STOP");
		SADP_Stop();
	}

	gHwndSADP = NULL;
	m_devlist.RemoveAll();
}

void CDlgHikSadp::fInsDevice(char *dvrip)
{
	SSADPDEV sdev;

	if(strlen(dvrip)<20)
	{
		strcpy(sdev.cdev, dvrip);
		m_devlist.AddTail(sdev);
	}

	return;
}

void CDlgHikSadp::fSetRetBuf(SSADPRET * pbuf)
{

	m_retbuf = pbuf;

	return;
}

BOOL CDlgHikSadp::fIfInUsed(char * sip)
{
	BOOL bret=FALSE;
	SSADPDEV stmp1,stmp2;
	
	memset(&stmp1, 0, sizeof(SSADPDEV));
	strcpy(stmp1.cdev, sip);

	POSITION pos = m_devlist.GetHeadPosition();
	for (int i=0;i < m_devlist.GetCount();i++)
	{
	   stmp2 = m_devlist.GetNext(pos);

	   if(0==strcmp(stmp2.cdev, stmp1.cdev))
	   {
			bret=TRUE;
		   break;
	   }
	}


	return bret;
}

long CDlgHikSadp::OnTabDraw(WPARAM wParam, LPARAM lParam)
{
	SSADPINFO * pinfo;
	CString str = _T("");

	pinfo = (SSADPINFO*)wParam;

	switch(pinfo->result)
	{
	case SADP_RESTART:	//设备重新启动
	case SADP_ADD:		//增加一设备
		fDevOnLine(pinfo);
		break;
	case SADP_DEC:		//设备下线
		fDevOffLine(pinfo);
		break;
	case SADP_UPDATE:	//更新设备
		fDevRefresh(pinfo);
		str.Format(GetText(26));
		GetDlgItem(IDC_STA_STATE)->SetWindowText(str);
		break;
	case SADP_UPDATEFAIL:
		//AfxMessageBox("修改失败!");		
		str.Format(GetText(27));
		GetDlgItem(IDC_STA_STATE)->SetWindowText(str);
		m_uport	=	atoi(g_csPort);
		GetDlgItem(IDC_IPADDRESS_MASK)->SetWindowText(g_csMask);
		GetDlgItem(IDC_IPADDRESS_ADDR)->SetWindowText(g_csIP);
		UpdateData(FALSE);
		break;
	default:
		OutputDebugString("unknow result!\n");
		break;
	}

	return 0;
}

void CDlgHikSadp::fDevOnLine(SSADPINFO *pinfo)
{
	//判断是否为已添加设备
	BOOL bused;

	bused = fIfInUsed(pinfo->spraddr);

	//检查一下是否重复插入
	CString strAddSerial, strListSerial, strAddMac, strListMac;
	BOOL bbingo = FALSE;
	
	strAddSerial = pinfo->szSerialNO;
	strAddMac = pinfo->shwaddr;

	for(int i=0;i<m_slist.GetItemCount();i++)
	{
		strListSerial = m_slist.GetItemText(i, 4);
		strListMac = m_slist.GetItemText(i, 6);
		
		if((strAddSerial == strListSerial) && (strAddMac == strListMac)) 
		{
			bbingo = TRUE;
			break;
		}
	}

	if(bbingo) 
	{
		OutputDebugString("wrong 003(重复插入了!!!)\n");
	}
#if 0
	//只显示包含eth字符的设备
	if(strAddSerial.Find("eth") == -1)
	{
		return;
	}
#endif
	//插入列表
	int item;
	char tbuf[32];
//	char tbuf1[32];

	sprintf(tbuf,"%03d",m_icount);
	
	item=m_slist.InsertItem(50000, tbuf);

// 	CString strDevType = _T("");
// 	strDevType = F_GetDeviceTypeName(pinfo->dev_type);
// 	sprintf(tbuf1,"%s",strDevType);
// 	m_slist.SetItemText(item,1,tbuf1);

	m_slist.SetItemText(item,1,(const char*)pinfo->szDevDesc);
	m_slist.SetItemText(item,2,pinfo->spraddr);
	sprintf(tbuf,"%ld",pinfo->port);
	m_slist.SetItemText(item,3,tbuf);
	m_slist.SetItemText(item,4,pinfo->szSerialNO);
	m_slist.SetItemText(item,5,pinfo->ssubnetmask);
	m_slist.SetItemText(item,6,pinfo->shwaddr);
	m_slist.SetItemText(item,8,pinfo->ssfwversion);

	sprintf(tbuf,"%d",pinfo->enccnt);
	m_slist.SetItemText(item,7,tbuf);
	m_slist.SetItemText(item,9,pinfo->sdspversion);

	m_slist.SetItemText(item,10,pinfo->sstarttime);

//	m_slist.SetItemText(item,12,(const char*)pinfo->szOEMinfo);
	m_slist.SetItemText(item,11,(const char*)pinfo->szIPv4Gateway);
	m_slist.SetItemText(item,12,(const char*)pinfo->szIPv6Address);
	m_slist.SetItemText(item,13,(const char*)pinfo->szIPv6Gateway);

	sprintf(tbuf,"%d",pinfo->byIPv6MaskLen);
	m_slist.SetItemText(item,14,tbuf);
	sprintf(tbuf,"%d",pinfo->bySupport);
	m_slist.SetItemText(item,15,tbuf);
	
	if (1 == pinfo->byDhcpEnabled)
	{
		m_slist.SetItemText(item,16,"ON");
	}
	else 
	{
		m_slist.SetItemText(item,16,"OFF");
	}

	if (0 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "not support");
		m_slist.SetItemText(item, 17, "not support");
		m_slist.SetItemText(item, 18, "not support");
	}
	else if(1 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "support");
		m_slist.SetItemText(item, 17, "not Support");
		m_slist.SetItemText(item, 18, "not Support");
	}
	else if(2 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "not support");
		m_slist.SetItemText(item, 17, "support");
		m_slist.SetItemText(item, 18, "not support");
	}
	else if(3 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "support");
		m_slist.SetItemText(item, 17, "support");
		m_slist.SetItemText(item, 18, "not support");
	}
	else if(4 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "not support");
		m_slist.SetItemText(item, 17, "not support");
		m_slist.SetItemText(item, 18, "support");
	}
	else if(5 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "support");
		m_slist.SetItemText(item, 17, "not support");
		m_slist.SetItemText(item, 18, "support");
	}
	else if(6 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "not support");
		m_slist.SetItemText(item, 17, "support");
		m_slist.SetItemText(item, 18, "support");
	}
	else if(7 == pinfo->bySupport)
	{
		m_slist.SetItemText(item, 15, "support");
		m_slist.SetItemText(item, 17, "support");
		m_slist.SetItemText(item, 18, "support");
	}
	else
	{
		m_slist.SetItemText(item, 15, "not support");
		m_slist.SetItemText(item, 17, "not support");
		m_slist.SetItemText(item, 18, "not support");
	}


	sprintf(tbuf,"%s",pinfo->szCmsIPv4);
	m_slist.SetItemText(item,19,tbuf);

	sprintf(tbuf,"%d",pinfo->wCmsPort);
	m_slist.SetItemText(item,20,tbuf);

	sprintf(tbuf,"%d",pinfo->wDigitalChannelNum);
	m_slist.SetItemText(item,21,tbuf);

	sprintf(tbuf,"%d",pinfo->wHttpPort);
	m_slist.SetItemText(item,22,tbuf);

	sprintf(tbuf,"%d",pinfo->byOEMCode);
	m_slist.SetItemText(item,23,tbuf);

	m_slist.SetItemText(item,24,(const char*)pinfo->szOEMinfo);
	
	//硬盘数
	sprintf(tbuf,"%d",pinfo->hdiskcnt);
	m_slist.SetItemText(item,25,tbuf);

	m_slist.SetItemData(item,bused?0:1);

	m_icount++;
}

void CDlgHikSadp::fDevOffLine(SSADPINFO *pinfo)
{
	//删除列表

	CString strDelSerial, strListSerial, strDelMac, strListMac;
	BOOL bbingo = FALSE;

	strDelSerial = pinfo->szSerialNO;
	strDelMac = pinfo->shwaddr;

	for(int i=0; i<m_slist.GetItemCount(); i++)
	{
		strListSerial = m_slist.GetItemText(i, 4);
		strListMac = m_slist.GetItemText(i, 6);

		if((strDelSerial == strListSerial) && (strDelMac == strListMac)) 
		{
			m_slist.DeleteItem(i);
			bbingo = TRUE;
			break;
		}
	}

	if(!bbingo) 
	{
		OutputDebugString("Delete item failed\n");
	}
}

void CDlgHikSadp::fDevReboot(SSADPINFO *pinfo)
{
	return;
}

void CDlgHikSadp::fDevRefresh(SSADPINFO *pinfo)
{
	//修改列表

	CString strUpdateSerial, strListSerial, strUpdateMac, strListMac;
	BOOL bbingo=FALSE;
	strUpdateSerial = pinfo->szSerialNO;
	strUpdateMac = pinfo->shwaddr;

	for(int i=0;i<m_slist.GetItemCount();i++)
	{
		strListSerial = m_slist.GetItemText(i, 4);
		strListMac = m_slist.GetItemText(i, 6);

		if((strUpdateSerial == strListSerial) && (strUpdateMac == strListMac)) 
		{
			char tbuf[32];
			m_slist.SetItemText(i,1,(const char*)pinfo->szDevDesc);
			m_slist.SetItemText(i,2,pinfo->spraddr);
			sprintf(tbuf,"%ld",pinfo->port);
			m_slist.SetItemText(i,3,tbuf);
			m_slist.SetItemText(i,5,pinfo->ssubnetmask);
			m_slist.SetItemText(i,6,pinfo->shwaddr);
			m_slist.SetItemText(i,8,pinfo->ssfwversion);

			m_slist.SetItemText(i,11,(const char*)pinfo->szIPv4Gateway);
			m_slist.SetItemText(i,12,(const char*)pinfo->szIPv6Address);
			m_slist.SetItemText(i,13,(const char*)pinfo->szIPv6Gateway);
			
			sprintf(tbuf,"%d",pinfo->byIPv6MaskLen);
			m_slist.SetItemText(i,14,tbuf);

			
			if (1 == pinfo->byDhcpEnabled)
			{
				m_slist.SetItemText(i,16,"ON");
			}
			else 
			{
				m_slist.SetItemText(i,16,"OFF");
			}

			if (0 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "not support");
				m_slist.SetItemText(i, 17, "not support");
				m_slist.SetItemText(i, 18, "not support");
			}
			else if(1 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "support");
				m_slist.SetItemText(i, 17, "not Support");
				m_slist.SetItemText(i, 18, "not Support");
			}
			else if(2 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "not support");
				m_slist.SetItemText(i, 17, "support");
				m_slist.SetItemText(i, 18, "not support");
			}
			else if(3 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "support");
				m_slist.SetItemText(i, 17, "support");
				m_slist.SetItemText(i, 18, "not support");
			}
			else if(4 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "not support");
				m_slist.SetItemText(i, 17, "not support");
				m_slist.SetItemText(i, 18, "support");
			}
			else if(5 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "support");
				m_slist.SetItemText(i, 17, "not support");
				m_slist.SetItemText(i, 18, "support");
			}
			else if(6 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "not support");
				m_slist.SetItemText(i, 17, "support");
				m_slist.SetItemText(i, 18, "support");
			}
			else if(7 == pinfo->bySupport)
			{
				m_slist.SetItemText(i, 15, "support");
				m_slist.SetItemText(i, 17, "support");
				m_slist.SetItemText(i, 18, "support");
			}
			else
			{
				m_slist.SetItemText(i, 15, "not support");
				m_slist.SetItemText(i, 17, "not support");
				m_slist.SetItemText(i, 18, "not support");
			}

			sprintf(tbuf,"%s",pinfo->szCmsIPv4);
			m_slist.SetItemText(i,19,tbuf);
			
			sprintf(tbuf,"%d",pinfo->wCmsPort);
			m_slist.SetItemText(i,20,tbuf);
			
			sprintf(tbuf,"%d",pinfo->wDigitalChannelNum);
			m_slist.SetItemText(i,21,tbuf);
			
			sprintf(tbuf,"%d",pinfo->wHttpPort);
			m_slist.SetItemText(i,22,tbuf);

			sprintf(tbuf,"%d",pinfo->byOEMCode);
			m_slist.SetItemText(i,23,tbuf);

			m_slist.SetItemText(i,24,(const char*)pinfo->szOEMinfo);

			sprintf(tbuf,"%d",pinfo->hdiskcnt);
			m_slist.SetItemText(i,25,tbuf);

			bbingo = TRUE;
			break;
		}
	}

	if(!bbingo) OutputDebugString("worng 002\n");

}

void CDlgHikSadp::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here

// 	BOOL bret = SADP_InstallNPF();
// 	if (!bret)
// 	{
// 		AfxMessageBox("Install NPF failed, Please reboot and try again");
// 		return;
// 	}

	
	BOOL bret = SADP_Start_V30(SadpDataCallBack);
	//SADP_Stop();

//	DWORD dwErr = SADP_GetLastError();

	
	if(bret)
	{
		m_brun = TRUE;
	}else
	{
		AfxMessageBox(GetText(19));
	}

	GetDlgItem(IDC_IPADDRESS_MASK)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPADDRESS_ADDR)->EnableWindow(FALSE);

	SetWindowText(GetText(0));

	GetDlgItem(IDC_STATIC_ID)->SetWindowText(GetText(1));
	GetDlgItem(IDC_STATIC_VERSION)->SetWindowText(GetText(2));
	GetDlgItem(IDC_STATIC_MASK)->SetWindowText(GetText(3));
	GetDlgItem(IDC_STATIC_IP)->SetWindowText(GetText(4));
	GetDlgItem(IDC_STATIC_PORT)->SetWindowText(GetText(5));
	GetDlgItem(IDC_STATIC_MAC)->SetWindowText(GetText(6));
	GetDlgItem(IDC_STATIC_PSWINPUT)->SetWindowText(GetText(7));
	GetDlgItem(IDC_BUTTON_MODIFY)->SetWindowText(GetText(8));
	GetDlgItem(IDC_BUTTON_CANSEL)->SetWindowText(GetText(9));
	GetDlgItem(IDC_BUTTON_SAFE)->SetWindowText(GetText(10));
	//GetDlgItem(IDC_BUTTON1)->SetWindowText(GetText(11));
	GetDlgItem(IDC_BUTTON_BACK)->SetWindowText(GetText(12));
	GetDlgItem(IDC_STATIC_MODIFYPSWD)->SetWindowText(GetText(28));
	GetDlgItem(IDC_BUTTON_DEFAULT_PW)->SetWindowText(GetText(17));
	GetDlgItem(IDC_STATIC_DEVICE_DESC)->SetWindowText(GetText(42));
	GetDlgItem(IDC_STATIC_VERSION_INFO)->SetWindowText(GetText(58));
}

void CDlgHikSadp::OnClose() 
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnClose();
}

void CDlgHikSadp::OnCancel() 
{
	// TODO: Add extra cleanup here
//	return;
	CDialog::OnCancel();
}

void CDlgHikSadp::OnOK() 
{
	// TODO: Add extra validation here
	return;
	CDialog::OnOK();
}

void CDlgHikSadp::OnButBack() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CDlgHikSadp::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here

	*pResult = 0;
}

void CDlgHikSadp::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here

	int nItem;

	POSITION pos = m_slist.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   TRACE0("No items were selected!\n");
	else
	{
	  nItem = m_slist.GetNextSelectedItem(pos);

	  GetDlgItem(IDC_BUTTON_MODIFY)->EnableWindow(TRUE);
	  GetDlgItem(IDC_BUTTON_CANSEL)->EnableWindow(FALSE);
	  GetDlgItem(IDC_BUTTON_SAFE)->EnableWindow(FALSE);

	  //显示信息
	  m_strDevDescription = m_slist.GetItemText(nItem, 1);
	  GetDlgItem(IDC_IPADDRESS_ADDR)->SetWindowText(m_slist.GetItemText(nItem, 2).GetBuffer(0));
	  m_uport	=	atoi(m_slist.GetItemText(nItem, 3).GetBuffer(0));
	  m_sdevid	=	m_slist.GetItemText(nItem, 4);
	  GetDlgItem(IDC_IPADDRESS_MASK)->SetWindowText(m_slist.GetItemText(nItem, 5).GetBuffer(0));
	  m_smac	=	m_slist.GetItemText(nItem, 6);
	  m_strSoftVersion = m_slist.GetItemText(nItem, 8);//20080403
	  GetDlgItem(IDC_IPADDRESS_GATEWAY)->SetWindowText(m_slist.GetItemText(nItem, 11).GetBuffer(0));
	  m_strIPv6Adress = m_slist.GetItemText(nItem, 12);
	  m_strIPv6Gateway = m_slist.GetItemText(nItem, 13);
	  m_strIPv6MaskLen = m_slist.GetItemText(nItem, 14);

	  GetDlgItem(IDC_IPADDRESS_CMS)->SetWindowText(m_slist.GetItemText(nItem, 19).GetBuffer(0));
	  m_wCmsPort	=	atoi(m_slist.GetItemText(nItem, 20).GetBuffer(0));
	  m_wHttpPort	=	atoi(m_slist.GetItemText(nItem, 22).GetBuffer(0));


	  int iDhcp = 0;
	  CString strDhcp = m_slist.GetItemText(nItem, 16);
	  if (0 == strDhcp.Compare("ON"))
	  {
		  iDhcp = 1;
	  }
	  else if (0 == strDhcp.Compare("OFF"))
	  {
		   iDhcp = 0;
	  }
	  else
	  {
		  iDhcp = 0;
	  }


	  m_btnDhcp->SetCheck(iDhcp);
	
	  

	  
	  
	  CString str = _T("");
	  GetDlgItem(IDC_STA_STATE)->SetWindowText(str);

	  UpdateData(FALSE);
	}
	
	*pResult = 0;
}

void CDlgHikSadp::fEnableButton(BOOL bedit)
{
	GetDlgItem(IDC_BUTTON_MODIFY)->EnableWindow(!bedit);
	GetDlgItem(IDC_LIST_DEV_INFO)->EnableWindow(!bedit);
	GetDlgItem(IDC_BUTTON_SELECT_DEV)->EnableWindow(!bedit);
	GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(!bedit);

	GetDlgItem(IDC_BUTTON_CANSEL)->EnableWindow(bedit);
	GetDlgItem(IDC_BUTTON_SAFE)->EnableWindow(bedit);

	GetDlgItem(IDC_IPADDRESS_MASK)->EnableWindow(bedit);
	GetDlgItem(IDC_IPADDRESS_ADDR)->EnableWindow(bedit);
	GetDlgItem(IDC_IPADDRESS_GATEWAY)->EnableWindow(bedit);
	GetDlgItem(IDC_EDIT_PORT)->EnableWindow(bedit);
	GetDlgItem(IDC_EDIT_HTTP_PORT)->EnableWindow(bedit);
	GetDlgItem(IDC_EDIT_IPv6_ADDRESS)->EnableWindow(bedit);
	GetDlgItem(IDC_EDIT_IPv6_GATEWAY)->EnableWindow(bedit);
	GetDlgItem(IDC_EDIT_MASK_LEN)->EnableWindow(bedit);

	GetDlgItem(IDC_EDIT_PASSWORD)->ShowWindow(bedit);
	GetDlgItem(IDC_EDIT_PASSWORD)->SetWindowText("");
	GetDlgItem(IDC_STATIC_PSWINPUT)->ShowWindow(bedit);

	GetDlgItem(IDC_CHECK_DHCP)->EnableWindow(bedit);

}

void CDlgHikSadp::OnButtonModify() 
{
	//modify
	UpdateData();
	CString sip,smask,sport;
	BYTE nfield0,nfield1,nfield2,nfield3;

	m_sip.GetAddress(nfield0,nfield1,nfield2,nfield3);
	m_sip.GetAddress(m_dwsip);
	sip.Format("%d.%d.%d.%d",nfield0,nfield1,nfield2,nfield3);

	m_smask.GetAddress(nfield0,nfield1,nfield2,nfield3);
	m_smask.GetAddress(m_dwsmask);
	smask.Format("%d.%d.%d.%d",nfield0,nfield1,nfield2,nfield3);


	sport.Format("%ld",m_uport);

	CString str = _T("");
	GetDlgItem(IDC_STA_STATE)->SetWindowText(str);

	m_bedit = TRUE;
	g_csPort = sport;
	g_csMask = smask;
	g_csIP   = sip;

	fEnableButton(m_bedit);
	
}

void CDlgHikSadp::OnButtonCancel() 
{
	//cancel
	m_sip.SetAddress(m_dwsip);
	m_smask.SetAddress(m_dwsmask);
	POSITION pos = m_slist.GetFirstSelectedItemPosition();
	int nItem = m_slist.GetNextSelectedItem(pos);
	m_uport	=	atoi(m_slist.GetItemText(nItem, 3).GetBuffer(0));
	UpdateData(FALSE);

	m_bedit = FALSE;

	fEnableButton(m_bedit);
}

void CDlgHikSadp::OnButtonSafe() 
{


	//check
	char tbuf[64];
	memset(tbuf, 0, 64);
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(tbuf, 64);
	if(strlen(tbuf)<=0)
	{
		AfxMessageBox(GetText(13));
		return;
	}	
	int iport = atoi(tbuf);
	memset(tbuf, 0, 64);
	GetDlgItem(IDC_EDIT_PASSWORD)->GetWindowText(tbuf, 64);
	if(strlen(tbuf)<0)
	{
		AfxMessageBox(GetText(14));
		return;
	}

	//save
	UpdateData(TRUE);//20080403
	if(iport<2000 || iport>65535)
	{
		AfxMessageBox(GetText(29));
		return;
	}

	//do it
	CString sip,smask,sport,strHttpPort;
	BYTE nfield0,nfield1,nfield2,nfield3;

	m_sip.GetAddress(nfield0,nfield1,nfield2,nfield3);
	sip.Format("%d.%d.%d.%d",nfield0,nfield1,nfield2,nfield3);

	m_smask.GetAddress(nfield0,nfield1,nfield2,nfield3);
	smask.Format("%d.%d.%d.%d",nfield0,nfield1,nfield2,nfield3);


	CString strIPv4Gateway;
	m_addrIPv4Gateway.GetAddress(nfield0,nfield1,nfield2,nfield3);
	strIPv4Gateway.Format("%d.%d.%d.%d",nfield0,nfield1,nfield2,nfield3);

// 	unsigned char szMaskLen[16] = {0};
// 	memset(szMaskLen, 0, 16);
// 	GetDlgItem(IDC_EDIT_MASK_LEN)->GetWindowText(szMaskLen, 16);
	unsigned char byMaskLen = atoi(m_strIPv6MaskLen);

	int iDhcpCheck = m_btnDhcp->GetCheck();





	sport.Format("%ld",m_uport);

	SADP_DEV_NET_PARAM struNetParam;
	memset(&struNetParam, 0, sizeof(SADP_DEV_NET_PARAM));
	strcpy(struNetParam.szIPv4Address, sip);
	strcpy(struNetParam.szIPv4SubNetMask, smask);
	strcpy(struNetParam.szIPv4Gateway, strIPv4Gateway);
	strcpy(struNetParam.szIPv6Address, m_strIPv6Adress);
	strcpy(struNetParam.szIPv6Gateway, m_strIPv6Gateway);
	struNetParam.wPort = atoi(sport);
	struNetParam.byDhcpEnable = iDhcpCheck;
	struNetParam.byIPv6MaskLen = byMaskLen;
	struNetParam.wHttpPort = m_wHttpPort;


	int bret = SADP_ModifyDeviceNetParam(m_smac, m_spsw, &struNetParam);

	TRACE("desDLC=%s,desIP=%s,subnetmask=%s,passwd=%s,port=%s",
		m_smac,sip,smask,m_spsw,sport);
	if (bret == 0)
	{
		GetDlgItem(IDC_EDIT_PASSWORD)->SetWindowText("");
		AfxMessageBox(GetText(20));
		return;			//20080325 yujl
	}
	else if (bret == 1)
	{
		AfxMessageBox(GetText(21));
		POSITION pos = m_slist.GetFirstSelectedItemPosition();
		int nItem = m_slist.GetNextSelectedItem(pos);
//		m_uport	=	atoi(m_slist.GetItemText(nItem, 3).GetBuffer(0));
		m_slist.SetItemText(nItem,3,sport);
		m_slist.SetItemText(nItem,5,smask);
		m_slist.SetItemText(nItem,2,sip);
		strHttpPort.Format("%d", m_wHttpPort);
		m_slist.SetItemText(nItem,22,strHttpPort);
	}
	GetDlgItem(IDC_EDIT_PASSWORD)->SetWindowText("");
	m_bedit = FALSE;

	fEnableButton(m_bedit);
}

void CDlgHikSadp::OnButtonCmsSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString strIPv4;
	BYTE nfield0,nfield1,nfield2,nfield3;
	m_addrCmsIPv4.GetAddress(nfield0,nfield1,nfield2,nfield3);
	strIPv4.Format("%d.%d.%d.%d",nfield0,nfield1,nfield2,nfield3);

	SADP_CMS_PARAM struCmsParam;
	memset(&struCmsParam, 0, sizeof(SADP_CMS_PARAM));
	strcpy(struCmsParam.szCmsIPv4, strIPv4);
	strcpy(struCmsParam.szCmsIPv6, m_strCmsIPv6);
	strcpy(struCmsParam.szPUID, m_strPUID);
	strcpy(struCmsParam.szPassword, m_strCmsPasswd);
	struCmsParam.wCmsPort = m_wCmsPort;
	int iRet = SADP_SetCMSInfo(m_smac, &struCmsParam);
	if (1 == iRet)
	{
		AfxMessageBox("Set successful!");
	}
	else if(0 == iRet)
	{
		AfxMessageBox("Fail to set!");
	}
	else
	{
		AfxMessageBox("unvalid return value!");
	}
}

void CDlgHikSadp::OnButtonSelectDev() 
{
	BOOL bRet = FALSE;
	CString strCaption;
	GetDlgItem(IDC_BUTTON_SELECT_DEV)->GetWindowText(strCaption);
	if (0 == strCaption.CompareNoCase("Stop"))
	{
		bRet = SADP_Stop();
		if (bRet)
		{
			GetDlgItem(IDC_BUTTON_SELECT_DEV)->SetWindowText("Start");
		}
		else
		{
			AfxMessageBox("Stop failed");
		}
		
	}
	else
	{
		bRet = SADP_Start_V30(SadpDataCallBack);
		if (bRet)
		{
			GetDlgItem(IDC_BUTTON_SELECT_DEV)->SetWindowText("Stop");
		}
		else
		{
			AfxMessageBox("Start failed");
		}
	}


// 	char tbuf[256];
// 	int nItem;
// 
// 	POSITION pos = m_slist.GetFirstSelectedItemPosition();
// 	if (pos == NULL)
// 	{
// 		AfxMessageBox(GetText(15));
// 		return;
// 	}else
// 	{
// 		nItem = m_slist.GetNextSelectedItem(pos);
// 
// 		sprintf(tbuf, GetText(16), m_slist.GetItemText(nItem, 4));
// 
// 		if(IDOK == AfxMessageBox(tbuf, MB_OKCANCEL|MB_ICONQUESTION))
// 		{
// 			if(m_retbuf)
// 			{
// 				memset(m_retbuf,0,sizeof(SSADPRET));
// 
// 				m_retbuf->bselect	=	TRUE;
// 
// 				strcpy(m_retbuf->sversion,m_slist.GetItemText(nItem, 8));
// 				strcpy(m_retbuf->sseries,m_slist.GetItemText(nItem, 1));
// 				strcpy(m_retbuf->szSerialNO,m_slist.GetItemText(nItem, 4));
// 				strcpy(m_retbuf->shwaddr,m_slist.GetItemText(nItem, 6));
// 				strcpy(m_retbuf->spraddr,m_slist.GetItemText(nItem, 2));
// 				strcpy(m_retbuf->ssubnetmask,m_slist.GetItemText(nItem, 5));
// 
// 				m_retbuf->port	=	atoi(m_slist.GetItemText(nItem, 3).GetBuffer(0));
// 				m_retbuf->enccnt=	atoi(m_slist.GetItemText(nItem, 7).GetBuffer(0));
// 
// 			}
// 
// 			CDialog::OnOK();
// 
// 		}else
// 		{
// 			if(m_retbuf)
// 			{
// 				memset(m_retbuf,0,sizeof(SSADPRET));
// 			}
// 		}
// 	}

}

void CDlgHikSadp::OnBnClickedButtonDefaultPW()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char TempPasswd[MAX_LEN]={0};      
	GetDlgItemText(IDC_EDIT_DEFAULT_PW,TempPasswd,MAX_LEN);


	if(strcmp(TempPasswd,"")==0 )
	{
		AfxMessageBox(GetText(25));
		return;
	}

	int iFlag = SADP_ResetDefaultPasswd(m_sdevid.GetBuffer(0), TempPasswd);

	SetDlgItemText(IDC_EDIT_DEFAULT_PW,"");
	if(iFlag == 1)
	{
		AfxMessageBox(GetText(30));
	}
	else if(iFlag == 0)
	{
		AfxMessageBox(GetText(31));
		return;			//20080325 yujl
	}
	GetDlgItem(IDC_EDIT_DEFAULT_PW)->SetWindowText("");
}



void CDlgHikSadp::OnCheckDhcp() 
{
	// TODO: Add your control notification handler code here
	if (BST_CHECKED == m_btnDhcp->GetCheck())
	{
		GetDlgItem(IDC_IPADDRESS_MASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_IPADDRESS_ADDR)->EnableWindow(FALSE);
		GetDlgItem(IDC_IPADDRESS_GATEWAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_IPv6_ADDRESS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_IPv6_GATEWAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_MASK_LEN)->EnableWindow(FALSE);
	}
	else if (BST_UNCHECKED == m_btnDhcp->GetCheck())
	{
		GetDlgItem(IDC_IPADDRESS_MASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_IPADDRESS_ADDR)->EnableWindow(TRUE);
		GetDlgItem(IDC_IPADDRESS_GATEWAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_IPv6_ADDRESS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_IPv6_GATEWAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_MASK_LEN)->EnableWindow(TRUE);
	}
}

//新增排序功能响应
void CDlgHikSadp::OnColumnclickListDevInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	int i = m_slist.GetItemCount();   
    for (int k=0;k<i;k++)   
    {   
		//把ListVCtrl各项的值设置到ItemData中，否则回调函数无法取出各项值
        m_slist.SetItemData(k,k);   
    }   
	//获取点击的列以及正序逆序排列的轮换
	if( pNMListView->iSubItem == m_slist.m_nSortedCol )
	{
		m_slist.m_fAsc = !m_slist.m_fAsc;
	}
	else
	{
		m_slist.m_fAsc = TRUE;
		m_slist.m_nSortedCol = pNMListView->iSubItem;
	}
	m_slist.SortItems( ListCompare, (DWORD)&m_slist );    
	
	*pResult = 0;
}
