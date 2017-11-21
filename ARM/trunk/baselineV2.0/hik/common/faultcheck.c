#include <stdlib.h>
#include <unistd.h>
#include "hik.h"
#include "its.h"
#include "LogSystem.h"
#include "common.h"
#include "configureManagement.h"
#include "sqlite3.h"
#include "sqlite_conf.h"
#include "canmsg.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_FAULT_OCCUR_COUNT	12
#define GET_RED_VAL(v) 			((v >> 1) & 1)
#define GET_GREEN_VAL(v) 		(v & 1)
#define GET_GREEN_RED_VAL(v) 	(v & 0x3)

//记录故障状态的bit位
#define GREEN_CONFLICT_BIT		1
#define RED_GREEN_CONFLICT_BIT	2
#define RED_LIGHT_OFF_BIT		3
/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
static int g_faultstatus[32] = {0}; //通道故障记录用于恢复
static char gFaultFlag = 0;	//故障检测标志,0:没有故障,1:有故障

UInt8 gRedCurrentValue[32];
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;

extern void ItsGetBoardVoltage(int boardNum, unsigned short *pboardInfo);
extern UInt8 ItsGetLightCurrent(int boardNum, int number);
extern void Hiktsc_Running_Status(void);

void ReadFaultStatus()
{
	sqlite3* pdatabase = NULL;

	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
	sqlite3_select_faultstatus(pdatabase, g_faultstatus);
	sqlite3_close_wrapper(pdatabase);
	pdatabase = NULL;
	//READ_BIN_CFG_PARAMS("/home/faultstatus.dat", g_faultstatus, sizeof(g_faultstatus));
}

void WriteFaultStatus()
{
	sqlite3* pdatabase = NULL;

	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
	sqlite3_insert_faultstatus(pdatabase, g_faultstatus);
	sqlite3_close_wrapper(pdatabase);
	pdatabase = NULL;
	//WRITE_BIN_CFG_PARAMS("/home/faultstatus.dat", g_faultstatus, sizeof(g_faultstatus));
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
	UInt8 setval, realval;
	int i, channel;

	if (lightValues == NULL)
		return;
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		channel = i + 1;
		setval = (lightValues[i / 4] >> ((i % 4) * 3)) & 0x7;
		realval = GetChannelVoltage(channel);
		//红绿冲突检测
		if ((GET_RED_VAL(setval) == 0 && GET_RED_VAL(realval) == 1)
			|| (GET_GREEN_RED_VAL(realval) == 0x3))
		{	//红灯不该亮而亮或者红绿同时点亮则为红绿冲突
			errorCountRed[i]++;
			normalCountRed[i] = 0;
			if (GET_BIT(g_faultstatus[i], RED_GREEN_CONFLICT_BIT) == 0
				&& errorCountRed[i] == MAX_FAULT_OCCUR_COUNT)
			{
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
				log_error("第%d通道红绿冲突", channel);
				ItsWriteFaultLog(RED_GREEN_CONFLICT, channel);
				SET_BIT(g_faultstatus[i], RED_GREEN_CONFLICT_BIT);
				gFaultFlag = 1;
				WriteFaultStatus();
			}
#if 0
			else if (errorCountRed[i] < MAX_FAULT_OCCUR_COUNT)
				INFO("第%d通道红灯电压异常(红绿冲突征兆), setval:%#x, realval:%#x", channel, setval, realval);
#endif
		}
		else if ((GET_RED_VAL(setval) == 0 && GET_RED_VAL(realval) == 0)
				|| (GET_GREEN_RED_VAL(realval) != 0x3))
		{
			normalCountRed[i]++;
			errorCountRed[i] = 0;
			if (GET_BIT(g_faultstatus[i], RED_GREEN_CONFLICT_BIT) == 1
				&& normalCountRed[i] == MAX_FAULT_OCCUR_COUNT)
			{
				log_debug("第%d通道红绿冲突解除", channel);
				ItsWriteFaultLog(RED_GREEN_CONFLICT_CLEAR, channel);
				CLR_BIT(g_faultstatus[i], RED_GREEN_CONFLICT_BIT);
				WriteFaultStatus();
			}
		}
		//绿冲突检测
		if (GET_GREEN_VAL(setval) == 0 && GET_GREEN_VAL(realval) == 1)
		{	//绿灯不该亮而亮则为绿冲突
			errorCountGreen[i]++;
			normalCountGreen[i] = 0;
			if (GET_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT) == 0
				&& errorCountGreen[i] == MAX_FAULT_OCCUR_COUNT)
			{
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
				log_error("第%d通道绿冲突", channel);
				ItsWriteFaultLog(GREEN_CONFLICT, SetGreenConflictValue(lightValues, channel));
				SET_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT);
				gFaultFlag = 1;
				WriteFaultStatus();
			}
#if 0
			else if (errorCountGreen[i] < MAX_FAULT_OCCUR_COUNT)
				INFO("第%d通道绿灯电压异常(绿冲突征兆), setval:%#x, realval:%#x", channel, setval, realval);
#endif
		}
		else if (GET_GREEN_VAL(setval) == 0 && GET_GREEN_VAL(realval) == 0)
		{
			normalCountGreen[i]++;
			errorCountGreen[i] = 0;
			if (GET_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT) == 1
				&& normalCountGreen[i] == MAX_FAULT_OCCUR_COUNT)
			{
				log_debug("第%d通道绿冲突解除", channel);
				ItsWriteFaultLog(GREEN_CONFLICT_CLEAR, channel);
				CLR_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT);
				WriteFaultStatus();
			}
		}
	}
}

static void RedExtinguish(UInt16 *lightValues) //红灯熄灭检测
{
	static int errorCountRed[NUM_CHANNEL] = {0};	//记录红灯熄灭的次数
	static int normalCountRed[NUM_CHANNEL] = {0};	//记录红灯正常的次数
	UInt8 setval, realval;
	int i, channel;

	if (lightValues == NULL)
		return;
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		channel = i + 1;
		realval = GetChannelRedCurrent(channel);
		gRedCurrentValue[i] = realval;
		setval = (lightValues[i / 4] >> ((i % 4) * 3)) & 0x7;
		if (GET_RED_VAL(setval) > 0 && realval < (gStructBinfileConfigPara.sCurrentParams[i].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i].RedCurrentDiff))
		{	//红灯该亮而不亮则为红灯熄灭
			errorCountRed[i]++;
			normalCountRed[i] = 0;
			if (GET_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT) == 0
				&& errorCountRed[i] == MAX_FAULT_OCCUR_COUNT)
			{
				if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch == 1)
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
				log_error("第%d通道红灯熄灭", channel);
				ItsWriteFaultLog(RED_LIGHT_OFF, channel);
				SET_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT);
				gFaultFlag = 1;
				WriteFaultStatus();
			}
#if 0
			else if (errorCountRed[i] < MAX_FAULT_OCCUR_COUNT)
				INFO("通道%d红灯亮并且检测电流异常(红灯熄灭征兆)", channel);
#endif
		}
		else if (GET_RED_VAL(setval) > 0 && realval > (gStructBinfileConfigPara.sCurrentParams[i].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i].RedCurrentDiff))
		{
			normalCountRed[i]++;
			errorCountRed[i] = 0;
			if (GET_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT) == 1
				&& normalCountRed[i] == MAX_FAULT_OCCUR_COUNT)
			{
				log_debug("第%d通道红灯熄灭解除", channel);
				ItsWriteFaultLog(RED_LIGHT_OFF_CLEAR, channel);
				CLR_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT);
				WriteFaultStatus();
			}
		}
	}
}

#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
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
#endif
