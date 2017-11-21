#include <unistd.h>
#include "its.h"
#include <errno.h>
#include <math.h>

#define FILE_VEHICLE_DAT        "/home/vehicle.dat"     //��¼�˵�λ���ڳ������յ������ݼ���ͳ�ƽ��

static PHASE_COUNTING_DOWN_FEEDBACK_PARAMS sCountDownParams;   //����ʱ�ӿ���Ϣ,����ÿ500ms�ӿ��л�ȡһ�Σ���������ͳ����Ϣ
extern pthread_rwlock_t gLockRealtimeVol;                       //����ʵʱ�����Ķ�д��
extern MsgRealtimeVolume gStructMsgRealtimeVolume;              //ʵʱ������ֻ��������ʵʱ��
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
 �� �� ��  : RecordVehicleFlowData 
             �������������ͳ�����ݱ��浽�ļ��У��ļ�Ĭ�ϴ�С��10M��ѭ�����ǣ��ļ��ĵ�һ���ֽڴ�ŵ�������һ����¼���ļ��ײ���ƫ��
             �ṹ������
 �������  : TimeAndHistoryVolume *timeAndHistoryVolume  
             int size                                    
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��9��24��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static int RecordVehicleFlowData(TimeAndHistoryVolume *timeAndHistoryVolume)
{
	FILE *fp = NULL;
	UINT64 offset = 0;//offset��¼�ˣ�������������µ�һ����¼���ļ�ͷ��TimeAndHistoryVolume����,��ֵ�洢���ļ���ͷ��4�ֽ�������
	
	fp = fopen(FILE_VEHICLE_DAT,"r+");//
	if(NULL == fp)//�������ļ������ڣ����½�֮
	{
	    if(2 == errno)//errno ���� 2�������ļ�������
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

    if(fread(&offset,sizeof(UINT64),1,fp) < 1)//�Ȼ�ȡƫ����
    {
        offset = 1;
    }
    if(offset >= 10*1024*1024/sizeof(TimeAndHistoryVolume))//�ļ�Ĭ�ϴ�С��10M������10M���Ҫ��ͷ��ʼ����,offset��4��ʼ
    {
        offset = 1;
    }
    fseek(fp,0,SEEK_SET);
    offset += 1;
    fwrite(&offset,sizeof(UINT64),1,fp);//д��ƫ����

    fseek(fp,sizeof(UINT64)+(offset-1)*sizeof(TimeAndHistoryVolume),SEEK_SET);//����ȷ��λ��д��
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
 �� �� ��  : CalcStatData
 ��������  : ���ݳ�������������������糵��ռ���ʵ�ͳ������
����ռ���ʵĹ�ʽ���:
http://wenku.baidu.com/view/8c2e3951a417866fb84a8e22.html

���ٵļ��㹫ʽ��:
���⽻ͨ�����µĳ���--����ͨ��ģ��Ϊ: 
��Ƴ���Us/(km/h)	ͨ������C (������)/(Pcu/h)	 
120	2200	0.93	1.88	4.85
100	2200	0.95	1.88	4.86
80	2000	1.00	1.88	4.90
60	1800	1.20	1.88	4.88



�Ŷӳ��ȵļ��������:
�ó���û�з���Ȩ��ʱ����Ըó�����ƽ�����٣����Ǹó����ں��ʱ�Ĵ����Ŷӳ���

�����ܶ�k��ĳһ˲�䣬��λ·�γ����ڵĳ�����,��λ����/km�����ݸ���ϣ���Σ�Greenshields���ٶ�-�ܶ�ģ��ģ�ͣ����Եõ��ٶ����ܶȵĹ�ϵ��:
Q = KUf(1- K/Kj),����Qָ���ǳ��٣�K���ǳ����ܶȣ�Uf�����ɳ��٣�Kj�������ܶ�.���������ʽ�����Է��Ƶõ�������k�복��Q�Ĺ�ϵΪ:
K = (Kj-sqrt(Kj* Kj  - 4* Kj*Q/Uf))/2

ʱ��ռ����o: ��������ʱ���ܼ��ȣ���һ���Ĺ۲�ʱ���ڣ�����ͨ�������ʱ��ռ�õ�ʱ����۲���ʱ��ı�ֵ�����չ�ʽ��ʱ��ռ�����복���ȼ����
����֮�ͳ����ȼ�o = (l+d)*k.

��ͷʱ���뽻ͨ���Ĺ�ϵ  ht=3600/Q 
��ͷ����뽻ͨ���ܶȵĹ�ϵhs=1000/K

 �������  : TimeAndHistoryVolume *timeAndHistoryVolume  nCycle Ϊ�������ڳ�
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��9��24��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void CalcStatData(TimeAndHistoryVolume *timeAndHistoryVolume,UINT32 nCycle)
{
    int i = 0;
    float tmp = 0;
    UINT8 nPhaseId = 0;
    UINT16 nWaitTime = 0;
    const UINT8 Kj = 120;//�����ܶ�--���������޷��ƶ�����������ͨ����ʱ�ĳ����ܶ�,��λ����/km.
    const UINT8 Uf = 72;//���ɳ����ٶȣ��������Ϊ��·����.

    //�����Ǽ���ƽ��������Ҫ�õ��Ĳ���
    const UINT8 Us = 60;//�ٶ��õ�·���ʱ����60km/h
    const float a1 = 1.2;//��1 ���ٹ�·����-����ͨ��ģ�Ͳ������е�����
    const float a2 = 1.88;//��1 ���ٹ�·����-����ͨ��ģ�Ͳ������е�����
    const float a3 = 4.88;//��1 ���ٹ�·����-����ͨ��ģ�Ͳ������е�����
    float b = 0;//�м�ֵ
    
    if(nCycle <= 0 || timeAndHistoryVolume == NULL)
    {
        return;
    }   
    for(i = 0; i < 48; i++)
    {
        if(timeAndHistoryVolume->struVolume[i].byDetectorVolume == 0)//�������������0����ôƽ�����پ���0�����ټ���
        {
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = 0;
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 0;
        }
        else
        {
            //����ƽ������
            tmp = 1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle;//�������������һСʱ�Ĺ�����
            b = a2 + a3*pow(1.0*tmp/1800,3);
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = (int)(100*a1*Us/(1+powf(tmp/1800,b)));//Ϊ��ȷ��λС�������Ǵ洢ʱ���ճ���100������ʵ��ʹ��ʱ��Ҫ����100.

            //���㳵���ܶȺ�ʱ��ռ����
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 100*Kj*(1 - timeAndHistoryVolume->struVolume[i].byVehicleSpeed/100/Uf);

            //���㳵ͷ���
            if(timeAndHistoryVolume->struVolume[i].wVehicleDensity != 0)
            {
                timeAndHistoryVolume->struVolume[i].wVehicleHeadDistance = 100*1000/(1.0*timeAndHistoryVolume->struVolume[i].wVehicleDensity/100);
            }

            //���㳵ͷʱ��
            timeAndHistoryVolume->struVolume[i].wVehicleHeadTimeDistance = 100*3600/(1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle);

            //�����Ŷӳ���,��λ�ĺ�Ƶȴ�ʱ����Գ��������õ��ĳ��������Ǹó����ĺ���Ŷӳ���
            nPhaseId = gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase; //�ҵ���������Ӧ����λ
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
    INFO("����: %d ��, ʱ��ռ����: %0.2f%%, ƽ������: %0.2f km/h, �Ŷӳ���: %0.2f m, �����ܶ�: %0.2f ��/km, \n\t\t\t��ͷ���: %0.2f m, ��ͷʱ��: %0.2f s, ����: %d s\n"
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
 �� �� ��  : VehicleFlowStat
 ��������  : ͳ�Ƴ�����������ȡ���ǵ�Ԫ�����еĲɼ����ڣ���λ�ǵ�Ԫ�����е�
             �ɼ���λ��ÿ������ͳ��һ�����ݣ������浽ָ�����ļ��У��ļ���С
             Ĭ����10M,�Ҳ��ɸ���,����10M���ѭ�����ǡ�
 �������  : boardNumֻ����1 2 3
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��9��24��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void VehicleFlowStat(UINT8 boardNum,UINT16 boardInfo)
{
    static UINT16 oldBoardInfo[3] = {0};  //������һ�εĹ������ݣ������ж��Ƿ��й����������bitλ��0���1����Ϊ�Ǹó�������һ�ι�����Ϣ
    static time_t startTime = 0;     //���μ�¼�Ŀ�ʼʱ�䣬Ҳ����Ҫ���浽�������ļ��е�������ʼʱ��
    static time_t oldTime = 0;//���������һ�μ��������ʱ��
    static UINT8 nFlag = 0;
    static struct timespec calOccupancyStartTime[48];//����ʱ��ռ������ʼʱ��
    static UINT8 nFlagIsCalcOccupancy[48] = {0};//�Ƿ�ʼ����ʱ��ռ���ʵı�־
    static struct timespec greenStartTime[48] ; //�̵ƿ�ʼʱ��
    static UINT32 nTimeOccupy[48] = {0};//��ǰ���������ڣ�����ռ�ó�������ʱ�䣬��λ��ms.
    struct timespec currentTime;//��ǰʱ��
    UINT64 nTempLong = 0;

    UINT16 nCycleTime = gSignalControlpara->stUnitPara.byFluxCollectCycle * ((gSignalControlpara->stUnitPara.byCollectCycleUnit == 0) ? 1 : 60);//�ɼ����ڣ���λ����
    UINT8 nIsHaveCar = 0;//�Ƿ��й�����1�����й�����0�����޹���
    UINT8 nPhaseId = 0;//�������Ӧ����λ
    
    int i = 0;
    time_t nowTime = time(NULL);            

    if(startTime == 0)
    {
        startTime = nowTime;
    }
    pthread_rwlock_wrlock(&gLockRealtimeVol);
    nCycleTime = (nCycleTime == 0 ? 5 : nCycleTime);

    //���ж���û�й�������
    for(i = 0; i < 16; i++)
    {
        nIsHaveCar = 0;
        if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))//����ǰһ������λΪ0����һ������λΪ1����Ϊ�г�������
        {
            gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].byDetectorVolume++;
            nIsHaveCar = 1;
        }

        nPhaseId = gSignalControlpara->AscVehicleDetectorTable[(boardNum - 1)*16 + i].byVehicleDetectorCallPhase;
        //�����ǰ�������Ӧ����λ���̵ƣ����й������ͼ�������ʱ��
        if((nPhaseId >=1) && (nPhaseId < 16)&&(sCountDownParams.stVehPhaseCountingDown[nPhaseId - 1][0] == 1))
        {
            if(nFlag == 0)
            {
                nFlag = 1;
                oldTime = nowTime;
            }
            if(nIsHaveCar == 0)//�����λ���̵ƣ���û�й�������ô����ʱ���Ҫ��1��
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

            //��λ�̵�,��һ��������0��Ȼ����1�������г���ʼͨ������������ʼͳ��ʱ��ռ����
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))
            {
                clock_gettime(CLOCK_MONOTONIC, &calOccupancyStartTime[(boardNum - 1)*16 + i]);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 0)
                {
                    nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 1;
                    greenStartTime[(boardNum - 1)*16 + i] = calOccupancyStartTime[(boardNum - 1)*16 + i];
                }
            }            
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 1) && (((boardInfo >> i) & 0x01) == 0))//��һ��������1��Ȼ����0���������������뿪������������Ҫ��¼��ʱ��            {
            {
                clock_gettime(CLOCK_MONOTONIC, &currentTime);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
                    nTimeOccupy[(boardNum - 1)*16 + i] += (currentTime.tv_sec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_sec)*1000+(currentTime.tv_nsec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_nsec)/1000000;
                //if(i == 0)
                 //   INFO("%d  %p\n",nTimeOccupy[0],&nTimeOccupy[0]);
            }        
        }
        else//���ʱ�������Ƿ��Ѿ�ͳ�ƹ���ʼʱ��������ʱ��ռ����
        {
            if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
            {
                nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 0;
                
                clock_gettime(CLOCK_MONOTONIC, &currentTime);
                //nTempLong��ʵ����ľ��Ǹó�������Ӧ����λ�̵�ʱ��
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
    
    //�ж������Ƿ��ѵ��ÿ�����ڼ�¼һ�����ݣ�ÿ��ͳ��һ������
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
	UINT8 nCount = 0;//ÿ500ms��ȡ����ʱ����һ�Σ���ֹƵ������Ӱ��Ч��
	UINT8 i = 0;
	
	while (1)
	{
		ret = ProcessKeyBoard();
		if (ret > 0)
		{
			schemeId = GetControlScheme(ret);
			if (ret != 2)	//�ֶ��������²���Ҫ������Ϣ
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
		usleep(100000);		//ÿ100ms�ɼ�һ��
	}
}
