/*hdr
**
**	
**
**	FILE NAME:	
**
**	AUTHOR:		
**
**	DATE:		
**
**	FILE DESCRIPTION:倒计时牌控制程序
**			
**			
**
**	FUNCTIONS:	
**			
**			
**			
**	NOTES:		
*/  
#include <stdio.h> 
#include <string.h> 
#include <sys/types.h> 
#include <errno.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <termios.h> 
#include <stdlib.h> 
#include "common.h"
#include <pthread.h>

//临时修改，避免越界
CountDownVeh countdown_veh[17];
CountDownPed countdown_ped[17];
extern COM_PARAMS g_com_params[4];                //全局串口参数


/*********************************************************************************
*
* 	打开指定串口
*
***********************************************************************************/
int open_port(int fd,int comport) 
{ 
	//char *dev[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"}; 
	//long  vdisable; 
	//串口1 
	if (comport==1)
	{
		fd = open( "/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     	} 
	 //串口2 
     else if(comport==2)
     {     
		fd = open( "/dev/ttyS1", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     	} 
	 //串口3 
     else if (comport==3)
     { 
		fd = open( "/dev/ttyS2", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     	} 
	 //串口4 
	 else if (comport==4)
     	{ 
		fd = open( "/dev/ttyS3", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     	 } 
	 //串口5 
	 else if (comport==5)
       { 
		fd = open( "/dev/ttyS4", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     	 } 
	 //串口6 
	 else if (comport==6)
       { 
		fd = open( "/dev/ttyS5", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     	 } 
	 //串口7 
	 else if (comport==7)
     	 { 
		fd = open( "/dev/ttyS6", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     	 } 
	 //恢复串口为阻塞状态
     	if(fcntl(fd, F_SETFL, 0)<0) 
	{
		printf("fcntl failed!\n"); 
	}
     	else
	{
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	}
	 //测试是否为终端设备 
     	if(isatty(STDIN_FILENO)==0) 
	{
		printf("standard input is not a terminal device\n"); 
	}
     	else 
	{
		printf("isatty success!\n"); 
	}
     	printf("fd-open=%d\n",fd); 
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
	 //保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
     	if( tcgetattr( fd,&oldtio) != 0) 
	{  
        	perror("SetupSerial 1"); 
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
     	printf("set done!\n"); 
     	return 0; 
} 

/*******************************
*
*	倒计时接口函数
*
*******************************/
void  count_down_interface(void)
{
	int i = 0;
	int fd = 0; 
    //int nread = 0;
	int nwrite = 0;
	char nEvent = 'N';
	char sendBuff[1024] = {0};//485协议数据
	//打开串口 
    if((fd=open_port(fd,5))<0)
	{
        perror("open_port error"); 
        return; 
    } 
	if(g_com_params[1].unParity == 0)
	{
		nEvent = 'N';
	}
	else if(g_com_params[1].unParity == 1)
	{
		nEvent = '0';
	}
	else if(g_com_params[1].unParity == 2)
	{
		nEvent = 'E';
	}
	else
	{
		nEvent = 'N';
	}
	//设置串口 
    if((i=set_opt(fd,g_com_params[1].unBaudRate,g_com_params[1].unDataBits,nEvent,g_com_params[1].unStopBits))<0)
	{
        perror("set_opt error"); 
        return; 
    } 
	while(1)
	{
		for(i = 1; i<17; i++)
		{
			if(countdown_veh[i].veh_color > 0)
			{
				printf("phase: %d ,color: %d ,phaseTime: %d \n",i,countdown_veh[i].veh_color,countdown_veh[i].veh_phaseTime);
				if((nwrite=write(fd,sendBuff,1024))>0)
				{
					printf("倒计时485输出成功\n");
				}
				else
				{
					printf("error:nwrite=%d\n",nwrite);
				}

			}
			else
			{
				continue;
			}
		}
		usleep(1000*500);
	}
}
/*********************************************************************************
*
* 倒计时线程初始化。
*
***********************************************************************************/
int count_down_thread_create()
{
	int result = 0;
	pthread_t CountDown_thread;
	memset((char*)countdown_veh, 0, sizeof(countdown_veh));
	memset((char*)countdown_ped, 0, sizeof(countdown_ped));
	result = pthread_create(&CountDown_thread,NULL,(void *) count_down_interface,NULL);	
	if(result != 0 )
	{
		printf("Create count down pthread error!\n");
		return 0;
	}
	return -1;

}

