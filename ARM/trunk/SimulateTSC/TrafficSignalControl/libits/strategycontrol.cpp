#include <cstring>
#include "its.h"
#include "ipc.h"
#include "strategycontrol.h"

using namespace HikStrategycontrol;

inline bool Strategycontrol::IsChange(ControlType newControlType, ControlType nowControlType,
	ControlMode newMode, ControlMode nowMode)
{
	if (newControlType < nowControlType)
		return false;	//如果当前新的控制类型优先级小于当前的控制类型则不响应
/*	if (nowControlType == AUTO_CONTROL
		&& nowControlType != newControlType
		&& newMode == SYSTEM_MODE)
		return false;	//如果当前是本地自动控制，接收的消息是上位机发送的系统控制则不改变控制模式
        */
#if 0
	if (nowMode < MANUAL_MODE && newMode > MANUAL_MODE)
		return false;	//不能直接从系统或是感应模式直接切换到黄闪全红等特殊控制模式
	if (nowMode >= MANUAL_MODE && newMode <= nowMode)
		return false;	//手动控制时优先级：黄闪 > 全红 > 关灯 > 步进，不能由低到高切换
#endif	
	if ((nowMode == YELLOWBLINK_MODE || nowMode == TURNOFF_LIGHTS_MODE 
			|| nowMode == ALLRED_MODE) && newMode == STEP_MODE)
		return false;	//不能由黄闪、全红或是关灯直接切换到步进
	return true;
}

void Strategycontrol::WriteControlModeChangeLog(ControlType controlType, ControlMode mode)
{
    FaultLogType type = INVALID_TYPE;
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
			//log_debug("auto control, mode: %d", mode);
			break;
		case WEB_CONTROL: 
			type = WEB_CONTROL_LOG; 
			gCurControlStatus = 2;	//系统协调控制
			//log_debug("web control, mode: %d", mode);
			break;
		case TOOL_CONTROL: 
			type = TOOL_CONTROL_LOG;
			gCurControlStatus = 2;	//系统协调控制
			//log_debug("tool control, mode: %d", mode);
			break;
		case KEY_CONTROL: gCurControlStatus = 5; break;	//手动面板控制
		case FAULT_CONTROL: 
			type = FAULT_FLASH;
			gCurControlStatus = 10;	//故障黄闪控制
			//log_debug("fault control, mode: %d", mode);
			break;
		default: type = INVALID_TYPE; break;
	}
	if (type != INVALID_TYPE)
		its.ItsWriteFaultLog(type, value);
	if (mode == SYSTEM_MODE)
		gCurControlStatus = 6;	//本地时段表控制
}

void Strategycontrol::run(void *arg)
{
	Ipc::ControlTypeMsg rmsg;
	Ipc::ControlModeMsg smsg;

	while (1)
	{
		std::memset(&rmsg, 0, sizeof(rmsg));
		if (ipc.MsgRecv(rmsg))
		{ 
            //INFO("recv controlType:%d, controlMode:%d  rmsg.mSchemeId:  %d\r\n", rmsg.controlType, rmsg.mode, rmsg.mSchemeId);
			if (IsChange(rmsg.controlType, curControlType, rmsg.mode, curMode))
			{

				if (rmsg.mode == SYSTEM_MODE || rmsg.controlType == AUTO_CONTROL)
					rmsg.mSchemeId = 0;
                //INFO("\nmode = %d (%d), controltype=%d (%d), stagenum=%d\n", curMode, rmsg.mode, curControlType, rmsg.controlType, rmsg.stageNum);
				//发送消息之后，更新controlType和mode
                if (rmsg.mode == STEP_MODE && rmsg.stageNum == -1)
                {	//说明是步进取消，把之前的控制模式发送给相位控制模块
                    rmsg.controlType = curControlType;
                    rmsg.mode = curMode;
                    rmsg.stageNum = 0;
                    rmsg.mSchemeId = curSchemeID;
                }
                else
                    rmsg.controlType = (rmsg.mode == SYSTEM_MODE) ? AUTO_CONTROL : rmsg.controlType;
                std::memcpy(&smsg, &rmsg, sizeof(rmsg));
                ipc.MsgSend(smsg);
                if (rmsg.mode != STEP_MODE)
                {
                    curControlType = rmsg.controlType;
                    curMode = rmsg.mode;
                    curSchemeID = rmsg.mSchemeId;
                }
                //INFO("mode = %d (%d), controltype=%d (%d), stagenum=%d\n", curMode, rmsg.mode, curControlType, rmsg.controlType, rmsg.stageNum);

				WriteControlModeChangeLog(rmsg.controlType, rmsg.mode);
			}
		}
	}
}

Strategycontrol::Strategycontrol(Ipc & c, Its & t) : ipc(c), its(t)
{
	curControlType = AUTO_CONTROL;	//默认系统自动控制类型
	curMode = SYSTEM_MODE;	//默认系统控制模式
	gCurControlStatus = 6;	//默认本地时段表控制
	start();
}

