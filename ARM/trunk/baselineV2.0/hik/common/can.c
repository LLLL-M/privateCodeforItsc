#include <stdio.h>
#include <string.h>
#include <sys/types.h>       /* basic system data types */
#include <sys/socket.h>      /* basic socket definitions */
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/if.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "its.h"
#include "configureManagement.h"
#include "libsocketcan.h"
#include "canmsg.h"
#include "HikConfig.h"


static int gCanFd = -1;  //CAN收发套接字

typedef union
{//车检数据，每bit代表一个车检器或是一个车道，0:无车，1:有车
	UInt64 bits;
	UInt8 data[8];
} VehPassData;
static VehPassData gVehPassData = {.bits = 0};
static struct timespec gEnterTimes[MAX_LANE_NUM] = {{0, 0}};	//有卡口时车辆进入的时间
static pthread_rwlock_t gVehLock = PTHREAD_RWLOCK_INITIALIZER;

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;//特殊参数定义  
//extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //混杂参数

extern MsgRealtimeVolume gStructMsgRealtimeVolume;
extern pthread_rwlock_t gLockRealtimeVol;
extern SignalControllerPara *gSignalControlpara;

#define MAX_VEHFLOW_FILESIZE	(10 * 1024 * 1024)
struct
{
	UInt64 offset;
	TimeAndHistoryVolume data[0];
} *gVehFlowHead = NULL;//vehicle flow record

static int InitVehicleflowFile()
{
	int fd_vehflow = -1;
	
	if (gVehFlowHead != NULL)
		return 0;
	fd_vehflow = open(FILE_VEHICLE_DAT, O_RDWR | O_APPEND | O_CREAT, 0666);
	if (fd_vehflow == -1)
	{
		ERR("open %s failed : %s , errno %d", FILE_VEHICLE_DAT, strerror(errno), errno);
        return -1;
	}
	if (ftruncate(fd_vehflow, MAX_VEHFLOW_FILESIZE) < 0)
    {   
        close(fd_vehflow);
		ERR("ftruncate %s failed : %s , errno %d", FILE_VEHICLE_DAT, strerror(errno), errno);
        return -1; 
    } 

	gVehFlowHead = mmap(NULL, MAX_VEHFLOW_FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_vehflow, 0);
	if (gVehFlowHead == MAP_FAILED)
	{
		ERR("mmap %s fail, error info: %s", FILE_VEHICLE_DAT, strerror(errno));
		close(fd_vehflow);
		gVehFlowHead = NULL;
		return -1;
	}
	if (gVehFlowHead->offset > 0)
	{	//说明程序启动之前已经存储过过车数据，需要把offset加1避免覆盖掉之前的数据内容
		gVehFlowHead->offset++;
		if (gVehFlowHead->offset >= (MAX_VEHFLOW_FILESIZE / sizeof(TimeAndHistoryVolume) - 1))
	        gVehFlowHead->offset = 0;
		msync(gVehFlowHead, sizeof(gVehFlowHead->offset), MS_ASYNC);
	}
	return 0;
}

static void RecordVehFlowData(TimeAndHistoryVolume *timeAndHistoryVolume, UINT8 next_cycle_flag)
{
	TimeAndHistoryVolume *p = NULL;
	
	if (gVehFlowHead == NULL)
		return;
	if (next_cycle_flag == 1)
	{
		gVehFlowHead->offset++;
		if (gVehFlowHead->offset >= (MAX_VEHFLOW_FILESIZE / sizeof(TimeAndHistoryVolume) - 1))
	        gVehFlowHead->offset = 0;
		msync(gVehFlowHead, sizeof(gVehFlowHead->offset), MS_ASYNC);
	}
	p = gVehFlowHead->data + gVehFlowHead->offset;
	memcpy(p, timeAndHistoryVolume, sizeof(TimeAndHistoryVolume));
	msync(p, sizeof(TimeAndHistoryVolume), MS_ASYNC);
}

void CalVehicleFlowData(VehPassData *pvehpass)
{
	static VehPassData veh_pass_data = {.bits = 0};
	static time_t startTime = 0;
	UINT8 vehpass_flag = 0, next_cycle_flag = 0;
	UINT8 lane = 0;
	UINT16 nCycleTime = gSignalControlpara->stUnitPara.byFluxCollectCycle * ((gSignalControlpara->stUnitPara.byCollectCycleUnit == 0) ? 1 : 60);
	int i = 0;
    time_t nowTime = time(NULL); 

	if (nCycleTime == 0)
		return;
	pthread_rwlock_wrlock(&gLockRealtimeVol);
	if (startTime == 0)
	{
		startTime = nowTime;
		gStructMsgRealtimeVolume.volumeOccupancy.dwTime = startTime;
	}
	if (nowTime - startTime > nCycleTime)
	{
		memset(gStructMsgRealtimeVolume.volumeOccupancy.struVolume, 0, sizeof(gStructMsgRealtimeVolume.volumeOccupancy.struVolume));
		do
		{	//如果若干个采集周期没有过车，则在随后有过车时也仍然会用空的数据存储之前的那几个采集周期
			startTime += nCycleTime;
			gStructMsgRealtimeVolume.volumeOccupancy.dwTime = startTime;
			RecordVehFlowData(&gStructMsgRealtimeVolume.volumeOccupancy, 1);
		} while (startTime + nCycleTime < nowTime);
	}
	for (lane = 0; lane < MAX_LANE_NUM; lane++)
	{
		if (GET_BIT(veh_pass_data.bits, lane) == 1 && GET_BIT(pvehpass->bits, lane) == 0)
		{//have car enter and leave
			gStructMsgRealtimeVolume.volumeOccupancy.struVolume[lane].byDetectorVolume++;
			vehpass_flag = 1;
		}
	}
	memcpy(&veh_pass_data, pvehpass, sizeof(VehPassData));

	if (vehpass_flag == 1)
		RecordVehFlowData(&gStructMsgRealtimeVolume.volumeOccupancy, 0);
	pthread_rwlock_unlock(&gLockRealtimeVol);
}

void DealVehPassData(UInt8 lane, Boolean isEnter)
{
	struct can_frame frame;
	static VehPassData vehPassData = {.bits = 0};
	UInt8 deviceId;
	
	isEnter ? SET_BIT(vehPassData.bits, lane - 1) : CLR_BIT(vehPassData.bits, lane - 1);
	if (gStructBinfileConfigPara.cCarDetectSwitch)
	{	//有车检板时直接把过车数据发给车检板
		deviceId = (lane - 1) / 8;		//用来区分对应的车检单片机
		memset(&frame, 0, sizeof(frame));
		frame.can_id = (lane <= 24) ? 0x205 : 0x206;
		frame.can_dlc = 3;
		frame.data[0] = deviceId;
		frame.data[1] = ((lane - 1) % 8 < 4) ? 0 : 4;      //用于区分高低位
		frame.data[2] = vehPassData.data[deviceId];
		write(gCanFd, &frame, sizeof(struct can_frame));
		INFO("send camera data to vehicle board!");
	}
	else
	{
		pthread_rwlock_wrlock(&gVehLock);
		gVehPassData.bits = vehPassData.bits;
		if (isEnter) //车辆进入时记录时间戳
			clock_gettime(CLOCK_MONOTONIC, &gEnterTimes[lane - 1]);
		pthread_rwlock_unlock(&gVehLock);
		CalVehicleFlowData(&gVehPassData);
	}
	
}

void SaveVehDectorData(struct can_frame *pframe)
{
	int i;
	struct timespec curTime;
	
	if (pframe == NULL)
		return;
	pthread_rwlock_wrlock(&gVehLock);
	if (pframe->can_id == 0x201)
		memcpy(&gVehPassData.data[0], pframe->data, 3);
	else if (pframe->can_id == 0x202)
		memcpy(&gVehPassData.data[3], pframe->data, 3);
	clock_gettime(CLOCK_MONOTONIC, &curTime);
	for (i = 0; i < MAX_LANE_NUM; i++)
	{
		if (GET_BIT(gVehPassData.bits, i) == 1)
			gEnterTimes[i] = curTime;
	}
	pthread_rwlock_unlock(&gVehLock);
	CalVehicleFlowData(&gVehPassData);
}

UInt64 ItsReadVehicleDetectorData()
{
	UInt64 data;
	int i;
	struct timespec curTime;
	
	clock_gettime(CLOCK_MONOTONIC, &curTime);
	pthread_rwlock_rdlock(&gVehLock);
	//默认车辆进入时间超过3s则清除过车数据表明车辆离开
	for (i = 0; i < MAX_LANE_NUM; i++)
	{
		if (GET_BIT(gVehPassData.bits, i) == 1 && curTime.tv_sec - gEnterTimes[i].tv_sec > 3)
			CLR_BIT(gVehPassData.bits, i);
	}
	data = gVehPassData.bits;
	pthread_rwlock_unlock(&gVehLock);
	return data;
}

void CanInit(UInt32 bitrate)
{
#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
	struct sockaddr_can addr;
    struct ifreq ifr;
	struct timeval timeout = {2, 0};

	can_do_stop("can0");
	if (can_set_bitrate("can0", bitrate) < 0)
	{
		ERR("set can0 bitrate(%u) fail!", bitrate);
		exit(1);
	}
	if (can_do_start("can0") < 0)
	{
		ERR("can0 start fail!");
		exit(1);
	}
    gCanFd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
	if (gCanFd == -1)
	{
		perror("create can socket fail!");
		exit(1);
	}
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(gCanFd, SIOCGIFINDEX, &ifr) == -1)
	{
		perror("ioctl get can index fail!");
		exit(1);
	}
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(gCanFd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("bind can socket fail!");
		exit(1);
	}
	//设置接收超时时间为2s
	setsockopt(gCanFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	InitVehicleflowFile();
#endif
}

void CanSend(struct can_frame *pframe)
{
	write(gCanFd, pframe, sizeof(struct can_frame));
}

int GetCanSockfd()
{
	return gCanFd;
}

