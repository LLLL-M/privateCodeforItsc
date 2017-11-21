#include <stdio.h>
#include <string.h>
#include "gbConfig.h"
#include "parse_ini.h"


#define DEG(fmt,...) fprintf(stderr,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)

#define MAX_SECTION_LEN	64

//#define PRINT_ARGS      //打印信息开关

/*****************************************************************************
 函 数 名  : ArrayToInt
 功能描述  : 将char型数组，转换成int型.
 输入参数  : unsigned char *array  
             int len               
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned int ArrayToInt(unsigned char *array,int len)
{
	if(!array)
	{
		return 0;
	}
	
	int i = 0;
	unsigned int ret = 0;
	
	for(; i < len; i++)
	{
		if(0 != array[i])
		{
			ret |= (1 << array[i]);
		}
			
	}
	
	return ret;
}


static void ReadUnitPara(GbConfig  *item)
{
    if(NULL == item)
    {
        return ;
    }
    
	char cSection[MAX_SECTION_LEN] = "UnitPara";

    item->bootBlinkTime = get_one_value(cSection,"bootBlinkTime");
    item->bootAllRedTime = get_one_value(cSection,"bootAllRedTime");
    item->flashFrequency = get_one_value(cSection,"flashFrequency");

#ifdef PRINT_ARGS
    DEG("%s  stUnitPara nBootAllRedTime  %d  , nBootYellowLightTime  %d byFluxCollectCycle  %d \n",__func__,
                        item->bootBlinkTime,
                        item->bootAllRedTime,
                        item->flashFrequency);
#endif
}

static void ReadPhaseItem(GbPhaseList *item, int num)
{
    if(NULL == item)
    {
        return ;
    }

    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"PhaseItem%d",i+1);

        item[i].phaseNo = get_one_value(cSection,"phaseNo");
        item[i].greenBlinkTime = get_one_value(cSection,"greenBlinkTime");
        item[i].pedestrianPassTime = get_one_value(cSection,"pedestrianPassTime");
        item[i].pedestrianClearTime = get_one_value(cSection,"pedestrianClearTime");

        item[i].minGreen = get_one_value(cSection,"minGreen");
        item[i].maxGreen_1 = get_one_value(cSection,"maxGreen_1");
        item[i].maxGreen_2 = get_one_value(cSection,"maxGreen_2");
        item[i].unitExtendGreen = get_one_value(cSection,"unitExtendGreen");
        item[i].phaseType = get_one_value(cSection,"phaseType");
        item[i].phaseOption = get_one_value(cSection,"phaseOption");

#ifdef PRINT_ARGS
        if(item[i].phaseOption != 0)
            DEG("%s  stPhase  nPhaseID  %d  , nGreenLightTime  %d nPedestrianPassTime  %d ,nPedestrianClearTime  %d, phaseOption  %d , \n",__func__,
                        item[i].phaseNo,
                        item[i].greenBlinkTime,
                        item[i].pedestrianPassTime,
                        item[i].pedestrianClearTime,
                        item[i].phaseOption
                        );
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadPhaseConflictItem(GbPhaseConflictList *item, int num)
{
    if(NULL == item)
    {
        return ;
    }

    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"PhaseConflictItem%d",i+1);

        item[i].phaseConflictNo = get_one_value(cSection,"phaseConflictNo");
        item[i].conflictPhase = get_one_value(cSection,"conflictPhase");

#ifdef PRINT_ARGS
        if(item[i].conflictPhase != 0)
            DEG("%s  ReadPhaseConflictItem  phaseConflictNo  %d  conflictPhase  %d\n",__func__,
                        item[i].phaseConflictNo,
                        item[i].conflictPhase);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadChannelItem(GbChannelList *item, int num)
{
    if(NULL == item)
    {
        return;
    }

    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"ChannelItem%d",i+1);

        item[i].channelNo = get_one_value(cSection,"channelNo");
        item[i].channelRelatedPhase = get_one_value(cSection,"channelRelatedPhase");
        item[i].channelControlType = get_one_value(cSection,"channelControlType");
        item[i].channelFlashStatus = get_one_value(cSection,"channelFlashStatus");

#ifdef PRINT_ARGS
        if(item[i].channelRelatedPhase != 0)
            DEG("%s  stChannel  nChannelID  %d  , nControllerID  %d  , nControllerType  %d    \n",__func__,
                        item[i].channelNo,
                        item[i].channelRelatedPhase,
                        item[i].channelControlType);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadSchemeItem(GbSchemeList *item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"SchemeItem%d",i+1);

        item[i].schemeNo = get_one_value(cSection,"schemeNo");
        item[i].cycleTime = get_one_value(cSection,"cycleTime");
        item[i].stageTimingNo = get_one_value(cSection,"stageTimingNo");
        item[i].coordinatePhase = get_one_value(cSection,"coordinatePhase");
        item[i].phaseGap = get_one_value(cSection,"phaseGap");
#ifdef PRINT_ARGS
        if(item[i].cycleTime != 0)
            DEG("%s  stScheme  nSchemeID  %d  , nCycleTime  %d  , coordinatePhase  %d  , phaseGap  %d   \n",__func__,
                        item[i].schemeNo,
                        item[i].cycleTime,
                        item[i].coordinatePhase,
                        item[i].phaseGap);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}


static void ReadTimeIntervalItem(GbTimeIntervalList *item, int nTimeIntervalId, int nTime)
{
    int j = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    for(j = 0 ; j < nTime ; j++)
    {
        sprintf(cSection,"TimeIntervalItem%d_%d",nTimeIntervalId+1,j+1);

        item[j].timeIntervalListNo = get_one_value(cSection,"timeIntervalListNo");
        //item[i][j].nTimeIntervalID = i+1;
        item[j].timeIntervalNo = get_one_value(cSection,"timeIntervalNo");
        item[j].hour = get_one_value(cSection,"hour");
        item[j].minute = get_one_value(cSection,"minute");
        item[j].schemeId = get_one_value(cSection,"schemeId");
        item[j].controlMode = get_one_value(cSection,"controlMode");
        
#ifdef PRINT_ARGS

        if(item[j].schemeId != 0)
            DEG("%s  ReadTimeIntervalItem  nTimeIntervalID  %d  , nTimeID  %d , cStartTimeHour  %d , cStartTimeMinute  %d , schemeId  %d  controlMode %d  \n",__func__,
                                item[j].timeIntervalListNo,
                                item[j].timeIntervalNo,
                                item[j].hour,
                                item[j].minute,
                                item[j].schemeId,
                                item[j].controlMode
                                );
#endif                                
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadPlanSchedule(GbScheduleList * item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    PlanTime planTime;
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"PlanScheduleItem%d",i+1);

        item[i].scheduleNo = get_one_value(cSection,"scheduleNo");
        item[i].timeIntervalListNo = get_one_value(cSection,"timeIntervalListNo");
        get_more_value(cSection,"month",planTime.month,sizeof(planTime.month));
        item[i].month = ArrayToInt(planTime.month,sizeof(planTime.month));
        
        get_more_value(cSection,"day",planTime.day,sizeof(planTime.day));
        item[i].day = ArrayToInt(planTime.day,sizeof(planTime.day));
        
        get_more_value(cSection,"week",planTime.week,sizeof(planTime.week));
        item[i].week = ArrayToInt(planTime.week,sizeof(planTime.week));
#ifdef PRINT_ARGS

        if(item[i].timeIntervalListNo != 0)
            DEG("%s  stPlanSchedule  scheduleNo  %d  , nTimeIntervalID  %d   Month  0x%x, Day 0x%x, Week  0x%x\n",__func__,
                                                                                    item[i].scheduleNo,
                                                                                     item[i].timeIntervalListNo,
                                                                                     item[i].month,
                                                                                     item[i].day,item[i].week);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadFollowPhaseItem(GbFollowPhaseList *item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    memset(cSection,0,sizeof(cSection));
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"FollowPhaseItem%d",i+1);

        item[i].followPhaseNo = get_one_value(cSection,"followPhaseNo");
        item[i].motherPhaseNum= get_one_value(cSection,"motherPhaseNum");
        get_more_value(cSection,"motherPhase",item[i].motherPhase,sizeof(item[i].motherPhase));

#ifdef PRINT_ARGS

        if(item[i].followPhaseNo  != 0)
            DEG("%s  stFollowPhase  nFollowPhaseID  %d  , nArrayMotherPhase[0]  %d , nArrayMotherPhase[1]  %d  , nArrayMotherPhase[2]  %d  \n",__func__,
                        item[i].followPhaseNo,
                        item[i].motherPhase[0],
                        item[i].motherPhase[1],
                        item[i].motherPhase[2]);

#endif
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadStageTimingItem(GbStageTimingList *item, int num,int stepNo)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"StageTimingItem%d_%d",stepNo+1,i+1);

        item[i].stageTimingNo = get_one_value(cSection,"stageTimingNo");
        item[i].stageNo = get_one_value(cSection,"stageNo");
        item[i].phaseNo = get_one_value(cSection,"phaseNo");
        item[i].greenTime = get_one_value(cSection,"greenTime");
        item[i].yellowTime = get_one_value(cSection,"yellowTime");
        item[i].allRedTime = get_one_value(cSection,"allRedTime");
        item[i].stageOption = get_one_value(cSection,"stageOption");
        
#ifdef PRINT_ARGS
        if(item[i].phaseNo != 0)
         DEG("%s  ReadStageTimingItem  stageTimingNo  %d  , stageNo  %d , phaseNo  %d \n",__func__,
                                item[i].stageTimingNo,
                                item[i].stageNo,
                                item[i].phaseNo);
#endif

        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadVehicleDetector(GbVehDetectorList *item, int num)
{
	int i;
	char section[MAX_SECTION_LEN]  = {0};
	
	for(i = 0; i < num; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "VehicleDetector", item[i].detectorNo);

        item[i].detectorNo = get_one_value(section,"detectorNo");
        item[i].requestPhase = get_one_value(section,"requestPhase");
        item[i].detectorOption = get_one_value(section,"detectorOption");
        item[i].detectorType = get_one_value(section,"detectorType");
        item[i].detectorDirection = get_one_value(section,"detectorDirection");
        item[i].detectorRequestValidTime = get_one_value(section,"detectorRequestValidTime");
        item[i].laneFullFlow = get_one_value(section,"laneFullFlow");
        item[i].laneFullRate = get_one_value(section,"laneFullRate");

        
	}
}


/*****************************************************************************
 函 数 名  : LoadDataFromCfg
 功能描述  : 从配置文件中加载配置数据到内存中
 输入参数  : 无
 输出参数  : 无
 返 回 值  : unsigned
 
 修改历史      :
  1.日    期   : 2014年7月31日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
Boolean LoadGbDataFromCfg(GbConfig *pSignalControlpara, const char *path)
{
    char cSection[256];
    int i = 0;
    GbConfig zero;
	
	if ((NULL == pSignalControlpara) || parse_start((path == NULL) ? CFG_NAME : path) == False)
    {
        return FALSE;
    }

	memset(pSignalControlpara, 0, sizeof(GbConfig));

    //加载单元参数 
    memset(cSection,0,sizeof(cSection));
    ReadUnitPara(pSignalControlpara);

    //加载相位参数 
    ReadPhaseItem(pSignalControlpara->phaseTable,MAX_PHASE_LIST_NUM);

    //加载通道参数 
    ReadChannelItem(pSignalControlpara->channelTable, MAX_CHANNEL_LIST_NUM);

    //加载方案表参数 
    ReadSchemeItem(pSignalControlpara->schemeTable,MAX_SCHEME_LIST_NUM);

    //加载阶段配时表
    for(i = 0; i < MAX_STAGE_TIMING_LIST_NUM; i++)
    {
        ReadStageTimingItem(pSignalControlpara->stageTimingTable[i],MAX_STAGE_NUM, i);
    }

    //加载相位冲突表
    ReadPhaseConflictItem(pSignalControlpara->phaseConflictTable,MAX_PHASE_LIST_NUM);

    //加载时段表参数
    for(i = 0; i < MAX_TIMEINTERVAL_LIST_NUM; i++)
    {
        ReadTimeIntervalItem(pSignalControlpara->timeIntervalTable[i], i,MAX_TIMEINTERVAL_NUM);
    }
    
    //加载调度参数
    ReadPlanSchedule(pSignalControlpara->scheduleTable,MAX_SCHEDULE_LIST_NUM);

    //加载跟随参数
    ReadFollowPhaseItem(pSignalControlpara->followPhaseTable,MAX_FOLLOW_PHASE_LIST_NUM);

    //
    ReadVehicleDetector(pSignalControlpara->vehDetectorTable,MAX_VEH_DETECTOR_NUM);

    //解析结束
    parse_end();

	memset(&zero, 0, sizeof(GbConfig));
	if (memcmp(pSignalControlpara, &zero, sizeof(GbConfig)) == 0) {	//说明没有配置信息
		DBG("config file is not exist or there is no  config information in file . \n");
		return FALSE;
	}	
	
	return TRUE;
}

