#include <string.h>
#include "HikConfig.h"
#include "its.h"
#include "platform.h"
#include "configureManagement.h"
#include "binfile.h"

extern SignalControllerPara *gSignalControlpara;
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //�����/home/desc.dat�����е����нṹ�壬������λ����/ͨ������/��������/�ƻ�����/��������

//�����ϴ��Ĳ���������Ҫ��ȡ��������Ϣ���ݵ���ʼ��ַ���ܴ�С
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

	switch (response->unExtraParamValue)
	{
		case 68: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseItem, gSignalControlpara->stPhase);
				 memset(phaseTable, 0, sizeof(phaseTable));
				 datalen = min(sizeof(phaseTable), datalen);
				 if (datalen > 0)
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
				 break;
		case 70: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(ChannelItem, gSignalControlpara->stChannel); 
				 if (response->unExtraParamFirst == 1)
					 memcpy(gSignalControlpara->stOldChannel, data, sizeof(gSignalControlpara->stOldChannel));
				 break;
		case 78: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(FollowPhaseItem, gSignalControlpara->stFollowPhase); 
				 if (response->unExtraParamFirst == 1)
					 memcpy(gSignalControlpara->stOldFollowPhase, data, sizeof(gSignalControlpara->stOldFollowPhase));
				 break;
		case 125: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_SignalTransEntry, gSignalControlpara->AscSignalTransTable);
				 if (response->unExtraParamFirst == 1)
					 memcpy(gSignalControlpara->OldAscSignalTransTable, data, sizeof(gSignalControlpara->OldAscSignalTransTable));
				 break;
		case 154: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseDescText, gStructBinfileDesc.phaseDescText);
				break;
	}
	if (datalen > 0)
	{
		memcpy(start, data, datalen);
		if (response->unExtraParamValue == 154)
			WRITE_BIN_CFG_PARAMS(FILE_DESC_DAT, &gStructBinfileDesc, sizeof(STRUCT_BINFILE_DESC));
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

	ItsGetConfig(NTCIP, gSignalControlpara);
	switch (response->unExtraParamValue)
	{
		case 94: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(TimeIntervalItem, gSignalControlpara->stTimeInterval); break;
		case 93: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PlanScheduleItem, gSignalControlpara->stPlanSchedule); break;
		case 87:
				 start = (char *)&gSignalControlpara->stUnitPara;
				 datalen = sizeof(UnitPara);
				 break;
		case 88: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseItem, gSignalControlpara->stPhase);
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
		case 89: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_N_PedestrianDetector, gSignalControlpara->AscPedestrianDetectorTable); break;
		case 90: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(ChannelItem, gSignalControlpara->stChannel); break;
		case 91: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_N_VehicleDetector, gSignalControlpara->AscVehicleDetectorTable); break;
		case 92: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(SchemeItem, gSignalControlpara->stScheme); break;
		case 95: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(GreenSignalRationItem, gSignalControlpara->stGreenSignalRation); break;
		case 96: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(PhaseTurnItem, gSignalControlpara->stPhaseTurn); break;
		case 97: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(ActionItem, gSignalControlpara->stAction); break;
		case 98: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(FollowPhaseItem, gSignalControlpara->stFollowPhase); break;
		case 126: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_SignalTransEntry, gSignalControlpara->AscSignalTransTable); break;
		case 99: CALCULATE_DATA_START_ADDRESS_AND_TOTALSIZE(struct STRU_N_Preempt, gSignalControlpara->AscPreemptTable); break;
		case 100:
				 start = (char *)&gSignalControlpara->AscCoordinationVariable;
				 datalen = sizeof(struct STRU_N_CoordinationVariable);
				 break;
		case 0xeeeeeeee: 
				 start = (char *)gSignalControlpara;
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

