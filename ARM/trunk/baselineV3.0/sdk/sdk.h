#pragma once

#include <string>
#include <atomic>
#include <functional>
#include <ctime>
#include <cstring>
#include "czmq.h"
#include "common.h"
#include "file.h"

#define REQ_REP_NOENCRYPT_PORT		30000			//传输公钥使用
#define REQ_REP_ENCRYPT_PORT		30001			//上下载使用
#define SUB_PUB_NOENCRYPT_PORT		30002			//主动上报使用
#define ROUTER_DEALER_ENCRYPT_PORT	30003			//平台异步分发使用

#define MAX_DATA_SIZE (409600)	//最大发送的数据大小为400k，超过此大小则使用未加密端口发送数据

using namespace std;

const std::string CurrentTime()
{
	char buf[20] = {0};
#ifdef __linux__
	time_t now = time(nullptr);
	struct tm t;
	localtime_r(&now, &t);
	strftime(buf, sizeof(buf), "%F %T", &t);
#else	//WIN32
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
#endif
	return std::string(buf);
}

struct TscHead
{
	//string ver = sdk.version;
	TscConfigType type;
	bool upload;
	int id;
	bool async;
	//string src;
	//string srcip
	TscHead(TscConfigType _type, bool _upload = true, int _id = 0, bool _async = false)
	{
		type = _type;
		upload = _upload;
		id = _id;
		async = _async;
	}

	string Json() const;
};

struct Log
{
	hik::file file;
	Log() : file("log.txt") {}
	template<typename ... Args>
	void operator()(const char *fmt, Args&&... args)
	{
		if (fmt == nullptr)
			return;
		if (sizeof...(args) == 0)
			Write(fmt, strlen(fmt));
		else
		{
			char buf[128] = {0};
			sprintf(buf, fmt, forward<Args>(args)...);
			Write(buf, strlen(buf));
		}
	}
	void operator()(const string &str)
	{
		if (!str.empty())
			Write(str.c_str(), str.size());
	}
	void Write(const void *msg, size_t size)
	{
		if (msg == nullptr || size == 0)
			return;
		string curtime = CurrentTime();
		file.Write(curtime.data(), curtime.size());
		file.Write(msg, size);
		file.Write("\r\n", 2);
		file.Fsync();
	}
};

static Log debug;

struct SDK
{
	string version;				//SDK版本号
	zsock_t *router;			//CZMQ路由模式句柄
	atomic_int	packages;		//分发的数据包个数
	string source;				//控制源
	string localip;				//本地ip
	zcert_t *cert;				//本地证书句柄
	zactor_t *auth;				//curve引擎
	string publickey;			//路由的公钥,bin存储格式

	SDK()
	{
		version = "V3.0.0.0";
		router = nullptr;
		packages = 0;
		cert = nullptr;
		auth = nullptr;
	}

	~SDK()
	{
		Exit();
	}

	void Init(const string &_localip, TscSource src);

	void Exit();

	void Send(zmsg_t *&sndmsg);

	TscRetval Recv(int timeout);
};

static SDK sdk;

#define TSC_RET_SUCC		0x63637573		//对应于字符串"succ"
#define TSC_RET_FAIL		0x6c696166		//对应于字符串"fail"

static TscRetval DealRespondMsg(zmsg_t *&rcvmsg, bool upload)
{
	TscRetval ret;
	if (rcvmsg == nullptr)
	{
		ret.Err("rcvmsg == nullptr!");
		return ret;
	}
	zframe_t *first = zmsg_first(rcvmsg);
	zframe_t *second = zmsg_next(rcvmsg);
	unsigned int *flag = (unsigned int *)(zframe_data(first));
	if (flag != nullptr && *flag == TSC_RET_SUCC)
	{
		if (upload)
		{
			if (second == nullptr)
				ret.Err("recv respond msg but not include upload data!");
			else
				ret.msg.assign((char *)zframe_data(second), zframe_size(second));
		}
		else
		{
			if (second != nullptr)
				ret.msg.assign((char *)zframe_data(second), zframe_size(second));
		}
	}
	else
	{
		string erro((char *)zframe_data(second),zframe_size(second));
		ret.Err(second ? erro.data() : nullptr);
	}
	zmsg_destroy(&rcvmsg);
	return ret;
}

string TscHead::Json() const
{
	string head = R"({"ver":")";
	head += sdk.version;
	head += R"(","type":)";
	head += to_string(type);
	head += (upload) ? R"(,"upload":true,"id":)" : R"(,"upload":false,"id":)";
	head += to_string(id);
	head += (async) ? R"(,"async":true,"src":")" : R"(,"async":false,"src":")";
	head += sdk.source;
	head += R"(","srcip":")";
	head += sdk.localip;
	head += R"("})";
	return head;
}

void SDK::Init(const string &_localip, TscSource src)
{
	localip = _localip;
	switch (src)
	{
		case CONFIG_TOOL: source = "config tool"; break;
		case TOUCH_SCREEN: source = "touch screen"; break;
		case MOBILE_APP: source = "mobile app"; break;
		default: source = "platform"; break;
	}
	if (router == nullptr && src == PLATFORM)
	{
		router = zsock_new(ZMQ_ROUTER);
		assert(router != nullptr);
	}
	//创建curve验证机制的引擎
	if (cert == nullptr)
	{
		byte pk[32] = {23,15,93,165,135,131,40,5,126,136,66,117,70,81,70,13,153,156,116,47,224,174,176,215,154,108,69,223,149,27,209,111};
    	byte sk[32] = {97,168,71,170,208,26,10,28,130,176,96,218,183,108,38,129,97,129,34,170,221,23,68,78,27,219,90,168,170,197,102,89};
    	cert = zcert_new_from(pk, sk);
    	assert(cert != nullptr);
	}
	if (auth == nullptr)
	{
		zcertstore_t *certstore = zcertstore_new(nullptr);
		assert(certstore != nullptr);
		zcert_t *tmp = zcert_dup(cert);	//创建一个临时的zcert，因为调用zcertstore_insert之后会把cert设置为NULL,而cert下文还要用到
		assert(tmp != nullptr);
		zcertstore_insert(certstore, &tmp);
		auth = zactor_new(zauth, certstore);
		assert(auth != nullptr);
	}
	if (router != nullptr)
	{
		zcert_t *server_cert = zcert_new();
		assert(server_cert != nullptr);
		publickey = zcert_public_txt(server_cert);
		zcert_apply(server_cert, router);
	    zsock_set_curve_server(router, 1);
	    zcert_destroy(&server_cert);
		//FIXME 需要判断bind是否成功
	    zsock_bind(router, "tcp://%s:%d", localip.c_str(), ROUTER_DEALER_ENCRYPT_PORT);
	}
}

void SDK::Exit()
{
	if (auth != nullptr)
	{
		zactor_destroy(&auth);
		auth = nullptr;
	}
	if (cert != nullptr)
	{
		zcert_destroy(&cert);
		cert = nullptr;
	}
	if (router != nullptr)
	{
		zsock_destroy(&router);
		router = nullptr;
	}
	source.clear();
	localip.clear();
	publickey.clear();
#if defined(WIN32) || defined(__WIN32__)
	zsys_shutdown();
#endif
}

void SDK::Send(zmsg_t *&sndmsg)
{
	if (router == nullptr || sndmsg == nullptr)
		return;
	if (0 != zmsg_send(&sndmsg, router))
		zmsg_destroy(&sndmsg);
	packages++;
}

TscRetval SDK::Recv(int timeout)
{
	TscRetval ret;
	zsock_set_rcvtimeo(router, timeout);
	do
	{
		zmsg_t *msg = zmsg_recv(router);
		ret = DealRespondMsg(msg, false);
		if (!ret.succ)
		{
			packages = 0;
			return ret;
		}
	} while(packages--);
	return ret;
}
