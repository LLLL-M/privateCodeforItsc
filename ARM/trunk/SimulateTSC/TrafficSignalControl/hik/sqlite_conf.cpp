#include "hik.h"
#include "HikConfig.h"
#include "common.h"
#include "platform.h"
#include "configureManagement.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

using namespace std;

int Sqliteconf::sqlite3_open_wrapper(const char* db_file, sqlite3 **ppdb)
{
	int ret = 0;

	ret = sqlite3_open(db_file, ppdb);
	if (ret != SQLITE_OK)             
    {  
       return -1;                   /*打开或创建失败返回*/
    }

	return 0;
}

int Sqliteconf::sqlite3_close_wrapper(sqlite3* pdb)
{
	sqlite3_close(pdb);
	pdb = NULL;
	return 0;
}

int Sqliteconf::sqlite3_begin(sqlite3* pdatabase)
{
    int ret = 0;
    ret = sqlite3_exec(pdatabase, "begin;", 0, 0, 0);
    if (ret != SQLITE_OK)
        return -1;
    return 0;
}

int Sqliteconf::sqlite3_commit(sqlite3* pdatabase)
{
    int ret = 0;
    ret = sqlite3_exec(pdatabase, "commit;", 0, 0, 0);
    if (ret != SQLITE_OK)
        return -1;
    return 0;

}

int Sqliteconf::sqlite3_create_table_wrapper(sqlite3* pdb, const char* sql)
{
	int ret = 0;
	char* zErrMsg = NULL;
	//char* create_table_sql = sql;
	//const char* create_table_sql = "CREATE TABLE conf_list (id integer primary key, conf_struct_desc varchar(128) UNIQUE, struct_value blob);";
	
	ret = sqlite3_exec(pdb, sql, 0, 0, &zErrMsg);
	if (ret != SQLITE_OK)
	{
		if (zErrMsg != NULL)
		{
			//ERR("sqlite3 create table failed, err=%s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		return -1;
	}
	return 0;
}

int Sqliteconf::sqlite3_drop_table_wrapper(sqlite3* pdb, const char* table_name)
{
	int ret = 0;
	char* zErrMsg = NULL;
	char sql[128] = {0};

	sprintf(sql, "drop table %s;", table_name);
	ret = sqlite3_exec(pdb, sql, 0, 0, &zErrMsg);
	if (ret != SQLITE_OK)
	{
		if (zErrMsg != NULL)
		{
			//ERR("sqlite3 drop table failed, err=%s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		return -1;
	}
	return 0;
}

int Sqliteconf::sqlite3_clear_table(sqlite3* pdb, const char* table_name)
{
	int ret = 0;
	char* zErrMsg = NULL;
	char sql[128] = {0};

	sprintf(sql, "delete from %s;", table_name);
	ret = sqlite3_exec(pdb, sql, 0, 0, &zErrMsg);
	if (ret != SQLITE_OK)
	{
		if (zErrMsg != NULL)
		{
			//ERR("sqlite3 drop table failed, err=%s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		return -1;
	}
	return 0;
}

int Sqliteconf::sqlite3_insert_blob_column(sqlite3* pdatabase, const char* table, const char* column_name, void* column_value, int value_size)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char str_sql[128] = {0};

	if (column_value == NULL || value_size <= 0)
		return -1;
	
	sprintf(str_sql, "insert into %s (%s) values(?);", table, column_name);
	ret = sqlite3_prepare(pdatabase, str_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	ret = sqlite3_bind_blob(stmt, 1, column_value, value_size, NULL);
	ret = sqlite3_step(stmt);
	if(ret != SQLITE_DONE)
	{
		ERR("insert blob column failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	return 0;
}


int Sqliteconf::sqlite3_update_blob_column(sqlite3* pdatabase, const char* table, const char* column_name, int row_num, void* column_value, int value_size, int offset)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char str_sql[128] = {0};
	int rowid = 0;
	int bytes = 0;
	sqlite3_blob* pblob = NULL;

	if (column_value == NULL || value_size <= 0)
		return -1;
	
	sprintf(str_sql, "select ROWID, %s from %s where id=%d", column_name, table, row_num);
	ret = sqlite3_prepare(pdatabase, str_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
	{
		//bytes = sqlite3_column_bytes(stmt, 1);
		rowid = sqlite3_column_int(stmt, 0);
		//ret = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);/*free stmt*/

	ret = sqlite3_blob_open(pdatabase, "main", table, column_name, rowid, 1, &pblob);
    if(ret != SQLITE_OK)
	{
		ERR("open blob failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	bytes = sqlite3_blob_bytes(pblob);
	if (value_size > (bytes - offset))/*write data size didn't over the blob's end*/
	{
		INFO("write data size over the blob's end\n");
		sqlite3_blob_close(pblob);
		return -1;
	}
	ret = sqlite3_blob_write(pblob, (void*)column_value, value_size, offset);
	if (ret != SQLITE_OK)
	{
		ERR("write blob error, err=%s\n", sqlite3_errmsg(pdatabase));
		sqlite3_blob_close(pblob);
		return -1;
	}
		
	sqlite3_blob_close(pblob);
	return 0;
}

int Sqliteconf::sqlite3_select_blob_column(sqlite3* pdatabase, const char* table, const char* column_name, int row_num, void* column_value, int value_size, int offset)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char str_sql[128] = {0};
	int rowid = 0;
	int bytes = 0;
	sqlite3_blob* pblob = NULL;
	
	if (column_value == NULL || value_size <= 0)
		return -1;
	
	sprintf(str_sql, "select ROWID, %s from %s where id=%d", column_name, table, row_num);
	ret = sqlite3_prepare(pdatabase, str_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
		return -1;
	
	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
	{
		//bytes = sqlite3_column_bytes(stmt, 1);
		rowid = sqlite3_column_int(stmt, 0);
		//ret = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);/*free stmt*/

	ret = sqlite3_blob_open(pdatabase, "main", table, column_name, rowid, 1, &pblob);
    if(ret != SQLITE_OK)
	{
		ERR("open blob failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	bytes = sqlite3_blob_bytes(pblob);
	if (value_size > (bytes - offset))/*read data size  over the blob's end*/
	{
		INFO("read data size over the blob's end\n");
		sqlite3_blob_close(pblob);
		return -1;
	}
	ret = sqlite3_blob_read(pblob, (void*)column_value, value_size, offset);
	if (ret != SQLITE_OK)
	{
		ERR("read blob error,err=%s\n", sqlite3_errmsg(pdatabase));
		sqlite3_blob_close(pblob);
		return -1;
	}
	
	sqlite3_blob_close(pblob);
	return 0;
}


int Sqliteconf::sqlite3_select_column(sqlite3* pdatabase, const char* table, const char* column_name, int row_num, void* column_value)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char sel_sql[128] = {0};
	int col_type = SQLITE_NULL;
	int col_bytes = 0;

	if (column_value == NULL)
		return -1;
	
	sprintf(sel_sql, "select %s from %s where ROWID=%d;", column_name, table, row_num);

	ret = sqlite3_prepare(pdatabase, sel_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
		return -1;
	ret = sqlite3_step(stmt);
	while (ret == SQLITE_ROW)
	{
		col_type = sqlite3_column_type(stmt, 0);
		switch (col_type)
		{
			case SQLITE_INTEGER:
				*((int*)column_value) = sqlite3_column_int(stmt, 0);
				break;
			case SQLITE_TEXT:
				col_bytes = sqlite3_column_bytes(stmt, 0);
				memcpy(column_value, (const char*)sqlite3_column_text(stmt, 0), col_bytes);
				break;
			case SQLITE_BLOB:
				col_bytes = sqlite3_column_bytes(stmt, 0);
				memcpy(column_value, sqlite3_column_blob(stmt, 0), col_bytes);
				break;
			default:
				break;
		}
		break;//just read 1st row values
	}

	sqlite3_finalize(stmt);
	
	return 0;
}

int Sqliteconf::sqlite3_insert_column(sqlite3* pdatabase, const char* table, const char* column_name, void* column_value, int value_size, int column_type)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	//char* insert_sql = "insert into STRUCT_BINFILE_CONFIG (iErrorDetectSwitch, iCurrentAlarmSwitch, iVoltageAlarmSwitch) values (?,?,?);";
	char insert_column_sql[128] = {0};
	
	if (column_value == NULL || value_size <= 0)
		return -1;
	sprintf(insert_column_sql, "insert into %s (%s) values (?)", table, column_name);
	ret = sqlite3_prepare(pdatabase, insert_column_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	switch (column_type)
	{
		case SQLITE_INTEGER:
			sqlite3_bind_int(stmt, 1, *((int*)(column_value)));
			break;
		case SQLITE_TEXT:
			sqlite3_bind_text(stmt, 1, (char*)column_value, value_size, NULL);
			break;
		case SQLITE_BLOB:
			sqlite3_bind_blob(stmt, 1, column_value, value_size, NULL);
			break;
		default: 
			break;
	}
	
	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert column(%s)  failed,err=%s\n", column_name, sqlite3_errmsg(pdatabase));
		return -1;
    }
	sqlite3_finalize(stmt);
	return 0;
}

/*
*params: value_size, column_type
*if column_type is SQLITE_TEXT,need set value_size to the length of text or set -1 as default(text size end with 0.)
*now just support SQLITE_INTEGER,SQLITE_TEXT.
*/
int Sqliteconf::sqlite3_update_column(sqlite3* pdatabase, const char* table_name, const char* column_name, int row_id, void* column_value, int value_size, int column_type)
{
	int ret = 0;
	int tmp = 0;
	sqlite3_stmt *stmt = NULL;
	char update_sql[256] = {0};
	if (column_value == NULL || value_size <= 0)
		return -1;

	switch (column_type)
	{
		case SQLITE_INTEGER:
			if (value_size == 1)
				tmp = *((UINT8*)column_value);
			else if (value_size == 2)
				tmp = *((UINT16*)column_value);
			else
				tmp = *((UINT32*)column_value);
			sprintf(update_sql, "update %s set %s=%d where ROWID=%d;", table_name, column_name, tmp, row_id);
			break;
		case SQLITE_FLOAT:
			break;
		case SQLITE_TEXT:
			sprintf(update_sql, "update %s set %s='%s' where ROWID=%d;", table_name, column_name, (char*)column_value, row_id);
		case SQLITE_BLOB:
			//sprintf(update_sql, "update %s set %s='%s' where ROWID=%d;", table_name, column_name, (char*)value, row_id);
			ret = sqlite3_update_blob_column(pdatabase, table_name, column_name, row_id, column_value, value_size, 0);
			return ret;
			break;
		case SQLITE_NULL:
			break;
		default:
			break;
	}
		
	//FPRINTF(stderr, "update_sql = %s\n", update_sql);

	ret = sqlite3_prepare(pdatabase, update_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("update column value failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	sqlite3_finalize(stmt);

	return 0;
}

/*sql="select ROEID from table where ..."*/
int get_row_id(sqlite3* pdatabase, char* sql)
{
	int ret = 0;
	int rowid = 0;
	sqlite3_stmt *stmt = NULL;
	
	ret = sqlite3_prepare(pdatabase, sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("sqlite3 prepare failed.\n");
		return -1;
	}
	
	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
	{
		rowid = sqlite3_column_int(stmt, 0);
		//ret = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);/*free stmt*/
	return rowid;
}

int Sqliteconf::sqlite3_update_text_blob(sqlite3* pdatabase, const char* table, const char* column_name, int rowid, void* column_value, int value_size, int offset)
{
	int ret = 0;
	ret = sqlite3_update_blob_column(pdatabase, table, column_name, rowid, column_value, value_size, offset);
	return ret;	
}
int Sqliteconf::sqlite3_select_text_blob(sqlite3* pdatabase, const char* table, const char* column_name, int rowid, void* column_value, int value_size, int offset)
{
	int ret = 0;
	ret = sqlite3_select_blob_column(pdatabase, table, column_name, rowid, column_value, value_size, offset);
	return ret;
}



int convert_chars_to_string(unsigned char arr[], int num, char ret_buf[1024])
{
	int i = 0;
	char* p = ret_buf;
	if (ret_buf == NULL || num <= 0)
	{
		INFO("ret_buf is NULL or arr is null, error param.\n");
		return -1;
	}
	memset(ret_buf, 0, 1024);
	for (i = 0; i < num - 1; i++) 
	{
		sprintf(p, "%d,", arr[i]);
		p = strchr(p, ',') + 1;
	}
	sprintf(p, "%d", arr[num - 1]);
	return 0;
}

int convert_string_to_chars(char* str, unsigned char arr[], int num)
{
	int i = 0;
	int value = 0;
	char* p = str;

	if (str == NULL || num <= 0)
	{
		INFO("ret_buf is NULL or arr is null, error param.\n");
		return -1;
	}
	memset(arr, 0, num);
	for (i = 0; i < num; i++) 
	{
		if (1 != sscanf(p, "%d,%*s", &value)) 
		{
			return (i == 0) ? 0 : i;
		}
		arr[i] = (unsigned char)value;
		p = strchr(p, ',');
		if (p == NULL) 
		{
			break;
		}
		p += 1;
	}
	return 0;
}


TableInfo config_tables[5] = {
	{TABLE_NAME_CONF_SPECIALPRM, "create table %s (id INTEGER PRIMARY KEY, \
					iErrorDetectSwitch integer, iCurrentAlarmSwitch integer, iVoltageAlarmSwitch integer, \
					iCurrentAlarmAndProcessSwitch integer, iVoltageAlarmAndProcessSwitch integer, iWatchdogSwitch integer, iGpsSwitch integer, \
					iSignalMachineType integer, iRedSignalCheckSwitch integer, iPhaseTakeOverSwtich integer, isCameraKakou integer);"},
	{TABLE_NAME_CONF_WIRELESS, "create table %s (id INTEGER PRIMARY KEY, \
					iSwitch integer, iOvertime integer, iCtrlMode integer, description varchar(64), ucChan varchar);"},
	{TABLE_NAME_CONF_FRONTBOARD, "create table %s (id INTEGER PRIMARY KEY, \
					iSwitch integer, description varchar(64), ucChan varchar);"},
	{TABLE_NAME_CONF, "create table %s (id INTEGER PRIMARY KEY, \
					cCarDetectSwitch integer, cPrintfLogSwitch integer, cFailureNumber integer, \
					RedCurrentBase integer, RedCurrentDiff integer);"},
	{TABLE_NAME_CONF_SYS_INFOS, "create table %s (id INTEGER PRIMARY KEY, \
 					cSSID varchar(32), cPSK varchar(32), cName varchar(32), cPasswd varchar(32), \
 					uDevID integer, cDevDesc varchar(64), iRGSignalSwitch integer, cMcastAddr varchar(16), uMcastPort integer, \
 					cCarDetectorType integer, stVedioDetIP_address varchar(16), stVedioDetIP_subnetMask varchar(16),\
					stVedioDetIP_gateway varchar(16));"}
	};

TableInfo custom_tables[4] = {
	{TABLE_NAME_CUSTOM_COUNTDOWN, "create table %s (id INTEGER PRIMARY KEY, \
					iCountDownMode integer, iFreeGreenTime integer,  iPulseGreenTime integer, iPulseRedTime integer, \
					iphase integer, iType integer, option integer, redFlashSec integer, cIsCountdownValueLimit integer);"},
	{TABLE_NAME_CUSTOM_COM, "create table %s (id INTEGER PRIMARY KEY, \
					unExtraParamValue integer, unBaudRate integer, unDataBits integer, unStopBits integer, unParity integer);"},
	{TABLE_NAME_CUSTOM_CHANNELLOCK, "create table %s (id INTEGER PRIMARY KEY, \
					ucChannelStatus varchar, ucWorkingTimeFlag integer, ucBeginTimeHour integer, ucBeginTimeMin integer, ucBeginTimeSec integer, \
					ucEndTimeHour integer, ucEndTimeMin integer, ucEndTimeSec integer, ucReserved integer, cChannelLockFlag integer);"},
	{TABLE_NAME_CUSTOM_MULCHANNELLOCK, "create table %s (id INTEGER PRIMARY KEY, \
					cLockFlag integer, ucChannelStatus varchar, ucWorkingTimeFlag integer, \
					ucBeginTimeHour integer, ucBeginTimeMin integer, ucBeginTimeSec integer, \
					ucEndTimeHour integer, ucEndTimeMin integer, ucEndTimeSec integer);"}
	};
TableInfo desc_tables[5] = {
	{TABLE_NAME_DESC_PHASE, "create table %s (id INTEGER PRIMARY KEY, \
					phaseTableNo integer, phaseID integer, phaseDescText varchar(64));"},
	{TABLE_NAME_DESC_CHANNEL, "create table %s (id INTEGER PRIMARY KEY, \
					channelNo integer, stChannelDesc varchar(64));"},
	{TABLE_NAME_DESC_PATTERN, "create table %s (id INTEGER PRIMARY KEY, \
					schemeNo integer, stPatternNameDesc varchar(64));"},
	{TABLE_NAME_DESC_PLAN, "create table %s (id INTEGER PRIMARY KEY, \
					timeIntervalNo integer, stPlanNameDesc varchar(64));"},
	{TABLE_NAME_DESC_DATE, "create table %s (id INTEGER PRIMARY KEY, \
					scheduleNo integer, dateType integer, dateName varchar(64));"}
	};
TableInfo misc_tables[1] = {
	{TABLE_NAME_MISC, "create table %s (id INTEGER PRIMARY KEY, \
					cIsCanRestartHiktscAllowed integer, time_zone_gap integer);"}
	};
TableInfo countdown_cfg_tables[1] = {
	{TABLE_NAME_COUNTDOWN_CFG, "create table %s (id INTEGER PRIMARY KEY, \
					cDeviceId integer, cControllerID varchar(128), cControllerType integer, nChannelFlag integer);"}
	};

TableInfo hikconfig_tables_name[12] = {
	{TABLE_NAME_UNIT, "create table %s (id INTEGER PRIMARY KEY, \
					nBootYellowLightTime integer, nBootAllRedTime integer, byTransCycle integer, byFlashFrequency integer, \
					byFluxCollectCycle integer, byCollectCycleUnit integer, cIsCanRestartHiktscAllowed integer);"},
	{TABLE_NAME_PHASE, "create table %s (id INTEGER PRIMARY KEY, \
					phaseTableNo integer, nPhaseID integer, nPedestrianPassTime integer, nPedestrianClearTime integer, \
					nMinGreen integer, nUnitExtendGreen integer, nMaxGreen_1 integer, nMaxGreen_2 integer, nYellowTime integer, nAllRedTime integer, \
					wPhaseOptions integer, nCircleID integer, byPhaseConcurrency varchar(64), nGreenLightTime integer);"},
	{TABLE_NAME_PHASE_TURN, "create table %s (id INTEGER PRIMARY KEY, \
					nPhaseTurnID integer, nCircleID integer, nTurnArray varchar(64));"},
	{TABLE_NAME_GREEN_SPLIT, "create table %s (id INTEGER PRIMARY KEY, \
					nGreenSignalRationID integer, nPhaseID integer, nGreenSignalRationTime integer, \
					nType integer, nIsCoordinate integer);"},
	{TABLE_NAME_CHANNEL, "create table %s (id INTEGER PRIMARY KEY, \
					channelTableNO integer, nChannelID integer, nControllerID integer, nControllerType integer);"},
	{TABLE_NAME_SCHEME, "create table %s (id INTEGER PRIMARY KEY, \
					nSchemeID integer, nCycleTime integer, nOffset integer, nGreenSignalRatioID integer, \
					nPhaseTurnID integer);"},
	{TABLE_NAME_ACTION, "create table %s (id INTEGER PRIMARY KEY, \
					nActionID integer, nSchemeID integer, nPhaseTableID integer, nChannelTableID integer);"},
	{TABLE_NAME_TIME_INTERVAL, "create table %s (id INTEGER PRIMARY KEY, \
					nTimeIntervalID integer, nTimeID integer, cStartTimeHour integer, cStartTimeMinute integer, nActionID integer);"},
	{TABLE_NAME_SCHEDULE, "create table %s (id INTEGER PRIMARY KEY, \
					nScheduleID integer, month integer, week integer, day integer, nTimeIntervalID integer);"},
	{TABLE_NAME_VEHICLE, "create table %s (id INTEGER PRIMARY KEY, \
					byVehicleDetectorNumber integer, byVehicleDetectorCallPhase integer,byVehicleDetectorQueueLimit integer, \
					byVehicleDetectorNoActivity integer, byVehicleDetectorMaxPresence integer, byVehicleDetectorErraticCounts integer, byVehicleDetectorFailTime integer);"},
	{TABLE_NAME_FOLLOW_PHASE, "create table %s (id INTEGER PRIMARY KEY, \
					followPhaseTableNo integer, nFollowPhaseID integer, nArrayMotherPhase varchar(64));"},
	{TABLE_NAME_SIGNAL_TRANS, "create table %s (id INTEGER PRIMARY KEY, \
					byPhaseNumber integer, byRedYellow integer, nGreenLightTime integer, bySafeRed integer)"}
};

void Sqliteconf::create_config_tables(sqlite3* pdatabase)
{
	int i = 0;
	char create_table_sql[1024] = {0};

	for (i = 0; i < 5; i++)
	{
		sqlite3_drop_table_wrapper(pdatabase, config_tables[i].name);
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, config_tables[i].sql, config_tables[i].name);		
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
}
void Sqliteconf::create_custom_tables(sqlite3* pdatabase)
{
	int i = 0;
	char create_table_sql[1024] = {0};

	for (i = 0; i < 4; i++)
	{
		sqlite3_drop_table_wrapper(pdatabase, custom_tables[i].name);
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, custom_tables[i].sql, custom_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}

}
void Sqliteconf::create_desc_tables(sqlite3* pdatabase)
{
	int i = 0;
	char create_table_sql[1024] = {0};

	for (i = 0; i < 5; i++)
	{
		sqlite3_drop_table_wrapper(pdatabase, desc_tables[i].name);
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, desc_tables[i].sql, desc_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
}

void Sqliteconf::create_misc_tables(sqlite3* pdatabase)
{
	int i = 0;
	char create_table_sql[1024] = {0};

	for (i = 0; i < 1; i++)
	{
		sqlite3_drop_table_wrapper(pdatabase, misc_tables[i].name);
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, misc_tables[i].sql, misc_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
}

void Sqliteconf::create_countdown_cfg_tables(sqlite3* pdatabase)
{
	int i = 0;
	char create_table_sql[1024] = {0};

	for (i = 0; i < 1; i++)
	{
		sqlite3_drop_table_wrapper(pdatabase, countdown_cfg_tables[i].name);
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, countdown_cfg_tables[i].sql, countdown_cfg_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
}
void Sqliteconf::init_db_file_tables(void)
{
	int i = 0;
	sqlite3* pdatabase = NULL;
	char create_table_sql[1024] = {0};
	
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);

	for (i = 0; i < 5; i++)
	{
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, config_tables[i].sql, config_tables[i].name);		
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
	for (i = 0; i < 4; i++)
	{
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, custom_tables[i].sql, custom_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
	for (i = 0; i < 5; i++)
	{
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, desc_tables[i].sql, desc_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
	for (i = 0; i < 1; i++)
	{
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, misc_tables[i].sql, misc_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
		if (get_row_id(pdatabase, "select ROWID from misc where id=1") == 0)
		{
			STRUCT_BINFILE_MISC misc;
			memset(&misc, 0, sizeof(STRUCT_BINFILE_MISC));
			write_misc(pdatabase, &misc);
		}
	}
	for (i = 0; i < 1; i++)
	{
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, countdown_cfg_tables[i].sql, countdown_cfg_tables[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
	sqlite3_close_wrapper(pdatabase);
	pdatabase = NULL;

	sqlite3_open_wrapper(DATABASE_HIKCONFIG, &pdatabase);
	for (i = 0; i < 11; i++)
	{
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, hikconfig_tables_name[i].sql, hikconfig_tables_name[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
	}
	sqlite3_close_wrapper(pdatabase);
	pdatabase = NULL;
	
}

int Sqliteconf::sqlite3_insert_specialparams(sqlite3* pdatabase, STRU_SPECIAL_PARAMS* pspecial_param)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into config_sSpecialParams (iErrorDetectSwitch, iCurrentAlarmSwitch, iVoltageAlarmSwitch, \
					iCurrentAlarmAndProcessSwitch, iVoltageAlarmAndProcessSwitch, iWatchdogSwitch, \
					iGpsSwitch, iSignalMachineType, iRedSignalCheckSwitch, iPhaseTakeOverSwtich, isCameraKakou) \
					values (?,?,?,?,?,?,?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_bind_int(stmt, 1, pspecial_param->iErrorDetectSwitch);
	ret = sqlite3_bind_int(stmt, 2, pspecial_param->iCurrentAlarmSwitch);
	ret = sqlite3_bind_int(stmt, 3, pspecial_param->iVoltageAlarmSwitch);
	ret = sqlite3_bind_int(stmt, 4, pspecial_param->iCurrentAlarmAndProcessSwitch);
	ret = sqlite3_bind_int(stmt, 5, pspecial_param->iVoltageAlarmAndProcessSwitch);
	ret = sqlite3_bind_int(stmt, 6, pspecial_param->iWatchdogSwitch);
	ret = sqlite3_bind_int(stmt, 7, pspecial_param->iGpsSwitch);
	ret = sqlite3_bind_int(stmt, 8, pspecial_param->iSignalMachineType);
	ret = sqlite3_bind_int(stmt, 9, pspecial_param->iRedSignalCheckSwitch);
	ret = sqlite3_bind_int(stmt, 10, pspecial_param->iPhaseTakeOverSwtich);
	ret = sqlite3_bind_int(stmt, 11, pspecial_param->isCameraKakou);

	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert config_sSpecialParams  failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_specialparams(sqlite3* pdatabase, STRU_SPECIAL_PARAMS* pspecial_param)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from config_sSpecialParams;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		pspecial_param->iErrorDetectSwitch = sqlite3_column_int(stmt, 1);
		pspecial_param->iCurrentAlarmSwitch = sqlite3_column_int(stmt, 2);
		pspecial_param->iVoltageAlarmSwitch = sqlite3_column_int(stmt, 3);
		pspecial_param->iCurrentAlarmAndProcessSwitch = sqlite3_column_int(stmt, 4);
		pspecial_param->iVoltageAlarmAndProcessSwitch = sqlite3_column_int(stmt, 5);
		pspecial_param->iWatchdogSwitch = sqlite3_column_int(stmt, 6);
		pspecial_param->iGpsSwitch = sqlite3_column_int(stmt, 7);
		pspecial_param->iSignalMachineType = sqlite3_column_int(stmt, 8);
		pspecial_param->iRedSignalCheckSwitch = sqlite3_column_int(stmt, 9);
		pspecial_param->iPhaseTakeOverSwtich = sqlite3_column_int(stmt, 10);
		pspecial_param->isCameraKakou = sqlite3_column_int(stmt, 11);
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_wireless_controller(sqlite3* pdatabase, STRU_WIRELESS_CONTROLLER_INFO* pwireless_ctrl)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into config_stWirelessController (iSwitch, iOvertime, iCtrlMode, description, ucChan) \
					values (?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_bind_int(stmt, 1, pwireless_ctrl->iSwitch);
	ret = sqlite3_bind_int(stmt, 2, pwireless_ctrl->iOvertime);
	ret = sqlite3_bind_int(stmt, 3, pwireless_ctrl->iCtrlMode);
	ret = sqlite3_bind_text(stmt, 4, pwireless_ctrl->key[0].description, 64, NULL);/*MAX_WIRELESS_KEY-1 */
	memset(str, 0, 1024);
	convert_chars_to_string(pwireless_ctrl->key[0].ucChan, 32, str);
	//FPRINTF(stderr, "ucChan=%s\n", str);
	ret = sqlite3_bind_text(stmt, 5, (const char*)(str), strlen(str), NULL);

	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert config_stWirelessController  failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	for (i = 1; i < (MAX_WIRELESS_KEY-1); i++)
	{
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_text(stmt, 4, pwireless_ctrl->key[i].description, 64, NULL);/*MAX_WIRELESS_KEY-1 */
		memset(str, 0, 1024);
		convert_chars_to_string(pwireless_ctrl->key[i].ucChan, 32, str);
		ret = sqlite3_bind_text(stmt, 5, (const char*)(str), strlen(str), NULL);
		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert config_stWirelessController key[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
	    }
	}
	sqlite3_finalize(stmt);
	return 0;


}

int Sqliteconf::sqlite3_select_wireless_controller(sqlite3* pdatabase, STRU_WIRELESS_CONTROLLER_INFO* pwireless_ctrl)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from config_stWirelessController;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		if (i < 1)
		{
			pwireless_ctrl->iSwitch = sqlite3_column_int(stmt, 1);
			pwireless_ctrl->iOvertime = sqlite3_column_int(stmt, 2);
			pwireless_ctrl->iCtrlMode = sqlite3_column_int(stmt, 3);
		}
		if (i < (MAX_WIRELESS_KEY-1))
		{
			memcpy(pwireless_ctrl->key[i].description, sqlite3_column_text(stmt, 4), 64);
			memset(str, 0, 1024);
			memcpy(str, sqlite3_column_text(stmt, 5), sqlite3_column_bytes(stmt, 5));
			convert_string_to_chars(str, pwireless_ctrl->key[i].ucChan, 32);
		}
		i++;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}


int Sqliteconf::sqlite3_insert_frontboardkey(sqlite3* pdatabase, STRU_FRONTBOARD_KEY_INFO* pfbk)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into config_sFrontBoardKeys (iSwitch, description, ucChan) \
					values (?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_bind_int(stmt, 1, pfbk->iSwitch);
	ret = sqlite3_bind_text(stmt, 2, pfbk->key[0].description, 64, NULL);/*MAX_FRONT_BOARD_KEY-5 */
	memset(str, 0, 1024);
	convert_chars_to_string(pfbk->key[0].ucChan, 32, str);
	ret = sqlite3_bind_text(stmt, 3, (const char*)(str), strlen(str), NULL);

	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert config_sFrontBoardKeys  failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	for (i = 1; i < (MAX_FRONT_BOARD_KEY-5); i++)
	{
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_text(stmt, 2, pfbk->key[i].description, 64, NULL);/*MAX_FRONT_BOARD_KEY-5*/
		memset(str, 0, 1024);
		convert_chars_to_string(pfbk->key[i].ucChan, 32, str);
		ret = sqlite3_bind_text(stmt, 3, (const char*)(str), strlen(str), NULL);
		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert config_sFrontBoardKeys key[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
	    }
	}
	sqlite3_finalize(stmt);
	return 0;	
}

int Sqliteconf::sqlite3_select_frontboardkey(sqlite3* pdatabase, STRU_FRONTBOARD_KEY_INFO* pfbk)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from config_sFrontBoardKeys;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		if (i < 1)
		{
			pfbk->iSwitch = sqlite3_column_int(stmt, 1);
		}
		if (i < (MAX_FRONT_BOARD_KEY-5))
		{
			memcpy(pfbk->key[i].description, sqlite3_column_text(stmt, 2), 64);
			memset(str, 0, 1024);
			memcpy(str, sqlite3_column_text(stmt, 3), sqlite3_column_bytes(stmt, 3));
			convert_string_to_chars(str, pfbk->key[i].ucChan, 32);
		}
		i++;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_config_pieces(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG* pconfig)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into config (cCarDetectSwitch, cPrintfLogSwitch, cFailureNumber, \
					RedCurrentBase, RedCurrentDiff) values (?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_bind_int(stmt, 1, pconfig->cCarDetectSwitch);
	ret = sqlite3_bind_int(stmt, 2, pconfig->cPrintfLogSwitch);
	ret = sqlite3_bind_int(stmt, 3, pconfig->cFailureNumber);
	ret = sqlite3_bind_int(stmt, 4, pconfig->sCurrentParams[0].RedCurrentBase);
	ret = sqlite3_bind_int(stmt, 5, pconfig->sCurrentParams[0].RedCurrentDiff);

	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert config  failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	for (i = 1; i < 32; i++)/*sCurrentParams[32], need insert 32 row*/
	{
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 4, pconfig->sCurrentParams[i].RedCurrentBase);
		ret = sqlite3_bind_int(stmt, 5, pconfig->sCurrentParams[i].RedCurrentDiff);	
		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			ERR("insert config's sCurrentParams[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
		}		
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_config_pieces(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG* pconfig)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from config;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		if (i < 1)
		{
			pconfig->cCarDetectSwitch = sqlite3_column_int(stmt, 1);
			pconfig->cPrintfLogSwitch = sqlite3_column_int(stmt, 2);
			pconfig->cFailureNumber = sqlite3_column_int(stmt, 3);
		}
		pconfig->sCurrentParams[i].RedCurrentBase = sqlite3_column_int(stmt, 4);
		pconfig->sCurrentParams[i].RedCurrentDiff = sqlite3_column_int(stmt, 5);
		i++;
		if (i >= 32)
			break;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_current_params(sqlite3* pdatabase, STRU_CURRENT_PARAMS* pcurrent_params)
{
return 0;
}

int Sqliteconf::sqlite3_select_current_params(sqlite3* pdatabase, STRU_CURRENT_PARAMS* pcurrent_params)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select RedCurrentBase, RedCurrentDiff from config;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		pcurrent_params[i].RedCurrentBase = sqlite3_column_int(stmt, 0);
		pcurrent_params[i].RedCurrentDiff = sqlite3_column_int(stmt, 1);
		i++;
		if (i >= 32)
			break;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_config_sys_infos(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG *pconfig)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into config_sys_infos (cSSID, cPSK, cName, cPasswd, \
 					uDevID, cDevDesc, iRGSignalSwitch, cMcastAddr, uMcastPort, \
 					cCarDetectorType, stVedioDetIP_address, stVedioDetIP_subnetMask,\
					stVedioDetIP_gateway) values (?,?,?,?,?,?,?,?,?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	sqlite3_bind_text(stmt, 1, (const char*)pconfig->stWifi.cSSID, 32, NULL);
	sqlite3_bind_text(stmt, 2, (const char*)pconfig->stWifi.cPSK, 32, NULL);
	sqlite3_bind_text(stmt, 3, (const char*)pconfig->stUser.cName, 32, NULL);
	sqlite3_bind_text(stmt, 4, (const char*)pconfig->stUser.cPasswd, 32, NULL);
	sqlite3_bind_int(stmt, 5, pconfig->stDevice.uDevID);
	sqlite3_bind_text(stmt, 6, (const char*)pconfig->stDevice.cDevDesc, 64, NULL);
	sqlite3_bind_int(stmt, 7, pconfig->stRGSignalCheck.iRGSignalSwitch);
	sqlite3_bind_text(stmt, 8, (const char*)pconfig->stRGSignalCheck.cMcastAddr, 16, NULL);
	sqlite3_bind_int(stmt, 9, pconfig->stRGSignalCheck.uMcastPort);
	sqlite3_bind_int(stmt, 10, pconfig->stCarDetector.cCarDetectorType);
	sqlite3_bind_text(stmt, 11, (const char*)pconfig->stCarDetector.stVedioDetIP.address, 16, NULL);
	sqlite3_bind_text(stmt, 12, (const char*)pconfig->stCarDetector.stVedioDetIP.subnetMask, 16, NULL);
	sqlite3_bind_text(stmt, 13, (const char*)pconfig->stCarDetector.stVedioDetIP.gateway, 16, NULL);
	
	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert config  failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_select_wifi_info(sqlite3* pdatabase, STRU_WIFI_INFO* pwifi)
{
	int ret = 0;
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cSSID", 1, pwifi->cSSID);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cPSK", 1, pwifi->cPSK);
	return ret;
}
int Sqliteconf::sqlite3_update_wifi_info(sqlite3* pdatabase, STRU_WIFI_INFO* pwifi)
{ 	int ret = 0;
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cSSID", 1, pwifi->cSSID, 32, 0);
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cPSK", 1, pwifi->cPSK, 32, 0);
	return ret;
}

int Sqliteconf::sqlite3_select_user_info(sqlite3* pdatabase, STRU_SYS_USER_INFO *puserinfo)
{
	int ret = 0;
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cName", 1, puserinfo->cName);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cPasswd", 1, puserinfo->cPasswd);
	return ret;
}

int Sqliteconf::sqlite3_update_user_info(sqlite3* pdatabase, STRU_SYS_USER_INFO *puserinfo)
{
	int ret = 0;
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cName", 1, puserinfo->cName, 32, 0);
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cPasswd", 1, puserinfo->cPasswd, 32, 0);
	return ret;
}

int Sqliteconf::sqlite3_select_device_info(sqlite3* pdatabase,STRU_DEVICE_INFO *pdeviceinfo)
{
	int ret = 0;
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "uDevID", 1, &pdeviceinfo->uDevID);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cDevDesc", 1, pdeviceinfo->cDevDesc);
	return ret;
}

int Sqliteconf::sqlite3_update_device_info(sqlite3* pdatabase,STRU_DEVICE_INFO *pdeviceinfo)
{
	int ret = 0;
	ret += sqlite3_update_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "uDevID", 1, &pdeviceinfo->uDevID, 4, SQLITE_INTEGER);
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cDevDesc", 1, pdeviceinfo->cDevDesc, 64, 0);
	return ret;
}

int Sqliteconf::sqlite3_select_rgsig_info(sqlite3* pdatabase,STRU_RGSIGNAL_CHECK_INFO *prgsig_info)
{
	int ret = 0;
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "iRGSignalSwitch", 1, &prgsig_info->iRGSignalSwitch);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cMcastAddr", 1, prgsig_info->cMcastAddr);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "uMcastPort", 1, &prgsig_info->uMcastPort);
	return ret;
}

int Sqliteconf::sqlite3_update_rgsig_info(sqlite3* pdatabase,STRU_RGSIGNAL_CHECK_INFO *prgsig_info)
{
	int ret = 0;
	ret += sqlite3_update_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "iRGSignalSwitch", 1, &prgsig_info->iRGSignalSwitch, 4, SQLITE_INTEGER);
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cMcastAddr", 1, prgsig_info->cMcastAddr, 16, 0);
	ret += sqlite3_update_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "uMcastPort", 1, &prgsig_info->uMcastPort, 4, SQLITE_INTEGER);
	return ret;
}

int Sqliteconf::sqlite3_select_car_det_info(sqlite3* pdatabase,STRU_CAR_DETECT_INFO *pcar_det)
{
	int ret = 0;
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "cCarDetectorType", 1, &pcar_det->cCarDetectorType);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "stVedioDetIP_address", 1, pcar_det->stVedioDetIP.address);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "stVedioDetIP_subnetMask", 1, pcar_det->stVedioDetIP.subnetMask);
	ret += sqlite3_select_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "stVedioDetIP_gateway", 1, pcar_det->stVedioDetIP.gateway);
	return ret;
}

int Sqliteconf::sqlite3_update_car_det_info(sqlite3* pdatabase,STRU_CAR_DETECT_INFO *pcar_det)
{
	int ret = 0;
	ret += sqlite3_update_column(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "iRGSignalSwitch", 1, &pcar_det->cCarDetectorType, 1, SQLITE_INTEGER);
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "stVedioDetIP_address", 1, pcar_det->stVedioDetIP.address, 16, 0);
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "stVedioDetIP_subnetMask", 1, pcar_det->stVedioDetIP.subnetMask, 16, 0);
	ret += sqlite3_update_text_blob(pdatabase, TABLE_NAME_CONF_SYS_INFOS, "stVedioDetIP_gateway", 1, pcar_det->stVedioDetIP.gateway, 16, 0);
	return ret;
}


int Sqliteconf::read_config(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG *pconfig)
{
	int ret = 0;

	ret += sqlite3_select_specialparams(pdatabase, &(pconfig->sSpecialParams));
	ret += sqlite3_select_wireless_controller(pdatabase, &(pconfig->stWirelessController));
	ret += sqlite3_select_frontboardkey(pdatabase, &(pconfig->sFrontBoardKeys));
	ret += sqlite3_select_config_pieces(pdatabase, pconfig);
	ret += sqlite3_select_wifi_info(pdatabase, &(pconfig->stWifi));
	ret += sqlite3_select_user_info(pdatabase, &(pconfig->stUser));
	ret += sqlite3_select_device_info(pdatabase, &(pconfig->stDevice));
	ret += sqlite3_select_rgsig_info(pdatabase, &(pconfig->stRGSignalCheck));
	ret += sqlite3_select_car_det_info(pdatabase, &(pconfig->stCarDetector));
	return (ret == 0) ? 0 : -1;
}
int Sqliteconf::write_config(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG *pconfig)
{
	int ret = 0;
	//INFO("write config begin\n");
	create_config_tables(pdatabase);
	
	ret += sqlite3_insert_specialparams(pdatabase, &(pconfig->sSpecialParams));
	ret += sqlite3_insert_wireless_controller(pdatabase, &(pconfig->stWirelessController));
	ret += sqlite3_insert_frontboardkey(pdatabase, &(pconfig->sFrontBoardKeys));
	ret += sqlite3_insert_config_pieces(pdatabase, pconfig);
	ret += sqlite3_insert_config_sys_infos(pdatabase, pconfig);
	//INFO("write config end\n");
	return (ret == 0) ? 0 : -1;
}

int Sqliteconf::sqlite3_insert_countdown_prm(sqlite3* pdatabase, STRU_Count_Down_Params* pcountdown_param)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into custom_sCountdownParams (iCountDownMode, iFreeGreenTime, iPulseGreenTime, iPulseRedTime, \
		iphase, iType, option, redFlashSec) values (?,?,?,?,?,?,?,?);";
	
		ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
		if (ret != SQLITE_OK)
		{
			ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
			return -1;
		}
	
		ret = sqlite3_bind_int(stmt, 1, pcountdown_param->iCountDownMode);
		ret = sqlite3_bind_int(stmt, 2, pcountdown_param->iFreeGreenTime);
		ret = sqlite3_bind_int(stmt, 3, pcountdown_param->iPulseGreenTime);
		ret = sqlite3_bind_int(stmt, 4, pcountdown_param->iPulseRedTime);
		ret = sqlite3_bind_int(stmt, 5, pcountdown_param->iPhaseOfChannel[0].iphase);/*array [32], need insert 32 row*/
		ret = sqlite3_bind_int(stmt, 6, pcountdown_param->iPhaseOfChannel[0].iType);
		ret = sqlite3_bind_int(stmt, 7, pcountdown_param->option);
		ret = sqlite3_bind_int(stmt, 8, pcountdown_param->redFlashSec);
		
		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			ERR("insert custom_sCountdownParams  failed,err=%s\n", sqlite3_errmsg(pdatabase));
			return -1;
		}
	
		for (i = 1; i < 32; i++)
		{
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
			ret = sqlite3_bind_int(stmt, 5, pcountdown_param->iPhaseOfChannel[i].iphase);/*array [32], need insert 32 row*/
			ret = sqlite3_bind_int(stmt, 6, pcountdown_param->iPhaseOfChannel[i].iType);
			ret = sqlite3_step(stmt);
			if(ret != SQLITE_DONE)
			{
				ERR("insert custom_sCountdownParams's  iPhaseOfChannel[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
				return -1;
			}
		}
		sqlite3_finalize(stmt);
		return 0;

}

int Sqliteconf::sqlite3_select_countdown_prm(sqlite3* pdatabase, STRU_Count_Down_Params* pcountdown_param)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from custom_sCountdownParams;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i == 0)
		{
			pcountdown_param->iCountDownMode = sqlite3_column_int(stmt, 1);
			pcountdown_param->iFreeGreenTime = sqlite3_column_int(stmt, 2);
			pcountdown_param->iPulseGreenTime = sqlite3_column_int(stmt, 3);
			pcountdown_param->iPulseRedTime = sqlite3_column_int(stmt, 4);
			
			pcountdown_param->option = sqlite3_column_int(stmt, 7);
			pcountdown_param->redFlashSec = sqlite3_column_int(stmt, 8);
			
		}
		if (i < 32)
		{
			pcountdown_param->iPhaseOfChannel[i].iphase = sqlite3_column_int(stmt, 5);/*array [32], need read 32 row*/
			pcountdown_param->iPhaseOfChannel[i].iType = sqlite3_column_int(stmt, 6);
		}
		i++;
		if (i >= 32)
			break;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_com_prm(sqlite3* pdatabase, COM_PARAMS* pcom_param)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into custom_sComParams (unExtraParamValue, unBaudRate, unDataBits, unStopBits, unParity) \
					values (?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < 4; i++)
	{
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, pcom_param[i].unExtraParamValue);
		ret = sqlite3_bind_int(stmt, 2, pcom_param[i].unBaudRate);
		ret = sqlite3_bind_int(stmt, 3, pcom_param[i].unDataBits);
		ret = sqlite3_bind_int(stmt, 4, pcom_param[i].unStopBits);
		ret = sqlite3_bind_int(stmt, 5, pcom_param[i].unParity);

		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert custom_sComParams  failed,err=%s\n", sqlite3_errmsg(pdatabase));
			return -1;
	    }
	}

	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_select_com_prm(sqlite3* pdatabase, COM_PARAMS* pcom_param)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from custom_sComParams;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		pcom_param[i].unExtraParamHead = 0x6e6e;
		pcom_param[i].unExtraParamID = 0xa2;
		pcom_param[i].unExtraParamValue = sqlite3_column_int(stmt, 1);
		pcom_param[i].unBaudRate = sqlite3_column_int(stmt, 2);
		pcom_param[i].unDataBits = sqlite3_column_int(stmt, 3);
		pcom_param[i].unStopBits = sqlite3_column_int(stmt, 4);
		pcom_param[i].unParity = sqlite3_column_int(stmt, 5);
		i++;
		if (i >= 4)
			break;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}
int Sqliteconf::sqlite3_insert_chlock_prm(sqlite3* pdatabase, CHANNEL_LOCK_PARAMS* pchlock_param)
{
	int ret = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into custom_sChannelLockedParams (ucChannelStatus, ucWorkingTimeFlag, \
		ucBeginTimeHour, ucBeginTimeMin, ucBeginTimeSec, \
		ucEndTimeHour, ucEndTimeMin, ucEndTimeSec, ucReserved) values (?,?,?,?,?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	memset(str, 0, 1024);
	convert_chars_to_string(pchlock_param->ucChannelStatus, 32, str);
	ret = sqlite3_bind_text(stmt, 1, (const char*)(str), strlen(str), NULL);
	ret = sqlite3_bind_int(stmt, 2, pchlock_param->ucWorkingTimeFlag);
	ret = sqlite3_bind_int(stmt, 3, pchlock_param->ucBeginTimeHour);
	ret = sqlite3_bind_int(stmt, 4, pchlock_param->ucBeginTimeMin);
	ret = sqlite3_bind_int(stmt, 5, pchlock_param->ucBeginTimeSec);
	ret = sqlite3_bind_int(stmt, 6, pchlock_param->ucEndTimeHour);
	ret = sqlite3_bind_int(stmt, 7, pchlock_param->ucEndTimeMin);
	ret = sqlite3_bind_int(stmt, 8, pchlock_param->ucEndTimeSec);
	ret = sqlite3_bind_int(stmt, 9, pchlock_param->ucReserved);
	

	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert custom_sChannelLockedParams  failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }
	
	sqlite3_finalize(stmt);
	return 0;
	
}

int Sqliteconf::sqlite3_select_chlock_prm(sqlite3* pdatabase, CHANNEL_LOCK_PARAMS* pchlock_param)
{
	int ret = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from custom_sChannelLockedParams;"};

	pchlock_param->unExtraParamHead = 0x6e6e;
	pchlock_param->unExtraParamID = 0xb9;
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		memset(str, 0, 1024);
		memcpy(str, sqlite3_column_text(stmt, 1), sqlite3_column_bytes(stmt, 1));
		convert_string_to_chars(str, pchlock_param->ucChannelStatus, 32);
		pchlock_param->ucWorkingTimeFlag = sqlite3_column_int(stmt, 2);
		pchlock_param->ucBeginTimeHour = sqlite3_column_int(stmt, 3);
		pchlock_param->ucBeginTimeMin = sqlite3_column_int(stmt, 4);
		pchlock_param->ucBeginTimeSec = sqlite3_column_int(stmt, 5);
		pchlock_param->ucEndTimeHour = sqlite3_column_int(stmt, 6);
		pchlock_param->ucEndTimeMin = sqlite3_column_int(stmt, 7);
		pchlock_param->ucEndTimeSec = sqlite3_column_int(stmt, 8);
		pchlock_param->ucReserved = sqlite3_column_int(stmt, 9);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_mp_chlock_prm(sqlite3* pdatabase, STU_MUL_PERIODS_CHAN_PARAMS *pmp_chlock_param)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into custom_sMulPeriodsChanLockParams (ucChannelStatus, ucWorkingTimeFlag, \
		ucBeginTimeHour, ucBeginTimeMin, ucBeginTimeSec, \
		ucEndTimeHour, ucEndTimeMin, ucEndTimeSec) values (?,?,?,?,?,?,?,?);";
	
	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	for (i = 0; i < MAX_CHAN_LOCK_PERIODS; i++)
	{
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		memset(str, 0, 1024);
		convert_chars_to_string(pmp_chlock_param->chans[i].ucChannelStatus, 32, str);
		ret = sqlite3_bind_text(stmt, 1, (const char*)(str), strlen(str), NULL);
		ret = sqlite3_bind_int(stmt, 2, pmp_chlock_param->chans[i].ucWorkingTimeFlag);
		ret = sqlite3_bind_int(stmt, 3, pmp_chlock_param->chans[i].ucBeginTimeHour);
		ret = sqlite3_bind_int(stmt, 4, pmp_chlock_param->chans[i].ucBeginTimeMin);
		ret = sqlite3_bind_int(stmt, 5, pmp_chlock_param->chans[i].ucBeginTimeSec);
		ret = sqlite3_bind_int(stmt, 6, pmp_chlock_param->chans[i].ucEndTimeHour);
		ret = sqlite3_bind_int(stmt, 7, pmp_chlock_param->chans[i].ucEndTimeMin);
		ret = sqlite3_bind_int(stmt, 8, pmp_chlock_param->chans[i].ucEndTimeSec);
		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert custom_sMulPeriodsChanLockParams  failed,err=%s\n", sqlite3_errmsg(pdatabase));
			return -1;
	    }
	}
	sqlite3_finalize(stmt);

	sqlite3_update_column(pdatabase, "custom_sMulPeriodsChanLockParams", "cLockFlag", 1, &(pmp_chlock_param->cLockFlag), 1, SQLITE_INTEGER);
	
	return 0;

}

int Sqliteconf::sqlite3_select_mp_chlock_prm(sqlite3* pdatabase, STU_MUL_PERIODS_CHAN_PARAMS *pmp_chlock_param)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from custom_sMulPeriodsChanLockParams;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_step(stmt);	

	while (ret == SQLITE_ROW)
	{
		if (i < 1)
		{
		pmp_chlock_param->cLockFlag = sqlite3_column_int(stmt, 1);
		}
		memset(str, 0, 1024);
		memcpy(str, sqlite3_column_text(stmt, 2), sqlite3_column_bytes(stmt, 2));
		convert_string_to_chars(str, pmp_chlock_param->chans[i].ucChannelStatus, 32);
		pmp_chlock_param->chans[i].ucWorkingTimeFlag = sqlite3_column_int(stmt, 3);
		pmp_chlock_param->chans[i].ucBeginTimeHour = sqlite3_column_int(stmt, 4);
		pmp_chlock_param->chans[i].ucBeginTimeMin = sqlite3_column_int(stmt, 5);
		pmp_chlock_param->chans[i].ucBeginTimeSec = sqlite3_column_int(stmt, 6);
		pmp_chlock_param->chans[i].ucEndTimeHour = sqlite3_column_int(stmt, 7);
		pmp_chlock_param->chans[i].ucEndTimeMin = sqlite3_column_int(stmt, 8);
		pmp_chlock_param->chans[i].ucEndTimeSec = sqlite3_column_int(stmt, 9);
		i++;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::read_custom(sqlite3* pdatabase, STRUCT_BINFILE_CUSTOM* pcustom)
{
	int ret = 0;

	ret += sqlite3_select_countdown_prm(pdatabase, &(pcustom->sCountdownParams));
	ret += sqlite3_select_com_prm(pdatabase, pcustom->sComParams);
	ret += sqlite3_select_chlock_prm(pdatabase, &(pcustom->sChannelLockedParams));
	ret += sqlite3_select_mp_chlock_prm(pdatabase, &(pcustom->sMulPeriodsChanLockParams));
	ret += sqlite3_select_column(pdatabase, "custom_sCountdownParams", "cIsCountdownValueLimit", 1, &(pcustom->cIsCountdownValueLimit));
	ret += sqlite3_select_column(pdatabase, "custom_sChannelLockedParams", "cChannelLockFlag", 1, &(pcustom->cChannelLockFlag));
	
	return (ret == 0) ? 0 : -1;
}
int Sqliteconf::write_custom(sqlite3* pdatabase, STRUCT_BINFILE_CUSTOM* pcustom)
{
	int ret = 0;
	//INFO("write custom begin\n");
	create_custom_tables(pdatabase);
	
	ret += sqlite3_insert_countdown_prm(pdatabase, &(pcustom->sCountdownParams));
	ret += sqlite3_insert_com_prm(pdatabase, pcustom->sComParams);
	ret += sqlite3_insert_chlock_prm(pdatabase, &(pcustom->sChannelLockedParams));
	ret += sqlite3_insert_mp_chlock_prm(pdatabase, &(pcustom->sMulPeriodsChanLockParams));

	/*update left custom pieces*/
	ret += sqlite3_update_column(pdatabase, "custom_sCountdownParams", "cIsCountdownValueLimit", 1, &(pcustom->cIsCountdownValueLimit), 1, SQLITE_INTEGER);
	ret += sqlite3_update_column(pdatabase, "custom_sChannelLockedParams", "cChannelLockFlag", 1, &(pcustom->cChannelLockFlag),  1, SQLITE_INTEGER);
	//INFO("write custom end\n");
	return (ret == 0) ? 0 : -1;
}

int Sqliteconf::sqlite3_insert_phase_desc(sqlite3* pdatabase, PhaseDescText phase_desc[16][16])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into desc_phaseDescText (phaseTableNo, phaseID, phaseDescText) values (?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < MAX_PHASE_TABLE_COUNT; i++)
	{
		for (j = 0; j < NUM_PHASE; j++)
		{
			if (0 == strlen((char*)phase_desc[i][j]))
				continue;
			
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
			ret = sqlite3_bind_int(stmt, 1, i + 1);
			ret = sqlite3_bind_int(stmt, 2, j + 1);
			ret = sqlite3_bind_text(stmt, 3, (const char*)(phase_desc[i][j]), 64, NULL);
			ret = sqlite3_step(stmt);
	    	if(ret != SQLITE_DONE)
			{
				ERR("insert column phaseDescText[%d][%d] value failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
				return -1;
	    	}	
		}
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_phase_desc(sqlite3* pdatabase, PhaseDescText phase_desc[16][16])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select phaseTableNo, phaseID, phaseDescText from desc_phaseDescText;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_PHASE)
		{
			j = 1;
			
			if (i >= MAX_PHASE_TABLE_COUNT)
				break;
		}
		i = sqlite3_column_int(stmt, 0);
		j = sqlite3_column_int(stmt, 1);
		memcpy(phase_desc[i-1][j-1], sqlite3_column_blob(stmt, 2), sqlite3_column_bytes(stmt, 2));
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_channel_desc(sqlite3* pdatabase, CHANNEL_DESC_PARAMS *pchannel_desc)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into desc_sChannelDescParams (channelNo, stChannelDesc) values (?,?);";
	
	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	for (i = 0; i < NUM_CHANNEL; i++)
	{
		if (0 == strlen((char*)pchannel_desc->stChannelDesc[i]))
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, i + 1);
		ret = sqlite3_bind_text(stmt, 2, (const char*)(pchannel_desc->stChannelDesc[i]), 64, NULL);
		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert column stChannelDesc[%d] value failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
	    }		
	}
	sqlite3_finalize(stmt);
	return 0;
	
}
int Sqliteconf::sqlite3_select_channel_desc(sqlite3* pdatabase, CHANNEL_DESC_PARAMS *pchannel_desc)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char select_sql[] = {"select channelNo, stChannelDesc from desc_sChannelDescParams;"};

	pchannel_desc->unExtraParamHead = 0x6e6e;
	pchannel_desc->unExtraParamID = 0x9d;
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= NUM_CHANNEL)
			break;
		i = sqlite3_column_int(stmt, 0);
		memcpy(pchannel_desc->stChannelDesc[i-1], sqlite3_column_blob(stmt, 1), 64);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_pattern_name_desc(sqlite3* pdatabase, PATTERN_NAME_PARAMS *ppattern_name)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into desc_sPatternNameParams (schemeNo, stPatternNameDesc) values (?,?);";
	
	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	for (i = 0; i < 16; i++)
	{
		if (0 == strlen((char*)ppattern_name->stPatternNameDesc[i]))
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, i * 3 + 1);
		ret = sqlite3_bind_text(stmt, 2, (const char*)(ppattern_name->stPatternNameDesc[i]), 64, NULL);
		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert column stPatternNameDesc[%d] value failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
	    }		
	}
	sqlite3_finalize(stmt);
	return 0;


}

int Sqliteconf::sqlite3_select_pattern_name_desc(sqlite3* pdatabase, PATTERN_NAME_PARAMS *ppattern_name)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char select_sql[] = {"select schemeNo, stPatternNameDesc from desc_sPatternNameParams;"};

	ppattern_name->unExtraParamHead = 0x6e6e;
	ppattern_name->unExtraParamID = 0xa5;
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if ((i / 3) >= 16)
			break;
		i = sqlite3_column_int(stmt, 0);
		memcpy(ppattern_name->stPatternNameDesc[i / 3], sqlite3_column_blob(stmt, 1), 64);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_plan_name(sqlite3* pdatabase, PLAN_NAME_PARAMS *pplan_name)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into desc_sPlanNameParams (timeIntervalNo, stPlanNameDesc) values (?,?);";
		
	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}


	for (i = 0; i < NUM_TIME_INTERVAL; i++)
	{
		if (0 == strlen((char*)pplan_name->stPlanNameDesc[i]))
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, i + 1);
		ret = sqlite3_bind_text(stmt, 2, (const char*)(pplan_name->stPlanNameDesc[i]), 64, NULL);
		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert column stPlanNameDesc[%d] value failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
	    }		
	}
	sqlite3_finalize(stmt);
	return 0;	
}

int Sqliteconf::sqlite3_select_plan_name(sqlite3* pdatabase, PLAN_NAME_PARAMS *pplan_name)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char select_sql[] = {"select timeIntervalNo, stPlanNameDesc from desc_sPlanNameParams;"};

	pplan_name->unExtraParamHead = 0x6e6e;
	pplan_name->unExtraParamID = 0xa7;
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= NUM_TIME_INTERVAL)
			break;
		i = sqlite3_column_int(stmt, 0);
		memcpy(pplan_name->stPlanNameDesc[i-1], sqlite3_column_blob(stmt, 1), 64);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_plan_date(sqlite3* pdatabase, DATE_NAME_PARAMS *pdate_name)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into desc_sDateNameParams (scheduleNo, dateType, dateName, id) values (?,?,?,?);";
		
	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	for (i = 0; i < 40; i++)
	{
		if (0 == strlen((char*)pdate_name->stNameDesc[i].dateName))
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, i + 1);
		ret = sqlite3_bind_int(stmt, 2, pdate_name->stNameDesc[i].dateType);
		ret = sqlite3_bind_text(stmt, 3, (const char*)(pdate_name->stNameDesc[i].dateName), 64, NULL);
		ret = sqlite3_bind_int(stmt, 4, i + 1);
		ret = sqlite3_step(stmt);
	    if(ret != SQLITE_DONE)
		{
			ERR("insert  stNameDesc[%d] value failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
	    }		
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_select_plan_date(sqlite3* pdatabase, DATE_NAME_PARAMS *pdate_name)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char select_sql[] = {"select scheduleNo, dateType, dateName from desc_sDateNameParams;"};

	pdate_name->unExtraParamHead = 0x6e6e;
	pdate_name->unExtraParamID = 0xa9;
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= 40)
			break;

		i = sqlite3_column_int(stmt, 0);
		pdate_name->stNameDesc[i-1].dateType = sqlite3_column_int(stmt, 1);
		memcpy(pdate_name->stNameDesc[i-1].dateName, sqlite3_column_blob(stmt, 2), 64);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::read_desc(sqlite3* pdatabase, STRUCT_BINFILE_DESC *pdesc)
{
	int ret = 0;
	
	ret += sqlite3_select_phase_desc(pdatabase, pdesc->phaseDescText);
	memcpy(pdesc->sPhaseDescParams.stPhaseDesc, pdesc->phaseDescText[0], 16 * 64);
	pdesc->sPhaseDescParams.unExtraParamHead = 0x6e6e;
	pdesc->sPhaseDescParams.unExtraParamID = 0x9b;
	
	ret += sqlite3_select_channel_desc(pdatabase, &(pdesc->sChannelDescParams));
	ret += sqlite3_select_pattern_name_desc(pdatabase, &(pdesc->sPatternNameParams));
	ret += sqlite3_select_plan_name(pdatabase, &(pdesc->sPlanNameParams));
	ret += sqlite3_select_plan_date(pdatabase, &(pdesc->sDateNameParams));

	return (ret == 0) ? 0 : -1;
}
int Sqliteconf::write_desc(sqlite3* pdatabase, STRUCT_BINFILE_DESC *pdesc)
{
	int ret = 0;
	int i = 0;
	//INFO("write desc begin\n");
	//create_desc_tables(pdatabase);
	for (i = 0; i < 5; i++)
	{
		sqlite3_clear_table(pdatabase, desc_tables[i].name);
	}
	
	ret += sqlite3_insert_phase_desc(pdatabase, pdesc->phaseDescText);
	//INFO("write desc sqlite3_insert_phase_desc\n");
	ret += sqlite3_insert_channel_desc(pdatabase, &(pdesc->sChannelDescParams));
	//INFO("write desc sqlite3_insert_channel_desc\n");
	ret += sqlite3_insert_pattern_name_desc(pdatabase, &(pdesc->sPatternNameParams));
	//INFO("write desc sqlite3_insert_pattern_name_desc\n");
	ret += sqlite3_insert_plan_name(pdatabase, &(pdesc->sPlanNameParams));
	//INFO("write desc sqlite3_insert_plan_name\n");
	ret += sqlite3_insert_plan_date(pdatabase, &(pdesc->sDateNameParams));
	//INFO("write desc end\n");
	return (ret == 0) ? 0 : -1;
}

int Sqliteconf::sqlite3_insert_countdown_cfg(sqlite3* pdatabase, CountDownCfg *pcountdown_cfg)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into CountDownCfg (cDeviceId, cControllerID, cControllerType, nChannelFlag) values (?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	for (i = 0; i < MAX_NUM_COUNTDOWN; i++)
	{
		if (pcountdown_cfg[i].cControllerType == 0)
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, pcountdown_cfg->cDeviceId[i]);
		memset(str, 0, 1024);
		convert_chars_to_string(pcountdown_cfg->cControllerID[i], MAX_NUM_COUNTDOWN, str);
		ret = sqlite3_bind_text(stmt, 2, (const char*)(str), strlen(str), NULL);
		ret = sqlite3_bind_int(stmt, 3, pcountdown_cfg->cControllerType[i]);
	    if (i == 0)
			ret = sqlite3_bind_int(stmt, 4, pcountdown_cfg->nChannelFlag);

		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			ERR("insert CountDownCfg[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
		}
	}

	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_select_countdown_cfg(sqlite3* pdatabase, CountDownCfg *pcountdown_cfg)
{
	int ret = 0;
	int i = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from CountDownCfg;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= MAX_NUM_COUNTDOWN)
			break;
		i = sqlite3_column_int(stmt, 0);
		pcountdown_cfg->cDeviceId[i-1] = sqlite3_column_int(stmt, 1);
		memset(str, 0, 1024);
		memcpy(str, sqlite3_column_text(stmt, 2), sqlite3_column_bytes(stmt, 2));
		convert_string_to_chars(str, pcountdown_cfg->cControllerID[i-1], MAX_NUM_COUNTDOWN);
		pcountdown_cfg->cControllerType[i-1] = sqlite3_column_int(stmt, 3);
		if ((i - 1) == 0)
			pcountdown_cfg->nChannelFlag = sqlite3_column_int(stmt, 4);
		
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_insert_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into misc (cIsCanRestartHiktscAllowed, time_zone_gap) values (?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_bind_int(stmt, 1, pmisc->cIsCanRestartHiktscAllowed);
	ret = sqlite3_bind_int(stmt, 2, pmisc->time_zone_gap);

	ret = sqlite3_step(stmt);
	if(ret != SQLITE_DONE)
	{
		ERR("insert misc failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::sqlite3_select_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from misc;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		pmisc->cIsCanRestartHiktscAllowed = sqlite3_column_int(stmt, 1);
		pmisc->time_zone_gap = sqlite3_column_int(stmt, 2);
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

int Sqliteconf::read_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc)
{
	int ret = 0;
	
	ret = sqlite3_select_misc(pdatabase, pmisc);
	return ret;
}

int Sqliteconf::write_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc)
{
	int ret = 0;
	//INFO("write misc begin\n");
	create_misc_tables(pdatabase);
	ret = sqlite3_insert_misc(pdatabase, pmisc);
	//INFO("write misc end\n");
	return ret;
}

int Sqliteconf::create_hikconfig_tables(sqlite3* pdatabase)
{
	int i = 0;
	char create_table_sql[1024] = {0};
	
	while (i < 11)/*before creat new table, drop old table*/
	{
		sqlite3_drop_table_wrapper(pdatabase, hikconfig_tables_name[i].name);
		i++;
	}
	i = 0;
	while (i < 11)
	{
		memset(create_table_sql, 0, sizeof(create_table_sql));
		sprintf(create_table_sql, hikconfig_tables_name[i].sql, hikconfig_tables_name[i].name);
		sqlite3_create_table_wrapper(pdatabase, create_table_sql);
		i++;
	}
	return 0;
}

int Sqliteconf::sqlite3_insert_unit(sqlite3* pdatabase, UnitPara *punit)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stUnitPara (nBootYellowLightTime, nBootAllRedTime, byTransCycle, byFlashFrequency, \
					byFluxCollectCycle, byCollectCycleUnit) values (?,?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	ret = sqlite3_bind_int(stmt, 1, punit->nBootYellowLightTime);
	ret = sqlite3_bind_int(stmt, 2, punit->nBootAllRedTime);
	ret = sqlite3_bind_int(stmt, 3, punit->byTransCycle);
	ret = sqlite3_bind_int(stmt, 4, punit->byFlashFrequency);
	ret = sqlite3_bind_int(stmt, 5, punit->byFluxCollectCycle);
	ret = sqlite3_bind_int(stmt, 6, punit->byCollectCycleUnit);
	
	ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE)
	{
		ERR("insert stUnitPara  failed,err=%s\n", sqlite3_errmsg(pdatabase));
		return -1;
    }

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_unit(sqlite3* pdatabase, UnitPara *punit)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stUnitPara;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		punit->nBootYellowLightTime = sqlite3_column_int(stmt, 1);
		punit->nBootAllRedTime = sqlite3_column_int(stmt, 2);
		punit->byTransCycle = sqlite3_column_int(stmt, 3);
		punit->byFlashFrequency = sqlite3_column_int(stmt, 4);
		punit->byFluxCollectCycle = sqlite3_column_int(stmt, 5);
		punit->byCollectCycleUnit = sqlite3_column_int(stmt, 6);
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

     int Sqliteconf::sqlite3_insert_phase(sqlite3* pdatabase, PhaseItem pphase[][NUM_PHASE])
     {
         int ret = 0;
         int i = 0;
         int j = 0;
         char str[1024] = {0};
         sqlite3_stmt *stmt = NULL;
         char* insert_sql = "insert into stPhase (phaseTableNo, nPhaseID, nPedestrianPassTime, nPedestrianClearTime, \
                         nMinGreen, nUnitExtendGreen, nMaxGreen_1, nMaxGreen_2, nYellowTime, nAllRedTime, wPhaseOptions, nCircleID, byPhaseConcurrency) \
                         values (?,?,?,?,?,?,?,?,?,?,?,?,?);";

         ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
         if (ret != SQLITE_OK)
         {
             ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
             return -1;
         }
         for (i = 0; i < MAX_PHASE_TABLE_COUNT; i++)
         {
             for (j = 0; j < NUM_PHASE; j++)
             {
                 if (IS_PHASE_INABLE(pphase[i][j].wPhaseOptions) == 0)
                 {
                     //memset(&(pphase[i][j].nPedestrianPassTime), 0, sizeof(PhaseItem) - 1);
                     continue;
                 }
                 sqlite3_reset(stmt);
                 sqlite3_clear_bindings(stmt);

                 ret = sqlite3_bind_int(stmt, 1, i + 1);
                 ret = sqlite3_bind_int(stmt, 2, pphase[i][j].nPhaseID);
                 ret = sqlite3_bind_int(stmt, 3, pphase[i][j].nPedestrianPassTime);
                 ret = sqlite3_bind_int(stmt, 4, pphase[i][j].nPedestrianClearTime);
                 ret = sqlite3_bind_int(stmt, 5, pphase[i][j].nMinGreen);
                 ret = sqlite3_bind_int(stmt, 6, pphase[i][j].nUnitExtendGreen);
                 ret = sqlite3_bind_int(stmt, 7, pphase[i][j].nMaxGreen_1);
                 ret = sqlite3_bind_int(stmt, 8, pphase[i][j].nMaxGreen_2);
                 ret = sqlite3_bind_int(stmt, 9, pphase[i][j].nYellowTime);
                 ret = sqlite3_bind_int(stmt, 10, pphase[i][j].nAllRedTime);
                 ret = sqlite3_bind_int(stmt, 11, pphase[i][j].wPhaseOptions);
                 ret = sqlite3_bind_int(stmt, 12, pphase[i][j].nCircleID);
                 memset(str, 0, 1024);
                 convert_chars_to_string(pphase[i][j].byPhaseConcurrency, NUM_PHASE, str);
                 ret = sqlite3_bind_text(stmt, 13, (const char*)(str), strlen(str), NULL);

                 ret = sqlite3_step(stmt);
                 if(ret != SQLITE_DONE)
                 {
                     ERR("insert stPhase[%d][%d] failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
                     return -1;
                 }
             }
         }

         sqlite3_finalize(stmt);
         return 0;

     }

     int Sqliteconf::sqlite3_select_phase(sqlite3* pdatabase, PhaseItem pphase[][NUM_PHASE])
     {
         int ret = 0;
         int i = 0;
         int j = 0;
         char str[1024] = {0};
         sqlite3_stmt *stmt = NULL;

         char select_sql[] = {"select * from stPhase;"};

         ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
         if (ret != SQLITE_OK)
         {
             ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
             return -1;
         }

         ret = sqlite3_step(stmt);

         while (ret == SQLITE_ROW)
         {
             if (j >= NUM_PHASE)
             {
                 j = 1;

                 if (i >= MAX_PHASE_TABLE_COUNT)
                     break;
             }
             i = sqlite3_column_int(stmt, 1);
             j = sqlite3_column_int(stmt, 2);

             pphase[i-1][j-1].nPhaseID = j;
             pphase[i-1][j-1].nPedestrianPassTime = sqlite3_column_int(stmt, 3);
             pphase[i-1][j-1].nPedestrianClearTime = sqlite3_column_int(stmt, 4);
             pphase[i-1][j-1].nMinGreen = sqlite3_column_int(stmt, 5);
             pphase[i-1][j-1].nUnitExtendGreen = sqlite3_column_int(stmt, 6);
             pphase[i-1][j-1].nMaxGreen_1 = sqlite3_column_int(stmt, 7);
             pphase[i-1][j-1].nMaxGreen_2 = sqlite3_column_int(stmt, 8);
             pphase[i-1][j-1].nYellowTime = sqlite3_column_int(stmt, 9);
             pphase[i-1][j-1].nAllRedTime = sqlite3_column_int(stmt, 10);
             pphase[i-1][j-1].wPhaseOptions = sqlite3_column_int(stmt, 11);
             pphase[i-1][j-1].nCircleID = sqlite3_column_int(stmt, 12);
             memset(str, 0, 1024);
             memcpy(str, sqlite3_column_text(stmt, 13), sqlite3_column_bytes(stmt, 13));
             convert_string_to_chars(str, pphase[i-1][j-1].byPhaseConcurrency, NUM_PHASE);
             /*INFO("pedestrianpass[%d][%d]= %d, pedestrianclear[%d][%d]= %d", i-1,j-1, pphase[i-1][j-1].nPedestrianPassTime,
                 i-1,j-1, pphase[i-1][j-1].nPedestrianClearTime);*/
             ret = sqlite3_step(stmt);
         }
         sqlite3_finalize(stmt);
         return 0;
     }

int Sqliteconf::sqlite3_insert_phase_turn(sqlite3* pdatabase, PhaseTurnItem pphase_turn[][NUM_RING_COUNT])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stPhaseTurn (nPhaseTurnID, nCircleID, nTurnArray) values (?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	for (i = 0; i < NUM_PHASE_TURN; i++)
	{
		for (j = 0; j < NUM_RING_COUNT; j++)
		{
			if (pphase_turn[i][j].nTurnArray[0] == 0)
				continue;
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
			ret = sqlite3_bind_int(stmt, 1, pphase_turn[i][j].nPhaseTurnID);
			ret = sqlite3_bind_int(stmt, 2, pphase_turn[i][j].nCircleID);
			memset(str, 0, 1024);
			convert_chars_to_string(pphase_turn[i][j].nTurnArray, 32, str);
			ret = sqlite3_bind_text(stmt, 3, (const char*)(str), strlen(str), NULL);

			ret = sqlite3_step(stmt);
		    if(ret != SQLITE_DONE)
			{
				ERR("insert stPhaseTurn[%d][%d] failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
				return -1;
		    }
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_phase_turn(sqlite3* pdatabase, PhaseTurnItem pphase_turn[][NUM_RING_COUNT])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stPhaseTurn;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_RING_COUNT)
		{
			j = 1;
			
			if (i >= NUM_PHASE_TURN)
				break;
		}
		i = sqlite3_column_int(stmt, 1);
		j = sqlite3_column_int(stmt, 2);
		pphase_turn[i-1][j-1].nPhaseTurnID = i;
		pphase_turn[i-1][j-1].nCircleID = j;
		memset(str, 0, 1024);
		memcpy(str, sqlite3_column_text(stmt, 3), sqlite3_column_bytes(stmt, 3));
		convert_string_to_chars(str, pphase_turn[i-1][j-1].nTurnArray, 32);

		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_green_split(sqlite3* pdatabase, GreenSignalRationItem pgreen_split[][NUM_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stGreenSignalRation (nGreenSignalRationID, nPhaseID, nGreenSignalRationTime, nType, nIsCoordinate) values (?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < NUM_GREEN_SIGNAL_RATION; i++)
	{
		for (j = 0; j < NUM_PHASE; j++)
		{
			if (pgreen_split[i][j].nGreenSignalRationTime == 0)
				continue;
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
			ret = sqlite3_bind_int(stmt, 1, pgreen_split[i][j].nGreenSignalRationID);
			ret = sqlite3_bind_int(stmt, 2, pgreen_split[i][j].nPhaseID);
			ret = sqlite3_bind_int(stmt, 3, pgreen_split[i][j].nGreenSignalRationTime);
			ret = sqlite3_bind_int(stmt, 4, pgreen_split[i][j].nType);
			ret = sqlite3_bind_int(stmt, 5, pgreen_split[i][j].nIsCoordinate);

			ret = sqlite3_step(stmt);
		    if(ret != SQLITE_DONE)
			{
				ERR("insert stGreenSignalRation[%d][%d] failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
				return -1;
		    }
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_green_split(sqlite3* pdatabase, GreenSignalRationItem pgreen_split[][NUM_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stGreenSignalRation;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_PHASE)
		{
			j = 1;
		
			if (i >= NUM_GREEN_SIGNAL_RATION)
				break;
		}
		i = sqlite3_column_int(stmt, 1);
		j = sqlite3_column_int(stmt, 2);
		pgreen_split[i-1][j-1].nGreenSignalRationID = i;
		pgreen_split[i-1][j-1].nPhaseID = j;
		pgreen_split[i-1][j-1].nGreenSignalRationTime = sqlite3_column_int(stmt, 3);
		pgreen_split[i-1][j-1].nType = sqlite3_column_int(stmt, 4);
		pgreen_split[i-1][j-1].nIsCoordinate = sqlite3_column_int(stmt, 5);

		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_channel(sqlite3* pdatabase, ChannelItem pchannel[][NUM_CHANNEL])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stChannel (channelTableNO, nChannelID, nControllerID, nControllerType) values (?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < MAX_CHANNEL_TABLE_COUNT; i++)
	{
		for (j = 0; j < NUM_CHANNEL; j++)
		{
			if (pchannel[i][j].nControllerID == 0)
				continue;
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
			ret = sqlite3_bind_int(stmt, 1, i + 1);
			ret = sqlite3_bind_int(stmt, 2, pchannel[i][j].nChannelID);
			ret = sqlite3_bind_int(stmt, 3, pchannel[i][j].nControllerID);
			ret = sqlite3_bind_int(stmt, 4, pchannel[i][j].nControllerType);

			ret = sqlite3_step(stmt);
		    if(ret != SQLITE_DONE)
			{
				ERR("insert stChannel[%d][%d] failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
				return -1;
		    }
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_channel(sqlite3* pdatabase, ChannelItem pchannel[][NUM_CHANNEL])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stChannel;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_CHANNEL)
		{
			j = 1;
			
			if (i >= MAX_CHANNEL_TABLE_COUNT)
				break;
		}
		i = sqlite3_column_int(stmt, 1);
		j = sqlite3_column_int(stmt, 2);
		pchannel[i-1][j-1].nChannelID = j;
		pchannel[i-1][j-1].nControllerID = sqlite3_column_int(stmt, 3);
		pchannel[i-1][j-1].nControllerType = sqlite3_column_int(stmt, 4);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_scheme(sqlite3* pdatabase, SchemeItem *pscheme)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stScheme (nSchemeID, nCycleTime, nOffset, nGreenSignalRatioID, nPhaseTurnID) values (?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	for (i = 0; i < NUM_SCHEME; i++)
	{
		if (pscheme[i].nGreenSignalRatioID == 0 || pscheme[i].nPhaseTurnID == 0)
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, pscheme[i].nSchemeID);
		ret = sqlite3_bind_int(stmt, 2, pscheme[i].nCycleTime);
		ret = sqlite3_bind_int(stmt, 3, pscheme[i].nOffset);
		ret = sqlite3_bind_int(stmt, 4, pscheme[i].nGreenSignalRatioID);
		ret = sqlite3_bind_int(stmt, 5, pscheme[i].nPhaseTurnID);
		
		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			ERR("insert stScheme[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_scheme(sqlite3* pdatabase, SchemeItem *pscheme)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stScheme;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= NUM_SCHEME)
			break;
		i = sqlite3_column_int(stmt, 1);
		pscheme[i-1].nSchemeID = i;
		pscheme[i-1].nCycleTime = sqlite3_column_int(stmt, 2);
		pscheme[i-1].nOffset = sqlite3_column_int(stmt, 3);
		pscheme[i-1].nGreenSignalRatioID = sqlite3_column_int(stmt, 4);
		pscheme[i-1].nPhaseTurnID = sqlite3_column_int(stmt, 5);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_action(sqlite3* pdatabase, ActionItem *paction)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stAction (nActionID, nSchemeID, nPhaseTableID, nChannelTableID) values (?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < NUM_ACTION; i++)
	{
		if (paction[i].nSchemeID == 0)
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, paction[i].nActionID);
		ret = sqlite3_bind_int(stmt, 2, paction[i].nSchemeID);
		ret = sqlite3_bind_int(stmt, 3, paction[i].nPhaseTableID);
		ret = sqlite3_bind_int(stmt, 4, paction[i].nChannelTableID);

		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			ERR("insert stAction[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_action(sqlite3* pdatabase, ActionItem *paction)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stAction;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= NUM_ACTION)
			break;
		i = sqlite3_column_int(stmt, 1);
		paction[i-1].nActionID = i;
		paction[i-1].nSchemeID = sqlite3_column_int(stmt, 2);
		paction[i-1].nPhaseTableID = sqlite3_column_int(stmt, 3);
		paction[i-1].nChannelTableID = sqlite3_column_int(stmt, 4);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}
int Sqliteconf::sqlite3_insert_timeinterval(sqlite3* pdatabase, TimeIntervalItem ptime_interval[][NUM_TIME_INTERVAL_ID])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stTimeInterval (nTimeIntervalID, nTimeID, cStartTimeHour, cStartTimeMinute, nActionID) values (?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < NUM_TIME_INTERVAL; i++)
	{
		for (j = 0; j < NUM_TIME_INTERVAL_ID; j++)
		{
			if (ptime_interval[i][j].nActionID == 0)
				continue;
			
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
			ret = sqlite3_bind_int(stmt, 1, ptime_interval[i][j].nTimeIntervalID);
			ret = sqlite3_bind_int(stmt, 2, ptime_interval[i][j].nTimeID);
			ret = sqlite3_bind_int(stmt, 3, ptime_interval[i][j].cStartTimeHour);
			ret = sqlite3_bind_int(stmt, 4, ptime_interval[i][j].cStartTimeMinute);
			ret = sqlite3_bind_int(stmt, 5, ptime_interval[i][j].nActionID);

			ret = sqlite3_step(stmt);
		    if(ret != SQLITE_DONE)
			{
				ERR("insert stTimeInterval[%d][%d] failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
				return -1;
		    }
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_timeinterval(sqlite3* pdatabase, TimeIntervalItem ptime_interval[][NUM_TIME_INTERVAL_ID])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stTimeInterval;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_TIME_INTERVAL_ID)
		{
			j = 1;
			
			if (i >= NUM_TIME_INTERVAL)
				break;
		}
		i = sqlite3_column_int(stmt, 1);
		j = sqlite3_column_int(stmt, 2);
		ptime_interval[i-1][j-1].nTimeIntervalID = i;
		ptime_interval[i-1][j-1].nTimeID = j;
		ptime_interval[i-1][j-1].cStartTimeHour = sqlite3_column_int(stmt, 3);
		ptime_interval[i-1][j-1].cStartTimeMinute = sqlite3_column_int(stmt, 4);
		ptime_interval[i-1][j-1].nActionID = sqlite3_column_int(stmt, 5);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_schedule(sqlite3* pdatabase, PlanScheduleItem *pschedule)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stPlanSchedule (nScheduleID, month, week, day, nTimeIntervalID) values (?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < NUM_SCHEDULE; i++)
	{
		if (pschedule[i].nTimeIntervalID == 0)
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, pschedule[i].nScheduleID);
		ret = sqlite3_bind_int(stmt, 2, pschedule[i].month);
		ret = sqlite3_bind_int(stmt, 3, pschedule[i].week);
		ret = sqlite3_bind_int(stmt, 4, pschedule[i].day);
		ret = sqlite3_bind_int(stmt, 5, pschedule[i].nTimeIntervalID);

		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			ERR("insert stPlanSchedule[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_schedule(sqlite3* pdatabase, PlanScheduleItem *pschedule)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stPlanSchedule;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= NUM_SCHEDULE)
			break;
		i = sqlite3_column_int(stmt, 1);
		pschedule[i-1].nScheduleID = i;
		pschedule[i-1].month = sqlite3_column_int(stmt, 2);
		pschedule[i-1].week = sqlite3_column_int(stmt, 3);
		pschedule[i-1].day = sqlite3_column_int(stmt, 4);
		pschedule[i-1].nTimeIntervalID = sqlite3_column_int(stmt, 5);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_follow_phase(sqlite3* pdatabase, FollowPhaseItem pfollow_phase[][NUM_FOLLOW_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into stFollowPhase (followPhaseTableNo, nFollowPhaseID, nArrayMotherPhase) values (?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < MAX_FOLLOW_PHASE_TABLE_COUNT; i++)
	{
		for (j = 0; j < NUM_FOLLOW_PHASE; j++)
		{
			if (pfollow_phase[i][j].nArrayMotherPhase[0] == 0)
				continue;
			
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
			ret = sqlite3_bind_int(stmt, 1, i + 1);
			ret = sqlite3_bind_int(stmt, 2, pfollow_phase[i][j].nFollowPhaseID);
			memset(str, 0, 1024);
			convert_chars_to_string(pfollow_phase[i][j].nArrayMotherPhase, NUM_PHASE, str);
			ret = sqlite3_bind_text(stmt, 3, (const char*)(str), strlen(str), NULL);

			ret = sqlite3_step(stmt);
		    if(ret != SQLITE_DONE)
			{
				ERR("insert stFollowPhase[%d][%d] failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
				return -1;
		    }
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_follow_phase(sqlite3* pdatabase, FollowPhaseItem pfollow_phase[][NUM_FOLLOW_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	char str[1024] = {0};
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from stFollowPhase;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_FOLLOW_PHASE)
		{
			j = 0;
			i++;
			if (i >= MAX_FOLLOW_PHASE_TABLE_COUNT)
				break;
		}
		i = sqlite3_column_int(stmt, 1);
		j = sqlite3_column_int(stmt, 2);
		pfollow_phase[i-1][j-1].nFollowPhaseID = j;
		memset(str, 0, 1024);
		memcpy(str, sqlite3_column_text(stmt, 3), sqlite3_column_bytes(stmt, 3));
		convert_string_to_chars(str, pfollow_phase[i-1][j-1].nArrayMotherPhase, NUM_PHASE);
		j++;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_vehicle(sqlite3* pdatabase, struct STRU_N_VehicleDetector *pvehicle)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into AscVehicleDetectorTable (byVehicleDetectorNumber, byVehicleDetectorCallPhase, byVehicleDetectorQueueLimit, \
					byVehicleDetectorNoActivity, byVehicleDetectorMaxPresence, byVehicleDetectorErraticCounts, byVehicleDetectorFailTime) values (?,?,?,?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < MAX_VEHICLEDETECTOR_COUNT; i++)
	{
		if (pvehicle[i].byVehicleDetectorCallPhase == 0 || pvehicle[i].byVehicleDetectorSwitchPhase == 0)
			continue;
		
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, pvehicle[i].byVehicleDetectorNumber);
		ret = sqlite3_bind_int(stmt, 2, pvehicle[i].byVehicleDetectorCallPhase);
		ret = sqlite3_bind_int(stmt, 3, pvehicle[i].byVehicleDetectorQueueLimit);
		ret = sqlite3_bind_int(stmt, 4, pvehicle[i].byVehicleDetectorNoActivity);
		ret = sqlite3_bind_int(stmt, 5, pvehicle[i].byVehicleDetectorMaxPresence);
		ret = sqlite3_bind_int(stmt, 6, pvehicle[i].byVehicleDetectorErraticCounts);
		ret = sqlite3_bind_int(stmt, 7, pvehicle[i].byVehicleDetectorFailTime);

		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			ERR("insert AscVehicleDetectorTable[%d] failed,err=%s\n", i, sqlite3_errmsg(pdatabase));
			return -1;
		}
	}

	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_select_vehicle(sqlite3* pdatabase, struct STRU_N_VehicleDetector *pvehicle)
{
	int ret = 0;
	int i = 0;
	sqlite3_stmt *stmt = NULL;
	
	char select_sql[] = {"select * from AscVehicleDetectorTable;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (i >= MAX_VEHICLEDETECTOR_COUNT)
			break;
		i = sqlite3_column_int(stmt, 1);
		pvehicle[i-1].byVehicleDetectorNumber = i;
		pvehicle[i-1].byVehicleDetectorCallPhase = sqlite3_column_int(stmt, 2);
		pvehicle[i-1].byVehicleDetectorQueueLimit = sqlite3_column_int(stmt, 3);
		pvehicle[i-1].byVehicleDetectorNoActivity = sqlite3_column_int(stmt, 4);
		pvehicle[i-1].byVehicleDetectorMaxPresence = sqlite3_column_int(stmt, 5);
		pvehicle[i-1].byVehicleDetectorErraticCounts = sqlite3_column_int(stmt, 6);
		pvehicle[i-1].byVehicleDetectorFailTime = sqlite3_column_int(stmt, 7);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int Sqliteconf::sqlite3_insert_signal_trans(sqlite3* pdatabase, struct STRU_SignalTransEntry psignal_trans[][NUM_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	int rowid = 0;
	sqlite3_stmt *stmt = NULL;
	char select_sql[] = {"select ROWID, phaseTableNo, nPhaseID from stPhase;"};

	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	ret = sqlite3_step(stmt);
	while (ret == SQLITE_ROW)
	{
		rowid = sqlite3_column_int(stmt, 0);
		i = sqlite3_column_int(stmt, 1);
		j = sqlite3_column_int(stmt, 2);
		sqlite3_update_column(pdatabase, TABLE_NAME_PHASE, "nGreenLightTime", rowid, &(psignal_trans[i-1][j-1].nGreenLightTime), 1, SQLITE_INTEGER);

		ret = sqlite3_step(stmt);
	}
	return 0;
}

int Sqliteconf::sqlite3_select_signal_trans(sqlite3* pdatabase, struct STRU_SignalTransEntry psignal_trans[][NUM_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;

	char select_sql[] = {"select phaseTableNo, nPhaseID, nGreenLightTime from stPhase;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_PHASE)
		{
			j = 0;
			
			if (i >= MAX_PHASE_TABLE_COUNT)
				break;
		}
		i = sqlite3_column_int(stmt, 0);
		j = sqlite3_column_int(stmt, 1);
		psignal_trans[i-1][j-1].nGreenLightTime = sqlite3_column_int(stmt, 2);
		
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}

#if 0
int sqlite3_insert_signal_trans(sqlite3* pdatabase, struct STRU_SignalTransEntry psignal_trans[][NUM_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	char* insert_sql = "insert into AscSignalTransTable (byPhaseNumber, byRedYellow, nGreenLightTime, bySafeRed) values (?,?,?,?);";

	ret = sqlite3_prepare(pdatabase, insert_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}
	
	for (i = 0; i < MAX_PHASE_TABLE_COUNT; i++)
	{
		for (j = 0; j < NUM_PHASE; j++)
		{
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		ret = sqlite3_bind_int(stmt, 1, psignal_trans[i][j].byPhaseNumber);
		ret = sqlite3_bind_int(stmt, 2, psignal_trans[i][j].byRedYellow);
		ret = sqlite3_bind_int(stmt, 3, psignal_trans[i][j].nGreenLightTime);
		ret = sqlite3_bind_int(stmt, 4, psignal_trans[i][j].bySafeRed);

		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE)
		{
			FPRINTF(stderr, "insert AscSignalTransTable[%d][%d] failed,err=%s\n", i, j, sqlite3_errmsg(pdatabase));
			return -1;
		}
		}
	}

	sqlite3_finalize(stmt);
	return 0;

}

int sqlite3_select_signal_trans(sqlite3* pdatabase, struct STRU_SignalTransEntry psignal_trans[][NUM_PHASE])
{
	int ret = 0;
	int i = 0;
	int j = 0;
	sqlite3_stmt *stmt = NULL;
	int ncol = 0;
	
	char select_sql[] = {"select * from AscSignalTransTable;"};
	
	ret = sqlite3_prepare(pdatabase, select_sql, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		ERR("prepare failed, err=%s", sqlite3_errmsg(pdatabase));
		return -1;
	}

	ret = sqlite3_step(stmt);

	while (ret == SQLITE_ROW)
	{
		if (j >= NUM_PHASE)
		{
			j = 0;
			i++;
			if (i >= MAX_PHASE_TABLE_COUNT)
				break;
		}
		psignal_trans[i][j].byPhaseNumber = sqlite3_column_int(stmt, 1);
		psignal_trans[i][j].byRedYellow = sqlite3_column_int(stmt, 2);
		psignal_trans[i][j].nGreenLightTime = sqlite3_column_int(stmt, 3);
		psignal_trans[i][j].bySafeRed = sqlite3_column_int(stmt, 4);
		j++;
		ret = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return 0;

}
#endif
int Sqliteconf::write_hikconfig(sqlite3* pdatabase, SignalControllerPara* pscp)
{
	int ret = 0;
	//log_debug("write hik_config begin\n");
	create_hikconfig_tables(pdatabase);
	ret += sqlite3_insert_unit(pdatabase, &(pscp->stUnitPara));
	//log_debug("write hik_config sqlite3_insert_unit\n");
	ret += sqlite3_insert_phase(pdatabase, pscp->stPhase);
	//log_debug("write hik_config sqlite3_insert_phase\n");
	ret += sqlite3_insert_phase_turn(pdatabase, pscp->stPhaseTurn);
	//log_debug("write hik_config sqlite3_insert_phase_turn\n");
	ret += sqlite3_insert_green_split(pdatabase, pscp->stGreenSignalRation);
	//log_debug("write hik_config sqlite3_insert_green_split\n");
	ret += sqlite3_insert_channel(pdatabase, pscp->stChannel);
	//log_debug("write hik_config sqlite3_insert_channel\n");
	ret += sqlite3_insert_scheme(pdatabase, pscp->stScheme);
	//log_debug("write hik_config sqlite3_insert_scheme\n");
	ret += sqlite3_insert_action(pdatabase, pscp->stAction);
	//log_debug("write hik_config sqlite3_insert_action\n");
	ret += sqlite3_insert_timeinterval(pdatabase, pscp->stTimeInterval);
	//log_debug("write hik_config sqlite3_insert_timeinterval\n");
	ret += sqlite3_insert_schedule(pdatabase, pscp->stPlanSchedule);
	//log_debug("write hik_config sqlite3_insert_schedule\n");
	ret += sqlite3_insert_follow_phase(pdatabase, pscp->stFollowPhase);
	//log_debug("write hik_config sqlite3_insert_follow_phase\n");
	ret += sqlite3_insert_vehicle(pdatabase, pscp->AscVehicleDetectorTable);
	//log_debug("write hik_config sqlite3_insert_vehicle\n");
	ret += sqlite3_insert_signal_trans(pdatabase, pscp->AscSignalTransTable);
	
	if (ret != 0)
	{
		ERR("insert tables occur error\n");
		return ret;
	}
	//log_debug("write hik_config end\n");
	return 0;
}

int Sqliteconf::read_hikconfig(sqlite3* pdatabase, SignalControllerPara* pscp)
{
	int ret = 0;

	ret += sqlite3_select_unit(pdatabase, &(pscp->stUnitPara));
	ret += sqlite3_select_phase(pdatabase, pscp->stPhase);
	ret += sqlite3_select_phase_turn(pdatabase, pscp->stPhaseTurn);
	ret += sqlite3_select_green_split(pdatabase, pscp->stGreenSignalRation);
	ret += sqlite3_select_channel(pdatabase, pscp->stChannel);
	ret += sqlite3_select_scheme(pdatabase, pscp->stScheme);
	ret += sqlite3_select_action(pdatabase, pscp->stAction);
	ret += sqlite3_select_timeinterval(pdatabase, pscp->stTimeInterval);
	ret += sqlite3_select_schedule(pdatabase, pscp->stPlanSchedule);
	ret += sqlite3_select_follow_phase(pdatabase, pscp->stFollowPhase);
	ret += sqlite3_select_vehicle(pdatabase, pscp->AscVehicleDetectorTable);
	ret += sqlite3_select_signal_trans(pdatabase, pscp->AscSignalTransTable);

	if (ret != 0)
	{
		ERR("select tables occur error\n");
		return -1;
	}
		
	return 0;
}

/*
format: char* col_name1,  char* col_name2, ...
*
*/
/*
int sqlite3_select_columns(const char* table, char* format, ...)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	int ncol = 0;
	char* col_buf = NULL;
	char sql_buf[1024] = {0};
	va_list argptr;
	
	sprintf(sql_buf, "select ");
	col_buf = sql_buf + strlen(sql_buf);
	va_start(argptr, format);
	vsnprintf(col_buf, sizeof(sql_buf)- strlen(sql_buf), format, argptr);
	va_end(argptr);
	strcat(col_buf," ");
	sprintf(sql_buf + strlen(sql_buf), " from %s;", table);

	
	
}
*/


