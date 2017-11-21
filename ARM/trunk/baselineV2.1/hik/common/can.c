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
#include "sqlite3.h"


static int gCanFd = -1;  //CAN收发套接字

typedef union
{//车检数据，每bit代表一个车检器或是一个车道，0:无车，1:有车
	UInt64 bits;
	UInt8 data[8];
} VehPassData;
static VehPassData gVehPassData = {.bits = 0};
static struct timespec gEnterTimes[MAX_LANE_NUM] = {{0, 0}};	//有卡口时车辆进入的时间
static pthread_rwlock_t gVehLock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t gVehFlowRecLock = PTHREAD_RWLOCK_INITIALIZER;

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;//特殊参数定义  
//extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //混杂参数
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;
extern MsgRealtimeVolume gStructMsgRealtimeVolume;
extern pthread_rwlock_t gLockRealtimeVol;
extern SignalControllerPara *gSignalControlpara;

#define MAX_VEHFLOW_FILESIZE	(10 * 1024 * 1024)
struct
{
	UInt64 offset;
	TimeAndHistoryVolume data[0];
} *gVehFlowHead = NULL;//vehicle flow record


typedef struct perVehDetectState
{
	time_t max_noresponse;
	time_t vehpass_limit_time;
	time_t max_presence_time;
	UINT32 vehpass_limit;//unit veh/min
}PerVehDetectState;
typedef struct vehpass_onesec
{
	UINT64 vehdata_1s;
	UINT8  inwindow;
}VehPassOneSec;
#define MAX_VEH_ENTERLEAVE 1024
#define HALF_VEH_EL (MAX_VEH_ENTERLEAVE/2)
typedef struct vehEnterLeave
{
	time_t e;
	time_t l;
}VehEnterLeave;
typedef struct vehFlowRecord
{
	UINT16 pos[MAX_LANE_NUM];
	VehEnterLeave vel[MAX_LANE_NUM][MAX_VEH_ENTERLEAVE];
}VehFlowRecord;
VehFlowRecord veh_flow_record;
void InitVehFlowRecord();
void VehEnter(int lane, time_t now);
void VehLeave(int lane, time_t now);
UINT64 VehNoResponse(int lane, int noresponsetime);
UINT64 VehMaxNumLimit(int lane, int maxnumlimit);
UINT64 VehMaxPersence(int lane, int presencetime);




UINT64 Noresponse_err = 0;
UINT64 Vehpasslimit_err = 0;
UINT64 MaxPresence_err = 0;
PerVehDetectState vehDetectStateArr[MAX_LANE_NUM];
UINT32 phase_step_time_limit = 0;
static int phase_step_over_time[NUM_PHASE] = {0};

VehPassOneSec vehdata_onesec;
UINT64 vehdata_havecars = 0;
static pthread_rwlock_t vehdata_lock = PTHREAD_RWLOCK_INITIALIZER;


//////////////////////pedestrian detector///////
UINT8 gPedestrianDetectorBits = 0;
UINT32 gReadPedestrianReqTime = 0;
////////////////////////////////////////////
#define BUS_GROUP 4 
#define SCAR_GROUP 4
#define BUS_SCAR_GROUP 4
extern UINT32 gBusDetectorData[BUS_GROUP];//���������������
extern UINT32 gSpecialCarData[SCAR_GROUP];//���ڳ����������




//////////////////////veh flow record///////////////////////////////

void InitVehFlowRecord()
{
	int i = 0;
	int lane = 0;
	time_t now = time(NULL);

	memset(veh_flow_record.vel, 0, sizeof(VehEnterLeave) * MAX_LANE_NUM * MAX_VEH_ENTERLEAVE);
	memset(veh_flow_record.pos, 0, MAX_LANE_NUM);
	for (lane = 0; lane < MAX_LANE_NUM; lane++)
	{
		veh_flow_record.vel[lane][0].e = now;
		veh_flow_record.vel[lane][0].l = now;
	}
}

void ClearPreHalf(int lane)
{
	VehEnterLeave* pv = veh_flow_record.vel[lane];
	//pthread_rwlock_wrlock(gVehFlowRecLock);
	memcpy(pv, pv + HALF_VEH_EL, HALF_VEH_EL * sizeof(VehEnterLeave));
	memset(pv + HALF_VEH_EL, 0, HALF_VEH_EL * sizeof(VehEnterLeave));
	veh_flow_record.pos[lane] = HALF_VEH_EL - 1;
	//pthread_rwlock_unlock(gVehFlowRecLock);
}

void VehEnter(int lane, time_t now)
{
	VehEnterLeave* pvel = veh_flow_record.vel[lane];
	UINT16* pos = veh_flow_record.pos;
	pthread_rwlock_wrlock(&gVehFlowRecLock);
	if (pos[lane] >= MAX_VEH_ENTERLEAVE)
	{
		ClearPreHalf(lane);
	}
	if (pvel[pos[lane]].l > 0)
	{
		pvel[++pos[lane]].e = now;
	}
	else if ((pvel[pos[lane]].e == 0 || pvel[pos[lane]].e > now) && pvel[pos[lane]].l == 0)//GPS or system adjust time&& vehicle don't recv leave
	{
		pvel[pos[lane]].e = now;
	}
	pthread_rwlock_unlock(&gVehFlowRecLock);
	return;	
}
void VehLeave(int lane, time_t now)
{
	VehEnterLeave* pvel = veh_flow_record.vel[lane];
	UINT16* pos = veh_flow_record.pos;
	pthread_rwlock_wrlock(&gVehFlowRecLock);
	if (pvel[pos[lane]].l == 0 || pvel[pos[lane]].l < now)//don't recv leave || leave time dulplicate
	{
		pvel[pos[lane]].l = now;
	}
	pthread_rwlock_unlock(&gVehFlowRecLock);
	return;
}
UINT16 FindVehRecordTimeRange(int lane, time_t t)
{
	VehEnterLeave* pvel = veh_flow_record.vel[lane];
	UINT16* pos = veh_flow_record.pos;
	UINT16 i = 0;
	time_t now = time(NULL);
	time_t tflag = now - t;
	i = pos[lane];
	while (i > 0)
	{
		if (pvel[i].e < tflag)
			break;
		i--;
	}
	return i;
}

UINT64 VehMaxNumLimit(int lane, int maxnumlimit)
{
	VehEnterLeave* pvel = veh_flow_record.vel[lane];
	UINT16* pos = veh_flow_record.pos;
	UINT16 start = 0;
	UINT64 err = 0;
	
	start = FindVehRecordTimeRange(lane, 60);//per minute
	if ((pos[lane] - start + 1) > maxnumlimit)
		SET_BIT(err, lane);
	return err;
}

UINT64 VehNoResponse(int lane, int noresponsetime)
{
	VehEnterLeave* pvel = veh_flow_record.vel[lane];
	UINT16* pos = veh_flow_record.pos;
	time_t now = time(NULL);
	time_t gap = 0;
	UINT64 err = 0;
	
	if (pvel[pos[lane]].l == 0)
		gap = now - pvel[pos[lane]].e;
	else 
		gap = now - pvel[pos[lane]].l;

	if (gap > noresponsetime)
		SET_BIT(err, lane);
	return err;
}
UINT64 VehMaxPersence(int lane, int presencetime)
{
	VehEnterLeave* pvel = veh_flow_record.vel[lane];
	UINT16* pos = veh_flow_record.pos;
	time_t now = time(NULL);
	time_t gap = 0;
	UINT64 err = 0;

	if (pvel[pos[lane]].e > 0 && pvel[pos[lane]].l == 0)
		gap = now - pvel[pos[lane]].e;
	if (gap > presencetime)
		SET_BIT(err, lane);
	return err;
}

void VehDetectorStateCheck()
{
	int i = 0;
	UINT64 err1 = 0, err2 = 0, err3 = 0;
	struct STRU_N_VehicleDetector* pveh = gSignalControlpara->AscVehicleDetectorTable;
	time_t now = time(NULL);
	
	pthread_rwlock_rdlock(&gVehFlowRecLock);
	for (i = 0; i < MAX_LANE_NUM; i++)
	{
		if (pveh[i].byVehicleDetectorCallPhase == 0 || pveh[i].byVehicleDetectorCallPhase > MAX_CHANNEL_NUM)
			continue;
		if (pveh[i].byVehicleDetectorNoActivity > 0)
		{
			if (VehNoResponse(i, pveh[i].byVehicleDetectorNoActivity * 60) > 0)
			{
				if (GET_BIT(Noresponse_err, i) == 0)
				{
					SET_BIT(Noresponse_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_NORESPONSE, i + 1);
					log_error("vehicle detector No. %d MaxNoResponse Error!!!", pveh[i].byVehicleDetectorNumber);
				}
			}
			else if (GET_BIT(Noresponse_err, i) == 1)
			{
				CLR_BIT(Noresponse_err, i);
				ItsWriteFaultLog(VEHICLE_DETECTOR_NORESPONSE_CLR, i + 1);
				log_error("vehicle detector No. %d MaxNoResponse recover normal!!!", pveh[i].byVehicleDetectorNumber);
			}
		}
		else //if (GET_BIT(Noresponse_err, i) == 1)
		{
			CLR_BIT(Noresponse_err, i);
		}

		if (pveh[i].byVehicleDetectorErraticCounts > 0)
		{
			if (VehMaxNumLimit(i, pveh[i].byVehicleDetectorErraticCounts) > 0)
			{
				if (GET_BIT(Vehpasslimit_err, i) == 0)
				{
					SET_BIT(Vehpasslimit_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_PASSLIMIT, i + 1);
					log_error("vehicle detector No. %d over vehpass limit error!!!", pveh[i].byVehicleDetectorNumber);
				}
			}
			else if (GET_BIT(Vehpasslimit_err, i) == 1)
			{
				CLR_BIT(Vehpasslimit_err, i);
				ItsWriteFaultLog(VEHICLE_DETECTOR_PASSLIMIT_CLR, i + 1);
				log_error("vehicle detector No. %d over vehpass limit recover normal!!!", pveh[i].byVehicleDetectorNumber);
			}
		}
		else //if (GET_BIT(Vehpasslimit_err, i) == 1)
		{
			CLR_BIT(Vehpasslimit_err, i);
		}

		if (pveh[i].byVehicleDetectorMaxPresence > 0)
		{
			if (VehMaxPersence(i, pveh[i].byVehicleDetectorMaxPresence) > 0)
			{
				if (GET_BIT(MaxPresence_err, i) == 0)
				{
					SET_BIT(MaxPresence_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_MAXPRESENCE, i + 1);
					log_error("vehicle detector No. %d Max presence time error!!!", pveh[i].byVehicleDetectorNumber);
				}
			}
			else if (GET_BIT(MaxPresence_err, i) == 1)
			{
				CLR_BIT(MaxPresence_err, i);
				ItsWriteFaultLog(VEHICLE_DETECTOR_MAXPRESENCE_CLR, i + 1);
				log_error("vehicle detector No. %d Max presence time recover normal!!!", pveh[i].byVehicleDetectorNumber);
			}
		}
		else //if (GET_BIT(MaxPresence_err, i) == 1)
		{
			CLR_BIT(MaxPresence_err, i);
		}
/*
		if (veh_flow_record.pos[i] > 0)
			{
				veh_flow_record.pos[i] = 0;
				memset(veh_flow_record.vel[i], 0, sizeof(VehEnterLeave) * MAX_VEH_ENTERLEAVE);
				veh_flow_record.vel[i][0].e = now;
				veh_flow_record.vel[i][0].l = now;
			}*/
	}
	pthread_rwlock_unlock(&gVehFlowRecLock);
}

/////////////////////////////////////////////////////////////////////////

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
	
	vehdata_havecars |= gVehPassData.bits; //have vehicle detector message
	for (lane = 0; lane < MAX_LANE_NUM; lane++)
	{
		if (GET_BIT(veh_pass_data.bits, lane) == 1 && GET_BIT(pvehpass->bits, lane) == 0)
		{//have car enter and leave
			gStructMsgRealtimeVolume.volumeOccupancy.struVolume[lane].byDetectorVolume++;
			vehpass_flag = 1;	
			VehLeave(lane, nowTime);
		}
		
		//add for vehdetector vehpass count limit
		if (GET_BIT(pvehpass->bits, lane) == 1)
		{	
			vehDetectStateArr[lane].vehpass_limit++;
			//INFO("veh message detectorNO %d have car", lane + 1);
			VehEnter(lane, nowTime);
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

UINT64 GetVehDetectorState()
{
	UINT64 vehdetector_err = 0;
	vehdetector_err = (Noresponse_err | Vehpasslimit_err | MaxPresence_err);
	return vehdetector_err;
}
void ClearVehDetectorState()
{	
	time_t now; 
	int i = 0;
	
	time(&now);
	for (i = 0; i < MAX_LANE_NUM; i++)
	{	
		vehDetectStateArr[i].max_noresponse = now;
		vehDetectStateArr[i].vehpass_limit_time = now;
		vehDetectStateArr[i].max_presence_time = now;
		vehDetectStateArr[i].vehpass_limit = 0;
	}
	Noresponse_err = 0;
	Vehpasslimit_err = 0;
	MaxPresence_err = 0;
	InitVehFlowRecord();
}
//when change control mode to inductive,reset vehicle inductive data
UINT8 ItsResetVehicleInductiveData()
{
	time_t now;
	int i  = 0;
	time(&now);
	for (i = 0; i < NUM_PHASE; i++)
		phase_step_over_time[i] = now;
	ClearVehDetectorState();
	
	vehdata_havecars = 0;
	vehdata_onesec.vehdata_1s = 0;
	vehdata_onesec.inwindow = 0;
	//log_debug("reset vehicle inductive data");
	return 0;
}

void ItsClearVehDataHaveCars(CalInfo *pcalinfo, UINT8 current_phase)
{
	PhaseTimeInfo* pphasetime = pcalinfo->phaseTimes;
	vehdata_havecars &= (~pphasetime[current_phase - 1].vehicleDetectorBits);
}

int ProcessVehCheckData()
{
	int i = 0;
	//UInt32 max_no_response = 2*60; //sec, config set
	//UINT32 vehpass_limit = 90; //veh/min;
	static time_t start_time = 0;
	time_t now; 
	
	time(&now);
	if (start_time == 0)
	{
		start_time = now;
		for (i = 0; i < MAX_LANE_NUM; i++)
		{	
			vehDetectStateArr[i].max_noresponse = now;
			vehDetectStateArr[i].vehpass_limit_time = now;
			vehDetectStateArr[i].max_presence_time= now;
			vehDetectStateArr[i].vehpass_limit = 0;
		}
	}
	
	for (i = 0; i < MAX_LANE_NUM; i++)
	{
		if (gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase > 0)
		{
			if (gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorNoActivity > 0)
			{
				if (GET_BIT(gVehPassData.bits, i) == 1)
				{	
					vehDetectStateArr[i].max_noresponse = now;//mark time now and clear No response error bit
					CLR_BIT(Noresponse_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_NORESPONSE_CLR, i + 1);
					
				}
				else if (GET_BIT(gVehPassData.bits, i) == 0)
				{
					if (GET_BIT(Noresponse_err, i) == 0 && 
						(now - vehDetectStateArr[i].max_noresponse > gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorNoActivity * 60))
					{	
						SET_BIT(Noresponse_err, i);
						ItsWriteFaultLog(VEHICLE_DETECTOR_NORESPONSE, i + 1);
						log_error("vehicle detector No. %d MaxNoResponse Error!!!", i + 1);
					}
					
					//vehDetectStateArr[i].vehpass_limit_time = now;
					//SET_BIT(Vehpasslimit_err, i);
				}
			}
			else
			{
				vehDetectStateArr[i].max_noresponse = now;
				CLR_BIT(Noresponse_err, i);
			}

			if ((gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorErraticCounts > 0))//one minute check vehpass limit (TODO:??)
			{
				if ((now - vehDetectStateArr[i].vehpass_limit_time) >= 60)
				{
				if (vehDetectStateArr[i].vehpass_limit > gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorErraticCounts)
				{
					SET_BIT(Vehpasslimit_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_PASSLIMIT, i + 1);
					log_error("vehicle detector No. %d over vehpass limit error!!!", i + 1);
				}
				else
				{
					CLR_BIT(Vehpasslimit_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_PASSLIMIT_CLR, i + 1);
				}
				vehDetectStateArr[i].vehpass_limit = 0;
				vehDetectStateArr[i].vehpass_limit_time = now;
				}
			}
			else 
			{
				CLR_BIT(Vehpasslimit_err, i);
				vehDetectStateArr[i].vehpass_limit = 0;
				vehDetectStateArr[i].vehpass_limit_time = now;
			}

			if (gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorMaxPresence > 0)
			{
				if (GET_BIT(gVehPassData.bits, i) == 0)
				{
					vehDetectStateArr[i].max_presence_time = now;
					CLR_BIT(MaxPresence_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_MAXPRESENCE_CLR, i + 1);
				}
				else if (GET_BIT(MaxPresence_err, i) == 0 && 
					now - vehDetectStateArr[i].max_presence_time > gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorMaxPresence)
				{
					SET_BIT(MaxPresence_err, i);
					ItsWriteFaultLog(VEHICLE_DETECTOR_MAXPRESENCE, i + 1);
					log_error("vehicle detector No. %d Max presence time error!!!", i + 1);
				}			
			}
			else
			{
				vehDetectStateArr[i].max_presence_time = now;
				CLR_BIT(MaxPresence_err, i);
			}
		}
		
	}
	
	return 0;
}
static UINT32 GetPhasePassTimes()
{
	UINT32 phasepasstimes = 0;
	/*sqlite3* pdb = NULL;
	sqlite3_open_wrapper(DATABASE_EXTCONFIG,&pdb);
	sqlite3_select_phasepasstimes(pdb, &phasepasstimes);
	sqlite3_close_wrapper(pdb);*/
	phasepasstimes = gSignalControlpara->stUnitPara.maxWaitTime;
	if (phasepasstimes <= 0)
		return 0xffffffff;//default MAX_UINT;
	else
		return phasepasstimes;
}
int ItsClearStepPhaseTimeGap(CalInfo *pcalinfo, UINT8 phaseid)
{
	int i = 0;
	time_t now;
	
	time(&now);
	if (pcalinfo == NULL || phaseid <= 0 || phaseid > MAX_PHASE_NUM)
		return 0;
	phase_step_over_time[phaseid - 1] = now;
	return 0;
}

int ItsCalcuInductiveStepStage(CalInfo *pcalinfo, UINT8 stagenum)
{
	UINT8 phase_turn = 0;
	UINT64 vehdetector_err = 0;
	int i = 0, j = 0, k = 0, n = 0, l = 0, p = 0;
	int phasenum = 0;
	time_t now;
	UINT8 breakloop = 0;
	
	UINT8 phaseid = 0;
	PhaseTimeInfo* pphasetime = pcalinfo->phaseTimes;
	UINT8 current_phaseid = pcalinfo->stageInfos[stagenum - 1].includePhases[0];
	UINT8 stage = 0, found = 0;
	UINT8 pedReqBits = 0;
	
	vehdetector_err = GetVehDetectorState();
	phase_turn = pcalinfo->phaseTurnId;
	phase_step_time_limit = (gSignalControlpara->stUnitPara.maxWaitTime > 0)? gSignalControlpara->stUnitPara.maxWaitTime : 0xffffffff;
	time(&now);
	for (j = 0; j < NUM_RING_COUNT && breakloop != 1; j++)
		for (i = 0; i < NUM_PHASE && breakloop != 1; i++)
		{
			if (gSignalControlpara->stPhaseTurn[phase_turn - 1][j].nTurnArray[i] == current_phaseid)
				breakloop = 1;
		}
		//INFO("phase index in PhaseTurn ring %d phase %d", j, i);
	if (gSignalControlpara->stPhaseTurn[phase_turn - 1][--j].nTurnArray[--i] != current_phaseid)
		return 0; //can't find current phaseid in phase turn tables

	pedReqBits = ItsReadPedestrianDetectorData(0);
	pthread_rwlock_wrlock(&vehdata_lock);
	for (k = i + 1; (k % 32)!= i; k++ )
	{
		phaseid = gSignalControlpara->stPhaseTurn[phase_turn - 1][j].nTurnArray[k % 32];
		if (phaseid == current_phaseid)
			break;
		if (phaseid == 0 || phaseid > NUM_PHASE)
			continue;
		//INFO("k = %d, phaseid = %d, veh havecar=%d", k, phaseid, !((pphasetime[phaseid - 1].vehicleDetectorBits & vehdata_havecars)== 0));
		stage = pcalinfo->phaseIncludeStage[phaseid - 1][0];
		for (n = 0; n < pcalinfo->stageInfos[stage - 1].includeNum; n++)
		{
			int phase = pcalinfo->stageInfos[stage - 1].includePhases[n];
			if (pphasetime[phase - 1].laneLevel == 1 ||      /*main lane phase*/
				(pphasetime[phase - 1].vehicleDetectorBits & vehdetector_err) ||  /*veh detector error*/
				(now - phase_step_over_time[phase - 1] >= phase_step_time_limit) ||
				(pphasetime[phase - 1].vehicleDetectorBits & vehdata_havecars)  || /*have vehicle wait*/
				(pphasetime[phase - 1].pedAutoRequestFlag == 0 && (pphasetime[phase - 1].pedestrianDetectorBits & pedReqBits))) //pedestrian detector request
			{
				/*if (pphasetime[phase - 1].laneLevel == 1)
					log_debug("--------------main lane phase=%d not step.", phase);
				if (pphasetime[phase - 1].laneLevel != 1)//not main lane
					log_debug("--------------sub lane: phase=%d, vehdata_havecars=%x, %x, veh detectorbits=%x, %x, havecare=%d", 
					phase, (UINT32)((vehdata_havecars>32) & 0xFFFFFFFF),(UINT32)(vehdata_havecars & 0xFFFFFFFF),
					(UINT32)((pphasetime[phase - 1].vehicleDetectorBits>32) & 0xFFFFFFFF),(UINT32)((pphasetime[phase - 1].vehicleDetectorBits) & 0xFFFFFFFF),
					((UINT32)(pphasetime[phase - 1].vehicleDetectorBits & vehdata_havecars)));
				if(now - phase_step_over_time[phase - 1] >= phase_step_time_limit)
					log_debug("---------------phase=%d step timepass %d ", phase, (unsigned int)(now - phase_step_over_time[phase - 1]));*/
				/*phase_step_over_time[phase - 1] = now;
				for (p = pcalinfo->stageInfos[stage - 1].includePhases[l++]; p > 0 && p <= NUM_PHASE && l <= NUM_PHASE; p = pcalinfo->stageInfos[stage - 1].includePhases[l++])
				{
					phase_step_over_time[p - 1] = now;
					//vehdata_havecars &= (~pphasetime[p - 1].vehicleDetectorBits);
				}*/
				found = 1;
				//log_debug("---------------phase=%d clear vehdata havecars = %x, %x", phase, (UINT32)((vehdata_havecars>32) & 0xFFFFFFFF),(UINT32)(vehdata_havecars & 0xFFFFFFFF));
				break;
			}
		}
		if (found)
			break;
		//phase_step_over_time[phaseid - 1]++;
	}
	phaseid = gSignalControlpara->stPhaseTurn[phase_turn - 1][j].nTurnArray[k % 32];
	
	//vehdata_havecars &= (~pphasetime[phaseid - 1].vehicleDetectorBits);
	//INFO("cleanbit, phase %d veh have car %d", phaseid, !((pphasetime[phaseid - 1].vehicleDetectorBits & vehdata_havecars) == 0));
	pthread_rwlock_unlock(&vehdata_lock);
	if (phaseid == current_phaseid)//step current phase, don't need to step.
		return 0; 
	return pcalinfo->phaseIncludeStage[phaseid - 1][0]; //first stage num of phase
	
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

int ItsGetOneSecVehPass(UINT8 inwindow, UINT64* vehdata_1s)
{
	pthread_rwlock_rdlock(&vehdata_lock);
	vehdata_onesec.inwindow = inwindow;
	if (!inwindow)
		vehdata_onesec.vehdata_1s = 0;
	*vehdata_1s = vehdata_onesec.vehdata_1s;
	pthread_rwlock_unlock(&vehdata_lock);
	return 0;
}


//////////////////////pedestrian detector //////////////

UINT8 ItsSetReadPedestrianReqTime(void)
{
	time_t now;
	time(&now);
	gReadPedestrianReqTime = now;
	return 0;
}

UINT8 ItsReadPedestrianDetectorData(UINT8 readNowFlag)
{
	int timegap = gSignalControlpara->stUnitPara.pedKeyPassGap;//can config time gap betwend twice press pedbutton
	int maxWaitTime = gSignalControlpara->stUnitPara.maxWaitTime;
	time_t now;

	if (readNowFlag == 1)//read detector bits right now, no wait time limit
		return gPedestrianDetectorBits;
	time(&now);
	if (now - gReadPedestrianReqTime > timegap)
	{
		if (gPedestrianDetectorBits > 0)
		{
			gReadPedestrianReqTime = now;
			return gPedestrianDetectorBits;
		}
	}
	//INFO("read pedestrian req , maxWaitTime=%d, now - time_read=%ld", maxWaitTime, now - gReadPedestrianReqTime);
	if (maxWaitTime > 0 && now - gReadPedestrianReqTime > maxWaitTime)//no pedestrian req during maxWaitTime second. auto request
	{
		gPedestrianDetectorBits |= 0x01;//set first pedestrian key pressed state.
		gReadPedestrianReqTime = now;
		return gPedestrianDetectorBits;
	}
	return 0;
}
UINT8 ItsClearPedestrianDetectorData(LineQueueData* data, UINT8 phaseId, UINT8 pedKey)
{
	if (pedKey > 8 || phaseId > MAX_PHASE_NUM || phaseId < 1)//pedestrian detector 1-8, phaseId 1-16
		return 0;
	if (pedKey == 0)
		gPedestrianDetectorBits &= (~data->phaseInfos[phaseId - 1].pedestrianDetectorBits);
	else
		CLR_BIT(gPedestrianDetectorBits, pedKey - 1); 
	return gPedestrianDetectorBits;
}

UINT16 ItsGetPedKeyDelayTime()
{
//ÐÐÈËÇëÇóÊ±ÏàÎ»Ò»°ãÍ£ÁôÔÚµÚÒ»¸öÏàÎ»£¬ÐÐÈË·ÅÐÐÑÓ³ÙÊ±¼äÒª´¦ÓÚ
//ÂÌµÆÊ±¼ä·¶Î§£¬³¬³ö·¶Î§ÔòÉèÖÃ×¤ÁôÂÌµÆÊ±¿ÌÔÚµÚÒ»ÏàÎ»¿ªÊ¼
	UINT16 pedDelay = 0;
	pedDelay = gSignalControlpara->stUnitPara.pedKeyDelayTime;
	if (pedDelay == 0 || 
		pedDelay > gSignalControlpara->stGreenSignalRation[0][0].nGreenSignalRationTime)
		pedDelay = 1;
	return pedDelay;
}

UINT32 ItsGetBusPrioData(UINT32* busData)
{

//using pedestrian detector  simulate bus data
	memcpy(busData, gBusDetectorData, BUS_GROUP * 4);
	return 0;
}
UINT32 ItsClearBusPrioData(LineQueueData *data, UINT8 phaseId)
{//test
	UINT8 i = 0;
	for (i = 0; i < BUS_GROUP; i++)
		gBusDetectorData[i] &= (~data->phaseInfos[phaseId - 1].busDetectorBits[i]);
	//UINT8 busDetectorBits = data->phaseInfos[phaseId - 1].busDetectorBits;
	//gPedestrianDetectorBits &= (~busDetectorBits);
	return 0;
}

////////////////////////
//Special Car Control

UINT8 ItsGetSpecialCarControlData(CalInfo *pcalinfo, SpecialCarControl* scarControl)
{
	int i = 0;
	int ret = 0;
	UINT8 channel = 0;
	STRU_N_SpecialCarDetector* scarDetector = gStructBinfileCustom.sSpecialCarDetector;
	if (gStructBinfileCustom.specialCarCheckSwitch == 0 || 
		(gSpecialCarData[0] <= 0 && gSpecialCarData[1] <= 0 && gSpecialCarData[2] <= 0 && gSpecialCarData[3] <= 0))
		return 0;
	memset(scarControl->SpecialCarChanLockStatus, RED, NUM_CHANNEL);
	for (i = 0; i < MAX_SCARDETECTOR_NUM; i++)
	{
		channel = scarDetector[i].bySCarDetectorCallChannel;
		if (0 >= channel && channel > NUM_CHANNEL)
			continue;
		if (GET_BIT(gSpecialCarData[i / 32], i % 32) && scarDetector[i].specialLevel == 1)//level = 1, channel lock
		{
			scarControl->SpecialCarChanLockStatus[channel - 1] = GREEN;
			scarControl->SpecialCarControlTime = 
				(scarControl->SpecialCarControlTime < scarDetector[i].passTime)? scarDetector[i].passTime : scarControl->SpecialCarControlTime;
			ret |= SPECIALCAR_LEVEL1;
		}
	}
	if (ret & SPECIALCAR_LEVEL1)
		scarControl->SpecialCarControlLevel = SPECIALCAR_LEVEL1;
	else
	    scarControl->SpecialCarControlLevel = SPECIALCAR_LEVEL2;//step phase 
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if ((gSpecialCarData[0] & pcalinfo->phaseTimes[i].scarDetectorBits[0]) || (gSpecialCarData[1] & pcalinfo->phaseTimes[i].scarDetectorBits[1])
			 || (gSpecialCarData[2] & pcalinfo->phaseTimes[i].scarDetectorBits[2]) || (gSpecialCarData[3] & pcalinfo->phaseTimes[i].scarDetectorBits[3]))
		{
			scarControl->SpecialCarPhase = i + 1;
			scarControl->SpecialCarStepStage = pcalinfo->phaseIncludeStage[i][0];
			if (scarControl->SpecialCarControlLevel == SPECIALCAR_LEVEL2)
				scarControl->SpecialCarControlTime = pcalinfo->phaseTimes[i].scarPassTime;
			break;
		}
	}
	
	return scarControl->SpecialCarControlLevel;
}
UINT8 ItsClearSpecialCarDataBits(CalInfo *pcalinfo, UINT8 phaseId)
{
	UINT8 i = 0;
	for (i = 0; i < SCAR_GROUP; i++)
		gSpecialCarData[i] &= (~pcalinfo->phaseTimes[phaseId - 1].scarDetectorBits[i]);
	return 0;
}

///////////////////////////
void *VehicleCheckModule(void *arg)
{
	int i = 0;
	int count = 0;
	phase_step_time_limit = GetPhasePassTimes(); //config table  misc's iPhasePassTimes, default 6.
	INFO("VehicleCheckModule start, phase_step_time_limit = %d", phase_step_time_limit);
	InitVehFlowRecord();
	while (1)
	{
		//sem_wait(&gSemForVeh);
		pthread_rwlock_rdlock(&vehdata_lock);
		if (vehdata_onesec.inwindow)
		{	
			vehdata_onesec.vehdata_1s |= gVehPassData.bits;
		}
		//vehdata_havecars |= gVehPassData.bits;
		pthread_rwlock_unlock(&vehdata_lock);
		//ProcessVehCheckData();
		if (i == 10)
		{
			VehDetectorStateCheck();
			i = 0;
		}
		i++;
		usleep(100000);//10 times per second
		
	}
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

