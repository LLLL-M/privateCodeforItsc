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


UInt8 gIgnoreOption = FORWARD_IGNORE;	//默认向前忽略
Boolean gAddTimeToFirstPhase;

extern void CanInit();

static inline void SetIgnorePhase(char *arg)
{
	int val = atoi(arg);
	switch (val)
	{
		case NONE_IGNORE: //不忽略，即把忽略相位时间保留
		case BACKWARD_IGNORE: //向后忽略，即把忽略相位时间给后面的相位
		case ALL_IGNORE: gIgnoreOption = val; break;//前后忽略，即把忽略相位时间均分给前面和后面的相位
		default: gIgnoreOption = FORWARD_IGNORE; break;//向前忽略，即把忽略相位时间给前面的相位
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
* 	主程序入口
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
	
	//打开sadp服务器程序
	system("killall -9 RTSC_sadp");
	system("/root/RTSC_sadp &> /dev/null &");

#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
	//CPU通过CPLD对外的IO口初始化
	IO_Init();
	
	//can通信初始化
	CanInit(500000);
	
	//默认点亮自动灯
	ProcessKeyBoardLight();

	KeyBoardInit();
#endif
	InitCommon();
	while (1) 
	{
		//清除系统缓存
        //sync(); //调用频率不能过高，用处不大暂时注释掉
        system("echo 1 > /proc/sys/vm/drop_caches");
        system("echo \"0\" >> /proc/sys/vm/overcommit_memory");
		sleep(3);	//睡眠时间不应小于2s,ItsThreadCheck函数调用频率不能太高,否则会检测到模块运行异常导致系统重启
		ItsThreadCheck();
	}
	return 0;
}

