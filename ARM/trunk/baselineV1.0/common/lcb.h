#ifndef __LCB_H
#define __LCB_H

/* light control board���LCB */
#define MAX_SUPPORT_PHASE_NUM	16	//���֧����λ����

/* can stand id */
#define LCB_CONFIG_CAN_ID	(0x1f)
#define LCB_CONFIG_CAN_BIT	6
#define LCB_CONFIG_CAN_MASK	(0x3f)

/*	�Եƿذ�����ʹ��can����չid
	index=0�����·�����baseinfo
	index>0�������ص��������еڼ���λ��phaseinfo
*/
#define LCB_CAN_STDID(index)	(LCB_CONFIG_CAN_ID | ((index) << LCB_CONFIG_CAN_BIT))
#define LCB_GET_CAN_INDEX(STDID)	(((STDID) >> LCB_CONFIG_CAN_BIT) & 0x1f)

typedef struct _LCB_phase_info_
{
	uint32_t splitTime:8;			//��λ�����ű�ʱ��
	uint32_t greenFlashTime:8;		//����ʱ��
	uint32_t yellowTime:8;			//�Ƶ�ʱ��
	uint32_t allredTime:8;			//ȫ��ʱ��
	uint32_t channelbits;			//��λ��Ӧ��ͨ��,bit[0-31]�ֱ��Ӧͨ��1-32
} LCBphaseinfo;

typedef struct _LCB_base_info_
{
	uint32_t startYellowFLashTime:8;					//���������ڵĻ���ʱ��
	uint32_t startAllredTime:8;							//���������ڵ�ȫ��ʱ��
	uint32_t cycleTime:8;								//����ʱ��
	uint32_t phaseNum:8;								//��λ����
} LCBbaseinfo;

typedef struct _LCB_config_
{
	LCBbaseinfo baseinfo;
	LCBphaseinfo phaseinfo[MAX_SUPPORT_PHASE_NUM];		//��λ�������Ϣ
#define CONFIG_IS_READY		(0x1234)
#define CONFIG_UPDATE		(0x2345)
	uint32_t configstate;
} LCBconfig;

static inline int CheckLCBconfigValidity(LCBconfig *p)
{
	uint8_t sum = 0;
	int i;
	
	for (i = 0; i < p->baseinfo.phaseNum; i++)
	{
		sum += p->phaseinfo[i].splitTime;
	}
	return (sum > 0 && sum == p->baseinfo.cycleTime);
}

#endif
