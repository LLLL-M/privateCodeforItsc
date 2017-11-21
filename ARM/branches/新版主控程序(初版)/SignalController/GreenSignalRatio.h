

#ifndef __GREEN_SIGNAL_RATION_H__
#define __GREEN_SIGNAL_RATION_H__



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef enum {

    UNDEFINEED = 0,//δ����
    OTHER_TYPE,//��������
    NONE,//��
    MIN_MOTO,//��С������Ӧ    
    MAX_MOTO,//�������Ӧ
    PEDESTRIAN_RES,//������Ӧ
    PEDESTRIAN_QEQ,//�����/��������
    IGNORE_Phase//������λ
}GreenSignalRationType;//���ű�ģʽ


typedef struct {

    unsigned short nGreenSignalRationID;//���űȺ�
    unsigned short nPhaseID;//��λ��
    unsigned short nGreenSignalRationTime;//���ű�ʱ��
    GreenSignalRationType nType;//���ű�����
    unsigned short nIsCoordinate;//�Ƿ���ΪЭ����λ  1:  ��ΪЭ����λ 0: ����ΪЭ����λ

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

