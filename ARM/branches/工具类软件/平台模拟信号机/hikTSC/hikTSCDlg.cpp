

// hikTSCDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "hikTSC.h"
#include "hikTSCDlg.h"
#include "afxdialogex.h"
#include <ws2tcpip.h> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_NUM_MACHINE	255	//本模拟程序支持的信号机个数
#define PORT_TCP		54321 //用来监听TCP 54321端口，一旦有链接，则新建一个线程用来和现有链接通讯

SOCKET	g_SocketArray[255] = {0};


struct STRU_N_DscTypeLogin
{
	WORD	wProtocol;		//信号机协议类型，1-NTCIP，2-HIK，3-GB
	WORD	wDscType;		//信号机型号，0:500型，1:300-44型，2:300-22型
	UINT	unPort;			//通讯端口
};

struct STRU_Extra_DscTypeLogin
{
	unsigned int unExtraParamHead;				//消息头，默认为0x6e6e
	unsigned int unExtraParamID;					//消息类型

	STRU_N_DscTypeLogin dscTypeLogin;		//流量占有率等结构体	
};


typedef struct UDP_INFO
{
	int iHead;                      //消息头，默认为0x6e6e
	int iType;						//消息类型，0x94 - 下载检测参数 | 0x88 - 故障清除 | 0x15b - 上载故障信息 | 0x97 - 上载红灯电流 | 0x93 - 上载检测参数 
	int iValue[16*64];						//
}STRU_UDP_INFO;


//倒计时接口获取反馈：
typedef struct STRU_Extra_Param_Phase_Counting_Down_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0x9E表示上载
     unsigned char    stVehPhaseCountingDown[16][2];    //16个机动车相位的当前灯色及倒计时
     							//第0列表示当前机动车相位的状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
     							//第1列表示当前机动车相位所属相位倒计时时间
     unsigned char    stPedPhaseCountingDown[16][2];    //16个行人相位的当前灯色及倒计时
     							//第0列表示当前机动车相位的状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
								//第1列表示当前行人相位所属相位倒计时时间
     unsigned char    ucPlanNo;				//当前运行方案号
     unsigned char    ucCurCycleTime;			//当前运行周期长
     unsigned char    ucCurRunningTime;		//当前运行时间
     unsigned char    ucChannelLockStatus;		//通道是否被锁定状态，1表示通道锁定，0表示通道未锁定
     unsigned char    ucCurPlanDsc[16];			//当前运行方案描述
     unsigned char    ucOverlap[16][2];                    //跟随相位状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
     unsigned char    stPhaseRunningInfo[16][2];	//各相位运行时间和绿信比
							//第0列表示该相位绿信比；第1列表示该相位运行时间，绿灯亮起后第1列才有数值，否则为0
     unsigned char    ucChannelStatus[32]; //32个通道状态，7:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪 ,0:不起效（不对该通道控制） 
     unsigned char    ucWorkingTimeFlag; //时间段控制起效标志，1表示以下时间段控制起效；0表示全部时间内起效，后续时间参数无效
     unsigned char    ucBeginTimeHour; //控制起效时间：小时
     unsigned char    ucBeginTimeMin; //控制起效时间：分钟
     unsigned char    ucBeginTimeSec; //控制起效时间：秒
     unsigned char    ucEndTimeHour; //控制结束时间：小时
     unsigned char    ucEndTimeMin; //控制结束时间：分钟
     unsigned char    ucEndTimeSec; //控制结束时间：秒
     unsigned char    ucReserved[9]; //预留
}PHASE_COUNTING_DOWN_FEEDBACK_PARAMS;    

typedef struct
{
	int port;
	int index;
}VirtualTSCParams;//虚拟机需要的两个参数

typedef struct
{
	int type;
	int id;
	int val;
	
}DownloadDataParams;//虚拟机响应上传数据结构体



//流量查询接口，每次返回数组
unsigned char cFlowArray_1[] = {0xD0, 0x10, 0x04 };

unsigned char cFlowArray_2[] = {0xC0, 0x12, 0x05, 0x01, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char cFlowArray_channel[] = {	0xC0, 0x12, 0x01, 0x01, 0x20, 0x01, 0x01, 0x02, 0x01, 0x00, 0x02, 0x02, 0x02, 0x01, 0x00, 0x03, 0x03, 0x02, 0x01, 0x00, 0x04, 0x04, 0x02, 0x01, 
																		0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 
																		0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 
																		0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x13, 
																		0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 
																		0x18, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 
																		0x00, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };

unsigned char cStopArray[] = {0xFF,0xFE,0xFF,0xFE};//当收到该消息后，就退出当前接收线程。



int nMachineNum;						//用户期望开启的虚拟机总数
int nPortNo;							//虚拟机的起始端口
int nIsMachineRunning;					//虚拟机是否开始运行
char cIP[64];							//	用户选择的本地IP地址
HANDLE nAllMachineArray[MAX_NUM_MACHINE];//存放的是所有虚拟机的主线程ID

PHASE_COUNTING_DOWN_FEEDBACK_PARAMS g_CountdownValue[MAX_NUM_MACHINE];//默认情况下发送的倒计时信息
DownloadDataParams					g_downloadParams;	//如果20000端口收到0x6e6e的数据，则解析其ID后，返回该结构体。


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


// ChikTSCDlg 对话框




ChikTSCDlg::ChikTSCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ChikTSCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ChikTSCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_IP, m_IP);
}

BEGIN_MESSAGE_MAP(ChikTSCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &ChikTSCDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()


// ChikTSCDlg 消息处理程序

BOOL ChikTSCDlg::OnInitDialog()
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码
	nIsMachineRunning = 0;
	getLocalhostAddress();
	GetDlgItem(IDC_EDIT_TSC_NUM)->SetWindowText(_T("1"));
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(_T("161"));
	SetDefaultCountDownValue();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void ChikTSCDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void ChikTSCDlg::OnPaint()
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
HCURSOR ChikTSCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//获取用户输入的数字并校验，只有校验正确后才开启模拟程序
int ChikTSCDlg::GetInputValue()
{
	CString str;
	GetDlgItem(IDC_EDIT_TSC_NUM)->GetWindowText(str);

	nMachineNum = _ttoi(str);
	if((nMachineNum < 1) || (nMachineNum > MAX_NUM_MACHINE))
	{
		MessageBox(_T("信号机台数暂时只支持1-255."));
		return 1;
	}
	
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(str);

	nPortNo  = _ttoi(str);
	if((nPortNo < 0) || (nPortNo > 65536))
	{
		MessageBox(_T("端口号请输入0-65536之间的数字."));
		return 2;
	}
	
	GetDlgItem(IDC_COMBO_IP)->GetWindowText(str);
	strcpy_s (cIP,str.GetBuffer());
	TRACE("设定的IP地址是 %s, 虚拟机个数是 %d, 起始端口是 %d\n",cIP,nMachineNum,nPortNo);
	return 0;
}

//开启模拟程序后，要禁用输入框。
void ChikTSCDlg::DisableInputEdit()
{
	if(nIsMachineRunning == 0)
	{
		nIsMachineRunning = 1;
		//disable input
		(GetDlgItem(IDC_EDIT_TSC_NUM))->EnableWindow(FALSE);
		(GetDlgItem(IDC_EDIT_PORT))->EnableWindow(FALSE);
		(GetDlgItem(IDC_COMBO_IP))->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_START)->SetWindowText(_T("停止模拟程序"));
		TRACE("<启动模拟程序>\n");
	}
	else
	{
		nIsMachineRunning = 0;
		(GetDlgItem(IDC_EDIT_TSC_NUM))->EnableWindow(TRUE);
		(GetDlgItem(IDC_EDIT_PORT))->EnableWindow(TRUE);
		(GetDlgItem(IDC_COMBO_IP))->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_START)->SetWindowText(_T("启动模拟程序"));
		TRACE("<停止模拟程序>\n");
	}	
}

#define PHASE_GREEN_SPLIT	30
#define PHASE_CIRCLE		120
#define YELLOW_LAMP 		3
#define ALL_RED				0
#define PHASE_NUM			4
#define GET_COLOR(val)    (((val) == 1) ? "绿" : ((val == 2) ? "红" : ((val == 3) ? "黄" : "")))


//默认倒计时参数
void ChikTSCDlg::SetDefaultCountDownValue()
{
	int i = 0;
	int j = 0;

	memset(g_CountdownValue,0,sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS)*MAX_NUM_MACHINE);
	
	for(i = 0; i < MAX_NUM_MACHINE; i++)
	{
		g_CountdownValue[i].unExtraParamHead = 0x6e6e;
		g_CountdownValue[i].unExtraParamID = 0x9e;
		
		g_CountdownValue[i].ucPlanNo = 1;
		g_CountdownValue[i].ucCurCycleTime = PHASE_CIRCLE;
		strcpy((char *)g_CountdownValue[i].ucCurPlanDsc,"Plan 1");

		
		for(j = 0; j < 4; j++)
		{
			g_CountdownValue[i].stVehPhaseCountingDown[j][0] = 2;
			g_CountdownValue[i].stVehPhaseCountingDown[j][1] = PHASE_GREEN_SPLIT*(j+1)+1;//每个相位的倒计时时间是绿信比的N倍。

			g_CountdownValue[i].stPhaseRunningInfo[j][0] = PHASE_GREEN_SPLIT;
		}
		g_CountdownValue[i].stVehPhaseCountingDown[0][0] = 1;//默认从相位1开始运行，那么只有相位1的初始状态是绿灯
	}

	//这是下载参数的默认值
	g_downloadParams.type = 0x6e6e;
	g_downloadParams.id = 0;
	g_downloadParams.val = 0;
}

//设置正常的相位倒计时
void SetRunningCountDownValue(int index)
{
	int i = 0;

	for(i = 0;  i < 4; i++)
	{
		g_CountdownValue[index].stVehPhaseCountingDown[i][1] -= 1;
	
		if((g_CountdownValue[index].stVehPhaseCountingDown[i][0] == 1) || (g_CountdownValue[index].stVehPhaseCountingDown[i][0] == 3))
		{
			g_CountdownValue[index].stPhaseRunningInfo[i][1]++;
		}

		if(g_CountdownValue[index].stVehPhaseCountingDown[i][1] == YELLOW_LAMP)
		{
			g_CountdownValue[index].stVehPhaseCountingDown[i][0] = 3;//黄灯时间
		}
		else if(g_CountdownValue[index].stVehPhaseCountingDown[i][1] == 0)
		{
			g_CountdownValue[index].stVehPhaseCountingDown[i][0] = 2;//red
			g_CountdownValue[index].stVehPhaseCountingDown[i][1] = PHASE_GREEN_SPLIT*4;
			g_CountdownValue[index].stPhaseRunningInfo[i][1] = 0;
			
			if(i == 3)
			{
				g_CountdownValue[index].stVehPhaseCountingDown[0][0] = 1;
				g_CountdownValue[index].stPhaseRunningInfo[0][1]++;
			}
			else
			{
				g_CountdownValue[index].stVehPhaseCountingDown[i+1][0] = 1;
			}
		}
	}

	if(++g_CountdownValue[index].ucCurRunningTime > g_CountdownValue[index].ucCurCycleTime)
	{
		g_CountdownValue[index].ucCurRunningTime = 0;	
	}

	if(index == 159)
	{
	  TRACE("PrintVehCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n",
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[0][0]),g_CountdownValue[index].stVehPhaseCountingDown[0][1],
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[1][0]),g_CountdownValue[index].stVehPhaseCountingDown[1][1],
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[2][0]),g_CountdownValue[index].stVehPhaseCountingDown[2][1],
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[3][0]),g_CountdownValue[index].stVehPhaseCountingDown[3][1],
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[4][0]),g_CountdownValue[index].stVehPhaseCountingDown[4][1],
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[5][0]),g_CountdownValue[index].stVehPhaseCountingDown[5][1],
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[6][0]),g_CountdownValue[index].stVehPhaseCountingDown[6][1],
	                                                        GET_COLOR(g_CountdownValue[index].stVehPhaseCountingDown[7][0]),g_CountdownValue[index].stVehPhaseCountingDown[7][1]);



	    TRACE("PrintRunInfo>  %d/%d--%d/%d--%d/%d--%d/%d     %d/%d\n\n",
	                                                        g_CountdownValue[index].stPhaseRunningInfo[0][1],g_CountdownValue[index].stPhaseRunningInfo[0][0],
	                                                        g_CountdownValue[index].stPhaseRunningInfo[1][1],g_CountdownValue[index].stPhaseRunningInfo[1][0],
	                                                        g_CountdownValue[index].stPhaseRunningInfo[2][1],g_CountdownValue[index].stPhaseRunningInfo[2][0],
	                                                        g_CountdownValue[index].stPhaseRunningInfo[3][1],g_CountdownValue[index].stPhaseRunningInfo[3][0],
	                                                        
	                                                        g_CountdownValue[index].ucCurRunningTime,
	                                                        g_CountdownValue[index].ucCurCycleTime);
	}

  


}

 DWORD WINAPI ServiceTestCountdown(LPVOID lpParam)
 {
	int i = 0;
 
	while(1){

		Sleep(1000);

		for(i = 0; i < 200; i++)
		{	
			SetRunningCountDownValue(i);
		}
	}
	return FALSE;
 }




//虚拟机连接、断开、流量查询等基础服务
DWORD WINAPI ServiceBasic(LPVOID  lpParam)
{
	VirtualTSCParams *pData = (VirtualTSCParams *)lpParam;
	//TRACE("ServiceBasic %d, %d, %p\n",pData->port,pData->index,pData);
	SOCKET sockfd = 0;
	char  cRecvBuf[1024] = {0};
	sockaddr_in localAddr ;
	sockaddr_in fromAddr ;
	int ret = 0;
	int fromLen= sizeof(fromAddr);
	int port = pData->port;
	int index = pData->index;
	unsigned char cTemp1= 0;
	unsigned char cTemp2= 0;
	unsigned char cTemp3= 0;
	unsigned char cTemp4= 0;
	sockfd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);	
	if(INVALID_SOCKET == sockfd)
	{
		TRACE("ServiceBasic create socket error. \n");
		return FALSE;
	}
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	
	//绑定接收套接字
	if (SOCKET_ERROR == bind(sockfd,(sockaddr*)&localAddr,sizeof(localAddr)))
	{
		TRACE("ChikTSCDlg.ServiceBasic bind socket error. errno %d , port %d\n",GetLastError(),port);
		
		closesocket(sockfd);
		return FALSE;
	}	
	TRACE("ServiceBasic %d Start .\n",index+1);
	while(1)
	{
		ret = recvfrom(sockfd, cRecvBuf, sizeof(cRecvBuf), 0, (struct sockaddr *)&fromAddr, &fromLen);

		if(ret == -1)
		{	
			TRACE("recv error . errno %d\n",GetLastError());
			continue;
		}
		cTemp1 = (cRecvBuf[0]&0xff);//这个决定了信号类型
		cTemp2 = (cRecvBuf[2]&0xff);//这个决定了消息ID
		cTemp3 =  (cRecvBuf[7]&0xff);
		cTemp4 =  (cRecvBuf[8]&0xff);
		if(cTemp1 == 0x7e)//连接、断开连接信息，直接将收到的信息返回即可。
		{
			sendto(sockfd, cRecvBuf, strlen(cRecvBuf)+1, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		}
		else if(cTemp1 == 0x90)//块数据
		{
			cFlowArray_1[2] = cTemp2;
			sendto(sockfd, (char *)cFlowArray_1, sizeof(cFlowArray_1), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));	
		}
		//else if(cTemp1 == 0x80)//流量数据
		//{
		//	cFlowArray_2[2] = cTemp2;
		//	sendto(sockfd, (char *)cFlowArray_2, sizeof(cFlowArray_2), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));	
		//}
		else if(cTemp1 == 0x80 && cTemp3 == 0x08 && cTemp4 == 0x02)//通道数据
		{
			cFlowArray_channel[2] = cTemp2;
			sendto(sockfd, (char *)cFlowArray_channel, sizeof(cFlowArray_channel), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));	
		}
		else if((cTemp1 == 0xFF)&&((cRecvBuf[1]&0xff) == 0xFE)&&(cTemp2 == 0xFF)&&((cRecvBuf[3]&0xff) == 0xFE))
		{
			//TRACE("Machine %d will exit .\n",index+1);
			break;
		}
		else
		{
			TRACE("Recv Unkown Msg , len %d ,content %#x %#x %#x %#x\n",ret,cRecvBuf[0],cRecvBuf[1],cRecvBuf[2],cRecvBuf[3]);
		}
	}
	TRACE("ServiceBasic %d Stop .\n",index+1);
	closesocket(sockfd);
	return TRUE;
}


//倒计时响应函数
 DWORD WINAPI ServiceCountdown(LPVOID lpParam)
{
	VirtualTSCParams *pData = (VirtualTSCParams *)lpParam;
	SOCKET sockfd = 0;
	char  cRecvBuf[10240] = {0};
	sockaddr_in localAddr ;
	sockaddr_in fromAddr ;
	int ret = 0;
	int fromLen = sizeof(fromAddr);
	int port = ((pData->port)+19839);
	int index = pData->index;
	sockfd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);	
	
	if(INVALID_SOCKET == sockfd)
	{
		TRACE("ChikTSCDlg.ServiceCountdown create socket error. \n");
		return FALSE;
	}
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	
	//绑定接收套接字
	if (SOCKET_ERROR == bind(sockfd,(sockaddr*)&localAddr,sizeof(localAddr)))
	{
		TRACE("ChikTSCDlg.ServiceCountdown bind socket error. \n");
		closesocket(sockfd);
		return FALSE;
	}	
	TRACE("ServiceCountdown %d Start .\n",index+1);
	while(1)
	{
		ret = recvfrom(sockfd, cRecvBuf, sizeof(cRecvBuf), 0, (struct sockaddr *)&fromAddr, &fromLen);

		if(ret == -1)
		{			
			continue;
		}
		//协议消息
		else if( ((cRecvBuf[0]&0xff) == 0x6e) && ((cRecvBuf[1]&0xff) == 0x6e) && ((cRecvBuf[4]&0xff) == 0xDF) )
		{
			//TRACE("Countdown Index:  %d\n",index);
			STRU_Extra_DscTypeLogin exDscTypeLogin;
			exDscTypeLogin.unExtraParamHead = 0x6e6e;
			exDscTypeLogin.unExtraParamID = 223;
			exDscTypeLogin.dscTypeLogin.wDscType = 0;
			exDscTypeLogin.dscTypeLogin.unPort = 161;
			exDscTypeLogin.dscTypeLogin.wProtocol = 1;

			sendto(sockfd, (char *)&exDscTypeLogin, sizeof(STRU_Extra_DscTypeLogin), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		}
		//倒计时消息
		else if(((cRecvBuf[0]&0xff) == 0x6e) && ((cRecvBuf[1]&0xff) == 0x6e) && ((cRecvBuf[4]&0xff) == 0x9e))
		{
			TRACE("Countdown Index:  %d\n",index);
			SetRunningCountDownValue(index);
			sendto(sockfd, (char *)&g_CountdownValue[index], sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		}
		else
		{
			g_downloadParams.id = ((cRecvBuf[4]&0xff) | ((cRecvBuf[5]&0xff)<<8) | ((cRecvBuf[6]&0xff)<<16) | ((cRecvBuf[7]&0xff)<<24));
			sendto(sockfd, (char *)&g_downloadParams, sizeof(g_downloadParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		}
	}
	TRACE("ServiceCountdown %d Stop .\n",index+1);
	closesocket(sockfd);
	return TRUE;
}

void SendTcpData(int index,unsigned char *array,int len)
{
	int ret = 0;
	
	if((index < MAX_NUM_MACHINE ) && (index > 0))
	{
		ret = send(g_SocketArray[index],(const char*)array,len,0);

		if(ret == SOCKET_ERROR)
		{
			TRACE("index  %d send data failed .\n",index);
		}
	}

}



////子线程，用来和 信号机进行TCP通讯的
//DWORD WINAPI ServiceTCPConnected(LPVOID lpParam)
//{
//	SOCKET pSocket = *(SOCKET *)lpParam;
//	int ret  = 0;
//	unsigned long ul=1;  
//	unsigned char buf[1024] = {0};
//	int err = 0;
//
//	ret=ioctlsocket(pSocket,FIONBIO,(unsigned long *)&ul);//设置成非阻塞模式。  
//
//	while(ret==SOCKET_ERROR){
//		
//		ioctlsocket(pSocket,FIONBIO,(unsigned long *)&ul);//设置成非阻塞模式。  
//	}
//
//
//	while(1)
//	{
//		ret = recv(pSocket,(char*)buf,sizeof(buf),0);		
//
//		if(ret==SOCKET_ERROR){
//
//			err=WSAGetLastError();  
//
//			if(err==WSAETIMEDOUT){
//
//				//TRACE("time out\n");
//			}else if(err==WSAENETDOWN){
//				TRACE("connect closed \n");
//			}
//				
//
//		}  
//		else{//deal ...
//
//		}
//	}
//
//
//}



////主要用来监听TCP端口，等待信号机链接。
//DWORD WINAPI ServiceTCPConnected(LPVOID lpParam)
//{
//	SOCKET sockfd = 0;
//	char  cRecvBuf[10240] = {0};
//	sockaddr_in localAddr ;
//	sockaddr_in fromAddr ;
//	int ret = 0;
//	int fromLen = sizeof(fromAddr);
//	sockfd = socket(AF_INET, SOCK_STREAM,0);	
//	
//	if(INVALID_SOCKET == sockfd)
//	{
//		TRACE("ServiceTCPConnected create socket error. \n");
//		return FALSE;
//	}
//	
//	localAddr.sin_family = AF_INET;
//	localAddr.sin_port = htons(PORT_TCP);
//	localAddr.sin_addr.s_addr  = inet_addr(cIP);  
//	
//	//绑定接收套接字
//	if (SOCKET_ERROR == bind(sockfd,(sockaddr*)&localAddr,sizeof(localAddr)))
//	{
//		TRACE("ServiceTCPConnected bind socket error. \n");
//		closesocket(sockfd);
//		return FALSE;
//	}	
//
//	//监听
//	listen(sockfd,5);
//	
//	TRACE("ServiceTCPConnected service Start .\n");
//	while(1)
//	{
//		SOCKET sockConnected = accept(sockfd,(SOCKADDR *) &fromAddr,&fromLen);
//	
//		ret = recv(sockfd, cRecvBuf, sizeof(cRecvBuf), 0);
//
//		if(ret > 0 )
//		{
//			//create child thread
//		}
//
//	}
//	//TRACE("ServiceCountdown %d Stop .\n",index+1);
//	closesocket(sockfd);
//	return TRUE;
//}



//虚拟信号机需要实现两个线程: 1. 连接、断开、流量查询	2. 倒计时接口
DWORD WINAPI VirtualTSC(LPVOID lpParam)
{
	VirtualTSCParams *pData = (VirtualTSCParams *)lpParam;

	HANDLE handleBasic = 0;
	HANDLE handleCountDown = 0;
	
	handleBasic = CreateThread(NULL, 0,ServiceBasic,pData, 0, NULL);
	handleCountDown = CreateThread(NULL, 0, ServiceCountdown,pData, 0, NULL);

	if(handleBasic != FALSE)
	{
		WaitForSingleObject(handleBasic,INFINITE);
		CloseHandle(handleBasic);
		TRACE("thread 1 exit\n");
	}
	if(handleCountDown != FALSE)
	{
		WaitForSingleObject(handleBasic,INFINITE);
		CloseHandle(handleCountDown);
		TRACE("thread 2 exit\n");
	}

	return TRUE;
}


void WaitAllMachineExit()
{
	int i = 0;
	for(i = 0; i < nMachineNum; i++)
	{
		WaitForSingleObject(nAllMachineArray[i],INFINITE);
		CloseHandle(nAllMachineArray[i]);
		
		TRACE("Machine %d exit\n",i+1);		
	}
}




//创建虚拟机线程
DWORD WINAPI CreateVirtualTSC(LPVOID lpParam)
{
	int i = 0;
	VirtualTSCParams para[MAX_NUM_MACHINE];
	HANDLE handleTemp;
	
	memset(para,0,sizeof(para));
	for(i = 0; i < nMachineNum; i++)
	{
		para[i].index = i;
		para[i].port = nPortNo+i;

		handleTemp = CreateThread(NULL, 0,VirtualTSC,&(para[i]), 0, NULL);

		if(handleTemp != FALSE)
		{
			nAllMachineArray[i] = handleTemp;
		}
	}
	//CreateThread(NULL, 0, ServiceTestCountdown,NULL, 0, NULL);
	WaitAllMachineExit();//必须等待所有虚拟机都退出后，本线程才能退出。

	return TRUE;
}

int StopAllMachine()
{
	int i = 0;
	SOCKET sockfd = 0;
	sockaddr_in localAddr ;
	int ret = 0;
	sockfd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);	
	if(INVALID_SOCKET == sockfd)
	{
		TRACE("StopAllMachine create socket error. \n");
		return FALSE;
	}
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	
	for(i = 0; i < nMachineNum; i++)
	{
		localAddr.sin_port = htons(nPortNo+i);
		sendto(sockfd, (char *)cStopArray, sizeof(cStopArray), 0, (struct sockaddr *)&localAddr, sizeof(localAddr));

		localAddr.sin_port = htons(nPortNo+19839+i);
		sendto(sockfd, (char *)cStopArray, sizeof(cStopArray), 0, (struct sockaddr *)&localAddr, sizeof(localAddr));
	}

	return TRUE;
}


//启动或者停止虚拟信号机
void ChikTSCDlg::DoService()
{
	if(nIsMachineRunning == 1)
	{
		CreateThread(NULL, 0,CreateVirtualTSC,NULL, 0, NULL);
	}
	else//停止所有虚拟机
	{
		StopAllMachine();
	}
}


//按键回调函数。
void ChikTSCDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if(GetInputValue() != 0)
	{
		return ;
	}

	DisableInputEdit();

	DoService();
}

 int ChikTSCDlg::getLocalhostAddress()
{
	int nRet;
	struct addrinfo *pResHeader, *pRes;
	WSADATA wsa={0}; 

	if (WSAStartup(MAKEWORD(2,1), &wsa))
	{ 
		return FALSE;
	}

   char hostname[256];
   nRet = gethostname(hostname, sizeof(hostname));
   if (nRet != 0) 
   {
      TRACE("Error: %u\n", WSAGetLastError());
	  return FALSE;
   }
   TRACE("hostname=%s\n", hostname);
   
	//ipv4
	nRet = getaddrinfo(hostname, 0,0,&pResHeader);
	if( (nRet == 0) && (pResHeader != NULL) )
	{
		pRes = pResHeader;
		while(pRes)
		{
			if(pRes->ai_family == AF_INET)
			{
				SOCKADDR_IN *pAddr = (SOCKADDR_IN *)pRes->ai_addr;
				TRACE("local ip address :  %s\n",inet_ntoa(pAddr->sin_addr));
				m_IP.AddString(inet_ntoa(pAddr->sin_addr));
			}

			pRes = pRes->ai_next;
		}
		freeaddrinfo(pResHeader);		
	}
	else
	{
		return FALSE;
	}

	m_IP.SetCurSel(0);//默认使用第一个IP地址作为虚拟信号机的地址
	return TRUE;
}

