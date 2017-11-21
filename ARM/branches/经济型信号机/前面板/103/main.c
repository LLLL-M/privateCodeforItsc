//前面板00000000000000000000000000000


/*----------------------------------------------*
 * 包含头文件                                   *
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
static void EXTI_IRQHandler_Config(void);
void SetKeyLight(uint8_t num,uint8_t keyStatus);
static void DisabledEXTI(uint32_t num);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


__IO uint32_t TimingDelay = 0;

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

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
static uint32_t g_CountRecved = 0;//仅仅是用来控制闪亮的频率


/**	@fn	void Delay(__IO uint32_t nCount)
  * @brief  延时,单位可以理解为毫秒
  * @param  nCount 循环计数
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
void CanGetLedStatus(CanRxMsg *CanHandle)
{
    uint8_t nGroupId;//灯控板从上到下，有四组灯，前三组是机动车灯有红黄绿的变化，而第4组为行人灯，只有红绿的变化。
    
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


    if(2 == (CanHandle->Data[0]))//22路独立输出 这时要把东南方向的状态信息复制到西北方向去。
    {
        //printf("===>   StdId  0x%x   GroupId  %d  \r\n",CanHandle->StdId,nGroupId);
        memcpy(g_nArrayLedGroupStatus[2],g_nArrayLedGroupStatus[0],4);
        memcpy(g_nArrayLedGroupStatus[3],g_nArrayLedGroupStatus[1],4);
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

    //设置GPIOA为输出模式
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


//分别是手动、自动、黄闪、全红、步进按键的状态
#define KEY0	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
#define KEY1	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)
#define KEY2	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)
#define KEY3	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)
#define KEY4	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)

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
    
	GPIO_WriteBit(GPIOA,GPIO_Pin_7,Bit_RESET);		   //先低后高，上升沿

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
	GPIO_WriteBit(GPIOA,GPIO_Pin_7,Bit_RESET);		   //先低后高，上升沿

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t nKeyStatus = 0;//按键LED灯的状态，低5位有效，分别表示自动、手动、黄闪、全红、步进的状态，1表示按下，0表示未按下。

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
    if(nKeyStatus != 0)//防止误操作
    {
        printf("===>  HAL_GPIO_EXTI_Callback , Pin  %d   nKeyStatus  %d\r\n",GPIO_Pin,nKeyStatus);

        //HAL_GPIO_TogglePin(GPIOC,GPIO_Pin_3);
        printf("Send Key Status :  0x%x \r\n",nKeyStatus);
        CanSendKeyStatus(nKeyStatus);
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
void CAN_RX1_IRQHandler_def(void)
{
//	int rc = -1;
	CanRxMsg RxMessage;
	
//	u8 str[128]="";
//	int i=0;
	
	IWDG_ReloadCounter();  //喂狗
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

   if((1 == RxMessage.DLC)&&(0x110 == RxMessage.StdId))//主控板给前面板发送的按键点亮状态信息
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
	//使能DMA时钟
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		
	//使能ADC和GPIOC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
	//使能USART和GPIOC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOC, ENABLE);
	//CAN时钟
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

void CFG_Usart()	//串口
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
    CAN_InitStructure.CAN_TTCM = DISABLE;//禁止时间触发通信模式
    CAN_InitStructure.CAN_ABOM=DISABLE;//，软件对CAN_MCR寄存器的INRQ位进行置1随后清0后，一旦硬件检测到128次11位连续的隐性位，就退出离线状态。
    CAN_InitStructure.CAN_AWUM=DISABLE;//睡眠模式通过清除CAN_MCR寄存器的SLEEP位，由软件唤醒
    CAN_InitStructure.CAN_NART=DISABLE;//DISABLE;CAN报文只被发送1次，不管发送的结果如何（成功、出错或仲裁丢失）
    CAN_InitStructure.CAN_RFLM=DISABLE;//在接收溢出时FIFO未被锁定，当接收FIFO的报文未被读出，下一个收到的报文会覆盖原有的报文
    CAN_InitStructure.CAN_TXFP=DISABLE;//发送FIFO优先级由报文的标识符来决定
    //CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
    CAN_InitStructure.CAN_Mode=CAN_Mode_Normal; //CAN硬件工作在正常模式 
    CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;//重新同步跳跃宽度1个时间单位
    CAN_InitStructure.CAN_BS1=CAN_BS1_3tq;//时间段1为8个时间单位
    CAN_InitStructure.CAN_BS2=CAN_BS2_2tq;//时间段2为7个时间单位
    CAN_InitStructure.CAN_Prescaler = 6;//(500Kbits) 9;//250Kbits设定了一个时间单位的长度9	 (36000000/(1+2+3)/12)
    CAN_Init(CAN1, &CAN_InitStructure);
    
    /* CAN filter init 过滤器初始化*/
    CAN_FilterInitStructure.CAN_FilterNumber = 0;//指定了待初始化的过滤器0
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//指定了过滤器将被初始化到的模式为标识符屏蔽位模式
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//给出了过滤器位宽1个32位过滤器
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;//用来设定过滤器标识符（32位位宽时为其高段位，16位位宽时为第一个）
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;//用来设定过滤器标识符（32位位宽时为其低段位，16位位宽时为第二个
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;//用来设定过滤器屏蔽标识符或者过滤器标识符（32位位宽时为其高段位，16位位宽时为第一个
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;//用来设定过滤器屏蔽标识符或者过滤器标识符（32位位宽时为其低段位，16位位宽时为第二个
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;//设定了指向过滤器的FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation  =ENABLE;//使能过滤器		 	
    CAN_FilterInit(&CAN_FilterInitStructure);
    
    /* CAN FIFO0 message pending interrupt enable */ 
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);//使能指定的CAN中断ol
}

void IWDG_Configuration(void)
{
	RCC_LSICmd(ENABLE);

	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

  	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  	/* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
  	IWDG_SetPrescaler(IWDG_Prescaler_256);

  	/* Set counter reload value to 299 */
  	IWDG_SetReload(312);   //2s   喂狗时间计算:(Prescaler/4)*0.1*RLR=2000ms

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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//高有效，默认是要拉低的
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//低有效，默认是要拉高的
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_15;//外部按键板控制LED使能脚  / 外部按键板控制时钟 /CAN信号接收指示灯
                                                                                // 外部按键板控制按键使能脚
    GPIO_Init(GPIOA, &GPIO_InitStructure);    


    GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET);	 //设置PA5高电平，使能红绿指示灯

    GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_RESET);//设置PA6低电平，使能按键控制LED

    GPIO_WriteBit(GPIOA,GPIO_Pin_7,Bit_SET);//

    GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_SET);//默认状态下，要把CAN接收指示灯灭掉，初始化只是给ODR寄存器一个RESET值，而这个RESET值正好是
                                                        //0，所以，Init结束后CAN指示灯会亮的.
    GPIO_WriteBit(GPIOA,GPIO_Pin_15,Bit_RESET);  //使能外部按键值读入


    //GPIOB
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;//
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //红绿指示灯总线

    GPIOB->ODR = 0xffff;

    //GPIOC
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|
                            GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|
                            GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //黄指示灯控制线

    GPIOC->ODR |= 0x1fff;

    //GPIOD
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //黄指示灯控制线

    GPIO_WriteBit(GPIOD,GPIO_Pin_2,Bit_SET);
	

	//CAN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					//管脚11
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			 	//输入模式
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO组

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;					//管脚12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			 	//输出模式
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO组

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

    //同理中断控制1 2 3 4
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
	RCC_Enable();	//使能时钟
	CFG_Nvic();		//NVIC配置
	CFG_Usart();	//串口
	CFG_Can();      //CAN
	IWDG_Configuration();
	CFG_GPio(); 	//引脚配置
	EXTI_IRQHandler_Config();//按键中断配置
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

