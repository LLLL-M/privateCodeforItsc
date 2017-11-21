#ifndef CHANLOCKCONTROL_H
#define CHANLOCKCONTROL_H
#include "HikConfig.h"
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "common.h"
#include "protocol.h"
#include "lock.h"
#include "config.h"
#include "custom.h"

using HikIts::Protocol;
using HikLock::RWlock;


typedef enum {
    E_CHAN_NOLOCK = 0,
    E_CHAN_LOCK = 1
}eChanLockFlag;

typedef enum {
    E_CHAN_NO_COLLISION = 0,
    E_CHAN_COLLISION = 1
}eChanCollisionFlag;


typedef enum
{
    E_CHAN_NO_LOCK = 0,
    E_CHAN_LOCK_TRANSITION_START = 0,
    E_CHAN_LOCK_TRANSITIONING = 1,
    E_CHAN_LOCK_LOCKING = 2,
    E_CHAN_UNLOCK_TRANSITIONING = 3,
}eChanLockStatus;

typedef struct lockTransitionTime
{
    UINT8 greenBlinkTime;
    UINT8 yellowTime;
    UINT8 yellowBlinkTime;
}STRU_LOCKTRANSITION_TIME;

#define NO_TRANSITION	254		//keep light status unchange while transitioning
#define CHAN_LOCK		255		//lock chan with some color, Red for example.

#define SET_TRANSITION_TIME(chan, v1,v2,v3)	({chan.greenBlinkTime = v1;\
                        chan.yellowTime = v2;	\
                        chan.yellowBlinkTime = v3;})
#define GET_MAX_TRANSITION_TIME(v0, ch)	(v0=((ch.greenBlinkTime+ch.yellowTime+ch.yellowBlinkTime)>v0?(ch.greenBlinkTime+ch.yellowTime+ch.yellowBlinkTime):v0))



class ChanLockControl
{
   public:
    STRU_LOCKTRANSITION_TIME gLockTransitionTable[32];
    STRU_LOCKTRANSITION_TIME gUnlockTransitionTable[32];

    UINT32 gChanCollisionTable[32];
    unsigned char gChanLockTypeFlag;
    UINT8 gLastChan[NUM_CHANNEL];
    unsigned char gMulPeriodsChanLockPeriodNum;
    unsigned char gWirelessControllerChanLockCurKey;
    UINT8 gtransitionStatus;
    Configfile* gConfig;
    Customfile* gCustom;

    ChanLockControl(Configfile* pconffile, Customfile* pcustomfile);
    ~ChanLockControl();

    char ChannelCollision(UINT8 *chan);
    void GetCurChanLockStatus(STRU_CHAN_LOCK_PARAMS *status);
    void setChanLockType(eChanLockFlag flag, unsigned char bit);
    eChanLockFlag RealTimeChanLock(UINT8 *chan);
    eChanLockFlag MulPeriodsChanLock(UINT8 *chan);
    eChanLockFlag WirelessControllerChanLock(UINT8 *chan);
    eChanLockFlag KeyBoardChanLock(UINT8 *chan);
    void ChanTransitionCheck(UINT8 *pLockstatus, UINT8**pTransitiontime,
                             STRU_LOCKTRANSITION_TIME *pTranstionconfig,
                             UINT8 lockflag, UINT8 *cur, UINT8 *lockstatus);

    void lockTransition(UINT8 lockflag, UINT8 *cur, UINT8 *lockstatus);
    void SetTransitionTimeByConfig(unsigned char *chan, STRU_LOCKTRANSITION_TIME* lock, STRU_LOCKTRANSITION_TIME* unlock);
    void SetTransitionTable(void);


};

#endif // CHANLOCKCONTROL_H
