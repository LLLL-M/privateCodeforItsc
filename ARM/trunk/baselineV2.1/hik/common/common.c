#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include "hik.h"
/*********************************************************************************
*
* 	��ָ������
*
***********************************************************************************/
int open_port(int comport) 
{ 
    char cDev[64] = {0};
    int fd = 0;
    sprintf(cDev,"/dev/ttyS%d",comport-1);

    if((fd = open(cDev, O_RDWR|O_NOCTTY|O_NDELAY)) == -1)
    {
        ERR("Error to open %s\n",cDev);
        return 0;
    }
	 //�ָ�����Ϊ����״̬
    if(fcntl(fd, F_SETFL, 0) < 0) 
	{
		ERR("fcntl failed!\n"); 
	}
	 //�����Ƿ�Ϊ�ն��豸 
    if(isatty(STDIN_FILENO) == 0) 
	{
		ERR("standard input is not a terminal device\n"); 
	}

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
	 
    if( tcgetattr( fd,&oldtio) != 0)//����������д��ڲ������ã�������������ںŵȳ���������صĳ�����Ϣ 
    {  
        ERR("error to SetupSerial : %s \n",strerror(errno)); 
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
 	
 	return 0; 
} 

