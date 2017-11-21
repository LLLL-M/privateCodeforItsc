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
**	FILE DESCRIPTION:串口接收GPS时间信息程序
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
#include <time.h>
#include <sys/times.h> 
#include <sys/time.h>
#include <string.h> 
#include <sys/types.h> 
#include <errno.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <termios.h> 
#include <stdlib.h> 

#define DEVICE_NAME "/dev/CPLD_IO"
int cpld_io_fd;
static int HiktcsRunningFlag = 0;


/*********************************************************************************
*
* 	CPU与CPLD交互初始化
*
***********************************************************************************/
void CPLD_IO_Init()
{
	
	cpld_io_fd = open(DEVICE_NAME,O_RDONLY);
	if(cpld_io_fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
	}
}

/*********************************************************************************
*
* 	写日志文件
*
***********************************************************************************/
int WriteLogInfos(const char *pFile,const char *pchMsg)
{
	//time_t now;  
	//struct tm *timenow; 
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;
	
	if(pchMsg == NULL)
	{
		printf("开机记录消息无效!\n");
		return -1;
	}
    
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("开机记录文件未能正确打开!\n");
        return -1;
    }
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("获取开机记录文件信息失败!\n");
		return -1;
	}
	if(f_stat.st_size > 100*1024)
	{
		fclose(pFileDebugInfo);
		//把文件的内容清0，即重写
		pFileDebugInfo = fopen(pFile, "w+");
	}

	//获取国际标准时间
	//time(&now);  
	  
	//转换为本地时间
	//timenow = localtime(&now); 

	fprintf(pFileDebugInfo,"%s\n",
		//timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		//timenow->tm_hour,timenow->tm_min,timenow->tm_sec,
		pchMsg);
    
    fclose(pFileDebugInfo);
    return 0;
}

/*********************************************************************************
*
* 	打开指定串口
*
***********************************************************************************/
int open_port(int fd,int comport) 
{ 
	char *dev[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"}; 
	long  vdisable; 
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

/*********************************************************************************
*
* 	获取GPS数据当前时间信息
*
***********************************************************************************/
int get_and_set_current_date(char * gps_str) 
{
	char * tmpstr1 = NULL;
	char tmpstr2[128] = "";
	char tmpstr3[128] = "";
	char cmdstr[64] = "";
	char datestr[256] = "";
	time_t time1 = 0;
	time_t scttime = 0;
	struct tm *tmsptr;
	struct tm curdate;
	struct timezone tz;
	struct timeval gtv;
	struct timeval tv;  
	int pos = 0;
	char *ptr = NULL;
	char tmpstr4[256] = "";
	static int FirstRun = 1;

	tmpstr1 = strstr(gps_str,"GPRMC");
	//获取到的时间信息完整
	if(tmpstr1 != NULL && strlen(tmpstr1) >= 68)
	{	
		//亮灯
		ioctl(cpld_io_fd,0x1011);

		ptr = strchr(tmpstr1, '*'); 
		if(ptr == NULL)
		{
			return 0;
		}

	 	pos =(ptr-tmpstr1) - 10;
		if(pos <= 0)
		{
			return 0;
		}

		//从GPS消息中截取年月日信息
		memcpy(datestr,tmpstr1 + pos,6);

		//printf("day-month-year:%s\n",datestr);
		//此时时间信息有效
		memcpy(tmpstr2,datestr+4,2);       //获取年
		curdate.tm_year = 2000 + atoi(tmpstr2) - 1900;
		memcpy(tmpstr2,datestr+2,2);       //获取月
		curdate.tm_mon = atoi(tmpstr2) - 1;
		memcpy(tmpstr2,datestr,2);       //获取日
		curdate.tm_mday = atoi(tmpstr2);
		memcpy(tmpstr2,tmpstr1+6,2);       //获取时
		curdate.tm_hour = atoi(tmpstr2);
		memcpy(tmpstr2,tmpstr1+8,2);       //获取分
		curdate.tm_min = atoi(tmpstr2);
		memcpy(tmpstr2,tmpstr1+10,2);       //获取秒
		curdate.tm_sec = atoi(tmpstr2);		

		//返回自国际标准时间后的秒数
		time1 = mktime(&curdate); 
		time1 = time1 +8 *60 *60;
		//转换成本地时间
		tmsptr = localtime(&time1);				
		printf("\nCurrent Time =%04d.%02d.%02d-%02d:%02d:%02d\n",tmsptr->tm_year + 1900,tmsptr->tm_mon + 1,tmsptr->tm_mday,
				tmsptr->tm_hour,tmsptr->tm_min,tmsptr->tm_sec);
		
		//判断该GPS信息是否为有效数据
		memset(tmpstr2,0,128);
		memcpy(tmpstr2,tmpstr1+16,1);       //获取GPS是否有效信息

		if(strcmp(tmpstr2,"A") != 0 && strcmp(tmpstr2,"V") != 0)
		{
			//GPS信息无效，则直接退出
			printf("GPS signal is invalid %s!!!\n",tmpstr2);
			return 0;
		}

		//获取当前时间
		time(&scttime);
		printf("time period = %d\n",abs(scttime -time1));
	
		//跟实际时间偏差超过2秒，则校时
		if(abs(scttime - time1) >= 2 ) 
		{			
			sprintf(cmdstr,"date -s %04d.%02d.%02d-%02d:%02d:%02d",tmsptr->tm_year + 1900,tmsptr->tm_mon + 1,tmsptr->tm_mday,
				tmsptr->tm_hour,tmsptr->tm_min,tmsptr->tm_sec);
			system(cmdstr);
			printf("%s\n",cmdstr);
			if(FirstRun == 1)
			{
				system("hwclock -w&");
				printf("set time OK\n");
				FirstRun = 0;
			}
		}

	}
	else
	{
		return 0;
	}
	return 1;
	
}


/*********************************************************************************
*
* 	GPS校时主程序入口
*
***********************************************************************************/
int main(void)  
{ 
    int fd = 0; 
    int nread = 0;
	int nwrite = 0;
	int i = 0; 
    char buff[2048]=""; 
	//打开串口 
    if((fd=open_port(fd,7))<0)
	{
		
        perror("open_port error"); 
        return -1; 
    } 
	//设置串口 
    if((i=set_opt(fd,9600,8,'N',1))<0)
	{
        perror("set_opt error"); 
        return -1; 
    } 
	CPLD_IO_Init();
	while(1)
  	{
   		while((nread = read(fd,buff,2040))>0)
   		{
      		buff[nread+1]='\0';
			
			//如果能够获取到GSP时间
			if(strstr(buff,"$GPRMC") != NULL)
			{
				printf("\nreceive GPS time signal succeed!!!\n");
				get_and_set_current_date(buff);
			}
			//如果无法收到GPS时间信号
			else if(strstr(buff,"$GPGLL,,,,,,V,N*64") != 0)
			{
				printf("\nCan not receive GPS time signal!!!\n");
				
			}
			usleep(350*1000);
			//灭灯
			ioctl(cpld_io_fd,0x0011);
   	 	}
		sleep(1);
  	}
} 