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
**	FILE DESCRIPTION:���ڽ���GPSʱ����Ϣ����
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
* 	CPU��CPLD������ʼ��
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
* 	д��־�ļ�
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
		printf("������¼��Ϣ��Ч!\n");
		return -1;
	}
    
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("������¼�ļ�δ����ȷ��!\n");
        return -1;
    }
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("��ȡ������¼�ļ���Ϣʧ��!\n");
		return -1;
	}
	if(f_stat.st_size > 100*1024)
	{
		fclose(pFileDebugInfo);
		//���ļ���������0������д
		pFileDebugInfo = fopen(pFile, "w+");
	}

	//��ȡ���ʱ�׼ʱ��
	//time(&now);  
	  
	//ת��Ϊ����ʱ��
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
* 	��ָ������
*
***********************************************************************************/
int open_port(int fd,int comport) 
{ 
	char *dev[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"}; 
	long  vdisable; 
	//����1 
	if (comport==1)
	{
		fd = open( "/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     } 
	 //����2 
     else if(comport==2)
     {     
		fd = open( "/dev/ttyS1", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     } 
	 //����3 
     else if (comport==3)
     { 
		fd = open( "/dev/ttyS2", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     } 
	 //����4 
	 else if (comport==4)
     { 
		fd = open( "/dev/ttyS3", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     } 
	 //����5 
	 else if (comport==5)
     { 
		fd = open( "/dev/ttyS4", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     } 
	 //����6 
	 else if (comport==6)
     { 
		fd = open( "/dev/ttyS5", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     } 
	 //����7 
	 else if (comport==7)
     { 
		fd = open( "/dev/ttyS6", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == fd)
		{ 
			perror("Can't Open Serial Port"); 
			return(-1); 
		} 
     } 
	 //�ָ�����Ϊ����״̬
     if(fcntl(fd, F_SETFL, 0)<0) 
	 {
		printf("fcntl failed!\n"); 
	 }
     else
	 {
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	 }
	 //�����Ƿ�Ϊ�ն��豸 
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
* 	���ô�����ز���
*
***********************************************************************************/
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop) 
{ 
     struct termios newtio,oldtio; 
	 //����������д��ڲ������ã�������������ںŵȳ���������صĳ�����Ϣ
     if( tcgetattr( fd,&oldtio) != 0) 
	 {  
        perror("SetupSerial 1"); 
		return -1; 
     } 
     bzero( &newtio, sizeof( newtio ) ); 
	 //����һ�������ַ���С
     newtio.c_cflag  |=  CLOCAL | CREAD;  
     newtio.c_cflag &= ~CSIZE;  
	 //����ֹͣλ 
     switch( nBits ) 
     { 
		case 7: 
			newtio.c_cflag |= CS7; 
			break; 
		case 8: 
			newtio.c_cflag |= CS8; 
			break; 
     } 
	 //������żУ��λ 
     switch( nEvent ) 
     { 
		case 'O': //���� 
			newtio.c_cflag |= PARENB; 
			newtio.c_cflag |= PARODD; 
			newtio.c_iflag |= (INPCK | ISTRIP); 
			break; 
		case 'E': //ż�� 
			newtio.c_iflag |= (INPCK | ISTRIP); 
			newtio.c_cflag |= PARENB; 
			newtio.c_cflag &= ~PARODD; 
			break; 
		case 'N':  //����żУ��λ 
			newtio.c_cflag &= ~PARENB; 
			break; 
     } 
     //���ò����� 
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
	 /*����ֹͣλ*/ 
     if( nStop == 1 ) 
      newtio.c_cflag &=  ~CSTOPB; 
     else if ( nStop == 2 ) 
      newtio.c_cflag |=  CSTOPB; 
	/*���õȴ�ʱ�����С�����ַ�*/ 
     newtio.c_cc[VTIME]  = 0; 
     newtio.c_cc[VMIN] = 0; 
	/*����δ�����ַ�*/ 
     tcflush(fd,TCIFLUSH); 
	/*����������*/ 
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
* 	��ȡGPS���ݵ�ǰʱ����Ϣ
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
	//��ȡ����ʱ����Ϣ����
	if(tmpstr1 != NULL && strlen(tmpstr1) >= 68)
	{	
		//����
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

		//��GPS��Ϣ�н�ȡ��������Ϣ
		memcpy(datestr,tmpstr1 + pos,6);

		//printf("day-month-year:%s\n",datestr);
		//��ʱʱ����Ϣ��Ч
		memcpy(tmpstr2,datestr+4,2);       //��ȡ��
		curdate.tm_year = 2000 + atoi(tmpstr2) - 1900;
		memcpy(tmpstr2,datestr+2,2);       //��ȡ��
		curdate.tm_mon = atoi(tmpstr2) - 1;
		memcpy(tmpstr2,datestr,2);       //��ȡ��
		curdate.tm_mday = atoi(tmpstr2);
		memcpy(tmpstr2,tmpstr1+6,2);       //��ȡʱ
		curdate.tm_hour = atoi(tmpstr2);
		memcpy(tmpstr2,tmpstr1+8,2);       //��ȡ��
		curdate.tm_min = atoi(tmpstr2);
		memcpy(tmpstr2,tmpstr1+10,2);       //��ȡ��
		curdate.tm_sec = atoi(tmpstr2);		

		//�����Թ��ʱ�׼ʱ��������
		time1 = mktime(&curdate); 
		time1 = time1 +8 *60 *60;
		//ת���ɱ���ʱ��
		tmsptr = localtime(&time1);				
		printf("\nCurrent Time =%04d.%02d.%02d-%02d:%02d:%02d\n",tmsptr->tm_year + 1900,tmsptr->tm_mon + 1,tmsptr->tm_mday,
				tmsptr->tm_hour,tmsptr->tm_min,tmsptr->tm_sec);
		
		//�жϸ�GPS��Ϣ�Ƿ�Ϊ��Ч����
		memset(tmpstr2,0,128);
		memcpy(tmpstr2,tmpstr1+16,1);       //��ȡGPS�Ƿ���Ч��Ϣ

		if(strcmp(tmpstr2,"A") != 0 && strcmp(tmpstr2,"V") != 0)
		{
			//GPS��Ϣ��Ч����ֱ���˳�
			printf("GPS signal is invalid %s!!!\n",tmpstr2);
			return 0;
		}

		//��ȡ��ǰʱ��
		time(&scttime);
		printf("time period = %d\n",abs(scttime -time1));
	
		//��ʵ��ʱ��ƫ���2�룬��Уʱ
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
* 	GPSУʱ���������
*
***********************************************************************************/
int main(void)  
{ 
    int fd = 0; 
    int nread = 0;
	int nwrite = 0;
	int i = 0; 
    char buff[2048]=""; 
	//�򿪴��� 
    if((fd=open_port(fd,7))<0)
	{
		
        perror("open_port error"); 
        return -1; 
    } 
	//���ô��� 
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
			
			//����ܹ���ȡ��GSPʱ��
			if(strstr(buff,"$GPRMC") != NULL)
			{
				printf("\nreceive GPS time signal succeed!!!\n");
				get_and_set_current_date(buff);
			}
			//����޷��յ�GPSʱ���ź�
			else if(strstr(buff,"$GPGLL,,,,,,V,N*64") != 0)
			{
				printf("\nCan not receive GPS time signal!!!\n");
				
			}
			usleep(350*1000);
			//���
			ioctl(cpld_io_fd,0x0011);
   	 	}
		sleep(1);
  	}
} 