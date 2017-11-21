#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "HikConfig.h"
//#include "parse_ini.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

#define DEG(fmt,...) fprintf(stderr,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)

#define MAX_SECTION_LEN	64

sqlite3* g_pdatabase;

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
unsigned int ArrayToInt(unsigned char *array,int len)
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

/*****************************************************************************
 函 数 名  : ReadUnitPara
 功能描述  : 从配置文件中读取单元参数
 输入参数  : PUnitPara item  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月4日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadUnitPara(PUnitPara item)
{
    sqlite3_select_unit(g_pdatabase, item);
}

/*****************************************************************************
 函 数 名  : ReadPhaseItem
 功能描述  : 读取相位表参数
 输入参数  : PPhaseItem item                      
             struct STRU_SignalTransEntry *entry  
             int num                              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月4日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadPhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE])
{
  sqlite3_select_phase(g_pdatabase, item);
  sqlite3_select_signal_trans(g_pdatabase, entry);
}


/*****************************************************************************
 函 数 名  : ReadChannelItem
 功能描述  : 单个读取通道参数
 输入参数  : PChannelItem item  
             int num            
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/

void ReadChannelItem(ChannelItem channel[][NUM_CHANNEL])
{
 	sqlite3_select_channel(g_pdatabase, channel);
}

/*****************************************************************************
 函 数 名  : ReadGreenSignalRationItem
 功能描述  : 单个读取绿信比参数
 输入参数  : PGreenSignalRationItem item  
             int nGreenSignalRationId     
             int nPhaseNum                
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE])
{
	sqlite3_select_green_split(g_pdatabase, green_split);
}

/*****************************************************************************
 函 数 名  : ReadPhaseTurnItem
 功能描述  : 单个读取相序参数
 输入参数  : PhaseTurnItem *item  
             int nCircleId        
             int num              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadPhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT])
{
	sqlite3_select_phase_turn(g_pdatabase, phase_turn);	

}

/*****************************************************************************
 函 数 名  : ReadSchemeItem
 功能描述  : 单个读取方案表参数
 输入参数  : PSchemeItem item  
             int num           
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadSchemeItem(SchemeItem *scheme)
{
 	sqlite3_select_scheme(g_pdatabase, scheme);
}

/*****************************************************************************
 函 数 名  : ReadActionItem
 功能描述  : 单个读取动作表参数
 输入参数  : PActionItem item  
             int num           
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadActionItem(ActionItem *action)
{
	sqlite3_select_action(g_pdatabase, action); 
}


/*****************************************************************************
 函 数 名  : ReadTimeIntervalItem
 功能描述  : 单个读取时段表参数
 输入参数  : TimeIntervalItem *item  
             int nTimeIntervalId     
             int nTime               
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID])
{
 	sqlite3_select_timeinterval(g_pdatabase, time_interval);
}

/*****************************************************************************
 函 数 名  : ReadPlanSchedule
 功能描述  : 单个读取方案表参数
 输入参数  : PPlanScheduleItem item  
             int num                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadPlanSchedule(PlanScheduleItem *schedule)
{
	sqlite3_select_schedule(g_pdatabase, schedule);
}

/*****************************************************************************
 函 数 名  : ReadFollowPhaseItem
 功能描述  : 单个读取跟随相位参数
 输入参数  : PFollowPhaseItem item  
             int num                
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE])
{
	sqlite3_select_follow_phase(g_pdatabase, follow_phase); 
}

/*****************************************************************************
 函 数 名  : ReadVehicleDetector
 功能描述  : 单个读取车辆检测器参数
 输入参数  : struct STRU_N_VehicleDetector *item  
             int num                              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadVehicleDetector(struct STRU_N_VehicleDetector *vehicle)
{
	sqlite3_select_vehicle(g_pdatabase, vehicle);
}

/*****************************************************************************
 函 数 名  : ReadPedestrianDetector
 功能描述  : 单个读取行人检测器参数
 输入参数  : struct STRU_N_PedestrianDetector *item  
             int num                                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadPedestrianDetector(struct STRU_N_PedestrianDetector *item, int num)
{

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
Boolean LoadDataFromCfg(SignalControllerPara *pSignalControlpara)
{
    SignalControllerPara zero;
	
	if ((NULL == pSignalControlpara))
    {
        return FALSE;
    }
	sqlite3_open_wrapper(DATABASE_HIKCONFIG, &g_pdatabase);
	memset(pSignalControlpara, 0, sizeof(SignalControllerPara));

    //加载单元参数 
    ReadUnitPara(&(pSignalControlpara->stUnitPara));

    //加载相位参数 
    ReadPhaseItem(pSignalControlpara->stPhase,pSignalControlpara->AscSignalTransTable);
    

    //加载通道参数 
    ReadChannelItem(pSignalControlpara->stChannel);
    

    //加载绿信比参数 
    ReadGreenSignalRationItem(pSignalControlpara->stGreenSignalRation);

    
    //To Do  相序表
    ReadPhaseTurnItem(pSignalControlpara->stPhaseTurn);
    

    //加载方案表参数 
    ReadSchemeItem(pSignalControlpara->stScheme);

    //加载动作表参数
    ReadActionItem(pSignalControlpara->stAction);

    //加载时段表参数
    ReadTimeIntervalItem(pSignalControlpara->stTimeInterval);
    
    
    //加载调度参数
    ReadPlanSchedule(pSignalControlpara->stPlanSchedule);

    //加载跟随参数
    ReadFollowPhaseItem(pSignalControlpara->stFollowPhase);

    //
    ReadVehicleDetector(pSignalControlpara->AscVehicleDetectorTable);

    //
    //ReadPedestrianDetector(pSignalControlpara->AscPedestrianDetectorTable,MAX_PEDESTRIANDETECTOR_COUNT);

	sqlite3_close_wrapper(g_pdatabase);
	g_pdatabase = NULL;
	
	memset(&zero, 0, sizeof(SignalControllerPara));
	if (memcmp(pSignalControlpara, &zero, sizeof(SignalControllerPara)) == 0) {	//说明没有配置信息
		//log_error("config file is not exist or there is no  config information in file . \n");
		return FALSE;
	}	
	
	return TRUE;
}


