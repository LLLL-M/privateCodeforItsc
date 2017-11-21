#ifndef __TSCDB_H__
#define __TSCDB_H__

#include "sqlite3.h"
#include "gb2016.h"
#include "common.h"
#include <sstream>

class Tscdb
{
private:
	sqlite3 *db;
	void StringToTurn(string &str, UInt8 (&turn)[MAX_PHASE_NUM]);
	int LoadTable(const char *tablename, stringstream &ss);
public:
	Tscdb()
	{
		db = NULL;
	}
	~Tscdb()
	{
		Close();
	}
	
	bool Open()
	{
		if (db != NULL)
		{
			INFO("tsc.db has already been open!");
			return true;
		}
		if (sqlite3_open("/home/tsc.db", &db) != SQLITE_OK)
		{
			ERR("open /home/tsc.db fail!");
			db = NULL;
			return false;
		}
		sqlite3_exec(db, "begin;", 0, 0, 0);
		return true;
	}
	
	void Close()
	{
		if (db != NULL)
		{
			sqlite3_exec(db, "commit;", 0, 0, 0);
			sqlite3_close(db);
			db = NULL;
		}
	}
	
	bool Store(ChannelItem (&table)[MAX_CHANNEL_NUM]);
	bool Store(PhaseItem (&table)[MAX_PHASE_NUM]);
	bool Store(SchemeItem (&table)[MAX_SCHEME_NUM]);
	bool Store(ScheduleItem (&table)[MAX_SCHEDULE_NUM]);
	bool Store(SwtichParam &param);
	bool Store(DetectorItem (&table)[MAX_DETECTOR_NUM]);
	
	void Load(ChannelItem (&table)[MAX_CHANNEL_NUM]);
	void Load(PhaseItem (&table)[MAX_PHASE_NUM]);
	void Load(SchemeItem (&table)[MAX_SCHEME_NUM]);
	void Load(ScheduleItem (&table)[MAX_SCHEDULE_NUM]);
	void Load(SwtichParam &param);
	void Load(DetectorItem (&table)[MAX_DETECTOR_NUM]);
	void Load(Basic &basic);
	
protected:

};

#endif
