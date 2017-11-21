#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <ctime>
#include "json/json.h"
#include "communication.h"
#include "singleton.h"
#include "tsc.h"
#include "thread.h"
#include "log.h"

#define TSC_NOENCRYPT_REP_PORT 	30000
#define TSC_ENCRYPT_REP_PORT 	30001
#define	TSC_PUB_PORT			30002
#define SDK_ROUTER_PORT			30003
#define	KEY_LEN					64
#define	MAX_CONNECT_SDK_NUMBER	5
#define HEARTBEAT_INTERVAL		2000
#define HEARTBEAT_TIMEOUT		10000
#define MAX_BUFF_MSG_NUM		30
#define UPDATEPKG_PATH			"/home/"

using namespace std;

Communication::Communication() : tsc(Singleton<Tsc>::GetInstance())
{
	rep_noencrypt = nullptr;
	rep_encrypt = nullptr;
	loop = nullptr;
	auth = nullptr;
	store_cert = nullptr;
	pub = nullptr;
}

void Communication::Run()
{
	zcert_t *server_cert = CertstoreInit();

	auto ZsockInit = [&server_cert](zsock_t* &sock, int type, int port, bool encrypt){
		// establish the no encryption socket
		sock = zsock_new(type);
		if (sock == nullptr)
			cout << "The no encrypt " << ((type == ZMQ_REP)?"rep":"pub") << " socket init failed!" << endl;
		// encrypt the socket
		if (encrypt)
		{
			zcert_apply(server_cert, sock);
			zsock_set_curve_server(sock, 1);
		}
		
		//bind the ip address and port
		int rc = zsock_bind(sock, "tcp://*:%d", port);
		if (rc != port)
			cout << "The encrypt " << ((type == ZMQ_REP)?"rep":"pub") << " socket bind failed!" << endl;
	};
	ZsockInit(rep_noencrypt, ZMQ_REP, TSC_NOENCRYPT_REP_PORT, false);
	ZsockInit(pub, ZMQ_PUB, TSC_PUB_PORT, false);
	zsock_set_sndhwm(pub, MAX_BUFF_MSG_NUM);

	ZsockInit(rep_encrypt, ZMQ_REP, TSC_ENCRYPT_REP_PORT, true);

	zcert_destroy(&server_cert);

	// zlooper initalization
	loop = zloop_new();
	if (loop == nullptr)
		cout << "The loop init failed" << endl;
#ifdef DEBUG
	zloop_set_verbose(loop, true);
#endif
	//Assert to zlooper
	LoopInsert(rep_noencrypt, ParseMsg, this);
	LoopInsert(rep_encrypt, ParseMsg, this);

	//zpoller_start for message received
	zloop_set_nonstop(loop, true);
	zloop_start(loop);
}

Communication::~Communication()
{
	if (rep_noencrypt != nullptr)
		zsock_destroy(&rep_noencrypt);
	if (rep_encrypt != nullptr)
		zsock_destroy(&rep_encrypt);
	if (pub != nullptr)
		zsock_destroy(&pub);
	if (store_cert != nullptr)
		zcert_destroy(&store_cert);
	if (auth != nullptr)
		zactor_destroy(&auth);
	if (loop != nullptr)
		zloop_destroy(&loop);
}

void Communication::LoopInsert(zsock_t* sock, zloop_reader_fn handler, void *arg)
{
	int rc = zloop_reader(loop, sock, handler, arg);
	if (rc != 0)
		cout << "zsock insert to the loop failed!" << endl;
	zloop_reader_set_tolerant(loop, sock);
}

zcert_t *Communication::CertstoreInit()
{
	// establish the random PK & SK
	zcert_t *server_cert = zcert_new();
	if (server_cert == nullptr)
		cout << "The tsc random cert init failed!" << endl;
	public_key = zcert_public_txt(server_cert);

	// stored pk & sk embeded in the program
	byte public_key_bin[32] = { 23, 15, 93, 165, 135, 131, 40, 5, 126, 136, 66, 117, 70, 81, 70, 13, 153, 156,
	                         116, 47, 224, 174, 176, 215, 154, 108, 69, 223, 149, 27, 209, 111};
	byte secret_key_bin[32] = { 97, 168, 71, 170, 208, 26, 10, 28, 130, 176, 96, 218, 183, 108, 38, 129,
	                         97, 129, 34, 170, 221, 23, 68, 78, 27, 219, 90, 168, 170, 197, 102, 89};

	zcert_t *_store_cert = zcert_new_from(public_key_bin, secret_key_bin);
	if (_store_cert == nullptr)
		cout << "The tsc stored cert init failed!" << endl;

	store_cert = zcert_dup(_store_cert);
	if (store_cert == nullptr)
		cout << "The tsc stored cert duplicate failed!" << endl;

	// establish the certstore in memory with the stored pk & sk
	zcertstore_t *certstore = zcertstore_new(NULL);
	if (certstore == nullptr)
		cout << "The tsc certstore init failed!" << endl;

	zcertstore_insert(certstore, &_store_cert);

	// new the authentication
	auth = zactor_new(zauth, certstore);
	if (auth == nullptr)
		cout << "The auth engine init failed!" << endl;
#ifdef DEBUG
	zstr_send(auth, "VERBOSE");
	zsock_wait(auth);
#endif
	return server_cert;
}

int Communication::ParseMsg(zloop_t *loop, zsock_t *reader, void *arg)
{
	Communication *obj = static_cast<Communication *>(arg);
	TscRetval ret;
	TscHead head;

	zmsg_t *msg = zmsg_recv(reader);
	if (msg == nullptr)//need to specify the error
		cout << "The msg receive failed!" << endl;
	
	if (zmsg_is(msg))
	{
		obj->tsc.HeadParse(zmsg_first(msg), head);

		if (head.type == TSC_TYPE_PUBKEY)
		{
			zmsg_t* sndmsg = zmsg_new();
			zmsg_addstr(sndmsg, obj->public_key.data());
			zmsg_send(&sndmsg, reader);
			zmsg_destroy(&msg);
			return 0;
		}
		else if (head.type == TSC_TYPE_LOGIN)
		{	//if Login successful, to create the dealer socket
			zframe_t *second = zmsg_next(msg);
			zframe_t *third = zmsg_next(msg);
			ret = obj->Login(second, third, head.srcip, head.src);
		}
		else if (head.type == TSC_TYPE_WHOLE)
		{
			if (head.upload)
			{
				zmsg_t* sndmsg = zmsg_new();
				obj->tsc.rwl.r_lock();
				zmsg_addstr(sndmsg, obj->tsc.unit.Json().msg.data());
				zmsg_addstr(sndmsg, obj->tsc.channel.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.phase.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.scheme.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.schedule.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.timeinterval.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.vehDetector.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.pedDetector.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.prior.Json(0).msg.data());
				zmsg_addstr(sndmsg, obj->tsc.countdown.Json(0).msg.data());
				obj->tsc.rwl.r_unlock();
				zmsg_send(&sndmsg, reader);
				zmsg_destroy(&msg);
			}
			else
			{
				if (zmsg_size(msg) != 11)
					ret.Err("The whole configuration frame number isn't correct");
				else
				{
					ret = obj->tsc.WholeParse(msg);
					ret += obj->tsc.WholeCheck();
					obj->tsc.UpdateWholeUsr(ret);
				}
			}
		}
		else if (head.type == TSC_TYPE_UPDATE)
		{
			ret = obj->UpdatepkgSave(msg);
		}
		else
		{
			ret = obj->tsc.ParseSecFrame(head, zmsg_next(msg));
		}
	}
#if 0
	string tmp = "Operation Sourc:[" + head.src + "] If Upload:[" + (head.upload?"true":"false") + "] Operation Type:[" 
				+ std::to_string(head.type) + "] If Operation Succeed:[" + (ret.succ?"true":"false") + "] Operation Info:[" + ret.msg.data() + "]";
	obj->tsc.log.Write(tmp);
#endif
	//reply the message via rep_encrypt
	obj->Reply(ret, reader);
	zmsg_destroy(&msg);
	return 0;
}

TscRetval Communication::UpdatepkgSave(zmsg_t *self)
{
	TscRetval ret;
	zframe_t* fnm = zmsg_next(self);
	zframe_t* pkg = zmsg_next(self);
	zframe_t* sum = zmsg_next(self);

	const unsigned char *data = (const unsigned char *)zframe_data(pkg);
	unsigned int checksum = 0;
	for (size_t i = 0; i < zframe_size(pkg); i++)
		checksum += data[i];

	unsigned int sumrecv = *(unsigned int*)zframe_data(sum);
	std::string filename((const char*)zframe_data(fnm), zframe_size(fnm));
	filename = UPDATEPKG_PATH + filename;

	if (sumrecv != checksum)
	    ret.Err("The file is incorrect! Transform failed!");
	else
	{
		std::ofstream outfile(filename ,std::ofstream::binary);
	    outfile.write((const char*)zframe_data(pkg), zframe_size(pkg));
	    outfile.close();
	}
    return ret;
}

TscRetval Communication::Login(zframe_t *frame, zframe_t *frameNext, const string &srcip, const string &src)
{
	TscRetval ret;
	if (zframe_size(frame) == 0 /*|| sizeNext != 40*/)
	{
		ret.Err("the size of password frame is 0!");
		return ret;
	}
	string recvPasswd((char *)zframe_data(frame), zframe_size(frame));
	if (recvPasswd == "hiklinux" || recvPasswd == tsc.password)
	{	
		if (src.compare("platform") == 0 )
		{
			if (zframe_size(frameNext) == 40)
			{
				string serverpk((char *)zframe_data(frameNext), zframe_size(frameNext));
				auto opearte = [&serverpk](SDKInfo &info)->bool{
					bool ret = (info.pk.compare(serverpk) == 0);
						return ret;
				};
				auto it = find_if(sdkinfo.begin(), sdkinfo.end(), opearte);
				if (it == sdkinfo.end())
				{
					if (sdkinfo.size() < MAX_CONNECT_SDK_NUMBER)
					{
						sdkinfo.emplace_back(srcip, serverpk, nullptr, nullptr);
						//zclock_sleep(500); //在同一IP起两个SDK的时候，第二个dealer如果先于REQ连接，则会因为
											 //第一次连接时候，底层存储的连接信息导致第一个消息失效（ZMQ底层机制）
											 //从而导致会使加密连接报错误。此处加上sleep等第二个SDK的req先连接
											 //则无报错，但会影响效率，报错并不影响使用，故注释掉。
						DealerInit(sdkinfo.back());
						//ret.msg = R"({"software":"2017-10.09-Rev-alpha","hardware":"TSC300"})";
						Log::Debug("ip %s login!!! serverpk: %s", srcip.c_str(), serverpk.c_str());
					}
					else
						ret.Err("The SDK is reached the max num!");
				}
				else
				{
					Log::Debug("[%s] connect again!", srcip.c_str());
				}
			}
			else
			{
				ret.Err("The SDK router pk isn't size of 40!");
			}
		}
		ret.msg = R"({"software":"2017-10.09-Rev-alpha","hardware":"TSC300"})";	
	}
	else
		ret.Err("The password isn't correct!");

	return ret;
}

void Communication::DealerInit(SDKInfo &sdkinfo)
{
	//establish the encryption dealer socket
	sdkinfo.dealer = zsock_new(ZMQ_DEALER);
	if (sdkinfo.dealer == nullptr)
		cout << "The dealer socket init failed!" << endl;

	//Configuration for dealer socket
	zsock_set_identity(sdkinfo.dealer, public_key.data());
    zsock_set_heartbeat_ivl(sdkinfo.dealer, HEARTBEAT_INTERVAL);	//heartbeat send interval ms
    zsock_set_heartbeat_timeout(sdkinfo.dealer, HEARTBEAT_TIMEOUT);	//heartbeat timeout ms

	// encrypt the dealer socket
	zcert_apply(store_cert, sdkinfo.dealer);
#ifdef DEBUG	
	std::cout << "SDKinfo pk" <<sdkinfo.pk.c_str()<<std::endl;
#endif
	zsock_set_curve_serverkey(sdkinfo.dealer, sdkinfo.pk.c_str());

	int rc = zsock_connect(sdkinfo.dealer, "tcp://%s:%d", sdkinfo.ip.c_str(), SDK_ROUTER_PORT);
	if (rc != 0)
		cout << "The dealer socket connect failed!" << endl;

	//establish zmonitor for each dealer to monitor the status of dealer
	sdkinfo.monitor = zactor_new(zmonitor, sdkinfo.dealer);
	if (sdkinfo.monitor == nullptr)
		cout << "The dealer's monitor socket init failed!" << endl;
#ifdef DEBUG
	zstr_sendx(sdkinfo.monitor, "VERBOSE", NULL);
#endif
	zstr_sendx(sdkinfo.monitor, "LISTEN","BIND_FAILED", "ACCEPT_FAILED",
	             "CLOSE_FAILED", "DISCONNECTED", NULL);

	LoopInsert(sdkinfo.dealer, ParseMsg, this);
	LoopInsert(zactor_sock(sdkinfo.monitor), DealerRemove, this);

	zstr_send(sdkinfo.monitor, "START");
	zsock_wait(sdkinfo.monitor);
}

int Communication::DealerRemove(zloop_t *loop, zsock_t *reader, void *arg)
{
	Communication *obj = static_cast<Communication *>(arg);
	auto opearte = [&reader, &loop](SDKInfo &info)->bool{
		bool ret = (zactor_sock(info.monitor) == reader);
		if (ret)
		{
			zloop_reader_end (loop, zactor_sock(info.monitor));
			zloop_reader_end (loop, info.dealer);
			zsock_destroy(&(info.dealer));
			zactor_destroy(&(info.monitor));
			Log::Debug("ip %s is offline!", info.ip.c_str());
		}
		return ret;
	};
	auto it = find_if(obj->sdkinfo.begin(), obj->sdkinfo.end(), opearte);
	obj->sdkinfo.erase(it/*, obj->sdkinfo.end()*/);
	return 0;
}

void Communication::Reply(const TscRetval &ret, zsock_t * zsock)
{
	zmsg_t * msg = zmsg_new();
	if (msg == nullptr)
		cout << "The rely msg init failed!" << endl;
	int rc = 0;

	if (ret.succ)
	{
		rc = zmsg_addstr(msg, "succ");
		if (rc != 0)
		cout << "The first frame of reply msg inserted failed!" << endl;
	}
	else
	{
		rc = zmsg_addstr(msg, "fail");
		if (rc != 0)
		cout << "The first frame of reply msg inserted failed!" << endl;
	}

	if (!ret.msg.empty())
		rc = zmsg_addmem(msg, ret.msg.c_str(), ret.msg.size());
#ifdef DEBUG
	cout << "reply[STATUS]" << ret.succ << endl;
	cout << "reply[START]" << ret.msg  << "[END]"<<endl;
#endif
	if (rc != 0)
		cout << "The second frame of reply msg inserted failed!" << endl;

	rc = zmsg_send(&msg, zsock);
	if (rc != 0)
	{
		cout << "The reply msg send failed!" << endl;
		zmsg_destroy(&msg);
	}
}

void Communication::AlarmSend(const string &alarm)
{
	zmsg_t *msg = zmsg_new();
	assert (msg);

	int rc = zmsg_addstrf(msg, "%d", TSC_RT_ALARM);
	assert (rc == 0);

	Json::Value root;
	Json::FastWriter writer;

	root[TSC_ALARM_TIME] = Log::CurrentTime();
	root[TSC_ALARM_MSG] = alarm;

	std::string ret = writer.write(root);

	rc = zmsg_addstr (msg, ret.c_str());
	assert (rc == 0);

	rc = zmsg_send(&msg, pub);
	assert (rc == 0);

	//zmsg_destroy(&msg);
}

void Communication::SendRealtimeStat(const string &data)
{
	TscRetval ret;
	zmsg_t * msg = zmsg_new();
	if (msg == nullptr)
		ret.Err("The pub msg init failed!");
	int rc = 0;
	
	TscRealtimeType st(TSC_RT_STATUS);
	rc = zmsg_addmem(msg, &st, sizeof(TscRealtimeType));
	if (rc != 0)
		ret.Err("The first frame of pub msg inserted failed!");
	
	rc = zmsg_addmem(msg, data.c_str(), data.size());
	if (rc != 0)
		ret.Err("The second frame of pub msg inserted failed!");

	rc = zmsg_send(&msg, pub);
	if (rc != 0)
	{
		ret.Err("The pub msg send failed!");
		zmsg_destroy(&msg);
	}
	#if 0
	INFO("[%s]",data.c_str());
	#endif
	//Log::Debug("Send realtime ******************");
	return;
}
