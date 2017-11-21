

// hikTSCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "hikTSC.h"
#include "hikTSCDlg.h"
#include "afxdialogex.h"
#include <ws2tcpip.h> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_NUM_MACHINE	255	//��ģ�����֧�ֵ��źŻ�����
#define PORT_TCP		54321 //��������TCP 54321�˿ڣ�һ�������ӣ����½�һ���߳���������������ͨѶ

SOCKET	g_SocketArray[255] = {0};


struct STRU_N_DscTypeLogin
{
	WORD	wProtocol;		//�źŻ�Э�����ͣ�1-NTCIP��2-HIK��3-GB
	WORD	wDscType;		//�źŻ��ͺţ�0:500�ͣ�1:300-44�ͣ�2:300-22��
	UINT	unPort;			//ͨѶ�˿�
};

struct STRU_Extra_DscTypeLogin
{
	unsigned int unExtraParamHead;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	unsigned int unExtraParamID;					//��Ϣ����

	STRU_N_DscTypeLogin dscTypeLogin;		//����ռ���ʵȽṹ��	
};


typedef struct UDP_INFO
{
	int iHead;                      //��Ϣͷ��Ĭ��Ϊ0x6e6e
	int iType;						//��Ϣ���ͣ�0x94 - ���ؼ����� | 0x88 - ������� | 0x15b - ���ع�����Ϣ | 0x97 - ���غ�Ƶ��� | 0x93 - ���ؼ����� 
	int iValue[16*64];						//
}STRU_UDP_INFO;


//����ʱ�ӿڻ�ȡ������
typedef struct STRU_Extra_Param_Phase_Counting_Down_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0x9E��ʾ����
     unsigned char    stVehPhaseCountingDown[16][2];    //16����������λ�ĵ�ǰ��ɫ������ʱ
     							//��0�б�ʾ��ǰ��������λ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
     							//��1�б�ʾ��ǰ��������λ������λ����ʱʱ��
     unsigned char    stPedPhaseCountingDown[16][2];    //16��������λ�ĵ�ǰ��ɫ������ʱ
     							//��0�б�ʾ��ǰ��������λ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
								//��1�б�ʾ��ǰ������λ������λ����ʱʱ��
     unsigned char    ucPlanNo;				//��ǰ���з�����
     unsigned char    ucCurCycleTime;			//��ǰ�������ڳ�
     unsigned char    ucCurRunningTime;		//��ǰ����ʱ��
     unsigned char    ucChannelLockStatus;		//ͨ���Ƿ�����״̬��1��ʾͨ��������0��ʾͨ��δ����
     unsigned char    ucCurPlanDsc[16];			//��ǰ���з�������
     unsigned char    ucOverlap[16][2];                    //������λ״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
     unsigned char    stPhaseRunningInfo[16][2];	//����λ����ʱ������ű�
							//��0�б�ʾ����λ���űȣ���1�б�ʾ����λ����ʱ�䣬�̵�������1�в�����ֵ������Ϊ0
     unsigned char    ucChannelStatus[32]; //32��ͨ��״̬��7:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������ ,0:����Ч�����Ը�ͨ�����ƣ� 
     unsigned char    ucWorkingTimeFlag; //ʱ��ο�����Ч��־��1��ʾ����ʱ��ο�����Ч��0��ʾȫ��ʱ������Ч������ʱ�������Ч
     unsigned char    ucBeginTimeHour; //������Чʱ�䣺Сʱ
     unsigned char    ucBeginTimeMin; //������Чʱ�䣺����
     unsigned char    ucBeginTimeSec; //������Чʱ�䣺��
     unsigned char    ucEndTimeHour; //���ƽ���ʱ�䣺Сʱ
     unsigned char    ucEndTimeMin; //���ƽ���ʱ�䣺����
     unsigned char    ucEndTimeSec; //���ƽ���ʱ�䣺��
     unsigned char    ucReserved[9]; //Ԥ��
}PHASE_COUNTING_DOWN_FEEDBACK_PARAMS;    

typedef struct
{
	int port;
	int index;
}VirtualTSCParams;//�������Ҫ����������

typedef struct
{
	int type;
	int id;
	int val;
	
}DownloadDataParams;//�������Ӧ�ϴ����ݽṹ��



//������ѯ�ӿڣ�ÿ�η�������
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

unsigned char cStopArray[] = {0xFF,0xFE,0xFF,0xFE};//���յ�����Ϣ�󣬾��˳���ǰ�����̡߳�



int nMachineNum;						//�û��������������������
int nPortNo;							//���������ʼ�˿�
int nIsMachineRunning;					//������Ƿ�ʼ����
char cIP[64];							//	�û�ѡ��ı���IP��ַ
HANDLE nAllMachineArray[MAX_NUM_MACHINE];//��ŵ�����������������߳�ID

PHASE_COUNTING_DOWN_FEEDBACK_PARAMS g_CountdownValue[MAX_NUM_MACHINE];//Ĭ������·��͵ĵ���ʱ��Ϣ
DownloadDataParams					g_downloadParams;	//���20000�˿��յ�0x6e6e�����ݣ��������ID�󣬷��ظýṹ�塣


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
}

BEGIN_MESSAGE_MAP(ChikTSCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &ChikTSCDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()


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
	GetDlgItem(IDC_EDIT_TSC_NUM)->SetWindowText(_T("1"));
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(_T("161"));
	SetDefaultCountDownValue();
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
	TRACE("�趨��IP��ַ�� %s, ����������� %d, ��ʼ�˿��� %d\n",cIP,nMachineNum,nPortNo);
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
		GetDlgItem(IDC_BUTTON_START)->SetWindowText(_T("ֹͣģ�����"));
		TRACE("<����ģ�����>\n");
	}
	else
	{
		nIsMachineRunning = 0;
		(GetDlgItem(IDC_EDIT_TSC_NUM))->EnableWindow(TRUE);
		(GetDlgItem(IDC_EDIT_PORT))->EnableWindow(TRUE);
		(GetDlgItem(IDC_COMBO_IP))->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_START)->SetWindowText(_T("����ģ�����"));
		TRACE("<ֹͣģ�����>\n");
	}	
}

#define PHASE_GREEN_SPLIT	30
#define PHASE_CIRCLE		120
#define YELLOW_LAMP 		3
#define ALL_RED				0
#define PHASE_NUM			4
#define GET_COLOR(val)    (((val) == 1) ? "��" : ((val == 2) ? "��" : ((val == 3) ? "��" : "")))


//Ĭ�ϵ���ʱ����
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
			g_CountdownValue[i].stVehPhaseCountingDown[j][1] = PHASE_GREEN_SPLIT*(j+1)+1;//ÿ����λ�ĵ���ʱʱ�������űȵ�N����

			g_CountdownValue[i].stPhaseRunningInfo[j][0] = PHASE_GREEN_SPLIT;
		}
		g_CountdownValue[i].stVehPhaseCountingDown[0][0] = 1;//Ĭ�ϴ���λ1��ʼ���У���ôֻ����λ1�ĳ�ʼ״̬���̵�
	}

	//�������ز�����Ĭ��ֵ
	g_downloadParams.type = 0x6e6e;
	g_downloadParams.id = 0;
	g_downloadParams.val = 0;
}

//������������λ����ʱ
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
			g_CountdownValue[index].stVehPhaseCountingDown[i][0] = 3;//�Ƶ�ʱ��
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




//��������ӡ��Ͽ���������ѯ�Ȼ�������
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
	
	//�󶨽����׽���
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
		cTemp1 = (cRecvBuf[0]&0xff);//����������ź�����
		cTemp2 = (cRecvBuf[2]&0xff);//�����������ϢID
		cTemp3 =  (cRecvBuf[7]&0xff);
		cTemp4 =  (cRecvBuf[8]&0xff);
		if(cTemp1 == 0x7e)//���ӡ��Ͽ�������Ϣ��ֱ�ӽ��յ�����Ϣ���ؼ��ɡ�
		{
			sendto(sockfd, cRecvBuf, strlen(cRecvBuf)+1, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		}
		else if(cTemp1 == 0x90)//������
		{
			cFlowArray_1[2] = cTemp2;
			sendto(sockfd, (char *)cFlowArray_1, sizeof(cFlowArray_1), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));	
		}
		//else if(cTemp1 == 0x80)//��������
		//{
		//	cFlowArray_2[2] = cTemp2;
		//	sendto(sockfd, (char *)cFlowArray_2, sizeof(cFlowArray_2), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));	
		//}
		else if(cTemp1 == 0x80 && cTemp3 == 0x08 && cTemp4 == 0x02)//ͨ������
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
		TRACE("ChikTSCDlg.ServiceCountdown create socket error. \n");
		return FALSE;
	}
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	localAddr.sin_addr.s_addr  = inet_addr(cIP);  
	
	//�󶨽����׽���
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
		//Э����Ϣ
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
		//����ʱ��Ϣ
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



////���̣߳������� �źŻ�����TCPͨѶ��
//DWORD WINAPI ServiceTCPConnected(LPVOID lpParam)
//{
//	SOCKET pSocket = *(SOCKET *)lpParam;
//	int ret  = 0;
//	unsigned long ul=1;  
//	unsigned char buf[1024] = {0};
//	int err = 0;
//
//	ret=ioctlsocket(pSocket,FIONBIO,(unsigned long *)&ul);//���óɷ�����ģʽ��  
//
//	while(ret==SOCKET_ERROR){
//		
//		ioctlsocket(pSocket,FIONBIO,(unsigned long *)&ul);//���óɷ�����ģʽ��  
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



////��Ҫ��������TCP�˿ڣ��ȴ��źŻ����ӡ�
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
//	//�󶨽����׽���
//	if (SOCKET_ERROR == bind(sockfd,(sockaddr*)&localAddr,sizeof(localAddr)))
//	{
//		TRACE("ServiceTCPConnected bind socket error. \n");
//		closesocket(sockfd);
//		return FALSE;
//	}	
//
//	//����
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
	//CreateThread(NULL, 0, ServiceTestCountdown,NULL, 0, NULL);
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


//��������ֹͣ�����źŻ�
void ChikTSCDlg::DoService()
{
	if(nIsMachineRunning == 1)
	{
		CreateThread(NULL, 0,CreateVirtualTSC,NULL, 0, NULL);
	}
	else//ֹͣ���������
	{
		StopAllMachine();
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

	m_IP.SetCurSel(0);//Ĭ��ʹ�õ�һ��IP��ַ��Ϊ�����źŻ��ĵ�ַ
	return TRUE;
}

