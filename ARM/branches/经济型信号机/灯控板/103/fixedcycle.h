#ifndef __FIXEDCYCLY_H
#define __FIXEDCYCLY_H

#include "lcb.h"

extern void HAL_Delay(__IO uint32_t nCount);
extern void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
extern void HAL_UART_RxCpltCallback(uint16_t val);


#if 0
typedef struct _bit64_
{
	uint32_t low;
	uint32_t high;
} Data64;
#endif

typedef struct _direction_
{
	uint16_t left:3;		//左转
	uint16_t straight:3;	//直行
	uint16_t right:3;		//右转
	uint16_t pedestrian:3;	//行人
} Direction;

typedef struct _direction_set_
{
	uint64_t east:12;		//东方向
	uint64_t south:12;		//南方向
	uint64_t west:12;		//西方向
	uint64_t north:12;		//北方向
} DirectionSet;

typedef struct _channel_set_
{
	uint64_t ch1:3;			//通道1
	uint64_t ch2:3;			//通道2
	uint64_t ch3:3;			//通道3
	uint64_t ch4:3;			//通道4
	uint64_t ch5:3;			//通道5
	uint64_t ch6:3;			//通道6
	uint64_t ch7:3;			//通道7
	uint64_t ch8:3;			//通道8
	uint64_t ch9:3;			//通道9
	uint64_t ch10:3;		//通道10
	uint64_t ch11:3;		//通道11
	uint64_t ch12:3;		//通道12
	uint64_t ch13:3;		//通道13
	uint64_t ch14:3;		//通道14
	uint64_t ch15:3;		//通道15
	uint64_t ch16:3;		//通道16
} ChannelSet;

typedef union _light_data_
{
	DirectionSet dirset;	//方向集合
	//Data64 data64;
	ChannelSet channelset;	//通道集合
	uint8_t data[6];		//点灯数据
} LightData;

typedef struct _phase_lightvalue_
{
	uint8_t lightvalue:4;	//运行相位的点灯值，参考enum LightValue
	uint8_t phase:4;		//运行的相位号
} PhaseLightValue;
typedef struct _run_data_
{	//每个环在当前1s运行的相位以及它的状态
	PhaseLightValue ring1;
	PhaseLightValue ring2;
} RunData;
#define MAX_RUNDATA_NUM		256

#define GET_BIT(v, n) (((v) >> (n)) & 0x1)		//取v的第 n bit位
static LightData gBoardVolt;	//can接受的灯控板电压值,0:无电压，1:有电压
static LightData gRedCur;		//红灯电流对应的点灯值，即电流<=10则灯灭为0，若电流>10则灯亮为1
static uint8_t gFaultFlag = 0;	//故障标志，0:没有故障，1:有故障
static LCBruninfo gRuninfo;		//主控板每秒发送给灯控板的运行信息

#define LCB_CONFIG_ADDR		(0x0801FC00)	//把用户可用的flash地址最后一页用来保存配置

static LCBconfig gLCBconfig[2];	//[0]:用来运行时使用,[1]:用来接收数据时备份用
/*	默认配置为	*/	
static LCBconfig gDefaultConfig = {
	/* 接管控制:开启, 电压检测:关闭, 电流检测:关闭, 相位个数:4, 最小红灯电流值:20, 方案数:1 */
	.baseinfo = {1, 0, 0, 4, 20, 1, 0},
	.phaseinfo = {
		[0] = {3, 3, 2, 3, 0x8686},	//相位1:东西直行,时间20s,对应通道2,3,10,11,8,16
		[1] = {3, 3, 2, 6, 0x101},	//相位2:东西左转,时间15s,对应通道1,9
		[2] = {3, 3, 2, 6, 0x6868},	//相位3:南北直行,时间20s,对应通道6,7,14,15,4,12
		[3] = {3, 3, 2, 6, 0x1010},	//相位4:南北左转,时间15s,对应通道5,13
	},	//每个相位统一绿闪3s,黄灯3s,全红2s,行人绿闪6s
	.phaseturninfo = {
		[0] = {	//相序为:相位1-->相位2-->相位3-->相位4
			.phases = {1, 2, 3, 4, 0, 0, 0, 0},
		},
	},
	.splitinfo = {
		[0] = {	//对应的绿信比为:20s,15s,20s,15s
			.times = {20, 15, 20, 15, 0, 0, 0, 0},
		},
	},
	.configstate = CONFIG_IS_READY,
	.controlBoardNo = 0,				//默认为第一块灯控板进行控制
	.allchannelbits = 0xffff,
};

static inline void ReadFlashData(uint32_t flashAddr, void *buf, int size)
{
	__IO uint32_t *address = (uint32_t *)flashAddr;
	uint32_t *data = (uint32_t *)buf;
	int count = size / sizeof(uint32_t);
	int i;
	
	for (i = 0; i < count; i++, address++)
	{
		data[i] = *address;
	}
}

static inline void WriteFlashData(uint32_t flashAddr, void *buf, int size)
{
	//uint32_t pageError = 0;
	uint32_t *data = (uint32_t *)buf;
	int count = size / sizeof(uint32_t);
	int i;
	
	FLASH_UnlockBank1();

	  /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
    
	//先擦除以前的数据
	if (FLASH_ErasePage((uint32_t)flashAddr) != FLASH_COMPLETE)	//擦除失败
		printf("erase flash fail\r\n");
	else
	{	//擦除成功则把接收的数据存入flash中
		printf("erase flash succ\r\n");
		for (i = 0; i < count; i++)
		{
			if (FLASH_ProgramWord(flashAddr + i * sizeof(uint32_t), data[i]) != FLASH_COMPLETE)
			{
				printf("write data to flash fail, and has write %d word yet!\r\n", i);
				break;
			}
		}
	}
	FLASH_LockBank1();
}

static void LCBconfigInit(void)
{
	memset(gLCBconfig, 0, sizeof(gLCBconfig));
	//从flash中读取配置存放在全局变量gLCBconfig[0]中
	ReadFlashData(LCB_CONFIG_ADDR, gLCBconfig, sizeof(LCBconfig));
	if (gLCBconfig->configstate != CONFIG_IS_READY || !CheckLCBconfigValidity(gLCBconfig))
	{
		printf("config isn't exist or invalid in flash, load default config!\r\n");
		memcpy(gLCBconfig, &gDefaultConfig, sizeof(LCBconfig));
	}
	else
		printf("load LCB config successful!\r\n");
}
static inline void SetAllChannelBits(LCBconfig *config)
{
	int i, n = config->baseinfo.phaseNum;
	config->allchannelbits = 0;
	for (i = 0; i < n; i++)
		config->allchannelbits |= config->phaseinfo[i].channelbits;
	config->controlBoardNo = (config->allchannelbits & 0xf) ? 0 :
							(((config->allchannelbits >> 4) & 0xf) ? 1 :
							(((config->allchannelbits >> 8) & 0xf) ? 2 :
							(((config->allchannelbits >> 12) & 0xf) ? 3 : 4)));
}
static void LCBconfigDeal(CanRxMsg *pRxMsg)
{
	static uint8_t reqno = 0;			//下一个请求的编号
	LCBconfig *config = &gLCBconfig[1];
	CanExtId *extid = (CanExtId *)&pRxMsg->ExtId;
	uint8_t reset = 0;
	
	if (extid->type == RUNINFO)
	{
		memcpy(&gRuninfo, pRxMsg->Data, sizeof(LCBruninfo));
		return;
	}

	if(extid->type == REBOOT)
	{
        HAL_UART_RxCpltCallback(0xf0);
	}
	
	if (extid->no == reqno)
	{
		switch (extid->type)
		{
			case BASEINFO:
				memcpy(&config->baseinfo, pRxMsg->Data, sizeof(LCBbaseinfo));
				if (config->baseinfo.minRedCurVal < 50)
					config->baseinfo.minRedCurVal = 50;
#if 0
				printf("store baseinfo, isTakeOver:%d, voltCheck:%d, curCheck:%d, minRedCurVal=%d, phaseNum=%d, schemeNum=%d, canTotalNum=%d\r\n",\
					config->baseinfo.isTakeOver, config->baseinfo.isVoltCheckEnable, config->baseinfo.isCurCheckEnable,
					config->baseinfo.minRedCurVal, config->baseinfo.phaseNum,
					config->baseinfo.schemeNum, config->baseinfo.canTotalNum);
#endif
				break;
			case PHASEINFO:
				if (extid->index >= MAX_SUPPORT_PHASE_NUM)
				{
					reset = 1;
					break;
				}
				memcpy(&config->phaseinfo[extid->index], pRxMsg->Data, sizeof(LCBphaseinfo));
#if 0
				printf("store phaseinfo %d, channelbits = %#x\r\n", extid->index + 1,
				config->phaseinfo[extid->index].channelbits);
#endif
				break;
			case PHASETURNINFO:
				if (extid->index >= MAX_SUPPORT_PHASETURN_NUM)
				{
					reset = 1;
					break;
				}
				memcpy(&config->phaseturninfo[extid->index], pRxMsg->Data, sizeof(LCBphaseturninfo));
				if (extid->index == 1)	//如果是相序表1的信息,则复制一份作为感应控制的相序
					memcpy(&config->phaseturninfo[0], pRxMsg->Data, sizeof(LCBphaseturninfo));
#if 0
				printf("store phaseturninfo %d, first:%#x, second:%#x, third:%#x, fourth:%#x\r\n", extid->index,
				config->phaseturninfo[extid->index].turn.first,
				config->phaseturninfo[extid->index].turn.second,
				config->phaseturninfo[extid->index].turn.third,
				config->phaseturninfo[extid->index].turn.fourth);
#endif
				break;
			case SPLITINFO:
				if (extid->index >= MAX_SUPPORT_SPLIT_NUM)
				{
					reset = 1;
					break;
				}
				memcpy(&config->splitinfo[extid->index], pRxMsg->Data, sizeof(LCBsplitinfo));
#if 0
				printf("store splitinfo %d, phase1:%d, phase2:%d, phase3:%d, phase4:%d\r\n", extid->index,
				config->splitinfo[extid->index].split.phase1,
				config->splitinfo[extid->index].split.phase2,
				config->splitinfo[extid->index].split.phase3,
				config->splitinfo[extid->index].split.phase4);
#endif
				if (extid->no == config->baseinfo.canTotalNum && extid->index == 0)
				{	/*说明已收到一个完整的配置,最后一个can数据包no等于canTotalNum,且index=0表明是感应控制的绿信比信息*/
					config->configstate = CONFIG_IS_READY;
					SetAllChannelBits(config);
					if (memcmp(config, gLCBconfig, sizeof(LCBconfig)) != 0 && CheckLCBconfigValidity(config))
					{	//不是重复的配置,并且检查配置有效,则可以写入到flash了
						WriteFlashData(LCB_CONFIG_ADDR, config, sizeof(LCBconfig));
						gLCBconfig->configstate = CONFIG_UPDATE;	//提醒配置已经更新,执行定周期时应该读取新的配置
						printf("LCBconfig is updata! controlBoardNo:%d\r\n", config->controlBoardNo);
					}
					else
						printf("the receive LCBconfig is invalid\r\n");
					reset = 1;
				}
				break;
			default: reset = 1;
		}
		reqno++;
	}
	else
		reset = 1;
	if (reset == 1)
	{
		reqno = 0;
		memset(config, 0, sizeof(LCBconfig));
	}
}

static LightValue GetPhaseLightValue(uint8_t splitTime, uint8_t runtime, LCBphaseinfo *phaseinfo)
{
	uint8_t greenTime = splitTime - phaseinfo->greenFlashTime
						- phaseinfo->yellowTime - phaseinfo->allredTime;
	int pedGreenTime = greenTime + phaseinfo->greenFlashTime - (phaseinfo->pedFlashTime&0xff);
	LightValue value = LRED;
	
	if (runtime < greenTime)//绿灯时段,此时如果配置了行人通道，则行人可能为绿闪
		value = (runtime >= pedGreenTime && (phaseinfo->channelbits & 0x8888)) ? LGREEN_FLASH_PED : LGREEN;
	else if (runtime >= greenTime && runtime < greenTime + phaseinfo->greenFlashTime)
		value = LGREEN_FLASH;	//绿闪时段
	else if (runtime >= greenTime + phaseinfo->greenFlashTime 
			&& runtime < greenTime + phaseinfo->greenFlashTime + phaseinfo->yellowTime)
		value = LYELLOW;	//黄灯时段
	return value;
}
//设置行人通道点灯值
static void SetPedLightValue(LightData *ld, uint8_t value, uint32_t channelbits)
{
	int i;
	ChannelSet *channelset = &ld->channelset;
	
	for (i = 3; i < 16; i+=4)
	{	//判断通道的bit位是否置位
		if (GET_BIT(channelbits, i) == 1)
		{
			if (i == 3)
				channelset->ch4 = value;
			else if (i == 7)
				channelset->ch8 = value;
			else if (i == 11)
				channelset->ch12 = value;
			else if (i == 15)
				channelset->ch16 = value;
		}
	}
}
//设置机动车通道点灯值
static void SetMotorLightValue(LightData *ld, uint8_t value, uint32_t channelbits)
{
	int i;
	ChannelSet *channelset = &ld->channelset;
	
	for (i = 0; i < 16; i++)
	{	//判断通道的bit位是否置位
		if (GET_BIT(channelbits, i) == 1)
		{
			if (i == 0)
				channelset->ch1 = value;
			else if (i == 1)
				channelset->ch2 = value;
			else if (i == 2)
				channelset->ch3 = value;
			else if (i == 4)
				channelset->ch5 = value;
			else if (i == 5)
				channelset->ch6 = value;
			else if (i == 6)
				channelset->ch7 = value;
			else if (i == 8)
				channelset->ch9 = value;
			else if (i == 9)
				channelset->ch10 = value;
			else if (i == 10)
				channelset->ch11 = value;
			else if (i == 12)
				channelset->ch13 = value;
			else if (i == 13)
				channelset->ch14 = value;
			else if (i == 14)
				channelset->ch15 = value;
		}
	}
}

void CAN_TX1_SendData(CanTxMsg *pTxMessage);
void SetLedGroupStatus(void);
void CanSendLampStatus(void);
static void SendLightCanMsg(LightData *ld)
{	
    CanTxMsg TxMessage;
	if (g_heart_count1 != g_heart_count2)
		return;
	TxMessage.StdId = 0x101;
    TxMessage.ExtId = 0;
    TxMessage.IDE = CAN_ID_STD;//
    TxMessage.RTR = CAN_RTR_DATA;//消息帧类型为数据帧 
    TxMessage.DLC = 7;//帧长度为7
	TxMessage.Data[0] = g_nOutPutMode;
	memcpy(&TxMessage.Data[1], ld->data, 6);
    CAN_TX1_SendData(&TxMessage);
	
	g_ArrayLedGroupStatusArray[0]= ld->channelset.ch1;
    g_ArrayLedGroupStatusArray[1]= ld->channelset.ch2;
    g_ArrayLedGroupStatusArray[2]= ld->channelset.ch3;
    g_ArrayLedGroupStatusArray[3]= ld->channelset.ch4;
    SetLedGroupStatus();//把自己的灯控板灯点亮
    CanSendLampStatus();//再将点灯信息发送到前面板，将前面板对应的LED阵列点亮
	IWDG_ReloadCounter();
}
#define IS_OWN_ACCESS(baseaddr) ((baseaddr) == gLCBconfig->controlBoardNo)
//点亮前面板按键的指示灯,用来表明主控板挂掉或者回复正常
#define ALL_BUTTON_LED	0x1f	//点亮所有按键LED,表明主控板挂掉,灯控板接管控制
#define AUTO_MANUAL_LED	0x03	//点亮自动和手动按键,表明检测到电压故障执行黄闪
#define YELLOW_RED_LED	0x0c	//点亮黄闪和全红按键,表明检测到电流故障执行黄闪
#define AUTO_BUTTON_LED	0x01	//点亮自动键LED,表示主控板恢复
static inline void LightFrontBoardButtonLed(uint8_t data)
{
    CanTxMsg TxMessage;
	if (IS_OWN_ACCESS(g_nBaseAddr))
	{
		TxMessage.StdId = 0x110;
		TxMessage.ExtId = 0;
		TxMessage.IDE = CAN_ID_STD;//
		TxMessage.RTR = CAN_RTR_DATA;//消息帧类型为数据帧 
		TxMessage.DLC = 1;//帧长度为1
		TxMessage.Data[0] = data;
		CAN_TX1_SendData(&TxMessage);
	}
}
//根据获取的红灯电流值转换为点灯bit值
static inline uint16_t RedCurConvert(uint8_t *data)
{
	Direction dir;
	
	dir.left = (data[0] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	dir.straight = (data[1] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	dir.right = (data[2] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	dir.pedestrian = (data[3] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	return *(uint16_t *)&dir;
}

#define MASK_YELLOW_VOLTBIT	0x6db	//每个方向黄灯灭的数据位B(011 011 011 011)=0x6db
//根据板子编号把转换的电压bit值和电流bit值写入到相应板子点灯的数据位
static inline void WriteBitsToLightData(uint8_t boardNo, 
	uint16_t voltBits, LightData *boardVolt,
	uint16_t curBits, LightData *redCur)
{
	if (boardNo == 0)
	{	//因为黄灯没有检测电压默认为1，因此屏蔽掉方便后续故障检测
		boardVolt->dirset.east = voltBits & MASK_YELLOW_VOLTBIT;
		redCur->dirset.east = curBits;
	}
	else if (boardNo == 1)
	{
		boardVolt->dirset.south = voltBits & MASK_YELLOW_VOLTBIT;
		redCur->dirset.south = curBits;
	}
	else if (boardNo == 2)
	{
		boardVolt->dirset.west = voltBits & MASK_YELLOW_VOLTBIT;
		redCur->dirset.west = curBits;
	}
	else if (boardNo == 3)
	{
		boardVolt->dirset.north = voltBits & MASK_YELLOW_VOLTBIT;
		redCur->dirset.north = curBits;
	}
}
/*	根据电流电压值转换为点灯值用来做检测用
	电压为1表示点亮，为0表示点灭
	电流小于设置的最小值即为点灭，反之则为点亮	*/
static inline void ConvertToLightData(CanRxMsg *pRxMsg)
{
	uint8_t boardNo = pRxMsg->ExtId & 0x7;
	uint16_t voltBits = (pRxMsg->ExtId >> 3) & 0xfff;
	uint16_t curBits = RedCurConvert(pRxMsg->Data);	//通过电流值转换得到的点灯bit值
	
	//printf("can recv boardNo:%d, voltbits: %#x\r\n", boardNo, voltBits);
	WriteBitsToLightData(boardNo - 1, voltBits, &gBoardVolt, curBits, &gRedCur);
}

extern void GetCurAverVal(void);
static inline uint16_t GetSelfCurBits(void)	//获取自身电流值转换的点灯bit值
{
	uint8_t i = 0, data[4] = {0};
    int16_t ndiff = 0;
	
	GetCurAverVal();
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

        data[i] = (uint8_t)ndiff;//红灯亮起时，其电流值应该大于10
    }
	return RedCurConvert(data);
}

extern uint16_t LampControlBoard_GetVoltage(void);
/*每DELAY_MSEC检测一次*/
#define ONESEC_CHECK_TIMES	(1000 / DELAY_MSEC)
#define MAX_ERROR_COUNT	(3 * ONESEC_CHECK_TIMES)		//总共错误的次数不能超过3s
#define MAX_RIGHT_COUNT ONESEC_CHECK_TIMES		//如果1s内检测都是正确的，则清除故障标志位
static void FaultCheck(DirectionSet *ds)	//用以红绿冲突和绿冲突的检测
{
	static uint8_t errorCount = 0;
	LightData voltcmp, curcmp;	//电压比较数据和电流比较数据
	LightData boardVolt = gBoardVolt, redCur = gRedCur;
	int retVoltCmp = 0, retCurCmp = 0;
	uint16_t voltBits = LampControlBoard_GetVoltage();	//获取自身的电压bit值
	uint16_t curBits = GetSelfCurBits();				//获取自身电流值转换的点灯bit值
	
	//memcpy(&boardVolt, &gBoardVolt, sizeof(LightData));
	//memcpy(&redCur, &gRedCur, sizeof(LightData));
	memset(&gBoardVolt, 0, sizeof(gBoardVolt));
	memset(&gRedCur, 0, sizeof(gRedCur));
	WriteBitsToLightData(g_nBaseAddr, voltBits, &boardVolt, curBits, &redCur);

	/*因为黄灯没有检测电压默认为1，因此屏蔽掉黄灯电压值，
	这样直接比较点灯值和电压值就可以同时检测红绿冲突和绿冲突了*/
	voltcmp.dirset.east = ds->east & MASK_YELLOW_VOLTBIT;
	voltcmp.dirset.south = ds->south & MASK_YELLOW_VOLTBIT;
	voltcmp.dirset.west = ds->west & MASK_YELLOW_VOLTBIT;
	voltcmp.dirset.north = ds->north & MASK_YELLOW_VOLTBIT;
	/*把实际的点灯值只保留红灯的点灯数值，用来比较红灯电流转换的点灯值判断红灯是否熄灭*/
	//每个方向的红灯点亮数值为B(010 010 010 010)=0x492
	curcmp.dirset.east = ds->east & 0x492;
	curcmp.dirset.south = ds->south & 0x492;
	curcmp.dirset.west = ds->west & 0x492;
	curcmp.dirset.north = ds->north & 0x492;
	retVoltCmp = memcmp(voltcmp.data, boardVolt.data, sizeof(voltcmp.data));
	retCurCmp = memcmp(curcmp.data, redCur.data, sizeof(curcmp.data));
	if ((gLCBconfig->baseinfo.isVoltCheckEnable == 1 
			&& gLCBconfig->baseinfo.isCurCheckEnable == 1
			&& retVoltCmp == 0 && retCurCmp == 0)	//电压电流检测都正常
		|| (gLCBconfig->baseinfo.isVoltCheckEnable == 1 
			&& gLCBconfig->baseinfo.isCurCheckEnable == 0
			&& retVoltCmp == 0)	//电压检测正常
		|| (gLCBconfig->baseinfo.isVoltCheckEnable == 0 
			&& gLCBconfig->baseinfo.isCurCheckEnable == 1
			&& retCurCmp == 0))	//电流检测正常
	{
		errorCount = 0;
	}
	else
	{
		errorCount++;
#if 0
		if (errorCount <= MAX_ERROR_COUNT)
		{
			printf("east: redCur:%#x, lightvalue:%#x\r\n", redCur.dirset.east, curcmp.dirset.east);
			printf("south: redCur:%#x, lightvalue:%#x\r\n", redCur.dirset.south, curcmp.dirset.south);
			printf("west: redCur:%#x, lightvalue:%#x\r\n", redCur.dirset.west, curcmp.dirset.west);
			printf("north: redCur:%#x, lightvalue:%#x\r\n", redCur.dirset.north, curcmp.dirset.north);
		}
#endif
	}
	if (errorCount >= MAX_ERROR_COUNT)
	{
		if (gLCBconfig->baseinfo.isVoltCheckEnable && retVoltCmp != 0)
			LightFrontBoardButtonLed(AUTO_MANUAL_LED);	//检测到电压故障时点亮自动和手动按键用以提示
		else if (gLCBconfig->baseinfo.isCurCheckEnable && retCurCmp != 0)
			LightFrontBoardButtonLed(YELLOW_RED_LED);	//检测到电流故障时点亮黄闪和全红按键用以提示
		gFaultFlag = 1;
		errorCount = 0;
	}
}
//执行定周期点灯
static void ExcuteFixedCycle(RunData *rd)
{
	int i, r, n = ONESEC_CHECK_TIMES;
	uint8_t motorValue, pedValue;
	LightData lightdata;
	PhaseLightValue tmp;
	
	memset(&lightdata, 0, sizeof(LightData));
	//初始化所有配置的通道全部为红灯
	SetMotorLightValue(&lightdata, LRED, gLCBconfig->allchannelbits);
	SetPedLightValue(&lightdata, LRED, gLCBconfig->allchannelbits);
	for (i = 0; i < n; i++)
	{
		for (r = 1; r <= 2; r++)
		{
			tmp = (r == 1) ? rd->ring1 : rd->ring2;
			if (tmp.phase == 0)
				break;
			if (tmp.lightvalue == LGREEN_FLASH_PED)
			{
				motorValue = LGREEN;
				pedValue = (i < n / 2) ? LOFF : LGREEN;
			}
			else if (tmp.lightvalue == LGREEN_FLASH)
			{
				pedValue = motorValue = (i < n / 2) ? LOFF : LGREEN;
			}
			else if (tmp.lightvalue == LYELLOW_FLASH)
			{
				motorValue = (i < n / 2) ? LOFF : LYELLOW;
				pedValue = LOFF;
			}
			else if (tmp.lightvalue == LYELLOW)
			{
				motorValue = LYELLOW;
				pedValue = LRED;
			}
			else
			{
				pedValue = motorValue = tmp.lightvalue;
			}
			SetMotorLightValue(&lightdata, motorValue, gLCBconfig->phaseinfo[tmp.phase - 1].channelbits);
			SetPedLightValue(&lightdata, pedValue, gLCBconfig->phaseinfo[tmp.phase - 1].channelbits);
		}
		SendLightCanMsg(&lightdata);
		
		HAL_Delay(DELAY_MSEC);
		IWDG_ReloadCounter();
		
		if (g_heart_count1 != g_heart_count2)
			return;
	
		if (gFaultFlag == 0 && (gLCBconfig->baseinfo.isVoltCheckEnable || gLCBconfig->baseinfo.isCurCheckEnable))
			FaultCheck(&lightdata.dirset);
		
		if ((i & 0x1) == 0)
			HAL_GPIO_TogglePin(GPIOA,GPIO_Pin_5);	//点亮发送指示灯
	}
}
//计算指定方案一个周期内运行的相位和状态,返回周期时间
static uint8_t CalRunData(uint8_t schemeid, RunData *rd)
{
	uint8_t *phases = gLCBconfig->phaseturninfo[schemeid].phases;
	uint8_t *times = gLCBconfig->splitinfo[schemeid].times;
	uint8_t splitTime, starttime, runtime = 0;
	PhaseLightValue tmp;
	int r, i, ret;
	
	memset(rd, 0, sizeof(RunData) * MAX_RUNDATA_NUM);
	while (1)
	{
		ret = 0;
		for (r = 1; r <= 2; r++)	//支持双环
		{
			starttime = 0;
			for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
			{	//相序信息的每个字节中低4bit为环1的相位，高4bit为环2的相位
				tmp.phase = (r == 1) ? (phases[i] & 0xf) : (phases[i] >> 4);
				if (tmp.phase == 0)
					break;
				splitTime = times[tmp.phase - 1];
				if (runtime < starttime + splitTime)
				{
					tmp.lightvalue = GetPhaseLightValue(splitTime, runtime - starttime, &gLCBconfig->phaseinfo[tmp.phase - 1]);
					ret = 1;
					if (r == 1)
						rd->ring1 = tmp;
					else
						rd->ring2 = tmp;
					break;
				}
				else
					starttime += splitTime;
			}
		}
		if (ret == 0)
			break;
		runtime++;
		rd++;
	}
	return runtime;
}
//找到无缝接管的位置
static uint8_t FindTakeOverPosition(LCBruninfo *runinfo, RunData *rd)
{
	int index;
	LCBruninfo tmp;

	memset(&tmp, 0, sizeof(tmp));
	tmp.schemeid = 1;
	if (memcmp(runinfo, &tmp, sizeof(LCBruninfo)) == 0)
		return 0;	//说明是一上来就直接接管默认执行方案1
	for (index = runinfo->runtime; index >= 0; index--)
	{	//当运行相位号以及点灯值都一样时则返回对应的index+1
		if (rd[index].ring1.phase == runinfo->phaseR1
			&& rd[index].ring1.lightvalue == runinfo->lightvalueR1
			&& rd[index].ring2.phase == runinfo->phaseR2
			&& rd[index].ring2.lightvalue == runinfo->lightvalueR2)
			return index + 1;	//接管后从下1s开始运行
	}
	IWDG_ReloadCounter();
	if (runinfo->lightvalueR1 == LGREEN || runinfo->lightvalueR2 == LGREEN)
	{	/*如果当前运行相位是绿灯值，由于在协调控制时过渡期会延长绿灯时间，这样可能导致原本机动车相位运行时
		  行人是直接绿闪的，然而由于过度增加了绿灯时间导致行人灯和机动车有一段的绿灯时间，但是灯控板计算的
		  是定周期没有这段机动车和行人一起绿灯的时段，因此可能找不到相匹配的index，此时就从头判断如果主控
		  板传递过来的是机动车和行人都是绿灯，而灯控板计算得出机动车绿灯行人绿闪这样也是可以匹配的*/
		for (index = 0; index <= runinfo->runtime; index++)
		{
			if (rd[index].ring1.phase == runinfo->phaseR1
				&& (rd[index].ring1.lightvalue == runinfo->lightvalueR1
					|| (rd[index].ring1.lightvalue == LGREEN_FLASH_PED
						&& runinfo->lightvalueR1 == LGREEN))
				&& rd[index].ring2.phase == runinfo->phaseR2
				&& (rd[index].ring2.lightvalue == runinfo->lightvalueR2
					|| (rd[index].ring2.lightvalue == LGREEN_FLASH_PED
						&& runinfo->lightvalueR2 == LGREEN)))
			return index;
		}
	}
	printf("!!!! Can't find position to take over the main control board !!!!\r\n");
	printf("runtime:%d, ring1:%d, %d, ring2:%d, %d\r\n", runinfo->runtime,
		rd[runinfo->runtime].ring1.phase, rd[runinfo->runtime].ring1.lightvalue,
		rd[runinfo->runtime].ring2.phase, rd[runinfo->runtime].ring2.lightvalue);
	printf("phaseR1:%d, lightvalue:%d, phaseR2:%d, lightvalue:%d\r\n", 
		runinfo->phaseR1, runinfo->lightvalueR1, runinfo->phaseR2, runinfo->lightvalueR2);
	return 0xff;	//返回一个无效的位置
}
//特殊控制点灯
static void SpecialControlLight(LightValue tmp)
{
	int i, n = ONESEC_CHECK_TIMES;
	uint8_t value;
	LightData lightdata;
	
	memset(&lightdata, 0, sizeof(LightData));
	//先判断此块灯控板是否允许接管执行点灯命令,若不允许则不执行点灯
	if (IS_OWN_ACCESS(g_nBaseAddr))
	{
		for (i = 0; i < n; i++)
		{
			if (tmp == LGREEN_FLASH)
				value = (i < n / 2) ? LOFF : LGREEN;
			else if (tmp == LYELLOW_FLASH)
				value = (i < n / 2) ? LOFF : LYELLOW;
			else
				value = tmp;
			SetMotorLightValue(&lightdata, value, gLCBconfig->allchannelbits);
			if (value == LRED)	//全红时点亮行人灯
				SetPedLightValue(&lightdata, LRED, gLCBconfig->allchannelbits);
			SendLightCanMsg(&lightdata);
			
			HAL_Delay(DELAY_MSEC);
			IWDG_ReloadCounter();
			
			if ((i & 0x1) == 0)
				HAL_GPIO_TogglePin(GPIOA,GPIO_Pin_5);	//点亮发送指示灯
		}
	}
}
#if 0
//点灯1s
static void LightOneSecond(uint8_t schemeid, RunData *rd)
{
	if (schemeid >= 0 && schemeid <= MAX_SUPPORT_SCHEME_NUM)
		ExcuteFixedCycle(rd);	//执行定周期
	else if (schemeid == 255)
		SpecialControlLight(LYELLOW_FLASH);		//黄闪控制
	else if (schemeid == 252)
		SpecialControlLight(LRED);				//全红控制
	else if (schemeid == 251)
		SpecialControlLight(LOFF);				//关灯控制
}
#endif

#endif
