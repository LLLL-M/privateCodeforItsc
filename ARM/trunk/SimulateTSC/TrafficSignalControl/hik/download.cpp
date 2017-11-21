#include <cstring>
#include "hikconf.h"

using namespace std;

typedef enum
{
	INVALID_BIT = 0,
	UNIT_BIT = 1,
	PHASE_BIT = 2,
	PHASETURN_BIT = 3,
	SPLIT_BIT = 4,
	VEHDETECTOR_BIT = 5,
	PEDDETECTOR_BIT = 6,
	CHANNEL_BIT = 7,
	SCHEME_BIT = 8,
	ACTION_BIT = 9,
	TIMEINTERVAL_BIT = 10,
	SCHEDULE_BIT = 11,
	
	FOLLOWPHASE_BIT = 13,
	
	STAGE_TIMING_BIT = 17,
	CONFLICTPHASE_BIT = 18,
} ConvertBit;	//转换比特位

#define SEND_TIMEINTERVAL_TIMES		16	//总共发送16次时段表信息
#define SEND_TIMEINTERVAL_NUM		48	//每次发送48个
inline void Hikconf::StoreTimeInterval(void *arg)
{
	TimeIntervalItem *item = static_cast<TimeIntervalItem *>(arg);
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

#define SEND_PLANSCHEDULE_TIMES		4	//总共发送4次调度表信息
#define	SEND_PLANSCHEDULE_NUM		10	//每次发送10个
inline void Hikconf::StorePlanSchedule(void *arg)
{
	PlanScheduleItem *item = static_cast<PlanScheduleItem *>(arg);
	int i;
	
	for (i = 0; i < SEND_PLANSCHEDULE_NUM; i++) 
	{
		if (item[i].nScheduleID > 0 && item[i].nScheduleID <= NUM_SCHEDULE)
		{
			gSignalControlpara->stPlanSchedule[item[i].nScheduleID - 1] = item[i];
		}
	}
}

inline void Hikconf::StoreUnit(void *arg)
{
	memcpy(&gSignalControlpara->stUnitPara, arg, sizeof(UnitPara));
}
/*****************************************************************************
 函 数 名  : StorePhase
 功能描述  : 保存相位参数
 输入参数  : void *arg  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月18日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年3月18日
    作    者   : 肖文虎
    修改内容   : 相位表中全红时间、单位延长绿、黄灯时间SDK传过来的单位是1/10秒
                 ，故这这里保存时要除以10，以保证校验正确。
*****************************************************************************/
void Hikconf::StorePhase(void *arg)
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

inline void Hikconf::StoreTransform(void *arg)
{
	memcpy(gSignalControlpara->OldAscSignalTransTable, arg, sizeof(gSignalControlpara->OldAscSignalTransTable));
	memcpy(gSignalControlpara->AscSignalTransTable[0], gSignalControlpara->OldAscSignalTransTable, sizeof(gSignalControlpara->OldAscSignalTransTable));
}

#define SEND_PHASETURN_TIMES	16	//总共发送16次相序表信息
#define	SEND_PHASETURN_NUM		4	//每次发送4个
inline void Hikconf::StorePhaseTurn(void *arg)
{
    PhaseTurnItem *item = static_cast<PhaseTurnItem *>(arg);
    int i = 0;

    for(i = 0 ; i < SEND_PHASETURN_NUM; i++)
    {
		if (item[i].nPhaseTurnID > 0 && item[i].nPhaseTurnID <= NUM_PHASE_TURN
			&& item[i].nCircleID > 0 && item[i].nCircleID <= NUM_RING_COUNT)
		{
			memcpy(&(gSignalControlpara->stPhaseTurn[item[i].nPhaseTurnID - 1][item[i].nCircleID - 1]), &item[i], sizeof(PhaseTurnItem));//这里的相位数目不是16，而是32，
		}
    }

}

#define SEND_SPLIT_TIMES	36	//总共发送36次绿信比表信息
#define	SEND_SPLIT_NUM		16	//每次发送16个
inline void Hikconf::StoreSplit(void *arg)
{
	GreenSignalRationItem *item = static_cast<GreenSignalRationItem *>(arg);
	int i;
	
	for (i = 0; i < SEND_SPLIT_NUM; i++)
	{
		if (item[i].nGreenSignalRationID > 0 && item[i].nGreenSignalRationID <= NUM_GREEN_SIGNAL_RATION
			&& item[i].nPhaseID > 0 && item[i].nPhaseID <= NUM_PHASE)
		{
			gSignalControlpara->stGreenSignalRation[item[i].nGreenSignalRationID - 1][item[i].nPhaseID - 1] = item[i];
			//INFO("phase:%d, splitTime:%d", item[i].nPhaseID, item[i].nGreenSignalRationTime);
		}
	}
}

inline void Hikconf::StoreChannel(void *arg)
{
	memcpy(gSignalControlpara->stOldChannel, arg, sizeof(gSignalControlpara->stOldChannel));
	memcpy(gSignalControlpara->stChannel[0], gSignalControlpara->stOldChannel, sizeof(gSignalControlpara->stOldChannel));
}

#define SEND_SCHEME_TIMES	6	//总共发送6次方案表信息
#define	SEND_SCHEME_NUM		18	//每次发送18个
void Hikconf::StoreOldScheme(void *arg)
{
	SchemeItemOld *item = static_cast<SchemeItemOld *>(arg);
	int i;

	for (i = 0; i < SEND_SCHEME_NUM; i++)
	{
		if (item[i].nSchemeID > 0 && item[i].nSchemeID <= NUM_SCHEME)
		{
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1] = item[i];
			gSignalControlpara->stScheme[item[i].nSchemeID - 1].nSchemeID = item[i].nSchemeID;
			gSignalControlpara->stScheme[item[i].nSchemeID - 1].nCycleTime = item[i].nCycleTime;
			gSignalControlpara->stScheme[item[i].nSchemeID - 1].nOffset = item[i].nOffset;
			gSignalControlpara->stScheme[item[i].nSchemeID - 1].nGreenSignalRatioID = item[i].nGreenSignalRatioID;
			gSignalControlpara->stScheme[item[i].nSchemeID - 1].nPhaseTurnID = item[i].nPhaseTurnID;
#if 0
			INFO("scheme:%d, cycle:%d, offset:%d, split:%d, phaseturn:%d", 
			item[i].nSchemeID, item[i].nCycleTime, item[i].nOffset, item[i].nGreenSignalRatioID, item[i].nPhaseTurnID);
#endif
		}
	}
}

void Hikconf::StoreScheme(void *arg)
{
	SchemeItem *item = static_cast<SchemeItem *>(arg);
	int i;
#if 1	
	for (i = 0; i < SEND_SCHEME_NUM; i++)
	{
		if (item[i].nSchemeID > 0 && item[i].nSchemeID <= NUM_SCHEME)
		{
			gSignalControlpara->stScheme[item[i].nSchemeID - 1] = item[i];
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nSchemeID = item[i].nSchemeID;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nCycleTime = item[i].nCycleTime;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nOffset = item[i].nOffset;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nGreenSignalRatioID = item[i].nGreenSignalRatioID;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nPhaseTurnID = item[i].nPhaseTurnID;
		}
	}
#else	//add by Jicky, 因为方案表的对应关系不一致
	for (i = 0; i < SEND_SCHEME_NUM; i++)
	{	
		if (item[i].nSchemeID >= 249)	
		{	//特殊方案
			gSignalControlpara->stScheme[item[i].nSchemeID - 1] = item[i];
			continue;
		}
		if ((item[i].nSchemeID - 1) % 3 == 0)
		{	//普通方案
			gSignalControlpara->stScheme[(item[i].nSchemeID - 1) / 3] = item[i];
			gSignalControlpara->stScheme[(item[i].nSchemeID - 1) / 3].nSchemeID = (item[i].nSchemeID - 1) / 3 + 1;
		}
	}
#endif
}

#define SEND_ACTION_TIMES	17	//总共发送17次动作表信息
#define	SEND_ACTION_NUM		15	//每次发送15个
void Hikconf::StoreAction(void *arg)
{
	ActionItem *item = static_cast<ActionItem *>(arg);
	int i;
	
	for (i = 0; i < SEND_ACTION_NUM; i++)
	{
        if (item[i].nActionID > 0)
		{
			if (item[i].nPhaseTableID == 0 || item[i].nPhaseTableID > MAX_PHASE_TABLE_COUNT)
				item[i].nPhaseTableID = 1;
			if (item[i].nChannelTableID == 0 || item[i].nChannelTableID > MAX_CHANNEL_TABLE_COUNT)
				item[i].nChannelTableID = 1;
			gSignalControlpara->stAction[item[i].nActionID - 1] = item[i];
		}
#if 0 //add by Jicky, 因为方案表的对应关系不一致
		if (item[i].nActionID < 115)
			gSignalControlpara->stAction[item[i].nActionID - 1].nSchemeID = (item[i].nSchemeID + 2) / 3;
#endif
	}
}

#define SEND_VEHICLEDETECTOR_TIMES	4	//总共发送4次车辆检测器表信息
#define	SEND_VEHICLEDETECTOR_NUM	18	//每次发送18个
inline void Hikconf::StoreVehicleDetector(void *arg)
{
	struct STRU_N_VehicleDetector *item = static_cast<struct STRU_N_VehicleDetector *>(arg);
	int i;
	
	for (i = 0; i < SEND_VEHICLEDETECTOR_NUM; i++)
	{
		if (item[i].byVehicleDetectorNumber > 0 && item[i].byVehicleDetectorNumber <= MAX_VEHICLEDETECTOR_COUNT)
		{
			gSignalControlpara->AscVehicleDetectorTable[item[i].byVehicleDetectorNumber - 1] = item[i];
		}
	}
}

inline void Hikconf::StorePedestrianDetector(void *arg)
{
	memcpy(gSignalControlpara->AscPedestrianDetectorTable, arg, sizeof(gSignalControlpara->AscPedestrianDetectorTable));
}

inline void Hikconf::StoreFollowPhase(void *arg)
{
	memcpy(gSignalControlpara->stOldFollowPhase, arg, sizeof(gSignalControlpara->stOldFollowPhase));
	memcpy(gSignalControlpara->stFollowPhase[0], gSignalControlpara->stOldFollowPhase, sizeof(gSignalControlpara->stOldFollowPhase));
}

void Hikconf::StoreBegin(UInt32 flag)
{
	int i;
	
	for (i = 0; i < 32; i++)
	{
		if (BIT(flag, i) == 0)
			continue;
		switch (i + 1)
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

void Hikconf::DownloadConfig(UInt32 type, void *arg)
{
	switch (type)
	{
		case 0xaa: StoreTimeInterval(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_TIME_INTERVAL); break;
		case 0xab: StorePlanSchedule(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_SCHEDULE); break;
		case 0xac: StoreUnit(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_UNIT); break;
		case 0xad: StorePhase(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_PHASE); break;
		case 0xae: StorePedestrianDetector(arg);  break;
		case 0xaf: StoreChannel(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_CHANNEL); break;
		case 0xb0: StoreVehicleDetector(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_VEHICLE); break;
		case 0xb1: StoreOldScheme(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_SCHEME); break;
		case 0xb2: StoreSplit(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_GREEN_SPLIT); break;
		case 0xb3: StorePhaseTurn(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_PHASE_TURN); break;
		case 0xb4: StoreAction(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_ACTION); break;
		case 0xb5: StoreFollowPhase(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_FOLLOW_PHASE); break;
		case 0xb6: StoreTransform(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_SIGNAL_TRANS); break;
		case 0xdd: StoreScheme(arg); SET_BIT(iIsSaveHikconfig, E_TABLE_NAME_SCHEME); break;
		//case 183: StoreBegin(arg); break;
		//case 184: StoreComplete(arg); break;
		case 0xeeeeeeee: memcpy(gSignalControlpara, arg, sizeof(SignalControllerPara)); break;
		default:  break;
	}

}
void Hikconf::save_hikconfig_to_database()
{
	int i = 0;
	sqlite3 *pdatabase = NULL;

	psqlite_conf->sqlite3_open_wrapper(DATABASE_HIKCONFIG, &pdatabase);
    psqlite_conf->sqlite3_begin(pdatabase);
	for (i = 0; i < E_TABLE_NAME_SIGNAL_TRANS + 1; i++)
	{
		if (GET_BIT(iIsSaveHikconfig, i) == 1)
		{	
			switch(i)
			{
				case E_TABLE_NAME_UNIT:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_UNIT);
					psqlite_conf->sqlite3_insert_unit(pdatabase, &(gSignalControlpara->stUnitPara));
					break;
				case E_TABLE_NAME_PHASE: 
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_PHASE);
					psqlite_conf->sqlite3_insert_phase(pdatabase, gSignalControlpara->stPhase);
					psqlite_conf->sqlite3_insert_signal_trans(pdatabase, gSignalControlpara->AscSignalTransTable);
					break;
				case E_TABLE_NAME_PHASE_TURN:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_PHASE_TURN);
					psqlite_conf->sqlite3_insert_phase_turn(pdatabase, gSignalControlpara->stPhaseTurn);
					break;
				case E_TABLE_NAME_GREEN_SPLIT:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_GREEN_SPLIT);
					psqlite_conf->sqlite3_insert_green_split(pdatabase, gSignalControlpara->stGreenSignalRation);
					break;
				case E_TABLE_NAME_CHANNEL: 
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CHANNEL);
					psqlite_conf->sqlite3_insert_channel(pdatabase, gSignalControlpara->stChannel);
					break;
				case E_TABLE_NAME_SCHEME:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_SCHEME);
					psqlite_conf->sqlite3_insert_scheme(pdatabase, gSignalControlpara->stScheme);
					break;
				case E_TABLE_NAME_ACTION:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_ACTION);
					psqlite_conf->sqlite3_insert_action(pdatabase, gSignalControlpara->stAction);
					break;
				case E_TABLE_NAME_TIME_INTERVAL:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_TIME_INTERVAL);
					psqlite_conf->sqlite3_insert_timeinterval(pdatabase, gSignalControlpara->stTimeInterval);
					break;
				case E_TABLE_NAME_SCHEDULE:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_SCHEDULE);
					psqlite_conf->sqlite3_insert_schedule(pdatabase, gSignalControlpara->stPlanSchedule);
					break;
				case E_TABLE_NAME_VEHICLE:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_VEHICLE);
					psqlite_conf->sqlite3_insert_vehicle(pdatabase, gSignalControlpara->AscVehicleDetectorTable);
					break;
				case E_TABLE_NAME_FOLLOW_PHASE:
					psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_FOLLOW_PHASE);
					psqlite_conf->sqlite3_insert_follow_phase(pdatabase, gSignalControlpara->stFollowPhase);
					break;
				/*case E_TABLE_NAME_SIGNAL_TRANS:
					sqlite3_clear_table(pdatabase, TABLE_NAME_SIGNAL_TRANS);
					sqlite3_insert_signal_trans(pdatabase, pconfig->AscSignalTransTable);
					break;*/
				default:
					break;
			}
		}
	}
    psqlite_conf->sqlite3_commit(pdatabase);
	psqlite_conf->sqlite3_close_wrapper(pdatabase);
    iIsSaveHikconfig = 0;
}

void Hikconf::LoadLocalConfigFile()
{
	sqlite3* pdb = NULL;
	
	if (gSignalControlpara == NULL)
		return;
	//file.Read(gSignalControlpara, sizeof(SignalControllerPara));
        if (psqlite_conf->sqlite3_open_wrapper(DATABASE_HIKCONFIG, &pdb) != 0)
		return;
    psqlite_conf->read_hikconfig(pdb, gSignalControlpara);//read_hikconfig();
    psqlite_conf->sqlite3_close_wrapper(pdb);

}

void Hikconf::WriteLoaclConfigFile()
{
	if (gSignalControlpara == NULL)
			return;

    save_hikconfig_to_database();
	iIsSaveHikconfig = 0;
		//sqlite3_bak_db(DATABASE_HIKCONFIG);
}

Hikconf::Hikconf(Sqliteconf* psqliteconf, Hiktsc* phiktsc):psqlite_conf(psqliteconf),pHiktsc(phiktsc)
{
	gSignalControlpara = new SignalControllerPara;
	if (gSignalControlpara == NULL)
		exit(-1);

	iIsSaveHikconfig = 0;
    memset(gSignalControlpara, 0, sizeof(SignalControllerPara));
    LoadLocalConfigFile();
    phiktsc->ItsSetConfig(gSignalControlpara, sizeof(SignalControllerPara));
}


Hikconf::~Hikconf()
{
	if (gSignalControlpara != NULL)
	{
		//file.Unmap(gSignalControlpara, sizeof(SignalControllerPara));
		//file.Close();
		delete gSignalControlpara;
	}
}
