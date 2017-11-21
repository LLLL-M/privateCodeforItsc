

#ifndef __GREEN_SIGNAL_RATION_H__
#define __GREEN_SIGNAL_RATION_H__



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef enum {

    UNDEFINEED = 0,//未定义
    OTHER_TYPE,//其他类型
    NONE,//无
    MIN_MOTO,//最小车辆响应    
    MAX_MOTO,//最大车辆响应
    PEDESTRIAN_RES,//行人响应
    PEDESTRIAN_QEQ,//最大车辆/行人请求
    IGNORE_Phase//忽略相位
}GreenSignalRationType;//绿信比模式


typedef struct {

    unsigned short nGreenSignalRationID;//绿信比号
    unsigned short nPhaseID;//相位号
    unsigned short nGreenSignalRationTime;//绿信比时间
    GreenSignalRationType nType;//绿信比类型
    unsigned short nIsCoordinate;//是否作为协调相位  1:  作为协调相位 0: 不作为协调相位

}GreenSignalRationItem,*PGreenSignalRationItem;

typedef struct _GreenSignalRationList{

    PGreenSignalRationItem pItem;

    struct _GreenSignalRationList *next;


}GreenSignalRationList,*PGreenSignalRationList;



extern int AddOrAlterGreenSignalRationItem(PGreenSignalRationList *pList,PGreenSignalRationItem pItem);
extern int ClearGreenSignalRationList(PGreenSignalRationList *pList);
extern int DeleteGreenSignalRationItem(PGreenSignalRationList *pList,PGreenSignalRationItem pItem);
extern int DestroyGreenSignalRationList(PGreenSignalRationList *pList);
extern int InitGreenSignalRationList(PGreenSignalRationList *pList);
int LoadDefaultGreenSignalRation(PGreenSignalRationList pList);
extern int SetPhaseGreenSignalRationItem(PGreenSignalRationList *pList,unsigned short nGreenSignalRationID,unsigned short nPhaseID);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SCHEMES_H__ */

