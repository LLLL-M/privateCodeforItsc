#include <cstring>
#include <cstdlib>
#include "tscdb.h"

bool Tscdb::Store(ChannelItem (&table)[MAX_CHANNEL_NUM])
{
	ostringstream ss;
	char *err = NULL;
	int i;
	
	if (db == NULL)
		return false;
	sqlite3_exec(db, "drop table channel;", 0, 0, 0);	//先删除表
	ss << "create table channel (channelId integer primary key, channelType integer, conflictChannel integer, direction integer);";	//再次创建表
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (table[i].channelId == 0)
			continue;
		ss << "insert into channel (channelId, channelType, conflictChannel, direction) values (" 
		<< (int)table[i].channelId << "," 
		<< (int)table[i].channelType << "," 
		<< (UInt32)table[i].conflictChannel << ","
		<< (int)table[i].direction << ");";
	}
	if (SQLITE_OK != sqlite3_exec(db, ss.str().c_str(), 0, 0, &err))
	{
		if (err != NULL)
		{
			ERR("store channel table fail, error: %s", err);
			sqlite3_free(err);
		}
		return false;
	}
	return true;
}

bool Tscdb::Store(PhaseItem (&table)[MAX_PHASE_NUM])
{
	ostringstream ss;
	char *err = NULL;
	int i;
	
	if (db == NULL)
		return false;
	sqlite3_exec(db, "drop table phase;", 0, 0, 0);	//先删除表
	ss << "create table phase (phaseId integer primary key, greenTime integer, greenBlinkTime integer, yellowTime integer, allRedTime integer, autoRequest nteger, pedClearTime integer, pedResponseTime integer, minGreenTime integer, maxGreenTime integer, maxGreenTime2 integer, unitExtendTime integer, checkTime integer, channelBits integer, vehDetectorBits integer, pedDetectorBits integer, advanceExtendTime integer);";	//再次创建表
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (table[i].phaseId == 0)
			continue;
		ss << "insert into phase (phaseId, greenTime, greenBlinkTime, yellowTime, allRedTime, autoRequest, pedClearTime, pedResponseTime, minGreenTime, maxGreenTime, maxGreenTime2, unitExtendTime, checkTime, channelBits, vehDetectorBits, pedDetectorBits, advanceExtendTime) values (" 
		<< (int)table[i].phaseId << "," 
		<< (int)table[i].greenTime << "," 
		<< (int)table[i].greenBlinkTime << "," 
		<< (int)table[i].yellowTime << "," 
		<< (int)table[i].allRedTime << "," 
		<< (int)table[i].autoRequest << "," 
		<< (int)table[i].pedClearTime << "," 
		<< (int)table[i].pedResponseTime << "," 
		<< (int)table[i].minGreenTime << "," 
		<< (int)table[i].maxGreenTime << "," 
		<< (int)table[i].maxGreenTime2 << "," 
		<< (int)table[i].unitExtendTime << "," 
		<< (int)table[i].checkTime << "," 
		<< (UInt32)table[i].channelBits << "," 
		<< (UInt32)table[i].vehDetectorBits << "," 
		<< (int)table[i].pedDetectorBits << "," 
		<< (int)table[i].advanceExtendTime << ");";
	}
	if (SQLITE_OK != sqlite3_exec(db, ss.str().c_str(), 0, 0, &err))
	{
		if (err != NULL)
		{
			ERR("store phase table fail, error: %s", err);
			sqlite3_free(err);
		}
		return false;
	}
	return true;
}

bool Tscdb::Store(SchemeItem (&table)[MAX_SCHEME_NUM])
{
	ostringstream ss;
	char *err = NULL;
	int i, j;
	
	if (db == NULL)
		return false;
	sqlite3_exec(db, "drop table scheme;", 0, 0, 0);	//先删除表
	ss << "create table scheme (schemeId integer primary key, cycleTime integer, phaseOffset integer, coordinatePhase integer, phaseturn char(128));";	//再次创建表
	for (i = 0; i < MAX_SCHEME_NUM; i++)
	{
		if (table[i].schemeId == 0)
			continue;
		ss << "insert into scheme (schemeId, cycleTime, phaseOffset, coordinatePhase, phaseturn) values (" 
		<< (int)table[i].schemeId << "," 
		<< (int)table[i].cycleTime << "," 
		<< (int)table[i].phaseOffset << "," 
		<< (int)table[i].coordinatePhase << ",";
		ss << "'";
		for (j = 0; j < MAX_PHASE_NUM; j++)
		{
			if (table[i].phaseturn[j] > 0)
				ss << (int)table[i].phaseturn[j] << ",";
			else
				break;
		}
		ss << "');";
	}
	if (SQLITE_OK != sqlite3_exec(db, ss.str().c_str(), 0, 0, &err))
	{
		if (err != NULL)
		{
			ERR("store scheme table fail, error: %s", err);
			sqlite3_free(err);
		}
		return false;
	}
	return true;
}

bool Tscdb::Store(ScheduleItem (&table)[MAX_SCHEDULE_NUM])
{
	ostringstream ss;
	char *err = NULL;
	int i;
	
	if (db == NULL)
		return false;
	sqlite3_exec(db, "drop table schedule;", 0, 0, 0);	//先删除表
	ss << "create table schedule (scheduleId integer primary key, week integer, month integer, day integer, startHour integer, startMin integer, endHour integer, endMin integer, ctrlType integer, ctrlMode integer, ctrlId integer);";	//再次创建表
	for (i = 0; i < MAX_SCHEDULE_NUM; i++)
	{
		if (table[i].scheduleId == 0)
			continue;
		ss << "insert into schedule (scheduleId, week, month, day, startHour, startMin, endHour, endMin, ctrlType, ctrlMode, ctrlId) values (" 
		<< (int)table[i].scheduleId << "," 
		<< (int)table[i].week << "," 
		<< (int)table[i].month << "," 
		<< (int)table[i].day << "," 
		<< (int)table[i].startHour << "," 
		<< (int)table[i].startMin << "," 
		<< (int)table[i].endHour << "," 
		<< (int)table[i].endMin << "," 
		<< (int)table[i].ctrlType << "," 
		<< (int)table[i].ctrlMode << "," 
		<< (int)table[i].ctrlId << ");";
	}
	if (SQLITE_OK != sqlite3_exec(db, ss.str().c_str(), 0, 0, &err))
	{
		if (err != NULL)
		{
			ERR("store schedule table fail, error: %s", err);
			sqlite3_free(err);
		}
		return false;
	}
	return true;
}

bool Tscdb::Store(SwtichParam &param)
{
	ostringstream ss;
	char *err = NULL;
	int i;

	if (db == NULL)
		return false;
	sqlite3_exec(db, "drop table switch;", 0, 0, 0);	//先删除表
	ss << "create table switch (gps integer, watchdog integer, voltCheck integer, curCheck integer, faultYellowFlash integer, takeover integer);";	//再次创建表
	ss << "insert into switch (gps, watchdog, voltCheck, curCheck, faultYellowFlash, takeover) values (" 
		<< (int)param.gps << "," 
		<< (int)param.watchdog << "," 
		<< (int)param.voltCheck << "," 
		<< (int)param.curCheck << "," 
		<< (int)param.faultYellowFlash << ","
		<< (int)param.takeover << ");";
	if (SQLITE_OK != sqlite3_exec(db, ss.str().c_str(), 0, 0, &err))
	{
		if (err != NULL)
		{
			ERR("store switch table fail, error: %s", err);
			sqlite3_free(err);
		}
		return false;
	}
	return true;
}

bool Tscdb::Store(DetectorItem (&table)[MAX_DETECTOR_NUM])
{
	ostringstream ss;
	char *err = NULL;
	int i;
	
	if (db == NULL)
		return false;
	sqlite3_exec(db, "drop table detector;", 0, 0, 0);	//先删除表
	ss << "create table detector (detectorId integer primary key, noResponseTime integer, maxContinuousTime integer, maxVehcileNum integer, detectorType integer);";	//再次创建表
	for (i = 0; i < MAX_DETECTOR_NUM; i++)
	{
		if (table[i].detectorId == 0)
			continue;
		ss << "insert into detector (detectorId, noResponseTime, maxContinuousTime, maxVehcileNum, detectorType) values (" 
		<< (int)table[i].detectorId << "," 
		<< (int)table[i].noResponseTime << "," 
		<< (int)table[i].maxContinuousTime << "," 
		<< (int)table[i].maxVehcileNum << ","
		<< (int)table[i].detectorType << ");";
	}
	if (SQLITE_OK != sqlite3_exec(db, ss.str().c_str(), 0, 0, &err))
	{
		if (err != NULL)
		{
			ERR("store detector table fail, error: %s", err);
			sqlite3_free(err);
		}
		return false;
	}
	return true;
}

int Tscdb::LoadTable(const char *tablename, stringstream &ss)
{
	int nRow, nColumn;
	char **dbResult;
	char *err = NULL;
	int i, num;
	string sql = string("select * from ") + tablename;
	
	if (db == NULL)
		return 0;
	if (SQLITE_OK != sqlite3_get_table(db, sql.c_str(), &dbResult, &nRow, &nColumn, &err))
	{
		if (err != NULL)
		{
			ERR("load %s table fail, error: %s", tablename, err);
			sqlite3_free(err);
		}
		return 0;
	}
	if (nRow < 1 || nColumn < 1)
	{
		ERR("load %s table fail, nRow = %d, nColumn = %d", tablename, nRow, nColumn);
		sqlite3_free_table(dbResult);
		return 0;
	}
	num = (nRow + 1) * nColumn;	//第一行为字段名称，nRow只表示有多少行数据
	for (i = nColumn; i < num; i++)
	{
		ss << dbResult[i] << " ";
	}
	sqlite3_free_table(dbResult);
	return nRow;
}

void Tscdb::Load(ChannelItem (&table)[MAX_CHANNEL_NUM])
{
	stringstream ss;
	UInt32 value = 0;
	int nRow = LoadTable("channel", ss);
	int i, index;
	
	for (i = 0; i < nRow; i++)
	{
		ss >> value;
		if (value < 1 || value > MAX_CHANNEL_NUM)
		{
			ERR("value %d is invalid when load channel table!", value);
			return;
		}
		index = value - 1;
		table[index].channelId = value;
		ss >> value;
		table[index].channelType = value;
		ss >> value;
		table[index].conflictChannel = value;
		ss >> value;
		table[index].direction = value;
	}
}

void Tscdb::Load(PhaseItem (&table)[MAX_PHASE_NUM])
{
	stringstream ss;
	UInt32 value = 0;
	int nRow = LoadTable("phase", ss);
	int i, index = 0;
	
	for (i = 0; i < nRow; i++)
	{
		ss >> value;
		if (value < 1 || value > MAX_PHASE_NUM)
		{
			ERR("value %d is invalid when load phase table!", value);
			return;
		}
		index = value - 1;
		table[index].phaseId = value;
		ss >> value;
		table[index].greenTime = value;
		ss >> value;
		table[index].greenBlinkTime = value;
		ss >> value;
		table[index].yellowTime = value;
		ss >> value;
		table[index].allRedTime = value;
		ss >> value;
		table[index].autoRequest = value;
		ss >> value;
		table[index].pedClearTime = value;
		ss >> value;
		table[index].pedResponseTime = value;
		ss >> value;
		table[index].minGreenTime = value;
		ss >> value;
		table[index].maxGreenTime = value;
		ss >> value;
		table[index].maxGreenTime2 = value;
		ss >> value;
		table[index].unitExtendTime = value;
		ss >> value;
		table[index].checkTime = value;
		ss >> value;
		table[index].channelBits = value;
		ss >> value;
		table[index].vehDetectorBits = value;
		ss >> value;
		table[index].pedDetectorBits = value;
		ss >> value;
		table[index].advanceExtendTime = value;
	}
}

void Tscdb::StringToTurn(string &str, UInt8 (&turn)[MAX_PHASE_NUM])
{
	int i, j = 0, pos = 0;
	
	for (i = 0; i < str.size(); i++)
	{
		if (str[i] == ',' && j < MAX_PHASE_NUM)
		{
			turn[j++] = (UInt8)atoi(str.substr(pos, i - pos).c_str());
			pos = i + 1;
		}
	}
}

void Tscdb::Load(SchemeItem (&table)[MAX_SCHEME_NUM])
{
	stringstream ss;
	UInt32 value = 0;
	int nRow = LoadTable("scheme", ss);
	int i, index;
	string str;
	
	for (i = 0; i < nRow; i++)
	{
		ss >> value;
		if (value < 1 || value > MAX_SCHEME_NUM)
		{
			ERR("value %d is invalid when load scheme table!", value);
			return;
		}
		index = value - 1;
		table[index].schemeId = value;
		ss >> value;
		table[index].cycleTime = value;
		ss >> value;
		table[index].phaseOffset = value;
		ss >> value;
		table[index].coordinatePhase = value;
		ss >> str;
		StringToTurn(str, table[index].phaseturn);
	}
}

void Tscdb::Load(ScheduleItem (&table)[MAX_SCHEDULE_NUM])
{
	stringstream ss;
	UInt32 value = 0;
	int nRow = LoadTable("schedule", ss);
	int i, index;
	
	for (i = 0; i < nRow; i++)
	{
		ss >> value;
		if (value < 1 || value > MAX_SCHEDULE_NUM)
		{
			ERR("value %d is invalid when load schedule table!", value);
			return;
		}
		index = value - 1;
		table[index].scheduleId = value;
		ss >> value;
		table[index].week = value;
		ss >> value;
		table[index].month = value;
		ss >> value;
		table[index].day = value;
		ss >> value;
		table[index].startHour = value;
		ss >> value;
		table[index].startMin = value;
		ss >> value;
		table[index].endHour = value;
		ss >> value;
		table[index].endMin = value;
		ss >> value;
		table[index].ctrlType = value;
		ss >> value;
		table[index].ctrlMode = value;
		ss >> value;
		table[index].ctrlId = value;
	}
}

void Tscdb::Load(SwtichParam &param)
{
	stringstream ss;
	UInt32 value = 0;
	
	if (LoadTable("switch", ss) > 0)
	{
		ss >> value;
		param.gps = value;
		ss >> value;
		param.watchdog = value;
		ss >> value;
		param.voltCheck = value;
		ss >> value;
		param.curCheck = value;
		ss >> value;
		param.faultYellowFlash = value;
		ss >> value;
		param.takeover = value;
	}
}

void Tscdb::Load(DetectorItem (&table)[MAX_DETECTOR_NUM])
{
	stringstream ss;
	UInt32 value = 0;
	int nRow = LoadTable("detector", ss);
	int i, index;
	
	for (i = 0; i < nRow; i++)
	{
		ss >> value;
		if (value < 1 || value > MAX_DETECTOR_NUM)
		{
			ERR("value %d is invalid when load detector table!", value);
			return;
		}
		index = value - 1;
		table[index].detectorId = value;
		ss >> value;
		table[index].noResponseTime = value;
		ss >> value;
		table[index].maxContinuousTime = value;
		ss >> value;
		table[index].maxVehcileNum = value;
		ss >> value;
		table[index].detectorType = value;
	}
}

void Tscdb::Load(Basic &basic)
{
	stringstream ss;
	UInt32 value = 0;

	if (LoadTable("basic", ss) > 0)
	{
		ss >> value;
		basic.areaNumber = value;
		ss >> value;
		basic.roadNumber = value;
		ss >> basic.version;
		ss >> basic.identifyCode;
		ss >> value;
		basic.bootYellowBlinkTime = value;
		ss >> value;
		basic.bootAllRedTime = value;
		ss >> value;
		basic.vehFlowUploadCycleTime = value;
		ss >> value;
		basic.transitionCycle = value;
		ss >> basic.ip;
		ss >> value;
		basic.port = value;
	}
}

