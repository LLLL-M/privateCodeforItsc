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
	uint16_t left:3;		//��ת
	uint16_t straight:3;	//ֱ��
	uint16_t right:3;		//��ת
	uint16_t pedestrian:3;	//����
} Direction;

typedef struct _direction_set_
{
	uint64_t east:12;		//������
	uint64_t south:12;		//�Ϸ���
	uint64_t west:12;		//������
	uint64_t north:12;		//������
} DirectionSet;

typedef struct _channel_set_
{
	uint64_t ch1:3;			//ͨ��1
	uint64_t ch2:3;			//ͨ��2
	uint64_t ch3:3;			//ͨ��3
	uint64_t ch4:3;			//ͨ��4
	uint64_t ch5:3;			//ͨ��5
	uint64_t ch6:3;			//ͨ��6
	uint64_t ch7:3;			//ͨ��7
	uint64_t ch8:3;			//ͨ��8
	uint64_t ch9:3;			//ͨ��9
	uint64_t ch10:3;		//ͨ��10
	uint64_t ch11:3;		//ͨ��11
	uint64_t ch12:3;		//ͨ��12
	uint64_t ch13:3;		//ͨ��13
	uint64_t ch14:3;		//ͨ��14
	uint64_t ch15:3;		//ͨ��15
	uint64_t ch16:3;		//ͨ��16
} ChannelSet;

typedef union _light_data_
{
	DirectionSet dirset;	//���򼯺�
	//Data64 data64;
	ChannelSet channelset;	//ͨ������
	uint8_t data[6];		//�������
} LightData;

typedef struct _phase_lightvalue_
{
	uint8_t lightvalue:4;	//������λ�ĵ��ֵ���ο�enum LightValue
	uint8_t phase:4;		//���е���λ��
} PhaseLightValue;
typedef struct _run_data_
{	//ÿ�����ڵ�ǰ1s���е���λ�Լ�����״̬
	PhaseLightValue ring1;
	PhaseLightValue ring2;
} RunData;
#define MAX_RUNDATA_NUM		256

#define GET_BIT(v, n) (((v) >> (n)) & 0x1)		//ȡv�ĵ� n bitλ
static LightData gBoardVolt;	//can���ܵĵƿذ��ѹֵ,0:�޵�ѹ��1:�е�ѹ
static LightData gRedCur;		//��Ƶ�����Ӧ�ĵ��ֵ��������<=10�����Ϊ0��������>10�����Ϊ1
static uint8_t gFaultFlag = 0;	//���ϱ�־��0:û�й��ϣ�1:�й���
static LCBruninfo gRuninfo;		//���ذ�ÿ�뷢�͸��ƿذ��������Ϣ

#define LCB_CONFIG_ADDR		(0x0801FC00)	//���û����õ�flash��ַ���һҳ������������

static LCBconfig gLCBconfig[2];	//[0]:��������ʱʹ��,[1]:������������ʱ������
/*	Ĭ������Ϊ	*/	
static LCBconfig gDefaultConfig = {
	/* �ӹܿ���:����, ��ѹ���:�ر�, �������:�ر�, ��λ����:4, ��С��Ƶ���ֵ:20, ������:1 */
	.baseinfo = {1, 0, 0, 4, 20, 1, 0},
	.phaseinfo = {
		[0] = {3, 3, 2, 3, 0x8686},	//��λ1:����ֱ��,ʱ��20s,��Ӧͨ��2,3,10,11,8,16
		[1] = {3, 3, 2, 6, 0x101},	//��λ2:������ת,ʱ��15s,��Ӧͨ��1,9
		[2] = {3, 3, 2, 6, 0x6868},	//��λ3:�ϱ�ֱ��,ʱ��20s,��Ӧͨ��6,7,14,15,4,12
		[3] = {3, 3, 2, 6, 0x1010},	//��λ4:�ϱ���ת,ʱ��15s,��Ӧͨ��5,13
	},	//ÿ����λͳһ����3s,�Ƶ�3s,ȫ��2s,��������6s
	.phaseturninfo = {
		[0] = {	//����Ϊ:��λ1-->��λ2-->��λ3-->��λ4
			.phases = {1, 2, 3, 4, 0, 0, 0, 0},
		},
	},
	.splitinfo = {
		[0] = {	//��Ӧ�����ű�Ϊ:20s,15s,20s,15s
			.times = {20, 15, 20, 15, 0, 0, 0, 0},
		},
	},
	.configstate = CONFIG_IS_READY,
	.controlBoardNo = 0,				//Ĭ��Ϊ��һ��ƿذ���п���
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
    
	//�Ȳ�����ǰ������
	if (FLASH_ErasePage((uint32_t)flashAddr) != FLASH_COMPLETE)	//����ʧ��
		printf("erase flash fail\r\n");
	else
	{	//�����ɹ���ѽ��յ����ݴ���flash��
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
	//��flash�ж�ȡ���ô����ȫ�ֱ���gLCBconfig[0]��
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
	static uint8_t reqno = 0;			//��һ������ı��
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
				if (extid->index == 1)	//����������1����Ϣ,����һ����Ϊ��Ӧ���Ƶ�����
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
				{	/*˵�����յ�һ������������,���һ��can���ݰ�no����canTotalNum,��index=0�����Ǹ�Ӧ���Ƶ����ű���Ϣ*/
					config->configstate = CONFIG_IS_READY;
					SetAllChannelBits(config);
					if (memcmp(config, gLCBconfig, sizeof(LCBconfig)) != 0 && CheckLCBconfigValidity(config))
					{	//�����ظ�������,���Ҽ��������Ч,�����д�뵽flash��
						WriteFlashData(LCB_CONFIG_ADDR, config, sizeof(LCBconfig));
						gLCBconfig->configstate = CONFIG_UPDATE;	//���������Ѿ�����,ִ�ж�����ʱӦ�ö�ȡ�µ�����
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
	
	if (runtime < greenTime)//�̵�ʱ��,��ʱ�������������ͨ���������˿���Ϊ����
		value = (runtime >= pedGreenTime && (phaseinfo->channelbits & 0x8888)) ? LGREEN_FLASH_PED : LGREEN;
	else if (runtime >= greenTime && runtime < greenTime + phaseinfo->greenFlashTime)
		value = LGREEN_FLASH;	//����ʱ��
	else if (runtime >= greenTime + phaseinfo->greenFlashTime 
			&& runtime < greenTime + phaseinfo->greenFlashTime + phaseinfo->yellowTime)
		value = LYELLOW;	//�Ƶ�ʱ��
	return value;
}
//��������ͨ�����ֵ
static void SetPedLightValue(LightData *ld, uint8_t value, uint32_t channelbits)
{
	int i;
	ChannelSet *channelset = &ld->channelset;
	
	for (i = 3; i < 16; i+=4)
	{	//�ж�ͨ����bitλ�Ƿ���λ
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
//���û�����ͨ�����ֵ
static void SetMotorLightValue(LightData *ld, uint8_t value, uint32_t channelbits)
{
	int i;
	ChannelSet *channelset = &ld->channelset;
	
	for (i = 0; i < 16; i++)
	{	//�ж�ͨ����bitλ�Ƿ���λ
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
    TxMessage.RTR = CAN_RTR_DATA;//��Ϣ֡����Ϊ����֡ 
    TxMessage.DLC = 7;//֡����Ϊ7
	TxMessage.Data[0] = g_nOutPutMode;
	memcpy(&TxMessage.Data[1], ld->data, 6);
    CAN_TX1_SendData(&TxMessage);
	
	g_ArrayLedGroupStatusArray[0]= ld->channelset.ch1;
    g_ArrayLedGroupStatusArray[1]= ld->channelset.ch2;
    g_ArrayLedGroupStatusArray[2]= ld->channelset.ch3;
    g_ArrayLedGroupStatusArray[3]= ld->channelset.ch4;
    SetLedGroupStatus();//���Լ��ĵƿذ�Ƶ���
    CanSendLampStatus();//�ٽ������Ϣ���͵�ǰ��壬��ǰ����Ӧ��LED���е���
	IWDG_ReloadCounter();
}
#define IS_OWN_ACCESS(baseaddr) ((baseaddr) == gLCBconfig->controlBoardNo)
//����ǰ��尴����ָʾ��,�����������ذ�ҵ����߻ظ�����
#define ALL_BUTTON_LED	0x1f	//�������а���LED,�������ذ�ҵ�,�ƿذ�ӹܿ���
#define AUTO_MANUAL_LED	0x03	//�����Զ����ֶ�����,������⵽��ѹ����ִ�л���
#define YELLOW_RED_LED	0x0c	//����������ȫ�찴��,������⵽��������ִ�л���
#define AUTO_BUTTON_LED	0x01	//�����Զ���LED,��ʾ���ذ�ָ�
static inline void LightFrontBoardButtonLed(uint8_t data)
{
    CanTxMsg TxMessage;
	if (IS_OWN_ACCESS(g_nBaseAddr))
	{
		TxMessage.StdId = 0x110;
		TxMessage.ExtId = 0;
		TxMessage.IDE = CAN_ID_STD;//
		TxMessage.RTR = CAN_RTR_DATA;//��Ϣ֡����Ϊ����֡ 
		TxMessage.DLC = 1;//֡����Ϊ1
		TxMessage.Data[0] = data;
		CAN_TX1_SendData(&TxMessage);
	}
}
//���ݻ�ȡ�ĺ�Ƶ���ֵת��Ϊ���bitֵ
static inline uint16_t RedCurConvert(uint8_t *data)
{
	Direction dir;
	
	dir.left = (data[0] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	dir.straight = (data[1] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	dir.right = (data[2] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	dir.pedestrian = (data[3] > gLCBconfig->baseinfo.minRedCurVal) ? LRED : LOFF;
	return *(uint16_t *)&dir;
}

#define MASK_YELLOW_VOLTBIT	0x6db	//ÿ������Ƶ��������λB(011 011 011 011)=0x6db
//���ݰ��ӱ�Ű�ת���ĵ�ѹbitֵ�͵���bitֵд�뵽��Ӧ���ӵ�Ƶ�����λ
static inline void WriteBitsToLightData(uint8_t boardNo, 
	uint16_t voltBits, LightData *boardVolt,
	uint16_t curBits, LightData *redCur)
{
	if (boardNo == 0)
	{	//��Ϊ�Ƶ�û�м���ѹĬ��Ϊ1��������ε�����������ϼ��
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
/*	���ݵ�����ѹֵת��Ϊ���ֵ�����������
	��ѹΪ1��ʾ������Ϊ0��ʾ����
	����С�����õ���Сֵ��Ϊ���𣬷�֮��Ϊ����	*/
static inline void ConvertToLightData(CanRxMsg *pRxMsg)
{
	uint8_t boardNo = pRxMsg->ExtId & 0x7;
	uint16_t voltBits = (pRxMsg->ExtId >> 3) & 0xfff;
	uint16_t curBits = RedCurConvert(pRxMsg->Data);	//ͨ������ֵת���õ��ĵ��bitֵ
	
	//printf("can recv boardNo:%d, voltbits: %#x\r\n", boardNo, voltBits);
	WriteBitsToLightData(boardNo - 1, voltBits, &gBoardVolt, curBits, &gRedCur);
}

extern void GetCurAverVal(void);
static inline uint16_t GetSelfCurBits(void)	//��ȡ�������ֵת���ĵ��bitֵ
{
	uint8_t i = 0, data[4] = {0};
    int16_t ndiff = 0;
	
	GetCurAverVal();
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

        data[i] = (uint8_t)ndiff;//�������ʱ�������ֵӦ�ô���10
    }
	return RedCurConvert(data);
}

extern uint16_t LampControlBoard_GetVoltage(void);
/*ÿDELAY_MSEC���һ��*/
#define ONESEC_CHECK_TIMES	(1000 / DELAY_MSEC)
#define MAX_ERROR_COUNT	(3 * ONESEC_CHECK_TIMES)		//�ܹ�����Ĵ������ܳ���3s
#define MAX_RIGHT_COUNT ONESEC_CHECK_TIMES		//���1s�ڼ�ⶼ����ȷ�ģ���������ϱ�־λ
static void FaultCheck(DirectionSet *ds)	//���Ժ��̳�ͻ���̳�ͻ�ļ��
{
	static uint8_t errorCount = 0;
	LightData voltcmp, curcmp;	//��ѹ�Ƚ����ݺ͵����Ƚ�����
	LightData boardVolt = gBoardVolt, redCur = gRedCur;
	int retVoltCmp = 0, retCurCmp = 0;
	uint16_t voltBits = LampControlBoard_GetVoltage();	//��ȡ����ĵ�ѹbitֵ
	uint16_t curBits = GetSelfCurBits();				//��ȡ�������ֵת���ĵ��bitֵ
	
	//memcpy(&boardVolt, &gBoardVolt, sizeof(LightData));
	//memcpy(&redCur, &gRedCur, sizeof(LightData));
	memset(&gBoardVolt, 0, sizeof(gBoardVolt));
	memset(&gRedCur, 0, sizeof(gRedCur));
	WriteBitsToLightData(g_nBaseAddr, voltBits, &boardVolt, curBits, &redCur);

	/*��Ϊ�Ƶ�û�м���ѹĬ��Ϊ1��������ε��ƵƵ�ѹֵ��
	����ֱ�ӱȽϵ��ֵ�͵�ѹֵ�Ϳ���ͬʱ�����̳�ͻ���̳�ͻ��*/
	voltcmp.dirset.east = ds->east & MASK_YELLOW_VOLTBIT;
	voltcmp.dirset.south = ds->south & MASK_YELLOW_VOLTBIT;
	voltcmp.dirset.west = ds->west & MASK_YELLOW_VOLTBIT;
	voltcmp.dirset.north = ds->north & MASK_YELLOW_VOLTBIT;
	/*��ʵ�ʵĵ��ֵֻ������Ƶĵ����ֵ�������ȽϺ�Ƶ���ת���ĵ��ֵ�жϺ���Ƿ�Ϩ��*/
	//ÿ������ĺ�Ƶ�����ֵΪB(010 010 010 010)=0x492
	curcmp.dirset.east = ds->east & 0x492;
	curcmp.dirset.south = ds->south & 0x492;
	curcmp.dirset.west = ds->west & 0x492;
	curcmp.dirset.north = ds->north & 0x492;
	retVoltCmp = memcmp(voltcmp.data, boardVolt.data, sizeof(voltcmp.data));
	retCurCmp = memcmp(curcmp.data, redCur.data, sizeof(curcmp.data));
	if ((gLCBconfig->baseinfo.isVoltCheckEnable == 1 
			&& gLCBconfig->baseinfo.isCurCheckEnable == 1
			&& retVoltCmp == 0 && retCurCmp == 0)	//��ѹ������ⶼ����
		|| (gLCBconfig->baseinfo.isVoltCheckEnable == 1 
			&& gLCBconfig->baseinfo.isCurCheckEnable == 0
			&& retVoltCmp == 0)	//��ѹ�������
		|| (gLCBconfig->baseinfo.isVoltCheckEnable == 0 
			&& gLCBconfig->baseinfo.isCurCheckEnable == 1
			&& retCurCmp == 0))	//�����������
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
			LightFrontBoardButtonLed(AUTO_MANUAL_LED);	//��⵽��ѹ����ʱ�����Զ����ֶ�����������ʾ
		else if (gLCBconfig->baseinfo.isCurCheckEnable && retCurCmp != 0)
			LightFrontBoardButtonLed(YELLOW_RED_LED);	//��⵽��������ʱ����������ȫ�찴��������ʾ
		gFaultFlag = 1;
		errorCount = 0;
	}
}
//ִ�ж����ڵ��
static void ExcuteFixedCycle(RunData *rd)
{
	int i, r, n = ONESEC_CHECK_TIMES;
	uint8_t motorValue, pedValue;
	LightData lightdata;
	PhaseLightValue tmp;
	
	memset(&lightdata, 0, sizeof(LightData));
	//��ʼ���������õ�ͨ��ȫ��Ϊ���
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
			HAL_GPIO_TogglePin(GPIOA,GPIO_Pin_5);	//��������ָʾ��
	}
}
//����ָ������һ�����������е���λ��״̬,��������ʱ��
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
		for (r = 1; r <= 2; r++)	//֧��˫��
		{
			starttime = 0;
			for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
			{	//������Ϣ��ÿ���ֽ��е�4bitΪ��1����λ����4bitΪ��2����λ
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
//�ҵ��޷�ӹܵ�λ��
static uint8_t FindTakeOverPosition(LCBruninfo *runinfo, RunData *rd)
{
	int index;
	LCBruninfo tmp;

	memset(&tmp, 0, sizeof(tmp));
	tmp.schemeid = 1;
	if (memcmp(runinfo, &tmp, sizeof(LCBruninfo)) == 0)
		return 0;	//˵����һ������ֱ�ӽӹ�Ĭ��ִ�з���1
	for (index = runinfo->runtime; index >= 0; index--)
	{	//��������λ���Լ����ֵ��һ��ʱ�򷵻ض�Ӧ��index+1
		if (rd[index].ring1.phase == runinfo->phaseR1
			&& rd[index].ring1.lightvalue == runinfo->lightvalueR1
			&& rd[index].ring2.phase == runinfo->phaseR2
			&& rd[index].ring2.lightvalue == runinfo->lightvalueR2)
			return index + 1;	//�ӹܺ����1s��ʼ����
	}
	IWDG_ReloadCounter();
	if (runinfo->lightvalueR1 == LGREEN || runinfo->lightvalueR2 == LGREEN)
	{	/*�����ǰ������λ���̵�ֵ��������Э������ʱ�����ڻ��ӳ��̵�ʱ�䣬�������ܵ���ԭ����������λ����ʱ
		  ������ֱ�������ģ�Ȼ�����ڹ����������̵�ʱ�䵼�����˵ƺͻ�������һ�ε��̵�ʱ�䣬���ǵƿذ�����
		  �Ƕ�����û����λ�����������һ���̵Ƶ�ʱ�Σ���˿����Ҳ�����ƥ���index����ʱ�ʹ�ͷ�ж��������
		  �崫�ݹ������ǻ����������˶����̵ƣ����ƿذ����ó��������̵�������������Ҳ�ǿ���ƥ���*/
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
	return 0xff;	//����һ����Ч��λ��
}
//������Ƶ��
static void SpecialControlLight(LightValue tmp)
{
	int i, n = ONESEC_CHECK_TIMES;
	uint8_t value;
	LightData lightdata;
	
	memset(&lightdata, 0, sizeof(LightData));
	//���жϴ˿�ƿذ��Ƿ�����ӹ�ִ�е������,����������ִ�е��
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
			if (value == LRED)	//ȫ��ʱ�������˵�
				SetPedLightValue(&lightdata, LRED, gLCBconfig->allchannelbits);
			SendLightCanMsg(&lightdata);
			
			HAL_Delay(DELAY_MSEC);
			IWDG_ReloadCounter();
			
			if ((i & 0x1) == 0)
				HAL_GPIO_TogglePin(GPIOA,GPIO_Pin_5);	//��������ָʾ��
		}
	}
}
#if 0
//���1s
static void LightOneSecond(uint8_t schemeid, RunData *rd)
{
	if (schemeid >= 0 && schemeid <= MAX_SUPPORT_SCHEME_NUM)
		ExcuteFixedCycle(rd);	//ִ�ж�����
	else if (schemeid == 255)
		SpecialControlLight(LYELLOW_FLASH);		//��������
	else if (schemeid == 252)
		SpecialControlLight(LRED);				//ȫ�����
	else if (schemeid == 251)
		SpecialControlLight(LOFF);				//�صƿ���
}
#endif

#endif
