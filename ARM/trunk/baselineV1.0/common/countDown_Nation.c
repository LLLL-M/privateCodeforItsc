#include "countDown_Nation.h"
#include "configureManagement.h"


static unsigned char gNationStandardCountdownNum = 0;       //ʵ������ʹ�õĵ���ʱ���������32��
static unsigned char g_SendBufNationStandard[68] = {0};     //�����У���෢��(2+1+32*2+1 = 68)�ֽ�����

extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //����ʱ�ӿ���Ϣ�������ڵ���ʱ�ӿ��м���
extern CountDownCfg        g_CountDownCfg;                         //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ

void PrintDataNationStandard()
{
    int i = 0;

    for(i=0; i < gNationStandardCountdownNum*2+4; i++)
    {
        fprintf(stderr," %x ",g_SendBufNationStandard[i]);

    }

    fprintf(stderr,"        ==\n");
}



static void SetCountValueNationStandard(unsigned char *cPosition,unsigned char cDeviceId)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;
    unsigned char cTemp = 0;
    
    GetRuningPhaseId(cDeviceId,&cPhaseCountDownTime,&cPhaseColor);    
	
	if((gStructBinfileCustom.cIsCountdownValueLimit == 1) && (gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime))//���ĳ��λ�ĵ���ʱʱ��ȸ�Ӧ���ʱ�������ʾʱ�����㣬������ʾ�������Ҫʵʱ��ʾ����ʱ״̬����ô����ͨ�����ù��ߣ�����Ԫ�����еĸ�Ӧ���ʱ�����Щ��
	{
		cPhaseColor = TURN_OFF;
		cPhaseCountDownTime = 0;
	}
    //INFO("SetCountValueNationStandard  id  %d, time  %d, color  %d\n",cDeviceId,cPhaseCountDownTime,cPhaseColor);

    switch(cPhaseColor)
    {
        case GREEN:         
        {
            cTemp = 1;
            cTemp |= (cDeviceId<<3);
            break;
        }
        case RED:           
        {
            cTemp = 3;
            cTemp |= (cDeviceId<<3);
            break;
        }
        case YELLOW:      
        {
            cTemp = 2;
            cTemp |= (cDeviceId<<3);
            break;
        }        
        case GREEN_BLINK:    
        {
            cTemp = 1;
            cTemp |= (0x01<<2);
            cTemp |= (cDeviceId<<3);
            break;
        }
        case YELLOW_BLINK:    
        {
            cTemp = 2;
            cTemp |= (0x01<<2);
            cTemp |= (cDeviceId<<3);
            break;
        }
        case TURN_OFF:          
        {
            cTemp |= (cDeviceId<<3);
            break;
        }  
        default:    cTemp |= (cDeviceId<<3); break;
    }


    g_SendBufNationStandard[*cPosition] = cTemp;

    *cPosition += 1;

    g_SendBufNationStandard[*cPosition] = cPhaseCountDownTime;

    *cPosition += 1;
}

static unsigned char GetCheckSum(unsigned char len)
{
    int i = 0;
    unsigned char cCheckSum = 0;

    for(i = 2; i < len; i++)//����Э����ָ��������֡ͷ��
    {
        cCheckSum ^= (g_SendBufNationStandard[i]&0xff);

    }

    return cCheckSum;
}


static void countDownByNationStandard()
{
    int i = 0;
    unsigned char cPosition = 3;

    memset(g_SendBufNationStandard,0,sizeof(g_SendBufNationStandard));
    
    g_SendBufNationStandard[0] = 0x55;
    g_SendBufNationStandard[1] = 0xAA;//ǰ�����ֽڣ���ʾһ֡�Ŀ�ʼ��
    
    g_SendBufNationStandard[2] = gNationStandardCountdownNum;//����ʱ����

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(g_CountDownCfg.cControllerType[i] != 0)//�õ���ʱ������
        {
            SetCountValueNationStandard(&cPosition,g_CountDownCfg.cDeviceId[i]);
            
        }

    }

    g_SendBufNationStandard[cPosition] = GetCheckSum(cPosition);

}


void NationStandardCountDownRun()
{
    SetCountDownValue(gCountDownParams);//ÿ�����������λ�ĵ���ʱ��Ϣ

    gNationStandardCountdownNum = GetCoundownNum();
    
    countDownByNationStandard();

    Send485Data(g_SendBufNationStandard, (gNationStandardCountdownNum*2 + 4));//��������

    //PrintDataNationStandard();
}




