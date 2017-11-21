/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer.
 */

/********************************* Includes ***********************************/

#include "main.h"

#include "Util.h"
#include "Net.h"
#include "parse_ini.h"

#include "WebsCallback.h"
#include "DataExchange.h"


SignalControllerPara *gSignalControlpara;//全局信号机参数  added by xwh 2014-8-13

extern  IPINFO IpInfo_eth0; //

static char_t		*rootWeb = T("www");			/* Root web directory */
static char_t		*demoWeb = T("wwwdemo");		/* Root web directory */
static char_t		*password = T("");				/* Security password */
static int			port = WEBS_DEFAULT_PORT;		/* Server port */
static int			retries = 5;					/* Server port retries */
static int			finished = 0;					/* Finished flag */

int initWebs(int demo);


/*
 *	Exit cleanly on interrupt
 */
void sigintHandler(int unused)
{
	finished = 1;
}


/*****************************************************************************
 函 数 名  : InitGolobalVar
 功能描述  : 服务器启动后，加载配置文件全局参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年8月13日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int InitGolobalVar()
{
    gSignalControlpara = (PSignalControllerPara)malloc(sizeof(SignalControllerPara));

    if(!gSignalControlpara)
    {
        return false;

    }
    memset(gSignalControlpara,0,sizeof(SignalControllerPara));
/*    if((LoadDataFromCfg(gSignalControlpara, NULL) == 1) && (IsSignalControlparaLegal(gSignalControlpara) == 0))
    {
        return true;
    }
*/
    if(GetSignalControlParams() == 1)
    {
        return true;
    }
    
    return false;
}

/*****************************************************************************
 函 数 名  : PrintVersionInfo
 功能描述  : 打印编译时间及版本信息，奇数为调试版本，偶数为发布版本
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void PrintVersionInfo()
{
    log_debug("Version compiled time:%s,  %s,  Version :  %s\n",__DATE__,__TIME__,VERSION_WEB_APP);
}


int main(int argc, char** argv)
{

    PrintVersionInfo();
    
	#ifdef USE_DEMO_MODE
	int demo = 1;
	#else
	int demo = 0;
	#endif

/*
 *	Initialize the memory allocator. Allow use of malloc and start
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);

/*
 *	Initialize the web server
 */
	if (initWebs(demo) < 0) {
		log_error("initWebs error.\n");	// added by lxp
		return -1;
	}

    GetLocalHost();

    //加载配置文件
    if(InitGolobalVar() == false)
    {
        log_error("Init SignalControllerPara error . \n");
    }
    else
    {
        log_debug("Init SignalControllerPara sucess . \n");
    }


#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
/*	websRequireSSL("/"); */	/* Require all files be served via https */
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	finished = 0;
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
	}
#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return 0;
}

int GetRunningEthName(char *ip)
{
	struct ifreq ifreqs[16];
	struct ifconf conf;
	int i, ifc_num;
	struct ifreq *ifreq = ifreqs;
	int sockfd = -1;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return 0;
	}
	conf.ifc_len = sizeof(ifreqs);
	conf.ifc_req = ifreqs;
	if (ioctl(sockfd, SIOCGIFCONF, &conf))
	{
		ERR("get conf fail, error info: %s\n", strerror(errno));
		close(sockfd);
		return 0;
	}

	ifc_num = conf.ifc_len / sizeof(struct ifreq);
	for (i = 0; i < ifc_num; i++, ifreq++)
	{
		if ((strncmp(ifreq->ifr_name, "lo", 2) == 0)
			|| (strncmp(ifreq->ifr_name, "can", 3) == 0))
			continue;
		ioctl(sockfd, SIOCGIFFLAGS, ifreq);
		if ((ifreq->ifr_flags & IFF_RUNNING) == 0)
			continue;
		/**********************************/
		strcpy(ip,inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
		break;
	}
	close(sockfd);

	return 1;
}


char * GetEth0Ip(char *dn_or_ip)
{
    struct hostent *host;
    struct ifreq req;
    int sock;

    if (dn_or_ip == NULL) return NULL;

    if (strcmp(dn_or_ip, "localhost") == 0)
    {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        strncpy(req.ifr_name, "eth1", IFNAMSIZ);

        if ( ioctl(sock, SIOCGIFADDR, &req) < 0 )
        {
            printf("ioctl error: %s\n", strerror (errno));
            return NULL;
        }

        dn_or_ip = (char *)inet_ntoa(*(struct in_addr *) &((struct sockaddr_in *) &req.ifr_addr)->sin_addr);
        shutdown(sock, 2);
        close(sock);
    }
    else
    {
        host = gethostbyname(dn_or_ip);
        if (host == NULL)
        {
            return NULL;
        }
        dn_or_ip = (char *)inet_ntoa(*(struct in_addr *)(host->h_addr));
    }
    return dn_or_ip;
}


/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith"));
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave."));

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, WEBS_DEFAULT_HOME);
		return 1;
	}
	return 0;
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks()
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY, 0666)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif


int initWebs(int demo)
{
	char			host[128], dir[128], webdir[128];
	char			*cp;
	char            current_ip_address[64];
	char_t			wbuf[128];

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

#ifdef USER_MANAGEMENT_SUPPORT
/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *	Define the local Ip address, host name, default home page and the
 *	root web directory.
 */

	if (gethostname(host, sizeof(host)) < 0) {
		error(E_L, E_LOG, T("Can't get hostname"));
		return -1;
	}

    #if 0
	current_ip_address = NULL;
	if ((current_ip_address = GetEth0Ip( "localhost")) == NULL)
	{
		printf ("Ip address convert error!\n");
		return -1;
	}
	else
	{
		printf("current eth1 ip address: %s\n", current_ip_address);
	}
	#endif

	if(GetRunningEthName(current_ip_address) == 0)
	{
		printf ("Ip address convert error!\n");
		return -1;	
	}
	else
	{
		//printf("current eth1 ip address: %s\n", current_ip_address);
	}
	//sprintf(host,"%s","localhost")


/*
 *	Set ../web as the root web. Modify this to suit your needs
 *	A "-demo" option to the command line will set a webdemo root
 */
	getcwd(dir, sizeof(dir));
	if ((cp = strrchr(dir, '/'))) {
		*cp = '\0';
	}
	if (demo) {
		sprintf(webdir, "/%s",  demoWeb);
	} else {
		sprintf(webdir, "/%s",  rootWeb);
	}
	log_debug("host:%s,ip:%s,webdir:%s,port:%d\n",host,current_ip_address,webdir,port);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir(webdir);
	//cp = inet_ntoa(intaddr);
	cp = current_ip_address;
	//printf("cp:%s\n",cp);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("default.asp"));
	websSetPassword(password);

/*
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security
 *	handler, forms handler and the default web page handler.
 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler,
		WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler,
		WEBS_HANDLER_LAST);

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);
	websFormDefine(T("loginTest"),loginTest);
	websFormDefine(T("unitParams"),unitParams);
	websFormDefine(T("PhaseTable"),PhaseTable);
	websFormDefine(T("ringAndPhase"),ringAndPhase);
	websFormDefine(T("channelTable"),channelTable);
	websFormDefine(T("splitTable"),splitTable);
	websFormDefine(T("specialControl"),specialControl);
	websFormDefine(T("greenRatio"),greenRatio);
	websFormDefine(T("faultDetectionSet"),faultDetectionSet);
	websFormDefine(T("sequenceTable"),sequenceTable);
	websFormDefine(T("programTable"),programTable);
	websFormDefine(T("timeBasedActionTable"),timeBasedActionTable);
	websFormDefine(T("timeInterval"),timeInterval);
	websFormDefine(T("scheduling"),scheduling);
	websFormDefine(T("overlapping"),overlapping);
	websFormDefine(T("coordinate"),coordinate);
	websFormDefine(T("vehicleDetector"),vehicleDetector);
	websFormDefine(T("pedestrianDetector"),pedestrianDetector);
	websFormDefine(T("faultConfig"),faultConfig);
	websFormDefine(T("TreeDynamicParameter"),TreeDynamicParameter);
	websFormDefine(T("actionDownload"),actionDownload);
	websFormDefine(T("upldForm"), upldForm);
	websFormDefine(T("saveAllPara"), saveAllPara);
	websFormDefine(T("resetAllPara"), resetAllPara);
	websFormDefine(T("clearAllPara"), clearAllPara);
	websFormDefine(T("libsInfo"), getLibsInfo);
/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);
	return 0;
}


/******************************************************************************/
