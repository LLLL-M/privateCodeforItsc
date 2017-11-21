#include "canmsg.h"
#include "lcb.h"
#include "configureManagement.h"
#include "HikConfig.h"
#include "its.h"

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;

static UInt8 LCBphaseTurnAndSplitInfoSet(SignalControllerPara *para, 
											LCBphaseturninfo *p,
											LCBsplitinfo *s)
{
	int phaseturnid, splitid;
	int i, j;
	
	//把感应控制作为第0方案，使用相序表1，绿信比为最小绿+黄灯时间+全红时间
	for (j = 0; j < MAX_SUPPORT_PHASE_NUM; j++)
	{
		p[0].phases[j] = ((para->stPhaseTurn[0][0].nTurnArray[j] & 0xf) 
						 | (para->stPhaseTurn[0][1].nTurnArray[j] << 4));
		s[0].times[j] = para->stPhase[0][j].nMinGreen + para->stPhase[0][j].nYellowTime + para->stPhase[0][j].nAllRedTime;
	}
	for (i = 0; i < MAX_SUPPORT_SCHEME_NUM; i++)
	{	
		if (para->stScheme[3 * i].nCycleTime == 0)
			break;
		phaseturnid = para->stScheme[3 * i].nPhaseTurnID;
		splitid = para->stScheme[3 * i].nGreenSignalRatioID;
		if (phaseturnid == 0 || phaseturnid > MAX_SUPPORT_SCHEME_NUM
			|| splitid == 0 || splitid > MAX_SUPPORT_SCHEME_NUM)
			break;
		for (j = 0; j < MAX_SUPPORT_PHASE_NUM; j++)
		{	//因为把感应控制作为第0方案，所以此处要i+1
			p[i + 1].phases[j] = ((para->stPhaseTurn[phaseturnid - 1][0].nTurnArray[j] & 0xf) 
							 | (para->stPhaseTurn[phaseturnid - 1][1].nTurnArray[j] << 4));
			s[i + 1].times[j] = para->stGreenSignalRation[splitid - 1][j].nGreenSignalRationTime;
		}
	}
	return i;
}

static UInt8 LCBphaseInfoSet(SignalControllerPara *para, LCBphaseinfo *phaseinfo)
{
	UInt8 phase = 0, nControllerID;
	int i, j, k;
	
	for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
	{
		phase = para->stPhase[0][i].nPhaseID;
		if (!IS_PHASE_INABLE(para->stPhase[0][i].wPhaseOptions) || phase == 0)
			break;
		phaseinfo[i].greenFlashTime = para->AscSignalTransTable[0][phase - 1].nGreenLightTime;
		phaseinfo[i].yellowTime = para->stPhase[0][phase - 1].nYellowTime;
		phaseinfo[i].allredTime = para->stPhase[0][phase - 1].nAllRedTime;
		phaseinfo[i].pedFlashTime = para->stPhase[0][phase - 1].nPedestrianClearTime;
		for (j = 0; j < NUM_CHANNEL; j++)
		{
			nControllerID = para->stChannel[0][j].nControllerID;
			if ((para->stChannel[0][j].nControllerType == MOTOR || para->stChannel[0][j].nControllerType == PEDESTRIAN)
				&& nControllerID == phase)
				phaseinfo[i].channelbits |= (1 << j);
			else if (para->stChannel[0][j].nControllerType == FOLLOW && nControllerID > 0 && nControllerID <= NUM_PHASE)
			{
				for (k = 0; k < NUM_PHASE; k++)
				{
					if (phase == para->stFollowPhase[0][nControllerID - 1].nArrayMotherPhase[k])
					{
						phaseinfo[i].channelbits |= (1 << j);
						break;
					}
				}
			}
		}
	}
	return i;
}

static inline void LCBconfigSet(SignalControllerPara *para, LCBconfig *config)
{
	LCBbaseinfo *baseinfo = &config->baseinfo;
	memset(config, 0, sizeof(LCBconfig));
	//设置基本信息,默认开启相位接管功能
	baseinfo->isTakeOver = 1;	//gStructBinfileConfigPara.sSpecialParams.iPhaseTakeOverSwtich;
	baseinfo->isVoltCheckEnable = gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmSwitch;
	baseinfo->isCurCheckEnable = gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmSwitch;
	baseinfo->phaseNum = LCBphaseInfoSet(para, config->phaseinfo);//设置相位的相关信息
	baseinfo->minRedCurVal = gStructBinfileConfigPara.sCurrentParams[0].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[0].RedCurrentDiff;
	//设置相序和绿信比的相关信息
	baseinfo->schemeNum = LCBphaseTurnAndSplitInfoSet(para, config->phaseturninfo, config->splitinfo);
	baseinfo->canTotalNum = baseinfo->schemeNum * 2 + 1 //加1是因为要多发送一个感应方案的绿信比信息
							+ baseinfo->phaseNum;
}

static inline void SendCanMsgToBoard(UInt32 can_id, void *data)
{
	struct can_frame canfram;
	
	canfram.can_id = can_id;
	canfram.can_dlc = 8;	//所有通过can发送的LCB配置长度都为8字节
	memcpy(canfram.data, data, 8);
	canits_send(&canfram);
	//INFO("send baseinfo LCB config succ!");
	usleep(100000);
}

void SendLCBconfigToBoard(SignalControllerPara *para)
{
	LCBconfig config;
	int i, no = 1;
#ifdef FOR_SERVER
	return;
#endif
	LCBconfigSet(para, &config);
	if (!CheckLCBconfigValidity(&config))
	{	//设置的LCB配置检查无效
		ERR("LCB config set is invalid, so don't send it to Light Control Board!");
		return;
	}
	//检查设置有效，先发送基本信息
	SendCanMsgToBoard(LCB_BASEINFO_CAN, &config.baseinfo);
	//发送相位信息,一次发送两个相位的信息
	for (i = 0; i < config.baseinfo.phaseNum; i++, no++)
		SendCanMsgToBoard(LCB_PHASEINFO_CAN(no, i), &config.phaseinfo[i]);
	//发送相序信息
	for (i = 1; i <= config.baseinfo.schemeNum; i++, no++)
		SendCanMsgToBoard(LCB_PHASETURNINFO_CAN(no, i), &config.phaseturninfo[i]);
	//发送绿信比信息
	for (i = 1; i <= config.baseinfo.schemeNum; i++, no++)
		SendCanMsgToBoard(LCB_SPLITINFO_CAN(no, i), &config.splitinfo[i]);
	//最后一次发送感应控制的绿信比信息
	SendCanMsgToBoard(LCB_CAN_EXTID(no, 0, SPLITINFO), &config.splitinfo[0]);
	INFO("/***** send LCB config complete! *****/");
}

void SendRunInfoTOBoard(LineQueueData *data, SignalControllerPara *para)
{
	LCBruninfo runinfo;
	UInt8 status;
	LightValue lightvalue;
	int i;
	
	memset(&runinfo, 0, sizeof(runinfo));
	runinfo.schemeid =  data->schemeId;
	runinfo.runtime = data->cycleTime - data->leftTime;
	for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
	{
		status = data->phaseInfos[i].phaseStatus;
		if (status == GREEN)
			lightvalue = (data->phaseInfos[i].pedestrianPhaseStatus == GREEN_BLINK) ? LGREEN_FLASH_PED : LGREEN;
		else if (status == GREEN_BLINK)
			lightvalue = LGREEN_FLASH;
		else if (status == YELLOW)
			lightvalue = LYELLOW;
		else if (status == ALLRED)
			lightvalue = LRED;
		else
			continue;
			
		if (para->stPhase[0][i].nCircleID == 1)
		{
			runinfo.phaseR1 = i + 1;
			runinfo.lightvalueR1 = lightvalue;
		}
		else if (para->stPhase[0][i].nCircleID == 2)
		{
			runinfo.phaseR2 = i + 1;
			runinfo.lightvalueR2 = lightvalue;
		}
	}
	SendCanMsgToBoard(LCB_CAN_EXTID(0, 0, RUNINFO), &runinfo);
}
