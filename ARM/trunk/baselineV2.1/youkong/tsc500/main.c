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
 * * 	��������źš�
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
* 	���������
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
	
	//canͨ�ų�ʼ��
	i_can_its_init();
	
	//CPUͨ��CPLD�����IO�ڳ�ʼ��
	CPLD_IO_Init();

	//Ĭ�ϵ����Զ���
	ProcessKeyBoardLight();

	//��sadp����������
	system("killall -9 RTSC_sadp");
	system("/root/RTSC_sadp &> /dev/null &");

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

