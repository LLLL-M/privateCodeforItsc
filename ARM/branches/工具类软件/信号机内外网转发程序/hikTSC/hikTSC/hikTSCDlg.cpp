
// hikTSCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "hikTSC.h"
#include "hikTSCDlg.h"
#include "afxdialogex.h"
#include <ws2tcpip.h> 
#include <stdio.h> 
#include <io.h> 
#include <fcntl.h> 
#include "parse_ini.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_NUM_MACHINE	255	//��ģ�����֧�ֵ��źŻ�����
#define PORT_TCP		54321 //��������TCP 54321�˿ڣ�һ�������ӣ����½�һ���߳���������������ͨѶ

#define MAX_REQUEST_NUM	1000*10 //����ͬʱ�����źŻ���Ϣ�����������

#define MAX_UDP_BUFFER	1024*2

#define MAX_BLOCK_NUM_UDP	100 //��������161 �� 2000�˿ڵ�SDK�������ݿ��������

#define PRINT_LOG	//�Ƿ�����ӡ��־�Ĵ���


#ifndef PRINT_LOG
#define printf TRACE	
#endif

typedef struct connect_header
{
	int type:8;			//1:id, 2:161 port, 3:20000 port
	int nMachineId:8;	//���������źŻ���ID
	int nSdkId:16;		//��Ҫ�����������ĸ�SDK���͵�������Ϣ
	int nMessageId;		//��Ҫ�����������ĸ���Ϣ��Ҳ������Ϣ��ID
	int nMessageLen;	//��ʶ��������Ҫ���յ��ֽ���
	int nLinuxGetMsgTime;   //linux�»�õ�ǰ��Ϣ��ʱ�䣬����ʱ�������ɺ���
	int nLinuxSendMsgTime;  //linux�·��͵�ǰ��Ϣ��ʱ�䣬����ʱ�������ɺ���
	int nWinGetMsgTime;     //windows�»�õ�ǰ��Ϣ��ʱ�䣬����ʱ�������ɺ���
	int nWinSendMsgTime;    //windows�»�õ�ǰ��Ϣ��ʱ�䣬����ʱ�������ɺ���	
} ConnectHeader;

typedef struct TCP_INFO
{
	SOCKET socket;
	int index;

}STRU_TCP_INFO;

typedef struct
{
	int port;
	int index;
}VirtualTSCParams;//�������Ҫ����������


typedef struct
{
	int index;
	char buf[MAX_UDP_BUFFER*3];
	int len;
}STRU_TCP_BUFFER;

typedef struct
{
	int index;
	char array[MAX_UDP_BUFFER*3];
	int len;
	int type;
	int requestId;	
}STRU_UDP_BUFFER;

typedef struct
{
	sockaddr_in s161Addr[MAX_REQUEST_NUM];	//161�˿ڵ�addr
	sockaddr_in s20000Addr[MAX_REQUEST_NUM];//20000�˿ڵ�addr
	int 		i161RequestIndex;				//ά��һ������index��ÿ�����󣬸���ÿ�������addr�Ƿ��ѱ��棬��ȷ��Ӧ�÷����indexֵ
	int			i20000RequestIndex;				//up
	
}STRU_REQUEST_INFO;	//SDK������Ϣ�ṹ��

#define MY_OUT_(c)  	do{HANDLE hdlWrite = GetStdHandle(STD_OUTPUT_HANDLE);WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);WriteConsole(hdlWrite, "/n", 1, NULL, NULL);}while(0)

#define CFG_FILE_PATH		".\\config.ini"

int nMachineNum = 0;						//�û��������������������
int nPortNo = 0;							//���������ʼ�˿�
int nIsMachineRunning = 0;					//������Ƿ�ʼ����
char cIP[64] = {0};							//	�û�ѡ��ı���IP��ַ
HANDLE nAllMachineArray[MAX_NUM_MACHINE];//��ŵ�����������������߳�ID

SOCKET		g_TCPSocketArray[MAX_NUM_MACHINE] = {0};//��ŵ���TCP���ӵ�sockfd����index��������
SOCKET		g_udpSocket = 0;//����udp��socket������ͨ����socket��ָ��SDK������Ϣ
STRU_TCP_INFO		g_tcpInfo[MAX_NUM_MACHINE] = {0};	//һ���źŻ����ӵ�����������ID�š�TCP socketfd�������ڱ��أ�����������ͨѶʹ��
STRU_REQUEST_INFO	g_requestInfo[MAX_NUM_MACHINE] = {0};

STRU_UDP_BUFFER g_TcpBuffer[MAX_NUM_MACHINE][MAX_BLOCK_NUM_UDP] = {0};//���յ�TCP��Ϣ��ͳһ���õ����������������̵߳�������udp�����ϲ����

int gOnLineMachineArray[MAX_NUM_MACHINE];	//�Ѿ����ӳɹ����źŻ�ID
int hCrt =0; //�������ƿ���̨�����Ϣ��

int nDisplayId = 0;

char cLocalIpList[16][64] ;//�洢���Ǳ���IP�б�

int nIsAutoRun = 0;	//�Ƿ��Զ�ִ��

//�õ�ϵͳ�ĺ���ֵ
static unsigned int GetWindowsLocalTime()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	
    return st.wHour*60*60*1000+st.wMinute*60*1000+st.wSecond*1000+st.wMilliseconds;
}

//���÷��ͳ�ʱʱ��Ϊ3���ӡ�
void SetSocketfdTimeout(SOCKET *sock)
{
	int timeout = 3000; //3s
	int timeout_recv = 20000; //20s
	
  	setsockopt(*sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(timeout));
	
	setsockopt(*sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout_recv,sizeof(timeout_recv));
}


//type == 1, 161 port,  type == 2, 20000 port .
int FindSavedIndex(int type,sockaddr_in *addr,STRU_REQUEST_INFO *info)
{
	int i = 0;

	for(i = 0; i < ((type == 1) ? info->i161RequestIndex : info->i20000RequestIndex) ; i++)
	{
		if(((type == 1)&&(memcmp(addr,info->s161Addr,sizeof(sockaddr_in)) == 0)) || ((type == 2)&&(memcmp(addr,info->s20000Addr,sizeof(sockaddr_in)) == 0)))
		{
			//printf("");
			return i+1;
		}
	}

	return -1;
}

//type == 1, 161 port
int AddNewRequestItem(int type,sockaddr_in *addr,STRU_REQUEST_INFO *info)
{
	int ret = 0;
	
	if(type == 1)
	{
		if(info->i161RequestIndex >= MAX_REQUEST_NUM )
		{
			info->i161RequestIndex = 1;
			ret = 1;
		}
		else
		{
			ret = ++info->i161RequestIndex;
		}

		memcpy(&(info->s161Addr[ret - 1]),addr,sizeof(sockaddr_in));
	}
	else
	{
		if(info->i20000RequestIndex >= MAX_REQUEST_NUM )
		{
			info->i20000RequestIndex = 1;
			ret = 1;
		}
		else
		{
			ret = ++info->i20000RequestIndex;
		}

		memcpy(&info->s20000Addr[ret - 1],addr,sizeof(sockaddr_in));
	}

	return ret;
}


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// ChikTSCDlg �Ի���


ChikTSCDlg::ChikTSCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ChikTSCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ChikTSCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_IP, m_IP);
	DDX_Control(pDX, IDC_BUTTON_START, m_ButtonStart);
	DDX_Control(pDX, IDC_STATIC_ONLINE_LIST, m_OnlineList);
}

BEGIN_MESSAGE_MAP(ChikTSCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &ChikTSCDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()

void ChikTSCDlg::ReSize()
{
	float fsp[2];
	POINT Newp; //��ȡ���ڶԻ���Ĵ�С
	CRect recta;    
	GetClientRect(&recta);     //ȡ�ͻ�����С  
	Newp.x=recta.right-recta.left;
	Newp.y=recta.bottom-recta.top;
	fsp[0]=(float)Newp.x/old.x;
	fsp[1]=(float)Newp.y/old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint,TLPoint; //���Ͻ�
	CPoint OldBRPoint,BRPoint; //���½�
	HWND  hwndChild=::GetWindow(m_hWnd,GW_CHILD);  //�г����пؼ�  
	while(hwndChild)    
	{    
		woc=::GetDlgCtrlID(hwndChild);//ȡ��ID
		GetDlgItem(woc)->GetWindowRect(Rect);  
		ScreenToClient(Rect);  
		OldTLPoint = Rect.TopLeft();  
		TLPoint.x = long(OldTLPoint.x*fsp[0]);  
		TLPoint.y = long(OldTLPoint.y*fsp[1]);  
		OldBRPoint = Rect.BottomRight();  
		BRPoint.x = long(OldBRPoint.x *fsp[0]);  
		BRPoint.y = long(OldBRPoint.y *fsp[1]);  
		Rect.SetRect(TLPoint,BRPoint);  
		GetDlgItem(woc)->MoveWindow(Rect,TRUE);
		hwndChild=::GetWindow(hwndChild, GW_HWNDNEXT);    
	}
	old=Newp;
}

void ChikTSCDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType==SIZE_RESTORED||nType==SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void TestFile()
{
	//��D��д��������ݿ�����ini�ļ���Ϣ��Ĭ����������
	CFileFind finder;   //�����Ƿ����ini�ļ����������ڣ�������һ���µ�Ĭ�����õ�ini�ļ��������ͱ�֤�����Ǹ��ĺ������ÿ�ζ�����
	
	BOOL ifFind = finder.FindFile(".\\RoadDataManagerApp.ini");
	if( !ifFind )
	{
		::WritePrivateProfileStringW(L"Database connection Info",L"IP",L"10.210.0.9",L".\\RoadDataManagerApp.ini");
	}

}

//�������ļ��ж�ȡ������Ϣ������ҳ���Ͻ���չʾ
void ReadCfgFile()
{
	char buf[64] = "0";
	
	::GetPrivateProfileString("TransferTool","IP","No IP",cIP,sizeof(cIP),CFG_FILE_PATH);

	::GetPrivateProfileString("TransferTool","MachineNum","No MachineNum",buf,10,CFG_FILE_PATH);

	nMachineNum = atoi(buf);
	
	::GetPrivateProfileString("TransferTool","PortNO","No PortNO",buf,10,CFG_FILE_PATH);

	nPortNo = atoi(buf);

	::GetPrivateProfileString("TransferTool","DisplayID","No DisplayID",buf,10,CFG_FILE_PATH);

	nDisplayId = atoi(buf);

	::GetPrivateProfileString("TransferTool","IsAutoRun","No IsAutoRun",buf,10,CFG_FILE_PATH);

	nIsAutoRun = atoi(buf);

	printf("ReadCfgFile %s, %d, %d, %d\n",cIP,nMachineNum,nPortNo,nDisplayId);
}

void WriteCfgFile()
{
	char  buf[128] = "0";
	
	::WritePrivateProfileString("TransferTool","IP",cIP,".\\config.ini"); 

	sprintf(buf,"%d",nMachineNum);
	::WritePrivateProfileString("TransferTool","MachineNum",buf,CFG_FILE_PATH);

	sprintf(buf,"%d",nPortNo);
	::WritePrivateProfileString("TransferTool","PortNO",buf,CFG_FILE_PATH);

	sprintf(buf,"%d",nDisplayId);
	::WritePrivateProfileString("TransferTool","DisplayID",buf,CFG_FILE_PATH);

	sprintf(buf,"%d",nIsAutoRun);
	::WritePrivateProfileString("TransferTool","IsAutoRun",buf,CFG_FILE_PATH);

	printf("WriteCfgFile %s, %d, %d, %d\n",cIP,nMachineNum,nPortNo,nDisplayId);
}

int GetLocalIpIndex(char *ip)
{
	int i = 0;

	for(i = 0; i < 15; i++)
	{
		if(strcmp(ip,cLocalIpList[i]) == 0)
		{
			return i;
		}
	}

	return 0;
}



// ChikTSCDlg ��Ϣ�������

BOOL ChikTSCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//ShowWindow(SW_MINIMIZE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	nIsMachineRunning = 0;
	getLocalhostAddress();

	ReadCfgFile();

	CString str;

	str.Format("%s",cIP);
	if(strlen(cIP) != 0)
	{
		m_IP.SetCurSel(GetLocalIpIndex(cIP));
	}
	str.Format("%d",nMachineNum);
	GetDlgItem(IDC_EDIT_TSC_NUM)->SetWindowText(nMachineNum == 0 ? _T("20") : str);
	str.Format("%d",nPortNo);
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(nPortNo == 0 ? _T("9001") : str);
	str.Format("%d",nDisplayId);
	GetDlgItem(IDC_EDIT_DISPLAY_ID)->SetWindowText(nDisplayId == 0 ? _T("1") : str);
#ifdef PRINT_LOG
    if ( AllocConsole() ) 
    { 
        hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT); 
        *stdout = *(::_fdopen(hCrt, "w")); 
        ::setvbuf(stdout, NULL, _IONBF, 0); 
        *stderr = *(::_fdopen(hCrt, "w")); 
        ::setvbuf(stderr, NULL, _IONBF, 0); 
    } 	
#endif	

	CRect rect;    
	GetClientRect(&rect);     //ȡ�ͻ�����С  
	old.x=rect.right-rect.left;
	old.y=rect.bottom-rect.top;

	//TestFile();
	if(nIsAutoRun == 1)
		OnBnClickedButtonStart();
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void ChikTSCDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR ChikTSCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//��ȡ�û���������ֲ�У�飬ֻ��У����ȷ��ſ���ģ�����
int ChikTSCDlg::GetInputValue()
{
	CString str;
	GetDlgItem(IDC_EDIT_TSC_NUM)->GetWindowText(str);

	nMachineNum = _ttoi(str);
	if((nMachineNum < 1) || (nMachineNum > MAX_NUM_MACHINE))
	{
		MessageBox(_T("�źŻ�̨����ʱֻ֧��1-255."));
		return 1;
	}
	
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(str);

	nPortNo  = _ttoi(str);
	if((nPortNo < 0) || (nPortNo > 65536))
	{
		MessageBox(_T("�˿ں�������0-65536֮�������."));
		return 2;
	}
	
	GetDlgItem(IDC_COMBO_IP)->GetWindowText(str);
	strcpy_s (cIP,str.GetBuffer());
	printf("�趨��IP��ַ�� %s, ����������� %d, ��ʼ�˿��� %d\n",cIP,nMachineNum,nPortNo);

	GetDlgItem(IDC_EDIT_DISPLAY_ID)->GetWindowText(str);
	nDisplayId  = _ttoi(str);

	return 0;
}

//����ģ������Ҫ���������
void ChikTSCDlg::DisableInputEdit()
{
	if(nIsMachineRunning == 0)
	{
		nIsMachineRunning = 1;
		//disable input
		(GetDlgItem(IDC_EDIT_TSC_NUM))->EnableWindow(FALSE);
		(GetDlgItem(IDC_EDIT_PORT))->EnableWindow(FALSE);
		(GetDlgItem(IDC_COMBO_IP))->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_START)->SetWindowText(_T("ֹͣ����"));
		printf("<��������>\n");
		WriteCfgFile();
		DoService();
	}
	else
	{
		if(AfxMessageBox("ȷ���˳�������? \n\n�˳����źŻ���ʧȥ���ӣ�ͨѶ�жϡ�",MB_YESNO) == IDYES)
		{
			exit(0);
		}
		else
		{
			return;
		}	
		nIsMachineRunning = 0;
		(GetDlgItem(IDC_EDIT_TSC_NUM))->EnableWindow(TRUE);
		(GetDlgItem(IDC_EDIT_PORT))->EnableWindow(TRUE);
		(GetDlgItem(IDC_COMBO_IP))->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_START)->SetWindowText(_T("��������"));
		printf("<ֹͣ����>\n");
	}	
}


DWORD WINAPI ThreadSendTcpData(LPVOID lpParam)
{
	int ret = 0;
	STRU_TCP_BUFFER *buf = (STRU_TCP_BUFFER *)lpParam;
	int index = buf->index;
	int len = buf->len;
	//int nOldTime = 0;
	
	if((index < MAX_NUM_MACHINE ) && (index >= 0))
	{
		//nOldTime = GetWindowsLocalTime();
		ret = send(g_TCPSocketArray[index],buf->buf,len,0);

		//printf("Send Tcp Data Cost %d ms \n",GetWindowsLocalTime()-nOldTime);
		//printf("SendTcpData  ret %d, len  %d , socket %d \n",ret,len,g_TCPSocketArray[index]);
		if(ret == SOCKET_ERROR)
		{
			printf("ThreadSendTcpData index  %d send data failed .  %d  ,socket id  %d \n",index,GetLastError(),g_TCPSocketArray[index]);
		}
	}

	//printf("SendTcpData ,  %d, ret %d\n",index,ret);

	return 0;
}

void CreateGlobalSocketfd()
{
	g_udpSocket = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);	

	if(INVALID_SOCKET == g_udpSocket)
	{
		printf("CreateGlobalSocketfd create socket error. \n");
	}	

}


//��������ӡ��Ͽ���������ѯ�Ȼ�������
DWORD WINAPI ServiceBasic(LPVOID  lpParam)
{
	VirtualTSCParams *pData = (VirtualTSCParams *)lpParam;
	//printf("ServiceBasic %d, %d, %p\n",pData->port,pData->index,pData);
	SOCKET sockfd = 0;
	char  cRecvBuf[MAX_UDP_BUFFER] = {0};
	sockaddr_in localAddr ;
	sockaddr_in fromAddr ;
	int ret = 0;
	int fromLen= sizeof(fromAddr);
	int port = pData->port;
	int index = pData->index;
	unsigned char cTemp1= 0;
	unsigned char cTemp2= 0;
	sockfd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);	
	if(INVALID_SOCKET == sockfd)
	{
		printf("ServiceBasic create socket error. \n");
		return FALSE;
	}
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	//localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	localAddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//ip��ַ
	
	//�󶨽����׽���
	if (SOCKET_ERROR == bind(sockfd,(sockaddr*)&localAddr,sizeof(localAddr)))
	{
		printf("ChikTSCDlg.ServiceBasic bind socket error. errno %d , port %d\n",GetLastError(),port);
		
		closesocket(sockfd);
		return FALSE;
	}	
	printf("ServiceBasic %d Start , thread id is : %d.\n",index+1,GetCurrentThreadId());
	ConnectHeader header ;
	int nBlockSize = (sizeof(cRecvBuf)+sizeof(header));
	int nRequestId = 0;
	int nBufIndex = 0;
	STRU_TCP_BUFFER buff[MAX_BLOCK_NUM_UDP];
	SYSTEMTIME sys; 
	unsigned int nTestIndex[MAX_NUM_MACHINE] = {0};		
	SetSocketfdTimeout(&sockfd);
	while(1)
	{
		ret = recvfrom(sockfd, cRecvBuf, sizeof(cRecvBuf), 0, (struct sockaddr *)&fromAddr, &fromLen);
		//printf("ServiceBasic  ret  %d\n",ret);
		if(ret <= 0)
		{	
			printf("ServiceBasic recv error . errno %d\n",GetLastError());
			Sleep(1000);
			continue;
		}
		if(g_tcpInfo[index].socket == 0)
			continue;
		if((nRequestId = FindSavedIndex(1,&fromAddr,&g_requestInfo[index])) == -1)
		{
		
			nRequestId = AddNewRequestItem(1,&fromAddr,&g_requestInfo[index]);
		}
		GetLocalTime( &sys ); 
		
		nTestIndex[index]++;
		header.type = 2;
		header.nMachineId = index+1;
		header.nSdkId = nRequestId;
		header.nMessageId = nTestIndex[index];
		header.nMessageLen = ret;
		header.nWinGetMsgTime = GetWindowsLocalTime();
		//printf("send data \n");
		if(nBufIndex == (MAX_BLOCK_NUM_UDP - 2))
		{
			nBufIndex = 0;
		}
		memcpy(buff[nBufIndex].buf,&header,sizeof(header));
		memcpy(buff[nBufIndex].buf+sizeof(header),cRecvBuf,ret);
		buff[nBufIndex].index = index;
		buff[nBufIndex].len = sizeof(header)+ret;

		if(index == (nDisplayId - 1))
		{
			//printf("UDP Recv ,Machine Index %d, Request Index %d,  %02d:%02d:%02d.%03d\n",index+1,nTestIndex[index],sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds);
			//printf("UDP Recv ,port %d machine %d, Index %d,  %02d:%02d:%02d.%03d size  %d \n",161+index,index+1,nTestIndex[index],sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,header.len);

		}

		//printf("====> ServiceBasic  ,recv size %d , request id  %d\n",ret,nRequestId);
		HANDLE hThread = CreateThread(NULL, 0,ThreadSendTcpData,&(buff[nBufIndex]), 0, NULL);
		CloseHandle(hThread);
		nBufIndex++;
		
	}
	printf("ServiceBasic %d Stop .\n",index+1);
	closesocket(sockfd);
	return TRUE;
}


//����ʱ��Ӧ����
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
		printf("ChikTSCDlg.ServiceCountdown create socket error. \n");
		return FALSE;
	}
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	//localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	localAddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//ip��ַ
	
	//�󶨽����׽���
	if (SOCKET_ERROR == bind(sockfd,(sockaddr*)&localAddr,sizeof(localAddr)))
	{
		printf("ChikTSCDlg.ServiceCountdown bind socket error. \n");
		closesocket(sockfd);
		return FALSE;
	}	
	ConnectHeader header ;
	int nBlockSize = (sizeof(cRecvBuf)+sizeof(header));
	int nRequestId = 0;
	int nBufIndex = 0;
	STRU_TCP_BUFFER buff[MAX_BLOCK_NUM_UDP];
	printf("ServiceCountdown %d Start thread id is :  %d.\n",index+1,GetCurrentThreadId());
	SYSTEMTIME sys; 
	unsigned int nTestIndex[MAX_NUM_MACHINE] = {0};			
	SetSocketfdTimeout(&sockfd);
	
	while(1)
	{
		ret = recvfrom(sockfd, cRecvBuf, sizeof(cRecvBuf), 0, (struct sockaddr *)&fromAddr, &fromLen);
		//printf("ServiceCountdown \n");
		if(ret <= 0)
		{	
			Sleep(1000);
			continue;
		}
		if(g_tcpInfo[index].socket == 0)
			continue;
		if((nRequestId = FindSavedIndex(2,&fromAddr,&g_requestInfo[index])) == -1)
		{
			nRequestId = AddNewRequestItem(2,&fromAddr,&g_requestInfo[index ]);
		}		

		nTestIndex[index]++;

		header.type = 3;
		header.nMachineId = index+1;
		header.nSdkId = nRequestId;
		header.nMessageLen = ret;
		header.nMessageId = nTestIndex[index];
		
		if(nBufIndex == (MAX_BLOCK_NUM_UDP - 2))
		{
			nBufIndex = 0;
		}
		memcpy(buff[nBufIndex].buf,&header,sizeof(header));
		memcpy(buff[nBufIndex].buf+sizeof(header),cRecvBuf,ret);
		buff[nBufIndex].index = index;
		buff[nBufIndex].len = sizeof(header)+ret;

		if(/*index == 1*/0)
		{
			GetLocalTime( &sys ); 
			printf("UDP Recv ,20000 Index %d,  %02d:%02d:%02d.%03d size  %d \n"
			,nTestIndex[index],sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,header.nMessageLen);
		}
		HANDLE hThread = CreateThread(NULL, 0,ThreadSendTcpData,&(buff[nBufIndex]), 0, NULL);
		CloseHandle(hThread);		
		nBufIndex++;	
	}
	printf("ServiceCountdown %d Stop .\n",index+1);
	closesocket(sockfd);
	return TRUE;
}


DWORD WINAPI ThreadSendUdpData(LPVOID lpParam)
{
	int ret = 0;
	STRU_UDP_BUFFER *buf = (STRU_UDP_BUFFER *)lpParam;

	//int nOldTime = GetWindowsLocalTime();
	if(buf->type == 0)//161
	{
		ret= sendto(g_udpSocket, (char *)buf->array, buf->len, 0, (struct sockaddr*)&g_requestInfo[buf->index- 1].s161Addr[buf->requestId - 1], sizeof(sockaddr_in));

	}else if(buf->type == 1)//20000
	{
		ret = sendto(g_udpSocket, (char *)buf->array, buf->len, 0, (struct sockaddr*)&g_requestInfo[buf->index - 1].s20000Addr[buf->requestId - 1], sizeof(sockaddr_in));
	}
	//printf("Send Udp Data Cost %d ms\n",GetWindowsLocalTime()-nOldTime);
	//printf(" SendUdpData  send size  %d , port : %d , request id : %d\n",ret,(type == 0 ) ? 161 : 20000,requestId);
	return 0;
}



//���̣߳������� �źŻ�����TCPͨѶ��
DWORD WINAPI ServiceChildTCPConnected(LPVOID lpParam)
{
	STRU_TCP_INFO *tcp = (STRU_TCP_INFO *)lpParam;
	SOCKET pSocket = tcp->socket;
	int index = tcp->index;
	int ret  = 0;
	unsigned long ul=1;  
	ConnectHeader sHeader;
	char buf[MAX_UDP_BUFFER];
	int err = 0;
	int nBufIndex = 0;
	//ret=ioctlsocket(pSocket,FIONBIO,(unsigned long *)&ul);//���óɷ�����ģʽ��  

	//while(ret==SOCKET_ERROR){
		
		//ioctlsocket(pSocket,FIONBIO,(unsigned long *)&ul);//���óɷ�����ģʽ��  
	//}

	SYSTEMTIME sys; 
	GetLocalTime( &sys ); 
	//printf("%02d:%02d:%02d.%03d   ServiceChildTCPConnected index %d , thread id is  %d \n",sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,index,GetCurrentThreadId());
	
	unsigned int nTestIndex = 0;

	gOnLineMachineArray[index - 1] = 1;//������ӳɹ�������ID-1��Ϊ��������Ӧ����ֵ��1

	while(1)
	{
		ret = recv(pSocket,(char *)&sHeader,sizeof(sHeader),0);	

		//printf("ServiceChildTCPConnected\n");
		if(ret <= 0){
			GetLocalTime( &sys ); 
			//printf("%02d:%02d:%02d.%03d  ServiceChildTCPConnected index  %d exit , error id is :  %d \n",sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,index,GetLastError());
			g_TCPSocketArray[index - 1] = 0;
			g_requestInfo[index - 1].i161RequestIndex = 0;
			g_requestInfo[index - 1].i20000RequestIndex = 0;
			g_tcpInfo[index - 1].socket = 0;
			gOnLineMachineArray[index - 1] = 0;
			closesocket(pSocket);
			break;	
		}  
		else{//deal ...

			ret = recv(pSocket,(char *)buf,sHeader.nMessageLen,0);	
			
			if(ret == SOCKET_ERROR){
				printf("ServiceChildTCPConnected  , recv data failed :  %d  , datlen  %d , ret  %d\n",GetLastError(),sHeader.nMessageLen,ret);
				Sleep(1000);
				continue;
			}

			if(nBufIndex == MAX_BLOCK_NUM_UDP)
			{
				nBufIndex = 0;
			}
			memcpy(g_TcpBuffer[index - 1][nBufIndex].array,buf,ret);
			g_TcpBuffer[index - 1][nBufIndex].index = index;
			g_TcpBuffer[index - 1][nBufIndex].len = ret;
			g_TcpBuffer[index - 1][nBufIndex].type = ((sHeader.type == 2) ? 0 : 1);
			g_TcpBuffer[index - 1][nBufIndex].requestId = sHeader.nSdkId;
			
			//printf("ServiceChildTCPConnected recv type  %d , recv size  %d\n",sHeader.type,ret);
			HANDLE hThread = CreateThread(NULL, 0,ThreadSendUdpData,&(g_TcpBuffer[index - 1][nBufIndex]), 0, NULL);
			CloseHandle(hThread);
			nBufIndex++;
			
			if((sHeader.type == 2)&&(index == nDisplayId))
			{
				sHeader.nWinSendMsgTime = GetWindowsLocalTime();
				if(sHeader.nWinSendMsgTime-sHeader.nWinGetMsgTime >= 1000)
					printf("\n >>>>>>>>>>>>>>>>>=<<<<<<<<<<<<<<<<<<<<<< \n");
				printf("Msg Id %d, Total cost %d ms, Board cost %d ms \n",
					sHeader.nMessageId,
					sHeader.nWinSendMsgTime-sHeader.nWinGetMsgTime,
					sHeader.nLinuxSendMsgTime-sHeader.nLinuxGetMsgTime);
				//printf("TCP Recv ,Machine Index %d, Request Index %d,  %02d:%02d:%02d.%03d \n",index,sHeader.nMessageId,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds);
				//printf("TCP Recv ,%d Index %d,  %02d:%02d:%02d.%03d  ,head size  %d , real size %d\n",(sHeader.type == 3) ? 20000:161,
					//sHeader.index,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,sHeader.len,ret);
			}
				
		}

	}

	return 0;
}


//��Ҫ��������TCP�˿ڣ��ȴ��źŻ����ӡ�
DWORD ChikTSCDlg::ServiceTCPConnected(LPVOID lpParam)
{
	ChikTSCDlg *p = (ChikTSCDlg*)lpParam;
	SOCKET sockfd = 0;
	ConnectHeader sHeader;
	
	sockaddr_in localAddr ;
	sockaddr_in fromAddr ;
	int ret = 0;
	int fromLen = sizeof(fromAddr);
	sockfd = socket(AF_INET, SOCK_STREAM,0);	
	
	if(INVALID_SOCKET == sockfd)
	{
		printf("ServiceTCPConnected create socket error. \n");
		return FALSE;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(PORT_TCP);
	localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	
	//�󶨽����׽���
	if (SOCKET_ERROR == bind(sockfd,(sockaddr*)&localAddr,sizeof(localAddr)))
	{
		printf("ServiceTCPConnected bind socket error. \n");
		closesocket(sockfd);
		return FALSE;
	}	
	
	//����
	listen(sockfd,5);
	
	printf("ServiceTCPConnected service Start ,thread id is  %d\n",GetCurrentThreadId());
	//SYSTEMTIME sys; 
	//GetLocalTime( &sys ); 
	//printf( "==>  %4d/%02d/%02d %02d:%02d:%02d.%03d ����%1d\n",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,sys.wDayOfWeek);
	SOCKET sockConnected = 0;
	while(1)
	{
		sockConnected = accept(sockfd,(SOCKADDR *) &fromAddr,&fromLen);
		
		ret = recv(sockConnected, (char *)&sHeader, sizeof(sHeader), 0);

		if(ret > 0 )
		{
			printf("ServiceTCPConnected  recv  type %d  , id  %d , ip : %s\n",sHeader.type,sHeader.nMachineId,inet_ntoa(fromAddr.sin_addr ));
			
			if(sHeader.type == 1)//�������½�����
			{
				//AfxMessageBox(str);
				if((sHeader.nMachineId >= 1 )&& (sHeader.nMachineId < MAX_NUM_MACHINE))
				{
					//if(g_tcpInfo[sHeader.id - 1].socket != 0)//�����Ѿ����豸�����ϣ�������е����ӶϿ�����֤ת������ͬʱֻ�ܷ���һ��ID��
					//{
					//	closesocket(g_tcpInfo[sHeader.id - 1].socket);
					//}
					SetSocketfdTimeout(&sockConnected);
					g_tcpInfo[sHeader.nMachineId - 1].index = sHeader.nMachineId;
					g_tcpInfo[sHeader.nMachineId - 1].socket = sockConnected;
					
					//create child thread
					HANDLE hThread = CreateThread(NULL, 0,ServiceChildTCPConnected,(void *)&g_tcpInfo[sHeader.nMachineId -1], 0, NULL);
					CloseHandle(hThread);
					
					//update global array
					g_TCPSocketArray[sHeader.nMachineId - 1] = sockConnected;

					//Sleep(10);
					//closesocket(sockConnected);
					//printf("create socket  %d \n",sockConnected);
				}
			}
		}
		else
		{
			printf("ServiceTCPConnected ret error , %d \n",WSAGetLastError());
			closesocket(sockConnected);
		}

	}
	//printf("ServiceCountdown %d Stop .\n",index+1);
	closesocket(sockfd);
	return TRUE;
}



//�����źŻ���Ҫʵ�������߳�: 1. ���ӡ��Ͽ���������ѯ	2. ����ʱ�ӿ�
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
		printf("thread 1 exit\n");
	}
	if(handleCountDown != FALSE)
	{
		WaitForSingleObject(handleBasic,INFINITE);
		CloseHandle(handleCountDown);
		printf("thread 2 exit\n");
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
		
		printf("Machine %d exit\n",i+1);		
	}
}




//����������߳�
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
	
	WaitAllMachineExit();//����ȴ�������������˳��󣬱��̲߳����˳���

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
		printf("StopAllMachine create socket error. \n");
		return FALSE;
	}
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	
	return TRUE;
}

DWORD ChikTSCDlg::SetOnlineMachine(LPVOID lpParam)
{
	ChikTSCDlg *p = (ChikTSCDlg*)lpParam;
	char cArray[MAX_NUM_MACHINE*3] = {0};
	int i ;
	CString str ;

	while(1)
	{
		for(i = 0; i < MAX_NUM_MACHINE; i++)
		{
			if(gOnLineMachineArray[i] == 1)
			{
				sprintf_s(cArray+strlen(cArray),sizeof(cArray),"%d ",i+1);

			}
		}
		str = cArray;
		p->m_OnlineList.SetWindowText(str);  

		memset(cArray,0,sizeof(cArray));

		Sleep(1000);
	}

	return TRUE;
}



//��������ֹͣ�����źŻ�
void ChikTSCDlg::DoService()
{
	
	if(nIsMachineRunning == 1)
	{
		CreateGlobalSocketfd();
		CreateThread(NULL, 0,CreateVirtualTSC,NULL, 0, NULL);
		::CreateThread(NULL, 0,ServiceTCPConnected,this, 0, NULL);
		CreateThread(NULL, 0,SetOnlineMachine,this, 0, NULL);
	}
	else//ֹͣ���������
	{
		if(AfxMessageBox("ddd",MB_YESNO) == IDYES)
		{
			exit(0);
			StopAllMachine();
		}
		
		//close(g_udpSocket);
	}
}


//�����ص�������
void ChikTSCDlg::OnBnClickedButtonStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(GetInputValue() != 0)
	{
		return ;
	}
	//printf("just \n");
	//MY_OUT_("just try");
	DisableInputEdit();
}

 int ChikTSCDlg::getLocalhostAddress()
{
	int nRet;
	int i = 0;
	
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
      printf("Error: %u\n", WSAGetLastError());
	  return FALSE;
   }
   printf("hostname=%s\n", hostname);
   memset(cLocalIpList,0,sizeof(cLocalIpList));
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
				//printf("local ip address :  %s\n",inet_ntoa(pAddr->sin_addr));
				//m_IP.AddString(inet_ntoa(pAddr->sin_addr));
				m_IP.InsertString(i,inet_ntoa(pAddr->sin_addr));
				if(i < 15)
				{
					strcpy(cLocalIpList[i],inet_ntoa(pAddr->sin_addr));
					TRACE("i %d, %s\n",i,cLocalIpList[i]);
				}
				i++;
			}

			pRes = pRes->ai_next;
		}
		freeaddrinfo(pResHeader);		
	}
	else
	{
		return FALSE;
	}

	m_IP.SetCurSel(0);//Ĭ��ʹ�õ�һ��IP��ַ��Ϊ�����źŻ��ĵ�ַ
	return TRUE;
}

