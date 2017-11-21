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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "HikConfig.h"
#include "platform.h"
#include "parse_ini.h"
#include "countDown.h"
#include "common.h"
#include "configureManagement.h"
#include "its.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_NUM_COUNTDOWN_LAISI   16
#define MAX_NUM_PHASE       16
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
extern CountDownCfg        g_CountDownCfg;                         //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识

static unsigned char g_SendBufLaiSi[48] = {0};

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
/*static void PrintData(unsigned char *nArray)
{
    int i = 0;
	
	if(nArray == NULL)
	{
		return;
	}
    for(i = 1; i <= 48; i++)
    {
        fprintf(stderr,"%2x ",nArray[i - 1]);
        if((i%12 == 0) && (i != 0))
        {
            fprintf(stderr,"\n");
        }
    }
    
    fprintf(stderr,"\n\n\n");
}*/


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

    GetRuningPhaseId(cDeviceId,&cPhaseCountDownTime,&cPhaseColor);

    ConvertData(iTempPassTime,cPhaseCountDownTime,cPhaseColor);
	
	if(gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime)//如果某相位的倒计时时间比感应检测时间大，则将显示时间清零，不予显示。如果需要实时显示倒计时状态，那么可以通过配置工具，将单元参数中的感应检测时间调大些。
	{
		iTempPassTime[0] = 0;
		iTempPassTime[1] = 0;
	}
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
8. 每秒钟调用一次即可。
*****************************************************************************/
void countDownByLaiSiProtocol()
{
    int i =0;
    unsigned char iTempPassTime[2] = {0};//[0]个位，[1]十位
    unsigned char tempBuf[48] ={0x50,0xff,0xff,0x51,0xff,0xff,0x52,0xff,0xff,0x53,0xff,0xff,
                                0x60,0xff,0xff,0x61,0xff,0xff,0x62,0xff,0xff,0x63,0xff,0xff,
                                0x70,0xff,0xff,0x71,0xff,0xff,0x72,0xff,0xff,0x73,0xff,0xff,
                                0x80,0xff,0xff,0x81,0xff,0xff,0x82,0xff,0xff,0x83,0xff,0xff};

    //按照深圳厂家的介绍，0x60 0x70 0x80与0x50的数据相同，依次类推
    for(i = 0; i < MAX_NUM_COUNTDOWN_LAISI; i++)
    {
        if(g_CountDownCfg.cControllerID[i][0] == 0)//如果某个倒计时没有被配置，则直接返回。
        {
            continue;
        }
    
        SetCountValue(iTempPassTime,g_CountDownCfg.cDeviceId[i]);
        
		if((iTempPassTime[0] == 0) && (iTempPassTime[1] == 0))//两位数均为0，表明该相位倒计时比感应检测时间大，不予显示。
		{
			continue;
		}
        tempBuf[3*i+1] = iTempPassTime[1];
        tempBuf[3*i+2] = iTempPassTime[0];//0x50

    }
   // PrintData(tempBuf);
    memcpy(g_SendBufLaiSi,tempBuf,sizeof(g_SendBufLaiSi));//使用莱斯协议的话，需要通过485发送48字节的数据

    Send485Data(g_SendBufLaiSi, sizeof(g_SendBufLaiSi));//发送数据
}


