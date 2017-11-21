#include <string.h>
#include "HikConfig.h"
#include "its.h"
#include "platform.h"
#include "configureManagement.h"
//#include "binfile.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

extern SignalControllerPara *gSignalControlpara;
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //存放在/home/desc.dat配置中的所有结构体，包括相位描述/通道描述/方案描述/计划描述/日期描述
extern unsigned int iIsSaveSpecialParams;
extern unsigned int iIsSaveCustomParams;
extern unsigned int iIsSaveDescParams;
extern unsigned int iIsSaveHikconfig;

//根据上传的参数计算需要获取的配置信息数据的起始地址和总大小
#define CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(membertype, member) do {\
	UInt32 totalNum = sizeof(member) / sizeof(membertype);\
	if (response->unExtraParamFirst == 0 || response->unExtraParamFirst > totalNum) \
	{\
		datalen = 0;\
	}\
	else\
	{\
		UInt32 leftNum = totalNum - response->unExtraParamFirst + 1;\
		start = (char *)((membertype *)(member) + (response->unExtraParamFirst - 1));\
		datalen = min(response->unExtraParamTotal, leftNum) * sizeof(membertype);\
	}\
} while (0)

int DownloadExtendConfig(struct STRU_Extra_Param_Response *response)
{
	PhaseItem phaseTable[NUM_PHASE];
	char *start = NULL, *data = (char *)response->data;
	int i;
	size_t datalen = 0;
	sqlite3* pdb = NULL;

	//INFO("recv: type = %d, firstline = %d, totalline = %d", response->unExtraParamValue, response->unExtraParamFirst, response->unExtraParamTotal);
	switch (response->unExtraParamValue)
	{
		case 68: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseItem, gSignalControlpara->stPhase);
				 memset(phaseTable, 0, sizeof(phaseTable));
				 datalen = min(sizeof(phaseTable), datalen);
				 if (datalen > 0)
				 {
					 memcpy(phaseTable, response->data, datalen);
					 for(i = 0; i < NUM_PHASE; i++)
					 {
						 if(phaseTable[i].nCircleID != 0)
						 {
							 phaseTable[i].nAllRedTime /= 10;
							 phaseTable[i].nYellowTime /= 10;
							 phaseTable[i].nUnitExtendGreen /= 10;
							 phaseTable[i].nRedProtectedTime /= 10;
							 phaseTable[i].byPhaseDynamicMaxStep /= 10;
						 }
					 }
					 data = (char *)phaseTable;
					 if (response->unExtraParamFirst == 1)
						 memcpy(gSignalControlpara->stOldPhase, phaseTable, sizeof(phaseTable));
				 }
				 SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_PHASE);
				 break;
		case 70: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(ChannelItem, gSignalControlpara->stChannel); 
				 if (response->unExtraParamFirst == 1)
					 memcpy(gSignalControlpara->stOldChannel, data, sizeof(gSignalControlpara->stOldChannel));
				 SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_CHANNEL);
				 break;
		case 78: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(FollowPhaseItem, gSignalControlpara->stFollowPhase); 
				 if (response->unExtraParamFirst == 1)
					 memcpy(gSignalControlpara->stOldFollowPhase, data, sizeof(gSignalControlpara->stOldFollowPhase));
				 SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_FOLLOW_PHASE);
				 break;
		case 125: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_SignalTransEntry, gSignalControlpara->AscSignalTransTable);
				 if (response->unExtraParamFirst == 1)
					 memcpy(gSignalControlpara->OldAscSignalTransTable, data, sizeof(gSignalControlpara->OldAscSignalTransTable));
				 SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_SIGNAL_TRANS);
				 break;
		case 154: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseDescText, gStructBinfileDesc.phaseDescText);
				SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_PHASE);
				break;
	}
	if (datalen > 0)
	{
		memcpy(start, data, datalen);
		if (response->unExtraParamValue == 154)
		{	//WRITE_BIN_CFG_PARAMS(FILE_DESC_DAT, &gStructBinfileDesc, sizeof(STRUCT_BINFILE_DESC));
			/*log_debug("download extend config, 154:phaseDescText");
			sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
			write_desc(pdb, &gStructBinfileDesc);
			sqlite3_close_wrapper(pdb); pdb = NULL;*/
		}
		//INFO("DEBUG type = %d, datalen = %d, firstLine = %d, totalLine = %d", response->unExtraParamValue, datalen, response->unExtraParamFirst, response->unExtraParamTotal);
		response->unExtraDataLen = datalen;
	}
	return sizeof(struct STRU_Extra_Param_Response);
}

int UploadConfig(struct STRU_Extra_Param_Response *response)
{
	PhaseItem phaseTable[NUM_PHASE];
	char *start = NULL;
	int i;
	size_t datalen = 0;
	SignalControllerPara config;

	ItsGetConfig(&config, sizeof(SignalControllerPara));
	switch (response->unExtraParamValue)
	{
		case 94: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(TimeIntervalItem, config.stTimeInterval); break;
		case 93: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PlanScheduleItem, config.stPlanSchedule); break;
		case 87:
				 start = (char *)&config.stUnitPara;
				 datalen = sizeof(UnitPara);
				 break;
		case 88: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseItem, config.stPhase);
				 memset(phaseTable, 0, sizeof(phaseTable));
				 datalen = min(sizeof(phaseTable), datalen);
				 memcpy(phaseTable, start, datalen);
				 for(i = 0; i < NUM_PHASE; i++)
				 {
					 if(phaseTable[i].nCircleID != 0)
					 {
						 phaseTable[i].nAllRedTime *= 10;
						 phaseTable[i].nYellowTime *= 10;
						 phaseTable[i].nUnitExtendGreen *= 10;
						 phaseTable[i].nRedProtectedTime *= 10;
						 phaseTable[i].byPhaseDynamicMaxStep *= 10;
					 }
				 }
				 start = (char *)phaseTable;
				 datalen = sizeof(phaseTable);
				 break;
		case 89: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_N_PedestrianDetector, config.AscPedestrianDetectorTable); break;
		case 90: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(ChannelItem, config.stChannel); break;
		case 91: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_N_VehicleDetector, config.AscVehicleDetectorTable); break;
		case 92: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(SchemeItemOld, config.stOldScheme); break;
		case 95: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(GreenSignalRationItem, config.stGreenSignalRation); break;
		case 96: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseTurnItem, config.stPhaseTurn); break;
		case 97: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(ActionItem, config.stAction); break;
		case 98: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(FollowPhaseItem, config.stFollowPhase); break;
		case 126: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_SignalTransEntry, config.AscSignalTransTable); break;
		case 99: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_N_Preempt, config.AscPreemptTable); break;
		case 100:
				 start = (char *)&config.AscCoordinationVariable;
				 datalen = sizeof(struct STRU_N_CoordinationVariable);
				 break;
		case 222: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(SchemeItem, config.stScheme); break;
		case 0xeeeeeeee: 
				 start = (char *)&config;
				 datalen = sizeof(SignalControllerPara);
				 break;
		case 155: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseDescText, gStructBinfileDesc.phaseDescText);
				break;
	}
	if (datalen > 0)
	{
		memcpy(response->data, start, datalen);
		//INFO("DEBUG type = %d, datalen = %d, firstLine = %d, totalLine = %d", response->unExtraParamValue, datalen, response->unExtraParamFirst, response->unExtraParamTotal);
		response->unExtraDataLen = datalen;
	}
	response->unExtraDataLen = datalen;
	return datalen + sizeof(struct STRU_Extra_Param_Response);
}

