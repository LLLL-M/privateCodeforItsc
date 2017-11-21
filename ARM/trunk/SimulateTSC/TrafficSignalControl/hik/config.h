#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <cstring>
#include "sqlite_conf.h"
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "common.h"
#include "sqlite3.h"




class Configfile
{
private:
	
public:
	/*
	STRU_SPECIAL_PARAMS sSpecialParams;		//特殊参数定义结构体
	STRU_WIFI_INFO				stWifi;		//Wifi配置信息
	STRU_DEVICE_INFO			stDevice;	//设备信息
	STRU_RGSIGNAL_CHECK_INFO	stRGSignalCheck;	//红绿信号检测
	UINT8	cCarDetectSwitch;	//车检板开关,建议使用下面的车检器配置
	UINT8	cPrintfLogSwitch;	//日志打印开关
	UINT32	cFailureNumber;		//错误序列号
	STRU_CURRENT_PARAMS	sCurrentParams[32];	//电流参数表
	STRU_WIRELESS_CONTROLLER_INFO	stWirelessController;//无线控制器模块参数
	STRU_CAR_DETECT_INFO	stCarDetector;	//车检器
	STRU_SYS_USER_INFO		stUser;			//登录用户信息
	STRU_FRONTBOARD_KEY_INFO	sFrontBoardKeys; //前面板按键参数
	*/
	STRUCT_BINFILE_CONFIG config;
	STRUCT_BINFILE_MISC misc;
    Sqliteconf* psqlite_conf;
	unsigned int iIsSaveSpecialParams;
	
    Configfile(Sqliteconf *psqliteconf);
    void read_conf_from_database();
    void read_misc_from_database();
    void save_conf_to_database();

    void DownloadSpecialParams(STRU_UDP_INFO* udp_info);
	void UploadSpecialParams(STRU_UDP_INFO* udp_info);
    void DownloadRedCurrentParams(STRU_UDP_INFO* udp_info);
    void UploadRedCurrentParams(STRU_UDP_INFO* udp_info);
	void DownloadSignalMechineType(STRU_UDP_INFO* udp_info);
	void UploadSignalMechineType(STRU_UDP_INFO* udp_info);
	
	void DownloadTimezone(STRU_UDP_INFO* udp_info);
	void UploadTimezone(STRU_UDP_INFO* udp_info);

	void SaveConfigUpdate();
	
};


#endif
