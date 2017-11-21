#include "Main.h"


PSignalControllerPara pSignalControllerPara = null;//信号机配置参数主结构全局变量


extern unsigned short gCurrentChannelArray[MAX_CHANNEL_NUM] ;//当前配置下，总共使用的通道数数组，可以用来做形参，传递给点灯函数
extern unsigned short gCurrentChannelNum ;//当前配置下，总共使用的通道总数

unsigned short IsRunning = 1;//如果该参数为1，则各个模块正常工作，如果为0，则模块正常退出 

/*****************************************************************************
 函 数 名  : InitSignalControllerPara
 功能描述  : 系统启动后先初始化全局的信号控制参数，其中包括各个表格链表的头
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月24日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitSignalControllerPara()
{
    pSignalControllerPara = (PSignalControllerPara)malloc(sizeof(SignalControllerPara));

    if(!pSignalControllerPara)
    {
        log_debug("%s  alloc pSignalControllerPara failed \n",__func__);
        return ;
    }

    memset(pSignalControllerPara,0,sizeof(SignalControllerPara));

    //初始化调度表
    InitTaskSchedule(&pSignalControllerPara->pPlanScheduleList);

    //初始化时段表
    InitTimeIntervalList(&pSignalControllerPara->pTimeIntervalList);
    
    //初始化动作表
    InitActionList(&pSignalControllerPara->pActionList);

    //初始化方案表
    InitShemeList(&pSignalControllerPara->pSchemeList);

    //初始化通道表
    InitChannelList(&pSignalControllerPara->pChannelList);

    //初始化绿信比表
    InitGreenSignalRationList(&pSignalControllerPara->pGreenSignalRationList);

    //初始化相位表
    InitPhaseList(&pSignalControllerPara->pPhaseList);

    //获得全局单元参数
    pSignalControllerPara->pUintPara = InitUnit();

    //初始化跟随相位表
    InitFollowPhaseList(&pSignalControllerPara->pFollowPhaseList);

    //初始化相序表
    InitPhaseTurnList(&pSignalControllerPara->pPhaseTurnList);

}


/*****************************************************************************
 函 数 名  : InitGolobalVar
 功能描述  : 按顺序初始化指定的全局变量
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月17日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void InitGolobalVar()
{
    //初始化本地日志系统
    InitLogSys(LOG_PATH, LOG_MAX_SIZE, LOG_MAX_SIZE_EACH);

    //初始化日期
    InitDateTime();

    //初始化线程监控 模块
    InitThreadControllerList();

    //初始化全局参数 
    InitSignalControllerPara();

    //初始化CAN通信及CPLD
    InitLampController();

}

/*****************************************************************************
 函 数 名  : SetGolobalVar
 功能描述  : 按一定顺序给全局变量赋值，这里只是加载默认值，其实应该是从数据
             库或者配置文件里面读取出来的。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月17日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void SetGolobalVar()
{
    LoadDefaultTaskSchedule(pSignalControllerPara->pPlanScheduleList);

    LoadDefaultTimeInterval(pSignalControllerPara->pTimeIntervalList);

    LoadDefaultSchemes(pSignalControllerPara->pSchemeList);

    LoadDefaultAction(pSignalControllerPara->pActionList);

    LoadDefaultPhase(pSignalControllerPara->pPhaseList);

    LoadDefaultChannel(pSignalControllerPara->pChannelList);

    LoadDefaultGreenSignalRation(pSignalControllerPara->pGreenSignalRationList);

    LoadDefaultUnitPara(pSignalControllerPara->pUintPara);

    LoadDefaultFollowPhase(pSignalControllerPara->pFollowPhaseList);

    LoadDefaultPhaseTurnList(pSignalControllerPara->pPhaseTurnList);
}



void DestroyGolobalVar()
{

    //TO DO  将其他的初始化变量统一添加在后面，并写成功标示。

    DestroyDateTime();


    DestroyThreadControllerList();

    DestroyLogSys();

    
}

/*****************************************************************************
 函 数 名  : PowerOnAllRed
 功能描述  : 系统启动运行黄闪后，再运行一段时间的全红
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月19日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void PowerOnAllRed()
{
    log_debug("%s time  %d\n",__func__,pSignalControllerPara->pUintPara->nBootAllRedTime);
    ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                    gCurrentChannelNum,gCurrentChannelArray,
                    Lamp_Red,pSignalControllerPara->pUintPara->nBootAllRedTime);

}
/*****************************************************************************
 函 数 名  : PowerOnYellowLight
 功能描述  : 系统启动后，先运行一段时间的黄闪
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月19日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
void PowerOnYellowLight()
{
    ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                    gCurrentChannelNum,gCurrentChannelArray,
                    Lamp_Yellow_Light,pSignalControllerPara->pUintPara->nBootYellowLightTime);
}


/*****************************************************************************
 函 数 名  : SignalControllerRun
 功能描述  : 信号机信号控制入口函数，其中包含了相位，通道等逻辑
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月19日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int SignalControllerRun(void)
{

    int nTimeIntervalID = 0;
    int nActionID = 0;
    int nSchemeID = 0;
    int nGreenSignalRatioID = 0;
    int nPhaseID = 0;
    int nControllerType = 0;
    int nPhaseTurnID = 0;
    int nPhaseNum = 0;
    unsigned short *nPhaseArray = null;
    
    
    PPhaseItem pTempPhaseItem = null;
    PChannelList pTempChannelList = pSignalControllerPara->pChannelList;

    unsigned short nArrayControllerChannel[MAX_CHANNEL_NUM];
    unsigned short nControllerChannelNum = 0;

    unsigned short nArrayControllerFollowChannel[MAX_CHANNEL_NUM];
    unsigned short nControllerFollowChannelNum = 0;

    //启动 黄闪
    PowerOnYellowLight();
    
    //启动全红
    PowerOnAllRed();


    while(IsRunning)
    {

        //根据当前时间，遍历调度表，得到时段表ID
        nTimeIntervalID = GetTimeIntervalIndex(&pSignalControllerPara->pPlanScheduleList);
        log_debug("nTimeIntervalID  :  %d\n",nTimeIntervalID);

        //根据时段表ID，得到动作表ID
        nActionID = GetActionID(&pSignalControllerPara->pTimeIntervalList, nTimeIntervalID);
        log_debug("nActionID  :  %d\n",nActionID);

        //根据动作表ID，得到方案表ID
        nSchemeID = GetSchemeID(&pSignalControllerPara->pActionList,nActionID);
        log_debug("nSchemeID   : %d\n",nSchemeID);

        //根据方案表ID，得到绿信比表ID
        nGreenSignalRatioID = GetGreenSignalRatioID(&pSignalControllerPara->pSchemeList,nSchemeID);
        log_debug("nGreenSignalRatioID  : %d\n",nGreenSignalRatioID);


        //根据方案表ID，得到相序表ID
        nPhaseTurnID = GetPhaseOrderID(&pSignalControllerPara->pSchemeList,nSchemeID);
        log_debug("nPhaseTurnID  : %d\n",nPhaseTurnID);

        //根据相序表ID得到相位总数
        if(nPhaseNum == 0)
        {
            nPhaseNum = GetPhaseNum(pSignalControllerPara->pPhaseTurnList,nPhaseTurnID);
            log_debug("nPhaseNum  : %d\n",nPhaseNum);

            nPhaseArray = GetPhaseArray(pSignalControllerPara->pPhaseTurnList,nPhaseTurnID);
        }
        
        //根据相位ID，得到相位项
        pTempPhaseItem = GetPhaseItem(pSignalControllerPara->pPhaseList, *nPhaseArray);
        if(!pTempPhaseItem)
        {
            log_debug("Phase %d doesn't exist \n",*nPhaseArray);
            nPhaseArray++;
            nPhaseNum--;
            continue;
        }

        
        nPhaseID = *nPhaseArray;
        nControllerType = MOTOR;

        log_debug("nPhaseID   %d\n",nPhaseID);

        //根据当前相位ID及当前的绿信比表ID，给相位表中的绿信比参数赋值。
        SetPhaseGreenSignalRationItem(&pSignalControllerPara->pGreenSignalRationList, nGreenSignalRatioID, nPhaseID);

        //log_debug("Current Color  : Green\n");
        //printf("Current Phase %d  Current Channel :",nPhaseID);

        nControllerChannelNum = 0;
        memset(nArrayControllerChannel,0,sizeof(nArrayControllerChannel));

        nControllerFollowChannelNum = 0;
        memset(nArrayControllerFollowChannel,0,sizeof(nArrayControllerFollowChannel));

        //找到当前相位下有多少通道
        while(pTempChannelList->next != null)
        {
            //机动车相位或行人相位
            if((pTempChannelList->pItem->nControllerID == nPhaseID) && (pTempChannelList->pItem->nControllerType == nControllerType))
            {
                //printf(" ########## %d\n",pTempChannelList->pItem->nChannelID);
                nArrayControllerChannel[nControllerChannelNum++] = pTempChannelList->pItem->nChannelID;
            }

            //判断是否是跟随相位
            if((pTempChannelList->pItem->nControllerType == FOLLOW) && (IsPhaseInFollowPhase(pSignalControllerPara->pFollowPhaseList,nPhaseID,pTempChannelList->pItem->nControllerID)))
            {
                nArrayControllerFollowChannel[nControllerFollowChannelNum++] = pTempChannelList->pItem->nChannelID;
            }

            
            pTempChannelList = pTempChannelList->next;

        }
        log_debug("Green\n");
        //绿灯时间
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Green, 
                        pTempPhaseItem->nGreenSignalRationTime - pTempPhaseItem->nAllRedTime - pTempPhaseItem->nGreenLightTime - pTempPhaseItem->nYellowTime);



        log_debug("Green Light\n");
        //绿闪时间
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Green_Light, 
                        pTempPhaseItem->nGreenLightTime);

        log_debug("Yellow Light\n");
        //黄灯时间
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Yellow,
                        pTempPhaseItem->nYellowTime);
        log_debug("All Red\n");                
        //全红时间
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Red, 
                        pTempPhaseItem->nYellowTime);

        //printf("\n");
        pTempChannelList = pSignalControllerPara->pChannelList;

        nPhaseArray++;
        nPhaseNum--;

    }

    return true;
}


/*****************************************************************************
 函 数 名  : main
 功能描述  : 信号机主函数
 输入参数  : int argc     
             char **argv  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月21日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int main(int argc, char **argv)
{
    InitGolobalVar();

    SetGolobalVar();

    ThreadCreate((void *)ThreadUpdateDateTime,NULL);

    ThreadCreate((void *)ThreadClearMem,NULL);
    
    ThreadCreate((void *)SignalControllerRun,NULL);




    MainThreadControllerFun();

    DestroyGolobalVar();

    
    return 0;
}

