#include <stdio.h>
#include <string.h>
#include "HikConfig.h"
#include "mxmlresolve.h"
#include "configureManagement.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

typedef void*(*xmlHandleFunc)(void *arg);
struct xmlBlockInfo
{
	char *typeName;		//msg type name
	xmlHandleFunc get;	//get function
	xmlHandleFunc set;	//set function
};
static void *WirelessControllerInfoGet(void *arg);
static void *WirelessControllerInfoSet(void *arg);
static void *KeyBoardInfoGet(void *arg);
static void *KeyBoardInfoSet(void *arg);
static void *CountdownTimerInfoGet(void *arg);
static void *CountdownTimerInfoSet(void *arg);
static void *CarDetectorInfoGet(void *arg);
static void *CarDetectorInfoSet(void *arg);
static void *SysUserInfoGet(void *arg);
static void *SysUserInfoSet(void *arg);
static void *WifiInfoGet(void *arg);
static void *WifiInfoSet(void *arg);
static void *DeviceInfoGet(void *arg);
static void *DeviceInfoSet(void *arg);
static void *RGCheckInfoGet(void *arg);
static void *RGCheckInfoSet(void *arg);
static void *CameraInfoSet(void *arg);
static void *CameraInfoGet(void *arg);


extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;
extern CountDownCfg        g_CountDownCfg; 

struct xmlBlockInfo gSupportTypes[]={
	{"CountdownTimer", 		CountdownTimerInfoGet, 		CountdownTimerInfoSet},
	{"Keyboard", 			KeyBoardInfoGet, 			KeyBoardInfoSet},
	{"WirelessController", 	WirelessControllerInfoGet,	WirelessControllerInfoSet},
	{"SysUser", 			SysUserInfoGet, 			SysUserInfoSet},
	{"Wifi", 				WifiInfoGet, 				WifiInfoSet},
	{"CarDetector", 		CarDetectorInfoGet, 		CarDetectorInfoSet},
	{"SMDeviceInfo", 		DeviceInfoGet, 				DeviceInfoSet},
	{"RGSignalCheck", 		RGCheckInfoGet, 			RGCheckInfoSet},
	{"CameraInfo", 			CameraInfoGet, 				CameraInfoSet},
};
static char XMLCalcChecksum(char *xml, int len)
{
	int i;
	char *p = xml;
	char checksum=0;
	
	if(xml == NULL)
		return 0;
	checksum = *p++;
	for(i=0 ; i<len-1; i++,p++)
		checksum ^= *p;
	//INFO("checksum: %x", checksum);
	
	return checksum;
}
static char XMLMsgCheck(STRU_EXTEND_UDP_MSG *msg)
{
	char ret=0;
	
	if(msg == NULL)
		return 0;

	//INFO("Msg checksum: %d, calc checksum: %d", msg->checksum, XMLCalcChecksum(msg->xml, msg->len));
	if(msg->checksum == XMLCalcChecksum(msg->xml, msg->len))
		ret = 1;
	
	return ret;
}
static char* XMLMsgParse(char *str)
{
	char *op=NULL;
	char *type=NULL;
	int i,len;
	
	if(mxmlParseStart(str))
	{
		if((type = mxmlGetMsgType()) == NULL)
			type = "Unknown";
		if((op = mxmlGetMsgOp()) == NULL)
			op = "Unknown";

		mxmlGenerateStart(type, op);
		len = sizeof(gSupportTypes)/sizeof(struct xmlBlockInfo);

		if(strcmp(op,"Get") == 0)
		{
			for(i=0; i<len; i++)			
			{
				if(strcmp(gSupportTypes[i].typeName,type) == 0)
					break;
			}
			if(i!=len && gSupportTypes[i].get!=NULL)
				gSupportTypes[i].get(NULL);
			else
				mxmlAddNodeVar(NULL, "response", MXML_FAILURE);
		}
		else if(strcmp(op, "Set") == 0)
		{
			for(i=0; i<len; i++)			
			{
				if(strcmp(gSupportTypes[i].typeName,type) == 0)
					break;
			}
			if(i!=len && gSupportTypes[i].set!=NULL)
			{
				gSupportTypes[i].set(NULL);
				mxmlAddNodeVar(NULL, "response", MXML_SUCCESS);
			}
			else
				mxmlAddNodeVar(NULL, "response", MXML_FAILURE);
		}
		else
			mxmlAddNodeVar(NULL, "response", MXML_FAILURE);
		
		mxmlParseEnd();
		
	}
	else
	{
		ERR("str is not a legal param!");
		mxmlGenerateStart("Unknown", "Unknown");
		mxmlAddNodeVar(NULL, "response", MXML_FAILURE);
	}
	return mxmlGenerateFinished();
}
char XMLMsgHandle(STRU_EXTEND_UDP_MSG *msg)
{
	char *xml=NULL;

	if(XMLMsgCheck(msg))
	{
		xml = XMLMsgParse(msg->xml);
		memset(msg, 0, sizeof(STRU_EXTEND_UDP_MSG));
		msg->head = EXTEND_MSG_HEAD;
		msg->len = strlen(xml);
		memcpy(msg->xml, xml, msg->len);
		msg->checksum = XMLCalcChecksum(xml, msg->len);
	}
	return 0;
}
/******************************************************/
static void *WirelessControllerInfoGet(void *arg)
{
	mxmlNode *p=mxmlGetRequest();	
	mxml_node_t * t=NULL;
	char tmp[NUM_CHANNEL*2]={0};
	char *ob[] = {"iSwitch", "iOvertime","iCtrlMode","key","description","ucChan"};

	STRU_WIRELESS_CONTROLLER_INFO *pWireless = &gStructBinfileConfigPara.stWirelessController;
	if(p == NULL)
	{
		int i;
		mxmlAddNodeVar(NULL, ob[0], pWireless->iSwitch);
		mxmlAddNodeVar(NULL, ob[1], pWireless->iOvertime);
		mxmlAddNodeVar(NULL, ob[2], pWireless->iCtrlMode);
		for(i=0; i<MAX_WIRELESS_KEY-1; i++)
		{
			t = mxmlAddArr1(NULL, ob[3], i+1);
			mxmlAddNodeStr(t, ob[4], pWireless->key[i].description);
			mxmlAddNodeArr(t, ob[5], mxmlArray2Str((char*)pWireless->key[i].ucChan, NUM_CHANNEL, tmp),0);
		}
	}
	else
	{	
		while(p != NULL)
		{
			if(strcmp(p->ob,ob[0]) == 0)
			{
				mxmlAddNodeVar(NULL,p->ob,pWireless->iSwitch);
			}
			else if(strcmp(p->ob,ob[1]) == 0)
			{
				mxmlAddNodeVar(NULL,p->ob, pWireless->iOvertime);
			}
			else if(strcmp(p->ob,ob[2]) == 0)
			{
				mxmlAddNodeVar(NULL,p->ob, pWireless->iCtrlMode);
			}
			else if(strcmp(p->ob, ob[3])==0 && p->sub == NULL)
			{
				if(p->id1 >0 && p->id1 <MAX_WIRELESS_KEY)
				{
					t=mxmlAddArr1(NULL, p->ob, p->id1);
					mxmlAddNodeStr(t, ob[4], pWireless->key[p->id1-1].description);
					mxmlAddNodeArr(t, ob[5], mxmlArray2Str((char*)pWireless->key[p->id1-1].ucChan, NUM_CHANNEL, tmp), 0);
				}
			}		
			else if(strcmp(p->ob, ob[3])==0 && p->sub != NULL)
			{
				t=mxmlAddArr1(NULL, p->ob, p->id1);			
				if(strcmp(p->sub->ob, ob[4])==0)	
					mxmlAddNodeStr(t, ob[4], pWireless->key[p->id1-1].description);
				else if(strcmp(p->sub->ob, ob[5])==0)
					mxmlAddNodeArr(t, ob[5], mxmlArray2Str((char*)pWireless->key[p->id1-1].ucChan, NUM_CHANNEL, tmp), 0);
			}
			p=mxmlGetRequest();	
		}
	}

	return NULL;
}
static void* WirelessControllerInfoSet(void *arg)
{
	int i;
	char *p=NULL;
	char tmp[NUM_CHANNEL]={0};
	int len=0;
	sqlite3 *pdb = NULL;
	char *ob[] = {"iSwitch", "iOvertime","iCtrlMode","key","description","ucChan"};
	STRU_WIRELESS_CONTROLLER_INFO *pWireless = &gStructBinfileConfigPara.stWirelessController;
	
	//INFO("Wireless set request...\n");
	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[0], 0, 0, 0,NULL,0,0)) != NULL)
	{
		pWireless->iSwitch = atoi(p);	
		//INFO("Wireless set: switch:%d\n",atoi(p));
	}
	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[1], 0, 0, 0,NULL,0,0)) != NULL)
	{
		pWireless->iOvertime = atoi(p);
		//INFO("Wireless set: overtime:%d\n",atoi(p));
	}
	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[2], 0, 0, 0,NULL,0,0)) != NULL)
	{
		pWireless->iCtrlMode = atoi(p);	
		//INFO("Wireless set: ctrlMode:%d\n",atoi(p));
	}

	for(i=0; i<MAX_WIRELESS_KEY-1; i++)	
	{
		if((p=mxmlGetNode(MXML_ONE_DIMENSIONAL_ARR, ob[3],i+1,0,MXML_SIMPLE_STR, ob[4], 0, 0)) != NULL)
		{
			memset(pWireless->key[i].description, 0, sizeof(pWireless->key[i].description));
			strncpy(pWireless->key[i].description, p, (strlen(p)>64)? 64 : strlen(p));
			//INFO("Wireless set: description:%s\n",p);
		}
		if((p=mxmlGetNode(MXML_ONE_DIMENSIONAL_ARR, ob[3],i+1,0,MXML_SIMPLE_ARR, ob[5], 0,0)) != NULL)
		{
			len = mxmlStr2Array(p,tmp);
			memcpy(pWireless->key[i].ucChan, tmp, len);
			//INFO("Wireless set: ucChan: 0-%d, 7-%d\n",pWireless->key[i].ucChan[0],pWireless->key[i].ucChan[7]);
		}
	}
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
	log_debug("insert TABLE_NAME_CONF_WIRELESS ");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_clear_table(pdb, TABLE_NAME_CONF_WIRELESS);
	sqlite3_insert_wireless_controller(pdb, pWireless);
	sqlite3_close_wrapper(pdb); pdb = NULL;

	return NULL;
}
/**********************************************/
#define MAX_FRONT_KEY_BOARD_ID	MAX_FRONT_BOARD_KEY-5
static void *KeyBoardInfoGet(void *arg)
{
	mxmlNode *p=mxmlGetRequest();	
	mxml_node_t * t=NULL;
	char tmp[NUM_CHANNEL*2]={0};
	char *ob[] = {"iSwitch","key","description","ucChan"};

	STRU_FRONTBOARD_KEY_INFO *pKeyboard = &gStructBinfileConfigPara.sFrontBoardKeys;
	if(p == NULL)
	{
		int i;
		mxmlAddNodeVar(NULL, ob[0], pKeyboard->iSwitch);
		for(i=0; i<MAX_FRONT_KEY_BOARD_ID; i++)
		{
			t = mxmlAddArr1(NULL, ob[1], i+1);
			mxmlAddNodeStr(t, ob[2], pKeyboard->key[i].description);
			mxmlAddNodeArr(t, ob[3], mxmlArray2Str((char*)pKeyboard->key[i].ucChan, NUM_CHANNEL, tmp),0);
		}
	}
	else
	{	
		while(p!=NULL)
		{
			if(strcmp(p->ob, ob[0]) == 0)
			{
				mxmlAddNodeVar(NULL,p->ob,pKeyboard->iSwitch);
			}
			else if(strcmp(p->ob, ob[1])==0 && p->sub == NULL)
			{
				if(p->id1 >0 && p->id1 <MAX_FRONT_KEY_BOARD_ID)
				{
					t=mxmlAddArr1(NULL, p->ob, p->id1);
					mxmlAddNodeStr(t, ob[2], pKeyboard->key[p->id1-1].description);
					mxmlAddNodeArr(t, ob[3], mxmlArray2Str((char*)pKeyboard->key[p->id1-1].ucChan, NUM_CHANNEL, tmp), 0);
				}
			}		
			else if(strcmp(p->ob, ob[1])==0 && p->sub != NULL)
			{
				t=mxmlAddArr1(NULL, p->ob, p->id1);			
				if(strcmp(p->sub->ob, ob[2])==0)	
					mxmlAddNodeStr(t, ob[2], pKeyboard->key[p->id1-1].description);
				else if(strcmp(p->sub->ob, ob[3])==0)
					mxmlAddNodeArr(t, ob[3], mxmlArray2Str((char*)pKeyboard->key[p->id1-1].ucChan, NUM_CHANNEL, tmp), 0);
			}
			p=mxmlGetRequest();	
		}
	}

	return NULL;
}
static void* KeyBoardInfoSet(void *arg)
{
	int i;
	char *p=NULL;
	char tmp[NUM_CHANNEL]={0};
	int len=0;
	sqlite3 *pdb = NULL;
	char *ob[] = {"iSwitch","key","description","ucChan"};
	
	STRU_FRONTBOARD_KEY_INFO *pKeyboard = &gStructBinfileConfigPara.sFrontBoardKeys;
	//INFO("KeyBoard set request...\n");
	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[0], 0, 0, 0,NULL,0,0)) != NULL)
	{
		pKeyboard->iSwitch = atoi(p);	
		//INFO("KeyBoard set: switch:%d\n",atoi(p));
	}

	for(i=0; i<MAX_FRONT_KEY_BOARD_ID; i++)	
	{
		if((p=mxmlGetNode(MXML_ONE_DIMENSIONAL_ARR, ob[1],i+1,0,MXML_SIMPLE_STR, ob[2], 0, 0)) != NULL)
		{
			memset(pKeyboard->key[i].description, 0, sizeof(pKeyboard->key[i].description));
			strncpy(pKeyboard->key[i].description, p, (strlen(p)>64)? 64 : strlen(p));
			//INFO("KeyBoard set: description:%s\n",p);
		}
		if((p=mxmlGetNode(MXML_ONE_DIMENSIONAL_ARR, ob[1],i+1,0,MXML_SIMPLE_ARR, ob[3], 0,0)) != NULL)
		{
			len = mxmlStr2Array(p,tmp);
			memcpy(pKeyboard->key[i].ucChan, tmp, len);
			//INFO("KeyBoard set: ucChan: 0-%d, 7-%d\n",pKeyboard->key[i].ucChan[0],pKeyboard->key[i].ucChan[7]);
		}
	}
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
	log_debug("insert TABLE_NAME_CONF_FRONTBOARD ");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_clear_table(pdb, TABLE_NAME_CONF_FRONTBOARD);
	sqlite3_insert_frontboardkey(pdb, pKeyboard);
	sqlite3_close_wrapper(pdb); pdb = NULL;
	return NULL;
}

/*********************************************/
static void *CountdownTimerInfoGet(void *arg)
{
	int i;
	mxml_node_t *tmp=NULL;
	mxmlNode *p=mxmlGetRequest();
	char *ob[] = {"cDeviceId", "cControllerID","cControllerType","nChannelFlag"};
	char buf[1024]={0};
	CountDownCfg *pCountdown  = &g_CountDownCfg;

	if(p == NULL)
	{
		mxmlAddNodeArr(NULL, ob[0], mxmlArray2Str((char*)pCountdown->cDeviceId, MAX_NUM_COUNTDOWN, buf),0);
		for(i=0; i<MAX_NUM_COUNTDOWN; i++)
		{
			mxmlAddNodeArr(NULL, ob[1], mxmlArray2Str((char*)pCountdown->cControllerID[i], MAX_CHANNEL_NUM, buf), i+1);
		}
		mxmlAddNodeArr(NULL, ob[2], mxmlArray2Str((char*)pCountdown->cControllerType, MAX_NUM_COUNTDOWN, buf),0);
		mxmlAddNodeVar(NULL, ob[3], pCountdown->nChannelFlag);
	}
	else
	{
		while(p != NULL)
		{
			if(strcmp(p->ob, ob[0]) == 0)
			{
				mxmlAddNodeArr(NULL, ob[0], mxmlArray2Str((char*)pCountdown->cDeviceId, MAX_NUM_COUNTDOWN, buf),0);			
			}
			else if(strcmp(p->ob, ob[1]) == 0)
			{
				if(p->id1>0 && p->id1<=MAX_NUM_COUNTDOWN)
					mxmlAddNodeArr(NULL, ob[1], mxmlArray2Str((char*)pCountdown->cControllerID[p->id1-1], MAX_CHANNEL_NUM, buf),p->id1);
				else
				{
					for(i=0; i<MAX_NUM_COUNTDOWN; i++)
						mxmlAddNodeArr(NULL, ob[1], mxmlArray2Str((char*)pCountdown->cControllerID[i], MAX_CHANNEL_NUM, buf),i+1);	
				}
			}
			else if(strcmp(p->ob, ob[2]) == 0)
			{
				mxmlAddNodeArr(NULL, ob[2], mxmlArray2Str((char*)pCountdown->cControllerType, MAX_NUM_COUNTDOWN, buf),0);
			}
			else if(strcmp(p->ob, ob[3])==0)
			{
				mxmlAddNodeVar(NULL, ob[3], pCountdown->nChannelFlag);
			}

			p=mxmlGetRequest();
		}
	}
	return NULL;
}
static void *CountdownTimerInfoSet(void *arg)
{
	int i;
	char *p = NULL;	
	char *ob[] = {"cDeviceId", "cControllerID","cControllerType","nChannelFlag"};
	char buf[1024*2]={0}; 
	int len =0;
	sqlite3 *pdb = NULL;
	CountDownCfg *pCountdown  = &g_CountDownCfg;

	if((p=mxmlGetNode(MXML_SIMPLE_ARR, ob[0], 0, 0, 0,NULL,0,0)) != NULL)
	{
		len	= mxmlStr2Array(p, buf);
		memcpy(pCountdown->cDeviceId, buf, (len<MAX_NUM_COUNTDOWN)? len : MAX_NUM_COUNTDOWN);
	}
	for(i=0; i<MAX_NUM_COUNTDOWN; i++)
	{
		if((p=mxmlGetNode(MXML_SIMPLE_ARR, ob[1], i+1, 0, 0,NULL,0,0)) != NULL)
		{
			len	= mxmlStr2Array(p, buf);
			memcpy(pCountdown->cControllerID[i], buf, (len<MAX_CHANNEL_NUM)? len : MAX_CHANNEL_NUM);
		}
	}
	if((p=mxmlGetNode(MXML_SIMPLE_ARR, ob[2], 0, 0, 0, NULL, 0, 0)) != NULL)
	{
		len	= mxmlStr2Array(p, buf);
		memcpy(pCountdown->cControllerType, buf, (len<MAX_NUM_COUNTDOWN)? len : MAX_NUM_COUNTDOWN);
	}
	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[3], 0, 0, 0,NULL,0,0)) != NULL)
	{
		pCountdown->nChannelFlag = atoi(p);	
		//INFO("Countdown set: nChannelFlag: %d\n",atoi(p));
	}
	
	//WRITE_BIN_CFG_PARAMS(FILE_COUNTDOWN_DAT,&g_CountDownCfg,sizeof(CountDownCfg));
	log_debug("insert TABLE_NAME_COUNTDOWN_CFG");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_clear_table(pdb, TABLE_NAME_COUNTDOWN_CFG);
	sqlite3_insert_countdown_cfg(pdb, pCountdown);
	sqlite3_close_wrapper(pdb); pdb = NULL;
	return NULL;
}
static void *CarDetectorInfoGet(void *arg)
{
	char *ob[]={"cCarDetectorType", "address", "subnetMask","gateway"};
	STRU_CAR_DETECT_INFO *pCarDetector = &gStructBinfileConfigPara.stCarDetector;
	mxmlNode *p = mxmlGetRequest();	
	if(p == NULL)
	{
		mxmlAddNodeVar(NULL, ob[0], pCarDetector->cCarDetectorType);
		mxmlAddNodeStr(NULL, ob[1], pCarDetector->stVedioDetIP.address);
		mxmlAddNodeStr(NULL, ob[2], pCarDetector->stVedioDetIP.subnetMask);
		mxmlAddNodeStr(NULL, ob[3], pCarDetector->stVedioDetIP.gateway);
	}
	else
	{
		while(p != NULL)
		{
			if(strcmp(p->ob, ob[0]) == 0)
			{
				mxmlAddNodeVar(NULL, ob[0], pCarDetector->cCarDetectorType);
			}
			else if(strcmp(p->ob, ob[1]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[1], pCarDetector->stVedioDetIP.address);
			}
			else if(strcmp(p->ob, ob[2]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[2], pCarDetector->stVedioDetIP.subnetMask);
			}
			else if(strcmp(p->ob, ob[3]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[3], pCarDetector->stVedioDetIP.gateway);
			}
			p = mxmlGetRequest();
		}
	}
	return NULL;
}
static void *CarDetectorInfoSet(void *arg)
{
	char *p=NULL;
	sqlite3 *pdb = NULL;
	STRU_CAR_DETECT_INFO *pCarDetector = &gStructBinfileConfigPara.stCarDetector;
	char *ob[]={"cCarDetectorType", "address", "subnetMask","gateway"};

	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[0],0,0,0,NULL,0,0)) != NULL)
	{
		pCarDetector->cCarDetectorType = atoi(p);
	}
	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[1],0,0,0,NULL,0,0)) != NULL)
	{
		memset(pCarDetector->stVedioDetIP.address, 0 , sizeof(pCarDetector->stVedioDetIP.address));
		memcpy(pCarDetector->stVedioDetIP.address, p, (strlen(p)>16)? 16 : strlen(p));
	}
	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[2],0,0,0,NULL,0,0)) != NULL)
	{
		memset(pCarDetector->stVedioDetIP.subnetMask, 0 , sizeof(pCarDetector->stVedioDetIP.subnetMask));
		memcpy(pCarDetector->stVedioDetIP.subnetMask, p, (strlen(p)>16)? 16 : strlen(p));
	}
	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[3],0,0,0,NULL,0,0)) != NULL)
	{
		memset(pCarDetector->stVedioDetIP.gateway, 0 , sizeof(pCarDetector->stVedioDetIP.gateway));
		memcpy(pCarDetector->stVedioDetIP.gateway, p, (strlen(p)>16)? 16 : strlen(p));
	}
	
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara, sizeof(STRUCT_BINFILE_CONFIG));
	log_debug("update car detector info");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_car_det_info(pdb, pCarDetector);
	sqlite3_close_wrapper(pdb); pdb = NULL;
	return NULL;	
}

static void *SysUserInfoGet(void *arg)
{
	char *ob[]={"cName","cPasswd"};
	STRU_SYS_USER_INFO *pUser = &gStructBinfileConfigPara.stUser;
	
	mxmlNode *p = mxmlGetRequest();	
	if(p == NULL)
	{
		mxmlAddNodeStr(NULL, ob[0], pUser->cName);
		mxmlAddNodeStr(NULL, ob[1], pUser->cPasswd);
	}
	else
	{
		while(p != NULL)
		{
			if(strcmp(p->ob, ob[0]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[0], pUser->cName);
			}
			else if(strcmp(p->ob, ob[1]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[1], pUser->cPasswd);
			}
			p=mxmlGetRequest();
		}
	}
	return NULL;
}
static void *SysUserInfoSet(void *arg)
{
	char *p=NULL;
	sqlite3 *pdb = NULL;
	char *ob[]={"cName","cPasswd"};
	STRU_SYS_USER_INFO *pUser = &gStructBinfileConfigPara.stUser;

	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[0], 0, 0, 0,NULL,0,0)) != NULL)
	{
		memset(pUser->cName, 0, sizeof(pUser->cName));
		strncpy(pUser->cName, p, (strlen(p)>32)? 32 : strlen(p));
	}
	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[1], 0, 0, 0,NULL,0,0)) != NULL)
	{
		memset(pUser->cPasswd, 0, sizeof(pUser->cPasswd));
		strncpy(pUser->cPasswd, p, (strlen(p)>32)? 32 : strlen(p));
	}
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara, sizeof(STRUCT_BINFILE_CONFIG));
	log_debug("update uesr info");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_user_info(pdb, pUser);
	sqlite3_close_wrapper(pdb); pdb = NULL;
	return NULL;
}

static void *WifiInfoGet(void *arg)
{
	char *ob[]={"cSSID","cPSK"};
	STRU_WIFI_INFO *pWifi = &gStructBinfileConfigPara.stWifi;
	
	mxmlNode *p = mxmlGetRequest();	
	if(p == NULL)
	{
		mxmlAddNodeStr(NULL, ob[0], pWifi->cSSID);
		mxmlAddNodeStr(NULL, ob[1], pWifi->cPSK);
	}
	else
	{
		while(p != NULL)
		{
			if(strcmp(p->ob, ob[0]) ==  0)
			{
				mxmlAddNodeStr(NULL, ob[0], pWifi->cSSID);
			}
			else if(strcmp(p->ob, ob[1]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[1], pWifi->cPSK);
			}
			p=mxmlGetRequest();
		}
	}
	return NULL;
}
static void *WifiInfoSet(void *arg)
{
	char *p=NULL;
	sqlite3 *pdb = NULL;
	char *ob[]={"cSSID","cPSK"};
	STRU_WIFI_INFO *pWifi = &gStructBinfileConfigPara.stWifi;

	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[0], 0,0,0,NULL,0,0)) != NULL)
	{
		memset(pWifi->cSSID, 0, sizeof(pWifi->cSSID));
		strncpy(pWifi->cSSID, p, (strlen(p)>32)? 32 : strlen(p));
	}
	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[1], 0,0,0,NULL,0,0)) != NULL)
	{
		memset(pWifi->cPSK, 0, sizeof(pWifi->cPSK));
		strncpy(pWifi->cPSK, p, (strlen(p)>32)? 32 : strlen(p));
	}
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara, sizeof(STRUCT_BINFILE_CONFIG));
	log_debug("update wifi info");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_wifi_info(pdb, pWifi);
	sqlite3_close_wrapper(pdb); pdb = NULL;
	return NULL;
}

static void *DeviceInfoGet(void *arg)
{
	char *ob[]={"uDevID","cDevDesc"};
	STRU_DEVICE_INFO *pDev = &gStructBinfileConfigPara.stDevice;
		
	mxmlNode *p = mxmlGetRequest();	
	if(p == NULL)
	{
		mxmlAddNodeVar(NULL, ob[0], pDev->uDevID);
		mxmlAddNodeStr(NULL, ob[1], pDev->cDevDesc);
	}
	else
	{
		while(p != NULL)
		{
			if(strcmp(p->ob, ob[0]) == 0)
			{
				mxmlAddNodeVar(NULL, ob[0], pDev->uDevID);
			}
			else if(strcmp(p->ob, ob[1]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[1], pDev->cDevDesc);
			}
			p=mxmlGetRequest();
		}
	}
	return NULL;
}
static void *DeviceInfoSet(void *arg)
{
	char *p=NULL;
	sqlite3 *pdb = NULL;
	char *ob[]={"uDevID","cDevDesc"};
	STRU_DEVICE_INFO *pDev = &gStructBinfileConfigPara.stDevice;

	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[0],0,0,0,NULL,0,0)) != NULL)	
	{
		pDev->uDevID = atoi(p);
	}
	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[1],0,0,0,NULL,0,0)) != NULL)	
	{
		memset(pDev->cDevDesc, 0, sizeof(pDev->cDevDesc));
		strncpy(pDev->cDevDesc, p, (strlen(p)>64)? 64 : strlen(p));
	}
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara, sizeof(STRUCT_BINFILE_CONFIG));
	log_debug("update device info");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_device_info(pdb, pDev);
	sqlite3_close_wrapper(pdb); pdb = NULL;
	return NULL;
}

static void *RGCheckInfoGet(void *arg)
{
	char *ob[]={"iRGSignalSwitch","cMcastAddr","uMcastPort"};
	STRU_RGSIGNAL_CHECK_INFO *pRGchk = &gStructBinfileConfigPara.stRGSignalCheck;
	
	mxmlNode *p=mxmlGetRequest();
	if(p == NULL)
	{
		mxmlAddNodeVar(NULL, ob[0], pRGchk->iRGSignalSwitch);
		mxmlAddNodeStr(NULL, ob[1], pRGchk->cMcastAddr);
		mxmlAddNodeVar(NULL, ob[2], pRGchk->uMcastPort);
	}
	else
	{
		while(p != NULL)
		{
			if(strcmp(p->ob, ob[0]) == 0)
			{
				mxmlAddNodeVar(NULL, ob[0], pRGchk->iRGSignalSwitch);
			}
			else if(strcmp(p->ob, ob[1]) == 0)
			{
				mxmlAddNodeStr(NULL, ob[1], pRGchk->cMcastAddr);
			}
			else if(strcmp(p->ob, ob[2]) == 0)
			{
				mxmlAddNodeVar(NULL, ob[2], pRGchk->uMcastPort);
			}
			p=mxmlGetRequest();
		}
	}
	
	return NULL;
}
static void *RGCheckInfoSet(void *arg)
{
	char *p=NULL;
	sqlite3 *pdb = NULL;
	char *ob[]={"iRGSignalSwitch","cMcastAddr","uMcastPort"};
	STRU_RGSIGNAL_CHECK_INFO *pRGchk = &gStructBinfileConfigPara.stRGSignalCheck;
	
	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[0],0,0,0,NULL,0,0)) != NULL)	
	{
		pRGchk->iRGSignalSwitch = atoi(p);
	}
	if((p=mxmlGetNode(MXML_SIMPLE_STR, ob[1],0,0,0,NULL,0,0)) != NULL)	
	{
		memset(pRGchk->cMcastAddr, 0, sizeof(pRGchk->cMcastAddr));
		strncpy(pRGchk->cMcastAddr, p, (strlen(p)>16)? 16 : strlen(p));
	}
	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[2],0,0,0,NULL,0,0)) != NULL)	
	{
		pRGchk->uMcastPort = atoi(p);
	}
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara, sizeof(STRUCT_BINFILE_CONFIG));
	log_debug("update RGSignal info");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_rgsig_info(pdb, pRGchk);
	sqlite3_close_wrapper(pdb); pdb = NULL;

	return NULL;
}
static void *CameraInfoSet(void *arg)
{
	char *p=NULL;
	sqlite3 *pdb = NULL;
	char *ob[]={"isCameraKakou"};

	if((p=mxmlGetNode(MXML_SIMPLE_VAR, ob[0],0,0,0,NULL,0,0)) != NULL)	
	{
		int temp = atoi(p);
		if(temp >=0 && temp <=1)
			gStructBinfileConfigPara.sSpecialParams.isCameraKakou = temp;
	}
	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara, sizeof(STRUCT_BINFILE_CONFIG));
	INFO("update sSpecialParams.isCameraKakou ");
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_column(pdb, TABLE_NAME_CONF_SPECIALPRM, "isCameraKakou", 1, &(gStructBinfileConfigPara.sSpecialParams.isCameraKakou), 1, SQLITE_INTEGER);
	sqlite3_close_wrapper(pdb); pdb = NULL;
	return NULL;
}
static void *CameraInfoGet(void *arg)
{
	char *ob[]={"isCameraKakou"};
	
	mxmlNode *p=mxmlGetRequest();
	if(p == NULL)
	{
		mxmlAddNodeVar(NULL, ob[0], gStructBinfileConfigPara.sSpecialParams.isCameraKakou);
	}
	else
	{
		while(p != NULL)
		{
			if(strcmp(p->ob, ob[0]) == 0)
			{
				mxmlAddNodeVar(NULL, ob[0], gStructBinfileConfigPara.sSpecialParams.isCameraKakou);
			}
			p=mxmlGetRequest();
		}
	}
	
	return NULL;	
}


