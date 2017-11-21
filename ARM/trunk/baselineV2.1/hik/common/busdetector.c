#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>       /* basic system data types */
#include <sys/socket.h>      /* basic socket definitions */
#include <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <fcntl.h>     /* nonblocking */
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "canmsg.h"
#include "platform.h"
#include "HikConfig.h"
#include "configureManagement.h"
#include "stream.h"


#define BUSDETECTOR_PORT 17202
#define MAX_BUSDETECTOR_NUM 128

////////////////rfid data struct define////////////////////
#define BEGIN_CODE 0xffffffff
#define PROTOCOL_CODE_TCP 
#define PROTOCOL_CODE_485 

enum ProtocolCode
{
	HOST_BROADCAST_485 = 0x0000,
	READER_BROADCAST_485 = 0xc101,
	READER_ENDTOEND = 0xc102,
	READER_BROADCAST_UDP = 0xc103,
};
typedef enum DataType
{
	PSD_TAG_DATA=1,
	PSD_HEART_DATA,
	PSD_TIMING_DATA,
	PSD_SEARCH, 
	PSD_VERSION
}DataType;

/*帧选项
b7
0b：上位机到阅读器帧
1b：阅读器到上位机帧	
b6
0b：广播帧
1b：点对点帧
b5
0b：普通帧
1b：系统确认帧	
b4
0b：上传新数据
1b：重传上一包数据 	
b3~b0 保留
*/

typedef struct OriginalData
{
	DataType type;
	BYTE byData[256];
	INT32 nLength;
}OriginalData;
			

typedef struct sutTagRecord 
{
	UINT32 nTagID;//标签编号
	UINT32 nDeviceAddress;//阅读器地址
	UINT32 bTagType;//0:标签记录，心跳记录	
	UINT32 nUserDefinition;//用户自定义
	UINT32 nOldTriggerID;//旧触发器地址
	UINT32 nNewTriggerID;//新触发器地址
	UINT32 nTime;//时间
	BYTE bRSSI;//rssi值
	BYTE bElecStatus;//电量状态0 :充足1 ：不足
	BYTE bAlarm;//报警标志，，报警，0，正常
	BYTE nNumber;//在阅读器过滤时间内接收到的次数	

}sutTagRecord;

typedef struct sutHeartRecord 
{
	UINT16 bRecordType;//0:标签记录，1心跳记录
	UINT16 nDeviceAddress;//阅读器地址
	UINT32 nTime;//时间
}sutHeartRecord;

typedef struct rfidTagBits
{//可以设置tag数最多128个
	UINT64 nLow;
	UINT64 nHigh;
}RFIDTagBits;
enum commandCode
{
	CLEAR_TAG_DATA = 0x03,
	HEART_DATA = 0x05,
	TIMING_DATA = 0x06,
	UPLOAD_TAGS = 0x08,
};

/////////////////////////////////////////////////////


extern SignalControllerPara *gSignalControlpara;
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;

UINT32 gBusDetectorData[4];  //bus detector data 128bits
UINT32 gSpecialCarData[4];   //special car detector data 128bits

const unsigned short crc_ta[16]={0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,0x8108,0x9129,
0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef};
unsigned short Crc16_DATAs(unsigned char *ptr, unsigned short len) 
{
	unsigned char da;   
	unsigned short  CRC_16_Data = 0;
	while(len-- != 0)
	{
		da = CRC_16_Data >> 12; 
		CRC_16_Data <<= 4; 
		CRC_16_Data ^= crc_ta[da^(*ptr/16)];                                   
		da = CRC_16_Data >> 12; 
		CRC_16_Data <<= 4; 
		CRC_16_Data ^= crc_ta[da^(*ptr&0x0f)]; 
		ptr++;
	}
	return  CRC_16_Data;
}


static int CreateTCPConnect(UInt16 port)
{
	struct sockaddr_in addr = 
    {   
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
        .sin_zero = {0},
    };
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int opt = 1;
	
	if (sockfd == -1)
	{
		perror("can't create tcp socket!");
		return -1;
	}
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt SO_REUSEADDR fail");
		close(sockfd);
        return -1;
	}
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1) 
	{
		perror("set tcp sockfd nonblock fail!");
		close(sockfd);
        return -1;
    }
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1)
    {
        perror("tcp sockfd bind error!");
		close(sockfd);
        return -1;
    }
	if (listen(sockfd, 32) == -1) 
    {
        perror("listen error");
		close(sockfd);
        return -1;
    }
	return sockfd;
}

int readData(int fd, UINT8* buf, int size)
{
	int len = 0;
	int left = 0;
	int ret = 0;
	if (buf == NULL || size <= 0)
		return 0;
	left = size;
	
	if (left > 0)
	{
		ret = read(fd, buf + len, left);
		if (ret > 0)
		{
			len += ret;
			left -= ret;
		}
		else if (ret == -1)// error
		{
			if (!(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			{
				close(fd);
			}
		}
		else if (ret == 0)
		{
			close(fd);
		}
	}
	return len;
}

int SplitData(UINT8* data, UINT32 size, OriginalData* orgData, UINT8 orgCount)
{
	UINT8 command = 0;
	UINT8 frm_status = 0;
	UINT8 i = 0;
	
	if (data == NULL || size <= 0)
		return 0;
	if (ntohs(*((UINT16*)data)) != Crc16_DATAs(data + 2,size - 2))//data crc check failed
		return 0;
	memset(orgData, 0, sizeof(OriginalData) * orgCount);
	frm_status = *(data + 2);
	if (frm_status != 0xc0)//reader to hostctrl, endtoend frame
		return 0;
	command = *(data + 3);

	switch(command)
	{
	case UPLOAD_TAGS:
		orgData[i].type = PSD_TAG_DATA;
		break;
	case HEART_DATA:
		orgData[i].type = PSD_HEART_DATA;
	case TIMING_DATA:
		orgData[i].type = PSD_TIMING_DATA;
	}
	orgData[i].nLength = size;
	memcpy(orgData[i].byData, data, size);
	i++;
	return i;
}

void analysTagData(UINT8* byData, sutTagRecord* tagRec)
{
	UINT8 status = 0;
	UINT32 tag = 0;
	UINT32 tim = 0;
	tagRec->bTagType = *((UINT8*)(byData));
	memcpy(&tag, byData + 1, 4);
	tagRec->nTagID =  ntohl(tag);
	tagRec->bRSSI = *((UINT8*)(byData + 5));
	tagRec->nNewTriggerID = ntohs(*((UINT16*)(byData + 6)));
	tagRec->nOldTriggerID = ntohs(*((UINT16*)(byData + 8)));
	tagRec->nUserDefinition = ntohl(*((UINT32*)(byData + 10)));
	status =  *((UINT8*)(byData + 14));
	tagRec->bElecStatus = GET_BIT(status, 7);
	tagRec->bAlarm = GET_BIT(status, 6);
	tagRec->nNumber = *((UINT8*)(byData + 15));
	memcpy(&tim, byData+16, 4);
	tagRec->nTime = ntohl(tim);
}

//map the bus tag data to busdetector bits
void SetBusDetectorData(sutTagRecord* tagRec, UINT8 tagRecCount, UINT32* busDetectorData)
{
	int i = 0, j = 0;
	
	if (tagRec == NULL || tagRecCount <= 0 || busDetectorData == NULL)
		return;
	//INFO("byBusTagID=%d  %d", gStructBinfileCustom.sBusDetector[0].byBusTagID, 
		//gStructBinfileCustom.sBusDetector[1].byBusTagID);
	for (i = 0; i < tagRecCount; i++)
	{
	INFO("rfid tagID=%d", tagRec[i].nTagID);
			for (j = 0; j < MAX_BUSDETECTOR_NUM; j++)
			{
				if (gStructBinfileCustom.sBusDetector[j].byBusTagID == tagRec[i].nTagID)
				{
					SET_BIT(busDetectorData[j / 32], j % 32);
					break;
				}
			}
	}
	INFO("bus detector databits=%08X", busDetectorData[0]);
	return;
}
//map the special car tag to special car detector bits
void SetSpecialCarDetectorData(sutTagRecord* tagRec, UINT8 tagRecCount, UINT32* scarDetectorData)
{
	int i = 0, j = 0;
	
	if (tagRec == NULL || tagRecCount <= 0 || scarDetectorData == NULL)
		return;
	for (i = 0; i < tagRecCount; i++)
	{
			for (j = 0; j < MAX_SCARDETECTOR_NUM; j++)
			{
				if (gStructBinfileCustom.sSpecialCarDetector[j].bySCarTagID == tagRec[i].nTagID)
				{
					SET_BIT(scarDetectorData[j / 32], j % 32);
				}
			}
	}
	return;
}

int ParseData(OriginalData* orgData,
					UINT8* response, UINT32* resLen)
{
	UINT8 command = 0;
	UINT16 readerAddr = 0;
	UINT8 frm_status = 0;
	UINT16 frm_seq = 0;
	UINT8 num = 0, i = 0;
	UINT8 len = 0;
	sutTagRecord tagRec[12];
	sutHeartRecord heartRec;
	UINT8 * data = NULL;
	time_t now = 0;
	UINT16 crc16 = 0;
	if (orgData == NULL)
		return 0;
	data = orgData->byData;
	//if (*((UINT16*)data) != Crc16_DATAs(data + 1,orgData->nLength - 1))//data crc check failed
		//return 0;
	
	frm_status = *(data + 2);
	if (frm_status != 0xc0)//reader to hostctrl, endtoend frame
		return 0;
	command = *(data + 3);
	readerAddr = ntohs(*((UINT16*)(data + 4)));
	//INFO("frm_status=%X command=%X, readeraddr=%04X", frm_status, command, readerAddr);
	memset(tagRec, 0, sizeof(sutTagRecord)*12);
	memset(&heartRec, 0, sizeof(sutHeartRecord));
	switch (command)
	{
	case UPLOAD_TAGS:
		frm_seq = ntohs(*((UINT16*)(data + 6)));
		num = *(data + 8);
		UINT8* p = data + 9;
		//INFO("frm_seq=%04X, Tags num=%d :", frm_seq, num);
		for (i = 0; i < num && i < 12; i++)
		{
			tagRec[i].nDeviceAddress = readerAddr;
			analysTagData(p, &tagRec[i]);
			//INFO("  Reader=%d, TagID=%04X, Number=%d, Time=%d", 
				//tagRec[i].nDeviceAddress, tagRec[i].nTagID, tagRec[i].nNumber, tagRec[i].nTime);
			p += 20;
		}
		SetBusDetectorData(tagRec, num, gBusDetectorData);
		if (gStructBinfileCustom.specialCarCheckSwitch == 1)//enable specialcar detect
			SetSpecialCarDetectorData(tagRec, num, gSpecialCarData);
		break;
	case HEART_DATA:
		heartRec.nDeviceAddress = readerAddr;
		heartRec.bRecordType = 1;
		heartRec.nTime = ntohs(*((UINT16*)(data + 6)));
		INFO("Heart Data time gap=%d", heartRec.nTime);
		break;
	case TIMING_DATA:
		INFO("TIMING_DATA");
		break;
	case CLEAR_TAG_DATA:
		//if (*((UINT8*)(data + 16)) == 0x00)//success
		
		break;
	}

	//packet response
	
	switch (command)
	{
	case UPLOAD_TAGS:
		len = 0;
		memset(response + len, 0xff, 4);
		len += 6;
		*((UINT16*)(response + len)) = htons(0xc102);
		len += 2;
		*((UINT16*)(response + len)) = htons(0x0009);
		len += 2;
		*((UINT16*)(response + 4)) = htons(Crc16_DATAs(response + 6, 4));
		//INFO("%02X%02X", *(response + 4), *(response + 5));
		len += 2;
		*((UINT8*)(response + len)) = 0x40;
		len++;
		*((UINT8*)(response + len)) = UPLOAD_TAGS;
		len++;
		*((UINT16*)(response + len)) = htons(readerAddr);
		len += 2;
		*((UINT16*)(response + len)) = htons(frm_seq);
		len += 2;
		*((UINT8*)(response + len)) = num;
		len++;
		*((UINT16*)(response + 10)) = htons(Crc16_DATAs(response + 12, len - 12));
		break;
	case HEART_DATA:
		len = 0;
		memset(response + len, 0xff, 4);
		len += 6;
		*((UINT16*)(response + len)) = htons(0xc102);
		len += 2;
		*((UINT16*)(response + len)) = htons(0x0006);
		len += 2;
	    *((UINT16*)(response + 4)) = htons(Crc16_DATAs(response + 6, 4));
		
		len += 2;
		*((UINT8*)(response + len)) = 0x40;
		len++;
		*((UINT8*)(response + len)) = HEART_DATA;
		len++;
		*((UINT16*)(response + len)) = htons(readerAddr);
		len += 2;
		*((UINT16*)(response + 10)) = htons(Crc16_DATAs(response + 12, len - 12));
		break;
	case TIMING_DATA:
		len = 0;
		memset(response + len, 0xff, 4);
		len += 6;
		*((UINT16*)(response + len)) = htons(0xc102);
		len += 2;
		*((UINT16*)(response + len)) = htons(0x000a);
		len += 2;
		*((UINT16*)(response + 4)) = htons(Crc16_DATAs(response + 6, 4));
		len += 2;
		*((UINT8*)(response + len)) = 0x40;
		len++;
		*((UINT8*)(response + len)) = TIMING_DATA;
		len++;
		*((UINT16*)(response + len)) = htons(readerAddr);
		len += 2;
		time(&now);
		*((UINT32*)(response + len)) = htonl(now);
		len += 4;
		*((UINT16*)(response + 10)) = htons(Crc16_DATAs(response + 12, len - 12));
		//memcpy(((UINT16*)(response + 10)), &crc16, 2); 
		break;
		default:
			len = 0;
			break;
	}
	*resLen = len;
	return num;
}


void ProcessBusData(int fd)
{
	UINT8 connect = 1;
	UINT8 buf[1024] = {0};
	UINT8 response[256] = {0};
	UINT32 resLen = 0;
	OriginalData orgData[12] = {{0}};
	UINT8 i = 0;
	UINT16 dataLen = 0;
	//UINT16 protocol_code = 0;
	INT32 ret = 0;

	
		INT32 headLen = 10;
		memset(buf, 0, 1024);
		memset(response, 0, 256);
		resLen = 0;
		//read head data and parse data length;
		//INFO("Read rfid message head, fd = %d", fd);
		ret = readData(fd, buf, 1024);//recv(fd, buf, 1024, 0);
		//if (readData(fd, buf, headLen) != 10)
			//return;
		//for (i = 0; i < ret; i++)
			//fprintf(stderr, "%02X ", *(buf + i));
		//todo parse head, data length
		if (buf[0] == 0xff && buf[1] == 0xff && buf[2] == 0xff && buf[3] == 0xff)//begin with 0xffffffff
		{
			UINT16 crc = ntohs(*((unsigned short*)(buf + 4)));
			if (crc != Crc16_DATAs(buf + 6, 4))
			{
				INFO("crc16 check failed");
				return;	
			}
			//protocol_code = *((short*)(buf + 6));
			dataLen = ntohs(*((unsigned short*)(buf + 8)));
			if ((ret - headLen) < dataLen)
				//recv(fd, buf + ret, dataLen + headLen - ret, 0);
				ret = readData(fd, buf + ret, dataLen + headLen - ret);
			//if (ret <= 0)
				//return;
			//INFO("split data and parse, dataLen = %d", dataLen);
			//SplitData(buf + headLen, dataLen, orgData, 12);
			memcpy(orgData[0].byData, buf + headLen, dataLen);
			ParseData(&orgData[0], response, &resLen);
			//for (i = 0; i < resLen; i++)
				//fprintf(stderr, "%02X ", *(response + i));
			if (resLen > 0)//response len > 0 ,need response data
				send(fd, response, resLen, 0);
		}
	return;
	
}

void SendClearTagData(UINT32 fd, UINT16 readerAddr)
{
	UINT8 buf[256] = {0};
	UINT16 len = 0;

	memset(buf + len, 0xff, 4);
	len += 6;
	*((UINT16*)(buf + len)) = 0xC102;
	len += 2;
	*((UINT16*)(buf + len)) = 0x06;
	len += 2;
	*((UINT16*)(buf + 4)) = Crc16_DATAs(buf + 6, 4);
		
	len += 2;
	*((UINT8*)(buf + len)) = 0x40;
	len++;
	*((UINT8*)(buf + len)) = CLEAR_TAG_DATA;
	len++;
	*((UINT16*)(buf + len)) = readerAddr;
	len += 2;
	*((UINT16*)(buf + 10)) = Crc16_DATAs(buf + 12, len - 12);
	
}

#if 0
void *BusDetectorCom(void *arg)
{
	int srvfd, clifd;
	struct epoll_event ev, events[32];
	int epollfd = -1;
	int i, nfds;
	struct sockaddr clientaddr;
	socklen_t socklen = sizeof(struct sockaddr);
	
	srvfd = CreateTCPConnect(BUSDETECTOR_PORT);
	if (srvfd == -1)
		pthread_exit(NULL);
	
	INFO("begin listen RFID message...");
	while (1)
	{
		
				clifd = accept(srvfd, &clientaddr, &socklen);
				if (clifd == -1)
					continue;
				fcntl(clifd, F_SETFL, fcntl(clifd, F_GETFL, 0) | O_NONBLOCK);
				ev.data.fd = clifd;
				ev.events = EPOLLIN;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clifd, &ev) == -1)
				{
					perror("add clifd to epoll error!");
					close(clifd);
				}
				else
					INFO("epoll add fd %d", clifd);
			
				ProcessBusData(events[i].data.fd);
			
		}
	}
	
	close(srvfd);
	close(epollfd);
	pthread_exit(NULL);
}
#endif

void *BusDetectorCom(void *arg)
{
	int srvfd, clifd;
	struct epoll_event ev, events[32];
	int epollfd = -1;
	int i, nfds;
	struct sockaddr clientaddr;
	socklen_t socklen = sizeof(struct sockaddr);
	
	srvfd = CreateTCPConnect(BUSDETECTOR_PORT);
	if (srvfd == -1)
		pthread_exit(NULL);
	epollfd = epoll_create(32);
	if (epollfd == -1)
	{
		perror("create epoll fail!");
		close(srvfd);
		pthread_exit(NULL);
	}
	ev.data.fd = srvfd;
	ev.events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, srvfd, &ev) == -1)
	{
		perror("add srvfd to epoll error!");
		close(srvfd);
		close(epollfd);
	}
	memset(events, 0, sizeof(events));
	INFO("begin listen RFID message...");
	while (1)
	{
		nfds = epoll_wait(epollfd, events, 32, -1);
		//INFO("nfds = %d", nfds);
		if (nfds == -1)
		{
			usleep(1000000);
			continue;
		}
		for (i = 0; i < nfds; i++)
		{
			if((events[i].events & EPOLLRDHUP)
				|| (!(events[i].events & EPOLLIN)))
			{
				ERR("epoll delete fd %d, events: %#x", events[i].data.fd, events[i].events);
				//StreamClose(events[i].data.fd);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				close(events[i].data.fd);
			}
			else if (events[i].data.fd == srvfd)
			{
				clifd = accept(srvfd, &clientaddr, &socklen);
				if (clifd == -1)
					continue;
				fcntl(clifd, F_SETFL, fcntl(clifd, F_GETFL, 0) | O_NONBLOCK);
				ev.data.fd = clifd;
				ev.events = EPOLLIN;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clifd, &ev) == -1)
				{
					perror("add clifd to epoll error!");
					close(clifd);
				}
				else
					INFO("epoll add fd %d", clifd);
			}
			else if ((events[i].events & EPOLLIN))
			{
				ProcessBusData(events[i].data.fd);
			}
		}
	}
	
	close(srvfd);
	close(epollfd);
	pthread_exit(NULL);
}



