#include <stdlib.h>
#include <unistd.h>
#include "hik.h"
#include "its.h"
#include "LogSystem.h"
#include "common.h"
#include "configureManagement.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_FAULT_OCCUR_COUNT	12
#define GET_RED_VAL(v) 			((v >> 1) & 1)
#define GET_GREEN_VAL(v) 		(v & 1)
#define GET_GREEN_RED_VAL(v) 	(v & 0x3)

/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
static int g_faultstatus[32] = {0}; //通道故障记录用于恢复
static char gFaultFlag = 0;	//故障检测标志,0:没有故障,1:有故障

UInt8 gRedCurrentValue[32];
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;

extern void i_can_its_get_Volt(int boardNum, unsigned short *pboardInfo);
extern unsigned short i_can_its_get_cur(int boardNum, int pahseNum, int redGreen);
extern void Hiktsc_Running_Status(void);

void ReadFaultStatus()
{
	READ_BIN_CFG_PARAMS("/home/faultstatus.dat", g_faultstatus, sizeof(g_faultstatus));
}

void WriteFaultStatus()
{
	WRITE_BIN_CFG_PARAMS("/home/faultstatus.dat", g_faultstatus, sizeof(g_faultstatus));
}

/*****************************************************************************
 函 数 名  : get_lamp_value
 功能描述  : 主要用来获取一组灯中具体某个灯的状态值
 输入参数  : UInt16 *lights  描述一组灯状态的指针
             int n                            具体是哪个灯，只能是0、1、2、3
 返 回 值  : 返回某个灯的状态值
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline UInt16 get_lamp_value(UInt16 *lights, int n)
{
	lamp_t *p = (lamp_t *)(lights);
	UInt16 value = 0;
	switch (n) 
	{
		case 0:	value = p->L0; break;
		case 1:	value = p->L1; break;
		case 2:	value = p->L2; break;
		case 3:	value = p->L3; break;
		default: break;
	}
	
	return value;
}
//绿冲突时要显示出与哪些通道冲突了
static inline int SetGreenConflictValue(UInt16 *lightValues, int conflictChannel)
{
	UInt16 setval;
	int i, j, channel;
	int n = 1, value = conflictChannel & 0x1f;

	for (i = 0; i < MAX_BOARD_NUM; i++)
	{
		for (j = 0; j < 4; j++)
		{
			channel = i * 4 + j + 1;
			setval = get_lamp_value(lightValues + i, j);
			if (GET_GREEN_VAL(setval) == 1)
			{
				value |= ((channel & 0x1f) << (n * 5));
				if (++n == 6)	//32bit,每5bit一个通道,最大只能容纳6个通道
					return value;
			}
		}
	}
	return value;
}

static void RedgreenCollision(UInt16 *lightValues)
{
	static int errorCountRed[NUM_CHANNEL] = {0};	//记录红绿冲突的次数
	static int normalCountRed[NUM_CHANNEL] = {0};	//记录红绿正常的次数
	static int errorCountGreen[NUM_CHANNEL] = {0};	//记录绿冲突的次数
	static int normalCountGreen[NUM_CHANNEL] = {0};	//记录绿正常的次数
	UInt16 boardInfo, setval, realval;
	int i, j, channel;

	for (i = 0; i < MAX_BOARD_NUM; i++)
	{
		i_can_its_get_Volt(i + 1, &boardInfo);
		for (j = 0; j < 4; j++)
		{
			channel = i * 4 + j + 1;
			setval = get_lamp_value(lightValues + i, j);
			realval = get_lamp_value(&boardInfo, j);
			//红绿冲突检测
			if ((GET_RED_VAL(setval) == 0 && GET_RED_VAL(realval) == 1)
				|| (GET_GREEN_RED_VAL(realval) == 0x3))
			{	//红灯不该亮而亮或者红绿同时点亮则为红绿冲突
				errorCountRed[channel - 1]++;
				normalCountRed[channel - 1] = 0;
				if (GET_BIT(g_faultstatus[channel - 1], 2) == 0
					&& errorCountRed[channel - 1] == MAX_FAULT_OCCUR_COUNT)
				{
					if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
						ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
					log_error("第%d通道红绿冲突", channel);
					ItsWriteFaultLog(RED_GREEN_CONFLICT, channel);
					SET_BIT(g_faultstatus[channel - 1], 2);
					gFaultFlag = 1;
					WriteFaultStatus();
				}
#if 0
				else if (errorCountRed[channel - 1] < MAX_FAULT_OCCUR_COUNT)
					INFO("第%d通道红灯电压异常(红绿冲突征兆), setval:%#x, realval:%#x", channel, setval, realval);
#endif
			}
			else if ((GET_RED_VAL(setval) == 0 && GET_RED_VAL(realval) == 0)
					|| (GET_GREEN_RED_VAL(realval) != 0x3))
			{
				normalCountRed[channel - 1]++;
				errorCountRed[channel - 1] = 0;
				if (GET_BIT(g_faultstatus[channel - 1], 2) == 1
					&& normalCountRed[channel - 1] == MAX_FAULT_OCCUR_COUNT)
				{
					log_debug("第%d通道红绿冲突解除", channel);
					ItsWriteFaultLog(RED_GREEN_CONFLICT_CLEAR, channel);
					CLR_BIT(g_faultstatus[channel - 1], 2);
					WriteFaultStatus();
				}
			}
			//绿冲突检测
			if (GET_GREEN_VAL(setval) == 0 && GET_GREEN_VAL(realval) == 1)
			{	//绿灯不该亮而亮则为绿冲突
				errorCountGreen[channel - 1]++;
				normalCountGreen[channel - 1] = 0;
				if (GET_BIT(g_faultstatus[channel - 1], 1) == 0
					&& errorCountGreen[channel - 1] == MAX_FAULT_OCCUR_COUNT)
				{
					if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
						ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
					log_error("第%d通道绿冲突", channel);
					ItsWriteFaultLog(GREEN_CONFLICT, SetGreenConflictValue(lightValues, channel));
					SET_BIT(g_faultstatus[channel - 1], 1);
					gFaultFlag = 1;
					WriteFaultStatus();
				}
#if 0
				else if (errorCountGreen[channel - 1] < MAX_FAULT_OCCUR_COUNT)
					INFO("第%d通道绿灯电压异常(绿冲突征兆), setval:%#x, realval:%#x", channel, setval, realval);
#endif
			}
			else if (GET_GREEN_VAL(setval) == 0 && GET_GREEN_VAL(realval) == 0)
			{
				normalCountGreen[channel - 1]++;
				errorCountGreen[channel - 1] = 0;
				if (GET_BIT(g_faultstatus[channel - 1], 1) == 1
					&& normalCountGreen[channel - 1] == MAX_FAULT_OCCUR_COUNT)
				{
					log_debug("第%d通道绿冲突解除", channel);
					ItsWriteFaultLog(GREEN_CONFLICT_CLEAR, channel);
					CLR_BIT(g_faultstatus[channel - 1], 1);
					WriteFaultStatus();
				}
			}
		}
	}
}

static void RedExtinguish(UInt16 *lightValues) //红灯熄灭检测
{
	static int errorCountRed[NUM_CHANNEL] = {0};	//记录红灯熄灭的次数
	static int normalCountRed[NUM_CHANNEL] = {0};	//记录红灯正常的次数
	UInt16 pcurInfo, setval, readval;
	int i, j, channel;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pcurInfo = i_can_its_get_cur(i+1, j+1, 1);   //第３个参数写死１,只获取红灯电流
			channel = i * 4 + j + 1;
			gRedCurrentValue[channel - 1] = pcurInfo;
#if 1
			if (channel == 16)
				return;
#endif
			setval = get_lamp_value(lightValues + i, j);
			if (GET_RED_VAL(setval) > 0 && pcurInfo < (gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentDiff))
			{	//红灯该亮而不亮则为红灯熄灭
				errorCountRed[channel - 1]++;
				normalCountRed[channel - 1] = 0;
				if (GET_BIT(g_faultstatus[channel - 1], 3) == 0
					&& errorCountRed[channel - 1] == MAX_FAULT_OCCUR_COUNT)
				{
					if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch == 1)
						ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
					log_error("第%d通道红灯熄灭", channel);
					ItsWriteFaultLog(RED_LIGHT_OFF, channel);
					SET_BIT(g_faultstatus[channel - 1], 3);
					gFaultFlag = 1;
					WriteFaultStatus();
				}
#if 0
				else if (errorCountRed[channel - 1] < MAX_FAULT_OCCUR_COUNT)
					INFO("通道%d红灯亮并且检测电流异常(红灯熄灭征兆)", channel);
#endif
			}
			else if (GET_RED_VAL(setval) > 0 && pcurInfo > (gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentDiff))
			{
				normalCountRed[channel - 1]++;
				errorCountRed[channel - 1] = 0;
				if (GET_BIT(g_faultstatus[channel - 1], 3) == 1
					&& normalCountRed[channel - 1] == MAX_FAULT_OCCUR_COUNT)
				{
					log_debug("第%d通道红灯熄灭解除", channel);
					ItsWriteFaultLog(RED_LIGHT_OFF_CLEAR, channel);
					CLR_BIT(g_faultstatus[channel - 1], 3);
					WriteFaultStatus();
				}
			}
		}
	}
}

void ItsFaultCheck(UInt16 *lightValues)
{
	static char count = 1;
	//主控板运行指示灯
	Hiktsc_Running_Status();
	if (count <= 48)	//启动时的6s黄闪和全红不进行检测,1s检测4次,48为12s
	{
		count++;
		return;
	}
	if (gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 0
		|| gFaultFlag == 1)
		return;	//故障检测总开关未开或者已有故障发生则不再检测
	if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmSwitch == 1 && gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 1)
		RedgreenCollision(lightValues);	//红绿冲突检测
	if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmSwitch == 1 && gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 1)
		RedExtinguish(lightValues); //红灯熄灭检测
}

