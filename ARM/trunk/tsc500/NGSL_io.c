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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include "HikConfig.h"
#include "platform.h"
#include "common.h"
#include "specialControl.h"

typedef unsigned short  uint16;
static uint16 g_nLampStatus[8] = {0};
uint16 temp_nLampStatus[8] = {0}; //用于脉冲式触发倒计时牌
char temp_Sign[16] = {0};//标志位
char lastRedLight[16] = {0}; //红灯上一次状态
char nowRedLight[16] = {0}; //红灯现在状态
char stayTimes[16] = {0};//延迟发送脉冲时间
int HardwareWatchdoghd = 0;
static int HardflashDogFlag = 0;
int is_WatchdogEnabled = 0;
//static int g_yellowflash = 0; //黄闪标志：1：开；0：关
static int g_errorstat = 0; //故障标志 :1 :有故障； 0:无故障
//static int g_closelampflag = 0; //黄闪关灯
//static uint16 g_yellowflashfreq = 1; //黄闪频率
static int g_faultstatus[32] = {0}; //通道故障记录用于恢复
static int CurDetectFreq = 0;
//临时修改，避免越界
extern CountDownVeh countdown_veh[17];
extern CountDownPed countdown_ped[17];
unsigned char g_RedCurrentValue[32] = {0};   //32路红灯电流值
struct FAILURE_INFO s_failure_info;       //故障信息结构体

/* add by Jicky */
extern SignalControllerPara *gSignalControlpara;    //全局配置信息
extern PHASE_COUNTING_DOWN_PARAMS *gCountDownParams;       //倒计时接口信息
extern void *gHandle;	//倒计时使用的队列句柄

extern UInt8 gStepFlag;    //步进标志，0：表示未步进，1：表示步进
extern UInt16 gLightArray[8];	//步进时使用的点灯数组
extern pthread_rwlock_t gLightArrayLock;    //步进点灯数组的锁

extern void CollectCountDownParams();
extern void StepPthreadInit(void);
extern UInt8 gSpecialControlSchemeId;       //特殊控制方案号

/* add over */

extern CHANNEL_LOCK_PARAMS gChannelLockedParams;
extern UInt8 gChannelLockFlag ;



#define TIMEOUT  20
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

extern int fd;
extern int auto_pressed;    			  //0:自动按键没有按下   1:自动按键已按下
extern int manual_pressed;				  //0:手动按键没有按下   1:手动按键已按下
extern int flashing_pressed;			  //0:黄闪按键没有按下   1:黄闪按键已按下
extern int allred_pressed;				  //0:全红按键没有按下   1:全红按键已按下
extern int step_by_step_pressed;		  //0:步进按键没有按下   1:步进按键已按下
extern int globalWatchdogFlagDisable;     //0:Enable  1:Disable.//WatchDog 使能开关. (弃用)
extern int RTC_USE_GPS_ENABLE;			  //非0:Enable  0:Disable //GPS使能开关.  (弃用)
extern struct SPECIAL_PARAMS s_special_params;   //特殊参数定义
extern struct CURRENT_PARAMS g_struRecCurrent[32];   //电流参数
extern unsigned int g_failurenumber;          //故障事件序号
extern struct Count_Down_Params g_struCountDown;    //倒计时参数
extern unsigned int g_printf_switch;                 //打印信息是否开启开关
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
		printf("开机记录消息无效!\n");
		return -1;
	}
    
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("开机记录文件未能正确打开!\n");
        return -1;
    }
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("获取开机记录文件信息失败!\n");
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
		printf("故障记录消息无效!\n");
		return -1;
	}
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("故障记录文件未能正确打开!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("获取故障记录文件信息失败!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			printf("故障记录文件清整后未能正确打开!\n");
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
    	printf("故障记录文件未能正确打开!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("获取故障记录文件信息失败!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			printf("故障记录文件清整后未能正确打开!\n");
			return -1;
		}
	}
	fwrite(&s_failure_info,sizeof(s_failure_info),1,pFileDebugInfo);
    fclose(pFileDebugInfo);
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
		printf("打开FaultStatus.dat失败,write\n");
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
		printf("打开FaultStatus.dat失败,read\n");
		return;
	}
	fread(g_faultstatus, sizeof(g_faultstatus), 1, pFile);
	fclose(pFile);
}



/*********************************************************************************
*
* 	硬件看门狗初始化。
*
***********************************************************************************/

void HardwareWatchdogInit()
{
	//增加硬件看门狗喂狗
	is_WatchdogEnabled = s_special_params.iWatchdogSwitch;
	if(is_WatchdogEnabled == 1)
	{
		//使能硬件看门狗，通过程序进行喂狗
		//关闭外部喂狗程序
		system("killall watchdog");
		//打开硬件看门狗
		HardwareWatchdoghd = open("/dev/watchdog", O_WRONLY);
		if(HardwareWatchdoghd == -1) 
		{
			printf("Open watchdog error!!!\n");
			return;
		}
		//激活硬件看门狗
		ioctl(HardwareWatchdoghd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);	
		printf("Watchdog is enabled!!!\n");
	}
	else
	{
		//外部喂狗程序继续工作(外部喂狗程序已经在开机启动时自动加载)
		printf("Watchdog is disabled!!!\n");
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
	if(1 == g_printf_switch)
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
	if(s_special_params.iGpsSwitch == 0)
	{
		//GPS开关没有打开
		printf("GPS is disabled!!!\n");
		system("killall -9 GPS");
		Set_LED2_OFF();
	}
	else 
	{
		//GPS开关已经打开
		printf("GPS is enabled!!!\n");
		system("killall -9 GPS");
		//熄灭GPS指示灯
		Set_LED2_OFF();
		system("/root/GPS&");
	}
}


/*********************************************************************************
*
* 读相位板电压信号。
*
***********************************************************************************/

extern void RedgreenCollision(int boardNum, uint16 boardInfo);
uint16 PhaseLampVoltInput(int boardNum)
{
	if (boardNum<1 || boardNum>8)
	{
		printf("boardNum error:%d\n", boardNum);
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
	printf("%s  INFO 100131: boardNum:%d, boardInfodes:%d, boardInfosrc:%d\n",msg, boardNum, boardInfo, g_nLampStatus[boardNum-1]&0xfff); 
	
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
    else//当起始时间比结束时间小时
    {
        if((iNow > iBeginTime) && (iNow < iEndTime))
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
    if(gChannelLockFlag == 0)//如果没有通道锁定命令，则直接返回。
    {
        return 0;//不锁定
    }

    //如果启用了工作区域但当前时间并不在工作区域内，则直接返回2。
    if((pChannnelLockedParams->ucWorkingTimeFlag == 1) && (IsDateInControlArea(pChannnelLockedParams) == 0))
    {
        gChannelLockFlag = 2;
        return 2;//待锁定
    }

    gChannelLockFlag = 1;//锁定
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
    int i = 0;
    unsigned short  value;
    unsigned char cFlag = 0;
    static unsigned char cArrayBlinkStatus[32] = {0};//保存在绿闪、黄闪时上一次灯色的状态，用来决定此时是否应该灭灯
    static unsigned char cArrayIsTimeEnd[32] = {0};//闪亮时，要保证状态持续时间达500ms.
    
    if((pChannnelLockedParams == NULL)||(IsStartLockChannel(pChannnelLockedParams) != 1))
    {
        return;
    }

    for(i = 0 ; i < 32; i++)
    {
        cFlag = 0;
        
        switch(pChannnelLockedParams->ucChannelStatus[i])
        {
            case GREEN:             value = 1;    break;
            case RED:               value = 2;    break;
            case YELLOW:            value = 4;    break;
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
            case TURN_OFF:          value = 0;    break;
            case INVALID:           cFlag = 1;    break;
            default :               cFlag = 1;    break;
        }

        if(cFlag == 0)//如果是忽略或者默认情况下，均不调用该接口。
        {   
            put_lamp_value(&g_nLampStatus[i/4], i % 4, value);
        }
    }
}

/*********************************************************************************
*
*  写灯控板信号灯。
*
***********************************************************************************/
int PhaseLampOutput(int boardNum, uint16 outLamp)
{

//	printf("INFO 100130:%d:0x%x|",boardNum, outLamp); 
//	print_data(outLamp);
    
	if (boardNum<1 || boardNum>8)
	{
		printf("boardNum error:%d\n", boardNum);
		return -1;
	}
	g_nLampStatus[boardNum-1] = outLamp;
	if (boardNum == 8)
	{
		if(1 == g_struCountDown.iCountDownMode)
		{
			Count_Down_pulse_All(g_nLampStatus);
		}
		else if(2 == g_struCountDown.iCountDownMode)
		{
			Count_Down_pulse_Half(g_nLampStatus);
		}
		LockChannel(&gChannelLockedParams);
		if (gStepFlag == 0)
		    i_can_its_send_led_request(boardNum, g_nLampStatus);
		else
		{
		    pthread_rwlock_rdlock(&gLightArrayLock);
            i_can_its_send_led_request(boardNum, gLightArray);
		    pthread_rwlock_unlock(&gLightArrayLock);
		}		
		//主控板运行指示灯
		Hiktsc_Running_Status();
		//硬件看门狗喂狗函数
		HardwareWatchdogKeepAlive(boardNum);
	}

	//电压检测
	if(s_special_params.iVoltageAlarmSwitch == 1 && s_special_params.iErrorDetectSwitch == 1)
	{
		PhaseLampVoltInput(boardNum);
	}
	
	
	return 0;
}


/*********************************************************************************
*
* 	读车检板信号。
*
***********************************************************************************/

uint16 VehicleCheckInput(int boardNum)
{
	uint16 boardInfo = 0;
	
	if (boardNum<1 || boardNum>3)
	{
		printf("boardNum error:%d\n", boardNum);
		return -1;
	}
	boardInfo = recv_date_from_vechile(boardNum);
	
	printf("INFO 100140:%d,boardInfo:%d\n",boardNum,boardInfo); 


	return boardInfo;
}


/*********************************************************************************
*
*	写硬黄闪信号。
*
***********************************************************************************/
extern void RedExtinguish(); //红灯该亮而不亮则为红灯熄灭
void HardflashDogOutput(void)
{
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

	CurDetectFreq++;
	if( CurDetectFreq > 2 )
	{
		//1秒钟同步一次nandflash，防止断电重启后文件丢失
		//system("sync &");
		//电流检测
		if(s_special_params.iCurrentAlarmSwitch == 1 && s_special_params.iErrorDetectSwitch == 1)
		{
			RedExtinguish();
		}
		CurDetectFreq = 0;
	}
	return;
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
	printf("INFO 100142 门开关信号: 0x%x\n",input); 


	return input;
}

/*********************************************************************************
*
* 	红绿冲突、绿冲突检测。
*
***********************************************************************************/
void RedgreenCollision(int boardNum, uint16 boardInfo) 
{
	uint16 j = 0;
	static uint16 errorcountred[32] = {0};
	static uint16 normalcountred[32] = {0};
	static uint16 errorcountgreen[32] = {0};
	static uint16 normalcountgreen[32] = {0};
	time_t now;  
	struct tm *timenow;
	
	char msg[128] = "";

	printf("**************************boardno:%d,g_nLampStatus:%d,boardinfo:%d\n", boardNum, g_nLampStatus[boardNum-1], boardInfo);
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
			printf("%s 第%d通道红灯电压异常(红绿冲突征兆)，errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
			
			if (++errorcountred[(boardNum-1)*4+j] > TIMEOUT)
			{
				errorcountred[(boardNum-1)*4+j] = 0;
				normalcountred[(boardNum-1)*4+j] = 0;
				if(s_special_params.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
				}
				if(g_faultstatus[(boardNum-1)*4+j] == 0)
				{
					g_failurenumber ++;
					printf("%s 第%d通道红绿冲突,errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
					sprintf(msg, "第%d通道红绿冲突", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = 0x0b;          //消息类型ID
					s_failure_info.nNumber = g_failurenumber;       //序列号
					s_failure_info.nTime = now;
	  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					set_failure_number(g_failurenumber);
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
					g_failurenumber++;
					sprintf(msg, "第%d通道红绿冲突故障消除", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = 0x0c;                      //红绿冲突清除消息类型ID
					s_failure_info.nNumber = g_failurenumber;       //序列号
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					set_failure_number(g_failurenumber);
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
			printf("%s 第%d通道绿灯电压异常，(绿冲突征兆),errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
			if (++errorcountgreen[(boardNum-1)*4+j] > TIMEOUT)
			{		
				errorcountgreen[(boardNum-1)*4+j] = 0;
				normalcountgreen[(boardNum-1)*4+j] = 0;
				
				if(s_special_params.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
				}
				g_failurenumber++;
				printf("%s 第%d通道绿冲突,errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
				sprintf(msg, "第%d通道绿冲突", (boardNum-1)*4+j+1);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				s_failure_info.nID = 0x0f;                      //绿冲突消息类型ID
				s_failure_info.nNumber = g_failurenumber;       //序列号
				s_failure_info.nTime = now;
  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
				WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				set_failure_number(g_failurenumber);
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
					g_failurenumber++;
					sprintf(msg, "第%d通道绿冲突故障消除", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = 0x10;                      //红绿冲突清除消息类型ID
					s_failure_info.nNumber = g_failurenumber;       //序列号
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //通道号   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					set_failure_number(g_failurenumber);
					g_faultstatus[(boardNum-1)*4+j] = 0;
					WriteFaultStatus();
				}
				
			}
			
		}
	}

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
	for (i=0; i<8; i++)
	{
		for (j=0; j<4; j++)
		{
			pcurInfo = i_can_its_get_cur(i+1, j+1, 1);   //第３个参数写死１,只获取红灯电流
			//pcurInfo = 100;
			g_RedCurrentValue[i*4+j] = pcurInfo;
			printf("通道%02d红灯电流值:%03d\n",i*4+j+1,pcurInfo);
			
			if ((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo < (g_struRecCurrent[i*4+j].RedCurrentBase - g_struRecCurrent[i*4+j].RedCurrentDiff) )
			{	
				printf("通道%02d红灯亮并且检测电流异常,errorcountred[%d]=%d\n",i*4+j+1,i*4+j+1,errorcountred[i*4+j]+1);
				//红灯亮并且电流检测异常
				if (++errorcountred[i*4+j] > 10)
				{
					errorcountred[i*4+j] = 0;
					normalcountred[i*4+j] = 0;
					if(s_special_params.iCurrentAlarmAndProcessSwitch == 1)
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
						
						
						g_failurenumber ++;
						printf("第%d通道红灯熄灭\n", i*4+j+1);
						sprintf(msg, "第%d通道红灯熄灭", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						s_failure_info.nID = 0x0d;                      //红灯熄灭消息类型ID
						s_failure_info.nNumber = g_failurenumber;       //序列号
						s_failure_info.nTime = now;
	  					s_failure_info.nValue = i*4+j+1;     //通道号   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						set_failure_number(g_failurenumber);
						g_faultstatus[i*4+j] = 3;
						//把g_faultstatus数组写到文件
						WriteFaultStatus();
						//return;
					}
				}
				//return;
			}
			else if((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo > (g_struRecCurrent[i*4+j].RedCurrentBase - g_struRecCurrent[i*4+j].RedCurrentDiff))
			{
				
				//该通道红灯正常亮并且电流值正常
				if (++normalcountred[i*4+j] > 10)
				{
					normalcountred[i*4+j] = 0;
					errorcountred[i*4+j] = 0;
					//该通道红灯写灭检测故障消除
					if (g_faultstatus[i*4+j] == 3)
					{
						time(&now); 
						sprintf(msg, "第%d通道红灯熄灭故障消除", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						g_failurenumber ++;
						s_failure_info.nID = 0x0e;                      //红灯熄灭消除消息类型ID
						s_failure_info.nNumber = g_failurenumber;       //序列号
						s_failure_info.nTime = now;
			  			s_failure_info.nValue = i*4+j+1;     //通道号   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "序号:%d,类型:0x%x,时间:%ld,通道:%d",
									s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						set_failure_number(g_failurenumber);
						g_faultstatus[i*4+j] = 0;
						WriteFaultStatus();
					}
				}
			}
		}
	}
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
		printf("boardNum or pahseNum error:%d, %d\n", boardNum, pahseNum);
		return 0;
	}
	uint16 curInfo = 0;
	if (g_errorstat == 1)
	{
		return curInfo;
	}
	printf("INFO 100132:%d  %d   %d\n",boardNum, pahseNum, redGreen); 
	curInfo = i_can_its_get_cur(boardNum, pahseNum, redGreen);
	printf("相位板电流值:%d\n", curInfo);

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

	printf("INFO 100141 行人按钮: 0x%x\n",input); 


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
int ProcessKey(void)
{
	int result = 0;
	char tmpstr[256] = "";
	result = ProcessKeyBoard();
	
	printf("INFO 100600: 键盘处理 return %d,manual_pressed=%d,auto_pressed=%d,flashing_pressed=%d,allred_pressed=%d,step_by_step_pressed=%d \n",
		result,manual_pressed,auto_pressed,
		flashing_pressed,allred_pressed,step_by_step_pressed);

	if(result != 0)
	{
		sprintf(tmpstr,"键盘处理 return %d,手动=%d,自动=%d,黄闪=%d,全红=%d,黄闪=%d \n",
			result,manual_pressed,auto_pressed,
			flashing_pressed,allred_pressed,step_by_step_pressed);
	
		WriteLogInfos("/home/Keyboard.log",tmpstr);
	}
	
	return result;
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
    exit(0);
}
//It can be called after main() or exit() automatically!
__attribute__((destructor)) void ProgramExit(void)
{
    if (gSignalControlpara == NULL)
        free(gSignalControlpara);
    if (gCountDownParams == NULL)
        free(gCountDownParams);
    if (gHandle == NULL)
        free(gHandle);
    INFO("The program exit unnormal!");
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
	signal(SIGTERM, SigHandler);    //for command 'kill' or 'killall'
	//写开机运行日志并记录软件版本号
	write_running_log();
	//从配置文件中获取所有参数配置
	InitCountDownParams();//必须要先给倒计时分配空间，再读文件，否则会造成段错误。
	get_all_params_from_config_file();
	InitSignalMachineConfig();
	StepPthreadInit();

	//can通信初始化
	i_can_its_init();
	
	//CPU通过CPLD对外的IO口初始化
	CPLD_IO_Init();

	//特殊参数udp服务启动
	udp_server_init();
	
	//倒计时牌接入
	//count_down_thread_create();

	//默认点亮自动灯
	ProcessKeyBoardLight();

	//软件看门狗初始化
	HardwareWatchdogInit();

	//GPS初始化
	GPSInit();

	//打开sadp服务器程序
	system("killall -9 RTSC_sadp");
	system("sleep 5 && /root/RTSC_sadp &");


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
	printf("rtc hw fc.\n");
}


/*其中color: 1:绿灯，2:红灯，3:黄灯*/
void CountDownOutVeh(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
	
	printf("veh:phase %d color %d phaseTime %d \n", phase, color, phaseTime);

    static unsigned char cIsFirstTime = 0;
    if((cIsFirstTime == 0) && (gSpecialControlSchemeId >= 251))
    {
        cIsFirstTime = 1;
        SendSpecialCtrolUdpMsg(gSpecialControlSchemeId);
    }
	countdown_veh[phase].veh_color = color;
	countdown_veh[phase].veh_phaseTime = phaseTime;
	gCountDownParams->stVehPhaseCountingDown[phase - 1][0] = color;
	gCountDownParams->stVehPhaseCountingDown[phase - 1][1] = phaseTime;
	CollectCountDownParams();
	
	/*
	if(SetFreeGreenTimeSign == 0)
	{
		SeFreeGreenTime(3);//感应检测时间
		SetFreeGreenTimeSign = 1;
	}*/
}

void CountDownOutPed(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
	printf("ped:phase %d color %d phaseTime %d \n", phase, color, phaseTime);
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
						if(g_struCountDown.iPhaseOfChannel[iChannel].iphase == iPhase)
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
			if(((countdown_veh[iPhase].veh_phaseTime == g_struCountDown.iPulseGreenTime) && (countdown_veh[iPhase].veh_color == 1))
			||((countdown_veh[iPhase].veh_phaseTime == g_struCountDown.iPulseRedTime) && (countdown_veh[iPhase].veh_color == 2)))//半程倒计时
			{
				if(temp_Sign[iPhase] == 0)
				{
					for(iChannel=0; iChannel<32; iChannel++)
					{
						if(g_struCountDown.iPhaseOfChannel[iChannel].iphase== iPhase)
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

