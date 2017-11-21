#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include "log.h"
#include "frame.h"

#ifdef __linux__
#define FAULT_LOG_FILE		"/home/faultlog.dat"
#define DB_LOG_FILE			"/home/log.db"
#else   //__WIN32__
#define FAULT_LOG_FILE		"faultlog.dat"
#define DB_LOG_FILE			"log.db"
#endif
#define FAULT_LOG_SIZE	262144	//256k
#define FAULT_LOG_MAP_SIZE	(FAULT_LOG_SIZE + sizeof(FaultLogHead))
#define FAULT_LOG_MAX_NUM	(FAULT_LOG_SIZE / sizeof(FaultLog) / 2)		//log dat的存储采用备份机制，当超过次数时进行备份
#define MAX_LOG_NUM			512		//每个log数据库表存储的最大log个数，超过则备份到bak表

Log::Log()
{
	head = nullptr;
	if (file.Open(FAULT_LOG_FILE))
	{
		if (file.Size() == 0)
			file.Size(FAULT_LOG_MAP_SIZE);
		head = static_cast<FaultLogHead *>(file.Mmap(FAULT_LOG_MAP_SIZE));
		if (head == nullptr)
		{
			file.Close();
			ERR("mmap %s file failed!", FAULT_LOG_FILE);
		}
	}
	db = nullptr;
	row = 0;
	if (sqlite3_open(DB_LOG_FILE, &db) != SQLITE_OK)
	{
		ERR("open log.db fail!");
		db = nullptr;
	}
	else
	{
		sqlite3_exec(db, 
			"create table if not exists log([seq] integer primary key, [time] timestamp not null default(datetime('now', 'localtime')), [content] varchar(128));\
			create table if not exists bak([seq] integer primary key, [time] timestamp not null default(datetime('now', 'localtime')), [content] varchar(128));\
			create table if not exists fault (channelId integer primary key, faultStatus integer);",
			0, 0, 0);
		int nColumn = 0;
		char **dbResult;
		sqlite3_get_table(db, "select seq from log;", &dbResult, &row, &nColumn, NULL);
		sqlite3_free_table(dbResult);
#if 0
		//默认插入48个通道故障状态为0(代表正常)，由于表创建时以通道号为主键，所以如果表中已经存在记录则插入是无效的。
		sqlite3_exec(db, "begin;", 0, 0, 0);
		sqlite3_exec(db, "\
						  insert into fault values(1, 0);\
						  insert into fault values(2, 0);\
						  insert into fault values(3, 0);\
						  insert into fault values(4, 0);\
						  insert into fault values(5, 0);\
						  insert into fault values(6, 0);\
						  insert into fault values(7, 0);\
						  insert into fault values(8, 0);\
						  insert into fault values(9, 0);\
						  insert into fault values(10, 0);\
						  insert into fault values(11, 0);\
						  insert into fault values(12, 0);\
						  insert into fault values(13, 0);\
						  insert into fault values(14, 0);\
						  insert into fault values(15, 0);\
						  insert into fault values(16, 0);\
						  insert into fault values(17, 0);\
						  insert into fault values(18, 0);\
						  insert into fault values(19, 0);\
						  insert into fault values(20, 0);\
						  insert into fault values(21, 0);\
						  insert into fault values(22, 0);\
						  insert into fault values(23, 0);\
						  insert into fault values(24, 0);\
						  insert into fault values(25, 0);\
						  insert into fault values(26, 0);\
						  insert into fault values(27, 0);\
						  insert into fault values(28, 0);\
						  insert into fault values(29, 0);\
						  insert into fault values(30, 0);\
						  insert into fault values(31, 0);\
						  insert into fault values(32, 0);\
						  insert into fault values(33, 0);\
						  insert into fault values(34, 0);\
						  insert into fault values(35, 0);\
						  insert into fault values(36, 0);\
						  insert into fault values(37, 0);\
						  insert into fault values(38, 0);\
						  insert into fault values(39, 0);\
						  insert into fault values(40, 0);\
						  insert into fault values(41, 0);\
						  insert into fault values(42, 0);\
						  insert into fault values(43, 0);\
						  insert into fault values(44, 0);\
						  insert into fault values(45, 0);\
						  insert into fault values(46, 0);\
						  insert into fault values(47, 0);\
						  insert into fault values(48, 0);", 0, 0, 0);
		sqlite3_exec(db, "commit;", 0, 0, 0);
#endif
	}
}

Log::~Log()
{
	if (head != nullptr)
	{
		file.Unmap(head, FAULT_LOG_MAP_SIZE);
		file.Close();
		head = nullptr;
	}
	if (db != nullptr)
	{
		sqlite3_close(db);
		db = nullptr;
	}
}

vector<FaultLog> Log::Read(UInt32 startTime, UInt32 lineNum)
{
	vector<FaultLog> log;
	if (head == nullptr || startTime == 0)
		return log;
	FaultLog *data = head->data + FAULT_LOG_MAX_NUM;
	FaultLog *start = nullptr;
	UInt32 num = 0, count = 0;
	if (head->num > 0 && startTime > data[head->num - 1].time)
		return log;
	if (startTime >= data[0].time)
	{
		start = data;
		num = FAULT_LOG_MAX_NUM;
	}
	else 
	{
		start = head->data;
		num = FAULT_LOG_MAX_NUM * 2;
	}
	rwlock.r_lock();
	for (int i = 0; i < num; i++)
	{
		if (start[i].time >= startTime)
		{
			if (++count <= lineNum)
				log.push_back(start[i]);
			else
				break;
		}
	}
	rwlock.r_unlock();
	return log;
}

void Log::Insert(const char *content)
{
	char sql[256] = {0};
	string cmd;
	char *err = nullptr;

	row++;
	snprintf(sql, 256, "insert into log(seq, content) values(%d, '%s');", row, content);
	cmd += sql;
	INFO("%s", sql);	
	if (row >= MAX_LOG_NUM)
	{
		row = 0;
		cmd += "drop table bak;create table bak as select * from log;delete from log;";
	}
	if (sqlite3_exec(db, cmd.c_str(), 0, 0, &err) != SQLITE_OK)
	{
		ERR("insert to log fail, error: %s", err);
		sqlite3_free(err);
	}
}

void Log::Write(const char *content)
{
	if (db == nullptr || content == nullptr)
		return;
	rwlock.w_lock();
	Insert(content);
	rwlock.w_unlock();
}

void Log::Write(FaultLogType type, const char *content, const UInt16 value)
{
	if (head == nullptr || db == nullptr)
		return;
	FaultLog *data = head->data + FAULT_LOG_MAX_NUM + head->num;
	Frame frame;

	rwlock.w_lock();
	data->seq = ++head->num;
	data->type = type;
	data->value = value;
	data->time = time(nullptr);
	file.Msync(data, sizeof(FaultLog));
	frame.Send(*data);
	if (head->num >= FAULT_LOG_MAX_NUM)//超过FAULT_LOG_MAX_NUM则进行备份
	{
		memcpy(head->data, head->data + FAULT_LOG_MAX_NUM, sizeof(FaultLog) * FAULT_LOG_MAX_NUM);
		file.Msync(head->data, sizeof(FaultLog) * FAULT_LOG_MAX_NUM);
		head->num = 0;
	}
	file.Msync(head, sizeof(head->num));
	if (content != nullptr)
		Insert(content);
	rwlock.w_unlock();
}

void Log::Clear()
{
	if (head == nullptr)
		return;
	rwlock.w_lock();
	head->num = 0;
	std::memset(head->data, 0, 2 * FAULT_LOG_MAX_NUM * sizeof(FaultLogHead));
	file.Msync(head, FAULT_LOG_MAP_SIZE);
	Insert("clear fault log!");
	rwlock.w_unlock();
}

void Log::UpdateFault(int channelId, int faultStatus)
{
#if 1
	if (head == nullptr || channelId < 1 || channelId > MAX_CHANNEL_NUM)
		return;
	head->faultStatus[channelId - 1] = (UInt8)faultStatus;
	file.Msync(head, sizeof(FaultLogHead));
#else
	char sql[128] = {0};
	sprintf(sql, "update fault set faultStatus=%d where channelId=%d;", faultStatus, channelId);
	sqlite3_exec(db, sql, 0, 0, 0);
#endif
}

const UInt8 Log::LoadFault(int channelId)
{
	if (head == nullptr || channelId < 1 || channelId > MAX_CHANNEL_NUM)
		return 0;
	return head->faultStatus[channelId - 1];
}

void Log::LoadFault(UInt8 (&faultStatus)[MAX_CHANNEL_NUM])
{
#if 1
	if (head == nullptr)
		return;
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		faultStatus[i] = head->faultStatus[i];
#else
	int nRow, nColumn;
	char **dbResult;
	char *err = NULL;
	if (SQLITE_OK != sqlite3_get_table(db, "select faultStatus from fault;", &dbResult, &nRow, &nColumn, &err))
	{
		if (err != NULL)
		{
			ERR("load fault table fail, error: %s", err);
			sqlite3_free(err);
		}
		return;
	}
	int num = (nRow + 1) * nColumn;	//第一行为字段名称，nRow只表示有多少行数据
	int index = 0;
	for (i = nColumn; i < num; i++)
	{
		fault[index++] = (UInt8)atoi(dbResult[i]);
		if (index >= MAX_CHANNEL_NUM)
			break;
	}
	sqlite3_free_table(dbResult);
#endif
}
