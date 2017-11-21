#pragma once

#include <string>
#include <atomic>
#include <map>
#include "common.h"
#include "thread.h"

using namespace std;


#ifndef TSM_DLL
#if defined(WIN32) && defined(_MSC_VER)
#define TSM_DLL __declspec(dllimport)
#else
#define TSM_DLL
#endif
#endif

/*
获取SDK版本号
 */
TSM_DLL const string /*__stdcall*/  GetSdkVersion();
/*
SDK初始化，参数为本机连接信号机的ip
此函数必须在使用SDK之前调用
 */
TSM_DLL void /*__stdcall*/  SdkInit(const string &_localip, TscSource src = PLATFORM);

TSM_DLL void /*__stdcall*/  SdkExit();

/*
异步接收，上位机执行异步下载之后需调用此函数来接收信号机的回复信息，信号机回复的消息若有一条失败则此函数立即返回
timeout: 接收回复信息的超时时间
成功时，返回值的succ为true，msg为空
失败时，返回值的succ为false，msg为出错信息
 */
TSM_DLL TscRetval /*_stdcall*/ AsyncRecv(int timeout = 10);

//前向声明
struct _zmsg_t;
typedef struct _zmsg_t zmsg_t;
struct _zsock_t;
typedef struct _zsock_t zsock_t;
struct TscHead;

class TSM_DLL TSM
{
private:
	const string 		ip;					//信号机ip
	zsock_t				*req_encrypt;		//CZMQ请求模式句柄
	zsock_t				*req_noenrypt;		//CZMQ请求模式句柄
	zsock_t				*sub;				//CZMQ订阅模式句柄
	string				serverkey;			//信号机公钥
	atomic_bool			online;				//上位机是否联机
	hik::thread 		thread;				//处理信号机实时上报数据的线程
	CallbackFunc		callback;			//上位机处理实时数据的回调函数
	int 				timeout;			//超时接受时间
	int 				timeoutcount;		//超时接受次数
	string				loginpassword;		//信号机登录密码

	/*请求信号机回复*/
	TscRetval Request(zsock_t *request, const TscHead &head, function<void(zmsg_t *)> fill/* = nullptr*/, function<TscRetval(zmsg_t *&, bool)> deal/* = DealRespondMsg*/);
	/*与信号机建立连接*/
	zsock_t *Connect(int type, int port, TscRetval &ret, bool isEncrypt = false);
	/*获取信号机公钥*/
	void GetServerKey(TscRetval &ret);
	/*验证登录密码*/
	void PasswordVerify(const string &password, TscRetval &ret);
	/*信号机离线处理*/
	void Offline();
	/*信号机订阅消息处理*/
	void Subscribe();

public:
	/*
	设置实时信息处理的回调函数
	_ip: 信号机ip地址
	_callback: 为处理实时主动上报数据的回调函数，此回调函数有两个参数
	回调函数参数1：TSMRealtimeType枚举类型，即需要处理的是什么信息
	回调函数参数2: 要处理的json字符串，具体内容如status.json、alarm.json、traffic.json、cyclestat.json所示
	 */
	TSM(const string &_ip, CallbackFunc _callback, const int _timeout = 3, const int _count = 1);
	~TSM();

	/*
	信号机登录
	password:	信号机登录密码
	若登陆成功，返回信号机软硬件版本信息
	 */
	TscRetval Login(const string &password);

	/*
	信号机注销
	 */
	void Logout();

	/*
	信号机是否在线
	 */
	bool Online();

	/*
	设置信号机配置
	type: 枚举变量，具体请参考common.h
	json: json格式的字符串，如unit.json,channel.json所示
	async: 是否使用异步下载模式，异步下载之后，信号机回复信息统一都是用AsyncRecv函数接受
	eg: Set(TSM_UNIT, unit.json字符串)即为设置信号机单元配置
	 */
	TscRetval Set(TscConfigType type, const string &json, bool async = false);

	/*
	获取信号机配置
	type: 枚举变量，具体请参考common.h
	id: 若指定了id则获取type表项中指定id的配置，不指定则返回全部type有效的配置
		id只对相位、方案、时段、调度这四项配置有效，其他统一是全部有效配置上传(即id=0)
	成功时，返回值的msg为json格式的字符串
	失败时，返回值的msg为出错字符串(非json格式)
	eg: Get(TSM_UNIT)即为返回整个单元的配置, 成功时返回值的msg为unit.json
		Get(TSM_PHASE, 1)即为返回相位id=1的配置, 成功时返回值的msg为phase.json
	 */
	TscRetval Get(TscConfigType type, const unsigned short id = 0);

	/*
	信号机整体设置，仅包括单元、通道、相位、方案、时段、调度、车辆检测器、行人检测器、优先表、倒计时表十项或者其中一两项
	type: 枚举变量，具体请参考common.h
	conf: 为map类型，key为TscConfigType的枚举变量,而value则为对应的json字符串
	例如：conf[TSM_UNIT] = unit.json中的样例
	 */
	TscRetval WholeSet(const map<TscConfigType, string> &conf);
	/*信号机整体配置获取，内容同上*/
	TscRetval WholeGet(map<TscConfigType, string> &conf);

	/*
	清除某段时间的log
	json: json格式的字符串，如log.json所示
	 */
	void ClearLog(const string &json);

	/*
	获取某段时间的log
	json: json格式的字符串，如log.json所示
	返回值为json格式的字符串，如log.json所示
	 */
	TscRetval GetLog(const string &json);

	/*
	信号机升级
	path: 升级包路径
	 */
	TscRetval Update(const char *path);
};

