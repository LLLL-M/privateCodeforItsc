//ǰ���00000000000000000000000000000


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

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void EXTI_IRQHandler_Config(void);
void SetKeyLight(uint8_t num,uint8_t keyStatus);
static void DisabledEXTI(uint32_t num);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


__IO uint32_t TimingDelay = 0;

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


static uint8_t g_nArrayLedGroupStatus[4][4];//�ֱ��ʾ�ƿذ�1-4�ĵ���1-4�ĵ�״̬
static uint32_t g_CountRecved = 0;//��������������������Ƶ��


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

void CAN_TX1_SendData(CanTxMsg *pTxMessage)
{
	//u16 i = 0;
	//u8 TransmitMailbox = 0;
	CAN_Transmit(CAN1, pTxMessage);

}


/*****************************************************************************
 �� �� ��  : CalLedOutputCode
 ��������  : ���ݵ�ɫ����Ӧ�ø��Ĵ�����ֵ
 �������  : LEDSTATUS nLedStatus  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��1��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �ٴ��޸��ĸ������˳��
*****************************************************************************/
uint32_t CalLedOutputCode(LEDSTATUS nLedStatus)
{
    uint8_t i = 0;//i ����ѭ�����������ĸ�����
    uint8_t j = 0;//j ����ѭ����ת��ֱ�С���ת�������ĸ����̵�
    uint32_t tmp = 0;

    for(i = 0;i < 4 ; i++)
    {
        for(j = 0; j < 4 ; j++)
        {
            if(nLedStatus != (g_nArrayLedGroupStatus[i][j]&nLedStatus))//�ж��̵ơ���ơ��Ƶ��Ƿ���Ҫ������Ĭ���ǵ�����
            {
                if(3 == j)//��������ˣ���˳��ֱ���GB13 GB14 GB15 GB16
                {
                    if(nLedStatus == YELLOW)//����ǻƵƵĻ���ֱ��continue���������м���
                    {
                        continue;
                    }
                    tmp |= ((uint16_t)0x01 << (12 + i)) ;
                    continue;
                }
            
                if(i < 3)//���������������ͬ���ְ�ΪGPIO 7 8 9, 4 5 6, 1 2 3, 10 11 12
                {
                    tmp |= ((uint16_t)0x01 << (6 + j - i*3)) ;
                }
                else
                {
                    tmp |= ((uint16_t)0x01 << (9 + j)) ;
                }
            }
        }
    }

    return tmp;
}

/*****************************************************************************
 �� �� ��  : SetGroupsLedStatus
 ��������  : ��CAN��Ϣ�������֮�󣬸��ݸ���Ƶ�״̬�����ε�ơ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : Ӳ����������ƵƸı�󣬴�������Ӧ������

  3.��    ��   : 2015��1��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���ն���ͨ��1234������ͨ��5678������ͨ��9 10 11 12������ͨ
                 ��13 14 15 16��˳�����µ��
*****************************************************************************/
void SetGroupsLedStatus(void)
{
    //green
    GPIOB->ODR = CalLedOutputCode(GREEN); 
    GPIO_WriteBit(GPIOC,GPIO_Pin_12,Bit_RESET);   
    GPIO_WriteBit(GPIOC,GPIO_Pin_12,Bit_SET); 

    //red
    GPIOB->ODR = CalLedOutputCode(RED); 
    GPIO_WriteBit(GPIOD,GPIO_Pin_2,Bit_RESET);   
    GPIO_WriteBit(GPIOD,GPIO_Pin_2,Bit_SET);      

    //yellow
    GPIOC->ODR = CalLedOutputCode(YELLOW);
}


/*****************************************************************************
 �� �� ��  : CanGetLedStatus
 ��������  : �������Եƿذ�ĵ����Ϣ
 �������  : CAN_HandleTypeDef *CanHandle  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2015��1��7��
    ��    ��   : xiaowh
    �޸�����   : �ж�22·ʱ�ķ�����1��ʾ44·��2��ʾ22·��Ӧ��ֱ���жϲ�Ӧ���жϵ�һλ��ֵ
*****************************************************************************/
void CanGetLedStatus(CanRxMsg *CanHandle)
{
    uint8_t nGroupId;//�ƿذ���ϵ��£�������ƣ�ǰ�����ǻ��������к���̵ı仯������4��Ϊ���˵ƣ�ֻ�к��̵ı仯��
    
    switch(CanHandle->StdId)
    {
        case 0x301: nGroupId = 0;break;
        case 0x302: nGroupId = 1;break;
        case 0x303: nGroupId = 2;break;
        case 0x304: nGroupId = 3;break;
        default:    nGroupId = 0;break;
    }

    g_nArrayLedGroupStatus[nGroupId][0] = CanHandle->Data[1]&0x07;
    g_nArrayLedGroupStatus[nGroupId][1] = CanHandle->Data[2]&0x07;
    g_nArrayLedGroupStatus[nGroupId][2] = CanHandle->Data[3]&0x07;
    g_nArrayLedGroupStatus[nGroupId][3] = CanHandle->Data[4]&0x07;


    if(2 == (CanHandle->Data[0]))//22·������� ��ʱҪ�Ѷ��Ϸ����״̬��Ϣ���Ƶ���������ȥ��
    {
        //printf("===>   StdId  0x%x   GroupId  %d  \r\n",CanHandle->StdId,nGroupId);
        memcpy(g_nArrayLedGroupStatus[2],g_nArrayLedGroupStatus[0],4);
        memcpy(g_nArrayLedGroupStatus[3],g_nArrayLedGroupStatus[1],4);
    }
}

/*****************************************************************************
 �� �� ��  : SetGPIOInputOrOutput
 ��������  : ɨ�谴������ʱӦ�ð�GPIOA0-4����Ϊ����ģʽ����������������LED��
             ʱӦ�øĳ��������ģʽ
 �������  : uint8_t mode  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetGPIOInputOrOutput(uint8_t mode)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    if(0 == mode)//input
    {
    	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AIN;//GPIO_MODE_AF_OD
    	//gGPIO_InitStruct.Pull  = GPIO_NOPULL;
    }
    else if(1 == mode)//output
    {
    	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;//GPIO_MODE_AF_OD
    	//gGPIO_InitStruct.Pull  = GPIO_PULLDOWN;
    }
    
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;

//	GPIO_DeInit(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4);

    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
}


/*****************************************************************************
 �� �� ��  : CanGetKeyStatus
 ��������  : �������Ժ��İ�İ���״̬��Ϣ
 �������  : CAN_HandleTypeDef *CanHandle  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void CanGetKeyStatus(CanRxMsg *CanHandle)
{

//    GPIO_InitTypeDef   GPIO_InitStructure;

    uint8_t i = 0;

    if(0x110 != CanHandle->StdId)
    {
        return ;
    }


    DisabledEXTI(EXTI_Line0);
    DisabledEXTI(EXTI_Line1);
    DisabledEXTI(EXTI_Line2);
    DisabledEXTI(EXTI_Line3);
    DisabledEXTI(EXTI_Line4);

    //����GPIOAΪ���ģʽ
    SetGPIOInputOrOutput(1);

    for(i = 0; i < 5 ; i++)
    {
        SetKeyLight(i+1,((CanHandle->Data[0]>>i) & 0x01));
    }

    EXTI_IRQHandler_Config();

}

void CanSendKeyStatus(uint8_t nKeyStatus)
{
    CanTxMsg msg;
    
    msg.StdId = 0x401;
    msg.ExtId = 0;
    msg.RTR = CAN_RTR_DATA;
    msg.IDE = CAN_ID_STD;
    msg.DLC = 1;
    msg.Data[0] = nKeyStatus;

   CAN_TX1_SendData(&msg);
}


//�ֱ����ֶ����Զ���������ȫ�졢����������״̬
#define KEY0	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
#define KEY1	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)
#define KEY2	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)
#define KEY3	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)
#define KEY4	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)

/*****************************************************************************
 �� �� ��  : SetKeyLight
 ��������  : ���ݰ���������Ӧ�ĵ�
 �������  : uint8_t num        
             uint8_t keyStatus  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetKeyLight(uint8_t num,uint8_t keyStatus)
{
    uint16_t GPIO_pin;
    
	GPIO_WriteBit(GPIOA,GPIO_Pin_7,Bit_RESET);		   //�ȵͺ�ߣ�������

    switch (num) {
        case 1: GPIO_pin = GPIO_Pin_0; break;
        case 2: GPIO_pin = GPIO_Pin_1; break;
        case 3: GPIO_pin = GPIO_Pin_2; break;
        case 4: GPIO_pin = GPIO_Pin_3; break;
        case 5: GPIO_pin = GPIO_Pin_4; break;
    }
    //printf("+++>  key   %d   Pin  %d\r\n",keyStatus,GPIO_pin);
    GPIO_WriteBit(GPIOA,GPIO_pin, keyStatus ? Bit_RESET : Bit_SET);

	GPIO_WriteBit(GPIOA,GPIO_Pin_7,Bit_SET);	
    //HAL_Delay(10);
	GPIO_WriteBit(GPIOA,GPIO_Pin_7,Bit_RESET);		   //�ȵͺ�ߣ�������

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t nKeyStatus = 0;//����LED�Ƶ�״̬����5λ��Ч���ֱ��ʾ�Զ����ֶ���������ȫ�졢������״̬��1��ʾ���£�0��ʾδ���¡�

    if ((GPIO_Pin == GPIO_Pin_0)&&(!KEY0))
    {
        nKeyStatus |= 0x01;
    }
    else if((GPIO_Pin == GPIO_Pin_1)&&(!KEY1))
    {
        nKeyStatus |= (0x01 << 0x01);
    }
    else if((GPIO_Pin == GPIO_Pin_2)&&(!KEY2))
    {
        nKeyStatus |= (0x01 << 0x02);
    }
    else if((GPIO_Pin == GPIO_Pin_3)&&(!KEY3))
    {
        nKeyStatus |= (0x01 << 0x03);
    }
    else if((GPIO_Pin == GPIO_Pin_4)&&(!KEY4))
    {
        nKeyStatus |= (0x01 << 0x04);
    }
    if(nKeyStatus != 0)//��ֹ�����
    {
        printf("===>  HAL_GPIO_EXTI_Callback , Pin  %d   nKeyStatus  %d\r\n",GPIO_Pin,nKeyStatus);

        //HAL_GPIO_TogglePin(GPIOC,GPIO_Pin_3);
        printf("Send Key Status :  0x%x \r\n",nKeyStatus);
        CanSendKeyStatus(nKeyStatus);
    }
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
//	int rc = -1;
	CanRxMsg RxMessage;
	
//	u8 str[128]="";
//	int i=0;
	
	IWDG_ReloadCounter();  //ι��
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

   if((1 == RxMessage.DLC)&&(0x110 == RxMessage.StdId))//���ذ��ǰ��巢�͵İ�������״̬��Ϣ
    {
        IWDG_ReloadCounter();
       // printf("StdID :  %d , Data[0]  :  0x%x \r\n",CanHandle->StdId,CanHandle->Data[0]);
        CanGetKeyStatus(&RxMessage);

    }
    else if((5 == RxMessage.DLC)&&((0x301 == RxMessage.StdId) || (0x302 == RxMessage.StdId) || (0x303 == RxMessage.StdId) || (0x304 == RxMessage.StdId)))
    {
        IWDG_ReloadCounter();
        CanGetLedStatus(&RxMessage);
        SetGroupsLedStatus();
    }

    if(3 <= g_CountRecved++)
    {
        g_CountRecved = 0;
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
    }else{
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    }

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
	//ʹ��ADC��GPIOCʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
	//ʹ��USART��GPIOCʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOC, ENABLE);
	//CANʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
}


void CFG_Nvic()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
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
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;

    NVIC_Init(&NVIC_InitStructure);

	/* Enable CAN RX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
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
	USART_Cmd(USART1, ENABLE);
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

	//GPIOA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//����Ч��Ĭ����Ҫ���͵�
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//����Ч��Ĭ����Ҫ���ߵ�
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_15;//�ⲿ���������LEDʹ�ܽ�  / �ⲿ���������ʱ�� /CAN�źŽ���ָʾ��
                                                                                // �ⲿ��������ư���ʹ�ܽ�
    GPIO_Init(GPIOA, &GPIO_InitStructure);    


    GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET);	 //����PA5�ߵ�ƽ��ʹ�ܺ���ָʾ��

    GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_RESET);//����PA6�͵�ƽ��ʹ�ܰ�������LED

    GPIO_WriteBit(GPIOA,GPIO_Pin_7,Bit_SET);//

    GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_SET);//Ĭ��״̬�£�Ҫ��CAN����ָʾ���������ʼ��ֻ�Ǹ�ODR�Ĵ���һ��RESETֵ�������RESETֵ������
                                                        //0�����ԣ�Init������CANָʾ�ƻ�����.
    GPIO_WriteBit(GPIOA,GPIO_Pin_15,Bit_RESET);  //ʹ���ⲿ����ֵ����


    //GPIOB
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;//
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //����ָʾ������

    GPIOB->ODR = 0xffff;

    //GPIOC
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|
                            GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|
                            GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //��ָʾ�ƿ�����

    GPIOC->ODR |= 0x1fff;

    //GPIOD
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //��ָʾ�ƿ�����

    GPIO_WriteBit(GPIOD,GPIO_Pin_2,Bit_SET);
	

	//CAN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					//�ܽ�11
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			 	//����ģʽ
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;					//�ܽ�12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			 	//���ģʽ
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO��

	//USART
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA , &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA, GPIO_Pin_11);
    GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

void DisabledEXTI(uint32_t num)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = num;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);
}

static void EXTI_IRQHandler_Config(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Configure PC.13 pin as input floating */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);

    /* Configure EXTI0 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //ͬ���жϿ���1 2 3 4
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_Init(&NVIC_InitStructure);

}

void init()
{
	RCC_Enable();	//ʹ��ʱ��
	CFG_Nvic();		//NVIC����
	CFG_Usart();	//����
	CFG_Can();      //CAN
	IWDG_Configuration();
	CFG_GPio(); 	//��������
	EXTI_IRQHandler_Config();//�����ж�����
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
    init();
    
     /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(72000000 / 1000))
    { 
        /* Capture error */ 
        printf("main set tick error .\r\n");
    }else{
        NVIC_SetPriority(SysTick_IRQn, 0x0); 
    }
    printf("==> System Restart ... \r\n");
    
    while(1)
    {
        HAL_Delay(1000);
        IWDG_ReloadCounter();
        //printf("g_nBaseAddr   %d \r\n",g_nBaseAddr);
       // printf("main  %d   -- %d \r\n",++i,SystemCoreClock);
    }
}

