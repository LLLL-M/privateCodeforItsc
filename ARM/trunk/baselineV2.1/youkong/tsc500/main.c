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


/*********************************************************************************
*
* 	主程序入口
*
***********************************************************************************/
int main(int argc, char *argv[])
{	
#if 0
	ArgInfo arginfos[] = {
		[0] = {
			.name = "--ignoreattr",
			.attr = REQUIRED_ARGUMENT,
			.doc = "set ignore phase attribute, 0:none, 1:forward, 2:backward, 3:all",
			.func = SetIgnorePhase,
		},
	};

	ArgDeal(argc, argv, arginfos, sizeof(arginfos) / sizeof(ArgInfo));
#endif
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

