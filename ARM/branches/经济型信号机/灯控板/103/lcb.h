#ifndef __LCB_H
#define __LCB_H

/*	灯控板进行相位接管的配置前提条件
	最大支持8个相位，8个普通方案(每个方案可以使用不同的绿信比和相序)
	另外也支持感应控制的无缝接管
	对于配置的相位必须从1开始连续，只配置1相位和3相位这种情况不予支持
	普通方案的配置也必须是从1开始连续的，对于只有方案1和方案3的情况也是不予支持的
	支持双环的无缝接管，但相位总是依然不能超过8个
	支持无缝接管的功能可配置，另外在接管过程中可以进行红绿冲突、绿冲突、红灯熄灭的故障检测，而且故障检测也可配置
	无缝接管不限于某块灯控板进行接管，具体根据配置的通道来决定，假如第一个灯控板没有配置通道，那么就使用第二个灯控板进行接管，如果第二个灯控板依然没有配置通道则使用第三个，以此类推
	
	不支持配置有忽略相位的接管
	对于跟随相位接管后无法做到跟随两个连续相位一直绿灯的情况，中间仍有间隔
	
	以下几种情况可能会不准：
	1.下载配置后如果没有间隔一个周期就进行接管
	2.协调控制的过渡期内
 */
 
 
/* light control board简称LCB */
#define MAX_SUPPORT_PHASE_NUM		8	//最大支持相位个数
#define MAX_SUPPORT_SCHEME_NUM		8	//最大支持的普通方案个数
/* 由于普通方案N对应相序表N和绿信比表N,再加上默认把感应控制作为第0方案,
   因此最大相序个数和最大绿信比个数即为(最大方案的个数 + 1) */
#define MAX_SUPPORT_PHASETURN_NUM	(MAX_SUPPORT_SCHEME_NUM + 1)	//最大支持的相序个数
#define MAX_SUPPORT_SPLIT_NUM		(MAX_SUPPORT_SCHEME_NUM	+ 1)	//最大支持的绿信比个数

/*	使用扩展can id,共29位,只用了18bit，具体定义如下:
	17-12(no)		11-8(index)			7-4(type)				3-0(id)
	发送can数据包	相序或是绿信比	0:基本信息,1:相位信息		0xf
	的编号,从0开始	的索引号[0-7]	2:相序信息,3:绿信比信息		避免与之前的can id冲突	
									4:每秒发送的运行信息,用来做无缝接管使用
	发送顺序为:基本信息-->相位信息-->相序信息-->绿信比信息	*/
#define LCB_CONFIG_ID	0x0f
#define LCB_CONFIG_MASK	LCB_CONFIG_ID
#define LCB_CAN_EXTID(no,index,type)	((1 << 31) | (no << 12) | (index << 8) | (type << 4) | LCB_CONFIG_ID)	//扩展id的第31bit必须为1
#define LCB_BASEINFO_CAN				LCB_CAN_EXTID(0, 0, BASEINFO)
#define LCB_PHASEINFO_CAN(no,index)		LCB_CAN_EXTID(no, index, PHASEINFO)
#define LCB_PHASETURNINFO_CAN(no,index)	LCB_CAN_EXTID(no, index, PHASETURNINFO)
#define LCB_SPLITINFO_CAN(no,index)		LCB_CAN_EXTID(no, index, SPLITINFO)

typedef struct
{
	uint32_t id:4;
	uint32_t type:4;
	uint32_t index:4;
	uint32_t no:6;
	uint32_t flag:2;
	uint32_t :12;
} CanExtId;

typedef enum
{
	BASEINFO = 0,
	PHASEINFO = 1,
	PHASETURNINFO = 2,
	SPLITINFO = 3,
	RUNINFO = 4,
	REBOOT = 5,
} LCBcanDataType;

typedef struct _LCB_phase_info_
{
	uint64_t greenFlashTime:8;		//绿闪时间
	uint64_t yellowTime:8;			//黄灯时间
	uint64_t allredTime:8;			//全红时间
	uint64_t pedFlashTime:8;		//行人绿闪时间
	uint64_t channelbits:16;		//相位对应的通道,bit[0-15]分别对应通道1-16
} LCBphaseinfo;

struct _LCB_phaseturn_info_
{	//存放相序中对应顺序的相位,每字节低四位为环1的相位,高四位为环2的相位
	uint64_t first:8;
	uint64_t second:8;
	uint64_t third:8;
	uint64_t fourth:8;
	uint64_t fifth:8;
	uint64_t sixth:8;
	uint64_t seventh:8;
	uint64_t eighth:8;
};
typedef union
{
	struct _LCB_phaseturn_info_ turn;
	uint8_t phases[MAX_SUPPORT_PHASE_NUM];
} LCBphaseturninfo;

struct _LCB_split_info_
{	//绿信比信息,存放1-8相位的绿信比
	uint64_t phase1:8;
	uint64_t phase2:8;
	uint64_t phase3:8;
	uint64_t phase4:8;
	uint64_t phase5:8;
	uint64_t phase6:8;
	uint64_t phase7:8;
	uint64_t phase8:8;
};
typedef union
{
	struct _LCB_split_info_ split;
	uint8_t times[MAX_SUPPORT_PHASE_NUM];
} LCBsplitinfo;

typedef enum _light_value_
{
	LOFF = 0,
	LGREEN = 1,
	LRED = 2,
	LYELLOW = 4,
	LGREEN_FLASH = 5,
	LYELLOW_FLASH = 6,
	LGREEN_FLASH_PED = 7,	//行人绿闪，此时机动车相位是绿灯
} LightValue;

typedef struct _LCB_run_info_
{
	uint64_t schemeid:8;		//当前运行的方案号
	uint64_t runtime:8;			//当前运行的时间,从0开始
	uint64_t phaseR1:8;			//环1此时运行的相位
	uint64_t phaseR2:8;			//环2此时运行的相位
	uint64_t lightvalueR1:4;	//环1此时运行相位的点灯值,其值参考 enum _light_value_
	uint64_t lightvalueR2:4;	//环2此时运行相位的点灯值,其值参考 enum _light_value_
} LCBruninfo;

typedef struct _LCB_base_info_
{
	uint64_t isTakeOver:1;			//是否接管控制
	uint64_t isVoltCheckEnable:1;	//是否开启电压检测,0:关闭,1:开启
	uint64_t isCurCheckEnable:1;	//是否开启电流检测,0:关闭,1:开启
	uint64_t phaseNum:5;			//相位个数
	uint64_t minRedCurVal:6;		//最小红灯电流值,低于这个值就认为无电流
	uint64_t schemeNum:4;			//普通方案的个数，不包括黄闪、全红、关灯、感应方案
	uint64_t canTotalNum:6;			//除去最初的1个基本信息包，其他总共发送给灯控板的can数据包个数
	uint64_t :40;					//保留40bit
} LCBbaseinfo;

typedef struct _LCB_config_
{
	LCBbaseinfo			baseinfo;
	LCBphaseinfo		phaseinfo[MAX_SUPPORT_PHASE_NUM];		//相位的相关信息
	LCBphaseturninfo	phaseturninfo[MAX_SUPPORT_PHASETURN_NUM];	//相序的相关信息
	LCBsplitinfo		splitinfo[MAX_SUPPORT_SPLIT_NUM];		//绿信比的相关信息
#define CONFIG_IS_READY		(0x1234)
#define CONFIG_UPDATE		(0x2345)
	uint32_t configstate:16;							//配置的状态,具体值为上面的两个宏定义
	uint32_t controlBoardNo:16;							//接管控制的灯控板号
	uint32_t allchannelbits;							//配置的所有通道
} LCBconfig;

static __INLINE int CheckLCBconfigValidity(LCBconfig *p)
{
	int i, j;
	int totalNum = p->baseinfo.schemeNum * 2 + 1 
					+ p->baseinfo.phaseNum;
	
	for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
	{
		if (p->phaseinfo[i].channelbits == 0)
			break;
	}
	for (j = 0; j < MAX_SUPPORT_PHASETURN_NUM; j++)
	{
		if (p->phaseturninfo[j].turn.first == 0
		|| p->splitinfo[j].split.phase1 == 0)
			break;
	}
	return (i > 0 && j > 0
			&& totalNum == p->baseinfo.canTotalNum
			&& i == p->baseinfo.phaseNum 
			&& j == p->baseinfo.schemeNum + 1);
}

#endif
