// videodetectDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#define NAME_LEN				32				/* �û������� */
#define SERIALNO_LEN        		48				/* ���кų��� */
#define MAX_TPS_RULE                  8				/*������������Ŀ*/

typedef struct
{		/* 24 bytes */
	struct in_addr	v4;							//< IPv4��ַ
	struct in6_addr	v6;							//< IPv6��ַ
	unsigned char	res[4];
}U_IN_ADDR;
typedef struct 
{
	DWORD dwLength;   						/*�����ܳ���*/
	BYTE    byRes1[2]; 						/* ���� */
	BYTE    byCommand;						 /* �������� 0x3a*/
	BYTE    byIPType;		 				/*IP���� 0-IPV4 1-IPV6 */
	DWORD dwVersion; 						/* �豸�汾��Ϣ */
	BYTE   sDVRName[NAME_LEN];     			/* �豸���� */
	BYTE   sSerialNumber[SERIALNO_LEN]; 		/* �豸���к� */
	U_IN_ADDR  struIPAddr;              			/*�豸IP��ַ*/ 
	WORD wPort; 							/* �豸�˿ں� */
	BYTE byMacAddr[6];	 					/* �豸MAC��ַ*/
	BYTE byRes3[8]; 							/* ���� */
}INTER_DVR_REQUEST_HEAD_V30;

typedef struct _NET_VCA_DEV_INFO_
{
	U_IN_ADDR  struDevIP;				/*ǰ���豸��ַ*/ 
	UINT16 wPort; 						/*ǰ���豸�˿ں�*/ 
	UINT8 byChannel;  					/*ǰ���豸ͨ��*/
	UINT8 byIvmsChannel;				/*��ΪIVMSͨ����*/ 
} NET_VCA_DEV_INFO;

typedef struct _TPS_NET_VCA_POINT_
{	
	UINT16 wX;								/*(0.000-1)*1000 X������ */
	UINT16 wY;								/*(0.000-1)*1000 Y������ */
} TPS_NET_VCA_POINT;

typedef struct _NET_LANE_QUEUE_
{
	TPS_NET_VCA_POINT   struHead;			/*����ͷ*/
	TPS_NET_VCA_POINT   struTail;				/*����β*/
	UINT32          dwlength;					/*ʵ�ʶ��г��� ��λΪ�� ������*1000*/
}NET_LANE_QUEUE; 

typedef enum tagTRAFFIC_DATA_VARY_TYPE
{    
	ENUM_TRAFFIC_VARY_NO               	= 0x00,   //�ޱ仯
	ENUM_TRAFFIC_VARY_VEHICLE_ENTER  = 0x01,   //��������������Ȧ
	ENUM_TRAFFIC_VARY_VEHICLE_LEAVE  = 0x02,   //�����뿪������Ȧ
	ENUM_TRAFFIC_VARY_QUEUE           	= 0x04,   //���б仯
	ENUM_TRAFFIC_VARY_STATISTIC        	= 0x08,   //ͳ�����ݱ仯��ÿ���ӱ仯һ�ΰ���ƽ���ٶȣ������ռ�/ʱ��ռ���ʣ���ͨ״̬��        
} TRAFFIC_DATA_VARY_TYPE;

typedef struct _NET_LANE_PARAM
{
	UINT8  byRuleName[NAME_LEN];  		/*������������ */
	UINT8  byRuleID;                 			/*������ţ�Ϊ�������ýṹ�±꣬0-7 */
	UINT8  byLaneType;			      		/*�������л�����*/
	UINT8  byTrafficState;					/*������ͨ״̬*/
	UINT8  byRes1;				        	/* ����*/
	UINT32 dwVaryType;					/* ������ͨ�����仯���� ����   TRAFFIC_DATA_VARY_TYPE */
	UINT32 dwTpsType;					/* ���ݱ仯���ͱ�־����ʾ��ǰ�ϴ���ͳ�Ʋ����У���Щ������Ч����ֵΪITS_TPS_TYPE���������*/
	UINT32 dwLaneVolume;	                   /* �������� ��ͳ���ж��ٳ���ͨ��*/
	UINT32 dwLaneVelocity;         			/*�����ٶȣ��������	������*1000*/
	UINT32 dwTimeHeadway ;       			/*��ͷʱ�࣬�������	������*1000*/
	UINT32 dwSpaceHeadway;       		/*��ͷ��࣬���������㸡����*1000*/
	UINT32 dwSpaceOccupyRation;   		/*����ռ���ʣ��ٷֱȼ��㣨�ռ���) ������*1000*/
	UINT32 dwTimeOccupyRation;          	/*ʱ��ռ����*/
	UINT32 dwLightVehicle;       			/* С�ͳ�����*/
	UINT32 dwMidVehicle;        			/* ���ͳ�����*/
	UINT32 dwHeavyVehicle;      			/* ���ͳ�����*/
	NET_LANE_QUEUE struLaneQueue;       	/*�������г���*/
	TPS_NET_VCA_POINT  struRuleLocation;	/*����λ��*/
	UINT8    byRes2[64];
}NET_LANE_PARAM;

//��ͨͳ�Ʋ�����Ϣ�ṹ��
typedef struct _NET_TPS_INFO_
{
	UINT32   dwLanNum;   				/* ��ͨ�����ĳ�����Ŀ*/
	NET_LANE_PARAM struLaneParam[MAX_TPS_RULE];
	UINT8     byRes[32];    				/*����*/
}NET_TPS_INFO;

typedef struct _NET_TPS_PED_ALARM_
{
	UINT32 dwSize;						/* �ṹ���С*/
	UINT32 dwRelativeTime;				/* ���ʱ��*/
	UINT32 dwAbsTime;					/* ����ʱ��*/
	NET_VCA_DEV_INFO struDevInfo;			/* ǰ���豸��Ϣ*/
	NET_TPS_INFO  struTPSInfo;				/* ��ͨ�¼���Ϣ*/
	//UINT8  byRes1[128];					/* �����ֽ�*/
	UINT8 dwDeviceId; /*�豸ID*/ 
	UINT8 byRes1[127]; /* �����ֽ�*/ 
}NET_TPS_PED_ALARM;

typedef struct _NET_TPS_ALARM_
{
	UINT32 dwSize;						/* �ṹ���С*/
	UINT32 dwRelativeTime;				/* ���ʱ��*/
	UINT32 dwAbsTime;					/* ����ʱ��*/
	NET_VCA_DEV_INFO struDevInfo;			/* ǰ���豸��Ϣ*/
	NET_TPS_INFO  struTPSInfo;				/* ��ͨ�¼���Ϣ*/
	//UINT8  byRes1[128];					/* �����ֽ�*/
	UINT32 dwDeviceId; /*�豸ID*/ 
	UINT8 byRes1[124]; /* �����ֽ�*/ 
}NET_TPS_ALARM;

typedef struct
{
	UINT8 nDetectorId;
	UINT8 nTotalFlow;
	UINT8 nLargeFlow;
	UINT8 nSmallFlow;
	UINT8 nPercent;
	UINT8 nSpeed;
	UINT8 nLength;
}STRU_DETECTOR_DATA;

typedef struct
{
	UINT8 nDetectorId;

	UINT8 noLive:1;
	UINT8 liveLong:1;
	UINT8 unStable:1;
	UINT8 commErr:1;
	UINT8 cfgErr:1;
	UINT8 :2;
	UINT8 unKown:1;

	UINT8 nOther:1;
	UINT8 nWatchdog:1;
	UINT8 nOpen:1;
	UINT8 nLow:1;
	UINT8 nHigh:1;
	UINT8 :3;
}STRU_DETECTOR_STATUS;


// CvideodetectDlg �Ի���
class CvideodetectDlg : public CDialog
{
// ����
public:
	CvideodetectDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_VIDEODETECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	//�豸ID
	CString m_strEquid;
	// ��������
	CString m_strVehileCount;
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnBnClickedBtnStop();
public:
	static UINT fnSendThread(PVOID);
	BOOL ConnectTSC();
	void CloseSocketDef();
	BOOL GetTSCip();
	BOOL SetTSCip();
private:	
	SOCKET m_nSocket;
	BOOL m_bStopSendThread;
	CString m_strIniFileName;
public:
	// �������
	BOOL m_bDetectType;
	// �źŻ�IP
	DWORD m_dwTSCip;
	CIPAddressCtrl m_ctlTSCip;
	CEdit m_data_total_flow;
	CEdit m_data_large_flow;
	afx_msg void OnEnKillfocusTotalFlow();
	CEdit m_data_small_flow;
	CEdit m_data_percent;
	CEdit m_data_speed;
	CEdit m_data_vechile_length;
	CButton m_detector_unlive;
	CButton m_detector_unstable;
	CButton m_detector_livelong;
	CButton m_detector_communication_err;
	CButton m_detector_configure_err;
	CButton m_detector_unknow_err;
	CButton m_detector_noraml;
	CButton m_induction_unkown_err;
	CButton m_induction_watchdog_err;
	CButton m_induction_open;
	CButton m_induction_low;
	CButton m_induction_high;
	CButton m_induction_normal;
	afx_msg void OnEnKillfocusLargeVechileFlow();
	afx_msg void OnEnKillfocusSmallVechileFlow();
	afx_msg void OnEnKillfocusPercent();
	afx_msg void OnEnKillfocusSpeed();
	afx_msg void OnEnKillfocusVechileLength();
	afx_msg void OnBnClickedDetectorNolive();
	afx_msg void OnBnClickedDetectorLiveLong();
	afx_msg void OnBnClickedDetectorUnstable();
	afx_msg void OnBnClickedDetectorCommunicationErr();
	afx_msg void OnBnClickedConfigureErr();
	afx_msg void OnBnClickedDetectorUnkonwErr();
	afx_msg void OnBnClickedDetectorNormal();
	afx_msg void OnBnClickedInductionUnknow();
	afx_msg void OnBnClickedInductionWatchdogErr();
	afx_msg void OnBnClickedInductionOpen();
	afx_msg void OnBnClickedInductionLow();
	afx_msg void OnBnClickedInductionHiigh();
	afx_msg void OnBnClickedInductionNormal();
	afx_msg void OnEnKillfocusEditEquid();
};
