#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

#include <iostream>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/route.h>
#include <unistd.h>
#endif

#ifdef __WIN32__
#include <WinSock2.h>
#endif

#include "common.h"
#include "HikConfig.h"
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "tsc.h"

#include "config.h"
#include "custom.h"
#include "desc.h"
#include "hikconf.h"
#include "countdown.h"
#include "hiktsc.h"
#include "thread.h"
#include "its.h"
#include "lock.h"




using HikThread::Thread;
using HikLock::RWlock;
using HikIts::Its;

using namespace std;
/*
//class Communication;
class Thread;
class Its;
class Hiktsc;
class Configfile;
class Customfile;
class Descfile;
class Hikconf;
class CountDown;
*/

enum socket_type
{
    TCP = 1,
    UDP = 2,
};

class SocketConnect
{
public:
    struct sockaddr_in fromAddr;
    int socketFd;
    unsigned short m_port;
#ifdef __WIN32__
    int fromLen;
#else
    socklen_t fromLen;
#endif



    SocketConnect(unsigned short port);
    SocketConnect();
    ~SocketConnect();
    void SetSocketPort(unsigned short port);
    int CreateSocket(char *ieth_name, int type);
    int Listen();
    int Accept();
    int SetSocketOpt(int sockfd, int level, int optname, const char* optval, int optlen);
    int GetSocketOpt(int sockfd, int level, int optname, char* optval, int *optlen);
    //udp send and recv data
    int RecvData(void* udp_buf, unsigned int size);
    int SendData(void* udp_buf, unsigned int size);
    //tcp send and recv data
    int Recv(int sockfd, char* buf, unsigned int size);
    int Send(int sockfd, char* buf, unsigned int size);
    void CloseSocket(int sockfd);

#if defined(__linux__)
    in_addr_t GetNetworkCardIp(char *eth);
    bool SetNetwork(char *name, UInt32 ip, UInt32 netmask, UInt32 gateway);
#endif

private:

protected:


};





class HikCommunication: public Thread
{

    public:
    HikCommunication(unsigned short port, Its* its, Hiktsc* hik_tsc, Configfile& config, Customfile& custom, Descfile& desc,
    Hikconf& hikconf /*CountDown& countdownMsgRealtimeVolume& realtimevol, UINT8* redcurrent*/);
    ~HikCommunication();
	
    //void* CommunicationModule(void *arg);
    void run(void* arg);//communication module thread: CommunicationModule()

    private:
    
    //int CreateCommunicationSocket(char *ieth_name, unsigned short port);
    void SendSuccessMsg();
    void SendFailureMsg();
    //unsigned long RecvUdpInfo();
    //void CloseSocket(int sockfd);
	
#if defined(__linux__)
    void DownloadIpAddress();
    void UploadIpAddress();
#endif
    void UploadVersionInfo();
    void UpAndDownLoadDeal();
    void UploadChannelCountdown();
    void UploadChannelStatus();
    void UploadPhaseStatus();
    void UploadFollowPhaseStatus();
    void UploadSyncStatus();
    void UpdateLocalCfg(int type);
	void UploadCountdownFeedback(STRU_UDP_INFO* udp_info);
	
    /*
    class SaveDB: public Thread
    {
    public:
        SaveDB(HikCommunication& hikc): hikcom(hikc) {}
        ~SaveDB() {}
        void run(void* arg)
        {
            hikcom.customParam.SaveCustomUpdate();
            hikcom.descParam.SaveDescUpdate();
            hikcom.configParam.SaveConfigUpdate();
        }

    protected:

    private:
        HikCommunication &hikcom;

    };

    friend class SaveDB;
    */
    struct UDP_INFO udp_info;

	RWlock rw_lock;

	Its *gIts;
	Hiktsc* gHiktsc;
    Configfile& configParam;
    Customfile& customParam;
    Descfile& descParam;
    Hikconf& hikconfParam;
    SocketConnect sockconnect;
	//CountDown &countdowncfgParam;
	//MsgRealtimeVolume &gStructMsgRealtimeVolume;
	//UInt8 *gRedCurrentValue;//gRedCurrentValue[32]
	
};


#endif
