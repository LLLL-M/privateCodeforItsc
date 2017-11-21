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
//#include "parse_ini.h"
#include "HikConfig.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "its.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define COL_DEF "\033[m"
#define COL_RED "\033[0;32;31m"
#define COL_GRE "\033[0;32;32m"
#define COL_BLU "\033[0;32;34m"
#define COL_YEL "\033[1;33m"

//是否开启中文错误打印
#define LOG_ERR_CHN


#define DEG(fmt,...) fprintf(stdout,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)

typedef enum 
{
	False = 0,
	True
} Bool;


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

static unsigned char cArrayGreenSignalRation[NUM_GREEN_SIGNAL_RATION] = {0};//存储的是实际被方案表使用的绿信比表ID

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
#ifndef LOG_ERR_CHN    
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
#endif
}

/*****************************************************************************
 函 数 名  : log_error_cn
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
static void log_error_cn(const char* format, ...)
{
#ifdef LOG_ERR_CHN
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
#endif
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
			//log_error_cn("turn %d, i %d\n",nPhaseTurnId,i);
            continue;

        }    
        
        for(j = 0 ; j < NUM_PHASE; j++)
        {
			
            if(pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] == 0)//轮询一个环就直接结束
            {
				//log_error_cn("===> turn %d,i %d, j %d\n",nPhaseTurnId,i,j);
                return temp;
            }
			//log_error_cn("nGreenSignalRationId %d,nPhaseTurnId %d, i %d, j %d, %d\n",nGreenSignalRationId,nPhaseTurnId,i,j,pSignalControlpara->stGreenSignalRation[nGreenSignalRationId - 1][pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] - 1].nGreenSignalRationTime);
            
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
static int IsBarrierGreenSignalRationEqual(SignalControllerPara *pSignalControlpara,unsigned char *cArray,int len,unsigned char nCircleId,int nPhaseTableNo)
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

        temp = pSignalControlpara->stPhase[nPhaseTableNo - 1][cArray[i] - 1].nCircleID;//找到环号

        cTempArray[temp - 1][cArray[i] - 1] = cArray[i];

       // DEG("===>  cTeampArray   %d\n",cTempArray[temp - 1][cArray[i] - 1]);
    }
#if 1
    //一个环内的相位的并发相位绿信比之和，应该和该环的相位的绿信比之和相等
    for(m = 0 ; m < NUM_GREEN_SIGNAL_RATION; m++)//轮询绿信比表，找到某个相位的绿信比时间
    {
        temp = 0;
        temp2 = 0;
        temp3 = 0;
        
		if(cArrayGreenSignalRation[m] == 0)//如果绿信比没有被调用，则不做计算
		{
			continue;
		}
		
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
    					temp3 = pSignalControlpara->stPhase[nPhaseTableNo - 1][cTempArray[i][j] - 1].byPhaseConcurrency[k];//找到该相位对应的并发相位
    					if(temp3 == 0)
    					{
    						break;
    					}

    					if(pSignalControlpara->stPhase[nPhaseTableNo - 1][temp3 - 1].nCircleID == nCircleId)//如果该并发相位的环号和形参相同，则求其绿信比之和
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
				//log_error_cn("GreenSignalRationId  %d\n",m+1);
                return  1;
            }
        }
    }
#endif    
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
        INFO("### i = %d , phase = %d",i,array[i]);
		if(array[i] == val)
		{
			return TRUE;
		}

	}

	return FALSE;
}

static int is_phase_valid(SignalControllerPara *pSignalControlpara,int phase_turn_id,int phase_id)
{
    int i,j;
    
    for(i = 0 ; i < NUM_RING_COUNT; i++)
    {
        for(j = 0; j < 32; j++)
        {
//            INFO("@@@ j = %d , turnarray = %d",j,pSignalControlpara->stPhaseTurn[phase_turn_id - 1][i].nTurnArray[j]);
            if(phase_id == pSignalControlpara->stPhaseTurn[phase_turn_id - 1][i].nTurnArray[j])
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
static int IsPhaseContinuousInPhaseTurn(SignalControllerPara *pSignalControlpara,unsigned char nCircleId,unsigned char *cArray,int len,int nPhaseTableNo)
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
        if((cArray[i] != 0) && (pSignalControlpara->stPhase[nPhaseTableNo - 1][cArray[i] - 1].nCircleID == nCircleId))
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

    unsigned char cTemp = 0;
        
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
static int IsSignalControllerParaIdLegal(SignalControllerPara *pSignalControlpara)
{
    int i = 0;
    int j = 0;
    
    unsigned short sTemp = 0;
    unsigned short sTemp2 = 0;

    //相位数组下标是否和相位号相同
    for(j = 0; j < MAX_PHASE_TABLE_COUNT ; j++)
    {
        for(i = 0 ; i < NUM_PHASE; i++)
        {
            if((IS_PHASE_INABLE(pSignalControlpara->stPhase[j][i].wPhaseOptions) == 1) &&(pSignalControlpara->stPhase[j][i].nPhaseID != (i+1)))
            {
                log_error("%s:%d stPhase[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_PHASE , Error Content: 相位%d下标不符合规范. \n",i+1);
                return ERROR_SUBSCRIPT_PHASE;
            }
        }


    }
    for(j = 0; j < MAX_PHASE_TABLE_COUNT ; j++)
    {
        for(i = 0 ; i < NUM_PHASE; i++)
        {
            //并发相位重复
            if(IsArrayRepeat(pSignalControlpara->stPhase[j][i].byPhaseConcurrency,NUM_PHASE) == 1)
            {
                log_error("%s:%d stConcurrentPhase[%d]  has repeat items \n",__func__,__LINE__,i+1);
                log_error_cn("ErrorCode: ERROR_REPEATE_CONCURRENT_PHASE , Error Content: 相位%d的并发相位有重复值. \n",pSignalControlpara->stPhase[j][i].nPhaseID);
                return ERROR_REPEATE_CONCURRENT_PHASE;
            }

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
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_PHASE_TURN , Error Content: 相序表%d下标不符合规范. \n",i+1);
                return ERROR_SUBSCRIPT_PHASE_TURN;
            }

            if(sTemp2 != (j+1))
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d] subscript is not equal to nCircleID\n ",__func__,__LINE__,i+1,j+1);
                log_error_cn("ErrorCode: ERROR_SUSCRIPT_PHASE_TURN_CIRCLE , Error Content: 相序表%d环%d下标不符合规范. \n",i+1,j+1);
                return ERROR_SUSCRIPT_PHASE_TURN_CIRCLE;
            }
            
            if(IsArrayRepeat(pSignalControlpara->stPhaseTurn[i][j].nTurnArray,16) == 1)
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d]  has repeat item \n ",__func__,__LINE__,i+1,j+1);
                log_error_cn("ErrorCode: ERROR_REPEATE_PHASE_TURN , Error Content: 相序表%d环%d有重复. \n",sTemp,sTemp2);
                return ERROR_REPEATE_PHASE_TURN;
            }
        }
    }

//考虑到校验要放到前端来处理，故此处重新开始校验    
#if 1
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
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION , Error Content: 绿信比表%d下标不符合规范. \n",i+1);
                return ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION;
            }
            
            if((sTemp2 != 0) && (sTemp2 != (j+1)))
            {
                log_error("%s:%d stGreenSignalRation[%d] : nPhaseId[%d]    nPhaseID  subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION_PHASE , Error Content: 绿信比表的相位%d下标不符合规范. \n",j+1);
                return ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION_PHASE;
            }        

         }
    }
#endif
    //Channel 
    for(j = 0; j < MAX_CHANNEL_TABLE_COUNT ; j++)
    {
        for(i = 0 ; i < NUM_CHANNEL; i++)
        {
            sTemp = pSignalControlpara->stChannel[j][i].nChannelID;

            if((pSignalControlpara->stChannel[j][i].nControllerID != 0) && (sTemp != (i+1)))
            {
                log_error("%s:%d stChannel[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_CHANNEL , Error Content: 通道表%d通道%d下标不符合规范. \n",j+1,i+1);
                return ERROR_SUBSCRIPT_CHANNEL;
            }
        }


    }
    //stScheme
    for(i = 0 ; i < NUM_SCHEME; i++)
    {
        sTemp = pSignalControlpara->stScheme[i].nSchemeID;

        if((pSignalControlpara->stScheme[i].nGreenSignalRatioID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stScheme[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);
            log_error_cn("ErrorCode: ERROR_SUBSCRIPT_SCHEME , Error Content: 方案表%d下标不符合规范. \n",i+1);
            return ERROR_SUBSCRIPT_SCHEME;
        }
    }

    //stAction
    for(i = 0 ; i < NUM_ACTION; i++)
    {
        sTemp = pSignalControlpara->stAction[i].nActionID;

        if((pSignalControlpara->stAction[i].nSchemeID != 0) && (sTemp != (i+1)) && (pSignalControlpara->stAction[i].nSchemeID < 249))
        {
            log_error("%s:%d stScheme[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);
            log_error_cn("ErrorCode: ERROR_SUBSCRIPT_ACTION , Error Content: 动作表%d下标不符合规范. \n",i+1);
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
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_TIMEINTERVAL , Error Content: 时段表%d下标不符合规范. \n",i+1);
                return ERROR_SUBSCRIPT_TIMEINTERVAL;
            }
            if((pSignalControlpara->stTimeInterval[i][j].nActionID != 0) && (sTemp2 != (j+1)))
            {
                log_error("%s:%d stTimeInterval[%d]:nTimeID[%d] nTimeID subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_TIMEINTERVAL_TIME , Error Content: 时段表%d时段号%d下标不符合规范. \n",i+1,j+1);
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
            log_error_cn("ErrorCode: ERROR_SUBSCRIPT_SCHEDULE , Error Content: 调度表%d下标不符合规范. \n",i+1);
            return ERROR_SUBSCRIPT_SCHEDULE;
        }
    }
    
    //FollowPhase
    for(j = 0; j < MAX_FOLLOW_PHASE_TABLE_COUNT ; j++)
    {
        for(i = 0 ; i < NUM_FOLLOW_PHASE ; i++)
        {
            if((pSignalControlpara->stFollowPhase[j][i].nArrayMotherPhase[0] != 0 ) && (pSignalControlpara->stFollowPhase[j][i].nFollowPhaseID != (i+1)))
            {
                log_error("%s:%d stFollowPhase[%d]  subscript is not equal to ID \n ",__func__,__LINE__,i+1);
                log_error_cn("ErrorCode: ERROR_SUBSCRIPT_FOLLOW_PHASE , Error Content: 跟随相位表%d下标不符合规范. \n",i+1);
                return ERROR_SUBSCRIPT_FOLLOW_PHASE;
            }

            if(IsArrayRepeat(pSignalControlpara->stFollowPhase[j][i].nArrayMotherPhase,NUM_PHASE) == 1)
            {
                log_error("%s:%d stFollowPhase[%d]  has repeat items \n ",__func__,__LINE__,i+1);
                log_error_cn("ErrorCode: ERROR_REPEATE_FOLLOW_PHASE , Error Content: 跟随相位表%d有重复相位. \n",i+1);
                return ERROR_REPEATE_FOLLOW_PHASE;
            }

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
static int find_next_index(SignalControllerPara *pSignalControlpara,unsigned char *phaseArray, unsigned short index,int nPhaseTableNo)
{
	int i,  ret = 0;
	unsigned short phaseId = phaseArray[index];
	PPhaseItem item = &pSignalControlpara->stPhase[nPhaseTableNo - 1][phaseId - 1];

    if(phaseId == 0)//使用控件下载的话，会出现相位为0的情况
    {
        return 0;
    }
	
	if (item->nPhaseID != phaseId) 
	{
		return index + 1;
	}
	phaseId = item->byPhaseConcurrency[0];
	item = &pSignalControlpara->stPhase[nPhaseTableNo - 1][phaseId - 1];
	
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
__attribute__((unused)) static int checkPhaseTrunInRing(SignalControllerPara *pSignalControlpara,unsigned short phaseTurn,int nPhaseTableNo)
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
			indexs[i] = find_next_index(pSignalControlpara,item->nTurnArray, indexs[i],nPhaseTableNo);
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
				if (IsItemInCharArray(pSignalControlpara->stPhase[nPhaseTableNo - 1][fatherPhase - 1].byPhaseConcurrency, NUM_PHASE, ringPhase[i]) == FALSE)
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
//判断并发相位的合法性
static int IsConcurrencyPhaseLegal(SignalControllerPara *pSignalControlpara,int nPhaseId,int nPhaseTableNo)
{
	int i = 0;
	int nTemp = 0;	
	for(i = 0 ; i < NUM_PHASE; i++)
    {
		nTemp = pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].byPhaseConcurrency[i];
		
		if(nTemp < 0 || nTemp > NUM_PHASE)
		{
			log_error_cn("相位%d的并发相位数组有不合法相位ID\n",nPhaseId);
			return ERROR_ID_LEGAL_PHASE;
		}
		
		if(nTemp == 0)
		{
			continue;
		}

		//并发相位不存在
		if(IS_PHASE_INABLE(pSignalControlpara->stPhase[nPhaseTableNo - 1][nTemp - 1].wPhaseOptions) == 0)
		{
			log_error_cn("ErrorCode: ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE , Error Content: 相位%d的并发相位%d不存在. \n",nPhaseId,nTemp);
			return ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE;
		}
#if 0
		//phase is in the same circle
		if(pSignalControlpara->stPhase[nPhaseTableNo - 1][nTemp - 1].nCircleID == pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].nCircleID)
		{
			log_error_cn("ErrorCode: ERROR_SAME_CIRCLE_CONCURRENT_PHASE , Error Content: 相位%d与其并发相位%d环号相同. \n",nTemp,nPhaseId);
			return ERROR_SAME_CIRCLE_CONCURRENT_PHASE;
		}

		//子相位的并发相位中没有父相位
		if(IsItemInCharArray(pSignalControlpara->stPhase[nPhaseTableNo - 1][nTemp - 1].byPhaseConcurrency,NUM_PHASE, nPhaseId) == 0)
		{
			log_error_cn("ErrorCode: ERROR_FATHER_NOT_EXIST_CONCURRENT_PHASE , Error Content: 相位%d不在其并发相位%d的并发相位中. \n",nPhaseId,nTemp);
			return ERROR_FATHER_NOT_EXIST_CONCURRENT_PHASE;
		}

		//子相位在相序表中是否连续      
		if(IsPhaseContinuousInPhaseTurn(pSignalControlpara, pSignalControlpara->stPhase[nPhaseTableNo - 1][nTemp - 1].nCircleID, pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].byPhaseConcurrency, NUM_PHASE,nPhaseTableNo) == 0)
		{
			log_error_cn("ErrorCode: ERROR_CHILD_NOT_CONTINUOUS_CONCURRENT_PHASE , Error Content: 相位%d的并发相位在环%d中不连续. \n",nPhaseId,pSignalControlpara->stPhase[nPhaseTableNo - 1][nTemp - 1].nCircleID);
			return ERROR_CHILD_NOT_CONTINUOUS_CONCURRENT_PHASE;
		}
#endif

		//相序表中，屏障一遍的绿信比之和必须相等   放到运行时添加测试
		/*if(IsBarrierGreenSignalRationEqual(pSignalControlpara,pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].byPhaseConcurrency, NUM_PHASE,pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].nCircleID,nPhaseTableNo) == 1)
		{
			log_error_cn("ErrorCode: ERROR_BARRIER_CONCURRENT_PHASE , Error Content: 相位%d屏障一侧的相位绿信比之和不相等. \n",nPhaseId);
			return ERROR_BARRIER_CONCURRENT_PHASE;
		 }*/
    }

	return 0;	
}

//判断跟随相位表是否合法
static int IsFollowPhaseLegal(SignalControllerPara *pSignalControlpara,int nFollowPhaseTableNo)
{
    int i = 0;
    int sTemp = 0;
    int j = 0;

    if(nFollowPhaseTableNo <= 0 || nFollowPhaseTableNo >= MAX_PHASE_TABLE_COUNT)
    {
        log_error_cn("跟随相位不合法，范围必须是[0,%d]\n",MAX_PHASE_TABLE_COUNT);
        return ERROR_ID_LEGAL_FOLLOW_PHASE;
    }
    
    /*13. 跟随相位中的母相位是否存在;*/
    for(i = 0 ; i < NUM_FOLLOW_PHASE; i++)
    {
        sTemp = pSignalControlpara->stFollowPhase[nFollowPhaseTableNo - 1][i].nFollowPhaseID;
        if(sTemp == 0)
        {
            continue;
        }
    
        for(j = 0 ; j < NUM_PHASE ; j++)
        {
            sTemp = pSignalControlpara->stFollowPhase[nFollowPhaseTableNo - 1][i].nArrayMotherPhase[j];

            if(sTemp == 0)
            {
                break;
            }
            if(IS_PHASE_INABLE(pSignalControlpara->stPhase[nFollowPhaseTableNo - 1][sTemp - 1].wPhaseOptions)== 0)
            {
                log_error_cn("ErrorCode: ERROR_NOT_EXIST_MOTHER_PHASE_FOLLOW_PHASE , Error Content: 跟随相位%d的母相位%d不存在 \n",i+1,sTemp);
                return ERROR_NOT_EXIST_MOTHER_PHASE_FOLLOW_PHASE;
            }

        }
    }

    return 0;
}

//判断相位是否合法
static int IsPhaseLegal(SignalControllerPara *pSignalControlpara,int nPhaseId,int nPhaseTableNo)
{
	if(nPhaseId <= 0 || nPhaseId > NUM_PHASE)
	{
		log_error_cn("相位%d不合法，范围必须为[1,%d]\n",nPhaseId,NUM_PHASE);
		return ERROR_ID_LEGAL_PHASE;
	}

	if(IS_PHASE_INABLE(pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].wPhaseOptions) == 0)
	{
		log_error_cn("相位表%d的相位%d不合法，相位未使能.\n",nPhaseTableNo,nPhaseId);
		return ERROR_PHASE_DISABLE;
	}

    return 0;
}

//获得所指定的相位表的相位数组
static void GetPhaseArray(SignalControllerPara *pSignalControlpara,unsigned short *nPhaseArray,int nPhaseTableNo)
{
    int i = 0;    

    if(nPhaseTableNo <= 0 || nPhaseTableNo >= MAX_PHASE_TABLE_COUNT)
    {
        return;
    }

    for(i = 0; i < NUM_PHASE; i++)
    {
        nPhaseArray[i] = pSignalControlpara->stPhase[nPhaseTableNo - 1][i].nPhaseID;
//        INFO("@@@ i = %d , phasearray = %d",i,nPhaseArray[i]);
    }
}
//获得所指定的跟随相位表的相位数组
static void GetFollowPhaseArray(SignalControllerPara *pSignalControlpara,unsigned short *nPhaseArray,int nPhaseTableNo)
{
    int i = 0;    
    if(nPhaseTableNo <= 0 || nPhaseTableNo >= MAX_FOLLOW_PHASE_TABLE_COUNT)
    {
        return;
    }
    
    for(i = 0; i < NUM_FOLLOW_PHASE; i++)
    {
        nPhaseArray[i] = pSignalControlpara->stFollowPhase[nPhaseTableNo - 1][i].nFollowPhaseID;
    }
}


//判断通道表是否合法
static int IsChannelLegal(SignalControllerPara *pSignalControlpara,int phase_turn_id,/* unsigned short *nFollowPhaseArray, */int nChannelNo)
{
    int i = 0;
    int sTemp = 0;

//    INFO("### phase_turn_id = %d , nChannelNo = %d",phase_turn_id,nChannelNo);

    if(nChannelNo <= 0 || nChannelNo >= MAX_CHANNEL_TABLE_COUNT)
    {
        log_error_cn("nChannelNo 不合法，越界. \n",nChannelNo);
        return ERROR_ID_LEGAL_CHANNEL;
    }
    
    for(i = 0 ; i < NUM_CHANNEL ; i++)
    {
        sTemp = pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerID ;
//        INFO("### chan = %d , controllerid = %d",i + 1,sTemp);
        if(sTemp == 0)
        {
            continue;
        }

//        INFO("### controllertype = %d",pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType);
        if((pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType == MOTOR ) || (pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType == PEDESTRIAN))
        {
//            INFO("### phaseArray:");
//            if(IsItemInShortArray(nPhaseArray,NUM_PHASE,sTemp) == 0)
            if(is_phase_valid(pSignalControlpara,phase_turn_id,sTemp) == 0)
            {
                log_error_cn("ErrorCode: ERROR_NOT_EXIST_SOURCE_CHANNEL , Error Content: 通道%d的控制源%d不存在. \n",i+1,sTemp);
                return ERROR_NOT_EXIST_SOURCE_CHANNEL;
            }
        }
#if 0        
        else if (pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType == FOLLOW)
        {
            INFO("### followPhaseArray:");
            if(IsItemInShortArray(nFollowPhaseArray,NUM_FOLLOW_PHASE,sTemp) == 0)
            {
                log_error_cn("ErrorCode: ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL , Error Content: 通道%d的跟随控制源%d不存在. \n",i+1,sTemp);
                return ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL;
            }
        }
#endif        
    }

    return 0;
}


//判断绿信比表是否合法
static int IsGreenSignalRationLegal(SignalControllerPara *pSignalControlpara,int nGreenSignalRationId,int nPhaseTableNo)
{
	int i = 0;
	int nPhaseId = 0;
	int nGreenLightTime = 0;
	int sTemp = 0;
    int ret = 0;
    int count = 0;
    
	if(nGreenSignalRationId <= 0 || nGreenSignalRationId > NUM_GREEN_SIGNAL_RATION )
	{
		log_error_cn("绿信比表%d ID不合法，范围必须为[1,%d]\n",nGreenSignalRationId,NUM_GREEN_SIGNAL_RATION);
		return ERROR_ID_LEGAL_SPLIT;	
	}
	
	for(i = 0; i < NUM_PHASE; i++)
	{
        nPhaseId = pSignalControlpara->stGreenSignalRation[nGreenSignalRationId - 1][i].nPhaseID;
        nGreenLightTime = pSignalControlpara->stGreenSignalRation[nGreenSignalRationId - 1][i].nGreenSignalRationTime;
        
        if((nPhaseId == 0) || (nGreenLightTime == 0))//每个绿信比表的绿信比号及相位号都不是0，所以没法判断到底哪个绿信比是没有使用的，故直接不校验绿信比时间为0的情况。
        {
            count++;
            continue;
        }
        
        if((ret = IsPhaseLegal(pSignalControlpara,nPhaseId,nPhaseTableNo)) != 0)
        {
            ERR("IsGreenSignalRationLegal failed , nPhaseTableNo  %d ,nPhaseId  %d",nPhaseTableNo,nPhaseId);
            return ret;
        }
		
		sTemp = pSignalControlpara->AscSignalTransTable[nPhaseTableNo - 1][nPhaseId - 1].nGreenLightTime + pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].nYellowTime +  pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].nAllRedTime;
	
		if(nGreenLightTime < sTemp)
		{
			log_error_cn("绿信比表%d中相位%d的绿信比时间不合法，绿信比时间必须比黄闪全红黄灯时间之和要大.\n",
													nGreenSignalRationId,
													nPhaseId);
			return ERROR_SPLIT_LOW_PEDESTRIAN_GREEN_SIGNAL_RATION;	
		}
	}

    if(count == NUM_PHASE)
    {
        log_error_cn("绿信比表%d不存在.\n",nGreenSignalRationId);
        return ERROR_NOT_EXIST_GREEN_SIGNAL_RATION_SCHEME;
    }
	
    return 0;
}


//判断相序表是否合法
static int IsPhaseTurnLegal(SignalControllerPara *pSignalControlpara,int nPhaseTurnId,int nPhaseTableNo)
{
	int i = 0;
	int ring = 0;
	int nPhaseId = 0;
	int nCircleId = 0;
	int ret = 0;

	if(nPhaseTurnId <= 0 || nPhaseTurnId > NUM_PHASE_TURN)
	{
		log_error_cn("相序表%d的相序号不合法，范围必须为[0,%d]\n",nPhaseTurnId,NUM_PHASE_TURN);
		return ERROR_ID_LEGAL_PHASE_TURN;	
	}
	for(ring = 0; ring < 4; ring++)
	{
		nCircleId = pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][ring].nCircleID;
		if(nCircleId < 0 || nCircleId > 4)
		{
			log_error_cn("相序表%d的环%d的环号不合法，范围必须为[0,4]\n",nPhaseTurnId,nCircleId);
			return ERROR_ID_LEGAL_PHASE_TURN_ID;	
		}

		if(nCircleId == 0)
			continue;
		
		for(i = 0; i < NUM_PHASE; i++)
		{
			nPhaseId = pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][ring].nTurnArray[i];
			if(nPhaseId == 0)
				continue;

             if((ret = IsPhaseLegal(pSignalControlpara,nPhaseId,nPhaseTableNo)) != 0)
             {
                ERR("IsPhaseTurnLegal  failed\n");
                return ret;
             }

			if(pSignalControlpara->stPhase[nPhaseTableNo - 1][nPhaseId - 1].nCircleID != nCircleId)
			{
				log_error_cn("相序表%d环号%d的相位%d的环号与相位表中不一致\n",nPhaseTurnId,nPhaseId);	
				return ERROR_NOT_EQUAL_PHASE_TURN_CIRCLE;
			}

			if((ret = IsConcurrencyPhaseLegal(pSignalControlpara,nPhaseId,nPhaseTableNo)) != 0)
			{
                return ret;
			}
		} 
	}

    /*if(checkPhaseTrunInRing(pSignalControlpara,nPhaseTurnId) == FALSE)
    {
        log_error_cn("ErrorCode: ERROR_NOT_CORRECT_PHASE_TURN , Error Content: 相序表%d并发相位有误. \n",nPhaseTurnId);
        return ERROR_NOT_CORRECT_PHASE_TURN;
    }*/

    return 0;
}


//判断方案表是否合法
static int IsSchemeLegal(SignalControllerPara *pSignalControlpara,int nSchemeId,int nPhaseTableNo) 
{
	int nGreenSignalRatioId = 0;
	int nPhaseTurnId = 0;
	int ret = 0;

    if((nSchemeId >= 240) && (nSchemeId <= 255))//特殊方案号
    {
        return 0;
    }
	
	if(nSchemeId <= 0 || nSchemeId > NUM_SCHEME)
	{
		log_error_cn("方案表%d ID不合法，范围必须为[0,%d]\n",nSchemeId,NUM_SCHEME);
		return ERROR_ID_LEGAL_SCHEME;	
	}

	if(nPhaseTableNo <= 0 || nPhaseTableNo >= MAX_PHASE_TABLE_COUNT)
	{
		log_error_cn("相位表%d ID不合法，范围必须为[0,%d]\n",nPhaseTableNo,MAX_PHASE_TABLE_COUNT);
        return ERROR_ID_LEGAL_PHASE;
	}

	nGreenSignalRatioId = pSignalControlpara->stScheme[nSchemeId - 1].nGreenSignalRatioID;
	nPhaseTurnId = pSignalControlpara->stScheme[nSchemeId - 1].nPhaseTurnID;

    if((nGreenSignalRatioId == 0) || (nPhaseTurnId == 0))
    {
        return 0;
    }

	//判断绿信比表是否合法
    if((ret = IsGreenSignalRationLegal(pSignalControlpara,nGreenSignalRatioId,nPhaseTableNo)) != 0)
	{
        return ret;
	}
	
	cArrayGreenSignalRation[nGreenSignalRatioId - 1] = nGreenSignalRatioId; 
	
	//判断相序表是否合法
    if((ret = IsPhaseTurnLegal(pSignalControlpara,nPhaseTurnId,nPhaseTableNo)) != 0)
	{
        return ret;
	}
	
    //方案表中的周期长是否等于相序表中单环绿信比时间之和
    if(GetCircleTime(pSignalControlpara,nGreenSignalRatioId,nPhaseTurnId) != pSignalControlpara->stScheme[nSchemeId - 1].nCycleTime)
    {
        log_error_cn("ErrorCode: ERROR_CIRCLE_TIME_SCHEME , Error Content: 方案表%d的周期长不合法.\n",nSchemeId);
		//log_error_cn("nPhaseTurnId %d  , nGreenSignalRatioId %d, %d  --- %d \n",nPhaseTurnId,nGreenSignalRatioId
		//					,GetCircleTime(pSignalControlpara,nPhaseTurnId,nGreenSignalRatioId),
		//				pSignalControlpara->stScheme[nSchemeId - 1].nCycleTime);
        return ERROR_CIRCLE_TIME_SCHEME;
    }

    return 0;
}

//判断动作表是否合法
static int IsActionLegal(SignalControllerPara *pSignalControlpara,int nActionId)
{
	int  nSchemeId = 0;
	int  nPhaseTableNo = 0;
    int  nChannelTableNo = 0;
    int  phase_turn_id = 0;
    int  nRet = 0;
//    unsigned short nPhaseArray[NUM_PHASE] = {0};//总共的相位数组
//    unsigned short nFollowPhaseArray[NUM_FOLLOW_PHASE] = {0};//跟随相位数组


	if(nActionId <=0 || nActionId > NUM_ACTION)
	{
		log_error_cn("动作表%d ID不合法，范围必须为[0,%d]\n",nActionId,NUM_ACTION);
		return ERROR_ID_LEGAL_ACTION;	
	}

	nSchemeId =  pSignalControlpara->stAction[nActionId - 1].nSchemeID;
	nPhaseTableNo = pSignalControlpara->stAction[nActionId - 1].nPhaseTableID;
	nChannelTableNo = pSignalControlpara->stAction[nActionId - 1].nChannelTableID;

    if((nRet = IsSchemeLegal(pSignalControlpara,nSchemeId,nPhaseTableNo)) != 0)//判断方案表是否合法
    {
        log_error_cn("IsActionLegal  nActionId :  %d\n",nActionId);
        return nRet;
    }
    //判断相位表是否合法,这个在方案表中判断
    
    //判断跟随相位表
    if((nRet = IsFollowPhaseLegal(pSignalControlpara,nPhaseTableNo)) != 0)
    {
        return nRet;
    }


    //判断通道表
//    GetPhaseArray(pSignalControlpara,nPhaseArray,nPhaseTableNo);
//    INFO("###actionid = %d , schemeid = %d",nActionId,nSchemeId);
    if(nSchemeId == YELLOWBLINK_SCHEMEID || nSchemeId == ALLRED_SCHEMEID || nSchemeId == TURNOFF_SCHEMEID)
    {
        //黄闪控制，全红控制，关灯控制，和相序表无关，和通道也没有关系，无需判断；
//        INFO("^^^^^^ schemeid = %d",nSchemeId);
        return 0;
    }
	
    phase_turn_id = pSignalControlpara->stScheme[nSchemeId - 1].nPhaseTurnID;
	if (nSchemeId > 108)
	{
		phase_turn_id = 1;
		nChannelTableNo = 1;
	}
//    INFO("### phase_turn_id = %d",phase_turn_id);
//    GetFollowPhaseArray(pSignalControlpara,nFollowPhaseArray,nPhaseTableNo);
    if((nRet = IsChannelLegal(pSignalControlpara,phase_turn_id,/* nFollowPhaseArray ,*/nChannelTableNo)) != 0)
    {
        return nRet;
    }
    
    return  nRet;
}


//判断时段表是否合法
static int IsTimeIntervalLegal(SignalControllerPara *pSignalControlpara,int nIntervalId)
{
	//默认不校验指针合法性了。
	int i = 0;
	int nTimeId = 0;
	int nActionId = 0;
    int ret = 0;
    int count = 0;

	if(nIntervalId <=0 || nIntervalId > NUM_TIME_INTERVAL)
	{
		log_error_cn("时段表%d的ID不合法，范围必须为[0,%d]\n",nIntervalId,NUM_TIME_INTERVAL);
		return ERROR_ID_LEGAL_INTERVAL;
	}

	for(i = 0; i < NUM_TIME_INTERVAL_ID; i++)
	{
		nTimeId = pSignalControlpara->stTimeInterval[nIntervalId - 1][i].nTimeID;

        if(nTimeId == 0)
		{
		    count++;
            continue;
		}
		
		if(nTimeId < 0 || nTimeId > NUM_TIME_INTERVAL_ID)//如果时段表中的时段ID不在合法区间，则亦认为ERROR。
		{
			log_error_cn("时段表%d的时段%d不合法，范围必须为[0,%d]\n",nIntervalId,nTimeId,NUM_TIME_INTERVAL_ID);
			return ERROR_ID_LEGAL_INTERVAL_TIME;	
		}

		//时段表无误后，再判断其对应的动作表是否合法。
		nActionId = pSignalControlpara->stTimeInterval[nIntervalId - 1][i].nActionID;
		if((nActionId != 0) && ((ret = IsActionLegal(pSignalControlpara,nActionId)) != 0))
		{
		    log_error_cn("IsTimeIntervalLegal nIntervalId  %d, nTimeId  %d,  nActionId  %d\n",nIntervalId,nTimeId,nActionId);
            return ret;
		}
	}

    if(count == NUM_TIME_INTERVAL_ID)
    {
        log_error_cn("时段表%d不存在.\n",nIntervalId);
        return ERROR_NOT_EXIST_TIMEINTERVAL_SCHEDULE;
    }

    return 0;
}

//判断调度表是否合法
static int IsPlanScheduleLegal(SignalControllerPara *pSignalControlpara,int nScheduleId)
{
	int nTimeIntervalId = 0;	
	
	if(nScheduleId <= 0 || nScheduleId > NUM_SCHEDULE)
	{
		log_error_cn("调度表%d的ID不合法，范围必须是[0,%d]\n",nScheduleId,NUM_SCHEDULE);	
		return ERROR_ID_LEGAL_SCHEDULE;
	}

	nTimeIntervalId = pSignalControlpara->stPlanSchedule[nScheduleId - 1].nTimeIntervalID;

	//判断时段表是否合法
	return (nTimeIntervalId <= 0) ? 0 : IsTimeIntervalLegal(pSignalControlpara,nTimeIntervalId);	
}


/*****************************************************************************
 函 数 名  : IsSignalControlparaLegal
 功能描述  : 全局信号控制参数合法性检查
 输入参数  : SignalControllerPara *pSignalControlpara  

 修改历史      :
  1.日    期   : 2014年8月6日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int IsSignalControlparaLegal(SignalControllerPara *pSignalControlpara)//合法的话，返回0，否则返回错误码
{
    int i = 0;
    int ret = 0;
	UnitPara zero;

    if(NULL == pSignalControlpara)
    {
        log_error_cn("ErrorCode: ERROR_NULL_POINTER , Error Content: 全局指针为空 \n");
        return ERROR_NULL_POINTER;
    }

	memset(&zero, 0, sizeof(UnitPara));
	if (memcmp(&pSignalControlpara->stUnitPara, &zero, sizeof(UnitPara)) == 0) {	//说明没有配置信息
		log_error_cn("ErrorCode: ERROR_NOT_CONFIG_INFORMATION , Error Content: 配置信息为空 \n");
		return ERROR_NOT_CONFIG_INFORMATION;
	}

    /*1.  各个表格数组下标是否和ID相同;*/
    if((ret = IsSignalControllerParaIdLegal(pSignalControlpara)) != 0)
    {
        return ret;
    }

	//以下是根据“使用则校验，无用则不校验”的原则进行校验
    //根据调度表，一层一层校验
    for(i = 0 ; i < NUM_SCHEDULE; i++)
    {
        if((ret = IsPlanScheduleLegal(pSignalControlpara,i+1)) != 0)
        {
            return ret;
        }
    }    

    return 0;
}









