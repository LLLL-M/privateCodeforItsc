#include <unistd.h>
#include <errno.h>
#include <math.h>
#include "its.h"
#include "configureManagement.h"
#include "gps.h"

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;
extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //混杂参数
#define WIRELESS_KEY_PRESS_GAP 3
extern UINT8 gPedestrianDetectorBits;
static inline UInt8 GetControlScheme(int ret)
{
	UInt8 schemeId = 0;
	FaultLogType type = INVALID_TYPE;
	switch (ret)
	{
		case 1:	schemeId = 0; type = MANUAL_TO_AUTO; break;
		case 2: schemeId = 0; type = AUTO_TO_MANUAL; break;
		case 3: schemeId = YELLOWBLINK_SCHEMEID; type = MANUAL_PANEL_FLASH; break;
		case 4: schemeId = ALLRED_SCHEMEID;	type = MANUAL_PANEL_ALL_RED; break;
		case 5: schemeId = STEP_SCHEMEID; type = MANUAL_PANEL_STEP; break;
	}
	ItsWriteFaultLog(type, 0);
	return schemeId;
}

UINT8 gExtendKey=0;
extern int lastKeyPressed;
void *DataCollectModule(void *arg)
{
	int ret = 0;
	UInt8 schemeId = 0;
	UINT8 nCount = 0;//每500ms获取一次GPS数据
	UINT8 keyboard = 0;
	UINT8 prewirelesskey = 0;
	time_t curtime = 0;
	time_t lasttime = 0;
	time_t readwireless = 0;

	while (1)
	{
		ret = ProcessKeyBoard();
		if(ret == 0)
			ret = GetKeyBoardStatus();//新版控制面板
		//INFO("ret=%d, Wireless switch:%d, ctrlMode: %d...", ret,gStructBinfileConfigPara.stWirelessController.iSwitch, gStructBinfileConfigPara.stWirelessController.iCtrlMode);
		if(ret == 0 && gStructBinfileConfigPara.stWirelessController.iSwitch == WIRELESS_CTRL_ON
			&& gStructBinfileConfigPara.stWirelessController.iCtrlMode == WIRELESS_CTRL_DEFAULT)
		{
			time(&curtime);
			ret = GetWirelessKeyStatus(E_KEY_CHECK);
			if (ret > 1 && (curtime - readwireless < WIRELESS_KEY_PRESS_GAP))
			{	//less than WIRELESS_KEY_PRESS_GAP , don't response wireless press key
				log_debug("second press key less than time gap WIRELESS_KEY_PRESS_GAP = %d , don't response wireless press key[%d]", WIRELESS_KEY_PRESS_GAP, ret);
				ret = 0;
			}
			if (ret > 0 && ret < 6)
			{
				readwireless = curtime;
				if (ret != prewirelesskey)
				{
					//INFO("new wireless key press, key=%d, prekey=%d", ret, prewirelesskey);
					prewirelesskey = ret;
					lasttime = curtime;
				}
			}
		/*	if (prewirelesskey == 2 && ((curtime - lasttime) >= WIRELESS_KEY_MANUAL_OVERTIME))
			{
				prewirelesskey = 1;
				keyboard = 0;
				INFO("ManualControl: Manualkey Overtime, Return to Autokey...");
				log_debug("ManualControl: Manualkey Overtime, Return to Autokey...");
			}*/
			if ((prewirelesskey > 2 && prewirelesskey < 6) && ((curtime - lasttime) > 
				((gStructBinfileConfigPara.stWirelessController.iOvertime > 0)? gStructBinfileConfigPara.stWirelessController.iOvertime : 300)))
			{
				ret = 1;//auto key
				prewirelesskey = 0;
				lastKeyPressed = 1;
				//INFO("wireless key timeout, recover auto");
			}
		}
		if (ret > 0 && ret < 6)
		{
			schemeId = GetControlScheme(ret);
			if (schemeId == STEP_SCHEMEID)
					keyboard = 5;
			if (ret != 2)	//手动按键按下不需要发送消息
			{	
				if (keyboard == 5 && ret == 1)//key board step to keyboard auto
				{
					keyboard = 0;
					//INFO("key board step to keyboard auto,cancle step");
					ItsCtl(KEY_CONTROL, STEP_SCHEMEID, -1);//send step cancle
				}
				else
					ItsCtl(KEY_CONTROL, schemeId, 0);
			/*	else if (keyboard == 2 && ret == 1)//press manul  to press auto
				{
					keyboard = 0;
					//INFO("press manul  to press auto");
					ItsCtl(KEY_CONTROL, schemeId, 0);
				}
				else
				{
					if (keyboard == 2 &&schemeId == STEP_SCHEMEID)
						keyboard = 5;
					//else 
						//keyboard = 0;
					if (keyboard == 2 || keyboard == 5)
					{
						//INFO("key %d press, scheme change %d", ret, schemeId);
						ItsCtl(KEY_CONTROL, schemeId, 0);
					}
				}*/
			}
			gExtendKey = 0;
		}
		else if(ret >= 6)
			gExtendKey = ret;
#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义		
		if(++nCount % 5 == 0)
		{
            nCount = 0;
			GetGPSTime(gStructBinfileConfigPara.sSpecialParams.iGpsSwitch, gStructBinfileMisc.time_zone_gap);
		}
#endif	
		gPedestrianDetectorBits |= (UINT8)PedestrianCheck();
		usleep(100000);		//每100ms采集一次
	}
}
