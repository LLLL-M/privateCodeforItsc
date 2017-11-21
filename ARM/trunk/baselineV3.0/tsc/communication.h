_Pragma("once")

#include <string>
#include <list>
#include "czmq.h"
#include "common.h"
#include "thread.h"

struct SDKInfo
{
	const string ip;	//SDK 的IP地址
	const string pk;	//SDK 的随机公钥
	zsock_t* dealer;	//SDK 的dealer zsock指针
	zactor_t* monitor;	//SDK 的dealer zsock对应的monitor指针

	//constructor, emplace_back needed
	SDKInfo(const string &_ip, const string &_pk, zsock_t* _dealer, zactor_t* _monitor) : ip(_ip), pk(_pk), dealer(_dealer), monitor(_monitor)
	{
	}
};

class Tsc;

class Communication : public hik::thread
{
	private:
		zsock_t *rep_noencrypt;			//ZMQ_REP 非加密ZMQ的请求回复模式sock指针
		zsock_t *rep_encrypt;			//ZMQ_REP 加密ZMQ的请求回复模式sock指针
		zsock_t *pub;					//ZMQ_PUB 非加密的发布模式sock指针
		zloop_t *loop;					//事件轮询器指针，控制事件轮询器的设置，启动，增减轮询条数。
		string public_key; 				//ECC公钥，用于加密连接中向外发送的公钥字符串
		zactor_t *auth;					//ECC验证引擎，用于加解密消息
		zcert_t *store_cert;			//ECC证书管理器，用于管理ECC证书的公钥和私钥
		std::list<SDKInfo> sdkinfo;		//TSC端用于管理连接成功的SDK信息的容器

		//zloop 轮询的回调函数，当zloop轮询的zsock有消息传入时，回调此函数，处理消息。
		//本函数负责将SDK传入的json文本进行解析并执行相关功能。
		static int ParseMsg(zloop_t *loop, zsock_t *reader, void *arg);
		
		//zloop 轮询的zmonitor的回调函数，当zloop轮询的zmonitor有消息传入时，回调此函数，处理消息。
		//本函数负责注销断开的SDK在TSC端保存的信息。
		static int DealerRemove(zloop_t *loop, zsock_t *reader, void *arg);	
		
		//ECC证书管理服务器初始化函数，负责ECC加密的初始化，包括证书管理器和加解密引擎的初始化。
		//回复：成功（初始化成功的随机公钥私钥证书指针）失败（空指针）。
		zcert_t *CertstoreInit();

		//回复函数，回复信号机处理生成的消息。
		//参数1: TscRetval 为消息内容，包括处理成功与否的bool变量和消息信息的string变量。
		//参数2: zsock_t* 为消息需要发送向的sock指针
		void Reply(const TscRetval &ret, zsock_t *socket);

		//轮询器新增条目函数，负责往轮询器中新增需要处理的zsock指针及其回调函数。
		//参数1: zsock* 为轮询器需要轮询的zsock指针。
		//参数2: zloop_reader_fn 为轮询器规定的回调函数指针。
		//参数3：轮询器提供的可传入变量类型。
		void LoopInsert(zsock_t *sock, zloop_reader_fn handler, void *arg);

		//Dealer初始化函数，当SDK的控制源为platform的时候，TSC端需要对应生成dealer sock，即调用函数生成对应的dealer sock并对其进行加密。
		//参数1：连接上的SDK相关信息，用于生成Dealer zsock和存储SDK的ip，公钥，zsock指针和对应的zmonitor指针。
		void DealerInit(SDKInfo &s);

		//登陆函数，负责处理SDK的登陆信息。
		//参数1：登陆消息除头帧外第一帧，此帧为SDK传入的登陆密码。
		//参数2：登陆消息除头帧外第二帧，此帧为SDK传入的加密使用的公钥。
		//参数3：登陆的SDK对应的IP字符串。
		//参数4：登陆的SDK所述的控制源类型字符串。
		//回复：登陆成功（成功、软硬件版本号）登陆失败（失败，错误信息）。
		TscRetval Login(zframe_t *frame, zframe_t *frameNext, const string &srcip, const string &src);
		
		//升级包存储函数，负责存储SDK发送来的升级包（可以为任意大小文件）
		//参数1：SDK发来的升级包消息体。
		//回复：成功（成功） 失败（失败、错误信息）
		TscRetval UpdatepkgSave(zmsg_t *self);
		
		Tsc &tsc;
	public:
		//构造函数
		Communication();
		//析构函数
		~Communication();

		//初始化函数，继承于HikThread::Thread::Run，在线程启动的时候，自动调用此函数
		void Run();

		//外部通信模块发送实时状态的接口，输入信息为实时状态的json字符串，字符串格式见state.json
		void SendRealtimeStat(const string &data);

		//外部通信模块发送报警信息的接口，输入信息为报警信息的json字符串，字符串格式见alarm.json
		void AlarmSend(const string &s); 
};
