/*hdr
**
**	
**
**	FILE NAME:	
**
**	AUTHOR:		
**
**	DATE:		
**
**	FILE DESCRIPTION:
**			
**			
**
**	FUNCTIONS:	
**			
**			
**			
**	NOTES:		
*/  

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "its.h"
#include "arg.h"
#include "canmsg.h"


UInt8 gIgnoreOption = FORWARD_IGNORE;	//Ĭ����ǰ����
Boolean gAddTimeToFirstPhase;

extern void CanInit();

static inline void SetIgnorePhase(char *arg)
{
	int val = atoi(arg);
	switch (val)
	{
		case NONE_IGNORE: //�����ԣ����Ѻ�����λʱ�䱣��
		case BACKWARD_IGNORE: //�����ԣ����Ѻ�����λʱ����������λ
		case ALL_IGNORE: gIgnoreOption = val; break;//ǰ����ԣ����Ѻ�����λʱ����ָ�ǰ��ͺ������λ
		default: gIgnoreOption = FORWARD_IGNORE; break;//��ǰ���ԣ����Ѻ�����λʱ���ǰ�����λ
	}
}

static inline void IsContinueRun(char *arg)
{
	ItsIsContinueRun((atoi(arg) == 0) ? FALSE : TRUE);
}

static inline void AddTimeToFirstPhase(char *arg)
{
	gAddTimeToFirstPhase = (atoi(arg) == 0) ? FALSE : TRUE;
}
/*********************************************************************************
*
* 	���������
*
***********************************************************************************/
int main(int argc, char *argv[])
{	
	ArgInfo arginfos[] = {
		[0] = {
			.name = "--ignoreattr",
			.attr = REQUIRED_ARGUMENT,
			.doc = "set ignore phase attribute, 0:none, 1:forward, 2:backward, 3:all",
			.func = SetIgnorePhase,
		},
		[1] = {
			.name = "--iscontinuerun",
			.attr = REQUIRED_ARGUMENT,
			.doc = "is continue run previous scheme after special control or not, only 0 or 1",
			.func = IsContinueRun,
		},
		[2] = {
			.name = "--addTimeToFirstPhase",
			.attr = REQUIRED_ARGUMENT,
			.doc = "where add time to first phase or not when excute inductive coordinate control, only 0 or 1",
			.func = AddTimeToFirstPhase,
		},
	};

	ArgDeal(argc, argv, arginfos, sizeof(arginfos) / sizeof(ArgInfo));
	INFO("********************************main()****************************\n");
	INFO("hikTSC300, version: %s, compile time: %s, %s", SOFTWARE_VERSION_INFO, __DATE__, __TIME__);
	
	//��sadp����������
	system("killall -9 RTSC_sadp");
	system("/root/RTSC_sadp &> /dev/null &");

#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
	//CPUͨ��CPLD�����IO�ڳ�ʼ��
	IO_Init();
	
	//canͨ�ų�ʼ��
	CanInit(500000);
	
	//Ĭ�ϵ����Զ���
	ProcessKeyBoardLight();

	KeyBoardInit();
#endif
	InitCommon();
	while (1) 
	{
		//���ϵͳ����
        //sync(); //����Ƶ�ʲ��ܹ��ߣ��ô�������ʱע�͵�
        system("echo 1 > /proc/sys/vm/drop_caches");
        system("echo \"0\" >> /proc/sys/vm/overcommit_memory");
		sleep(3);	//˯��ʱ�䲻ӦС��2s,ItsThreadCheck��������Ƶ�ʲ���̫��,������⵽ģ�������쳣����ϵͳ����
		ItsThreadCheck();
	}
	return 0;
}

