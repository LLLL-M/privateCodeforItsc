#include <time.h>
#include <unistd.h>
#include "hikmsg.h"
#include "LogSystem.h"

static UInt8 gCurControlStatus = 6;	//默认本地时段表控制
extern pthread_rwlock_t gConfigLock;
extern int msgid;

extern UInt8 GetSchemeIdAndTimeGap(struct tm *now, int *ret);

UInt8 ItsControlStatusGet(void)
{
	return gCurControlStatus;
}

static inline Boolean IsModeChange(ControlMode newMode, ControlMode nowMode)
{
#if 0
	if (nowMode < MANUAL_MODE && newMode > MANUAL_MODE)
		return FALSE;	//不能直接从系统或是感应模式直接切换到黄闪全红等特殊控制模式
	if (nowMode >= MANUAL_MODE && newMode <= nowMode)
		return FALSE;	//手动控制时优先级：黄闪 > 全红 > 关灯 > 步进，不能由低到高切换
#endif	
	if ((nowMode == YELLOWBLINK_MODE || nowMode == TURNOFF_LIGHTS_MODE 
			|| nowMode == ALLRED_MODE) && newMode == STEP_MODE)
		return FALSE;	//手动控制时不能由黄闪、全红或是关灯直接切换到步进
	if (nowMode == newMode && nowMode != STEP_MODE && nowMode != MANUAL_MODE)
		return FALSE;
	return TRUE;
}

static ControlMode GetLoaclControlMode()
{
	UInt8 nSchemeID;
	time_t timep = time(NULL);    
    struct tm now;
	ControlMode mode;
	
    localtime_r(&timep, &now);
	pthread_rwlock_rdlock(&gConfigLock);
	nSchemeID = GetSchemeIdAndTimeGap(&now, NULL);//根据时段表ID，得到动作表ID    
	switch (nSchemeID)
	{
		case 0: mode = YELLOWBLINK_MODE; break; 					//找不到方案默认黄闪控制
		case INDUCTIVE_SCHEMEID: mode = INDUCTIVE_MODE; break;		//感应控制
		case YELLOWBLINK_SCHEMEID: mode = YELLOWBLINK_MODE; break;	//黄闪控制
		case ALLRED_SCHEMEID: mode = ALLRED_MODE; break;			//全红控制
		case TURNOFF_SCHEMEID: mode = TURNOFF_LIGHTS_MODE; break;	//关灯控制
		default: mode = SYSTEM_MODE; break;
	}
	pthread_rwlock_unlock(&gConfigLock);
	return mode;
}

static inline void WriteControlModeChangeLog(ControlType controlType, ControlMode mode)
{
	FaultLogType type;
	int value = mode;

	switch (controlType)
	{
		case AUTO_CONTROL:
			switch (mode)
			{
				case INDUCTIVE_MODE: type = TIMEINTERVAL_INDUCTIVE; break;		//感应控制
				case YELLOWBLINK_MODE: type = TIMEINTERVAL_FLASH; break;		//黄闪控制
				case ALLRED_MODE: type = TIMEINTERVAL_ALL_RED; break;			//全红控制
				case TURNOFF_LIGHTS_MODE: type = TIMEINTERVAL_TURN_OFF; break;	//关灯控制
				default: type = INVALID_TYPE; break;
			}
			value = 0;
			gCurControlStatus = 6;	//本地时段表控制
			log_debug("auto control, mode: %d", mode);
			break;
		case WEB_CONTROL: 
			type = WEB_CONTROL_LOG; 
			gCurControlStatus = 2;	//系统协调控制
			log_debug("web control, mode: %d", mode);
			break;
		case TOOL_CONTROL: 
			type = TOOL_CONTROL_LOG;
			gCurControlStatus = 2;	//系统协调控制
			log_debug("tool control, mode: %d", mode);
			break;
		case KEY_CONTROL: gCurControlStatus = 5; break;	//手动面板控制
		case FAULT_CONTROL: 
			type = FAULT_FLASH;
			log_debug("fault control, mode: %d", mode);
			break;
		default: type = INVALID_TYPE; break;
	}
	if (type != INVALID_TYPE)
		ItsWriteFaultLog(type, value);
	if (mode == SYSTEM_MODE)
		gCurControlStatus = 6;	//本地时段表控制
}

void *StrategyControlModule(void *arg)
{
	struct msgbuf msg;
	ControlType	controlType = AUTO_CONTROL;
	ControlMode mode = SYSTEM_MODE, localMode;

	while (1)
	{
		if (-1 != msgrcv(msgid, &msg, MSGSIZE, MSG_CONTROL_TYPE, IPC_NOWAIT))
		{
			INFO("recv controlType:%d, controlMode:%d", msg.msgControlType, msg.msgMode);
			if (msg.msgControlType >= controlType)
			{	//高优先级的控制类型
				//当切换为系统控制时改变控制类型为AUTO_CONTROL
				controlType = (msg.msgMode == SYSTEM_MODE) ? AUTO_CONTROL : msg.msgControlType;
				if (IsModeChange(msg.msgMode, mode))
				{
					mode = msg.msgMode;
					msg.mtype = MSG_CONTROL_MODE;
					msgsnd(msgid, &msg, MSGSIZE, 0);
					WriteControlModeChangeLog(msg.msgControlType, msg.msgMode);
				}
			}
		}	
		else
		{
			localMode = GetLoaclControlMode();
			if (controlType == AUTO_CONTROL && mode != localMode)
			{	//系统自动控制时从黄闪、关灯或是全红模式切回系统定周期控制或是感应控制
				mode = localMode;
				msg.mtype = MSG_CONTROL_MODE;
				msg.msgMode = mode;
				msgsnd(msgid, &msg, MSGSIZE, 0);
				WriteControlModeChangeLog(AUTO_CONTROL, mode);
			}
		}
		usleep(50000);	//每隔50ms检测一次
	}
	pthread_exit(NULL);
}

