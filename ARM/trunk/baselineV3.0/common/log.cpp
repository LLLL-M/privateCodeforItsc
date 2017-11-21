#include <algorithm>
#include <cassert>
#include "log.h"
#if defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#endif

string Log::ConvertTime(time_t t)
{
	struct tm tm;
#ifdef __linux__
	localtime_r(&t, &tm);
#else //__WIN32__ or WIN32
	localtime_s(&tm, &t);
#endif
	char buf[24] = {0};
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return string(buf);
}

void Log::LimitDeal(const TimeType &now, const TimeType &master)
{
	if (++count <= limit)
		return;
	//超过设定上限时删掉一半的记录
	TimeType *data = new TimeType[count];
	if (data == nullptr)
	{
		db.appendfmt(now - 1, "Can't alloc memory when function[%s];", __func__);
		db.append(master, now - 1);
		assert(data == nullptr);
	}
	memset(data, 0, count * sizeof(TimeType));
	size_t size = count * sizeof(TimeType);
	size_t half = limit / 2;
	if (db.fetch(master, data, &size) && (size / sizeof(TimeType) > half))
	{
		size_t n = size / sizeof(TimeType);
		sort(data, data + n);	//对存储的key进行排序，然后再从头根据key的范围删除一半log
		for (size_t i = 0; i < half; i++)
			db.del(data[i]);
		count -= half;
		db.store(master, data + half, sizeof(TimeType) * (n - half));
	}
	else
	{	/*说明log数据库存储有问题应该全部删除再重新开始插入*/
		db.del_if();
		TimeType prev = now - 1;
		db.store(prev, "The database for storing log occur fault, init it!");
		db.store(master, prev);
		count = 2;
	}
	delete [] data;
}

void Log::Write(const char * data)
{
	TimeType now = (TimeType)time(nullptr);
	TimeType master = 0;

	LimitDeal(now, master);

	db.appendfmt(now, "%s;", data);	//以当前时间作为key写入log
	db.append(master, now);				//并把当前时间存入master
	db.commit();
	Debug(data);
}

void Log::Write(const string & data)
{
	TimeType now = (TimeType)time(nullptr);
	TimeType master = 0;

	LimitDeal(now, master);

	db.appendfmt(now, "%s;", data.c_str());	//以当前时间作为key写入log
	db.append(master, now);				//并把当前时间存入master
	db.commit();
	Debug(data.c_str());
}

template<typename T, typename ... Args>
void Log::Write(const char * fmt, T && t, Args&&... args)
{
	TimeType now = (TimeType)time(nullptr);
	TimeType master = 0;
	string s = fmt;
	s += ';';	//添加;分隔符

	LimitDeal(now, master);

	db.appendfmt(now, s.c_str(), t, forward<Args>(args)...);	//以当前时间作为key写入log
	db.append(master, now);				//并把当前时间存入master
	db.commit();
	Debug(fmt, t, forward<Args>(args)...);
}

vector<pair<string, string>> Log::Read(time_t start/* = 0*/, time_t end/* = 0*/, bool isConvert/* = true*/)
{
	auto condition = [&start, &end](const string &key)->bool{
		if (key.size() != sizeof(TimeType))
			return false;
		time_t t = *(const TimeType *)key.data();
		if ((start == 0 && end == 0 && t > 0)	//t>0避免读取master数据，因为master的key=0
			|| (start > 0 && end == 0 && t >= start)
			|| (start > 0 && end > 0 && t >= start && t < end))
			return true;
		return false;
	};
	using Pair = pair<string, string>;
	vector<Pair> &&vec = db.fetch_if(condition);

	auto cmp = [](const Pair & p1, const Pair & p2)->bool{
		TimeType k1 = *(const TimeType *)(p1.first.data());
		TimeType k2 = *(const TimeType *)(p2.first.data());
		return k1 < k2;
	};
	sort(vec.begin(), vec.end(), cmp);

	if (isConvert)
	{
		for (auto &i: vec)//把time_t时间值转换成"2017-07-20 17:01:00"字符串格式
		{
			time_t t = *(const TimeType *)(i.first.data());
			i.first = ConvertTime(t);
		}
	}
	return vec;
}

string Log::Read2Json(time_t start/* = 0*/, time_t end/* = 0*/)
{
	vector<pair<string, string>> &&vec = Read(start, end);
	if (vec.empty())
		return "[]";

	string log = "[";
	for (auto &i: vec)
	{
		log += R"({"time":")";
		log += i.first;
		log += R"(","msg":")";
		log += i.second;
		log += R"("},)";
	}
	log.back() = ']';	//把最后一个多余的逗号替换成]
	return log;
}

void Log::Erase(time_t start/* = 0*/, time_t end/* = 0*/)
{
	if (end > 0 && start > end)
		return;
	if (start == 0 && end == 0)
	{	//删除全部记录
		db.del_if();
		return;
	}

	TimeType master = 0;
	TimeType *data = new TimeType[count];
	if (data == nullptr)
	{
		TimeType now = (TimeType)time(nullptr);
		db.appendfmt(now, "Can't alloc memory when function[%s];", __func__);
		db.append(master, now);
		assert(data == nullptr);
	}

	size_t size = count * sizeof(TimeType);
	if (db.fetch(master, data, &size))
	{
		int n = size / sizeof(TimeType);
		vector<TimeType> vec;
		for (int i = 0; i < n; i++)
		{
			if (data[i] >= (TimeType)start && (end == 0 || data[i] < (TimeType)end))
				db.del(data[i]);
			else
				vec.emplace_back(data[i]);	//把未删除的key临时存放到vector里
		}
		if (vec.empty())
		{	//说明记录全部清除
			db.del(master);	//清除master
			count = 0;
		}
		else
		{
			db.store(master, vec.data(), vec.size() * sizeof(TimeType));	//把未删除的key存入master
			count = vec.size() + 1;	//加1是因为还有一个master
		}
	}

	delete [] data;
}
