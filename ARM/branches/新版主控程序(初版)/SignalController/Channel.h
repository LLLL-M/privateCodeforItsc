/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Channels.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : Channels.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __CHANNEL_H__
#define __CHANNEL_H__


#include "Util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum {

    OTHER = 0,//��������
    MOTOR,//����������
    PEDESTRIAN,//��������
    FOLLOW//��������
    
}ControllerType;//����Դ������


typedef enum {

    ALTERNATE = 0,//����
    REDLIGHT,//����
    YELLOWLIGHT//����

}FlashLightType;//����ģʽ



typedef struct {

    unsigned short nChannelID;//ͨ����
    unsigned short nControllerID;//����Դ��
    ControllerType nControllerType;//����Դ����
    FlashLightType nFlashLightType;//����ģʽ

}ChannelItem,*PChannelItem;

typedef struct _ChannelList{

    PChannelItem pItem;

    struct _ChannelList *next;


}ChannelList,*PChannelList;



extern int AddOrAlterChanneltem(PChannelList *pList,PChannelItem pItem);
extern int ClearChannelList(PChannelList *pList);
extern int DeleteChannelItem(PChannelList *pList,PChannelItem pItem);
extern int DestroyChannelList(PChannelList *pList);
extern int InitChannelList(PChannelList *pList);
extern int LoadDefaultChannel(PChannelList pList);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SCHEMES_H__ */
