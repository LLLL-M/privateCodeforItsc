#ifndef __HIKCONFIG_H__
#define __HIKCONFIG_H__

#include <stdio.h>

#define NUM_SCHEDULE                    40               //���ȱ�����
#define NUM_TIME_INTERVAL               16               //ʱ�α�����
#define NUM_TIME_INTERVAL_ID            48               //ÿ��ʱ�α��е�ʱ�κ�����

#define BIT(val, n)	(((val) >> (n)) & 0x1)

typedef struct {
    unsigned short nScheduleID;//���ȼƻ���
	unsigned short month;	//bit[1-12]�ֱ����1-12�£�0:δѡ�У�1:ѡ��
	unsigned int week;		//bit[1-7]�ֱ��������һ�������գ�0:δѡ�У�1:ѡ��
	unsigned int day;		//bit[1-31]�ֱ����ÿ�µ�1-31�ţ�0:δѡ�У�1:ѡ��
    unsigned int nTimeIntervalID;//ʱ�α��
} PlanScheduleItem,*PPlanScheduleItem; //����������


typedef struct {

    unsigned char nTimeIntervalID;//ʱ�α�ţ�1��ʱ��������N��ʱ�α�1��ʱ�α������N��ʱ�κ�

    unsigned char nTimeID;//ʱ�κ�

    unsigned char cStartTimeHour;//��ʼʱ��  ʱ
    unsigned char cStartTimeMinute;//��ʼʱ�� ��

    unsigned char nActionID;//������

} TimeIntervalItem,*PTimeIntervalItem;


typedef struct {
    PlanScheduleItem			stPlanSchedule[NUM_SCHEDULE];//���ȱ�
    TimeIntervalItem			stTimeInterval[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID];//ʱ�α�
} SignalControllerPara,*PSignalControllerPara; //�źŻ����ò������ṹ


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
