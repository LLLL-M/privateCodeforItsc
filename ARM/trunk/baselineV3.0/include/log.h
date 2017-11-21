_Pragma("once")

#include <ctime>
#include "db.h"
#include "hik.h"

class Log
{
private:
	hik::database db;
	atomic_uint count;
	const size_t limit;
	typedef unsigned int TimeType;

	void LimitDeal(const TimeType &now, const TimeType &master);
public:
	Log(const char *path = "log.db", const size_t _limit = 0x10000) : db(path, true), limit(_limit)
	{
		count = db.count();
		//cout << "count = " << count << endl;
	}
	//~Log() 
	void Write(const char * data);
	void Write(const string & data);
	template<typename T, typename ... Args>
	void Write(const char * fmt, T && t, Args&&... args);
	//读取log内容，默认是全部读取，并且把时间值转换为2017-09-22 10:00:00格式，可以按照起始与结束时间去读取
	vector<pair<string, string>> Read(time_t start = 0, time_t end = 0, bool isConvert = true);
	//读取log内容并返回json格式
	string Read2Json(time_t start = 0, time_t end = 0);
	//清除log，默认全部清除
	void Erase(time_t start = 0, time_t end = 0);

	//调试打印接口
	template<typename ... Args>
	static void Debug(const char *fmt, Args&&... args)
	{
		cout << COL_GRE << "time" << COL_YEL << "[" << CurrentTime() << "] " << COL_DEF;
		printf(fmt, forward<Args>(args)...);
		cout << endl;
	}
	//出错打印接口
	template<typename ... Args>
	static void Error(const char *fmt, Args&&... args)
	{
		cerr << COL_RED << "time" << COL_YEL << "[" << CurrentTime() << "] " << COL_DEF;
		fprintf(stderr, fmt, forward<Args>(args)...);
		cerr << endl;
	}
	//把time_t时间值转换为2017-09-22 10:00:00格式的string对象
	static string ConvertTime(time_t t);
	//把当前时间值转换为2017-09-22 10:00:00格式的string对象
	static string CurrentTime()
	{
		return ConvertTime(time(nullptr));
	}
	
protected:
};
