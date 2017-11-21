//103灯控板-----------------------

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


volatile uint16_t   g_ADCxConvertedValuesArray[4];

#define ADC1_DR_Address    ((uint32_t)0x4001244C)	 //ADC数据寄存器的基地址
// 注：ADC为12位模数转换器，只有ADCConvertedValue的低12位有效
//__IO uint16_t ADCConvertedValue[2]={0};  //AD0,AD1

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
//static uint16_t g_nCurVauleArray[4] = {0};//电流检测结果存放在该数组中
static uint8_t g_nGreenLampVoltageVauleArray[4] = {0};//四组绿灯电压值
static uint8_t g_nRedLampVoltageVauleArray[4] = {0};//四组红灯电压值
static __IO uint16_t g_nLampVol = 0;//只需要12位，从低到高分别为第N组的绿黄红电压。
static uint8_t g_nOutPutMode = 1;//输出模式，1:44路独立输出，2:22路独立输出 
static uint32_t g_ArrayLedGroupStatusArray[4] = {0x01,0x01,0x01,0x01};//四组灯的状态，均为低3位有效，分别表示绿灯、红灯、黄灯的状态，1亮0灭,改成32位是为了方便保存到flash
static uint8_t g_nBaseAddr;//灯控板基地址，0、1、2、3  从PB3 PB4获取.信号机基地址可以通过串口进行设定，信号机基地址保存到flash中。如果读flash不为零，则直接用flash地址作为基地址。否则就用GPIO口的值作为
//基地址(相当于flash的优先级最高，如果现场出现两个板子地址冲突的情况，可以通过串口来修改)。

static uint8_t  g_heart_count1 = 0;//用来监测有无收到来自主控板的CAN消息
static uint8_t  g_heart_count2 = 0;
//static uint8_t g_interrupted = 0;
static uint32_t g_CountRecved = 0;//仅仅是用来控制闪亮的频率
static uint32_t g_CountSend = 0;//仅仅是用来控制闪亮的频率
//static uint8_t g_IsCanRecvExit = 0;//判断CAN的接收中断是否异常终止，若终止了需在main函数里手动重新启动CAN的接收。


uint32_t gCurValAverArray[4][5];//4组红灯电流的实时值及基准值，前4位为实时值，最后1位位基准值。
uint8_t gCurValIndexArray[4] = {0};//4组红灯电流求评价值是当前数组的更新顺序
uint8_t gIsCurCalFirstTime[4] = {0};//4组红灯电流的计算是否是第一次填满数组

static uint8_t gCurPrint = 0;//电流打印开关，默认为关闭
static uint8_t gVolPrint = 0;//电压打印开关，默认为关闭
static uint8_t gCanMsgContentPrint = 0;//CAN消息内容打印开关，默认为关闭
static uint8_t gIsUseFlaseAddr = 0;//是否使用flash中存放的地址作为灯控板的基地址，为避免flash读的太频繁，一旦确定从flash中使用基地址的话
                                    //程序就不会一直轮询flash了。

__IO uint32_t TimingDelay = 0;

#define LAST_STATUS_ADDR		(0x08007C00 - 0x400)	//把用户可用的flash地址最后一页用来保存配置
#define BOARD_BASE_ADDR         (LAST_STATUS_ADDR - 0x400)  //板子基地址的flash存放位置

#define DELAY_MSEC	250
#define ONE_SECOND_HEARTS	(1000 / DELAY_MSEC)	//1s钟心跳计数
//超时心跳数,如果心跳数超过这个值还未接收到can信息则灯控板各自执行黄闪控制
#define TIMEOUT_HEARTS	(3 * ONE_SECOND_HEARTS)

#define EXCUTE_FIXED_CYCLE
#define USE_UART_RECV_IT

#ifdef EXCUTE_FIXED_CYCLE
#include "fixedcycle.h"		//所有的函数实现以及结构体和全局变量的定义全在这里面
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
		break; //重启系统
		case 0xf1:	//擦除配置,恢复默认
			WriteFlashData(LCB_CONFIG_ADDR, &gDefaultConfig, sizeof(LCBconfig));
			memcpy(gLCBconfig,&gDefaultConfig,sizeof(LCBconfig));
			gLCBconfig->configstate = CONFIG_UPDATE;
			printf("restore default config!\r\n");
			break;
		case 0xf2:	//打印配置
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
        case 0xf4://开启或关闭电流打印开关
            gCurPrint = (gCurPrint == 0 ? 1 : 0 );
            break;
        case 0xf5://开启或关闭电压检测开关
            gVolPrint = (gVolPrint == 0 ? 1 : 0 );
            break;
        case 0xf6://打印收到的CAN消息内容
            gCanMsgContentPrint = (gCanMsgContentPrint == 0 ? 1 : 0 );
            break;
        case 0xf7://读取GPIO的基地址
            printf("the GPIO base addr is :  %d \r\n",GetBaseAddr());
            break;                
        case 0xf8://读取存放在flash的基地址
            ReadFlashData(BOARD_BASE_ADDR,&tmp,sizeof(tmp));
            printf("the FLASH base addr is :  %d\r\n",tmp);
            break;         
        case 0xf9://设置存放在flash的基地址
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
                if(val >= 0 && val <= 7)//最多支持8个灯控板
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
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
//static void EXTI_IRQHandler_Config(void);
void SetKeyLight(uint8_t num,uint8_t keyStatus);
//static void DisabledEXTI(uint32_t num);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


//__IO uint32_t TimingDelay = 0;

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


//static uint8_t g_nArrayLedGroupStatus[4][4];//分别表示灯控板1-4的灯组1-4的灯状态
//static uint32_t g_CountRecved = 0;//仅仅是用来控制闪亮的频率


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

//上电后，首先恢复上一次的点灯状态，如果读出来的数据为空，就认为上次无效，无效不做处理。
void RecorveryLastStatus()
{
  int i = 0;
  //读取上一次的状态
  ReadFlashData(LAST_STATUS_ADDR,g_ArrayLedGroupStatusArray,sizeof(g_ArrayLedGroupStatusArray));
  //判断读取的值是否有效
  for(i = 0; i < 4; i++)
  {
    if(g_ArrayLedGroupStatusArray[i] > 7 || g_ArrayLedGroupStatusArray[i] <= 0)
    {
      return;
    }
  }
  //点灯
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
    g_nRedLampVoltageVauleArray[0] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[1] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[2] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0 ? 1 : 0);
    g_nRedLampVoltageVauleArray[3] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0 ? 1 : 0);

    //读取绿灯电压值
    g_nGreenLampVoltageVauleArray[0] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[1] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[2] = (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0 ? 1 : 0);
    g_nGreenLampVoltageVauleArray[3] = (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == 0 ? 1 : 0);

    //将四组电流电压值重新组织成12位变量
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
    static uint8_t flagIsContinue[4] = {0};//如果只是偶然连续2次，实时值偏差较大，则不计入计算，否认长时间偏差较大就要进入计算了.
    
    CanTxMsg TxMessage;
    memset(&TxMessage,0,sizeof(TxMessage));
	
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
    TxMessage.StdId = 0;
    TxMessage.ExtId = (1 << (16+g_nBaseAddr))|(1 << 15) | (g_nLampVol << 3) | (nTempBaseAddr + 1 );//0-2 板号、3-14 绿红黄灯电压、15 1
    TxMessage.IDE = CAN_ID_EXT;//标示符类型为扩展
    TxMessage.RTR = CAN_RTR_DATA;//消息帧类型为数据帧
    TxMessage.DLC = 4;//帧长度为4
    
    //g_ADCxConvertedValuesArray[0] = GetAdcValues(ADC_Channel_1);
    //g_ADCxConvertedValuesArray[1] = GetAdcValues(ADC_Channel_2);
    //g_ADCxConvertedValuesArray[2] = GetAdcValues(ADC_Channel_3);
    
    
    for(i = 0 ; i < 4; i++)
    {
      
        ndiff = g_ADCxConvertedValuesArray[i]-gCurValAverArray[i][4];//拿实时值减去基值作为电流值

        //如果该实时值波动较大，则不进入计算,
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

        if(ndiff > 255)//保证CAN的data要在[0,255]范围内
        {
            ndiff = 255;
        }

        if(ndiff < 0)
        {
            ndiff = 0;
        }
        if(gCurPrint == 1)
            printf(" %d-%d-%d  ",g_ADCxConvertedValuesArray[i],gCurValAverArray[i][4],ndiff);
        TxMessage.Data[i] = ndiff;//红灯亮起时，其电流值应该大于10    
		
    }
    if(gCurPrint == 1)
        printf("  ~~~~~~~~  0x%x\r\n",TxMessage.ExtId);
    CAN_TX1_SendData(&TxMessage);
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
    CanTxMsg TxMessage;
	uint8_t nTempBaseAddr;    
    memset(&TxMessage,0,sizeof(TxMessage));

    nTempBaseAddr = g_nBaseAddr;//判断当前应该发的是哪个ID，只有是22路的时候才需要变更StdId

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
    
    TxMessage.StdId = 0x301 + nTempBaseAddr;//
    TxMessage.ExtId = 0;
    TxMessage.IDE = CAN_ID_STD;//
    TxMessage.RTR = CAN_RTR_DATA;//消息帧类型为数据帧 
    TxMessage.DLC = 5;//帧长度为5
    TxMessage.Data[0] = g_nOutPutMode;
    TxMessage.Data[1] = g_ArrayLedGroupStatusArray[0]&0x07;
    TxMessage.Data[2] = g_ArrayLedGroupStatusArray[1]&0x07;
    TxMessage.Data[3] = g_ArrayLedGroupStatusArray[2]&0x07;
    TxMessage.Data[4] = g_ArrayLedGroupStatusArray[3]&0x07;

    CAN_TX1_SendData(&TxMessage);
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
		IWDG_ReloadCounter();
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
void DecodeCanMsg(const CanRxMsg *CanHandle)
{
    uint8_t i = 0;
    uint8_t j = 0;

    uint16_t nCurBoardVal = 0;//当前灯控板的12位灯状态字，每3位一个通道，分别表示绿红黄的状态;获取CAN消息的12位灯状态字再逐个点灯
    uint8_t nTempBaseAddr = g_nBaseAddr;//当前板子应该读取CAN消息的第几个12位
    
    g_nOutPutMode = CanHandle->Data[0];//如果是22位的话，则Data[1]-[3]分别表示东西方向和南北方向的。

    if(nTempBaseAddr > 3)//表明地址是5 、6板子
    {
        if(g_nOutPutMode == 3)//且收到的数据是扩展的CAN消息，则修改需要读取的位置
            nTempBaseAddr -= 4;
        else        //不是扩展的消息，则什么也不做
            return;
    }

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
    
    nCurBoardVal |= CanHandle->Data[i]>>((nTempBaseAddr*12)%8);//取低位
    nCurBoardVal |= (uint16_t)(CanHandle->Data[i+1])<<j;//取高位        

    //给全局变量赋值
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
    uint8_t tmp = 0;//每个板子在不同的槽位都对应着不同的地址，分别为0 1 2 3

    if(gIsUseFlaseAddr == 1)
        return g_nBaseAddr;
    
    tmp = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3);
	
    tmp *= 2;
    tmp += GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4);
	//printf("--->  %d  -- %d\r\n",GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3),GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4));
    return tmp;
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
void GetCurAverVal(void)
{
    uint8_t i = 0;
    uint32_t tempTotalVal = 0;
    uint8_t tempOffOn = 0;//临时变量，判断当前红灯的亮灭状态，只有灭灯的时候才读电流值
    static uint8_t flagIsContinue[4] = {0};//如果只是偶然连续2次，实时值偏差较大，则不计入计算，否认长时间偏差较大就要进入计算了.

  //  g_ADCxConvertedValuesArray[2] = GetAdcValues(ADC_Channel_2);
    for(i = 0; i < 4; i++)
    {
        if(0 == i)//第一组红灯
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
        if(1 == tempOffOn)//灯亮的时候，直接continue掉
        {
            continue;
        }
        //如果该实时值波动较大，则不进入计算,
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
            gCurValIndexArray[i] = 0;//下次读取的值放到数组第一位
            gIsCurCalFirstTime[i] = 1;
        }
        
        if(gIsCurCalFirstTime[i] == 1)//第一次必须等填满数组后再计算，后面的就只需要每次过来就计算
        {
            tempTotalVal = gCurValAverArray[i][0] + gCurValAverArray[i][1] + gCurValAverArray[i][2] + gCurValAverArray[i][3];
            gCurValAverArray[i][4] = (tempTotalVal >> 2);//右移两位相当于除以4
        }
    }
   // printf("\r\n");
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
	 CanRxMsg RxMessage;
	IWDG_ReloadCounter();  //喂狗
	memset(&RxMessage,0,sizeof(RxMessage));
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

   if((RxMessage.DLC == (g_nBaseAddr > 3 ? 4 : 7))&&(0x101 == RxMessage.StdId))
   {
        IWDG_ReloadCounter();
        
        //解析数据并点灯
        DecodeCanMsg(&RxMessage);
        SetLedGroupStatus();
        CanSendLampStatus();//给前面板发送点灯信息

        //监测电流电压值并发送
        //LampControlBoard_GetCur();//得到电流参数
        LampControlBoard_GetVoltage();//通过GPIO读取电压
        GetCurAverVal();//定时更新电流基值
        CanSendCurVolData();//给主控板发送电流电压值
        
    	g_heart_count1++;	   //用于控制黄闪
//    	g_interrupted = 1;   //用于控制黄闪

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
	{	//第15位为1并且长度为4的can消息表示是灯控板电流及电压反馈
		ConvertToLightData(&RxMessage);	//把获取的电流电压转换成实际的点灯数据
	}
#endif
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
 	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	//使能ADC和GPIOC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
	//使能USART和GPIOC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);
	//CAN时钟
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
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5 |GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9 |GPIO_Pin_10 |GPIO_Pin_11|GPIO_Pin_12;	 //输出
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //输入
  	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//CAN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					//管脚11
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			 	//输入模式
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO组

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;					//管脚12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			 	//输出模式
 	GPIO_Init(GPIOA, &GPIO_InitStructure);						//GPIO组

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
	DMA_DeInit(DMA1_Channel1);		  								  		//开启DMA1的第一通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;		  		//DMA对应的外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t )&g_ADCxConvertedValuesArray;     //内存存储基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;				  		//DMA的转换模式为SRC模式，由外设搬移到内存
	DMA_InitStructure.DMA_BufferSize = 4;		   							//DMA缓存大小，2个
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//接收一次数据后，设备地址禁止后移
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//关闭接收一次数据后，目标内存地址后移
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //定义外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  	//DMA搬移数据尺寸，HalfWord就是为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   						//转换模式，循环缓存模式。
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//DMA优先级高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  					//M2M模式禁用
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);          
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

void CFG_Adc()		//模数转换
{
	ADC_InitTypeDef ADC_InitStructure;
	 /* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		//独立的转换模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		    //开启扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;      //开启连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ADC外部开关，关闭状态
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   //对齐方式,ADC为12位中，右对齐方式
	ADC_InitStructure.ADC_NbrOfChannel = 4;	 				 //开启通道数，2个
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel9 configuration */ //ADC通道组， 第9个通道 采样顺序1，转换时间
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);  
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5);  
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_239Cycles5);  

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);	  //ADC命令，使能		
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);  //开启ADC1
	
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);	  //重新校准
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));  //等待重新校准完成
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);		//开始校准
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));	   //等待校准完成
	 
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//连续转换开始，ADC通过DMA方式不断的更新RAM区.

}

void init()
{
	RCC_Enable();	//使能时钟
	CFG_GPio(); 	//引脚配置
	
    //在这里添加上次的保留状态的话，可以在重启时不关灯的情况下，保持灯色不变
    RecorveryLastStatus();

	CFG_Nvic();		//NVIC配置
	CFG_Dma();		//DMA
	CFG_Adc();		//模数转换
	CFG_Can();      //CAN
	IWDG_Configuration();
	CFG_Usart();	//串口
	
}

//得到AD采样值的类型，如果AD是[0,372]，返回0，[3721,4468]返回1，[1675,2420]返回2 
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

//背板改版后，不是直接读取GPIO口的值作为灯控板的基地址了，具体逻辑原理如下:
//使用PC0 PC1作为AD采集引脚，其中PC0作为低位，PC1作为高位
//		PC1的AD值			PC0的AD值					基地址
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
    uint32_t heart = 0;
    uint8_t sleepCount = 0;//主控程序停掉，或者重启灯控板，要等待sleepCount后，再运行黄闪或相位接管。
    uint32_t tmp = 0;
#ifdef EXCUTE_FIXED_CYCLE
	uint8_t initYellowFlashSec = 20; //初始化时黄闪20s防止主控板还未点灯时灯控板自主运行
	uint8_t initAllRedSec = 6; //初始化时黄闪结束后还未收到主控板消息则全红6s
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
			sleepCount = 0;
			HAL_Delay(DELAY_MSEC);
			IWDG_ReloadCounter();
		}
        HAL_GPIO_TogglePin(GPIOA,GPIO_Pin_6);
    }

//    return 0;
}

