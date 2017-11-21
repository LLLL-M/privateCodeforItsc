#ifndef __PHASE_H__
#define __PHASE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum {

    RED = 0,//红灯
    YELLOW,//黄灯    
    GREEN,//绿灯    
    ALL_RED,//全红    
    PEDESTRIAN_CLEAR//行人清空
    
}InitSetting;//初始化设置


typedef struct {

    unsigned short nPhaseID;//相位号
    unsigned short nCircleID;//

    //基础时间
    unsigned short nMinGreen;//最小绿
    unsigned short nMaxGreen_1;//最大绿1
    unsigned short nMaxGreen_2;//最大绿2
    unsigned short nUnitExtendGreen;//单位延长绿
    unsigned short nYellowTime;//黄灯时间
    unsigned short nAllRedTime;//全红时间
    unsigned short nRedProtectedTime;//红灯保护
    unsigned short nRedYellowTime;//红黄时间
    unsigned short nGreenLightTime;//绿闪时间
    unsigned short nSafeRedTime;//安全红灯

    //行人
    unsigned short nPedestrianPassTime;//行人放行时间
    unsigned short nPedestrianClearTime;//行人清空时间

    //选项
    unsigned char cIsEnablePhase;//使能相位
    unsigned char cYellowEnter;//黄灯进入
    unsigned char cYellowExit;//黄灯退出
    unsigned char cDetectorLocked;//检测器非锁定
    unsigned char cMinGreenReq;//最小绿请求
    unsigned char cMaxGreenReq;//最大绿请求
    unsigned char cPedestrianReq;//行人请求
    unsigned char cAutoReq;//自动请求
    unsigned char cCircleReq;//环请求
    unsigned char cAutoPedestrianPass;//自动行人放行

    //综合
    InitSetting initSetting;//初始化设置

    //相位方向

    //车道方向

    //新增属性:绿信比时间 绿信比模式  是否作为协调相位  在一个环中的触发顺序
    unsigned short nGreenSignalRationTime;//绿信比时间
    GreenSignalRationType nType;//绿信比类型
    unsigned short nIsCoordinate;//是否作为协调相位  1:  作为协调相位 0: 不作为协调相位
    unsigned short nTurn;//触发顺序

}PhaseItem,*PPhaseItem;

typedef struct _PhaseList{

    PPhaseItem pItem;

    struct _PhaseList *next;


}PhaseList,*PPhaseList;



extern int AddOrAlterPhaseItem(PPhaseList *pList,PPhaseItem pItem);
extern int ClearPhaseList(PPhaseList *pList);
extern int DeletePhaseItem(PPhaseList *pList,PPhaseItem pItem);
extern int DestroyPhaseList(PPhaseList *pList);
extern int InitPhaseList(PPhaseList *pList);

extern int LoadDefaultPhase(PPhaseList pList);

extern PPhaseItem GetPhaseItem(PPhaseList pList,unsigned short nPhaseID);


extern int SetPhaseGreenSignalRatioPara(PPhaseList *pList,unsigned short nGreenSignalRationTime,GreenSignalRationType nType,
                                            unsigned short nIsCoordinate,unsigned short nPhaseID);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SCHEMES_H__ */





