
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>



#ifndef _SVID_SOURCE
#define _SVID_SOURCE	//????glibc2?·Ú,????stime()?????????????
#endif


#include "communication.h"
//#include "hiktsc.h"


SocketConnect::SocketConnect()
{
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    fromLen = sizeof(fromAddr);

#ifdef __WIN32__
    WSAData ws;
    if (WSAStartup(MAKEWORD(1,1),&ws))
    {
        cout<<"WSAStartup failed"<<WSAGetLastError()<<endl;
    }
#endif

}

SocketConnect::SocketConnect(unsigned short port)
{
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    fromLen = sizeof(fromAddr);
    m_port = port;
#ifdef __WIN32__
    WSAData ws;
    if (WSAStartup(MAKEWORD(1,1),&ws))
    {
        cout<<"WSAStartup failed"<<WSAGetLastError()<<endl;
    }
#endif
}

SocketConnect::~SocketConnect()
{
#ifdef __WIN32__
        WSACleanup();
#endif
}

void SocketConnect::SetSocketPort(unsigned short port)
{
    m_port = port;
}

int SocketConnect::SetSocketOpt(int sockfd, int level, int optname, const char* optval, int optlen)
{
   return setsockopt(sockfd, level, optname, optval, optlen);
}

int SocketConnect::GetSocketOpt(int sockfd, int level, int optname, char* optval, int *optlen)
{
#ifdef __linux__
    return getsockopt(sockfd, level, optname, optval, (socklen_t*)optlen);
#else
    return getsockopt(sockfd, level, optname, optval, optlen);
#endif
}

int SocketConnect::CreateSocket(char *ieth_name, int type)
{
    struct sockaddr_in addr;


    addr.sin_family = PF_INET;
    #ifdef __linux__
    addr.sin_addr = {(ieth_name == NULL) ? INADDR_ANY : GetNetworkCardIp(ieth_name)};
    #endif
    #ifdef __WIN32__
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    #endif
    addr.sin_port = htons(m_port);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    int len = sizeof(struct sockaddr);
    int opt = 1;
    if ((socketFd = socket(PF_INET, (type == TCP) ? SOCK_STREAM : SOCK_DGRAM, (type == TCP) ? IPPROTO_TCP : IPPROTO_UDP)) == -1)
    {
        //ERR("create socket fail, error info: %s\n", strerror(errno));
        cout<<"create socket fail"<<endl;
        return -1;
    }
    SetSocketOpt(socketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(int));
    if (bind(socketFd, (struct sockaddr*)(&addr), len) == -1)
    {
        //ERR("bind socket fail, error info:%s\n", strerror(errno));
        cout<<"bind socket fail"<<endl;
        CloseSocket(socketFd);
        return -1;
    }
    return socketFd;
}

int SocketConnect::Listen()
{
    int ret = 0;

    ret = listen(socketFd, 5);
    if (ret < 0)
    {
        cout << "listen() faild!!! "<< endl;
        CloseSocket(socketFd);
    }
    return ret;
}

int SocketConnect::Accept()
{
    int conFd = -1;
    conFd = accept(socketFd, (struct sockaddr*)(&fromAddr), &fromLen);
    if (conFd == -1)
    {
        cout << "accept connect failed !!!" << endl;
        CloseSocket(socketFd);
    }
    return conFd;
}

void SocketConnect::CloseSocket(int sockfd)
{
#if defined(__linux__)
    close(sockfd);
#else
    closesocket(sockfd);

#endif

}

int SocketConnect::RecvData(void* udp_buf, unsigned int size)
{
    ssize_t result;

    memset(udp_buf,0,size);
    result = recvfrom(socketFd, (char*)(udp_buf), size, 0, (struct sockaddr *)&fromAddr, &fromLen);
    if(-1 == result)
    {
        //ERR("############===>  Failed Error   %s\n",strerror(errno));
        cout<<"############===>  Failed Error "<<strerror(errno)<<endl;
    }
    return result;
}

int SocketConnect::SendData(void* udp_buf, unsigned int size)
{
   return sendto(socketFd, (char*)udp_buf, size, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

int SocketConnect::Recv(int sockfd, char* buf, unsigned int size)
{
   int result = 0;
   int len = 0;
   while(len < size)
   {
       result = recv(sockfd, buf, size, 0);
       if (result < 0)
       {
           cout << "recv socket data failed !!!" <<  endl;
           break;
       }
       else if (result == size)
       {
           len += result;
           buf += result;
       }
       else if (result == 0)//remote close
           break;
   }

   return result;
}

int SocketConnect::Send(int sockfd, char* buf, unsigned int size)
{
    int result = 0;
    int len = 0;
    while(size > 0)
    {
        result = send(sockfd, buf, size, 0);
        if (result < 0)
        {
            if(errno == EAGAIN)
            {
                usleep(1000);
                continue;
            }
            else
            {
                cout << "send data failed !!!" << endl;
                return result;
            }
        }
        else if (result > 0)
        {
            len += result;
            buf += result;
            size -= result;
        }
    }
    return len;
}




HikCommunication::HikCommunication(unsigned short port, Its* its, Hiktsc* hik_tsc, Configfile& config, Customfile& custom, Descfile& desc,
    Hikconf& hikconf) : gIts(its), gHiktsc(hik_tsc), configParam(config), customParam(custom),
    descParam(desc), hikconfParam(hikconf)
{
    memset((char*)&udp_info, 0, sizeof(udp_info));
    sockconnect.SetSocketPort(port);
}

HikCommunication::~HikCommunication()
{
}



void HikCommunication::SendSuccessMsg()
{
    ssize_t result = 0;

    udp_info.iValue[0] = 0;
    result = sockconnect.SendData(&udp_info, 12);//sendto(socketFd, (char*)(&udp_info), 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    if ( result == -1 )
    {
        //DBG("sendto udp info error!!!\n");
        cout<<"sendto udp info error!!!"<<endl;
    }

}

void HikCommunication::SendFailureMsg()
{
    ssize_t result = 0;

    udp_info.iValue[0] = 1;
    result = sockconnect.SendData(&udp_info, 12);//sendto(socketFd, (char*)(&udp_info), 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    if ( result == -1 )
    {
        //DBG("sendto udp info error!!!\n");
        cout<<"sendto udp info error!!!"<<endl;
    }

}


typedef struct
{
    UInt32 startline;
    UInt32 linenum;
    int sockfd;
    struct sockaddr addr;
} UploadFaultLogNetArg; //????????????????????
/*
void HikCommunication::UploadFaultLog(void *arg, void *data, int datasize)
{
    UploadFaultLogNetArg *netArg = (UploadFaultLogNetArg *)arg;
    char buf[2048] = {0};
    struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)buf;

    response->unExtraParamHead = 0x6e6e;        //???????
    response->unExtraParamID = 0xc0;            //???????ID
    response->unExtraParamValue = 0x15b;        //??????????
    response->unExtraParamFirst = netArg->startline;
    response->unExtraParamTotal = datasize / sizeof(FaultLogInfo);
    response->unExtraDataLen = datasize;
    memcpy(response->data, data, datasize);
    sendto(netArg->sockfd, response, sizeof(struct STRU_Extra_Param_Response) + datasize, 0, &netArg->addr, sizeof(struct sockaddr));
}
*/

#if defined(__linux__)
in_addr_t SocketConnect::GetNetworkCardIp(char *eth)
{
    struct ifreq ifreqs[8];
    struct ifconf conf;
    int i, ifc_num;
    struct ifreq *ifr = ifreqs;
    struct in_addr addr = {0};
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    if (sockfd == -1)
    {
        //ERR("create socket fail, error info: %s\n", strerror(errno));
        return addr.s_addr;
    }
    memset(ifr, 0, sizeof(ifreqs));
    if (eth != NULL)
    {
        strcpy(ifr->ifr_name, eth);
        if (ioctl(sockfd, SIOCGIFADDR, ifr) != 0)
            return addr.s_addr;
        addr = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr;
        //INFO("Use network card %s, ip: %s\n", ifr->ifr_name, inet_ntoa(addr));
        return addr.s_addr;
    }
    //eth为NULL时，获取正在运行的网卡的ip地址
    conf.ifc_len = sizeof(ifreqs);
    conf.ifc_req = ifreqs;
    if (ioctl(sockfd, SIOCGIFCONF, &conf))
    {
        //ERR("get conf fail, error info: %s\n", strerror(errno));
        return addr.s_addr;
    }

    ifc_num = conf.ifc_len / sizeof(struct ifreq);
    for (i = 0; i < ifc_num; i++, ifr++)
    {
        if ((strncmp(ifr->ifr_name, "lo", 2) == 0)
            || (strncmp(ifr->ifr_name, "can", 3) == 0))
            continue;
        ioctl(sockfd, SIOCGIFFLAGS, ifr);
        if (ifr->ifr_flags & IFF_RUNNING)
        {
            addr = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr;
            //INFO("Use network card %s, ip: %s\n", ifr->ifr_name, inet_ntoa(addr));
            break;
        }
    }
    return addr.s_addr;
}

bool SocketConnect::SetNetwork(char *name, UInt32 ip, UInt32 netmask, UInt32 gateway)
{
    struct ifreq ifr;
    struct rtentry rt;
    struct sockaddr_in gateway_addr;
    struct sockaddr_in *addr = (struct sockaddr_in *)&(ifr.ifr_addr);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        ERR("socket called fail, error info: %s\n", strerror(errno));
        return FALSE;
    }
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, name);
    //设置ip
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = ip;
    if (ioctl(sockfd, SIOCSIFADDR, &ifr) == -1)
    {
        ERR("set %s ip %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
        close(sockfd);
        return FALSE;
    }
    INFO("set %s ip %s successful!", name, inet_ntoa(addr->sin_addr));
    //设置netmask
    addr->sin_addr.s_addr = netmask;
    if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1)
    {
        ERR("set %s netmask %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
        close(sockfd);
        return FALSE;
    }
    INFO("set %s netmask %s successful!", name, inet_ntoa(addr->sin_addr));
    //设置网关
    addr->sin_addr.s_addr = gateway;
    memset(&rt, 0, sizeof(rt));
    memset(&gateway_addr, 0, sizeof(struct sockaddr_in));
    rt.rt_dev = name;
    gateway_addr.sin_family = PF_INET;
    //inet_aton("0.0.0.0",&gateway_addr.sin_addr);
    memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
    memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
    ioctl(sockfd, SIOCDELRT, &rt); 	//设置网关之前先删除之前的

    rt.rt_flags = RTF_UP | RTF_GATEWAY;
    gateway_addr.sin_addr.s_addr = gateway;
    memcpy(&rt.rt_gateway, &gateway_addr, sizeof(struct sockaddr_in));
    inet_aton("0.0.0.0", &gateway_addr.sin_addr);
    memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
    memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
    if(ioctl(sockfd, SIOCADDRT, &rt) == -1)		//重新添加新的
    {
        ERR("set %s gateway %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
        close(sockfd);
        return FALSE;
    }
    INFO("set %s gateway %s successful!", name, inet_ntoa(addr->sin_addr));
    close(sockfd);
    return TRUE;
}

void HikCommunication::DownloadIpAddress()
{
    struct STRU_N_IP_ADDRESS ip_info;
    struct in_addr inaddr;
    char *name = NULL;
    char cmd[256] = {0};
    FILE *fp = NULL;
    int line = 0;

    if(udp_info.iType == 0x15d)
        name = "eth1";
    else if(udp_info.iType == 0x15f)
        name = "eth0";
    else if(udp_info.iType == 0x161)
        name = "wlan0";
    memset(&ip_info,0,sizeof(ip_info));
    inaddr.s_addr = (unsigned long)udp_info.iValue[0];
    strcpy(ip_info.address, inet_ntoa(inaddr));
    inaddr.s_addr = (unsigned long)udp_info.iValue[1];
    strcpy(ip_info.subnetMask, inet_ntoa(inaddr));
    inaddr.s_addr = (unsigned long)udp_info.iValue[2];
    strcpy(ip_info.gateway, inet_ntoa(inaddr));
    sockconnect.SetNetwork(name, udp_info.iValue[0], udp_info.iValue[1], udp_info.iValue[2]);
    SendSuccessMsg();

    sprintf(cmd, "grep 'iface %s' /etc/network/interfaces -n | cut -d ':' -f 1", name);
    sprintf(cmd, "grep 'iface %s' /etc/network/interfaces -A4 -n | grep address | cut -d '-' -f 1", name);
    fp = popen(cmd, "r");
    if (fp != NULL)
    {	//????????????????address????ï¿½ï¿½??/etc/network/interfaces?????
        fscanf(fp, "%d", &line);
        pclose(fp);
        if (line > 0)
        {	//???sed???????????ï¿½I???ï¿½I??????????
            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "sed -i '%d,%dc address %s\\\nnetmask %s\\\ngateway %s' /etc/network/interfaces",
                    line, line + 2, ip_info.address, ip_info.subnetMask, ip_info.gateway);
            system(cmd);
        }
    }
}

void HikCommunication::UploadIpAddress()
{
    struct STRU_N_IP_ADDRESS ip_info;
    char *cmd = NULL;
    FILE *fp = NULL;

    memset(&ip_info,0,sizeof(ip_info));
    if(udp_info.iType == 0x15e)
        cmd = "grep 'iface eth1' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
    else if(udp_info.iType == 0x160)
        cmd = "grep 'iface eth0' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
    else if(udp_info.iType == 0x162)
        cmd = "grep 'iface wlan0' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
    fp = popen(cmd, "r");
    if (fp != NULL)
    {	//从/etc/network/interfaces文件中获取IP信息
        fscanf(fp, "address %s\nnetmask %s\ngateway %s\n", ip_info.address, ip_info.subnetMask, ip_info.gateway);
        pclose(fp);
        INFO("ip=%s,netmask=%s,address=%s\n", ip_info.address, ip_info.subnetMask, ip_info.gateway);
        udp_info.iValue[0] = inet_addr(ip_info.address);
        udp_info.iValue[1] = inet_addr(ip_info.subnetMask);
        udp_info.iValue[2] = inet_addr(ip_info.gateway);
    }
    sockconnect.SendData(&udp_info, 20); //sendto(socketFd, &udp_info, 20, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
}
#endif

/*
void HikCommunication::UploadVersionInfo(int socketFd,struct sockaddr_in fromAddr)
{
    DEVICE_VERSION_PARAMS device_version_params;           //?锟斤拷????????
    char *hv = HARDWARE_VERSION_INFO;
    char *sv = SOFTWARE_VERSION_INFO;

    memset(&device_version_params,0,sizeof(device_version_params));
    device_version_params.unExtraParamHead = udp_info.iHead;
    device_version_params.unExtraParamID = udp_info.iType;
    memcpy(device_version_params.hardVersionInfo, hv, strlen(hv));        //??32ï¿½ï¿½??????????
    if (strcmp((char *)device_version_params.hardVersionInfo, "DS-TSC300") == 0)
    {
        if (configParam.config.sSpecialParams.iSignalMachineType == 2)
            strcat((char *)device_version_params.hardVersionInfo, "-22");
        else
            strcat((char *)device_version_params.hardVersionInfo, "-44");
    }
    memcpy(device_version_params.softVersionInfo, sv, strlen(sv));       //??32ï¿½ï¿½???????????
    sendto(socketFd, (char*)&device_version_params, sizeof(device_version_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //?????????????????
}
*/


#define UP_DOWN_LOAD_TIMEOUT	60		//上下载超时时间60s
void HikCommunication::UpAndDownLoadDeal()	//上下载处理
{
    static UInt8 uploadNum = 0;
    static UInt32 downloadFlag = 0;//存放的是开始下载时SDK传送来的具体哪些参数需要保存的flag
    static struct timespec uploadStartTime = {0, 0}, downloadStartTime = {0, 0};
    struct timespec currentTime;
    struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)&udp_info;
    int ret = 12;
    //UploadFaultLogNetArg netArg;

    if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb7)//下载参数开始
    {
        //log_debug("download config begin");
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
        {	//说明有其他客户端正在下载
            udp_info.iValue[0] = 1;
            sockconnect.SendData(&udp_info, 12);//sendto(socketFd, (char*)&udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            return;
        }
        if (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
        {	//说明有其他客户端正在上载
            udp_info.iValue[0] = 2;
            sockconnect.SendData(&udp_info, 12);//sendto(socketFd, (char*)&udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            return;
        }
        downloadFlag = *(UInt32 *)(udp_info.iValue);
        downloadStartTime = currentTime;
        hikconfParam.StoreBegin(downloadFlag);
        udp_info.iValue[0] = 0;
    }
    else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//下载配置完成
    {
        udp_info.iValue[0] = 0;//IsSignalControlparaLegal(gSignalControlpara);
        //log_debug("download config over, check result %d", udp_info.iValue[0]);
        if (downloadFlag > 0 && udp_info.iValue[0] == 0)
        {
            gHiktsc->ItsSetConfig(hikconfParam.gSignalControlpara, sizeof(SignalControllerPara));
            if (BIT(downloadFlag, 16)) //表示是否写flash
            {
                INFO("begin to write hikconfig to database!!!\n");
                hikconfParam.WriteLoaclConfigFile();	//写入本地配置文件里
                INFO("end to write hikconfig to database!!!\n");
            }
            //log_debug("config information update!");
            gIts->ItsWriteFaultLog(LOCAL_CONFIG_UPDATE, 0);
        }
        else
            gHiktsc->ItsGetConfig(hikconfParam.gSignalControlpara, sizeof(SignalControllerPara));
        downloadFlag = 0;
        downloadStartTime.tv_sec = 0;
    }
    else if(udp_info.iHead == 0x6e6e && ((udp_info.iType >= 0xaa && udp_info.iType <= 0xb6)
        || udp_info.iType == 0xdd || udp_info.iType == 0xeeeeeeee))//下载配置信息,0xdd代表新的方案下载,0xeeeeeeee表示web下载
    {
        //log_debug("download config, type = %#x", udp_info.iType);
        hikconfParam.DownloadConfig(udp_info.iType, (void *)udp_info.iValue);//0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 只是为了ctrl+f搜索方便!
    }
    else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xce)	//上载开始
    {
        //log_debug("upload config begin");
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)	//说明有其他客户端正在下载
        {
            udp_info.iValue[0] = 1;
            sockconnect.SendData(&udp_info, 12);//sendto(socketFd, (char*)&udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            return;
        }
        //INFO("DEBUG: upload begin, downloadFlag = %#x, timegap = %d", downloadFlag, currentTime.tv_sec - downloadStartTime.tv_sec);
        //有其他客户端上载已经超时的情况下uploadNum=1,其他时候都加1
        uploadNum = (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) > UP_DOWN_LOAD_TIMEOUT) ? 1 : (uploadNum + 1);
        uploadStartTime = currentTime;
        udp_info.iValue[0] = 0;
    }
    else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xcf)	//上载结束
    {
        //log_debug("upload config over");
        if (uploadNum > 1)
            uploadNum--;
        else if (uploadNum == 1)
        {
            uploadNum = 0;
            uploadStartTime.tv_sec = 0;
        }
    }
    else if (udp_info.iHead == 0x6e6e && udp_info.iType == 0xc0)	//上载配置
    {
        ret = sizeof(struct STRU_Extra_Param_Response);
        if (response->unExtraParamValue == 0x15b)	//上载故障信息
        {
            //INFO("upload fault log, startline = %u, linenum = %u", response->unExtraParamFirst, response->unExtraParamTotal);
            //netArg.startline = response->unExtraParamFirst;
            //netArg.linenum = response->unExtraParamTotal;
            //netArg.sockfd = sockconnect.socketFd;
            //memcpy(&netArg.addr, &sockconnect.fromAddr, sizeof(struct sockaddr));
            //ItsReadFaultLog(response->unExtraParamFirst, response->unExtraParamTotal, &netArg, sizeof(UploadFaultLogNetArg), UploadFaultLog);
            //return;	//故障信息的上载统一都在故障日志模块回复，所以这里直接返回不在此回复
        }
        else if (response->unExtraParamValue == 68 || response->unExtraParamValue == 70
                || response->unExtraParamValue == 78 || response->unExtraParamValue == 125
                || response->unExtraParamValue == 154)
            ret = hikconfParam.DownloadExtendConfig(response);
        else
        {
            //log_debug("upload config, unExtraParamValue = %u", response->unExtraParamValue);
            ret = hikconfParam.UploadConfig(response);
        }
    }
    sockconnect.SendData(&udp_info, ret);//sendto(socketFd, (char*)&udp_info, ret, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

/*
#define MAX_TERMINAL_NUM	(NUM_CHANNEL * 3 + 19)
static inline void ChannelCheckDeal(int nTerminal)
{	//??????????20,21,22???????1?????????23,24,25???????2????????????????
    if (nTerminal < 20 || nTerminal > MAX_TERMINAL_NUM)
        return;
    int channelId = (nTerminal - 20) / 3 + 1;
    int left = (nTerminal - 20) % 3;
    LightStatus status = (left == 0) ? RED : ((left == 1) ? YELLOW : GREEN);

    ItsChannelCheck(channelId, status);
}*/

//发送给触摸屏各个通道的倒计时值和状态
inline void HikCommunication::UploadChannelCountdown()
{
    PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gp = (PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *)&udp_info;

    memset(udp_info.iValue, 0, sizeof(udp_info.iValue));
    gHiktsc->ItsCountDownGet(gp, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
    gp->unExtraParamHead = 0x6e6e;
    gp->unExtraParamID = 0xeeeeeeec;
    sockconnect.SendData(&udp_info, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
    //sendto(socketFd, (char*)&udp_info,sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));

    //INFO("UploadChannelCountdown  ->  %s\n",inet_ntoa(fromAddr.sin_addr));
}


inline void HikCommunication::UploadChannelStatus()
{
    struct STRU_N_ChannelStatusGroup *gp = (struct STRU_N_ChannelStatusGroup *)udp_info.iValue;
    int i, j, datasize = sizeof(*gp) * 4;
    PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

    memset(gp, 0, datasize);
    gHiktsc->ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
    for (i = 0; i < 4; i++)	//总共4组状态
    {
        gp->byChannelStatusGroupNumber = i + 1;
        for (j = 0; j < 8; j++)	//每组包含8个通道的状态，1bit代表一个通道
        {
            switch (countDownParams.ucChannelStatus[i * 8 + j])
            {
                case GREEN_BLINK:
                case GREEN: gp->byChannelStatusGroupGreens |= (1 << j); break;
                case YELLOW_BLINK:
                case YELLOW: gp->byChannelStatusGroupYellows |= (1 << j); break;
                case RED_BLINK:
                case ALLRED:
                case RED: gp->byChannelStatusGroupReds |= (1 << j); break;
            }
        }
        gp++;
    }
    sockconnect.SendData(&udp_info, 8 + datasize);
    //sendto(socketFd, (char*)&udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

inline void HikCommunication::UploadPhaseStatus()
{
    struct STRU_N_PhaseStatusGroup *gp = (struct STRU_N_PhaseStatusGroup *)udp_info.iValue;
    int i, j, datasize = sizeof(*gp) * 2;
    PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

    memset(gp, 0, datasize);
    gHiktsc->ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
    for (i = 0; i < 2; i++)	//总共2组状态
    {
        gp->byPhaseStatusGroupNumber = i + 1;
        for (j = 0; j < 8; j++)	//每组包含8个相位的状态，1bit代表一个相位
        {
            switch (countDownParams.stVehPhaseCountingDown[i * 8 + j][0])
            {
                case GREEN_BLINK:
                case GREEN: gp->byPhaseStatusGroupGreens |= (1 << j); break;
                case YELLOW_BLINK:
                case YELLOW: gp->byPhaseStatusGroupYellows |= (1 << j); break;
                case RED_BLINK:
                case ALLRED:
                case RED: gp->byPhaseStatusGroupReds |= (1 << j); break;
            }
        }
        gp++;
    }
    sockconnect.SendData(&udp_info, 8 + datasize);
    //sendto(socketFd, (char*)&udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

inline void HikCommunication::UploadFollowPhaseStatus()
{
    struct STRU_N_OverlapStatusGroup *gp = (struct STRU_N_OverlapStatusGroup *)udp_info.iValue;
    int i, j, datasize = sizeof(*gp) * 2;
    PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

    memset(gp, 0, datasize);
    gHiktsc->ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
    for (i = 0; i < 2; i++)	//总共2组状态
    {
        gp->byOverlapStatusGroupNumber = i + 1;
        for (j = 0; j < 8; j++)	//每组包含8个跟随相位的状态，1bit代表一个跟随相位
        {
            switch (countDownParams.ucOverlap[i * 8 + j][0])
            {
                case GREEN_BLINK:
                case GREEN: gp->byOverlapStatusGroupGreens |= (1 << j); break;
                case YELLOW_BLINK:
                case YELLOW: gp->byOverlapStatusGroupYellows |= (1 << j); break;
                case RED_BLINK:
                case ALLRED:
                case RED: gp->byOverlapStatusGroupReds |= (1 << j); break;
            }
        }
        gp++;
    }
    sockconnect.SendData(&udp_info, 8 + datasize);
    //sendto(socketFd, (char*)&udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

inline void HikCommunication::UploadSyncStatus()
{
    struct STRU_CoordinateStatus *gp = (struct STRU_CoordinateStatus *)udp_info.iValue;
    int datasize = sizeof(*gp);
    PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

    memset(gp, 0, datasize);
    gHiktsc->ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
    gp->byCoordPatternStatus = countDownParams.ucPlanNo;
    gp->wCoordCycleStatus = countDownParams.ucCurCycleTime - countDownParams.ucCurRunningTime;
    gp->wCoordSyncStatus = countDownParams.ucCurRunningTime;
    gp->byUnitControlStatus = gHiktsc->ItsControlStatusGet();
    sockconnect.SendData(&udp_info, 8 + datasize);
    //sendto(socketFd, (char*)&udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

//????type???ï¿½ï¿½??????,??????????????????????????????????????????????,TODO
inline void HikCommunication::UpdateLocalCfg(int type)
{
    sleep(1);//?????????????????ï¿½ï¿½???????????????????????
    switch(type)
    {
        case 1://????hikconfig.dat
        {
            SignalControllerPara para;
            memset(&para,0,sizeof(para));
            hikconfParam.LoadLocalConfigFile();

            //READ_BIN_CFG_PARAMS(FILE_TSC_CFG_DAT, &para, sizeof(para));
            //if(IsSignalControlparaLegal(&para) == 0)
            {
                gHiktsc->ItsSetConfig(hikconfParam.gSignalControlpara, sizeof(SignalControllerPara));
                //INFO("UpdateLocalCfg hikconfig \n");
            }
            break ;
        }
        case 9://config.dat
        {

            configParam.read_conf_from_database();
            //READ_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));

            //INFO("UpdateLocalCfg config \n");
            break;
        }
        case 3://desc.dat
        {
            descParam.read_desc_from_database();
            //READ_BIN_CFG_PARAMS(FILE_DESC_DAT,&gStructBinfileDesc,sizeof(STRUCT_BINFILE_DESC));

            //INFO("UpdateLocalCfg desc \n");
            break;
        }
        case 4://custom.dat
        {
            customParam.read_custom_from_database();
            //READ_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
            //INFO("UpdateLocalCfg custom \n");
            break;
        }
        /*
        case 5://countdown.dat
        {
            sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
            sqlite3_select_countdown_cfg(pdb, &(countdowncfgParam.g_CountDownCfg));
            sqlite3_close_wrapper(pdb);
            //READ_BIN_CFG_PARAMS(FILE_COUNTDOWN_DAT,&g_CountDownCfg,sizeof(CountDownCfg));
            //INFO("UpdateLocalCfg countdown \n");
            break;
        }*/
        case 6://misc.dat
        {
            configParam.read_misc_from_database();
            //READ_BIN_CFG_PARAMS(FILE_MISC_DAT,&gStructBinfileMisc,sizeof(STRUCT_BINFILE_MISC));
            //INFO("UpdateLocalCfg misc \n");
            break;
        }
        case 2://gbconfig.dat
        {
#if 0
            GbConfig gbCfg;
            memset(&gbCfg,0,sizeof(gbCfg));
            READ_BIN_CFG_PARAMS("/home/gbconfig.dat",&gbCfg,sizeof(gbCfg));
            ItsSetConfig(GB2007, &gbCfg);
#endif
            ///INFO("UpdateLocalCfg gbconfig \n");
            break;
        }
        default:
        {
            break;
        }
    }
}
void HikCommunication::UploadCountdownFeedback(STRU_UDP_INFO* udp_info)
{
    PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

    gHiktsc->ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
    if(customParam.custom.cChannelLockFlag != 1 || customParam.custom.sChannelLockedParams.ucReserved == 1)//reserved==1 ï¿½ï¿½Ê¾ï¿½É±ä³µï¿½ï¿½ï¿½Æ¿ï¿½ï¿½ï¿½
        countDownParams.ucChannelLockStatus = 0;
    if(countDownParams.ucPlanNo > 0 && countDownParams.ucPlanNo <= NUM_SCHEME)
    {
        if((countDownParams.ucPlanNo-1)%3 == 0)//普通定周期方案是可以让用户自定义描述的
        {
            if((countDownParams.ucPlanNo <= 47) && (strlen((char *)descParam.desc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3]) > 0))
                memcpy(countDownParams.ucCurPlanDsc, descParam.desc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3], sizeof(countDownParams.ucCurPlanDsc));	//ï¿½ï¿½ï¿½Ó·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
            else
                snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"定周期 %d",(countDownParams.ucPlanNo+2)/3);
        }
        else if((countDownParams.ucPlanNo-2)%3 == 0)//感应
        {
            if((countDownParams.ucPlanNo <= 47) && (strlen((char *)descParam.desc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3]) > 0))
                snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"感应 %s",descParam.desc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3]);
            else
                snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"感应 %d",(countDownParams.ucPlanNo+1)/3);
                countDownParams.ucPlanNo = INDUCTIVE_SCHEMEID;	//感应控制时倒计时默认返回方案254
        }
        else if(countDownParams.ucPlanNo%3 == 0) //协感
        {
            if((countDownParams.ucPlanNo <= 47) && (strlen((char *)descParam.desc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3 - 1]) > 0))
                snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"协感 %s",descParam.desc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3 - 1]);
            else
                snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"协感 %d",countDownParams.ucPlanNo/3);
                countDownParams.ucPlanNo = INDUCTIVE_COORDINATE_SCHEMEID;//协感控制时倒计时默认返回方案250
         }
    }
    memcpy(udp_info, &countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
}

//void *HikCommunication::CommunicationModule(void *arg)
void HikCommunication::run(void *arg)
{
    //ï¿½ï¿½ï¿½ï¿½UDPï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
    ssize_t result = 0;

    INFO("hik communication run run\n");
    struct STRU_N_PatternNew *communication = (struct STRU_N_PatternNew *)udp_info.iValue;
    STEP_CTRL_PARAMS *stepCtrlParams = (STEP_CTRL_PARAMS *)&udp_info;
    CANCEL_STEP_FEEDBACK_PARAMS *cancelStepFeedback = (CANCEL_STEP_FEEDBACK_PARAMS *)&udp_info;
    int specialControlSchemeId = 0;
    struct SAdjustTime *timep = (struct SAdjustTime *)udp_info.iValue;
    UInt8 schemeId = 0;
    //UInt8 need_write_extconfig = 0;

    memset(&udp_info,0,sizeof(struct UDP_INFO));

    if (-1 == sockconnect.CreateSocket(NULL, UDP) )
    {
        printf("socket udp init error!!!\n");
        return;
        //pthread_exit(NULL);
    }
    while(1)
    {

        result = sockconnect.RecvData(&udp_info, sizeof(udp_info));

        if(-1 == result)
        {
            //ERR("############===>  Failed Error   %s\n",strerror(errno));

        }
        else
        {
            //INFO("CommunicationModule  0x%x \n",udp_info.iType);
            if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x3eb4)//心跳包
                sockconnect.SendData(&udp_info, result);//sendto(socketFd, (char*)&udp_info, result, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xdf)
            {
                communication->wProtocol = 2;	//此协议是HIK协议
                communication->wDscType = configParam.config.sSpecialParams.iSignalMachineType;
                communication->unPort = 20000;
                sockconnect.SendData(&udp_info, sizeof(struct STRU_N_PatternNew) + 8);
                //sendto(socketFd, (char*)&udp_info, sizeof(struct STRU_N_PatternNew) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)//下载特殊检测参数
            {
                //log_debug("download special check parameters, value = %#x", udp_info.iValue[0]);
                configParam.DownloadSpecialParams(&udp_info);
                SendSuccessMsg();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x93)//上载特殊检测参数
            {
                //log_debug("upload special check parameters");
                configParam.UploadSpecialParams(&udp_info);
                result = sockconnect.SendData(&udp_info, 12);
                //sendto(socketFd, (char*)&udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)//故障清除
            {
                //log_debug("clear fault log information");
                SendSuccessMsg();
                gIts->ItsClearFaultLog();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x97)//上载红灯电流
            {
                //log_debug("upload red light current values");
                //memcpy(udp_cur_value_info->redCurrentValue, gRedCurrentValue, sizeof(gRedCurrentValue));
                result = sockconnect.SendData(&udp_info, sizeof(struct UDP_CUR_VALUE_INFO));
                //sendto(socketFd, (char*)&udp_info, sizeof(struct UDP_CUR_VALUE_INFO), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x95)//上载电流参数
            {
                //log_debug("upload red light current parameters");
                configParam.UploadRedCurrentParams(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(struct CURRENT_PARAMS_UDP));
                //sendto(socketFd, (char*)&udp_info, sizeof(struct CURRENT_PARAMS_UDP), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)//下载电流参数
            {
                //log_debug("download red light current parameters");
                configParam.DownloadRedCurrentParams(&udp_info);
                SendSuccessMsg();
            }
#if defined(__linux__)
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)//恢复默认参数
            {
            #if defined(__linux__) && defined(__arm__)
                //log_debug("recover default parameters");
                system("rm -rf /home/*");
                SendSuccessMsg();
            #endif
            }
            else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15d || udp_info.iType == 0x15f || udp_info.iType == 0x161))
            {
                //log_debug("set ip address");
                DownloadIpAddress();//下载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
            }
            else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15e || udp_info.iType == 0x160 || udp_info.iType == 0x162))
            {
                //log_debug("get ip address");
                UploadIpAddress();//上载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
            }
#endif
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9a)//下载相位描述
            {
                //log_debug("download phase describe");
                descParam.DownloadPhasedesc(&udp_info);
                SendSuccessMsg();

            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)//上载相位描述
            {
                //log_debug("upload phase describe");
                descParam.UploadPhasedesc(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(descParam.desc.sPhaseDescParams));
                //sendto(socketFd, (char*)&udp_info, sizeof(descParam.desc.sPhaseDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)//下载通道描述
            {
                //log_debug("download channel describe");
                descParam.DownloadChanneldesc(&udp_info);
                SendSuccessMsg();

            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)//上载通道描述
            {
                //log_debug("upload channel describe");
                descParam.UploadChanneldesc(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(descParam.desc.sChannelDescParams));
                //sendto(socketFd, (char*)&udp_info, sizeof(descParam.desc.sChannelDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)//下载方案描述
            {
                //log_debug("download scheme describe");
                descParam.DownloadSchemedesc(&udp_info);
                SendSuccessMsg();

            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)//上载方案描述
            {
                //log_debug("upload scheme describe");
                descParam.UploadSchemedesc(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(descParam.desc.sPatternNameParams));
                //sendto(socketFd, (char*)&udp_info, sizeof(descParam.desc.sPatternNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)//下载计划描述
            {
                //log_debug("download plan describe");
                descParam.DownloadPlandesc(&udp_info);
                SendSuccessMsg();

            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)//上载计划描述
            {
                //log_debug("upload plan describe");
                descParam.UploadPlandesc(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(descParam.desc.sPlanNameParams));
                //sendto(socketFd, (char*)&udp_info, sizeof(descParam.desc.sPlanNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)//下载日期描述
            {
                //log_debug("download date describe");
                descParam.DownloadDatedesc(&udp_info);
                SendSuccessMsg();

            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)//上载日期描述
            {
                //log_debug("upload date describe");
                descParam.UploadDatedesc(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(descParam.desc.sDateNameParams));
                //sendto(socketFd, (char*)&udp_info, sizeof(descParam.desc.sDateNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa1)//上载软硬件版本信息
            {
                //UploadVersionInfo(socketFd,fromAddr);
                result = sockconnect.SendData(&udp_info, sizeof(descParam.desc.sDateNameParams));
                //sendto(socketFd, (char*)&udp_info, sizeof(descParam.desc.sDateNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9f)//下载倒计时牌设置
            {
                //log_debug("download countdown set information");
                customParam.DownloadCountdownParams(&udp_info);
                SendSuccessMsg();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)//上载倒计时牌设置
            {
                //log_debug("upload countdown set information");
                customParam.UploadCountdownParams(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(customParam.custom.sCountdownParams) + 8);
                //sendto(socketFd, (char*)&udp_info, sizeof(customParam.custom.sCountdownParams) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)//下载串口配置
            {
                //log_debug("download serial config");
                if((udp_info.iValue[0] > 0) && (udp_info.iValue[0] <= 4))
                {
                    customParam.DownloadComParams(&udp_info);
                    SendSuccessMsg();
                }
                else
                {
                    result = -1;
                }
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa2)//上载串口配置
            {
                //log_debug("upload serial config");
                if((udp_info.iValue[0] > 0) && (udp_info.iValue[0] <= 4))
                {
                    customParam.UploadComParams(&udp_info);
                    result = sockconnect.SendData(&udp_info, sizeof(COM_PARAMS));
                    //sendto(socketFd, (char*)&udp_info, sizeof(COM_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
                }
                else
                {
                    result = -1;
                }
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9e)//获取倒计时参数信息
            {
                UploadCountdownFeedback(&udp_info);
                result = sockconnect.SendData(&udp_info, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
                //sendto(socketFd, (char*)&udp_info, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //锟斤拷锟斤拷锟斤拷息头锟斤拷锟斤拷息锟斤拷锟斤拷
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)//信号机重启
            {
            #if defined(__linux__) && defined(__arm__)
                //log_debug("System will restart");
                gIts->ItsWriteFaultLog(UNNORMAL_OR_SOFTWARE_REBOOT, 0);
                sync();
                sleep(1);
                system("reboot");
			#else
                SendSuccessMsg();
            #endif
            }
            else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_LOCK)//开启通道锁定
            {
                //log_debug("enable channel lock");
                customParam.EnableChannelLock(&udp_info);
                SendSuccessMsg();
            }
            else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_UNLOCK)//关闭通道锁定
            {
                customParam.DisableChannelLock(&udp_info);
                SendSuccessMsg();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd7)//设置降级参数
            {
                //log_debug("set demotion parameters");
                memcpy(&customParam.custom.demotionParams,&udp_info,sizeof(customParam.custom.demotionParams));
                SendSuccessMsg();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd8)//获取降级参数
            {
                //log_debug("get demotion parameters");
                customParam.custom.demotionParams.unExtraParamID = 0xd8;
                result = sockconnect.SendData((char*)&customParam.custom.demotionParams, sizeof(customParam.custom.demotionParams));
                //sendto(socketFd, (char*)&customParam.custom.demotionParams, sizeof(customParam.custom.demotionParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbb)//手动特殊控制命令
            {
                schemeId = udp_info.iValue[0];
                //log_debug("manual special control command, schemeid = %d", schemeId);
                if ((schemeId > 0) && (schemeId <= NUM_SCHEME) && (2 == schemeId % 3))	//手动感应方案控制
                    gIts->ItsCtl(TOOL_CONTROL, INDUCTIVE_SCHEMEID, schemeId);
                else if ((schemeId > 0) && (schemeId <= NUM_SCHEME) && (0 == schemeId % 3))	//手动协调感应方案控制
                    gIts->ItsCtl(TOOL_CONTROL, INDUCTIVE_COORDINATE_SCHEMEID, schemeId);
                else	//其他黄闪、全红、关灯、步进或手动执行定周期方案控制
                    gIts->ItsCtl(TOOL_CONTROL, schemeId, schemeId);
                specialControlSchemeId = schemeId;	//存下方案号用来上载时使用
                SendSuccessMsg();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6d)//获取当前控制方案号
            {
                udp_info.iValue[0] = specialControlSchemeId;
                result = sockconnect.SendData(&udp_info, 12);
                //sendto(socketFd, (char*)&udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbc)//信号机步进控制
            {
                //log_debug("excute step control, stageNO = %d", stepCtrlParams->unStepNo);
                gIts->ItsCtl(TOOL_CONTROL, STEP_SCHEMEID, stepCtrlParams->unStepNo);
                stepCtrlParams->unStepNo = 1;	//表示步进成功
                result = sockconnect.SendData(&udp_info, sizeof(STEP_CTRL_FEEDBACK_PARAMS));
                //sendto(socketFd, (char*)stepCtrlParams, sizeof(STEP_CTRL_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbd)//信号机取消步进
            {
                //log_debug("cancel step control");
                //gIts->ItsCtl(TOOL_CONTROL, 0, 0); //发送系统控制消息
                gIts->ItsCtl(TOOL_CONTROL, STEP_SCHEMEID, -1);//取消步进控制，步进号-1
                //specialControlSchemeId = 0;	//取消步进时默认进行系统控制
                cancelStepFeedback->unValue = 1;
                result = sockconnect.SendData(&udp_info, sizeof(CANCEL_STEP_FEEDBACK_PARAMS));
                //sendto(socketFd, (char*)cancelStepFeedback, sizeof(CANCEL_STEP_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbe)//设置经济型信号机型号
            {
                configParam.DownloadSignalMechineType(&udp_info);
                SendSuccessMsg();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbf)//获取经济型信号机型号
            {
                configParam.UploadSignalMechineType(&udp_info);
                result = sockconnect.SendData(&udp_info, 12);
                //sendto(socketFd, (char*)&udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
#if 0
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x71)//下载对时
            {
                struct tm settime;
                //char timebuf[64] = {0};
                configParam.DownloadTimezone(&udp_info);
                timep->ulGlobalTime += timep->unTimeZone;
				#if defined(__linux__) && defined(__arm__)
                stime((time_t *)&timep->ulGlobalTime);
                system("hwclock -w");
                localtime_r((time_t *)&timep->ulGlobalTime, &settime);
                //strftime(timebuf, sizeof(timebuf), "%F %T", &settime);
                //log_debug("set system time: %s", timebuf);
                #endif
                SendSuccessMsg();
            }
#endif
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x72)//上载对时
            {
                //log_debug("get system time");
//				timep->ulGlobalTime = time(NULL) - 8 * 3600;
//				timep->unTimeZone = 8 * 3600;
                configParam.UploadTimezone(&udp_info);
                result = sockconnect.SendData(&udp_info, 8 + sizeof(*timep));
                //sendto(socketFd, (char*)&udp_info, 8 + sizeof(*timep), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6c)//通道检测
            {
                //log_debug("excute channel check, terminal = %d", udp_info.iValue[0]);
                //ChannelCheckDeal(udp_info.iValue[0]);
                SendSuccessMsg();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x159)//上载通道状态
            {
                UploadChannelStatus();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x157)//上载相位状态
            {
                UploadPhaseStatus();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x2b)//上载跟随相位状态
            {
                UploadFollowPhaseStatus();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15a)//上载同步状态
            {
                UploadSyncStatus();
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd0)//实时运行方案信息
            {
                //log_debug("get real time running scheme information");
                MsgRealtimePattern* p = (MsgRealtimePattern*)(&udp_info);
                gHiktsc->ItsGetRealtimePattern(&udp_info);
				memcpy(p->phaseDesc, descParam.desc.phaseDescText[p->nPhaseTableId - 1], sizeof(p->phaseDesc));
                result = sockconnect.SendData(&udp_info, sizeof(MsgRealtimePattern));
                //sendto(socketFd, (char*)&udp_info, sizeof(MsgRealtimePattern), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd1)//实时流量统计获取接口
            {
                    /*
                        pthread_rwlock_rdlock(&gLockRealtimeVol);
                        gStructMsgRealtimeVolume.unExtraParamHead = 0x6e6e;
                        gStructMsgRealtimeVolume.unExtraParamID = 0xd1;
                        memcpy(&udp_info, &gStructMsgRealtimeVolume, sizeof(gStructMsgRealtimeVolume));
                        pthread_rwlock_unlock(&gLockRealtimeVol);
                        */
               result = sockconnect.SendData(&udp_info, sizeof(MsgRealtimeVolume));
               //sendto(socketFd, (char*)&udp_info, sizeof(MsgRealtimeVolume), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd2)//平台1049协议
            {  
                MsgPhaseSchemeId cStructMsg1049;
                //Set1049MsgContent(&cStructMsg1049);
                result = sockconnect.SendData((char*)&cStructMsg1049, sizeof(cStructMsg1049));
                //sendto(socketFd, (char*)&cStructMsg1049, sizeof(cStructMsg1049), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xeeeeeeec)//给触摸板发送通道倒计时信息
            {
                UploadChannelCountdown();
                //INFO("get countdown information");
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xeeeeeeed)//
            {
                UpdateLocalCfg(udp_info.iValue[0]);
            }
            else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_SET)//多时段通道锁定(解锁和原通道锁定共用一个消息)
            {
                if(customParam.custom.cChannelLockFlag == 1)//原通道已处于锁定状态不生效
                {
                    //log_debug("Can't override realtime channel lock");
                    SendFailureMsg();
                }
                else
                {
                    //log_debug("Enable mult periods channel lock");
                    customParam.DownloadMulChLock(&udp_info);
                    SendSuccessMsg();
                    //log_debug("Mult periods channel lock");
                }
            }
            else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_GET)//多时段通道锁定查询
            {
                //log_debug("Get mult periods channel lock info");
                customParam.UploadMulChLock(&udp_info);
                result = sockconnect.SendData(&udp_info, 12 + sizeof(customParam.custom.sMulPeriodsChanLockParams.chans));
                //sendto(socketFd, (char*)&udp_info, 12+sizeof(customParam.custom.sMulPeriodsChanLockParams.chans), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_LOCK_STATUS_GET)//通道锁定状态获取
            {
                STRU_CHAN_LOCK_PARAMS curLockStatus;
                memset(&curLockStatus, 0, sizeof(STRU_CHAN_LOCK_PARAMS));
                gHiktsc->gChanLockCtrl->GetCurChanLockStatus(&curLockStatus);
                memcpy(&udp_info.iValue[0], (char*)&curLockStatus, sizeof(STRU_CHAN_LOCK_PARAMS));
                result = sockconnect.SendData(&udp_info, 8+sizeof(STRU_CHAN_LOCK_PARAMS));
                //sendto(socketFd, (char*)&udp_info, 8+sizeof(STRU_CHAN_LOCK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else if(udp_info.iHead == EXTEND_MSG_HEAD)
            {
                STRU_EXTEND_UDP_MSG *msg = (STRU_EXTEND_UDP_MSG*)&udp_info;
                //INFO("-->udp recv: %x,%x,%x,%x", udp_info.iHead, udp_info.iType, udp_info.iValue[0],udp_info.iValue[1]);
                //INFO("-->udp recv xml: %s", msg->xml);
                //XMLMsgHandle(msg);
                //INFO("-->udp send xml: %s", msg->xml);
                result = sockconnect.SendData(&udp_info, msg->len + 8);
                //sendto(socketFd, (char*)&udp_info, msg->len+8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
            }
            else
            {
                UpAndDownLoadDeal();	//上下载处理
                if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//下载配置完成
                {
                    //保存参数到配置文件
                    
                    INFO("update hikconfig end!!!\n");

                    customParam.SaveCustomUpdate();
                    descParam.SaveDescUpdate();
                    configParam.SaveConfigUpdate();
                    //saveExtconfig.start();
                    INFO("update extend config end\n");

                }
            }

            if (result == -1)
            {
                ERR("sendto udp info error!!!\n");
            }
            usleep(10000);
        }
    }
}
