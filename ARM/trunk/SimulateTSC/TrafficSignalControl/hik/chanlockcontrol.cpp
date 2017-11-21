#include <iostream>
#include "its.h"
#include "common.h"
#include "HikConfig.h"
#include "configureManagement.h"
#include "chanlockcontrol.h"

using namespace std;

//extern void ItsGetConflictChannel(UInt32 *conflictChannels);
//extern void WirelessSetAuto(void);
//extern void KeyBoardSetKey(unsigned char keyv);


ChanLockControl::ChanLockControl(Configfile* pconffile, Customfile* pcustomfile):gConfig(pconffile), gCustom(pcustomfile)
{
    memset(gLockTransitionTable, 0, sizeof(gLockTransitionTable));
    memset(gUnlockTransitionTable, 0, sizeof(gUnlockTransitionTable));
    memset(gChanCollisionTable, 0, sizeof(gChanCollisionTable));
    gChanLockTypeFlag = 0;
    gMulPeriodsChanLockPeriodNum = 0;
    gWirelessControllerChanLockCurKey = 0;
    memset(gLastChan, INVALID, NUM_CHANNEL);
    gtransitionStatus=E_CHAN_NO_LOCK;
}

ChanLockControl::~ChanLockControl()
{

}

char ChanLockControl::ChannelCollision(UINT8 *chan)
{
    int i,j;
    char ret=E_CHAN_NO_COLLISION;

    if(chan == NULL)
    {
        ERR("Can't judge without lockstatus!");
        return ret;
    }

    for(i=0; i<NUM_CHANNEL; i++)
    {
        for(j=i+1; j<NUM_CHANNEL; j++)
        {
            if((gChanCollisionTable[i] & (0x00000001<<j))!=0)
            {
                if((chan[i] == GREEN && (chan[j] == GREEN
                                        || chan[j]== GREEN_BLINK
                                        || chan[j]== YELLOW
                                        || chan[j]== YELLOW_BLINK
                                        || chan[j]== INVALID)) ||
                    (chan[i] == GREEN_BLINK && (chan[j] == GREEN
                                        || chan[j]== GREEN_BLINK
                                        || chan[j]== YELLOW
                                        || chan[j]== YELLOW_BLINK
                                        || chan[j]== INVALID)) ||
                    (chan[i] == YELLOW && (chan[j] == GREEN
                                        || chan[j]== GREEN_BLINK
                                        || chan[j]== YELLOW
                                        || chan[j]== YELLOW_BLINK
                                        || chan[j]== INVALID)) ||
                    (chan[i] == YELLOW_BLINK && (chan[j] == GREEN
                                        || chan[j]== GREEN_BLINK
                                        || chan[j]== YELLOW
                                        || chan[j]== YELLOW_BLINK
                                        || chan[j]== INVALID)) ||
                    (chan[i] == INVALID && (chan[j] == GREEN
                                        || chan[j]== GREEN_BLINK
                                        || chan[j]== YELLOW
                                        || chan[j]== YELLOW_BLINK))	)
                {
                    ret = E_CHAN_COLLISION;	//collision, can't lock
                    ERR("!!! Value:%x, Chan:%d and chan:%d collisioned!",gChanCollisionTable[i],i+1,j+1);
                    break;
                }
            }
        }
        if(j!=NUM_CHANNEL)
            break;
    }
    return ret;
}
/*************************
** BIT0:  MultPeriods Lock
** BIT1:  RealTime Lock
** BIT2:  WirelessController Lock
** BIT3:  FrontKeyBoard Lock

*************************/

void ChanLockControl::GetCurChanLockStatus(STRU_CHAN_LOCK_PARAMS *status)
{
    if(status == NULL)
        return;

    status->ucReserved = gChanLockTypeFlag;
    memcpy(status->ucChannelStatus, gLastChan, NUM_CHANNEL);
}
void ChanLockControl::setChanLockType(eChanLockFlag flag, unsigned char bit)
{
    if(flag == E_CHAN_LOCK && GET_BIT(gChanLockTypeFlag, bit) != 0x1)
        SET_BIT(gChanLockTypeFlag, bit);
    else if(flag == E_CHAN_NOLOCK && GET_BIT(gChanLockTypeFlag, bit) != 0)
        CLR_BIT(gChanLockTypeFlag, bit);
}
eChanLockFlag ChanLockControl::RealTimeChanLock(UINT8 *chan)
{
    int i;

    if(chan == NULL)
        return E_CHAN_NOLOCK;

    if(gCustom->custom.cChannelLockFlag == 0)
        return E_CHAN_NOLOCK;

    if(ChannelCollision(gCustom->custom.sChannelLockedParams.ucChannelStatus) == E_CHAN_COLLISION)
    {
        ERR("ChanCollisioned: Realtime channel lock auto unlock and clear config!");
        //added by Jicky
        gCustom->custom.cChannelLockFlag = 0;
        memset(&gCustom->custom.sChannelLockedParams, 0, sizeof(gCustom->custom.sChannelLockedParams));
        //WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
        return E_CHAN_NOLOCK;
    }

    for(i=0; i<NUM_CHANNEL; i++)
    {
        if(gCustom->custom.sChannelLockedParams.ucChannelStatus[i] != INVALID)
            chan[i] = gCustom->custom.sChannelLockedParams.ucChannelStatus[i];
    }

    return E_CHAN_LOCK;
}

eChanLockFlag ChanLockControl::MulPeriodsChanLock(UINT8 *chan)
{
    int i,j;
    time_t start,end,now;
    struct tm tmp;
    eChanLockFlag ret=E_CHAN_NOLOCK;

    if(chan == NULL)
        return E_CHAN_NOLOCK;

    if(gCustom->custom.sMulPeriodsChanLockParams.cLockFlag == 0)
    {
        gMulPeriodsChanLockPeriodNum = 0;
        return E_CHAN_NOLOCK;
    }

    time(&now);
    localtime_r(&now, &tmp);
    for(i=0; i<MAX_CHAN_LOCK_PERIODS; i++)
    {
        if(gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucWorkingTimeFlag == 0)
            continue;

        tmp.tm_hour = gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucBeginTimeHour;
        tmp.tm_min = gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucBeginTimeMin;
        tmp.tm_sec = gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucBeginTimeSec;
        start = mktime(&tmp);
        tmp.tm_hour = gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucEndTimeHour;
        tmp.tm_min = gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucEndTimeMin;
        tmp.tm_sec = gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucEndTimeSec;
        end = mktime(&tmp);
        if((start <= end) && (now >= start && now < end))
        {
            break;
        }
        else if((start > end) && (now >= start || now < end))
        {
            break;
        }
    }
    if(i != MAX_CHAN_LOCK_PERIODS)
    {
        if(ChannelCollision(gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucChannelStatus) == E_CHAN_NO_COLLISION)
        {
            gMulPeriodsChanLockPeriodNum = i+1;
            for(j=0; j<NUM_CHANNEL; j++)
            {
                if(gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucChannelStatus[j] != INVALID)
                    chan[j] = gCustom->custom.sMulPeriodsChanLockParams.chans[i].ucChannelStatus[j];
            }
            //memcpy(chan, gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucChannelStatus, NUM_CHANNEL);
            ret = E_CHAN_LOCK;
        }
        else
        {
            ERR("ChanCollisioned: MultiPeriods channel lock auto unlock!");
            gCustom->custom.sMulPeriodsChanLockParams.cLockFlag = E_CHAN_NOLOCK;
        }
    }
    else
        gMulPeriodsChanLockPeriodNum = 0;

    return ret;
}
#if defined(__linux__) && defined(__arm__)
eChanLockFlag ChanLockControl::WirelessControllerChanLock(UINT8 *chan)
{
    static unsigned char keyPressed=0;
    static unsigned char lastPressed=0;
    unsigned char key = 0;
    time_t curtime=0;
    static time_t lasttime=0;
    int i;
    eChanLockFlag ret = E_CHAN_NOLOCK;

    if(chan == NULL)
        return ret;

    time(&curtime);
    //INFO("switch: %d, ctrlMode: %d",gStructBinfileConfigPara.stWirelessController.iSwitch, gStructBinfileConfigPara.stWirelessController.iCtrlMode);
    if(gConfig->config.stWirelessController.iSwitch == WIRELESS_CTRL_ON
        && gConfig->config.stWirelessController.iCtrlMode == WIRELESS_CTRL_SELFDEF)
    {
        key = GetWirelessKeyStatus(E_KEY_NO_CHECK);
        if(key <0 || key > MAX_WIRELESS_KEY)
        {
            ERR("Illegal key value returned!");
            return E_CHAN_NOLOCK;
        }
        //INFO("--->Key :%d pressed...", key);
        if(key != 0)//new key pressed (1-5)
        {
            if(key == 2)//key 2 --> return to auto control
                key = 0; // no lock
            else if(key != 1)
                key -= 1;
            keyPressed = key;//(0-4)
        }
        if(keyPressed != lastPressed)// different key
        {
            lastPressed = keyPressed;
            lasttime = curtime;//update the start time of new key pressed
        }
        if((gConfig->config.stWirelessController.iOvertime != 0) &&
            ((curtime-lasttime)>gConfig->config.stWirelessController.iOvertime))
            keyPressed = 0;//return to auto control

        if(keyPressed)//lock
        {
            //INFO("--->Chan: %d, %d, %d, %d, %d...", gStructBinfileConfigPara.stWirelessController.key[keyPressed-1].ucChan[0]
            //	, gStructBinfileConfigPara.stWirelessController.key[keyPressed-1].ucChan[1]
            //	, gStructBinfileConfigPara.stWirelessController.key[keyPressed-1].ucChan[2]
            //	, gStructBinfileConfigPara.stWirelessController.key[keyPressed-1].ucChan[3]
            //	, gStructBinfileConfigPara.stWirelessController.key[keyPressed-1].ucChan[4]
            //);

            if(ChannelCollision(gConfig->config.stWirelessController.key[keyPressed-1].ucChan) == E_CHAN_NO_COLLISION)
            {
                memcpy(chan, gConfig->config.stWirelessController.key[keyPressed-1].ucChan, NUM_CHANNEL);
                ret = E_CHAN_LOCK;
            }
            else
            {
                ERR("ChanCollisioned: WirelessController return to Auto!");
                WirelessSetAuto();
                ret = E_CHAN_NOLOCK;
            }
        }
        gWirelessControllerChanLockCurKey = ((ret == E_CHAN_NOLOCK)?0 : keyPressed);
    }
    else
        gWirelessControllerChanLockCurKey = 0;

    return ret;
}
extern UINT8 gExtendKey;
unsigned char KeyDefLockStatus[8][NUM_CHANNEL]={
// east		south	  west		north
 {2,2,2,2, 2,2,2,2, 1,1,1,2, 2,2,2,1, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//east go
 {2,2,2,1, 2,2,2,2, 2,2,2,2, 1,1,1,2, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//south go
 {1,1,1,2, 2,2,2,1, 2,2,2,2, 2,2,2,2, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//west go
 {2,2,2,2, 1,1,1,2, 2,2,2,1, 2,2,2,2, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//north go

 {2,1,2,2, 2,2,2,1, 2,1,2,2, 2,2,2,1, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//east and west direct go
 {2,2,2,1, 2,1,2,2, 2,2,2,1, 2,1,2,2, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//south and north direct go
 {1,2,2,2, 2,2,2,2, 1,2,2,2, 2,2,2,2, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//east and west left go
 {2,2,2,2, 1,2,2,2, 2,2,2,2, 1,2,2,2, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},//south and north left go
};
#define IS_KEY_INRANGE(key)	(((key)>=6 && (key)<=13)? 1 : 0)
eChanLockFlag ChanLockControl::KeyBoardChanLock(UINT8 *chan)
{
    unsigned char *p=NULL;

    if(chan == NULL)
        return E_CHAN_NOLOCK;

    if(!IS_KEY_INRANGE(gExtendKey))//extend keys, id range: 6-13
        return E_CHAN_NOLOCK;

    if(E_KEY_CTRL_SELFDEF == gConfig->config.sFrontBoardKeys.iSwitch)//自定义通道值
        p = gConfig->config.sFrontBoardKeys.key[gExtendKey-6].ucChan;
    else
        p = KeyDefLockStatus[gExtendKey-6];

    if(ChannelCollision(p) == E_CHAN_COLLISION)
    {
        INFO("ChanCollisioned: KeyBoard return to Auto!");
        KeyBoardSetKey(1);//auto
        return E_CHAN_NOLOCK;
    }
    memcpy(chan, p, NUM_CHANNEL);
    return E_CHAN_LOCK;
}
#endif

void ChanLockControl::ChanTransitionCheck(UINT8 *pLockstatus, UINT8**pTransitiontime, STRU_LOCKTRANSITION_TIME *pTranstionconfig, UINT8 lockflag, UINT8 *cur, UINT8 *lockstatus)
{
    int i;

    for(i=0; i<NUM_CHANNEL; i++)
    {
        switch(pLockstatus[i])
        {
            case GREEN_BLINK:
                if(*pTransitiontime[i] == 0)
                {
                    pLockstatus[i] = pTranstionconfig[i].yellowTime!=0 ? YELLOW:(pTranstionconfig[i].yellowBlinkTime!=0 ? YELLOW_BLINK : INVALID);
                    pTransitiontime[i] = pTranstionconfig[i].yellowTime!=0 ? (&pTranstionconfig[i].yellowTime):(pTranstionconfig[i].yellowBlinkTime!=0?(&pTranstionconfig[i].yellowBlinkTime):NULL);
                    if(pLockstatus[i] != INVALID)
                    {
                        cur[i] = pLockstatus[i];
                        (*pTransitiontime[i])--;
                    }
                    else//only greenblink configured, no yellow and yb
                    {
                        pLockstatus[i] = CHAN_LOCK;
                        SET_TRANSITION_TIME(pTranstionconfig[i], 0, 0, 0);
                        cur[i] = RED;
                    }

                }
                else// within 3s gb
                {
                    cur[i] = pLockstatus[i];
                    (*pTransitiontime[i])--;
                }
                break;
            case YELLOW:
                if(*pTransitiontime[i] == 0)
                {
                    pLockstatus[i] = pTranstionconfig[i].yellowBlinkTime!=0 ? YELLOW_BLINK : INVALID;
                    pTransitiontime[i] = pTranstionconfig[i].yellowBlinkTime!=0?(&pTranstionconfig[i].yellowBlinkTime):NULL;
                    if(pLockstatus[i] != INVALID)
                    {
                        cur[i] = pLockstatus[i];
                        (*pTransitiontime[i])--;
                    }
                    else//INVALID
                    {
                        pLockstatus[i] = CHAN_LOCK;
                        SET_TRANSITION_TIME(pTranstionconfig[i], 0, 0, 0);
                        cur[i] = RED;//lockstatus[i];
                    }
                }
                else// within 3s yellow
                {
                    cur[i] = pLockstatus[i];
                    (*pTransitiontime[i])--;
                }
                break;
            case YELLOW_BLINK:
                if(*pTransitiontime[i] == 0)
                {
                    pLockstatus[i]=CHAN_LOCK;
                    pTransitiontime[i]= NULL;
                    cur[i] = RED;//lockstatus[i];
                    SET_TRANSITION_TIME(pTranstionconfig[i], 0, 0, 0);
                }
                else//within 3s yb
                {
                    cur[i] = pLockstatus[i];
                    (*pTransitiontime[i])--;
                }
                break;
            case NO_TRANSITION://other channel still need transition
                    if(INVALID != lockstatus[i])
                        cur[i] = lockstatus[i];
                    break;
            case CHAN_LOCK:
                    cur[i] = RED;
                    break;
            case INVALID://NO LOCK CHAN
            default:
                break;
        }
    }

}

void ChanLockControl::lockTransition(UINT8 lockflag, UINT8 *cur, UINT8 *lockstatus)
{
    static UINT8 lastLockStatus[NUM_CHANNEL]={INVALID};
    static UINT8* ltransitionTime[NUM_CHANNEL]={NULL};
    static UINT8 ltransitionLockStatus[NUM_CHANNEL]={0};
    STRU_LOCKTRANSITION_TIME zero[NUM_CHANNEL]={{0,0,0}};
    static STRU_LOCKTRANSITION_TIME transitionConfig[NUM_CHANNEL]={{0,0,0}};
    UINT8 temp[NUM_CHANNEL]={INVALID};
    int i,maxTransitionTime=0;

    if(cur == NULL || lockstatus == NULL)
        return;

    if(E_CHAN_LOCK == lockflag)
    {
        if(memcmp(lockstatus, lastLockStatus, NUM_CHANNEL) != 0 && gtransitionStatus != E_CHAN_LOCK_TRANSITIONING)
        {
            memcpy(temp, lockstatus, NUM_CHANNEL);
            if(gtransitionStatus == E_CHAN_LOCK_LOCKING)
            {
                for(i=0; i<NUM_CHANNEL; i++)
                {
                    if((lastLockStatus[i]*lockstatus[i])!=0 &&(lastLockStatus[i] ==  lockstatus[i]))// same lock
                        cur[i] = lastLockStatus[i];
                    else if(lastLockStatus[i] != INVALID && lockstatus[i] == INVALID)//part unlock
                    {
                        lockstatus[i] = cur[i];
                        cur[i] = lastLockStatus[i];
                    }
                    else if((lastLockStatus[i]*lockstatus[i])!=0)//chan lock value update
                    {
//						INFO("#### chan%d update: %d --> %d", i, lastLockStatus[i], lockstatus[i]);
                        cur[i] = lastLockStatus[i];
                    }
                }
            }
            gtransitionStatus = E_CHAN_LOCK_TRANSITION_START;
            memcpy(lastLockStatus, temp, NUM_CHANNEL);
        }
        switch(gtransitionStatus)
        {
            case E_CHAN_LOCK_TRANSITION_START:
                //INFO("---> lock transition start...");
                memset(transitionConfig, 0, sizeof(transitionConfig));
                memset(temp, 0, NUM_CHANNEL);
                memcpy(transitionConfig, gLockTransitionTable, sizeof(gLockTransitionTable));
                memcpy(temp, cur, NUM_CHANNEL);
                maxTransitionTime = 0;
                for(i=0; i<NUM_CHANNEL; i++)
                {
                    if(lockstatus[i] == INVALID)
                    {
                        SET_TRANSITION_TIME(transitionConfig[i], 0, 0, 0);
                        continue;
                    }

                    ltransitionLockStatus[i] = transitionConfig[i].greenBlinkTime!=0 ? GREEN_BLINK:(transitionConfig[i].yellowTime!=0 ? YELLOW:(transitionConfig[i].yellowBlinkTime!=0 ? YELLOW_BLINK : NO_TRANSITION));
                    ltransitionTime[i] = transitionConfig[i].greenBlinkTime!=0 ? &transitionConfig[i].greenBlinkTime:(transitionConfig[i].yellowTime!=0 ? (&transitionConfig[i].yellowTime):(transitionConfig[i].yellowBlinkTime!=0?(&transitionConfig[i].yellowBlinkTime):NULL));

                    if(ltransitionLockStatus[i] == NO_TRANSITION) // no transition config
                    {
                        cur[i] = RED;//lockstatus[i];
                        ltransitionLockStatus[i] = CHAN_LOCK;	//NOT
                        continue;
                    }

                    if(GREEN == cur[i] && RED == lockstatus[i])
                    {
                        //INFO("---> G->R: chan%d...", i);
                        GET_MAX_TRANSITION_TIME(maxTransitionTime, transitionConfig[i]);
                        cur[i] = ltransitionLockStatus[i];
                        (*ltransitionTime[i])--;
                    }
                    else if(GREEN_BLINK == cur[i] && RED == lockstatus[i])
                    {
                        //INFO("---> GB->R: chan%d...", i);
                        if(ltransitionLockStatus[i] == GREEN_BLINK)
                        {
                            ltransitionLockStatus[i] = transitionConfig[i].yellowTime!=0 ? YELLOW:(transitionConfig[i].yellowBlinkTime!=0 ? YELLOW_BLINK : NO_TRANSITION);
                            ltransitionTime[i] = transitionConfig[i].yellowTime!=0 ? (&transitionConfig[i].yellowTime):(transitionConfig[i].yellowBlinkTime!=0?(&transitionConfig[i].yellowBlinkTime):NULL);
                        }
                        if(ltransitionLockStatus[i] != NO_TRANSITION)
                        {
                            cur[i] = ltransitionLockStatus[i];
                            GET_MAX_TRANSITION_TIME(maxTransitionTime, transitionConfig[i]);
                            (*ltransitionTime[i])--;
                        }
                        else//only greenblink configured, no yellow and yb
                        {
                            SET_TRANSITION_TIME(transitionConfig[i], 0, 0, 0);
                            ltransitionLockStatus[i] = CHAN_LOCK;
                            cur[i] = RED;//lockstatus[i];
                        }
                    }
                    else if(cur[i] == lockstatus[i])
                    {
                        ltransitionLockStatus[i] = NO_TRANSITION;
                        ltransitionTime[i] = NULL;
                        SET_TRANSITION_TIME(transitionConfig[i], 0, 0, 0);
                        cur[i] = lockstatus[i];
                    }
                    else
                    {
                        ltransitionTime[i] = NULL;
                        SET_TRANSITION_TIME(transitionConfig[i], 0, 0, 0);
                        //INFO("------> NO transition ch%d, set time to 0...", i);
                        ltransitionLockStatus[i] = CHAN_LOCK;//NO_TRANSITION;//INVALID;
                        cur[i] = RED;//;lockstatus[i]
                    }

                }
                if(maxTransitionTime == 0)
                {
                    for(i=0; i<NUM_CHANNEL; i++)
                    {
                        if(lockstatus[i] == INVALID)
                            cur[i] = temp[i];
                        else
                            cur[i] = lockstatus[i];
                    }
                    gtransitionStatus = E_CHAN_LOCK_LOCKING;
                }
                else
                    gtransitionStatus = E_CHAN_LOCK_TRANSITIONING;
                break;
            case E_CHAN_LOCK_TRANSITIONING:
                //INFO("---> lock transitioning...");
                ChanTransitionCheck(ltransitionLockStatus, ltransitionTime, transitionConfig, lockflag, cur, lockstatus);
                if(!memcmp(zero, transitionConfig, NUM_CHANNEL*sizeof(STRU_LOCKTRANSITION_TIME)))
                {
                    gtransitionStatus = E_CHAN_LOCK_LOCKING;
                    memset(ltransitionLockStatus, 0, NUM_CHANNEL);
                    memset(ltransitionTime, 0, NUM_CHANNEL);
                }
                break;
            case E_CHAN_LOCK_LOCKING://transition complete, channel locking
                //INFO("---> chan locking...");
                for(i=0; i<NUM_CHANNEL; i++)
                {
                    if(lockstatus[i]!=INVALID)
                        cur[i] = lockstatus[i];
                }
                break;
            default:
                break;
        }
    }
    else//unlock or no lock
    {
        switch(gtransitionStatus)
        {
            case E_CHAN_LOCK_LOCKING://unlock
                memset(transitionConfig, 0, sizeof(transitionConfig));
                memset(temp, 0, NUM_CHANNEL);
                memcpy(transitionConfig, gUnlockTransitionTable, sizeof(gUnlockTransitionTable));
                memcpy(temp, cur, NUM_CHANNEL);
                maxTransitionTime = 0;
                //INFO("---> start to unlock...");
                for(i=0; i<NUM_CHANNEL; i++)
                {
                    if(lastLockStatus[i] == INVALID)
                    {
                        SET_TRANSITION_TIME(transitionConfig[i], 0, 0, 0);
                        continue;
                    }
                    ltransitionLockStatus[i] = transitionConfig[i].greenBlinkTime!=0 ? GREEN_BLINK:(transitionConfig[i].yellowTime!=0 ? YELLOW:(transitionConfig[i].yellowBlinkTime!=0 ? YELLOW_BLINK : NO_TRANSITION));
                    ltransitionTime[i] = transitionConfig[i].greenBlinkTime!=0 ? &transitionConfig[i].greenBlinkTime:(transitionConfig[i].yellowTime!=0 ? (&transitionConfig[i].yellowTime):(transitionConfig[i].yellowBlinkTime!=0?(&transitionConfig[i].yellowBlinkTime):NULL));

                    if(ltransitionLockStatus[i] == NO_TRANSITION) // no transition configure
                    {
                        //ltransitionLockStatus[i] = CHAN_LOCK;
                        cur[i] = lastLockStatus[i];//RED;
                        continue;
                    }

                    if(RED == cur[i] && GREEN == lastLockStatus[i])
                    {
                        GET_MAX_TRANSITION_TIME(maxTransitionTime, transitionConfig[i]);
                        cur[i] = ltransitionLockStatus[i];
                        (*ltransitionTime[i])--;
                    }
                    else if(RED == cur[i] && GREEN_BLINK == lastLockStatus[i])
                    {
                        if(ltransitionLockStatus[i] == GREEN_BLINK)
                        {
                            ltransitionLockStatus[i] = transitionConfig[i].yellowTime!=0 ? YELLOW:(transitionConfig[i].yellowBlinkTime!=0 ? YELLOW_BLINK : NO_TRANSITION);
                            ltransitionTime[i] = transitionConfig[i].yellowTime!=0 ? (&transitionConfig[i].yellowTime):(transitionConfig[i].yellowBlinkTime!=0?(&transitionConfig[i].yellowBlinkTime):NULL);
                        }
                        if(ltransitionLockStatus[i] != NO_TRANSITION)
                        {
                            GET_MAX_TRANSITION_TIME(maxTransitionTime, transitionConfig[i]);
                            cur[i] = ltransitionLockStatus[i];
                            (*ltransitionTime[i])--;
                        }
                        else
                        {
                            SET_TRANSITION_TIME(transitionConfig[i], 0, 0, 0);
                            //ltransitionLockStatus[i] = CHAN_LOCK;
                            //cur[i] = RED;
                            ltransitionLockStatus[i] = NO_TRANSITION;
                            cur[i] = lastLockStatus[i];
                        }
                    }
                    else
                    {
                        ltransitionLockStatus[i] = NO_TRANSITION;
                        //ltransitionLockStatus[i] = CHAN_LOCK;
                        ltransitionTime[i] = NULL;
                        SET_TRANSITION_TIME(transitionConfig[i], 0, 0, 0);
                        cur[i] = lastLockStatus[i];//RED;
                    }
                }
                if(maxTransitionTime == 0)
                {
                    for(i=0; i<NUM_CHANNEL; i++)
                    {
                        if(lockstatus[i] != INVALID)
                            cur[i] = temp[i];
                    }
                    gtransitionStatus = E_CHAN_NO_LOCK;
                }
                else
                    gtransitionStatus = E_CHAN_UNLOCK_TRANSITIONING;
//				gtransitionStatus = (maxTransitionTime == 0 ? E_CHAN_NO_LOCK : E_CHAN_UNLOCK_TRANSITIONING);
                break;
            case E_CHAN_UNLOCK_TRANSITIONING:
                ChanTransitionCheck(ltransitionLockStatus, ltransitionTime, transitionConfig, lockflag, cur, lastLockStatus);
                if(!memcmp(zero, transitionConfig, NUM_CHANNEL*sizeof(STRU_LOCKTRANSITION_TIME)))
                {
                    gtransitionStatus = E_CHAN_NO_LOCK;
                    memset(lastLockStatus, 0, NUM_CHANNEL);
                    memset(ltransitionLockStatus, 0, NUM_CHANNEL);
                    memset(ltransitionTime, 0, NUM_CHANNEL);
                }
                break;
            case E_CHAN_NO_LOCK:// no lock
                //INFO("---> unlock transition compeleted...");
            default:
                break;
        }
    }
}
void ChanLockControl::SetTransitionTimeByConfig(unsigned char *chan, STRU_LOCKTRANSITION_TIME* lock, STRU_LOCKTRANSITION_TIME* unlock)
{
    int i;
    if(NULL == chan)
        return;

    for(i=0; i<NUM_CHANNEL; i++)
    {
        if(chan[i] != INVALID)
        {
            if(lock)
                SET_TRANSITION_TIME(gLockTransitionTable[i], lock->greenBlinkTime, lock->yellowTime, lock->yellowBlinkTime);

            if(unlock)
                SET_TRANSITION_TIME(gUnlockTransitionTable[i], unlock->greenBlinkTime, unlock->yellowTime, unlock->yellowBlinkTime);
        }
    }
}
void ChanLockControl::SetTransitionTable(void)
{
    static unsigned char multPeriodsDelay=0;//unlcok delay time, save config for unlock transition
    static unsigned char realtimeDelay=0;

    STRU_LOCKTRANSITION_TIME tLock={0, 0, 0};//lock transiton config
    STRU_LOCKTRANSITION_TIME tUnlock={0, 0, 0};//unlock transition config
    static unsigned char lastPeriodNum=0;// last period number of MulPeriodsChanLock
#if defined(__linux__) && defined(__arm__)
    static unsigned char wirelessControllerDelay=0;
    static unsigned char keyboardDelay=0;
    static unsigned char lastWirelssKeyNum=0;
    static unsigned char lastKeyboardNum=0;
    unsigned char *p=NULL;
#endif
    unsigned char tmp=0;


    //multi periods lock
    if(E_CHAN_LOCK == gCustom->custom.sMulPeriodsChanLockParams.cLockFlag || multPeriodsDelay != 0)
    {
        if(gMulPeriodsChanLockPeriodNum != 0 || multPeriodsDelay != 0)
        {
            SET_TRANSITION_TIME(tLock, 3, 3, 0);
            SET_TRANSITION_TIME(tUnlock, 3, 3, 0);
            tmp = (gMulPeriodsChanLockPeriodNum == 0)?lastPeriodNum : gMulPeriodsChanLockPeriodNum;
            SetTransitionTimeByConfig(gCustom->custom.sMulPeriodsChanLockParams.chans[tmp-1].ucChannelStatus, &tLock, &tUnlock);
        }

        if(E_CHAN_LOCK == gCustom->custom.sMulPeriodsChanLockParams.cLockFlag && gMulPeriodsChanLockPeriodNum !=0 && multPeriodsDelay ==0)
            multPeriodsDelay = 1;
        else
            multPeriodsDelay = 0;
        //if((E_CHAN_NOLOCK == gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag && multPeriodsDelay != 0)
        //	|| (E_CHAN_LOCK == gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag && gMulPeriodsChanLockPeriodNum == 0&& multPeriodsDelay != 0))
        //	multPeriodsDelay = 0;
        //if(gMulPeriodsChanLockPeriodNum != 0)
        lastPeriodNum = gMulPeriodsChanLockPeriodNum;
    }

    //realtime lock
    if(E_CHAN_LOCK == gCustom->custom.cChannelLockFlag || realtimeDelay != 0)
    {
        SET_TRANSITION_TIME(tLock, 3, 3, 0);
        SET_TRANSITION_TIME(tUnlock, 3, 3, 0);
        SetTransitionTimeByConfig(gCustom->custom.sChannelLockedParams.ucChannelStatus, &tLock, &tUnlock);
        if(E_CHAN_LOCK == gCustom->custom.cChannelLockFlag && realtimeDelay ==0)
            realtimeDelay = 1;
        if(E_CHAN_NOLOCK == gCustom->custom.cChannelLockFlag && realtimeDelay != 0)
            realtimeDelay = 0;
    }
#if defined(__linux__) && defined(__arm__)
    //wireless controller
    if((gConfig->config.stWirelessController.iSwitch == WIRELESS_CTRL_ON
        && gConfig->config.stWirelessController.iCtrlMode == WIRELESS_CTRL_SELFDEF)
        || wirelessControllerDelay != 0)
    {
        if(gWirelessControllerChanLockCurKey != 0 || wirelessControllerDelay != 0)
        {
            SET_TRANSITION_TIME(tLock, 3, 3, 0);
            SET_TRANSITION_TIME(tUnlock, 3, 3, 0);
            tmp = (gWirelessControllerChanLockCurKey == 0)?lastWirelssKeyNum : gWirelessControllerChanLockCurKey;
            SetTransitionTimeByConfig(gConfig->config.stWirelessController.key[tmp-1].ucChan, &tLock, &tUnlock);
        }
        if((gConfig->config.stWirelessController.iSwitch == WIRELESS_CTRL_ON
        && gConfig->config.stWirelessController.iCtrlMode == WIRELESS_CTRL_SELFDEF)
        && gWirelessControllerChanLockCurKey !=0 && wirelessControllerDelay == 0)
            wirelessControllerDelay = 1;
        else
            wirelessControllerDelay = 0;

        lastWirelssKeyNum = gWirelessControllerChanLockCurKey;
    }

    //keyboard
    if(IS_KEY_INRANGE(gExtendKey) || keyboardDelay != 0)
    {
        if(IS_KEY_INRANGE(gExtendKey) || keyboardDelay != 0)
        {
            SET_TRANSITION_TIME(tLock, 3, 3, 0);
            SET_TRANSITION_TIME(tUnlock, 3, 3, 0);
            tmp = (!IS_KEY_INRANGE(gExtendKey))?lastKeyboardNum : gExtendKey;
            if(E_KEY_CTRL_SELFDEF == gConfig->config.sFrontBoardKeys.iSwitch)//自定义通道值
                p = gConfig->config.sFrontBoardKeys.key[tmp-6].ucChan;
            else
                p = KeyDefLockStatus[tmp-6];
            SetTransitionTimeByConfig(p, &tLock, &tUnlock);
        }
        if(IS_KEY_INRANGE(gExtendKey) && keyboardDelay ==0)
            keyboardDelay = 1;
        else
            keyboardDelay = 0;
        //if(gMulPeriodsChanLockPeriodNum != 0)
        lastKeyboardNum = gExtendKey;
    }
#endif
}


