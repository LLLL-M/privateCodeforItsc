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
#include <unistd.h>
#include "its.h"
#include "canmsg.h"
#include "arg.h"

/*********************************************************************************
 * *
 * * 	读车检板信号。
 * *
 * ***********************************************************************************/

UInt16 ItsReadVehicleDetectorData(int boardNum)
{
	UInt16 boardInfo = 0;

	if (boardNum < 1 || boardNum > 3)
	{
		ERR("vehicle board num %d is error", boardNum);
		return 0;
	}
	boardInfo = recv_date_from_vechile(boardNum);
	return boardInfo;
}

static inline void SetIgnorePhase(char *arg)
{
	int val = atoi(arg);
	switch (val)
	{
		case NONE_IGNORE: ItsSetIgnoreAttr(NONE_IGNORE); break;//不忽略，即把忽略相位时间保留
		case BACKWARD_IGNORE: ItsSetIgnoreAttr(BACKWARD_IGNORE); break;//向后忽略，即把忽略相位时间给后面的相位
		case ALL_IGNORE: ItsSetIgnoreAttr(ALL_IGNORE);	break;//前后忽略，即把忽略相位时间均分给前面和后面的相位
		default: ItsSetIgnoreAttr(FORWARD_IGNORE);	break;//向前忽略，即把忽略相位时间给前面的相位
	}
}

static inline void SetRedCountDownSec(char *arg)
{
	ItsSetRedFlashSec(atoi(arg));
}

static inline void IsYellowLampFlash(char *arg)
{
	ItsYellowLampFlash((atoi(arg) == 0) ? FALSE : TRUE);
}

static inline void IsContinueRun(char *arg)
{
	ItsIsContinueRun((atoi(arg) == 0) ? FALSE : TRUE);
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
			.name = "--redflashsec",
			.attr = REQUIRED_ARGUMENT,
			.doc = "set red light count down seconds, range[0,255]",
			.func = SetRedCountDownSec,
		},
		[2] = {
			.name = "--isyellowlampflash",
			.attr = REQUIRED_ARGUMENT,
			.doc = "is yellow lamp flash or not, only 0 or 1",
			.func = IsYellowLampFlash,
		},
		[3] = {
			.name = "--iscontinuerun",
			.attr = REQUIRED_ARGUMENT,
			.doc = "is continue run previous scheme after special control or not, only 0 or 1",
			.func = IsContinueRun,
		},
	};

	ArgDeal(argc, argv, arginfos, sizeof(arginfos) / sizeof(ArgInfo));
	INFO("********************************main()****************************\n");
	INFO("hikTSC500, compile time: %s, %s", __DATE__, __TIME__);
	
	//can通信初始化
	i_can_its_init();
	
	//CPU通过CPLD对外的IO口初始化
	CPLD_IO_Init();

	//默认点亮自动灯
	ProcessKeyBoardLight();

	//打开sadp服务器程序
	system("killall -9 RTSC_sadp");
	system("/root/RTSC_sadp &> /dev/null &");

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

