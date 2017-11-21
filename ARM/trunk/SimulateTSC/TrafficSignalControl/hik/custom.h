#ifndef __CUSTOM_H__
#define __CUSTOM_H__

#include <cstring>
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "common.h"
#include "sqlite3.h"
#include "sqlite_conf.h"



class Customfile
{
private:

public:
	/*
	STRU_Count_Down_Params	sCountdownParams;	//针对倒计时牌的配置
	UINT8	cIsCountdownValueLimit;                //倒计时值是否受感应检测时间的限制，默认是不受限的.
	COM_PARAMS 				sComParams[4];		//针对串口参数的配置
	CHANNEL_LOCK_PARAMS 	sChannelLockedParams;//针对通道锁定参数的配置
	DemotionParams 			demotionParams;		//降级的相关参数
	UINT8					cChannelLockFlag;								//通道锁定标识//0表示未锁定，1表示锁定,2表示待锁定（待锁定状态表示收到了通道锁定命令但是当前时间为非锁定时间段的状态）
	
	STU_MUL_PERIODS_CHAN_PARAMS	sMulPeriodsChanLockParams;		//多时段通道锁定参数
	*/
	STRUCT_BINFILE_CUSTOM custom;
	Sqliteconf* psqlite_conf;
	unsigned int iIsSaveCustomParams;
	
    Customfile(Sqliteconf* psqliteconf);
    void read_custom_from_database();
    void save_custom_to_database();

	void DownloadCountdownParams(STRU_UDP_INFO* udp_info);
	void UploadCountdownParams(STRU_UDP_INFO* udp_info);
	void DownloadComParams(STRU_UDP_INFO* udp_info);
	void UploadComParams(STRU_UDP_INFO* udp_info);
	void EnableChannelLock(STRU_UDP_INFO* udp_info);
	void DisableChannelLock(STRU_UDP_INFO* udp_info);
	void DownloadMulChLock(STRU_UDP_INFO* udp_info);
	void UploadMulChLock(STRU_UDP_INFO* udp_info);


	void SaveCustomUpdate();
	
};



#endif
