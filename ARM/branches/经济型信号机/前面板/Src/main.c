/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月9日
  最近修改   :
  功能描述   : 这是前面板的MCU程序，主要功能是接收来自灯控板的CAN消息，点亮
               前面板上的模拟路口红绿灯
  函数列表   :
              assert_failed
              CanGetKeyStatus
              CanGetLedStatus
              CanSendKeyStatus
              Can_Get_MailBox
              Can_Send_Recv_Data
              Cfg_Can
              Cfg_IWDG
              Cfg_Uart
              Error_Handler
              EXTI_IRQHandler_Config
              fputc
              HAL_CAN_RxCpltCallback
              HAL_CAN_TxCpltCallback
              HAL_GPIO_EXTI_Callback
              InitGPIOA
              InitGPIOB
              InitGPIOC
              InitGPIOD
              IsRecvEnd
              main
              SetGPIOInputOrOutput
              SetGroupsLedStatus
              SetKeyLight
              SleepNoEnd
              SystemClock_Config
  修改历史   :
  1.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 创建文件

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 添加一些注释信息
******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"
#include <stdio.h>

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
static void Cfg_Can();
static void EXTI_IRQHandler_Config(void);

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
CAN_HandleTypeDef    g_CanHandle;
UART_HandleTypeDef g_UartHandle;
IWDG_HandleTypeDef g_IwdgHandle;

//nLedStatus 0x01 Green, 0x02 Red, 0x04 Yellow
typedef enum
{
    GREEN = 0x01,
    RED = 0x02,
    YELLOW = 0x04,
}LEDSTATUS;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static uint8_t g_nArrayLedGroupStatus[4][4];//分别表示灯控板1-4的灯组1-4的灯状态
static uint8_t g_nArrayRecvedGroup[4];//已接收到灯控信息的组数
static uint8_t g_nMaxGroup = 1;//这个值应该根据CAN消息得到多少路输出，如果是22路输出则为2，如果是44路输出则是4;
static uint32_t g_CountRecved = 0;//仅仅是用来控制闪亮的频率


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
 功能描述  : 系统时钟配置
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
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
  if (HAL_OK != HAL_RCC_OscConfig(&RCC_OscInitStruct))
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK and PCLK1 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  
  if (HAL_OK != HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1))
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
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
 功能描述  : 死循环，防止程序终止。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static void SleepNoEnd(void)
{
    while(1)
    {
        HAL_Delay(500);
    }

}

/*****************************************************************************
 函 数 名  : SetGPIOInputOrOutput
 功能描述  : 扫描按键操作时应该把GPIOA0-4设置为输入模式，而当其用来控制LED灯
             时应该改成推挽输出模式
 输入参数  : uint8_t mode  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void SetGPIOInputOrOutput(uint8_t mode)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    if(0 == mode)//input
    {
    	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;//GPIO_MODE_AF_OD
    	//gGPIO_InitStruct.Pull  = GPIO_NOPULL;
    }
    else if(1 == mode)//output
    {
    	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;//GPIO_MODE_AF_OD
    	//gGPIO_InitStruct.Pull  = GPIO_PULLDOWN;
    }
    
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;

	HAL_GPIO_DeInit(GPIOA,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
}


//分别是手动、自动、黄闪、全红、步进按键的状态
#define KEY0	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)
#define KEY1	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1)
#define KEY2	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2)
#define KEY3	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3)
#define KEY4	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)

/*****************************************************************************
 函 数 名  : SetKeyLight
 功能描述  : 根据按键点亮对应的灯
 输入参数  : uint8_t num        
             uint8_t keyStatus  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void SetKeyLight(uint8_t num,uint8_t keyStatus)
{
    uint16_t GPIO_pin;
    
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);		   //先低后高，上升沿

    switch (num) {
        case 1: GPIO_pin = GPIO_PIN_0; break;
        case 2: GPIO_pin = GPIO_PIN_1; break;
        case 3: GPIO_pin = GPIO_PIN_2; break;
        case 4: GPIO_pin = GPIO_PIN_3; break;
        case 5: GPIO_pin = GPIO_PIN_4; break;
    }
    printf("+++>  key   %d   Pin  %d\r\n",keyStatus,GPIO_pin);
    HAL_GPIO_WritePin(GPIOA,GPIO_pin, keyStatus ? GPIO_PIN_RESET : GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);	
    //HAL_Delay(10);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);		   //先低后高，上升沿

}

/*****************************************************************************
 函 数 名  : Cfg_Uart
 功能描述  : 串口配置
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 将判断返回值改为不等于!
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
}

/*****************************************************************************
 函 数 名  : Cfg_Can
 功能描述  : 对CAN的工作模式、波特率等进行配置
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 增加CAN初始化失败次数
*****************************************************************************/
void Cfg_Can(void)
{
    CAN_FilterConfTypeDef  sFilterConfig;
    static CanTxMsgTypeDef        TxMessage;
    static CanRxMsgTypeDef        RxMessage;
    uint8_t err_time = 0;//错误计数
    
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
        if(255 == err_time)
        {
            printf("Can Init Error \r\n");
            return;
        }
    }

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
 函 数 名  : CalLedOutputCode
 功能描述  : 根据灯色计算应该给寄存器赋值
 输入参数  : LEDSTATUS nLedStatus  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年1月6日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年3月10日
    作    者   : 肖文虎
    修改内容   : 再次修改四个方向的顺序
*****************************************************************************/
uint32_t CalLedOutputCode(LEDSTATUS nLedStatus)
{
    uint8_t i = 0;//i 用来循环东南西北四个方向
    uint8_t j = 0;//j 用来循环左转、直行、右转、行人四个红绿灯
    uint32_t tmp = 0;

    for(i = 0;i < 4 ; i++)
    {
        for(j = 0; j < 4 ; j++)
        {
            if(nLedStatus != (g_nArrayLedGroupStatus[i][j]&nLedStatus))//判断绿灯、红灯、黄灯是否需要点亮，默认是点亮的
            {
                if(3 == j)//如果是行人，则顺序分别是GB13 GB14 GB15 GB16
                {
                    if(nLedStatus == YELLOW)//如果是黄灯的话，直接continue掉，不进行计算
                    {
                        continue;
                    }
                    tmp |= ((uint16_t)0x01 << (12 + i)) ;
                    continue;
                }
            
                if(i < 3)//东南西方向规律相同，分班为GPIO 7 8 9, 4 5 6, 1 2 3, 10 11 12
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
 函 数 名  : SetGroupsLedStatus
 功能描述  : 在CAN消息接收完成之后，根据各组灯的状态，依次点灯。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月9日
    作    者   : 肖文虎
    修改内容   : 硬件的西方向黄灯改变后，代码做相应调整。

  3.日    期   : 2015年1月6日
    作    者   : 肖文虎
    修改内容   : 按照东面通道1234，南面通道5678，西面通道9 10 11 12及北面通
                 道13 14 15 16的顺序，重新点灯
*****************************************************************************/
void SetGroupsLedStatus(void)
{
    //green
    GPIOB->ODR = CalLedOutputCode(GREEN); 
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);   
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET); 

    //red
    GPIOB->ODR = CalLedOutputCode(RED); 
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);   
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);      

    //yellow
    GPIOC->ODR = CalLedOutputCode(YELLOW);
}


/*****************************************************************************
 函 数 名  : CanGetLedStatus
 功能描述  : 解析来自灯控板的点灯信息
 输入参数  : CAN_HandleTypeDef *CanHandle  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 2015年1月7日
    作    者   : xiaowh
    修改内容   : 判断22路时的方法，1表示44路，2表示22路，应该直接判断不应该判断第一位的值
*****************************************************************************/
void CanGetLedStatus(CAN_HandleTypeDef *CanHandle)
{
    uint8_t nGroupId;//灯控板从上到下，有四组灯，前三组是机动车灯有红黄绿的变化，而第4组为行人灯，只有红绿的变化。
    
    switch(CanHandle->pRxMsg->StdId)
    {
        case 0x301: nGroupId = 0;break;
        case 0x302: nGroupId = 1;break;
        case 0x303: nGroupId = 2;break;
        case 0x304: nGroupId = 3;break;
        default:    nGroupId = 0;break;
    }

    g_nArrayLedGroupStatus[nGroupId][0] = CanHandle->pRxMsg->Data[1]&0x07;
    g_nArrayLedGroupStatus[nGroupId][1] = CanHandle->pRxMsg->Data[2]&0x07;
    g_nArrayLedGroupStatus[nGroupId][2] = CanHandle->pRxMsg->Data[3]&0x07;
    g_nArrayLedGroupStatus[nGroupId][3] = CanHandle->pRxMsg->Data[4]&0x07;

    g_nArrayRecvedGroup[nGroupId] = 1;//判断收到多少组了

    if(1 == (CanHandle->pRxMsg->Data[0]))//44路独立输出
    {
        g_nMaxGroup = 4;
    }
    else if(2 == (CanHandle->pRxMsg->Data[0]))//22路独立输出 这时要把东南方向的状态信息复制到西北方向去。
    {
        g_nMaxGroup = 2;
        //printf("===>   StdId  0x%x   GroupId  %d  \r\n",CanHandle->pRxMsg->StdId,nGroupId);
        memcpy(g_nArrayLedGroupStatus[2],g_nArrayLedGroupStatus[0],4);
        memcpy(g_nArrayLedGroupStatus[3],g_nArrayLedGroupStatus[1],4);
    }
}

/*****************************************************************************
 函 数 名  : CanGetKeyStatus
 功能描述  : 解析来自核心板的按键状态信息
 输入参数  : CAN_HandleTypeDef *CanHandle  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void CanGetKeyStatus(CAN_HandleTypeDef *CanHandle)
{
    if(0x110 != CanHandle->pRxMsg->StdId)
    {
        return ;
    }

    GPIO_InitTypeDef   GPIO_InitStructure;

    uint8_t i = 0;

    //设置GPIOA为输出模式
    SetGPIOInputOrOutput(1);
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_15_IRQn); 
    
    for(i = 0; i < 5 ; i++)
    {
        SetKeyLight(i+1,((CanHandle->pRxMsg->Data[0]>>i) & 0x01));
    }

    EXTI_IRQHandler_Config();
}

/*****************************************************************************
 函 数 名  : IsRecvEnd
 功能描述  : 判断是否接收完成，只有当全部接受结束后才统一进行点灯操作、
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t IsRecvEnd(void)
{
    uint8_t i ; 
    uint8_t nRecvNum = 0;
    
    for(i = 0 ; i < 4; i++)
    {
        if(1 == g_nArrayRecvedGroup[i])
        {
            nRecvNum++;
        }
    }

    if(nRecvNum == g_nMaxGroup)
    {
        memset(g_nArrayRecvedGroup,0,sizeof(g_nArrayRecvedGroup));
        return 1;
    }
    else
    {
        return 0;
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
uint8_t Can_Get_MailBox(CAN_HandleTypeDef* hcan)
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
                printf("Can Recv Error \r\n");                
                return 0;
            }
        }
    }
    else if(0 == nSendOrRecv)//发送CAN消息中断
    {
        while(HAL_OK != (err_code = HAL_CAN_Transmit(CanHandle, 10)))//Cube库有点问题，中断发送时可能会造成锁死CAN，故先采用直接发送方式
        //while(HAL_CAN_Transmit_IT(CanHandle) != HAL_OK)//CAN驱动修改后，仍使用中断方式
        {
            err_time++;

            printf("#######Can  Send Failed , State  0x%x  Locked  0x%x transmitmailbox  0x%x err_code 0x%x\r\n",CanHandle->State,CanHandle->Lock,Can_Get_MailBox(CanHandle),err_code);
            if(10 == err_time)
            {
                printf("Can Send Error \r\n");
                return 0;
            }
        }
    }

    return 1;
}

/*****************************************************************************
 函 数 名  : HAL_CAN_RxCpltCallback
 功能描述  : CAN接收完成回调函数
 输入参数  : CAN_HandleTypeDef *CanHandle  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 2015年1月7日
    作    者   : xiaowh
    修改内容   : 修改喂狗时机及闪亮频率
*****************************************************************************/
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *CanHandle)
{
    if((0x110 == CanHandle->pRxMsg->StdId)&&(1 == CanHandle->pRxMsg->DLC))//主控板给前面板发送的按键点亮状态信息
    {
        HAL_IWDG_Refresh(&g_IwdgHandle);
        printf("StdID :  %d , Data[0]  :  0x%x \r\n",CanHandle->pRxMsg->StdId,CanHandle->pRxMsg->Data[0]);
        CanGetKeyStatus(CanHandle);
		g_CountRecved++;
        //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_5);
    }
    else if((5 == CanHandle->pRxMsg->DLC)&&((0x301 == CanHandle->pRxMsg->StdId) || (0x302 == CanHandle->pRxMsg->StdId) || (0x303 == CanHandle->pRxMsg->StdId) || (0x304 == CanHandle->pRxMsg->StdId)))
    {
        HAL_IWDG_Refresh(&g_IwdgHandle);
        CanGetLedStatus(CanHandle);

        //if(IsRecvEnd() == 1)
        {
            SetGroupsLedStatus();
        }            
		g_CountRecved++;
    }

    if(5 == g_CountRecved)
    {
        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_8);
        g_CountRecved = 0;
    }
    Can_Send_Recv_Data(CanHandle,1);//接收新的CAN消息
}

/*****************************************************************************
 函 数 名  : HAL_CAN_TxCpltCallback
 功能描述  : CAN消息发送完成回调函数。
 输入参数  : CAN_HandleTypeDef* hcan  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 2015年1月7日
    作    者   : xiaowh
    修改内容   : 发送完成后不再喂狗
*****************************************************************************/
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
    //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_4);
    
    //HAL_IWDG_Refresh(&g_IwdgHandle);
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
//    NVIC_SystemReset();//重启系统
}

/*****************************************************************************
 函 数 名  : EXTI_IRQHandler_Config
 功能描述  : 配置按键中断
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static void EXTI_IRQHandler_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Configure PC.13 pin as input floating */
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

/*****************************************************************************
 函 数 名  : CanSendKeyStatus
 功能描述  : 当有按键按下时，前面板向主控板发送所有按键的状态信息，或者当核
             心板有请求时发送以下信息。
 输入参数  : uint8_t nKeyStatus  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void CanSendKeyStatus(uint8_t nKeyStatus)
{
    g_CanHandle.pTxMsg->StdId = 0x401;
    g_CanHandle.pTxMsg->ExtId = 0;
    g_CanHandle.pTxMsg->RTR = CAN_RTR_DATA;
    g_CanHandle.pTxMsg->IDE = CAN_ID_STD;
    g_CanHandle.pTxMsg->DLC = 1;
    g_CanHandle.pTxMsg->Data[0] = nKeyStatus;

    Can_Send_Recv_Data(&g_CanHandle,0);
}

/*****************************************************************************
 函 数 名  : HAL_GPIO_EXTI_Callback
 功能描述  : 按键中断回调函数
 输入参数  : uint16_t GPIO_Pin  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t nKeyStatus = 0;//按键LED灯的状态，低5位有效，分别表示自动、手动、黄闪、全红、步进的状态，1表示按下，0表示未按下。

    if ((GPIO_Pin == GPIO_PIN_0)&&(!KEY0))
    {
        nKeyStatus |= 0x01;
    }
    else if((GPIO_Pin == GPIO_PIN_1)&&(!KEY1))
    {
        nKeyStatus |= (0x01 << 0x01);
    }
    else if((GPIO_Pin == GPIO_PIN_2)&&(!KEY2))
    {
        nKeyStatus |= (0x01 << 0x02);
    }
    else if((GPIO_Pin == GPIO_PIN_3)&&(!KEY3))
    {
        nKeyStatus |= (0x01 << 0x03);
    }
    else if((GPIO_Pin == GPIO_PIN_4)&&(!KEY4))
    {
        nKeyStatus |= (0x01 << 0x04);
    }
    printf("===>  HAL_GPIO_EXTI_Callback , Pin  %d   nKeyStatus  %d\r\n",GPIO_Pin,nKeyStatus);
    if(nKeyStatus != 0)//防止误操作
    {
        //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_3);
        printf("Send Key Status :  0x%x \r\n",nKeyStatus);
        CanSendKeyStatus(nKeyStatus);
    }
}

/*****************************************************************************
 函 数 名  : InitGPIOA
 功能描述  : 初始化GPIOA
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitGPIOA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//高有效，默认是要拉低的
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin = GPIO_PIN_5;//红绿指示灯使能脚
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//低有效，默认是要拉高的
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_15;//外部按键板控制LED使能脚  / 外部按键板控制时钟 /CAN信号接收指示灯
                                                                                // 外部按键板控制按键使能脚
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    


    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);	 //设置PA5高电平，使能红绿指示灯

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);//设置PA6低电平，使能按键控制LED

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);//

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);//默认状态下，要把CAN接收指示灯灭掉，初始化只是给ODR寄存器一个RESET值，而这个RESET值正好是
                                                        //0，所以，Init结束后CAN指示灯会亮的.
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);  //使能外部按键值读入

    //CAN通信引脚PA11 PA12在专有CAN初始化函数里完成初始化。
}

/*****************************************************************************
 函 数 名  : InitGPIOB
 功能描述  : 初始化GPIOB
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitGPIOB(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    __GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Pull = GPIO_PULLUP;//起始拉高
    GPIO_InitStruct.Pin = GPIO_PIN_All;//
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  //红绿指示灯总线

    GPIOB->ODR = 0xffff;
}

/*****************************************************************************
 函 数 名  : InitGPIOC
 功能描述  : 初始化GPIOC
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitGPIOC(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    __GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Pull = GPIO_PULLUP;//起始拉高
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|
                            GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|
                            GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;//
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);  //黄指示灯控制线

    GPIOC->ODR |= 0x1fff;
}

/*****************************************************************************
 函 数 名  : InitGPIOD
 功能描述  : 初始化GPIOD
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitGPIOD(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    __GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Pull = GPIO_PULLUP;//起始拉高
    GPIO_InitStruct.Pin = GPIO_PIN_2;//
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);  //黄指示灯控制线

    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
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
  1.日    期   : 2014年11月4日
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
 函 数 名  : TestLedIndex
 功能描述  : 测试函数，用来测试实际LED灯的顺序
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年1月6日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void TestLedIndex()
{
    uint8_t i = 0;
    uint8_t j = 0;

    while(1)
    {

        GPIOB->ODR = 0;

        for(i = 0 ; i < 16 ; i++)
        {
            GPIOB->ODR |= (1 << i);
            
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);   
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);         

            HAL_Delay(1000);
        }

    }



    return;
    for(i = 0 ; i < 4; i++)
    {
        for(j = 0 ; j < 4; j++)
        {
            g_nArrayLedGroupStatus[i][j] = GREEN;
            HAL_Delay(400);
            SetGroupsLedStatus();
        }
    }
    HAL_Delay(50);
    memset(g_nArrayLedGroupStatus,0,sizeof(g_nArrayLedGroupStatus));
    for(i = 0 ; i < 4; i++)
    {
        for(j = 0 ; j < 4; j++)
        {
            g_nArrayLedGroupStatus[i][j] = RED;
            HAL_Delay(400);
            SetGroupsLedStatus();
        }
    }
    memset(g_nArrayLedGroupStatus,0,sizeof(g_nArrayLedGroupStatus));

    HAL_Delay(50);
    for(i = 0 ; i < 4; i++)
    {
        for(j = 0 ; j < 4; j++)
        {
            g_nArrayLedGroupStatus[i][j] = YELLOW;
            HAL_Delay(400);
            SetGroupsLedStatus();
        }
    }
    
}


/*****************************************************************************
 函 数 名  : main
 功能描述  : 前面板主函数入口
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月4日
    作    者   : xiaowh
    修改内容   : 新生成函数
  2.日    期   : 2015年1月7日
    作    者   : xiaowh
    修改内容   : 在主循环里已不再喂狗，只在收到CAN消息后才喂狗，否则就会重启，先启动看门狗再启动CAN接收中断
	
*****************************************************************************/
int main(void)
{
    HAL_Init();
    
    SystemClock_Config();//Configure the system clock to 48 MHz

    if (0 != __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        HAL_Delay(1000);
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    else
    {
        //加1s延时，保证时钟稳定
        HAL_Delay(1000);
    }

    InitGPIOA();
    InitGPIOB();
    InitGPIOC();
    InitGPIOD();
    Cfg_Uart();
    Cfg_Can();
    Cfg_IWDG();
    //TestLedIndex();
    if(HAL_OK != HAL_IWDG_Start(&g_IwdgHandle))// Start the IWDG
    {
        Error_Handler();
    }    
	
    //以中断方式接收按键动作
    EXTI_IRQHandler_Config();


    if(HAL_OK != HAL_CAN_Receive_IT(&g_CanHandle, CAN_FIFO0))
    {
        Error_Handler();
    }

    printf("==> System Restart ... \r\n");

    while (1)
    {
        HAL_Delay(500);
#if 1
        if (HAL_OK != HAL_IWDG_Refresh(&g_IwdgHandle))//喂狗
        {
          Error_Handler();
        }
#endif
    }
}


