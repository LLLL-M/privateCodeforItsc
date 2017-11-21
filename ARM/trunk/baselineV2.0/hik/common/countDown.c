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

//��ʱ�޸ģ�����Խ��
//CountDownVeh countdown_veh[17];             //��Ҫ����ȫ�̡���̵���ʱ
//CountDownPed countdown_ped[17];             //��Ҫ����ȫ�̡���̵���ʱ

extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //����ʱ�ӿ���Ϣ�������ڵ���ʱ�ӿ��м���
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
extern CountDownCfg        g_CountDownCfg;              //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����

static	UInt8 gChannelStatus[MAX_CHANNEL_NUM]= {0};	//����ͨ����״̬
static	UInt16 gChannelCountdown[MAX_CHANNEL_NUM] = {0};	//ͨ���ĵ���ʱ

int fd_485 = 0;//485����ͨ�����������������ʹ������ݡ�

#if 0
/*********************************************************************************
*
* 	��ָ������
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

	 //�ָ�����Ϊ����״̬
    if(fcntl(fd, F_SETFL, 0) < 0) 
	{
		ERR("fcntl failed!\n"); 
	}
	 //�����Ƿ�Ϊ�ն��豸 
    if(isatty(STDIN_FILENO) == 0) 
	{
		ERR("standard input is not a terminal device\n"); 
	}

 	return fd; 
}


/*********************************************************************************
*
* 	���ô�����ز���
*
***********************************************************************************/
static int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop) 
{ 
	struct termios newtio,oldtio; 
	 
    if( tcgetattr( fd,&oldtio) != 0)//����������д��ڲ������ã�������������ںŵȳ���������صĳ�����Ϣ 
    {  
        ERR("error to SetupSerial : %s \n",strerror(errno)); 
        return -1; 
    } 
 	bzero( &newtio, sizeof( newtio ) ); 
 	
	//����һ�������ַ���С
 	newtio.c_cflag  |=  CLOCAL | CREAD;  
 	newtio.c_cflag &= ~CSIZE;  
 	
	//����ֹͣλ 
 	switch( nBits ) 
 	{ 
    	case 7: 
    		newtio.c_cflag |= CS7; 
    		break; 
    	case 8: 
    		newtio.c_cflag |= CS8; 
    		break; 
 	} 
	//������żУ��λ 
 	switch( nEvent ) 
 	{ 
    	case 'O': //���� 
    		newtio.c_cflag |= PARENB; 
    		newtio.c_cflag |= PARODD; 
    		newtio.c_iflag |= (INPCK | ISTRIP); 
    		break; 
    	case 'E': //ż�� 
    		newtio.c_iflag |= (INPCK | ISTRIP); 
    		newtio.c_cflag |= PARENB; 
    		newtio.c_cflag &= ~PARODD; 
    		break; 
    	case 'N':  //����żУ��λ 
    		newtio.c_cflag &= ~PARENB; 
    		break; 
 	} 
 	//���ò����� 
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
	 /*����ֹͣλ*/ 
 	if( nStop == 1 ) 
      	newtio.c_cflag &=  ~CSTOPB; 
 	else if ( nStop == 2 ) 
      newtio.c_cflag |=  CSTOPB; 
      
	/*���õȴ�ʱ�����С�����ַ�*/ 
 	newtio.c_cc[VTIME]  = 0; 
 	newtio.c_cc[VMIN] = 0; 
 	
    /*����δ�����ַ�*/ 
 	tcflush(fd,TCIFLUSH); 
 	
	/*����������*/ 
	if((tcsetattr(fd,TCSANOW,&newtio))!=0) 
 	{ 
  		perror("com set error"); 
  		return -1; 
 	}	 
 	
 	return 0; 
} 
#endif
/*****************************************************************************
 �� �� ��  : Init485Serial
 ��������  : ��ʼ��485����
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��22��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void Init485Serial()
{
	char nEvent = 'N';
    
	//�򿪴��� 
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
	
	//���ô��� 
    if(set_opt(fd_485,gStructBinfileCustom.sComParams[1].unBaudRate,gStructBinfileCustom.sComParams[1].unDataBits,nEvent,gStructBinfileCustom.sComParams[1].unStopBits) < 0)
	{
        ERR("Init485Serial set_opt error \n"); 
        return; 
    } 
    INFO("baud = %d , databits = %d , parity = %c, stopbits = %d\n",gStructBinfileCustom.sComParams[1].unBaudRate,gStructBinfileCustom.sComParams[1].unDataBits,nEvent,gStructBinfileCustom.sComParams[1].unStopBits);
 	Set_ttys4_send_enable();
}
/*****************************************************************************
 �� �� ��  : Send485Data
 ��������  : ����485���ݣ�ע�⣬�������������͵ģ�����ڲ��Է�����������̫��
             ʱ�����Կ��Ǹĳɷ�������ʽ��
 �������  : unsigned char *cSendBuf  
             int nLen                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��22��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void Send485Data(unsigned char *cSendBuf,int nLen)
{
#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
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
 �� �� ��  : SetCountdownValue
 ��������  : ����ĳ���豸��ǰ��Ҫ���е���λ�ţ��õ������λ�ţ������������ʱӦ����ʾ�ĵ�ɫ����ֵ
Э�����͵���ʱ��ʾ����:
1. ���ҵ�����ʱֵ�����̵ƣ�����ҵ���ֱ����ʾ��
2. �������ҵ���ʱֵ���ĻƵƣ�����ҵ���ֱ����ʾ��
3. �������ҵ���ʱֵ��С�ĺ�ƣ�����ҵ���ֱ����ʾ��
4. ǰ��Ķ��������������ͷ��͹صƣ�����ʱֵ���㣬ͬʱ����
 
 �������  : unsigned char cDeviceId  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetCountdownValue(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor)
{
    int i = 0;
    unsigned char nChannelId = g_CountDownCfg.cControllerID[cDeviceId][0];

    unsigned short nMaxGreenValue = 0;//����ʱֵ�����̵�
    unsigned short nMaxYellowValue = 0;//����ʱֵ���ĻƵ�
    unsigned short nMinRedValue = gChannelCountdown[nChannelId - 1];//����ʱֵ��С�ĺ��

    *pPhaseCountDownTime = 0;
    *pPhaseColor = TURN_OFF;

    //һ���Լ���������Ҫ��ֵ
    for(i = 0; i < NUM_CHANNEL; i++)
    {
        nChannelId = g_CountDownCfg.cControllerID[cDeviceId][i];
        //INFO("cDeviceId  %d ,nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",cDeviceId,nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);
        
        if(nChannelId <= 0 || nChannelId > NUM_CHANNEL)//���ͨ����Ϊ0�������ѱ���������ͨ��
        {
            break;
        }
        //INFO("nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);

        //�ҵ�����̵�
        if((gChannelStatus[nChannelId - 1] == GREEN || gChannelStatus[nChannelId - 1] == GREEN_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxGreenValue))
        {
            nMaxGreenValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxGreenValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //�ҵ����Ƶ�
        if((nMaxGreenValue == 0)
            && (gChannelStatus[nChannelId - 1] == YELLOW || gChannelStatus[nChannelId - 1] == YELLOW_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxYellowValue))
        {
            nMaxYellowValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxYellowValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //�ҵ���С���
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
 �� �� ��  : SetCountDownValue
 ��������  : ֻ�Ǽ򵥵ı���һ�¸���ͨ���ĵ���ʱ��ֵ��Ŀ����Ϊ���ø���Э���ʵ������޹ء�
 
 �������  : LineQueueData *data  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��23��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : CountDownInterface
 ��������  : �����ṩ�ĵ���ʱ�ӿڣ��������ù����·��ĵ���ʱ���ͣ�ѡ���Ӧ��
             Э�顣�ú������ÿ���ӵ���һ�Ρ�
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��22��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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


