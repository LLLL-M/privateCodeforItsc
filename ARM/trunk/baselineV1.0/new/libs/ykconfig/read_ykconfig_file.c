#include <stdio.h>
#include <string.h>
#include "ykconfig.h"
#include "parse_ini.h"


#define DEG(fmt,...) fprintf(stderr,"ykConfig library debug : "fmt "\n",##__VA_ARGS__)

#define MAX_SECTION_LEN	64

#define PRINT_ARGS      //打印信息开关

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


static void ReadUnitPara(YK_WholeConfig  *item)
{
    if(NULL == item)
    {
        return ;
    }
    
	char cSection[MAX_SECTION_LEN] = "UnitPara";

    item->bootYellowBlinkTime = get_one_value(cSection,"bootYellowBlinkTime");
    item->bootAllRedTime = get_one_value(cSection,"bootAllRedTime");
    item->vehFlowCollectCycleTime = get_one_value(cSection,"vehFlowCollectCycleTime");

	item->transitionCycle = get_one_value(cSection,"transitionCycle");
    item->adaptCtlEndRunCycleNum = get_one_value(cSection,"adaptCtlEndRunCycleNum");
    item->watchdogEnable = get_one_value(cSection,"watchdogEnable");
    item->signalMachineType = get_one_value(cSection,"signalMachineType");
    item->weekPreemptSchedule = get_one_value(cSection,"weekPreemptSchedule");
	
#ifdef PRINT_ARGS
    DEG("%s  stUnitPara nBootAllRedTime  %d  , nBootYellowLightTime  %d byFluxCollectCycle  %d \n",__func__,
                        item->bootYellowBlinkTime,
                        item->bootAllRedTime,
                        item->vehFlowCollectCycleTime);
#endif
}

static void ReadChannelItem(YK_ChannelItem *item, int num)
{
    if(NULL == item)
    {
        return;
    }

    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    unsigned char cChannelArray[32] = {0};
	
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"ChannelItem%d",i+1);

        item[i].nChannelID = get_one_value(cSection,"nChannelID");
        item[i].nControllerType = get_one_value(cSection,"nControllerType");
        item[i].nFlashLightType = get_one_value(cSection,"nFlashLightType");
        item[i].nVehDetectorNum = get_one_value(cSection,"nVehDetectorNum");
		get_more_value(cSection,"conflictChannel",cChannelArray,sizeof(cChannelArray));
        item[i].conflictChannel = ArrayToInt(cChannelArray,sizeof(cChannelArray));
		
#ifdef PRINT_ARGS
        if(item[i].nControllerType != 0)
            DEG("%s  stChannel  nChannelID  %d  , nVehDetectorNum  %d  , nControllerType  %d    \n",__func__,
                        item[i].nChannelID,
                        item[i].nVehDetectorNum,
                        item[i].nControllerType);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadSchemeItem(YK_SchemeItem *item, int nSchemeNum, int nPhaseNum)
{
    int i = 0;
	int j = 0;
    char cSection[MAX_SECTION_LEN] = {0};
	unsigned char cChannelArray[32] = {0};
	
    for(i = 0 ; i < nSchemeNum; i++)
    {
		for(j = 0; j < nPhaseNum; j++)
		{
			sprintf(cSection,"SchemeItem%d_%d",i+1,j+1);
			
			if(j == 0)
			{
				item[i].nSchemeId = get_one_value(cSection,"nSchemeId");
				item[i].cycleTime = get_one_value(cSection,"cycleTime");
				item[i].totalPhaseNum = get_one_value(cSection,"totalPhaseNum");	
				nPhaseNum =	item[i].totalPhaseNum; 				
			}
			
			item[i].phaseInfo[j].greenTime = get_one_value(cSection,"greenTime");
			item[i].phaseInfo[j].greenBlinkTime = get_one_value(cSection,"greenBlinkTime");
			item[i].phaseInfo[j].yellowTime = get_one_value(cSection,"yellowTime");
			item[i].phaseInfo[j].redYellowTime = get_one_value(cSection,"redYellowTime");			
			item[i].phaseInfo[j].allRedTime = get_one_value(cSection,"allRedTime");
			item[i].phaseInfo[j].minGreenTime = get_one_value(cSection,"minGreenTime");
			item[i].phaseInfo[j].maxGreenTime = get_one_value(cSection,"maxGreenTime");
			item[i].phaseInfo[j].unitExtendTime = get_one_value(cSection,"unitExtendTime");
			get_more_value(cSection,"channelBits",cChannelArray,sizeof(cChannelArray));
			item[i].phaseInfo[j].channelBits = ArrayToInt(cChannelArray,sizeof(cChannelArray));
	#ifdef PRINT_ARGS
			if(item[i].cycleTime != 0)
				DEG("%s  stScheme  nSchemeID  %d  , nCycleTime  %d  , totalPhaseNum  %d  \n",__func__,
							item[i].nSchemeId,
							item[i].cycleTime,
							item[i].totalPhaseNum);
	#endif
			memset(cSection,0,sizeof(cSection));			
		}

    }
}


static void ReadTimeIntervalItem(YK_TimeIntervalItem *item, int nTimeIntervalId, int nTime)
{
    int j = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    for(j = 0 ; j < nTime ; j++)
    {
        sprintf(cSection,"TimeIntervalItem%d_%d",nTimeIntervalId+1,j+1);

        item[j].nTimeIntervalID = get_one_value(cSection,"nTimeIntervalID");
        //item[i][j].nTimeIntervalID = i+1;
        item[j].nTimeID = get_one_value(cSection,"nTimeID");
        item[j].cStartTimeHour = get_one_value(cSection,"cStartTimeHour");
        item[j].cStartTimeMinute = get_one_value(cSection,"cStartTimeMinute");
        item[j].nSchemeId = get_one_value(cSection,"nSchemeId");
        item[j].IsCorrdinateCtl = get_one_value(cSection,"IsCorrdinateCtl");
        item[j].phaseOffset = get_one_value(cSection,"phaseOffset");
		
#ifdef PRINT_ARGS

        if(item[j].nSchemeId != 0)
            DEG("%s  ReadTimeIntervalItem  nTimeIntervalID  %d  , nTimeID  %d , cStartTimeHour  %d , cStartTimeMinute  %d , schemeId  %d  controlMode %d  \n",__func__,
                                item[j].nTimeIntervalID,
                                item[j].nTimeID,
                                item[j].cStartTimeHour,
                                item[j].cStartTimeMinute,
                                item[j].nSchemeId,
                                item[j].IsCorrdinateCtl
                                );
#endif                                
        memset(cSection,0,sizeof(cSection));
    }
}

static void ReadPlanSchedule(YK_PlanScheduleItem * item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    PlanTime planTime;
    for(i = 0 ; i < num; i++)
    {
		memset(&planTime,0,sizeof(planTime));
        sprintf(cSection,"PlanScheduleItem%d",i+1);

        item[i].nScheduleID = get_one_value(cSection,"scheduleNo");
        item[i].nTimeIntervalID = get_one_value(cSection,"timeIntervalListNo");
        get_more_value(cSection,"month",planTime.month,sizeof(planTime.month));
        item[i].month = ArrayToInt(planTime.month,sizeof(planTime.month));
        
        get_more_value(cSection,"day",planTime.day,sizeof(planTime.day));
        item[i].day = ArrayToInt(planTime.day,sizeof(planTime.day));
        
        get_more_value(cSection,"week",planTime.week,sizeof(planTime.week));
        item[i].week = ArrayToInt(planTime.week,sizeof(planTime.week));
#ifdef PRINT_ARGS

        if(item[i].nTimeIntervalID != 0)
            DEG("%s  stPlanSchedule  scheduleNo  %d  , nTimeIntervalID  %d   Month  0x%x, Day 0x%x, Week  0x%x\n",__func__,
                                                                                    item[i].nScheduleID,
                                                                                     item[i].nTimeIntervalID,
                                                                                     item[i].month,
                                                                                     item[i].day,item[i].week);
#endif
        memset(cSection,0,sizeof(cSection));
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
Boolean LoadykDataFromCfg(YK_Config *pSignalControlpara, const char *path)
{
    char cSection[256];
    int i = 0;
    YK_Config zero;
	
	if ((NULL == pSignalControlpara) || parse_start((path == NULL) ? "/home/ykconfig.dat" : path) == False)
    {
        return FALSE;
    }

	memset(pSignalControlpara, 0, sizeof(YK_Config));

    //加载单元参数 
    memset(cSection,0,sizeof(cSection));
    ReadUnitPara(&pSignalControlpara->wholeConfig);

    //加载通道参数 
    ReadChannelItem(pSignalControlpara->channelTable, NUM_CHANNEL);

    //加载方案表参数 
    ReadSchemeItem(pSignalControlpara->schemeTable,NUM_SCHEME,NUM_PHASE);

    //加载时段表参数
    for(i = 0; i < NUM_TIME_INTERVAL; i++)
    {
        ReadTimeIntervalItem(pSignalControlpara->timeIntervalTable[i], i,NUM_TIME_INTERVAL_ID);
    }
    
    //加载调度参数
    ReadPlanSchedule(pSignalControlpara->scheduleTable,NUM_SCHEDULE);

    //解析结束
    parse_end();

	memset(&zero, 0, sizeof(YK_Config));
	if (memcmp(pSignalControlpara, &zero, sizeof(YK_Config)) == 0) {	//说明没有配置信息
		DEG("config file is not exist or there is no  config information in file . \n");
		return FALSE;
	}	
	
	return TRUE;
}

