#ifndef __HIKCONFIG_H__
#define __HIKCONFIG_H__

#include <stdio.h>

#define NUM_SCHEDULE                    40               //调度表总数
#define NUM_TIME_INTERVAL               16               //时段表总数
#define NUM_TIME_INTERVAL_ID            48               //每个时段表含有的时段号总数

#define BIT(val, n)	(((val) >> (n)) & 0x1)

typedef struct {
    unsigned short nScheduleID;//调度计划号
	unsigned short month;	//bit[1-12]分别代表1-12月，0:未选中，1:选中
	unsigned int week;		//bit[1-7]分别代表星期一到星期日，0:未选中，1:选中
	unsigned int day;		//bit[1-31]分别代表每月的1-31号，0:未选中，1:选中
    unsigned int nTimeIntervalID;//时段表号
} PlanScheduleItem,*PPlanScheduleItem; //单个调度项


typedef struct {

    unsigned char nTimeIntervalID;//时段表号，1个时段链表有N个时段表，1个时段表可以有N个时段号

    unsigned char nTimeID;//时段号

    unsigned char cStartTimeHour;//开始时间  时
    unsigned char cStartTimeMinute;//开始时间 分

    unsigned char nActionID;//动作号

} TimeIntervalItem,*PTimeIntervalItem;


typedef struct {
    PlanScheduleItem			stPlanSchedule[NUM_SCHEDULE];//调度表
    TimeIntervalItem			stTimeInterval[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID];//时段表
} SignalControllerPara,*PSignalControllerPara; //信号机配置参数主结构


#ifdef __cplusplus
extern "C" {
#endif

extern void storeTimeIntervalToIni(void *arg);
extern void storePlanScheduleToIni(void *arg);
extern void adjustControlType();


#ifdef __cplusplus
}
#endif


#endif
