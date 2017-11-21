#ifndef __LCB_H
#define __LCB_H

/* light control board简称LCB */
#define MAX_SUPPORT_PHASE_NUM	16	//最大支持相位个数

/* can stand id */
#define LCB_CONFIG_CAN_ID	(0x1f)
#define LCB_CONFIG_CAN_BIT	6
#define LCB_CONFIG_CAN_MASK	(0x3f)

/*	对灯控板配置使用can的扩展id
	index=0表明下发的是baseinfo
	index>0表明下载的是相序中第几相位的phaseinfo
*/
#define LCB_CAN_STDID(index)	(LCB_CONFIG_CAN_ID | ((index) << LCB_CONFIG_CAN_BIT))
#define LCB_GET_CAN_INDEX(STDID)	(((STDID) >> LCB_CONFIG_CAN_BIT) & 0x1f)

typedef struct _LCB_phase_info_
{
	uint32_t splitTime:8;			//相位的绿信比时间
	uint32_t greenFlashTime:8;		//绿闪时间
	uint32_t yellowTime:8;			//黄灯时间
	uint32_t allredTime:8;			//全红时间
	uint32_t channelbits;			//相位对应的通道,bit[0-31]分别对应通道1-32
} LCBphaseinfo;

typedef struct _LCB_base_info_
{
	uint32_t startYellowFLashTime:8;					//启动定周期的黄闪时间
	uint32_t startAllredTime:8;							//启动定周期的全红时间
	uint32_t cycleTime:8;								//周期时间
	uint32_t phaseNum:8;								//相位个数
} LCBbaseinfo;

typedef struct _LCB_config_
{
	LCBbaseinfo baseinfo;
	LCBphaseinfo phaseinfo[MAX_SUPPORT_PHASE_NUM];		//相位的相关信息
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
