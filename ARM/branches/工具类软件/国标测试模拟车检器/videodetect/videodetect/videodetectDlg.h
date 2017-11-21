// videodetectDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#define NAME_LEN				32				/* 用户名长度 */
#define SERIALNO_LEN        		48				/* 序列号长度 */
#define MAX_TPS_RULE                  8				/*最大参数规则数目*/

typedef struct
{		/* 24 bytes */
	struct in_addr	v4;							//< IPv4地址
	struct in6_addr	v6;							//< IPv6地址
	unsigned char	res[4];
}U_IN_ADDR;
typedef struct 
{
	DWORD dwLength;   						/*报文总长度*/
	BYTE    byRes1[2]; 						/* 保留 */
	BYTE    byCommand;						 /* 请求命令 0x3a*/
	BYTE    byIPType;		 				/*IP类型 0-IPV4 1-IPV6 */
	DWORD dwVersion; 						/* 设备版本信息 */
	BYTE   sDVRName[NAME_LEN];     			/* 设备名称 */
	BYTE   sSerialNumber[SERIALNO_LEN]; 		/* 设备序列号 */
	U_IN_ADDR  struIPAddr;              			/*设备IP地址*/ 
	WORD wPort; 							/* 设备端口号 */
	BYTE byMacAddr[6];	 					/* 设备MAC地址*/
	BYTE byRes3[8]; 							/* 保留 */
}INTER_DVR_REQUEST_HEAD_V30;

typedef struct _NET_VCA_DEV_INFO_
{
	U_IN_ADDR  struDevIP;				/*前端设备地址*/ 
	UINT16 wPort; 						/*前端设备端口号*/ 
	UINT8 byChannel;  					/*前端设备通道*/
	UINT8 byIvmsChannel;				/*作为IVMS通道号*/ 
} NET_VCA_DEV_INFO;

typedef struct _TPS_NET_VCA_POINT_
{	
	UINT16 wX;								/*(0.000-1)*1000 X轴坐标 */
	UINT16 wY;								/*(0.000-1)*1000 Y轴坐标 */
} TPS_NET_VCA_POINT;

typedef struct _NET_LANE_QUEUE_
{
	TPS_NET_VCA_POINT   struHead;			/*队列头*/
	TPS_NET_VCA_POINT   struTail;				/*队列尾*/
	UINT32          dwlength;					/*实际队列长度 单位为米 浮点数*1000*/
}NET_LANE_QUEUE; 

typedef enum tagTRAFFIC_DATA_VARY_TYPE
{    
	ENUM_TRAFFIC_VARY_NO               	= 0x00,   //无变化
	ENUM_TRAFFIC_VARY_VEHICLE_ENTER  = 0x01,   //车辆进入虚拟线圈
	ENUM_TRAFFIC_VARY_VEHICLE_LEAVE  = 0x02,   //车辆离开虚拟线圈
	ENUM_TRAFFIC_VARY_QUEUE           	= 0x04,   //队列变化
	ENUM_TRAFFIC_VARY_STATISTIC        	= 0x08,   //统计数据变化（每分钟变化一次包括平均速度，车道空间/时间占有率，交通状态）        
} TRAFFIC_DATA_VARY_TYPE;

typedef struct _NET_LANE_PARAM
{
	UINT8  byRuleName[NAME_LEN];  		/*车道规则名称 */
	UINT8  byRuleID;                 			/*规则序号，为规则配置结构下标，0-7 */
	UINT8  byLaneType;			      		/*车道下行或下行*/
	UINT8  byTrafficState;					/*车道交通状态*/
	UINT8  byRes1;				        	/* 保留*/
	UINT32 dwVaryType;					/* 车道交通参数变化类型 参照   TRAFFIC_DATA_VARY_TYPE */
	UINT32 dwTpsType;					/* 数据变化类型标志，表示当前上传的统计参数中，哪些数据有效，其值为ITS_TPS_TYPE的任意组合*/
	UINT32 dwLaneVolume;	                   /* 车道流量 ，统计有多少车子通过*/
	UINT32 dwLaneVelocity;         			/*车道速度，公里计算	浮点数*1000*/
	UINT32 dwTimeHeadway ;       			/*车头时距，以秒计算	浮点数*1000*/
	UINT32 dwSpaceHeadway;       		/*车头间距，以米来计算浮点数*1000*/
	UINT32 dwSpaceOccupyRation;   		/*车道占有率，百分比计算（空间上) 浮点数*1000*/
	UINT32 dwTimeOccupyRation;          	/*时间占有率*/
	UINT32 dwLightVehicle;       			/* 小型车数量*/
	UINT32 dwMidVehicle;        			/* 中型车数量*/
	UINT32 dwHeavyVehicle;      			/* 重型车数量*/
	NET_LANE_QUEUE struLaneQueue;       	/*车道队列长度*/
	TPS_NET_VCA_POINT  struRuleLocation;	/*规则位置*/
	UINT8    byRes2[64];
}NET_LANE_PARAM;

//交通统计参数信息结构体
typedef struct _NET_TPS_INFO_
{
	UINT32   dwLanNum;   				/* 交通参数的车道数目*/
	NET_LANE_PARAM struLaneParam[MAX_TPS_RULE];
	UINT8     byRes[32];    				/*保留*/
}NET_TPS_INFO;

typedef struct _NET_TPS_PED_ALARM_
{
	UINT32 dwSize;						/* 结构体大小*/
	UINT32 dwRelativeTime;				/* 相对时标*/
	UINT32 dwAbsTime;					/* 绝对时标*/
	NET_VCA_DEV_INFO struDevInfo;			/* 前端设备信息*/
	NET_TPS_INFO  struTPSInfo;				/* 交通事件信息*/
	//UINT8  byRes1[128];					/* 保留字节*/
	UINT8 dwDeviceId; /*设备ID*/ 
	UINT8 byRes1[127]; /* 保留字节*/ 
}NET_TPS_PED_ALARM;

typedef struct _NET_TPS_ALARM_
{
	UINT32 dwSize;						/* 结构体大小*/
	UINT32 dwRelativeTime;				/* 相对时标*/
	UINT32 dwAbsTime;					/* 绝对时标*/
	NET_VCA_DEV_INFO struDevInfo;			/* 前端设备信息*/
	NET_TPS_INFO  struTPSInfo;				/* 交通事件信息*/
	//UINT8  byRes1[128];					/* 保留字节*/
	UINT32 dwDeviceId; /*设备ID*/ 
	UINT8 byRes1[124]; /* 保留字节*/ 
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


// CvideodetectDlg 对话框
class CvideodetectDlg : public CDialog
{
// 构造
public:
	CvideodetectDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_VIDEODETECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	//设备ID
	CString m_strEquid;
	// 过车数量
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
	// 检测类型
	BOOL m_bDetectType;
	// 信号机IP
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
