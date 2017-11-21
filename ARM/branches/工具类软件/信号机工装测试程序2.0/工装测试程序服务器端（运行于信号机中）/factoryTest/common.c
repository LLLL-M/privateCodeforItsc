#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "common.h"
#include "debug.h"
/***********************************************
 和调用system函数效果相同，
 不同的是可以获取执行后的输出结果
 cmd: 要执行命令字符串
 result: 存储执行后的输出结果的buff
 buffsize: 存储空间result的大小
 ************************************************/
int executeCMD(const char *cmd, char *result, int buffsize)
{
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r")) != NULL)
    {
        while(fgets(buf_ps, 1024, ptr)!=NULL)
        {
           if((strlen(result) + strlen(buf_ps)) >= buffsize)
               break;
           strncat(result, buf_ps, 1024);
        } 
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        printf("popen %s error\n", ps);
    }
	return strlen(result);
}
 /* 串口设置为:
      波特率：9600， 8bit
      停止位：1
      奇偶校验位：N   */
int open_port(int comport) 
{ 
	char devname[40]={0}; 
	struct termios newtio,oldtio;
	int fd = -1;
	
	sprintf(devname, "/dev/ttyS%d", comport - 1);
	fd = open(devname, O_RDWR|O_NOCTTY|O_NDELAY); 
	if (-1 == fd)
	{ 
		INFO("Can't Open Serial Port %s", devname); 
		return -1; 
	}
	//鎭㈠涓插彛涓洪樆濉炵姸鎬?
	if(fcntl(fd, F_SETFL, 0)<0) 
	{
		INFO("fcntl failed!"); 
	}
	//娴嬭瘯鏄惁涓虹粓绔澶?
	if(isatty(STDIN_FILENO)==0) 
	{
		INFO("standard input is not a terminal device"); 
	}
	
	//淇濆瓨娴嬭瘯鐜版湁涓插彛鍙傛暟璁剧疆锛屽湪杩欓噷濡傛灉涓插彛鍙风瓑鍑洪敊锛屼細鏈夌浉鍏崇殑鍑洪敊淇℃伅
     if(tcgetattr(fd,&oldtio) != 0) 
	 {  
        INFO("SetupSerial 1"); 
		return -1; 
     } 
     memset(&newtio, 0, sizeof(newtio)); 
	 //姝ラ涓€锛岃缃瓧绗﹀ぇ灏?
     newtio.c_cflag  |=  CLOCAL | CREAD;  
     newtio.c_cflag &= ~CSIZE; 
	 newtio.c_cflag |= CS8;	//璁剧疆鍋滄浣?
	 newtio.c_cflag &= ~PARENB;	//璁剧疆濂囧伓鏍￠獙浣?
	 //璁剧疆娉㈢壒鐜?
	 cfsetispeed(&newtio, B9600); 
	 cfsetospeed(&newtio, B9600); 	 
	 /*璁剧疆鍋滄浣? */ 
      newtio.c_cflag &=  ~CSTOPB; 
	/*璁剧疆绛夊緟鏃堕棿鍜屾渶灏忔帴鏀跺瓧绗? */ 
     newtio.c_cc[VTIME]  = 0; 
     newtio.c_cc[VMIN] = 0; 
	/*澶勭悊鏈帴鏀跺瓧绗? */ 
     tcflush(fd,TCIFLUSH); 
	/*婵€娲绘柊閰嶇疆*/ 
	if((tcsetattr(fd,TCSANOW,&newtio))!=0) 
     { 
      ERR("com set error"); 
      return -1; 
     } 
	return fd;
}
/*********************************************************************************
*
* 	设置串口相关参数
*
***********************************************************************************/
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop) 
{ 
	struct termios newtio,oldtio; 
	 
    if( tcgetattr( fd,&oldtio) != 0)//保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息 
    {  
        ERR("error to SetupSerial : %s \n",strerror(errno)); 
        return -1; 
    } 
 	bzero( &newtio, sizeof( newtio ) ); 
 	
	//步骤一，设置字符大小
 	newtio.c_cflag  |=  CLOCAL | CREAD;  
 	newtio.c_cflag &= ~CSIZE;  
 	
	//设置停止位 
 	switch( nBits ) 
 	{ 
    	case 7: 
    		newtio.c_cflag |= CS7; 
    		break; 
    	case 8: 
    		newtio.c_cflag |= CS8; 
    		break; 
 	} 
	//设置奇偶校验位 
 	switch( nEvent ) 
 	{ 
    	case 'O': //奇数 
    		newtio.c_cflag |= PARENB; 
    		newtio.c_cflag |= PARODD; 
    		newtio.c_iflag |= (INPCK | ISTRIP); 
    		break; 
    	case 'E': //偶数 
    		newtio.c_iflag |= (INPCK | ISTRIP); 
    		newtio.c_cflag |= PARENB; 
    		newtio.c_cflag &= ~PARODD; 
    		break; 
    	case 'N':  //无奇偶校验位 
    		newtio.c_cflag &= ~PARENB; 
    		break; 
 	} 
 	//设置波特率 
	switch( nSpeed ) 
 	{ 
    	case 2400: 
    		cfsetispeed(&newtio, B2400); 
    		cfsetospeed(&newtio, B2400); 
    		break; 
    	case 4800: 
    		cfsetispeed(&newtio, B4800); 
    		cfsetospeed(&newtio, B4800); 
    		break; 
    	case 9600: 
    		cfsetispeed(&newtio, B9600); 
    		cfsetospeed(&newtio, B9600); 
    		break; 
    	case 115200: 
    		cfsetispeed(&newtio, B115200); 
    		cfsetospeed(&newtio, B115200); 
    		break; 
    	case 460800: 
    		cfsetispeed(&newtio, B460800); 
    		cfsetospeed(&newtio, B460800); 
    		break; 
    	default: 
    		cfsetispeed(&newtio, B9600); 
    		cfsetospeed(&newtio, B9600); 
    		break; 
 	} 
	 /*设置停止位*/ 
 	if( nStop == 1 ) 
      	newtio.c_cflag &=  ~CSTOPB; 
 	else if ( nStop == 2 ) 
      newtio.c_cflag |=  CSTOPB; 
      
	/*设置等待时间和最小接收字符*/ 
 	newtio.c_cc[VTIME]  = 0; 
 	newtio.c_cc[VMIN] = 0; 
 	
    /*处理未接收字符*/ 
 	tcflush(fd,TCIFLUSH); 
 	
	/*激活新配置*/ 
	if((tcsetattr(fd,TCSANOW,&newtio))!=0) 
 	{ 
  		perror("com set error"); 
  		return -1; 
 	}	 
 	
 	return 0; 
} 
