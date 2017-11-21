_Pragma("once")

#include <string>
#include "hik.h"
#include "common.h"
#include <bitset>
#include <vector>
#include <map>
#include "json/json.h"

struct _zframe_t;
typedef struct _zframe_t zframe_t;

//单元配置
struct TscUnit
{
	UInt8	area = 0;			//区域编号
	UInt16	junction = 0;		//交叉路口编号
	UInt8	bootFlash = 6;		//启动黄闪时间
	UInt8	bootAllRed = 6;		//启动全红时间
	UInt16	collect = 300;		//交通流采集周期
	UInt8	transition = 2;		//协调过渡周期
	bool	GPS = true;			//GPS开关
	bool 	watchdog = false;	//watchdog开关
	bool 	voltCheck = false;	//电压检测开关
	bool 	curCheck = false;	//电流检测开关
	bool 	faultFlash = false;	//故障黄闪开关
	bool 	takeover = false;	//灯控接管开关

	/*解析unit.json*/
	TscRetval Parse(zframe_t *frame);
	TscRetval Parse(std::string str);
	TscRetval Parse(const char *data, size_t size);

	/*生成unit.json*/
	Json::Value Json();
};

struct TscChannel
{
	UInt8	id = 0;								//通道编号
	UInt8	type = 0;							//通道类型
	UInt8	status = 0;							//通道指定状态
	bitset<MAX_CHANNEL_NUM>	conflictBits = 0;	//冲突通道bits
	UInt8	countdown = 0;						//通道对应的倒计时牌编号
	bitset<MAX_VEHDETECTOR_NUM> vehBits = 0;	//通道对应的车辆检测器编号bits
	bitset<MAX_PEDDETECTOR_NUM> pedBits = 0;	//通道对应的行人检测器编号bits
	string	desc;								//通道描述

	/*解析channel.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成channel.json的一个表项*/
	Json::Value Json();
};

struct TscPhase
{
	UInt8	id = 0;								//相位编号
	string 	desc;								//相位描述
	UInt8	greenFlash = 0;						//绿闪时间
	UInt8	yellow = 0;							//黄灯时间
	UInt8	allred = 0;							//全红时间
	UInt8	redyellow = 0;						//红黄时间
	UInt16	pedClear = 0;						//行人清空时间
	UInt16	minGreen = 0;						//最小绿
	UInt16	maxGreen = 0;						//最大绿
	UInt16	maxGreen2 = 0;						//最大绿2
	UInt8	unitExtend = 0;						//单位延长绿
	UInt8	checkTime = 0;						//感应检测时间
	bitset<MAX_CHANNEL_NUM>	channelBits = 0;	//包含通道bits
	bool 	autoReq = false;					//自动请求开关

	/*解析phase.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成phase.json的一个表项*/
	Json::Value Json();
};

struct TscStage
{
	UInt8 	id;							//阶段ID号
	vector<UInt8> phase;				//阶段内包含相位号
	TscStage(UInt8 _id):id(_id)
	{
	}
};

struct TscScheme
{
	UInt8	id = 0;				//方案编号
	string	desc;				//方案描述
	UInt16	cycle = 0;			//周期时间
	UInt16	offset = 0;			//相位差
	UInt8	coordPhase = 0;		//协调相位

	struct PhaseInfo
	{
		UInt8	phase = 0;		//相位编号
		UInt16	time = 0;		//相位时长
		UInt8	status = 0;		//相位指定状态
		UInt8	barrier = 0;	//相位所处的屏障号
	};
	map<int, PhaseInfo> timing;
	vector<vector<UInt8>> turn;

	vector<TscStage> stage;

	/*解析scheme.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成scheme.json的一个表项*/
	Json::Value Json();
};

struct TscTimeinterval
{
	UInt8	id = 0;				//时段表编号
	struct TimeSection
	{
		UInt8	hour = 0;		//起始小时
		UInt8	minute = 0;		//起始分钟
		UInt8	mode = 0;		//控制模式
		UInt8	scheme = 0;		//控制方案
	};
	vector<TimeSection> section;

	/*解析timeinterval.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成timeinterval.json的一个表项*/
	Json::Value Json();
};

struct TscSchedule
{
	UInt8	id = 0;				//调度编号
	struct TscDate
	{
		UInt16	year = 0;		//年份
		UInt8	month = 0;		//月份
		bitset<32>	days = 0;	//日子，阳历
	};
	vector<TscDate>	date;		//日期
	bitset<8>	week = 0;		//星期
	UInt8 timeinterval = 0;		//时段表

	/*解析schedule.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成schedule.json的一个表项*/
	Json::Value Json();
};

struct TscVehDetector
{
	UInt8	id = 0;				//车辆检测器编号
	UInt8	type = 0;			//车辆检测器类型
	string	desc;				//车检器描述
	UInt8	coilLen = 0;		//线圈长度
	UInt8	gap = 0;			//检测线圈距离停止线距离
	UInt8	noResponse = 0;		//无响应时间
	UInt8	maxContinuous = 0;	//最大持续时间
	UInt8	maxVehcileNum = 0;	//每分钟最大过车数
	UInt8	minTimeGap = 0;		//最小车头时距
	UInt8	maxTimeGap = 0;		//最大车头时距
	UInt8	minQueueLen = 0;	//最小排队长度
	UInt8	maxQueueLen = 0;	//最大排队长度
	UInt8	occupyJam = 0;		//拥堵占有率

	/*解析vehdetector.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成vehdetector.json的一个表项*/
	Json::Value Json();
};

struct TscPedDetector
{
	UInt8	id = 0;				//行人检测器编号
	UInt8	delayPass = 0;		//行人延迟放行时间
	UInt16	maxWait = 0;		//行人最大等待时间

	/*解析peddetector.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成peddetector.json的一个表项*/
	Json::Value Json();
};

struct TscCtrl
{
	UInt8	ctrlType = 0;	//控制类型
	UInt8	ctrlMode = 0;	//控制模式
	UInt8	ctrlId = 0;		//控制编号
	UInt32	duration = 0;	//控制持续时间
	vector<UInt8> status;		//锁定状态
	//array无法在声明的时候初始化

	/*解析ctrl.json的一个表项*/
	TscRetval Parse(const Json::Value &value);
	TscRetval Parse(zframe_t *frame);

	/*生成ctrl.json的一个表项*/
	Json::Value Json();
};

struct TscPrior
{
	UInt8	id = 0;				//优先编号
	string	label;				//优先标识，车牌号或RFID标签号
	string	reader;				//RFID阅读器标识
	UInt8	channel = 0;		//优先要放行的通道
	TscCtrl	ctrl;				//按键控制

	/*解析prior.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成prior.json的一个表项*/
	Json::Value Json();
};

struct TscCountdown
{
	UInt8	id = 0;				//倒计时牌编号
	UInt8	mode = 0;			//倒计时使用的模式
	UInt8	pulseGreen = 0;		//半程脉冲倒计时绿灯时间
	UInt8	pulseRed = 0;		//半程脉冲倒计时红灯时间

	/*解析countdown.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成countdown.json的一个表项*/
	Json::Value Json();
};

struct TscNetwork
{
	UInt8	id = 0;			//网口编号
	string	desc;			//网口描述
	string	mac;			//mac地址
	string	ip;				//ip地址
	string	netmask;		//子网掩码
	string	gateway;		//网关

	/*解析network.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成network.json的一个表项*/
	Json::Value Json();
};

struct TscSerial
{
	UInt8	id = 0;				//串口编号
	string	desc;				//串口描述
	UInt32	baudRate = 0;		//波特率
	UInt8	dataBits = 0;		//数据位
	UInt8	stopBits = 0;		//停止位
	string	parity;				//校验

	/*解析serial.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成serial.json的一个表项*/
	Json::Value Json();
};

struct TscWifi
{
	string ssid;				//wifi的识别名
	string password;			//wifi密码

	/*解析wifi.json*/
	TscRetval Parse(zframe_t *frame);
	TscRetval Parse(std::string str);
	TscRetval Parse(const char *data, size_t size);

	/*生成wifi.json*/
	Json::Value Json();
};

struct TscRedCheck
{
	bool 	enable = false;		//红灯信号检测使能开关
	string	ip;					//发送红灯信号的组播地址
	UInt16	port = 0;			//组播端口

	/*解析redcheck.json*/
	TscRetval Parse(zframe_t *frame);
	TscRetval Parse(std::string str);
	TscRetval Parse(const char *data, size_t size);

	/*生成redcheck.json*/
	Json::Value Json();
};

struct TscKey
{
	UInt8	id = 0;				//按键编号
	bool 	enable = false;		//按键开关
	string	desc;				//按键描述
	TscCtrl	ctrl;				//按键控制

	/*解析key.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成key.json的一个表项*/
	Json::Value Json();
};

struct TscMultiLock
{
	UInt8	id = 0;							//锁定时段编号
	UInt8	hour = 0;						//锁定开始小时
	UInt8	minute = 0;						//锁定开始分钟
	UInt8	sec = 0;						//锁定开始秒数
	UInt16	duration = 0;					//锁定时长
	vector<UInt8> status;					//锁定状态

	/*解析multilock.json的一个表项*/
	TscRetval Parse(const Json::Value &value);

	/*生成multilock.json的一个表项*/
	Json::Value Json();

};

struct TscLog
{
	UInt64	start = 0;				//开始UTC时间
	UInt64 	end = 0;				//结束UTC时间
	/*解析serial.json的一个表项*/
	TscRetval Parse(zframe_t *frame);

	/*生成serial.json的一个表项*/
	TscRetval Json();
};
