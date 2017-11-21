#ifndef __PHASE_H__
#define __PHASE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum {

    RED = 0,//���
    YELLOW,//�Ƶ�    
    GREEN,//�̵�    
    ALL_RED,//ȫ��    
    PEDESTRIAN_CLEAR//�������
    
}InitSetting;//��ʼ������


typedef struct {

    unsigned short nPhaseID;//��λ��
    unsigned short nCircleID;//

    //����ʱ��
    unsigned short nMinGreen;//��С��
    unsigned short nMaxGreen_1;//�����1
    unsigned short nMaxGreen_2;//�����2
    unsigned short nUnitExtendGreen;//��λ�ӳ���
    unsigned short nYellowTime;//�Ƶ�ʱ��
    unsigned short nAllRedTime;//ȫ��ʱ��
    unsigned short nRedProtectedTime;//��Ʊ���
    unsigned short nRedYellowTime;//���ʱ��
    unsigned short nGreenLightTime;//����ʱ��
    unsigned short nSafeRedTime;//��ȫ���

    //����
    unsigned short nPedestrianPassTime;//���˷���ʱ��
    unsigned short nPedestrianClearTime;//�������ʱ��

    //ѡ��
    unsigned char cIsEnablePhase;//ʹ����λ
    unsigned char cYellowEnter;//�Ƶƽ���
    unsigned char cYellowExit;//�Ƶ��˳�
    unsigned char cDetectorLocked;//�����������
    unsigned char cMinGreenReq;//��С������
    unsigned char cMaxGreenReq;//���������
    unsigned char cPedestrianReq;//��������
    unsigned char cAutoReq;//�Զ�����
    unsigned char cCircleReq;//������
    unsigned char cAutoPedestrianPass;//�Զ����˷���

    //�ۺ�
    InitSetting initSetting;//��ʼ������

    //��λ����

    //��������

    //��������:���ű�ʱ�� ���ű�ģʽ  �Ƿ���ΪЭ����λ  ��һ�����еĴ���˳��
    unsigned short nGreenSignalRationTime;//���ű�ʱ��
    GreenSignalRationType nType;//���ű�����
    unsigned short nIsCoordinate;//�Ƿ���ΪЭ����λ  1:  ��ΪЭ����λ 0: ����ΪЭ����λ
    unsigned short nTurn;//����˳��

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





