#include <unistd.h>
#include <errno.h>
#include "its.h"
#include "hikmsg.h"
#include "LogSystem.h"

static UInt8 gCurControlStatus = 6;	//Ĭ�ϱ���ʱ�α����
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
		return FALSE;	//�����ǰ�µĿ����������ȼ�С�ڵ�ǰ�Ŀ�����������Ӧ
#if 0
	if (nowControlType == AUTO_CONTROL 
		&& nowControlType != newControlType
		&& newMode == SYSTEM_MODE)
		return FALSE;	//�����ǰ�Ǳ����Զ����ƣ����յ���Ϣ����λ�����͵�ϵͳ�����򲻸ı����ģʽ
#endif
	if (nowControlType == AUTO_CONTROL 
		&& newControlType == AUTO_CONTROL
		&& newMode == nowMode)
		return FALSE;	//����Ǳ����Զ����ƣ�ֻ��ģʽ��һ��ʱ�Ż��л�����ģʽ
#if 0
	if (nowMode < MANUAL_MODE && newMode > MANUAL_MODE)
		return FALSE;	//����ֱ�Ӵ�ϵͳ���Ǹ�Ӧģʽֱ���л�������ȫ����������ģʽ
	if (nowMode >= MANUAL_MODE && newMode <= nowMode)
		return FALSE;	//�ֶ�����ʱ���ȼ������� > ȫ�� > �ص� > �����������ɵ͵����л�
#endif	
	if ((nowMode == YELLOWBLINK_MODE || nowMode == TURNOFF_LIGHTS_MODE 
			|| nowMode == ALLRED_MODE) && newMode == STEP_MODE)
		return FALSE;	//�����ɻ�����ȫ����ǹص�ֱ���л�������
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
				case INDUCTIVE_COORDINATE_MODE: type = TIMEINTERVAL_COORDINATE_INDUCTIVE; break;	//Э����Ӧ����
				case INDUCTIVE_MODE: type = TIMEINTERVAL_INDUCTIVE; break;		//��Ӧ����
				case YELLOWBLINK_MODE: type = TIMEINTERVAL_FLASH; break;		//��������
				case ALLRED_MODE: type = TIMEINTERVAL_ALL_RED; break;			//ȫ�����
				case TURNOFF_LIGHTS_MODE: type = TIMEINTERVAL_TURN_OFF; break;	//�صƿ���
				default: type = INVALID_TYPE; break;
			}
			gCurControlStatus = 6;	//����ʱ�α����
			log_debug("auto control, mode: %d", mode);
			break;
		case WEB_CONTROL: 
			type = WEB_CONTROL_LOG; 
			gCurControlStatus = 2;	//ϵͳЭ������
			log_debug("web control, mode: %d", mode);
			break;
		case TOOL_CONTROL: 
			type = TOOL_CONTROL_LOG;
			gCurControlStatus = 2;	//ϵͳЭ������
			log_debug("tool control, mode: %d", mode);
			break;
		case KEY_CONTROL: gCurControlStatus = 5; break;	//�ֶ�������
		case FAULT_CONTROL: 
			type = FAULT_FLASH;
			gCurControlStatus = 10;	//���ϻ�������
			log_debug("fault control, mode: %d", mode);
			break;
		default: type = INVALID_TYPE; break;
	}
	if (type != INVALID_TYPE)
		ItsWriteFaultLog(type, value);
	if (mode == SYSTEM_MODE)
		gCurControlStatus = 6;	//����ʱ�α����
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
				{	//˵���ǲ���ȡ������֮ǰ�Ŀ���ģʽ���͸���λ����ģ��
					msg.msgControlType = controlType;
					msg.msgMode = mode;
				}
				else
					msg.msgControlType = (msg.msgMode == SYSTEM_MODE) ? AUTO_CONTROL : msg.msgControlType;
				if (-1 == msgsnd(msgid, &msg, MSGSIZE, 0))
				{	//������Ϣʧ��
					ItsWriteFaultLog(SENDMSG_FAIL, MSG_CONTROL_MODE);
					log_error("%s msgsnd error, controlType:%d, mode:%d, msgmSchemeId:%d, error info:%s", __func__, controlType, mode, msg.msgmSchemeId, strerror(errno));
				}
				else
				{	//������Ϣ�ɹ�������controlType��mode
					if (msg.msgMode != STEP_MODE)
					{	//�ǲ���ģʽ�Ż���¿������ͺͿ���ģʽ
						controlType = msg.msgControlType;
						mode = msg.msgMode;
					}
					if (msg.msgMode == INDUCTIVE_MODE || 
						msg.msgMode == SINGLE_ADAPT_MODE || 
						msg.msgMode == INDUCTIVE_COORDINATE_MODE)//��Ӧ����ģʽ�����賵���Ӧ������
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

