//103�ƿذ�-----------------------

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"	 
#include "stm32f10x_can.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"	  
#include <stdio.h>
#include <string.h>


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/


volatile uint16_t   g_ADCxConvertedValuesArray[4];

#define ADC1_DR_Address    ((uint32_t)0x4001244C)	 //ADC���ݼĴ����Ļ���ַ
// ע��ADCΪ12λģ��ת������ֻ��ADCConvertedValue�ĵ�12λ��Ч
//__IO uint16_t ADCConvertedValue[2]={0};  //AD0,AD1

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//static uint16_t g_nCurVauleArray[4] = {0};//�������������ڸ�������
static uint8_t g_nGreenLampVoltageVauleArray[4] = {0};//�����̵Ƶ�ѹֵ
static uint8_t g_nRedLampVoltageVauleArray[4] = {0};//�����Ƶ�ѹֵ
static __IO uint16_t g_nLampVol = 0;//ֻ��Ҫ12λ���ӵ͵��߷ֱ�Ϊ��N����̻ƺ��ѹ��
static uint8_t g_nOutPutMode = 1;//���ģʽ��1:44·���������2:22·������� 
static uint32_t g_ArrayLedGroupStatusArray[4] = {0x01,0x01,0x01,0x01};//����Ƶ�״̬����Ϊ��3λ��Ч���ֱ��ʾ�̵ơ���ơ��ƵƵ�״̬��1��0��,�ĳ�32λ��Ϊ�˷��㱣�浽flash
static uint8_t g_nBaseAddr;//�ƿذ����ַ��0��1��2��3  ��PB3 PB4��ȡ.�źŻ�����ַ����ͨ�����ڽ����趨���źŻ�����ַ���浽flash�С������flash��Ϊ�㣬��ֱ����flash��ַ��Ϊ����ַ���������GPIO�ڵ�ֵ��Ϊ
//����ַ(�൱��flash�����ȼ���ߣ�����ֳ������������ӵ�ַ��ͻ�����������ͨ���������޸�)��

static uint8_t  g_heart_count1 = 0;//������������յ��������ذ��CAN��Ϣ
static uint8_t  g_heart_count2 = 0;
//static uint8_t g_interrupted = 0;
static uint32_t g_CountRecved = 0;//��������������������Ƶ��
static uint32_t g_CountSend = 0;//��������������������Ƶ��
//static uint8_t g_IsCanRecvExit = 0;//�ж�CAN�Ľ����ж��Ƿ��쳣��ֹ������ֹ������main�������ֶ���������CAN�Ľ��ա�


uint32_t gCurValAverArray[4][5];//4���Ƶ�����ʵʱֵ����׼ֵ��ǰ4λΪʵʱֵ�����1λλ��׼ֵ��
uint8_t gCurValIndexArray[4] = {0};//4���Ƶ���������ֵ�ǵ�ǰ����ĸ���˳��
uint8_t gIsCurCalFirstTime[4] = {0};//4���Ƶ����ļ����Ƿ��ǵ�һ����������

static uint8_t gCurPrint = 0;//������ӡ���أ�Ĭ��Ϊ�ر�
static uint8_t gVolPrint = 0;//��ѹ��ӡ���أ�Ĭ��Ϊ�ر�
static uint8_t gCanMsgContentPrint = 0;//CAN��Ϣ���ݴ�ӡ���أ�Ĭ��Ϊ�ر�
static uint8_t gIsUseFlaseAddr = 0;//�Ƿ�ʹ��flash�д�ŵĵ�ַ��Ϊ�ƿذ�Ļ���ַ��Ϊ����flash����̫Ƶ����һ��ȷ����flash��ʹ�û���ַ�Ļ�
                                    //����Ͳ���һֱ��ѯflash�ˡ�

__IO uint32_t TimingDelay = 0;

#define LAST_STATUS_ADDR		(0x08007C00 - 0x400)	//���û����õ�flash��ַ���һҳ������������
#define BOARD_BASE_ADDR         (LAST_STATUS_ADDR - 0x400)  //���ӻ���ַ��flash���λ��

#define DELAY_MSEC	250
#define ONE_SECOND_HEARTS	(1000 / DELAY_MSEC)	//1s����������
//��ʱ������,����������������ֵ��δ���յ�can��Ϣ��ƿذ����ִ�л�������
#define TIMEOUT_HEARTS	(3 * ONE_SECOND_HEARTS)

#define EXCUTE_FIXED_CYCLE
#define USE_UART_RECV_IT

#ifdef EXCUTE_FIXED_CYCLE
#include "fixedcycle.h"		//���еĺ���ʵ���Լ��ṹ���ȫ�ֱ����Ķ���ȫ��������
#endif

static uint8_t GetBaseAddr(void);


void ReadAllFlash()
{
    uint32_t  cBuffer[10];
    uint32_t cStartAddr = 0x08000000;
    uint32_t i = 0;
//    uint32_t tmp = cStartAddr;
    
    ReadFlashData(cStartAddr,cBuffer,sizeof(cBuffer));

    for(i = 0; i < 10; i++)
    {
        printf("0x%x \n",cBuffer[i]);
    }

    //WriteFlashData(cStartAddr,cBuffer,4);
}


#ifdef USE_UART_RECV_IT

void HAL_UART_RxCpltCallback(uint16_t val)
{
	int i;
	uint32_t tmp = 0;
    static uint8_t flagIsWriteBaseAddr = 0;
	
	switch (val)
	{
		case 0xf0: 
		WriteFlashData(LAST_STATUS_ADDR,g_ArrayLedGroupStatusArray,sizeof(g_ArrayLedGroupStatusArray));
		NVIC_SystemReset(); 
		break; //����ϵͳ
		case 0xf1:	//��������,�ָ�Ĭ��
			WriteFlashData(LCB_CONFIG_ADDR, &gDefaultConfig, sizeof(LCBconfig));
			memcpy(gLCBconfig,&gDefaultConfig,sizeof(LCBconfig));
			gLCBconfig->configstate = CONFIG_UPDATE;
			printf("restore default config!\r\n");
			break;
		case 0xf2:	//��ӡ����
			printf("baseinfo.isTakeOver = %d\r\n", gLCBconfig->baseinfo.isTakeOver);
			printf("baseinfo.isVoltCheckEnable = %d\r\n", gLCBconfig->baseinfo.isVoltCheckEnable);
			printf("baseinfo.isCurCheckEnable = %d\r\n", gLCBconfig->baseinfo.isCurCheckEnable);
			printf("baseinfo.phaseNum = %d\r\n", gLCBconfig->baseinfo.phaseNum);
			printf("baseinfo.minRedCurVal = %d\r\n", gLCBconfig->baseinfo.minRedCurVal);
			printf("baseinfo.schemeNum = %d\r\n", gLCBconfig->baseinfo.schemeNum);
			printf("baseinfo.canTotalNum = %d\r\n", gLCBconfig->baseinfo.canTotalNum);
			IWDG_ReloadCounter();
			for (i = 0; i < gLCBconfig->baseinfo.phaseNum; i++)
			{
				printf("phaseid %d, greenFlashTime:%d, yellowTime:%d, allredTime:%d, pedFlashTime:%d, channelbits:%#x\r\n", i + 1,\
					gLCBconfig->phaseinfo[i].greenFlashTime, gLCBconfig->phaseinfo[i].yellowTime,\
					 gLCBconfig->phaseinfo[i].allredTime, gLCBconfig->phaseinfo[i].pedFlashTime&0xff,\
					gLCBconfig->phaseinfo[i].channelbits);
			}
			IWDG_ReloadCounter();
			for (i = 0; i <= gLCBconfig->baseinfo.schemeNum; i++)
			{
                printf("phaseturn:%d, first:%#x, second:%#x, third:%#x, fourth:%#x\r\n", i,
					gLCBconfig->phaseturninfo[i].turn.first,
					gLCBconfig->phaseturninfo[i].turn.second,
					gLCBconfig->phaseturninfo[i].turn.third,
					gLCBconfig->phaseturninfo[i].turn.fourth);
				printf("split:%d, phase1:%d, phase2:%d, phase3:%d, phase4:%d\r\n", i,
					gLCBconfig->splitinfo[i].split.phase1,
					gLCBconfig->splitinfo[i].split.phase2,
					gLCBconfig->splitinfo[i].split.phase3,
					gLCBconfig->splitinfo[i].split.phase4);
			}
			printf("controlBoardNo:%d, allchannelbits = %#x\r\n", gLCBconfig->controlBoardNo,gLCBconfig->allchannelbits);
			break;
        case 0xf3:
            ReadAllFlash();
            break;
        case 0xf4://������رյ�����ӡ����
            gCurPrint = (gCurPrint == 0 ? 1 : 0 );
            break;
        case 0xf5://������رյ�ѹ��⿪��
            gVolPrint = (gVolPrint == 0 ? 1 : 0 );
            break;
        case 0xf6://��ӡ�յ���CAN��Ϣ����
            gCanMsgContentPrint = (gCanMsgContentPrint == 0 ? 1 : 0 );
            break;
        case 0xf7://��ȡGPIO�Ļ���ַ
            printf("the GPIO base addr is :  %d \r\n",GetBaseAddr());
            break;                
        case 0xf8://��ȡ�����flash�Ļ���ַ
            ReadFlashData(BOARD_BASE_ADDR,&tmp,sizeof(tmp));
            printf("the FLASH base addr is :  %d\r\n",tmp);
            break;         
        case 0xf9://���ô����flash�Ļ���ַ
            flagIsWriteBaseAddr = 1;
            printf("please input base Addr :  \r\n");
            break;  
        case 0xfa:
            printf("current base addr :  %d \r\n",g_nBaseAddr);
            break;
        case 0xfb:
            gIsUseFlaseAddr = 0;
			val = 0xff;
            WriteFlashData(BOARD_BASE_ADDR,&val,sizeof(int));
            flagIsWriteBaseAddr = 0;      
            printf("clear flash base addr succ\n");
            break;            
		default:
		    if(flagIsWriteBaseAddr == 1)
		    {
                if(val >= 0 && val <= 7)//���֧��8���ƿذ�
                {
					g_nBaseAddr = val;
                    WriteFlashData(BOARD_BASE_ADDR,&val,sizeof(int));
                    flagIsWriteBaseAddr = 0;
                    printf("set flash base addr succ\n");
                    
                }else{
                    printf("your input value is invalid , the number must be [0,7] , please retry.\r\n");
                }
		    }else{
                printf("data = %#x\r\n", val);
                printf("f0 : reboot\r\n");
                printf("f1 : lcb restore default\r\n");
                printf("f2 : lcb print\r\n");
                printf("f3 : read all flash \r\n");
                printf("f4 : cur print switch\r\n");
                printf("f5 : vol print switch \r\n");
                printf("f6 : can msg content\r\n");
                printf("f7 : GPIO  base addr\r\n");
                printf("f8 : flash base addr \r\n");
                printf("f9 : set flash base addr \r\n");
                printf("fa : current base addr\r\n");
                printf("fb : clear flash base addr\r\n");
		    }
	}
	
	IWDG_ReloadCounter();

}

#endif
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
//static void EXTI_IRQHandler_Config(void);
void SetKeyLight(uint8_t num,uint8_t keyStatus);
//static void DisabledEXTI(uint32_t num);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


//__IO uint32_t TimingDelay = 0;

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

typedef enum
{
    GREEN = 0x01,
    RED = 0x02,
    YELLOW = 0x04,
}LEDSTATUS;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


//static uint8_t g_nArrayLedGroupStatus[4][4];//�ֱ��ʾ�ƿذ�1-4�ĵ���1-4�ĵ�״̬
//static uint32_t g_CountRecved = 0;//��������������������Ƶ��


/**	@fn	void Delay(__IO uint32_t nCount)
  * @brief  ��ʱ,��λ�������Ϊ����
  * @param  nCount ѭ������
  * @retval None
  */
static void HAL_Delay(__IO uint32_t nCount)
{
  TimingDelay = nCount/2;

  while(TimingDelay != 0);
}

void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->ODR ^= GPIO_Pin;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

//�ϵ�����Ȼָ���һ�εĵ��״̬�����������������Ϊ�գ�����Ϊ�ϴ���Ч����Ч��������
void RecorveryLastStatus()
{
  int i = 0;
  //��ȡ��һ�ε�״̬
  ReadFlashData(LAST_STATUS_ADDR,g_ArrayLedGroupStatusArray,sizeof(g_ArrayLedGroupStatusArray));
  //�ж϶�ȡ��ֵ�Ƿ���Ч
  for(i = 0; i < 4; i++)
  {
    if(g_ArrayLedGroupStatusArray[i] > 7 || g_ArrayLedGroupStatusArray[i] <= 0)
    {
      return;
    }
  }
  //���
  SetLedGroupStatus();
}


void CAN_TX1_SendData(CanTxMsg *pTxMessage)
{
	CAN_Transmit(CAN1, pTxMessage);

    if(g_CountSend++ == 3)
    {
        g_CountSend = 0;
        HAL_GPIO_TogglePin(GPIOA, GPIO_Pin_5);
    }
}


/*****************************************************************************
 �� �� ��  : LampControlBoard_GetVoltage
 ��������  : ��ȡ���̵Ƶ�ѹֵ
 �������  : ��
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
uint16_t LampControlBoard_GetVoltage(void)
{
    int8_t i = 0;

    //��ȡ��Ƶ�ѹֵ
    g_nRedLampVoltageVauleArray[0] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[1] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[2] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[3] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0 ? 1 : 0);

    //��ȡ�̵Ƶ�ѹֵ
    g_nGreenLampVoltageVauleArray[0] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[1] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[2] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[3] = (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == 0 ? 1 : 0);

    //�����������ѹֵ������֯��12λ����
    g_nLampVol = 0;

    for(i = 0 ;i < 4 ; i++)
    {
        g_nLampVol |= (((1<<2) | ((g_nRedLampVoltageVauleArray[i]&0xff) << 1) | (g_nGreenLampVoltageVauleArray[i]&0xff)) << i*3);
       
    }
    if(gVolPrint == 1)
        printf("red  %d-%d-%d-%d, green %d-%d-%d-%d  g_nLampVol 0x%x \r\n",g_nRedLampVoltageVauleArray[0],g_nRedLampVoltageVauleArray[1],g_nRedLampVoltageVauleArray[2],g_nRedLampVoltageVauleArray[3],
                                                     g_nGreenLampVoltageVauleArray[0],g_nGreenLampVoltageVauleArray[1],g_nGreenLampVoltageVauleArray[2],g_nGreenLampVoltageVauleArray[3],
                                                     g_nLampVol&0xfff);
    
	return g_nLampVol & 0xfff;
 
}

uint16_t GetAdcValues(uint8_t ch)
{
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_55Cycles5); 
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));

    return ADC_GetConversionValue(ADC1);
}

/*****************************************************************************
 �� �� ��  : CanSendCurVolData
 ��������  : �ƿذ������ذ巢�͵�����ѹ�������
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void CanSendCurVolData(void)
{
    uint8_t i = 0;
    int16_t ndiff = 0;
    uint8_t nTempBaseAddr = g_nBaseAddr;  //��ǰ����Ӧ�ö�ȡCAN��Ϣ�ĵڼ���12λ
    static uint8_t flagIsContinue[4] = {0};//���ֻ��żȻ����2�Σ�ʵʱֵƫ��ϴ��򲻼�����㣬���ϳ�ʱ��ƫ��ϴ��Ҫ���������.
    
    CanTxMsg TxMessage;
    memset(&TxMessage,0,sizeof(TxMessage));
	
    if(2 == g_nOutPutMode)//22·���ʱ��0��2�����ȡData��ǰ12λ��1��3�����ȡ�����ŵ�12λ
    {
        if(2 == nTempBaseAddr )
        {
            nTempBaseAddr = 0;
        }
        else if(3 == nTempBaseAddr )
        {
            nTempBaseAddr = 1;
        }
    }
    TxMessage.StdId = 0;
    TxMessage.ExtId = (1 << (16+g_nBaseAddr))|(1 << 15) | (g_nLampVol << 3) | (nTempBaseAddr + 1 );//0-2 ��š�3-14 �̺�ƵƵ�ѹ��15 1
    TxMessage.IDE = CAN_ID_EXT;//��ʾ������Ϊ��չ
    TxMessage.RTR = CAN_RTR_DATA;//��Ϣ֡����Ϊ����֡
    TxMessage.DLC = 4;//֡����Ϊ4
    
    //g_ADCxConvertedValuesArray[0] = GetAdcValues(ADC_Channel_1);
    //g_ADCxConvertedValuesArray[1] = GetAdcValues(ADC_Channel_2);
    //g_ADCxConvertedValuesArray[2] = GetAdcValues(ADC_Channel_3);
    
    
    for(i = 0 ; i < 4; i++)
    {
      
        ndiff = g_ADCxConvertedValuesArray[i]-gCurValAverArray[i][4];//��ʵʱֵ��ȥ��ֵ��Ϊ����ֵ

        //�����ʵʱֵ�����ϴ��򲻽������,
        if(g_ADCxConvertedValuesArray[i] > (gCurValAverArray[i][gCurValIndexArray[i] - 1] + 15) || 
            g_ADCxConvertedValuesArray[i] < (gCurValAverArray[i][gCurValIndexArray[i] - 1] - 15))
        {
            flagIsContinue[i]++; 

            if(flagIsContinue[i] <= 2)
            {
             //   ndiff = 0;
            }
        }
        else
        {
            flagIsContinue[i] = 0;
        }    

        if(ndiff > 255)//��֤CAN��dataҪ��[0,255]��Χ��
        {
            ndiff = 255;
        }

        if(ndiff < 0)
        {
            ndiff = 0;
        }
        if(gCurPrint == 1)
            printf(" %d-%d-%d  ",g_ADCxConvertedValuesArray[i],gCurValAverArray[i][4],ndiff);
        TxMessage.Data[i] = ndiff;//�������ʱ�������ֵӦ�ô���10    
		
    }
    if(gCurPrint == 1)
        printf("  ~~~~~~~~  0x%x\r\n",TxMessage.ExtId);
    CAN_TX1_SendData(&TxMessage);
}

/*****************************************************************************
 �� �� ��  : SetLedGroupStatus
 ��������  : �ƿذ���ݺ��İ��ָ�����gArrayLedGroupStatus��ֵ���ε�����-
             LED��
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetLedGroupStatus(void)
{
    //group  1
    GPIO_WriteBit(GPIOC,GPIO_Pin_7,(0x01 == (g_ArrayLedGroupStatusArray[0]&0x01)) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC,GPIO_Pin_4,(0x02 == (g_ArrayLedGroupStatusArray[0]&0x02)) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC,GPIO_Pin_10,(0x04 == (g_ArrayLedGroupStatusArray[0]&0x04)) ? Bit_SET : Bit_RESET);

    //group  2
    GPIO_WriteBit(GPIOC,GPIO_Pin_8,(0x01 == (g_ArrayLedGroupStatusArray[1]&0x01)) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC,GPIO_Pin_5,(0x02 == (g_ArrayLedGroupStatusArray[1]&0x02)) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC,GPIO_Pin_11,(0x04 == (g_ArrayLedGroupStatusArray[1]&0x04)) ? Bit_SET : Bit_RESET);

    //group  3
    GPIO_WriteBit(GPIOC,GPIO_Pin_9,(0x01 == (g_ArrayLedGroupStatusArray[2]&0x01)) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOB,GPIO_Pin_0,(0x02 == (g_ArrayLedGroupStatusArray[2]&0x02)) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC,GPIO_Pin_12,(0x04 == (g_ArrayLedGroupStatusArray[2]&0x04)) ? Bit_SET : Bit_RESET);

    //group  4
    GPIO_WriteBit(GPIOA,GPIO_Pin_8,(0x01 == (g_ArrayLedGroupStatusArray[3]&0x01)) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOB,GPIO_Pin_1,(0x02 == (g_ArrayLedGroupStatusArray[3]&0x02)) ? Bit_SET : Bit_RESET);
}


/*****************************************************************************
 �� �� ��  : CanSendLampStatus
 ��������  : �ƿذ���ǰ��巢����Ϣ����������·��LED����
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void CanSendLampStatus(void)
{
    CanTxMsg TxMessage;
	uint8_t nTempBaseAddr;    
    memset(&TxMessage,0,sizeof(TxMessage));

    nTempBaseAddr = g_nBaseAddr;//�жϵ�ǰӦ�÷������ĸ�ID��ֻ����22·��ʱ�����Ҫ���StdId

    if(2 == g_nOutPutMode)//22·���ʱ����2����ӵķ�����ϢID�͵�0�������ͬ����3����Ӻ͵�1�����ID��ͬ��
    {
        if(2 == nTempBaseAddr )
        {
            nTempBaseAddr = 0;
        }
        else if(3 == nTempBaseAddr )
        {
            nTempBaseAddr = 1;
        }
    }
    
    TxMessage.StdId = 0x301 + nTempBaseAddr;//
    TxMessage.ExtId = 0;
    TxMessage.IDE = CAN_ID_STD;//
    TxMessage.RTR = CAN_RTR_DATA;//��Ϣ֡����Ϊ����֡ 
    TxMessage.DLC = 5;//֡����Ϊ5
    TxMessage.Data[0] = g_nOutPutMode;
    TxMessage.Data[1] = g_ArrayLedGroupStatusArray[0]&0x07;
    TxMessage.Data[2] = g_ArrayLedGroupStatusArray[1]&0x07;
    TxMessage.Data[3] = g_ArrayLedGroupStatusArray[2]&0x07;
    TxMessage.Data[4] = g_ArrayLedGroupStatusArray[3]&0x07;

    CAN_TX1_SendData(&TxMessage);
}

/*****************************************************************************
 �� �� ��  : CanSendYellowBlink
 ��������  : ���ƿذ�û���յ����Ժ��İ�ĵ����Ϣʱ���ͷ��ͻ�����Ϣ��ǰ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void CanSendYellowBlink(void)
{
	int i, n = ONE_SECOND_HEARTS;
	
	for (i = 0; i < n; i++)
	{
		g_ArrayLedGroupStatusArray[0]= (i < n / 2) ? 0 : 0x04;
		g_ArrayLedGroupStatusArray[1]= (i < n / 2) ? 0 : 0x04;
		g_ArrayLedGroupStatusArray[2]= (i < n / 2) ? 0 : 0x04;
		g_ArrayLedGroupStatusArray[3]= (i < n / 2) ? 0 : 0x04;
		if (g_heart_count1 != g_heart_count2)
			return;
		SetLedGroupStatus();//���Լ��ĵƿذ�Ƶ���
		CanSendLampStatus();//�ٽ������Ϣ���͵�ǰ��壬��ǰ����Ӧ��LED���е���
		
		if (g_heart_count1 != g_heart_count2)
			return;
		IWDG_ReloadCounter();
		HAL_Delay(DELAY_MSEC);
	}
}

/*****************************************************************************
 �� �� ��  : DecodeCanMsg
 ��������  : �ƿذ�������Ժ��İ����Ϣ
 �������  : CAN_HandleTypeDef *CanHandle  CAN���
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void DecodeCanMsg(const CanRxMsg *CanHandle)
{
    uint8_t i = 0;
    uint8_t j = 0;

    uint16_t nCurBoardVal = 0;//��ǰ�ƿذ��12λ��״̬�֣�ÿ3λһ��ͨ�����ֱ��ʾ�̺�Ƶ�״̬;��ȡCAN��Ϣ��12λ��״̬����������
    uint8_t nTempBaseAddr = g_nBaseAddr;//��ǰ����Ӧ�ö�ȡCAN��Ϣ�ĵڼ���12λ
    
    g_nOutPutMode = CanHandle->Data[0];//�����22λ�Ļ�����Data[1]-[3]�ֱ��ʾ����������ϱ�����ġ�

    if(nTempBaseAddr > 3)//������ַ��5 ��6����
    {
        if(g_nOutPutMode == 3)//���յ�����������չ��CAN��Ϣ�����޸���Ҫ��ȡ��λ��
            nTempBaseAddr -= 4;
        else        //������չ����Ϣ����ʲôҲ����
            return;
    }

    if(2 == g_nOutPutMode)//22·���ʱ��0��2�����ȡData��ǰ12λ��1��3�����ȡ�����ŵ�12λ
    {
        if(2 == nTempBaseAddr )
        {
            nTempBaseAddr = 0;
        }
        else if(3 == nTempBaseAddr )
        {
            nTempBaseAddr = 1;
        }
    }

    i = nTempBaseAddr*12/8 + 1;//��λ����ʼ
    j += 8 - ((nTempBaseAddr*12)%8);//��λ����ʼ
    
    nCurBoardVal |= CanHandle->Data[i]>>((nTempBaseAddr*12)%8);//ȡ��λ
    nCurBoardVal |= (uint16_t)(CanHandle->Data[i+1])<<j;//ȡ��λ        

    //��ȫ�ֱ�����ֵ
    for(i = 0 ; i < 4; i++)
    {
        g_ArrayLedGroupStatusArray[i] = (nCurBoardVal>>(3*i))&0x07 ;
    }
    if(gCanMsgContentPrint == 1)
        printf("==> 0x%x 0x%x  0x%x ----->   id 0x%x, len  0x%x, g_nBaseAddr 0x%x  g_nOutPutMode  0x%x  nTempBaseAddr  0x%x\n",CanHandle->Data[1],CanHandle->Data[2],CanHandle->Data[3],
                                                                CanHandle->StdId,CanHandle->DLC,g_nBaseAddr,g_nOutPutMode,nTempBaseAddr);
}

uint8_t GetBaseAddr(void)
{
    uint8_t tmp = 0;//ÿ�������ڲ�ͬ�Ĳ�λ����Ӧ�Ų�ͬ�ĵ�ַ���ֱ�Ϊ0 1 2 3

    if(gIsUseFlaseAddr == 1)
        return g_nBaseAddr;
    
    tmp = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3);
	
    tmp *= 2;
    tmp += GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4);
	//printf("--->  %d  -- %d\r\n",GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3),GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4));
    return tmp;
}


/*****************************************************************************
 �� �� ��  : GetCurAverVal
 ��������  : �õ�������ֵ: ȡ����4�κ���������µĵ���ADֵ�ľ�ֵ��Ϊ��ֵ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��12��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void GetCurAverVal(void)
{
    uint8_t i = 0;
    uint32_t tempTotalVal = 0;
    uint8_t tempOffOn = 0;//��ʱ�������жϵ�ǰ��Ƶ�����״̬��ֻ����Ƶ�ʱ��Ŷ�����ֵ
    static uint8_t flagIsContinue[4] = {0};//���ֻ��żȻ����2�Σ�ʵʱֵƫ��ϴ��򲻼�����㣬���ϳ�ʱ��ƫ��ϴ��Ҫ���������.

  //  g_ADCxConvertedValuesArray[2] = GetAdcValues(ADC_Channel_2);
    for(i = 0; i < 4; i++)
    {
        if(0 == i)//��һ����
        {
            tempOffOn = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4);
        }
        else if(1 == i)
        {
            tempOffOn = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5);
        }
        else if(2 == i)
        {
            tempOffOn = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0);
        }
        else
        {
            tempOffOn = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
        }

       // printf(" %d -- ",g_ADCxConvertedValuesArray[i]);
        if(1 == tempOffOn)//������ʱ��ֱ��continue��
        {
            continue;
        }
        //�����ʵʱֵ�����ϴ��򲻽������,
        if(g_ADCxConvertedValuesArray[i] > (gCurValAverArray[i][gCurValIndexArray[i] - 1] + 15) || 
            g_ADCxConvertedValuesArray[i] < (gCurValAverArray[i][gCurValIndexArray[i] - 1] - 15))
        {
            flagIsContinue[i]++; 

            if(flagIsContinue[i] <= 2)
            {
                //continue;
            }
        }
        else
        {
            flagIsContinue[i] = 0;
        }
        gCurValAverArray[i][gCurValIndexArray[i]] = g_ADCxConvertedValuesArray[i];
        gCurValIndexArray[i]++;

        if(gCurValIndexArray[i] == 4)
        {
            gCurValIndexArray[i] = 0;//�´ζ�ȡ��ֵ�ŵ������һλ
            gIsCurCalFirstTime[i] = 1;
        }
        
        if(gIsCurCalFirstTime[i] == 1)//��һ�α��������������ټ��㣬����ľ�ֻ��Ҫÿ�ι����ͼ���
        {
            tempTotalVal = gCurValAverArray[i][0] + gCurValAverArray[i][1] + gCurValAverArray[i][2] + gCurValAverArray[i][3];
            gCurValAverArray[i][4] = (tempTotalVal >> 2);//������λ�൱�ڳ���4
        }
    }
   // printf("\r\n");
}

/*****************************************************************************
 �� �� ��  : HAL_CAN_RxCpltCallback
 ��������  : CAN������ɻص�����
 �������  : CAN_HandleTypeDef *CanHandle  CAN���
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2015��1��6��
    ��    ��   : xiaowh
    �޸�����   : ֻ�н��յ����Ժ��İ�ĵ����Ϣ��ι�������Ľ���ָʾ�Ƶ�����Ƶ��
*****************************************************************************/
void CAN_RX1_IRQHandler_def(void)
{
	 CanRxMsg RxMessage;
	IWDG_ReloadCounter();  //ι��
	memset(&RxMessage,0,sizeof(RxMessage));
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

   if((RxMessage.DLC == (g_nBaseAddr > 3 ? 4 : 7))&&(0x101 == RxMessage.StdId))
   {
        IWDG_ReloadCounter();
        
        //�������ݲ����
        DecodeCanMsg(&RxMessage);
        SetLedGroupStatus();
        CanSendLampStatus();//��ǰ��巢�͵����Ϣ

        //��������ѹֵ������
        //LampControlBoard_GetCur();//�õ���������
        LampControlBoard_GetVoltage();//ͨ��GPIO��ȡ��ѹ
        GetCurAverVal();//��ʱ���µ�����ֵ
        CanSendCurVolData();//�����ذ巢�͵�����ѹֵ
        
    	g_heart_count1++;	   //���ڿ��ƻ���
//    	g_interrupted = 1;   //���ڿ��ƻ���

    	//print debug info
    	//printf("==>  nBaseAddr : %d  g_ArrayLedGroupStatusArray: %d  %d  %d  %d\r\n",g_nBaseAddr,g_ArrayLedGroupStatusArray[0],g_ArrayLedGroupStatusArray[1],
    	  //                                                                      g_ArrayLedGroupStatusArray[2],g_ArrayLedGroupStatusArray[3]);
        if(2 == g_CountRecved++)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_Pin_4);
            g_CountRecved = 0;
        }
    }
#ifdef EXCUTE_FIXED_CYCLE
	else if ((RxMessage.ExtId & LCB_CONFIG_MASK) == LCB_CONFIG_ID
			&& RxMessage.DLC == 8)
	{
		//printf("recv can msg, stdid: %#x, DLC: %d\r\n", RxMessage.StdId, RxMessage.DLC);
		LCBconfigDeal(&RxMessage);
		IWDG_ReloadCounter();
	}
	else if (GET_BIT(RxMessage.ExtId, 15) == 1 && RxMessage.DLC == 4) 
	{	//��15λΪ1���ҳ���Ϊ4��can��Ϣ��ʾ�ǵƿذ��������ѹ����
		ConvertToLightData(&RxMessage);	//�ѻ�ȡ�ĵ�����ѹת����ʵ�ʵĵ������
	}
#endif
}

/*****************************************************************************
 �� �� ��  : fputc
 ��������  : �ض���fputc���� ,�����Ϳ���ʹ��printfL
 �������  : int ch   
             FILE *f  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : ��ʽ��ȥ����ӡ��Ϣ��
*****************************************************************************/
int fputc(int ch, FILE *f)
{   
  USART_SendData(USART1, (unsigned char) ch);
  while (!(USART1->SR & USART_FLAG_TXE));
  return (ch); 
}

void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;
 
  	RCC_DeInit(); 												 /* RCC system reset(for debug purpose) */
  	RCC_HSEConfig(RCC_HSE_ON); 									 /* Enable HSE */
  	HSEStartUpStatus = RCC_WaitForHSEStartUp();					 /* Wait till HSE is ready */

  	if(HSEStartUpStatus == SUCCESS)
  	{
      	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);      /* Enable Prefetch Buffer */
      	FLASH_SetLatency(FLASH_Latency_2);                         /* Flash 2 wait state */
      	RCC_HCLKConfig(RCC_SYSCLK_Div1);                           /* HCLK = SYSCLK */
      	RCC_PCLK2Config(RCC_HCLK_Div1);                            /* PCLK2 = HCLK */
      	RCC_PCLK1Config(RCC_HCLK_Div2);                            /* PCLK1 = HCLK/2 */
      	RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_9);       /* PLLCLK = 8MHz * 9 /2= 36 MHz */
      	RCC_PLLCmd(ENABLE);                                        /* Enable PLL */ 
      	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)         /* Wait till PLL is ready */
      	{
      	}
      	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);                 /* Select PLL as system clock source */
      	while(RCC_GetSYSCLKSource() != 0x08)                       /* Wait till PLL is used as system clock source */
      	{
      	}
  	}
}

void RCC_Enable()
{	
	RCC_Configuration();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	//ʹ��DMAʱ��
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		
 	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	//ʹ��ADC��GPIOCʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
	//ʹ��USART��GPIOCʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);
	//CANʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
}


void CFG_Nvic()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
#ifdef  VECT_TAB_RAM  
	/* Set the Vector Table base location at 0x20000000 */ 
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */ 
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;

    NVIC_Init(&NVIC_InitStructure);

	/* Enable CAN RX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void CFG_Usart()	//����
{
	USART_InitTypeDef USART_InitStructure;		  		 
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	USART_Cmd(USART1, ENABLE);
	USART_ClearFlag(USART1,USART_FLAG_TC);      
}

void CFG_Can()
{
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	 /* CAN register init */
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    /* CAN cell init */
    CAN_InitStructure.CAN_TTCM = DISABLE;//��ֹʱ�䴥��ͨ��ģʽ
    CAN_InitStructure.CAN_ABOM=DISABLE;//�������CAN_MCR�Ĵ�����INRQλ������1�����0��һ��Ӳ����⵽128��11λ����������λ�����˳�����״̬��
    CAN_InitStructure.CAN_AWUM=DISABLE;//˯��ģʽͨ�����CAN_MCR�Ĵ�����SLEEPλ�����������
    CAN_InitStructure.CAN_NART=DISABLE;//DISABLE;CAN����ֻ������1�Σ����ܷ��͵Ľ����Σ��ɹ���������ٲö�ʧ��
    CAN_InitStructure.CAN_RFLM=DISABLE;//�ڽ������ʱFIFOδ��������������FIFO�ı���δ����������һ���յ��ı��ĻḲ��ԭ�еı���
    CAN_InitStructure.CAN_TXFP=DISABLE;//����FIFO���ȼ��ɱ��ĵı�ʶ��������
    //CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
    CAN_InitStructure.CAN_Mode=CAN_Mode_Normal; //CANӲ������������ģʽ 
    CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;//����ͬ����Ծ���1��ʱ�䵥λ
    CAN_InitStructure.CAN_BS1=CAN_BS1_3tq;//ʱ���1Ϊ8��ʱ�䵥λ
    CAN_InitStructure.CAN_BS2=CAN_BS2_2tq;//ʱ���2Ϊ7��ʱ�䵥λ
    CAN_InitStructure.CAN_Prescaler = 6;//(500Kbits) 9;//250Kbits�趨��һ��ʱ�䵥λ�ĳ���9	 (36000000/(1+2+3)/12)
    CAN_Init(CAN1, &CAN_InitStructure);
    
    /* CAN filter init ��������ʼ��*/
    CAN_FilterInitStructure.CAN_FilterNumber = 0;//ָ���˴���ʼ���Ĺ�����0
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ָ���˹�����������ʼ������ģʽΪ��ʶ������λģʽ
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//�����˹�����λ��1��32λ������
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;//�����趨��������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ����
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;//�����趨��������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ��
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;//�趨��ָ���������FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation  =ENABLE;//ʹ�ܹ�����		 	
    CAN_FilterInit(&CAN_FilterInitStructure);
    
    /* CAN FIFO0 message pending interrupt enable */ 
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);//ʹ��ָ����CAN�ж�ol
}

void IWDG_Configuration(void)
{
	RCC_LSICmd(ENABLE);

	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

  	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  	/* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
  	IWDG_SetPrescaler(IWDG_Prescaler_256);

  	/* Set counter reload value to 299 */
  	IWDG_SetReload(312);   //2s   ι��ʱ�����:(Prescaler/4)*0.1*RLR=2000ms

 	 /* Reload IWDG counter */
  	IWDG_ReloadCounter();

  	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  	IWDG_Enable();
}

void CFG_GPio() 	
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	
	
	//GPIOA ADC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_2 |GPIO_Pin_3 ;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7 |GPIO_Pin_8;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);

	//GPIOB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 |GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_10|GPIO_Pin_11 |GPIO_Pin_12 |GPIO_Pin_13 |GPIO_Pin_14|GPIO_Pin_15;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);//

	//GPIOC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5 |GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9 |GPIO_Pin_10 |GPIO_Pin_11|GPIO_Pin_12;	 //���
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //����
  	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//CAN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					//�ܽ�11
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			 	//����ģʽ
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;					//�ܽ�12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			 	//���ģʽ
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO��

    /* Configure USARTy Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
    GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

void CFG_Dma()		//DMA
{
	DMA_InitTypeDef DMA_InitStructure;
	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);		  								  		//����DMA1�ĵ�һͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;		  		//DMA��Ӧ���������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t )&g_ADCxConvertedValuesArray;     //�ڴ�洢����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;				  		//DMA��ת��ģʽΪSRCģʽ����������Ƶ��ڴ�
	DMA_InitStructure.DMA_BufferSize = 4;		   							//DMA�����С��2��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//����һ�����ݺ��豸��ַ��ֹ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//�رս���һ�����ݺ�Ŀ���ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //�����������ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  	//DMA�������ݳߴ磬HalfWord����Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   						//ת��ģʽ��ѭ������ģʽ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//DMA���ȼ���
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  					//M2Mģʽ����
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);          
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

void CFG_Adc()		//ģ��ת��
{
	ADC_InitTypeDef ADC_InitStructure;
	 /* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		//������ת��ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		    //����ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;      //��������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ADC�ⲿ���أ��ر�״̬
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   //���뷽ʽ,ADCΪ12λ�У��Ҷ��뷽ʽ
	ADC_InitStructure.ADC_NbrOfChannel = 4;	 				 //����ͨ������2��
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel9 configuration */ //ADCͨ���飬 ��9��ͨ�� ����˳��1��ת��ʱ��
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);  
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5);  
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_239Cycles5);  

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);	  //ADC���ʹ��		
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);  //����ADC1
	
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);	  //����У׼
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));  //�ȴ�����У׼���
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);		//��ʼУ׼
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));	   //�ȴ�У׼���
	 
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//����ת����ʼ��ADCͨ��DMA��ʽ���ϵĸ���RAM��.

}

void init()
{
	RCC_Enable();	//ʹ��ʱ��
	CFG_GPio(); 	//��������
	
    //����������ϴεı���״̬�Ļ�������������ʱ���صƵ�����£����ֵ�ɫ����
    RecorveryLastStatus();

	CFG_Nvic();		//NVIC����
	CFG_Dma();		//DMA
	CFG_Adc();		//ģ��ת��
	CFG_Can();      //CAN
	IWDG_Configuration();
	CFG_Usart();	//����
	
}

//�õ�AD����ֵ�����ͣ����AD��[0,372]������0��[3721,4468]����1��[1675,2420]����2 
uint8_t GetADValueType(int val)
{
	if(val >= 0 && val <= 372)
	{
		return 0;	
	}else if(val >= 3721 && val <= 4468)
	{
		return 1;	
	}else if(val >= 1675 && val <= 2420)
	{	
		return 2;	
	}else
	{
		return 0;	
	}	
}

//����İ�󣬲���ֱ�Ӷ�ȡGPIO�ڵ�ֵ��Ϊ�ƿذ�Ļ���ַ�ˣ������߼�ԭ������:
//ʹ��PC0 PC1��ΪAD�ɼ����ţ�����PC0��Ϊ��λ��PC1��Ϊ��λ
//		PC1��ADֵ			PC0��ADֵ					����ַ
//			0					0							0  	
// 			0					4096						1
//			4096				0						    2
//			4096				4096						3
//			0					2048						4
//			2048				0							5
uint8_t GetNewBaseAddr(volatile uint16_t valPC0,volatile uint16_t valPC1)
{
	uint8_t nTypePC0 = GetADValueType(valPC0);
	uint8_t nTypePC1 = GetADValueType(valPC1);
	
	printf("pc0  %d ,  pc1  %d \r\n",valPC0,valPC1);
	if(nTypePC0 <= 1 && nTypePC1 <= 1)
	{
		return (nTypePC1*2+nTypePC0);	
	}else{
	   return ((nTypePC0 == 2) ? 4 : 5);
	}	
}


/*****************************************************************************
 �� �� ��  : main
 ��������  : ������
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : ����ָʾ�Ƹ�ΪPA6

  3.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ʱ������Ϣ���򿪺󣬽�����LED�Ƶ�����
  4.��    ��   : 2015��1��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : ����������ʱʱ��,�ڿ��Ź��������ٿ���CAN�����ж�,��ѭ���ﲻ���ṩι������������	
*****************************************************************************/
int main(void)
{
    uint32_t heart = 0;
    uint8_t sleepCount = 0;//���س���ͣ�������������ƿذ壬Ҫ�ȴ�sleepCount�������л�������λ�ӹܡ�
    uint32_t tmp = 0;
#ifdef EXCUTE_FIXED_CYCLE
	uint8_t initYellowFlashSec = 20; //��ʼ��ʱ����20s��ֹ���ذ廹δ���ʱ�ƿذ���������
	uint8_t initAllRedSec = 6; //��ʼ��ʱ����������δ�յ����ذ���Ϣ��ȫ��6s
	LCBruninfo runinfo;
	uint8_t cycleTime = 0;
	RunData rundata[MAX_RUNDATA_NUM];
#endif
    
    init();
    
     /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(72000000 / 1000))
    { 
        /* Capture error */ 
        printf("main set tick error .\r\n");
    }else{
        NVIC_SetPriority(SysTick_IRQn, 0x0); 
    }
    
    ReadFlashData(BOARD_BASE_ADDR,&tmp,sizeof(tmp));
    if(tmp >= 0 && tmp <= 7){
        g_nBaseAddr = tmp;
        gIsUseFlaseAddr = 1;
    }else{
        g_nBaseAddr = GetBaseAddr();
    }
    
#ifdef EXCUTE_FIXED_CYCLE
	//InitDefaultConfig();
	LCBconfigInit();
#endif
    printf("==> System Restart ... g_nBaseAddr  %d \r\n",g_nBaseAddr);

    while(1)
    {
        g_nBaseAddr = GetBaseAddr();
        IWDG_ReloadCounter();
		//GetNewBaseAddr(g_ADCxConvertedValuesArray[0],g_ADCxConvertedValuesArray[1]);
	//	printf("Base Addr is :  0x%x\r\n",GetNewBaseAddr(g_ADCxConvertedValuesArray[0],g_ADCxConvertedValuesArray[1]));        
		if (g_heart_count1 == g_heart_count2)
		{	
            if(++sleepCount <= 6)
            {
                if(initYellowFlashSec != 0)
                {
                    initYellowFlashSec -= sleepCount;
                }
                HAL_Delay(1000);
                continue;
            }
		    
			heart++;
#ifdef EXCUTE_FIXED_CYCLE
			if (initYellowFlashSec > 0)
			{	//���ϵ�ʱÿ���ƿذ����ִ�л����ȴ����ذ巢�͵����Ϣ
				CanSendYellowBlink();
				initYellowFlashSec--;
				continue;
			}
			if (initAllRedSec > 0)
			{	//�������������δ�յ����ذ���Ϣ��ƿذ������ӹ�ִ��ȫ��
				SpecialControlLight(LRED);				//ȫ�����
				initAllRedSec--;
				heart = ONE_SECOND_HEARTS - 1;
				LightFrontBoardButtonLed(ALL_BUTTON_LED);
				gRuninfo.schemeid = 1;	//Ĭ��һ�����ӹ�ִ�з���1
				continue;
			}
			else if (gFaultFlag == 1)
			{
				SpecialControlLight(LYELLOW_FLASH);		//��������
				continue;
			}
			if (heart <= ONE_SECOND_HEARTS)
			{
				if (gLCBconfig->configstate == CONFIG_UPDATE)
					ReadFlashData(LCB_CONFIG_ADDR, gLCBconfig, sizeof(LCBconfig));
				if (heart > ONE_SECOND_HEARTS / 2)//����ǰ���ȫ��������LED�������ذ�canͨ���쳣
					LightFrontBoardButtonLed(ALL_BUTTON_LED);
				if (heart == ONE_SECOND_HEARTS)	
				{	
					runinfo = gRuninfo;
					if (runinfo.schemeid == 254)
						runinfo.schemeid = 0;	//��Ӧ����Ϊ����0
					if (runinfo.schemeid <= MAX_SUPPORT_SCHEME_NUM)
					{
						cycleTime = CalRunData(runinfo.schemeid, rundata);
						runinfo.runtime = FindTakeOverPosition(&runinfo, rundata);
					}
					printf("runinfo.schemeid = %d, runinfo.runtime = %d, cycleTime = %d\r\n", runinfo.schemeid, runinfo.runtime, cycleTime);
				}
				HAL_Delay(DELAY_MSEC);
				IWDG_ReloadCounter();
			}
			else if (heart > ONE_SECOND_HEARTS && heart <= TIMEOUT_HEARTS)
			{
				if (gLCBconfig->baseinfo.isTakeOver == 1
					&& IS_OWN_ACCESS(g_nBaseAddr)
					&& (runinfo.schemeid <= MAX_SUPPORT_SCHEME_NUM)
					&& runinfo.runtime <= cycleTime)
				{
					if (runinfo.runtime >= cycleTime)
						runinfo.runtime = 0;
					ExcuteFixedCycle(&rundata[runinfo.runtime++]);
					heart = ONE_SECOND_HEARTS;
				}
				else
				{
					HAL_Delay(DELAY_MSEC);
					IWDG_ReloadCounter();
				}
			}
			else
			{	//���3s�ڻ�û���յ�������˵���ӹ�ʧ�ܣ��ƿذ����ִ�л���
				printf("isTakeOver:%d, baseaddr:%d, controlBoardNo:%d, runtime:%d, schemeid:%d\r\n", 
					gLCBconfig->baseinfo.isTakeOver, g_nBaseAddr, gLCBconfig->controlBoardNo, 
					runinfo.runtime, runinfo.schemeid);
				CanSendYellowBlink();//����
				heart = TIMEOUT_HEARTS;
			}
#else
			if (heart > TIMEOUT_HEARTS)	//3�����ղ������ذ�can�����Ϣ�ͻ���
			{
				CanSendYellowBlink();//����
				heart = TIMEOUT_HEARTS;
			}	
#endif
		}
		else
		{
			g_heart_count2 = g_heart_count1;
#ifdef EXCUTE_FIXED_CYCLE
			if (heart > ONE_SECOND_HEARTS / 2)//����ǰ����Զ�������LED�������ذ�canͨ�Żָ�����
				LightFrontBoardButtonLed(AUTO_BUTTON_LED);
			initYellowFlashSec = 0;
			initAllRedSec = 0;
			gFaultFlag = 0;
#endif
			heart = 0;
			sleepCount = 0;
			HAL_Delay(DELAY_MSEC);
			IWDG_ReloadCounter();
		}
        HAL_GPIO_TogglePin(GPIOA,GPIO_Pin_6);
    }

//    return 0;
}

