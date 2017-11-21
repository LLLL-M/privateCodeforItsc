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
#include "common.h"
#include "HikConfig.h"

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
static int SetFreeGreenTimeSign = 0;//感应时间标志位
unsigned char g_RedCurrentValue[32] = {0};   //32路红灯电流值
struct FAILURE_INFO s_failure_info;       //故障信息结构体

#define TIMEOUT  20
#define _YELLOWFLASH_

PHASE_COUNTING_DOWN_PARAMS *gCountDownParams = NULL;        //倒计时接口信息
UInt8 gSpecialControlSchemeId = 0;       //特殊控制方案号
PhaseTurnItem gPhaseTurn[4];

extern CHANNEL_LOCK_PARAMS gChannelLockedParams;
extern UInt8 gChannelLockFlag ;


typedef enum
{
         TURN_OFF = 0,
         GREEN = 1,
         RED = 2,
         YELLOW = 3,
         GREEN_BLINK = 4,
         YELLOW_BLINK = 5,
         ALLRED = 6,
         INVALID = 7,
} PhaseChannelStatus;

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
unsigned int g_printf_switch;                 //打印信息是否开启开关
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


typedef struct 
{
	unsigned short L0:3;
	unsigned short L1:3;
	unsigned short L2:3;
	unsigned short L3:3;
	unsigned short unused:4;
} lamp_t;

/*****************************************************************************
 函 数 名  : put_lamp_value
 功能描述  : 主要用来设置一组灯中某个灯的状态值
 输入参数  : volatile unsigned short *lights  描述一组灯状态的指针
             int n                            具体是哪个灯，只能是0、1、2、3
             unsigned short value             要设置的灯的状态值
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline void put_lamp_value(unsigned short *lights, int n, unsigned short value)
{
	lamp_t *p = (lamp_t *)(lights);
	switch (n) 
	{
		case 0:	p->L0 = value; break;
		case 1:	p->L1 = value; break;
		case 2:	p->L2 = value; break;
		case 3:	p->L3 = value; break;
		default: break;
	}
}


#define Time_To_Minutes(hour,minute,second) (hour*3600+minute*60+second)

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
    unsigned short tmp, value;
    unsigned char cFlag = 0;
    static unsigned char cArrayBlinkStatus[32] = {0};//保存在绿闪、黄闪时上一次灯色的状态，用来决定此时是否应该灭灯
    static unsigned char cArrayIsTimeEnd[32] = {0};//闪亮时，要保证状态持续时间达500ms.
    
    if(gChannelLockFlag == 0)//如果没有通道锁定命令，则直接返回。
    {
        return;
    }
    //如果启用了工作区域但当前时间并不在工作区域内，则直接返回。
    if((pChannnelLockedParams->ucWorkingTimeFlag == 1) && (IsDateInControlArea(pChannnelLockedParams) == 0))
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
	char *temp_LampStatus = NULL;
	//fprintf(stderr,"PhaseLampOutput %d:0x%x \n",boardNum, outLamp); 
	//print_data(outLamp);
	
	if (boardNum<1 || boardNum>8)
	{
		printf("boardNum error:%d\n", boardNum);
		return -1;
	}

	g_nLampStatus[boardNum-1] = outLamp;
	if (boardNum == 8)
	{
	    //fprintf(stderr,"PhaseLampOutput  0x%x || 0x%x  || 0x%x  ||  0x%x  || 0x%x\n",g_nLampStatus[0],g_nLampStatus[1],g_nLampStatus[2],g_nLampStatus[3],g_nLampStatus[4]);
		if(1 == g_struCountDown.iCountDownMode)
		{
			Count_Down_pulse_All(g_nLampStatus);
		}
		else if(2 == g_struCountDown.iCountDownMode)
		{
			Count_Down_pulse_Half(g_nLampStatus);
		}
		LockChannel(&gChannelLockedParams);
		i_can_its_send_led_request(boardNum, g_nLampStatus);

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
	int iPedNO = 0;//相机编号
	
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
	uint16 i = 0;
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
					udp_send_yellowflash();
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
					udp_send_yellowflash();
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
						udp_send_yellowflash();
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

void TestPhase()
{
    gCountDownParams = (PHASE_COUNTING_DOWN_PARAMS *)malloc(sizeof(PHASE_COUNTING_DOWN_PARAMS));
    memset(gCountDownParams,0,sizeof(PHASE_COUNTING_DOWN_PARAMS));


    memset(gPhaseTurn,0,4*sizeof(PhaseTurnItem));

    gPhaseTurn[0].nPhaseTurnID = 1;
    gPhaseTurn[0].nCircleID = 1;

    int i = 0; 
    for(i = 0 ; i < 4; i++)
    {
        gPhaseTurn[0].nTurnArray[i] = i+1;
    }
    
    gPhaseTurn[1].nPhaseTurnID = 1;
    gPhaseTurn[1].nCircleID = 2;

    for(i = 0 ; i < 4; i++)
    {
        gPhaseTurn[1].nTurnArray[i] = i+5;
    }    

    gChannelLockFlag = 1;

    gChannelLockedParams.ucWorkingTimeFlag = 0;

    gChannelLockedParams.ucBeginTimeHour = 5;
    gChannelLockedParams.ucEndTimeHour = 5;
    gChannelLockedParams.ucEndTimeMin = 6;
    
    for(i = 0 ; i < 32 ; i++)
    {
        gChannelLockedParams.ucChannelStatus[i] = INVALID;
    }

    gChannelLockedParams.ucChannelStatus[13] = GREEN;
    gChannelLockedParams.ucChannelStatus[1] = GREEN_BLINK;
    gChannelLockedParams.ucChannelStatus[15] = YELLOW;
    gChannelLockedParams.ucChannelStatus[16] = YELLOW_BLINK;
    gChannelLockedParams.ucChannelStatus[17] = RED;
    gChannelLockedParams.ucChannelStatus[18] = YELLOW;
}


/*********************************************************************************
*
* 	主程序入口
*
***********************************************************************************/
int main(int argc, char *argv[])
{	
	printf("********************************main()****************************\n");
	TestPhase();

	//写开机运行日志并记录软件版本号
	write_running_log();

	//从配置文件中获取所有参数配置
	get_all_params_from_config_file();
	

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

/*****************************************************************************
 函 数 名  : UpdateCountDownTime
 功能描述  : 保证感应发生时，所有相位的倒计时都能保持相同增量的变化
 输入参数  : int iDiffValue                                
             unsigned char *pPhaseArray                    
             int *pOldPhaseCountDownVaule                  
             PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void UpdateCountDownTime(int iDiffValue,unsigned char *pPhaseArray,int *pOldPhaseCountDownVaule,PHASE_COUNTING_DOWN_PARAMS *pCountDownParams)
{
    int i = 0;
    unsigned char cPhaseId = 0;
    
    if((iDiffValue < 0) || (pPhaseArray == NULL) || (pCountDownParams == NULL) || (pOldPhaseCountDownVaule == NULL))
    {
        return;
    }

    for(i = 0 ; i < 16; i++)
    {
        cPhaseId = pPhaseArray[i];
        if(cPhaseId == 0)
        {
            break;
        }
        //如果该环内，有相位的倒计时跳变值不等于绿灯相位跳变的值，则手动更改其倒计时时间

        if(pOldPhaseCountDownVaule[cPhaseId - 1] == 0)//表明是第一次调用该函数，上一次的倒计时数据还没有保存下来，就直接保存
        {
            pOldPhaseCountDownVaule[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];
            continue;
        }
        
        if(((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] - pOldPhaseCountDownVaule[cPhaseId - 1]) != iDiffValue) && (pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0))
        {
            //fprintf(stderr,"Changed %d , old  %d Diff %d \n\n",cPhaseId,pOldPhaseCountDownVaule[cPhaseId - 1],iDiffValue);
            pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] = pOldPhaseCountDownVaule[cPhaseId - 1] + iDiffValue;

           // pOldPhaseCountDownVaule[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];
        }
    }

}

/*****************************************************************************
 函 数 名  : StoreOldCountDownData
 功能描述  : 保存上一次的倒计时信息
 输入参数  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             int *nArray                                   
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月6日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void StoreOldCountDownData(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,int *nArray)
{
    int i = 0;

    if((nArray == NULL) || (pCountDownParams == NULL))
    {

    }

    for(i = 0 ; i < 16; i++)
    {

        nArray[i] = pCountDownParams->stVehPhaseCountingDown[i][1];
    }


}

/*****************************************************************************
 函 数 名  : CalcGreenSplit
 功能描述  : 计算相位的绿信比
 输入参数  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             unsigned char cCircleNo                       
             PhaseTurnItem *pPhaseTurn                     
             unsigned char uIndex                          
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void CalcGreenSplit(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,unsigned char cCircleNo,PhaseTurnItem *pPhaseTurn,unsigned char uIndex)
{
    int k = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;

    for(k = 0 ; k < uIndex - 2; k++)//向后遍历
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//得到当前的相位号
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//得到该相序中下一个相位

        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            break;
        }
		
		if(((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] != 0)) && 
			((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] != 0)))
		{
			pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
																	((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);		
		}

    }

    for(k = uIndex; k < NUM_PHASE - 1; k++)//向前遍历
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//得到当前的相位号
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//得到该相序中下一个相位
        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            if((uIndex >= 2) && (cPhaseId != 0))//
            {
				if(((pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] != 0)) && 
					((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] != 0)))
				{				
					pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] == 0) ?  pCountDownParams->stPedPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] : pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1]) - 
																			((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);
				
				}
            }
            
            break;
        }

		if(((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] != 0)) && 
			((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] != 0)))
		{		
			pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
																	((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]); 
		}
    }
}

/*****************************************************************************
 函 数 名  : CalcCircleTime
 功能描述  : 计算运行周期长
 输入参数  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             PhaseTurnItem *pPhaseTurn                     
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void CalcCircleTime(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn)
{
    int i = 0;
    int j = 0;
    
    int iTempVal = 0;
    
    for(i = 0 ; i < 4; i++)
    {
        if(0 == pPhaseTurn[i].nTurnArray[0])//如果某个环没有配置，则不继续计算
        {
            continue;
        } 
        iTempVal = 0;
        
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            if(0 == pPhaseTurn[i].nTurnArray[j])
            {
                break;
            }
            iTempVal += pCountDownParams->stPhaseRunningInfo[pPhaseTurn[i].nTurnArray[j] - 1][0];
        }

        pCountDownParams->ucCurCycleTime = iTempVal;
        break;
    }
}

/*****************************************************************************
 函 数 名  : AlterPhaseGreenLightTime
 功能描述  : 根据相位绿闪时间，将特定时间段的绿灯状态改为绿闪
 输入参数  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             unsigned char *pPhaseGreenBlink               
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月10日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void AlterPhaseGreenLightTime(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,unsigned char *pPhaseGreenBlink)
{
    int i = 0;

    for(i = 0 ; i < 16 ; i++)
    {
        if((pCountDownParams->stVehPhaseCountingDown[i][0] == GREEN) && (pCountDownParams->stVehPhaseCountingDown[i][1] <= pPhaseGreenBlink[i]))
        {
            pCountDownParams->stVehPhaseCountingDown[i][0] = GREEN_BLINK;
           // fprintf(stderr,"===>  Phase  Green Blink %d\n",i+1);
        }
    }
}


/*****************************************************************************
 函 数 名  : CalcPhaseRunTimeAndSplit
 功能描述  : 在倒计时中计算各相位的绿信比时间、运行时间及整个周期时间、运行时间
 输入参数  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  倒计时结构体指针
             PhaseTurnItem *pPhaseTurn                     当前运行的相序表         二维数组[4][16]
             unsigned char *pPhaseGreenLight                相位绿闪时间指针        长度是16
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月4日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 暂不考虑行人感应的情况
*****************************************************************************/
void CalcPhaseRunTimeAndSplit(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn,unsigned char *pPhaseGreenBlink)
{
    int i = 0;
    int j = 0;
    int k = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;
    int index = 0;
    int iTempVal = 0 ;
    int iDiffVal = 0;//用来记录感应时当前相位的绿灯增加值
    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//记录当前绿灯相位号，当该环内，绿灯相位发生变动时，将上个相位的相位运行时间清零
    static int nArrayCurrentPhaseCountDown[16] = {0};//本地记录一份各个相位的倒计时信息，用来感应时，保证各个相位的倒计时能同时增加
    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

   // fprintf(stderr,"===>  %d  ||  %d  ||  %d ||  %d\n",pCountDownParams->stVehPhaseCountingDown[0][1],pCountDownParams->stPedPhaseCountingDown[1][1],
                                                      //  pCountDownParams->stPedPhaseCountingDown[2][1],pCountDownParams->stVehPhaseCountingDown[3][1]);

    //轮询相序表，计算单环中各个相位的绿信比
    for(i = 0 ; i < 4 ; i++)
    {
        if(0 >= pPhaseTurn[i].nTurnArray[0])//如果某个环没有配置，则不继续计算
        {
            continue;
        }
        for(j = 0 ; j < NUM_PHASE; j++)//轮询该环内的相位
        {
            cPhaseId = pPhaseTurn[i].nTurnArray[j];
            if(0 >= cPhaseId)//如果该环内已找到相位ID为0的相位，则表明已轮询结束，直接break掉，不再计算该环。
            {
                break;
            }

            if(pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] != 0)//如果当前相位运行时间不为0，则表明继续运行
            {
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]++;
            }

            if((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0]) || (GREEN == pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0]))//找到灯色为绿色的相位
            {
                if(nArrayCurrentPhaseId[i] == -1)
                {
                    nArrayCurrentPhaseId[i] = cPhaseId;//记录当前绿灯相位
                }
                if(cPhaseId != nArrayCurrentPhaseId[i])//如果运行相位发生变动，则清空上个相位的运行时间
                {
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = cPhaseId;//更新当前绿灯相位
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 1;//如果运行时间是0，则将运行时间改为1
                }

                index = j+1;//记录绿灯相位的顺序，以此为起点，依次做差值，可得除首末相位外的绿信比

                //在感应时，要保证一个环内所有相位的倒计时时间都要同步增加相同值
                if(nArrayCurrentPhaseCountDown[cPhaseId - 1] == 0)
                {
                    nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];//刚开始时记录
                }
                iDiffVal = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] - nArrayCurrentPhaseCountDown[cPhaseId - 1];


                if((iDiffVal >= 0) && (nArrayCurrentPhaseCountDown[cPhaseId - 1] != 1 ) && (pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0))//只有在相位未发生改变且倒计时跳变时才调用。
                {
                    //fprintf(stderr,"===>phase %d  countdown   %d\n",cPhaseId,nArrayCurrentPhaseCountDown[cPhaseId - 1]);
                    UpdateCountDownTime(iDiffVal,pPhaseTurn[i].nTurnArray, nArrayCurrentPhaseCountDown,pCountDownParams);
                }

                nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];

                //计算当前相位的绿信比时间,等于下一个相位的倒计时时间加上当前相位的运行时间
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//如果该相位的下一个相位不存在，则下一个相位为相序表起始值
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
				
				if((pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] != 0) || (pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] != 0))
				{
				    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) + pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] - 1;
				}
				

                //用做差值的方式计算除最后一个相位以外的绿信比
                CalcGreenSplit(pCountDownParams,i,pPhaseTurn,index);
            }
        }
    }
    
    //运行时间大于或等于周期后，需要清零，重新开始下一个周期
    if((pCountDownParams->ucCurRunningTime >= pCountDownParams->ucCurCycleTime) && (pCountDownParams->ucCurCycleTime != 0))
    {
        pCountDownParams->ucCurRunningTime = 0;
    }
    //计算运行时间
    pCountDownParams->ucCurRunningTime++;

    //计算运行周期
    CalcCircleTime(pCountDownParams,pPhaseTurn);

    //store old data
    StoreOldCountDownData(pCountDownParams,nArrayCurrentPhaseCountDown);

    //修改绿闪状态
    AlterPhaseGreenLightTime(pCountDownParams,pPhaseGreenBlink);
}

int iRecvPhase = 0;//已经收到的相位个数
unsigned char gGreenBlink[16] = {0};
/*其中color: 1:绿灯，2:红灯，3:黄灯*/
void CountDownOutVeh(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    //fprintf(stderr,"CountDownOutVeh  phase %d  %s phaseTime %d \n\n\n", phase, ((color == 1) ? "绿灯" : (color == 2) ? "红灯" : "黄灯"), phaseTime);
    gCountDownParams->stVehPhaseCountingDown[phase - 1][0] = color;
    gCountDownParams->stVehPhaseCountingDown[phase - 1][1] = phaseTime;

    gGreenBlink[phase - 1] = 3;
   
    iRecvPhase++;
#if 1
	if(iRecvPhase == 4)
	{  // iRecvPhase = 0;
       // fprintf(stderr,"<phase%d %s phaseTime %d >\n\n", phase, ((color == 1) ? "绿灯" : (color == 2) ? "红灯" : "黄灯"), phaseTime);
        
	}
	else
	{
        //fprintf(stderr,"<phase%d %s phaseTime %d >  ", phase, ((color == 1) ? "绿灯" : (color == 2) ? "红灯" : "黄灯"), phaseTime);
	}
	//return;
    if(iRecvPhase == 7)
    {
        iRecvPhase = 0;
        //return;
        CalcPhaseRunTimeAndSplit(gCountDownParams,gPhaseTurn,gGreenBlink);

        fprintf(stderr,"Phase>  %d/%d--%d/%d--%d/%d--%d/%d   %d/%d--%d/%d--%d/%d--%d/%d  %d/%d\n\n",
                                                            gCountDownParams->stPhaseRunningInfo[0][1],gCountDownParams->stPhaseRunningInfo[0][0],
                                                            gCountDownParams->stPhaseRunningInfo[1][1],gCountDownParams->stPhaseRunningInfo[1][0],
                                                            gCountDownParams->stPhaseRunningInfo[2][1],gCountDownParams->stPhaseRunningInfo[2][0],
                                                            gCountDownParams->stPhaseRunningInfo[3][1],gCountDownParams->stPhaseRunningInfo[3][0],
                                                            gCountDownParams->stPhaseRunningInfo[4][1],gCountDownParams->stPhaseRunningInfo[4][0],
                                                            gCountDownParams->stPhaseRunningInfo[5][1],gCountDownParams->stPhaseRunningInfo[5][0],
                                                            gCountDownParams->stPhaseRunningInfo[6][1],gCountDownParams->stPhaseRunningInfo[6][0],
                                                            gCountDownParams->stPhaseRunningInfo[7][1],gCountDownParams->stPhaseRunningInfo[7][0],
                                                            
                                                            gCountDownParams->ucCurRunningTime,
                                                            gCountDownParams->ucCurCycleTime);



    }

#endif    
	countdown_veh[phase].veh_color = color;
	countdown_veh[phase].veh_phaseTime = phaseTime;
	/*
	if(SetFreeGreenTimeSign == 0)
	{
		SeFreeGreenTime(3);//感应检测时间
		SetFreeGreenTimeSign = 1;
	}*/
}

void CountDownOutPed(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    gCountDownParams->stPedPhaseCountingDown[phase - 1][0] = color;
    gCountDownParams->stPedPhaseCountingDown[phase - 1][1] = phaseTime;
    iRecvPhase++;
    //fprintf(stderr,"CountDownOutPed <phase%d %s phaseTime %d >  ", phase, ((color == 1) ? "绿灯" : (color == 2) ? "红灯" : "黄灯"), phaseTime);
	countdown_ped[phase].ped_color = color;
	countdown_ped[phase].ped_phaseTime = phaseTime;
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

