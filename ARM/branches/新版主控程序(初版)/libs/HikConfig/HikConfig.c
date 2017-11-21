/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : HikConfig.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月2日
  最近修改   :
  功能描述   : 本文件完成对配置文件进行读取、参数校验的接口
  函数列表   :
              ArrayToInt
              checkPhaseTrunInRing
              DateTimeCmp
              find_next_index
              GetCircleTime
              IsArrayRepeat
              IsBarrierGreenSignalRationEqual
              IsItemInCharArray
              IsItemInShortArray
              IsPhaseContinuousInPhaseTurn
              isPhaseInArray
              IsSignalControllerParaIdLegal
              IsSignalControlparaLegal
              LoadDataFromCfg
              log_error
  修改历史   :
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

  2.日    期   : 2014年12月3日
    作    者   : 肖文虎
    修改内容   : 按照新的结构体，修改相关接口
******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "parse_ini.h"
#include "HikConfig.h"
#include <stdarg.h>
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define COL_DEF "\033[m"
#define COL_RED "\033[0;32;31m"
#define COL_GRE "\033[0;32;32m"
#define COL_BLU "\033[0;32;34m"
#define COL_YEL "\033[1;33m"

#define DEG(fmt,...) fprintf(stdout,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
char *gErrorContent = NULL;//该全局变量存储错误信息，供调用者进行分析

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*****************************************************************************
 函 数 名  : log_error
 功能描述  : 打印错误信息接口，其中错误内容会 保存到全局缓冲区，外部接口可直
             接查看其内容。
 输入参数  : const char* format  
             ...                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数
  2.日    期   : 2014年12月10日
    作    者   : 肖文虎
    修改内容   : 修改缓存区大小为1024字节
  3.日    期   : 2014年12月10日
    作    者   : 肖文虎
    修改内容   : 将多个strcat连接字符串改为一个sprintf连接所有字符串	
*****************************************************************************/
static void log_error(const char* format, ...)
{
	char buff[1024] = {0};
    if(NULL == gErrorContent)
    {
        gErrorContent = calloc(1024,1);

    }
	sprintf(buff,"%s HikConfig library error: %s <ERROR>  ",COL_BLU,COL_YEL);
	va_list argptr;
	va_start(argptr, format);

    if(NULL != gErrorContent)
    {
    	memset(gErrorContent,0,1024);
    	vsnprintf(gErrorContent,1024,format,argptr);
    }

	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//将buff的位置向右移动strlen(buff)字节，最前面这24字节用来存放当前时间，紧接着是2字节的空格用来分割内容和时间
	va_end(argptr);
	strcat(buff,COL_DEF);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);

}

/*****************************************************************************
 函 数 名  : DateTimeCmp
 功能描述  : 比较两个调度时间之间是否有重叠
 输入参数  : PPlanScheduleItem item_1  
             PPlanScheduleItem item_2  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static int DateTimeCmp(PPlanScheduleItem item_1,PPlanScheduleItem item_2)
{
    int i = 1;
    int j = 1;
    int k = 1;
    
    for(i = 1 ; i <= 12 ; i++)
    {
        if((1 == BIT(item_1->month,i)) && (1 == BIT(item_2->month,i)))
        {
            for(j = 1 ; j <= 31; j++)
            {
                if((1 == BIT(item_1->day,j)) && (1 == BIT(item_2->day,j)))
                {
                    return 1;
                }
            }
            for(k = 1; k <= 7; k++)
            {
                if((1 == BIT(item_1->week,k)) && (1 == BIT(item_2->week,k)))
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : GetCircleTime
 功能描述  : 根据绿信比号和相序号得到相序表中，环号不为0的绿信比时间之和
 输入参数  : SignalControllerPara *pSignalControlpara  
             unsigned char nGreenSignalRationId        
             unsigned char nPhaseTurnId                
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static int GetCircleTime(SignalControllerPara *pSignalControlpara,unsigned char nGreenSignalRationId,unsigned char nPhaseTurnId)
{

    int i = 0; 
    int j = 0;

    int temp = 0;

    for(i = 0 ; i < 4; i++)//轮询环
    {
        if(pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nPhaseTurnID == 0)//找到有实际数据的环
        {
            continue;

        }    
        
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            if(pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] == 0)//轮询一个环就直接结束
            {
                return temp;
            }

            
            temp += pSignalControlpara->stGreenSignalRation[nGreenSignalRationId - 1][pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] - 1].nGreenSignalRationTime;

        }

    }

    return  temp;
}



/*****************************************************************************
 函 数 名  : IsBarrierGreenSignalRationEqual
 功能描述  : 判断一个屏障两侧的绿信比时间是否相同
 输入参数  : SignalControllerPara *pSignalControlpara  
             unsigned char *cArray                     
             int len                                   
             unsigned char nCircleId                   
 输出参数  : 无
 
 修改历史      :
  1.日    期   : 2014年8月8日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static int IsBarrierGreenSignalRationEqual(SignalControllerPara *pSignalControlpara,unsigned char *cArray,int len,unsigned char nCircleId)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int m = 0;
    
    int temp = 0;
    int temp2 = 0;
    int temp3 = 0;

    unsigned char cTempArray[4][NUM_PHASE];//四个环，每个环存放num_phase个相位

    memset(cTempArray,0,sizeof(cTempArray));

    //给并发相位分组，按照环的大小顺序
    for(i = 0 ; i < len; i++)
    {
        if(cArray[i] == 0)
        {
            break;
        }

        temp = pSignalControlpara->stPhase[cArray[i] - 1].nCircleID;//找到环号

        cTempArray[temp - 1][cArray[i] - 1] = cArray[i];

       // DEG("===>  cTeampArray   %d\n",cTempArray[temp - 1][cArray[i] - 1]);
    }

    //一个环内的相位的并发相位绿信比之和，应该和该环的相位的绿信比之和相等
    for(m = 0 ; m < NUM_GREEN_SIGNAL_RATION; m++)//轮询绿信比表，找到某个相位的绿信比时间
    {
        temp = 0;
        temp2 = 0;
        temp3 = 0;
        
        for(i = 0 ; i < 4; i++)//轮询环
        {
            temp = 0;
            temp2 = 0;
            
            for(j = 0 ;  j < NUM_PHASE; j++)//轮询环里面的相位
            {
                if(cTempArray[i][j] == 0)
                {
                    continue;
                }

    			if(pSignalControlpara->stGreenSignalRation[m][cTempArray[i][j] - 1].nGreenSignalRationID == 0)
    			{
    				continue;
    			}

    			temp += pSignalControlpara->stGreenSignalRation[m][cTempArray[i][j] - 1].nGreenSignalRationTime;//根据相位号，找到该相位的绿信比时间

    			if(temp2 == 0)
    			{
    				for(k = 0; k < NUM_PHASE; k++)//轮询该相位的并发相位
    				{
    					temp3 = pSignalControlpara->stPhase[cTempArray[i][j] - 1].byPhaseConcurrency[k];//找到该相位对应的并发相位
    					if(temp3 == 0)
    					{
    						break;
    					}

    					if(pSignalControlpara->stPhase[temp3 - 1].nCircleID == nCircleId)//如果该并发相位的环号和形参相同，则求其绿信比之和
    					{
    						temp2 += pSignalControlpara->stGreenSignalRation[m][temp3 - 1].nGreenSignalRationTime;//求并发相位的绿信比之和
    					}
    				    //DEG("====>temp %d temp2   %d   phase  %d\n",temp,temp2,temp3);
    				}
    			}
                //DEG("=========  temp  %d\n",temp);

    		}
    		
            if(temp != temp2)
            {
                return  1;
            }
        }
    }
    return 0;
}

/*****************************************************************************
 函 数 名  : IsItemInShortArray
 功能描述  : 判断某变量是否在一个short型数组中。
 输入参数  : unsigned short *array  
             int length             
             int val                
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static int IsItemInShortArray(unsigned short *array,int length,int val)
{
	if(!array)
	{
		return False;
	}

	int i = 0;
	for(i = 0 ; i < length ; i++)
	{
		if(array[i] == val)
		{
			return TRUE;
		}

	}

	return FALSE;
}

/*****************************************************************************
 函 数 名  : IsItemInCharArray
 功能描述  : 判断一个char型数据是否在一个char型数组中。
 输入参数  : unsigned char *array   
             unsigned short length  
             unsigned short val     
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static int IsItemInCharArray(unsigned char *array,unsigned short length,unsigned short val)
{
	if(!array)
	{
		return FALSE;
	}

	unsigned short i = 0;
	for(i = 0 ; i < length ; i++)
	{
		if(array[i] == val)
		{
			return i+1;
		}

	}
	return FALSE;

}

/*****************************************************************************
 函 数 名  : IsPhaseContinuousInPhaseTurn
 功能描述  : 判断相序表中的相序是否按照并发相位里指定的并发顺序
 输入参数  : SignalControllerPara *pSignalControlpara  
             unsigned char nCircleId                   
             unsigned char *cArray                     
             int len                                   
 输出参数  : 无
 
 修改历史      :
  1.日    期   : 2014年8月8日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static int IsPhaseContinuousInPhaseTurn(SignalControllerPara *pSignalControlpara,unsigned char nCircleId,unsigned char *cArray,int len)
{
    int i = 0;
    int j = 0;

    unsigned short sTemp = 0;
    unsigned short sTemp2 = 0;
    unsigned char num = 0;
    //非零数据的个数
    for(i = 0 ; i < len; i++)
    {
       // DEG("===>   %d  :   %d  len %d\n",cArray[i],pSignalControlpara->stPhase[cArray[i] - 1].nCircleID,len);
        if((cArray[i] != 0) && (pSignalControlpara->stPhase[cArray[i] - 1].nCircleID == nCircleId))
        {
            sTemp2++;
        }
    }

    for(i = 0 ; i < NUM_PHASE_TURN; i++)
    {
        sTemp = pSignalControlpara->stPhaseTurn[i][nCircleId -1].nPhaseTurnID ;
        if(sTemp == 0)
        {
            continue;
        }

        for(j = 0 ; j < NUM_PHASE; j++)
        {
            sTemp = pSignalControlpara->stPhaseTurn[i][nCircleId -1].nTurnArray[j];
            if(sTemp == 0)
            {
                break;
            }
           // DEG("===>  stemp   %d\n",sTemp);
            if(IsItemInCharArray(cArray,len, sTemp) > 0)
            {
                num++;

                if(num == sTemp2)
                {
                    return 1;
                }
            }
            else
            {
                num = 0;
            }

        }
    }
   // DEG("===> circleId   %d  cArray[0]   %d  sTemp2  %d  num  %d\n",nCircleId,cArray[0],sTemp2,num);
    return 0;
}

/*****************************************************************************
 函 数 名  : IsArrayRepeat
 功能描述  : 判断数组是否有重复元素
 输入参数  : unsigned char *cArray  
             int len                
 输出参数  : 无
 
 修改历史      :
  1.日    期   : 2014年8月8日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static int IsArrayRepeat(unsigned char *cArray,int len)
{
    int i = 0;
    int j = 0;

    unsigned short cTemp = 0;
        
    for(i = 0 ; i < len ; i++)
    {
        cTemp = cArray[i];

        if(cTemp == 0)
        {
            continue;
        }

        for(j = i+1 ; j < len ; j++)
        {
            if(cTemp == cArray[j])
            {
                return 1;
            }
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : IsSignalControllerParaIdLegal
 功能描述  : 单独分离这个函数，用来判断参数的下标是否是索引。
 输入参数  : SignalControllerPara *pSignalControlpara  
             unsigned short *nPhaseArray               
             unsigned short *nFollowPhaseArray         
 输出参数  : 无
 
 修改历史      :
  1.日    期   : 2014年8月8日
    作    者   : xiaowh
    修改内容   : 新生成函数

  2.日    期   : 2014年12月3日
    作    者   : 肖文虎
    修改内容   : 按照新的结构体，重新校验


  3.日    期   : 2015年2月6日
    作    者   : 肖文虎
    修改内容   : 修改判断各表是否为空的方法
*****************************************************************************/
static int IsSignalControllerParaIdLegal(SignalControllerPara *pSignalControlpara,unsigned short *nPhaseArray,unsigned short *nFollowPhaseArray)
{
    int i = 0;
    int j = 0;
    
    unsigned short sTemp = 0;
    unsigned short sTemp2 = 0;

    //相位数组下标是否和相位号相同
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        nPhaseArray[i] = pSignalControlpara->stPhase[i].nPhaseID;
        if((pSignalControlpara->stPhase[i].nCircleID != 0) &&(nPhaseArray[i] != (i+1)))
        {
            log_error("%s:%d stPhase[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);
            
            return ERROR_SUBSCRIPT_PHASE;
        }
    }

    //并发相位数组下标是否和相位号相同
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        //并发相位重复
        if(IsArrayRepeat(pSignalControlpara->stPhase[i].byPhaseConcurrency,NUM_PHASE) == 1)
        {
            log_error("%s:%d stConcurrentPhase[%d]  has repeat items \n",__func__,__LINE__,i+1);
            return ERROR_REPEATE_CONCURRENT_PHASE;
        }

    }

    //相序表    相序表相序有重复
    for(i = 0 ; i < NUM_PHASE_TURN; i++)
    {
        for(j = 0 ; j < NUM_RING_COUNT; j++)
        {
            sTemp = pSignalControlpara->stPhaseTurn[i][j].nPhaseTurnID;
            sTemp2 = pSignalControlpara->stPhaseTurn[i][j].nCircleID;

            if(sTemp2 == 0)
            {
                continue;
            }

            if(sTemp != (i+1))
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d]   subscript is not equal to nPhaseTurnID  \n ",__func__,__LINE__,i+1,j+1);

                return ERROR_SUBSCRIPT_PHASE_TURN;
            }

            if(sTemp2 != (j+1))
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d] subscript is not equal to nCircleID\n ",__func__,__LINE__,i+1,j+1);

                return ERROR_SUSCRIPT_PHASE_TURN_CIRCLE;
            }
            
            if(IsArrayRepeat(pSignalControlpara->stPhaseTurn[i][j].nTurnArray,16) == 1)
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d]  has repeat item \n ",__func__,__LINE__,i+1,j+1);
                return ERROR_REPEATE_PHASE_TURN;
            }
        }
    }

    

    //绿信比表
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION; i++)
    {
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            sTemp = pSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationID;
            sTemp2 = pSignalControlpara->stGreenSignalRation[i][j].nPhaseID;
            
            if((sTemp2 != 0) && (sTemp != (i+1)))
            {
                log_error("%s:%d stGreenSignalRation[%d] : nPhaseId[%d]   nGreenSignalRationID  subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);
    
                return ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION;
            }
            
            if((sTemp2 != 0) && (sTemp2 != (j+1)))
            {
                log_error("%s:%d stGreenSignalRation[%d] : nPhaseId[%d]    nPhaseID  subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);
    
                return ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION_PHASE;
            }        

         }
    }

    //Channel 
    for(i = 0 ; i < NUM_CHANNEL; i++)
    {
        sTemp = pSignalControlpara->stChannel[i].nChannelID;

        if((pSignalControlpara->stChannel[i].nControllerID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stChannel[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);

            return ERROR_SUBSCRIPT_CHANNEL;
        }
    }

    //stScheme
    for(i = 0 ; i < NUM_SCHEME; i++)
    {
        sTemp = pSignalControlpara->stScheme[i].nSchemeID;

        if((pSignalControlpara->stScheme[i].nGreenSignalRatioID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stScheme[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);


            return ERROR_SUBSCRIPT_SCHEME;
        }
    }

    //stAction
    for(i = 0 ; i < NUM_ACTION; i++)
    {
        sTemp = pSignalControlpara->stAction[i].nActionID;

        if((pSignalControlpara->stAction[i].nSchemeID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stScheme[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);


            return ERROR_SUBSCRIPT_ACTION;
        }
    }

    //stTimeInterval
    for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID; j++)
        {
            sTemp = pSignalControlpara->stTimeInterval[i][j].nTimeIntervalID ;
            sTemp2 = pSignalControlpara->stTimeInterval[i][j].nTimeID;
            
            if((pSignalControlpara->stTimeInterval[i][j].nActionID != 0) && (sTemp != (i+1)))
            {
                log_error("%s:%d stTimeInterval[%d]:nTimeID[%d] nTimeIntervalID subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);


                return ERROR_SUBSCRIPT_TIMEINTERVAL;
            }
            if((pSignalControlpara->stTimeInterval[i][j].nActionID != 0) && (sTemp2 != (j+1)))
            {
                log_error("%s:%d stTimeInterval[%d]:nTimeID[%d] nTimeID subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);


                return ERROR_SUBSCRIPT_TIMEINTERVAL_TIME;
            }

            
        }

    }

    //stPlanSchedule
    for(i = 0 ; i < NUM_SCHEDULE; i++)
    {
        sTemp = pSignalControlpara->stPlanSchedule[i].nScheduleID;

        if((pSignalControlpara->stPlanSchedule[i].nTimeIntervalID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stPlanSchedule[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);


            return ERROR_SUBSCRIPT_SCHEDULE;
        }
    }
    
    //FollowPhase
    for(i = 0 ; i < NUM_FOLLOW_PHASE ; i++)
    {
        nFollowPhaseArray[i] = pSignalControlpara->stFollowPhase[i].nFollowPhaseID;

        if((pSignalControlpara->stFollowPhase[i].nArrayMotherPhase[0] != 0 ) && (nFollowPhaseArray[i] != (i+1)))
        {
            log_error("%s:%d stFollowPhase[%d]  subscript is not equal to ID \n ",__func__,__LINE__,i+1);

            return ERROR_SUBSCRIPT_FOLLOW_PHASE;
        }

        if(IsArrayRepeat(pSignalControlpara->stFollowPhase[i].nArrayMotherPhase,NUM_PHASE) == 1)
        {
            log_error("%s:%d stFollowPhase[%d]  has repeat items \n ",__func__,__LINE__,i+1);
            return ERROR_REPEATE_FOLLOW_PHASE;
        }

    }


    return 0;
}

/*****************************************************************************
 函 数 名  : find_next_index
 功能描述  : 从当前环的相序中找出下一个屏障起始相位在相序数组中的索引
 输入参数  : SignalControllerPara *pSignalControlpara  总配置参数的结构体指针
             unsigned char *phaseArray                 环的相序数组
             unsigned short index                      起始的索引值
 返 回 值  : 找到则返回索引值，找不到则返回0
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static int find_next_index(SignalControllerPara *pSignalControlpara,unsigned char *phaseArray, unsigned short index)
{
	int i,  ret = 0;
	unsigned short phaseId = phaseArray[index];
	PPhaseItem item = &pSignalControlpara->stPhase[phaseId - 1];
	
	if (item->nPhaseID != phaseId) 
	{
		return index + 1;
	}
	phaseId = item->byPhaseConcurrency[0];
	item = &pSignalControlpara->stPhase[phaseId - 1];
	
	for (i = index + 1; i < NUM_PHASE; i++) 
	{
		if (phaseArray[i] == 0) 
		{
			ret = 0;
			break;
		}
		if (IsItemInCharArray(item->byPhaseConcurrency,NUM_PHASE, phaseArray[i]) == FALSE) 
		{
			ret = i;
			break;
		}
	}
	return ret;
}

/*****************************************************************************
 函 数 名  : checkPhaseTrunInRing
 功能描述  : 检查相序表中每个环中的相序是否并发正确
 输入参数  : SignalControllerPara *pSignalControlpara  总配置参数的结构体指针
             unsigned short phaseTurn                  相序表号
 返 回 值  : 若是相序表中每个环中的相序全部并发正确则返回True，反之返回False
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static int checkPhaseTrunInRing(SignalControllerPara *pSignalControlpara,unsigned short phaseTurn)
{
	PPhaseTurnItem item = NULL;
	int i = 0;
	unsigned short fatherPhase = 0;
	int indexs[4] = {0};	//存放每个环中对应屏障的起始相位的索引
	unsigned short ringPhase[4] = {0};	//存放每个环中对应屏障的起始相位，用来判断这些环是否是并发关系
	

	while (1) 
	{	//提取每个环中对应屏障的起始相位存放在数字ringPhase中，然后找寻下一个屏障每个环中的起始相位的索引
		for (i = 0; i < 4; i++) 
		{
			item = &pSignalControlpara->stPhaseTurn[phaseTurn - 1][i];
			if (item->nPhaseTurnID != phaseTurn || item->nCircleID != (i + 1))
				continue;
			ringPhase[i] = item->nTurnArray[indexs[i]];
			indexs[i] = find_next_index(pSignalControlpara,item->nTurnArray, indexs[i]);
		}
		//判断ringPhase数组中提取的相位是否并发
		//printf("DEBUG: ringPhase = {%d, %d, %d, %d}\n", ringPhase[0], ringPhase[1], ringPhase[2], ringPhase[3]);
		//printf("DEBUG: indexs = {%d, %d, %d, %d}\n", indexs[0], indexs[1], indexs[2], indexs[3]);
		for (i = 0; i < 4; i++) 
		{
			if (ringPhase[i] == 0)
				continue;
			if (fatherPhase == 0) 
			{
				fatherPhase = ringPhase[i];
			} 
			else 
			{
				if (IsItemInCharArray(pSignalControlpara->stPhase[fatherPhase - 1].byPhaseConcurrency, NUM_PHASE, ringPhase[i]) == FALSE)
				{
#if 0
					printf("DEBUG: fatherPhase = %d, correspond = {%d, %d}, ringPhase[%d] = %d\n",
							fatherPhase,
							pSignalControlpara->stPhase[fatherPhase - 1].byPhaseConcurrency[0],
							pSignalControlpara->stPhase[fatherPhase - 1].byPhaseConcurrency[1],
							i, ringPhase[i]);
#endif
					return FALSE;
				}
			}
			
		}
		fatherPhase = 0;
		//当每个环中对应屏障的起始相位的索引四个都为0时，说明判断结束
		if (indexs[0] == 0 && indexs[1] == 0 && indexs[2] == 0 && indexs[3] == 0)
		    break;
	}
	return TRUE;
}


/*****************************************************************************
 函 数 名  : IsSignalControlparaLegal
 功能描述  : 全局信号控制参数合法性检查
 输入参数  : SignalControllerPara *pSignalControlpara  

1.  各个表格数组下标是否和ID相同;
2.  并发相位的相位是否存在;
3.  并发相位中是否存在并发冲突: 同一个环内的相位不能并发，相位号和环号是否与相位表对应;
4.  相序表的相位序号是否存在;
5.  环号和相位序号是否对应;
6.  绿信比表中某相位的绿信比时间是否比该相位的绿闪时间、黄灯时间、全红时间之和要大，或者比行人清空时间、行人放行时间之和大;
7.  通道表中控制源是否存在;
8.  方案表中绿信比号及相序表号是否存在;
9.  动作表中方案表是否存在;
10. 时段表中动作号是否存在;
11. 调度表中时段表号是否存在;
12. 调度表中不同调度之间是否存在重叠现象;
13. 跟随相位中的母相位是否存在;
14. 控制源类型有误
15. 相序表相序有重复
16. 并发相位、跟随相位有重复
17. 并发相位是否配置符合逻辑
18. 在有并发相位的时候，相序表是否配置正确
19. 并发相位绿信比是否满足要求  这个要放到运行时检查，因为我不知道某个相位的具体绿信比时间是多少
 
 修改历史      :
  1.日    期   : 2014年8月6日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int IsSignalControlparaLegal(SignalControllerPara *pSignalControlpara)//合法的话，返回0，否则返回错误码
{
    int i = 0;
    int j = 0;
    int k = 0;
    
    unsigned short sTemp = 0;
    unsigned short sTemp2 = 0;

    unsigned short nPhaseArray[NUM_PHASE] = {0};//总共的相位数组
    unsigned short nFollowPhaseArray[NUM_FOLLOW_PHASE] = {0};//跟随相位数组

	SignalControllerPara zero;

    if(!pSignalControlpara)
    {
        log_error("%s  pSignalControlpara  is null  \n",__func__);
        
        return ERROR_NULL_POINTER;
    }

	memset(&zero, 0, sizeof(SignalControllerPara));
	if (memcmp(pSignalControlpara, &zero, sizeof(SignalControllerPara)) == 0) {	//说明没有配置信息
		log_error("You haven't configed! Please config it.\n");
		return ERROR_NOT_CONFIG_INFORMATION;
	}

    /*1.  各个表格数组下标是否和ID相同;*/
    k = IsSignalControllerParaIdLegal(pSignalControlpara,nPhaseArray,nFollowPhaseArray);
    if(k > 0)
    {
        return k;
    }

 
    /*结构体内部逻辑合法性检查*/

    /*2.  并发相位的相位是否存在;*/
    /*3.  并发相位中是否存在并发冲突: 同一个环内的相位不能并发，相位号和环号是否与相位表对应;*/
    /*环号是否和相位表中的不匹配*/
     /*并发相位是否配置符合逻辑*/
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        sTemp = pSignalControlpara->stPhase[i].nPhaseID;
        if(pSignalControlpara->stPhase[i].nCircleID == 0)
        {
            continue;
        }
        for(j = 0 ;j < NUM_PHASE ; j++)
        {
            sTemp = pSignalControlpara->stPhase[i].byPhaseConcurrency[j];
            sTemp2 = pSignalControlpara->stPhase[i].nPhaseID;
            if(sTemp == 0)
            {
                continue;
            }
            //phase is not exist
            if(IsItemInShortArray(nPhaseArray,NUM_PHASE,sTemp) == 0)
            {
                log_error("%s:%d stConcurrentPhase  phase  %d is not exist \n",__func__,__LINE__,sTemp);
                return ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE;
            }

            //phase is in the same circle
            if(pSignalControlpara->stPhase[sTemp - 1].nCircleID == pSignalControlpara->stPhase[i].nCircleID)
            {
                log_error("%s:%d stConcurrentPhase  nPhaseId %d :  ConcurrentPase  %d is in the same circle   \n",__func__,__LINE__,pSignalControlpara->stPhase[i].nPhaseID,sTemp);
                return ERROR_SAME_CIRCLE_CONCURRENT_PHASE;
            }

            //子相位的并发相位中没有父相位
            if(IsItemInCharArray(pSignalControlpara->stPhase[sTemp - 1].byPhaseConcurrency,NUM_PHASE, sTemp2) == 0)
            {
                log_error("%s:%d stConcurrentPhase[%d]  phase  %d is not in it's child array \n",__func__,__LINE__,i+1,sTemp2);
                return ERROR_FATHER_NOT_EXIST_CONCURRENT_PHASE;
            }

            //子相位在相序表中是否连续      
            if(IsPhaseContinuousInPhaseTurn(pSignalControlpara, pSignalControlpara->stPhase[sTemp - 1].nCircleID, pSignalControlpara->stPhase[i].byPhaseConcurrency, NUM_PHASE) == 0)
            {
                log_error("%s:%d stConcurrentPhase[%d]  phase is not  continuous in it's turn array ,circle Id is %d   %d \n",__func__,__LINE__,i+1,pSignalControlpara->stPhase[i].nCircleID,sTemp);
                return ERROR_CHILD_NOT_CONTINUOUS_CONCURRENT_PHASE;
            }

            //相序表中，屏障一遍的绿信比之和必须相等   放到运行时添加测试
            if(IsBarrierGreenSignalRationEqual(pSignalControlpara,pSignalControlpara->stPhase[i].byPhaseConcurrency, NUM_PHASE,pSignalControlpara->stPhase[i].nCircleID) == 1)
            {
                log_error("%s:%d stConcurrentPhase  Barrier GreenSignalRation is error ,please check it again \n",__func__,__LINE__);
                return ERROR_BARRIER_CONCURRENT_PHASE;
             }
        }
    }

    /*4.  相序表的相位序号是否存在;*/
    /*5.  环号和相位序号是否对应;*/
    for(i = 0 ; i < NUM_PHASE_TURN ; i++)
    {
        for(k = 0 ; k < NUM_RING_COUNT; k++)
        {
            if(pSignalControlpara->stPhaseTurn[i][k].nCircleID == 0)
            {
                continue;
            }
        
            for(j = 0 ; j < NUM_PHASE ; j++)
            {
                sTemp = pSignalControlpara->stPhaseTurn[i][k].nTurnArray[j];

                if(sTemp == 0)
                {
                    continue;
                }

                if(pSignalControlpara->stPhase[sTemp - 1].nCircleID == 0)
                {
                    log_error("%s:%d  stPhaseTurn nPhaseTurnID[%d]:Circle[%d]   stPhase  %d is not exist \n ",__func__,__LINE__,i+1,k+1,sTemp);
                    return ERROR_NOT_EXIST_PHASE_TURN_PHASE;
                }

                /*环号和相位序号是否对应;*/
                if(pSignalControlpara->stPhase[sTemp - 1].nCircleID != pSignalControlpara->stPhaseTurn[i][k].nCircleID)
                {
                    log_error("%s:%d  stPhaseTurn  nPhaseTurnID[%d]:Circle[%d]  nPhase %d  nCircleID  is not equal to stPhase \n ",__func__,__LINE__,i+1,k+1,sTemp);
                    return ERROR_NOT_EQUAL_PHASE_TURN_CIRCLE;
                }
            }

        }

        if(checkPhaseTrunInRing(pSignalControlpara,i+1) == FALSE)
        {
            log_error("%s:%d  stPhaseTurn  phase is not  correct in its turn array ",__func__,__LINE__);
            return ERROR_NOT_CORRECT_PHASE_TURN;
        }
    }

    /*6.  绿信比表中某相位的绿信比时间是否比该相位的绿闪时间、黄灯时间、全红时间之和要大，或者比行人清空时间、行人放行时间之和大;*/
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION ; i++)
    {
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            sTemp = pSignalControlpara->stGreenSignalRation[i][j].nPhaseID;

            if(sTemp == 0)
            {
                continue;
            }
            
            sTemp2 = pSignalControlpara->AscSignalTransTable[sTemp - 1].nGreenLightTime + pSignalControlpara->stPhase[sTemp - 1].nYellowTime +  pSignalControlpara->stPhase[sTemp - 1].nAllRedTime;
            if(pSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationTime < sTemp2)
            {
                log_error("%s:%d  stGreenSignalRation  %d  is lower than sTemp2  Moto  %d\n",__func__,__LINE__,sTemp,sTemp2);
                return ERROR_SPLIT_LOW_MOTO_GREEN_SIGNAL_RATION;
            }
        }
    }

    /*7.  通道表中控制源是否存在;*/
     /* 控制源类型有误*/
    for(i = 0 ; i < NUM_CHANNEL ; i++)
    {
        sTemp = pSignalControlpara->stChannel[i].nControllerID ;

        if(sTemp == 0)
        {
            continue;
        }

        if((pSignalControlpara->stChannel[i].nControllerType == MOTOR ) || (pSignalControlpara->stChannel[i].nControllerType == PEDESTRIAN))
        {
            if(IsItemInShortArray(nPhaseArray,NUM_PHASE,sTemp) == 0)
            {
                log_error("%s:%d  stChannel[%d]   controller id is not exist  \n ",__func__,__LINE__,i+1);
                return ERROR_NOT_EXIST_SOURCE_CHANNEL;
            }
        }
        else if (pSignalControlpara->stChannel[i].nControllerType == FOLLOW)
        {
            if(IsItemInShortArray(nFollowPhaseArray,NUM_FOLLOW_PHASE,sTemp) == 0)
            {
                log_error("%s:%d  stChannel[%d]   controller id is not exist  \n ",__func__,__LINE__,i+1);
                return ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL;
            }

        }
        else if(OTHER == pSignalControlpara->stChannel[i].nControllerType)
        {
            //to do
        }
        else
        {
            log_error("%s:%d  stChannel[%d]   controller nControllerType is not exist  \n ",__func__,__LINE__,i+1);
            return 30;
        }
    }
    
    /*8.  方案表中绿信比号及相序表号是否存在;*/
    for(i = 0 ; i < NUM_SCHEME; i++)
    {
        sTemp = pSignalControlpara->stScheme[i].nSchemeID;

        if(sTemp == 0)
        {
            continue;
        }

        sTemp = pSignalControlpara->stScheme[i].nPhaseTurnID;
        sTemp2 =  pSignalControlpara->stScheme[i].nGreenSignalRatioID;

        if((sTemp <= 0) || (sTemp2 <= 0))
        {
            log_error("%s:%d   stScheme  nPhaseTurnID  %d  or nGreenSignalRatioID  is not exist\n",__func__,__LINE__,sTemp,sTemp2);
            return ERROR_ILLEGAL_SCHME;
        }
        if(pSignalControlpara->stPhaseTurn[sTemp - 1][0].nPhaseTurnID == 0)
        {
            log_error("%s:%d  stScheme[%d]  nPhaseTurnID  %d is not exist  \n  ",__func__,__LINE__,i+1,sTemp);
            return ERROR_NOT_EXIST_PHASE_TURN_SHCEME;
        }
        if(pSignalControlpara->stGreenSignalRation[sTemp2 - 1][0].nGreenSignalRationID == 0)
        {
            log_error("%s:%d  stScheme[%d]  stGreenSignalRation  %d is not exist  \n",__func__,__LINE__,i+1,sTemp2);
            return ERROR_NOT_EXIST_GREEN_SIGNAL_RATION_SCHEME;
        }
        //方案表中的周期长是否等于相序表中单环绿信比时间之和
        if(GetCircleTime(pSignalControlpara,sTemp2,sTemp) != pSignalControlpara->stScheme[i].nCycleTime)
        {
            log_error("%s:%d  stScheme[%d]  cycle time is not correct  ,should be %d\n",__func__,__LINE__,i+1,GetCircleTime(pSignalControlpara,sTemp2,sTemp));
            return ERROR_CIRCLE_TIME_SCHEME;
        }
        
    }

    /*9.  动作表中方案表是否存在;*/
    for(i = 0 ; i < NUM_ACTION ; i++)
    {
        sTemp = pSignalControlpara->stAction[i].nActionID;

        if(sTemp == 0)
        {
            continue;
        }

        sTemp2 = pSignalControlpara->stAction[i].nSchemeID;

        if((sTemp2 >= 251) && (sTemp2 <= 255))//特殊方案号
        {
            continue;
        }

        if((sTemp2 < 1) || (pSignalControlpara->stScheme[sTemp2  -1].nSchemeID == 0))
        {
            log_error("%s:%d  stAction[%d]  nSchemeID  %d is not exist  \n",__func__,__LINE__,i+1,sTemp2);
            return ERROR_NOT_EXIST_SCHEME_ACTION;

        }
    }

    /*10. 时段表中动作号是否存在;*/
    for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID; j++)
        {
            sTemp = pSignalControlpara->stTimeInterval[i][j].nActionID;

            if((sTemp == 0) || ((sTemp >= 201) && (sTemp <= 204)))
            {
                continue;
            }

            if(pSignalControlpara->stAction[sTemp - 1].nActionID == 0)
            {
                log_error("%s:%d stTimeInterval[%d]:nTimeID[%d]  nActionId   %d  is not exist \n ",__func__,__LINE__,i+1,j+1,sTemp);
                return ERROR_NOT_EXIST_ACTION_TIMEINTERVAL;
            }
        }
    }

    /*11. 调度表中时段表号是否存在;*/
    for(i = 0 ;  i < NUM_SCHEDULE; i++)
    {
        sTemp = pSignalControlpara->stPlanSchedule[i].nScheduleID;

        if(sTemp == 0)
        {
            continue;
        }

        sTemp2 = pSignalControlpara->stPlanSchedule[i].nTimeIntervalID;

        if((sTemp2 <= 0) || (pSignalControlpara->stTimeInterval[sTemp2 - 1][0].nTimeIntervalID == 0))
        {
            log_error("%s:%d  stPlanSchedule[%d]   nTimeIntervalID  %d is not exist  \n",__func__,__LINE__,i+1,sTemp2);
            return ERROR_NOT_EXIST_TIMEINTERVAL_SCHEDULE;
        }

    }

    /*12. 调度表中不同调度之间是否存在重叠现象;*/
    for(i = 0 ; i < NUM_SCHEDULE; i++)
    {
        for(j = i+1 ; j < NUM_SCHEDULE; j++)
        {
            //DEG("~~~~~~~   %p  %#x\n",&pSignalControlpara->stPlanSchedule[i],&pSignalControlpara->stPlanSchedule[j]);
            if(DateTimeCmp(&(pSignalControlpara->stPlanSchedule[i]), &(pSignalControlpara->stPlanSchedule[j]))== 1)
            {
                log_error("%s:%d   stPlanSchedule_%d  and   stPlanSchedule_%d overlap  \n",__func__,__LINE__,i+1,j+1);
                return ERROR_REPEATE_SCHEDULE;
            }
        }
    }

    /*13. 跟随相位中的母相位是否存在;*/
    for(i = 0 ; i < NUM_FOLLOW_PHASE; i++)
    {
        sTemp = pSignalControlpara->stFollowPhase[i].nFollowPhaseID;
        if(sTemp == 0)
        {
            continue;
        }
    
        for(j = 0 ; j < NUM_PHASE ; j++)
        {
            sTemp = pSignalControlpara->stFollowPhase[i].nArrayMotherPhase[j];

            if(sTemp == 0)
            {
                break;
            }
            if(IS_PHASE_INABLE(pSignalControlpara->stPhase[sTemp - 1].wPhaseOptions)== 0)
            {
                log_error("%s:%d   stFollowPhase[%d]  nPhaseID  %d  is not exist  \n ",__func__,__LINE__,i+1,sTemp);
                return ERROR_NOT_EXIST_MOTHER_PHASE_FOLLOW_PHASE;
            }

        }
    }

    //========================>>>>>>>>>>>>>>>  新增1，相位表中的相位必须存在于并发相位里
    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        sTemp = pSignalControlpara->stPhase[i].nPhaseID;
        if(IS_PHASE_INABLE(pSignalControlpara->stPhase[i].wPhaseOptions)== 1)
        {
            sTemp2 = pSignalControlpara->stPhase[i].nCircleID;
            //新增2,相位表中的相位必须存在于相序表中
            for(j = 0 ; j < NUM_PHASE_TURN; j++)
            {   
                if(pSignalControlpara->stPhaseTurn[j][sTemp2 - 1].nTurnArray[0] == 0)
                {
                    continue;
                }
            
                if(IsItemInCharArray(pSignalControlpara->stPhaseTurn[j][sTemp2 - 1].nTurnArray,NUM_PHASE,sTemp) == 0)
                {
                    log_error("%s:%d phase %d is not in stPhaseTurnArray %d  circleId  %d\n",__func__,__LINE__,sTemp,j,sTemp2);
                    return ERROR_NOT_EXIST_PHASE_TURN_PHASE_2;
                }
            }
            //新增3, 相位不在绿信比表中
            for(j = 0 ; j < NUM_GREEN_SIGNAL_RATION; j++)
            {
                if((pSignalControlpara->stGreenSignalRation[i][sTemp - 1].nGreenSignalRationID != 0)&&
                    (pSignalControlpara->stGreenSignalRation[i][sTemp - 1].nPhaseID == 0))
                {
                    log_error("%s:%d phase %d is not in stGreenSignalRation \n",__func__,__LINE__,sTemp);
                    return ERROR_NOT_EXIST_PHASE_GREEN_SIGNAL_RATION;                

                }

            }
        }
    }

    return 0;
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
Boolean LoadDataFromCfg(SignalControllerPara *pSignalControlpara, const char *path)
{
    char cSection[256];
    int i = 0;
    SignalControllerPara zero;
	
	if ((NULL == pSignalControlpara) || parse_start((path == NULL) ? CFG_NAME : path) == False)
    {
        return FALSE;
    }

	memset(pSignalControlpara, 0, sizeof(SignalControllerPara));

    //加载单元参数 
    memset(cSection,0,sizeof(cSection));
    ReadUnitPara(&(pSignalControlpara->stUnitPara));

    //加载相位参数 
    ReadPhaseItem(pSignalControlpara->stPhase,pSignalControlpara->AscSignalTransTable,NUM_PHASE);

    //加载通道参数 
    ReadChannelItem(pSignalControlpara->stChannel, NUM_CHANNEL);

    //加载绿信比参数 
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION ; i++)
    {
        ReadGreenSignalRationItem(pSignalControlpara->stGreenSignalRation[i],i,NUM_PHASE);

    }

    //To Do  相序表
    for(i = 0 ; i < NUM_PHASE_TURN ; i++)
    {
        ReadPhaseTurnItem(pSignalControlpara->stPhaseTurn[i],i,4);
    }

    //加载方案表参数 
    ReadSchemeItem(pSignalControlpara->stScheme,NUM_SCHEME);

    //加载动作表参数
    ReadActionItem(pSignalControlpara->stAction,NUM_ACTION);

    //加载时段表参数
    for(i = 0; i < NUM_TIME_INTERVAL; i++)
    {
        ReadTimeIntervalItem(pSignalControlpara->stTimeInterval[i], i,NUM_TIME_INTERVAL_ID);
    }
    
    //加载调度参数
    ReadPlanSchedule(pSignalControlpara->stPlanSchedule,NUM_SCHEDULE);

    //加载跟随参数
    ReadFollowPhaseItem(pSignalControlpara->stFollowPhase,NUM_FOLLOW_PHASE);

    //
    ReadVehicleDetector(pSignalControlpara->AscVehicleDetectorTable,MAX_VEHICLEDETECTOR_COUNT);

    //
    ReadPedestrianDetector(pSignalControlpara->AscPedestrianDetectorTable,MAX_PEDESTRIANDETECTOR_COUNT);

    //解析结束
    parse_end();

	memset(&zero, 0, sizeof(SignalControllerPara));
	if (memcmp(pSignalControlpara, &zero, sizeof(SignalControllerPara)) == 0) {	//说明没有配置信息
		log_error("config file is not exist or there is no  config information in file . \n");
		return FALSE;
	}	
	
	return TRUE;
}







