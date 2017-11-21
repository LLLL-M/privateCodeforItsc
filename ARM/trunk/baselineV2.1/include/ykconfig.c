#include <stdio.h>
#include <string.h>
#include "its.h"
#include "ykconfig.h"

int main(void)
{
	FILE *fp = fopen("ykconfig.dat", "w+");
	YK_Config ykconfig;
	YK_TimeIntervalItem *timeIntervalItem = NULL;
	YK_ChannelItem *channelItem = NULL;
	
	if (fp == NULL)
	{
		puts("fopen ykconfig.dat fail");
		return -1;
	}
	memset(&ykconfig, 0, sizeof(YK_Config));
	//设置整体基本信息
	ykconfig.wholeConfig.bootYellowBlinkTime = 6;
	ykconfig.wholeConfig.bootAllRedTime = 6;
	ykconfig.wholeConfig.vehFlowCollectCycleTime = 5;
	ykconfig.wholeConfig.transitionCycle = 3;
	ykconfig.wholeConfig.adaptCtlEndRunCycleNum = 3;
	ykconfig.wholeConfig.watchdogEnable = 0;
	ykconfig.wholeConfig.signalMachineType = 1;
	ykconfig.wholeConfig.weekPreemptSchedule = 0;
	
	//设置调度表信息
	ykconfig.scheduleTable.week[0] = 1;
	ykconfig.scheduleTable.week[1] = 1;
	ykconfig.scheduleTable.week[2] = 1;
	ykconfig.scheduleTable.week[3] = 1;
	ykconfig.scheduleTable.week[4] = 1;
	ykconfig.scheduleTable.week[5] = 1;
	ykconfig.scheduleTable.week[6] = 1;
	
	//设置时段表
	timeIntervalItem = (YK_TimeIntervalItem *)&ykconfig.timeIntervalTable[0];
	timeIntervalItem->nTimeIntervalID = 1;
	timeIntervalItem->nTimeID = 1;
	timeIntervalItem->cStartTimeHour = 0;
	timeIntervalItem->cStartTimeMinute = 0;
	timeIntervalItem->nSchemeId = 1;
	timeIntervalItem->phaseOffset = 10;
	timeIntervalItem->IsCorrdinateCtl = 0;
	
	//设置通道表
	channelItem = (YK_ChannelItem *)&ykconfig.channelTable;
	//设置通道1
	channelItem[0].nChannelID = 1;
	channelItem[0].nControllerType = 2;
	channelItem[0].nFlashLightType = 2;
	channelItem[0].nVehDetectorNum = 0;
	channelItem[0].conflictChannel = 0;
	//设置通道2
	channelItem[1].nChannelID = 2;
	channelItem[1].nControllerType = 2;
	channelItem[1].nFlashLightType = 2;
	channelItem[1].nVehDetectorNum = 0;
	channelItem[1].conflictChannel = 0;
	//设置通道4
	channelItem[3].nChannelID = 4;
	channelItem[3].nControllerType = 3;
	channelItem[3].nFlashLightType = 2;
	channelItem[3].nVehDetectorNum = 0;
	channelItem[3].conflictChannel = 0;
	//设置通道5
	channelItem[4].nChannelID = 5;
	channelItem[4].nControllerType = 2;
	channelItem[4].nFlashLightType = 2;
	channelItem[4].nVehDetectorNum = 0;
	channelItem[4].conflictChannel = 0;
	//设置通道6
	channelItem[5].nChannelID = 6;
	channelItem[5].nControllerType = 2;
	channelItem[5].nFlashLightType = 2;
	channelItem[5].nVehDetectorNum = 0;
	channelItem[5].conflictChannel = 0;
	//设置通道9
	channelItem[8].nChannelID = 9;
	channelItem[8].nControllerType = 2;
	channelItem[8].nFlashLightType = 2;
	channelItem[8].nVehDetectorNum = 0;
	channelItem[8].conflictChannel = 0;
	//设置通道10
	channelItem[9].nChannelID = 10;
	channelItem[9].nControllerType = 2;
	channelItem[9].nFlashLightType = 2;
	channelItem[9].nVehDetectorNum = 0;
	channelItem[9].conflictChannel = 0;
	//设置通道12
	channelItem[11].nChannelID = 12;
	channelItem[11].nControllerType = 3;
	channelItem[11].nFlashLightType = 2;
	channelItem[11].nVehDetectorNum = 0;
	channelItem[11].conflictChannel = 0;
	//设置通道13
	channelItem[12].nChannelID = 13;
	channelItem[12].nControllerType = 2;
	channelItem[12].nFlashLightType = 2;
	channelItem[12].nVehDetectorNum = 0;
	channelItem[12].conflictChannel = 0;
	//设置通道14
	channelItem[13].nChannelID = 14;
	channelItem[13].nControllerType = 2;
	channelItem[13].nFlashLightType = 2;
	channelItem[13].nVehDetectorNum = 0;
	channelItem[13].conflictChannel = 0;
	
	//设置方案表
	YK_PhaseInfo phaseInfo = {
		.greenTime = 6,
		.greenBlinkTime = 3,
		.yellowTime = 3,
		.redYellowTime = 3,
		.pedestrianClearTime = 10,
		.minGreenTime = 10,
		.maxGreenTime = 20,
		.unitExtendTime = 3,
	};
	ykconfig.schemeTable[0].nSchemeId = 1;
	ykconfig.schemeTable[0].cycleTime = 60;
	ykconfig.schemeTable[0].totalPhaseNum = 4;
	//设置相位1
	memcpy(&ykconfig.schemeTable[0].phaseInfo[0], &phaseInfo, sizeof(YK_PhaseInfo));
	ykconfig.schemeTable[0].phaseInfo[0].channelBits = 0x1119;
	//设置相位2
	memcpy(&ykconfig.schemeTable[0].phaseInfo[1], &phaseInfo, sizeof(YK_PhaseInfo));
	ykconfig.schemeTable[0].phaseInfo[1].channelBits = 0x2a22;
	//设置相位3
	memcpy(&ykconfig.schemeTable[0].phaseInfo[2], &phaseInfo, sizeof(YK_PhaseInfo));
	ykconfig.schemeTable[0].phaseInfo[2].channelBits = 0x030b;
	//设置相位4
	memcpy(&ykconfig.schemeTable[0].phaseInfo[3], &phaseInfo, sizeof(YK_PhaseInfo));
	ykconfig.schemeTable[0].phaseInfo[3].channelBits = 0x3830;
	
	//写入ykconfig.dat配置文件
	fwrite(&ykconfig, sizeof(YK_Config), 1, fp);
	fclose(fp);
	return 0;
}