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

//��ʱ�޸ģ�����Խ��
//CountDownVeh countdown_veh[17];             //��Ҫ����ȫ�̡���̵���ʱ
//CountDownPed countdown_ped[17];             //��Ҫ����ȫ�̡���̵���ʱ

extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //����ʱ�ӿ���Ϣ�������ڵ���ʱ�ӿ��м���
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
CountDownParams     g_CountDownParams[4][MAX_NUM_PHASE];    //�ֱ������������͡������������ˡ������浹��ʱ�����������Ҫÿ����и���,��λID��Ϊ�����±ꡣ
extern CountDownCfg        g_CountDownCfg;              //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����


int fd_485 = 0;//485����ͨ�����������������ʹ������ݡ�


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
 �� �� ��  : GetNextControllerId
 ��������  : ���㵱ǰ���е���λID����һ����λ
 �������  : unsigned char cDeviceId  
             unsigned char cNowId     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
    //�����ǰ����λ��������һ����������һ��ֵ��0����ô��Ҫ���ص�һ������ԴID
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
 �� �� ��  : GetRuningPhaseId
 ��������  : ����ĳ���豸��ǰ��Ҫ���е���λ�ţ��õ������λ�ţ������������ʱӦ����ʾ�ĵ�ɫ����ֵ
 �������  : unsigned char cDeviceId  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
    //�������λ��ǰ�ǻƵƣ��һƵƵĵ���ʱ��1����ô���´θ÷���ĵ���ʱҪ��ʾ��һ����λ
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
 �� �� ��  : SetCountDownValue
 ��������  : ����ʱ������ֵ�������ڵ���ʱ�ӿ��е��á�
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��23��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
        //����������ʱ
        g_CountDownParams[MOTOR-1][i].cControllerID = i+1;
        g_CountDownParams[MOTOR-1][i].cColor = phaseInfos[i].phaseStatus;
        g_CountDownParams[MOTOR-1][i].cTime = phaseInfos[i].phaseLeftTime;

        //���˵���ʱ
        g_CountDownParams[PEDESTRIAN-1][i].cControllerID = i+1;
        g_CountDownParams[PEDESTRIAN-1][i].cColor = phaseInfos[i].pedestrianPhaseStatus;
        g_CountDownParams[PEDESTRIAN-1][i].cTime = phaseInfos[i].pedestrianPhaseLeftTime;

        //������λ����ʱ
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


