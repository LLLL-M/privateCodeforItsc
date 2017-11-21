#if defined(WIN32) && defined(_MSC_VER)
#define TSM_DLL __declspec(dllexport)
#else	//__GNUC__
#define TSM_DLL
#endif

#include <cassert>
#include <cstring>
#include "sdk.h"
#include "tsm.h"

TSM_DLL const string GetSdkVersion()
{
	return sdk.version;
}

TSM_DLL void SdkInit(const string &_localip, TscSource src)
{
	if (sdk.localip == _localip)
		return;
	sdk.Init(_localip, src);
}

TSM_DLL void SdkExit()
{
	sdk.Exit();
}

TSM_DLL TscRetval AsyncRecv(int timeout/* = 10*/)
{
	return sdk.Recv(timeout * 1000);
}

TscRetval TSM::Request(zsock_t *request, const TscHead &head, function<void(zmsg_t *)> fill = nullptr, function<TscRetval(zmsg_t *&, bool)> deal = DealRespondMsg)
{
	TscRetval ret;
	if (request == nullptr || (head.type != TSC_TYPE_PUBKEY && head.type != TSC_TYPE_LOGIN && !online))
	{
		ret.Err("Can't operate when offline, you must login at first!");
		return ret;
	}
	//封装发送消息
	zmsg_t *sndmsg = zmsg_new();
	if (sndmsg == nullptr)
	{
		ret.Err("zmsg_new() fail!");
		return ret;
	}
	zmsg_addstr(sndmsg, head.Json().data());
	if (fill != nullptr)
		fill(sndmsg);
	if (head.async)
	{	//异步模式则使用路由下发信息
		zframe_t *first = zframe_new(serverkey.data(), serverkey.size());
		if (first == nullptr)
		{
			ret.Err("zframe_new() fail!");
			return ret;
		}
		zmsg_prepend(sndmsg, &first);	//把信号机的公钥作为第一帧，因为信号机dealer要用此做为识别
		sdk.Send(sndmsg);
		return ret;
	}
	//发送消息并接收
	if (zmsg_content_size(sndmsg) > MAX_DATA_SIZE)
		request = req_noenrypt;	//当发送的消息过大时如果使用加密传输会导致设备端解密占用cpu过高，因此当超过设定的限制则使用非加密传输
	zmsg_t *rcvmsg = nullptr;
	for (int i = 0; i < timeoutcount; ++i)
	{	//循环超时次数发送数据
		zmsg_t *msg = zmsg_dup(sndmsg);
		if (0 != zmsg_send(&msg, request))
			zmsg_destroy(&msg);
		rcvmsg = zmsg_recv(request);
		if (rcvmsg != nullptr && zmsg_size(rcvmsg) > 0 && zmsg_content_size(rcvmsg) > 0)
			break;
		else
			zmsg_destroy(&rcvmsg);
	}
	zmsg_destroy(&sndmsg);
	if (rcvmsg == nullptr)
	{
		ret.Err("Can't recv respond msg from TSM[%s] when %s! TscConfigType:%d", ip.data(), head.upload ? "upload" : "download", head.type);
		return ret;
	}
	//处理接收到的消息
	return deal(rcvmsg, head.upload);
}

zsock_t * TSM::Connect(int type, int port, TscRetval &ret, bool isEncrypt/* = false*/)
{
	zsock_t *zsock = zsock_new(type);
	if (zsock == nullptr)
	{
		ret.Err("Can't create req or sub when zsock_new(), ip:%s", ip.data());
		return nullptr;
	}
	if (isEncrypt)
	{
		zcert_apply(sdk.cert, zsock);
    	zsock_set_curve_serverkey(zsock, serverkey.data());
	}
	zsock_set_connect_timeout(zsock, timeout);
	for (int i = 0; i < timeoutcount; i++)
	{
		if (0 == zsock_connect(zsock, "tcp://%s:%d", ip.data(), port))
		{
			zsock_set_rcvtimeo(zsock, timeout);
			return zsock;
		}
	}
	ret.Err("zsock_connect failed because of timeout! ip:%s", ip.data());
	zsock_destroy(&zsock);
	return nullptr;
};

void TSM::GetServerKey(TscRetval &ret)
{
	if (!serverkey.empty())
		return;
	//发送获取公钥头部给信号机用以获取信号机公钥
	TscHead head(TSC_TYPE_PUBKEY);
	auto DealMsg = [this](zmsg_t *&rcvmsg, bool upload)->TscRetval{
		TscRetval ret;
		zframe_t *frame = zmsg_first(rcvmsg);
		if (zframe_size(frame) == 40)
			serverkey.assign((const char *)zframe_data(frame), zframe_size(frame));
		else
			ret.Err("the length of serverkey isn't 40");
		zmsg_destroy(&rcvmsg);
		return ret;
	};
	ret = Request(req_noenrypt, head, nullptr, DealMsg);
}

void TSM::PasswordVerify(const string &password, TscRetval &ret)
{
	TscHead head(TSC_TYPE_LOGIN, false);
	auto FillMsg = [&password](zmsg_t *sndmsg){
		zmsg_addmem(sndmsg, password.data(), password.size());//发送登录密码用以验证
		zmsg_addmem(sndmsg, sdk.publickey.data(), sdk.publickey.size());//发送随机公钥
	};
	ret = Request(req_encrypt, head, FillMsg);
	if (!ret.succ)
		Offline();
	else
		loginpassword = password;
}

void TSM::Offline()
{
	online = false;
	if (req_noenrypt != nullptr)
		zsock_destroy(&req_noenrypt);
	if (req_encrypt != nullptr)
		zsock_destroy(&req_encrypt);
	if (sub != nullptr)
		zsock_destroy(&sub);
	serverkey.clear();
}

void TSM::Subscribe()
{
	auto Alarm = [this](const string &msg){
		if (callback)
		{
			string alarm = R"({"time":")";
			alarm += CurrentTime();
			alarm += R"(","alarm":")";
			alarm += msg;
			alarm + R"("})";
			callback(TSC_RT_ALARM, alarm);	//上报离线信息
		}
	};

	zmsg_t *msg = nullptr;
	while (true)
	{
		if (!online && sub == nullptr)
		{	//断线自动重新连接
			if (loginpassword.empty())
				break;
			TscRetval ret = Login(loginpassword);
			if (ret)
				Alarm(ip + " auto reconnect!");
			else
			{
				Alarm(ip + " auto login failed, because " + ret.msg);
				zclock_sleep(timeout);	//延时上位机设置的超时时间然后再次自动重连
				continue;
			}	
		}

		msg = zmsg_recv(sub);
		if (msg == nullptr)
		{
			Offline();
			Alarm(ip + " has already offline!!!");
			continue;
		}
		if (online)
		{	//只有在线时才处理订阅信息
			zframe_t *first = zmsg_first(msg);
			zframe_t *second = zmsg_next(msg);
			if (callback != nullptr && first != nullptr && second != nullptr/* && zframe_size(first) == 4*/)
			{
				int type = *(int *)zframe_data(first);
				string data((char *)zframe_data(second), zframe_size(second));
				callback(static_cast<TscRealtimeType>(type), data);
				//debug(data);
			}
		}
		zmsg_destroy(&msg);
	}
}

TSM::TSM(const string &_ip, CallbackFunc _callback, const int _timeout/* = 3*/, const int _count/* = 1*/) : ip(_ip), callback(_callback)
{
	req_noenrypt = nullptr;
	req_encrypt = nullptr;
	sub = nullptr;
	online = false;
	timeout = _timeout * 1000;	//因为超时单位为ms
	timeoutcount = _count;
}

TSM::~TSM()
{
	thread.stop();
	Offline();
}

TscRetval TSM::Login(const string &password)
{
	TscRetval ret;
	if (online)	//已经处于联机状态直接返回成功
		return ret;
	if (password.empty())
	{
		ret.Err("the argument error when login, password:%s", password.data());
		return ret;
	}

	/*创建用于上下载的请求zsock，不加密*/
	if (req_noenrypt == nullptr)
	{
		req_noenrypt = Connect(ZMQ_REQ, REQ_REP_NOENCRYPT_PORT, ret);
		if (req_noenrypt == nullptr)
			return ret;
		zsock_set_sndhwm(req_noenrypt, 0);	//设置发送不缓存msg
	}
	GetServerKey(ret);	//获取信号机的公钥
	if (!ret)
		return ret;

	/*创建用于上下载的请求zsock，并连接设置curve加密*/
	if (req_encrypt == nullptr)
	{
		req_encrypt = Connect(ZMQ_REQ, REQ_REP_ENCRYPT_PORT, ret, true);
		if (req_encrypt == nullptr)
		{
			Offline();
			return ret;
		}
		zsock_set_sndhwm(req_encrypt, 0);	//设置发送不缓存msg
	}
	PasswordVerify(password, ret);//登录密码验证
	if (!ret)
		return ret;

	/*密码验证成功开始创建用于订阅信号机实时信息的zsock, 不加密*/
	if (sub == nullptr)
	{
		sub = Connect(ZMQ_SUB, SUB_PUB_NOENCRYPT_PORT, ret);
		if (sub == nullptr)
		{
			Offline();
			return ret;
		}
		zsock_set_subscribe(sub, "");	//设置订阅消息第一帧的过滤
		zsock_set_rcvtimeo(sub, timeout * timeoutcount + 1);	//设置订阅超时接受时间
	}

	online = true;
	if (thread.isdead())
		thread.start(bind(&TSM::Subscribe, ref(*this)));	//启动订阅线程
	return ret;
}

void TSM::Logout()
{
	online = false;
}

bool TSM::Online()
{
	return online;
}

TscRetval TSM::Set(TscConfigType type, const string &json, bool async/* = false*/)
{
	TscHead head(type, false, 0, async);
	auto FillMsg = [&json](zmsg_t *sndmsg){
		zmsg_addmem(sndmsg, json.data(), json.size());
	};
	return Request(req_encrypt, head, FillMsg);
}

TscRetval TSM::Get(TscConfigType type, const unsigned short id/* = 0*/)
{
	TscHead head(type, true, id);
	return Request(req_encrypt, head);
}

TscRetval TSM::WholeSet(const map<TscConfigType, string> &conf)
{
	TscHead head(TSC_TYPE_WHOLE, false);
	auto FillMsg = [&conf](zmsg_t *sndmsg){
		auto Fill = [&sndmsg, &conf](TscConfigType type){
			auto it = conf.find(type);
			if (it != conf.end())
				zmsg_addmem(sndmsg, it->second.data(), it->second.size());
			else
				zmsg_addmem(sndmsg, nullptr, 0);
		};

		Fill(TSC_TYPE_UNIT);			//第一帧为单元
		Fill(TSC_TYPE_CHANNEL);			//第二帧为通道配置
		Fill(TSC_TYPE_PHASE);			//第三帧为相位配置
		Fill(TSC_TYPE_SCHEME);			//第四帧为方案配置
		Fill(TSC_TYPE_TIMEINTERVAL);	//第五帧为时段配置
		Fill(TSC_TYPE_SCHEDULE);		//第六帧为调度配置
		Fill(TSC_TYPE_VEHDETECTOR);		//第七帧为车辆检测器配置
		Fill(TSC_TYPE_PEDDETECTOR);		//第八帧为行人检测器配置
		Fill(TSC_TYPE_PRIOR);			//第九帧为优先配置
		Fill(TSC_TYPE_COUNTDOWN);		//第十帧为倒计时牌配置
	};
	return Request(req_encrypt, head, FillMsg);
}

TscRetval TSM::WholeGet(map<TscConfigType, string> &conf)
{
	TscHead head(TSC_TYPE_WHOLE);
	auto DealMsg = [&conf, this](zmsg_t *&rcvmsg, bool upload)->TscRetval{
		TscRetval ret;
		zframe_t *frame = zmsg_first(rcvmsg);
		unsigned int *flag = (unsigned int *)(zframe_data(frame));
		if (flag == nullptr || *flag == TSC_RET_FAIL)
		{
			ret.succ = false;
			frame = zmsg_next(rcvmsg);
			if (frame != nullptr && zframe_size(frame) > 0)
				ret.msg.assign((char *)zframe_data(frame), zframe_size(frame));
			return ret;
		}

		auto Write = [&conf, &rcvmsg](TscConfigType type){
			zframe_t *frame = zmsg_next(rcvmsg);
			if (frame != nullptr && zframe_size(frame) > 0)
				conf[type].assign((char *)zframe_data(frame), zframe_size(frame));
		};
		Write(TSC_TYPE_UNIT);			//第一帧为单元
		Write(TSC_TYPE_CHANNEL);		//第二帧为通道配置
		Write(TSC_TYPE_PHASE);			//第三帧为相位配置
		Write(TSC_TYPE_SCHEME);			//第四帧为方案配置
		Write(TSC_TYPE_TIMEINTERVAL);	//第五帧为时段配置
		Write(TSC_TYPE_SCHEDULE);		//第六帧为调度配置
		Write(TSC_TYPE_VEHDETECTOR);	//第七帧为车辆检测器配置
		Write(TSC_TYPE_PEDDETECTOR);	//第八帧为行人检测器配置
		Write(TSC_TYPE_PRIOR);			//第九帧为优先配置
		Write(TSC_TYPE_COUNTDOWN);		//第十帧为倒计时牌配置

		zmsg_destroy(&rcvmsg);
		return ret;
	};
	conf.clear();
	return Request(req_encrypt, head, nullptr, DealMsg);
}

void TSM::ClearLog(const string &json)
{
	Set(TSC_TYPE_LOG, json);
}

TscRetval TSM::GetLog(const string &json)
{
	TscHead head(TSC_TYPE_LOG, true);
	auto FillMsg = [&json](zmsg_t *sndmsg){
		zmsg_addmem(sndmsg, json.data(), json.size());
	};
	return Request(req_encrypt, head, FillMsg);
}

TscRetval TSM::Update(const char *path)
{
	TscRetval ret;
	if (path == nullptr)
	{
		ret.Err("the path is null!");
		return ret;
	}

	hik::file file(path);
	size_t size = file.Size();
	if (size == 0)
	{
		ret.Err("the size of file[%s] is 0!", path);
		return ret;
	}

	TscHead head(TSC_TYPE_UPDATE, false);
	auto FillMsg = [&file, &size, &path](zmsg_t *sndmsg){
		zframe_t *frame = zframe_new(nullptr, size);
		if (frame == nullptr)
			return;
		//填充升级包名字
		string arg = path;
    	size_t pos = arg.find_last_of(R"(\/)");
    	string name = arg.substr(pos == string::npos ? 0 : (pos + 1));
		zmsg_addstr(sndmsg, name.data());	//文件名
		//填充升级包
		unsigned char *data = zframe_data(frame);
		file.Read(data, size);
		unsigned int checksum = 0;
		for (size_t i = 0; i < size; i++)
			checksum += data[i];
		zmsg_append(sndmsg, &frame);	//把升级包的frame追加到zmsg
		zmsg_addmem(sndmsg, &checksum, sizeof(checksum));	//填充升级包的校验值
	};
	return Request(req_noenrypt, head, FillMsg);
}
