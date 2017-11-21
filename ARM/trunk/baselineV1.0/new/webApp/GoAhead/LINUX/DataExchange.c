/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : DataExchange.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��11��25��
  ����޸�   :
  ��������   : ��Դ�ļ���Ҫ������webCallback��Ҫ��getter setter���������ڴ�
               ȫ��ָ���л�ȡ����
  �����б�   :
              ClearConcurrentPhaseArrayItem
              getChannelTableInfo
              getCoordinateInfo
              getFaultConfigInfo
              getFaultDetectionSetInfo
              getGreenRatioInfo
              getLoginInfo
              getOverlappingInfo
              getPedestrianInfo
              getPhaseTableInfo
              getProgramTableInfo
              getringAndPhaseInfo
              getSchedulingInfo
              getSequenceTableInfo
              getTimeBasedActionTableInfo
              getTimeIntervalInfo
              getTreeDynamicParameter
              getUnitParamsInfo
              getVehicleDetectorInfo
              saveCoordinate2Ini
              saveFaultConfig2Ini
              saveFaultDetectionSet2Ini
              savePedestrian2Ini
              saveTreeDynamicParameter
              saveVehicleDetector2Ini
              setChannelTableInfo
              setGreenRatioInfo
              setOverlappingInfo
              setPhaseTableInfo
              setProgramTableInfo
              setringAndPhaseInfo
              setSchedulingInfo
              setSequenceTableInfo
              setTimeBasedActionTableInfo
              setTimeIntervalInfo
              setUnitParamsInfo
  �޸���ʷ   :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "DataExchange.h"


/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern SignalControllerPara *gSignalControlpara;//ȫ��ָ�룬�洢���źŻ�������Ҫ�����б���
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

static void itoa ( unsigned long val, char *buf, unsigned radix )
{
        char *p;                /* pointer to traverse string */
        char *firstdig;         /* pointer to first digit */
        char temp;              /* temp char */
        unsigned digval;        /* value of digit */

        p = buf;
        firstdig = p;           /* save pointer to first digit */

        do {
            digval = (unsigned) (val % radix);
            val /= radix;       /* get next digit */

            /* convert to ascii and store */
            if (digval > 9)
                *p++ = (char ) (digval - 10 + 'a');  /* a letter */
            else
                *p++ = (char ) (digval + '0');       /* a digit */
        } while (val > 0);

        /* We now have the digit of the number in the buffer, but in reverse
           order.  Thus we reverse them now. */

        *p-- = '\0';            /* terminate string; p points to last digit */

        do {
            temp = *p;
            *p = *firstdig;
            *firstdig = temp;   /* swap *p and *firstdig */
            --p;
            ++firstdig;         /* advance to next two digits */
        } while (firstdig < p); /* repeat until halfway */
}


/*****************************************************************************
 �� �� ��  : getLoginInfo
 ��������  : ���ݱ��������ļ�����ȡ�û���Ϣ
 �������  : char_t * username  
             char_t * password  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getLoginInfo(char_t * username,char_t * password)
{

	//const char * file = "/home/login.ini";
//	char * section = "LOGIN";
	char value[BUF_SIZE] = "";
	char realUsername[16] = { 0 };
	char realPassword[16] = { 0 };
	char loginUsername[16] = { 0 };
	char loginPassword[16] = { 0 };
	memcpy(loginUsername,username,16);
	memcpy(loginPassword,password,16);
	if(!read_profile_string("LOGIN","username",value,BUF_SIZE,"","/home/login.ini"))
	{

		return  0;
	}
	else
	{
		memcpy(realUsername,value,16);
	}
	if(!read_profile_string("LOGIN","password",value,BUF_SIZE,"","/home/login.ini"))
	{

		return  0;
	}
	else
	{
		memcpy(realPassword,value,16);
	}
	if (strcmp(loginUsername,realUsername) == 0 && strcmp(loginPassword,realPassword) == 0)
	{
		return 1;
	}
	else if(strcmp(loginUsername,realUsername) != 0)
	{
		return -1;
	}
	else if(strcmp(loginUsername,realUsername) == 0 && strcmp(loginPassword,realPassword) != 0)
	{
		return -2;
	}

	return 1;
}

/*****************************************************************************
 �� �� ��  : getUnitParamsInfo
 ��������  : ��ȫ��ָ���л�ȡ��Ԫ����
 �������  : stUnitParams *pstUnitParams  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��1��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : ������ȡ�����ɼ����ڵ�
*****************************************************************************/
int getUnitParamsInfo(stUnitParams *pstUnitParams)
{
	memset(pstUnitParams,0,sizeof(stUnitParams));

    if(!gSignalControlpara)
    {
        return 0;
    }

    //�����ʼ״̬Ϊ�գ������ֵ��
    if((0 == gSignalControlpara->stUnitPara.nBootAllRedTime) || (0 == gSignalControlpara->stUnitPara.nBootYellowLightTime))
    {
        gSignalControlpara->stUnitPara.nBootAllRedTime = 6;
        gSignalControlpara->stUnitPara.nBootYellowLightTime = 6;
        gSignalControlpara->stUnitPara.byFlashFrequency = 60;
        gSignalControlpara->stUnitPara.byTransCycle = 2;
        gSignalControlpara->stUnitPara.byCollectCycleUnit = 0;
         gSignalControlpara->stUnitPara.byFluxCollectCycle = 120;
    }

    pstUnitParams->iStartAllRedTime = gSignalControlpara->stUnitPara.nBootAllRedTime;
    pstUnitParams->iStartFlashingYellowTime = gSignalControlpara->stUnitPara.nBootYellowLightTime;
    pstUnitParams->iFlashingFrequency = 60;
    pstUnitParams->iSmoothTransitionPeriod = gSignalControlpara->stUnitPara.byTransCycle;
    pstUnitParams->iCollectUnit = gSignalControlpara->stUnitPara.byCollectCycleUnit;
    pstUnitParams->iFlowCollectionPeriod = gSignalControlpara->stUnitPara.byFluxCollectCycle;
    
	return 1;
}

/*****************************************************************************
 �� �� ��  : setUnitParamsInfo
 ��������  : ��WEB������Ϣ���浽ȫ��ָ���С�
 �������  : stUnitParams *pstUnitParams  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setUnitParamsInfo(stUnitParams *pstUnitParams)
{
    if(!gSignalControlpara)
    {
        return 0;
    }
    memset(&gSignalControlpara->stUnitPara,0,sizeof(UnitPara));

    gSignalControlpara->stUnitPara.nBootAllRedTime = pstUnitParams->iStartAllRedTime;
    gSignalControlpara->stUnitPara.nBootYellowLightTime = pstUnitParams->iStartFlashingYellowTime;
    gSignalControlpara->stUnitPara.cIsPedestrianAutoClear = pstUnitParams->iAutoPedestrianEmpty;
    gSignalControlpara->stUnitPara.byTransCycle = pstUnitParams->iSmoothTransitionPeriod;
    gSignalControlpara->stUnitPara.byCollectCycleUnit=  pstUnitParams->iCollectUnit;
    gSignalControlpara->stUnitPara.byFluxCollectCycle = pstUnitParams->iFlowCollectionPeriod;

    log_debug("%s nBootAllRedTime   %d     nBootYellowLightTime  %d    cIsPedestrianAutoClear  %d iSmoothTransitionPeriod  %d byCollectCycleUnit %d  byFluxCollectCycle  %d\n",__func__,
                            gSignalControlpara->stUnitPara.nBootAllRedTime,
                            gSignalControlpara->stUnitPara.nBootYellowLightTime,
                            gSignalControlpara->stUnitPara.cIsPedestrianAutoClear,
                            gSignalControlpara->stUnitPara.byTransCycle,
                            gSignalControlpara->stUnitPara.byCollectCycleUnit,
                            gSignalControlpara->stUnitPara.byFluxCollectCycle);


	return 1;
}

/*****************************************************************************
 �� �� ��  : getringAndPhaseInfo
 ��������  : ��ȫ��ָ���л�ò�����λ��Ϣ
 �������  : PConcurrentPhaseItem item  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getringAndPhaseInfo(PConcurrentPhaseItem item)
{
    if(!gSignalControlpara)
    {
        return 0;
    }

    int i = 0;

    for(i = 0; i < NUM_PHASE; i++)
    {
        item[i].nPhaseID = gSignalControlpara->stPhase[i].nPhaseID;
        item[i].nCircleID = gSignalControlpara->stPhase[i].nCircleID;
        item[i].nArrayConcurrentPase[i] = gSignalControlpara->stPhase[i].nPhaseID;
        memcpy(item[i].nArrayConcurrentPase,gSignalControlpara->stPhase[i].byPhaseConcurrency,sizeof(unsigned char)*NUM_PHASE);
    }

    return 1;
}


/*****************************************************************************
 �� �� ��  : setringAndPhaseInfo
 ��������  : ��WEB���õĲ�����λ��Ϣ���浽ȫ��ָ���С�
 �������  : PConcurrentPhaseItem item  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setringAndPhaseInfo(PConcurrentPhaseItem item)
{
    if(!gSignalControlpara)
    {
        return 0;
    }

    int i = 0;

    for(i = 0; i < NUM_PHASE; i++)
    {
        gSignalControlpara->stPhase[i].nPhaseID = item[i].nPhaseID;
        gSignalControlpara->stPhase[i].nCircleID = item[i].nCircleID;
        memcpy(gSignalControlpara->stPhase[i].byPhaseConcurrency,item[i].nArrayConcurrentPase,sizeof(unsigned char)*NUM_PHASE);
    }

    return 1;
}


/*****************************************************************************
 �� �� ��  : getPhaseTableInfo
 ��������  : ��ȫ��ָ���л����λ�����
 �������  : stPhaseTable *pstPhaseTable  
             int i                        
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��1��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : ����Ӧ������ز�����ӽ���
*****************************************************************************/
int getPhaseTableInfo(stPhaseTable *pstPhaseTable,int i)
{
	memset(pstPhaseTable,0,sizeof(stPhaseTable));

    if((!gSignalControlpara) || (i < 1) || (i > NUM_PHASE))
    {
        return 0;
    }

    //�������������λ�������λ��ʼֵ
    //if(gSignalControlpara->stPhase[i - 1].nCircleID == 0)
    if(IS_PHASE_INABLE(gSignalControlpara->stPhase[i - 1].wPhaseOptions) == 0)
    {
        gSignalControlpara->stPhase[i - 1].nPhaseID = i;
        gSignalControlpara->stPhase[i - 1].nCircleID  = 1;
        gSignalControlpara->stPhase[i - 1].nYellowTime = 3;
        gSignalControlpara->stPhase[i - 1].nAllRedTime = 2;
        gSignalControlpara->AscSignalTransTable[i - 1].nGreenLightTime = 2;
        gSignalControlpara->stPhase[i - 1].wPhaseOptions |= 0x01;
        gSignalControlpara->stPhase[i - 1].nPedestrianPassTime = 10;
        gSignalControlpara->stPhase[i - 1].nPedestrianClearTime = 6;
        gSignalControlpara->stPhase[i - 1].nMinGreen = 15;
        gSignalControlpara->stPhase[i - 1].nMaxGreen_1 = 30;
        gSignalControlpara->stPhase[i - 1].nMaxGreen_2 = 20;
        gSignalControlpara->stPhase[i - 1].nUnitExtendGreen= 3;
        //gSignalControlpara->stPhase[i - 1].cAutoPedestrianPass = 0;
    }

    pstPhaseTable->iPhaseNo = i - 1;
    pstPhaseTable->nCircleNo = gSignalControlpara->stPhase[i - 1].nCircleID;//��������
    pstPhaseTable->iYellowLightTime = gSignalControlpara->stPhase[i - 1].nYellowTime;
    pstPhaseTable->iAllRedTime = gSignalControlpara->stPhase[i - 1].nAllRedTime;
    pstPhaseTable->nGreenLightTime = gSignalControlpara->AscSignalTransTable[i - 1].nGreenLightTime;
    pstPhaseTable->nIsEnable = (gSignalControlpara->stPhase[i - 1].wPhaseOptions)&0x01;
    pstPhaseTable->iPedestrianRelease = gSignalControlpara->stPhase[i - 1].nPedestrianPassTime;
    pstPhaseTable->iPedestrianCleaned = gSignalControlpara->stPhase[i - 1].nPedestrianClearTime;
    //pstPhaseTable->nAutoPedestrianPass = gSignalControlpara->stPhase[i - 1].cAutoPedestrianPass;
    pstPhaseTable->iMinimumGreen = gSignalControlpara->stPhase[i - 1].nMinGreen;
    pstPhaseTable->iMaximumGreenOne = gSignalControlpara->stPhase[i - 1].nMaxGreen_1;
    pstPhaseTable->iMaximumGreenTwo = gSignalControlpara->stPhase[i - 1].nMaxGreen_2;
    pstPhaseTable->iExtensionGreen = gSignalControlpara->stPhase[i - 1].nUnitExtendGreen;


	return 1;
}

/*****************************************************************************
 �� �� ��  : setPhaseTableInfo
 ��������  : ��WEB��������λ��Ϣ���浽ȫ�ֽṹָ���С�
 �������  : stPhaseTable *pstPhaseTable  
             int i                        
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��1��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : ������Ӧ��ز����ı���
*****************************************************************************/
int setPhaseTableInfo(stPhaseTable *pstPhaseTable,int i)
{
    if((!gSignalControlpara) || (i < 1) || (i > NUM_PHASE))
    {
        return 0;
    }

    memset(&gSignalControlpara->stPhase[i - 1],0,sizeof(PhaseItem));

    gSignalControlpara->stPhase[i - 1].nPhaseID = i;
    gSignalControlpara->stPhase[i - 1].nCircleID = pstPhaseTable->nCircleNo  ;//��������
    gSignalControlpara->stPhase[i - 1].nYellowTime = pstPhaseTable->iYellowLightTime;
    gSignalControlpara->stPhase[i - 1].nAllRedTime = pstPhaseTable->iAllRedTime;
    gSignalControlpara->AscSignalTransTable[i - 1].nGreenLightTime = pstPhaseTable->nGreenLightTime ;
    gSignalControlpara->stPhase[i - 1].wPhaseOptions |= pstPhaseTable->nIsEnable;
    gSignalControlpara->stPhase[i - 1].nPedestrianPassTime = pstPhaseTable->iPedestrianRelease;
    gSignalControlpara->stPhase[i - 1].nPedestrianClearTime = pstPhaseTable->iPedestrianCleaned ;
    //gSignalControlpara->stPhase[i - 1].cAutoPedestrianPass = pstPhaseTable->nAutoPedestrianPass;
    gSignalControlpara->stPhase[i - 1].nMinGreen = pstPhaseTable->iMinimumGreen;
    gSignalControlpara->stPhase[i - 1].nMaxGreen_1 = pstPhaseTable->iMaximumGreenOne;
    gSignalControlpara->stPhase[i - 1].nMaxGreen_2 = pstPhaseTable->iMaximumGreenTwo;
    gSignalControlpara->stPhase[i - 1].nUnitExtendGreen = pstPhaseTable->iExtensionGreen;

    //���űȱ��ø���,UI�Ǳ߻��Զ�����
    log_debug("%s  nPhaseId  %d  nCircleId  %d  nGreenLightTime  %d  nYellowTime  %d  nAllRedTime  %d nMinGreen %d nUnitExtendGreen %d nMaxGreen_1 %d\n",__func__,
                gSignalControlpara->stPhase[i - 1].nPhaseID,
                gSignalControlpara->stPhase[i - 1].nCircleID,
                gSignalControlpara->AscSignalTransTable[i - 1].nGreenLightTime,
                gSignalControlpara->stPhase[i - 1].nYellowTime,
                gSignalControlpara->stPhase[i - 1].nAllRedTime,
                gSignalControlpara->stPhase[i - 1].nMinGreen,
                gSignalControlpara->stPhase[i - 1].nUnitExtendGreen,
                gSignalControlpara->stPhase[i - 1].nMaxGreen_1);
	return 1;
}

/*****************************************************************************
 �� �� ��  : getChannelTableInfo
 ��������  : ��ȫ�ֽṹָ���л��ͨ������Ϣ
 �������  : stChannelTable *pstChannelTable  
             int i                            
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getChannelTableInfo(stChannelTable *pstChannelTable, int i)
{	//��ȡ�����ļ�����
	memset(pstChannelTable,0,sizeof(stChannelTable));

    if((!gSignalControlpara) || (i < 1) || (i > NUM_CHANNEL))
    {
        return  0;
    }

    pstChannelTable->iControlSource = gSignalControlpara->stChannel[i - 1].nControllerID;
    pstChannelTable->iControlType = gSignalControlpara->stChannel[i - 1].nControllerType;
    pstChannelTable->iFlashMode = gSignalControlpara->stChannel[i - 1].nFlashLightType;

    if(gSignalControlpara->stChannel[i - 1].nChannelID == 0)
    {
        pstChannelTable->iControlType = 2;
    }


	return 1;
}

/*****************************************************************************
 �� �� ��  : setChannelTableInfo
 ��������  : ��WEB���õ�ͨ������Ϣ���浽ȫ��ָ���С�
 �������  : stChannelTable *pstChannelTable  
             int i                            
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setChannelTableInfo(stChannelTable *pstChannelTable, int i)
{
    if((!gSignalControlpara) || (i < 1) || (i > NUM_CHANNEL))
    {
        return  0;
    }

    memset(&gSignalControlpara->stChannel[i - 1],0,sizeof(ChannelItem));

    gSignalControlpara->stChannel[i - 1].nControllerID = pstChannelTable->iControlSource;
    gSignalControlpara->stChannel[i - 1].nControllerType = pstChannelTable->iControlType;
    gSignalControlpara->stChannel[i - 1].nFlashLightType = pstChannelTable->iFlashMode ;
    gSignalControlpara->stChannel[i - 1].nChannelID = i;


    log_debug("%s   channelId  %d nControllerId  %d  nControllerType  %d  \n",__func__,i,
                gSignalControlpara->stChannel[i - 1].nControllerID,
                gSignalControlpara->stChannel[i - 1].nControllerType);

	return 1;
}

/*****************************************************************************
 �� �� ��  : getGreenRatioInfo
 ��������  : ��ȫ��ָ���л�����űȱ���Ϣ
 �������  : stGreenRatio *pstGreenRatio  
             int iSplit                   
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getGreenRatioInfo(stGreenRatio *pstGreenRatio,int iSplit)
{
	memset(pstGreenRatio,0,sizeof(stGreenRatio));

    if((!gSignalControlpara) || (iSplit < 1) || (iSplit > NUM_GREEN_SIGNAL_RATION))
    {
        return 0;
    }

    pstGreenRatio->iSplitNo = iSplit;

    int i = 0;

    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        if(gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nPhaseID == 0)
        {
            continue;
        }

        pstGreenRatio->iSplitForPhase[i] = gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nGreenSignalRationTime;
        pstGreenRatio->iModeForPhase[i] = gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nType;
        pstGreenRatio->iCoordinatePhase[i] = gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nIsCoordinate;
        pstGreenRatio->nPhaseId[i] = gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nPhaseID;

    }

	return 1;
}

/*****************************************************************************
 �� �� ��  : setGreenRatioInfo
 ��������  : ��WEB���õ����ű���Ϣ���浽ȫ�ֽṹָ���С�
 �������  : stGreenRatio *pstGreenRatio  
             int iSplit                   
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setGreenRatioInfo(stGreenRatio *pstGreenRatio,int iSplit)
{

    if((!gSignalControlpara) || (iSplit < 1) || (iSplit > NUM_GREEN_SIGNAL_RATION))
    {
        return 0;
    }

    memset((void *)gSignalControlpara->stGreenSignalRation[iSplit - 1],0,sizeof(GreenSignalRationItem));

    int i = 0;

    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nGreenSignalRationID = iSplit;
        gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nGreenSignalRationTime = pstGreenRatio->iSplitForPhase[i];
        gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nType = pstGreenRatio->iModeForPhase[i];
        gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nIsCoordinate = pstGreenRatio->iCoordinatePhase[i] ;
        gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nPhaseID = pstGreenRatio->nPhaseId[i];

        if(pstGreenRatio->nPhaseId[i] != 0)
        {
            log_debug("%s GreenRationId  %d  phase  %d ration  %d  nIsCoordinate  %d\n",__func__,
                                iSplit,
                                i+1,
                                gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nGreenSignalRationTime,
                                gSignalControlpara->stGreenSignalRation[iSplit - 1][i].nIsCoordinate);
        }

    }

	return 1;
}

/*****************************************************************************
 �� �� ��  : getFaultDetectionSetInfo
 ��������  : ��ȫ�ֽṹָ���л�ù��ϼ����Ϣ
 �������  : stFaultDetectionSet *pstFaultDetectionSet  
             char *strConfigPath                        
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getFaultDetectionSetInfo(stFaultDetectionSet *pstFaultDetectionSet,char *strConfigPath)
{
	return 0;
	char *section = "config";
	char value[BUF_SIZE] = "";
	int i;
	char sChanNum0[48],sChanNum1[48],sChanNum2[48];

	memset(pstFaultDetectionSet,0,sizeof(stFaultDetectionSet));
	if(!read_profile_string(section,"VoltageDetectionTimes",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->VoltageDetectionTimes = atoi(value);
	}
	if(!read_profile_string(section,"RedLightDetectionTimes",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->RedLightDetectionTimes = atoi(value);
	}
	if(!read_profile_string(section,"ConflictDetectionAttempts",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->ConflictDetectionAttempts = atoi(value);
	}
	if(!read_profile_string(section,"ManualPanelKeyNumber",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->ManualPanelKeyNumber = atoi(value);
	}
		if(!read_profile_string(section,"RemoteControlKeyNumber",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->RemoteControlKeyNumber = atoi(value);
	}
	if(!read_profile_string(section,"ErrorDetectSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->SenseSwitch = atoi(value);
	}
	if(!read_profile_string(section,"CurrentAlarmSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->DynamicStep = atoi(value);
	}
	if(!read_profile_string(section,"VoltageAlarmSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->CurrentFaultDetection = atoi(value);
	}
	if(!read_profile_string(section,"CurrentAlarmAndProcessSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->AlarmAndFaultCurrent = atoi(value);
	}
	if(!read_profile_string(section,"VoltageAlarmAndProcessSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->AlarmAndFaultVoltage = atoi(value);
	}
	if(!read_profile_string(section,"WatchdogSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->EnableWatchdog = atoi(value);
	}
	if(!read_profile_string(section,"GpsSwitch",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultDetectionSet->EnableGPS = atoi(value);
	}
	section = "currentparams";
	for(i=0;i<=31;i++)
	{
		sprintf(sChanNum0,"RedCurrentBase%d", i+1);
		sprintf(sChanNum1,"RedCurrentDiff%d", i+1);
		sprintf(sChanNum2,"RedCurrentRefer%d", i+1);
		if(!read_profile_string(section,sChanNum0,value,BUF_SIZE,"",strConfigPath))
		{
			//��д�ļ��쳣
			return  0;
		}
		else
		{
			pstFaultDetectionSet->CNum[i][0] = atoi(value);
		}		if(!read_profile_string(section,sChanNum1,value,BUF_SIZE,"",strConfigPath))
		{
			//��д�ļ��쳣
			return  0;
		}
		else
		{
			pstFaultDetectionSet->CNum[i][1] = atoi(value);
		}		if(!read_profile_string(section,sChanNum2,value,BUF_SIZE,"",strConfigPath))
		{
			//��д�ļ��쳣
			return  0;
		}
		else
		{
			pstFaultDetectionSet->CNum[i][2] = atoi(value);
		}
	}
	return 1;
}

/*****************************************************************************
 �� �� ��  : saveFaultDetectionSet2Ini
 ��������  : ��WEB���õĹ��ϼ����Ϣ���浽�����ļ���
 �������  : stFaultDetectionSet stFaultDetectionSetEx  
             char * strConfigPath                       
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int saveFaultDetectionSet2Ini(stFaultDetectionSet stFaultDetectionSetEx,char * strConfigPath)
{
	char *section = "config";
//	char tmpstr[48] = { 0 };
	int i;
	char sPhaseNum0[48],sPhaseNum1[48],sPhaseNum2[48];

    if(parse_start(strConfigPath) == 1)
    {
        add_one_key(section, "VoltageDetectionTimes",stFaultDetectionSetEx.VoltageDetectionTimes);
        add_one_key(section, "RedLightDetectionTimes",stFaultDetectionSetEx.RedLightDetectionTimes);
        add_one_key(section, "ConflictDetectionAttempts",stFaultDetectionSetEx.ConflictDetectionAttempts);
        add_one_key(section, "ManualPanelKeyNumber",stFaultDetectionSetEx.ManualPanelKeyNumber);
        add_one_key(section, "RemoteControlKeyNumber",stFaultDetectionSetEx.RemoteControlKeyNumber);
        add_one_key(section, "ErrorDetectSwitch",stFaultDetectionSetEx.SenseSwitch);
        add_one_key(section, "CurrentAlarmSwitch",stFaultDetectionSetEx.DynamicStep);
        add_one_key(section, "VoltageAlarmSwitch",stFaultDetectionSetEx.CurrentFaultDetection);
        add_one_key(section, "CurrentAlarmAndProcessSwitch",stFaultDetectionSetEx.AlarmAndFaultCurrent);
        add_one_key(section, "VoltageAlarmAndProcessSwitch",stFaultDetectionSetEx.AlarmAndFaultVoltage);
        add_one_key(section, "WatchdogSwitch",stFaultDetectionSetEx.EnableWatchdog);

        add_one_key(section, "GpsSwitch",stFaultDetectionSetEx.EnableGPS);

    	section = "currentparams";
    	for(i=0;i<=31;i++)
    	{
    		sprintf(sPhaseNum0,"RedCurrentBase%d", i+1);
    		sprintf(sPhaseNum1,"RedCurrentDiff%d", i+1);
    		sprintf(sPhaseNum2,"RedCurrentRefer%d", i+1);
    		add_one_key(section,sPhaseNum0,stFaultDetectionSetEx.CNum[i][0]);
    		add_one_key(section,sPhaseNum1,stFaultDetectionSetEx.CNum[i][1]);
    		add_one_key(section,sPhaseNum2,stFaultDetectionSetEx.CNum[i][2]);
    	}

        parse_end();

        return 1;
    }


    return 0;

}

/*****************************************************************************
 �� �� ��  : getSequenceTableInfo
 ��������  : ��ȫ�ֽṹָ���л�÷�������Ϣ
 �������  : stSequenceTable *pstSequenceTable  
             int nPhaseTurnId                   
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getSequenceTableInfo(stSequenceTable *pstSequenceTable,int nPhaseTurnId)
{
	memset(pstSequenceTable,0,sizeof(stSequenceTable));

    if((!gSignalControlpara) || (nPhaseTurnId < 1) || (nPhaseTurnId > NUM_PHASE_TURN))
    {
        return 0;
    }

    int i = 0;
    int j = 0;

    pstSequenceTable->SequenceTableNo = nPhaseTurnId;

    for(i = 0 ; i < 4 ; i++)
    {
        if(gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nCircleID == 0)
        {
            continue;
        }
        for(j = 0 ; j < NUM_PHASE ; j++)
        {
            pstSequenceTable->SNum[i][j] = gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j];
        }

    }

	return 1;
}

/*****************************************************************************
 �� �� ��  : setSequenceTableInfo
 ��������  : ��WEB���õķ�������Ϣ���浽ȫ�ֽṹָ���С�
 �������  : stSequenceTable *pstSequenceTable  
             int nPhaseTurnId                   
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setSequenceTableInfo(stSequenceTable *pstSequenceTable,int nPhaseTurnId)
{

    if((!gSignalControlpara) || (nPhaseTurnId < 1) || (nPhaseTurnId > NUM_PHASE_TURN))
    {
        return 0;
    }

    int i = 0;
    int j = 0;

    memset((void *)gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1],0,sizeof(PhaseTurnItem)*4);

    for(i = 0 ; i < 4 ; i++)
    {
        for(j = 0 ; j < NUM_PHASE ; j++)
        {
            gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nPhaseTurnID = pstSequenceTable->SequenceTableNo;
            gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] = pstSequenceTable->SNum[i][j];
            gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nCircleID = i+1;


            //if(gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] != 0)
             //  printf("%d  ",gSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j]);
        }
       // printf("\n");
    }

	return 1;
}

/*****************************************************************************
 �� �� ��  : getProgramTableInfo
 ��������  : ��ȫ�ֽṹָ���л�÷�������Ϣ
 �������  : stProgramTable *pstProgramTable  
             int num                          
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getProgramTableInfo(stProgramTable *pstProgramTable, int num)
{
	memset(pstProgramTable,0,sizeof(stProgramTable));

    if((!gSignalControlpara) || (num < 1) || (num > NUM_SCHEME))
    {
        return 0;
    }

    pstProgramTable->nSchemeID = num;

    pstProgramTable->nCycleTime = gSignalControlpara->stScheme[num - 1].nCycleTime;
    pstProgramTable->nGreenSignalRatioID= gSignalControlpara->stScheme[num - 1].nGreenSignalRatioID;
    pstProgramTable->nOffset= gSignalControlpara->stScheme[num - 1].nOffset;
    pstProgramTable->nPhaseTurnID= gSignalControlpara->stScheme[num - 1].nPhaseTurnID;

	return 1;
}

/*****************************************************************************
 �� �� ��  : setProgramTableInfo
 ��������  : ��WEB���õķ�������Ϣ���浽ȫ�ֽṹָ���С�
 �������  : stProgramTable *pstProgramTable  
             int num                          
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setProgramTableInfo(stProgramTable *pstProgramTable, int num)
{
    if((!gSignalControlpara) || (num < 1) || (num > NUM_SCHEME))
    {
        return 0;
    }

    gSignalControlpara->stScheme[num - 1].nSchemeID = pstProgramTable->nSchemeID;
    gSignalControlpara->stScheme[num - 1].nCycleTime = pstProgramTable->nCycleTime ;
    gSignalControlpara->stScheme[num - 1].nGreenSignalRatioID = pstProgramTable->nGreenSignalRatioID;
    gSignalControlpara->stScheme[num - 1].nOffset = pstProgramTable->nOffset;
    gSignalControlpara->stScheme[num - 1].nPhaseTurnID= pstProgramTable->nPhaseTurnID;

	//num = 10;
    log_debug("%s num %d nSchemeID  %d  cycleTime  %d  greenRationId  %d  offset  %d  phaseTurnId  %d \n ",__func__,num,
                gSignalControlpara->stScheme[num - 1].nSchemeID,
                gSignalControlpara->stScheme[num - 1].nCycleTime,
                gSignalControlpara->stScheme[num - 1].nGreenSignalRatioID,
                gSignalControlpara->stScheme[num - 1].nOffset,
                gSignalControlpara->stScheme[num - 1].nPhaseTurnID);
	return 1;
}

/*****************************************************************************
 �� �� ��  : getTimeBasedActionTableInfo
 ��������  : ��ȫ�ֽṹָ���л�ö�������Ϣ
 �������  : stTimeBasedActionTable *pstTimeBasedActionTable  
             int num                                          
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getTimeBasedActionTableInfo(stTimeBasedActionTable *pstTimeBasedActionTable,int num)
{		//��ȡ�����ļ�����

	memset((void *)pstTimeBasedActionTable,0,sizeof(stTimeBasedActionTable));
    if((!gSignalControlpara) || (num < 1) || (num > NUM_ACTION))
    {
        return 0;
    }

    int i = 0;

    pstTimeBasedActionTable->ActionTable = num;
    //��������Ҫ��ӳ��
    pstTimeBasedActionTable->ProgramNo = (gSignalControlpara->stAction[num - 1].nSchemeID+2)/3;

    for(i = 0; i < 3; i++)
    {
        pstTimeBasedActionTable->AssistFunction[i] = ((gSignalControlpara->stAction[num - 1].cAuxiliary)>>i)&0x01;
    }

    for(i = 0; i < 8; i++)
    {
        pstTimeBasedActionTable->SpecialFunction[i] = ((gSignalControlpara->stAction[num - 1].cSpecialFun)>>i)&0x01;
    }

	return 1;
}

/*****************************************************************************
 �� �� ��  : setTimeBasedActionTableInfo
 ��������  : ��WEB���õĶ�������Ϣ���浽ȫ�ֽṹ��ָ���С�
 �������  : stTimeBasedActionTable *pstTimeBasedActionTable  
             int num                                          
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setTimeBasedActionTableInfo(stTimeBasedActionTable *pstTimeBasedActionTable,int num)
{
    if((!gSignalControlpara) || (num < 1) || (num > NUM_ACTION))
    {
        return 0;
    }
    int i = 0;

    memset(&gSignalControlpara->stAction[num - 1],0,sizeof(ActionItem));
    gSignalControlpara->stAction[num - 1].nActionID = pstTimeBasedActionTable->ActionTable;
    gSignalControlpara->stAction[num - 1].nSchemeID = pstTimeBasedActionTable->ProgramNo;

    gSignalControlpara->stAction[num - 1].cAuxiliary = 0;
    gSignalControlpara->stAction[num - 1].cSpecialFun = 0;
    
    for(i = 0; i < 3; i++)
    {
        gSignalControlpara->stAction[num - 1].cAuxiliary |= (pstTimeBasedActionTable->AssistFunction[i]<<i);
    }

    for(i = 0; i < 8; i++)
    {
        gSignalControlpara->stAction[num - 1].cSpecialFun |= (pstTimeBasedActionTable->SpecialFunction[i]<<i);
    }
    

    log_debug("%s ActionId  %d  schemeId  %d \n",__func__,num,gSignalControlpara->stAction[num - 1].nSchemeID);
	return 1;
}

/*****************************************************************************
 �� �� ��  : getTimeIntervalInfo
 ��������  : ��ȫ�ֽṹָ���л��ʱ�α���Ϣ
 �������  : PTimeIntervalItem pItem  
             int id                   
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getTimeIntervalInfo(PTimeIntervalItem pItem,int id)
{		//��ȡ�����ļ�����
    if((!gSignalControlpara) || (id < 1) || (id > NUM_TIME_INTERVAL))
    {
        return 0;
    }

    int i = 0;

    for(i = 0 ; i < NUM_TIME_INTERVAL_ID ; i++)
    {
        if(gSignalControlpara->stTimeInterval[id - 1][i].nTimeID != 0)
        {
            pItem[i].nTimeIntervalID = id - 1;
            pItem[i].nTimeID = gSignalControlpara->stTimeInterval[id - 1][i].nTimeID;
            pItem[i].nActionID = gSignalControlpara->stTimeInterval[id - 1][i].nActionID;
            pItem[i].nTimeID = gSignalControlpara->stTimeInterval[id - 1][i].nTimeID;
            pItem[i].cStartTimeHour = gSignalControlpara->stTimeInterval[id - 1][i].cStartTimeHour;
            pItem[i].cStartTimeMinute= gSignalControlpara->stTimeInterval[id - 1][i].cStartTimeMinute;

            //log_debug("nActionID   %d\n",pItem[i].nActionID);
        }

    }
	return 1;
}

/*****************************************************************************
 �� �� ��  : setTimeIntervalInfo
 ��������  : ����WEB���õ�ʱ�α���Ϣ���浽ȫ�ֽṹ��ָ���С�
 �������  : PTimeIntervalItem pItem  
             int id                   
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setTimeIntervalInfo(PTimeIntervalItem pItem,int id)
{
    if((!gSignalControlpara) || (id < 1) || (id > NUM_TIME_INTERVAL))
    {
        return 0;
    }

    int i = 0;

    memset((void *)gSignalControlpara->stTimeInterval[id - 1],0,sizeof(TimeIntervalItem)*NUM_TIME_INTERVAL_ID);

    for(i = 0 ; i < NUM_TIME_INTERVAL_ID ; i++)
    {
        gSignalControlpara->stTimeInterval[id - 1][i].nTimeIntervalID = pItem[i].nTimeIntervalID ;
        gSignalControlpara->stTimeInterval[id - 1][i].nTimeID = pItem[i].nTimeID ;
        gSignalControlpara->stTimeInterval[id - 1][i].nActionID = pItem[i].nActionID;
        gSignalControlpara->stTimeInterval[id - 1][i].cStartTimeHour = pItem[i].cStartTimeHour;
        gSignalControlpara->stTimeInterval[id - 1][i].cStartTimeMinute= pItem[i].cStartTimeMinute;

        if(pItem[i].nTimeIntervalID  != 0)
        {
            log_debug("%s  nTimeIntervalID %d  nTimeID  %d  nActionID  %d  cStartTimeHour  %d  cStartTimeMinute  %d",__func__,
                gSignalControlpara->stTimeInterval[id - 1][i].nTimeIntervalID,
                gSignalControlpara->stTimeInterval[id - 1][i].nTimeID,
                gSignalControlpara->stTimeInterval[id - 1][i].nActionID,
                gSignalControlpara->stTimeInterval[id - 1][i].cStartTimeHour,
                gSignalControlpara->stTimeInterval[id - 1][i].cStartTimeMinute);
        }


    }

	return 1;
}

/*****************************************************************************
 �� �� ��  : getSchedulingInfo
 ��������  : ��ȫ��ָ���л�õ��ȱ���Ϣ
 �������  : stScheduling *pstScheduling  
             int Id                       
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getSchedulingInfo(stScheduling *pstScheduling,int Id)
{		//��ȡ�����ļ�����

	memset(pstScheduling,0,sizeof(stScheduling));

    if((!gSignalControlpara) || (Id < 1) || (Id > NUM_SCHEDULE))
    {
        return 0;
    }

    pstScheduling->SchedulingNo = gSignalControlpara->stPlanSchedule[Id - 1].nScheduleID;
    pstScheduling->TimeIntervalNum = gSignalControlpara->stPlanSchedule[Id - 1].nTimeIntervalID;

    int i = 0;

    for(i = 0 ; i < 7 ; i++)
    {
        pstScheduling->WeekDay[i] = BIT(gSignalControlpara->stPlanSchedule[Id - 1].week,i+1);
    }

    for(i = 0 ; i < 12 ; i++)
    {
        pstScheduling->Month[i] = BIT(gSignalControlpara->stPlanSchedule[Id - 1].month,i+1);
    }

    for(i = 0 ; i < 31 ; i++)
    {
        pstScheduling->Day[i] = BIT(gSignalControlpara->stPlanSchedule[Id - 1].day,i+1);
    }
	return 1;
}

/*****************************************************************************
 �� �� ��  : setSchedulingInfo
 ��������  : �����ȱ���Ϣ���浽ȫ�ֽṹ��ָ���С�
 �������  : stScheduling *pstScheduling  
             int Id                       
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setSchedulingInfo(stScheduling *pstScheduling,int Id)
{
    if((!gSignalControlpara) || (Id < 1) || (Id > NUM_SCHEDULE))
    {
        return 0;
    }

    memset(&gSignalControlpara->stPlanSchedule[Id - 1],0,sizeof(PlanScheduleItem));
    gSignalControlpara->stPlanSchedule[Id - 1].nScheduleID = pstScheduling->SchedulingNo;
    gSignalControlpara->stPlanSchedule[Id - 1].nTimeIntervalID = pstScheduling->TimeIntervalNum;

    int i = 0;

    for(i = 0 ; i < 7 ; i++)
    {
        gSignalControlpara->stPlanSchedule[Id - 1].week |= (pstScheduling->WeekDay[i] << (i+1));
    }

    for(i = 0 ; i < 12 ; i++)
    {
        gSignalControlpara->stPlanSchedule[Id - 1].month |= (pstScheduling->Month[i] << (i+1));
    }

    for(i = 0 ; i < 31 ; i++)
    {
        gSignalControlpara->stPlanSchedule[Id - 1].day |= (pstScheduling->Day[i] << (i+1));

    }

    log_debug("%s  scheduleId  %d   timeIntervalId  %d  Week  0x%x  Month  0x%x  Day  0x%x\n",__func__,gSignalControlpara->stPlanSchedule[Id - 1].nScheduleID,
                            gSignalControlpara->stPlanSchedule[Id - 1].nTimeIntervalID,
                            gSignalControlpara->stPlanSchedule[Id - 1].week,
                            gSignalControlpara->stPlanSchedule[Id - 1].month,
                            gSignalControlpara->stPlanSchedule[Id - 1].day);
	return 1;
}

/*****************************************************************************
 �� �� ��  : getOverlappingInfo
 ��������  : ��ȫ�ֽṹ��ָ���л�ø�����λ����Ϣ
 �������  : stOverlapping *pstOverlapping  
             int Id                         
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getOverlappingInfo(stOverlapping *pstOverlapping,int Id)
{		//��ȡ�����ļ�����

	memset(pstOverlapping,0,sizeof(stOverlapping));
    if((!gSignalControlpara) || (Id < 1) || (Id > NUM_FOLLOW_PHASE))
    {
        return 0;
    }

    pstOverlapping->FollowPhase = Id ;

    int i = 0;

    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        if(gSignalControlpara->stFollowPhase[Id - 1].nArrayMotherPhase[i] != 0)
        {
            pstOverlapping->ParentPhase[gSignalControlpara->stFollowPhase[Id - 1].nArrayMotherPhase[i] - 1] = 1 ;
        }

    }

	return 1;
}

/*****************************************************************************
 �� �� ��  : setOverlappingInfo
 ��������  : ��WEB���õĸ�����λ����Ϣ���浽ȫ�ֽṹָ���С�
 �������  : stOverlapping *pstOverlapping  
             int Id                         
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setOverlappingInfo(stOverlapping *pstOverlapping,int Id)
{
    if((!gSignalControlpara) || (Id < 1) || (Id > NUM_FOLLOW_PHASE))
    {
        return 0;
    }

    memset(&gSignalControlpara->stFollowPhase[Id - 1],0,sizeof(FollowPhaseItem));
    gSignalControlpara->stFollowPhase[Id - 1].nFollowPhaseID = Id ;

    int i = 0;
    int index = 0;

    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        if(pstOverlapping->ParentPhase[i] == 1)
        {
            gSignalControlpara->stFollowPhase[Id - 1].nArrayMotherPhase[index] = i+1;
            index++;
        }

    }
    log_debug("%s   nFollowPhase Id  %d  mother[0] %d monther[1]  %d \n",__func__,Id,
                                gSignalControlpara->stFollowPhase[Id - 1].nArrayMotherPhase[0],
                                gSignalControlpara->stFollowPhase[Id - 1].nArrayMotherPhase[1]);
	return 1;
}

/*****************************************************************************
 �� �� ��  : getCoordinateInfo
 ��������  : ���ļ��л�ȡЭ����λ��Ϣ(δ����)
 �������  : stCoordinate *pstCoordinate  
             char *strConfigPath          
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getCoordinateInfo(stCoordinate *pstCoordinate,char *strConfigPath)
{		//��ȡ�����ļ�����
	const char * section = "Coordinate";
	char value[BUF_SIZE] = "";
	memset(pstCoordinate,0,sizeof(stCoordinate));
	if(!read_profile_string(section,"ControlModel",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstCoordinate->ControlModel = atoi(value);
	}
	if(!read_profile_string(section,"ManualMethod",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstCoordinate->ManualMethod = atoi(value);
	}
	if(!read_profile_string(section,"CoordinationMode",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstCoordinate->CoordinationMode = atoi(value);
	}
	if(!read_profile_string(section,"CoordinateMaxMode",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstCoordinate->CoordinateMaxMode = atoi(value);
	}
	if(!read_profile_string(section,"CoordinateForceMode",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstCoordinate->CoordinateForceMode = atoi(value);
	}
	return 1;
}

/*****************************************************************************
 �� �� ��  : saveCoordinate2Ini
 ��������  : ��WEB���õ�Э����λ��Ϣ���浽�����ļ���
 �������  : stCoordinate stCoordinateEx  
             char * strConfigPath         
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int saveCoordinate2Ini(stCoordinate stCoordinateEx,char * strConfigPath)
{
	const char * section = "Coordinate";
	char tmpstr[48] = { 0 };
	itoa(stCoordinateEx.ControlModel,tmpstr,10);
	if(write_profile_string(section, "ControlModel",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.ManualMethod,tmpstr,10);
	if(write_profile_string(section, "ManualMethod",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.CoordinationMode,tmpstr,10);
	if(write_profile_string(section, "CoordinationMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.CoordinateMaxMode,tmpstr,10);
	if(write_profile_string(section, "CoordinateMaxMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stCoordinateEx.CoordinateForceMode,tmpstr,10);
	if(write_profile_string(section, "CoordinateForceMode",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	return 1;
}


/*****************************************************************************
 �� �� ��  : getVehicleDetectorInfo
 ��������  : �������ļ��л�ó�����������Ϣ
 �������  : stVehicleDetector *pstVehicleDetector  
             char *strConfigPath                    
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �޸Ĵ�ȫ�ֲ������ȡ���������Ϣ
*****************************************************************************/
int getVehicleDetectorInfo(stVehicleDetector *pstVehicleDetector,int id)
{		
	memset(pstVehicleDetector,0,sizeof(stVehicleDetector));

    if((!gSignalControlpara) || (id < 1) || (id > MAX_VEHICLEDETECTOR_COUNT))
    {
        return 0;
    }

    if(gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorCallPhase == 0)
    {
        gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorExtend = 3;
        gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorNoActivity = 30;
        gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorMaxPresence = 30;

    }
    pstVehicleDetector->DetectorNo = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorNumber;
    pstVehicleDetector->RequestPhase = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorCallPhase;
    pstVehicleDetector->SwitchPhase = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorSwitchPhase;
    pstVehicleDetector->Delay = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorDelay;
    pstVehicleDetector->FailureTime = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorFailTime;
    pstVehicleDetector->QueueLimit = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorQueueLimit;
    pstVehicleDetector->NoResponseTime = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorNoActivity;
    pstVehicleDetector->MaxDuration = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorMaxPresence;
    pstVehicleDetector->Extend = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorExtend;
    pstVehicleDetector->MaxVehicle = gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorErraticCounts;
    pstVehicleDetector->Flow = (gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions & 0x01);
    pstVehicleDetector->Occupancy = ((gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions >> 1) & 0x01);
    pstVehicleDetector->YellowInterval = ((gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions >> 2) & 0x01);
    pstVehicleDetector->RedInterval = ((gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions >> 3) & 0x01);
    pstVehicleDetector->ProlongGreen = ((gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions >> 4) & 0x01);
    pstVehicleDetector->AccumulateInitial = ((gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions >> 5) & 0x01);
    pstVehicleDetector->Queue = ((gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions >> 6) & 0x01);
    pstVehicleDetector->Request = ((gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions >> 7) & 0x01);

	return 1;
}

/*****************************************************************************
 �� �� ��  : saveVehicleDetector2Ini
 ��������  : ��������������Ϣ���浽�����ļ��С�
 �������  : stVehicleDetector stVehicleDetectorEx  
             char * strConfigPath                   
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����������Ϣ���浽ȫ�ֽṹ����
*****************************************************************************/
int setVehicleDetectorInfo(stVehicleDetector *pstVehicleDetector,int id)
{
    if((!gSignalControlpara) || (id < 1) || (id > MAX_VEHICLEDETECTOR_COUNT))
    {
        return 0;
    }

    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorNumber = pstVehicleDetector->DetectorNo;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorCallPhase = pstVehicleDetector->RequestPhase;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorSwitchPhase = pstVehicleDetector->SwitchPhase;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorDelay = pstVehicleDetector->Delay;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorFailTime = pstVehicleDetector->FailureTime;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorQueueLimit = pstVehicleDetector->QueueLimit;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorNoActivity = pstVehicleDetector->NoResponseTime;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorMaxPresence = pstVehicleDetector->MaxDuration;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorExtend = pstVehicleDetector->Extend;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorErraticCounts = pstVehicleDetector->MaxVehicle;

    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |= pstVehicleDetector->Flow;
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |= (pstVehicleDetector->Occupancy << 1);
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |=(pstVehicleDetector->YellowInterval << 2);
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |= (pstVehicleDetector->RedInterval << 3);
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |= (pstVehicleDetector->ProlongGreen << 4);
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |= (pstVehicleDetector->AccumulateInitial << 5);
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |= (pstVehicleDetector->Queue << 6);
    gSignalControlpara->AscVehicleDetectorTable[id - 1].byVehicleDetectorOptions |= (pstVehicleDetector->Request << 7);
    
    log_debug("%s DetectorNo %d ,RequestPhase %d  Delay %d  FailureTime %d\n",__func__,pstVehicleDetector->DetectorNo,
                                        pstVehicleDetector->RequestPhase,pstVehicleDetector->Delay,pstVehicleDetector->FailureTime);
    
	return 1;
}

/*****************************************************************************
 �� �� ��  : getPedestrianInfo
 ��������  : �������ļ��л�����˼������Ϣ
 �������  : stPedestrian *pstPedestrian  
             char *strConfigPath          
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getPedestrianInfo(stPedestrian *pstPedestrian,int id)
{		
	memset(pstPedestrian,0,sizeof(stPedestrian));

    if((!gSignalControlpara) || (id < 1) || (id > MAX_PEDESTRIANDETECTOR_COUNT))
    {
        return 0;
    }

    pstPedestrian->DetectorNo = gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorNumber;
    pstPedestrian->RequestPhase = gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorCallPhase;
    pstPedestrian->NoResponseTime = gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorNoActivity;
    pstPedestrian->MaxDuration = gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorMaxPresence;
    pstPedestrian->InductionNumber = gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorErraticCounts;

	return 1;
}

/*****************************************************************************
 �� �� ��  : savePedestrian2Ini
 ��������  : �����˼������Ϣ���浽�����ļ���
 �������  : stPedestrian stPedestrianEx  
             char * strConfigPath         
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int setPedestrianInfo(stPedestrian *pstPedestrianEx,int id)
{
    if((!gSignalControlpara) || (id < 1) || (id > MAX_PEDESTRIANDETECTOR_COUNT))
    {
        return 0;
    }

    gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorNumber = pstPedestrianEx->DetectorNo;
    gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorCallPhase = pstPedestrianEx->RequestPhase;
    gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorNoActivity = pstPedestrianEx->NoResponseTime;
    gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorMaxPresence = pstPedestrianEx->MaxDuration;
    gSignalControlpara->AscPedestrianDetectorTable[id - 1].byPedestrianDetectorErraticCounts = pstPedestrianEx->InductionNumber;
    
	return 1;
}

/*****************************************************************************
 �� �� ��  : getFaultConfigInfo
 ��������  : �������ļ��л�ȡ����������Ϣ
 �������  : stFaultConfig *pstFaultConfig  
             char *strConfigPath            
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getFaultConfigInfo(stFaultConfig *pstFaultConfig,char *strConfigPath)
{		//��ȡ�����ļ�����
	const char * section = "FaultConfig";
	char value[BUF_SIZE] = "";
	memset(pstFaultConfig,0,sizeof(stFaultConfig));
	if(!read_profile_string(section,"ControlRecord",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultConfig->ControlRecord = atoi(value);
	}
	if(!read_profile_string(section,"LogRecord",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultConfig->LogRecord = atoi(value);
	}
#if 0
	if(!read_profile_string(section,"CommunicatRecord",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultConfig->CommunicatRecord = atoi(value);
	}
	if(!read_profile_string(section,"DetectorRecord",value,BUF_SIZE,"",strConfigPath))
	{
		//��д�ļ��쳣
		return  0;
	}
	else
	{
		pstFaultConfig->DetectorRecord = atoi(value);
	}
#endif
	return 1;
}

/*****************************************************************************
 �� �� ��  : saveFaultConfig2Ini
 ��������  : ������������Ϣ���浽�ļ��С�
 �������  : stFaultConfig stFaultConfigEx  
             char * strConfigPath           
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int saveFaultConfig2Ini(stFaultConfig stFaultConfigEx,char * strConfigPath)
{
	const char * section = "FaultConfig";
	char tmpstr[48] = { 0 };
	itoa(stFaultConfigEx.ControlRecord,tmpstr,10);
	if(write_profile_string(section, "ControlRecord",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultConfigEx.LogRecord,tmpstr,10);
	if(write_profile_string(section, "LogRecord",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
#if 0
	itoa(stFaultConfigEx.CommunicatRecord,tmpstr,10);
	if(write_profile_string(section, "CommunicatRecord",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
	itoa(stFaultConfigEx.DetectorRecord,tmpstr,10);
	if(write_profile_string(section, "DetectorRecord",tmpstr,strConfigPath) == 0)
	{
		return 0;
	}
#endif
	return 1;
}

/*****************************************************************************
 �� �� ��  : getTreeDynamicParameter
 ��������  : ��ȫ�ֽṹ��ָ���л�ȡ��̬��
 �������  : stTreeDynamicPara *pstTreeDynamicPara  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int getTreeDynamicParameter(stTreeDynamicPara *pstTreeDynamicPara)
{		//��ȡ�����ļ�����
	memset(pstTreeDynamicPara,0,sizeof(stTreeDynamicPara));

    if(!gSignalControlpara)
    {
        return 0;
    }

    int i = 0;
    int j = 0;

    //��λ������
    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        if(gSignalControlpara->stPhase[i].nPhaseID != 0)
        {
            pstTreeDynamicPara->addCount++;
        }
    }

    //ͨ��������
    for(i = 0 ; i < NUM_CHANNEL ; i++)
    {
        if(gSignalControlpara->stChannel[i].nChannelID != 0)
        {
            pstTreeDynamicPara->addChannel++;
        }

    }

    //���űȱ�����
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION ; i++)
    {
        for(j = 0 ; j <  NUM_PHASE; j++)
        {
            if(gSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationID != 0)
            {
                pstTreeDynamicPara->addSplit++;
                break;
            }
        }
    }

    //����������
    for(i = 0 ; i < NUM_SCHEME; i++)
    {
        if(gSignalControlpara->stScheme[i].nSchemeID!= 0)
        {
            pstTreeDynamicPara->addProgram++;
        }

    }

    //���������
    for(i = 0 ;  i < NUM_PHASE_TURN ; i++)
    {
        for(j = 0 ; j < 4 ; j++)
        {
            if(gSignalControlpara->stPhaseTurn[i][j].nPhaseTurnID != 0)
            {
                pstTreeDynamicPara->nPhaseTurnCount++;
                break;
            }
        }


    }
    //����������
    for(i = 0 ; i < NUM_ACTION ; i++)
    {
        if(gSignalControlpara->stAction[i].nActionID != 0)
        {
            pstTreeDynamicPara->nActionCount++;
        }
    }

    //ʱ�α�����
    for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID ; j++)
        {
            if(gSignalControlpara->stTimeInterval[i][j].nTimeID != 0)
            {
                pstTreeDynamicPara->nTimeInterval++;
                break;
            }
        }
    }

    //���ȼƻ�������
    for(i = 0 ; i < NUM_SCHEDULE ; i++)
    {
        if(gSignalControlpara->stPlanSchedule[i].nScheduleID != 0)
        {
            pstTreeDynamicPara->nScheduleCount++;
        }

    }

    //������λ����
    for(i = 0 ; i < NUM_FOLLOW_PHASE; i++)
    {
        if(gSignalControlpara->stFollowPhase[i].nFollowPhaseID != 0)
        {
            pstTreeDynamicPara->nFollowPhaseCount++;
        }

    }


	return 1;
}

/*****************************************************************************
 �� �� ��  : ClearConcurrentPhaseArrayItem
 ��������  : �ڲ����������������λ��
 �������  : SignalControllerPara *pData  
             unsigned short nPhaseId      
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ClearConcurrentPhaseArrayItem(SignalControllerPara *pData,unsigned short nPhaseId)
{
    if(!pData)
    {
        return;
    }

    int i = 0;
    int j = 0;

    for(i = 0 ; i < NUM_PHASE; i++)
    {
        if(pData->stPhase[i].nPhaseID == 0 )
        {
            continue;
        }

        for(j = 0 ; j < NUM_PHASE; j++)
        {
            if(pData->stPhase[i].byPhaseConcurrency[j] > nPhaseId)
            {
                pData->stPhase[i].byPhaseConcurrency[j] = 0;
            }
        }
    }
}


/*****************************************************************************
 �� �� ��  : saveTreeDynamicParameter
 ��������  : ���ݶ�̬�������ݾ�����Ҫ������Щ��
 �������  : stTreeDynamicPara *stTreeDynamicParaEx  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int saveTreeDynamicParameter(stTreeDynamicPara *stTreeDynamicParaEx)
{
    int i = 0;
    int j = 0;
    int k = 0;

    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        if(gSignalControlpara->stPhase[i].nPhaseID > stTreeDynamicParaEx->addCount)
        {
            gSignalControlpara->stPhase[i].nPhaseID = 0;
        }

    }

    ClearConcurrentPhaseArrayItem(gSignalControlpara,stTreeDynamicParaEx->addCount);//������λ�Ŵ��ڵ�ǰ���ģ�ͳһ��0

    for(i = 0 ; i < NUM_CHANNEL ; i++)
    {
        if(gSignalControlpara->stChannel[i].nChannelID > stTreeDynamicParaEx->addChannel)
        {
            gSignalControlpara->stChannel[i].nChannelID = 0;
        }

        if(gSignalControlpara->stChannel[i].nControllerID > stTreeDynamicParaEx->addCount)//���ͨ�����п���ԴID�Ѿ���ɾ�����򽫸ÿ���Դ��λ1
        {
            gSignalControlpara->stChannel[i].nControllerID = 1;
        }
    }

    for(i = 0 ; i < NUM_SCHEME ; i++)
    {
       // if(gSignalControlpara->stScheme[i].nSchemeID > stTreeDynamicParaEx->addProgram)
        {
         //   gSignalControlpara->stScheme[i].nSchemeID = 0;
        }

       // if(gSignalControlpara->stScheme[i].nPhaseTurnID > stTreeDynamicParaEx->nPhaseTurnCount)
        {
         //   gSignalControlpara->stScheme[i].nPhaseTurnID = 1;//����������к��д���
        }

       /// if(gSignalControlpara->stScheme[i].nGreenSignalRatioID > stTreeDynamicParaEx->addSplit)
        {
         //   gSignalControlpara->stScheme[i].nGreenSignalRatioID = 1;//
        }
    }

    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION; i++)
    {
        for(j = 0 ; j < NUM_PHASE ; j++)
        {
            if(gSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationID > stTreeDynamicParaEx->addSplit)
            {
                gSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationID = 0;

            }

        }
    }

    for(i = 0 ; i < NUM_PHASE_TURN; i++)
    {
        for(j = 0 ; j < 4; j++)
        {
            if(gSignalControlpara->stPhaseTurn[i][j].nPhaseTurnID > stTreeDynamicParaEx->nPhaseTurnCount)
            {
                 gSignalControlpara->stPhaseTurn[i][j].nPhaseTurnID= 0;
            }

            for(k = 0 ; k < 16 ; k++)//���������д��ڴ�����λ�ŵ���λ����Ѹ���λ��0
            {
                if(gSignalControlpara->stPhaseTurn[i][j].nTurnArray[k] > stTreeDynamicParaEx->addCount)
                {
                    gSignalControlpara->stPhaseTurn[i][j].nTurnArray[k] = 0;
                }
            }
        }
    }

    for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID; j++)
        {
            if(gSignalControlpara->stTimeInterval[i][j].nTimeIntervalID > stTreeDynamicParaEx->nTimeInterval)
            {
                gSignalControlpara->stTimeInterval[i][j].nTimeIntervalID = 0;

            }

            if(gSignalControlpara->stTimeInterval[i][j].nActionID > stTreeDynamicParaEx->nActionCount)
            {
                gSignalControlpara->stTimeInterval[i][j].nActionID = 1;
            }
        }
    }

    for(i = 0 ; i < NUM_ACTION; i++)
    {
        if(gSignalControlpara->stAction[i].nActionID > stTreeDynamicParaEx->nActionCount)
        {
            gSignalControlpara->stAction[i].nActionID = 0;
        }

        if(gSignalControlpara->stAction[i].nSchemeID > stTreeDynamicParaEx->addProgram)
        {
            gSignalControlpara->stAction[i].nSchemeID = 1;
        }
    }

    for(i = 0 ; i < NUM_SCHEDULE; i++)
    {
        if(gSignalControlpara->stPlanSchedule[i].nScheduleID > stTreeDynamicParaEx->nScheduleCount)
        {
            gSignalControlpara->stPlanSchedule[i].nScheduleID = 0;
        }
        if(gSignalControlpara->stPlanSchedule[i].nTimeIntervalID > stTreeDynamicParaEx->nTimeInterval)
        {
            gSignalControlpara->stPlanSchedule[i].nTimeIntervalID = 1;
        }
    }

    for(i = 0 ; i < NUM_FOLLOW_PHASE; i++)
    {
        if(gSignalControlpara->stFollowPhase[i].nFollowPhaseID > stTreeDynamicParaEx->nFollowPhaseCount)
        {
            gSignalControlpara->stFollowPhase[i].nFollowPhaseID = 0;
        }

        for(j = 0 ;j < NUM_PHASE ; j++)//������λ��������ڴ��������λ�ĺţ���Ѹø�����λ����0
        {
            if(gSignalControlpara->stFollowPhase[i].nArrayMotherPhase[j] > stTreeDynamicParaEx->addCount)
            {
                gSignalControlpara->stFollowPhase[i].nArrayMotherPhase[j] = 0;
            }
        }

    }

    log_debug("%s addCount %d  addChannel  %d  addProgram  %d  addSplit  %d   nPhaseTurnCount  %d   nTimeInterval  %d  nActionCount  %d nScheduleCount  %d  nFollowPhaseCount  %d\n",
                        __func__,
                        stTreeDynamicParaEx->addCount,
                        stTreeDynamicParaEx->addChannel,
                        stTreeDynamicParaEx->addProgram,
                        stTreeDynamicParaEx->addSplit,
                        stTreeDynamicParaEx->nPhaseTurnCount,
                        stTreeDynamicParaEx->nTimeInterval,
                        stTreeDynamicParaEx->nActionCount,
                        stTreeDynamicParaEx->nScheduleCount,
                        stTreeDynamicParaEx->nFollowPhaseCount);


	return 1;
}



