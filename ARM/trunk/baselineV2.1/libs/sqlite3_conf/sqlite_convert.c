#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "hik.h"
#include "HikConfig.h"
#include "common.h"
#include "platform.h"
#include "configureManagement.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

#define DATABASE_HIKCONFIG "/home/hikconfig.db"
#define DATABASE_CONF_CUSTOM "/home/extconfig.db"


//config.dat
typedef struct
{
	STRU_SPECIAL_PARAMS sSpecialParams;											//特殊参数定义结构体
	STRU_WIFI_INFO				stWifi;											//Wifi配置信息
	STRU_DEVICE_INFO			stDevice;										//设备信息
	STRU_RGSIGNAL_CHECK_INFO	stRGSignalCheck;								//红绿信号检测
	UINT8				cReservedValue1[512 - 4 -64 -68 -24-1];					//针对特殊参数定义结构体的保留字节,减去4是增加了一个相位接管开关
																				// -wifi(64)	-device(68) - rgsCheck(24)-kakou(1)
	UINT8				cCarDetectSwitch;										//车检板开关, 建议使用下面的车检器配置
	UINT8 				cPrintfLogSwitch;										//日志打印开关
	UINT32				cFailureNumber;											//错误序列号
	STRU_CURRENT_PARAMS	sCurrentParams[32];										//电流参数表
	STRU_WIRELESS_CONTROLLER_INFO	stWirelessController;						//无线控制器模块参数
	STRU_CAR_DETECT_INFO	stCarDetector;										//车检器
	STRU_SYS_USER_INFO		stUser;												//登录用户信息
	STRU_FRONTBOARD_KEY_INFO	sFrontBoardKeys;									//前面板按键参数
}STRUCT_BINFILE_CONFIG_OLD;

//custom.dat
typedef struct
{
	STRU_Count_Down_Params	sCountdownParams;									//针对倒计时牌的配置
	UINT8                   cIsCountdownValueLimit;                             //倒计时值是否受感应检测时间的限制，默认是不受限的.
	UINT8 					cReservedValue1[512 - 8 - 1];	//保留字节,减去8是因为增加黄灯、红灯闪烁、倒计时是否受限的功能
	COM_PARAMS 				sComParams[4];										//针对串口参数的配置
	UINT8 					cReservedValue2[512];
	CHANNEL_LOCK_PARAMS 	sChannelLockedParams;								//针对通道锁定参数的配置
	DemotionParams 			demotionParams;		//降级的相关参数
	UINT8 					cReservedValue3[512 - sizeof(DemotionParams)];
	UINT8					cChannelLockFlag;									//通道锁定标识//0表示未锁定，1表示锁定,2表示待锁定（待锁定状态表示收到了通道锁定命令但是当前时间为非锁定时间段的状态）
	UINT8					cSpecialControlSchemeId;							//特殊控制方案号
	STU_MUL_PERIODS_CHAN_PARAMS	sMulPeriodsChanLockParams;						//多时段通道锁定参数
}STRUCT_BINFILE_CUSTOM_OLD;

typedef UINT8 PhaseDescText_old[64];
//desc.dat
typedef struct
{
	PHASE_DESC_PARAMS 		sPhaseDescParams;									//相位描述 
	UINT8 					cReservedValue1[512];
	CHANNEL_DESC_PARAMS		sChannelDescParams;   								//通道描述
	UINT8 					cReservedValue2[512];
	PATTERN_NAME_PARAMS 	sPatternNameParams;   								//方案描述
	UINT8 					cReservedValue3[512];
	PLAN_NAME_PARAMS 		sPlanNameParams; 							        //计划描述
	UINT8 					cReservedValue4[512];
	DATE_NAME_PARAMS 		sDateNameParams;							        //日期描述
	UINT8				    cReservedValue6[512];
	//added by Jicky
    PhaseDescText_old   		phaseDescText[16][16];     //16个相位表中每个相位的描述信息，每个相位最多支持32个中文
}STRUCT_BINFILE_DESC_OLD;

//misc.dat
typedef struct
{
    UINT8                   cIsCanRestartHiktscAllowed;                         //是否运行CAN监控线程重启hikTSC程序
    unsigned int            time_zone_gap;                                      //平台或配置工具校时时发的时区差，以秒为单位 
}STRUCT_BINFILE_MISC_OLD;

typedef struct
{
    unsigned char cDeviceId[MAX_NUM_COUNTDOWN];                          //倒计时ID，通过操作倒计时面板上的两个按键来设置倒计时的ID，分别是0 1 2 3 ...,数组下标做ID号
    unsigned char cControllerID[MAX_NUM_COUNTDOWN][MAX_CHANNEL_NUM];      //该倒计时对应显示的控制源，一个倒计时可以显示不止一个控制源(以前是相位，现在是通道)，
                                                                        //比如说可以同时显示一个方向的直行和左转倒计时信息,如果控制源有多个，在配置文件中 以逗号隔开，
                                                                        //连续存放，有几个显示几个,通道号从1开始。
    unsigned char cControllerType[MAX_NUM_COUNTDOWN];                   //控制源的类型, 可以取ControllerType里面的值。注意暂时我们只考虑机动车、行人、跟随三种控制类型
    unsigned int  nChannelFlag;                                         //脉冲倒计时通道关闭标志，置1为脉冲，置0为自学习
                                                                        //每一位代表一个倒计时通道。
    unsigned char cReservedValue1[508];                                 //值分别是其他1、机动车2、行人3、跟随4，
    
}CountDownCfg_OLD; 

//const char* database_hikconfig = "/home/hik_config.db";/*sqlite 数据库文件*/
//const char* database_conf_custom = "/home/conf_custom.db";

sqlite3* gdb;


void compatible_v1(SignalControllerPara *pscp)
{
	int i = 0;
	ChannelItem zero;
	memset(&zero, 0, sizeof(zero));
		if (memcmp(&zero, &pscp->stChannel[0][0], sizeof(zero)) == 0)
		{	//说明是第一次沿用之前老的配置
			for (i = 0; i < NUM_ACTION; i++)
			{
				if (pscp->stAction[i].nPhaseTableID == 0)
					pscp->stAction[i].nPhaseTableID = 1;
				if (pscp->stAction[i].nChannelTableID == 0)
					pscp->stAction[i].nChannelTableID = 1;
			}
			memcpy(pscp->stPhase[0], pscp->stOldPhase, sizeof(pscp->stOldPhase));
			memcpy(pscp->AscSignalTransTable[0], pscp->OldAscSignalTransTable, sizeof(pscp->OldAscSignalTransTable));
			memcpy(pscp->stChannel[0], pscp->stOldChannel, sizeof(pscp->stOldChannel));
			memcpy(pscp->stFollowPhase[0], pscp->stOldFollowPhase, sizeof(pscp->stOldFollowPhase));
		}
		if (pscp->stScheme[0].nCycleTime == 0)
		{
			for (i = 0; i < NUM_SCHEME; i++)
			{
				pscp->stScheme[i].nSchemeID = pscp->stOldScheme[i].nSchemeID;
				pscp->stScheme[i].nCycleTime = pscp->stOldScheme[i].nCycleTime;
				pscp->stScheme[i].nOffset = pscp->stOldScheme[i].nOffset;
				pscp->stScheme[i].nGreenSignalRatioID = pscp->stOldScheme[i].nGreenSignalRatioID;
				pscp->stScheme[i].nPhaseTurnID = pscp->stOldScheme[i].nPhaseTurnID;
			}
		}

}

int read_binfile(const char* filename, void* buf, unsigned int size)
{
	int fd = -1;
	int ret = 0;
	
	fd = open(filename,O_RDONLY);
	if(-1 == fd)
	{
		ERR("open %s failed : %s .\n", filename, strerror(errno));
		return -1;
	}
	ret = read(fd, buf, size);
	if(ret < 0)
	{
		ERR("read %s failed : %s .\n", filename, strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);
	return ret;
}

void convert_config(STRUCT_BINFILE_CONFIG *config, STRUCT_BINFILE_CONFIG_OLD* config_old)
{
	memset(config, 0, sizeof(STRUCT_BINFILE_CONFIG));
	memcpy(&config->sSpecialParams, &config_old->sSpecialParams, sizeof(config_old->sSpecialParams));
	memcpy(&config->stWifi, &config_old->stWifi, sizeof(config_old->stWifi));
	memcpy(&config->stDevice, &config_old->stDevice, sizeof(config_old->stDevice));
	memcpy(&config->stRGSignalCheck, &config_old->stRGSignalCheck, sizeof(config_old->stRGSignalCheck));
	memcpy(config->sCurrentParams, config_old->sCurrentParams, sizeof(config_old->sCurrentParams));
	memcpy(&config->stWirelessController, &config_old->stWirelessController, sizeof(config_old->stWirelessController));
	memcpy(&config->stCarDetector, &config_old->stCarDetector, sizeof(config_old->stCarDetector));
	memcpy(&config->stUser, &config_old->stUser, sizeof(config_old->stUser));
	memcpy(&config->sFrontBoardKeys, &config_old->sFrontBoardKeys, sizeof(config_old->sFrontBoardKeys));
	config->cCarDetectSwitch = config_old->cCarDetectSwitch;
	config->cPrintfLogSwitch = config_old->cPrintfLogSwitch;
	config->cFailureNumber = config_old->cFailureNumber;
}
void convert_custom(STRUCT_BINFILE_CUSTOM* custom, STRUCT_BINFILE_CUSTOM_OLD* custom_old)
{
	memset(custom, 0, sizeof(STRUCT_BINFILE_CUSTOM));
	memcpy(&custom->sCountdownParams, &custom_old->sCountdownParams, sizeof(custom_old->sCountdownParams));
	memcpy(custom->sComParams, custom_old->sComParams, sizeof(custom_old->sComParams));
	memcpy(&custom->sChannelLockedParams, &custom_old->sChannelLockedParams, sizeof(custom_old->sChannelLockedParams));
	memcpy(&custom->demotionParams, &custom_old->demotionParams, sizeof(custom_old->demotionParams));
	memcpy(&custom->sMulPeriodsChanLockParams, &custom_old->sMulPeriodsChanLockParams, sizeof(custom_old->sMulPeriodsChanLockParams));
	custom->cChannelLockFlag = custom_old->cChannelLockFlag;
	custom->cIsCountdownValueLimit = custom_old->cIsCountdownValueLimit;
	custom->cSpecialControlSchemeId = custom_old->cSpecialControlSchemeId;
}
void convert_desc(STRUCT_BINFILE_DESC* desc, STRUCT_BINFILE_DESC_OLD* desc_old)
{
	memset(desc, 0, sizeof(STRUCT_BINFILE_DESC));
	memcpy(&desc->sPhaseDescParams, &desc_old->sPhaseDescParams, sizeof(desc_old->sPhaseDescParams));
	memcpy(&desc->sChannelDescParams, &desc_old->sChannelDescParams, sizeof(desc_old->sChannelDescParams));
	memcpy(&desc->sPatternNameParams, &desc_old->sPatternNameParams, sizeof(desc_old->sPatternNameParams));
	memcpy(&desc->sPlanNameParams, &desc_old->sPlanNameParams, sizeof(desc_old->sPlanNameParams));
	memcpy(&desc->sDateNameParams, &desc_old->sDateNameParams, sizeof(desc_old->sDateNameParams));
	memcpy(desc->phaseDescText, desc_old->phaseDescText, sizeof(desc_old->phaseDescText));
}
void convert_misc(STRUCT_BINFILE_MISC* misc, STRUCT_BINFILE_MISC_OLD* misc_old)
{
	memset(misc, 0, sizeof(STRUCT_BINFILE_MISC));
	misc->cIsCanRestartHiktscAllowed = misc_old->cIsCanRestartHiktscAllowed;
	misc->time_zone_gap = misc->time_zone_gap;
}
void convert_cdc(CountDownCfg* cdc, CountDownCfg_OLD* cdc_old)
{
	memset(cdc, 0, sizeof(CountDownCfg));
	memcpy(cdc->cDeviceId, cdc_old->cDeviceId, sizeof(cdc_old->cDeviceId));
	memcpy(cdc->cControllerID, cdc_old->cControllerID, sizeof(cdc_old->cControllerID));
	memcpy(cdc->cControllerType, cdc_old->cControllerType, sizeof(cdc_old->cControllerType));
	cdc->nChannelFlag = cdc_old->nChannelFlag;
}

static int IsItemInArray(unsigned short *array,int length,int val)
{
	if(!array)
	{
		return 0;
	}

	int i = 0;
	for(i = 0 ; i < length ; i++)
	{
		if(array[i] == val)
		{
			return 1;
		}

	}

	return 0;
}
static void CopyPhaseArray(SignalControllerPara *pSignalControlpara,unsigned short *nPhaseArray,int nPhaseTableNo)
{
    int i = 0;    

    if(nPhaseTableNo <= 0 || nPhaseTableNo >= MAX_PHASE_TABLE_COUNT)
    {
        return;
    }

    for(i = 0; i < NUM_PHASE; i++)
    {
        nPhaseArray[i] = pSignalControlpara->stPhase[nPhaseTableNo - 1][i].nPhaseID;
    }
}
static void CopyFollowPhaseArray(SignalControllerPara *pSignalControlpara,unsigned short *nPhaseArray,int nPhaseTableNo)
{
    int i = 0;    
    if(nPhaseTableNo <= 0 || nPhaseTableNo >= MAX_FOLLOW_PHASE_TABLE_COUNT)
    {
        return;
    }
    
    for(i = 0; i < NUM_FOLLOW_PHASE; i++)
    {
        nPhaseArray[i] = pSignalControlpara->stFollowPhase[nPhaseTableNo - 1][i].nFollowPhaseID;
    }
}
static int MakeSureChannelLegal(SignalControllerPara *pSignalControlpara,unsigned short *nPhaseArray,unsigned short *nFollowPhaseArray,int nChannelNo)
{
    int i = 0;
    int sTemp = 0;

    if(nChannelNo <= 0 || nChannelNo >= MAX_CHANNEL_TABLE_COUNT)
    {
        //log_error_cn("nChannelNo 不合法，越界. \n",nChannelNo);
        return ERROR_ID_LEGAL_CHANNEL;
    }
    
    for(i = 0 ; i < NUM_CHANNEL ; i++)
    {
        sTemp = pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerID ;
        if(sTemp == 0)
        {
            continue;
        }

        if((pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType == MOTOR ) || (pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType == PEDESTRIAN))
        {
            if(IsItemInArray(nPhaseArray,NUM_PHASE,sTemp) == 0)
            {
            	pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType = 0;//set controller type 0(unused)
                //log_error_cn("ErrorCode: ERROR_NOT_EXIST_SOURCE_CHANNEL , Error Content: 通道%d的控制源%d不存在. \n",i+1,sTemp);
                //return ERROR_NOT_EXIST_SOURCE_CHANNEL;
            }
        }
        else if (pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType == FOLLOW)
        {
            if(IsItemInArray(nFollowPhaseArray,NUM_FOLLOW_PHASE,sTemp) == 0)
            {
            	pSignalControlpara->stChannel[nChannelNo - 1][i].nControllerType = 0;//set controller type 0(unused)
                //log_error_cn("ErrorCode: ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL , Error Content: 通道%d的跟随控制源%d不存在. \n",i+1,sTemp);
                //return ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL;
            }
        }
    }

    return 0;
}


static int CheckChannelControllerType(SignalControllerPara *pSignalControlpara)
{
	int nActionId = 0;
	int  nSchemeId = 0;
	int  nPhaseTableNo = 0;
    int  nChannelTableNo = 0;
    int  nRet = 0;
    unsigned short nPhaseArray[NUM_PHASE] = {0};//总共的相位数组
    unsigned short nFollowPhaseArray[NUM_FOLLOW_PHASE] = {0};//跟随相位数组

	for (nActionId = 0; nActionId < NUM_ACTION; nActionId++)
	{
		nSchemeId =  pSignalControlpara->stAction[nActionId - 1].nSchemeID;
		if (nSchemeId == 0)
			continue;
		nPhaseTableNo = pSignalControlpara->stAction[nActionId - 1].nPhaseTableID;
		nChannelTableNo = pSignalControlpara->stAction[nActionId - 1].nChannelTableID;
		CopyPhaseArray(pSignalControlpara,nPhaseArray,nPhaseTableNo);
    	CopyFollowPhaseArray(pSignalControlpara,nFollowPhaseArray,nPhaseTableNo);
    	MakeSureChannelLegal(pSignalControlpara,nPhaseArray,nFollowPhaseArray,nChannelTableNo);
	}
    
    return  0;
}


int main(int argc, char* argv[])
{
	int ret = 0;
	STRUCT_BINFILE_CONFIG_OLD BinfileConfigPara_old;
	STRUCT_BINFILE_CUSTOM_OLD BinfileCustom_old;
	STRUCT_BINFILE_DESC_OLD BinfileDesc_old;
	STRUCT_BINFILE_MISC_OLD BinfileMisc_old;
	CountDownCfg_OLD countdown_cfg_old;

	STRUCT_BINFILE_CONFIG BinfileConfigPara;
	STRUCT_BINFILE_CUSTOM BinfileCustom;
	STRUCT_BINFILE_DESC BinfileDesc;
	STRUCT_BINFILE_MISC BinfileMisc;
	CountDownCfg countdown_cfg;
	
	SignalControllerPara scp;
	int i = 0;
	int j = 0;
	char* config_file[] = {"hikconfig.dat",
							"config.dat",
							"custom.dat",
							"desc.dat",
							"misc.dat",
							"countdown.dat"};
	init_db_file_tables();
	if (argc < 2)
	{
		FPRINTF(stderr, "params invalid, usage: sqlite_conf (filename), the filename can be config.dat, custom.dat, desc.dat, hikconfig.dat, countdown.dat\n");
		return -1;
	}
	
	for (j = 1; j < argc; j++)
	{
		if (IS_FILE_EXIST(argv[i]) == 0)
			continue;
		for (i = 0; i < 6; i++)
		{
			if (strstr(argv[j], config_file[i]) != NULL)
			{
				break;
			}
		}
		if (i < 6)
			FPRINTF(stderr, "convert %s to database\n", argv[j]);
		else
		{
			FPRINTF(stderr, "input file name invalid, you can input file name like: \
				config.dat\
				custom.dat\
				desc.dat\
				misc.dat\
				hikconfig.dat\
				countdown.dat\n");
				continue;
		}
		switch (i)
		{
			case 0:
				sqlite3_open_wrapper(DATABASE_HIKCONFIG, &gdb);
				create_hikconfig_tables(gdb);
				read_binfile(argv[j], &scp, sizeof(SignalControllerPara));
				compatible_v1(&scp);
				CheckChannelControllerType(&scp);
				write_hikconfig(gdb, &scp);
				break;
			case 1:
				sqlite3_open_wrapper(DATABASE_EXTCONFIG, &gdb);
				create_config_tables(gdb);
				read_binfile(argv[j], &BinfileConfigPara_old, sizeof(STRUCT_BINFILE_CONFIG_OLD));
				convert_config(&BinfileConfigPara, &BinfileConfigPara_old);
				write_config(gdb, &BinfileConfigPara);
				break;
			case 2:
				sqlite3_open_wrapper(DATABASE_EXTCONFIG, &gdb);
				create_custom_tables(gdb);
				read_binfile(argv[j], &BinfileCustom_old, sizeof(STRUCT_BINFILE_CUSTOM_OLD));
				convert_custom(&BinfileCustom, &BinfileCustom_old);
				write_custom(gdb, &BinfileCustom);
				break;
			case 3:
				sqlite3_open_wrapper(DATABASE_EXTCONFIG, &gdb);
				create_desc_tables(gdb);
				read_binfile(argv[j], &BinfileDesc_old, sizeof(STRUCT_BINFILE_DESC_OLD));
				convert_desc(&BinfileDesc, &BinfileDesc_old);
				write_desc(gdb, &BinfileDesc);
				break;
			case 4:
				sqlite3_open_wrapper(DATABASE_EXTCONFIG, &gdb);
				create_misc_tables(gdb);
				read_binfile(argv[j], &BinfileMisc_old, sizeof(STRUCT_BINFILE_MISC_OLD));
				convert_misc(&BinfileMisc, &BinfileMisc_old);
				write_misc(gdb, &BinfileMisc);
				break;
			case 5:
				sqlite3_open_wrapper(DATABASE_EXTCONFIG, &gdb);
				create_countdown_cfg_tables(gdb);
				read_binfile(argv[j], &countdown_cfg_old, sizeof(CountDownCfg_OLD));
				convert_cdc(&countdown_cfg, &countdown_cfg_old);
				sqlite3_insert_countdown_cfg(gdb, &countdown_cfg);
				break;
			default:
				break;
		}
		sqlite3_close_wrapper(gdb);
		gdb = NULL;
	}
	
	return 0;
}
