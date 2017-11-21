#include <unistd.h>
#include <errno.h>
#include "its.h"
#include "hikmsg.h"
#include "LogSystem.h"

static UInt8 gCurControlStatus = 6;	//默认本地时段表控制
extern int msgid;
extern StepInductiveInfo stepInductive;
extern UINT8 VehicleGreenLightKeep;

UInt8 ItsControlStatusGet(void)
{
	return gCurControlStatus;
}
UINT8 ItsResetVehicleInductiveData()
{
	return 0;
}
UINT8 ItsResetNextCycleExtendTime()
{
	return 0;
}
void ItsGetCurControlType(UINT8 controltype)
{
	return;
}

static inline Boolean IsChange(ControlType newControlType, ControlType nowControlType,
	ControlMode newMode, ControlMode nowMode)
{
	if (newControlType < nowControlType)
		return FALSE;	//如果当前新的控制类型优先级小于当前的控制类型则不响应
#if 0
	if (nowControlType == AUTO_CONTROL 
		&& nowControlType != newControlType
		&& newMode == SYSTEM_MODE)
		return FALSE;	//如果当前是本地自动控制，接收的消息是上位机发送的系统控制则不改变控制模式
#endif
	if (nowControlType == AUTO_CONTROL 
		&& newControlType == AUTO_CONTROL
		&& newMode == nowMode)
		return FALSE;	//如果是本地自动控制，只有模式不一样时才会切换控制模式
#if 0
	if (nowMode < MANUAL_MODE && newMode > MANUAL_MODE)
		return FALSE;	//不能直接从系统或是感应模式直接切换到黄闪全红等特殊控制模式
	if (nowMode >= MANUAL_MODE && newMode <= nowMode)
		return FALSE;	//手动控制时优先级：黄闪 > 全红 > 关灯 > 步进，不能由低到高切换
#endif	
	if ((nowMode == YELLOWBLINK_MODE || nowMode == TURNOFF_LIGHTS_MODE 
			|| nowMode == ALLRED_MODE) && newMode == STEP_MODE)
		return FALSE;	//不能由黄闪、全红或是关灯直接切换到步进
	return TRUE;
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
				case INDUCTIVE_COORDINATE_MODE: type = TIMEINTERVAL_COORDINATE_INDUCTIVE; break;	//协调感应控制
				case INDUCTIVE_MODE: type = TIMEINTERVAL_INDUCTIVE; break;		//感应控制
				case YELLOWBLINK_MODE: type = TIMEINTERVAL_FLASH; break;		//黄闪控制
				case ALLRED_MODE: type = TIMEINTERVAL_ALL_RED; break;			//全红控制
				case TURNOFF_LIGHTS_MODE: type = TIMEINTERVAL_TURN_OFF; break;	//关灯控制
				default: type = INVALID_TYPE; break;
			}
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
			gCurControlStatus = 10;	//故障黄闪控制
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
	ControlMode mode = SYSTEM_MODE;

	while (1)
	{
		memset(&msg, 0, sizeof(msg));
		if (-1 != msgrcv(msgid, &msg, MSGSIZE, MSG_CONTROL_TYPE, 0))
		{ 
			//INFO("StrategyControlModule recv controlType:%d, controlMode:%d (%d) msg.msgmSchemeId:  %d", msg.msgControlType, msg.msgMode, mode, msg.msgmSchemeId);
			if (IsChange(msg.msgControlType, controlType, msg.msgMode, mode))
			{
				WriteControlModeChangeLog(msg.msgControlType, msg.msgMode);
				if (msg.msgMode == SYSTEM_MODE || msg.msgControlType == AUTO_CONTROL)
					msg.msgmSchemeId = 0;
				msg.mtype = MSG_CONTROL_MODE;
				if (msg.msgMode == STEP_MODE && msg.msgStageNum == -1)
				{	//说明是步进取消，把之前的控制模式发送给相位控制模块
					msg.msgControlType = controlType;
					msg.msgMode = mode;
				}
				else
					msg.msgControlType = (msg.msgMode == SYSTEM_MODE) ? AUTO_CONTROL : msg.msgControlType;
				if (-1 == msgsnd(msgid, &msg, MSGSIZE, 0))
				{	//发送消息失败
					ItsWriteFaultLog(SENDMSG_FAIL, MSG_CONTROL_MODE);
					log_error("%s msgsnd error, controlType:%d, mode:%d, msgmSchemeId:%d, error info:%s", __func__, controlType, mode, msg.msgmSchemeId, strerror(errno));
				}
				else
				{	//发送消息成功，更新controlType和mode
					if (msg.msgMode != STEP_MODE)
					{	//非步进模式才会更新控制类型和控制模式
						controlType = msg.msgControlType;
						mode = msg.msgMode;
					}
					if (msg.msgMode == INDUCTIVE_MODE || 
						msg.msgMode == SINGLE_ADAPT_MODE || 
						msg.msgMode == INDUCTIVE_COORDINATE_MODE)//感应控制模式，重设车检感应的数据
					{
						ItsResetVehicleInductiveData();
						ItsResetNextCycleExtendTime();
						stepInductive.stepNow = 0;
						stepInductive.stepStageNum = 0;
					}
					else if (msg.msgMode == SINGLE_SPOT_OPTIMIZE)
						ItsResetNextCycleExtendTime();
					else if (msg.msgMode == PEDESTRIAN_REQ_MODE)
						ItsSetReadPedestrianReqTime();
					if (msg.msgMode != PEDESTRIAN_REQ_MODE)
					{	
						//INFO("clear VehicleGreenLightKeep, msgmode=%d" msg.msgMode);
						VehicleGreenLightKeep = 0;
					}
					ItsGetCurControlType(msg.msgControlType);
				}
			}
		}
	}
	pthread_exit(NULL);
}

