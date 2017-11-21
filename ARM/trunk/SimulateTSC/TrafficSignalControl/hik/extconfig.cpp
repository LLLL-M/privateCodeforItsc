#include <cstring>
#include <time.h>
#include "sqlite_conf.h"
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "common.h"
#include "sqlite3.h"

#include "config.h"
#include "custom.h"
#include "desc.h"


Configfile::Configfile(Sqliteconf *psqliteconf): psqlite_conf(psqliteconf)
{/*
    std::memset(&sSpecialParams, 0, sizeof(sSpecialParams));
    std::memset(&stWifi, 0, sizeof(stWifi));
    std::memset(&stDevice, 0, sizeof(stDevice));
    std::memset(&stRGSignalCheck, 0, sizeof(stRGSignalCheck));
    cCarDetectSwitch = 0;
    cPrintfLogSwitch = 0;
    cFailureNumber = 0;
    std::memset(sCurrentParams, 0, sizeof(sCurrentParams));
    std::memset(&stWirelessController, 0, sizeof(stWirelessController));
    std::memset(&stCarDetector, 0, sizeof(stCarDetector));
    std::memset(&stUser, 0, sizeof(stUser));
    std::memset(&sFrontBoardKeys, 0, sizeof(sFrontBoardKeys));*/
    std::memset(&config, 0, sizeof(config));
    misc.cIsCanRestartHiktscAllowed = 0;
    misc.time_zone_gap = 0;
    iIsSaveSpecialParams = 0;
    read_conf_from_database();
    read_misc_from_database();
}

void Configfile::read_conf_from_database()
{
    sqlite3 *pdatabase = NULL;

    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
    psqlite_conf->read_config(pdatabase, &config);
    psqlite_conf->sqlite3_close_wrapper(pdatabase);
    pdatabase = NULL;

}

void Configfile::read_misc_from_database()
{
    sqlite3 *pdatabase = NULL;

    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
    psqlite_conf->read_misc(pdatabase, &misc);
    psqlite_conf->sqlite3_close_wrapper(pdatabase);
    pdatabase = NULL;

}
void Configfile::save_conf_to_database()
{
    int i = 0;
    sqlite3 *pdatabase = NULL;

    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
    psqlite_conf->sqlite3_begin(pdatabase);
    for (i = E_TABLE_NAME_CONF_SPECIALPRM; i < E_TABLE_NAME_CONF_SYS_INFOS + 1; i++)
    {
        if (GET_BIT(iIsSaveSpecialParams, i) == 1)
        {
            switch(i)
            {
                case E_TABLE_NAME_CONF_SPECIALPRM:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_SPECIALPRM);
                    psqlite_conf->sqlite3_insert_specialparams(pdatabase, &(config.sSpecialParams));
                    break;
                case E_TABLE_NAME_CONF_WIRELESS:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_WIRELESS);
                    psqlite_conf->sqlite3_insert_wireless_controller(pdatabase, &(config.stWirelessController));
                    break;
                case E_TABLE_NAME_CONF_FRONTBOARD:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_FRONTBOARD);
                    psqlite_conf->sqlite3_insert_frontboardkey(pdatabase, &(config.sFrontBoardKeys));
                    break;
                case E_TABLE_NAME_CONF:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CONF);
                    psqlite_conf->sqlite3_insert_config_pieces(pdatabase, &config);
                    break;
                case E_TABLE_NAME_CONF_SYS_INFOS:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_SYS_INFOS);
                    psqlite_conf->sqlite3_insert_config_sys_infos(pdatabase, &config);
                    break;

                default:
                    break;
            }
        }
    }
    psqlite_conf->sqlite3_commit(pdatabase);
    psqlite_conf->sqlite3_close_wrapper(pdatabase);
    iIsSaveSpecialParams = 0;
}

void Configfile::DownloadSpecialParams(STRU_UDP_INFO *udp_info)
{
    STRU_SPECIAL_PARAMS SpecialParams;

    memset(&SpecialParams, 0, sizeof(STRU_SPECIAL_PARAMS));

    SpecialParams.iErrorDetectSwitch = GET_BIT(udp_info->iValue[0], 0);
    SpecialParams.iCurrentAlarmSwitch = GET_BIT(udp_info->iValue[0], 1);
    SpecialParams.iVoltageAlarmSwitch = GET_BIT(udp_info->iValue[0], 2);
    SpecialParams.iCurrentAlarmAndProcessSwitch = GET_BIT(udp_info->iValue[0], 3);
    SpecialParams.iVoltageAlarmAndProcessSwitch = GET_BIT(udp_info->iValue[0], 4);
    SpecialParams.iWatchdogSwitch = GET_BIT(udp_info->iValue[0], 5);
    SpecialParams.iGpsSwitch = GET_BIT(udp_info->iValue[0], 6);
    SpecialParams.iPhaseTakeOverSwtich = GET_BIT(udp_info->iValue[0], 7);

#if defined(__linux__) && defined(__arm__)
    //??????????GPS????
    if((config.sSpecialParams.iWatchdogSwitch == 0) && (SpecialParams.iWatchdogSwitch == 1))
    {
        system("killall -9 watchdog &");
        config.sSpecialParams.iWatchdogSwitch = 1;
    }
    else if((config.sSpecialParams.iWatchdogSwitch == 1) && (SpecialParams.iWatchdogSwitch == 0))
    {
        config.sSpecialParams.iWatchdogSwitch = 0;
    }
#endif
    memcpy(&config.sSpecialParams,&SpecialParams,sizeof(config.sSpecialParams));
    SET_BIT(iIsSaveSpecialParams, E_TABLE_NAME_CONF_SPECIALPRM);

}

void Configfile::UploadSpecialParams(STRU_UDP_INFO* udp_info)
{
    udp_info->iValue[0] = config.sSpecialParams.iErrorDetectSwitch
                    | (config.sSpecialParams.iCurrentAlarmSwitch << 1)
                    | (config.sSpecialParams.iVoltageAlarmSwitch << 2)
                    | (config.sSpecialParams.iCurrentAlarmAndProcessSwitch << 3)
                    | (config.sSpecialParams.iCurrentAlarmAndProcessSwitch << 4)
                    | (config.sSpecialParams.iWatchdogSwitch << 5)
                    | (config.sSpecialParams.iGpsSwitch << 6)
                    | (config.sSpecialParams.iPhaseTakeOverSwtich << 7);
}

void Configfile::UploadRedCurrentParams(STRU_UDP_INFO* udp_info)
{
    memcpy(((STRU_CURRENT_PARAMS_UDP*)udp_info)->struRecCurrent, config.sCurrentParams, sizeof(config.sCurrentParams));
}
void Configfile::DownloadRedCurrentParams(STRU_UDP_INFO* udp_info)
{
    memcpy(&config.sCurrentParams, ((STRU_CURRENT_PARAMS_UDP*)udp_info)->struRecCurrent, sizeof(config.sCurrentParams));
    SET_BIT(iIsSaveSpecialParams, E_TABLE_NAME_CONF);
}

void Configfile::DownloadTimezone(STRU_UDP_INFO* udp_info)
{
    sqlite3* pdb = NULL;

    misc.time_zone_gap = ((SAdjustTime*)(udp_info))->unTimeZone;
    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
    psqlite_conf->sqlite3_update_column(pdb, TABLE_NAME_MISC, "time_zone_gap", 1, &(misc.time_zone_gap),  4, SQLITE_INTEGER);
    psqlite_conf->sqlite3_close_wrapper(pdb);
    pdb = NULL;
}
void Configfile::UploadTimezone(STRU_UDP_INFO* udp_info)
{
    SAdjustTime* timep = (SAdjustTime*)udp_info;
    timep->ulGlobalTime = time(NULL) - misc.time_zone_gap;
    timep->unTimeZone = misc.time_zone_gap;
}

void Configfile::DownloadSignalMechineType(STRU_UDP_INFO* udp_info)
{
    config.sSpecialParams.iSignalMachineType = udp_info->iValue[0];
    SET_BIT(iIsSaveSpecialParams, E_TABLE_NAME_CONF_SPECIALPRM);
}

void Configfile::UploadSignalMechineType(STRU_UDP_INFO* udp_info)
{
    udp_info->iValue[0] = config.sSpecialParams.iSignalMachineType;
}

void Configfile::SaveConfigUpdate()
{
    if(iIsSaveSpecialParams == 0)
    {
        return;
    }
    //log_debug("write config to database");
    save_conf_to_database();
    iIsSaveSpecialParams = 0;
}


Customfile::Customfile(Sqliteconf* psqliteconf): psqlite_conf(psqliteconf)
{
/*
    std::memset(&sCountdownParams, 0, sizeof(sCountdownParams));
    cIsCountdownValueLimit = 0;
    std::memset(sComParams, 0, sizeof(sComParams));
    std::memset(&sChannelLockedParams, 0, sizeof(sChannelLockedParams));
    std::memset(&demotionParams, 0, sizeof(demotionParams));
    cChannelLockFlag = 0;
    std::memset(&sMulPeriodsChanLockParams, 0, sizeof(sMulPeriodsChanLockParams));
    */
    std::memset(&custom, 0, sizeof(custom));
    iIsSaveCustomParams = 0;
    read_custom_from_database();
}

void Customfile::read_custom_from_database()
{
    sqlite3 *pdatabase = NULL;

    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
    psqlite_conf->read_custom(pdatabase, &custom);
    psqlite_conf->sqlite3_close_wrapper(pdatabase);
    pdatabase = NULL;

}

void Customfile::save_custom_to_database()
{
    int i = 0;
    sqlite3 *pdatabase = NULL;


    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
    psqlite_conf->sqlite3_begin(pdatabase);
    for (i = E_TABLE_NAME_CUSTOM_COUNTDOWN; i < E_TABLE_NAME_CUSTOM_MULCHANNELLOCK + 1; i++)
    {
        if (GET_BIT(iIsSaveCustomParams, i) == 1)
        {
            switch(i)
            {
                case E_TABLE_NAME_CUSTOM_COUNTDOWN:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_COUNTDOWN);
                    psqlite_conf->sqlite3_insert_countdown_prm(pdatabase, &(custom.sCountdownParams));
                    psqlite_conf->sqlite3_update_column(pdatabase, TABLE_NAME_CUSTOM_COUNTDOWN, "cIsCountdownValueLimit", 1, &(custom.cIsCountdownValueLimit), 1, SQLITE_INTEGER);
                    break;
                case E_TABLE_NAME_CUSTOM_COM:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_COM);
                    psqlite_conf->sqlite3_insert_com_prm(pdatabase, custom.sComParams);
                    break;
                case E_TABLE_NAME_CUSTOM_CHANNELLOCK:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_CHANNELLOCK);
                    psqlite_conf->sqlite3_insert_chlock_prm(pdatabase, &(custom.sChannelLockedParams));
                    psqlite_conf->sqlite3_update_column(pdatabase, TABLE_NAME_CUSTOM_CHANNELLOCK, "cChannelLockFlag", 1, &(custom.cChannelLockFlag),  1, SQLITE_INTEGER);
                    break;
                case E_TABLE_NAME_CUSTOM_MULCHANNELLOCK:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_MULCHANNELLOCK);
                    psqlite_conf->sqlite3_insert_mp_chlock_prm(pdatabase, &(custom.sMulPeriodsChanLockParams));
                    break;
                default:
                    break;
            }
        }
    }
    psqlite_conf->sqlite3_commit(pdatabase);
    psqlite_conf->sqlite3_close_wrapper(pdatabase);
    iIsSaveCustomParams = 0;
}

void Customfile::DownloadCountdownParams(STRU_UDP_INFO* udp_info)
{
    memcpy(&custom.sCountdownParams, udp_info->iValue, sizeof(custom.sCountdownParams));
    SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_COUNTDOWN);
}
void Customfile::UploadCountdownParams(STRU_UDP_INFO* udp_info)
{
    memcpy(udp_info->iValue,&custom.sCountdownParams,sizeof(custom.sCountdownParams));
}

void Customfile::DownloadComParams(STRU_UDP_INFO* udp_info)
{
    if((udp_info->iValue[0] > 0) && (udp_info->iValue[0] <= 4))
    {
        memcpy(&custom.sComParams[udp_info->iValue[0] - 1],udp_info,sizeof(COM_PARAMS));
        SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_COM);
    }
}
void Customfile::UploadComParams(STRU_UDP_INFO* udp_info)
{
    if((udp_info->iValue[0] > 0) && (udp_info->iValue[0] <= 4))
    {
        custom.sComParams[udp_info->iValue[0] - 1].unExtraParamHead = udp_info->iHead;
        custom.sComParams[udp_info->iValue[0] - 1].unExtraParamID = udp_info->iType;
        memcpy(&udp_info, &custom.sComParams[udp_info->iValue[0] - 1], sizeof(COM_PARAMS));
    }
}

void Customfile::EnableChannelLock(STRU_UDP_INFO* udp_info)
{
    memcpy(&custom.sChannelLockedParams, udp_info, sizeof(custom.sChannelLockedParams));
    custom.cChannelLockFlag = 1;
    SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_CHANNELLOCK);
}
void Customfile::DisableChannelLock(STRU_UDP_INFO* udp_info)
{
    sqlite3* pdb = NULL;

    switch(udp_info->iValue[0])
    {
        case 0://原通锟斤拷锟斤拷锟斤拷
        //log_debug("disable channel lock");
        if(custom.cChannelLockFlag == 1)
        {
            custom.cChannelLockFlag = 0;
            //ItsChannelUnlock();
        }
        break;
        case 1://锟斤拷时锟斤拷通锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        //log_debug("Disable mult periods channel lock");
        custom.sMulPeriodsChanLockParams.cLockFlag = 0;
        break;
        case 2://锟斤拷时锟斤拷通锟斤拷锟斤拷锟斤拷锟街革拷
        //log_debug("Recover mult periods channel lock");
        custom.sMulPeriodsChanLockParams.cLockFlag = 1;
        break;
        default:
        //log_debug("Unknown Msg type in ChanUnlock!");
        break;
    }
    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
    psqlite_conf->sqlite3_update_column(pdb, TABLE_NAME_CUSTOM_CHANNELLOCK, "cChannelLockFlag", 1, &(custom.cChannelLockFlag),  1, SQLITE_INTEGER);
    psqlite_conf->sqlite3_update_column(pdb, TABLE_NAME_CUSTOM_MULCHANNELLOCK, "cLockFlag", 1, &(custom.sMulPeriodsChanLockParams.cLockFlag),  1, SQLITE_INTEGER);
    psqlite_conf->sqlite3_close_wrapper(pdb); pdb = NULL;

}

void Customfile::DownloadMulChLock(STRU_UDP_INFO* udp_info)
{
    memcpy((char*)custom.sMulPeriodsChanLockParams.chans,(char*)udp_info->iValue,sizeof(custom.sMulPeriodsChanLockParams.chans));
    custom.sMulPeriodsChanLockParams.cLockFlag = 1;
    SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_MULCHANNELLOCK);
}

void Customfile::UploadMulChLock(STRU_UDP_INFO* udp_info)
{
    memcpy(&udp_info->iValue[0], (char*)custom.sMulPeriodsChanLockParams.chans, sizeof(custom.sMulPeriodsChanLockParams.chans));
    *((char *)udp_info->iValue + sizeof(custom.sMulPeriodsChanLockParams.chans)) = custom.sMulPeriodsChanLockParams.cLockFlag;

}

void Customfile::SaveCustomUpdate()
{
    if(iIsSaveCustomParams == 0)
    {
        return;
    }
    //log_debug("write custom to database");
    save_custom_to_database();
    iIsSaveCustomParams = 0;
}



Descfile::Descfile(Sqliteconf* psqliteconf): psqlite_conf(psqliteconf)
{
/*
    std::memset(&sPhaseDescParams, 0, sizeof(sPhaseDescParams));
    std::memset(&sChannelDescParams, 0, sizeof(sChannelDescParams));
    std::memset(&sPatternNameParams, 0, sizeof(sPatternNameParams));
    std::memset(&sPlanNameParams, 0, sizeof(sPlanNameParams));
    std::memset(&sDateNameParams, 0, sizeof(sDateNameParams));
    std::memset(phaseDescText, 0, sizeof(phaseDescText));
    */
    std::memset(&desc, 0, sizeof(desc));
    iIsSaveDescParams = 0;
    read_desc_from_database();
}

void Descfile::read_desc_from_database()
{
    sqlite3 *pdatabase = NULL;

    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
    psqlite_conf->read_desc(pdatabase, &desc);
    psqlite_conf->sqlite3_close_wrapper(pdatabase);
    pdatabase = NULL;

}

void Descfile::save_desc_to_database()
{
    int i = 0;
    sqlite3 *pdatabase = NULL;

    psqlite_conf->sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
    psqlite_conf->sqlite3_begin(pdatabase);
    for (i = E_TABLE_NAME_DESC_PHASE; i < E_TABLE_NAME_DESC_DATE + 1; i++)
    {
        if (GET_BIT(iIsSaveDescParams, i) == 1)
        {
            switch(i)
            {
                case E_TABLE_NAME_DESC_PHASE:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_PHASE);
                    psqlite_conf->sqlite3_insert_phase_desc(pdatabase, desc.phaseDescText);
                    break;
                case E_TABLE_NAME_DESC_CHANNEL:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_CHANNEL);
                    psqlite_conf->sqlite3_insert_channel_desc(pdatabase, &(desc.sChannelDescParams));
                    break;
                case E_TABLE_NAME_DESC_PATTERN:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_PATTERN);
                    psqlite_conf->sqlite3_insert_pattern_name_desc(pdatabase, &(desc.sPatternNameParams));
                    break;
                case E_TABLE_NAME_DESC_PLAN:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_PLAN);
                    psqlite_conf->sqlite3_insert_plan_name(pdatabase, &(desc.sPlanNameParams));
                    break;
                case E_TABLE_NAME_DESC_DATE:
                    psqlite_conf->sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_DATE);
                    psqlite_conf->sqlite3_insert_plan_date(pdatabase, &(desc.sDateNameParams));
                    break;
                default:
                    break;
            }
        }
    }
    psqlite_conf->sqlite3_commit(pdatabase);
    psqlite_conf->sqlite3_close_wrapper(pdatabase);
    iIsSaveDescParams = 0;
}

void Descfile::DownloadPhasedesc(STRU_UDP_INFO* udp_info)
{
    memcpy(&desc.sPhaseDescParams,udp_info,sizeof(desc.sPhaseDescParams));
    memcpy(desc.phaseDescText[0], &desc.sPhaseDescParams.stPhaseDesc, sizeof(desc.sPhaseDescParams.stPhaseDesc));
    SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_PHASE);
}

void Descfile::UploadPhasedesc(STRU_UDP_INFO* udp_info)
{
    desc.sPhaseDescParams.unExtraParamHead = udp_info->iHead;
    desc.sPhaseDescParams.unExtraParamID = udp_info->iType;
    memcpy(udp_info, &desc.sPhaseDescParams, sizeof(desc.sPhaseDescParams));
}

void Descfile::DownloadChanneldesc(STRU_UDP_INFO* udp_info)
{
    memcpy(&desc.sChannelDescParams,udp_info,sizeof(desc.sChannelDescParams));
    SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_CHANNEL);
}
void Descfile::UploadChanneldesc(STRU_UDP_INFO* udp_info)
{
    desc.sChannelDescParams.unExtraParamHead = udp_info->iHead;
    desc.sChannelDescParams.unExtraParamID = udp_info->iType;
    memcpy(udp_info, &desc.sChannelDescParams, sizeof(desc.sChannelDescParams));
}

void Descfile::DownloadPlandesc(STRU_UDP_INFO* udp_info)
{
    memcpy(&desc.sPlanNameParams,udp_info,sizeof(desc.sPlanNameParams));
    SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_PLAN);
}
void Descfile::UploadPlandesc(STRU_UDP_INFO* udp_info)
{
    desc.sPlanNameParams.unExtraParamHead = udp_info->iHead;
    desc.sPlanNameParams.unExtraParamID = udp_info->iType;
    memcpy(udp_info, &desc.sPlanNameParams, sizeof(desc.sPlanNameParams));
}

void Descfile::DownloadSchemedesc(STRU_UDP_INFO* udp_info)
{
    memcpy(&desc.sPatternNameParams,udp_info,sizeof(desc.sPatternNameParams));
    SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_PATTERN);
}
void Descfile::UploadSchemedesc(STRU_UDP_INFO* udp_info)
{
    desc.sPatternNameParams.unExtraParamHead = udp_info->iHead;
    desc.sPatternNameParams.unExtraParamID = udp_info->iType;
    memcpy(udp_info, &desc.sPatternNameParams, sizeof(desc.sPatternNameParams));
}

void Descfile::DownloadDatedesc(STRU_UDP_INFO* udp_info)
{
    memcpy(&desc.sDateNameParams,udp_info,sizeof(desc.sDateNameParams));
    SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_DATE);
}
void Descfile::UploadDatedesc(STRU_UDP_INFO* udp_info)
{
    desc.sDateNameParams.unExtraParamHead = udp_info->iHead;
    desc.sDateNameParams.unExtraParamID = udp_info->iType;
    memcpy(udp_info, &desc.sDateNameParams, sizeof(desc.sDateNameParams));
}

void Descfile::SaveDescUpdate()
{
    if(iIsSaveDescParams == 0)
    {
        return;
    }
    //log_debug("write desc to database");
    save_desc_to_database();
    iIsSaveDescParams = 0;

}
