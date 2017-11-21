/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月9日
  最近修改   :
  功能描述   : 这是灯控板MCU程序，主要功能是接收核心板红绿灯设置命令，点亮路
               口红绿灯，同时需要检测电压及电流
  函数列表   :
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
  修改历史   :
  1.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 创建文件

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 针对第二版板子做相应的改动
******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
void GetCurAverVal();

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
CAN_HandleTypeDef    g_CanHandle;
ADC_HandleTypeDef    g_AdcHandle;
UART_HandleTypeDef g_UartHandle;
IWDG_HandleTypeDef g_IwdgHandle;
volatile uint16_t   g_ADCxConvertedValuesArray[ADCCONVERTEDVALUES_BUFFER_SIZE];

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static uint16_t g_nCurVauleArray[4] = {0};//电流检测结果存放在该数组中
static uint8_t g_nGreenLampVoltageVauleArray[4] = {0};//四组绿灯电压值
static uint8_t g_nRedLampVoltageVauleArray[4] = {0};//四组红灯电压值
static uint16_t g_nLampVol = 0;//只需要12位，从低到高分别为第N组的绿黄红电压。
static uint8_t g_nOutPutMode = 1;//输出模式，1:44路独立输出，2:22路独立输出 
static uint8_t g_ArrayLedGroupStatusArray[4] = {0};//四组灯的状态，均为低3位有效，分别表示绿灯、红灯、黄灯的状态，1亮0灭
static uint8_t g_nBaseAddr;//灯控板基地址，0、1、2、3  从PB3 PB4获取
static uint8_t  g_heart_count1 = 0;//用来监测有无收到来自主控板的CAN消息
static uint8_t  g_heart_count2 = 0;
//static uint8_t g_interrupted = 0;
static uint32_t g_CountRecved = 0;//仅仅是用来控制闪亮的频率
static uint32_t g_CountSend = 0;//仅仅是用来控制闪亮的频率
static uint8_t g_IsCanRecvExit = 0;//判断CAN的接收中断是否异常终止，若终止了需在main函数里手动重新启动CAN的接收。


uint32_t gCurValAverArray[4][5];//4组红灯电流的实时值及基准值，前4位为实时值，最后1位位基准值。
uint8_t gCurValIndexArray[4] = {0};//4组红灯电流求评价值是当前数组的更新顺序
uint8_t gIsCurCalFirstTime[4] = {0};//4组红灯电流的计算是否是第一次填满数组

#define DELAY_MSEC	250
#define ONE_SECOND_HEARTS	(1000 / DELAY_MSEC)	//1s钟心跳计数
//超时心跳数,如果心跳数超过这个值还未接收到can信息则灯控板各自执行黄闪控制
#define TIMEOUT_HEARTS	(3 * ONE_SECOND_HEARTS)
#define EXCUTE_FIXED_CYCLE
#define USE_UART_RECV_IT

#ifdef EXCUTE_FIXED_CYCLE
#include "fixedcycle.h"		//所有的函数实现以及结构体和全局变量的定义全在这里面
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
		case 0x1111ffff: NVIC_SystemReset(); break; //重启系统
		case 0xffff1111:	//擦除配置,恢复默认
			WriteFlashData(LCB_CONFIG_ADDR, &gDefaultConfig, sizeof(LCBconfig));
			gLCBconfig->configstate = CONFIG_UPDATE;
			printf("restore default config!\r\n");
			break;
		case 0x11112222:	//打印配置
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

  //开启时钟安全系统
  HAL_RCC_EnableCSS();
}


/*****************************************************************************
 函 数 名  : SystemClock_Config
 功能描述  : 系统时钟配置函数
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 

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
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : Error_Handler
 功能描述  : 出错处理函数
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : SleepNoEnd
 功能描述  : 测试用函数，死循环，防止程序终止
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : Cfg_ADC
 功能描述  : 灯控板AD参数设置
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2015年2月27日
    作    者   : 肖文虎
    修改内容   : 将AD转换修改为不连续方式

  3.日    期   : 2015年2月27日
    作    者   : 肖文虎
    修改内容   : 整合代码，将启动AD转换的代码添加进来
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

    if(HAL_OK != HAL_ADCEx_Calibration_Start(&g_AdcHandle))//校准
    {
        Error_Handler();
    }
    
    if(HAL_OK != HAL_ADC_Start_DMA(&g_AdcHandle, (uint32_t *)g_ADCxConvertedValuesArray, ADCCONVERTEDVALUES_BUFFER_SIZE))//启动扫描
    {
        Error_Handler();
    }


}

/*****************************************************************************
 函 数 名  : LampControlBoard_GetVoltage
 功能描述  : 读取红绿灯电压值
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
uint16_t LampControlBoard_GetVoltage(void)
{
    int8_t i = 0;

    //读取红灯电压值
    g_nRedLampVoltageVauleArray[0] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[1] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[2] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[3] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12) == 0 ? 1 : 0);

    //读取绿灯电压值
    g_nGreenLampVoltageVauleArray[0] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[1] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[2] = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[3] = (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_6) == 0 ? 1 : 0);

    //将四组电流电压值重新组织成12位变量
    g_nLampVol = 0;

    for(i = 0 ;i < 4 ; i++)
    {
        g_nLampVol |= ((1<<2) | (g_nRedLampVoltageVauleArray[i] << 1) | g_nGreenLampVoltageVauleArray[i]) << i*3;
    }
	return g_nLampVol & 0xfff;
}


/*****************************************************************************
 函 数 名  : LampControlBoard_GetCur
 功能描述  : 通过DMA读取电流值，这个电流值暂时没有校验，但是结果已经很准确了
             。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : Cfg_Uart
 功能描述  : 串口初始化函数，设置波特率是9600
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : Cfg_IWDG
 功能描述  : 初始化独立看门狗，看门狗的喂狗周期是2秒
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 2015年1月6日
    作    者   : xiaowh
    修改内容   : 将喂狗时间改为1s
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
 函 数 名  : Cfg_Can
 功能描述  : 对CAN的工作模式、波特率等进行配置  //这里面最终会调用HAL_CAN_M-
             spInit这个函数，在该函数里会设置GPIO口及开CAN时钟。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : CAN初始化失败次数增加。
*****************************************************************************/
void Cfg_Can(void)
{
    static CanTxMsgTypeDef        TxMessage;
    static CanRxMsgTypeDef        RxMessage;
	CAN_FilterConfTypeDef  sFilterConfig;
    uint8_t err_time = 0;//错误计数，在某些情况下，CAN初始化会失败

    /*##-1- Configure the CAN peripheral #######################################*/
    g_CanHandle.Instance = CAN;
    g_CanHandle.pTxMsg = &TxMessage;
    g_CanHandle.pRxMsg = &RxMessage;

    g_CanHandle.Init.TTCM = DISABLE;//禁止时间触发通信模式
    g_CanHandle.Init.ABOM = DISABLE;//软件对CAN_MCR寄存器的INRQ位进行置1随后清0后，一旦硬件检测到128次11位连续的隐性位，就退出离线状态。
    g_CanHandle.Init.AWUM = DISABLE;//睡眠模式通过清除CAN_MCR寄存器的SLEEP位，由软件唤醒
    g_CanHandle.Init.NART = DISABLE;//CAN报文只被发送1次，不管发送的结果如何（成功、出错或仲裁丢失）
    g_CanHandle.Init.RFLM = DISABLE;//在接收溢出时FIFO未被锁定，当接收FIFO的报文未被读出，下一个收到的报文会覆盖原有的报文
    g_CanHandle.Init.TXFP = DISABLE;//发送FIFO优先级由报文的标识符来决定
    g_CanHandle.Init.Mode = CAN_MODE_NORMAL;//正常工作时，开启这个
    //g_CanHandle.Init.Mode = CAN_MODE_LOOPBACK;//测试时单机可用这个
    g_CanHandle.Init.SJW = CAN_SJW_1TQ;//重新同步跳跃宽度1个时间单位
    g_CanHandle.Init.BS1 = CAN_BS1_13TQ;//时间段1为8个时间单位
    g_CanHandle.Init.BS2 = CAN_BS2_2TQ;//时间段2为7个时间单位
    g_CanHandle.Init.Prescaler = 6;//波特率设置和核心板相同，为500K,48M/6/(1+13+2);!!!!!!!!!!!!!!!!!!

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
    sFilterConfig.FilterNumber = 0;//指定了待初始化的过滤器0
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;//指定了过滤器将被初始化到的模式为标识符屏蔽位模式
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;//给出了过滤器位宽1个32位过滤器
    sFilterConfig.FilterIdHigh = 0x0000;//用来设定过滤器标识符（32位位宽时为其高段位，16位位宽时为第一个）
    sFilterConfig.FilterIdLow = 0x0000;//用来设定过滤器标识符（32位位宽时为其低段位，16位位宽时为第二个
    sFilterConfig.FilterMaskIdHigh = 0x0000;//用来设定过滤器屏蔽标识符或者过滤器标识符（32位位宽时为其高段位，16位位宽时为第一个
    sFilterConfig.FilterMaskIdLow = 0x0000;//用来设定过滤器屏蔽标识符或者过滤器标识符（32位位宽时为其低段位，16位位宽时为第二个
    sFilterConfig.FilterFIFOAssignment = CAN_FIFO0;//设定了指向过滤器的FIFO0
    sFilterConfig.FilterActivation = ENABLE;//使能过滤器		
    sFilterConfig.BankNumber = 14;

    if (HAL_OK != HAL_CAN_ConfigFilter(&g_CanHandle, &sFilterConfig))
    {
        Error_Handler();
    }

}

/*****************************************************************************
 函 数 名  : Can_Get_MailBox
 功能描述  : 获得空闲的CAN发件箱
 输入参数  : CAN_HandleTypeDef* hcan  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : Can_Send_Recv_Data
 功能描述  : 单独封装一个CAN发送和接收的函数，用来跟踪CAN消息失败的具体原
             因
 输入参数  : CAN_HandleTypeDef *CanHandle  CAN句柄
             uint8_t nSendOrRecv           0 表示发送消息, 1 表示接收消息
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 增加返回值，成功返回1，错误返回0

  3.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : CAN驱动程序修改后，仍使用中断方式
*****************************************************************************/
uint8_t Can_Send_Recv_Data(CAN_HandleTypeDef *CanHandle,uint8_t nSendOrRecv)
{
    uint8_t err_time = 0;//错误计数
    uint8_t err_code = 0;
    
    if(1 == nSendOrRecv)//接收CAN消息中断
    {
        while(HAL_OK != (err_code = HAL_CAN_Receive_IT(CanHandle, CAN_FIFO0)))
        {
            err_time++;

            printf("#######Can  Recv Failed ,State  0x%x  Locked  0x%x ,err_code 0x%x\r\n",CanHandle->State,CanHandle->Lock,err_code);
            if(10 == err_time)
            {
                g_IsCanRecvExit = 1;//CAN的接收注册中断异常退出
                printf("Can Recv Error \r\n");                
                return 0;
            }
        }
    }
    else if(0 == nSendOrRecv)//发送CAN消息中断
    {
		g_CountSend++;
#if 1
		return HAL_CAN_Transmit(CanHandle, 10);
#else
        //while(HAL_OK != (err_code = HAL_CAN_Transmit(CanHandle, 10)))Cube库有点问题，中断发送时可能会造成锁死CAN，故先采用直接发送方式
        while(HAL_CAN_Transmit_IT(CanHandle) != HAL_OK)//CAN驱动修改后，仍使用中断方式
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
 函 数 名  : CanSendCurVolData
 功能描述  : 灯控板向主控板发送电流电压检测数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void CanSendCurVolData(void)
{
    uint8_t i = 0;
    int16_t ndiff = 0;
    uint8_t nTempBaseAddr = g_nBaseAddr;  //当前板子应该读取CAN消息的第几个12位
    memset(g_CanHandle.pTxMsg,0,sizeof(CanTxMsgTypeDef));
	
    if(2 == g_nOutPutMode)//22路输出时，0、2两块板取Data的前12位，1、3两块板取紧接着的12位
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
    g_CanHandle.pTxMsg->ExtId = (1 << (16+g_nBaseAddr))|(1 << 15) | (g_nLampVol << 3) | (nTempBaseAddr + 1 );//0-2 板号、3-14 绿红黄灯电压、15 1
    g_CanHandle.pTxMsg->IDE = CAN_ID_EXT;//标示符类型为扩展
    g_CanHandle.pTxMsg->RTR = CAN_RTR_DATA;//消息帧类型为数据帧
    g_CanHandle.pTxMsg->DLC = 4;//帧长度为4

    for(i = 0 ; i < 4; i++)
    {
        ndiff = g_ADCxConvertedValuesArray[i]-gCurValAverArray[i][4];//拿实时值减去基值作为电流值

        if(ndiff > 255)//保证CAN的data要在[0,255]范围内
        {
            ndiff = 255;
        }

        if(ndiff < 0)
        {
            ndiff = 0;
        }

        g_CanHandle.pTxMsg->Data[i] = ndiff;//红灯亮起时，其电流值应该大于10    
		
    }

    Can_Send_Recv_Data(&g_CanHandle,0);
}

/*****************************************************************************
 函 数 名  : SetLedGroupStatus
 功能描述  : 灯控板根据核心板的指令，按照gArrayLedGroupStatus的值依次点亮各-
             LED灯
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : CanSendLampStatus
 功能描述  : 灯控板向前面板发送消息，驱动虚拟路口LED阵列
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void CanSendLampStatus(void)
{
    memset(g_CanHandle.pTxMsg,0,sizeof(CanTxMsgTypeDef));

    uint8_t nTempBaseAddr = g_nBaseAddr;//判断当前应该发的是哪个ID，只有是22路的时候才需要变更StdId

    if(2 == g_nOutPutMode)//22路输出时，第2块板子的发送信息ID和第0块板子相同；第3块板子和第1块板子ID相同。
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
    g_CanHandle.pTxMsg->RTR = CAN_RTR_DATA;//消息帧类型为数据帧 
    g_CanHandle.pTxMsg->DLC = 5;//帧长度为5
    g_CanHandle.pTxMsg->Data[0] = g_nOutPutMode;
    g_CanHandle.pTxMsg->Data[1] = g_ArrayLedGroupStatusArray[0]&0x07;
    g_CanHandle.pTxMsg->Data[2] = g_ArrayLedGroupStatusArray[1]&0x07;
    g_CanHandle.pTxMsg->Data[3] = g_ArrayLedGroupStatusArray[2]&0x07;
    g_CanHandle.pTxMsg->Data[4] = g_ArrayLedGroupStatusArray[3]&0x07;

    Can_Send_Recv_Data(&g_CanHandle,0);
}

/*****************************************************************************
 函 数 名  : CanSendYellowBlink
 功能描述  : 当灯控板没有收到来自核心板的点灯信息时，就发送黄闪信息给前面板
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
		SetLedGroupStatus();//把自己的灯控板灯点亮
		CanSendLampStatus();//再将点灯信息发送到前面板，将前面板对应的LED阵列点亮
		
		if (g_heart_count1 != g_heart_count2)
			return;
		HAL_IWDG_Refresh(&g_IwdgHandle);
		HAL_Delay(DELAY_MSEC);
	}
}

/*****************************************************************************
 函 数 名  : DecodeCanMsg
 功能描述  : 灯控板解析来自核心板的消息
 输入参数  : CAN_HandleTypeDef *CanHandle  CAN句柄
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void DecodeCanMsg(const CAN_HandleTypeDef *CanHandle)
{
    uint8_t i = 0;
    uint8_t j = 0;

    uint16_t nCurBoardVal = 0;//当前灯控板的12位灯状态字，每3位一个通道，分别表示绿红黄的状态;获取CAN消息的12位灯状态字再逐个点灯
    uint8_t nTempBaseAddr = g_nBaseAddr;//当前板子应该读取CAN消息的第几个12位
    
    g_nOutPutMode = CanHandle->pRxMsg->Data[0];//如果是22位的话，则Data[1]-[3]分别表示东西方向和南北方向的。

    if(2 == g_nOutPutMode)//22路输出时，0、2两块板取Data的前12位，1、3两块板取紧接着的12位
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

    i = nTempBaseAddr*12/8 + 1;//低位的起始
    j += 8 - ((nTempBaseAddr*12)%8);//高位的起始
    
    nCurBoardVal |= CanHandle->pRxMsg->Data[i]>>((nTempBaseAddr*12)%8);//取低位
    nCurBoardVal |= (uint16_t)(CanHandle->pRxMsg->Data[i+1])<<j;//取高位        

    //给全局变量赋值
    for(i = 0 ; i < 4; i++)
    {
        g_ArrayLedGroupStatusArray[i] = (nCurBoardVal>>(3*i))&0x07 ;
    }

    
}

/*****************************************************************************
 函 数 名  : HAL_CAN_RxCpltCallback
 功能描述  : CAN接收完成回调函数
 输入参数  : CAN_HandleTypeDef *CanHandle  CAN句柄
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 2015年1月6日
    作    者   : xiaowh
    修改内容   : 只有接收到来自核心板的点灯信息才喂狗，更改接收指示灯的闪亮频率
*****************************************************************************/
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *CanHandle)
{  //接收到来自核心板的CAN消息后，要解析CAN数据，得到本板各组灯的状态，并给gArrayLedGroupStatus数组赋值。
    if((7 == CanHandle->pRxMsg->DLC)&&(0x101 == CanHandle->pRxMsg->StdId))
    {
		g_heart_count1++;
        HAL_IWDG_Refresh(&g_IwdgHandle);
			
		//解析数据并点灯
		DecodeCanMsg(CanHandle);
		SetLedGroupStatus();
		CanSendLampStatus();//给前面板发送点灯信息

		//监测电流电压值并发送
		HAL_ADC_Start(&g_AdcHandle);
		LampControlBoard_GetCur();//得到电流参数
		LampControlBoard_GetVoltage();//通过GPIO读取电压
		GetCurAverVal();//定时更新电流基值
		CanSendCurVolData();//给主控板发送电流电压值
    	//g_interrupted = 1;   //用于控制黄闪

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
	{	//第15位为1并且长度为4的can消息表示是灯控板电流及电压反馈
		ConvertToLightData(CanHandle->pRxMsg);	//把获取的电流电压转换成实际的点灯数据
	}
#endif
    Can_Send_Recv_Data(CanHandle,1);//按照官方例子的做法，读完后，继续中断接收。
}

/*****************************************************************************
 函 数 名  : HAL_CAN_TxCpltCallback
 功能描述  : CAN发送完成回调函数
 输入参数  : CAN_HandleTypeDef* hcan  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 在CAN发送成功后不再喂狗
*****************************************************************************/
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
    HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
	//printf("HAL_CAN_TxCpltCallback called!\n");
    HAL_IWDG_Refresh(&g_IwdgHandle);
}

/*****************************************************************************
 函 数 名  : HAL_CAN_ErrorCallback
 功能描述  : CAN通讯故障回调函数
 输入参数  : CAN_HandleTypeDef *hcan  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 20115年1月6日
    作    者   : xiaowh
    修改内容   : 在CAN发生错误后，在没有解决问题前，直接重启MCU程序
*****************************************************************************/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	printf("Can Communication Error , Error Code is : 0x%x  , State Code is : 0x%x\r\n",hcan->ErrorCode,hcan->State);
    printf("Software will restart ....\r\n");
    //HAL_Delay(1200);//延时1.2s使watchdog 生效，软件重启。
    NVIC_SystemReset();//重启系统
}


/*****************************************************************************
 函 数 名  : GetBaseAddr
 功能描述  : 根据PB3 PB4的值，获得当前灯控板的地址(序号)
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 当前灯控板的地址，从0开始，0,1,2,3，
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t GetBaseAddr(void)
{
    uint8_t tmp = 0;//每个板子在不同的槽位都对应着不同的地址，分别为0 1 2 3
    
    tmp = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);

    tmp *= 2;
    tmp += HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4);

    return tmp;
}


/*****************************************************************************
 函 数 名  : InitGPIOA
 功能描述  : 初始化GPIOA相关的引脚
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitGPIOA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;//GPIO_PULLDOWN;   
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  //各种指示灯的初始化

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);//电源指示灯亮
}

/*****************************************************************************
 函 数 名  : InitGPIOB
 功能描述  : 初始化GPIOB相关的引脚
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitGPIOB(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;//GPIO_PULLDOWN;   
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);//红灯控制管脚

    //以下是只读引脚用来读取红绿电压值是否正确
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;//
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    

}

/*****************************************************************************
 函 数 名  : InitGPIOC
 功能描述  : 初始化GPIOC相关的引脚
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

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

    //以下是只读引脚用来读取红绿电压值是否正确
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;//
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 

}

/*****************************************************************************
 函 数 名  : fputc
 功能描述  : 重定义fputc函数 ,这样就可以使用printfL
 输入参数  : int ch   
             FILE *f  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 正式版去掉打印信息。
*****************************************************************************/
int fputc(int ch, FILE *f)
{   
    //return ch;
	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕   
        USART1->TDR = (uint8_t) ch;      

	return ch;
}

/*****************************************************************************
 函 数 名  : TestAllLedLight
 功能描述  : 测试用函数，点亮所有LED灯
 输入参数  : val 1,点亮,0熄灭
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 添加参数，根据参数决定点亮或熄灭灯。
*****************************************************************************/
void TestAllLedLight(uint8_t val)
{
    //红灯
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

    //绿灯
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

    //黄灯
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

    //CAN收发
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    //运行灯
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,(val == 1)? GPIO_PIN_SET : GPIO_PIN_RESET);

}

/*****************************************************************************
 函 数 名  : GetCurAverVal
 功能描述  : 得到电流基值: 取连续4次红灯灭灯情况下的电流AD值的均值作为基值
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月12日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void GetCurAverVal()
{
    uint8_t i = 0;
    uint32_t tempTotalVal = 0;
    uint8_t tempOffOn = 0;//临时变量，判断当前红灯的亮灭状态，只有灭灯的时候才读电流值

    for(i = 0; i < 4; i++)
    {
        if(0 == i)//第一组红灯
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

        if(1 == tempOffOn)//灯亮的时候，直接continue掉
        {
            continue;
        }

        gCurValAverArray[i][gCurValIndexArray[i]] = g_ADCxConvertedValuesArray[i];
        gCurValIndexArray[i]++;

        if(gCurValIndexArray[i] == 4)
        {
            gCurValIndexArray[i] = 0;//下次读取的值放到数组第一位
            gIsCurCalFirstTime[i] = 1;
        }
        
        if(gIsCurCalFirstTime[i] == 1)//第一次必须等填满数组后再计算，后面的就只需要每次过来就计算
        {
            tempTotalVal = gCurValAverArray[i][0] + gCurValAverArray[i][1] + gCurValAverArray[i][2] + gCurValAverArray[i][3];
            gCurValAverArray[i][4] = (tempTotalVal >> 2);//右移两位相当于除以4
        }
    }
}

/*****************************************************************************
 函 数 名  : main
 功能描述  : 主函数
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 运行指示灯改为PA6

  3.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 添加临时调试信息，打开后，将所有LED灯点亮。
  4.日    期   : 2015年1月6日
    作    者   : 肖文虎
    修改内容   : 减少启动延时时间,在看门狗启动后再开启CAN接收中断,主循环里不再提供喂狗及黄闪功能	
*****************************************************************************/
int main(void)
{
	uint8_t heart = 0;		//心跳计数
    uint8_t error_time = 0;
#ifdef EXCUTE_FIXED_CYCLE
	uint8_t initYellowFlashSec = 20; //初始化时黄闪20s防止主控板还未点灯时灯控板自主运行
	uint8_t initAllRedSec = 6; //初始化时黄闪结束后还未收到主控板消息则全红6s
	LCBruninfo runinfo;
	uint8_t cycleTime = 0;
	RunData rundata[MAX_RUNDATA_NUM];
#endif
    HAL_Init();//使用CUBE库必须首先进行HAL初始化操作
    SystemClock_Config_HSE();//配置系统时钟为48 MHz

    if (0 != __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        HAL_Delay(1000);//modified by xiaowh
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    else
    {
        HAL_Delay(1000);//加1s延时，保证时钟稳定
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

	Can_Send_Recv_Data(&g_CanHandle,1);//CAN消息中断接收
    printf("==> System Restart ... \r\n");
    while(1)
    {
        g_nBaseAddr = GetBaseAddr();
        if(1 == g_IsCanRecvExit)//CAN的接收异常后，重新启动接收
        {
             g_IsCanRecvExit = 0;
             if(1 == Can_Send_Recv_Data(&g_CanHandle,1))//CAN消息中断接收
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
                NVIC_SystemReset();//重启系统
             }
             
             printf("Restart CAN Recv Process ... \r\n");
        }

		if (g_heart_count1 == g_heart_count2)
		{	
			heart++;
#ifdef EXCUTE_FIXED_CYCLE
			if (initYellowFlashSec > 0)
			{	//刚上电时每个灯控板各自执行黄闪等待主控板发送点灯消息
				CanSendYellowBlink();
				initYellowFlashSec--;
				continue;
			}
			if (initAllRedSec > 0)
			{	//如果黄闪结束后还未收到主控板消息则灯控板自主接管执行全红
				SpecialControlLight(LRED);				//全红控制
				initAllRedSec--;
				heart = ONE_SECOND_HEARTS - 1;
				LightFrontBoardButtonLed(ALL_BUTTON_LED);
				gRuninfo.schemeid = 1;	//默认一上来接管执行方案1
				continue;
			}
			else if (gFaultFlag == 1)
			{
				SpecialControlLight(LYELLOW_FLASH);		//黄闪控制
				continue;
			}
			if (heart <= ONE_SECOND_HEARTS)
			{
				if (gLCBconfig->configstate == CONFIG_UPDATE)
					ReadFlashData(LCB_CONFIG_ADDR, gLCBconfig, sizeof(LCBconfig));
				if (heart > ONE_SECOND_HEARTS / 2)//点亮前面板全部按键的LED表明主控板can通信异常
					LightFrontBoardButtonLed(ALL_BUTTON_LED);
				if (heart == ONE_SECOND_HEARTS)	
				{	
					runinfo = gRuninfo;
					if (runinfo.schemeid == 254)
						runinfo.schemeid = 0;	//感应控制为方案0
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
			{	//如果3s内还没接收到数据则说明接管失败，灯控板各自执行黄闪
				printf("isTakeOver:%d, baseaddr:%d, controlBoardNo:%d, runtime:%d, schemeid:%d\r\n", 
					gLCBconfig->baseinfo.isTakeOver, g_nBaseAddr, gLCBconfig->controlBoardNo, 
					runinfo.runtime, runinfo.schemeid);
				CanSendYellowBlink();//黄闪
				heart = TIMEOUT_HEARTS;
			}
#else
			if (heart > TIMEOUT_HEARTS)	//3秒钟收不到主控板can点灯信息就黄闪
			{
				CanSendYellowBlink();//黄闪
				heart = TIMEOUT_HEARTS;
			}	
#endif
		}
		else
		{
			g_heart_count2 = g_heart_count1;
#ifdef EXCUTE_FIXED_CYCLE
			if (heart > ONE_SECOND_HEARTS / 2)//点亮前面板自动按键的LED表明主控板can通信恢复正常
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

