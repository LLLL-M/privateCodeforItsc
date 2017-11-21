#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "its.h"
#include "convert.h"
#include "configureManagement.h"

#ifdef USE_GB_PROTOCOL
#include "gb.h"
extern GbConfig *gGbconfig;
extern void GbInit();
#endif

extern SignalControllerPara *gSignalControlpara;
UInt32	ntcipToGbFlag;

extern void SendLCBconfigToBoard(SignalControllerPara *para);

#define SEND_TIMEINTERVAL_TIMES		16	//�ܹ�����16��ʱ�α���Ϣ
#define SEND_TIMEINTERVAL_NUM		48	//ÿ�η���48��
static inline void StoreTimeInterval(void *arg)
{
	TimeIntervalItem *item = (TimeIntervalItem *)arg;
	int i;
	
	for (i = 0; i < SEND_TIMEINTERVAL_NUM; i++) 
	{
		if (item[i].nTimeIntervalID > 0 && item[i].nTimeIntervalID <= NUM_TIME_INTERVAL
			&& item[i].nTimeID > 0 && item[i].nTimeID <= NUM_TIME_INTERVAL_ID)
		{
			gSignalControlpara->stTimeInterval[item[i].nTimeIntervalID - 1][item[i].nTimeID - 1] = item[i];
		}
	}
}

#define SEND_PLANSCHEDULE_TIMES		4	//�ܹ�����4�ε��ȱ���Ϣ
#define	SEND_PLANSCHEDULE_NUM		10	//ÿ�η���10��
static inline void StorePlanSchedule(void *arg)
{
	PlanScheduleItem *item = (PlanScheduleItem *)arg;
	int i;
	
	for (i = 0; i < SEND_PLANSCHEDULE_NUM; i++) 
	{
		if (item[i].nScheduleID > 0 && item[i].nScheduleID <= NUM_SCHEDULE)
		{
			gSignalControlpara->stPlanSchedule[item[i].nScheduleID - 1] = item[i];
		}
	}
}

static inline void StoreUnit(void *arg)
{
	memcpy(&gSignalControlpara->stUnitPara, arg, sizeof(UnitPara));
}
/*****************************************************************************
 �� �� ��  : StorePhase
 ��������  : ������λ����
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��18��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��18��
    ��    ��   : Ф�Ļ�
    �޸�����   : ��λ����ȫ��ʱ�䡢��λ�ӳ��̡��Ƶ�ʱ��SDK�������ĵ�λ��1/10��
                 ���������ﱣ��ʱҪ����10���Ա�֤У����ȷ��
*****************************************************************************/
static inline void StorePhase(void *arg)
{
    int i = 0;
    
	memcpy(gSignalControlpara->stOldPhase, arg, sizeof(gSignalControlpara->stOldPhase));
    for(i = 0; i < NUM_PHASE; i++)
    {
        if(gSignalControlpara->stOldPhase[i].nCircleID != 0)
        {
            gSignalControlpara->stOldPhase[i].nAllRedTime /= 10;
            gSignalControlpara->stOldPhase[i].nYellowTime /= 10;
            gSignalControlpara->stOldPhase[i].nUnitExtendGreen /= 10;
			gSignalControlpara->stOldPhase[i].nRedProtectedTime /= 10;
			gSignalControlpara->stOldPhase[i].byPhaseDynamicMaxStep /= 10;
        }
    }
	memcpy(gSignalControlpara->stPhase[0], gSignalControlpara->stOldPhase, sizeof(gSignalControlpara->stOldPhase));
}
static inline void StoreTransform(void *arg)
{
	memcpy(gSignalControlpara->OldAscSignalTransTable, arg, sizeof(gSignalControlpara->OldAscSignalTransTable));
	memcpy(gSignalControlpara->AscSignalTransTable[0], gSignalControlpara->OldAscSignalTransTable, sizeof(gSignalControlpara->OldAscSignalTransTable));
}
#define SEND_PHASETURN_TIMES	16	//�ܹ�����16���������Ϣ
#define	SEND_PHASETURN_NUM		4	//ÿ�η���4��
static inline void StorePhaseTurn(void *arg)
{
    PhaseTurnItem *item = (PhaseTurnItem *)arg;
    int i = 0;

    for(i = 0 ; i < SEND_PHASETURN_NUM; i++)
    {
		if (item[i].nPhaseTurnID > 0 && item[i].nPhaseTurnID <= NUM_PHASE_TURN
			&& item[i].nCircleID > 0 && item[i].nCircleID <= NUM_RING_COUNT)
		{
			memcpy(&(gSignalControlpara->stPhaseTurn[item[i].nPhaseTurnID - 1][item[i].nCircleID - 1]), &item[i], sizeof(PhaseTurnItem));//�������λ��Ŀ����16������32��
		}
    }

}
#define SEND_SPLIT_TIMES	36	//�ܹ�����36�����űȱ���Ϣ
#define	SEND_SPLIT_NUM		16	//ÿ�η���16��
static inline void StoreSplit(void *arg)
{
	GreenSignalRationItem *item = (GreenSignalRationItem *)arg;
	int i;
	
	for (i = 0; i < SEND_SPLIT_NUM; i++)
	{
		if (item[i].nGreenSignalRationID > 0 && item[i].nGreenSignalRationID <= NUM_GREEN_SIGNAL_RATION
			&& item[i].nPhaseID > 0 && item[i].nPhaseID <= NUM_PHASE)
		{
			gSignalControlpara->stGreenSignalRation[item[i].nGreenSignalRationID - 1][item[i].nPhaseID - 1] = item[i];
		}
	}
}
static inline void StoreChannel(void *arg)
{
	memcpy(gSignalControlpara->stOldChannel, arg, sizeof(gSignalControlpara->stOldChannel));
	memcpy(gSignalControlpara->stChannel[0], gSignalControlpara->stOldChannel, sizeof(gSignalControlpara->stOldChannel));
}
#define SEND_SCHEME_TIMES	6	//�ܹ�����6�η�������Ϣ
#define	SEND_SCHEME_NUM		18	//ÿ�η���18��
static inline void StoreScheme(void *arg)
{
	SchemeItem *item = (SchemeItem *)arg;
	int i;
#if 1	
	for (i = 0; i < SEND_SCHEME_NUM; i++)
	{
		if (item[i].nSchemeID > 0 && item[i].nSchemeID <= NUM_SCHEME)
		{
			gSignalControlpara->stScheme[item[i].nSchemeID - 1] = item[i];
		}
	}
#else	//add by Jicky, ��Ϊ������Ķ�Ӧ��ϵ��һ��
	for (i = 0; i < SEND_SCHEME_NUM; i++)
	{	
		if (item[i].nSchemeID >= 249)	
		{	//���ⷽ��
			gSignalControlpara->stScheme[item[i].nSchemeID - 1] = item[i];
			continue;
		}
		if ((item[i].nSchemeID - 1) % 3 == 0)
		{	//��ͨ����
			gSignalControlpara->stScheme[(item[i].nSchemeID - 1) / 3] = item[i];
			gSignalControlpara->stScheme[(item[i].nSchemeID - 1) / 3].nSchemeID = (item[i].nSchemeID - 1) / 3 + 1;
		}
	}
#endif
}
#define SEND_ACTION_TIMES	17	//�ܹ�����17�ζ�������Ϣ
#define	SEND_ACTION_NUM		15	//ÿ�η���15��
static inline void StoreAction(void *arg)
{
	ActionItem *item = (ActionItem *)arg;
	int i;
	
	for (i = 0; i < SEND_ACTION_NUM; i++)
	{
		if (item[i].nActionID > 0 && item[i].nActionID <= NUM_ACTION)
		{
			if (item[i].nPhaseTableID == 0 || item[i].nPhaseTableID > MAX_PHASE_TABLE_COUNT)
				item[i].nPhaseTableID = 1;
			if (item[i].nChannelTableID == 0 || item[i].nChannelTableID > MAX_CHANNEL_TABLE_COUNT)
				item[i].nChannelTableID = 1;
			gSignalControlpara->stAction[item[i].nActionID - 1] = item[i];
		}
#if 0 //add by Jicky, ��Ϊ������Ķ�Ӧ��ϵ��һ��
		if (item[i].nActionID < 115)
			gSignalControlpara->stAction[item[i].nActionID - 1].nSchemeID = (item[i].nSchemeID + 2) / 3;
#endif
	}
}
#define SEND_VEHICLEDETECTOR_TIMES	4	//�ܹ�����4�γ������������Ϣ
#define	SEND_VEHICLEDETECTOR_NUM	18	//ÿ�η���18��
static inline void StoreVehicleDetector(void *arg)
{
	struct STRU_N_VehicleDetector *item = (struct STRU_N_VehicleDetector *)arg;
	int i;
	
	for (i = 0; i < SEND_VEHICLEDETECTOR_NUM; i++)
	{
		if (item[i].byVehicleDetectorNumber > 0 && item[i].byVehicleDetectorNumber <= MAX_VEHICLEDETECTOR_COUNT)
		{
			gSignalControlpara->AscVehicleDetectorTable[item[i].byVehicleDetectorNumber - 1] = item[i];
		}
	}
}
static inline void StorePedestrianDetector(void *arg)
{
	memcpy(gSignalControlpara->AscPedestrianDetectorTable, arg, sizeof(gSignalControlpara->AscPedestrianDetectorTable));
}
static inline void StoreFollowPhase(void *arg)
{
	memcpy(gSignalControlpara->stOldFollowPhase, arg, sizeof(gSignalControlpara->stOldFollowPhase));
	memcpy(gSignalControlpara->stFollowPhase[0], gSignalControlpara->stOldFollowPhase, sizeof(gSignalControlpara->stOldFollowPhase));
}

void StoreBegin(void *arg)
{
	UInt32 flag = *(UInt32 *)arg;
	int i;
	
	ntcipToGbFlag = (flag << 1);	//�����λ��Ϊ��Чλ
	for (i = 1; i <= 32; i++)
	{
		if (BIT(ntcipToGbFlag, i) == 0)
			continue;
		switch (i)
		{
			case PHASETURN_BIT: memset(gSignalControlpara->stPhaseTurn, 0, sizeof(gSignalControlpara->stPhaseTurn)); break;
			case SPLIT_BIT: memset(gSignalControlpara->stGreenSignalRation, 0, sizeof(gSignalControlpara->stGreenSignalRation)); break;
			case VEHDETECTOR_BIT: memset(gSignalControlpara->AscVehicleDetectorTable, 0, sizeof(gSignalControlpara->AscVehicleDetectorTable)); break;
			case PEDDETECTOR_BIT: memset(gSignalControlpara->AscPedestrianDetectorTable, 0, sizeof(gSignalControlpara->AscPedestrianDetectorTable)); break;
			case SCHEME_BIT: memset(gSignalControlpara->stScheme, 0, sizeof(gSignalControlpara->stScheme)); break;
			case ACTION_BIT: memset(gSignalControlpara->stAction, 0, sizeof(gSignalControlpara->stAction)); break;
			case TIMEINTERVAL_BIT: memset(gSignalControlpara->stTimeInterval, 0, sizeof(gSignalControlpara->stTimeInterval)); break;
			case SCHEDULE_BIT: memset(gSignalControlpara->stPlanSchedule, 0, sizeof(gSignalControlpara->stPlanSchedule)); break;
			default: break;
		}
	}
}

void DownloadConfig(int type, void *arg)
{
	switch (type)
	{
		case 0xaa: StoreTimeInterval(arg);  break;
		case 0xab: StorePlanSchedule(arg); break;
		case 0xac: StoreUnit(arg); break;
		case 0xad: StorePhase(arg); break;
		case 0xae: StorePedestrianDetector(arg); break;
		case 0xaf: StoreChannel(arg); break;
		case 0xb0: StoreVehicleDetector(arg); break;
		case 0xb1: StoreScheme(arg); break;
		case 0xb2: StoreSplit(arg); break;
		case 0xb3: StorePhaseTurn(arg); break;
		case 0xb4: StoreAction(arg); break;
		case 0xb5: StoreFollowPhase(arg); break;
		case 0xb6: StoreTransform(arg); break;
		//case 183: StoreBegin(arg); break;
		//case 184: StoreComplete(arg); break;
		case 0xeeeeeeee: memcpy(gSignalControlpara, arg, sizeof(SignalControllerPara)); break;
		default:  break;
	}
}

void LoadLocalConfigFile(ProtocolType type, void *config)
{
	int i;
	ChannelItem zero;

	if (config == NULL)
		return;
	switch (type)
	{
		case NTCIP:
    		READ_BIN_CFG_PARAMS(FILE_TSC_CFG_DAT, config, sizeof(SignalControllerPara));
			memset(&zero, 0, sizeof(zero));
			if (memcmp(&zero, &gSignalControlpara->stChannel[0][0], sizeof(zero)) == 0)
			{	//˵���ǵ�һ������֮ǰ�ϵ�����
				for (i = 0; i < NUM_ACTION; i++)
				{
					if (gSignalControlpara->stAction[i].nPhaseTableID == 0)
						gSignalControlpara->stAction[i].nPhaseTableID = 1;
					if (gSignalControlpara->stAction[i].nChannelTableID == 0)
						gSignalControlpara->stAction[i].nChannelTableID = 1;
				}
				memcpy(gSignalControlpara->stPhase[0], gSignalControlpara->stOldPhase, sizeof(gSignalControlpara->stOldPhase));
				memcpy(gSignalControlpara->AscSignalTransTable[0], gSignalControlpara->OldAscSignalTransTable, sizeof(gSignalControlpara->OldAscSignalTransTable));
				memcpy(gSignalControlpara->stChannel[0], gSignalControlpara->stOldChannel, sizeof(gSignalControlpara->stOldChannel));
				memcpy(gSignalControlpara->stFollowPhase[0], gSignalControlpara->stOldFollowPhase, sizeof(gSignalControlpara->stOldFollowPhase));
				WRITE_BIN_CFG_PARAMS(FILE_TSC_CFG_DAT, config, sizeof(SignalControllerPara));
			}
#if 1
			if (IsSignalControlparaLegal(config) == 0) //��������Ƿ����
			{	
				SendLCBconfigToBoard(config);
				INFO("Load loacl config file is successful!");
				//ItsWriteFaultLog(INIT_LOCAL_CONFIG_SUCC, 0);
			}
			else
			{
				ERR("Some parameters in the loacl config file is wrong!");
				memset(config, 0, sizeof(SignalControllerPara));
				//ItsWriteFaultLog(INIT_LOCAL_CONFIG_FAIL, 0);
			}
#endif
			break;
		case GB2007:
#ifdef USE_GB_PROTOCOL
    		READ_BIN_CFG_PARAMS(FILE_TSC_GB_CFG_DAT, config, sizeof(GbConfig));
#endif
			break;
	}
}

void WriteLoaclConfigFile(ProtocolType type, void *config)
{
	if (config == NULL)
		return;
	switch (type)
	{
		case NTCIP:
			WRITE_BIN_CFG_PARAMS(FILE_TSC_CFG_DAT, config, sizeof(SignalControllerPara));
			SendLCBconfigToBoard(config);
			break;
		case GB2007:
#ifdef USE_GB_PROTOCOL
			WRITE_BIN_CFG_PARAMS(FILE_TSC_GB_CFG_DAT, config, sizeof(GbConfig));
#endif
			break;
	}
}

