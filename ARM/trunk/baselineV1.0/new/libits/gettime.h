#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#ifndef _SVID_SOURCE
#define _SVID_SOURCE    //����glibc2�汾,����stime()��Ҫ��������꣬
#endif
#include <time.h>

/*	GPSģ����������ݸ�ʽ������ʾ:
$GPRMC,085551.00,A,2505.71988,N,10454.79941,E,0.029,,111215,,,A*7B
$GPVTG,,T,,M,0.029,N,0.053,K,A*2E
$GPGGA,085551.00,2505.71988,N,10454.79941,E,1,11,1.03,1175.8,M,-26.7,M,,*4D
$GPGSA,A,3,15,31,10,24,14,18,25,21,20,22,12,,2.47,1.03,2.25*0C
$GPGSV,3,1,11,10,54,340,48,12,25,097,35,14,44,298,42,15,11,066,22*78
$GPGSV,3,2,11,18,72,053,39,20,18,121,23,21,43,195,38,22,48,329,50*73
$GPGSV,3,3,11,24,37,040,31,25,25,141,37,31,18,214,33*41
$GPGLL,2505.71988,N,10454.79941,E,085551.00,A,A*6E
*/
static time_t GetGPSTime(int fd) 
{
	char buf[1024] = {0};
	struct tm now;
	time_t timep = 0;
	char *start, *end;
	
	read(fd, buf, sizeof(buf));
	start = strstr(buf, "GPRMC");
	end = strchr(buf, '*');
	//fprintf(stderr, "GPS data:\n%s", buf);
	
	if(strstr(buf, "$GPGLL,,,,,,V,N*64") != NULL	//�޷��յ�GPS�ź�
		|| start == NULL || strlen(start) < 68		//��ȡ����ʱ����Ϣ������
		|| end == NULL || (end - start) <= 10
		|| (start[16] != 'A' && start[16] != 'V'))
		return 0;

	if (3 != sscanf(end - 10, "%2d%2d%2d", &now.tm_mday, &now.tm_mon, &now.tm_year)//��ȡ������
		|| 3 != sscanf(start+6, "%2d%2d%2d", &now.tm_hour, &now.tm_min, &now.tm_sec))//��ȡʱ����
		return 0;
	now.tm_year += 2000 - 1900;	//������һЩ����
	now.tm_mon--;	//�·ݴ�0��ʼ������Ҫ��1
	timep = mktime(&now) + 8 * 3600;	//����8��ʱ����Ϊ����ʱ��
	if (time(NULL) != timep)
	{
		stime(&timep);	//����ϵͳʱ��
		//system("hwclock -w");
	}
	return timep;
}

/* ��������Ϊ:
	�����ʣ�9600�� 8bit
	ֹͣλ��1
	��żУ��λ��N	*/
static int OpenGpsPort() 
{ 
	struct termios options;
	int fd = open( "/dev/ttyS6", O_RDWR | O_NOCTTY | O_NDELAY);
	
	if (fd == -1)
	{
		printf("open /dev/ttyS6 fail!\n");
		return -1;
	}
#if 0
	//�����Ƿ�Ϊ�ն��豸 
	if(isatty(STDIN_FILENO) == 0) 
	{
		printf("standard input is not a terminal device\n"); 
		close(fd);
		return -1;
	}
#endif
	//����������д��ڲ������ã�������������ںŵȳ���������صĳ�����Ϣ
    if(tcgetattr(fd, &options) != 0) 
	{  
        printf("get serial options error\n");
		close(fd);
		return -1; 
    }
	//����CR�س���
	options.c_iflag |= IGNCR;
	//ʹ�ô��ڷǹ淶ģʽ
	options.c_lflag &= ~ICANON;
	//���ñ��ؽ���ģʽ
	options.c_cflag |= CLOCAL | CREAD;
	//����8bit����λ
	options.c_cflag &= ~CSIZE; 
	options.c_cflag |= CS8;
	//��������żУ��λ 
	options.c_cflag &= ~PARENB;
	//����9600������
	cfsetispeed(&options, B9600); 
	cfsetospeed(&options, B9600); 	 
	/*����ֹͣλ1*/
	options.c_cflag &= ~CSTOPB; 
	/*���ó�ʱ�ȴ�ʱ�����С�����ַ�*/ 
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;
	/*����������*/ 
	if((tcsetattr(fd, TCSAFLUSH, &options)) != 0)
	{ 
		printf("serial attr set failed!\n");
		close(fd);
		return -1;
	}
	else
	{
		printf("serial attr set successful!\n");
		return fd;
	}
}

static time_t GetLocalTime(void)
{
	static int fd = -1;
	time_t timep = 0;
	
	if (fd == -1)
		fd = OpenGpsPort();
	if (fd > 0)
	{	//����ʹ��GPS��ȡʱ��
		timep = GetGPSTime(fd);
	}
	if (timep == 0)	
	{	//���GPS��ȡ����ʱ����ʹ��ϵͳʱ��
		timep = time(NULL);
	}
	return timep;
}
