/*
 * main.c -- Main program for the GoAhead WebServer
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	Main program for the GoAhead WebServer. This is a demonstration
 *	program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/

#include	<direct.h>
#include	<windows.h>
#include	<winuser.h>
#include	<process.h>

#include	"../wsIntrn.h"
#include    "../inifile.h"
#include    "../util_xml.h"

#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"../um.h"
void		formDefineUserMgmt(void);
#endif

#define USE_DEMO_MODE 1
#define BUF_SIZE 256 

/********************************** Defines ***********************************/
/*
	Enable USE_DEMO_MODE to run Webs with the documentation tree and examples 
*/
/* #define USE_DEMO_MODE		1 */

#define	IDM_ABOUTBOX		0xF200
#define	IDS_ABOUTBOX		"AboutBox"
#define	IDC_VERSION			101
#define	IDC_BUILDDATE		102
#define	IDC_DISMISS			103
#define	SOCK_DFT_SVC_TIME	20

#define METHOD_NULL        0x00
#define METHOD_GET           0x01
#define METHOD_POST         0x02
#define METHOD_PUT           0x04
#define METHOD_DELETE     0x10

/*********************************** Globals **********************************/
/*
 *	User configurable globals	
 */
static char_t	*rootWeb = T("www");			/* Root web directory */
static char_t	*demoWeb = T("wwwdemo");		/* Root web directory */
static char_t	*password = T("");				/* Security password */

/* Globals */
static int		port = WEBS_DEFAULT_PORT;		/* Server port */
static char_t	*title = T("GoAhead WebServer");/* Window title */
static char_t	*name = T("gowebs");			/* Window name */
static HWND		hwnd;							/* Main Window handle */
static HWND		hwndAbout;						/* About Window handle */
static int		retries = 5;					/* Server port retries */
static int		finished;						/* Finished flag */
static int		sockServiceTime;				/* in milliseconds */

/****************************** Forward Declarations **************************/

static int		initWebs(int demo);
static long		CALLBACK websWindProc(HWND hwnd, unsigned int msg, 
				unsigned int wp, long lp);
static long		CALLBACK websAboutProc(HWND hwnd, unsigned int msg, 
				unsigned int wp, long lp);
static int		registerAboutBox(HINSTANCE hInstance);
static int		createAboutBox(HINSTANCE hInstance, HWND hwnd);
static int		aspTest(int eid, webs_t wp, int argc, char_t **argv);
static WPARAM	checkWindowsMsgLoop();
static void		formTest(webs_t wp, char_t *path, char_t *query);
static void		loginTest(webs_t wp, char_t *path, char_t *query);//登录接口
static void     unitParamsTest(webs_t wp, char_t *path, char_t *query);//单位参数
static void     PhaseTableTest(webs_t wp, char_t *path, char_t *query);//相位表
static void     ringAndPhaseTest(webs_t wp, char_t *path, char_t *query);//环并发相位
static void     channelTableTest(webs_t wp, char_t *path, char_t *query);//通道表
static void     greenRatioTest(webs_t wp, char_t *path, char_t *query);//绿信比
static void     faultDetectionSetTest(webs_t wp, char_t *path, char_t *query);//故障检测设置
static void     sequenceTableTest(webs_t wp, char_t *path, char_t *query);//相序表
static void     programTableTest(webs_t wp, char_t *path, char_t *query);//方案表
static void     timeBasedActionTableTest(webs_t wp, char_t *path, char_t *query);//时基动作表
static void     timeIntervalTest(webs_t wp, char_t *path, char_t *query);//时段
static void     schedulingTest(webs_t wp, char_t *path, char_t *query);//调度计划
static void     overlappingTest(webs_t wp, char_t *path, char_t *query);//重叠表
static void     coordinateTest(webs_t wp, char_t *path, char_t *query);//协调
static void     vehicleDetectorTest(webs_t wp, char_t *path, char_t *query);//车辆检测器
static void     pedestrianDetectorTest(webs_t wp, char_t *path, char_t *query);//行人检测器
static void     faultConfigTest(webs_t wp, char_t *path, char_t *query);//故障配置
static void     TreeDynamicParameter(webs_t wp, char_t *path, char_t *query);//树动态参数
static void     actionDownload(webs_t wp, char_t *path, char_t *query);//参数文件下载
static char  *  getXMLValue(char * XMLStr,char * rootTag,char * rootTagName,char * returnValue);
static int		windowsInit(HINSTANCE hinstance);
static int		windowsClose(HINSTANCE hinstance);
static int		websHomePageHandler(webs_t wp, char_t *urlPrefix,
					char_t *webDir, int arg, char_t *url, char_t *path,
					char_t *query);
static void		printMemStats(int handle, char_t *fmt, ...);
static void		memLeaks();

static LPWORD	lpwAlign(LPWORD);
static int		nCopyAnsiToWideChar(LPWORD, LPSTR);
static void		centerWindowOnDisplay(HWND hwndCenter);
/*单位参数*/
typedef struct 
{
	int iStartFlashingYellowTime;								//启动黄闪时间，单位秒
	int iStartAllRedTime;										//启动全红时间，单位秒
	int iDegradationTime;										//降级时间，单位秒
	int iSpeedFactor;											//速度因子（/1000）
	int iMinimumRedLightTime;									//最小红灯时间（/10）,单位秒
	int iCommunicationTimeout;									//通信超时,单位分钟
	int iFlashingFrequency;										//闪光频率,单位次/秒
	int iTwiceCrossingTimeInterval;								//二次过街时差，单位秒
	int iTwiceCrossingReverseTimeInterval;						//二次过街逆向时差，单位秒
	int iSmoothTransitionPeriod;								//平滑过渡周期
	int iFlowCollectionPeriod;									//流量采集周期
	int iCollectUnit;                                           //采集单位
	int iAutoPedestrianEmpty;									//自动行人清空，勾选框
	int iOverpressureDetection;									//检测过压，降级黄闪，勾选框
} stUnitParams;
stUnitParams gUnitParams;
/*相位表*/        //add by lxp
typedef struct 
{
	int iPhaseNo;								            //相位号
	/*基础时间*/
	int iMinimumGreen;										//最小绿
	int iMaximumGreenOne;								    //最大绿1
	int iMaximumGreenTwo;									//最大绿2
	int iExtensionGreen;									//延长绿(/10)
	int iMaximumRestrict;									//最大值限制
	int iDynamicStep;										//动态步长(/10)
	int iYellowLightTime;								    //黄灯时间(/10)
	int iAllRedTime;						                //全红时间(/10)
	int iRedLightProtect;								    //红灯保护(/10)
	/*密度时间*/
	int iIncreaseInitValue;								//增加初始值(/10)
	int iIncreaseInitialValueCalculation;                   //增加初始值计算
	int iMaximumInitialValue;						        //最大初始值
	int iTimeBeforeDecrease;								//递减前时间
	int iVehicleBeforeDecrease;								//递减前车辆
	int iDecreaseTime;										//递减时间
	int iUnitDeclineTime;									//单位递减率
	int iMinimumInterval;									//最小间隔(/10)
	/*行人*/
	int iPedestrianRelease;									//行人放行
	int iPedestrianCleaned;                                 //行人清空
	int iKeepPedestrianRelease;							    //保持行人放行
	/*选项*/
	int iNoLockDetentionRequest;						    //不锁定滞留请求
	int iDoubleEntrancePhase;								//双入口相位
	int iGuaranteeFluxDensityExtensionGreen;				//保证流量密度延长绿
	int iConditionalServiceValid;							//有条件服务有效
	int iMeanwhileEmptyLoseEfficacy;						//同时空当失效
	/*综合*/
	int iInitialize;									    //初始化
	int iNonInduction;									    //非感应
	int iVehicleAutomaticRequest;							//机动车自动请求
	int iPedestrianAutomaticRequest;						//行人自动请求
	int iAutomaticFlashInto;						        //自动闪光进入
	int iAutomaticFlashExit;								//自动闪光退出

} stPhaseTable;
stPhaseTable gPhaseTable;
/*环并发相位*/        //add by lxp
typedef struct 
{
	int iRingNum;								            //环序号
	int iPhaseNum[32];                                      //相位
	int iPhaseNumAll;                                       //相位全选
} stRingAndPhase;
stRingAndPhase gRingAndPhase;
/*通道号*/        //add by lxp
typedef struct 
{
	int iControlSource;                                      //控制源
	int iControlType;                                        //控制类型
	int iFlashMode;                                          //闪光模式
	int iBrightMode;                                         //灰度模式
} stChannelTable;
stChannelTable gChannelTable;
/*绿信比*/        //add by lxp
typedef struct 
{
	int greenNum;								         //绿信比表
	int split;                                           //绿信比
	int greenType;                                       //模式
	int coordinatePhase;                                 //作为协调相位
	int keyPhase;                                        //作为关键相位
	int fixedPhase;								         //作为固定相位
	int pedestrianMove;                                  //行人放行
	int pedestrianEmpty;                                 //行人清空
	int frontierGreen;                                   //临界绿灯
	int emptyType;                                       //清空模式
} stGreenRatio;
stGreenRatio gGreenRatio;
/*故障检测设置*/        //add by lxp
typedef struct 
{
	int VoltageDetectionTimes;								            //过欠压检测次数
	int RedLightDetectionTimes;                                         //红灯熄灭检测次数
	int ConflictDetectionAttempts;                                      //冲突检测次数
	int ManualPanelKeyNumber;								            //手动面板按键次数
	int RemoteControlKeyNumber;                                         //遥控器按键次数
	int SenseSwitch;                                                    //检测开关
	int DynamicStep;								                    //电流故障检测
	int CurrentFaultDetection;                                          //电压故障检测
	int AlarmAndFaultCurrent;                                           //电流故障报警并处理
	int AlarmAndFaultVoltage;								            //电压故障报警并处理
	int EnableWatchdog;                                                 //启用看门狗
	int CNum[32][3];                                                    //通道号参数
} stFaultDetectionSet;
stFaultDetectionSet gFaultDetectionSet;
/*相序表*/        //add by lxp
typedef struct 
{
	int SequenceTableNo;								                //相序表编号
	int SNum[4][16];                                                    //相序号
} stSequenceTable;
stSequenceTable gSequenceTable;
/*方案表*/        //add by lxp
typedef struct 
{
	int PNum[4][4];                                                    //方案号
} stProgramTable;
stProgramTable gProgramTable;
/*时基动作表*/        //add by lxp
typedef struct 
{
	int ActionTable;													//动作表
	int ProgramNo;														//方案号
	int AssistFunction[4];                                              //辅助功能
	int SpecialFunction[8];                                             //特殊功能
} stTimeBasedActionTable;
stTimeBasedActionTable gTimeBasedActionTable;
/*时段表*/        //add by lxp
typedef struct 
{
	int TimeIntervalNo;													//时段表号
	int Time[4][3];                                                     //时段信息
} stTimeInterval;
stTimeInterval gTimeInterval;
/*调度计划*/        //add by lxp
typedef struct 
{
	int SchedulingNo;													//调度计划表
	int TimeIntervalNum;                                                //时段表
	int Month[13];                                                      //月份
	int Day[32];                                                        //日期
	int WeekDay[8];                                                     //星期
} stScheduling;
stScheduling gScheduling;
/*重叠表*/        //add by lxp
typedef struct 
{
	int FollowPhase;													//跟随相位
	int GreenLight;														//绿灯
	int RedLight;                                                       //红灯
	int YellowLight;                                                    //黄灯
	int GreenFlash;                                                     //绿闪
	int ModifiedPhase;                                                  //修正相位
	int ParentPhase[32];                                                //母相位
} stOverlapping;
stOverlapping gOverlapping;
/*协调*/        //add by lxp
typedef struct 
{
	int ControlModel;													//控制模式
	int ManualMethod;													//手动方案
	int CoordinationMode;                                               //协调方式
	int CoordinateMaxMode;                                              //协调最大方式
	int CoordinateForceMode;                                            //协调强制方式
} stCoordinate;
stCoordinate gCoordinate;
/*车辆检测器*/        //add by lxp
typedef struct 
{
	int DetectorNo;													    //检测器号
	int RequestPhase;													//请求相位
	int SwitchPhase;                                                    //开关相位
	int Delay;                                                          //延迟
	int FailureTime;                                                    //失败时间
	int QueueLimit;													    //队列限制
	int NoResponseTime;													//无响应时间
	int MaxDuration;                                                    //最大持续时间
	int Extend;                                                         //延长
	int MaxVehicle;                                                     //最大车辆数
	int Flow;													        //流量
	int Occupancy;													    //占有率
	int ProlongGreen;                                                   //延长绿
	int AccumulateInitial;                                              //积累初始
	int Queue;                                                          //排队
	int Request;													    //请求
	int RedInterval;													//红灯区间
	int YellowInterval;                                                 //黄灯时间
} stVehicleDetector;
stVehicleDetector gVehicleDetector;
/*行人检测器*/        //add by lxp
typedef struct 
{
	int DetectorNo;													    //检测器编号
	int RequestPhase;													//请求相位
	int NoResponseTime;                                                 //无响应时间：
	int MaxDuration;                                                    //最大持续时间：
	int InductionNumber;                                                //感应数
} stPedestrian;
stPedestrian gPedestrian;
/*故障配置*/        //add by lxp
typedef struct 
{
	int ControlRecord;													//控制器故障
	int CommunicatRecord;													//通信故障
	int DetectorRecord;                                               //检测器故障
} stFaultConfig;
stFaultConfig gFaultConfig;
/*树动态参数*/        //add by lxp
typedef struct 
{
	int addCount;													//相位表总数
	int addChannel;                                                 //通道号总数
	int addProgram;                                                 //方案表总数
} stTreeDynamicPara;
stTreeDynamicPara gTreeDynamicPara;
/*********************************** Code *************************************/
/*
 *	WinMain -- entry point from Windows
 */
int APIENTRY WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance,
						char *args, int cmd_show)
{
	WPARAM	rc;
#ifdef USE_DEMO_MODE
	int demo = 1;
#else
	int demo = 0;
#endif /* USE_DEMO_MODE */
	
/*
 *	Initialize the memory allocator. Allow use of malloc and start 
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);

/* 
 *	Store the instance handle (used in socket.c)
 */

	if (windowsInit(hinstance) < 0) {
		return FALSE;
	}

/*
 *	Initialize the web server
 */
	if (initWebs(demo) < 0) {
		return FALSE;
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
/*	websRequireSSL("/"); */ /* Require all files be served via https */
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, sockServiceTime)) {
			socketProcess(-1);
		}
		emfSchedProcess();
		websCgiCleanup();
		if ((rc = checkWindowsMsgLoop()) != 0) {
			break;
		}
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

/*
 *	Close the User Management database
 */
#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();

/*
 *	Free up Windows resources
 */
	windowsClose(hinstance);

#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return rc;
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs(int demo)
{
	struct hostent	*hp;
	struct in_addr	intaddr;
	char			*cp;
	char			host[64], dir[128];
	char_t			dir_t[128];
	char_t			wbuf[256];

	

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

/*
 *	Initialize the User Management database
 */
#ifdef USER_MANAGEMENT_SUPPORT
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *	Define the local Ip address, host name, default home page and the 
 *	root web directory.
 */
	if (gethostname(host, sizeof(host)) < 0) {
		error(E_L, E_LOG, T("Can't get hostname"));
		return -1;
	}
	if ((hp = gethostbyname(host)) == NULL) {
		error(E_L, E_LOG, T("Can't get host address"));
		return -1;
	}
	memcpy((void *) &intaddr, (void *) hp->h_addr_list[0],
		(size_t) hp->h_length);

/*
 *	Set ../web as the root web. Modify this to suit your needs
 */
	getcwd(dir, sizeof(dir)); 
	for (cp = dir; *cp; cp++) {
		if (*cp == '\\')
			*cp = '/';
	}
	if (cp = strrchr(dir, '/')) {
		*cp = '\0';
	}
	ascToUni(dir_t, dir, sizeof(dir_t));

	if (demo) {
		gsprintf(wbuf, T("%s/%s"), dir_t, demoWeb);
	} else {
		gsprintf(wbuf, T("%s/%s"), dir_t, rootWeb);
	}
	websSetDefaultDir(wbuf);
	cp = inet_ntoa(intaddr);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, hp->h_name, min(strlen(hp->h_name) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("default.asp"));
	websSetPassword(password);

/* 
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security 
 *	handler, forms handler and the default web page handler.
 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler, 
		WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler, 
		WEBS_HANDLER_LAST); 

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);
	websFormDefine(T("loginTest"),loginTest);
	websFormDefine(T("unitParamsTest"),unitParamsTest);
	websFormDefine(T("PhaseTableTest"),PhaseTableTest);
	websFormDefine(T("ringAndPhaseTest"),ringAndPhaseTest);
	websFormDefine(T("channelTableTest"),channelTableTest);
	websFormDefine(T("greenRatioTest"),greenRatioTest);	
	websFormDefine(T("faultDetectionSetTest"),faultDetectionSetTest);
	websFormDefine(T("sequenceTableTest"),sequenceTableTest);
	websFormDefine(T("programTableTest"),programTableTest);
	websFormDefine(T("timeBasedActionTableTest"),timeBasedActionTableTest);
	websFormDefine(T("timeIntervalTest"),timeIntervalTest);
	websFormDefine(T("schedulingTest"),schedulingTest);
	websFormDefine(T("overlappingTest"),overlappingTest);
	websFormDefine(T("coordinateTest"),coordinateTest);
	websFormDefine(T("vehicleDetectorTest"),vehicleDetectorTest);
	websFormDefine(T("pedestrianDetectorTest"),pedestrianDetectorTest);
	websFormDefine(T("faultConfigTest"),faultConfigTest);
	websFormDefine(T("TreeDynamicParameter"),TreeDynamicParameter);
	websFormDefine(T("actionDownload"),actionDownload);
/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0); 

/* 
 *	Set the socket service timeout to the default
 */
	sockServiceTime = SOCK_DFT_SVC_TIME;				

	return 0;
}


/******************************************************************************/
/*
 *	Create a taskbar entry. Register the window class and create a window
 */

static int windowsInit(HINSTANCE hinstance)
{
	WNDCLASS  		wc;						/* Window class */
	HMENU			hSysMenu;

	emfInstSet((int) hinstance);

	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hinstance;
	wc.hIcon		 = NULL;
	wc.lpfnWndProc	 = (WNDPROC) websWindProc;
	wc.lpszMenuName	 = wc.lpszClassName = name;
	if (! RegisterClass(&wc)) {
		return -1;
	}

/*
 *	Create a window just so we can have a taskbar to close this web server
 */
	hwnd = CreateWindow(name, title, WS_MINIMIZE | WS_POPUPWINDOW,
		CW_USEDEFAULT, 0, 0, 0, NULL, NULL, hinstance, NULL);
	if (hwnd == NULL) {
		return -1;
	}

/*
 *	Add the about box menu item to the system menu
 *	a_assert: IDM_ABOUTBOX must be in the system command range.
 */
	hSysMenu = GetSystemMenu(hwnd, FALSE);
	if (hSysMenu != NULL)
	{
		AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hSysMenu, MF_STRING, IDM_ABOUTBOX, T("About WebServer"));
	}

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	hwndAbout = NULL;
	return 0;
}

/******************************************************************************/
/*
 *	Close windows resources
 */

static int windowsClose(HINSTANCE hInstance)
{
	int nReturn;

	nReturn = UnregisterClass(name, hInstance);

	if (hwndAbout) {
		DestroyWindow(hwndAbout);
	}

	return nReturn;
}

/******************************************************************************/
/*
 *	Main menu window message handler.
 */

static long CALLBACK websWindProc(HWND hwnd, unsigned int msg, 
									unsigned int wp, long lp)
{
	switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			finished++;
			return 0;

		case WM_SYSCOMMAND:
			if (wp == IDM_ABOUTBOX) {
				if (!hwndAbout) {
					createAboutBox((HINSTANCE) emfInstGet(), hwnd);
				}
				if (hwndAbout) {
					ShowWindow(hwndAbout, SW_SHOWNORMAL);
				}
			}
			break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

/******************************************************************************/
/*
 *	Check for Windows Messages
 */

WPARAM checkWindowsMsgLoop()
{
	MSG		msg;					/* Message block */

	if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage(&msg, NULL, 0, 0) || msg.message == WM_QUIT) {
			return msg.wParam;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

/******************************************************************************/
/*
 *	Windows message handler for the About Box
 */

static long CALLBACK websAboutProc(HWND hwndDlg, unsigned int msg, 
									unsigned int wp, long lp)
{
	long lResult;
	HWND hwnd;

	lResult = DefWindowProc(hwndDlg, msg, wp, lp);

	switch (msg) {
		case WM_CREATE:
			hwndAbout = hwndDlg;
			break;

		case WM_DESTROY:
			hwndAbout = NULL;
			break;

		case WM_COMMAND:
			if (wp == IDOK) {
				EndDialog(hwndDlg, 0);
				PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case WM_INITDIALOG:
/*
 *			Set the version and build date values
 */
			hwnd = GetDlgItem(hwndDlg, IDC_VERSION);
			if (hwnd) {
				SetWindowText(hwnd, WEBS_VERSION);
			}

			hwnd = GetDlgItem(hwndDlg, IDC_BUILDDATE);
			if (hwnd) {
				SetWindowText(hwnd, __DATE__);
			}

			SetWindowText(hwndDlg, T("GoAhead WebServer"));
			centerWindowOnDisplay(hwndDlg);

			hwndAbout = hwndDlg;

			lResult = FALSE;
			break;
	}

	return lResult;
}

/******************************************************************************/
/*
 *	Registers the About Box class
 */

static int registerAboutBox(HINSTANCE hInstance)
{
	WNDCLASS  wc;

    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)websAboutProc;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = IDS_ABOUTBOX;

    if (!RegisterClass(&wc)) {
		return 0;
	}

 	return 1;
}

/******************************************************************************/
/*
 *   Helper routine.  Take an input pointer, return closest
 *    pointer that is aligned on a DWORD (4 byte) boundary.
 */

static LPWORD lpwAlign(LPWORD lpIn)
{
	ULONG ul;

	ul = (ULONG) lpIn;
	ul +=3;
	ul >>=2;
	ul <<=2;
	return (LPWORD) ul;
}

/******************************************************************************/
/*
 *   Helper routine.  Takes second parameter as Ansi string, copies
 *    it to first parameter as wide character (16-bits / char) string,
 *    and returns integer number of wide characters (words) in string
 *    (including the trailing wide char NULL).
 */

static int nCopyAnsiToWideChar(LPWORD lpWCStr, LPSTR lpAnsiIn)
{
	int cchAnsi = lstrlen(lpAnsiIn);

	return MultiByteToWideChar(GetACP(), 
		MB_PRECOMPOSED, lpAnsiIn, cchAnsi, lpWCStr, cchAnsi) + 1;
}

/******************************************************************************/
/*
 *	Creates an About Box Window
 */

static int createAboutBox(HINSTANCE hInstance, HWND hwnd)
{
	WORD	*p, *pdlgtemplate;
	int		nchar;
	DWORD	lStyle;
	HWND	hwndReturn;

/* 
 *	Allocate some memory to play with  
 */
	pdlgtemplate = p = (PWORD) LocalAlloc(LPTR, 1000);

/*
 *	Start to fill in the dlgtemplate information.  addressing by WORDs 
 */

	lStyle = WS_DLGFRAME | WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | DS_SETFONT;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;          /* LOWORD (lExtendedStyle) */
	*p++ = 0;          /* HIWORD (lExtendedStyle) */
	*p++ = 7;          /* Number Of Items	*/
	*p++ = 210;        /* x */
	*p++ = 10;         /* y */
	*p++ = 200;        /* cx */
	*p++ = 100;        /* cy */
	*p++ = 0;          /* Menu */
	*p++ = 0;          /* Class */

/* 
 *	Copy the title of the dialog 
 */
	nchar = nCopyAnsiToWideChar(p, WEBS_NAME);
	p += nchar;

/*	
 *	Font information because of DS_SETFONT
 */
	*p++ = 11;     /* point size */
	nchar = nCopyAnsiToWideChar(p, T("Arial Bold"));
	p += nchar;

/*
 *	Make sure the first item starts on a DWORD boundary
 */
	p = lpwAlign(p);

/*
 *	Now start with the first item (Product Identifier)
 */
	lStyle = SS_CENTER | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;			/* LOWORD (lExtendedStyle) */
	*p++ = 0;			/* HIWORD (lExtendedStyle) */
	*p++ = 10;			/* x */
	*p++ = 10;			/* y  */
	*p++ = 180;			/* cx */
	*p++ = 15;			/* cy */
	*p++ = 1;			/* ID */

/*
 *	Fill in class i.d., this time by name
 */
	nchar = nCopyAnsiToWideChar(p, TEXT("STATIC"));
	p += nchar;

/*
 *	Copy the text of the first item
 */
	nchar = nCopyAnsiToWideChar(p, 
		TEXT("GoAhead WebServer ") WEBS_VERSION);
	p += nchar;
#ifdef WEBS_SSL_SUPPORT
	p -= sizeof(char_t);
	nchar = nCopyAnsiToWideChar(p, 
		TEXT("\n") SSL_NAME TEXT(" ") SSL_VERSION);
	p += nchar;
#endif
/*
 *	Advance pointer over nExtraStuff WORD
 */
	*p++ = 0;  

/*
 *	Make sure the next item starts on a DWORD boundary
 */
	p = lpwAlign(p);

/*
 *	Next, the Copyright Notice.
 */
	lStyle = SS_CENTER | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;			/* LOWORD (lExtendedStyle) */
	*p++ = 0;			/* HIWORD (lExtendedStyle) */
	*p++ = 10;			/* x */
	*p++ = 30;			/* y  */
	*p++ = 180;			/* cx */
	*p++ = 15;			/* cy */
	*p++ = 1;			/* ID */

/*
 *	Fill in class i.d. by name
 */
	nchar = nCopyAnsiToWideChar(p, TEXT("STATIC"));
	p += nchar;

/*
 *	Copy the text of the item
 */
	nchar = nCopyAnsiToWideChar(p, GOAHEAD_COPYRIGHT);
	p += nchar;

/*
 *	Advance pointer over nExtraStuff WORD
 */
	*p++ = 0;  
/*
 *	Make sure the next item starts on a DWORD boundary
 */
	p = lpwAlign(p);

/*
 *	Add third item ("Version:")
 */
	lStyle = SS_RIGHT | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;          
	*p++ = 0;
	*p++ = 28;
	*p++ = 50;
	*p++ = 70;
	*p++ = 10;
	*p++ = 1;
	nchar = nCopyAnsiToWideChar(p, T("STATIC"));
	p += nchar;
	nchar = nCopyAnsiToWideChar(p, T("Version:"));
	p += nchar;
	*p++ = 0;

/*
 *	Add fourth Item (IDC_VERSION)
 */
	p = lpwAlign(p);
	lStyle = SS_LEFT | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;          
	*p++ = 0;
	*p++ = 102;
	*p++ = 50;
	*p++ = 70;
	*p++ = 10;
	*p++ = IDC_VERSION;
	nchar = nCopyAnsiToWideChar(p, T("STATIC"));
	p += nchar;
	nchar = nCopyAnsiToWideChar(p, T("version"));
	p += nchar;
	*p++ = 0;
/*
 *	Add fifth item ("Build Date:")
 */
	p = lpwAlign(p);
	lStyle = SS_RIGHT | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;          
	*p++ = 0;
	*p++ = 28;
	*p++ = 65;
	*p++ = 70;
	*p++ = 10;
	*p++ = 1;
	nchar = nCopyAnsiToWideChar(p, T("STATIC"));
	p += nchar;
	nchar = nCopyAnsiToWideChar(p, T("Build Date:"));
	p += nchar;
	*p++ = 0;
/*
 *	Add sixth item (IDC_BUILDDATE)
 */
	p = lpwAlign(p);
	lStyle = SS_LEFT | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;          
	*p++ = 0;
	*p++ = 102;
	*p++ = 65;
	*p++ = 70;
	*p++ = 10;
	*p++ = IDC_BUILDDATE;
	nchar = nCopyAnsiToWideChar(p, T("STATIC"));
	p += nchar;
	nchar = nCopyAnsiToWideChar(p, T("Build Date"));
	p += nchar;
	*p++ = 0;
/*
 *	Add seventh item (IDOK)
 */
	p = lpwAlign(p);
	lStyle = BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	*p++ = LOWORD(lStyle);
	*p++ = HIWORD(lStyle);
	*p++ = 0;          
	*p++ = 0;
	*p++ = 80;
	*p++ = 80;
	*p++ = 40;
	*p++ = 10;
	*p++ = IDOK;
	nchar = nCopyAnsiToWideChar(p, T("BUTTON"));
	p += nchar;
	nchar = nCopyAnsiToWideChar(p, T("OK"));
	p += nchar;
	*p++ = 0;

	hwndReturn = CreateDialogIndirect(hInstance, 
		(LPDLGTEMPLATE) pdlgtemplate, hwnd, (DLGPROC) websAboutProc);

	LocalFree(LocalHandle(pdlgtemplate));

	return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to 
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith")); 
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave.")); 

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 * @brief : 用户登录处理函数
 * @author : liujie
 * @date : 2013/1/29 16:45
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void loginTest(webs_t wp, char_t *path, char_t *query)
{
	char usernameStr[16] = {0};
	char passwordStr[16] = {0};
	char *username = usernameStr;
	char *password = passwordStr;
	int result = 0;
	getXMLValue(query,"Login","username",username);
	getXMLValue(query,"Login","password",password);
	result = getLoginInfo(usernameStr,passwordStr);
	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<Login>\r\n"));
	websWrite(wp, T("<username>%s</username>\r\n"), username);
	websWrite(wp, T("<password>%s</password>\r\n"), password);
	websWrite(wp, T("<loginresult>%d</loginresult>\r\n"), result);
	websWrite(wp, T("</Login>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 * @brief : 单元参数配置页面处理函数
 * @author : liujie
 * @date : 2013/2/1 16:45
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void unitParamsTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};

	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getUnitParamsInfo(&gUnitParams,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<UNITPARAMS>\r\n"));
		websWrite(wp, T("<StartFlashingYellowTime>%d</StartFlashingYellowTime>\r\n"), gUnitParams.iStartFlashingYellowTime);
		websWrite(wp, T("<StartAllRedTime>%d</StartAllRedTime>\r\n"), gUnitParams.iStartAllRedTime);
		websWrite(wp, T("<DegradationTime>%d</DegradationTime>\r\n"), gUnitParams.iDegradationTime);
		websWrite(wp, T("<SpeedFactor>%d</SpeedFactor>\r\n"), gUnitParams.iSpeedFactor);
		websWrite(wp, T("<MinimumRedLightTime>%d</MinimumRedLightTime>\r\n"), gUnitParams.iMinimumRedLightTime);
		websWrite(wp, T("<CommunicationTimeout>%d</CommunicationTimeout>\r\n"), gUnitParams.iCommunicationTimeout);
		websWrite(wp, T("<FlashingFrequency>%d</FlashingFrequency>\r\n"), gUnitParams.iFlashingFrequency);
		websWrite(wp, T("<TwiceCrossingTimeInterval>%d</TwiceCrossingTimeInterval>\r\n"), gUnitParams.iTwiceCrossingTimeInterval);
		websWrite(wp, T("<TwiceCrossingReverseTimeInterval>%d</TwiceCrossingReverseTimeInterval>\r\n"), gUnitParams.iTwiceCrossingReverseTimeInterval);
		websWrite(wp, T("<SmoothTransitionPeriod>%d</SmoothTransitionPeriod>\r\n"), gUnitParams.iSmoothTransitionPeriod);
		websWrite(wp, T("<FlowCollectionPeriod>%d</FlowCollectionPeriod>\r\n"), gUnitParams.iFlowCollectionPeriod);
		websWrite(wp, T("<CollectUnit>%d</CollectUnit>\r\n"), gUnitParams.iCollectUnit);
		websWrite(wp, T("<AutoPedestrianEmpty>%d</AutoPedestrianEmpty>\r\n"), gUnitParams.iAutoPedestrianEmpty);
		websWrite(wp, T("<OverpressureDetection>%d</OverpressureDetection>\r\n"), gUnitParams.iOverpressureDetection);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</UNITPARAMS>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回g
		memset(&gUnitParams,0,sizeof(stUnitParams));
		getXMLValue(query,"UNITPARAMS","StartFlashingYellowTime",tmpStr);
		gUnitParams.iStartFlashingYellowTime = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","StartAllRedTime",tmpStr);
		gUnitParams.iStartAllRedTime = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","DegradationTime",tmpStr);
		gUnitParams.iDegradationTime = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","SpeedFactor",tmpStr);
		gUnitParams.iSpeedFactor = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","MinimumRedLightTime",tmpStr);
		gUnitParams.iMinimumRedLightTime = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","CommunicationTimeout",tmpStr);
		gUnitParams.iCommunicationTimeout = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","FlashingFrequency",tmpStr);
		gUnitParams.iFlashingFrequency = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","TwiceCrossingTimeInterval",tmpStr);
		gUnitParams.iTwiceCrossingTimeInterval = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","TwiceCrossingReverseTimeInterval",tmpStr);
		gUnitParams.iTwiceCrossingReverseTimeInterval = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","SmoothTransitionPeriod",tmpStr);
		gUnitParams.iSmoothTransitionPeriod = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","FlowCollectionPeriod",tmpStr);
		gUnitParams.iFlowCollectionPeriod = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","CollectUnit",tmpStr);
		gUnitParams.iCollectUnit = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","AutoPedestrianEmpty",tmpStr);
		gUnitParams.iAutoPedestrianEmpty = atoi(tmpStr);
		getXMLValue(query,"UNITPARAMS","OverpressureDetection",tmpStr);
		gUnitParams.iOverpressureDetection = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = saveUnitParams2Ini(gUnitParams,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<UNITPARAMS>\r\n"));
		websWrite(wp, T("<StartFlashingYellowTime>%d</StartFlashingYellowTime>\r\n"), gUnitParams.iStartFlashingYellowTime);
		websWrite(wp, T("<StartAllRedTime>%d</StartAllRedTime>\r\n"), gUnitParams.iStartAllRedTime);
		websWrite(wp, T("<DegradationTime>%d</DegradationTime>\r\n"), gUnitParams.iDegradationTime);
		websWrite(wp, T("<SpeedFactor>%d</SpeedFactor>\r\n"), gUnitParams.iSpeedFactor);
		websWrite(wp, T("<MinimumRedLightTime>%d</MinimumRedLightTime>\r\n"), gUnitParams.iMinimumRedLightTime);
		websWrite(wp, T("<CommunicationTimeout>%d</CommunicationTimeout>\r\n"), gUnitParams.iCommunicationTimeout);
		websWrite(wp, T("<FlashingFrequency>%d</FlashingFrequency>\r\n"), gUnitParams.iFlashingFrequency);
		websWrite(wp, T("<TwiceCrossingTimeInterval>%d</TwiceCrossingTimeInterval>\r\n"), gUnitParams.iTwiceCrossingTimeInterval);
		websWrite(wp, T("<TwiceCrossingReverseTimeInterval>%d</TwiceCrossingReverseTimeInterval>\r\n"), gUnitParams.iTwiceCrossingReverseTimeInterval);
		websWrite(wp, T("<SmoothTransitionPeriod>%d</SmoothTransitionPeriod>\r\n"), gUnitParams.iSmoothTransitionPeriod);
		websWrite(wp, T("<FlowCollectionPeriod>%d</FlowCollectionPeriod>\r\n"), gUnitParams.iFlowCollectionPeriod);
		websWrite(wp, T("<CollectUnit>%d</CollectUnit>\r\n"), gUnitParams.iCollectUnit);
		websWrite(wp, T("<AutoPedestrianEmpty>%d</AutoPedestrianEmpty>\r\n"), gUnitParams.iAutoPedestrianEmpty);
		websWrite(wp, T("<OverpressureDetection>%d</OverpressureDetection>\r\n"), gUnitParams.iOverpressureDetection);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result>0?1:0);
		websWrite(wp, T("</UNITPARAMS>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 相位编辑处理函数
 * @author : liuxupu
 * @date : 2013/3/8 11:06
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void PhaseTableTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int account = 0;
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET命令，则把配置文件中获取的参数发给网页客户端	
		getXMLValue(query,"PhaseNo","PhaseAccount",tmpStr);
		account = atoi(tmpStr) - 1;
		result = getPhaseTableInfo(&gPhaseTable,"login.ini" ,account);
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<PhaseNo>\r\n"));
		websWrite(wp, T("<MinimumGreen>%d</MinimumGreen>\r\n"), gPhaseTable.iMinimumGreen);
		websWrite(wp, T("<MaximumGreenOne>%d</MaximumGreenOne>\r\n"), gPhaseTable.iMaximumGreenOne);
		websWrite(wp, T("<MaximumGreenTwo>%d</MaximumGreenTwo>\r\n"), gPhaseTable.iMaximumGreenTwo);
		websWrite(wp, T("<ExtensionGreen>%d</ExtensionGreen>\r\n"), gPhaseTable.iExtensionGreen);
		websWrite(wp, T("<MaximumRestrict>%d</MaximumRestrict>\r\n"), gPhaseTable.iMaximumRestrict);
		websWrite(wp, T("<DynamicStep>%d</DynamicStep>\r\n"), gPhaseTable.iDynamicStep);
		websWrite(wp, T("<YellowLightTime>%d</YellowLightTime>\r\n"), gPhaseTable.iYellowLightTime);
		websWrite(wp, T("<AllRedTime>%d</AllRedTime>\r\n"), gPhaseTable.iAllRedTime);
		websWrite(wp, T("<RedLightProtect>%d</RedLightProtect>\r\n"), gPhaseTable.iRedLightProtect);
		websWrite(wp, T("<IncreaseInitValue>%d</IncreaseInitValue>\r\n"), gPhaseTable.iIncreaseInitValue);
		websWrite(wp, T("<IncreaseInitialValueCalculation>%d</IncreaseInitialValueCalculation>\r\n"), gPhaseTable.iIncreaseInitialValueCalculation);
		websWrite(wp, T("<MaximumInitialValue>%d</MaximumInitialValue>\r\n"), gPhaseTable.iMaximumInitialValue);
		websWrite(wp, T("<TimeBeforeDecrease>%d</TimeBeforeDecrease>\r\n"), gPhaseTable.iTimeBeforeDecrease);
		websWrite(wp, T("<VehicleBeforeDecrease>%d</VehicleBeforeDecrease>\r\n"), gPhaseTable.iVehicleBeforeDecrease);
		websWrite(wp, T("<DecreaseTime>%d</DecreaseTime>\r\n"), gPhaseTable.iDecreaseTime);
		websWrite(wp, T("<UnitDeclineTime>%d</UnitDeclineTime>\r\n"), gPhaseTable.iUnitDeclineTime);
		websWrite(wp, T("<MinimumInterval>%d</MinimumInterval>\r\n"), gPhaseTable.iMinimumInterval);
		websWrite(wp, T("<PedestrianRelease>%d</PedestrianRelease>\r\n"), gPhaseTable.iPedestrianRelease);
		websWrite(wp, T("<PedestrianCleaned>%d</PedestrianCleaned>\r\n"), gPhaseTable.iPedestrianCleaned);
		websWrite(wp, T("<KeepPedestrianRelease>%d</KeepPedestrianRelease>\r\n"), gPhaseTable.iKeepPedestrianRelease);
		websWrite(wp, T("<NoLockDetentionRequest>%d</NoLockDetentionRequest>\r\n"), gPhaseTable.iNoLockDetentionRequest);
		websWrite(wp, T("<DoubleEntrancePhase>%d</DoubleEntrancePhase>\r\n"), gPhaseTable.iDoubleEntrancePhase);
		websWrite(wp, T("<GuaranteeFluxDensityExtensionGreen>%d</GuaranteeFluxDensityExtensionGreen>\r\n"), gPhaseTable.iGuaranteeFluxDensityExtensionGreen);
		websWrite(wp, T("<ConditionalServiceValid>%d</ConditionalServiceValid>\r\n"), gPhaseTable.iConditionalServiceValid);
		websWrite(wp, T("<MeanwhileEmptyLoseEfficacy>%d</MeanwhileEmptyLoseEfficacy>\r\n"), gPhaseTable.iMeanwhileEmptyLoseEfficacy);
		websWrite(wp, T("<Initialize>%d</Initialize>\r\n"), gPhaseTable.iInitialize);
		websWrite(wp, T("<NonInduction>%d</NonInduction>\r\n"), gPhaseTable.iNonInduction);
		websWrite(wp, T("<VehicleAutomaticRequest>%d</VehicleAutomaticRequest>\r\n"), gPhaseTable.iVehicleAutomaticRequest);
		websWrite(wp, T("<PedestrianAutomaticRequest>%d</PedestrianAutomaticRequest>\r\n"), gPhaseTable.iPedestrianAutomaticRequest);
		websWrite(wp, T("<AutomaticFlashInto>%d</AutomaticFlashInto>\r\n"), gPhaseTable.iAutomaticFlashInto);
		websWrite(wp, T("<AutomaticFlashExit>%d</AutomaticFlashExit>\r\n"), gPhaseTable.iAutomaticFlashExit);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</PhaseNo>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是POST命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回g
		memset(&gPhaseTable,0,sizeof(stPhaseTable));
		getXMLValue(query,"PhaseNo","PhaseAccount",tmpStr);
		account = atoi(tmpStr) - 1;
		getXMLValue(query,"PhaseNo","MinimumGreen",tmpStr);
		gPhaseTable.iMinimumGreen = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","MaximumGreenOne",tmpStr);
		gPhaseTable.iMaximumGreenOne = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","MaximumGreenTwo",tmpStr);
		gPhaseTable.iMaximumGreenTwo = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","ExtensionGreen",tmpStr);
		gPhaseTable.iExtensionGreen = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","MaximumRestrict",tmpStr);
		gPhaseTable.iMaximumRestrict = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","DynamicStep",tmpStr);
		gPhaseTable.iDynamicStep = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","YellowLightTime",tmpStr);
		gPhaseTable.iYellowLightTime = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","AllRedTime",tmpStr);
		gPhaseTable.iAllRedTime = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","RedLightProtect",tmpStr);
		gPhaseTable.iRedLightProtect = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","IncreaseInitValue",tmpStr);
		gPhaseTable.iIncreaseInitValue = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","IncreaseInitialValueCalculation",tmpStr);
		gPhaseTable.iIncreaseInitialValueCalculation = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","MaximumInitialValue",tmpStr);
		gPhaseTable.iMaximumInitialValue = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","TimeBeforeDecrease",tmpStr);
		gPhaseTable.iTimeBeforeDecrease = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","VehicleBeforeDecrease",tmpStr);
		gPhaseTable.iVehicleBeforeDecrease = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","DecreaseTime",tmpStr);
		gPhaseTable.iDecreaseTime = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","UnitDeclineTime",tmpStr);
		gPhaseTable.iUnitDeclineTime = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","MinimumInterval",tmpStr);
		gPhaseTable.iMinimumInterval = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","PedestrianRelease",tmpStr);
		gPhaseTable.iPedestrianRelease = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","PedestrianCleaned",tmpStr);
		gPhaseTable.iPedestrianCleaned = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","KeepPedestrianRelease",tmpStr);
		gPhaseTable.iKeepPedestrianRelease = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","NoLockDetentionRequest",tmpStr);
		gPhaseTable.iNoLockDetentionRequest = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","DoubleEntrancePhase",tmpStr);
		gPhaseTable.iDoubleEntrancePhase = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","GuaranteeFluxDensityExtensionGreen",tmpStr);
		gPhaseTable.iGuaranteeFluxDensityExtensionGreen = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","ConditionalServiceValid",tmpStr);
		gPhaseTable.iConditionalServiceValid = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","MeanwhileEmptyLoseEfficacy",tmpStr);
		gPhaseTable.iMeanwhileEmptyLoseEfficacy = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","Initialize",tmpStr);
		gPhaseTable.iInitialize = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","NonInduction",tmpStr);
		gPhaseTable.iNonInduction = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","VehicleAutomaticRequest",tmpStr);
		gPhaseTable.iVehicleAutomaticRequest = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","PedestrianAutomaticRequest",tmpStr);
		gPhaseTable.iPedestrianAutomaticRequest = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","AutomaticFlashInto",tmpStr);
		gPhaseTable.iAutomaticFlashInto = atoi(tmpStr);
		getXMLValue(query,"PhaseNo","AutomaticFlashExit",tmpStr);
		gPhaseTable.iAutomaticFlashExit = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = savePhaseTable2Ini(gPhaseTable,"login.ini", account);
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<PhaseNo>\r\n"));
		websWrite(wp, T("<MinimumGreen>%d</MinimumGreen>\r\n"), gPhaseTable.iMinimumGreen);
		websWrite(wp, T("<MaximumGreenOne>%d</MaximumGreenOne>\r\n"), gPhaseTable.iMaximumGreenOne);
		websWrite(wp, T("<MaximumGreenTwo>%d</MaximumGreenTwo>\r\n"), gPhaseTable.iMaximumGreenTwo);
		websWrite(wp, T("<ExtensionGreen>%d</ExtensionGreen>\r\n"), gPhaseTable.iExtensionGreen);
		websWrite(wp, T("<MaximumRestrict>%d</MaximumRestrict>\r\n"), gPhaseTable.iMaximumRestrict);
		websWrite(wp, T("<DynamicStep>%d</DynamicStep>\r\n"), gPhaseTable.iDynamicStep);
		websWrite(wp, T("<YellowLightTime>%d</YellowLightTime>\r\n"), gPhaseTable.iYellowLightTime);
		websWrite(wp, T("<AllRedTime>%d</AllRedTime>\r\n"), gPhaseTable.iAllRedTime);
		websWrite(wp, T("<RedLightProtect>%d</RedLightProtect>\r\n"), gPhaseTable.iRedLightProtect);
		websWrite(wp, T("<IncreaseInitValue>%d</IncreaseInitValue>\r\n"), gPhaseTable.iIncreaseInitValue);
		websWrite(wp, T("<IncreaseInitialValueCalculation>%d</IncreaseInitialValueCalculation>\r\n"), gPhaseTable.iIncreaseInitialValueCalculation);
		websWrite(wp, T("<MaximumInitialValue>%d</MaximumInitialValue>\r\n"), gPhaseTable.iMaximumInitialValue);
		websWrite(wp, T("<TimeBeforeDecrease>%d</TimeBeforeDecrease>\r\n"), gPhaseTable.iTimeBeforeDecrease);
		websWrite(wp, T("<VehicleBeforeDecrease>%d</VehicleBeforeDecrease>\r\n"), gPhaseTable.iVehicleBeforeDecrease);
		websWrite(wp, T("<DecreaseTime>%d</DecreaseTime>\r\n"), gPhaseTable.iDecreaseTime);
		websWrite(wp, T("<UnitDeclineTime>%d</UnitDeclineTime>\r\n"), gPhaseTable.iUnitDeclineTime);
		websWrite(wp, T("<MinimumInterval>%d</MinimumInterval>\r\n"), gPhaseTable.iMinimumInterval);
		websWrite(wp, T("<PedestrianRelease>%d</PedestrianRelease>\r\n"), gPhaseTable.iPedestrianRelease);
		websWrite(wp, T("<PedestrianCleaned>%d</PedestrianCleaned>\r\n"), gPhaseTable.iPedestrianCleaned);
		websWrite(wp, T("<KeepPedestrianRelease>%d</KeepPedestrianRelease>\r\n"), gPhaseTable.iKeepPedestrianRelease);
		websWrite(wp, T("<NoLockDetentionRequest>%d</NoLockDetentionRequest>\r\n"), gPhaseTable.iNoLockDetentionRequest);
		websWrite(wp, T("<DoubleEntrancePhase>%d</DoubleEntrancePhase>\r\n"), gPhaseTable.iDoubleEntrancePhase);
		websWrite(wp, T("<GuaranteeFluxDensityExtensionGreen>%d</GuaranteeFluxDensityExtensionGreen>\r\n"), gPhaseTable.iGuaranteeFluxDensityExtensionGreen);
		websWrite(wp, T("<ConditionalServiceValid>%d</ConditionalServiceValid>\r\n"), gPhaseTable.iConditionalServiceValid);
		websWrite(wp, T("<MeanwhileEmptyLoseEfficacy>%d</MeanwhileEmptyLoseEfficacy>\r\n"), gPhaseTable.iMeanwhileEmptyLoseEfficacy);
		websWrite(wp, T("<Initialize>%d</Initialize>\r\n"), gPhaseTable.iInitialize);
		websWrite(wp, T("<NonInduction>%d</NonInduction>\r\n"), gPhaseTable.iNonInduction);
		websWrite(wp, T("<VehicleAutomaticRequest>%d</VehicleAutomaticRequest>\r\n"), gPhaseTable.iVehicleAutomaticRequest);
		websWrite(wp, T("<PedestrianAutomaticRequest>%d</PedestrianAutomaticRequest>\r\n"), gPhaseTable.iPedestrianAutomaticRequest);
		websWrite(wp, T("<AutomaticFlashInto>%d</AutomaticFlashInto>\r\n"), gPhaseTable.iAutomaticFlashInto);
		websWrite(wp, T("<AutomaticFlashExit>%d</AutomaticFlashExit>\r\n"), gPhaseTable.iAutomaticFlashExit);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</PhaseNo>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}

/*****************************************************************************
 * @brief : 环/并发相位页面处理函数
 * @author : liuxupu
 * @date : 2013/3/29 
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void ringAndPhaseTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int phaseNum;
	char phaseNo[12] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getRingAndPhaseInfo(&gRingAndPhase,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<RingAndPhase>\r\n"));
		websWrite(wp, T("<ringNum>%d</ringNum>\r\n"), gRingAndPhase.iRingNum);
		websWrite(wp, T("<phaseNumAll>%d</phaseNumAll>\r\n"), gRingAndPhase.iPhaseNumAll);
		for(phaseNum=0;phaseNum<=31;phaseNum++)
		{
			websWrite(wp, T("<phaseNum%d>%d</phaseNum%d>\r\n"), phaseNum+1,gRingAndPhase.iPhaseNum[phaseNum],phaseNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</RingAndPhase>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gRingAndPhase,0,sizeof(stRingAndPhase));
		getXMLValue(query,"RingAndPhase","ringNum",tmpStr);
		gRingAndPhase.iRingNum = atoi(tmpStr);
		getXMLValue(query,"RingAndPhase","phaseNumAll",tmpStr);
		gRingAndPhase.iPhaseNumAll = atoi(tmpStr);
		for(phaseNum=0;phaseNum<=31;phaseNum++)
		{
			sprintf(phaseNo,"%s%d","phaseNum",phaseNum+1);
			getXMLValue(query,"RingAndPhase",phaseNo,tmpStr);
			gRingAndPhase.iPhaseNum[phaseNum] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveRingAndPhase2Ini(gRingAndPhase,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<RingAndPhase>\r\n"));
		websWrite(wp, T("<ringNum>%d</ringNum>\r\n"), gRingAndPhase.iRingNum);
		websWrite(wp, T("<phaseNumAll>%d</phaseNumAll>\r\n"), gRingAndPhase.iPhaseNumAll);
		for(phaseNum=0;phaseNum<=31;phaseNum++)
		{
			websWrite(wp, T("<phaseNum%d>%d</phaseNum%d>\r\n"), phaseNum+1,gRingAndPhase.iPhaseNum[phaseNum],phaseNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</RingAndPhase>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 通道表页面处理函数
 * @author : liuxupu
 * @date : 2013/4/02
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void channelTableTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	int account = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		getXMLValue(query,"ChannelTable","channelNum",tmpStr);
		account = atoi(tmpStr) - 1;
		result = getChannelTableInfo(&gChannelTable,"login.ini", account);
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<ChannelTable>\r\n"));
		websWrite(wp, T("<controlSource>%d</controlSource>\r\n"), gChannelTable.iControlSource);
		websWrite(wp, T("<controlType>%d</controlType>\r\n"), gChannelTable.iControlType);
		websWrite(wp, T("<flashMode>%d</flashMode>\r\n"), gChannelTable.iFlashMode);
		websWrite(wp, T("<brightMode>%d</brightMode>\r\n"), gChannelTable.iBrightMode);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</ChannelTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gChannelTable,0,sizeof(gChannelTable));
		getXMLValue(query,"ChannelTable","channelNum",tmpStr);
		account = atoi(tmpStr) - 1;
		getXMLValue(query,"ChannelTable","controlSource",tmpStr);
		gChannelTable.iControlSource = atoi(tmpStr);
		getXMLValue(query,"ChannelTable","controlType",tmpStr);
		gChannelTable.iControlType = atoi(tmpStr);
		getXMLValue(query,"ChannelTable","flashMode",tmpStr);
		gChannelTable.iFlashMode = atoi(tmpStr);
		getXMLValue(query,"ChannelTable","brightMode",tmpStr);
		gChannelTable.iBrightMode = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = saveChannelTable2Ini(gChannelTable,"login.ini", account);
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<ChannelTable>\r\n"));
		websWrite(wp, T("<controlSource>%d</controlSource>\r\n"), gChannelTable.iControlSource);
		websWrite(wp, T("<controlType>%d</controlType>\r\n"), gChannelTable.iControlType);
		websWrite(wp, T("<flashMode>%d</flashMode>\r\n"), gChannelTable.iFlashMode);
		websWrite(wp, T("<brightMode>%d</brightMode>\r\n"), gChannelTable.iBrightMode);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</ChannelTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 绿信比页面处理函数
 * @author : liuxupu
 * @date : 2013/4/03
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void greenRatioTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getGreenRatioInfo(&gGreenRatio,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<GreenRatio>\r\n"));
		websWrite(wp, T("<greenNum>%d</greenNum>\r\n"), gGreenRatio.greenNum);
		websWrite(wp, T("<split>%d</split>\r\n"), gGreenRatio.split);
		websWrite(wp, T("<greenType>%d</greenType>\r\n"), gGreenRatio.greenType);
		websWrite(wp, T("<coordinatePhase>%d</coordinatePhase>\r\n"), gGreenRatio.coordinatePhase);
		websWrite(wp, T("<keyPhase>%d</keyPhase>\r\n"), gGreenRatio.keyPhase);
		websWrite(wp, T("<fixedPhase>%d</fixedPhase>\r\n"), gGreenRatio.fixedPhase);
		websWrite(wp, T("<pedestrianMove>%d</pedestrianMove>\r\n"), gGreenRatio.pedestrianMove);
		websWrite(wp, T("<pedestrianEmpty>%d</pedestrianEmpty>\r\n"), gGreenRatio.pedestrianEmpty);
		websWrite(wp, T("<frontierGreen>%d</frontierGreen>\r\n"), gGreenRatio.frontierGreen);
		websWrite(wp, T("<emptyType>%d</emptyType>\r\n"), gGreenRatio.emptyType);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</GreenRatio>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gGreenRatio,0,sizeof(gGreenRatio));
		getXMLValue(query,"GreenRatio","greenNum",tmpStr);
		gGreenRatio.greenNum = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","split",tmpStr);
		gGreenRatio.split = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","greenType",tmpStr);
		gGreenRatio.greenType = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","coordinatePhase",tmpStr);
		gGreenRatio.coordinatePhase = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","keyPhase",tmpStr);
		gGreenRatio.keyPhase = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","fixedPhase",tmpStr);
		gGreenRatio.fixedPhase = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","pedestrianMove",tmpStr);
		gGreenRatio.pedestrianMove = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","pedestrianEmpty",tmpStr);
		gGreenRatio.pedestrianEmpty = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","frontierGreen",tmpStr);
		gGreenRatio.frontierGreen = atoi(tmpStr);
		getXMLValue(query,"GreenRatio","emptyType",tmpStr);
		gGreenRatio.emptyType = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = saveGreenRatio2Ini(gGreenRatio,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<GreenRatio>\r\n"));
		websWrite(wp, T("<greenNum>%d</greenNum>\r\n"), gGreenRatio.greenNum);
		websWrite(wp, T("<split>%d</split>\r\n"), gGreenRatio.split);
		websWrite(wp, T("<greenType>%d</greenType>\r\n"), gGreenRatio.greenType);
		websWrite(wp, T("<coordinatePhase>%d</coordinatePhase>\r\n"), gGreenRatio.coordinatePhase);
		websWrite(wp, T("<keyPhase>%d</keyPhase>\r\n"), gGreenRatio.keyPhase);
		websWrite(wp, T("<fixedPhase>%d</fixedPhase>\r\n"), gGreenRatio.fixedPhase);
		websWrite(wp, T("<pedestrianMove>%d</pedestrianMove>\r\n"), gGreenRatio.pedestrianMove);
		websWrite(wp, T("<pedestrianEmpty>%d</pedestrianEmpty>\r\n"), gGreenRatio.pedestrianEmpty);
		websWrite(wp, T("<frontierGreen>%d</frontierGreen>\r\n"), gGreenRatio.frontierGreen);
		websWrite(wp, T("<emptyType>%d</emptyType>\r\n"), gGreenRatio.emptyType);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</GreenRatio>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 故障检测设置页面处理函数
 * @author : liuxupu
 * @date : 2013/4/8 
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void faultDetectionSetTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int ChanNum;
	char strChan0[12], strChan1[12], strChan2[12];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getFaultDetectionSetInfo(&gFaultDetectionSet,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<FaultDetectionSet>\r\n"));
		websWrite(wp, T("<VoltageDetectionTimes>%d</VoltageDetectionTimes>\r\n"), gFaultDetectionSet.VoltageDetectionTimes);
		websWrite(wp, T("<RedLightDetectionTimes>%d</RedLightDetectionTimes>\r\n"), gFaultDetectionSet.RedLightDetectionTimes);
		websWrite(wp, T("<ConflictDetectionAttempts>%d</ConflictDetectionAttempts>\r\n"), gFaultDetectionSet.ConflictDetectionAttempts);
		websWrite(wp, T("<ManualPanelKeyNumber>%d</ManualPanelKeyNumber>\r\n"), gFaultDetectionSet.ManualPanelKeyNumber);
		websWrite(wp, T("<RemoteControlKeyNumber>%d</RemoteControlKeyNumber>\r\n"), gFaultDetectionSet.RemoteControlKeyNumber);
		websWrite(wp, T("<SenseSwitch>%d</SenseSwitch>\r\n"), gFaultDetectionSet.SenseSwitch);
		websWrite(wp, T("<DynamicStep>%d</DynamicStep>\r\n"), gFaultDetectionSet.DynamicStep);
		websWrite(wp, T("<CurrentFaultDetection>%d</CurrentFaultDetection>\r\n"), gFaultDetectionSet.CurrentFaultDetection);
		websWrite(wp, T("<AlarmAndFaultCurrent>%d</AlarmAndFaultCurrent>\r\n"), gFaultDetectionSet.AlarmAndFaultCurrent);
		websWrite(wp, T("<AlarmAndFaultVoltage>%d</AlarmAndFaultVoltage>\r\n"), gFaultDetectionSet.AlarmAndFaultVoltage);
		websWrite(wp, T("<EnableWatchdog>%d</EnableWatchdog>\r\n"), gFaultDetectionSet.EnableWatchdog);
		for(ChanNum=0;ChanNum<=31;ChanNum++)
		{
			websWrite(wp, T("<CNum%d_0>%d</CNum%d_0>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][0],ChanNum+1);
			websWrite(wp, T("<CNum%d_1>%d</CNum%d_1>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][1],ChanNum+1);
			websWrite(wp, T("<CNum%d_2>%d</CNum%d_2>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][2],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</FaultDetectionSet>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gFaultDetectionSet,0,sizeof(stFaultDetectionSet));
		getXMLValue(query,"FaultDetectionSet","VoltageDetectionTimes",tmpStr);
		gFaultDetectionSet.VoltageDetectionTimes = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","RedLightDetectionTimes",tmpStr);
		gFaultDetectionSet.RedLightDetectionTimes = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","ConflictDetectionAttempts",tmpStr);
		gFaultDetectionSet.ConflictDetectionAttempts = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","ManualPanelKeyNumber",tmpStr);
		gFaultDetectionSet.ManualPanelKeyNumber = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","RemoteControlKeyNumber",tmpStr);
		gFaultDetectionSet.RemoteControlKeyNumber = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","SenseSwitch",tmpStr);
		gFaultDetectionSet.SenseSwitch = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","DynamicStep",tmpStr);
		gFaultDetectionSet.DynamicStep = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","CurrentFaultDetection",tmpStr);
		gFaultDetectionSet.CurrentFaultDetection = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","AlarmAndFaultCurrent",tmpStr);
		gFaultDetectionSet.AlarmAndFaultCurrent = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","AlarmAndFaultVoltage",tmpStr);
		gFaultDetectionSet.AlarmAndFaultVoltage = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","EnableWatchdog",tmpStr);
		gFaultDetectionSet.EnableWatchdog = atoi(tmpStr);
		for(ChanNum=0;ChanNum<=31;ChanNum++)
		{
			sprintf(strChan0,"%s%d_0","CNum",ChanNum+1);
			sprintf(strChan1,"%s%d_1","CNum",ChanNum+1);
			sprintf(strChan2,"%s%d_2","CNum",ChanNum+1);
			getXMLValue(query,"FaultDetectionSet",strChan0,tmpStr);
			gFaultDetectionSet.CNum[ChanNum][0] = atoi(tmpStr);
			getXMLValue(query,"FaultDetectionSet",strChan1,tmpStr);
			gFaultDetectionSet.CNum[ChanNum][1] = atoi(tmpStr);
			getXMLValue(query,"FaultDetectionSet",strChan2,tmpStr);
			gFaultDetectionSet.CNum[ChanNum][2] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveFaultDetectionSet2Ini(gFaultDetectionSet,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<FaultDetectionSet>\r\n"));
		websWrite(wp, T("<VoltageDetectionTimes>%d</VoltageDetectionTimes>\r\n"), gFaultDetectionSet.VoltageDetectionTimes);
		websWrite(wp, T("<RedLightDetectionTimes>%d</RedLightDetectionTimes>\r\n"), gFaultDetectionSet.RedLightDetectionTimes);
		websWrite(wp, T("<ConflictDetectionAttempts>%d</ConflictDetectionAttempts>\r\n"), gFaultDetectionSet.ConflictDetectionAttempts);
		websWrite(wp, T("<ManualPanelKeyNumber>%d</ManualPanelKeyNumber>\r\n"), gFaultDetectionSet.ManualPanelKeyNumber);
		websWrite(wp, T("<RemoteControlKeyNumber>%d</RemoteControlKeyNumber>\r\n"), gFaultDetectionSet.RemoteControlKeyNumber);
		websWrite(wp, T("<SenseSwitch>%d</SenseSwitch>\r\n"), gFaultDetectionSet.SenseSwitch);
		websWrite(wp, T("<DynamicStep>%d</DynamicStep>\r\n"), gFaultDetectionSet.DynamicStep);
		websWrite(wp, T("<CurrentFaultDetection>%d</CurrentFaultDetection>\r\n"), gFaultDetectionSet.CurrentFaultDetection);
		websWrite(wp, T("<AlarmAndFaultCurrent>%d</AlarmAndFaultCurrent>\r\n"), gFaultDetectionSet.AlarmAndFaultCurrent);
		websWrite(wp, T("<AlarmAndFaultVoltage>%d</AlarmAndFaultVoltage>\r\n"), gFaultDetectionSet.AlarmAndFaultVoltage);
		websWrite(wp, T("<EnableWatchdog>%d</EnableWatchdog>\r\n"), gFaultDetectionSet.EnableWatchdog);
		for(ChanNum=0;ChanNum<=31;ChanNum++)
		{
			websWrite(wp, T("<CNum%d_0>%d</CNum%d_0>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][0],ChanNum+1);
			websWrite(wp, T("<CNum%d_1>%d</CNum%d_1>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][1],ChanNum+1);
			websWrite(wp, T("<CNum%d_2>%d</CNum%d_2>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][2],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</FaultDetectionSet>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 相序表页面处理函数
 * @author : liuxupu
 * @date : 2013/4/9
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void sequenceTableTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int ChanNum;
	char seqNum1[12], seqNum2[12], seqNum3[12],seqNum4[12];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getSequenceTableInfo(&gSequenceTable,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<SequenceTable>\r\n"));
		websWrite(wp, T("<SequenceTableNo>%d</SequenceTableNo>\r\n"), gSequenceTable.SequenceTableNo);
		for(ChanNum=0;ChanNum<=15;ChanNum++)
		{
			websWrite(wp, T("<SNum1_%d>%d</SNum1_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[0][ChanNum],ChanNum+1);
			websWrite(wp, T("<SNum2_%d>%d</SNum2_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[1][ChanNum],ChanNum+1);
			websWrite(wp, T("<SNum3_%d>%d</SNum3_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[2][ChanNum],ChanNum+1);
			websWrite(wp, T("<SNum4_%d>%d</SNum4_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[3][ChanNum],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</SequenceTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gSequenceTable,0,sizeof(stSequenceTable));
		getXMLValue(query,"SequenceTable","SequenceTableNo",tmpStr);
		gSequenceTable.SequenceTableNo = atoi(tmpStr);
		for(ChanNum=0;ChanNum<=15;ChanNum++)
		{
			sprintf(seqNum1,"%s1_%d","SNum",ChanNum+1);
			sprintf(seqNum2,"%s2_%d","SNum",ChanNum+1);
			sprintf(seqNum3,"%s3_%d","SNum",ChanNum+1);
			sprintf(seqNum4,"%s4_%d","SNum",ChanNum+1);
			getXMLValue(query,"SequenceTable",seqNum1,tmpStr);
			gSequenceTable.SNum[0][ChanNum] = atoi(tmpStr);
			getXMLValue(query,"SequenceTable",seqNum2,tmpStr);
			gSequenceTable.SNum[1][ChanNum] = atoi(tmpStr);
			getXMLValue(query,"SequenceTable",seqNum3,tmpStr);
			gSequenceTable.SNum[2][ChanNum] = atoi(tmpStr);
			getXMLValue(query,"SequenceTable",seqNum4,tmpStr);
			gSequenceTable.SNum[3][ChanNum] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveSequenceTable2Ini(gSequenceTable,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<SequenceTable>\r\n"));
		websWrite(wp, T("<SequenceTableNo>%d</SequenceTableNo>\r\n"), gSequenceTable.SequenceTableNo);
		for(ChanNum=0;ChanNum<=15;ChanNum++)
		{
			websWrite(wp, T("<SNum1_%d>%d</SNum1_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[0][ChanNum],ChanNum+1);
			websWrite(wp, T("<SNum2_%d>%d</SNum2_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[1][ChanNum],ChanNum+1);
			websWrite(wp, T("<SNum3_%d>%d</SNum3_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[2][ChanNum],ChanNum+1);
			websWrite(wp, T("<SNum4_%d>%d</SNum4_%d>\r\n"), ChanNum+1,gSequenceTable.SNum[3][ChanNum],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</SequenceTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 方案表页面处理函数
 * @author : liuxupu
 * @date : 2013/4/9
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void programTableTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	char seqNum1[12], seqNum2[12], seqNum3[12],seqNum4[12];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	int ChanNum;
	int account = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端	
		getXMLValue(query,"ProgramTable","ProgramTableNo",tmpStr);
		account = atoi(tmpStr) - 1;
		result = getProgramTableInfo(&gProgramTable,"login.ini", account);
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<ProgramTable>\r\n"));
		for(ChanNum=0;ChanNum<=3;ChanNum++)
		{
			websWrite(wp, T("<PNum1_%d>%d</PNum1_%d>\r\n"), ChanNum+1,gProgramTable.PNum[0][ChanNum],ChanNum+1);
			websWrite(wp, T("<PNum2_%d>%d</PNum2_%d>\r\n"), ChanNum+1,gProgramTable.PNum[1][ChanNum],ChanNum+1);
			websWrite(wp, T("<PNum3_%d>%d</PNum3_%d>\r\n"), ChanNum+1,gProgramTable.PNum[2][ChanNum],ChanNum+1);
			websWrite(wp, T("<PNum4_%d>%d</PNum4_%d>\r\n"), ChanNum+1,gProgramTable.PNum[3][ChanNum],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</ProgramTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gProgramTable,0,sizeof(stProgramTable));
		getXMLValue(query,"ProgramTable","ProgramTableNo",tmpStr);
		account = atoi(tmpStr) - 1;
		for(ChanNum=0;ChanNum<=3;ChanNum++)
		{
			sprintf(seqNum1,"%s1_%d","SNum",ChanNum+1);
			sprintf(seqNum2,"%s2_%d","SNum",ChanNum+1);
			sprintf(seqNum3,"%s3_%d","SNum",ChanNum+1);
			sprintf(seqNum4,"%s4_%d","SNum",ChanNum+1);
			getXMLValue(query,"ProgramTable",seqNum1,tmpStr);
			gProgramTable.PNum[0][ChanNum] = atoi(tmpStr);
			getXMLValue(query,"ProgramTable",seqNum2,tmpStr);
			gProgramTable.PNum[1][ChanNum] = atoi(tmpStr);
			getXMLValue(query,"ProgramTable",seqNum3,tmpStr);
			gProgramTable.PNum[2][ChanNum] = atoi(tmpStr);
			getXMLValue(query,"ProgramTable",seqNum4,tmpStr);
			gProgramTable.PNum[3][ChanNum] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveProgramTable2Ini(gProgramTable,"login.ini", account);
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<ProgramTable>\r\n"));
		for(ChanNum=0;ChanNum<=3;ChanNum++)
		{
			websWrite(wp, T("<PNum1_%d>%d</PNum1_%d>\r\n"), ChanNum+1,gProgramTable.PNum[0][ChanNum],ChanNum+1);
			websWrite(wp, T("<PNum2_%d>%d</PNum2_%d>\r\n"), ChanNum+1,gProgramTable.PNum[1][ChanNum],ChanNum+1);
			websWrite(wp, T("<PNum3_%d>%d</PNum3_%d>\r\n"), ChanNum+1,gProgramTable.PNum[2][ChanNum],ChanNum+1);
			websWrite(wp, T("<PNum4_%d>%d</PNum4_%d>\r\n"), ChanNum+1,gProgramTable.PNum[3][ChanNum],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</ProgramTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 时基动作表页面处理函数
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void timeBasedActionTableTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int ChanNum;
	char seqNum[20];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getTimeBasedActionTableInfo(&gTimeBasedActionTable,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<TimeBasedActionTable>\r\n"));
		websWrite(wp, T("<ActionTable>%d</ActionTable>\r\n"), gTimeBasedActionTable.ActionTable);
		websWrite(wp, T("<ProgramNo>%d</ProgramNo>\r\n"), gTimeBasedActionTable.ProgramNo);
		for(ChanNum=0;ChanNum<=3;ChanNum++)
		{
			websWrite(wp, T("<AssistFunction%d>%d</AssistFunction%d>\r\n"), ChanNum+1,gTimeBasedActionTable.AssistFunction[ChanNum],ChanNum+1);
		}
		for(ChanNum=0;ChanNum<=7;ChanNum++)
		{
			websWrite(wp, T("<SpecialFunction%d>%d</SpecialFunction%d>\r\n"), ChanNum+1,gTimeBasedActionTable.SpecialFunction[ChanNum],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</TimeBasedActionTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gTimeBasedActionTable,0,sizeof(stTimeBasedActionTable));
		getXMLValue(query,"TimeBasedActionTable","ActionTable",tmpStr);
		gTimeBasedActionTable.ActionTable = atoi(tmpStr);
		getXMLValue(query,"TimeBasedActionTable","ProgramNo",tmpStr);
		gTimeBasedActionTable.ProgramNo = atoi(tmpStr);
		for(ChanNum=0;ChanNum<=3;ChanNum++)
		{
			sprintf(seqNum,"%s%d","AssistFunction",ChanNum+1);
			getXMLValue(query,"TimeBasedActionTable",seqNum,tmpStr);
			gTimeBasedActionTable.AssistFunction[ChanNum] = atoi(tmpStr);
		}
		for(ChanNum=0;ChanNum<=7;ChanNum++)
		{
			sprintf(seqNum,"%s%d","SpecialFunction",ChanNum+1);
			getXMLValue(query,"TimeBasedActionTable",seqNum,tmpStr);
			gTimeBasedActionTable.SpecialFunction[ChanNum] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveTimeBasedActionTable2Ini(gTimeBasedActionTable,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<TimeBasedActionTable>\r\n"));
		websWrite(wp, T("<ActionTable>%d</ActionTable>\r\n"), gTimeBasedActionTable.ActionTable);
		websWrite(wp, T("<ProgramNo>%d</ProgramNo>\r\n"), gTimeBasedActionTable.ProgramNo);
		for(ChanNum=0;ChanNum<=3;ChanNum++)
		{
			websWrite(wp, T("<AssistFunction%d>%d</AssistFunction%d>\r\n"), ChanNum+1,gTimeBasedActionTable.AssistFunction[ChanNum],ChanNum+1);
		}
		for(ChanNum=0;ChanNum<=7;ChanNum++)
		{
			websWrite(wp, T("<SpecialFunction%d>%d</SpecialFunction%d>\r\n"), ChanNum+1,gTimeBasedActionTable.SpecialFunction[ChanNum],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</TimeBasedActionTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 时段页面处理函数
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void timeIntervalTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int Tcount;
	char timeNum1[20],timeNum2[20],timeNum3[20];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getTimeIntervalInfo(&gTimeInterval,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<TimeInterval>\r\n"));
		websWrite(wp, T("<TimeIntervalNo>%d</TimeIntervalNo>\r\n"), gTimeInterval.TimeIntervalNo);
		for(Tcount=0;Tcount<=3;Tcount++)
		{
			websWrite(wp, T("<Time%d_1>%d</Time%d_1>\r\n"), Tcount+1,gTimeInterval.Time[Tcount][0],Tcount+1);
			websWrite(wp, T("<Time%d_2>%d</Time%d_2>\r\n"), Tcount+1,gTimeInterval.Time[Tcount][1],Tcount+1);
			websWrite(wp, T("<Time%d_3>%d</Time%d_3>\r\n"), Tcount+1,gTimeInterval.Time[Tcount][2],Tcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</TimeInterval>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gTimeInterval,0,sizeof(stTimeInterval));
		getXMLValue(query,"TimeInterval","TimeIntervalNo",tmpStr);
		gTimeInterval.TimeIntervalNo = atoi(tmpStr);
		for(Tcount=0;Tcount<=3;Tcount++)
		{
			sprintf(timeNum1,"%s%d_1","Time",Tcount+1);
			sprintf(timeNum2,"%s%d_2","Time",Tcount+1);
			sprintf(timeNum3,"%s%d_3","Time",Tcount+1);
			getXMLValue(query,"TimeInterval",timeNum1,tmpStr);
			gTimeInterval.Time[Tcount][0] = atoi(tmpStr);
			getXMLValue(query,"TimeInterval",timeNum2,tmpStr);
			gTimeInterval.Time[Tcount][1] = atoi(tmpStr);
			getXMLValue(query,"TimeInterval",timeNum3,tmpStr);
			gTimeInterval.Time[Tcount][2] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveTimeInterval2Ini(gTimeInterval,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<TimeInterval>\r\n"));
		websWrite(wp, T("<TimeIntervalNo>%d</TimeIntervalNo>\r\n"), gTimeInterval.TimeIntervalNo);
		for(Tcount=0;Tcount<=3;Tcount++)
		{
			websWrite(wp, T("<Time%d_1>%d</Time%d_1>\r\n"), Tcount+1,gTimeInterval.Time[Tcount][0],Tcount+1);
			websWrite(wp, T("<Time%d_2>%d</Time%d_2>\r\n"), Tcount+1,gTimeInterval.Time[Tcount][1],Tcount+1);
			websWrite(wp, T("<Time%d_3>%d</Time%d_3>\r\n"), Tcount+1,gTimeInterval.Time[Tcount][2],Tcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</TimeInterval>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 调度计划页面处理函数
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void schedulingTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int Tcount;
	char MDW1[20],MDW2[20],MDW3[20];                    //MDW:Month,Day,WeekDay
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getSchedulingInfo(&gScheduling,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Scheduling>\r\n"));
		websWrite(wp, T("<SchedulingNo>%d</SchedulingNo>\r\n"), gScheduling.SchedulingNo);
		websWrite(wp, T("<TimeIntervalNum>%d</TimeIntervalNum>\r\n"), gScheduling.TimeIntervalNum);
		for(Tcount=0;Tcount<=12;Tcount++)
		{
			websWrite(wp, T("<Month%d>%d</Month%d>\r\n"), Tcount+1,gScheduling.Month[Tcount],Tcount+1);
		}
		for(Tcount=0;Tcount<=31;Tcount++)
		{
			websWrite(wp, T("<Day%d>%d</Day%d>\r\n"), Tcount+1,gScheduling.Day[Tcount],Tcount+1);
		}
		for(Tcount=0;Tcount<=7;Tcount++)
		{
			websWrite(wp, T("<WeekDay%d>%d</WeekDay%d>\r\n"), Tcount+1,gScheduling.WeekDay[Tcount],Tcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Scheduling>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gScheduling,0,sizeof(stScheduling));
		getXMLValue(query,"Scheduling","SchedulingNo",tmpStr);
		gScheduling.SchedulingNo = atoi(tmpStr);
		getXMLValue(query,"Scheduling","TimeIntervalNum",tmpStr);
		gScheduling.TimeIntervalNum = atoi(tmpStr);
		for(Tcount=0;Tcount<=12;Tcount++)
		{
			sprintf(MDW1,"%s%d","Month",Tcount+1);
			getXMLValue(query,"Scheduling",MDW1,tmpStr);
			gScheduling.Month[Tcount] = atoi(tmpStr);
		}
		for(Tcount=0;Tcount<=31;Tcount++)
		{
			sprintf(MDW2,"%s%d","Day",Tcount+1);
			getXMLValue(query,"Scheduling",MDW2,tmpStr);
			gScheduling.Day[Tcount] = atoi(tmpStr);
		}
		for(Tcount=0;Tcount<=7;Tcount++)
		{
			sprintf(MDW3,"%s%d","WeekDay",Tcount+1);
			getXMLValue(query,"Scheduling",MDW3,tmpStr);
			gScheduling.WeekDay[Tcount] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveScheduling2Ini(gScheduling,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Scheduling>\r\n"));
		websWrite(wp, T("<SchedulingNo>%d</SchedulingNo>\r\n"), gScheduling.SchedulingNo);
		websWrite(wp, T("<TimeIntervalNum>%d</TimeIntervalNum>\r\n"), gScheduling.TimeIntervalNum);
		for(Tcount=0;Tcount<=12;Tcount++)
		{
			websWrite(wp, T("<Month%d>%d</Month%d>\r\n"), Tcount+1,gScheduling.Month[Tcount],Tcount+1);
		}
		for(Tcount=0;Tcount<=31;Tcount++)
		{
			websWrite(wp, T("<Day%d>%d</Day%d>\r\n"), Tcount+1,gScheduling.Day[Tcount],Tcount+1);
		}
		for(Tcount=0;Tcount<=7;Tcount++)
		{
			websWrite(wp, T("<WeekDay%d>%d</WeekDay%d>\r\n"), Tcount+1,gScheduling.WeekDay[Tcount],Tcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Scheduling>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 重叠表页面处理函数
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void overlappingTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int Overcount;
	char PPhase[20];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getOverlappingInfo(&gOverlapping,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Overlapping>\r\n"));
		websWrite(wp, T("<FollowPhase>%d</FollowPhase>\r\n"), gOverlapping.FollowPhase);
		websWrite(wp, T("<GreenLight>%d</GreenLight>\r\n"), gOverlapping.GreenLight);
		websWrite(wp, T("<RedLight>%d</RedLight>\r\n"), gOverlapping.RedLight);
		websWrite(wp, T("<YellowLight>%d</YellowLight>\r\n"), gOverlapping.YellowLight);
		websWrite(wp, T("<GreenFlash>%d</GreenFlash>\r\n"), gOverlapping.GreenFlash);
		websWrite(wp, T("<ModifiedPhase>%d</ModifiedPhase>\r\n"), gOverlapping.ModifiedPhase);
		for(Overcount=0;Overcount<=31;Overcount++)
		{
			websWrite(wp, T("<ParentPhase%d>%d</ParentPhase%d>\r\n"), Overcount+1,gOverlapping.ParentPhase[Overcount],Overcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Overlapping>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gScheduling,0,sizeof(stScheduling));
		getXMLValue(query,"Overlapping","FollowPhase",tmpStr);
		gOverlapping.FollowPhase = atoi(tmpStr);
		getXMLValue(query,"Overlapping","GreenLight",tmpStr);
		gOverlapping.GreenLight = atoi(tmpStr);
		getXMLValue(query,"Overlapping","RedLight",tmpStr);
		gOverlapping.RedLight = atoi(tmpStr);
		getXMLValue(query,"Overlapping","YellowLight",tmpStr);
		gOverlapping.YellowLight = atoi(tmpStr);
		getXMLValue(query,"Overlapping","GreenFlash",tmpStr);
		gOverlapping.GreenFlash = atoi(tmpStr);
		getXMLValue(query,"Overlapping","ModifiedPhase",tmpStr);
		gOverlapping.ModifiedPhase = atoi(tmpStr);
		for(Overcount=0;Overcount<=31;Overcount++)
		{
			sprintf(PPhase,"%s%d","ParentPhase",Overcount+1);
			getXMLValue(query,"Overlapping",PPhase,tmpStr);
			gOverlapping.ParentPhase[Overcount] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = saveOverlapping2Ini(gOverlapping,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Overlapping>\r\n"));
		websWrite(wp, T("<FollowPhase>%d</FollowPhase>\r\n"), gOverlapping.FollowPhase);
		websWrite(wp, T("<GreenLight>%d</GreenLight>\r\n"), gOverlapping.GreenLight);
		websWrite(wp, T("<RedLight>%d</RedLight>\r\n"), gOverlapping.RedLight);
		websWrite(wp, T("<YellowLight>%d</YellowLight>\r\n"), gOverlapping.YellowLight);
		websWrite(wp, T("<GreenFlash>%d</GreenFlash>\r\n"), gOverlapping.GreenFlash);
		websWrite(wp, T("<ModifiedPhase>%d</ModifiedPhase>\r\n"), gOverlapping.ModifiedPhase);
		for(Overcount=0;Overcount<=31;Overcount++)
		{
			websWrite(wp, T("<ParentPhase%d>%d</ParentPhase%d>\r\n"), Overcount+1,gOverlapping.ParentPhase[Overcount],Overcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Overlapping>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 协调页面处理函数
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void coordinateTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getCoordinateInfo(&gCoordinate,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Coordinate>\r\n"));
		websWrite(wp, T("<ControlModel>%d</ControlModel>\r\n"), gCoordinate.ControlModel);
		websWrite(wp, T("<ManualMethod>%d</ManualMethod>\r\n"), gCoordinate.ManualMethod);
		websWrite(wp, T("<CoordinationMode>%d</CoordinationMode>\r\n"), gCoordinate.CoordinationMode);
		websWrite(wp, T("<CoordinateMaxMode>%d</CoordinateMaxMode>\r\n"), gCoordinate.CoordinateMaxMode);
		websWrite(wp, T("<CoordinateForceMode>%d</CoordinateForceMode>\r\n"), gCoordinate.CoordinateForceMode);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Coordinate>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gCoordinate,0,sizeof(stCoordinate));
		getXMLValue(query,"Coordinate","ControlModel",tmpStr);
		gCoordinate.ControlModel = atoi(tmpStr);
		getXMLValue(query,"Coordinate","ManualMethod",tmpStr);
		gCoordinate.ManualMethod = atoi(tmpStr);
		getXMLValue(query,"Coordinate","CoordinationMode",tmpStr);
		gCoordinate.CoordinationMode = atoi(tmpStr);
		getXMLValue(query,"Coordinate","CoordinateMaxMode",tmpStr);
		gCoordinate.CoordinateMaxMode = atoi(tmpStr);
		getXMLValue(query,"Coordinate","CoordinateForceMode",tmpStr);
		gCoordinate.CoordinateForceMode = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = saveCoordinate2Ini(gCoordinate,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Coordinate>\r\n"));
		websWrite(wp, T("<ControlModel>%d</ControlModel>\r\n"), gCoordinate.ControlModel);
		websWrite(wp, T("<ManualMethod>%d</ManualMethod>\r\n"), gCoordinate.ManualMethod);
		websWrite(wp, T("<CoordinationMode>%d</CoordinationMode>\r\n"), gCoordinate.CoordinationMode);
		websWrite(wp, T("<CoordinateMaxMode>%d</CoordinateMaxMode>\r\n"), gCoordinate.CoordinateMaxMode);
		websWrite(wp, T("<CoordinateForceMode>%d</CoordinateForceMode>\r\n"), gCoordinate.CoordinateForceMode);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Coordinate>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 车辆检测器页面处理函数
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void vehicleDetectorTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getVehicleDetectorInfo(&gVehicleDetector,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<VehicleDetector>\r\n"));
		websWrite(wp, T("<DetectorNo>%d</DetectorNo>\r\n"), gVehicleDetector.DetectorNo);
		websWrite(wp, T("<RequestPhase>%d</RequestPhase>\r\n"), gVehicleDetector.RequestPhase);
		websWrite(wp, T("<SwitchPhase>%d</SwitchPhase>\r\n"), gVehicleDetector.SwitchPhase);
		websWrite(wp, T("<Delay>%d</Delay>\r\n"), gVehicleDetector.Delay);
		websWrite(wp, T("<FailureTime>%d</FailureTime>\r\n"), gVehicleDetector.FailureTime);
		websWrite(wp, T("<QueueLimit>%d</QueueLimit>\r\n"), gVehicleDetector.QueueLimit);
		websWrite(wp, T("<NoResponseTime>%d</NoResponseTime>\r\n"), gVehicleDetector.NoResponseTime);
		websWrite(wp, T("<MaxDuration>%d</MaxDuration>\r\n"), gVehicleDetector.MaxDuration);
		websWrite(wp, T("<Extend>%d</Extend>\r\n"), gVehicleDetector.Extend);
		websWrite(wp, T("<MaxVehicle>%d</MaxVehicle>\r\n"), gVehicleDetector.MaxVehicle);
		websWrite(wp, T("<Flow>%d</Flow>\r\n"), gVehicleDetector.Flow);
		websWrite(wp, T("<Occupancy>%d</Occupancy>\r\n"), gVehicleDetector.Occupancy);
		websWrite(wp, T("<ProlongGreen>%d</ProlongGreen>\r\n"), gVehicleDetector.ProlongGreen);
		websWrite(wp, T("<AccumulateInitial>%d</AccumulateInitial>\r\n"), gVehicleDetector.AccumulateInitial);
		websWrite(wp, T("<Queue>%d</Queue>\r\n"), gVehicleDetector.Queue);
		websWrite(wp, T("<Request>%d</Request>\r\n"), gVehicleDetector.Request);
		websWrite(wp, T("<RedInterval>%d</RedInterval>\r\n"), gVehicleDetector.RedInterval);
		websWrite(wp, T("<YellowInterval>%d</YellowInterval>\r\n"), gVehicleDetector.YellowInterval);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</VehicleDetector>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gVehicleDetector,0,sizeof(stVehicleDetector));
		getXMLValue(query,"VehicleDetector","DetectorNo",tmpStr);
		gVehicleDetector.DetectorNo = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","RequestPhase",tmpStr);
		gVehicleDetector.RequestPhase = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","SwitchPhase",tmpStr);
		gVehicleDetector.SwitchPhase = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","Delay",tmpStr);
		gVehicleDetector.Delay = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","FailureTime",tmpStr);
		gVehicleDetector.FailureTime = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","QueueLimit",tmpStr);
		gVehicleDetector.QueueLimit = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","NoResponseTime",tmpStr);
		gVehicleDetector.NoResponseTime = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","MaxDuration",tmpStr);
		gVehicleDetector.MaxDuration = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","Extend",tmpStr);
		gVehicleDetector.Extend = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","MaxVehicle",tmpStr);
		gVehicleDetector.MaxVehicle = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","Flow",tmpStr);
		gVehicleDetector.Flow = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","Occupancy",tmpStr);
		gVehicleDetector.Occupancy = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","ProlongGreen",tmpStr);
		gVehicleDetector.ProlongGreen = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","AccumulateInitial",tmpStr);
		gVehicleDetector.AccumulateInitial = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","Queue",tmpStr);
		gVehicleDetector.Queue = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","Request",tmpStr);
		gVehicleDetector.Request = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","RedInterval",tmpStr);
		gVehicleDetector.RedInterval = atoi(tmpStr);
		getXMLValue(query,"VehicleDetector","YellowInterval",tmpStr);
		gVehicleDetector.YellowInterval = atoi(tmpStr);

		//把单元参数保存到指定ini配置文件中
		result = saveVehicleDetector2Ini(gVehicleDetector,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<VehicleDetector>\r\n"));
		websWrite(wp, T("<DetectorNo>%d</DetectorNo>\r\n"), gVehicleDetector.DetectorNo);
		websWrite(wp, T("<RequestPhase>%d</RequestPhase>\r\n"), gVehicleDetector.RequestPhase);
		websWrite(wp, T("<SwitchPhase>%d</SwitchPhase>\r\n"), gVehicleDetector.SwitchPhase);
		websWrite(wp, T("<Delay>%d</Delay>\r\n"), gVehicleDetector.Delay);
		websWrite(wp, T("<FailureTime>%d</FailureTime>\r\n"), gVehicleDetector.FailureTime);
		websWrite(wp, T("<QueueLimit>%d</QueueLimit>\r\n"), gVehicleDetector.QueueLimit);
		websWrite(wp, T("<NoResponseTime>%d</NoResponseTime>\r\n"), gVehicleDetector.NoResponseTime);
		websWrite(wp, T("<MaxDuration>%d</MaxDuration>\r\n"), gVehicleDetector.MaxDuration);
		websWrite(wp, T("<Extend>%d</Extend>\r\n"), gVehicleDetector.Extend);
		websWrite(wp, T("<MaxVehicle>%d</MaxVehicle>\r\n"), gVehicleDetector.MaxVehicle);
		websWrite(wp, T("<Flow>%d</Flow>\r\n"), gVehicleDetector.Flow);
		websWrite(wp, T("<Occupancy>%d</Occupancy>\r\n"), gVehicleDetector.Occupancy);
		websWrite(wp, T("<ProlongGreen>%d</ProlongGreen>\r\n"), gVehicleDetector.ProlongGreen);
		websWrite(wp, T("<AccumulateInitial>%d</AccumulateInitial>\r\n"), gVehicleDetector.AccumulateInitial);
		websWrite(wp, T("<Queue>%d</Queue>\r\n"), gVehicleDetector.Queue);
		websWrite(wp, T("<Request>%d</Request>\r\n"), gVehicleDetector.Request);
		websWrite(wp, T("<RedInterval>%d</RedInterval>\r\n"), gVehicleDetector.RedInterval);
		websWrite(wp, T("<YellowInterval>%d</YellowInterval>\r\n"), gVehicleDetector.YellowInterval);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</VehicleDetector>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 行人检测器页面处理函数
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void pedestrianDetectorTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getPedestrianInfo(&gPedestrian,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<PedestrianDetector>\r\n"));
		websWrite(wp, T("<DetectorNo>%d</DetectorNo>\r\n"), gPedestrian.DetectorNo);
		websWrite(wp, T("<RequestPhase>%d</RequestPhase>\r\n"), gPedestrian.RequestPhase);
		websWrite(wp, T("<NoResponseTime>%d</NoResponseTime>\r\n"), gPedestrian.NoResponseTime);
		websWrite(wp, T("<MaxDuration>%d</MaxDuration>\r\n"), gPedestrian.MaxDuration);
		websWrite(wp, T("<InductionNumber>%d</InductionNumber>\r\n"), gPedestrian.InductionNumber);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</PedestrianDetector>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gPedestrian,0,sizeof(stPedestrian));
		getXMLValue(query,"PedestrianDetector","DetectorNo",tmpStr);
		gPedestrian.DetectorNo = atoi(tmpStr);
		getXMLValue(query,"PedestrianDetector","RequestPhase",tmpStr);
		gPedestrian.RequestPhase = atoi(tmpStr);
		getXMLValue(query,"PedestrianDetector","NoResponseTime",tmpStr);
		gPedestrian.NoResponseTime = atoi(tmpStr);
		getXMLValue(query,"PedestrianDetector","MaxDuration",tmpStr);
		gPedestrian.MaxDuration = atoi(tmpStr);
		getXMLValue(query,"PedestrianDetector","InductionNumber",tmpStr);
		gPedestrian.InductionNumber = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = savePedestrian2Ini(gPedestrian,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<PedestrianDetector>\r\n"));
		websWrite(wp, T("<DetectorNo>%d</DetectorNo>\r\n"), gPedestrian.DetectorNo);
		websWrite(wp, T("<RequestPhase>%d</RequestPhase>\r\n"), gPedestrian.RequestPhase);
		websWrite(wp, T("<NoResponseTime>%d</NoResponseTime>\r\n"), gPedestrian.NoResponseTime);
		websWrite(wp, T("<MaxDuration>%d</MaxDuration>\r\n"), gPedestrian.MaxDuration);
		websWrite(wp, T("<InductionNumber>%d</InductionNumber>\r\n"), gPedestrian.InductionNumber);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</PedestrianDetector>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 故障配置页面处理函数
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void faultConfigTest(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getFaultConfigInfo(&gFaultConfig,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<FaultConfig>\r\n"));
		websWrite(wp, T("<ControlRecord>%d</ControlRecord>\r\n"), gFaultConfig.ControlRecord);
		websWrite(wp, T("<CommunicatRecord>%d</CommunicatRecord>\r\n"), gFaultConfig.CommunicatRecord);
		websWrite(wp, T("<DetectorRecord>%d</DetectorRecord>\r\n"), gFaultConfig.DetectorRecord);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</FaultConfig>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gFaultConfig,0,sizeof(stFaultConfig));
		getXMLValue(query,"FaultConfig","ControlRecord",tmpStr);
		gFaultConfig.ControlRecord = atoi(tmpStr);
		getXMLValue(query,"FaultConfig","CommunicatRecord",tmpStr);
		gFaultConfig.CommunicatRecord = atoi(tmpStr);
		getXMLValue(query,"FaultConfig","DetectorRecord",tmpStr);
		gFaultConfig.DetectorRecord = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = saveFaultConfig2Ini(gFaultConfig,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<FaultConfig>\r\n"));
		websWrite(wp, T("<ControlRecord>%d</ControlRecord>\r\n"), gFaultConfig.ControlRecord);
		websWrite(wp, T("<CommunicatRecord>%d</CommunicatRecord>\r\n"), gFaultConfig.CommunicatRecord);
		websWrite(wp, T("<DetectorRecord>%d</DetectorRecord>\r\n"), gFaultConfig.DetectorRecord);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</FaultConfig>\r\n"));;
		websFooter(wp);
		websDone(wp, 200);
	}
}
/*****************************************************************************
 * @brief : 参数下载处理函数
 * @author : liuxupu
 * @date : 2013/5/28 10:33
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void actionDownload(webs_t wp, char_t *path, char_t *query)
{
	char buffer[1024*10];
	const char * file = "login.ini";
	int result = 0;
    FILE *stream = NULL;

    stream = fopen(file, "r");
	result = fread(buffer,1,strlen(buffer),stream);
    fclose(stream);
	buffer[result] = '\0';
	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<SaveParams>\r\n"));
	websWrite(wp, T("<download>%s</download>\r\n"), buffer);
	websWrite(wp, T("<statusCode>1</statusCode>\r\n"));
	websWrite(wp, T("</SaveParams>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}
/*****************************************************************************
 * @brief : 树动态参数处理函数
 * @author : liuxupu
 * @date : 2013/4/27
 * @version : ver 1.0
 * @inparam : webs_t wp, char_t *path, char_t *query
* @outparam : 空
*****************************************************************************/
static void TreeDynamicParameter(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端		
		result = getTreeDynamicParameter(&gTreeDynamicPara,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<TreeDynamicParameter>\r\n"));
		websWrite(wp, T("<addCount>%d</addCount>\r\n"), gTreeDynamicPara.addCount);
		websWrite(wp, T("<addChannel>%d</addChannel>\r\n"), gTreeDynamicPara.addChannel);
		websWrite(wp, T("<addProgram>%d</addProgram>\r\n"), gTreeDynamicPara.addProgram);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</TreeDynamicParameter>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}	
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gTreeDynamicPara,0,sizeof(stTreeDynamicPara));
		getXMLValue(query,"TreeDynamicParameter","addCount",tmpStr);
		gTreeDynamicPara.addCount = atoi(tmpStr);
		getXMLValue(query,"TreeDynamicParameter","addChannel",tmpStr);
		gTreeDynamicPara.addChannel = atoi(tmpStr);
		getXMLValue(query,"TreeDynamicParameter","addProgram",tmpStr);
		gTreeDynamicPara.addProgram = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = saveTreeDynamicParameter(gTreeDynamicPara,"login.ini");
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<TreeDynamicParameter>\r\n"));
		websWrite(wp, T("<addCount>%d</addCount>\r\n"), gTreeDynamicPara.addCount);
		websWrite(wp, T("<addChannel>%d</addChannel>\r\n"), gTreeDynamicPara.addChannel);
		websWrite(wp, T("<addProgram>%d</addProgram>\r\n"), gTreeDynamicPara.addProgram);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</TreeDynamicParameter>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}
static char * getXMLValue(char * XMLStr,char * rootTag,char * rootTagName,char * returnValue)
{
	//char returnValue[256] = { 0 };
	int len;
    char *pRes;
    UTIL_XML_REQ req;
    UTIL_XML_TAG *tag;
    
    util_xml_init(&req);
    if((len=util_xml_validate(&req, XMLStr, strlen(XMLStr))) < 0)
    {
        util_xml_cleanup(&req);
        return "error";
    }
    tag = req.root_tag;
    if(!tag || util_xml_matchtag(rootTag, tag->name) != 0)
    {
        util_xml_cleanup(&req);
        return "error";
    }
	tag = tag->first_child;
	if(!tag)
    {
        util_xml_cleanup(&req);
        return "error";
    }
	while(tag)
	{
		if(strcmp((const char *)rootTagName, tag->name) ==0)
		{
			if(tag->value)
			{
				strcpy(returnValue,tag->value);
			}
			else
			{
				//如果没有值传过来，则去空值
				strcpy(returnValue,"");
			}
			break;
		}
		tag = tag->next;
	}
    util_xml_cleanup(&req);
	return returnValue;
}


/*****************************************************************************
 * @brief : web登录校验用户名密码函数
 * @author : liujie
 * @date : 2013/1/29 18:28
 * @version : ver 1.0
 * @inparam : 网页发送的用户名和密码
 * @outparam : 如果都正确，则返回1；如果用户名不对，则返回-1；密码不对，返回-2；如果读写文件异常，则返回0
 *****************************************************************************/
static int getLoginInfo(char_t * username,char_t * password)
{
	//读取配置文件参数
	const char * file = "login.ini";
	char * section = "LOGIN";
	char value[BUF_SIZE] = "";
	char realUsername[16] = { 0 };
	char realPassword[16] = { 0 };
	char loginUsername[16] = { 0 };
	char loginPassword[16] = { 0 };
	memcpy(loginUsername,username,16);
	memcpy(loginPassword,password,16);
	if(!read_profile_string("LOGIN","username",value,BUF_SIZE,"","login.ini"))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		memcpy(realUsername,value,16);
	}
	if(!read_profile_string("LOGIN","password",value,BUF_SIZE,"","login.ini"))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		memcpy(realPassword,value,16);
	}
	if (strcmp(loginUsername,realUsername) == 0 && strcmp(loginPassword,realPassword) == 0)
	{
		return 1;
	}
	else if(strcmp(loginUsername,realUsername) != 0)
	{
		return -1;
	}
	else if(strcmp(loginUsername,realUsername) == 0 && strcmp(loginPassword,realPassword) != 0)
	{
		return -2;
	}

	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取单元参数
 * @author : liujie
 * @date : 2013/1/29 18:28
 * @version : ver 1.0
 * @inparam : 网页发送的用户名和密码
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getUnitParamsInfo(stUnitParams *pstUnitParams,char *strConfigPath)
{
	//读取配置文件参数
	const char * section = "UNITPARAMS";
	char value[BUF_SIZE] = "";
	memset(pstUnitParams,0,sizeof(stUnitParams));
	if(!read_profile_string(section,"StartFlashingYellowTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iStartFlashingYellowTime = atoi(value);
	}
	if(!read_profile_string(section,"StartAllRedTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iStartAllRedTime = atoi(value);
	}
	if(!read_profile_string(section,"DegradationTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iDegradationTime = atoi(value);
	}
	if(!read_profile_string(section,"SpeedFactor",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iSpeedFactor = atoi(value);
	}
	if(!read_profile_string(section,"MinimumRedLightTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iMinimumRedLightTime = atoi(value);
	}
	if(!read_profile_string(section,"CommunicationTimeout",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iCommunicationTimeout = atoi(value);
	}
	if(!read_profile_string(section,"FlashingFrequency",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iFlashingFrequency = atoi(value);
	}
	if(!read_profile_string(section,"TwiceCrossingTimeInterval",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iTwiceCrossingTimeInterval = atoi(value);
	}
	if(!read_profile_string(section,"TwiceCrossingReverseTimeInterval",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iTwiceCrossingReverseTimeInterval = atoi(value);
	}
	if(!read_profile_string(section,"SmoothTransitionPeriod",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iSmoothTransitionPeriod = atoi(value);
	}
	if(!read_profile_string(section,"FlowCollectionPeriod",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iFlowCollectionPeriod = atoi(value);
	}
	if(!read_profile_string(section,"CollectUnit",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iCollectUnit = atoi(value);
	}
	if(!read_profile_string(section,"AutoPedestrianEmpty",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iAutoPedestrianEmpty = atoi(value);
	}
	if(!read_profile_string(section,"OverpressureDetection",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstUnitParams->iOverpressureDetection = atoi(value);
	}
	
	return 1;
}

/*****************************************************************************
 * @brief : 设置单元参数到指定ini文件中
 * @author : liujie
 * @date : 2013/2/1 
 * @version : ver 1.0
 * @inparam : 网页发送的用户名和密码
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveUnitParams2Ini(stUnitParams stUnitParamsEx,char * strConfigPath)
{
	const char * section = "UNITPARAMS";
	char tmpstr[48] = { 0 };
	itoa(stUnitParamsEx.iStartFlashingYellowTime,tmpstr,10);
	if(write_profile_string(section, "StartFlashingYellowTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iStartAllRedTime,tmpstr,10);
	if(write_profile_string(section, "StartAllRedTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iDegradationTime,tmpstr,10);
	if(write_profile_string(section, "DegradationTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iSpeedFactor,tmpstr,10);
	if(write_profile_string(section, "SpeedFactor",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iMinimumRedLightTime,tmpstr,10);
	if(write_profile_string(section, "MinimumRedLightTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iCommunicationTimeout,tmpstr,10);
	if(write_profile_string(section, "CommunicationTimeout",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iFlashingFrequency,tmpstr,10);
	if(write_profile_string(section, "FlashingFrequency",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iTwiceCrossingTimeInterval,tmpstr,10);
	if(write_profile_string(section, "TwiceCrossingTimeInterval",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iTwiceCrossingReverseTimeInterval,tmpstr,10);
	if(write_profile_string(section, "TwiceCrossingReverseTimeInterval",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iSmoothTransitionPeriod,tmpstr,10);
	if(write_profile_string(section, "SmoothTransitionPeriod",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iFlowCollectionPeriod,tmpstr,10);
	if(write_profile_string(section, "FlowCollectionPeriod",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iCollectUnit,tmpstr,10);
	if(write_profile_string(section, "CollectUnit",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iAutoPedestrianEmpty,tmpstr,10);
	if(write_profile_string(section, "AutoPedestrianEmpty",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stUnitParamsEx.iOverpressureDetection,tmpstr,10);
	if(write_profile_string(section, "OverpressureDetection",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取相位信息
 * @author : liuxupu
 * @date : 2013/3/8
 * @version : ver 1.0
 * @inparam : 网页发送的用户名和密码
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getPhaseTableInfo(stPhaseTable *pstPhaseTable,char *strConfigPath, int i)
{
	//读取配置文件参数
	char temp[24] = {0};
	const char * section = NULL;
	char value[BUF_SIZE] = "";
	sprintf(temp,"%02dPhaseNo",i);
	section = temp;
	memset(pstPhaseTable,0,sizeof(stPhaseTable));
	if(!read_profile_string(section,"MinimumGreen",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iMinimumGreen = atoi(value);
	}
	if(!read_profile_string(section,"MaximumGreenOne",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iMaximumGreenOne = atoi(value);
	}
	if(!read_profile_string(section,"MaximumGreenTwo",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iMaximumGreenTwo = atoi(value);
	}
	if(!read_profile_string(section,"ExtensionGreen",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iExtensionGreen = atoi(value);
	}
	if(!read_profile_string(section,"MaximumRestrict",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iMaximumRestrict = atoi(value);
	}
	if(!read_profile_string(section,"DynamicStep",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iDynamicStep = atoi(value);
	}
	if(!read_profile_string(section,"YellowLightTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iYellowLightTime = atoi(value);
	}
	if(!read_profile_string(section,"AllRedTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iAllRedTime = atoi(value);
	}
	if(!read_profile_string(section,"RedLightProtect",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iRedLightProtect = atoi(value);
	}
	if(!read_profile_string(section,"IncreaseInitValue",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iIncreaseInitValue = atoi(value);
	}
	if(!read_profile_string(section,"IncreaseInitialValueCalculation",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iIncreaseInitialValueCalculation = atoi(value);
	}
	if(!read_profile_string(section,"MaximumInitialValue",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iMaximumInitialValue = atoi(value);
	}
	if(!read_profile_string(section,"TimeBeforeDecrease",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iTimeBeforeDecrease = atoi(value);
	}
	if(!read_profile_string(section,"VehicleBeforeDecrease",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iVehicleBeforeDecrease = atoi(value);
	}
	if(!read_profile_string(section,"DecreaseTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iDecreaseTime = atoi(value);
	}
	if(!read_profile_string(section,"UnitDeclineTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iUnitDeclineTime = atoi(value);
	}
	if(!read_profile_string(section,"MinimumInterval",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iMinimumInterval = atoi(value);
	}
	if(!read_profile_string(section,"PedestrianRelease",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iPedestrianRelease= atoi(value);
	}
	if(!read_profile_string(section,"PedestrianCleaned",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iPedestrianCleaned = atoi(value);
	}
	if(!read_profile_string(section,"KeepPedestrianRelease",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iKeepPedestrianRelease = atoi(value);
	}
	if(!read_profile_string(section,"NoLockDetentionRequest",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iNoLockDetentionRequest = atoi(value);
	}
	if(!read_profile_string(section,"DoubleEntrancePhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iDoubleEntrancePhase = atoi(value);
	}
	if(!read_profile_string(section,"GuaranteeFluxDensityExtensionGreen",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iGuaranteeFluxDensityExtensionGreen = atoi(value);
	}
	if(!read_profile_string(section,"ConditionalServiceValid",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iConditionalServiceValid = atoi(value);
	}
	if(!read_profile_string(section,"MeanwhileEmptyLoseEfficacy",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iMeanwhileEmptyLoseEfficacy = atoi(value);
	}
	if(!read_profile_string(section,"Initialize",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iInitialize = atoi(value);
	}
	if(!read_profile_string(section,"NonInduction",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iNonInduction = atoi(value);
	}
	if(!read_profile_string(section,"VehicleAutomaticRequest",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iVehicleAutomaticRequest = atoi(value);
	}
	if(!read_profile_string(section,"PedestrianAutomaticRequest",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iPedestrianAutomaticRequest = atoi(value);
	}
	if(!read_profile_string(section,"AutomaticFlashInto",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iAutomaticFlashInto = atoi(value);
	}
	if(!read_profile_string(section,"AutomaticFlashExit",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPhaseTable->iAutomaticFlashExit = atoi(value);
	}
	
	return 1;
}

/*****************************************************************************
 * @brief : 设置相位表参数到指定ini文件中
 * @author : liuxupu
 * @date : 2013/3/8
 * @version : ver 1.0
 * @inparam : 网页配置的相位参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int savePhaseTable2Ini(stPhaseTable stPhaseTableEx,char * strConfigPath, int i)
{
	char temp[24] = {0};
	char tmpstr[48] = { 0 };
	const char * section = NULL;
	sprintf(temp,"%02dPhaseNo",i);
	section = temp;
	
	itoa(stPhaseTableEx.iMinimumGreen,tmpstr,10);
	if(write_profile_string(section, "MinimumGreen",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iMaximumGreenOne,tmpstr,10);
	if(write_profile_string(section, "MaximumGreenOne",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iMaximumGreenTwo,tmpstr,10);
	if(write_profile_string(section, "MaximumGreenTwo",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iExtensionGreen,tmpstr,10);
	if(write_profile_string(section, "ExtensionGreen",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iMaximumRestrict,tmpstr,10);
	if(write_profile_string(section, "MaximumRestrict",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iDynamicStep,tmpstr,10);
	if(write_profile_string(section, "DynamicStep",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iYellowLightTime,tmpstr,10);
	if(write_profile_string(section, "YellowLightTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iAllRedTime,tmpstr,10);
	if(write_profile_string(section, "AllRedTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iRedLightProtect,tmpstr,10);
	if(write_profile_string(section, "RedLightProtect",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iIncreaseInitValue,tmpstr,10);
	if(write_profile_string(section, "IncreaseInitValue",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iIncreaseInitialValueCalculation,tmpstr,10);
	if(write_profile_string(section, "IncreaseInitialValueCalculation",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iMaximumInitialValue,tmpstr,10);
	if(write_profile_string(section, "MaximumInitialValue",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iTimeBeforeDecrease,tmpstr,10);
	if(write_profile_string(section, "TimeBeforeDecrease",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iVehicleBeforeDecrease,tmpstr,10);
	if(write_profile_string(section, "VehicleBeforeDecrease",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iDecreaseTime,tmpstr,10);
	if(write_profile_string(section, "DecreaseTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iUnitDeclineTime,tmpstr,10);
	if(write_profile_string(section, "UnitDeclineTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iMinimumInterval,tmpstr,10);
	if(write_profile_string(section, "MinimumInterval",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iPedestrianRelease,tmpstr,10);
	if(write_profile_string(section, "PedestrianRelease",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iPedestrianCleaned,tmpstr,10);
	if(write_profile_string(section, "PedestrianCleaned",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iKeepPedestrianRelease,tmpstr,10);
	if(write_profile_string(section, "KeepPedestrianRelease",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iNoLockDetentionRequest,tmpstr,10);
	if(write_profile_string(section, "NoLockDetentionRequest",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iDoubleEntrancePhase,tmpstr,10);
	if(write_profile_string(section, "DoubleEntrancePhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iGuaranteeFluxDensityExtensionGreen,tmpstr,10);
	if(write_profile_string(section, "GuaranteeFluxDensityExtensionGreen",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iConditionalServiceValid,tmpstr,10);
	if(write_profile_string(section, "ConditionalServiceValid",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iMeanwhileEmptyLoseEfficacy,tmpstr,10);
	if(write_profile_string(section, "MeanwhileEmptyLoseEfficacy",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iInitialize,tmpstr,10);
	if(write_profile_string(section, "Initialize",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iNonInduction,tmpstr,10);
	if(write_profile_string(section, "NonInduction",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iVehicleAutomaticRequest,tmpstr,10);
	if(write_profile_string(section, "VehicleAutomaticRequest",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iPedestrianAutomaticRequest,tmpstr,10);
	if(write_profile_string(section, "PedestrianAutomaticRequest",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iAutomaticFlashInto,tmpstr,10);
	if(write_profile_string(section, "AutomaticFlashInto",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPhaseTableEx.iAutomaticFlashExit,tmpstr,10);
	if(write_profile_string(section, "AutomaticFlashExit",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取环/并发相位
 * @author : liuxupu
 * @date : 2013/3/29
 * @version : ver 1.0
 * @inparam : 网页发送的环/并发相位参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getRingAndPhaseInfo(stRingAndPhase *pstRingAndPhase,char *strConfigPath)
{
	//读取配置文件参数
	const char * section = "RingAndPhase";
	char value[BUF_SIZE] = "";
	int i;
	char sRingNum[12];
	
	memset(pstRingAndPhase,0,sizeof(stRingAndPhase));
	if(!read_profile_string(section,"ringNum",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstRingAndPhase->iRingNum = atoi(value);
	}
	if(!read_profile_string(section,"phaseNumAll",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstRingAndPhase->iPhaseNumAll = atoi(value);
	}
	for(i=0;i<=31;i++)
	{
		sprintf(sRingNum,"%s%02d","phaseNum",i+1);
		if(!read_profile_string(section,sRingNum,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstRingAndPhase->iPhaseNum[i] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置环/并发相位到指定ini文件中
 * @author : liuxupu
 * @date : 2013/3/29
 * @version : ver 1.0
 * @inparam : 网页发送的环/并发相位参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveRingAndPhase2Ini(stRingAndPhase stRingAndPhaseEx,char * strConfigPath)
{
	const char * section = "RingAndPhase";
	char tmpstr[48] = { 0 };
	int i;
	char sPhaseNum[12];
	itoa(stRingAndPhaseEx.iRingNum,tmpstr,10);
	if(write_profile_string(section, "ringNum",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stRingAndPhaseEx.iPhaseNumAll,tmpstr,10);
	if(write_profile_string(section, "phaseNumAll",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	for(i=0;i<=31;i++)
	{
		sprintf(sPhaseNum,"%s%02d","phaseNum",i+1);
		itoa(stRingAndPhaseEx.iPhaseNum[i],tmpstr,10);
		if(write_profile_string(section,sPhaseNum,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取通道表参数
 * @author : liuxupu
 * @date : 2013/4/02 
 * @version : ver 1.0
 * @inparam : 网页的通道表get参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getChannelTableInfo(stChannelTable *pstChannelTable,char *strConfigPath, int i)
{
	//读取配置文件参数
	char temp[24] = {0};
	const char * section = NULL;
	char value[BUF_SIZE] = "";
	sprintf(temp,"%02dChannelTable",i);
	section = temp;
	memset(pstChannelTable,0,sizeof(stChannelTable));
	if(!read_profile_string(section,"controlSource",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstChannelTable->iControlSource = atoi(value);
	}
	if(!read_profile_string(section,"controlType",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstChannelTable->iControlType = atoi(value);
	}
	if(!read_profile_string(section,"flashMode",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstChannelTable->iFlashMode = atoi(value);
	}
	if(!read_profile_string(section,"brightMode",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstChannelTable->iBrightMode = atoi(value);
	}
	return 1;
}

/*****************************************************************************
 * @brief : 设置通道表参数到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/2
 * @version : ver 1.0
 * @inparam : 网页发送的通道表set
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveChannelTable2Ini(stChannelTable stChannelTableEx,char * strConfigPath, int i)
{
	char temp[24] = {0};
	char tmpstr[48] = { 0 };
	const char * section = NULL;
	sprintf(temp,"%02dChannelTable",i);
	section = temp;
	itoa(stChannelTableEx.iControlSource,tmpstr,10);
	if(write_profile_string(section, "controlSource",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stChannelTableEx.iControlType,tmpstr,10);
	if(write_profile_string(section, "controlType",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stChannelTableEx.iFlashMode,tmpstr,10);
	if(write_profile_string(section, "flashMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stChannelTableEx.iBrightMode,tmpstr,10);
	if(write_profile_string(section, "brightMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取绿信比参数
 * @author : liuxupu
 * @date : 2013/4/7
 * @version : ver 1.0
 * @inparam : 网页的通道表get参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getGreenRatioInfo(stGreenRatio *pstGreenRatio,char *strConfigPath)
{
	//读取配置文件参数
	const char * section = "GreenRatio";
	char value[BUF_SIZE] = "";
	memset(pstGreenRatio,0,sizeof(stGreenRatio));
	if(!read_profile_string(section,"greenNum",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->greenNum = atoi(value);
	}
	if(!read_profile_string(section,"split",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->split = atoi(value);
	}
	if(!read_profile_string(section,"greenType",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->greenType = atoi(value);
	}
	if(!read_profile_string(section,"coordinatePhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->coordinatePhase = atoi(value);
	}
	if(!read_profile_string(section,"keyPhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->keyPhase = atoi(value);
	}
	if(!read_profile_string(section,"fixedPhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->fixedPhase = atoi(value);
	}
	if(!read_profile_string(section,"pedestrianMove",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->pedestrianMove = atoi(value);
	}
	if(!read_profile_string(section,"pedestrianEmpty",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->pedestrianEmpty = atoi(value);
	}
	if(!read_profile_string(section,"frontierGreen",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->frontierGreen = atoi(value);
	}
	if(!read_profile_string(section,"emptyType",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstGreenRatio->emptyType = atoi(value);
	}		
	return 1;
}

/*****************************************************************************
 * @brief : 设置绿信比参数到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/7
 * @version : ver 1.0
 * @inparam : 网页发送的通道表set
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveGreenRatio2Ini(stGreenRatio stGreenRatioEx,char * strConfigPath)
{
	const char * section = "GreenRatio";
	char tmpstr[48] = { 0 };
	itoa(stGreenRatioEx.greenNum,tmpstr,10);
	if(write_profile_string(section, "greenNum",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.split,tmpstr,10);
	if(write_profile_string(section, "split",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.greenType,tmpstr,10);
	if(write_profile_string(section, "greenType",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.coordinatePhase,tmpstr,10);
	if(write_profile_string(section, "coordinatePhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.keyPhase,tmpstr,10);
	if(write_profile_string(section, "keyPhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.fixedPhase,tmpstr,10);
	if(write_profile_string(section, "fixedPhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.pedestrianMove,tmpstr,10);
	if(write_profile_string(section, "pedestrianMove",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.pedestrianEmpty,tmpstr,10);
	if(write_profile_string(section, "pedestrianEmpty",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.frontierGreen,tmpstr,10);
	if(write_profile_string(section, "frontierGreen",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stGreenRatioEx.emptyType,tmpstr,10);
	if(write_profile_string(section, "emptyType",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}	
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取故障检测设置
 * @author : liuxupu
 * @date : 2013/4/8
 * @version : ver 1.0
 * @inparam : 网页发送的故障诊断设置参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getFaultDetectionSetInfo(stFaultDetectionSet *pstFaultDetectionSet,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "FaultDetectionSet";
	char value[BUF_SIZE] = "";
	int i;
	char sChanNum0[12],sChanNum1[12],sChanNum2[12];
	
	memset(pstFaultDetectionSet,0,sizeof(stFaultDetectionSet));
	if(!read_profile_string(section,"VoltageDetectionTimes",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->VoltageDetectionTimes = atoi(value);
	}
	if(!read_profile_string(section,"RedLightDetectionTimes",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->RedLightDetectionTimes = atoi(value);
	}
	if(!read_profile_string(section,"ConflictDetectionAttempts",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->ConflictDetectionAttempts = atoi(value);
	}
	if(!read_profile_string(section,"ManualPanelKeyNumber",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->ManualPanelKeyNumber = atoi(value);
	}
		if(!read_profile_string(section,"RemoteControlKeyNumber",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->RemoteControlKeyNumber = atoi(value);
	}
	if(!read_profile_string(section,"SenseSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->SenseSwitch = atoi(value);
	}
	if(!read_profile_string(section,"DynamicStep",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->DynamicStep = atoi(value);
	}
	if(!read_profile_string(section,"CurrentFaultDetection",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->CurrentFaultDetection = atoi(value);
	}
	if(!read_profile_string(section,"AlarmAndFaultCurrent",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->AlarmAndFaultCurrent = atoi(value);
	}
	if(!read_profile_string(section,"AlarmAndFaultVoltage",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->AlarmAndFaultVoltage = atoi(value);
	}
	if(!read_profile_string(section,"EnableWatchdog",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultDetectionSet->EnableWatchdog = atoi(value);
	}
	for(i=0;i<=31;i++)
	{
		sprintf(sChanNum0,"%s%02d_0","CNum",i+1);
		sprintf(sChanNum1,"%s%02d_1","CNum",i+1);
		sprintf(sChanNum2,"%s%02d_2","CNum",i+1);
		if(!read_profile_string(section,sChanNum0,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstFaultDetectionSet->CNum[i][0] = atoi(value);
		}
		if(!read_profile_string(section,sChanNum1,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstFaultDetectionSet->CNum[i][1] = atoi(value);
		}
		if(!read_profile_string(section,sChanNum2,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstFaultDetectionSet->CNum[i][2] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置故障检测设置到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/8
 * @version : ver 1.0
 * @inparam : 网页发送的故障检测设置参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveFaultDetectionSet2Ini(stFaultDetectionSet stFaultDetectionSetEx,char * strConfigPath)
{
	const char * section = "FaultDetectionSet";
	char tmpstr[48] = { 0 };
	int i;
	char sPhaseNum0[12],sPhaseNum1[12],sPhaseNum2[12];
	itoa(stFaultDetectionSetEx.VoltageDetectionTimes,tmpstr,10);
	if(write_profile_string(section, "VoltageDetectionTimes",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.RedLightDetectionTimes,tmpstr,10);
	if(write_profile_string(section, "RedLightDetectionTimes",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.ConflictDetectionAttempts,tmpstr,10);
	if(write_profile_string(section, "ConflictDetectionAttempts",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.ManualPanelKeyNumber,tmpstr,10);
	if(write_profile_string(section, "ManualPanelKeyNumber",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.RemoteControlKeyNumber,tmpstr,10);
	if(write_profile_string(section, "RemoteControlKeyNumber",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.SenseSwitch,tmpstr,10);
	if(write_profile_string(section, "SenseSwitch",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.DynamicStep,tmpstr,10);
	if(write_profile_string(section, "DynamicStep",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.CurrentFaultDetection,tmpstr,10);
	if(write_profile_string(section, "CurrentFaultDetection",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.AlarmAndFaultCurrent,tmpstr,10);
	if(write_profile_string(section, "AlarmAndFaultCurrent",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.AlarmAndFaultVoltage,tmpstr,10);
	if(write_profile_string(section, "AlarmAndFaultVoltage",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultDetectionSetEx.EnableWatchdog,tmpstr,10);
	if(write_profile_string(section, "EnableWatchdog",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	for(i=0;i<=31;i++)
	{
		sprintf(sPhaseNum0,"%s%02d_0","CNum",i+1);
		sprintf(sPhaseNum1,"%s%02d_1","CNum",i+1);
		sprintf(sPhaseNum2,"%s%02d_2","CNum",i+1);
		itoa(stFaultDetectionSetEx.CNum[i][0],tmpstr,10);
		if(write_profile_string(section,sPhaseNum0,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stFaultDetectionSetEx.CNum[i][1],tmpstr,10);
		if(write_profile_string(section,sPhaseNum1,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stFaultDetectionSetEx.CNum[i][2],tmpstr,10);
		if(write_profile_string(section,sPhaseNum2,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取相序表
 * @author : liuxupu
 * @date : 2013/4/9
 * @version : ver 1.0
 * @inparam : 网页发送的相序表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getSequenceTableInfo(stSequenceTable *pstSequenceTable,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "SequenceTable";
	char value[BUF_SIZE] = "";
	int i;
	char sNum1[12],sNum2[12],sNum3[12],sNum4[12];
	
	memset(pstSequenceTable,0,sizeof(stSequenceTable));
	if(!read_profile_string(section,"SequenceTableNo",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstSequenceTable->SequenceTableNo = atoi(value);
	}
	for(i=0;i<=15;i++)
	{
		sprintf(sNum1,"%s1_%02d","SNum",i+1);
		sprintf(sNum2,"%s2_%02d","SNum",i+1);
		sprintf(sNum3,"%s3_%02d","SNum",i+1);
		sprintf(sNum4,"%s4_%02d","SNum",i+1);
		if(!read_profile_string(section,sNum1,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstSequenceTable->SNum[0][i] = atoi(value);
		}
		if(!read_profile_string(section,sNum2,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstSequenceTable->SNum[1][i] = atoi(value);
		}
		if(!read_profile_string(section,sNum3,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstSequenceTable->SNum[2][i] = atoi(value);
		}
		if(!read_profile_string(section,sNum4,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstSequenceTable->SNum[3][i] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置相序表到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/9
 * @version : ver 1.0
 * @inparam : 网页发送的相序表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveSequenceTable2Ini(stSequenceTable stSequenceTableEx,char * strConfigPath)
{
	const char * section = "SequenceTable";
	char tmpstr[48] = { 0 };
	int i;
	char sNum1[12],sNum2[12],sNum3[12],sNum4[12];
	itoa(stSequenceTableEx.SequenceTableNo,tmpstr,10);
	if(write_profile_string(section, "SequenceTableNo",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	for(i=0;i<=15;i++)
	{
		sprintf(sNum1,"%s1_%02d","SNum",i+1);
		sprintf(sNum2,"%s2_%02d","SNum",i+1);
		sprintf(sNum3,"%s3_%02d","SNum",i+1);
		sprintf(sNum4,"%s4_%02d","SNum",i+1);
		itoa(stSequenceTableEx.SNum[0][i],tmpstr,10);
		if(write_profile_string(section,sNum1,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stSequenceTableEx.SNum[1][i],tmpstr,10);
		if(write_profile_string(section,sNum2,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stSequenceTableEx.SNum[2][i],tmpstr,10);
		if(write_profile_string(section,sNum3,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stSequenceTableEx.SNum[3][i],tmpstr,10);
		if(write_profile_string(section,sNum4,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取方案表
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : 网页发送的方案表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getProgramTableInfo(stProgramTable *pstProgramTable,char *strConfigPath, int num)
{	
	//读取配置文件参数
	const char * section = NULL;
	char value[BUF_SIZE] = "";
	char temp[24] = {0};
	char pNum1[12],pNum2[12],pNum3[12],pNum4[12];
	int i;

	sprintf(temp,"%02dProgramTable",num);
	section = temp;
	memset(pstProgramTable,0,sizeof(stProgramTable));
	for(i=0;i<=3;i++)
	{
		sprintf(pNum1,"%s1_%02d","PNum",i+1);
		sprintf(pNum2,"%s2_%02d","PNum",i+1);
		sprintf(pNum3,"%s3_%02d","PNum",i+1);
		sprintf(pNum4,"%s4_%02d","PNum",i+1);
		if(!read_profile_string(section,pNum1,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstProgramTable->PNum[0][i] = atoi(value);
		}
		if(!read_profile_string(section,pNum2,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstProgramTable->PNum[1][i] = atoi(value);
		}
		if(!read_profile_string(section,pNum3,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstProgramTable->PNum[2][i] = atoi(value);
		}
		if(!read_profile_string(section,pNum4,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstProgramTable->PNum[3][i] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置方案表到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : 网页发送的方案表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveProgramTable2Ini(stProgramTable stProgramTableEx,char * strConfigPath, int num)
{
	const char * section = NULL;
	char tmpstr[48] = { 0 };
	char temp[24] = {0};
	char pNum1[12],pNum2[12],pNum3[12],pNum4[12];
	int i;
	sprintf(temp,"%02dProgramTable",num);
	section = temp;

	for(i=0;i<=3;i++)
	{
		sprintf(pNum1,"%s1_%02d","PNum",i+1);
		sprintf(pNum2,"%s2_%02d","PNum",i+1);
		sprintf(pNum3,"%s3_%02d","PNum",i+1);
		sprintf(pNum4,"%s4_%02d","PNum",i+1);
		itoa(stProgramTableEx.PNum[0][i],tmpstr,10);
		if(write_profile_string(section,pNum1,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stProgramTableEx.PNum[1][i],tmpstr,10);
		if(write_profile_string(section,pNum2,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stProgramTableEx.PNum[2][i],tmpstr,10);
		if(write_profile_string(section,pNum3,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stProgramTableEx.PNum[3][i],tmpstr,10);
		if(write_profile_string(section,pNum4,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取时基动作表
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : 网页发送的时基动作表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getTimeBasedActionTableInfo(stTimeBasedActionTable *pstTimeBasedActionTable,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "TimeBasedActionTable";
	char value[BUF_SIZE] = "";
	int i;
	char TNum[20];
	
	memset(pstTimeBasedActionTable,0,sizeof(stTimeBasedActionTable));
	if(!read_profile_string(section,"ActionTable",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstTimeBasedActionTable->ActionTable = atoi(value);
	}
	if(!read_profile_string(section,"ProgramNo",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstTimeBasedActionTable->ProgramNo = atoi(value);
	}
	for(i=0;i<=3;i++)
	{
		sprintf(TNum,"%s%02d","AssistFunction",i+1);
		if(!read_profile_string(section,TNum,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstTimeBasedActionTable->AssistFunction[i] = atoi(value);
		}
	}
	for(i=0;i<=7;i++)
	{
		sprintf(TNum,"%s%02d","SpecialFunction",i+1);
		if(!read_profile_string(section,TNum,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstTimeBasedActionTable->SpecialFunction[i] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置时基动作表到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : 网页发送的时基动作表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveTimeBasedActionTable2Ini(stTimeBasedActionTable stTimeBasedActionTableEx,char * strConfigPath)
{
	const char * section = "TimeBasedActionTable";
	char tmpstr[48] = { 0 };
	int i;
	char TimeNum[20];
	itoa(stTimeBasedActionTableEx.ActionTable,tmpstr,10);
	if(write_profile_string(section, "ActionTable",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stTimeBasedActionTableEx.ProgramNo,tmpstr,10);
	if(write_profile_string(section, "ProgramNo",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	for(i=0;i<=3;i++)
	{
		sprintf(TimeNum,"%s%02d","AssistFunction",i+1);
		itoa(stTimeBasedActionTableEx.AssistFunction[i],tmpstr,10);
		if(write_profile_string(section,TimeNum,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	for(i=0;i<=7;i++)
	{
		sprintf(TimeNum,"%s%02d","SpecialFunction",i+1);
		itoa(stTimeBasedActionTableEx.SpecialFunction[i],tmpstr,10);
		if(write_profile_string(section,TimeNum,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取时段表
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : 网页发送的时段表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getTimeIntervalInfo(stTimeInterval *pstTimeInterval,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "TimeInterval";
	char value[BUF_SIZE] = "";
	int i;
	char TNum1[20],TNum2[20],TNum3[20];
	
	memset(pstTimeInterval,0,sizeof(stTimeInterval));
	if(!read_profile_string(section,"TimeIntervalNo",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstTimeInterval->TimeIntervalNo = atoi(value);
	}
	for(i=0;i<=3;i++)
	{
		sprintf(TNum1,"%s%02d_1","Time",i+1);
		sprintf(TNum2,"%s%02d_2","Time",i+1);
		sprintf(TNum3,"%s%02d_3","Time",i+1);
		if(!read_profile_string(section,TNum1,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstTimeInterval->Time[i][0] = atoi(value);
		}
		if(!read_profile_string(section,TNum2,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstTimeInterval->Time[i][1] = atoi(value);
		}
		if(!read_profile_string(section,TNum3,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstTimeInterval->Time[i][2] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置时段表到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/10
 * @version : ver 1.0
 * @inparam : 网页发送的时段表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveTimeInterval2Ini(stTimeInterval stTimeIntervalEx,char * strConfigPath)
{
	const char * section = "TimeInterval";
	char tmpstr[48] = { 0 };
	int i;
	char TimeNum1[20],TimeNum2[20],TimeNum3[20];
	itoa(stTimeIntervalEx.TimeIntervalNo,tmpstr,10);
	if(write_profile_string(section, "TimeIntervalNo",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	for(i=0;i<=3;i++)
	{
		sprintf(TimeNum1,"%s%02d_1","Time",i+1);
		sprintf(TimeNum2,"%s%02d_2","Time",i+1);
		sprintf(TimeNum3,"%s%02d_3","Time",i+1);
		itoa(stTimeIntervalEx.Time[i][0],tmpstr,10);
		if(write_profile_string(section,TimeNum1,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stTimeIntervalEx.Time[i][1],tmpstr,10);
		if(write_profile_string(section,TimeNum2,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
		itoa(stTimeIntervalEx.Time[i][2],tmpstr,10);
		if(write_profile_string(section,TimeNum3,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取调度计划
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : 网页发送的调度计划参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getSchedulingInfo(stScheduling *pstScheduling,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "Scheduling";
	char value[BUF_SIZE] = "";
	int i;
	char TNum1[20],TNum2[20],TNum3[20];
	
	memset(pstScheduling,0,sizeof(stScheduling));
	if(!read_profile_string(section,"SchedulingNo",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstScheduling->SchedulingNo = atoi(value);
	}
	if(!read_profile_string(section,"TimeIntervalNum",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstScheduling->TimeIntervalNum = atoi(value);
	}
	for(i=0;i<=12;i++)
	{
		sprintf(TNum1,"%s%02d","Month",i+1);
		if(!read_profile_string(section,TNum1,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstScheduling->Month[i] = atoi(value);
		}
	}
	for(i=0;i<=31;i++)
	{
		sprintf(TNum2,"%s%02d","Day",i+1);
		if(!read_profile_string(section,TNum2,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstScheduling->Day[i] = atoi(value);
		}
	}
	for(i=0;i<=7;i++)
	{
		sprintf(TNum3,"%s%02d","WeekDay",i+1);
		if(!read_profile_string(section,TNum3,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstScheduling->WeekDay[i] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置调度计划到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : 网页发送的调度计划参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveScheduling2Ini(stScheduling stSchedulingEx,char * strConfigPath)
{
	const char * section = "Scheduling";
	char tmpstr[48] = { 0 };
	int i;
	char TimeNum1[20],TimeNum2[20],TimeNum3[20];
	itoa(stSchedulingEx.SchedulingNo,tmpstr,10);
	if(write_profile_string(section, "SchedulingNo",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stSchedulingEx.TimeIntervalNum,tmpstr,10);
	if(write_profile_string(section, "TimeIntervalNum",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	for(i=0;i<=12;i++)
	{
		sprintf(TimeNum1,"%s%02d","Month",i+1);
		itoa(stSchedulingEx.Month[i],tmpstr,10);
		if(write_profile_string(section,TimeNum1,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	for(i=0;i<=31;i++)
	{
		sprintf(TimeNum2,"%s%02d","Day",i+1);
		itoa(stSchedulingEx.Day[i],tmpstr,10);
		if(write_profile_string(section,TimeNum2,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	for(i=0;i<=7;i++)
	{
		sprintf(TimeNum3,"%s%02d","WeekDay",i+1);
		itoa(stSchedulingEx.WeekDay[i],tmpstr,10);
		if(write_profile_string(section,TimeNum3,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取重叠表
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : 网页发送的重叠表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getOverlappingInfo(stOverlapping *pstOverlapping,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "Overlapping";
	char value[BUF_SIZE] = "";
	int i;
	char ONum[20];
	
	memset(pstOverlapping,0,sizeof(stOverlapping));
	if(!read_profile_string(section,"FollowPhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstOverlapping->FollowPhase = atoi(value);
	}
	if(!read_profile_string(section,"GreenLight",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstOverlapping->GreenLight = atoi(value);
	}
	if(!read_profile_string(section,"RedLight",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstOverlapping->RedLight = atoi(value);
	}
	if(!read_profile_string(section,"YellowLight",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstOverlapping->YellowLight = atoi(value);
	}
	if(!read_profile_string(section,"GreenFlash",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstOverlapping->GreenFlash = atoi(value);
	}
	if(!read_profile_string(section,"ModifiedPhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstOverlapping->ModifiedPhase = atoi(value);
	}
	for(i=0;i<=31;i++)
	{
		sprintf(ONum,"%s%02d","ParentPhase",i+1);
		if(!read_profile_string(section,ONum,value,BUF_SIZE,"",strConfigPath))
		{
			//读写文件异常
			return  0;
		}
		else
		{
			pstOverlapping->ParentPhase[i] = atoi(value);
		}
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置重叠表到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : 网页发送的重叠表参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveOverlapping2Ini(stOverlapping stOverlappingEx,char * strConfigPath)
{
	const char * section = "Overlapping";
	char tmpstr[48] = { 0 };
	int i;
	char OverNum[20];
	itoa(stOverlappingEx.FollowPhase,tmpstr,10);
	if(write_profile_string(section, "FollowPhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stOverlappingEx.GreenLight,tmpstr,10);
	if(write_profile_string(section, "GreenLight",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stOverlappingEx.RedLight,tmpstr,10);
	if(write_profile_string(section, "RedLight",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stOverlappingEx.YellowLight,tmpstr,10);
	if(write_profile_string(section, "YellowLight",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stOverlappingEx.GreenFlash,tmpstr,10);
	if(write_profile_string(section, "GreenFlash",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stOverlappingEx.ModifiedPhase,tmpstr,10);
	if(write_profile_string(section, "ModifiedPhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	for(i=0;i<=31;i++)
	{
		sprintf(OverNum,"%s%02d","ParentPhase",i+1);
		itoa(stOverlappingEx.ParentPhase[i],tmpstr,10);
		if(write_profile_string(section,OverNum,tmpstr,strConfigPath) == 0)
		{
			return 0;
		}
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取协调信息
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : 网页发送的协调参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getCoordinateInfo(stCoordinate *pstCoordinate,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "Coordinate";
	char value[BUF_SIZE] = "";
	memset(pstCoordinate,0,sizeof(stCoordinate));
	if(!read_profile_string(section,"ControlModel",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstCoordinate->ControlModel = atoi(value);
	}
	if(!read_profile_string(section,"ManualMethod",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstCoordinate->ManualMethod = atoi(value);
	}
	if(!read_profile_string(section,"CoordinationMode",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstCoordinate->CoordinationMode = atoi(value);
	}
	if(!read_profile_string(section,"CoordinateMaxMode",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstCoordinate->CoordinateMaxMode = atoi(value);
	}
	if(!read_profile_string(section,"CoordinateForceMode",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstCoordinate->CoordinateForceMode = atoi(value);
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置协调到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/11
 * @version : ver 1.0
 * @inparam : 网页发送的协调参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveCoordinate2Ini(stCoordinate stCoordinateEx,char * strConfigPath)
{
	const char * section = "Coordinate";
	char tmpstr[48] = { 0 };
	itoa(stCoordinateEx.ControlModel,tmpstr,10);
	if(write_profile_string(section, "ControlModel",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.ManualMethod,tmpstr,10);
	if(write_profile_string(section, "ManualMethod",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.CoordinationMode,tmpstr,10);
	if(write_profile_string(section, "CoordinationMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.CoordinateMaxMode,tmpstr,10);
	if(write_profile_string(section, "CoordinateMaxMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.CoordinateForceMode,tmpstr,10);
	if(write_profile_string(section, "CoordinateForceMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取车辆检测器信息
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : 网页发送的车辆检测器参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getVehicleDetectorInfo(stVehicleDetector *pstVehicleDetector,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "VehicleDetector";
	char value[BUF_SIZE] = "";
	memset(pstVehicleDetector,0,sizeof(stVehicleDetector));
	if(!read_profile_string(section,"DetectorNo",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->DetectorNo = atoi(value);
	}
	if(!read_profile_string(section,"RequestPhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->RequestPhase = atoi(value);
	}
	if(!read_profile_string(section,"SwitchPhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->SwitchPhase = atoi(value);
	}
	if(!read_profile_string(section,"Delay",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->Delay = atoi(value);
	}
	if(!read_profile_string(section,"FailureTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->FailureTime = atoi(value);
	}
	if(!read_profile_string(section,"QueueLimit",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->QueueLimit = atoi(value);
	}
	if(!read_profile_string(section,"NoResponseTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->NoResponseTime = atoi(value);
	}
	if(!read_profile_string(section,"MaxDuration",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->MaxDuration = atoi(value);
	}
	if(!read_profile_string(section,"Extend",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->Extend = atoi(value);
	}
	if(!read_profile_string(section,"MaxVehicle",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->MaxVehicle = atoi(value);
	}
	if(!read_profile_string(section,"Flow",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->Flow = atoi(value);
	}
	if(!read_profile_string(section,"Occupancy",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->Occupancy = atoi(value);
	}
	if(!read_profile_string(section,"ProlongGreen",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->ProlongGreen = atoi(value);
	}
	if(!read_profile_string(section,"AccumulateInitial",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->AccumulateInitial = atoi(value);
	}
	if(!read_profile_string(section,"Queue",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->Queue = atoi(value);
	}
	if(!read_profile_string(section,"Request",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->Request = atoi(value);
	}
	if(!read_profile_string(section,"RedInterval",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->RedInterval = atoi(value);
	}
	if(!read_profile_string(section,"YellowInterval",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstVehicleDetector->YellowInterval = atoi(value);
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置车辆检测器到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : 网页发送的车辆检测器参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveVehicleDetector2Ini(stVehicleDetector stVehicleDetectorEx,char * strConfigPath)
{
	const char * section = "VehicleDetector";
	char tmpstr[48] = { 0 };
	itoa(stVehicleDetectorEx.DetectorNo,tmpstr,10);
	if(write_profile_string(section, "DetectorNo",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.RequestPhase,tmpstr,10);
	if(write_profile_string(section, "RequestPhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.SwitchPhase,tmpstr,10);
	if(write_profile_string(section, "SwitchPhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.Delay,tmpstr,10);
	if(write_profile_string(section, "Delay",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.FailureTime,tmpstr,10);
	if(write_profile_string(section, "FailureTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.QueueLimit,tmpstr,10);
	if(write_profile_string(section, "QueueLimit",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.NoResponseTime,tmpstr,10);
	if(write_profile_string(section, "NoResponseTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.MaxDuration,tmpstr,10);
	if(write_profile_string(section, "MaxDuration",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.Extend,tmpstr,10);
	if(write_profile_string(section, "Extend",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.MaxVehicle,tmpstr,10);
	if(write_profile_string(section, "MaxVehicle",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.Flow,tmpstr,10);
	if(write_profile_string(section, "Flow",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.Occupancy,tmpstr,10);
	if(write_profile_string(section, "Occupancy",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.ProlongGreen,tmpstr,10);
	if(write_profile_string(section, "ProlongGreen",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.AccumulateInitial,tmpstr,10);
	if(write_profile_string(section, "AccumulateInitial",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.Queue,tmpstr,10);
	if(write_profile_string(section, "Queue",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.Request,tmpstr,10);
	if(write_profile_string(section, "Request",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.RedInterval,tmpstr,10);
	if(write_profile_string(section, "RedInterval",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stVehicleDetectorEx.YellowInterval,tmpstr,10);
	if(write_profile_string(section, "YellowInterval",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取行人检测器信息
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : 网页发送的行人检测器参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getPedestrianInfo(stPedestrian *pstPedestrian,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "PedestrianDetector";
	char value[BUF_SIZE] = "";
	memset(pstPedestrian,0,sizeof(stPedestrian));
	if(!read_profile_string(section,"DetectorNo",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPedestrian->DetectorNo = atoi(value);
	}
	if(!read_profile_string(section,"RequestPhase",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPedestrian->RequestPhase = atoi(value);
	}
	if(!read_profile_string(section,"NoResponseTime",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPedestrian->NoResponseTime = atoi(value);
	}
	if(!read_profile_string(section,"MaxDuration",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPedestrian->MaxDuration = atoi(value);
	}
	if(!read_profile_string(section,"InductionNumber",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstPedestrian->InductionNumber = atoi(value);
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置行人检测器到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : 网页发送的行人检测器参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int savePedestrian2Ini(stPedestrian stPedestrianEx,char * strConfigPath)
{
	const char * section = "PedestrianDetector";
	char tmpstr[48] = { 0 };
	itoa(stPedestrianEx.DetectorNo,tmpstr,10);
	if(write_profile_string(section, "DetectorNo",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPedestrianEx.RequestPhase,tmpstr,10);
	if(write_profile_string(section, "RequestPhase",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPedestrianEx.NoResponseTime,tmpstr,10);
	if(write_profile_string(section, "NoResponseTime",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPedestrianEx.MaxDuration,tmpstr,10);
	if(write_profile_string(section, "MaxDuration",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stPedestrianEx.InductionNumber,tmpstr,10);
	if(write_profile_string(section, "InductionNumber",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取故障配置信息
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : 网页发送的故障配置参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getFaultConfigInfo(stFaultConfig *pstFaultConfig,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "FaultConfig";
	char value[BUF_SIZE] = "";
	memset(pstFaultConfig,0,sizeof(stFaultConfig));
	if(!read_profile_string(section,"ControlRecord",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultConfig->ControlRecord = atoi(value);
	}
	if(!read_profile_string(section,"CommunicatRecord",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultConfig->CommunicatRecord = atoi(value);
	}
	if(!read_profile_string(section,"DetectorRecord",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstFaultConfig->DetectorRecord = atoi(value);
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置故障配置到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/12
 * @version : ver 1.0
 * @inparam : 网页发送的故障配置参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveFaultConfig2Ini(stFaultConfig stFaultConfigEx,char * strConfigPath)
{
	const char * section = "FaultConfig";
	char tmpstr[48] = { 0 };
	itoa(stFaultConfigEx.ControlRecord,tmpstr,10);
	if(write_profile_string(section, "ControlRecord",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultConfigEx.CommunicatRecord,tmpstr,10);
	if(write_profile_string(section, "CommunicatRecord",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultConfigEx.DetectorRecord,tmpstr,10);
	if(write_profile_string(section, "DetectorRecord",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 * @brief : 从配置文件中获取树动态参数信息
 * @author : liuxupu
 * @date : 2013/4/27
 * @version : ver 1.0
 * @inparam : 网页发送的树动态参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int getTreeDynamicParameter(stTreeDynamicPara *pstTreeDynamicPara,char *strConfigPath)
{	
	//读取配置文件参数
	const char * section = "TreeDynamicParameter";
	char value[BUF_SIZE] = "";
	memset(pstTreeDynamicPara,0,sizeof(stTreeDynamicPara));
	if(!read_profile_string(section,"addCount",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstTreeDynamicPara->addCount = atoi(value);
	}
	if(!read_profile_string(section,"addChannel",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstTreeDynamicPara->addChannel = atoi(value);
	}
	if(!read_profile_string(section,"addProgram",value,BUF_SIZE,"",strConfigPath))
	{
		//读写文件异常
		return  0;
	}
	else
	{
		pstTreeDynamicPara->addProgram = atoi(value);
	}
	return 1;

}

/*****************************************************************************
 * @brief : 设置树动态参数到指定ini文件中
 * @author : liuxupu
 * @date : 2013/4/27
 * @version : ver 1.0
 * @inparam : 网页发送的树动态参数参数
 * @outparam : 如果都正确，则返回1；如果读写文件异常，则返回0
 *****************************************************************************/
static int saveTreeDynamicParameter(stTreeDynamicPara stTreeDynamicParaEx,char * strConfigPath)
{
	const char * section = "TreeDynamicParameter";
	char tmpstr[48] = { 0 };
	itoa(stTreeDynamicParaEx.addCount,tmpstr,10);
	if(write_profile_string(section, "addCount",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stTreeDynamicParaEx.addChannel,tmpstr,10);
	if(write_profile_string(section, "addChannel",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stTreeDynamicParaEx.addProgram,tmpstr,10);
	if(write_profile_string(section, "addProgram",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}
/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
							int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, WEBS_DEFAULT_HOME);
		return 1;
	}
	return 0;
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks() 
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks  
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif

/******************************************************************************/
/*
 *	Center window on screen
 */

#define RCWIDTH(rc) ((rc).right - (rc).left)
#define RCHEIGHT(rc) ((rc).bottom - (rc).top)

static void centerWindowOnDisplay(HWND hwndCenter)
{
	int			xLeft, yTop, cxDisp, cyDisp;
	RECT		rcDlg;

	a_assert(IsWindow(hwndCenter));

/*
 *	Get Window Size
 */
	GetWindowRect(hwndCenter, &rcDlg);

/*
 *	Get monitor width and height
 */
	cxDisp = GetSystemMetrics(SM_CXFULLSCREEN);
	cyDisp = GetSystemMetrics(SM_CYFULLSCREEN);

/*
 *	Find dialog's upper left based on screen size
 */
	xLeft = cxDisp / 2 - RCWIDTH(rcDlg) / 2;
	yTop = cyDisp / 2 - RCHEIGHT(rcDlg) / 2;

/*
 *	If the dialog is outside the screen, move it inside
 */
	if (xLeft < 0) {
		xLeft = 0;
	} else if (xLeft + RCWIDTH(rcDlg) > cxDisp) {
		xLeft = cxDisp - RCWIDTH(rcDlg);
	}

	if (yTop < 0) {
		yTop = 0;
	} else if (yTop + RCHEIGHT(rcDlg) > cyDisp) {
		yTop = cyDisp - RCHEIGHT(rcDlg);
	}

/*
 *	Move the window
 */
	SetWindowPos(hwndCenter, HWND_TOP, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

/******************************************************************************/
