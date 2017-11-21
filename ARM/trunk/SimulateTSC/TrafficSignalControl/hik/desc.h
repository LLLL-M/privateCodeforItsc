#ifndef __DESC_H__
#define __DESC_H__

#include <cstring>
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "common.h"
#include "sqlite3.h"
#include "sqlite_conf.h"



class Descfile
{
private:
	typedef UINT8 PhaseDescText[64];
public:
	/*
	PHASE_DESC_PARAMS 		sPhaseDescParams;	//相位描述 
	CHANNEL_DESC_PARAMS		sChannelDescParams;	//通道描述
	PATTERN_NAME_PARAMS 	sPatternNameParams;	//方案描述
	PLAN_NAME_PARAMS 		sPlanNameParams;	//计划描述
	DATE_NAME_PARAMS 		sDateNameParams;	//日期描述
    PhaseDescText   		phaseDescText[16][16]; //扩展相位描述
    */
    STRUCT_BINFILE_DESC desc;
	Sqliteconf *psqlite_conf;
	unsigned int iIsSaveDescParams;
	
    Descfile(Sqliteconf* psqliteconf);
    void read_desc_from_database();
    void save_desc_to_database();


	void DownloadPhasedesc(STRU_UDP_INFO* udp_info);
	void UploadPhasedesc(STRU_UDP_INFO* udp_info);
	void DownloadChanneldesc(STRU_UDP_INFO* udp_info);
	void UploadChanneldesc(STRU_UDP_INFO* udp_info);
	void DownloadPlandesc(STRU_UDP_INFO* udp_info);
	void UploadPlandesc(STRU_UDP_INFO* udp_info);
	void DownloadSchemedesc(STRU_UDP_INFO* udp_info);
	void UploadSchemedesc(STRU_UDP_INFO* udp_info);
	void DownloadDatedesc(STRU_UDP_INFO* udp_info);
	void UploadDatedesc(STRU_UDP_INFO* udp_info);

	void SaveDescUpdate();
	
};




#endif
