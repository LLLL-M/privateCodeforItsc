#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <array>
#include <bitset>
#include "hik.h"

#define MAX_PHASE_NUM					16	//最大相位个数
#define MAX_CHANNEL_NUM					32	//最大通道个数
#define MAX_DETECTOR_NUM				32	//最大检测器个数
#define MAX_SCHEDULE_NUM				256	//最大特殊日期个数
#define MAX_SCHEME_NUM					16	//最大方案个数

#define MAX_LIGHT_BITS					128	//最大点灯字节数

typedef bitset<MAX_LIGHT_BITS> LightBits;

//描述相位或是通道当前1s的状态
typedef enum 
{
    INVALID = 0,
	GREEN = 1,
	RED = 2,
	YELLOW = 3,
	GREEN_BLINK = 4,
	YELLOW_BLINK = 5,
	ALLRED = 6,
	TURN_OFF = 7,
	RED_BLINK = 8,
	RED_YELLOW = 9,
	
	OFF_GREEN = 100,	//绿灯脉冲倒计时，第一个250ms关灯
	OFF_YELLOW = 101,	//绿灯脉冲倒计时，第一个250ms关灯
	OFF_RED = 102,		//红灯脉冲倒计时，第一个250ms关灯
} Status;

typedef array<Status, MAX_CHANNEL_NUM> ChannelStatusArray;

enum
{
	LOCAL_CONTROL = 0,				//本地控制
	CLIENT_CONTROL = 1,				//客户端或中心平台上位机控制
	KEY_CONTROL = 2,				//按键控制
	FAULT_CONTROL = 3,				//故障控制
	
	SYSTEM_MODE = 0,				//系统控制模式
	TURNOFF_MODE = 1,				//关灯控制模式
	YELLOWBLINK_MODE = 2,			//黄闪控制模式
	ALLRED_MODE = 3,				//全红控制模式
	FIXEDCYCLE_MODE = 4,			//定周期控制模式
	COORDINATE_MODE = 5,			//协调控制模式
	INDUCTIVE_MODE = 6,				//感应控制模式
	SINGLE_SPOT_OPTIMIZE = 7,		//单点优化
	STEP_MODE = 8,					//步进控制模式
	CANCEL_STEP = 9,				//取消步进控制
	PEDESTRIAN_INDUCTIVE_MODE = 10,	//行人感应控制模式
	BUS_ADVANCE_MODE = 11,			//公交优先控制模式
	
	INDUCTIVE_COORDINATE_MODE = 12,	//协调感应模式
	SINGLE_ADAPT_MODE = 13,         //自适应控制
};

struct ControlRule
{
	UInt8	ctrlType;					/*控制类型
										 0:本地时段控制
										 1:平台或客户端控制
										 2:按键控制
										 3:故障控制*/
	UInt8 	ctrlMode;					/*控制模式
										 0:系统控制,只能手动执行,用来恢复本地控制
										 1:关灯控制,可时段配置或手动执行
										 2:黄闪控制,可时段配置或手动执行
										 3:全红控制,可时段配置或手动执行
										 4:定周期控制,可时段配置或手动执行
										 5:协调控制,只能时段配置
										 6:感应控制,可时段配置或手动执行
										 7:单点优化控制,可时段配置或手动执行
										 8:步进控制,只能手动执行
										 9:取消步进控制,只能手动执行
										 10:行人感应控制,可时段配置或手动执行
										 11:公交优先控制,可时段配置或手动执行
										 /以下几种控制国标未做要求，不予实现/
										 12:协调感应控制,只能时段配置
										 13:自适应控制,默认只使用方案号1,可时段配置或手动执行
										 14:通道锁定控制,可时段配置或手动执行
										 15:通道解锁控制,只能手动执行
										 */
    UInt8   ctrlId;                     /*使用的方案号[0,15],黄闪、全红、关灯时方案号为0
										  或者步进号(0:单步步进,其他:跳转步进)
										  或者通道锁定表号[1,15]
										 */
	bool operator==(const ControlRule &r) const
	{
		return (r.ctrlType == ctrlType && r.ctrlMode == ctrlMode && r.ctrlId == ctrlId);
	}
	bool operator!=(const ControlRule &r) const
	{
		return (r.ctrlType != ctrlType || r.ctrlMode != ctrlMode || r.ctrlId != ctrlId);
	}
	bool SpecialMode() const
	{
		return (ctrlMode == YELLOWBLINK_MODE || ctrlMode == TURNOFF_MODE || ctrlMode == ALLRED_MODE);
	}
	void Print() const
	{
		INFO("rule, ctrlType: %d, ctrlMode: %d, ctrlId: %d", ctrlType, ctrlMode, ctrlId);
	}
};

struct ColorStep
{
	UInt16 stepTime;
	Status status;
	UInt16 countdown;
	ColorStep(UInt16 _stepTime = 0, Status _status = INVALID, UInt16 _countdown = 0) : stepTime(_stepTime), status(_status), countdown(_countdown) {}
};

enum
{
	NOUSE = 0,
	MOTOR = 1,
	PEDESTRIAN = 2,
};
struct ChannelInfo
{
	UInt8 channelId = 0;
	UInt8 channelType = 0;
	UInt8 countdownId = 0;
	UInt8 countdownType = 0;
	ChannelInfo() = default;
};

struct PhaseInfo
{
	UInt8	phaseId = 0;							//当前运行的相位号
	UInt16	splitTime = 0;							//相位绿信比时间
	UInt8	checkTime = 0;							//感应检测时间
	UInt64	vehDetectorBits = 0;					//相位关联的车检器
	UInt16	unitExtendTime = 0;						//单位延长绿
	UInt16	maxExtendTime = 0;						//最大可以延长的绿灯时间
	UInt8 	pedDetectorBits;						//相位关联行人或公交检测器，bit0-bit7分别代表车检器1-8
	UInt16	advanceExtendTime = 0;					//优先延长时间
	UInt16	pedResponseTime = 0;					//行人请求响应时间
	bool	pedPhase = false;						//是否是行人相位
	bool	pedRequest = false;						//行人请求
	bool	motorRequest = false;					//机动车请求
	bitset<MAX_CHANNEL_NUM>	channelBits = 0;					//相位关联通道bit
	vector<ColorStep> motor;
	vector<ColorStep> pedestrian;
};

struct SchemeInfo
{
	UInt16				cycleTime = 0;				//当前周期总时间
	ControlRule			rule;	//当前使用的控制规则
	vector<PhaseInfo>	phaseturn;					//当前运行的阶段号
	array<ChannelInfo, MAX_CHANNEL_NUM> channelInfo;
	void Clear()
	{
		cycleTime = 0;
		rule.ctrlType = 0;
		rule.ctrlMode = 0;
		rule.ctrlId = 0;
		phaseturn.clear();
		channelInfo.fill(ChannelInfo());
	}
};

#endif
