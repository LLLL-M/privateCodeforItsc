#ifndef __HIKCONF_H__
#define __HIKCONF_H__

#include "HikConfig.h"
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "common.h"
#include "sqlite3.h"
#include "sqlite_conf.h"
#include "hiktsc.h"
#include "file.h"
#include "lock.h"


class Hikconf
{
private:

    //SignalControllerPara *gRunConfigPara;		//信号机运行的配置参数
	HikFile::File file;
	RWlock rwlock;
	Sqliteconf* psqlite_conf;
	Hiktsc* pHiktsc;
	unsigned int iIsSaveHikconfig;
	
	void StoreTimeInterval(void *arg);
	void StorePlanSchedule(void *arg);
	void StoreUnit(void *arg);
	void StorePhase(void *arg);
	void StoreTransform(void *arg);
	void StorePhaseTurn(void *arg);
	void StoreSplit(void *arg);
	void StoreChannel(void *arg);
	void StoreOldScheme(void *arg);
	void StoreScheme(void *arg);
	void StoreAction(void *arg);
	void StoreVehicleDetector(void *arg);
	void StorePedestrianDetector(void *arg);
	void StoreFollowPhase(void *arg);
public:
    SignalControllerPara *gSignalControlpara;	//信号机上下载的配置参数

	Hikconf(Sqliteconf* psqliteconf, Hiktsc* phiktsc);
    ~Hikconf();

	void LoadLocalConfigFile();
	void WriteLoaclConfigFile();
	void StoreBegin(UInt32 flag);
	void DownloadConfig(UInt32 type, void *arg);
	int DownloadExtendConfig(struct STRU_Extra_Param_Response *response);
	int UploadConfig(struct STRU_Extra_Param_Response *response);
	void save_hikconfig_to_database();
	
protected:
};

#endif
