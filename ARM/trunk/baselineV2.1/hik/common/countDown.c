#include <stdio.h> 
#include <string.h> 
#include <sys/types.h> 
#include <errno.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <termios.h> 
#include <stdlib.h> 
#include "common.h"
#include <pthread.h>
#include "countDown.h"
#include "platform.h"
#include "hik.h"
#include "configureManagement.h"
#include "its.h"
#include "countdown_nation_2004.h"

//临时修改，避免越界
//CountDownVeh countdown_veh[17];             //主要用在全程、半程倒计时
//CountDownPed countdown_ped[17];             //主要用在全程、半程倒计时

extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //倒计时接口信息，用来在倒计时接口中计算
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
extern CountDownCfg        g_CountDownCfg;              //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存

static	UInt8 gChannelStatus[MAX_CHANNEL_NUM]= {0};	//所有通道的状态
static	UInt16 gChannelCountdown[MAX_CHANNEL_NUM] = {0};	//通道的倒计时

int fd_485 = 0;//485串口通信描述符，用来发送串口数据。

#if 0
/*********************************************************************************
*
* 	打开指定串口
*
***********************************************************************************/
static int open_port(int comport) 
{ 
    char cDev[64] = {0};
    int fd = 0;
    sprintf(cDev,"/dev/ttyS%d",comport-1);

    if((fd = open(cDev, O_RDWR|O_NOCTTY|O_NDELAY)) == -1)
    {
        ERR("Error to open %s\n",cDev);
        return 0;
    }

	 //恢复串口为阻塞状态
    if(fcntl(fd, F_SETFL, 0) < 0) 
	{
		ERR("fcntl failed!\n"); 
	}
	 //测试是否为终端设备 
    if(isatty(STDIN_FILENO) == 0) 
	{
		ERR("standard input is not a terminal device\n"); 
	}

 	return fd; 
}


/*********************************************************************************
*
* 	设置串口相关参数
*
***********************************************************************************/
static int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop) 
{ 
	struct termios newtio,oldtio; 
	 
    if( tcgetattr( fd,&oldtio) != 0)//保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息 
    {  
        ERR("error to SetupSerial : %s \n",strerror(errno)); 
        return -1; 
    } 
 	bzero( &newtio, sizeof( newtio ) ); 
 	
	//步骤一，设置字符大小
 	newtio.c_cflag  |=  CLOCAL | CREAD;  
 	newtio.c_cflag &= ~CSIZE;  
 	
	//设置停止位 
 	switch( nBits ) 
 	{ 
    	case 7: 
    		newtio.c_cflag |= CS7; 
    		break; 
    	case 8: 
    		newtio.c_cflag |= CS8; 
    		break; 
 	} 
	//设置奇偶校验位 
 	switch( nEvent ) 
 	{ 
    	case 'O': //奇数 
    		newtio.c_cflag |= PARENB; 
    		newtio.c_cflag |= PARODD; 
    		newtio.c_iflag |= (INPCK | ISTRIP); 
    		break; 
    	case 'E': //偶数 
    		newtio.c_iflag |= (INPCK | ISTRIP); 
    		newtio.c_cflag |= PARENB; 
    		newtio.c_cflag &= ~PARODD; 
    		break; 
    	case 'N':  //无奇偶校验位 
    		newtio.c_cflag &= ~PARENB; 
    		break; 
 	} 
 	//设置波特率 
	switch( nSpeed ) 
 	{ 
    	case 2400: 
    		cfsetispeed(&newtio, B2400); 
    		cfsetospeed(&newtio, B2400); 
    		break; 
    	case 4800: 
    		cfsetispeed(&newtio, B4800); 
    		cfsetospeed(&newtio, B4800); 
    		break; 
    	case 9600: 
    		cfsetispeed(&newtio, B9600); 
    		cfsetospeed(&newtio, B9600); 
    		break; 
    	case 115200: 
    		cfsetispeed(&newtio, B115200); 
    		cfsetospeed(&newtio, B115200); 
    		break; 
    	case 460800: 
    		cfsetispeed(&newtio, B460800); 
    		cfsetospeed(&newtio, B460800); 
    		break; 
    	default: 
    		cfsetispeed(&newtio, B9600); 
    		cfsetospeed(&newtio, B9600); 
    		break; 
 	} 
	 /*设置停止位*/ 
 	if( nStop == 1 ) 
      	newtio.c_cflag &=  ~CSTOPB; 
 	else if ( nStop == 2 ) 
      newtio.c_cflag |=  CSTOPB; 
      
	/*设置等待时间和最小接收字符*/ 
 	newtio.c_cc[VTIME]  = 0; 
 	newtio.c_cc[VMIN] = 0; 
 	
    /*处理未接收字符*/ 
 	tcflush(fd,TCIFLUSH); 
 	
	/*激活新配置*/ 
	if((tcsetattr(fd,TCSANOW,&newtio))!=0) 
 	{ 
  		perror("com set error"); 
  		return -1; 
 	}	 
 	
 	return 0; 
} 
#endif
/*****************************************************************************
 函 数 名  : Init485Serial
 功能描述  : 初始化485串口
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月22日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void Init485Serial()
{
	char nEvent = 'N';
    
	//打开串口 
    if((fd_485 = open_port(5)) <= 0)
	{
        ERR("Init485Serial open port  error \n"); 
        return; 
    } 
    
	if(gStructBinfileCustom.sComParams[1].unParity == 1)
	{
		nEvent = '0';
	}
	else if(gStructBinfileCustom.sComParams[1].unParity == 2)
	{
		nEvent = 'E';
	}
	
	//设置串口 
    if(set_opt(fd_485,gStructBinfileCustom.sComParams[1].unBaudRate,gStructBinfileCustom.sComParams[1].unDataBits,nEvent,gStructBinfileCustom.sComParams[1].unStopBits) < 0)
	{
        ERR("Init485Serial set_opt error \n"); 
        return; 
    } 
    INFO("baud = %d , databits = %d , parity = %c, stopbits = %d\n",gStructBinfileCustom.sComParams[1].unBaudRate,gStructBinfileCustom.sComParams[1].unDataBits,nEvent,gStructBinfileCustom.sComParams[1].unStopBits);
 	Set_ttys4_send_enable();
}
/*****************************************************************************
 函 数 名  : Send485Data
 功能描述  : 发送485数据，注意，现在是阻塞发送的，如后期测试发现阻塞发送太耗
             时，可以考虑改成非阻塞方式。
 输入参数  : unsigned char *cSendBuf  
             int nLen                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月22日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void Send485Data(unsigned char *cSendBuf,int nLen)
{
#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
    if(fd_485 == 0)
    {
        Init485Serial();
    }

    if(nLen == 0)
    {
        return;
    }

    if(write(fd_485,cSendBuf,nLen) < 0)
    {
        ERR("failed to Send485Data \n");
        close(fd_485);
        fd_485 = 0;
    }
#endif
}

int GetCoundownNum()
{
	int i = 0;
    int ret = 0;
    
    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(g_CountDownCfg.cControllerType[i] != 0)
        {
            ret++;
        }
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : SetCountdownValue
 功能描述  : 计算某个设备当前正要运行的相位号，得到这个相位号，就能算出倒计时应该显示的灯色及数值
协议类型倒计时显示策略:
1. 先找到倒计时值最大的绿灯，如果找到则直接显示；
2. 否则，再找倒计时值最大的黄灯，如果找到则直接显示；
3. 否则，再找倒计时值最小的红灯，如果找到则直接显示；
4. 前面的都不满足条件，就发送关灯，倒计时值清零，同时报错。
 
 输入参数  : unsigned char cDeviceId  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void SetCountdownValue(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor)
{
    int i = 0;
    unsigned char nChannelId = g_CountDownCfg.cControllerID[cDeviceId][0];

    unsigned short nMaxGreenValue = 0;//倒计时值最大的绿灯
    unsigned short nMaxYellowValue = 0;//倒计时值最大的黄灯
    unsigned short nMinRedValue = gChannelCountdown[nChannelId - 1];//倒计时值最小的红灯

    *pPhaseCountDownTime = 0;
    *pPhaseColor = TURN_OFF;

    //一次性计算所有需要的值
    for(i = 0; i < NUM_CHANNEL; i++)
    {
        nChannelId = g_CountDownCfg.cControllerID[cDeviceId][i];
        //INFO("cDeviceId  %d ,nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",cDeviceId,nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);
        
        if(nChannelId <= 0 || nChannelId > NUM_CHANNEL)//如果通道号为0，表明已遍历完所有通道
        {
            break;
        }
        //INFO("nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);

        //找到最大绿灯
        if((gChannelStatus[nChannelId - 1] == GREEN || gChannelStatus[nChannelId - 1] == GREEN_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxGreenValue))
        {
            nMaxGreenValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxGreenValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //找到最大黄灯
        if((nMaxGreenValue == 0)
            && (gChannelStatus[nChannelId - 1] == YELLOW || gChannelStatus[nChannelId - 1] == YELLOW_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxYellowValue))
        {
            nMaxYellowValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxYellowValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //找到最小红灯
        if((nMaxGreenValue == 0)
            &&(nMaxYellowValue == 0)
            &&(gChannelStatus[nChannelId - 1] == RED || gChannelStatus[nChannelId - 1] == RED_BLINK || gChannelStatus[nChannelId - 1] == ALLRED) 
            && (gChannelCountdown[nChannelId - 1] <= nMinRedValue))
        {
            nMinRedValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMinRedValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1] == ALLRED ? RED : gChannelStatus[nChannelId - 1];
        }
    }

    OFTEN("cDeviceId  %d   pPhaseColor :  %s  pPhaseCountDownTime  %d\n",cDeviceId,GET_COLOR(*pPhaseColor),*pPhaseCountDownTime);
    
}

/*****************************************************************************
 函 数 名  : SetCountDownValue
 功能描述  : 只是简单的保存一下各个通道的倒计时和值，目的是为了让各个协议的实现与库无关。
 
 输入参数  : LineQueueData *data  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月23日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void SaveCountDownValue(const LineQueueData *data)
{
    int i = 0;

    if(data == NULL)
    {
        return;
    }
    
    memcpy(gChannelStatus,data->allChannels,sizeof(data->allChannels));
    memcpy(gChannelCountdown,data->channelCountdown,sizeof(data->channelCountdown));                                    
}


/*****************************************************************************
 函 数 名  : CountDownInterface
 功能描述  : 对外提供的倒计时接口，根据配置工具下发的倒计时类型，选择对应的
             协议。该函数最好每秒钟调用一次。
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月22日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void CountDownInterface(LineQueueData *data)
{
	if (((data->schemeId == 0 || data->schemeId > NUM_SCHEME) 
			&& data->schemeId != INDUCTIVE_SCHEMEID && data->schemeId != INDUCTIVE_COORDINATE_SCHEMEID)
		|| data->phaseTableId == 0 || data->phaseTableId > MAX_PHASE_TABLE_COUNT
		|| data->channelTableId == 0 || data->channelTableId > MAX_CHANNEL_TABLE_COUNT)
		return;
    SaveCountDownValue(data);
    switch(gStructBinfileCustom.sCountdownParams.iCountDownMode)
    {
        case SelfLearning:      break;
        case FullPulse:         countDownByAllPulse(data,g_CountDownCfg.nChannelFlag);break;
        case HalfPulse:         countDownByHalfPulse(data,g_CountDownCfg.nChannelFlag);break;
        case NationStandard:    countDownByNationStandard();break;
        case LaiSiStandard:     countDownByLaiSiProtocol(); break;
        case HisenseStandard:   countDownByHisenseStandard();break;
        case NationStandard2004:    countDownByNationStandard2004();break;
        default:                break;
    }
}


