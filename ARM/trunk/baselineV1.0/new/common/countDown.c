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

//临时修改，避免越界
//CountDownVeh countdown_veh[17];             //主要用在全程、半程倒计时
//CountDownPed countdown_ped[17];             //主要用在全程、半程倒计时

extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //倒计时接口信息，用来在倒计时接口中计算
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
CountDownParams     g_CountDownParams[4][MAX_NUM_PHASE];    //分别定义了其他类型、机动车、行人、及跟随倒计时参数，这个需要每秒进行更新,相位ID作为数组下标。
extern CountDownCfg        g_CountDownCfg;              //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存


int fd_485 = 0;//485串口通信描述符，用来发送串口数据。


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
#ifdef FOR_SERVER
    return;
#endif
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
 函 数 名  : GetNextControllerId
 功能描述  : 计算当前运行的相位ID的下一个相位
 输入参数  : unsigned char cDeviceId  
             unsigned char cNowId     
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned char GetNextControllerId(unsigned char cDeviceId,unsigned char cNowId)
{
    int i = 0;
    unsigned char cIndex = 0;
    
    for(i = 0; i < MAX_NUM_PHASE; i++)
    {
        if(cNowId == g_CountDownCfg.cControllerID[cDeviceId][i])
        {
            cIndex = i;
        }
    }
    //如果当前是相位数组的最后一个或者其下一个值是0，那么就要返回第一个控制源ID
    if((cIndex == (MAX_NUM_PHASE - 1)) || (g_CountDownCfg.cControllerID[cDeviceId][cIndex+1] == 0))
    {
        return g_CountDownCfg.cControllerID[cDeviceId][0];
    }
    else
    {
        return g_CountDownCfg.cControllerID[cDeviceId][cIndex+1];
    }
}

/*****************************************************************************
 函 数 名  : GetRuningPhaseId
 功能描述  : 计算某个设备当前正要运行的相位号，得到这个相位号，就能算出倒计时应该显示的灯色及数值
 输入参数  : unsigned char cDeviceId  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
unsigned char GetRuningPhaseId(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor)
{
    int i = 0;
    unsigned char cControllerID = 0;

    unsigned char cControllerType = g_CountDownCfg.cControllerType[cDeviceId];

    static unsigned char cLastControllerId[MAX_NUM_COUNTDOWN] = {0};

    for(i = 0; i < MAX_NUM_PHASE; i++)
    {
        cControllerID = g_CountDownCfg.cControllerID[cDeviceId][i];

        if(cControllerID == 0)
        {
            break;
        }

        if(g_CountDownParams[cControllerType-1][cControllerID - 1].cColor == GREEN)
        {
            cLastControllerId[cDeviceId] = cControllerID;
            *pPhaseCountDownTime = g_CountDownParams[g_CountDownCfg.cControllerType[cDeviceId]-1][cControllerID - 1].cTime;
            *pPhaseColor = g_CountDownParams[g_CountDownCfg.cControllerType[cDeviceId]-1][cControllerID - 1].cColor;

            return cControllerID;
        }
    }

    cControllerID = cLastControllerId[cDeviceId];
    //如果该相位当前是黄灯，且黄灯的倒计时是1，那么，下次该方向的倒计时要显示下一个相位
    if((g_CountDownParams[cControllerType-1][cControllerID - 1].cColor == YELLOW)&&
        (g_CountDownParams[cControllerType-1][cControllerID - 1].cTime == 1))
    {
        cLastControllerId[cDeviceId] = GetNextControllerId(cDeviceId,cControllerID);
    }

    //INFO("GetRuningPhaseId   %d\n",cControllerID);
    *pPhaseCountDownTime = g_CountDownParams[g_CountDownCfg.cControllerType[cDeviceId]-1][cControllerID - 1].cTime;
    *pPhaseColor = g_CountDownParams[g_CountDownCfg.cControllerType[cDeviceId]-1][cControllerID - 1].cColor;
    
    return cControllerID;
}


/*****************************************************************************
 函 数 名  : SetCountDownValue
 功能描述  : 倒计时参数赋值，可以在倒计时接口中调用。
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月23日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/

void SetCountDownValue(PhaseInfo *phaseInfos)
{
    if(phaseInfos == NULL)
    {
        return;
    }

    int i = 0;
    
    for(i = 0; i < MAX_NUM_PHASE; i++)
    {
        //机动车倒计时
        g_CountDownParams[MOTOR-1][i].cControllerID = i+1;
        g_CountDownParams[MOTOR-1][i].cColor = phaseInfos[i].phaseStatus;
        g_CountDownParams[MOTOR-1][i].cTime = phaseInfos[i].phaseLeftTime;

        //行人倒计时
        g_CountDownParams[PEDESTRIAN-1][i].cControllerID = i+1;
        g_CountDownParams[PEDESTRIAN-1][i].cColor = phaseInfos[i].pedestrianPhaseStatus;
        g_CountDownParams[PEDESTRIAN-1][i].cTime = phaseInfos[i].pedestrianPhaseLeftTime;

        //跟随相位倒计时
        g_CountDownParams[FOLLOW-1][i].cControllerID = i+1;
        g_CountDownParams[FOLLOW-1][i].cColor = phaseInfos[i].followPhaseStatus;
        g_CountDownParams[FOLLOW-1][i].cTime = phaseInfos[i].followPhaseLeftTime;

    }

  //  INFO("%s:%d\t%s:%d\t%s:%d\t%s:%d\n",GET_COLOR(g_CountDownParams[1][0].cColor),g_CountDownParams[1][0].cTime
  //                                      ,GET_COLOR(g_CountDownParams[1][1].cColor),g_CountDownParams[1][1].cTime
  //                                      ,GET_COLOR(g_CountDownParams[1][2].cColor),g_CountDownParams[1][2].cTime
  //                                      ,GET_COLOR(g_CountDownParams[1][3].cColor),g_CountDownParams[1][3].cTime);
                                        
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
	if (data->schemeId == 0 || data->schemeId > NUM_SCHEME
		|| data->phaseTableId == 0 || data->phaseTableId > MAX_PHASE_TABLE_COUNT
		|| data->channelTableId == 0 || data->channelTableId > MAX_CHANNEL_TABLE_COUNT)
		return;
    SetCountDownValue(data->phaseInfos);
    switch(gStructBinfileCustom.sCountdownParams.iCountDownMode)
    {
        case SelfLearning:      break;
        case FullPulse:         countDownByAllPulse(data);break;
        case HalfPulse:         countDownByHalfPulse(data);break;
        case NationStandard:    countDownByNationStandard();break;
        case LaiSiStandard:     countDownByLaiSiProtocol(); break;
        case HisenseStandard:   countDownByHisenseStandard();break;
        default:                break;
    }
}


