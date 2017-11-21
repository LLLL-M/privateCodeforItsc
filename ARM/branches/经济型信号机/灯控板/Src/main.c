/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��9��
  ����޸�   :
  ��������   : ���ǵƿذ�MCU������Ҫ�����ǽ��պ��İ���̵������������·
               �ں��̵ƣ�ͬʱ��Ҫ����ѹ������
  �����б�   :
              assert_failed
              CanSendCurVolData
              CanSendLampStatus
              CanSendYellowBlink
              Can_Get_MailBox
              Can_Send_Recv_Data
              Cfg_ADC
              Cfg_Can
              Cfg_IWDG
              Cfg_Uart
              DecodeCanMsg
              Error_Handler
              fputc
              GetBaseAddr
              HAL_CAN_ErrorCallback
              HAL_CAN_RxCpltCallback
              HAL_CAN_TxCpltCallback
              InitGPIOA
              InitGPIOB
              InitGPIOC
              LampControlBoard_GetCur
              LampControlBoard_GetVoltage
              main
              SetLedGroupStatus
              SleepNoEnd
              SystemClock_Config
              TestAllLedLight
  �޸���ʷ   :
  1.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : ��Եڶ����������Ӧ�ĸĶ�
******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "main.h"
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
static void SystemClock_Config(void);
static void Error_Handler(void);
void GetCurAverVal();

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
CAN_HandleTypeDef    g_CanHandle;
ADC_HandleTypeDef    g_AdcHandle;
UART_HandleTypeDef g_UartHandle;
IWDG_HandleTypeDef g_IwdgHandle;
volatile uint16_t   g_ADCxConvertedValuesArray[ADCCONVERTEDVALUES_BUFFER_SIZE];

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static uint16_t g_nCurVauleArray[4] = {0};//�������������ڸ�������
static uint8_t g_nGreenLampVoltageVauleArray[4] = {0};//�����̵Ƶ�ѹֵ
static uint8_t g_nRedLampVoltageVauleArray[4] = {0};//�����Ƶ�ѹֵ
static uint16_t g_nLampVol = 0;//ֻ��Ҫ12λ���ӵ͵��߷ֱ�Ϊ��N����̻ƺ��ѹ��
static uint8_t g_nOutPutMode = 1;//���ģʽ��1:44·���������2:22·������� 
static uint8_t g_ArrayLedGroupStatusArray[4] = {0};//����Ƶ�״̬����Ϊ��3λ��Ч���ֱ��ʾ�̵ơ���ơ��ƵƵ�״̬��1��0��
static uint8_t g_nBaseAddr;//�ƿذ����ַ��0��1��2��3  ��PB3 PB4��ȡ
static uint8_t  g_heart_count1 = 0;//������������յ��������ذ��CAN��Ϣ
static uint8_t  g_heart_count2 = 0;
//static uint8_t g_interrupted = 0;
static uint32_t g_CountRecved = 0;//��������������������Ƶ��
static uint32_t g_CountSend = 0;//��������������������Ƶ��
static uint8_t g_IsCanRecvExit = 0;//�ж�CAN�Ľ����ж��Ƿ��쳣��ֹ������ֹ������main�������ֶ���������CAN�Ľ��ա�


uint32_t gCurValAverArray[4][5];//4���Ƶ�����ʵʱֵ����׼ֵ��ǰ4λΪʵʱֵ�����1λλ��׼ֵ��
uint8_t gCurValIndexArray[4] = {0};//4���Ƶ���������ֵ�ǵ�ǰ����ĸ���˳��
uint8_t gIsCurCalFirstTime[4] = {0};//4���Ƶ����ļ����Ƿ��ǵ�һ����������

#define DELAY_MSEC	250
#define ONE_SECOND_HEARTS	(1000 / DELAY_MSEC)	//1s����������
//��ʱ������,����������������ֵ��δ���յ�can��Ϣ��ƿذ����ִ�л�������
#define TIMEOUT_HEARTS	(3 * ONE_SECOND_HEARTS)
#define EXCUTE_FIXED_CYCLE
#define USE_UART_RECV_IT

#ifdef EXCUTE_FIXED_CYCLE
#include "fixedcycle.h"		//���еĺ���ʵ���Լ��ṹ���ȫ�ֱ����Ķ���ȫ��������
#endif


#ifdef USE_UART_RECV_IT
#define UARTBUFFSIZE	4	/* Buffer used for reception */
static uint8_t gUartBuffer[UARTBUFFSIZE] = {0};
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	uint32_t *data = (uint32_t *)gUartBuffer;
	int i;
	switch (*data)
	{
		case 0x1111ffff: NVIC_SystemReset(); break; //����ϵͳ
		case 0xffff1111:	//��������,�ָ�Ĭ��
			WriteFlashData(LCB_CONFIG_ADDR, &gDefaultConfig, sizeof(LCBconfig));
			gLCBconfig->configstate = CONFIG_UPDATE;
			printf("restore default config!\r\n");
			break;
		case 0x11112222:	//��ӡ����
			printf("baseinfo.isTakeOver = %d\r\n", gLCBconfig->baseinfo.isTakeOver);
			printf("baseinfo.isVoltCheckEnable = %d\r\n", gLCBconfig->baseinfo.isVoltCheckEnable);
			printf("baseinfo.isCurCheckEnable = %d\r\n", gLCBconfig->baseinfo.isCurCheckEnable);
			printf("baseinfo.phaseNum = %d\r\n", gLCBconfig->baseinfo.phaseNum);
			printf("baseinfo.minRedCurVal = %d\r\n", gLCBconfig->baseinfo.minRedCurVal);
			printf("baseinfo.schemeNum = %d\r\n", gLCBconfig->baseinfo.schemeNum);
			printf("baseinfo.canTotalNum = %d\r\n", gLCBconfig->baseinfo.canTotalNum);
			HAL_IWDG_Refresh(&g_IwdgHandle);
			for (i = 0; i < gLCBconfig->baseinfo.phaseNum; i++)
			{
				printf("phaseid %d, greenFlashTime:%d, yellowTime:%d, allredTime:%d, pedFlashTime:%d, channelbits:%#x\r\n", i + 1,\
					gLCBconfig->phaseinfo[i].greenFlashTime, gLCBconfig->phaseinfo[i].yellowTime,\
					 gLCBconfig->phaseinfo[i].allredTime, gLCBconfig->phaseinfo[i].pedFlashTime,\
					gLCBconfig->phaseinfo[i].channelbits);
			}
			HAL_IWDG_Refresh(&g_IwdgHandle);
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
		default: printf("data = %#x\r\n", *data);
	}
	
	HAL_IWDG_Refresh(&g_IwdgHandle);
	if(HAL_UART_Receive_IT(&g_UartHandle, (uint8_t *)gUartBuffer, UARTBUFFSIZE) != HAL_OK)
		printf("HAL_UART_Receive_IT called fail\r\n");
}
#endif

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 48000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PREDIV                         = 1
  *            PLLMUL                         = 6
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  */
static void SystemClock_Config_HSE(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable HSE Oscillator and Activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    Error_Handler(); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)!= HAL_OK)
  {
    Error_Handler(); 
  }

  //����ʱ�Ӱ�ȫϵͳ
  HAL_RCC_EnableCSS();
}


/*****************************************************************************
 �� �� ��  : SystemClock_Config
 ��������  : ϵͳʱ�����ú���
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 

  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI48)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 48000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 48000000
  *            PREDIV                         = 2
  *            PLLMUL                         = 2
  *            Flash Latency(WS)              = 1
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Select HSI48 Oscillator as PLL source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI48;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK and PCLK1 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)!= HAL_OK)
  {
    Error_Handler();
  }
}

/*****************************************************************************
 �� �� ��  : Error_Handler
 ��������  : ��������
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */

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

/*****************************************************************************
 �� �� ��  : SleepNoEnd
 ��������  : �����ú�������ѭ������ֹ������ֹ
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
static void SleepNoEnd(void)
{
    while(1)
    {
        HAL_Delay(500);
        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_7);
    }

}

/*****************************************************************************
 �� �� ��  : Cfg_ADC
 ��������  : �ƿذ�AD��������
 �������  : ��
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��27��
    ��    ��   : Ф�Ļ�
    �޸�����   : ��ADת���޸�Ϊ��������ʽ

  3.��    ��   : 2015��2��27��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���ϴ��룬������ADת���Ĵ�����ӽ���
*****************************************************************************/
static void Cfg_ADC(void)
{
    ADC_ChannelConfTypeDef   sConfig;

    /* Configuration of ADCx init structure: ADC parameters and regular group */
    g_AdcHandle.Instance = ADC1;

    g_AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC;
    g_AdcHandle.Init.Resolution            = ADC_RESOLUTION12b;
    g_AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    g_AdcHandle.Init.ScanConvMode          = ENABLE;                        /* Sequencer enabled (ADC conversion on several channels, successively, following settings below) */
    g_AdcHandle.Init.EOCSelection          = EOC_SINGLE_CONV;
    g_AdcHandle.Init.LowPowerAutoWait      = DISABLE;
    g_AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* Continuous mode disabled to have only 1 rank converted at each conversion trig, and because discontinuous mode is enabled */
    g_AdcHandle.Init.DiscontinuousConvMode = ENABLE;                        /* Sequencer of regular group will convert the sequence in several sub-divided sequences */
    g_AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* Trig of conversion start done manually by software, without external event */
    g_AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    g_AdcHandle.Init.DMAContinuousRequests = ENABLE;
    g_AdcHandle.Init.Overrun               = OVR_DATA_OVERWRITTEN;
 
    if (HAL_OK != HAL_ADC_Init(&g_AdcHandle))
    {
        Error_Handler();
    }

    /* Configuration of channel on ADCx regular group on sequencer rank 1 */
    /* Note: Considering IT occurring after each ADC conversion (IT by DMA end  */
    /*       of transfer), select sampling time and ADC clock with sufficient   */
    /*       duration to not create an overhead situation in IRQHandler.        */
    /* Note: Set long sampling time due to internal channels (VrefInt,          */
    /*       temperature sensor) constraints.                                   */
    /*       For example, sampling time of temperature sensor must be higher    */
    /*       than 17.1us. Refer to device datasheet for min/typ/max values.     */
    sConfig.Channel      = ADC_CHANNEL_0;
    sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    if (HAL_OK != HAL_ADC_ConfigChannel(&g_AdcHandle, &sConfig))
    {
        Error_Handler();
    }

    /* Configuration of channel on ADCx regular group on sequencer rank 2 */
    /* Replicate previous rank settings, change only channel */
    /* Note: On STM32F0, rank is defined by channel number. ADC Channel         */
    /*       ADC_CHANNEL_TEMPSENSOR is on ADC channel 17, there are 2 other     */
    /*       channels enabled with lower channel number. Therefore,             */
    /*       ADC_CHANNEL_TEMPSENSOR will be converted by the sequencer as the   */
    /*       3rd rank.                                                          */
    sConfig.Channel      = ADC_CHANNEL_1;

    if (HAL_OK != HAL_ADC_ConfigChannel(&g_AdcHandle, &sConfig))
    {
        Error_Handler();
    }

    /* Configuration of channel on ADCx regular group on sequencer rank 3 */
    /* Replicate previous rank settings, change only channel */
    /* Note: On STM32F0, rank is defined by channel number. ADC Channel         */
    /*       ADC_CHANNEL_VREFINT is on ADC channel 17, there are 2 other        */
    /*       channels enabled with lower channel number. Therefore,             */
    /*       ADC_CHANNEL_VREFINT will be converted by the sequencer as the      */
    /*       3rd rank.                                                          */
    sConfig.Channel      = ADC_CHANNEL_2;

    if (HAL_OK != HAL_ADC_ConfigChannel(&g_AdcHandle, &sConfig))
    {
        Error_Handler();
    }

    sConfig.Channel      = ADC_CHANNEL_3;

    if (HAL_OK != HAL_ADC_ConfigChannel(&g_AdcHandle, &sConfig))
    {
        Error_Handler();
    }

    if(HAL_OK != HAL_ADCEx_Calibration_Start(&g_AdcHandle))//У׼
    {
        Error_Handler();
    }
    
    if(HAL_OK != HAL_ADC_Start_DMA(&g_AdcHandle, (uint32_t *)g_ADCxConvertedValuesArray, ADCCONVERTEDVALUES_BUFFER_SIZE))//����ɨ��
    {
        Error_Handler();
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
    g_nRedLampVoltageVauleArray[0] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[1] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[2] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[3] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12) == 0 ? 1 : 0);

    //��ȡ�̵Ƶ�ѹֵ
    g_nGreenLampVoltageVauleArray[0] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[1] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[2] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[3] = (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_6) == 0 ? 1 : 0);

    //�����������ѹֵ������֯��12λ����
    g_nLampVol = 0;

    for(i = 0 ;i < 4 ; i++)
    {
        g_nLampVol |= ((1<<2) | (g_nRedLampVoltageVauleArray[i] << 1) | g_nGreenLampVoltageVauleArray[i]) << i*3;
    }
	return g_nLampVol & 0xfff;
}


/*****************************************************************************
 �� �� ��  : LampControlBoard_GetCur
 ��������  : ͨ��DMA��ȡ����ֵ���������ֵ��ʱû��У�飬���ǽ���Ѿ���׼ȷ��
             ��
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
void LampControlBoard_GetCur(void)
{
    /* Wait for conversion completion before conditional check hereafter */
    HAL_ADC_PollForConversion(&g_AdcHandle, 1);

    g_nCurVauleArray[0] = g_ADCxConvertedValuesArray[0];
    g_nCurVauleArray[1] = g_ADCxConvertedValuesArray[1];
    g_nCurVauleArray[2] = g_ADCxConvertedValuesArray[2];
    g_nCurVauleArray[3] = g_ADCxConvertedValuesArray[3];
}

/*****************************************************************************
 �� �� ��  : Cfg_Uart
 ��������  : ���ڳ�ʼ�����������ò�������9600
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
void Cfg_Uart(void)
{
  g_UartHandle.Instance        = USARTx;

  g_UartHandle.Init.BaudRate   = 9600;
  g_UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  g_UartHandle.Init.StopBits   = UART_STOPBITS_1;
  g_UartHandle.Init.Parity     = UART_PARITY_NONE;
  g_UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  g_UartHandle.Init.Mode       = UART_MODE_TX_RX;
  g_UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT; 
  
  if(HAL_OK != HAL_UART_DeInit(&g_UartHandle))
  {
    Error_Handler();
  }  
  
  if(HAL_OK != HAL_UART_Init(&g_UartHandle))
  {
    Error_Handler();
  }
#ifdef USE_UART_RECV_IT
  if(HAL_UART_Receive_IT(&g_UartHandle, (uint8_t *)gUartBuffer, UARTBUFFSIZE) != HAL_OK)
  {
    Error_Handler();
  }
#endif
}

/*****************************************************************************
 �� �� ��  : Cfg_IWDG
 ��������  : ��ʼ���������Ź������Ź���ι��������2��
 �������  : ��
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
    �޸�����   : ��ι��ʱ���Ϊ1s
*****************************************************************************/
void Cfg_IWDG(void)
{
    g_IwdgHandle.Instance = IWDG;

    g_IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;
    //g_IwdgHandle.Init.Reload    = 2500;//2500 / 1.25 == 2000ms
    g_IwdgHandle.Init.Reload    = 1250;//1250 / 1.25 = 1000ms
    g_IwdgHandle.Init.Window    = IWDG_WINDOW_DISABLE;

    if (HAL_OK != HAL_IWDG_Init(&g_IwdgHandle))
    {
        /* Initialization Error */
        Error_Handler();
    }
}


/*****************************************************************************
 �� �� ��  : Cfg_Can
 ��������  : ��CAN�Ĺ���ģʽ�������ʵȽ�������  //���������ջ����HAL_CAN_M-
             spInit����������ڸú����������GPIO�ڼ���CANʱ�ӡ�
 �������  : ��
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
    �޸�����   : CAN��ʼ��ʧ�ܴ������ӡ�
*****************************************************************************/
void Cfg_Can(void)
{
    static CanTxMsgTypeDef        TxMessage;
    static CanRxMsgTypeDef        RxMessage;
	CAN_FilterConfTypeDef  sFilterConfig;
    uint8_t err_time = 0;//�����������ĳЩ����£�CAN��ʼ����ʧ��

    /*##-1- Configure the CAN peripheral #######################################*/
    g_CanHandle.Instance = CAN;
    g_CanHandle.pTxMsg = &TxMessage;
    g_CanHandle.pRxMsg = &RxMessage;

    g_CanHandle.Init.TTCM = DISABLE;//��ֹʱ�䴥��ͨ��ģʽ
    g_CanHandle.Init.ABOM = DISABLE;//�����CAN_MCR�Ĵ�����INRQλ������1�����0��һ��Ӳ����⵽128��11λ����������λ�����˳�����״̬��
    g_CanHandle.Init.AWUM = DISABLE;//˯��ģʽͨ�����CAN_MCR�Ĵ�����SLEEPλ�����������
    g_CanHandle.Init.NART = DISABLE;//CAN����ֻ������1�Σ����ܷ��͵Ľ����Σ��ɹ���������ٲö�ʧ��
    g_CanHandle.Init.RFLM = DISABLE;//�ڽ������ʱFIFOδ��������������FIFO�ı���δ����������һ���յ��ı��ĻḲ��ԭ�еı���
    g_CanHandle.Init.TXFP = DISABLE;//����FIFO���ȼ��ɱ��ĵı�ʶ��������
    g_CanHandle.Init.Mode = CAN_MODE_NORMAL;//��������ʱ���������
    //g_CanHandle.Init.Mode = CAN_MODE_LOOPBACK;//����ʱ�����������
    g_CanHandle.Init.SJW = CAN_SJW_1TQ;//����ͬ����Ծ���1��ʱ�䵥λ
    g_CanHandle.Init.BS1 = CAN_BS1_13TQ;//ʱ���1Ϊ8��ʱ�䵥λ
    g_CanHandle.Init.BS2 = CAN_BS2_2TQ;//ʱ���2Ϊ7��ʱ�䵥λ
    g_CanHandle.Init.Prescaler = 6;//���������úͺ��İ���ͬ��Ϊ500K,48M/6/(1+13+2);!!!!!!!!!!!!!!!!!!

    while(HAL_OK != HAL_CAN_Init(&g_CanHandle))
    {
        err_time++;
        //HAL_Delay(1);
        if(255 == err_time)
        {
            printf("Can Init Error \r\n");
            return;
        }
    }
    printf("Can Init Ok , Error Time :  %d\r\n",err_time);
    
    /*##-2- Configure the CAN Filter ###########################################*/
    sFilterConfig.FilterNumber = 0;//ָ���˴���ʼ���Ĺ�����0
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;//ָ���˹�����������ʼ������ģʽΪ��ʶ������λģʽ
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;//�����˹�����λ��1��32λ������
    sFilterConfig.FilterIdHigh = 0x0000;//�����趨��������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ����
    sFilterConfig.FilterIdLow = 0x0000;//�����趨��������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
    sFilterConfig.FilterMaskIdHigh = 0x0000;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ��
    sFilterConfig.FilterMaskIdLow = 0x0000;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
    sFilterConfig.FilterFIFOAssignment = CAN_FIFO0;//�趨��ָ���������FIFO0
    sFilterConfig.FilterActivation = ENABLE;//ʹ�ܹ�����		
    sFilterConfig.BankNumber = 14;

    if (HAL_OK != HAL_CAN_ConfigFilter(&g_CanHandle, &sFilterConfig))
    {
        Error_Handler();
    }

}

/*****************************************************************************
 �� �� ��  : Can_Get_MailBox
 ��������  : ��ÿ��е�CAN������
 �������  : CAN_HandleTypeDef* hcan  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t Can_Get_MailBox(const CAN_HandleTypeDef* hcan)
{
    uint8_t transmitmailbox = CAN_TXSTATUS_NOMAILBOX;

    if(CAN_TSR_TME0 == (hcan->Instance->TSR&CAN_TSR_TME0))
    {
      transmitmailbox = 0;
    }
    else if(CAN_TSR_TME1 == (hcan->Instance->TSR&CAN_TSR_TME1))
    {
      transmitmailbox = 1;
    }
    else if(CAN_TSR_TME2 == (hcan->Instance->TSR&CAN_TSR_TME2))
    {
      transmitmailbox = 2;
    }

    return transmitmailbox;
}


/*****************************************************************************
 �� �� ��  : Can_Send_Recv_Data
 ��������  : ������װһ��CAN���ͺͽ��յĺ�������������CAN��Ϣʧ�ܵľ���ԭ
             ��
 �������  : CAN_HandleTypeDef *CanHandle  CAN���
             uint8_t nSendOrRecv           0 ��ʾ������Ϣ, 1 ��ʾ������Ϣ
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
    �޸�����   : ���ӷ���ֵ���ɹ�����1�����󷵻�0

  3.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : CAN���������޸ĺ���ʹ���жϷ�ʽ
*****************************************************************************/
uint8_t Can_Send_Recv_Data(CAN_HandleTypeDef *CanHandle,uint8_t nSendOrRecv)
{
    uint8_t err_time = 0;//�������
    uint8_t err_code = 0;
    
    if(1 == nSendOrRecv)//����CAN��Ϣ�ж�
    {
        while(HAL_OK != (err_code = HAL_CAN_Receive_IT(CanHandle, CAN_FIFO0)))
        {
            err_time++;

            printf("#######Can  Recv Failed ,State  0x%x  Locked  0x%x ,err_code 0x%x\r\n",CanHandle->State,CanHandle->Lock,err_code);
            if(10 == err_time)
            {
                g_IsCanRecvExit = 1;//CAN�Ľ���ע���ж��쳣�˳�
                printf("Can Recv Error \r\n");                
                return 0;
            }
        }
    }
    else if(0 == nSendOrRecv)//����CAN��Ϣ�ж�
    {
		g_CountSend++;
#if 1
		return HAL_CAN_Transmit(CanHandle, 10);
#else
        //while(HAL_OK != (err_code = HAL_CAN_Transmit(CanHandle, 10)))Cube���е����⣬�жϷ���ʱ���ܻ��������CAN�����Ȳ���ֱ�ӷ��ͷ�ʽ
        while(HAL_CAN_Transmit_IT(CanHandle) != HAL_OK)//CAN�����޸ĺ���ʹ���жϷ�ʽ
        {
            err_time++;

            printf("#######Can  Send Failed , State  0x%x  Locked  0x%x transmitmailbox  0x%x err_code 0x%x\r\n",CanHandle->State,CanHandle->Lock,Can_Get_MailBox(CanHandle),err_code);
            if(10 == err_time)
            {
                printf("Can Send Error \r\n");
                return 0;
            }
        }
#endif
    }

    return 1;
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
    memset(g_CanHandle.pTxMsg,0,sizeof(CanTxMsgTypeDef));
	
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
    g_CanHandle.pTxMsg->StdId = 0;
    g_CanHandle.pTxMsg->ExtId = (1 << (16+g_nBaseAddr))|(1 << 15) | (g_nLampVol << 3) | (nTempBaseAddr + 1 );//0-2 ��š�3-14 �̺�ƵƵ�ѹ��15 1
    g_CanHandle.pTxMsg->IDE = CAN_ID_EXT;//��ʾ������Ϊ��չ
    g_CanHandle.pTxMsg->RTR = CAN_RTR_DATA;//��Ϣ֡����Ϊ����֡
    g_CanHandle.pTxMsg->DLC = 4;//֡����Ϊ4

    for(i = 0 ; i < 4; i++)
    {
        ndiff = g_ADCxConvertedValuesArray[i]-gCurValAverArray[i][4];//��ʵʱֵ��ȥ��ֵ��Ϊ����ֵ

        if(ndiff > 255)//��֤CAN��dataҪ��[0,255]��Χ��
        {
            ndiff = 255;
        }

        if(ndiff < 0)
        {
            ndiff = 0;
        }

        g_CanHandle.pTxMsg->Data[i] = ndiff;//�������ʱ�������ֵӦ�ô���10    
		
    }

    Can_Send_Recv_Data(&g_CanHandle,0);
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
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,(0x01 == (g_ArrayLedGroupStatusArray[0]&0x01)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,(0x02 == (g_ArrayLedGroupStatusArray[0]&0x02)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,(0x04 == (g_ArrayLedGroupStatusArray[0]&0x04)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    //group  2
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,(0x01 == (g_ArrayLedGroupStatusArray[1]&0x01)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,(0x02 == (g_ArrayLedGroupStatusArray[1]&0x02)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,(0x04 == (g_ArrayLedGroupStatusArray[1]&0x04)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    //group  3
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,(0x01 == (g_ArrayLedGroupStatusArray[2]&0x01)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,(0x02 == (g_ArrayLedGroupStatusArray[2]&0x02)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,(0x04 == (g_ArrayLedGroupStatusArray[2]&0x04)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    //group  4
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,(0x01 == (g_ArrayLedGroupStatusArray[3]&0x01)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,(0x02 == (g_ArrayLedGroupStatusArray[3]&0x02)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
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
    memset(g_CanHandle.pTxMsg,0,sizeof(CanTxMsgTypeDef));

    uint8_t nTempBaseAddr = g_nBaseAddr;//�жϵ�ǰӦ�÷������ĸ�ID��ֻ����22·��ʱ�����Ҫ���StdId

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
    
    g_CanHandle.pTxMsg->StdId = 0x301 + nTempBaseAddr;//
    g_CanHandle.pTxMsg->ExtId = 0;
    g_CanHandle.pTxMsg->IDE = CAN_ID_STD;//
    g_CanHandle.pTxMsg->RTR = CAN_RTR_DATA;//��Ϣ֡����Ϊ����֡ 
    g_CanHandle.pTxMsg->DLC = 5;//֡����Ϊ5
    g_CanHandle.pTxMsg->Data[0] = g_nOutPutMode;
    g_CanHandle.pTxMsg->Data[1] = g_ArrayLedGroupStatusArray[0]&0x07;
    g_CanHandle.pTxMsg->Data[2] = g_ArrayLedGroupStatusArray[1]&0x07;
    g_CanHandle.pTxMsg->Data[3] = g_ArrayLedGroupStatusArray[2]&0x07;
    g_CanHandle.pTxMsg->Data[4] = g_ArrayLedGroupStatusArray[3]&0x07;

    Can_Send_Recv_Data(&g_CanHandle,0);
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
		HAL_IWDG_Refresh(&g_IwdgHandle);
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
void DecodeCanMsg(const CAN_HandleTypeDef *CanHandle)
{
    uint8_t i = 0;
    uint8_t j = 0;

    uint16_t nCurBoardVal = 0;//��ǰ�ƿذ��12λ��״̬�֣�ÿ3λһ��ͨ�����ֱ��ʾ�̺�Ƶ�״̬;��ȡCAN��Ϣ��12λ��״̬����������
    uint8_t nTempBaseAddr = g_nBaseAddr;//��ǰ����Ӧ�ö�ȡCAN��Ϣ�ĵڼ���12λ
    
    g_nOutPutMode = CanHandle->pRxMsg->Data[0];//�����22λ�Ļ�����Data[1]-[3]�ֱ��ʾ����������ϱ�����ġ�

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
    
    nCurBoardVal |= CanHandle->pRxMsg->Data[i]>>((nTempBaseAddr*12)%8);//ȡ��λ
    nCurBoardVal |= (uint16_t)(CanHandle->pRxMsg->Data[i+1])<<j;//ȡ��λ        

    //��ȫ�ֱ�����ֵ
    for(i = 0 ; i < 4; i++)
    {
        g_ArrayLedGroupStatusArray[i] = (nCurBoardVal>>(3*i))&0x07 ;
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
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *CanHandle)
{  //���յ����Ժ��İ��CAN��Ϣ��Ҫ����CAN���ݣ��õ��������Ƶ�״̬������gArrayLedGroupStatus���鸳ֵ��
    if((7 == CanHandle->pRxMsg->DLC)&&(0x101 == CanHandle->pRxMsg->StdId))
    {
		g_heart_count1++;
        HAL_IWDG_Refresh(&g_IwdgHandle);
			
		//�������ݲ����
		DecodeCanMsg(CanHandle);
		SetLedGroupStatus();
		CanSendLampStatus();//��ǰ��巢�͵����Ϣ

		//��������ѹֵ������
		HAL_ADC_Start(&g_AdcHandle);
		LampControlBoard_GetCur();//�õ���������
		LampControlBoard_GetVoltage();//ͨ��GPIO��ȡ��ѹ
		GetCurAverVal();//��ʱ���µ�����ֵ
		CanSendCurVolData();//�����ذ巢�͵�����ѹֵ
    	//g_interrupted = 1;   //���ڿ��ƻ���

    	//print debug info
    	//printf("==>  nBaseAddr : %d  g_ArrayLedGroupStatusArray: %d  %d  %d  %d\r\n",g_nBaseAddr,g_ArrayLedGroupStatusArray[0],g_ArrayLedGroupStatusArray[1],
    	  //                                                                      g_ArrayLedGroupStatusArray[2],g_ArrayLedGroupStatusArray[3]);
        if(2 == g_CountRecved++)
        {
            HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_4);
            g_CountRecved = 0;
        }

        if(3 <= g_CountSend)
        {
            g_CountSend = 0;
            HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
        }
    }
#ifdef EXCUTE_FIXED_CYCLE
	else if ((CanHandle->pRxMsg->ExtId & LCB_CONFIG_MASK) == LCB_CONFIG_ID
			&& CanHandle->pRxMsg->DLC == 8)
	{
		//printf("recv can msg, stdid: %#x, DLC: %d\r\n", CanHandle->pRxMsg->StdId, CanHandle->pRxMsg->DLC);
		LCBconfigDeal(CanHandle->pRxMsg);
		HAL_IWDG_Refresh(&g_IwdgHandle);
	}
	else if (GET_BIT(CanHandle->pRxMsg->ExtId, 15) == 1 && CanHandle->pRxMsg->DLC == 4) 
	{	//��15λΪ1���ҳ���Ϊ4��can��Ϣ��ʾ�ǵƿذ��������ѹ����
		ConvertToLightData(CanHandle->pRxMsg);	//�ѻ�ȡ�ĵ�����ѹת����ʵ�ʵĵ������
	}
#endif
    Can_Send_Recv_Data(CanHandle,1);//���չٷ����ӵ�����������󣬼����жϽ��ա�
}

/*****************************************************************************
 �� �� ��  : HAL_CAN_TxCpltCallback
 ��������  : CAN������ɻص�����
 �������  : CAN_HandleTypeDef* hcan  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : ��CAN���ͳɹ�����ι��
*****************************************************************************/
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
    HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
	//printf("HAL_CAN_TxCpltCallback called!\n");
    HAL_IWDG_Refresh(&g_IwdgHandle);
}

/*****************************************************************************
 �� �� ��  : HAL_CAN_ErrorCallback
 ��������  : CANͨѶ���ϻص�����
 �������  : CAN_HandleTypeDef *hcan  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 20115��1��6��
    ��    ��   : xiaowh
    �޸�����   : ��CAN�����������û�н������ǰ��ֱ������MCU����
*****************************************************************************/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	printf("Can Communication Error , Error Code is : 0x%x  , State Code is : 0x%x\r\n",hcan->ErrorCode,hcan->State);
    printf("Software will restart ....\r\n");
    //HAL_Delay(1200);//��ʱ1.2sʹwatchdog ��Ч�����������
    NVIC_SystemReset();//����ϵͳ
}


/*****************************************************************************
 �� �� ��  : GetBaseAddr
 ��������  : ����PB3 PB4��ֵ����õ�ǰ�ƿذ�ĵ�ַ(���)
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��ǰ�ƿذ�ĵ�ַ����0��ʼ��0,1,2,3��
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t GetBaseAddr(void)
{
    uint8_t tmp = 0;//ÿ�������ڲ�ͬ�Ĳ�λ����Ӧ�Ų�ͬ�ĵ�ַ���ֱ�Ϊ0 1 2 3
    
    tmp = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);

    tmp *= 2;
    tmp += HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4);

    return tmp;
}


/*****************************************************************************
 �� �� ��  : InitGPIOA
 ��������  : ��ʼ��GPIOA��ص�����
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
void InitGPIOA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;//GPIO_PULLDOWN;   
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  //����ָʾ�Ƶĳ�ʼ��

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);//��Դָʾ����
}

/*****************************************************************************
 �� �� ��  : InitGPIOB
 ��������  : ��ʼ��GPIOB��ص�����
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
void InitGPIOB(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;//GPIO_PULLDOWN;   
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);//��ƿ��ƹܽ�

    //������ֻ������������ȡ���̵�ѹֵ�Ƿ���ȷ
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;//
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    

}

/*****************************************************************************
 �� �� ��  : InitGPIOC
 ��������  : ��ʼ��GPIOC��ص�����
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
void InitGPIOC(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull  = GPIO_PULLDOWN;//;   
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    //������ֻ������������ȡ���̵�ѹֵ�Ƿ���ȷ
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;//
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 

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
    //return ch;
	while((USART1->ISR&0X40)==0);//ѭ������,ֱ���������   
        USART1->TDR = (uint8_t) ch;      

	return ch;
}

/*****************************************************************************
 �� �� ��  : TestAllLedLight
 ��������  : �����ú�������������LED��
 �������  : val 1,����,0Ϩ��
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
    �޸�����   : ��Ӳ��������ݲ�������������Ϩ��ơ�
*****************************************************************************/
void TestAllLedLight(uint8_t val)
{
    //���
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

    //�̵�
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

    //�Ƶ�
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

    //CAN�շ�
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    //���е�
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

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
void GetCurAverVal()
{
    uint8_t i = 0;
    uint32_t tempTotalVal = 0;
    uint8_t tempOffOn = 0;//��ʱ�������жϵ�ǰ��Ƶ�����״̬��ֻ����Ƶ�ʱ��Ŷ�����ֵ

    for(i = 0; i < 4; i++)
    {
        if(0 == i)//��һ����
        {
            tempOffOn = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_4);
        }
        else if(1 == i)
        {
            tempOffOn = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_5);
        }
        else if(2 == i)
        {
            tempOffOn = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
        }
        else
        {
            tempOffOn = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
        }

        if(1 == tempOffOn)//������ʱ��ֱ��continue��
        {
            continue;
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
	uint8_t heart = 0;		//��������
    uint8_t error_time = 0;
#ifdef EXCUTE_FIXED_CYCLE
	uint8_t initYellowFlashSec = 20; //��ʼ��ʱ����20s��ֹ���ذ廹δ���ʱ�ƿذ���������
	uint8_t initAllRedSec = 6; //��ʼ��ʱ����������δ�յ����ذ���Ϣ��ȫ��6s
	LCBruninfo runinfo;
	uint8_t cycleTime = 0;
	RunData rundata[MAX_RUNDATA_NUM];
#endif
    HAL_Init();//ʹ��CUBE��������Ƚ���HAL��ʼ������
    SystemClock_Config_HSE();//����ϵͳʱ��Ϊ48 MHz

    if (0 != __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        HAL_Delay(1000);//modified by xiaowh
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    else
    {
        HAL_Delay(1000);//��1s��ʱ����֤ʱ���ȶ�
    }
    
    InitGPIOA();
    InitGPIOB();
    InitGPIOC();
    Cfg_Uart();
    Cfg_Can();
    Cfg_ADC();
    Cfg_IWDG();
#ifdef EXCUTE_FIXED_CYCLE
	LCBconfigInit();
#endif
    
    g_nBaseAddr = GetBaseAddr();     
    if(HAL_OK != HAL_IWDG_Start(&g_IwdgHandle))// Start the IWDG
    {
        Error_Handler();
    }

	Can_Send_Recv_Data(&g_CanHandle,1);//CAN��Ϣ�жϽ���
    printf("==> System Restart ... \r\n");
    while(1)
    {
        g_nBaseAddr = GetBaseAddr();
        if(1 == g_IsCanRecvExit)//CAN�Ľ����쳣��������������
        {
             g_IsCanRecvExit = 0;
             if(1 == Can_Send_Recv_Data(&g_CanHandle,1))//CAN��Ϣ�жϽ���
             {
                error_time = 0;
             }
             else
             {
                error_time++;
             }

             if(2 == error_time)
             {
                error_time = 0;
                printf("CAN Recv Process Restart Failed , We Will Reset MCU \r\n");
                NVIC_SystemReset();//����ϵͳ
             }
             
             printf("Restart CAN Recv Process ... \r\n");
        }

		if (g_heart_count1 == g_heart_count2)
		{	
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
				HAL_IWDG_Refresh(&g_IwdgHandle);
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
					HAL_IWDG_Refresh(&g_IwdgHandle);
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
			HAL_Delay(DELAY_MSEC);
			HAL_IWDG_Refresh(&g_IwdgHandle);
		}
        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_6);
    }

    return 0;
}

