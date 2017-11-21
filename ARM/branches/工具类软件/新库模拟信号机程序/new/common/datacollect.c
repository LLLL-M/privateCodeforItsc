#include <unistd.h>
#include "its.h"
#include <errno.h>
#include <math.h>

#define FILE_VEHICLE_DAT        "/home/vehicle.dat"     //记录了单位周期车检器收到的数据及其统计结果

static PHASE_COUNTING_DOWN_FEEDBACK_PARAMS sCountDownParams;   //倒计时接口信息,这里每500ms从库中获取一次，用来计算统计信息
extern pthread_rwlock_t gLockRealtimeVol;                       //保护实时流量的读写锁
extern MsgRealtimeVolume gStructMsgRealtimeVolume;              //实时流量，只有流量是实时的
extern SignalControllerPara *gSignalControlpara;


static inline UInt8 GetControlScheme(int ret)
{
	UInt8 schemeId = 0;
	FaultLogType type = INVALID_TYPE;
	switch (ret)
	{
		case 1:	schemeId = 0; type = MANUAL_TO_AUTO; break;
		case 2: schemeId = 0; type = AUTO_TO_MANUAL; break;
		case 3: schemeId = YELLOWBLINK_SCHEMEID; type = MANUAL_PANEL_FLASH; break;
		case 4: schemeId = ALLRED_SCHEMEID;	type = MANUAL_PANEL_ALL_RED; break;
		case 5: schemeId = STEP_SCHEMEID; type = MANUAL_PANEL_STEP; break;
	}
	ItsWriteFaultLog(type, 0);
	return schemeId;
}


/*****************************************************************************
 函 数 名  : RecordVehicleFlowData 
             将各个检测器的统计数据保存到文件中，文件默认大小是10M，循环覆盖，文件的第一个字节存放的是最新一条记录距文件首部的偏移
             结构体数量
 输入参数  : TimeAndHistoryVolume *timeAndHistoryVolume  
             int size                                    
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年9月24日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static int RecordVehicleFlowData(TimeAndHistoryVolume *timeAndHistoryVolume)
{
	FILE *fp = NULL;
	UINT64 offset = 0;//offset记录了，即将插入的最新的一条记录距文件头的TimeAndHistoryVolume个数,该值存储在文件开头的4字节区域内
	
	fp = fopen(FILE_VEHICLE_DAT,"r+");//
	if(NULL == fp)//可能是文件不存在，就新建之
	{
	    if(2 == errno)//errno 等于 2表明是文件不存在
	    {
            if((fp = fopen(FILE_VEHICLE_DAT,"a+")) == NULL)
            {
                INFO("open %s failed : %s , errno %d .\n",FILE_VEHICLE_DAT,strerror(errno),errno);
                return 0;
            }
	    }
	    else
	    {
    		INFO("open %s failed : %s , errno %d .\n",FILE_VEHICLE_DAT,strerror(errno),errno);
    		return 0;
	    }
	}

    if(fread(&offset,sizeof(UINT64),1,fp) < 1)//先获取偏移量
    {
        offset = 1;
    }
    if(offset >= 10*1024*1024/sizeof(TimeAndHistoryVolume))//文件默认大小是10M，超过10M后就要从头开始覆盖,offset从4开始
    {
        offset = 1;
    }
    fseek(fp,0,SEEK_SET);
    offset += 1;
    fwrite(&offset,sizeof(UINT64),1,fp);//写入偏移量

    fseek(fp,sizeof(UINT64)+(offset-1)*sizeof(TimeAndHistoryVolume),SEEK_SET);//在正确的位置写入
	if(fwrite(timeAndHistoryVolume,sizeof(TimeAndHistoryVolume),1,fp) < 1)
	{
		INFO("read %s failed : %s .\n",FILE_VEHICLE_DAT,strerror(errno));
		fclose(fp);
		return 0;
	}
    fflush(fp);
	fclose(fp);
	return 1; 
}

/*****************************************************************************
 函 数 名  : CalcStatData
 功能描述  : 根据车检器的流量，计算比如车道占有率等统计数据
车道占有率的公式详见:
http://wenku.baidu.com/view/8c2e3951a417866fb84a8e22.html

车速的计算公式是:
任意交通负荷下的车速--流量通用模型为: 
设计车速Us/(km/h)	通行能力C (单车道)/(Pcu/h)	 
120	2200	0.93	1.88	4.85
100	2200	0.95	1.88	4.86
80	2000	1.00	1.88	4.90
60	1800	1.20	1.88	4.88



排队长度的计算策略是:
该车道没有放行权的时间乘以该车道的平均车速，就是该车道在红灯时的大致排队长度

车流密度k：某一瞬间，单位路段长度内的车辆数,单位是辆/km，根据格林希尔治（Greenshields）速度-密度模型模型，可以得到速度与密度的关系是:
Q = KUf(1- K/Kj),其中Q指的是车速，K就是车流密度，Uf是自由车速，Kj是阻塞密度.根据这个公式，可以反推得到车流密k与车速Q的关系为:
K = (Kj-sqrt(Kj* Kj  - 4* Kj*Q/Uf))/2

时间占有率o: 即车辆的时间密集度，在一定的观测时间内，车辆通过检测器时所占用的时间与观测总时间的比值。按照公式，时间占有率与车身长度检测器
长度之和成正比即o = (l+d)*k.

车头时距与交通量的关系  ht=3600/Q 
车头间距与交通流密度的关系hs=1000/K

 输入参数  : TimeAndHistoryVolume *timeAndHistoryVolume  nCycle 为测试周期长
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年9月24日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void CalcStatData(TimeAndHistoryVolume *timeAndHistoryVolume,UINT32 nCycle)
{
    int i = 0;
    float tmp = 0;
    UINT8 nPhaseId = 0;
    UINT16 nWaitTime = 0;
    const UINT8 Kj = 120;//阻塞密度--车流几乎无法移动，即发生交通阻塞时的车流密度,单位是辆/km.
    const UINT8 Uf = 72;//自由车流速度，可以理解为道路限速.

    //下面是计算平均车速需要用到的参数
    const UINT8 Us = 60;//假定该道路设计时速是60km/h
    const float a1 = 1.2;//表1 高速公路车速-流量通用模型参数表中的数据
    const float a2 = 1.88;//表1 高速公路车速-流量通用模型参数表中的数据
    const float a3 = 4.88;//表1 高速公路车速-流量通用模型参数表中的数据
    float b = 0;//中间值
    
    if(nCycle <= 0 || timeAndHistoryVolume == NULL)
    {
        return;
    }   
    for(i = 0; i < 48; i++)
    {
        if(timeAndHistoryVolume->struVolume[i].byDetectorVolume == 0)//如果车流量等于0，那么平均车速就是0，不再计算
        {
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = 0;
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 0;
        }
        else
        {
            //计算平均车速
            tmp = 1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle;//将车流量换算成一小时的过车量
            b = a2 + a3*pow(1.0*tmp/1800,3);
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = (int)(100*a1*Us/(1+powf(tmp/1800,b)));//为精确两位小数，我们存储时按照乘以100来做，实际使用时需要除以100.

            //计算车流密度和时间占有率
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 100*Kj*(1 - timeAndHistoryVolume->struVolume[i].byVehicleSpeed/100/Uf);

            //计算车头间距
            if(timeAndHistoryVolume->struVolume[i].wVehicleDensity != 0)
            {
                timeAndHistoryVolume->struVolume[i].wVehicleHeadDistance = 100*1000/(1.0*timeAndHistoryVolume->struVolume[i].wVehicleDensity/100);
            }

            //计算车头时距
            timeAndHistoryVolume->struVolume[i].wVehicleHeadTimeDistance = 100*3600/(1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle);

            //计算排队长度,相位的红灯等待时间除以车流量，得到的车辆数就是该车道的红灯排队长度
            nPhaseId = gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase; //找到车检器对应的相位
            nWaitTime = 0;
            if(nPhaseId >= 1 && nPhaseId <= 16)
            {
                nWaitTime = sCountDownParams.ucCurCycleTime - sCountDownParams.stPhaseRunningInfo[nPhaseId - 1][0];
            }
            if(timeAndHistoryVolume->struVolume[i].byDetectorVolume == 0)
            {
                timeAndHistoryVolume->struVolume[i].wQueueLengh = 0;
                continue;
            }
            timeAndHistoryVolume->struVolume[i].wQueueLengh = 100*nWaitTime/(1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume/nCycle);
            
        }
    }
    
    /*
    i = 0;
    INFO("总数: %d 辆, 时间占有率: %0.2f%%, 平均车速: %0.2f km/h, 排队长度: %0.2f m, 车流密度: %0.2f 辆/km, \n\t\t\t车头间距: %0.2f m, 车头时距: %0.2f s, 绿损: %d s\n"
                                                ,timeAndHistoryVolume->struVolume[i].byDetectorVolume
                                                ,timeAndHistoryVolume->struVolume[i].byDetectorOccupancy/100.0
                                                ,timeAndHistoryVolume->struVolume[i].byVehicleSpeed/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wQueueLengh/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleDensity/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleHeadDistance/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleHeadTimeDistance/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wGreenLost/100);
                                              
    */
}



/*****************************************************************************
 函 数 名  : VehicleFlowStat
 功能描述  : 统计车流量，周期取的是单元参数中的采集周期，单位是单元参数中的
             采集单位，每个周期统计一次数据，并保存到指定的文件中，文件大小
             默认是10M,且不可更改,超过10M后就循环覆盖。
 输入参数  : boardNum只能是1 2 3
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年9月24日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void VehicleFlowStat(UINT8 boardNum,UINT16 boardInfo)
{
    static UINT16 oldBoardInfo[3] = {0};  //保存上一次的过车数据，用来判断是否有过车，如果该bit位由0变成1就认为是该车检器有一次过车信息
    static time_t startTime = 0;     //本次记录的开始时间，也是需要保存到二进制文件中的周期起始时间
    static time_t oldTime = 0;//保存的是上一次计算绿损的时间
    static UINT8 nFlag = 0;
    static struct timespec calOccupancyStartTime[48];//计算时间占有率起始时间
    static UINT8 nFlagIsCalcOccupancy[48] = {0};//是否开始计算时间占有率的标志
    static struct timespec greenStartTime[48] ; //绿灯开始时间
    static UINT32 nTimeOccupy[48] = {0};//当前计算周期内，车辆占用车检器的时间，单位是ms.
    struct timespec currentTime;//当前时间
    UINT64 nTempLong = 0;

    UINT16 nCycleTime = gSignalControlpara->stUnitPara.byFluxCollectCycle * ((gSignalControlpara->stUnitPara.byCollectCycleUnit == 0) ? 1 : 60);//采集周期，单位是秒
    UINT8 nIsHaveCar = 0;//是否有过车，1表明有过车，0表明无过车
    UINT8 nPhaseId = 0;//检测器对应的相位
    
    int i = 0;
    time_t nowTime = time(NULL);            

    if(startTime == 0)
    {
        startTime = nowTime;
    }
    pthread_rwlock_wrlock(&gLockRealtimeVol);
    nCycleTime = (nCycleTime == 0 ? 5 : nCycleTime);

    //先判断有没有过车数据
    for(i = 0; i < 16; i++)
    {
        nIsHaveCar = 0;
        if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))//根据前一次数据位为0，后一次数据位为1来作为有车的条件
        {
            gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].byDetectorVolume++;
            nIsHaveCar = 1;
        }

        nPhaseId = gSignalControlpara->AscVehicleDetectorTable[(boardNum - 1)*16 + i].byVehicleDetectorCallPhase;
        //如果当前检测器对应的相位是绿灯，且有过车，就计算绿损时间
        if((nPhaseId >=1) && (nPhaseId < 16)&&(sCountDownParams.stVehPhaseCountingDown[nPhaseId - 1][0] == 1))
        {
            if(nFlag == 0)
            {
                nFlag = 1;
                oldTime = nowTime;
            }
            if(nIsHaveCar == 0)//如果相位是绿灯，但没有过车，那么绿损时间就要加1秒
            {
                if(nowTime - oldTime >= 1)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].wGreenLost += 1*100;
                    nFlag = 0;
                }
            }
            else
            {
                oldTime += 1;
            }

            //相位绿灯,第一次数据是0，然后是1，表明有车开始通过车检器，开始统计时间占有率
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))
            {
                clock_gettime(CLOCK_MONOTONIC, &calOccupancyStartTime[(boardNum - 1)*16 + i]);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 0)
                {
                    nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 1;
                    greenStartTime[(boardNum - 1)*16 + i] = calOccupancyStartTime[(boardNum - 1)*16 + i];
                }
            }            
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 1) && (((boardInfo >> i) & 0x01) == 0))//第一次数据是1，然后是0，表明车辆即将离开车检器，就需要记录该时间            {
            {
                clock_gettime(CLOCK_MONOTONIC, &currentTime);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
                    nTimeOccupy[(boardNum - 1)*16 + i] += (currentTime.tv_sec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_sec)*1000+(currentTime.tv_nsec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_nsec)/1000000;
                //if(i == 0)
                 //   INFO("%d  %p\n",nTimeOccupy[0],&nTimeOccupy[0]);
            }        
        }
        else//红灯时，根据是否已经统计过起始时间来计算时间占有率
        {
            if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
            {
                nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 0;
                
                clock_gettime(CLOCK_MONOTONIC, &currentTime);
                //nTempLong其实计算的就是该车检器对应的相位绿灯时间
                nTempLong = (currentTime.tv_sec - greenStartTime[(boardNum - 1)*16 + i].tv_sec)*1000+(currentTime.tv_nsec - greenStartTime[(boardNum - 1)*16 + i].tv_nsec)/1000000;

                if(nTempLong != 0)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].byDetectorOccupancy = 100.0*nTimeOccupy[(boardNum - 1)*16 + i]/nTempLong;
                }
                memset(nTimeOccupy,0,sizeof(nTimeOccupy));
            }
        }
    }

    gStructMsgRealtimeVolume.volumeOccupancy.dwTime = startTime;
    CalcStatData(&gStructMsgRealtimeVolume.volumeOccupancy,nowTime - startTime);
    
    //判断周期是否已到达，每个周期记录一次数据，每周统计一次数据
    if(nowTime - startTime >= nCycleTime)
    {
        for(i = 0; i < 48; i++)
        {
            if(nFlagIsCalcOccupancy[i] == 1)
            {
                nFlagIsCalcOccupancy[i] = 0;
                
                clock_gettime(CLOCK_MONOTONIC, &currentTime);
                nTempLong = (currentTime.tv_sec - greenStartTime[i].tv_sec)*1000+(currentTime.tv_nsec - greenStartTime[i].tv_nsec)/1000000;
                if(nTempLong != 0)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[i].byDetectorOccupancy = 10000.0*nTimeOccupy[i]/(1.0*nTempLong);
                
                    /*if(i == 0)
                    {
                        INFO("===> volumeOccupancy  %0.2f%% \n",gStructMsgRealtimeVolume.volumeOccupancy.struVolume[i].byDetectorOccupancy/100.0);
                    }*/
                }
                nTimeOccupy[i] = 0;
            }
        }
    
       // CalcStatData(&gStructMsgRealtimeVolume.volumeOccupancy,nowTime - startTime);
        RecordVehicleFlowData(&gStructMsgRealtimeVolume.volumeOccupancy);

        startTime = nowTime;
        nFlag = 0;
        memset(&gStructMsgRealtimeVolume,0,sizeof(gStructMsgRealtimeVolume));
    }
    oldBoardInfo[boardNum - 1] = boardInfo;

    pthread_rwlock_unlock(&gLockRealtimeVol);
}

void *DataCollectModule(void *arg)
{
	int ret = 0;
	UInt8 schemeId = 0;
	UINT8 nCount = 0;//每500ms获取倒计时参数一次，防止频繁加锁影响效率
	UINT8 i = 0;
	
	while (1)
	{
		ret = ProcessKeyBoard();
		if (ret > 0)
		{
			schemeId = GetControlScheme(ret);
			if (ret != 2)	//手动按键按下不需要发送消息
				ItsCtl(KEY_CONTROL, schemeId, 0);
		}
		if(++nCount%5 == 0)
		{
            nCount = 0;
            ItsCountDownGet(&sCountDownParams);
		}
		for(i = 0; i < 3; i++)
		{
    		VehicleFlowStat(i+1,recv_date_from_vechile(i+1));
		}
		usleep(100000);		//每100ms采集一次
	}
}
