/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : countDown_LaiSi.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年4月2日
  最近修改   :
  功能描述   : 本源文件实现了莱斯协议倒计时，并提供了读取、修改、保存莱斯配
               置文件的接口，同时提供调用莱斯协议的接口。
  函数列表   :
              ConvertData
              countDownByLaiSiProtocol
              GetNextControllerId
              GetRuningPhaseId
              PrintData
              ReadLaiSiCfgFromIni
              SetCountDownValueLaiSi
              SetCountValue
              UpdateLaiSiCfg
              WriteLaiSiCfgToIni
  修改历史   :
  1.日    期   : 2015年4月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "countDown_LaiSi.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

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
//莱斯协议对应的转码
static unsigned char  gLaiSiConvertArray[16] = {0x02, 0x9e, 0x24, 0x0c,  0x98, 0x48, 0x40, 0x1e, 0x00, 0x08, 0x10, 0x0c0, 0x62, 0x84, 0x60, 0x70};

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static CountDownCfgLaiSi        g_CountDownCfgLaiSi;                       //全局参数，存放的是莱斯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
static CountDownParamsLaiSi     g_CountDownParamsLaiSi[4][MAX_NUM_PHASE];    //分别定义了其他类型、机动车、行人、及跟随倒计时参数，这个需要每秒进行更新,相位ID作为数组下标。

/*****************************************************************************
 函 数 名  : PrintData
 功能描述  : 测试用，打印发送缓冲区内容。
 输入参数  : unsigned char *nArray  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void PrintData(unsigned char *nArray)
{
    int i = 0;

    for(i = 1; i <= 48; i++)
    {
        fprintf(stderr,"0x%x ",nArray[i - 1]);
        if((i%12 == 0) && (i != 0))
        {
            fprintf(stderr,"\n");
        }
    }
    
    fprintf(stderr,"\n\n\n");
}


/*****************************************************************************
 函 数 名  : ConvertData
 功能描述  : 根据倒计时时间，将十进制倒计时时间，按照莱斯协议转换成符合要求
             的十位数及个位数。
 输入参数  : unsigned char iTempPassTime[2]     分别存放转码后的个位和十位
             unsigned char cPhaseCountDownTime  转码前的倒计时时间。
             unsigned char cPhaseColor          当前灯色
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void ConvertData(unsigned char iTempPassTime[2],unsigned char cPhaseCountDownTime,unsigned char cPhaseColor)
{
    iTempPassTime[0] = cPhaseCountDownTime%10;//个位
    iTempPassTime[1] = cPhaseCountDownTime/10;//十位

    //转码
    if((iTempPassTime[0] >= 0) && (iTempPassTime[0] <= 15) && (iTempPassTime[1] >= 0) && (iTempPassTime[1] <= 15)) 
    {
        iTempPassTime[0] = gLaiSiConvertArray[iTempPassTime[0]];
        iTempPassTime[1] = gLaiSiConvertArray[iTempPassTime[1]];
    }

    if((GREEN == cPhaseColor) || (GREEN_BLINK == cPhaseColor))//1 0
    {
        iTempPassTime[0] |= 0x00; 
        iTempPassTime[1] |= 0x01; 
    }
    else if(RED == cPhaseColor)//0 1
    {
        iTempPassTime[0] |= 0x01;
        iTempPassTime[1] |= 0x00;
    }
    else if((YELLOW == cPhaseColor) || (YELLOW_BLINK == cPhaseColor))//0 0 
    {
        iTempPassTime[0] |= 0x00; 
        iTempPassTime[1] |= 0x00; 
    }
}

/*****************************************************************************
 函 数 名  : GetNextControllerId
 功能描述  : 计算当前运行的相位ID的下一个相位
 输入参数  : unsigned char cDeviceId  
             unsigned char cNowId     
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned char GetNextControllerId(unsigned char cDeviceId,unsigned char cNowId)
{
    int i = 0;
    unsigned char cIndex = 0;
    
    for(i = 0; i < MAX_NUM_PHASE; i++)
    {
        if(cNowId == g_CountDownCfgLaiSi.cControllerID[cDeviceId][i])
        {
            cIndex = i;
        }
    }
    //如果当前是相位数组的最后一个或者其下一个值是0，那么就要返回第一个控制源ID
    if((cIndex == (MAX_NUM_PHASE - 1)) || (g_CountDownCfgLaiSi.cControllerID[cDeviceId][cIndex+1] == 0))
    {
        return g_CountDownCfgLaiSi.cControllerID[cDeviceId][0];
    }
    else
    {
        return g_CountDownCfgLaiSi.cControllerID[cDeviceId][cIndex+1];
    }
}

/*****************************************************************************
 函 数 名  : GetRuningPhaseId
 功能描述  : 计算某个设备当前正要运行的相位号
 输入参数  : unsigned char cDeviceId  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned char GetRuningPhaseId(unsigned char cDeviceId)
{
    int i = 0;
    unsigned char cControllerID = 0;
    unsigned char cControllerType = g_CountDownCfgLaiSi.cControllerType[cDeviceId];

    static unsigned char cLastControllerId[MAX_NUM_COUNTDOWN] = {0};

    for(i = 0; i < MAX_NUM_PHASE; i++)
    {
        cControllerID = g_CountDownCfgLaiSi.cControllerID[cDeviceId][i];

        if(cControllerID == 0)
        {
            break;
        }

        if(g_CountDownParamsLaiSi[cControllerType-1][cControllerID - 1].cColor == GREEN)
        {
            cLastControllerId[cDeviceId] = cControllerID;

            return cControllerID;
        }
    }

    cControllerID = cLastControllerId[cDeviceId];
    //如果该相位当前是黄灯，且黄灯的倒计时是1，那么，下次该方向的倒计时要显示下一个相位
    if((g_CountDownParamsLaiSi[cControllerType-1][cControllerID - 1].cColor == YELLOW)&&
        (g_CountDownParamsLaiSi[cControllerType-1][cControllerID - 1].cTime == 1))
    {
        cLastControllerId[cDeviceId] = GetNextControllerId(cDeviceId,cControllerID);
    }
    
    return cControllerID;
}
#define GET_COLOR(val)    (((val) == 1) ? "绿" : ((val == 2) ? "红" : ((val == 3) ? "黄" : "")))

/*****************************************************************************
 函 数 名  : SetCountValue
 功能描述  : 计算给定设备ID的倒计时及灯色信息。
 输入参数  : unsigned char iTempPassTime[2]  
             unsigned char cDeviceId         
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void SetCountValue(unsigned char iTempPassTime[2],unsigned char cDeviceId)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;
    unsigned char cPhaseId = 0;

    cPhaseId = GetRuningPhaseId(cDeviceId);
    
    cPhaseCountDownTime = g_CountDownParamsLaiSi[g_CountDownCfgLaiSi.cControllerType[cDeviceId]-1][cPhaseId - 1].cTime;
    cPhaseColor = g_CountDownParamsLaiSi[g_CountDownCfgLaiSi.cControllerType[cDeviceId]-1][cPhaseId - 1].cColor;

    //fprintf(stderr,"SetCountValue  --->cDeviceId %d, cPhaseId %d,  countdown %d  color %s\n",cDeviceId,cPhaseId,cPhaseCountDownTime,GET_COLOR(cPhaseColor));

    ConvertData(iTempPassTime,cPhaseCountDownTime,cPhaseColor);
}


/*****************************************************************************
 函 数 名  : countDownByLaiSiProtocol
 功能描述  : 莱斯协议具体实现
 返 回 值  :                      
 修改历史  
  1.日    期   : 2015年3月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数
具体思路:
1. 比如说需要发送 a秒的红灯时间，a的十位数字是B,个位数字是C
2. 把B和C按照约定的转码规则进行转码:（0x02, 0x9e, 0x24, 0x0c,  0x98, 0x48, 0x40, 0x1e, 0x00, 0x08, 0x10, 0x0c0, 0x62, 0x84, 0x60, 0x70）分别对应数字0 ~ 15 的七段码。
3. 比如说a=24s的话，那么B就是2、C就是4，其对应的转码分别是0x24 0x98,红色是01
4. 十位和个位分别与灯色进行位或操作，比如0x24 | 0x00 , 0x98 | 0x01，结果就是0x24 0x99
5. 那么发给倒计时牌的数据就是0x24 0x99
6. 0x50 0x51 0x52 0x53其实分别对应的ID为0 1 2 3的倒计时设备,这些设备并没有东南西北之分，需要根据实际情况来做
7. 0x60 0x70 0x80后面跟的16位数字要和0x50后面跟的数字相同，同理0x61 ... 只有这样，倒计时才认为校验正确。
*****************************************************************************/
void countDownByLaiSiProtocol(unsigned char *pBuf,unsigned char *pLen)
{
    int i =0;
    unsigned char iTempPassTime[2] = {0};//[0]个位，[1]十位
    unsigned char tempBuf[48] ={0x50,0xff,0xff,0x51,0xff,0xff,0x52,0xff,0xff,0x53,0xff,0xff,
                                0x60,0xff,0xff,0x61,0xff,0xff,0x62,0xff,0xff,0x63,0xff,0xff,
                                0x70,0xff,0xff,0x71,0xff,0xff,0x72,0xff,0xff,0x73,0xff,0xff,
                                0x80,0xff,0xff,0x81,0xff,0xff,0x82,0xff,0xff,0x83,0xff,0xff};

    if((NULL == pBuf) || (NULL == pLen))
    {
        return;
    }

    //按照深圳厂家的介绍，0x60 0x70 0x80与0x50的数据相同，依次类推
    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(g_CountDownCfgLaiSi.cControllerID[i][0] == 0)//如果某个倒计时没有被配置，则直接返回。
        {
            continue;
        }
    
        SetCountValue(iTempPassTime,g_CountDownCfgLaiSi.cDeviceId[i]);
        
        tempBuf[3*i+1] = iTempPassTime[1];
        tempBuf[3*i+2] = iTempPassTime[0];//0x50

        tempBuf[3*i+13] = iTempPassTime[1];
        tempBuf[3*i+14] = iTempPassTime[0];//0x60

        tempBuf[3*i+25] = iTempPassTime[1];
        tempBuf[3*i+26] = iTempPassTime[0];//0x70

        tempBuf[3*i+37] = iTempPassTime[1];
        tempBuf[3*i+38] = iTempPassTime[0];//0x80
    }
   // PrintData(tempBuf);
    memcpy(pBuf,tempBuf,48);
    *pLen = 48;//使用莱斯协议的话，需要通过485发送48字节的数据
}


/*****************************************************************************
 函 数 名  : SetCountDownValueLaiSi
 功能描述  : 给莱斯协议倒计时参数赋值，可以在倒计时接口中调用。
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void SetCountDownValueLaiSi(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend)
{
    if(pCountDownParamsSend == NULL)
    {
        return;
    }

    int i = 0;
    
    for(i = 0; i < 16; i++)
    {
        //机动车倒计时
        g_CountDownParamsLaiSi[MOTOR-1][i].cControllerID = i+1;
        g_CountDownParamsLaiSi[MOTOR-1][i].cColor = pCountDownParamsSend->stVehPhaseCountingDown[i][0];
        g_CountDownParamsLaiSi[MOTOR-1][i].cTime = pCountDownParamsSend->stVehPhaseCountingDown[i][1];

        //行人倒计时
        g_CountDownParamsLaiSi[PEDESTRIAN-1][i].cControllerID = i+1;
        g_CountDownParamsLaiSi[PEDESTRIAN-1][i].cColor = pCountDownParamsSend->stPedPhaseCountingDown[i][0];
        g_CountDownParamsLaiSi[PEDESTRIAN-1][i].cTime = pCountDownParamsSend->stPedPhaseCountingDown[i][1];

        //跟随相位倒计时
        g_CountDownParamsLaiSi[FOLLOW-1][i].cControllerID = i+1;
        g_CountDownParamsLaiSi[FOLLOW-1][i].cColor = pCountDownParamsSend->ucOverlap[i][0];
        g_CountDownParamsLaiSi[FOLLOW-1][i].cTime = pCountDownParamsSend->ucOverlap[i][1];

       // if(i == 0)
        //ERR("%d ,  %s,  %d\n",g_CountDownParamsLaiSi[MOTOR-1][i].cControllerID,
          //                      GET_COLOR(g_CountDownParamsLaiSi[MOTOR-1][i].cColor),
            //                    g_CountDownParamsLaiSi[MOTOR-1][i].cTime);
    }
}


/*****************************************************************************
 函 数 名  : ReadLaiSiCfgFromIni
 功能描述  : 从配置文件中读取莱斯倒计时牌的配置信息
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadLaiSiCfgFromIni()
{
	parse_start(CFG_NAME_LAISI);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        sprintf(tmpstr,"LaiSiDevice_%d",i);

        g_CountDownCfgLaiSi.cDeviceId[i] = i;
        get_more_value(tmpstr,"cControllerID",g_CountDownCfgLaiSi.cControllerID[i],MAX_NUM_PHASE);
        g_CountDownCfgLaiSi.cControllerType[i] = get_one_value(tmpstr,"cControllerType");

        ERR("===>  id %d, cControllerId  %d, type %d \n",g_CountDownCfgLaiSi.cDeviceId[i],g_CountDownCfgLaiSi.cControllerID[i][0],g_CountDownCfgLaiSi.cControllerType[i]);
    }

    parse_end();
}

/*****************************************************************************
 函 数 名  : WriteLaiSiCfgToIni
 功能描述  : 将莱斯倒计时信息保存到配置文件中
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void WriteLaiSiCfgToIni()
{
	parse_start(CFG_NAME_LAISI);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        sprintf(tmpstr,"LaiSiDevice_%d",i);

        add_one_key(tmpstr,"cControllerType",g_CountDownCfgLaiSi.cControllerType[i]);
        add_more_key(tmpstr,"cControllerID",g_CountDownCfgLaiSi.cControllerID[i],MAX_NUM_PHASE);
    }

    parse_end();
}

/*****************************************************************************
 函 数 名  : UpdateLaiSiCfg
 功能描述  : 更新莱斯配置信息
 输入参数  : CountDownCfgLaiSi *pData  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void UpdateLaiSiCfg(CountDownCfgLaiSi *pData)
{
    if(!pData)
    {
        return;
    }

    memcpy(&g_CountDownCfgLaiSi,pData,sizeof(g_CountDownCfgLaiSi));
}

