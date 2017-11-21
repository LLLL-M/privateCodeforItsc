#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"	 
#include "stm32f10x_can.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"	  
#include <stdio.h>
#include <string.h>


#define ADC1_DR_Address    ((uint32_t)0x4001244C)	 //ADC���ݼĴ����Ļ���ַ


// ע��ADCΪ12λģ��ת������ֻ��ADCConvertedValue�ĵ�12λ��Ч
__IO uint16_t ADCConvertedValue[2]={0};  //AD0,AD1

u8 msg[128]="";   //���ڴ�ӡ��buf

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

u8 heart_count1 = 0;
u8 heart_count2 = 0;
u8 interrupted = 0;

u8 slotAddr = 0;   		   //������ַ  
u8 g_lampboardNo = 0;
u32 g_lampVolt = 0;		   //�Ƶ�ѹ
u8 g_lampCur[6] = {0};     //�Ƶ��� 

/**	@fn	void Delay(__IO uint32_t nCount)
  * @brief  ��ʱ
  * @param  nCount ѭ������
  * @retval None
  */
static void Delay(__IO uint32_t nCount)
{
	for (; nCount != 0; nCount--);
}

/*
 * =====================================================================
 * Function: IWDG_Configuration
 * Description: 
 * Input:    none
 * Output:  none
 * Return:  none
 * Discrip: 
 *======================================================================
 */
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

/**	@fn	void CFG_GPio()
  * @brief  ��������
  * @param  None
  * @retval None
  */
void CFG_GPio() 	
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	
	//��Դָʾ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);

	//CTL
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_2 |GPIO_Pin_3 |GPIO_Pin_4;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//ADDR
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 |GPIO_Pin_7 |GPIO_Pin_8 |GPIO_Pin_9;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);	  //�õ�

	//DATA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_10|GPIO_Pin_14;	 //���
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12| GPIO_Pin_13; //����
  	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//ADC                        
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;		//�ܽ�0,1
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;			 	//����ģʽ
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO��

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

	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET);  //ʹ�ܴ�
	GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_RESET);  //ʹ�ܴ�

	GPIO_SetBits(GPIOA, GPIO_Pin_11);
    GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

/**	@fn	void CFG_Adc()
  * @brief  A/D�ṹ����
  * @param  None
  * @retval None
  */
void CFG_Adc()		//ģ��ת��
{
	ADC_InitTypeDef ADC_InitStructure;
	 /* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		//������ת��ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		    //����ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;      //��������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ADC�ⲿ���أ��ر�״̬
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   //���뷽ʽ,ADCΪ12λ�У��Ҷ��뷽ʽ
	ADC_InitStructure.ADC_NbrOfChannel = 2;	 				 //����ͨ������2��
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel9 configuration */ //ADCͨ���飬 ��9��ͨ�� ����˳��1��ת��ʱ��
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);                   	
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

/**	@fn	void CFG_Dma()
  * @brief  DMA�ṹ����
  * @param  None
  * @retval None
  */
void CFG_Dma()		//DMA
{
	DMA_InitTypeDef DMA_InitStructure;
	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);		  								  		//����DMA1�ĵ�һͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;		  		//DMA��Ӧ���������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;     //�ڴ�洢����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;				  		//DMA��ת��ģʽΪSRCģʽ����������Ƶ��ڴ�
	DMA_InitStructure.DMA_BufferSize = 2;		   							//DMA�����С��2��
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

/**	@fn	void CFG_Usart()
  * @brief  ���ڽṹ����
  * @param  None
  * @retval None
  */
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

/**	@fn	void SendData_Series(u8 *chStr)
  * @brief  ͨ�����ڷ�������
  * @param  chStr  �ַ�����Ϣ
  * @retval None
  */
void SendData_Series(u8 *chStr)
{
	int i=0;
	for (i=0; chStr[i]!='\0'; i++)
	{
		USART_SendData(USART1, chStr[i]);		  //����һ�ֽ�����
		/* Loop until the end of transmission */
 		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	}
}

/**	@fn	void CFG_Usart()
  * @brief  ʹ��ʱ��
  * @param  None
  * @retval None
  */
void RCC_Enable()
{	
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

/**	@fn	void CFG_Can()
  * @brief  CAN��������
  * @param  None
  * @retval None
  */
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
    CAN_InitStructure.CAN_BS1=CAN_BS1_6tq;//ʱ���1Ϊ8��ʱ�䵥λ
    CAN_InitStructure.CAN_BS2=CAN_BS2_3tq;//ʱ���2Ϊ7��ʱ�䵥λ
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

/**	@fn	void init()
  * @brief  ��ʼ��
  * @param  None
  * @retval None
  */												
void init()
{
	RCC_Enable();	//ʹ��ʱ��
	CFG_Nvic();		//NVIC����
	CFG_Dma();		//DMA
	CFG_Adc();		//ģ��ת��
	CFG_Usart();	//����
	CFG_Can();      //CAN
	IWDG_Configuration();
	CFG_GPio(); 	//��������
}

/**	@fn	u8 GetRequest()
  * @brief  ��ȡCPLD����ĵ�����
  * @param  None
  * @retval CPLD����ĵ�����
  */	
u8 GetRequest()
{
	u8 ReadValue6, ReadValue7, ReadValue8, ReadValue9; 
	ReadValue6 = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);
	ReadValue7 = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7);
	ReadValue8 = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8);
	ReadValue9 = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9);

	sprintf((char*)msg, "��ַ��: A3:%d,A2:%d,A1:%d,A0:%d\r\n", ReadValue6, ReadValue7, ReadValue8, ReadValue9);
	SendData_Series(msg);
	return (ReadValue6<<3)| (ReadValue7<<2)| (ReadValue8<<1)| ReadValue9;
}


//���� 2013.08.07
void WriteLedSatus(u8 led_status)
{
	sprintf((char*)msg, "WriteLedSatus:%d\r\n", led_status);
	SendData_Series(msg);
	switch (led_status)
	{
	case 0:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_RESET);
		break;
	case 1:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_RESET);
		break;
	case 2:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_RESET);
		break;
	case 3:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_RESET);
		break;
	case 4:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_SET);
		break;
	case 5:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_SET);
		break;
	case 6:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_SET);
		break;
	case 7:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_SET);
		break;
	default:
		break;
	}
}

//���� 2013.08.07
void SetLedGroup(u8 led_group_no)
{
	sprintf((char*)msg, "led_group_no:%d\r\n", led_group_no);
	SendData_Series(msg);
	Delay(1000);
	switch (led_group_no)
	{
	case 1:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;
	case 2:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;
	case 3:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;
	case 4:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;
	case 5:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;
	case 6:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;
	case 7:	 //7-12��ʾ1-6��ĵƵĵ�ѹ����
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;
	case 8:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		break;
	case 9:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		break;
	case 10:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		break;
	case 11:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		break;
	case 12:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		break;
	case 13:   //����CPLD�������
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		break;
	case 15: //��ȡ������ַ
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		break;
	default:
		break;
	}
}

//���� 2013.08.07
u8 GetLedVStatus(u8 led_group_no)
{
	u8 red, yellow, green; 
	yellow = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
	red = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
	green = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);

	if (led_group_no == 0xf)	  //������ַ,�ߵ�λ����
	{
		sprintf((char*)msg, "������ַ: %d,%d,%d\r\n", green, red, yellow);
		SendData_Series(msg);
		return (green<<2)| (red<<1)| yellow;
	}
	sprintf((char*)msg, "��%d���״̬: YELLOW:%d,RED:%d,GREEN:%d\r\n", led_group_no-6, yellow, red, green);
	SendData_Series(msg);
	
	return (yellow<<2)| (red<<1)| green;
}

//���� 2013.08.07
//����������
void SetRequest(u8 led_group_no, u8 led_status)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_RESET);
	Delay(1000);
	WriteLedSatus(led_status);
	SetLedGroup(led_group_no);
	Delay(1000);
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);
}

//���� 2013.08.07
//��ȡ��ƺ�״̬
u8 GetResponse(u8 led_group_no)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_RESET);
	Delay(1000);
	SetLedGroup(led_group_no);
	Delay(1000);
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);
	Delay(1000);
	return GetLedVStatus(led_group_no);
}


/**	@fn	u8 SetChannel(u8 led_no)
  * @brief  ʹ���ⲿ����
  * @param  led_no 	������(0-15)
  * @retval ������(0-5)
  */
u8 SetChannel(u8 led_no)
{
	u8 group_no = 1;
	sprintf((char*)msg, "SetChannel_led_no:%d\r\n", led_no);
	SendData_Series(msg);
	switch (led_no)
	{
	case 1:
		GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_RESET);
		group_no = 1;
		break;
	case 2:
		GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_RESET);
		group_no = 2;
		break;
	case 3:
		GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_RESET);
		group_no = 3;
		break;
	case 4:
		GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_RESET);
		group_no = 4;
		break;
	case 5:
		GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_SET);
		group_no = 5;
		break;
	case 6:
		GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_SET);
		group_no = 6;
		break;
	default:
		break;
	}
	return group_no;
}

/**	@fn	void CAN_TX1_SendData(u8 *pData, u8 nLen)
  * @brief  CAN���߷�������
  * @param  pData	����
  * @param  nLen	���ݳ���
  * @retval None
  */
void CAN_TX1_SendData(CanTxMsg *pTxMessage)
{
	u16 i = 0;
	u8 TransmitMailbox = 0;
	TransmitMailbox = CAN_Transmit(CAN1, pTxMessage);

	while((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFF))	  //�ȴ����ͽ���
	{
		i++; 	
	}
	i = 0;
	while((CAN_MessagePending(CAN1, CAN_FIFO0) < 1) && (i != 0xFF))
	{
		i++;
	}

 	if (i >= 0XFF)
	{
		SendData_Series("can_senddata_failed\r\n");	
	}
	else
	{
		SendData_Series("can_senddata_success\r\n");
	}
}

void GetlampVoltCur(u8 byStartNo, u8 byLampCount)
{
	u8 i = 0;
	u8 volt = 0;
	uint16_t adcRedValue = 0;
	g_lampVolt = 0;
	for (i=0; i<byLampCount; i++)
	{
		volt = GetResponse(byStartNo+i+6);
		volt = (~volt)&0x7;
		g_lampVolt |= ((volt&0x7)<<(i*3));
		sprintf((char*)msg, "g_lampVolt(%d):%d\r\n", i, g_lampVolt);
		SendData_Series(msg);
		//Ƭѡ,���ڲɼ�����
		SetChannel(i+1);
		Delay(1000);
		adcRedValue = ADCConvertedValue[0];
   		sprintf((char*)msg, "���:%d\r\n",adcRedValue);
		SendData_Series(msg);
		g_lampCur[i] = (adcRedValue*255/4095);//(adcRedValue*330/4095);	 
	}
}

void ControlLamp(u8 byStartNo, u8 byLampCount, u32 wled_status)
{
	u8 i = 0;
	u32 led_status = wled_status;
	sprintf((char*)msg, "byStartNo:%d, byLampCount:%d\r\n", byStartNo, byLampCount);
	SendData_Series(msg);
	for (i=0; i<byLampCount; i++)
	{
		SetRequest(byStartNo+i, (led_status>>(3*i))&7);
	}
}

int HandleLamp(u8 slotAddr, CanRxMsg *pTxMessage)
{
	int rc = 0;
	u32 ledstatus = 0; 
	switch (slotAddr)
	{
	case 2:	  //�ƿذ�1
		if (pTxMessage->Data[0] == 1)
		{
			ledstatus = ((pTxMessage->Data[3]&3)<<16 | pTxMessage->Data[2]<<8 | pTxMessage->Data[1])&0x3ffff;
			sprintf((char*)msg, "slotAddr:%d,ledstatus:%d, data[0]:%d\r\n", slotAddr, ledstatus, pTxMessage->Data[0]);
			SendData_Series(msg);
			ControlLamp(1, 6, ledstatus);
			GetlampVoltCur(1, 6);
			g_lampboardNo = 1;	
		}
		else
		{
			rc = -1;
		}
		break;
	case 3:	  //�ƿذ�2
		if (pTxMessage->Data[0] == 1)
		{
			ledstatus = ((pTxMessage->Data[5]&0xf)<<14 | pTxMessage->Data[4]<<6 | (pTxMessage->Data[3]>>2)&0x3f)&0x3ffff;
			ControlLamp(1, 6, ledstatus);
			GetlampVoltCur(1, 6);
			g_lampboardNo = 2;	
		}
		else
		{
			rc = -1;
		}
		break;
	case 4:	  //�ƿذ�3
		if (pTxMessage->Data[0] == 1)
		{
			ledstatus = ((pTxMessage->Data[7]&0x3f)<<12 | pTxMessage->Data[6]<<4 | (pTxMessage->Data[5]>>4)&0xf)&0x3ffff;
			ControlLamp(1, 6, ledstatus);
			GetlampVoltCur(1, 6);
			g_lampboardNo = 3;	
		}
		else
		{
			rc = -1;
		}
		break;
	case 5:	  //�ƿذ�4
		if (pTxMessage->Data[0] == 2)
		{
			ledstatus = ((pTxMessage->Data[3]&3)<<16 | pTxMessage->Data[2]<<8 | pTxMessage->Data[1])&0x3ffff;
			ControlLamp(1, 6, ledstatus);
			GetlampVoltCur(1, 6);
			g_lampboardNo = 4;	
		}
		else
		{
			rc = -1;
		}
		break;
	case 6:	  //�ƿذ�5
		if (pTxMessage->Data[0] == 2)
		{
			ledstatus = ((pTxMessage->Data[5]&0xf)<<14 | pTxMessage->Data[4]<<6 | (pTxMessage->Data[3]>>2)&0x3f)&0x3ffff;
			ControlLamp(1, 6, ledstatus);
			GetlampVoltCur(1, 6);
			g_lampboardNo = 5;	
		}
		else
		{
			rc = -1;
		}
		break;
	case 7:	  //�ƿذ�6
		if (pTxMessage->Data[0] == 2)
		{
			ledstatus = ((pTxMessage->Data[6]&0x3)<<4 | (pTxMessage->Data[5]>>4)&0xf)&0x3f;
			ControlLamp(1, 2, ledstatus);
			GetlampVoltCur(1, 2);	
			g_lampboardNo = 6;
		}
		else
		{
			rc = -1;
		}
		break;
	default:
		rc = -1;
		break;
	}
	return rc;
}	

static unsigned short led_state_count = 0;
static unsigned short led_flag = 1;  //Ĭ�ϵ���

void led_display()
{
	if (led_state_count % 3  == 0)
	{
		if (led_flag == 0)
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_7);
			led_flag = 1;
		}
		else if (led_flag == 1)
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_7);
			led_flag = 0;
		}	
	}
}

void led_off()
{
	if (led_state_count % 5  == 0)
	{
		 GPIO_SetBits(GPIOA, GPIO_Pin_7);
	}
}

/**	@fn	void CAN_RX1_IRQHandler_def(void)
  * @brief  ����CAN�ж�����
  * @param  None
  * @retval None
  */
void CAN_RX1_IRQHandler_def(void)
{
	int rc = -1;
	CanRxMsg RxMessage;
	CanRxMsg TxMessage;
	u8 str[128]="";
	int i=0;
	
	IWDG_ReloadCounter();  //ι��
	RxMessage.StdId=0;
	RxMessage.ExtId=0; 
	RxMessage.IDE=0; 
	RxMessage.DLC=0;
	RxMessage.FMI=0;
	RxMessage.Data[0]=0x00; 
	RxMessage.Data[1]=0x00;
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	sprintf((char*)msg, "StdId:%x,ExtId:%x,IDE:%x,RTR:%x,DLC:%x,FMI:%x\r\n",
		RxMessage.StdId, RxMessage.ExtId, RxMessage.IDE, RxMessage.RTR, RxMessage.DLC, RxMessage.FMI);
	SendData_Series(msg);

	if (RxMessage.StdId < 0x101 || RxMessage.StdId > 0x106)
	{
		SendData_Series("This is not a ledctrl command.\r\n");
		return;
	}
	//GPIO_SetBits(GPIOA, GPIO_Pin_7);
	led_state_count++;     //ledָʾ�Ƽ���
	led_display();
	if (RxMessage.DLC > 8 || RxMessage.DLC == 0)
	{
		SendData_Series("RxMessage.DLC ERROR\r\n");
		//GPIO_ResetBits(GPIOA, GPIO_Pin_7);
		return;
	}
	memset(msg, 0, sizeof(msg));
	for (i=0; i<RxMessage.DLC; i++)
	{
		sprintf((char*)str, "RxMessage.Data[%d]:%x,", i, RxMessage.Data[i]);
		strcat((char*)msg, (char*)str);
	}
	strcat((char*)msg, "\r\n");
	SendData_Series(msg);
	//���
	rc = HandleLamp(slotAddr, &RxMessage);
	if (rc < 0)	 //����������ǵ�ǰ�����˳�
	{
		SendData_Series("HandleLamp failed!\r\n");
		//GPIO_ResetBits(GPIOA, GPIO_Pin_7);
		return;  
	}
	
	heart_count1++;	   //���ڿ��ƻ���
	interrupted = 1;   //���ڿ��ƻ���

	memset(&TxMessage, 0, sizeof(TxMessage));
	TxMessage.StdId = 0;
	TxMessage.ExtId= (1<<21 | (g_lampVolt&0x3ffff)<<3 | g_lampboardNo&0x7)&0x3fffff; 
	TxMessage.RTR = CAN_RTR_DATA;	//����RTRλΪ����֡
	TxMessage.IDE = CAN_ID_EXT;//CAN_ID_STD;		//��ʶ����չλ��Ϊ��׼֡ 
	TxMessage.DLC=6;
	TxMessage.Data[0] = g_lampCur[0]; 	//0-5��ʾһ�����6���Ƶ���ֵ
	TxMessage.Data[1] = g_lampCur[1];
	TxMessage.Data[2] = g_lampCur[2]; 	  		
	TxMessage.Data[3] = g_lampCur[3];
	TxMessage.Data[4] = g_lampCur[4];        	 
	TxMessage.Data[5] = g_lampCur[5];
	CAN_TX1_SendData((CanTxMsg*)(&TxMessage));
	//GPIO_ResetBits(GPIOA, GPIO_Pin_7); 
}

void yellowon()
{
	u8 i = 0;
	for (i=0; i<6; i++)
	{
		SetRequest(i+1, 4);
	}
}

void yellowoff()
{
	u8 i = 0;
	for (i=0; i<6; i++)
	{
		SetRequest(i+1, 0);
	}
}

void yellowflash()
{
	SendData_Series("***************************************************����*****************************************\r\n");
	yellowon();
	Delay(5000000);
	yellowoff();
	Delay(5000000);
}

u8 getslotaddr()
{
	//�����ַ����
	SetRequest(0xf, 0);
	Delay(1000);
	//��ȡ��ֵַ
	return GetLedVStatus(0xf);
}

/**	@fn	int main(void)
  * @brief  ������
  * @param  None
  * @retval 0
  */
int main(void)
{
	u32 heart = 0;
	u8 i = 0;													 
	init();

	SetRequest(13, 0); //��ַ��13����֪ͨCPLD��ʼ��Ӧ�����������
		  
	while (1)
	{
		//��ȡ������ַ
		slotAddr = getslotaddr();

		IWDG_ReloadCounter(); //ι��
#if 0	//���Դ���
		for (i=0; i<3; ++i)
		{
			SetRequest(6, 1<<i);
			Delay(1000);
			GetResponse(12);
			Delay(50000000);
		}
#else
		if (heart_count1 == heart_count2)
		{
			if (interrupted == 0)
			{
				//����
				yellowflash();
			}
			else
			{
				heart++;
				if (heart > 200)
				{
					//����
					yellowflash();
					heart = 0xffffff00;
				}
			}
		}
		else
		{
			heart_count2 = heart_count1;
			heart = 0;
		}
#endif	
		SendData_Series("*********************************************running******************************\r\n");		 
		Delay(1000);
	}
	return 0;
}

