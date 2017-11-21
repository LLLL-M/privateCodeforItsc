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

static inline void SetIgnorePhase(char *arg)
{
	int val = atoi(arg);
	switch (val)
	{
		case NONE_IGNORE: ItsSetIgnoreAttr(NONE_IGNORE); break;//�����ԣ����Ѻ�����λʱ�䱣��
		case BACKWARD_IGNORE: ItsSetIgnoreAttr(BACKWARD_IGNORE); break;//�����ԣ����Ѻ�����λʱ����������λ
		case ALL_IGNORE: ItsSetIgnoreAttr(ALL_IGNORE);	break;//ǰ����ԣ����Ѻ�����λʱ����ָ�ǰ��ͺ������λ
		default: ItsSetIgnoreAttr(FORWARD_IGNORE);	break;//��ǰ���ԣ����Ѻ�����λʱ���ǰ�����λ
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

