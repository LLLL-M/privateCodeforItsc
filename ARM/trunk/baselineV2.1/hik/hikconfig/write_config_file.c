#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>
#include "HikConfig.h"
//#include "parse_ini.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

#define MAX_ARRAY_LEN	64
#define MAX_SECTION_LEN	64

#define DEG(fmt,...) fprintf(stderr,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)


extern sqlite3* g_pdatabase;

//#define PRINT_ARGS      //打印信息开关

/*****************************************************************************
 函 数 名  : StoreBitValueToArray
 功能描述  : 把数值value中从第1bit开始的bit值等于1的bit位置存入到数组中
 输入参数  : const unsigned int value  数值
             const int num             要存储的最大bit位置
             unsigned char *array      数组
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline int StoreBitValueToArray(const unsigned int value, 
									   const int num,
									   unsigned char *array)
{
	int i, v = 0;
	
	memset(array, 0, MAX_ARRAY_LEN);
	for (i = 1; i <= num; i++) 
	{
		if (BIT(value, i) == 0)
			continue;
		if (num == 7)
		{	//当value表示存放星期的数值时，bit1-7分别表示星期日到星期六，并不是星期一到星期日
			array[v++] = (i == 1) ? 7 : (i - 1);
		}
		else
			array[v++] = (unsigned char)i;
	}
	return v;
}

/*****************************************************************************
 函 数 名  : StoreVaildValueToArray
 功能描述  : 把数组values中不等于0的数值存入到另一个数组array中
 输入参数  : const unsigned char *values  原始数组
             const int num                原始数组元素的个数
             unsigned char *array         要存入数据的数组
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline int StoreVaildValueToArray(const unsigned char *values,
										 const int num,
										 unsigned char *array)
{
	int i, v = 0;
	
	memset(array, 0, MAX_ARRAY_LEN);
	for (i = 0; i < num; i++) 
	{
		if (values[i] == 0)
			continue;
		array[v++] = values[i];
	}
	return v;
}

/*****************************************************************************
 函 数 名  : WritePlanSchedule
 功能描述  : 写调度表到配置文件，每次写入num个项，使用之前要调用ini解析库的-
             parse_start，使用完成要调用parse_end
 输入参数  : PPlanScheduleItem item  
             int num                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WritePlanSchedule(PlanScheduleItem *schedule)
{
	sqlite3_insert_schedule(g_pdatabase, schedule);
}

/*****************************************************************************
 函 数 名  : WriteTimeIntervalItem
 功能描述  : 写时段表到配置文件，每次写入num个项，调用之前应先调用ini解析库
             的parse_start，调用之后应调用parse_end
 输入参数  : TimeIntervalItem *item  
             int num                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WriteTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID])
{
	sqlite3_insert_timeinterval(g_pdatabase, time_interval);
}

/*****************************************************************************
 函 数 名  : WriteActionItem
 功能描述  : 写动作表到配置文件，每次写入num个项，调用之前应先调用ini解析库
             的parse_start，调用之后应调用parse_end
 输入参数  : PActionItem item  
             int num           
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WriteActionItem(ActionItem *action)
{
	sqlite3_insert_action(g_pdatabase, action); 
}

/*****************************************************************************
 函 数 名  : WriteSchemeItem
 功能描述  : 写方案表到配置文件，每次写入num个项，调用之前应先调用ini解析库
             的parse_start，调用之后应调用parse_end
 输入参数  : PSchemeItem item  
             int num           
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WriteSchemeItem(SchemeItem *scheme)
{
	sqlite3_insert_scheme(g_pdatabase, scheme);
}

/*****************************************************************************
 函 数 名  : WriteChannelItem
 功能描述  : 写通道表到配置文件，每次写入num个项，调用之前应先调用ini解析库
             的parse_start，调用之后应调用parse_end
 输入参数  : PChannelItem item  
             int num            
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WriteChannelItem(ChannelItem channel[][NUM_CHANNEL])
{
 	sqlite3_insert_channel(g_pdatabase, channel);

}

/*****************************************************************************
 函 数 名  : WriteGreenSignalRationItem
 功能描述  : 写绿信比表到配置文件，每次写入num个项，调用之前应先调用ini解析
             库的parse_start，调用之后应调用parse_end
 输入参数  : GreenSignalRationItem *item  
             int num                      
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

  2.日    期   : 2015年3月18日
    作    者   : 肖文虎
    修改内容   : 按照吉凯的方式修改形参
*****************************************************************************/
void WriteGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE])
{
	sqlite3_insert_green_split(g_pdatabase, green_split);
}

/*****************************************************************************
 函 数 名  : WritePhaseItem
 功能描述  : 写相位表到配置文件，每次写入num个项，调用之前应先调用ini解析库
             的parse_start，调用之后应调用parse_end
 输入参数  : PPhaseItem item                      
             struct STRU_SignalTransEntry *entry  包含绿闪时间的结构体指针
             int num                              
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

  2.日    期   : 2015年2月4日
    作    者   : 肖文虎
    修改内容   : 添加最小绿等其他参数的保存

  3.日    期   : 2015年3月18日
    作    者   : 肖文虎
    修改内容   : 判断相位是否为空的方法改为相位是否使能，只有相位使能后才写
                 入文件中。
*****************************************************************************/
void WritePhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE])
{
	sqlite3_insert_phase(g_pdatabase, item);
  	sqlite3_insert_signal_trans(g_pdatabase, entry);
}

/*****************************************************************************
 函 数 名  : WriteUnitPara
 功能描述  : 写单元参数到配置文件，每次写入num个项，调用之前应先调用ini解析
             库的parse_start，调用之后应调用parse_end
 输入参数  : PUnitPara item  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

  2.日    期   : 2015年2月4日
    作    者   : 肖文虎
    修改内容   : 新增流量采集周期及过渡周期的存储
*****************************************************************************/
void WriteUnitPara(PUnitPara item)
{
	sqlite3_insert_unit(g_pdatabase, item);
}

/*****************************************************************************
 函 数 名  : WriteFollowPhaseItem
 功能描述  : 写跟随相位表到配置文件，每次写入num个项，调用之前应先调用ini解
             析库的parse_start，调用之后应调用parse_end
 输入参数  : PFollowPhaseItem item  
             int num                
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WriteFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE])
{
	sqlite3_insert_follow_phase(g_pdatabase, follow_phase); 
}

/*****************************************************************************
 函 数 名  : WritePhaseTurnItem
 功能描述  : 写相序表到配置文件，每次写入num个项，调用之前应先调用ini解析库
             的parse_start，调用之后应调用parse_end
 输入参数  : PhaseTurnItem *item  
             int num              
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WritePhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT])
{
	sqlite3_insert_phase_turn(g_pdatabase, phase_turn);
}

/*****************************************************************************
 函 数 名  : WriteVehicleDetector
 功能描述  : 写车辆检测器表到配置文件，每次写入num个项，调用之前应先调用ini
             解析库的parse_start，调用之后应调用parse_end
 输入参数  : struct STRU_N_VehicleDetector *item  
             int num                              
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WriteVehicleDetector(struct STRU_N_VehicleDetector *vehicle)
{
	sqlite3_insert_vehicle(g_pdatabase, vehicle);
}

/*****************************************************************************
 函 数 名  : WritePedestrianDetector
 功能描述  : 写行人检测器表到配置文件，每次写入num个项，调用之前应先调用ini
             解析库的parse_start，调用之后应调用parse_end
 输入参数  : struct STRU_N_PedestrianDetector *item  
             int num                                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void WritePedestrianDetector(struct STRU_N_PedestrianDetector *item)
{

}

/*****************************************************************************
 函 数 名  : WriteConfigFile
 功能描述  : 写所有的配置项到配置文件，无需再调用parse_start和parse_end
 输入参数  : SignalControllerPara *param  总参数结构体指针
             const char *path             要写入的配置文件路径，如果不指定默认为/config.ini
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
Boolean WriteConfigFile(SignalControllerPara *param)
{
	sqlite3_open_wrapper(DATABASE_HIKCONFIG, &g_pdatabase);
	//unlink(filename);	//删除之前的配置文件，因为要重新写入
	
	WriteUnitPara(&(param->stUnitPara));	//写单元参数

    WritePhaseItem(param->stPhase, param->AscSignalTransTable);	//写相位表

    WriteChannelItem(param->stChannel);	//写通道表

	WriteGreenSignalRationItem(param->stGreenSignalRation);	//写绿信比表
	
	WritePhaseTurnItem(param->stPhaseTurn);	//写相序表
	
	WriteSchemeItem(param->stScheme);	//写方案表

	WriteActionItem(param->stAction);	//写动作表

	WriteTimeIntervalItem(param->stTimeInterval);	//写时间表
	
	WritePlanSchedule(param->stPlanSchedule);	//写调度表

    WriteFollowPhaseItem(param->stFollowPhase);	//写跟随相位表

    WriteVehicleDetector(param->AscVehicleDetectorTable);//写车辆检测器
    
    WritePedestrianDetector(param->AscPedestrianDetectorTable);//写行人检测器

	sqlite3_close_wrapper(g_pdatabase);
	g_pdatabase = NULL;

    return TRUE;
}

