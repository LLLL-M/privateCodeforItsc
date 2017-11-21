#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include "msg.h"
#include "ftest.h"
#include "cpld.h"
//#include "canmsg.h"

extern unsigned char g_GpsFlag;
static int socketfd = -1;
static unsigned int g_tscid;
struct sockaddr_in g_wireless_servaddr;
struct sockaddr_in g_keyboard_servaddr;
struct sockaddr_in g_ioInput_servaddr;
int gTsctype = 0;
int createUdpSocket(int port)
{
	struct sockaddr_in addr = 
	{
		.sin_family = PF_INET,
		.sin_addr = INADDR_ANY,
		.sin_port = htons(port),
		.sin_zero = {0},
	};
	socklen_t len = sizeof(struct sockaddr);
	int opt = 1;
	int sockfd = -1;

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		printf("create socket fail, error info: %s\n", strerror(errno));
		return -1;
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(int));
	if (bind(sockfd, SA(addr), len) == -1) 
	{
		printf("bind socket fail, error info:%s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	return sockfd;
}
void sendResponse(int fd, ST_UDP_MSG *msg, int datalen, struct sockaddr_in *from)
//void sendResponse(int fd, char*msg, int datalen, struct sockaddr_in *from)
{
	if(fd == -1 || msg == NULL || from == NULL)
	{
		printf("params error.\n");
		return;
	}
	//printf("send response to %s\n", inet_ntoa(from->sin_addr));
	sendto(fd, msg, 12+datalen, 0, (struct sockaddr*)from, sizeof(struct sockaddr_in));
}

void sendMessage(char msgtype, char *data, int datalen, struct sockaddr_in *addr)
{
	ST_UDP_MSG msg;
	bzero(&msg, sizeof(ST_UDP_MSG));
	msg.uVer = FTEST_PRO_VER2;
	msg.uHead = FTEST_UDP_HEAD;
	msg.uTscid = g_tscid;
	msg.uType = msgtype;
	if(data != NULL)
		memcpy((char *)msg.data, data, datalen);
	sendResponse(socketfd, &msg, datalen, addr);
}

int main(int argc, char *argv[])
{
	int ret = -1;
	ST_UDP_MSG *udpreq = NULL;
	struct sockaddr_in fromAddr;
	socklen_t fromLen = sizeof(fromAddr);
	//char connFlag = 0;
	//int tsctype = 0;
	g_tscid = ((unsigned int)&ret)%99999+1;
	int len = 0;
	
	if(!(udpreq = malloc(sizeof(ST_UDP_MSG))))
	{
		printf("malloc for udpreq error!\n");
		return -1;
	}
	memset(udpreq, 0, sizeof(ST_UDP_MSG));
	socketfd = createUdpSocket(UDP_SOCK_PORT);
	if(-1 == socketfd)
	{
		printf("create udp socket error!");
		free(udpreq);
		return -1;
	}
	printf("TSCID: %d\n", g_tscid);
	while(1)
	{
		bzero(&fromAddr, sizeof(fromAddr));
		memset(udpreq, 0, sizeof(ST_UDP_MSG));
		if(-1 == (ret = recvfrom(socketfd, udpreq, sizeof(ST_UDP_MSG), 0, (struct sockaddr*)&fromAddr, &fromLen)))
		{
			printf("UDP RECV: recv data error - %s\n", strerror(errno));
		}
		else if(FTEST_PRO_VER2 == udpreq->uVer && FTEST_UDP_HEAD == udpreq->uHead){
			if(g_tscid != udpreq->uTscid && udpreq->uType != FTEST_MSG_CONNECT)
			{
				printf("Please connect device first!\n");
				udpreq->data[0] = MSG_EX_FAIL;
				sendResponse(socketfd, udpreq, 4, &fromAddr);
				
				udpreq->uTscid = g_tscid;
				udpreq->uType = FTEST_MSG_REQ_RECONNECT;
				sendResponse(socketfd, udpreq, 0, &fromAddr);
				continue;
			}
			
			//printf("recv type: %x, data:%s, len:%d\n", udpreq->uType, (char *)udpreq->data, ret);
			//memset((char*)udpreq.data, 0, 1024*8);
			switch(udpreq->uType)
			{
				case FTEST_MSG_CONNECT:
					if((gTsctype = get_tsc_type()) == udpreq->data[0])
					{
						Ftest_init();
						memset((char*)udpreq->data, 0, 10240);
						udpreq->uTscid = g_tscid;
						udpreq->data[0] = MSG_EX_SUCC;
						len = 4;
						//memcpy((char *)udpreq.data, "req allowed", 11);
						//sendResponse(socketfd, &udpreq, 11, &fromAddr);
					}
					else
					{
						memset((char*)udpreq->data, 0, 10240);
						udpreq->uTscid = g_tscid;
						udpreq->data[0] = MSG_EX_FAIL;					
						udpreq->data[1] = gTsctype;
						len = 8;
					}
					sendResponse(socketfd, udpreq, len, &fromAddr);
					//connFlag = 1;
					break;
				case FTEST_MSG_USB://300 & 500 test
					memset((char*)udpreq->data, 0, 10240);
					if(0 == usb_test((char *)&udpreq->data[1]))
						udpreq->data[0] = MSG_EX_FAIL;
					else
						udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4+strlen((char *)&udpreq->data[1]), &fromAddr);
					break;			
				case FTEST_MSG_RS232:// 500 test
					memset((char*)udpreq->data, 0, 10240);
					if(0 == RS232_test((char *)&udpreq->data[1]))
						udpreq->data[0] = MSG_EX_FAIL;
					else
						udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4+strlen((char *)&udpreq->data[1]), &fromAddr);				
					break;					
				case FTEST_MSG_RS422:// 500 test
					memset((char*)udpreq->data, 0, 10240);
					if(0 == RS422_test((char *)&udpreq->data[1]))
						udpreq->data[0] = MSG_EX_FAIL;
					else
						udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4+strlen((char *)&udpreq->data[1]), &fromAddr);				
					break;				
				case FTEST_MSG_RS485://300 & 500
					memset((char*)udpreq->data, 0, 10240);
					if(0 == TSC_RS485_Test((char *)&udpreq->data[1]))
						udpreq->data[0] = MSG_EX_FAIL;
					else
						udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4+strlen((char *)&udpreq->data[1]), &fromAddr);				
					break;
				case FTEST_MSG_CURVOLT://300 & 500
					//memset((char*)udpreq->data, 0, 10240);
					if(0 == cur_Volt_Test((char *)&udpreq->data))
						udpreq->data[0] = MSG_EX_FAIL;
					else
						udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4+strlen((char *)&udpreq->data[1]), &fromAddr);
					break;				
				case FTEST_MSG_WIFI://300 & 500
					memset((char*)udpreq->data, 0, 10240);
					if(0 == wifi_state_check((char *)&udpreq->data[1]))
						udpreq->data[0] = MSG_EX_FAIL;
					else
						udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4+strlen((char *)&udpreq->data[1]), &fromAddr);
					break;
				case FTEST_MSG_AUTO:
					
					break;				
				case FTEST_MSG_LAMP://300 & 500
					light_on_inOrder(udpreq->data);
					//memset((char*)udpreq->data, 0, 10240);
					udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4, &fromAddr);
					break;
				case FTEST_MSG_FRONTBOARD: //300
					memset((char*)udpreq->data, 0, 10240);
					font_board_led_Test();
					udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4, &fromAddr);
					break;				
				case FTEST_MSG_GPS:
					if(udpreq->data[0]>0 && udpreq->data[0]<3)//1,2
						g_GpsFlag = (udpreq->data[0] == 2 ? 0: 1);
					
					memset((char*)udpreq->data, 0, 10240);
					udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4, &fromAddr);
					break;
				case FTEST_MSG_KEYBOARD:
					bzero(&g_keyboard_servaddr, sizeof(struct sockaddr_in));
					memcpy(&g_keyboard_servaddr, &fromAddr, sizeof(struct sockaddr_in));				
					Key_board_check_ctrl(udpreq->data[0]);
					udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4, &fromAddr);
					break;
				case FTEST_MSG_WIRELESS: //300
					//printf("--> recv wireless ctrl: %d\n", udpreq->data[0]);
					bzero(&g_wireless_servaddr, sizeof(struct sockaddr_in));
					memcpy(&g_wireless_servaddr, &fromAddr, sizeof(struct sockaddr_in));
					wireless_check_ctrl(udpreq->data[0]);//开启和关闭无线遥控器检测功能
					memset((char*)udpreq->data, 0, 10240);
					udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4, &fromAddr);
					break;
				case FTEST_MSG_WIRELESS_DATA:
					break;
				case FTEST_MSG_PEDKEY://300
					memset((char*)udpreq->data, 0, 10240);
					udpreq->data[0] = MSG_EX_SUCC;
					get_ped_key_status(&udpreq->data[1]);// int arr[8]
					sendResponse(socketfd, udpreq, 9*4, &fromAddr);
					//printf("status: %d,%d,%d,%d,%d,%d,%d,%d\n", udpreq.data[1], udpreq.data[2], udpreq.data[3], udpreq.data[4], udpreq.data[5], udpreq.data[6], udpreq.data[7], udpreq.data[8]);
					break;				
				case FTEST_MSG_IO_OUTPUT_H://500
					if(IO_OUT_High_TEST() == 0)
						udpreq->data[0] = MSG_EX_SUCC;
					else
						udpreq->data[0] = MSG_EX_FAIL; 
						
					sendResponse(socketfd, udpreq, 4,  &fromAddr);
					break;
				case FTEST_MSG_IO_OUTPUT_L://500
					if(IO_OUT_Low_TEST() == 0)
						udpreq->data[0] = MSG_EX_SUCC;
					else
						udpreq->data[0] = MSG_EX_FAIL; 
						
					sendResponse(socketfd, udpreq, 4,  &fromAddr);
					break;
				case FTEST_MSG_CAR_DETECTOR://500
					memset((char*)udpreq->data, 0, 10240);
					udpreq->data[0] = MSG_EX_SUCC;
					Get_CarDetector_state(&udpreq->data[1]);// int arr[48]
					printf("get car detector status...\n");
					sendResponse(socketfd, udpreq, 49*4, &fromAddr);
					break;
				case FTEST_MSG_IO_INPUT://500
					//printf("--> recv ioInput Check ctrl: %d\n", udpreq->data[0]);
					IOinput_Check_Ctrl(udpreq->data[0]);//开启和关闭无线遥控器检测功能
					bzero(&g_ioInput_servaddr, sizeof(struct sockaddr_in));
					memcpy(&g_ioInput_servaddr, &fromAddr, sizeof(struct sockaddr_in));
					memset((char*)udpreq->data, 0, 10240);
					udpreq->data[0] = MSG_EX_SUCC;
					sendResponse(socketfd, udpreq, 4, &fromAddr);
					break;
				case FTEST_MSG_APP_EXIT:
					Ftest_finished();
					break;
				case FTEST_MSG_TEST:
					printf("recv data:%s, len:%d\n", (char *)udpreq->data, ret);
					memset((char*)udpreq->data, 0, 10240);
					memcpy((char *)udpreq->data, "response send ", 13);
					sendResponse(socketfd, udpreq, 11, &fromAddr);				
					break;
				default:
					printf("unknown type msg.\n");
					break;
			}
			if(udpreq->uType == FTEST_MSG_APP_EXIT)
				break;
		}
		else
			printf("unknown ver or head msg.\n");
		
		usleep(10*1000);
	}
	
	free(udpreq);
	return 0;
}