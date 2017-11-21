#include "TS_sadp.h"
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <net/route.h>
#include <linux/sockios.h>
#include <errno.h>
#include <fcntl.h>


typedef unsigned int UINT32;
#define MAX_ETHERNET		2
static unsigned int old_gateway[MAX_ETHERNET] = {0};
#define ROUTE_METRIC_BASE       100

static int netcfg(UINT32 ip, UINT32 mask,UINT32 gateway,UINT32 port);
int resetDfltPasswd(char *arg)
{
	//printf("***********************resetDfltPasswd********************\n");
	return 0;
}

/********************************************************************************
  Function:     // getLocalAddrInfo
  Description:  // 获取本地网口的mac、IP及子网掩码
  Input:        // ifname:网口名
  Output:	// macAddr:mac地址数组; ipAddr:本地IP; netmask:子网掩码
  Return:	// 0:获取成功; -1:获取信息失败
********************************************************************************/
int getLocalAddrInfo(const char *ifname, unsigned char macAddr[],
                     int *ipAddr, int *netmask)
{
    int		sock;
    int		status;
    int		i;
    struct 	ifreq ifr;
    struct 	sockaddr_in *SockAddrIn = NULL;
    unsigned char *ptr;

    memset((char *)&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1)
    {
        perror("socket error!");
        return -1;
    }

    /* 获取mac地址 */
    status = ioctl(sock, SIOCGIFHWADDR, &ifr);
    if(status < 0)
    {
        perror("ioctl1 error!");
        close(sock);
        return -1;
    }
    ptr = ifr.ifr_hwaddr.sa_data;
    for(i=0; i<6; i++)
    {
        macAddr[i] = ptr[i];
    }

//    printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n",macAddr[0], macAddr[1],macAddr[2],
//        macAddr[3],macAddr[4],macAddr[5]);

    /* 获取ip地址 */
    status = ioctl(sock, SIOCGIFADDR, &ifr);
    if(status < 0)
    {
        perror("ioctl2 error!");
        close(sock);
        return -1;
    }

    SockAddrIn = (struct sockaddr_in *)(&ifr.ifr_addr);
    *ipAddr = SockAddrIn->sin_addr.s_addr;
//    printf("addr: %s\n", inet_ntoa(SockAddrIn->sin_addr));

    /* 获取netmask地址 */
    status = ioctl(sock, SIOCGIFNETMASK, &ifr);
    if(status < 0)
    {
        perror("ioctl3 error!");
        close(sock);
        return -1;
    }

    SockAddrIn = (struct sockaddr_in *)(&ifr.ifr_addr);
    *netmask = SockAddrIn->sin_addr.s_addr;
//    printf("netmask: %s\n", inet_ntoa(SockAddrIn->sin_addr));
    close(sock);

    return 0;
}

/********************************************************************************
  Function:     // getLinkedNetIf
  Description:  // 获取终端服务器连接了网线的网口名称
  Input:        // None
  Output:		// ifname:网口名
  Return:		// 0:获取成功; -1:获取信息失败
********************************************************************************/
#if 0
int getLinkedNetIf(char *linkedIfName)
{
    FILE  *stream = NULL;
    char  retBuf[512] = {0};
    char  *pTemp = NULL;

    stream = popen("mii-tool 2>/dev/NULL", "r");
    while(fgets(retBuf, sizeof(retBuf), stream) != NULL)
    {
        if(strstr(retBuf, "link ok") != NULL)
        {
            pTemp = strchr(retBuf, ':');
            memcpy(linkedIfName, retBuf, pTemp-retBuf);
//            printf("linkedIfName: %s!!!\n", linkedIfName);
            pclose(stream);
            return 0;
        }
    }
    pclose(stream);
    return -1;
}
#else
int getLinkedNetIf(char *linkedIfName)
{
    FILE  *stream = NULL;
    char  retBuf[512] = {0};
    char  *pTemp = NULL;
	char  chName[32] = "";
	int   i = 0;
	for (i=0; i<2; i++)
	{
		sprintf(chName, "ifconfig eth%d", i);
		stream = popen(chName, "r");
		while(fgets(retBuf, sizeof(retBuf), stream) != NULL)
		{
			if(strstr(retBuf, "RUNNING") != NULL)
			{
				pTemp = strstr(chName, "eth");
				if (pTemp != NULL)
				{
					strcpy(linkedIfName, pTemp);
				}
				else
				{
					return -1;
				}
				
				//printf("[%s]:%s\n", chName, retBuf);
				pclose(stream);
				return 0;
			}
		}
		pclose(stream);
	}
    return -1;
}
#endif

/********************************************************************************
  Function:     // getMachineBootTime
  Description:  // 获取终端服务器启动时间
  Input:        // None
  Output:		// bootTimeBuf:启动时间的字符串
  Return:		// 0:获取成功; -1:获取信息失败
********************************************************************************/
int getMachineBootTime(char *bootTimeBuf)
{
    FILE  *stream = NULL;
    char  retBuf[256] = {0};

    stream = popen("date -d \"$(awk -F. '{print $1}' /proc/uptime) second ago\" +\"%Y-%m-%d %H:%M:%S\"", "r");
    while(fgets(retBuf, sizeof(retBuf), stream) != NULL)
    {
        strcpy(bootTimeBuf, retBuf);
        pclose(stream);
        return 0;
    }
    pclose(stream);
    return -1;
}

/********************************************************************************
  Function:     // getSoftwareVersion
  Description:  // 获取终端服务器软件版本号
  Input:        // None
  Output:		// softVerBuf:软件版本号字符串，hardVerBuf:硬件版本号字符串
  Return:		// 0:获取成功; -1:获取信息失败
********************************************************************************/
int getSoftwareVersion(char *softVerBuf,char * hardVerBuf)
{
	int val = 0; 
       DEVICE_VERSION_PARAMS st_device_version_params;
	 memset(&st_device_version_params,0,sizeof(DEVICE_VERSION_PARAMS));
       int sock = 0;
	//sendto中使用的对方地址
	struct sockaddr_in toAddr;
	//在recvfrom中使用的对方主机地址
	struct sockaddr_in fromAddr;
	unsigned int fromLen;
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0)
	{
		printf("创建套接字失败!\r\n");
		return -1;
	}
	//非阻塞型socket
	val = fcntl(sock, F_GETFL, 0); 
	fcntl(sock, F_SETFL, val | O_NONBLOCK);
	memset(&toAddr,0,sizeof(toAddr));
	
	toAddr.sin_family=AF_INET;
	toAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	toAddr.sin_port = htons(20000);
	st_device_version_params.unExtraParamID = 0xa1;
	st_device_version_params.unExtraParamHead = 0x6e6e;
	if(sendto(sock,&st_device_version_params,9,0,(struct sockaddr*)&toAddr,sizeof(toAddr)) == -1)
	{
	 	printf("sendto() 失败\r\n");
		 close(sock);
	 	return -1;
	}
	sleep(1);
	fromLen = sizeof(fromAddr);
	if(recvfrom(sock,&st_device_version_params,sizeof(DEVICE_VERSION_PARAMS),0,(struct sockaddr*)&fromAddr,&fromLen)<=0)
	{
	 	printf("recvfrom()失败\r\n");
	 	close(sock);
	 	return -1;
	}
	printf("hikTSC HardVersion:%s\r\nhikTSC SoftVersion:%s\r\n",st_device_version_params.hardVersionInfo,st_device_version_params.softVersionInfo);
	strcpy(softVerBuf, st_device_version_params.softVersionInfo);
	strcpy(hardVerBuf,st_device_version_params.hardVersionInfo);
	close(sock);
	return 0;
}

/********************************************************************************
  Function:     // initSADP
  Description:  // 初始化sadp库
  Input:        // IfName:网口名称, localIpAddr:ip地址, maskAddr:子网掩码, macAddr:mac地址
  Output:		// None
  Return:		// 0:初始化成功
********************************************************************************/
int initSADP(char IfName[], int localIpAddr, int maskAddr, char macAddr[])
{
	dev_info info;
	memset(&info, 0, sizeof(dev_info));
	getSoftwareVersion(info.software_version,info.serial_no);    //获取信号机软硬件版本号
	info.dev_type = 2;
	info.enc_cnt = 0; //66535;                      //通道个数用来表示信号机设备好
	info.port = 8000;
	info.hdisk_cnt = 0;//9600;

	init_sadp_lib2(IfName, ntohl(localIpAddr), ntohl(maskAddr), macAddr, "12345", strlen("12345"),
               &info, netcfg, resetDfltPasswd);

	return 0;
}

/*******************************************
  函数名: set_netmask
  功能描述：设置子网掩码
  输入参数： 接口选择和子网掩码
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
int set_netmask(char * interface_name,char * netmask)
{
    int s;
    struct ifreq ifr;
    struct sockaddr_in netmask_addr;

    if(!interface_name || !netmask)
    {
        return -1;
    }
    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0) 
    {
        printf("set netmask socket failed\n");
        return -1;
    }

    strcpy(ifr.ifr_name,interface_name);

    bzero(&netmask_addr,sizeof(struct sockaddr_in));
    netmask_addr.sin_family = PF_INET;
    inet_aton(netmask,&netmask_addr.sin_addr);

    memcpy(&ifr.ifr_ifru.ifru_netmask,&netmask_addr,sizeof(struct sockaddr_in));

    if(ioctl(s,SIOCSIFNETMASK,&ifr) < 0) 
    {
        printf("set_netmask ioctl error and errno=%d\n",errno);
        close(s);
        return -1;
    }
    close(s);

    //printf("=============set netmask OK!!!===========\n");
    return 0;
}

/*******************************************
  函数名: set_ipaddr
  功能描述：设置底层的ip地址
  输入参数：接口选择和ip地址指针
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
int set_ipaddr(char * interface_name,char * ip)
{
    int s;
    struct ifreq ifr;
    struct sockaddr_in addr;

    if(!ip || !interface_name)
    {
        return -1;
    }
    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
        return -1;
    }

    strcpy(ifr.ifr_name,interface_name);

    bzero(&addr,sizeof(struct sockaddr_in));
    addr.sin_family = PF_INET;
    inet_aton(ip,&addr.sin_addr);

    memcpy(&ifr.ifr_ifru.ifru_addr,&addr,sizeof(struct sockaddr_in));

    if(ioctl(s,SIOCSIFADDR,&ifr) < 0)
    {
        printf("set_ipaddr ioctl error and errno=%d\n",errno);
        close(s);
        return -1;
    }

    close(s);

    printf("=============set ipaddr OK!!!===========\n");
    return 0;
}

/*******************************************
  函数名: del_gateway
  功能描述：删除网关
  输入参数： 无
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
int del_gateway(const char *ifName)
{
    int s;
    struct rtentry rt;
    struct sockaddr_in gateway_addr;

    if((s = socket(PF_INET,SOCK_DGRAM,0)) < 0) {
        return -1;
    }

    memset((char *)&rt,0,sizeof(struct rtentry) );
    //rt.rt_flags = RTF_UP|RTF_GATEWAY;

    bzero(&gateway_addr,sizeof(struct sockaddr_in));
#if 0
    gateway_addr.sin_family = PF_INET;
    inet_aton(gateway,&gateway_addr.sin_addr);
    memcpy(&rt.rt_gateway,&gateway_addr,sizeof(struct sockaddr_in));
#endif
    rt.rt_dev = (char *)ifName;
    bzero(&gateway_addr,sizeof(struct sockaddr_in));
    inet_aton("0.0.0.0",&gateway_addr.sin_addr);
    gateway_addr.sin_family = PF_INET;
    memcpy(&rt.rt_genmask,&gateway_addr,sizeof(struct sockaddr_in));
    memcpy(&rt.rt_dst,&gateway_addr,sizeof(struct sockaddr_in));

    if(ioctl(s,SIOCDELRT,&rt) < 0) {
        close(s);
        return -1;
    }
    close(s);
    
    printf("=============del gateway OK!!!===========\n");
    return 0;

}

/*******************************************
  函数名: del_route
  功能描述：删除路由设置
  输入参数： 路由和子网掩码
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
/* Add this function to delete route host - Aug29,2008 - xiemq */
 int del_route(char * route,char * mask, const char *ifName, int ishost)
{
    int s;
    struct rtentry rt;
    struct sockaddr_in gateway_addr;

    if((s = socket(PF_INET,SOCK_DGRAM,0)) < 0)
    {
        return -1;
    }

    memset((char *)&rt,0,sizeof(struct rtentry) );
    if(ishost)
    {
        rt.rt_flags = RTF_UP | RTF_HOST;
    }
    else
    {
        rt.rt_flags = RTF_UP;
    }

    bzero(&gateway_addr,sizeof(struct sockaddr_in));
    gateway_addr.sin_family = PF_INET;
    inet_aton(route,&gateway_addr.sin_addr);
    memcpy(&rt.rt_dst,&gateway_addr,sizeof(struct sockaddr_in));

    bzero(&gateway_addr,sizeof(struct sockaddr_in));
    inet_aton(mask,&gateway_addr.sin_addr);
    gateway_addr.sin_family = PF_INET;
    memcpy(&rt.rt_genmask,&gateway_addr,sizeof(struct sockaddr_in));

    rt.rt_dev = (char *)ifName;
    if(ioctl(s,SIOCDELRT,&rt) < 0) 
    {
        printf("del_route ioctl error and errno=%d\n",errno);
        close(s);
        return -1;
    }
    close(s);
    return 0;
}                                               

/*******************************************
  函数名: get_ipaddr
  功能描述：获取底层的ip地址
  输入参数：接口选择和ip地址以及ip地址长度
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
int get_ipaddr(const char * interface_name,char * ip,int len)
{
    int s;
    struct ifreq ifr;
    struct sockaddr_in * ptr;

    if(len<16) 
    {
        printf("The ip need 16 byte !\n");
        return -1;
    }

    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0) 
    {
        return -1;
    }

    strcpy(ifr.ifr_name,interface_name);

    if(ioctl(s,SIOCGIFADDR,&ifr) < 0) 
    {
        //printf("%s get_ipaddr ioctl error and errno=%d\n", interface_name, errno);
        close(s);
        return -1;
    }
    ptr = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_netmask;
    snprintf(ip,len,"%s",inet_ntoa(ptr->sin_addr));
    close(s);
    return 0;
}

/*******************************************
  函数名: get_netmask
  功能描述：获取底层的子网掩码
  输入参数： 接口选择和子网掩码指针以及长度
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
int get_netmask(const char * interface_name,char * netmask,int len)
{
    int s;
    struct ifreq ifr;
    struct sockaddr_in* ptr;

    if(len<16) {
        printf("The netmask need 16 byte !\n");
        return -1;
    }

    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        return -1;
    }

    strcpy(ifr.ifr_name,interface_name);

    if(ioctl(s,SIOCGIFNETMASK,&ifr) < 0) {
        printf("%s get_netmask ioctl error and errno=%d\n", interface_name, errno);
        close(s);
        return -1;
    }

    ptr = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_netmask;
    snprintf(netmask,len,"%s",inet_ntoa(ptr->sin_addr));
    close(s);
    return 0;
}


/*******************************************
  函数名: isSameSubnet
  功能描述：判断是否是相同的子网段
  输入参数： 网关
  输出参数： 无
  返回值：   0-不是;1-是
  其他信息：
 *******************************************/
int isSameSubnet(char *gateway, const char *ifName)
{
    unsigned int i = 0;
    unsigned int g = 0;
    struct in_addr gw;
    struct in_addr ip;
    struct in_addr mask;
    char tmp[16] = {0};

    if(gateway == NULL)
    {
        return 1;
    }

    /* get eth0 ip address */
    get_ipaddr(ifName, tmp, sizeof(tmp));
    memset(&ip, 0, sizeof(struct in_addr));
    inet_aton(tmp, &ip);
    //printf("ip address: 0x%x\n", ip.s_addr));                                              

    /* get eth0 subnet mask */
    memset(tmp, 0, sizeof(tmp));
    get_netmask(ifName, tmp, sizeof(tmp));
    memset(&mask, 0, sizeof(struct in_addr));
    inet_aton(tmp, &mask);
    //printf("ip mask: 0x%x\n", mask.s_addr));

    if(mask.s_addr == 0)
    {
        return 1;
    }

    /* get gateway */
    memset(&gw, 0, sizeof(struct in_addr));
    inet_aton(gateway, &gw);
    //printf("ip gateway: 0x%x\n", gw.s_addr));

    i = (ip.s_addr & mask.s_addr);          /* IP & Mask */
    g = (gw.s_addr & mask.s_addr);          /* Gateway & Mask */

    if(i == g)
    {
        printf("eth0 1\n");
        return 1;
    }

    printf("eth0 2\n");
    return 0;
}

/*******************************************
  函数名: set_route
  功能描述：设置路由
  输入参数： 路由地址和子网掩码地址和是否是虚拟主机
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
int set_route(char * route,char * mask, int ishost, const char *ifName, int metric)
{
    int s;
    struct rtentry rt;
    struct sockaddr_in gateway_addr;

    if((s = socket(PF_INET,SOCK_DGRAM,0)) < 0) 
    {
        return -1;
    }

    memset((char *)&rt,0,sizeof(struct rtentry) );

    /* Aug29,2008 - xiemq add  RTF_HOST
     * Nov13,2008 - xiemq add ishost parameters
     */
    if(ishost)
    {
            rt.rt_flags = RTF_HOST;
    }
    else
    {
            rt.rt_flags = RTF_UP;
    }

    bzero(&gateway_addr,sizeof(struct sockaddr_in));
    gateway_addr.sin_family = PF_INET;
    inet_aton(route,&gateway_addr.sin_addr);
    memcpy(&rt.rt_dst,&gateway_addr,sizeof(struct sockaddr_in));

    bzero(&gateway_addr,sizeof(struct sockaddr_in));
    inet_aton(mask,&gateway_addr.sin_addr);
    gateway_addr.sin_family = PF_INET;
    memcpy(&rt.rt_genmask,&gateway_addr,sizeof(struct sockaddr_in));

    rt.rt_dev = (char *)ifName;
    if(ioctl(s,SIOCADDRT,&rt) < 0)
    {
        printf("set_route ioctl error and errno=%d\n",errno);
        close(s);
        return -1;
    }
    close(s);

    printf("=============set route OK!!!===========\n");
    return 0;
}

/*******************************************
  函数名: set_gateway
  功能描述：设置网关
  输入参数：网关指针
  输出参数： 无
  返回值：   0成功;-1失败
  其他信息：
 *******************************************/
int set_gateway(char * gateway, const char *ifName)
{
    int ethNum;
    int s, i;
    int metric = 0;
    char etherIfName[IF_NAMESIZE] = {0};
    struct rtentry rt;
    struct sockaddr_in gateway_addr;

    /* Aug29,2008 - xiemq add */
    char gwip[16];
    struct in_addr gw; 

    if(!gateway || !ifName)
    {
        return -1;
    }
    while(del_gateway(ifName)==0);

    ethNum = 0;
    for(i=0; i<MAX_ETHERNET; i++)
    {
        snprintf(etherIfName, IF_NAMESIZE, "eth%d", i);
        if(!strcasecmp(etherIfName, ifName))
        {
            ethNum = i;
            metric = (0 != i)?(ROUTE_METRIC_BASE+i):0;
            break;
        }
    }
    
    /* Aug29,2008 - xiemq add */
    /* delete old gateway */
    if(old_gateway[ethNum] > 0)
    {
        memset(gwip, 0, sizeof(gwip));
        memset(&gw, 0, sizeof(struct in_addr));
        gw.s_addr = old_gateway[ethNum];
        strncpy(gwip, inet_ntoa(gw), sizeof(gwip));
        del_route(gwip, "0.0.0.0", ifName, 1);
    }

    if(!isSameSubnet(gateway, ifName))
    {
        /* set new gateway */
        if(set_route(gateway, "0.0.0.0", 1, ifName, 0) != 0)
        {
            return -1;
        }

        /* store new gateway */
        memset(&gw, 0, sizeof(struct in_addr));
        inet_aton(gateway, &gw);
        old_gateway[ethNum] = gw.s_addr;                    
    }
    /* end add section */
    
    if((s = socket(PF_INET,SOCK_DGRAM,0)) < 0) 
    {
        return -1;
    }

    memset((char *)&rt,0,sizeof(struct rtentry) );
    rt.rt_flags = RTF_UP|RTF_GATEWAY;
    rt.rt_metric = metric;
    rt.rt_dev = (char *)ifName;

    bzero(&gateway_addr,sizeof(struct sockaddr_in));
    gateway_addr.sin_family = PF_INET;
    inet_aton(gateway,&gateway_addr.sin_addr);
    memcpy(&rt.rt_gateway,&gateway_addr,sizeof(struct sockaddr_in));

    bzero(&gateway_addr,sizeof(struct sockaddr_in));
    inet_aton("0.0.0.0",&gateway_addr.sin_addr);
    gateway_addr.sin_family = PF_INET;
    memcpy(&rt.rt_genmask,&gateway_addr,sizeof(struct sockaddr_in));

    memcpy(&rt.rt_dst,&gateway_addr,sizeof(struct sockaddr_in));

    if(ioctl(s,SIOCADDRT,&rt) < 0)
    {
        printf("set_gateway ioctl error and errno=%d\n",errno);
        close(s);
        return -1;
    }
    close(s);

    printf("=============set gateway OK!!!===========\n");
    return 0;

}

static char g_strNetParam[32][32] = {0};

//保存IP,MASK, GATEWAY到interfaces文件
#if 0
int SaveNetparam(const char *ifName, const char *chIP, const char *chMask, const char *chGateway)
{
	int i = 0;
	int index = 0;
	FILE *fp = NULL;
	if (NULL == ifName || NULL == chIP || NULL == chMask || NULL == chGateway)
	{
		return -1;
	}
	printf("ifName:%s,chIP:%s,chMask:%s,chGateway:%s\n", ifName,chIP,chMask,chGateway);
	fp = fopen("/etc/network/interfaces", "r");
	if (NULL == fp)
	{
		return -2;
	}
	
	while (NULL != fgets(g_strNetParam[i], 32, fp) && i<32)
	{
		if (0 == strcmp(ifName, "eth0"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth0"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					sprintf(g_strNetParam[i], "address %s\n", chIP);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					sprintf(g_strNetParam[i], "netmask %s\n", chMask);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					sprintf(g_strNetParam[i], "gateway %s\n", chGateway);
				}
			}
		}
		else if (0 == strcmp(ifName, "eth1"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth1"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					sprintf(g_strNetParam[i], "address %s\n", chIP);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					sprintf(g_strNetParam[i], "netmask %s\n", chMask);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					sprintf(g_strNetParam[i], "gateway %s\n", chGateway);
				}
			}
		}
		i++;
	}
	fclose(fp);
	if (index > 0)
	{
		fp = fopen("/etc/network/interfaces2", "w");
		if (NULL == fp)
		{
			return;
		}
		fwrite(g_strNetParam[0], 32, i, fp);
		fclose(fp);
		return 0;
	}
	return -3;
}
#endif

int UpdateNetparam();

int SaveNetparam(const char *ifName, const char *chIP, const char *chMask, const char *chGateway)
{
	int i = 0;
	int index = 0;
	FILE *fp = NULL;
	if (NULL == ifName || NULL == chIP || NULL == chMask || NULL == chGateway)
	{
		return -1;
	}
	printf("ifName:%s,chIP:%s,chMask:%s,chGateway:%s\n", ifName,chIP,chMask,chGateway);
	fp = fopen("/etc/network/interfaces", "r");
	if (NULL == fp)
	{
		return -2;
	}
	
	while (NULL != fgets(g_strNetParam[i], 32, fp) && i<32)
	{
		if (0 == strcmp(ifName, "eth0"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth0"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					sprintf(g_strNetParam[i], "address %s\n", chIP);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					sprintf(g_strNetParam[i], "netmask %s\n", chMask);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					sprintf(g_strNetParam[i], "gateway %s\n", chGateway);
				}
			}
		}
		else if (0 == strcmp(ifName, "eth1"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth1"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					sprintf(g_strNetParam[i], "address %s\n", chIP);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					sprintf(g_strNetParam[i], "netmask %s\n", chMask);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					sprintf(g_strNetParam[i], "gateway %s\n", chGateway);
				}
			}
		}
		i++;
	}
	fclose(fp);
	if (index > 0)
	{
		fp = fopen("/etc/network/interfaces2", "w");
		if (NULL == fp)
		{
			return;
		}
		fwrite(g_strNetParam[0], 32, i, fp);
		fclose(fp);
		return 0;
	}
	return -3;
}


/********************************************************************************
  Function:     // netcfg
  Description:  // 回调函数，暂时不实现
********************************************************************************/
int netcfg(UINT32 ip, UINT32 mask,UINT32 gateway,UINT32 port) 
{
	struct in_addr strip, strmask, strgw;
	char   IfName[16] = {0};
	UINT32 dwIp = ntohl(ip);
	UINT32 dwMask = ntohl(mask);
	UINT32 dwGw = ntohl(gateway);
	char   netParam[3][32] = {0};
	printf("******************callback_test***************\n");
	printf("ip:%lu, mask:%lu, gw:%lu, port:%d\n", ip, mask, gateway, port);
	memcpy(&strip, &dwIp, 4);
	memcpy(&strmask, &dwMask, 4);
	memcpy(&strgw, &dwGw, 4);
	printf("ip:%s\n", inet_ntoa(strip));
	printf("mask:%s\n", inet_ntoa(strmask));
	printf("gateway:%s\n", inet_ntoa(strgw));
	
	strcpy(netParam[0], inet_ntoa(strip));
	strcpy(netParam[1], inet_ntoa(strmask));
	strcpy(netParam[2], inet_ntoa(strgw));
	
	if (0 != getLinkedNetIf(IfName))
	{
		return -1;
	}
	//set_ipaddr(IfName, inet_ntoa(strip));
	//set_netmask(IfName, inet_ntoa(strmask));
	//set_gateway(inet_ntoa(strgw), IfName);
	//save netparam
	SaveNetparam(IfName, netParam[0], netParam[1], netParam[2]);

	system("/etc/init.d/S40network restart");
	printf("modiyf ip success!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	return 0;
}


int main()
{
    int     retval = -1;
    int     localIpAddr = 0, maskAddr = 0;
    int     lastGetIpAddr = 0;  // 前一次获取到的IP地址，如果IP发生变化，则重新进行初始化
    char    macAddr[16] = {0};
    char    IfName[16] = {0};
    printf("************main***************\n");
    printf("RTSC_SADP Version:RTSC-SADP-V1.0.0.0 build 20141106\n");
	

    while(1)
    {
		
        retval = getLinkedNetIf(IfName);
        if(retval == -1)
        {
            sleep(3);
            continue;
        }
		
        retval = getLocalAddrInfo(IfName, macAddr, &localIpAddr, &maskAddr);
        if(retval != 0)
        {
            sleep(3);
            continue;
        }
		
        // 重复调用initSADP()会造成网口反复设置为Promiscuous mode
        if(localIpAddr != lastGetIpAddr)
        {
            initSADP(IfName, localIpAddr, maskAddr, macAddr);
            sleep(1);
			start_sadp_cap();
            sadp_login();
            sleep(3);
            //fini_sadp_lib();
            sleep(1);
            lastGetIpAddr = localIpAddr;
        }
        else
        {
            sadp_login();
            sleep(3);
        }
    }
    fini_sadp_lib();

    return 0;
}
