#ifndef SQLITE_CONF_H__
#define SQLITE_CONF_H__

#include "hik.h"
#include "HikConfig.h"
#include "configureManagement.h"

#define DEBUG_SQLITE 1
#ifdef DEBUG_SQLITE
#define FPRINTF(err_out, fmt, ...) fprintf(err_out, fmt, ## __VA_ARGS__)
#else
#define FPRINTF(err_out, fmt, ...) 
#endif

#define TABLE_NAME_UNIT                             "stUnitPara"
#define TABLE_NAME_PHASE                            "stPhase"        
//#define TABLE_NAME_PHASE_CONCURRENCY                "phase_concurrency"
#define TABLE_NAME_PHASE_TURN                       "stPhaseTurn"
#define TABLE_NAME_GREEN_SPLIT                      "stGreenSignalRation"
#define TABLE_NAME_CHANNEL                          "stChannel"
#define TABLE_NAME_SCHEME                           "stScheme"
#define TABLE_NAME_ACTION                           "stAction"
#define TABLE_NAME_TIME_INTERVAL                    "stTimeInterval"
#define TABLE_NAME_SCHEDULE                         "stPlanSchedule"
#define TABLE_NAME_VEHICLE                          "AscVehicleDetectorTable"
#define TABLE_NAME_FOLLOW_PHASE                     "stFollowPhase"
#define TABLE_NAME_SIGNAL_TRANS						"AscSignalTransTable"


#define	TABLE_NAME_CONF_SPECIALPRM					"config_sSpecialParams"
#define	TABLE_NAME_CONF_WIRELESS					"config_stWirelessController"
#define	TABLE_NAME_CONF_FRONTBOARD					"config_sFrontBoardKeys"
#define	TABLE_NAME_CONF								"config"
#define TABLE_NAME_CONF_SYS_INFOS					"config_sys_infos"

#define	TABLE_NAME_CUSTOM_COUNTDOWN					"custom_sCountdownParams"
#define	TABLE_NAME_CUSTOM_COM						"custom_sComParams"
#define	TABLE_NAME_CUSTOM_CHANNELLOCK				"custom_sChannelLockedParams"
#define	TABLE_NAME_CUSTOM_MULCHANNELLOCK			"custom_sMulPeriodsChanLockParams"

#define	TABLE_NAME_DESC_PHASE						"desc_phaseDescText"
#define	TABLE_NAME_DESC_CHANNEL						"desc_sChannelDescParams"
#define	TABLE_NAME_DESC_PATTERN						"desc_sPatternNameParams"
#define	TABLE_NAME_DESC_PLAN						"desc_sPlanNameParams"
#define	TABLE_NAME_DESC_DATE						"desc_sDateNameParams"

#define TABLE_NAME_MISC								"misc"

#define	TABLE_NAME_COUNTDOWN_CFG					"CountDownCfg"

#define TABLE_NAME_IPINFOS							"IpInfos"

typedef enum
{
	E_TABLE_NAME_UNIT = 0,                             
	E_TABLE_NAME_PHASE,                                
	E_TABLE_NAME_PHASE_TURN,                       
	E_TABLE_NAME_GREEN_SPLIT,                      
	E_TABLE_NAME_CHANNEL,                          
	E_TABLE_NAME_SCHEME,                           
	E_TABLE_NAME_ACTION,                           
	E_TABLE_NAME_TIME_INTERVAL,                   
	E_TABLE_NAME_SCHEDULE,                         
	E_TABLE_NAME_VEHICLE,                          
	E_TABLE_NAME_FOLLOW_PHASE,                     
	E_TABLE_NAME_SIGNAL_TRANS,						
	
	
	E_TABLE_NAME_CONF_SPECIALPRM,					
	E_TABLE_NAME_CONF_WIRELESS,					
	E_TABLE_NAME_CONF_FRONTBOARD,					
	E_TABLE_NAME_CONF,								
 	E_TABLE_NAME_CONF_SYS_INFOS,					
	
	E_TABLE_NAME_CUSTOM_COUNTDOWN,					
	E_TABLE_NAME_CUSTOM_COM,						
	E_TABLE_NAME_CUSTOM_CHANNELLOCK,				
	E_TABLE_NAME_CUSTOM_MULCHANNELLOCK,			
	
	E_TABLE_NAME_DESC_PHASE,						
	E_TABLE_NAME_DESC_CHANNEL,						
	E_TABLE_NAME_DESC_PATTERN,						
	E_TABLE_NAME_DESC_PLAN,						
	E_TABLE_NAME_DESC_DATE,						
	
 	E_TABLE_NAME_MISC,								
	
	E_TABLE_NAME_COUNTDOWN_CFG,					

	E_TABLE_NAME_IPINFOS,
}TableNo;


typedef struct create_table_info
{
	char* name;
	char* sql;
}TableInfo;


int sqlite3_open_wrapper(const char* db_file, sqlite3** ppdb);
int sqlite3_close_wrapper(sqlite3* pdb);
int sqlite3_begin(sqlite3* pdatabase);
int sqlite3_commit(sqlite3* pdatabase);
int sqlite3_insert_blob_column(sqlite3* pdatabase, const char* table, const char* column_name, void* column_value, int value_size);
int sqlite3_update_blob_column(sqlite3* pdatabase, const char* table, const char* column_name, int row_num, void* column_value, int value_size, int offset);
int sqlite3_select_blob_column(sqlite3* pdatabase, const char* table, const char* column_name, int row_num, void* column_value, int value_size, int offset);
int sqlite3_select_column(sqlite3* pdatabase, const char* table, const char* column_name, int row_num, void* column_value, int value_size);
int sqlite3_insert_column(sqlite3* pdatabase, const char* table, const char* column_name, void* column_value, int val_size, int column_type);
int sqlite3_update_column(sqlite3* pdatabase, const char* table_name, const char* column_name, int row_id, void* value, int val_size, int column_type);
/*
int sqlite3_insert_config(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG* pconfig, const char* table_name);
int sqlite3_select_config(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG* pconfig, const char* table_name);
int sqlite3_insert_custom(sqlite3* pdatabase, STRUCT_BINFILE_CUSTOM *pcustom);
int sqlite3_select_custom(sqlite3* pdatabase, STRUCT_BINFILE_CUSTOM *pcustom, const char* table);
int sqlite3_insert_desc(sqlite3* pdatabase, STRUCT_BINFILE_DESC *pdesc);
int sqlite3_select_desc(sqlite3* pdatabase, STRUCT_BINFILE_DESC *pdesc, const char* table);
int sqlite3_insert_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc);
int sqlite3_select_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc, const char* table);
*/

void sqlite3_bak_db(const char* dbfile);
void init_db_file_tables(void);


int sqlite3_insert_specialparams(sqlite3* pdatabase, STRU_SPECIAL_PARAMS* pspecial_param);
int sqlite3_select_specialparams(sqlite3* pdatabase, STRU_SPECIAL_PARAMS* pspecial_param);
int sqlite3_insert_wireless_controller(sqlite3* pdatabase, STRU_WIRELESS_CONTROLLER_INFO* pwireless_ctrl);
int sqlite3_select_wireless_controller(sqlite3* pdatabase, STRU_WIRELESS_CONTROLLER_INFO* pwireless_ctrl);
int sqlite3_insert_frontboardkey(sqlite3* pdatabase, STRU_FRONTBOARD_KEY_INFO* pfbk);
int sqlite3_select_frontboardkey(sqlite3* pdatabase, STRU_FRONTBOARD_KEY_INFO* pfbk);
int sqlite3_insert_config_pieces(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG* pconfig);
int sqlite3_select_config_pieces(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG* pconfig);
int sqlite3_select_current_params(sqlite3* pdatabase, STRU_CURRENT_PARAMS* pcurrent_params);
int sqlite3_insert_config_sys_infos(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG *pconfig);
int sqlite3_insert_netcard_addr(sqlite3* pdatabase, struct STRU_N_IP_ADDRESS *ip);
int sqlite3_select_netcard_addr(sqlite3* pdatabase, struct STRU_N_IP_ADDRESS *ip);
int sqlite3_update_netcard_addr(sqlite3* pdatabase, struct STRU_N_IP_ADDRESS *ip, UINT8 netcard_index);

int read_config(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG *pconfig);
int write_config(sqlite3* pdatabase, STRUCT_BINFILE_CONFIG *pconfig);


int sqlite3_insert_countdown_prm(sqlite3* pdatabase, STRU_Count_Down_Params* pcountdown_param);
int sqlite3_select_countdown_prm(sqlite3* pdatabase, STRU_Count_Down_Params* pcountdown_param);
int sqlite3_insert_com_prm(sqlite3* pdatabase, COM_PARAMS* pcom_param);
int sqlite3_select_com_prm(sqlite3* pdatabase, COM_PARAMS* pcom_param);
int sqlite3_insert_chlock_prm(sqlite3* pdatabase, CHANNEL_LOCK_PARAMS* pchlock_param);
int sqlite3_select_chlock_prm(sqlite3* pdatabase, CHANNEL_LOCK_PARAMS* pchlock_param);
int sqlite3_insert_mp_chlock_prm(sqlite3* pdatabase, STU_MUL_PERIODS_CHAN_PARAMS *pmp_chlock_param);
int sqlite3_select_mp_chlock_prm(sqlite3* pdatabase, STU_MUL_PERIODS_CHAN_PARAMS *pmp_chlock_param);
int read_custom(sqlite3* pdatabase, STRUCT_BINFILE_CUSTOM* pcustom);
int write_custom(sqlite3* pdatabase, STRUCT_BINFILE_CUSTOM* pcustom);


int sqlite3_insert_phase_desc(sqlite3* pdatabase, PhaseDescText phase_desc[16][16]);
int sqlite3_select_phase_desc(sqlite3* pdatabase, PhaseDescText phase_desc[16][16]);
int sqlite3_insert_channel_desc(sqlite3* pdatabase, CHANNEL_DESC_PARAMS *pchannel_desc);
int sqlite3_select_channel_desc(sqlite3* pdatabase, CHANNEL_DESC_PARAMS *pchannel_desc);
int sqlite3_insert_pattern_name_desc(sqlite3* pdatabase, PATTERN_NAME_PARAMS *ppattern_name);
int sqlite3_select_pattern_name_desc(sqlite3* pdatabase, PATTERN_NAME_PARAMS *ppattern_name);
int sqlite3_insert_plan_name(sqlite3* pdatabase, PLAN_NAME_PARAMS *pplan_name);
int sqlite3_select_plan_name(sqlite3* pdatabase, PLAN_NAME_PARAMS *pplan_name);
int sqlite3_insert_plan_date(sqlite3* pdatabase, DATE_NAME_PARAMS *pdate_name);
int sqlite3_select_plan_date(sqlite3* pdatabase, DATE_NAME_PARAMS *pdate_name);
int read_desc(sqlite3* pdatabase, STRUCT_BINFILE_DESC *pdesc);
int write_desc(sqlite3* pdatabase, STRUCT_BINFILE_DESC *pdesc);
int sqlite3_insert_countdown_cfg(sqlite3* pdatabase, CountDownCfg *pcountdown_cfg);
int sqlite3_select_countdown_cfg(sqlite3* pdatabase, CountDownCfg *pcountdown_cfg);
int sqlite3_insert_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc);
int sqlite3_select_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc);
int sqlite3_insert_faultstatus(sqlite3* pdatabase, int* faultstatus);
int sqlite3_select_faultstatus(sqlite3* pdatabase, int* faultstatus);


int read_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc);
int write_misc(sqlite3* pdatabase, STRUCT_BINFILE_MISC *pmisc);

int sqlite3_insert_unit(sqlite3* pdatabase, UnitPara *punit);
int sqlite3_select_unit(sqlite3* pdatabase, UnitPara *punit);
int sqlite3_insert_phase(sqlite3* pdatabase, PhaseItem pphase[][NUM_PHASE]);
int sqlite3_select_phase(sqlite3* pdatabase, PhaseItem pphase[][NUM_PHASE]);
int sqlite3_insert_phase_turn(sqlite3* pdatabase, PhaseTurnItem pphase_turn[][NUM_RING_COUNT]);
int sqlite3_select_phase_turn(sqlite3* pdatabase, PhaseTurnItem pphase_turn[][NUM_RING_COUNT]);
int sqlite3_insert_green_split(sqlite3* pdatabase, GreenSignalRationItem pgreen_split[][NUM_PHASE]);
int sqlite3_select_green_split(sqlite3* pdatabase, GreenSignalRationItem pgreen_split[][NUM_PHASE]);
int sqlite3_insert_channel(sqlite3* pdatabase, ChannelItem pchannel[][NUM_CHANNEL]);
int sqlite3_select_channel(sqlite3* pdatabase, ChannelItem pchannel[][NUM_CHANNEL]);
int sqlite3_insert_scheme(sqlite3* pdatabase, SchemeItem *pscheme);
int sqlite3_select_scheme(sqlite3* pdatabase, SchemeItem *pscheme);
int sqlite3_insert_action(sqlite3* pdatabase, ActionItem *paction);
int sqlite3_select_action(sqlite3* pdatabase, ActionItem *paction);
int sqlite3_insert_timeinterval(sqlite3* pdatabase, TimeIntervalItem ptime_interval[][NUM_TIME_INTERVAL_ID]);
int sqlite3_select_timeinterval(sqlite3* pdatabase, TimeIntervalItem ptime_interval[][NUM_TIME_INTERVAL_ID]);
int sqlite3_insert_schedule(sqlite3* pdatabase, PlanScheduleItem *pschedule);
int sqlite3_select_schedule(sqlite3* pdatabase, PlanScheduleItem *pschedule);
int sqlite3_insert_follow_phase(sqlite3* pdatabase, FollowPhaseItem pfollow_phase[][NUM_FOLLOW_PHASE]);
int sqlite3_select_follow_phase(sqlite3* pdatabase, FollowPhaseItem pfollow_phase[][NUM_FOLLOW_PHASE]);
int sqlite3_insert_vehicle(sqlite3* pdatabase, struct STRU_N_VehicleDetector *pvehicle);
int sqlite3_select_vehicle(sqlite3* pdatabase, struct STRU_N_VehicleDetector *pvehicle);
int sqlite3_insert_signal_trans(sqlite3* pdatabase, struct STRU_SignalTransEntry psignal_trans[][NUM_PHASE]);
int sqlite3_select_signal_trans(sqlite3* pdatabase, struct STRU_SignalTransEntry psignal_trans[][NUM_PHASE]);


void create_config_tables(sqlite3* pdatabase);
void create_custom_tables(sqlite3* pdatabase);
void create_desc_tables(sqlite3* pdatabase);
void create_countdown_cfg_tables(sqlite3* pdatabase);
void create_misc_tables(sqlite3* pdatabase);

int create_hikconfig_tables(sqlite3* pdatabase);
int write_hikconfig(sqlite3* pdatabase, SignalControllerPara* pscp);
int read_hikconfig(sqlite3* pdatabase, SignalControllerPara* pscp);


#endif



