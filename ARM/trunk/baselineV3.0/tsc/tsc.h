_Pragma("once")

#include <array>
#include <functional>
#include "czmq.h"
#include "config.h"
#include "rwlock.h"
#include "db.h"

using namespace std;

struct TscHead
{
	string	ver;			//信号机协议版本
	int 	type = -1;		//信号机数据类型，取值如枚举TSMConfigType所示
	bool	upload = false;	//是否是上载
	int 	id = -1;		//上载某个表项，只在上载时使用
	string	src;			//控制源，例如platform or tool
	string 	srcip;			//控制源ip
	bool 	async = false;	//控制源是否异步，异步为router模式
							//的消息，同步为其他模式的消息。
};

template<typename T, int minval, int maxval>
struct TscList
{
	map<int, T>	bak;		//备份数据，用以做参数检查
	map<int, T> use;		//实际运用的数据
	/*解析json*/
	TscRetval Parse(string && data)
	{
		if (data.empty())
			return false;
		return Parse(data.data(), data.size());
	}
	TscRetval Parse(const string &data)
	{
		if (data.empty())
			return false;
		return Parse(data.data(), data.size());
	}
	TscRetval Parse(zframe_t *frame)
	{
		if (frame == nullptr)
			return false;
		return Parse((const char *)zframe_data(frame), zframe_size(frame));
	}
	TscRetval Parse(const char * data, size_t size);
	/*生成json*/
	TscRetval Json(int id = 0);

	/*判断id对应的表项是否存在*/
	bool Exist(int id)
	{
		return (bak.find(id) != bak.end());
	};
};

template<typename T>
struct TscSingle
{
	T conf;
	/*解析json*/
	TscRetval Parse(string && data)
	{
		if (data.empty())
			return false;
		return Parse(data.data(), data.size());
	}
	TscRetval Parse(const string &data)
	{
		if (data.empty())
			return false;
		return Parse(data.data(), data.size());
	}
	TscRetval Parse(zframe_t *frame)
	{
		if (frame == nullptr)
			return false;
		return Parse((const char *)zframe_data(frame), zframe_size(frame));
	}
	TscRetval Parse(const char * data, size_t size)
	{
		if (data == nullptr || size == 0)
			return false;
		return conf.Parse(data, size);
	}
	/*生成json*/
	TscRetval Json();
};

struct Phase;
class Cycle;
class Channel;
struct TscHead;
class Log;
struct TscBarrier;
struct ControlRule;

class Tsc
{
	private:
		UInt8 FindTimeinterval(tm *calTime);
		void FindRule(ControlRule &rule, tm *calTime, int id);
		UInt8 FindRingId(const TscScheme &item, int phaseid);
		Cycle* FixedCycleInit(const ControlRule &rule);

		void UpdateWholeUsr(TscRetval &ret);
		TscRetval WholeParse(zmsg_t* &msg);
		TscRetval WholeCheck();
		TscRetval ScheduleCheck();
		TscRetval TimeintervalCheck();
		TscRetval SchemeCheck();
		TscRetval PhaseCheck();
		TscRetval ChannelCheck();
		TscRetval PriorCheck();
		TscRetval MultilockCheck();

		TscRetval SetStage(TscScheme &sche);
		void StageEstablish(const vector<TscBarrier> &barr, TscScheme &scheme);
		TscRetval BarrierConflictCheck(const vector<TscBarrier> &barr, const TscScheme &scheme);
		
		template<typename T, typename D>
		TscRetval Deal(T && t, bool upload, D && data, UInt8 id, const char *name, function<TscRetval()> Check);

		template<typename T>
		void Load(T &t, const char* name);

		hik::rwlock rwl;
		Log &log;
		hik::database db;

		TscSingle<TscUnit>								unit;				//单元
		TscList<TscChannel, 1, MAX_CHANNEL_NUM>			channel;			//通道表
		TscList<TscPhase, 1, MAX_PHASE_NUM>				phase;				//相位表
		TscList<TscScheme, 0, MAX_SCHEME_NUM>			scheme;				//方案表
		TscList<TscTimeinterval, 1, MAX_TIMELIST_NUM>	timeinterval;		//时段表
		TscList<TscSchedule, 1, MAX_SCHEDULE_NUM>		schedule;			//调度表
		TscList<TscVehDetector, 1, MAX_VEHDETECTOR_NUM>	vehDetector;		//车辆检测器
		TscList<TscPedDetector, 1, MAX_PEDDETECTOR_NUM>	pedDetector;		//行人检测器
		TscList<TscPrior, 1, MAX_PRIOR_NUM>				prior;				//优先表
		TscList<TscCountdown, 1, MAX_COUNTDOWN_NUM>		countdown;			//倒计时牌表
		TscList<TscNetwork, 1, MAX_NETWORK_NUM>			network;			//网络表
		TscList<TscSerial, 1, MAX_SERIAL_NUM>			serial;				//串口表
		TscList<TscKey, 1, MAX_WIRELESS_NUM>			wireless;			//无线遥控器按键表
		TscList<TscKey, 1, MAX_PANEL_NUM>				panel;				//手动面板按键表
		TscList<TscMultiLock, 1, MAX_CHANNEL_NUM>		multilock;			//多通道锁定表
		TscSingle<TscWifi>								wifi;				//wifi配置
		TscSingle<TscRedCheck>							redcheck;			//红灯信号检测器
		int 											timezone = 0;		//时区
		string											password = "hiklinux";//信号机联机密码

		TscRetval HeadParse(zframe_t *frame, TscHead &head);
		TscRetval ParseSecFrame(const TscHead &head, zframe_t* frame);

		TscRetval TimeSet(zframe_t *frame);
		TscRetval PasswordUpdate(zframe_t *frame);

	public:
		friend class Communication;
		Tsc();
		Cycle* GetInitCycle(ControlRule &rule);
		Cycle* GetCycle(ControlRule &rule);

};


