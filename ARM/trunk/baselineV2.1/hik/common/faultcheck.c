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
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_FAULT_OCCUR_COUNT	12
#define GET_RED_VAL(v) 			((v >> 1) & 1)
#define GET_GREEN_VAL(v) 		(v & 1)
#define GET_GREEN_RED_VAL(v) 	(v & 0x3)

//��¼����״̬��bitλ
#define GREEN_CONFLICT_BIT		1
#define RED_GREEN_CONFLICT_BIT	2
#define RED_LIGHT_OFF_BIT		3
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
static int g_faultstatus[32] = {0}; //ͨ�����ϼ�¼���ڻָ�
static char gFaultFlag = 0;	//���ϼ���־,0:û�й���,1:�й���

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
 �� �� ��  : get_lamp_value
 ��������  : ��Ҫ������ȡһ����о���ĳ���Ƶ�״ֵ̬
 �������  : UInt16 *lights  ����һ���״̬��ָ��
             int n                            �������ĸ��ƣ�ֻ����0��1��2��3
 �� �� ֵ  : ����ĳ���Ƶ�״ֵ̬
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
//�̳�ͻʱҪ��ʾ������Щͨ����ͻ��
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
				if (++n == 6)	//32bit,ÿ5bitһ��ͨ��,���ֻ������6��ͨ��
					return value;
			}
		}
	}
	return value;
}

static void RedgreenCollision(UInt16 *lightValues)
{
	static int errorCountRed[NUM_CHANNEL] = {0};	//��¼���̳�ͻ�Ĵ���
	static int normalCountRed[NUM_CHANNEL] = {0};	//��¼���������Ĵ���
	static int errorCountGreen[NUM_CHANNEL] = {0};	//��¼�̳�ͻ�Ĵ���
	static int normalCountGreen[NUM_CHANNEL] = {0};	//��¼�������Ĵ���
	UInt8 setval, realval;
	int i, channel;

	if (lightValues == NULL)
		return;
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		channel = i + 1;
		setval = (lightValues[i / 4] >> ((i % 4) * 3)) & 0x7;
		realval = GetChannelVoltage(channel);
		//���̳�ͻ���
		if ((GET_RED_VAL(setval) == 0 && GET_RED_VAL(realval) == 1)
			|| (GET_GREEN_RED_VAL(realval) == 0x3))
		{	//��Ʋ������������ߺ���ͬʱ������Ϊ���̳�ͻ
			errorCountRed[i]++;
			normalCountRed[i] = 0;
			if (GET_BIT(g_faultstatus[i], RED_GREEN_CONFLICT_BIT) == 0
				&& errorCountRed[i] == MAX_FAULT_OCCUR_COUNT)
			{
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
				log_error("��%dͨ�����̳�ͻ", channel);
				ItsWriteFaultLog(RED_GREEN_CONFLICT, channel);
				SET_BIT(g_faultstatus[i], RED_GREEN_CONFLICT_BIT);
				gFaultFlag = 1;
				WriteFaultStatus();
			}
#if 0
			else if (errorCountRed[i] < MAX_FAULT_OCCUR_COUNT)
				INFO("��%dͨ����Ƶ�ѹ�쳣(���̳�ͻ����), setval:%#x, realval:%#x", channel, setval, realval);
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
				log_debug("��%dͨ�����̳�ͻ���", channel);
				ItsWriteFaultLog(RED_GREEN_CONFLICT_CLEAR, channel);
				CLR_BIT(g_faultstatus[i], RED_GREEN_CONFLICT_BIT);
				WriteFaultStatus();
			}
		}
		//�̳�ͻ���
		if (GET_GREEN_VAL(setval) == 0 && GET_GREEN_VAL(realval) == 1)
		{	//�̵Ʋ�����������Ϊ�̳�ͻ
			errorCountGreen[i]++;
			normalCountGreen[i] = 0;
			if (GET_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT) == 0
				&& errorCountGreen[i] == MAX_FAULT_OCCUR_COUNT)
			{
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
				log_error("��%dͨ���̳�ͻ", channel);
				ItsWriteFaultLog(GREEN_CONFLICT, SetGreenConflictValue(lightValues, channel));
				SET_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT);
				gFaultFlag = 1;
				WriteFaultStatus();
			}
#if 0
			else if (errorCountGreen[i] < MAX_FAULT_OCCUR_COUNT)
				INFO("��%dͨ���̵Ƶ�ѹ�쳣(�̳�ͻ����), setval:%#x, realval:%#x", channel, setval, realval);
#endif
		}
		else if (GET_GREEN_VAL(setval) == 0 && GET_GREEN_VAL(realval) == 0)
		{
			normalCountGreen[i]++;
			errorCountGreen[i] = 0;
			if (GET_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT) == 1
				&& normalCountGreen[i] == MAX_FAULT_OCCUR_COUNT)
			{
				log_debug("��%dͨ���̳�ͻ���", channel);
				ItsWriteFaultLog(GREEN_CONFLICT_CLEAR, channel);
				CLR_BIT(g_faultstatus[i], GREEN_CONFLICT_BIT);
				WriteFaultStatus();
			}
		}
	}
}

static void RedExtinguish(UInt16 *lightValues) //���Ϩ����
{
	static int errorCountRed[NUM_CHANNEL] = {0};	//��¼���Ϩ��Ĵ���
	static int normalCountRed[NUM_CHANNEL] = {0};	//��¼��������Ĵ���
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
		{	//��Ƹ�����������Ϊ���Ϩ��
			errorCountRed[i]++;
			normalCountRed[i] = 0;
			if (GET_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT) == 0
				&& errorCountRed[i] == MAX_FAULT_OCCUR_COUNT)
			{
				if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch == 1)
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
				log_error("��%dͨ�����Ϩ��", channel);
				ItsWriteFaultLog(RED_LIGHT_OFF, channel);
				SET_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT);
				gFaultFlag = 1;
				WriteFaultStatus();
			}
#if 0
			else if (errorCountRed[i] < MAX_FAULT_OCCUR_COUNT)
				INFO("ͨ��%d��������Ҽ������쳣(���Ϩ������)", channel);
#endif
		}
		else if (GET_RED_VAL(setval) > 0 && realval > (gStructBinfileConfigPara.sCurrentParams[i].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i].RedCurrentDiff))
		{
			normalCountRed[i]++;
			errorCountRed[i] = 0;
			if (GET_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT) == 1
				&& normalCountRed[i] == MAX_FAULT_OCCUR_COUNT)
			{
				log_debug("��%dͨ�����Ϩ����", channel);
				ItsWriteFaultLog(RED_LIGHT_OFF_CLEAR, channel);
				CLR_BIT(g_faultstatus[i], RED_LIGHT_OFF_BIT);
				WriteFaultStatus();
			}
		}
	}
}

#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
void ItsFaultCheck(UInt16 *lightValues)
{
	static char count = 1;
	//���ذ�����ָʾ��
	Hiktsc_Running_Status();
	if (count <= 48)	//����ʱ��6s������ȫ�첻���м��,1s���4��,48Ϊ12s
	{
		count++;
		return;
	}
	if (gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 0
		|| gFaultFlag == 1)
		return;	//���ϼ���ܿ���δ���������й��Ϸ������ټ��
	if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmSwitch == 1 && gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 1)
		RedgreenCollision(lightValues);	//���̳�ͻ���
	if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmSwitch == 1 && gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 1)
		RedExtinguish(lightValues); //���Ϩ����
}
#endif
