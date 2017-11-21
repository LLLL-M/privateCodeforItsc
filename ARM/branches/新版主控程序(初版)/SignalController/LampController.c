/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : LampController.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月18日
  最近修改   :
  功能描述   : 这里定义了外部接口函数，SetLampStatus，设置通道灯的颜色
  函数列表   :
              SetLampArray
              SetLampStatus
  修改历史   :
  1.日    期   : 2014年7月18日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#include "LampController.h"


static unsigned short g_nLampStatus[8] = {0};

#define N 32
#define M(X) (1u<<(X))
static void __attribute__((unused)) print_data(unsigned int c)    
{ 
	int i;    
	for (i = N - 1; i >= 0; i--)	{         
	   putchar(((c & M(i)) == 0) ? '0':'1');
	   if(i % 8 == 0) printf(" ");
	}       
	printf("\n");
}


static int HardflashDogFlag = 0;

/*********************************************************************************
*
*	写硬黄闪信号。
*
***********************************************************************************/

void HardflashDogOutput(void)
{
	if (HardflashDogFlag == 1)
	{
		HardflashDogFlag = 0;
	}
	else
	{
		HardflashDogFlag = 1;
	}
#ifdef ARM_PLATFORM
	
	HardflashDogCtrl(HardflashDogFlag);
#endif
	return;
}




/*********************************************************************************
*
*  写灯控板信号灯。
*
***********************************************************************************/
static int PhaseLampOutput(int boardNum, unsigned short outLamp)
{

	//printf("=====>   %s   ===>INFO 100130:%d:0x%x|",__func__,boardNum, outLamp); 
	//print_data(outLamp);
	
	if (boardNum<1 || boardNum>8)
	{
		//printf("boardNum error:%d\n", boardNum);
		return -1;
	}

	g_nLampStatus[boardNum-1] = outLamp;
#ifdef ARM_PLATFORM
	
	if (boardNum == 8)
	{
		i_can_its_send_led_request(boardNum, g_nLampStatus);
		//主控板运行指示灯
		Hiktsc_Running_Status();
		//硬件看门狗喂狗函数
        //
		//HardwareWatchdogKeepAlive(boardNum);
	}
#endif	
	
	return 0;
}



/*****************************************************************************
 函 数 名  : SetLampArray
 功能描述  : 根据通道号及需要设置的灯色，得到可以调用CAN接口的16位值
 输入参数  : unsigned short *nOutLampArray  
             unsigned short val             
             LampColor type                 
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月18日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static void SetLampArray(unsigned short *nOutLampArray,unsigned short val,LampColor type)
{
    unsigned short nOutLampIndex = 0;
    
    if(nOutLampArray)
    {
        
        if(val <= 4)
        {
            nOutLampIndex = 1;
            //printf("type  :  0x%x   val  :  0x%x  nOutLampArray[n] 0x%x \n",type,val,nOutLampArray[nOutLampIndex - 1]);
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 8) && (val >= 5))
        {
            nOutLampIndex = 2;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 12) && (val >= 9))
        {
            nOutLampIndex = 3;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 16) && (val >= 13))
        {
            nOutLampIndex = 4;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 20) && (val >= 17))
        {
            nOutLampIndex = 5;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 24) && (val >= 21))
        {
            nOutLampIndex = 6;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 28) && (val >= 25))
        {
            nOutLampIndex = 7;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 32) && (val >= 29))
        {
            nOutLampIndex = 8;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }

       // printf("type  0x%x   val  0x%x nOutLampArray[nOutLampIndex - 1]  0x%x   \n",type,val,nOutLampArray[nOutLampIndex - 1]);
    }

}


/*****************************************************************************
 函 数 名  : SetLampStatus
 功能描述  : 设置通道灯色
 输入参数  : unsigned short nTotalControllerLampNum     该信号机在该模式下需要工作的通道总数  比如，信号机工运行8个通道
             unsigned short *nTotalControllerLampArray  该信号机在该模式下工作需要的具体通道数组
             unsigned short nNum                      本次需要同时点亮操作的通道的个数        比如，在当前相位下，需要运行2个通道  
             unsigned short *nArray                    本次需要同时点亮操作的通道数组指针      比如，当前运行的通道号为1 4，最小是1，从1开始计数 
             LampColor type                            需要点亮的具体操作，是黄灯还是绿灯红灯 
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月18日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static void SetLampStatus(unsigned short nTotalControllerLampNum,unsigned short *nTotalControllerLampArray,
                        unsigned short nNum,unsigned short *nArray,LampColor type)
{
    if(nNum > 32 || nTotalControllerLampNum > 32)
    {
        return ;
    }

    int i = 0;
    unsigned short nOutLampArray[8] = {0};

    for(i = 0 ;i < 8 ; i++)
    {
        nOutLampArray[i] = 0x5000;
    }



    //首先给需要直接点灯的通道通道赋值。

    if(type != Lamp_Off)
    {
        for(i=0; i < nNum; i++ )
        {
            SetLampArray(nOutLampArray,nArray[i],type);

        }
    }

    unsigned short nLeftArrayLen = 0;
    unsigned short nLeftArray[32] = {0};
    int j = 0;
    int flag = 0;

    //找到剩余的通道
    for(i=0; i < nTotalControllerLampNum; i++)
    {
        for(j=0; j < nNum; j++)
        {
            if(nTotalControllerLampArray[i] == nArray[j])
            {
                flag = 1;
                break;
            }
        }

        if(flag == 0)
        {
            nLeftArray[nLeftArrayLen] = nTotalControllerLampArray[i];
            nLeftArrayLen++;
        }

        flag = 0;
    }

    //
    for(i=0; i < nLeftArrayLen; i++)
    {
        SetLampArray(nOutLampArray, nLeftArray[i], Lamp_Red);
    }

    for(i=1; i <= 8; i++)
    {
        //在这里调用CAN通信代码，用来发送点灯请求i_can_its_send_led_request
        //printf("1 ====>   0x%x\n",nOutLampArray[i-1]);
        PhaseLampOutput(i, nOutLampArray[i-1]);

    }
}

/*****************************************************************************
 函 数 名  : ControlLampLight
 功能描述  : 这个是留给外部调用的接口函数，控制信号灯的颜色及亮灭
 输入参数  : unsigned short nTotalControllerLampNum     该信号机在该模式下需要工作的通道总数  比如，信号机工运行8个通道
             unsigned short *nTotalControllerLampArray  该信号机在该模式下工作需要的具体通道数组
             unsigned short nNum                      本次需要同时点亮操作的通道的个数        比如，在当前相位下，需要运行2个通道  
             unsigned short *nArray                    本次需要同时点亮操作的通道数组指针      比如，当前运行的通道号为1 4，最小是1，从1开始计数 
             LampColor type                            需要点亮的具体操作，是黄灯还是绿灯红灯 
             short nLightTime                          该灯持续该种状态的时间             
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月19日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void ControlLampLight(unsigned short nTotalControllerLampNum,unsigned short *nTotalControllerLampArray,
                        unsigned short nNum,unsigned short *nArray,LampColor type,short nLightTime)
{

    if(!nArray || !nTotalControllerLampArray || nLightTime <= 0)
    {
        return;
    }
    nLightTime++;
    do
    {
        if((type == Lamp_Green) || (type == Lamp_Yellow) || (type == Lamp_Red))
        {
           SetLampStatus(nTotalControllerLampNum,nTotalControllerLampArray,nNum,nArray,type);

           sleep(1);

           //HardflashDogOutput();
          // continue;
        }        

        if((type == Lamp_Green_Light) || (type == Lamp_Yellow_Light) || (type == Lamp_Red_Light))
        {
           // printf("Red\n");
           SetLampStatus(nTotalControllerLampNum,nTotalControllerLampArray,nNum,nArray,type-4);//先变成指定的颜色
           //sleep(1);//等待1秒钟
           usleep(500*1000);
          // nLightTime--;
          // printf("Off\n");
           SetLampStatus(nTotalControllerLampNum,nTotalControllerLampArray,nNum,nArray,Lamp_Off);//把当前指定灯灭掉
           //sleep(1);
           usleep(500*1000);
           //HardflashDogOutput();
           //continue;
        } 


    }while(--nLightTime > 0);




}


void InitLampController()
{
#ifdef ARM_PLATFORM
	//can通信初始化
	i_can_its_init();
	
	//CPU通过CPLD对外的IO口初始化
	CPLD_IO_Init();
#endif
}

#if 0

int main(void)
{
    unsigned short nTotalControllerLampArray[] = {1,2,3,4,5,6,7,8,9};
    unsigned short nArrayLamp[] = {1,3,5,7};



    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Red,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Green,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Yellow,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Red_Light,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Green_Light,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Yellow_Light,3);
    


    return 0;
    while(1)
    {
        SetLampStatus(9,nTotalControllerLampArray,0, nArrayLamp, Lamp_Green);
        sleep(1);
        SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);
    }
    printf("All Red   :\n");



    printf("1 7 Green\n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);
    
    printf("1 7 Yellow \n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Yellow);

    printf("2 8 Green\n");
    nArrayLamp[0] = 2;
    nArrayLamp[1] = 8;
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);

    printf("2 8 Yellow\n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Yellow);

    printf("3 9 Green\n");
    nArrayLamp[0] = 3;
    nArrayLamp[1] = 9;
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);

    printf("3 9 Yellow\n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Yellow);

    return 0;

    SetLampStatus(8,nTotalControllerLampArray,0, nArrayLamp, Lamp_Green);
    SetLampStatus(8,nTotalControllerLampArray,1, nArrayLamp, Lamp_Yellow);

    printf("\n===============\n");
    unsigned short nArrayLamp2[] = {2};
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp2, Lamp_Green);
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp2, Lamp_Yellow);

    printf("\n===============\n");
    unsigned short nArrayLamp3[] = {3};
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp3, Lamp_Green);
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp3, Lamp_Yellow);



    return 0;
}
#endif
