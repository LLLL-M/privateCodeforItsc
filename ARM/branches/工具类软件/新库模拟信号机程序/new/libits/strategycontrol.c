#include <time.h>
#include <unistd.h>
#include "hikmsg.h"
#include "LogSystem.h"

static UInt8 gCurControlStatus = 6;	//Ĭ�ϱ���ʱ�α����
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
		return FALSE;	//����ֱ�Ӵ�ϵͳ���Ǹ�Ӧģʽֱ���л�������ȫ����������ģʽ
	if (nowMode >= MANUAL_MODE && newMode <= nowMode)
		return FALSE;	//�ֶ�����ʱ���ȼ������� > ȫ�� > �ص� > �����������ɵ͵����л�
#endif	
	if ((nowMode == YELLOWBLINK_MODE || nowMode == TURNOFF_LIGHTS_MODE 
			|| nowMode == ALLRED_MODE) && newMode == STEP_MODE)
		return FALSE;	//�ֶ�����ʱ�����ɻ�����ȫ����ǹص�ֱ���л�������
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
	nSchemeID = GetSchemeIdAndTimeGap(&now, NULL);//����ʱ�α�ID���õ�������ID    
	switch (nSchemeID)
	{
		case 0: mode = YELLOWBLINK_MODE; break; 					//�Ҳ�������Ĭ�ϻ�������
		case INDUCTIVE_SCHEMEID: mode = INDUCTIVE_MODE; break;		//��Ӧ����
		case YELLOWBLINK_SCHEMEID: mode = YELLOWBLINK_MODE; break;	//��������
		case ALLRED_SCHEMEID: mode = ALLRED_MODE; break;			//ȫ�����
		case TURNOFF_SCHEMEID: mode = TURNOFF_LIGHTS_MODE; break;	//�صƿ���
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
				case INDUCTIVE_MODE: type = TIMEINTERVAL_INDUCTIVE; break;		//��Ӧ����
				case YELLOWBLINK_MODE: type = TIMEINTERVAL_FLASH; break;		//��������
				case ALLRED_MODE: type = TIMEINTERVAL_ALL_RED; break;			//ȫ�����
				case TURNOFF_LIGHTS_MODE: type = TIMEINTERVAL_TURN_OFF; break;	//�صƿ���
				default: type = INVALID_TYPE; break;
			}
			value = 0;
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
	ControlMode mode = SYSTEM_MODE, localMode;

	while (1)
	{
		if (-1 != msgrcv(msgid, &msg, MSGSIZE, MSG_CONTROL_TYPE, IPC_NOWAIT))
		{
			INFO("recv controlType:%d, controlMode:%d", msg.msgControlType, msg.msgMode);
			if (msg.msgControlType >= controlType)
			{	//�����ȼ��Ŀ�������
				//���л�Ϊϵͳ����ʱ�ı��������ΪAUTO_CONTROL
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
			{	//ϵͳ�Զ�����ʱ�ӻ������صƻ���ȫ��ģʽ�л�ϵͳ�����ڿ��ƻ��Ǹ�Ӧ����
				mode = localMode;
				msg.mtype = MSG_CONTROL_MODE;
				msg.msgMode = mode;
				msgsnd(msgid, &msg, MSGSIZE, 0);
				WriteControlModeChangeLog(AUTO_CONTROL, mode);
			}
		}
		usleep(50000);	//ÿ��50ms���һ��
	}
	pthread_exit(NULL);
}

