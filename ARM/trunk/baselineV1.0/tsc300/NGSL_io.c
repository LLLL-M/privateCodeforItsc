/*hdr
**
**	
**
**	FILE NAME:	
**
**	AUTHOR:		
**
**	DATE:		
**
**	FILE DESCRIPTION:
**			
**			
**
**	FUNCTIONS:	
**			
**			
**			
**	NOTES:		
*/  

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include "common.h"
#include "canmsg.h"
#include "io.h"
#include "hik.h"
#include "platform.h"
#include "HikConfig.h"
#include "specialControl.h"
#include "binfile.h"
#include "configureManagement.h"

int gOftenPrintFlag = 0;
typedef unsigned short  uint16;
static uint16 g_nLampStatus[8] = {0};
uint16 temp_nLampStatus[8] = {0}; //用于脉冲式触发倒计时牌
char temp_Sign[16] = {0};//标志位
char lastRedLight[16] = {0}; //红灯上一次状态
char nowRedLight[16] = {0}; //红灯现在状态
char stayTimes[16] = {0};//延迟发送脉冲时间
int HardwareWatchdoghd = 0;
//static int HardflashDogFlag = 0;
int is_WatchdogEnabled = 0;
//static int g_yellowflash = 0; //黄闪标志：1：开；0：关
static int g_errorstat = 0; //故障标志 :1 :有故障； 0:无故障
//static int g_closelampflag = 0; //黄闪关灯
//static uint16 g_yellowflashfreq = 1; //黄闪频率
static int g_faultstatus[32] = {0}; //通道故障记录用于恢复
static int CurDetectFreq = 0;

//added by liujie 20160506
unsigned char finalChannelStatus[32] = {0};       //最终信号灯输出状态
static uint16 nLampStatus[4][8] = {0};         //1S4次点灯值存储






//临时修改，避免越界
extern CountDownVeh countdown_veh[17];
extern CountDownPed countdown_ped[17];
static int SetFreeGreenTimeSign = 0;//感应时间标志位
unsigned char g_RedCurrentValue[32] = {0};   //32路红灯电流值
time_t pedLastTime[4] = {0};//行人检测相机超时时间
int theFirstTimeSign = 0;//第一次标志位
//static int pedOnState = 0;//行人进入状态
struct FAILURE_INFO s_failure_info;       //故障信息结构体

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //存放在/home/config.dat配置中的所有结构体，包括特殊参数定义结构体/车检板开关/日志打印开关/错误序列号/电流参数表
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //存放在/home/desc.dat配置中的所有结构体，包括相位描述/通道描述/方案描述/计划描述/日期描述pthread_t thread;//接受电流电压消息的CAN句柄
extern CountDownCfg        g_CountDownCfg;              //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //混杂参数

extern pthread_rwlock_t gCountDownLock;
extern pthread_rwlock_t gLockRealtimeVol ;//保护实时流量的读写锁
extern MsgRealtimeVolume gStructMsgRealtimeVolume;                 //实时流量，只有流量是实时的


void ItsCustom(void);
void CountDownPulse(void);
void WirelessChanLock(void);

static void LockByChan(unsigned char flag, unsigned char *chanConfig, unsigned char *delayFlag,unsigned char *chanChangedFlag,unsigned char *cChanStatus,unsigned char *cArrayBlinkStatus,unsigned char *cArrayIsTimeEnd);
static int transitionOnChanLockStart(unsigned char *chan, unsigned char *chanConfig);
static int transitionOnChanLockEnd(unsigned char *chan);
static unsigned char transitionControl(unsigned char *chan, unsigned char *config);
static int trainsitionOnChanChange(unsigned char *cur, unsigned char *config);

extern UINT8 gFollowPhaseGreenBlinkFlag[NUM_PHASE];//运行相位绿闪标志
extern UINT8 gFollowPhaseMotherPhase[NUM_PHASE];//跟随相位正在跟随的母相位
extern int gWirelessCtrlLockChanId;
int gLastWirelessCtrlId=0;

#define TIMEOUT  15
#define _YELLOWFLASH_


#define N 32
#define M 1<<(N-1)
void print_data(unsigned int c)    
{       
	int i;    
	for (i = 0; i < N; i++)	{       
	   if(i % 8 == 0) printf(" ");  
	   putchar(((c & M) == 0) ? '0':'1'); 
	   c <<= 1;    
	}       
	printf("\n");    
}
/*********************************************************************************
*
* 	设定感应检测时间。
*
***********************************************************************************/
extern void SeFreeGreenTime(unsigned char time);
void Count_Down_pulse_All(uint16 *g_nLampStatus);
void Count_Down_pulse_Half(uint16 *g_nLampStatus);


extern int g_auto_pressed;    			  //0:自动按键没有按下   1:自动按键已按下
extern int g_manual_pressed;				  //0:手动按键没有按下   1:手动按键已按下
extern int g_flashing_pressed;			  //0:黄闪按键没有按下   1:黄闪按键已按下
extern int g_allred_pressed;				  //0:全红按键没有按下   1:全红按键已按下
extern int g_step_by_step_pressed;		  //0:步进按键没有按下   1:步进按键已按下
extern int globalWatchdogFlagDisable;     //0:Enable  1:Disable.//WatchDog 使能开关. (弃用)
extern int RTC_USE_GPS_ENABLE;			  //非0:Enable  0:Disable //GPS使能开关.  (弃用)
extern PedDetectParams g_struPedDetect; //行人检测参数

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //存放在/home/config.dat配置中的所有结构体，包括特殊参数定义结构体/车检板开关/日志打印开关/错误序列号/电流参数表
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //存放在/home/desc.dat配置中的所有结构体，包括相位描述/通道描述/方案描述/计划描述/日期描述

/* add by Jicky */
extern SignalControllerPara *gSignalControlpara;    //全局配置信息
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //倒计时接口信息

extern int msgid;
extern UInt8 gStepFlag;    //步进标志，0：表示未步进，1：表示步进
extern UInt16 gLightArray[8];	//步进时使用的点灯数组
extern pthread_rwlock_t gLightArrayLock;    //步进点灯数组的锁
extern unsigned short g_nLightStatus[8];

extern int RedSignalCheckInit(void);
extern void CollectCountDownParams();
extern void StepPthreadInit(void);

/* add over */


/*********************************************************************************
*
* 	每次开机将程序运行时间记录到指定文件。
*
***********************************************************************************/
int WriteLogInfos(const char *pFile,const char *pchMsg)
{
	time_t now;  
	struct tm *timenow; 
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;
	
	if(pchMsg == NULL)
	{
		ERR("开机记录消息无效!\n");
		return -1;
	}
    
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	ERR("开机记录文件未能正确打开!\n");
        return -1;
    }
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		ERR("获取开机记录文件信息失败!\n");
		return -1;
	}
	
	if(f_stat.st_size > 100*1024)
	{
		fclose(pFileDebugInfo);
		//把文件的内容清0，即重写
		pFileDebugInfo = fopen(pFile, "w+");
	}

	//获取国际标准时间
	time(&now);  
	  
	//转换为本地时间
	timenow = localtime(&now); 

	fprintf(pFileDebugInfo,"%04d.%02d.%02d-%02d:%02d:%02d %s\n",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec,
		pchMsg);
    
    fclose(pFileDebugInfo);
    return 0;
}

/*********************************************************************************
*
* 	写故障日志。
*
***********************************************************************************/
int WriteErrorInfos(const char *pFile,const char *pchMsg)
{
	time_t now;  
	struct tm *timenow; 
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;
	if(pchMsg == NULL)
	{
		ERR("故障记录消息无效!\n");
		return -1;
	}
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	ERR("故障记录文件未能正确打开!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		ERR("获取故障记录文件信息失败!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			ERR("故障记录文件清整后未能正确打开!\n");
			return -1;
		}
	}
	time(&now);  
	timenow = localtime(&now); 
	fprintf(pFileDebugInfo,"%04d.%02d.%02d-%02d:%02d:%02d %s\n",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec,
		pchMsg);
    fclose(pFileDebugInfo);
    return 0;
}


/*********************************************************************************
*
* 	写二进制故障日志。
*
***********************************************************************************/
int WriteFailureInfos(const char *pFile,struct FAILURE_INFO s_failure_info)
{
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;

    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	ERR("故障记录文件未能正确打开!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		ERR("获取故障记录文件信息失败!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			ERR("故障记录文件清整后未能正确打开!\n");
			return -1;
		}
	}
	fwrite(&s_failure_info,sizeof(s_failure_info),1,pFileDebugInfo);
    fclose(pFileDebugInfo);

    if(gStructBinfileConfigPara.cFailureNumber >= 0xffff)
    {
        gStructBinfileConfigPara.cFailureNumber = 0;//如果错误序列号超过，65536，就清零。
    }

    WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
    return 0;
}

/*********************************************************************************
*
* 	写错误状态日志
*
***********************************************************************************/
void WriteFaultStatus()
{
	FILE *pFile = NULL;
	pFile = fopen("/home/FaultStatus.dat", "wb");
	if (pFile == NULL)
	{
		ERR("打开FaultStatus.dat失败,write\n");
		return;
	}
	fwrite(g_faultstatus, sizeof(g_faultstatus), 1, pFile);
	fclose(pFile);
}

/*********************************************************************************
*
* 	读错误状态日志
*
***********************************************************************************/
void ReadFaultStatus()
{
	FILE *pFile = NULL;
	pFile = fopen("/home/FaultStatus.dat", "rb");
	if (pFile == NULL)
	{
		ERR("打开FaultStatus.dat失败,read\n");
		return;
	}
	fread(g_faultstatus, sizeof(g_faultstatus), 1, pFile);
	fclose(pFile);
}

/*********************************************************************************
*
*	获取最终输出通道状态(1S刷新一次，供倒计时接口使用)
*
***********************************************************************************/

void GetFinalChannelStatus()
{
	int i = 0;
	int j = 0;
	UInt16 tmp1LampStatus[8] = {0};     //保存闪烁通道信息
	UInt16 tmp2LampStatus[8] = {0};		//保存亮灯通道信息
	UInt16 tmp3LampStatus[8] = {0};		//保存灭灯通道信息
	
	for(j = 0;j<8;j++)
	{
		tmp1LampStatus[j] = /*nLampStatus[0][j]^*/nLampStatus[1][j]^nLampStatus[2][j]/*^nLampStatus[3][j]*/;
		tmp2LampStatus[j] = nLampStatus[0][j]&nLampStatus[1][j]&nLampStatus[2][j]&nLampStatus[3][j];
		tmp3LampStatus[j] = ~(nLampStatus[0][j]|nLampStatus[1][j]|nLampStatus[2][j]|nLampStatus[3][j]);
		for(i = 0;i<4;i++)
		{
			//闪烁通道赋值
			if(BIT(tmp1LampStatus[j],i*3+0) == 1)
			{
				finalChannelStatus[j*4+i] = GREEN_BLINK;
			}
			if(BIT(tmp1LampStatus[j],i*3+1) == 1)
			{
				finalChannelStatus[j*4+i] = RED_BLINK;
			}
			if(BIT(tmp1LampStatus[j],i*3+2) == 1)
			{
				finalChannelStatus[j*4+i] = YELLOW_BLINK;
			}	
			
			//亮灯通道赋值
			if(BIT(tmp2LampStatus[j],i*3+0) == 1)
			{
				finalChannelStatus[j*4+i] = GREEN;
			}
			if(BIT(tmp2LampStatus[j],i*3+1) == 1)
			{
				finalChannelStatus[j*4+i] = RED;
			}
			if(BIT(tmp2LampStatus[j],i*3+2) == 1)
			{
				finalChannelStatus[j*4+i] = YELLOW;
			}
			//灭灯通道赋值
			if(BIT(tmp3LampStatus[j],i*3+0) == 1 &&BIT(tmp3LampStatus[j],i*3+1) == 1&&BIT(tmp3LampStatus[j],i*3+2) == 1)
			{
				finalChannelStatus[j*4+i] = TURN_OFF;
			}
			
		}
			
	}
	
	//测试打印
	
	/*INFO("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		finalChannelStatus[0],finalChannelStatus[1],finalChannelStatus[2],finalChannelStatus[3],
		finalChannelStatus[4],finalChannelStatus[5],finalChannelStatus[6],finalChannelStatus[7],
		finalChannelStatus[8],finalChannelStatus[9],finalChannelStatus[10],finalChannelStatus[11],
		finalChannelStatus[12],finalChannelStatus[13],finalChannelStatus[14],finalChannelStatus[15],
		finalChannelStatus[16],finalChannelStatus[17],finalChannelStatus[18],finalChannelStatus[19],
		finalChannelStatus[20],finalChannelStatus[21],finalChannelStatus[22],finalChannelStatus[23],
		finalChannelStatus[24],finalChannelStatus[25],finalChannelStatus[26],finalChannelStatus[27],
		finalChannelStatus[28],finalChannelStatus[29],finalChannelStatus[30],finalChannelStatus[31]);
	*/
	
}


/*********************************************************************************
*
* 	硬件看门狗初始化。
*
***********************************************************************************/

void HardwareWatchdogInit()
{
	//增加硬件看门狗喂狗
	is_WatchdogEnabled = gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch;
	if(is_WatchdogEnabled == 1)
	{
		//使能硬件看门狗，通过程序进行喂狗
		//关闭外部喂狗程序
		system("killall watchdog");
		sleep(1);
		//打开硬件看门狗
		HardwareWatchdoghd = open("/dev/watchdog", O_WRONLY);
		if(HardwareWatchdoghd == -1) 
		{
			ERR("Open watchdog error,retry!!!\n");
			HardwareWatchdoghd = open("/dev/watchdog", O_WRONLY);
			//return;
		}
		//激活硬件看门狗
		ioctl(HardwareWatchdoghd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);	
		INFO("Watchdog is enabled!!!\n");
	}
	else
	{
		//外部喂狗程序继续工作(外部喂狗程序已经在开机启动时自动加载)
		INFO("Watchdog is disabled!!!\n");
	}
}

/*********************************************************************************
*
* 	硬件看门狗喂狗。
*
***********************************************************************************/

unsigned int HardwareWatchdogKeepAlive(int boardNum)
{
	int dummy = 0;
	//硬件看门狗使能时并且boardNum为8时喂狗
	if(boardNum == 8 && is_WatchdogEnabled == 1 )
	{
		//发送喂狗命令
		ioctl(HardwareWatchdoghd, WDIOC_KEEPALIVE, &dummy);
	}
	//打开日志
	if(1 == gStructBinfileConfigPara.cPrintfLogSwitch)
    {
        freopen("/dev/tty","w",stdout);//打开日志
    }
    else
    {
        freopen("/dev/tty","w",stderr);//关闭日志
    }
	return 1;
}

/*********************************************************************************
*
* 	GPS是否启用控制函数。
*
***********************************************************************************/

void GPSInit()
{
	
	//GPS开关是否打开
	if(gStructBinfileConfigPara.sSpecialParams.iGpsSwitch == 0)
	{
		//GPS开关没有打开
		INFO("GPS is disabled!!!\n");
		system("killall -9 GPS");
		Set_LED2_OFF();
	}
	else 
	{
		//GPS开关已经打开
		INFO("GPS is enabled!!!\n");
		system("killall -9 GPS");
		//熄灭GPS指示灯
		Set_LED2_OFF();
		system("/root/GPS&");
	}
}

/*****************************************************************************
 函 数 名  : RecordVehicleFlowData 
             将各个检测器的统计数据保存到文件中，文件默认大小是10M，循环覆盖，文件的第一个字节存放的是最新一条记录距文件首部的偏移
             结构体数量
 输入参数  : TimeAndHistoryVolume *timeAndHistoryVolume  
             int size                                    
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年9月24日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static int RecordVehicleFlowData(TimeAndHistoryVolume *timeAndHistoryVolume)
{
	FILE *fp = NULL;
	UINT64 offset = 0;//offset记录了，即将插入的最新的一条记录距文件头的TimeAndHistoryVolume个数,该值存储在文件开头的4字节区域内
	
	fp = fopen(FILE_VEHICLE_DAT,"r+");//
	if(NULL == fp)//可能是文件不存在，就新建之
	{
	    if(2 == errno)//errno 等于 2表明是文件不存在
	    {
            if((fp = fopen(FILE_VEHICLE_DAT,"a+")) == NULL)
            {
                INFO("open %s failed : %s , errno %d .\n",FILE_VEHICLE_DAT,strerror(errno),errno);
                return 0;
            }
	    }
	    else
	    {
    		INFO("open %s failed : %s , errno %d .\n",FILE_VEHICLE_DAT,strerror(errno),errno);
    		return 0;
	    }
	}

    if(fread(&offset,sizeof(UINT64),1,fp) < 1)//先获取偏移量
    {
        offset = 1;
    }
    if(offset >= 10*1024*1024/sizeof(TimeAndHistoryVolume))//文件默认大小是10M，超过10M后就要从头开始覆盖,offset从4开始
    {
        offset = 1;
    }
    fseek(fp,0,SEEK_SET);
    offset += 1;
    fwrite(&offset,sizeof(UINT64),1,fp);//写入偏移量

    fseek(fp,sizeof(UINT64)+(offset-1)*sizeof(TimeAndHistoryVolume),SEEK_SET);//在正确的位置写入
	if(fwrite(timeAndHistoryVolume,sizeof(TimeAndHistoryVolume),1,fp) < 1)
	{
		INFO("read %s failed : %s .\n",FILE_VEHICLE_DAT,strerror(errno));
		fclose(fp);
		return 0;
	}
    fflush(fp);
	fclose(fp);
	return 1; 
}

/*****************************************************************************
 函 数 名  : CalcStatData
 功能描述  : 根据车检器的流量，计算比如车道占有率等统计数据
车道占有率的公式详见:
http://wenku.baidu.com/view/8c2e3951a417866fb84a8e22.html

车速的计算公式是:
任意交通负荷下的车速--流量通用模型为: 
设计车速Us/(km/h)	通行能力C (单车道)/(Pcu/h)	 
120	2200	0.93	1.88	4.85
100	2200	0.95	1.88	4.86
80	2000	1.00	1.88	4.90
60	1800	1.20	1.88	4.88



排队长度的计算策略是:
该车道没有放行权的时间乘以该车道的平均车速，就是该车道在红灯时的大致排队长度

车流密度k：某一瞬间，单位路段长度内的车辆数,单位是辆/km，根据格林希尔治（Greenshields）速度-密度模型模型，可以得到速度与密度的关系是:
Q = KUf(1- K/Kj),其中Q指的是车速，K就是车流密度，Uf是自由车速，Kj是阻塞密度.根据这个公式，可以反推得到车流密k与车速Q的关系为:
K = (Kj-sqrt(Kj* Kj  - 4* Kj*Q/Uf))/2

时间占有率o: 即车辆的时间密集度，在一定的观测时间内，车辆通过检测器时所占用的时间与观测总时间的比值。按照公式，时间占有率与车身长度检测器
长度之和成正比即o = (l+d)*k.

车头时距与交通量的关系  ht=3600/Q 
车头间距与交通流密度的关系hs=1000/K

 输入参数  : TimeAndHistoryVolume *timeAndHistoryVolume  nCycle 为测试周期长
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年9月24日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void CalcStatData(TimeAndHistoryVolume *timeAndHistoryVolume,UINT32 nCycle)
{
    int i = 0;
    float tmp = 0;
    UINT8 nPhaseId = 0;
    UINT16 nWaitTime = 0;
    const UINT8 Kj = 120;//阻塞密度--车流几乎无法移动，即发生交通阻塞时的车流密度,单位是辆/km.
    const UINT8 Uf = 72;//自由车流速度，可以理解为道路限速.

    //下面是计算平均车速需要用到的参数
    const UINT8 Us = 60;//假定该道路设计时速是60km/h
    const float a1 = 1.2;//表1 高速公路车速-流量通用模型参数表中的数据
    const float a2 = 1.88;//表1 高速公路车速-流量通用模型参数表中的数据
    const float a3 = 4.88;//表1 高速公路车速-流量通用模型参数表中的数据
    float b = 0;//中间值
    
    if(nCycle <= 0)
    {
        return;
    }   
    pthread_rwlock_rdlock(&gCountDownLock);
    for(i = 0; i < 48; i++)
    {
        if(timeAndHistoryVolume->struVolume[i].byDetectorVolume == 0)//如果车流量等于0，那么平均车速就是0，不再计算
        {
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = 0;
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 0;
        }
        else
        {
            //计算平均车速
            tmp = 1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle;//将车流量换算成一小时的过车量
            b = a2 + a3*pow(1.0*tmp/1800,3);
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = (int)(100*a1*Us/(1+powf(tmp/1800,b)));//为精确两位小数，我们存储时按照乘以100来做，实际使用时需要除以100.

            //计算车流密度和时间占有率
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 100*Kj*(1 - timeAndHistoryVolume->struVolume[i].byVehicleSpeed/100/Uf);

            //计算车头间距
            if(timeAndHistoryVolume->struVolume[i].wVehicleDensity != 0)
            {
                timeAndHistoryVolume->struVolume[i].wVehicleHeadDistance = 100*1000/(1.0*timeAndHistoryVolume->struVolume[i].wVehicleDensity/100);
            }

            //计算车头时距
            timeAndHistoryVolume->struVolume[i].wVehicleHeadTimeDistance = 100*3600/(1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle);

            //计算排队长度,相位的红灯等待时间除以车流量，得到的车辆数就是该车道的红灯排队长度
            nPhaseId = gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase; //找到车检器对应的相位
            nWaitTime = 0;
            if(nPhaseId >= 1 && nPhaseId <= 16)
            {
                nWaitTime = gCountDownParams->ucCurCycleTime - gCountDownParams->stPhaseRunningInfo[nPhaseId - 1][0];
            }
            if(timeAndHistoryVolume->struVolume[i].byDetectorVolume == 0)
            {
                timeAndHistoryVolume->struVolume[i].wQueueLengh = 0;
                continue;
            }
            timeAndHistoryVolume->struVolume[i].wQueueLengh = 100*nWaitTime/(1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume/nCycle);
            
        }
    }
    pthread_rwlock_unlock(&gCountDownLock);
    
    /*
    i = 0;
    INFO("总数: %d 辆, 时间占有率: %0.2f%%, 平均车速: %0.2f km/h, 排队长度: %0.2f m, 车流密度: %0.2f 辆/km, \n\t\t\t车头间距: %0.2f m, 车头时距: %0.2f s, 绿损: %d s\n"
                                                ,timeAndHistoryVolume->struVolume[i].byDetectorVolume
                                                ,timeAndHistoryVolume->struVolume[i].byDetectorOccupancy/100.0
                                                ,timeAndHistoryVolume->struVolume[i].byVehicleSpeed/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wQueueLengh/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleDensity/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleHeadDistance/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleHeadTimeDistance/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wGreenLost/100);
                                              
    */
}



/*****************************************************************************
 函 数 名  : VehicleFlowStat
 功能描述  : 统计车流量，周期取的是单元参数中的采集周期，单位是单元参数中的
             采集单位，每个周期统计一次数据，并保存到指定的文件中，文件大小
             默认是10M,且不可更改,超过10M后就循环覆盖。
 输入参数  : boardNum只能是1 2 3
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年9月24日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void VehicleFlowStat(UINT8 boardNum,UINT16 boardInfo)
{
    static UINT16 oldBoardInfo[3] = {0};  //保存上一次的过车数据，用来判断是否有过车，如果该bit位由0变成1就认为是该车检器有一次过车信息
    static time_t startTime = 0;     //本次记录的开始时间，也是需要保存到二进制文件中的周期起始时间
    static time_t oldTime = 0;//保存的是上一次计算绿损的时间
    static UINT8 nFlag = 0;
    static struct timespec calOccupancyStartTime[48];//计算时间占有率起始时间
    static UINT8 nFlagIsCalcOccupancy[48] = {0};//是否开始计算时间占有率的标志
    static struct timespec greenStartTime[48] ; //绿灯开始时间
    static UINT32 nTimeOccupy[48] = {0};//当前计算周期内，车辆占用车检器的时间，单位是ms.
    struct timespec currentTime;//当前时间
    UINT64 nTempLong = 0;

    UINT16 nCycleTime = gSignalControlpara->stUnitPara.byFluxCollectCycle * ((gSignalControlpara->stUnitPara.byCollectCycleUnit == 0) ? 1 : 60);//采集周期，单位是秒
    UINT8 nIsHaveCar = 0;//是否有过车，1表明有过车，0表明无过车
    UINT8 nPhaseId = 0;//检测器对应的相位
    
    int i = 0;
    time_t nowTime = time(NULL);            

    if(startTime == 0)
    {
        startTime = nowTime;
    }
    pthread_rwlock_wrlock(&gLockRealtimeVol);
    nCycleTime = (nCycleTime == 0 ? 5 : nCycleTime);

    //先判断有没有过车数据
    for(i = 0; i < 16; i++)
    {
        nIsHaveCar = 0;
        if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))//根据前一次数据位为0，后一次数据位为1来作为有车的条件
        {
            gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].byDetectorVolume++;
            nIsHaveCar = 1;
        }

        nPhaseId = gSignalControlpara->AscVehicleDetectorTable[(boardNum - 1)*16 + i].byVehicleDetectorCallPhase;
        //如果当前检测器对应的相位是绿灯，且有过车，就计算绿损时间
        if((nPhaseId >=1) && (nPhaseId < 16)&&(gCountDownParams->stVehPhaseCountingDown[nPhaseId - 1][0] == 1))
        {
            if(nFlag == 0)
            {
                nFlag = 1;
                oldTime = nowTime;
            }
            if(nIsHaveCar == 0)//如果相位是绿灯，但没有过车，那么绿损时间就要加1秒
            {
                if(nowTime - oldTime >= 1)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].wGreenLost += 1*100;
                    nFlag = 0;
                }
            }
            else
            {
                oldTime += 1;
            }

            //相位绿灯,第一次数据是0，然后是1，表明有车开始通过车检器，开始统计时间占有率
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))
            {
                clock_gettime(CLOCK_MONOTONIC, &calOccupancyStartTime[(boardNum - 1)*16 + i]);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 0)
                {
                    nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 1;
                    greenStartTime[(boardNum - 1)*16 + i] = calOccupancyStartTime[(boardNum - 1)*16 + i];
                }
            }            
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 1) && (((boardInfo >> i) & 0x01) == 0))//第一次数据是1，然后是0，表明车辆即将离开车检器，就需要记录该时间            {
            {
                clock_gettime(CLOCK_MONOTONIC, &currentTime);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
                    nTimeOccupy[(boardNum - 1)*16 + i] += (currentTime.tv_sec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_sec)*1000+(currentTime.tv_nsec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_nsec)/1000000;
                //if(i == 0)
                 //   INFO("%d  %p\n",nTimeOccupy[0],&nTimeOccupy[0]);
            }        
        }
        else//红灯时，根据是否已经统计过起始时间来计算时间占有率
        {
            if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
            {
                nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 0;
                
                clock_gettime(CLOCK_MONOTONIC, &currentTime);
                //nTempLong其实计算的就是该车检器对应的相位绿灯时间
                nTempLong = (currentTime.tv_sec - greenStartTime[(boardNum - 1)*16 + i].tv_sec)*1000+(currentTime.tv_nsec - greenStartTime[(boardNum - 1)*16 + i].tv_nsec)/1000000;

                if(nTempLong != 0)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].byDetectorOccupancy = 100.0*nTimeOccupy[(boardNum - 1)*16 + i]/nTempLong;
                }
                memset(nTimeOccupy,0,sizeof(nTimeOccupy));
            }
        }
    }

    gStructMsgRealtimeVolume.volumeOccupancy.dwTime = startTime;
    CalcStatData(&gStructMsgRealtimeVolume.volumeOccupancy,nowTime - startTime);
    
    //判断周期是否已到达，每个周期记录一次数据，每周统计一次数据
    if(nowTime - startTime >= nCycleTime)
    {
        for(i = 0; i < 48; i++)
        {
            if(nFlagIsCalcOccupancy[i] == 1)
            {
                nFlagIsCalcOccupancy[i] = 0;
                
                clock_gettime(CLOCK_MONOTONIC, &currentTime);
                nTempLong = (currentTime.tv_sec - greenStartTime[i].tv_sec)*1000+(currentTime.tv_nsec - greenStartTime[i].tv_nsec)/1000000;
                if(nTempLong != 0)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[i].byDetectorOccupancy = 10000.0*nTimeOccupy[i]/(1.0*nTempLong);
                
                   /* if(i == 0)
                    {
                        INFO("===>   %0.2f%% \n",gStructMsgRealtimeVolume.volumeOccupancy.struVolume[i].byDetectorOccupancy/100.0);
                    }*/
                }
                nTimeOccupy[i] = 0;
            }
        }
    
       // CalcStatData(&gStructMsgRealtimeVolume.volumeOccupancy,nowTime - startTime);
        RecordVehicleFlowData(&gStructMsgRealtimeVolume.volumeOccupancy);

        startTime = nowTime;
        nFlag = 0;
        memset(&gStructMsgRealtimeVolume,0,sizeof(gStructMsgRealtimeVolume));
    }
    oldBoardInfo[boardNum - 1] = boardInfo;

    pthread_rwlock_unlock(&gLockRealtimeVol);
}


/*********************************************************************************
*
* 	读车检板信号。
*
***********************************************************************************/

uint16 VehicleCheckInput(int boardNum)
{
	uint16 boardInfo = 0;
	//int iPedNO = 0;//相机编号
	
	if (boardNum<1 || boardNum>3)
	{
		INFO("boardNum error:%d\n", boardNum);
		return -1;
	}
	boardInfo = recv_date_from_vechile(boardNum);

	VehicleFlowStat(boardNum,boardInfo);
	DBG("INFO 100140:%d,boardInfo:%d\n",boardNum,boardInfo); 

	return boardInfo;
}

/*********************************************************************************
*
* 	读门开关信号。
*
***********************************************************************************/

uint16 DoorCheckInput(void)
{
	uint16 input = 0;

	input = DoorCheck();
	DBG("INFO 100142 门开关信号: 0x%x\n",input); 


	return input;
}

/*********************************************************************************
*
* 	红绿冲突、绿冲突检测。
*
***********************************************************************************/
void RedgreenCollision(int boardNum, uint16 boardInfo) 
{
	//uint16 i = 0;
	uint16 j = 0;
	static uint16 errorcountred[32] = {0};
	static uint16 normalcountred[32] = {0};
	static uint16 errorcountgreen[32] = {0};
	static uint16 normalcountgreen[32] = {0};
	time_t now;  
	struct tm *timenow;
	
	char msg[128] = "";

	if(boardNum > 4)
	{
		//只考虑16通道
		return ;
	}

	//INFO("**************************boardno:%d,g_nLampStatus:%x,boardinfo:%x\n", boardNum, g_nLampStatus[boardNum-1], boardInfo);
	for (j=0; j<4; j++)
	{
		//红绿冲突检测
		if ((((g_nLampStatus[boardNum-1]>>(j*3+1))&0x1) == 0)  && (((boardInfo>>(j*3+1))&0x1) == 1)) 
		{
			//红灯不该亮而亮则为红绿冲突
			//获取国际标准时间
			time(&now);   
			//转换为本地时间
			timenow = localtime(&now); 

			sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
				timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
				timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
			//INFO("%s 第%d通道红灯电压异常(红绿冲突征兆)，errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
			
			if (++errorcountred[(boardNum-1)*4+j] > TIMEOUT)
			{
				errorcountred[(boardNum-1)*4+j] = 0;
				normalcountred[(boardNum-1)*4+j] = 0;
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
				}
				if(g_faultstatus[(boardNum-1)*4+j] == 0)
				{
					gStructBinfileConfigPara.cFailureNumber ++;
					ERR("%s 第%d通道红绿冲突,errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
					sprintf(msg, "第%d通道红绿冲突", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = RED_GREEN_CONFLICT;          //消息类型ID
					s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //序列号
					s_failure_info.nTime = now;
	  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					
					g_faultstatus[(boardNum-1)*4+j] = 2;
					//把g_faultstatus数组写到文件
					WriteFaultStatus();
				}
			}		
		}
		else if ((((g_nLampStatus[boardNum-1]>>(j*3+1))&0x1) == 0)  && (((boardInfo>>(j*3+1))&0x1) == 0)) 
		{
			//该灭的红灯都熄灭了，正常
			if(++normalcountred[(boardNum-1)*4+j] > TIMEOUT)
			{	
				normalcountred[(boardNum-1)*4+j] = 0;
				errorcountred[(boardNum-1)*4+j] = 0;
				if(g_faultstatus[(boardNum-1)*4+j] == 2)
				{
					//获取国际标准时间
					time(&now); 
					gStructBinfileConfigPara.cFailureNumber++;
					sprintf(msg, "第%d通道红绿冲突故障消除", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = RED_GREEN_CONFLICT_CLEAR;                      //红绿冲突清除消息类型ID
					s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //序列号
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					g_faultstatus[(boardNum-1)*4+j] = 0;
					WriteFaultStatus();
				}
				
			}
			
		}

		//绿冲突检测
		if ((((g_nLampStatus[boardNum-1]>>(j*3))&0x1) == 0) && (((boardInfo>>(j*3))&0x1) == 1)) //绿灯不该亮而亮则为绿冲突
		{
			//获取国际标准时间
			time(&now);  
			  
			//转换为本地时间
			timenow = localtime(&now); 

			sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
				timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
				timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
			//INFO("%s 第%d通道绿灯电压异常，(绿冲突征兆),errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
			if (++errorcountgreen[(boardNum-1)*4+j] > TIMEOUT)
			{		
				errorcountgreen[(boardNum-1)*4+j] = 0;
				normalcountgreen[(boardNum-1)*4+j] = 0;
				
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
				}
				gStructBinfileConfigPara.cFailureNumber++;
				ERR("%s 第%d通道绿冲突,errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
				sprintf(msg, "第%d通道绿冲突", (boardNum-1)*4+j+1);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				s_failure_info.nID = GREEN_CONFLICT;                      //绿冲突消息类型ID
				s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //序列号
				s_failure_info.nTime = now;
  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
				WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				g_faultstatus[(boardNum-1)*4+j] = 1;
				//把g_faultstatus数组写到文件
				WriteFaultStatus();				
			}		
		}
		else if((((g_nLampStatus[boardNum-1]>>(j*3))&0x1) == 0) && (((boardInfo>>(j*3))&0x1) == 0))
		{
			//该灭的绿灯都灭了，正常
			if(++normalcountgreen[(boardNum-1)*4+j] > TIMEOUT)
			{	
				normalcountgreen[(boardNum-1)*4+j] = 0;
				errorcountgreen[(boardNum-1)*4+j] = 0;
				if(g_faultstatus[(boardNum-1)*4+j] == 1)
				{
					//获取国际标准时间
					time(&now); 
					gStructBinfileConfigPara.cFailureNumber++;
					sprintf(msg, "第%d通道绿冲突故障消除", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = GREEN_CONFLICT_CLEAR;                      //红绿冲突清除消息类型ID
					s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //序列号
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					g_faultstatus[(boardNum-1)*4+j] = 0;
					WriteFaultStatus();
				}
				
			}
			
		}
	}

}

/*********************************************************************************
*
* 读相位板电压信号。
*
***********************************************************************************/

uint16 PhaseLampVoltInput(int boardNum)
{
	if (boardNum<1 || boardNum>8)
	{
		ERR("boardNum error:%d\n", boardNum);
		return 0;
	}
	char msg[128] = "";
	time_t now;  
	struct tm *timenow;
	uint16 boardInfo = 0;
	if (g_errorstat == 1)
	{
		return ~boardInfo;
	}
	i_can_its_get_Volt(boardNum, &boardInfo);
	//获取国际标准时间
	time(&now);  
	  
	//转换为本地时间
	timenow = localtime(&now); 

	sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
	DBG("%s  INFO 100131: boardNum:%d, boardInfodes:%d, boardInfosrc:%d\n",msg, boardNum, boardInfo, g_nLampStatus[boardNum-1]&0xfff); 
	
	//红绿冲突检测
	RedgreenCollision(boardNum, boardInfo);
	return ~boardInfo;
}

#define Time_To_Minutes(hour,minute,second) ((hour)*3600+(minute)*60+second)

/*****************************************************************************
 函 数 名  : IsDateInControlArea
 功能描述  : 判断当前时间是否在指定时间段内
 输入参数  : CHANNEL_LOCK_PARAMS *pChannnelLockedParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月10日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned char IsDateInControlArea(CHANNEL_LOCK_PARAMS *pChannnelLockedParams)
{
	time_t timep = time(NULL);
    struct tm now;

	localtime_r(&timep, &now);

    unsigned int iBeginTime = 0;
    unsigned int iEndTime = 0;
    unsigned int iNow = 0;

    iBeginTime = Time_To_Minutes(pChannnelLockedParams->ucBeginTimeHour,pChannnelLockedParams->ucBeginTimeMin,pChannnelLockedParams->ucBeginTimeSec);
    iEndTime = Time_To_Minutes(pChannnelLockedParams->ucEndTimeHour,pChannnelLockedParams->ucEndTimeMin,pChannnelLockedParams->ucEndTimeSec);
    iNow = Time_To_Minutes(now.tm_hour,now.tm_min,now.tm_sec);

    //normal 
    if(iBeginTime < iEndTime)
    {
        if((iNow > iBeginTime) && (iNow < iEndTime))
        {
            return 1;
        }
    }
    else//当起始时间比结束时间大时
    {
        if((iNow > iEndTime ) && (iNow < iBeginTime))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : IsStartLockChannel
 功能描述  : 判断是否开始锁定通道，如果没有收到锁定命令则返回0，若收到锁定命
             令但当前时间段内不锁定，则返回待锁定，否则返回锁定
 输入参数  : CHANNEL_LOCK_PARAMS *pChannnelLockedParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月31日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
unsigned char IsStartLockChannel(CHANNEL_LOCK_PARAMS *pChannnelLockedParams)
{
    if(gStructBinfileCustom.cChannelLockFlag == 0)//如果没有通道锁定命令，则直接返回。
    {
        return 0;//不锁定
    }

    //如果启用了工作区域但当前时间并不在工作区域内，则直接返回2。
    if((pChannnelLockedParams->ucWorkingTimeFlag == 1) && (IsDateInControlArea(pChannnelLockedParams) == 0))
    {
        gStructBinfileCustom.cChannelLockFlag = 2;
        return 2;//待锁定
    }

    gStructBinfileCustom.cChannelLockFlag = 1;//锁定
    return 1;
}

/*****************************************************************************
 函 数 名  : LockChannel
 功能描述  : 根据是否锁定通道改变通道状态
 输入参数  : CHANNEL_LOCK_PARAMS *pChannnelLockedParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月10日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void LockChannel(CHANNEL_LOCK_PARAMS *pChannnelLockedParams)
{
	unsigned char lock=0;
	static unsigned char delayFlag=0;
	static unsigned char cArrayBlinkStatus[32] = {0};//保存在绿闪、黄闪时上一次灯色的状态，用来决定此时是否应该灭灯    
	static unsigned char cArrayIsTimeEnd[32] = {0};//闪亮时，要保证状态持续时间达500ms.	
	static unsigned char chanChanged=0;
	static unsigned char cChanStatus[32] = {0};
	
	if(pChannnelLockedParams == NULL)//如果没有通道锁定命令，则直接返回。    
	{
		return;
	}
	if(IsStartLockChannel(pChannnelLockedParams) != 1)
		lock=0;
	else
		lock=1;
	LockByChan(lock, pChannnelLockedParams->ucChannelStatus, &delayFlag, &chanChanged, cChanStatus,cArrayBlinkStatus,cArrayIsTimeEnd);
}
static UINT8 IsDateInControlAreas(STRU_Channel_Lock_V2_INFO *pNewChannnelLockedParams)
{
	int i=0;
	Boolean timeFlag = FALSE;
	time_t timep = time(NULL);
    struct tm now;

	localtime_r(&timep, &now);

    unsigned int iBeginTime = 0;
    unsigned int iEndTime = 0;
    unsigned int iNow = 0;

    iNow = Time_To_Minutes(now.tm_hour,now.tm_min,now.tm_sec);

	for(i=0; i<16; i++)
	{
		if(pNewChannnelLockedParams->stChannelLockPeriods[i].ucWorkingTimeFlag != 1)
			continue;
		iBeginTime = Time_To_Minutes(pNewChannnelLockedParams->stChannelLockPeriods[i].ucBeginTimeHour, pNewChannnelLockedParams->stChannelLockPeriods[i].ucBeginTimeMin,pNewChannnelLockedParams->stChannelLockPeriods[i].ucBeginTimeSec);
		iEndTime = Time_To_Minutes(pNewChannnelLockedParams->stChannelLockPeriods[i].ucEndTimeHour,pNewChannnelLockedParams->stChannelLockPeriods[i].ucEndTimeMin,pNewChannnelLockedParams->stChannelLockPeriods[i].ucEndTimeSec);
		if(iBeginTime <= iEndTime)
		{
			if(iNow >=iBeginTime && iNow < iEndTime)
				return i+16;	//锁定,返回时段号+16
		}
		else
		{
			if(iNow >= iBeginTime || iNow < iEndTime)
				return i+16;	//锁定
		}
		if(timeFlag == FALSE)
			timeFlag = TRUE;
	}
	if(timeFlag == TRUE)
		return 2;		//当前时间不在时段内，待锁定
	return 0;//未锁定
}

static UINT8 IsStartPeriodLockChannel(STRU_Channel_Lock_V2_INFO *pNewChannnelLockedParams)
{
	UINT8 ret=0;
	if(gStructBinfileCustom.sNewChannelLockedParams.uChannelLockFlag == 0 || gStructBinfileCustom.cChannelLockFlag == 1)//原通道锁定优先
		return 0;
	ret = IsDateInControlAreas(pNewChannnelLockedParams);
	if(ret >= 16)
		gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus = 1;
	else
		gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus = ret;
	
	return ret;
}
void PeriodLockChannel(STRU_Channel_Lock_V2_INFO *pNewChannnelLockedParams)
{
	unsigned char lock=0;
	UINT8 iPeriodNum = 0;
	unsigned char *pchan=NULL;
	static unsigned char delayFlag=0;
	static unsigned char cArrayBlinkStatus[32] = {0};//保存在绿闪、黄闪时上一次灯色的状态，用来决定此时是否应该灭灯    
	static unsigned char cArrayIsTimeEnd[32] = {0};//闪亮时，要保证状态持续时间达500ms.	
	static unsigned char chanChanged=0;
	static unsigned char cChanStatus[32] = {0};

	//启用原通道锁定时，新通道锁定不起作用
	if(pNewChannnelLockedParams == NULL)
	{
		return;
	}
	if((iPeriodNum = IsStartPeriodLockChannel(pNewChannnelLockedParams)) <16)
	{
		lock = 0;
	}
	else
	{
		iPeriodNum -=  16;//转换为索引,即为时段号
		if(iPeriodNum<0 || iPeriodNum >= 16)
		{
			ERR("Wrong period number: %d", iPeriodNum);
			return;
		}
		lock = 1;
		pchan = pNewChannnelLockedParams->stChannelLockPeriods[iPeriodNum].ucChannelStatus;
	}
	

	LockByChan(lock, pchan, &delayFlag, &chanChanged, cChanStatus,cArrayBlinkStatus,cArrayIsTimeEnd);
	return;
}


/*********************************************************************************
*
*  写灯控板信号灯。
*
***********************************************************************************/
int PhaseLampOutput(int boardNum, uint16 outLamp)
{
	struct msgbuf msg;
	static int nCount = 0;
	//char *temp_LampStatus = NULL;
	DBG("INFO 100130:%d:0x%x|",boardNum, outLamp); 
#ifdef DEBUG
	print_data(outLamp);
#endif
	
	if (boardNum<1 || boardNum>8)
	{
		ERR("boardNum error:%d\n", boardNum);
		return -1;
	}

	g_nLampStatus[boardNum-1] = outLamp;
	g_nLightStatus[boardNum-1] = outLamp;
	if (boardNum == 8)
	{
		nCount ++;
		ItsCustom();
		CountDownPulse();
		PeriodLockChannel(&gStructBinfileCustom.sNewChannelLockedParams);
		WirelessChanLock();
		LockChannel(&gStructBinfileCustom.sChannelLockedParams);
		msg.mtype = MSG_RED_SIGNAL_CHECK;
		if (gStepFlag == 0)
		{
			memcpy(msg.lightArray, g_nLampStatus, sizeof(msg.lightArray));
			i_can_its_send_led_request(boardNum, g_nLampStatus);
		}
		else
		{
		    pthread_rwlock_rdlock(&gLightArrayLock);
			memcpy(msg.lightArray, gLightArray, sizeof(msg.lightArray));
            i_can_its_send_led_request(boardNum, gLightArray);
		    pthread_rwlock_unlock(&gLightArrayLock);
		}
		if (gStructBinfileConfigPara.sSpecialParams.iRedSignalCheckSwitch == 1)
			msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
			
		//将1s4次的点灯值存到nLampStatus二维数组中
		memcpy(nLampStatus[nCount-1],msg.lightArray,sizeof(msg.lightArray));
		if(nCount == 4)
		{
			//获取最终输出通道状态(1S一次，供倒计时接口输出使用)
			GetFinalChannelStatus();
			nCount = 0;
		}
		
		//主控板运行指示灯
		Hiktsc_Running_Status();
		//硬件看门狗喂狗函数
		HardwareWatchdogKeepAlive(boardNum);
	}

	//电压检测
	if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmSwitch == 1)
	{
		PhaseLampVoltInput(boardNum);
	}
	
	
	return 0;
}


/*********************************************************************************
*
* 	红灯熄灭检测。
*
***********************************************************************************/
void RedExtinguish() //红灯该亮而不亮则为红灯熄灭
{
	uint16 i = 0;
	uint16 j = 0;
	uint16 pcurInfo = 0;
	static uint16 errorcountred[32] = {0};
	static uint16 normalcountred[32] = {0};
	char msg[128] = "";
	time_t now; 
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pcurInfo = i_can_its_get_cur(i+1, j+1, 1);   //第３个参数写死１,只获取红灯电流
			g_RedCurrentValue[i*4+j] = pcurInfo;

			//if(pcurInfo > 0)
			//{
			//	INFO("通道%02d红灯电流值:%03d\n",i*4+j+1,pcurInfo);
			//}
			if ((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo < (gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentDiff) )
			{	
				//INFO("通道%02d红灯亮并且检测电流异常,errorcountred[%d]=%d pcurInfo=%d\n",i*4+j+1,i*4+j+1,errorcountred[i*4+j]+1,pcurInfo);
				//红灯亮并且电流检测异常
				if (++errorcountred[i*4+j] > 5)
				{
					errorcountred[i*4+j] = 0;
					normalcountred[i*4+j] = 0;
					if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch == 1)
					{
						//g_yellowflash = 1;
						//标记为有故障
						g_errorstat = 1;
						SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
					}
					if(g_faultstatus[i*4+j] == 0)
					{
						//获取国际标准时间
						time(&now); 
						
						
						gStructBinfileConfigPara.cFailureNumber ++;
						ERR("第%d通道红灯熄灭\n", i*4+j+1);
						sprintf(msg, "第%d通道红灯熄灭", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						s_failure_info.nID = RED_LIGHT_OFF;                      //红灯熄灭消息类型ID
						s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //序列号
						s_failure_info.nTime = now;
	  					s_failure_info.nValue = i*4+j+1;     //通道号   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						g_faultstatus[i*4+j] = 3;
						//把g_faultstatus数组写到文件
						WriteFaultStatus();
						//return;
					}
				}
				//return;
			}
			else if((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo > (gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentDiff))
			{
				
				//该通道红灯正常亮并且电流值正常
				if (++normalcountred[i*4+j] > 5)
				{
					normalcountred[i*4+j] = 0;
					errorcountred[i*4+j] = 0;
					//该通道红灯写灭检测故障消除
					if (g_faultstatus[i*4+j] == 3)
					{
						time(&now); 
						sprintf(msg, "第%d通道红灯熄灭故障消除", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						gStructBinfileConfigPara.cFailureNumber ++;
						s_failure_info.nID = RED_LIGHT_OFF_CLEAR;                      //红灯熄灭消除消息类型ID
						s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //序列号
						s_failure_info.nTime = now;
			  			s_failure_info.nValue = i*4+j+1;     //通道号   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
									s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						g_faultstatus[i*4+j] = 0;
						WriteFaultStatus();
					}
				}
			}
		}
	}
}

/*********************************************************************************
*
*	写硬黄闪信号。
*
***********************************************************************************/

void HardflashDogOutput(void)
{
#if 0	//changed by Jicky
	uint16 pcurInfo = 0;
	if (HardflashDogFlag == 1)
	{
		HardflashDogFlag = 0;
	}
	else
	{
		HardflashDogFlag = 1;
	}
	
	HardflashDogCtrl(HardflashDogFlag);

	printf("INFO 100150 硬黄闪信号:%d\n", HardflashDogFlag); 
#endif
	CurDetectFreq++;
	if( CurDetectFreq > 2 )
	{
		//电流检测
		if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmSwitch == 1
			&& gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 1)
		{
			RedExtinguish();
		}
		CurDetectFreq = 0;
	}
	return;
}

unsigned char VehicleCheckFaultInput(unsigned char phase)
{
	return  0; 
}


/*********************************************************************************
*
* 	读相位板电流值。(弃用)
*
***********************************************************************************/

unsigned int PhaseLampCurInput(int boardNum, int pahseNum, int redGreen)
{
	if (boardNum<1 || boardNum>8 || pahseNum<1 || pahseNum>4)
	{
		ERR("boardNum or pahseNum error:%d, %d\n", boardNum, pahseNum);
		return 0;
	}
	uint16 curInfo = 0;
	if (g_errorstat == 1)
	{
		return curInfo;
	}
	DBG("INFO 100132:%d  %d   %d\n",boardNum, pahseNum, redGreen); 
	curInfo = i_can_its_get_cur(boardNum, pahseNum, redGreen);
	DBG("相位板电流值:%d\n", curInfo);

	//RedExtinguish();

	curInfo &= 0x00ff;
	if (curInfo == 0xff)
	{
		curInfo = 0;
	}

	return curInfo;
}

/*********************************************************************************
*
* 	读行人按钮信号。
*
***********************************************************************************/

uint16 PedestrianCheckInput(void)
{
	uint16 input = PedestrianCheck();

	DBG("INFO 100141 行人按钮: 0x%x\n",input); 
	//INFO("---PedestrianCheckInput...");
	if(WIRELESS_CTRL_ON == gStructBinfileConfigPara.stWirelessController.iSwitch)
		WirelessKeyCheck();

	return input;
}


/*********************************************************************************
*
*  键盘处理
	返回值:
	0: 无键按下.
	1:自动键按下,且已放开.
	2:手动键按下,且已放开.
	3:黄闪键按下,且已放开.
	4:全红键按下,且已放开.
	5:步进键按下,且已放开.
*
***********************************************************************************/
extern int KeyControlDeal(int result);
int ProcessKey(void)
{
	int result = 0;
	char tmpstr[256] = "";
	result = ProcessKeyBoard();
	
	DBG("INFO 100600: 键盘处理 return %d,g_manual_pressed=%d,g_auto_pressed=%d,g_flashing_pressed=%d,g_allred_pressed=%d,g_step_by_step_pressed=%d \n",
		result,g_manual_pressed,g_auto_pressed,
		g_flashing_pressed,g_allred_pressed,g_step_by_step_pressed);

	if(result != 0)
	{
		sprintf(tmpstr,"键盘处理 return %d,手动=%d,自动=%d,黄闪=%d,全红=%d,步进=%d \n",
			result,g_manual_pressed,g_auto_pressed,
			g_flashing_pressed,g_allred_pressed,g_step_by_step_pressed);
	
		WriteLogInfos("/home/Keyboard.log",tmpstr);
	}
	
	return KeyControlDeal(result);
	//return (0);
}

/*********************************************************************************
*
* 	写开机运行日志。
*
***********************************************************************************/
void write_running_log()
{
	//写开机日志(注:文件的上级目录必须已经存在，否则无法正常写开机日志)
	char msg[128] = "";
	sprintf(msg,"信号机程序开始运行,当前程序版本号:%s",SOFTWARE_VERSION_INFO);
	WriteLogInfos("/home/StartUp.log",msg);
	ReadFaultStatus();
}

int itstaskmain(int argc, char *argv[]);

void SigHandler(int signum) //kill or killall will be called to free memory!
{
    INFO("signum = %d", signum);
	if (signum == SIGUSR1)
	{
		gOftenPrintFlag = !gOftenPrintFlag;
		return;
	}
	else
    	exit(0);
}
//It can be called after main() or exit() automatically!
__attribute__((destructor)) void ProgramExit(void)
{
    if (gSignalControlpara == NULL)
        free(gSignalControlpara);
    if (gCountDownParams == NULL)
        free(gCountDownParams);
    INFO("The program exit normaly!");
}
/*********************************************************************************
*
* 	主程序入口
*
***********************************************************************************/
int main(int argc, char *argv[])
{	
	INFO("********************************main()****************************\n");
	INFO("compile time: %s, %s", __DATE__, __TIME__);
	//注册一些信号用于释放一些资源或是其他用途
	signal(SIGTERM, SigHandler);    //for command 'kill' or 'killall'
	signal(SIGINT, SigHandler);		//for ctrl + C
	signal(SIGUSR1, SigHandler);	//for OFTEN print
	//写开机运行日志并记录软件版本号
	write_running_log();

	//从配置文件中获取所有参数配置
	InitCountDownParams();//必须要先给倒计时分配空间，再读文件，否则会造成段错误。
	ReadBinAllCfgParams(&gStructBinfileConfigPara,&gStructBinfileCustom,&gStructBinfileDesc,&g_CountDownCfg,&gStructBinfileMisc);
	InitSignalMachineConfig();
	StepPthreadInit();
	//if (gStructBinfileConfigPara.sSpecialParams.iRedSignalCheckSwitch == 1)
		gStructBinfileConfigPara.sSpecialParams.iRedSignalCheckSwitch = RedSignalCheckInit();
	
	//打开sadp服务器程序
	system("killall -9 RTSC_sadp");
	system("sleep 5 && /root/RTSC_sadp  >& /dev/null &");
	system("killall transfer");
	system("/root/transfer  >& /dev/null &");

	//CPU通过CPLD对外的IO口初始化
	IO_Init();
		
	//can通信初始化
	i_can_its_init();

	//特殊参数udp服务启动
	udp_server_init();
	
	//默认点亮自动灯
	ProcessKeyBoardLight();

	//软件看门狗初始化
	HardwareWatchdogInit();

	//GPS初始化
	GPSInit();
	
    RecordNewFault(POW_ON_REBOOT);
	return itstaskmain(argc, argv);
}


/*********************************************************************************
*
* 	保存时间到硬件RTC
*
***********************************************************************************/

void RTC_HW_fc(void)
{
	system("hwclock -w&");
	INFO("rtc hw fc.\n");
}

/*其中color: 1:绿灯，2:红灯，3:黄灯*/
void CountDownOutVeh(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    //从20151225起，步进不再给老库发，如果当前正在步进，则此处不再被调用
    if(gStepFlag != 0)
    {
        return;
    }    

	//OFTEN("veh:phase %d color %d phaseTime %d \n", phase, color, phaseTime);
	countdown_veh[phase].veh_color = color;
	countdown_veh[phase].veh_phaseTime = phaseTime;

	if(!(gCountDownParams->stVehPhaseCountingDown[phase - 1][0] == GREEN_BLINK
	    && color == GREEN))//绿闪是在计算倒计时接口中计算出来的，如果相位是绿闪且老库传过来的状态是绿灯，则不予修改其颜色状态
	{
    	gCountDownParams->stVehPhaseCountingDown[phase - 1][0] = color;
	}

	gCountDownParams->stVehPhaseCountingDown[phase - 1][1] = phaseTime;
	CollectCountDownParams();

	/*设置感应检测时间*/
	if((SetFreeGreenTimeSign == 0)&&(gStructBinfileCustom.sCountdownParams.iFreeGreenTime > 3))
	{
		SeFreeGreenTime(gStructBinfileCustom.sCountdownParams.iFreeGreenTime);//感应检测时间,缺省为9秒.
		//printf("#Set freeGreenTime %d\n",gStructBinfileCustom.sCountdownParams.iFreeGreenTime);
		SetFreeGreenTimeSign = 1;
	}
}

void CountDownOutPed(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    //从20151225起，步进不再给老库发，如果当前正在步进，则此处不再被调用
    if(gStepFlag != 0)
    {
        return;
    }    

	//INFO("ped:phase %d color %d phaseTime %d \n", phase, color, phaseTime);
	countdown_ped[phase].ped_color = color;
	countdown_ped[phase].ped_phaseTime = phaseTime;
	gCountDownParams->stPedPhaseCountingDown[phase - 1][0] = color;
	gCountDownParams->stPedPhaseCountingDown[phase - 1][1] = phaseTime;
	CollectCountDownParams();
}
//脉冲全程倒计时
void Count_Down_pulse_All(uint16 *g_nLampStatus)
{
	int iPhase = 0;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;

	memcpy(temp_nLampStatus,g_nLampStatus,16);
	for(iPhase = 1; iPhase<16; iPhase++)
	{
		if(countdown_veh[iPhase].veh_color > 0)
		{
			if(countdown_veh[iPhase].veh_color == 2)
			{
				nowRedLight[iPhase] = 1;//红灯
			}
			else
			{
				nowRedLight[iPhase] = 0;//不是红灯
			}
			if(nowRedLight[iPhase] != lastRedLight[iPhase])
			{
				stayTimes[iPhase]++;
				if(stayTimes[iPhase] == 2)
				{
					lastRedLight[iPhase] = nowRedLight[iPhase];
					for(iChannel=0; iChannel<32; iChannel++)
					{
						if(gStructBinfileCustom.sCountdownParams.iPhaseOfChannel[iChannel].iphase == iPhase)
						{
							dataNum1 = iChannel/4;
							dataNum2 = iChannel%4;
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+1)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+2)));
						}
					}
				}
			}
			else
			{
				stayTimes[iPhase] = 0;//记数复位
			}
		}
	}
	memcpy(g_nLampStatus,temp_nLampStatus,16);
}
//脉冲半程倒计时
void Count_Down_pulse_Half(uint16 *g_nLampStatus)
{
	int iPhase = 0;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;

	memcpy(temp_nLampStatus,g_nLampStatus,16);
	for(iPhase = 1; iPhase<16; iPhase++)
	{
		if(countdown_veh[iPhase].veh_color > 0)
		{
			if(((countdown_veh[iPhase].veh_phaseTime == gStructBinfileCustom.sCountdownParams.iPulseGreenTime) && (countdown_veh[iPhase].veh_color == 1))
			||((countdown_veh[iPhase].veh_phaseTime == gStructBinfileCustom.sCountdownParams.iPulseRedTime) && (countdown_veh[iPhase].veh_color == 2)))//半程倒计时
			{
				if(temp_Sign[iPhase] == 0)
				{
					for(iChannel=0; iChannel<32; iChannel++)
					{
						if(gStructBinfileCustom.sCountdownParams.iPhaseOfChannel[iChannel].iphase== iPhase)
						{
							dataNum1 = iChannel/4;
							dataNum2 = iChannel%4;
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+1)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+2)));
						}
					}
					temp_Sign[iPhase] = 1;
				}
			}
			else
			{
				temp_Sign[iPhase] = 0;//标志位复位
			}
		}
	}
	memcpy(g_nLampStatus,temp_nLampStatus,16);
}
void CountDownPulse(void)
{
	if(1 == gStructBinfileCustom.sCountdownParams.iCountDownMode)
	{
		Count_Down_pulse_All(g_nLampStatus);
	}
	else if(2 == gStructBinfileCustom.sCountdownParams.iCountDownMode)
	{
		Count_Down_pulse_Half(g_nLampStatus);
	}
}
static void LockByChan(unsigned char flag, unsigned char *chanConfig, unsigned char *delayFlag,unsigned char *chanChangedFlag,unsigned char *cChanStatus,unsigned char *cArrayBlinkStatus,unsigned char *cArrayIsTimeEnd)
{
	int i = 0;
	unsigned short  value;
	unsigned char cFlag = 0;
/*
	time:	1s	3s	3s	---------	3s	---
	isLock:	0	0	0	1			1	0
	action:	lib	GB	Y	chanstatus	Y	lib
*/
	int ret=0;
	
	if(flag==0)
	{
		if(*delayFlag!=0)//lock just stop
		{
			if(transitionOnChanLockEnd(cChanStatus) == 1)//lock end transition finished
			{
				*delayFlag=0;
				memset(cChanStatus, 0, 32);
				return;
			}
		}
		else//no lock
		{
			return;
		}
	}
	if(!(*delayFlag))
	{
		ret = transitionOnChanLockStart(cChanStatus, chanConfig);
		if(ret == 0)
			return;
		else if(ret == 2)
		{
			memcpy(cChanStatus, chanConfig, 32);
			*delayFlag=1;
		}
	}
	else
	{
		if((flag == 1)&&(strncmp((char*)cChanStatus, (char*)chanConfig, 32)!=0))//channel value changed
		{
			//INFO("channel value changed!");
			*chanChangedFlag=1;
		}
	}
	if(*chanChangedFlag)
	{
		if(trainsitionOnChanChange(cChanStatus, chanConfig) == 1)
		{
			*chanChangedFlag=0;
			memcpy(cChanStatus, chanConfig, 32);
		}
	}
	for(i = 0 ; i < 32; i++)
	{
		cFlag = 0;
		
//		  switch(pChannnelLockedParams->ucChannelStatus[i])
		switch(cChanStatus[i])
		{
			case GREEN: 			value = 1;	  break;
			case RED:				value = 2;	  break;
			case YELLOW:			value = 4;	  break;
			case GREEN_BLINK:		
									if(++cArrayIsTimeEnd[i] < 2)
									{
										value = cArrayBlinkStatus[i];
										break;
									}
									cArrayIsTimeEnd[i] = 0;
									value = (cArrayBlinkStatus[i] == 0) ? 1 : 0;	
									cArrayBlinkStatus[i] = value;
									break;
			case YELLOW_BLINK:		
									if(++cArrayIsTimeEnd[i] < 2)
									{
										value = cArrayBlinkStatus[i];
										break;
									}
									cArrayIsTimeEnd[i] = 0; 								   
									value = (cArrayBlinkStatus[i] == 0) ? 4 : 0;  
									cArrayBlinkStatus[i] = value;
									break;
			case TURN_OFF:			value = 0;	  break;
			case INVALID:			cFlag = 1;	  break;
			default :				cFlag = 1;	  break;
		}

		if(cFlag == 0)//如果是忽略或者默认情况下，均不调用该接口。
		{
			put_lamp_value(&g_nLampStatus[i/4], i % 4, value);
		}
	}
}

#define SECS_OF_BLINK 	3
static int transitionOnChanLockStart(unsigned char *chan, unsigned char *chanConfig)
{
	int i,ret;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;
	static unsigned char chanLockCounter=0;
	static unsigned char locktime=0;
	static unsigned char lastChans[3][32]={{0},{0},{0}};
	unsigned short lightv[3]={0};//red yellow green
	unsigned short lights[3]={GREEN, RED, YELLOW};

	if(chan==NULL || chanConfig==NULL)
	{
		ERR("Channel value can't be NULL!");
		return 2;
	}
	
	chanLockCounter++;
	if(chanLockCounter < 4)
	{
		ret=0;
		for(iChannel=0; iChannel<32; iChannel++)
		{
			dataNum1 = iChannel/4;
			dataNum2 = iChannel%4;
			for(i=0; i<3; i++)
			{
				lightv[i] = (0x0001<<(dataNum2*3+i));
				if((g_nLampStatus[dataNum1] & lightv[i]) == lightv[i])
				{
					lastChans[chanLockCounter-1][iChannel]=lights[i];
					break;
				}
			}
			if(i == 3)
				lastChans[chanLockCounter-1][iChannel] = 0;	
		}
	
		if(3 == chanLockCounter)
		{
			for(i=0; i<32; i++)
			{
				//INFO("chan:%d, val: %d-%d-%d", i,lastChans[0][i],lastChans[1][i],lastChans[2][i]);
				if(chanConfig[i]==INVALID || chanConfig[i]==TURN_OFF)
				{
					chan[i]=chanConfig[i];
					continue;
				}
				if((lastChans[0][i]==lastChans[1][i]) && (lastChans[1][i]==lastChans[2][i]))
				{
					if(lastChans[0][i]==0)
						chan[i]=TURN_OFF;
					else
						chan[i]=lastChans[0][i];
					if(chan[i] == GREEN)
						INFO("chan:%d state green",i);
					continue;
				}
				if(((lastChans[0][i]&lastChans[1][i])&lastChans[2][i]) ==0)//blink
				{
					if(lastChans[0][i]==0)
						chan[i]=lastChans[2][i];
					else
						chan[i]=lastChans[0][i];
					chan[i]=((chan[i]==GREEN)?GREEN_BLINK : YELLOW_BLINK);
					INFO("---> Blink: chan:%d,value:%d...",i, chan[i]);
				}
				else//no blink 
				{
					chan[i]=lastChans[2][i];
				}
			}
			locktime = transitionControl(chan, chanConfig);
			INFO("--->lockstart: locktime: %d", locktime);
			memset(lastChans, 0, 3*32);
			if(locktime == 0)
			{
				chanLockCounter = 0;
				ret=2;
			}
			else
				ret=1;
		}
	}
	else
	{
		ret=1;//lock
		if(chanLockCounter%4==0)
			INFO("-->lockstart: lock 1s...");
		if((locktime>=(SECS_OF_BLINK*2)) && (chanLockCounter==(SECS_OF_BLINK*4+3)))
		{
			INFO("-->Start to Yellow...");
			for(i=0; i<32; i++)
			{
				if(chan[i]==GREEN_BLINK)
					chan[i]=YELLOW;
			}
		}
		if(chanLockCounter >= (locktime*4+3))
		{
			chanLockCounter = 0;
			ret=2;// release lock
		}
	}
	
	return ret;
}
static int transitionOnChanLockEnd(unsigned char *chan)
{
	int iChannel,ret=0;
	int dataNum1 = 0;
	int dataNum2 = 0;
	unsigned short lightv=0;//red
	static unsigned char chanLockCounter=0;
	static unsigned char locktime=0;//SECS_OF_BLINK;

	chanLockCounter++;	
	if(chan == NULL)
		return ret;

	if(chanLockCounter == 1)
	{
		for(iChannel=0; iChannel<32; iChannel++)
		{
			if(chan[iChannel] == INVALID || chan[iChannel] == TURN_OFF)
				continue;
			
			dataNum1 = iChannel/4;
			dataNum2 = iChannel%4;
			lightv=(0x0001<<(dataNum2*3+1));//red
			if((g_nLampStatus[dataNum1] & lightv) == lightv)
			{
				if(chan[iChannel]==GREEN || chan[iChannel]==GREEN_BLINK || chan[iChannel]==YELLOW_BLINK)
				{// 	green/greenblink/yellowblink --> yellow
					chan[iChannel] = YELLOW;
					locktime = SECS_OF_BLINK;
				}
			}
		}
		INFO("---lockend: locktime %d", locktime);
		if(locktime == 0)
		{
			chanLockCounter = 0;
			ret = 1;
		}
		INFO("---chan 1/2/3/4: %d,%d,%d,%d", chan[0], chan[1], chan[2], chan[3]);
	}
	else
	{
		if(chanLockCounter%4==0)
			INFO("---lockend: lock 1s...");
		if(chanLockCounter >= (locktime*4))
		{
			chanLockCounter = 0;
			ret = 1;
		}
	}
	return ret;
}
static unsigned char transitionControl(unsigned char *chan, unsigned char *config)
{
	int i,time=0;
	if(chan==NULL || config==NULL)
		return 0;

	for(i=0; i<32; i++)
	{
		if(config[i] == INVALID || config[i] == TURN_OFF || config[i] != RED)
		{
			continue;
		}
		
		if(chan[i]==GREEN || chan[i]==GREEN_BLINK)
		{
			chan[i]=GREEN_BLINK;
			if(time < SECS_OF_BLINK*2)
				time = SECS_OF_BLINK*2;
		}
	}
	return time;
}
static int trainsitionOnChanChange(unsigned char *cur, unsigned char *config)
{
	int i,ret=0;
	static unsigned char counter=0;
	static unsigned char time=0;
	
	if(cur == NULL || config == NULL)
		return 0;

	counter++;
	if(counter == 1)
	{
		for(i=0; i<32; i++)
		{
			if(config[i] == INVALID || config[i] == TURN_OFF || config[i] != RED)
			{
				continue;
			}

			if(cur[i]==GREEN || cur[i] == GREEN_BLINK)
			{
				cur[i]=YELLOW;
				if(time==0)
					time=SECS_OF_BLINK;
			}
		}
		if(time==0)
		{
			counter=0;
			ret=1;
		}
	}
	else
	{
		if(counter >= (time*4))
		{
			counter=0;
			ret=1;
			time=0;
		}
	}

	return ret;
}
void FollowPhaseGreenBlink(UINT8 *chan)
{
	int i;
	int ctrlId=0;

	for(i=0; i<NUM_CHANNEL; i++)
	{
		if((gSignalControlpara->stChannel[i].nControllerID==0) || (gSignalControlpara->stChannel[i].nControllerType!=FOLLOW))
			continue;
		
		ctrlId = gSignalControlpara->stChannel[i].nControllerID-1;
		if(ctrlId >= NUM_PHASE)
		{
			ERR("Wrong phase id!");
			return;
		}
		if(!gFollowPhaseGreenBlinkFlag[ctrlId])
			continue;
		chan[i] = GREEN_BLINK;
	}
}
void WirelessChanLock(void)
{
	UINT8 flag=1;
	static unsigned int wirelessCounter=0;
	static unsigned char transitionLock=0;
	static unsigned char cArrayBlinkStatus[32] = {0};//保存在绿闪、黄闪时上一次灯色的状态，用来决定此时是否应该灭灯    
	static unsigned char cArrayIsTimeEnd[32] = {0};//闪亮时，要保证状态持续时间达500ms.	
	static unsigned char chanChanged=0;
	static unsigned char cChanStatus[32] = {0};
	unsigned char *chan=NULL;
	
	if(0 == gWirelessCtrlLockChanId)
	{
		flag = 0;
	}
	else
	{
	
		wirelessCounter++;
		if(gLastWirelessCtrlId != gWirelessCtrlLockChanId)
		{
			wirelessCounter = 0;
			gLastWirelessCtrlId = gWirelessCtrlLockChanId;
		}
		if((gStructBinfileConfigPara.stWirelessController.iOvertime!=0)&&(wirelessCounter>=(4*gStructBinfileConfigPara.stWirelessController.iOvertime)))
		{
			gWirelessCtrlLockChanId=0;
			flag = 0;
			wirelessCounter=0;
		}
		chan = gStructBinfileConfigPara.stWirelessController.key[gWirelessCtrlLockChanId-1].ucChan;
	}
	LockByChan(flag, chan, &transitionLock, &chanChanged, cChanStatus,cArrayBlinkStatus,cArrayIsTimeEnd);
}
void GetChanStatus(UINT16 *lampStatus, UINT8 *chanStatus)
{
	int ibit = 0;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;
	unsigned short lightv[3]={0};//red yellow green
	unsigned char lights[3]={GREEN, RED, YELLOW};

	if(lampStatus == NULL || chanStatus == NULL)
		return;
	
	for(iChannel=0; iChannel<NUM_CHANNEL; iChannel++)
	{
		dataNum1 = iChannel/4;
		dataNum2 = iChannel%4;
		for(ibit=0; ibit<3; ibit++)
		{
			lightv[ibit] = (0x0001<<(dataNum2*3+ibit));
			if((lampStatus[dataNum1] & lightv[ibit]) == lightv[ibit])
			{
				chanStatus[iChannel]=lights[ibit];
				break;
			}
		}
		if(ibit == 3)
			chanStatus[iChannel] = TURN_OFF; 
	}
}
void SetLampStatus(uint16 *lampStatus, UINT8 *chanStatus)
{
	int i;
	unsigned short  value;
	unsigned char cFlag = 0;
    static unsigned char cArrayBlinkStatus[NUM_CHANNEL] = {0};//保存在绿闪、黄闪时上一次灯色的状态，用来决定此时是否应该灭灯    
    static unsigned char cArrayIsTimeEnd[NUM_CHANNEL] = {0};//闪亮时，要保证状态持续时间达500ms.

	if(lampStatus == NULL || chanStatus == NULL)
		return;
	for(i = 0 ; i < NUM_CHANNEL; i++)
	{
		cFlag = 0;	
		switch(chanStatus[i])
		{
			case GREEN: 	value = 1;	  break;
			case RED:		value = 2;	  break;
			case YELLOW:	value = 4;	  break;
			case GREEN_BLINK:		
								if(++cArrayIsTimeEnd[i] < 2)
								{
									value = cArrayBlinkStatus[i];
									break;
								}
								cArrayIsTimeEnd[i] = 0;
								value = (cArrayBlinkStatus[i] == 0) ? 1 : 0;	
								cArrayBlinkStatus[i] = value;
								break;
			case YELLOW_BLINK:		
								if(++cArrayIsTimeEnd[i] < 2)
								{
									value = cArrayBlinkStatus[i];
									break;
								}
								cArrayIsTimeEnd[i] = 0; 								   
								value = (cArrayBlinkStatus[i] == 0) ? 4 : 0;  
								cArrayBlinkStatus[i] = value;
								break;
			case TURN_OFF:	value = 0;	  break;
			case INVALID:	cFlag = 1;	  break;
			default :		cFlag = 1;	  break;
		}

		if(cFlag == 0)//如果是忽略或者默认情况下，均不调用该接口。
		{
			put_lamp_value(&lampStatus[i/4], i % 4, value);
		}
	}
}
void PedestrianFollowCtrl(UINT8 *chan)
{
	int i;
	UINT8 itime=0;
	int followPhase=0;
	int motherPhase=0;

	if(SPECIAL_CONTROL_SYSTEM != gStructBinfileCustom.cSpecialControlSchemeId)
		return;
	
	for(i=0; i<NUM_CHANNEL; i++)
	{
		if(gSignalControlpara->stChannel[i].nControllerType == OTHER)//通道控制源类型OTHER表示行人跟随
		{
			if(chan[i] == YELLOW || chan[i] == YELLOW_BLINK)//行人相位没有黄灯和黄闪
				chan[i] = RED;

			followPhase = gSignalControlpara->stChannel[i].nControllerID-1;
			if(followPhase >= NUM_PHASE || followPhase<0)
			{
				ERR("Wrong follow phase id!");
				continue;
			}
			motherPhase = gFollowPhaseMotherPhase[followPhase]-1;
			if(motherPhase < 0)//下一相位也在跟随相位表中
				continue;
			//不计算绿闪、黄灯和全红时间
			itime = gCountDownParams->stPhaseRunningInfo[motherPhase][0]
				-(gSignalControlpara->stPhase[motherPhase].nYellowTime + 
				gSignalControlpara->stPhase[motherPhase].nAllRedTime + 
				gSignalControlpara->AscSignalTransTable[motherPhase].nGreenLightTime);

			if(gCountDownParams->ucOverlap[followPhase][0]==GREEN_BLINK)//母相位绿闪时，行人相位为红灯
				chan[i] = RED;

			if(chan[i] == GREEN && 
				gCountDownParams->stPhaseRunningInfo[motherPhase][1] > (itime-gSignalControlpara->stPhase[motherPhase].nPedestrianClearTime))
				chan[i] = GREEN_BLINK;
		}
	}
}
void ItsCustom(void)
{
	unsigned char chan[NUM_CHANNEL]={INVALID};
	GetChanStatus(g_nLampStatus, chan);//获取的是当前通道的灯色(no blink status)

	/******start custom*******/
	FollowPhaseGreenBlink(chan);
	PedestrianFollowCtrl(chan);
	
	/******end custom*******/
	SetLampStatus(g_nLampStatus, chan);
}
