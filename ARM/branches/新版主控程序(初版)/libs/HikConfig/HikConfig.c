/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : HikConfig.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��2��
  ����޸�   :
  ��������   : ���ļ���ɶ������ļ����ж�ȡ������У��Ľӿ�
  �����б�   :
              ArrayToInt
              checkPhaseTrunInRing
              DateTimeCmp
              find_next_index
              GetCircleTime
              IsArrayRepeat
              IsBarrierGreenSignalRationEqual
              IsItemInCharArray
              IsItemInShortArray
              IsPhaseContinuousInPhaseTurn
              isPhaseInArray
              IsSignalControllerParaIdLegal
              IsSignalControlparaLegal
              LoadDataFromCfg
              log_error
  �޸���ʷ   :
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

  2.��    ��   : 2014��12��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����µĽṹ�壬�޸���ؽӿ�
******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "parse_ini.h"
#include "HikConfig.h"
#include <stdarg.h>
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define COL_DEF "\033[m"
#define COL_RED "\033[0;32;31m"
#define COL_GRE "\033[0;32;32m"
#define COL_BLU "\033[0;32;34m"
#define COL_YEL "\033[1;33m"

#define DEG(fmt,...) fprintf(stdout,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
char *gErrorContent = NULL;//��ȫ�ֱ����洢������Ϣ���������߽��з���

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*****************************************************************************
 �� �� ��  : log_error
 ��������  : ��ӡ������Ϣ�ӿڣ����д������ݻ� ���浽ȫ�ֻ��������ⲿ�ӿڿ�ֱ
             �Ӳ鿴�����ݡ�
 �������  : const char* format  
             ...                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���
  2.��    ��   : 2014��12��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �޸Ļ�������СΪ1024�ֽ�
  3.��    ��   : 2014��12��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����strcat�����ַ�����Ϊһ��sprintf���������ַ���	
*****************************************************************************/
static void log_error(const char* format, ...)
{
	char buff[1024] = {0};
    if(NULL == gErrorContent)
    {
        gErrorContent = calloc(1024,1);

    }
	sprintf(buff,"%s HikConfig library error: %s <ERROR>  ",COL_BLU,COL_YEL);
	va_list argptr;
	va_start(argptr, format);

    if(NULL != gErrorContent)
    {
    	memset(gErrorContent,0,1024);
    	vsnprintf(gErrorContent,1024,format,argptr);
    }

	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//��buff��λ�������ƶ�strlen(buff)�ֽڣ���ǰ����24�ֽ�������ŵ�ǰʱ�䣬��������2�ֽڵĿո������ָ����ݺ�ʱ��
	va_end(argptr);
	strcat(buff,COL_DEF);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);

}

/*****************************************************************************
 �� �� ��  : DateTimeCmp
 ��������  : �Ƚ���������ʱ��֮���Ƿ����ص�
 �������  : PPlanScheduleItem item_1  
             PPlanScheduleItem item_2  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static int DateTimeCmp(PPlanScheduleItem item_1,PPlanScheduleItem item_2)
{
    int i = 1;
    int j = 1;
    int k = 1;
    
    for(i = 1 ; i <= 12 ; i++)
    {
        if((1 == BIT(item_1->month,i)) && (1 == BIT(item_2->month,i)))
        {
            for(j = 1 ; j <= 31; j++)
            {
                if((1 == BIT(item_1->day,j)) && (1 == BIT(item_2->day,j)))
                {
                    return 1;
                }
            }
            for(k = 1; k <= 7; k++)
            {
                if((1 == BIT(item_1->week,k)) && (1 == BIT(item_2->week,k)))
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}

/*****************************************************************************
 �� �� ��  : GetCircleTime
 ��������  : �������űȺź�����ŵõ�������У����Ų�Ϊ0�����ű�ʱ��֮��
 �������  : SignalControllerPara *pSignalControlpara  
             unsigned char nGreenSignalRationId        
             unsigned char nPhaseTurnId                
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static int GetCircleTime(SignalControllerPara *pSignalControlpara,unsigned char nGreenSignalRationId,unsigned char nPhaseTurnId)
{

    int i = 0; 
    int j = 0;

    int temp = 0;

    for(i = 0 ; i < 4; i++)//��ѯ��
    {
        if(pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nPhaseTurnID == 0)//�ҵ���ʵ�����ݵĻ�
        {
            continue;

        }    
        
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            if(pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] == 0)//��ѯһ������ֱ�ӽ���
            {
                return temp;
            }

            
            temp += pSignalControlpara->stGreenSignalRation[nGreenSignalRationId - 1][pSignalControlpara->stPhaseTurn[nPhaseTurnId - 1][i].nTurnArray[j] - 1].nGreenSignalRationTime;

        }

    }

    return  temp;
}



/*****************************************************************************
 �� �� ��  : IsBarrierGreenSignalRationEqual
 ��������  : �ж�һ��������������ű�ʱ���Ƿ���ͬ
 �������  : SignalControllerPara *pSignalControlpara  
             unsigned char *cArray                     
             int len                                   
             unsigned char nCircleId                   
 �������  : ��
 
 �޸���ʷ      :
  1.��    ��   : 2014��8��8��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static int IsBarrierGreenSignalRationEqual(SignalControllerPara *pSignalControlpara,unsigned char *cArray,int len,unsigned char nCircleId)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int m = 0;
    
    int temp = 0;
    int temp2 = 0;
    int temp3 = 0;

    unsigned char cTempArray[4][NUM_PHASE];//�ĸ�����ÿ�������num_phase����λ

    memset(cTempArray,0,sizeof(cTempArray));

    //��������λ���飬���ջ��Ĵ�С˳��
    for(i = 0 ; i < len; i++)
    {
        if(cArray[i] == 0)
        {
            break;
        }

        temp = pSignalControlpara->stPhase[cArray[i] - 1].nCircleID;//�ҵ�����

        cTempArray[temp - 1][cArray[i] - 1] = cArray[i];

       // DEG("===>  cTeampArray   %d\n",cTempArray[temp - 1][cArray[i] - 1]);
    }

    //һ�����ڵ���λ�Ĳ�����λ���ű�֮�ͣ�Ӧ�ú͸û�����λ�����ű�֮�����
    for(m = 0 ; m < NUM_GREEN_SIGNAL_RATION; m++)//��ѯ���űȱ��ҵ�ĳ����λ�����ű�ʱ��
    {
        temp = 0;
        temp2 = 0;
        temp3 = 0;
        
        for(i = 0 ; i < 4; i++)//��ѯ��
        {
            temp = 0;
            temp2 = 0;
            
            for(j = 0 ;  j < NUM_PHASE; j++)//��ѯ���������λ
            {
                if(cTempArray[i][j] == 0)
                {
                    continue;
                }

    			if(pSignalControlpara->stGreenSignalRation[m][cTempArray[i][j] - 1].nGreenSignalRationID == 0)
    			{
    				continue;
    			}

    			temp += pSignalControlpara->stGreenSignalRation[m][cTempArray[i][j] - 1].nGreenSignalRationTime;//������λ�ţ��ҵ�����λ�����ű�ʱ��

    			if(temp2 == 0)
    			{
    				for(k = 0; k < NUM_PHASE; k++)//��ѯ����λ�Ĳ�����λ
    				{
    					temp3 = pSignalControlpara->stPhase[cTempArray[i][j] - 1].byPhaseConcurrency[k];//�ҵ�����λ��Ӧ�Ĳ�����λ
    					if(temp3 == 0)
    					{
    						break;
    					}

    					if(pSignalControlpara->stPhase[temp3 - 1].nCircleID == nCircleId)//����ò�����λ�Ļ��ź��β���ͬ�����������ű�֮��
    					{
    						temp2 += pSignalControlpara->stGreenSignalRation[m][temp3 - 1].nGreenSignalRationTime;//�󲢷���λ�����ű�֮��
    					}
    				    //DEG("====>temp %d temp2   %d   phase  %d\n",temp,temp2,temp3);
    				}
    			}
                //DEG("=========  temp  %d\n",temp);

    		}
    		
            if(temp != temp2)
            {
                return  1;
            }
        }
    }
    return 0;
}

/*****************************************************************************
 �� �� ��  : IsItemInShortArray
 ��������  : �ж�ĳ�����Ƿ���һ��short�������С�
 �������  : unsigned short *array  
             int length             
             int val                
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static int IsItemInShortArray(unsigned short *array,int length,int val)
{
	if(!array)
	{
		return False;
	}

	int i = 0;
	for(i = 0 ; i < length ; i++)
	{
		if(array[i] == val)
		{
			return TRUE;
		}

	}

	return FALSE;
}

/*****************************************************************************
 �� �� ��  : IsItemInCharArray
 ��������  : �ж�һ��char�������Ƿ���һ��char�������С�
 �������  : unsigned char *array   
             unsigned short length  
             unsigned short val     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static int IsItemInCharArray(unsigned char *array,unsigned short length,unsigned short val)
{
	if(!array)
	{
		return FALSE;
	}

	unsigned short i = 0;
	for(i = 0 ; i < length ; i++)
	{
		if(array[i] == val)
		{
			return i+1;
		}

	}
	return FALSE;

}

/*****************************************************************************
 �� �� ��  : IsPhaseContinuousInPhaseTurn
 ��������  : �ж�������е������Ƿ��ղ�����λ��ָ���Ĳ���˳��
 �������  : SignalControllerPara *pSignalControlpara  
             unsigned char nCircleId                   
             unsigned char *cArray                     
             int len                                   
 �������  : ��
 
 �޸���ʷ      :
  1.��    ��   : 2014��8��8��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static int IsPhaseContinuousInPhaseTurn(SignalControllerPara *pSignalControlpara,unsigned char nCircleId,unsigned char *cArray,int len)
{
    int i = 0;
    int j = 0;

    unsigned short sTemp = 0;
    unsigned short sTemp2 = 0;
    unsigned char num = 0;
    //�������ݵĸ���
    for(i = 0 ; i < len; i++)
    {
       // DEG("===>   %d  :   %d  len %d\n",cArray[i],pSignalControlpara->stPhase[cArray[i] - 1].nCircleID,len);
        if((cArray[i] != 0) && (pSignalControlpara->stPhase[cArray[i] - 1].nCircleID == nCircleId))
        {
            sTemp2++;
        }
    }

    for(i = 0 ; i < NUM_PHASE_TURN; i++)
    {
        sTemp = pSignalControlpara->stPhaseTurn[i][nCircleId -1].nPhaseTurnID ;
        if(sTemp == 0)
        {
            continue;
        }

        for(j = 0 ; j < NUM_PHASE; j++)
        {
            sTemp = pSignalControlpara->stPhaseTurn[i][nCircleId -1].nTurnArray[j];
            if(sTemp == 0)
            {
                break;
            }
           // DEG("===>  stemp   %d\n",sTemp);
            if(IsItemInCharArray(cArray,len, sTemp) > 0)
            {
                num++;

                if(num == sTemp2)
                {
                    return 1;
                }
            }
            else
            {
                num = 0;
            }

        }
    }
   // DEG("===> circleId   %d  cArray[0]   %d  sTemp2  %d  num  %d\n",nCircleId,cArray[0],sTemp2,num);
    return 0;
}

/*****************************************************************************
 �� �� ��  : IsArrayRepeat
 ��������  : �ж������Ƿ����ظ�Ԫ��
 �������  : unsigned char *cArray  
             int len                
 �������  : ��
 
 �޸���ʷ      :
  1.��    ��   : 2014��8��8��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static int IsArrayRepeat(unsigned char *cArray,int len)
{
    int i = 0;
    int j = 0;

    unsigned short cTemp = 0;
        
    for(i = 0 ; i < len ; i++)
    {
        cTemp = cArray[i];

        if(cTemp == 0)
        {
            continue;
        }

        for(j = i+1 ; j < len ; j++)
        {
            if(cTemp == cArray[j])
            {
                return 1;
            }
        }
    }

    return 0;
}

/*****************************************************************************
 �� �� ��  : IsSignalControllerParaIdLegal
 ��������  : ����������������������жϲ������±��Ƿ���������
 �������  : SignalControllerPara *pSignalControlpara  
             unsigned short *nPhaseArray               
             unsigned short *nFollowPhaseArray         
 �������  : ��
 
 �޸���ʷ      :
  1.��    ��   : 2014��8��8��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����µĽṹ�壬����У��


  3.��    ��   : 2015��2��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : �޸��жϸ����Ƿ�Ϊ�յķ���
*****************************************************************************/
static int IsSignalControllerParaIdLegal(SignalControllerPara *pSignalControlpara,unsigned short *nPhaseArray,unsigned short *nFollowPhaseArray)
{
    int i = 0;
    int j = 0;
    
    unsigned short sTemp = 0;
    unsigned short sTemp2 = 0;

    //��λ�����±��Ƿ����λ����ͬ
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        nPhaseArray[i] = pSignalControlpara->stPhase[i].nPhaseID;
        if((pSignalControlpara->stPhase[i].nCircleID != 0) &&(nPhaseArray[i] != (i+1)))
        {
            log_error("%s:%d stPhase[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);
            
            return ERROR_SUBSCRIPT_PHASE;
        }
    }

    //������λ�����±��Ƿ����λ����ͬ
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        //������λ�ظ�
        if(IsArrayRepeat(pSignalControlpara->stPhase[i].byPhaseConcurrency,NUM_PHASE) == 1)
        {
            log_error("%s:%d stConcurrentPhase[%d]  has repeat items \n",__func__,__LINE__,i+1);
            return ERROR_REPEATE_CONCURRENT_PHASE;
        }

    }

    //�����    ������������ظ�
    for(i = 0 ; i < NUM_PHASE_TURN; i++)
    {
        for(j = 0 ; j < NUM_RING_COUNT; j++)
        {
            sTemp = pSignalControlpara->stPhaseTurn[i][j].nPhaseTurnID;
            sTemp2 = pSignalControlpara->stPhaseTurn[i][j].nCircleID;

            if(sTemp2 == 0)
            {
                continue;
            }

            if(sTemp != (i+1))
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d]   subscript is not equal to nPhaseTurnID  \n ",__func__,__LINE__,i+1,j+1);

                return ERROR_SUBSCRIPT_PHASE_TURN;
            }

            if(sTemp2 != (j+1))
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d] subscript is not equal to nCircleID\n ",__func__,__LINE__,i+1,j+1);

                return ERROR_SUSCRIPT_PHASE_TURN_CIRCLE;
            }
            
            if(IsArrayRepeat(pSignalControlpara->stPhaseTurn[i][j].nTurnArray,16) == 1)
            {
                log_error("%s:%d   stPhaseTurn[%d] Circle[%d]  has repeat item \n ",__func__,__LINE__,i+1,j+1);
                return ERROR_REPEATE_PHASE_TURN;
            }
        }
    }

    

    //���űȱ�
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION; i++)
    {
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            sTemp = pSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationID;
            sTemp2 = pSignalControlpara->stGreenSignalRation[i][j].nPhaseID;
            
            if((sTemp2 != 0) && (sTemp != (i+1)))
            {
                log_error("%s:%d stGreenSignalRation[%d] : nPhaseId[%d]   nGreenSignalRationID  subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);
    
                return ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION;
            }
            
            if((sTemp2 != 0) && (sTemp2 != (j+1)))
            {
                log_error("%s:%d stGreenSignalRation[%d] : nPhaseId[%d]    nPhaseID  subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);
    
                return ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION_PHASE;
            }        

         }
    }

    //Channel 
    for(i = 0 ; i < NUM_CHANNEL; i++)
    {
        sTemp = pSignalControlpara->stChannel[i].nChannelID;

        if((pSignalControlpara->stChannel[i].nControllerID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stChannel[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);

            return ERROR_SUBSCRIPT_CHANNEL;
        }
    }

    //stScheme
    for(i = 0 ; i < NUM_SCHEME; i++)
    {
        sTemp = pSignalControlpara->stScheme[i].nSchemeID;

        if((pSignalControlpara->stScheme[i].nGreenSignalRatioID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stScheme[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);


            return ERROR_SUBSCRIPT_SCHEME;
        }
    }

    //stAction
    for(i = 0 ; i < NUM_ACTION; i++)
    {
        sTemp = pSignalControlpara->stAction[i].nActionID;

        if((pSignalControlpara->stAction[i].nSchemeID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stScheme[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);


            return ERROR_SUBSCRIPT_ACTION;
        }
    }

    //stTimeInterval
    for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID; j++)
        {
            sTemp = pSignalControlpara->stTimeInterval[i][j].nTimeIntervalID ;
            sTemp2 = pSignalControlpara->stTimeInterval[i][j].nTimeID;
            
            if((pSignalControlpara->stTimeInterval[i][j].nActionID != 0) && (sTemp != (i+1)))
            {
                log_error("%s:%d stTimeInterval[%d]:nTimeID[%d] nTimeIntervalID subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);


                return ERROR_SUBSCRIPT_TIMEINTERVAL;
            }
            if((pSignalControlpara->stTimeInterval[i][j].nActionID != 0) && (sTemp2 != (j+1)))
            {
                log_error("%s:%d stTimeInterval[%d]:nTimeID[%d] nTimeID subscript is not equal to ID \n",__func__,__LINE__,i+1,j+1);


                return ERROR_SUBSCRIPT_TIMEINTERVAL_TIME;
            }

            
        }

    }

    //stPlanSchedule
    for(i = 0 ; i < NUM_SCHEDULE; i++)
    {
        sTemp = pSignalControlpara->stPlanSchedule[i].nScheduleID;

        if((pSignalControlpara->stPlanSchedule[i].nTimeIntervalID != 0) && (sTemp != (i+1)))
        {
            log_error("%s:%d stPlanSchedule[%d]  subscript is not equal to ID \n",__func__,__LINE__,i+1);


            return ERROR_SUBSCRIPT_SCHEDULE;
        }
    }
    
    //FollowPhase
    for(i = 0 ; i < NUM_FOLLOW_PHASE ; i++)
    {
        nFollowPhaseArray[i] = pSignalControlpara->stFollowPhase[i].nFollowPhaseID;

        if((pSignalControlpara->stFollowPhase[i].nArrayMotherPhase[0] != 0 ) && (nFollowPhaseArray[i] != (i+1)))
        {
            log_error("%s:%d stFollowPhase[%d]  subscript is not equal to ID \n ",__func__,__LINE__,i+1);

            return ERROR_SUBSCRIPT_FOLLOW_PHASE;
        }

        if(IsArrayRepeat(pSignalControlpara->stFollowPhase[i].nArrayMotherPhase,NUM_PHASE) == 1)
        {
            log_error("%s:%d stFollowPhase[%d]  has repeat items \n ",__func__,__LINE__,i+1);
            return ERROR_REPEATE_FOLLOW_PHASE;
        }

    }


    return 0;
}

/*****************************************************************************
 �� �� ��  : find_next_index
 ��������  : �ӵ�ǰ�����������ҳ���һ��������ʼ��λ�����������е�����
 �������  : SignalControllerPara *pSignalControlpara  �����ò����Ľṹ��ָ��
             unsigned char *phaseArray                 ������������
             unsigned short index                      ��ʼ������ֵ
 �� �� ֵ  : �ҵ��򷵻�����ֵ���Ҳ����򷵻�0
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static int find_next_index(SignalControllerPara *pSignalControlpara,unsigned char *phaseArray, unsigned short index)
{
	int i,  ret = 0;
	unsigned short phaseId = phaseArray[index];
	PPhaseItem item = &pSignalControlpara->stPhase[phaseId - 1];
	
	if (item->nPhaseID != phaseId) 
	{
		return index + 1;
	}
	phaseId = item->byPhaseConcurrency[0];
	item = &pSignalControlpara->stPhase[phaseId - 1];
	
	for (i = index + 1; i < NUM_PHASE; i++) 
	{
		if (phaseArray[i] == 0) 
		{
			ret = 0;
			break;
		}
		if (IsItemInCharArray(item->byPhaseConcurrency,NUM_PHASE, phaseArray[i]) == FALSE) 
		{
			ret = i;
			break;
		}
	}
	return ret;
}

/*****************************************************************************
 �� �� ��  : checkPhaseTrunInRing
 ��������  : ����������ÿ�����е������Ƿ񲢷���ȷ
 �������  : SignalControllerPara *pSignalControlpara  �����ò����Ľṹ��ָ��
             unsigned short phaseTurn                  ������
 �� �� ֵ  : �����������ÿ�����е�����ȫ��������ȷ�򷵻�True����֮����False
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static int checkPhaseTrunInRing(SignalControllerPara *pSignalControlpara,unsigned short phaseTurn)
{
	PPhaseTurnItem item = NULL;
	int i = 0;
	unsigned short fatherPhase = 0;
	int indexs[4] = {0};	//���ÿ�����ж�Ӧ���ϵ���ʼ��λ������
	unsigned short ringPhase[4] = {0};	//���ÿ�����ж�Ӧ���ϵ���ʼ��λ�������ж���Щ���Ƿ��ǲ�����ϵ
	

	while (1) 
	{	//��ȡÿ�����ж�Ӧ���ϵ���ʼ��λ���������ringPhase�У�Ȼ����Ѱ��һ������ÿ�����е���ʼ��λ������
		for (i = 0; i < 4; i++) 
		{
			item = &pSignalControlpara->stPhaseTurn[phaseTurn - 1][i];
			if (item->nPhaseTurnID != phaseTurn || item->nCircleID != (i + 1))
				continue;
			ringPhase[i] = item->nTurnArray[indexs[i]];
			indexs[i] = find_next_index(pSignalControlpara,item->nTurnArray, indexs[i]);
		}
		//�ж�ringPhase��������ȡ����λ�Ƿ񲢷�
		//printf("DEBUG: ringPhase = {%d, %d, %d, %d}\n", ringPhase[0], ringPhase[1], ringPhase[2], ringPhase[3]);
		//printf("DEBUG: indexs = {%d, %d, %d, %d}\n", indexs[0], indexs[1], indexs[2], indexs[3]);
		for (i = 0; i < 4; i++) 
		{
			if (ringPhase[i] == 0)
				continue;
			if (fatherPhase == 0) 
			{
				fatherPhase = ringPhase[i];
			} 
			else 
			{
				if (IsItemInCharArray(pSignalControlpara->stPhase[fatherPhase - 1].byPhaseConcurrency, NUM_PHASE, ringPhase[i]) == FALSE)
				{
#if 0
					printf("DEBUG: fatherPhase = %d, correspond = {%d, %d}, ringPhase[%d] = %d\n",
							fatherPhase,
							pSignalControlpara->stPhase[fatherPhase - 1].byPhaseConcurrency[0],
							pSignalControlpara->stPhase[fatherPhase - 1].byPhaseConcurrency[1],
							i, ringPhase[i]);
#endif
					return FALSE;
				}
			}
			
		}
		fatherPhase = 0;
		//��ÿ�����ж�Ӧ���ϵ���ʼ��λ�������ĸ���Ϊ0ʱ��˵���жϽ���
		if (indexs[0] == 0 && indexs[1] == 0 && indexs[2] == 0 && indexs[3] == 0)
		    break;
	}
	return TRUE;
}


/*****************************************************************************
 �� �� ��  : IsSignalControlparaLegal
 ��������  : ȫ���źſ��Ʋ����Ϸ��Լ��
 �������  : SignalControllerPara *pSignalControlpara  

1.  ������������±��Ƿ��ID��ͬ;
2.  ������λ����λ�Ƿ����;
3.  ������λ���Ƿ���ڲ�����ͻ: ͬһ�����ڵ���λ���ܲ�������λ�źͻ����Ƿ�����λ���Ӧ;
4.  ��������λ����Ƿ����;
5.  ���ź���λ����Ƿ��Ӧ;
6.  ���űȱ���ĳ��λ�����ű�ʱ���Ƿ�ȸ���λ������ʱ�䡢�Ƶ�ʱ�䡢ȫ��ʱ��֮��Ҫ�󣬻��߱��������ʱ�䡢���˷���ʱ��֮�ʹ�;
7.  ͨ�����п���Դ�Ƿ����;
8.  �����������űȺż��������Ƿ����;
9.  �������з������Ƿ����;
10. ʱ�α��ж������Ƿ����;
11. ���ȱ���ʱ�α���Ƿ����;
12. ���ȱ��в�ͬ����֮���Ƿ�����ص�����;
13. ������λ�е�ĸ��λ�Ƿ����;
14. ����Դ��������
15. ������������ظ�
16. ������λ��������λ���ظ�
17. ������λ�Ƿ����÷����߼�
18. ���в�����λ��ʱ��������Ƿ�������ȷ
19. ������λ���ű��Ƿ�����Ҫ��  ���Ҫ�ŵ�����ʱ��飬��Ϊ�Ҳ�֪��ĳ����λ�ľ������ű�ʱ���Ƕ���
 
 �޸���ʷ      :
  1.��    ��   : 2014��8��6��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int IsSignalControlparaLegal(SignalControllerPara *pSignalControlpara)//�Ϸ��Ļ�������0�����򷵻ش�����
{
    int i = 0;
    int j = 0;
    int k = 0;
    
    unsigned short sTemp = 0;
    unsigned short sTemp2 = 0;

    unsigned short nPhaseArray[NUM_PHASE] = {0};//�ܹ�����λ����
    unsigned short nFollowPhaseArray[NUM_FOLLOW_PHASE] = {0};//������λ����

	SignalControllerPara zero;

    if(!pSignalControlpara)
    {
        log_error("%s  pSignalControlpara  is null  \n",__func__);
        
        return ERROR_NULL_POINTER;
    }

	memset(&zero, 0, sizeof(SignalControllerPara));
	if (memcmp(pSignalControlpara, &zero, sizeof(SignalControllerPara)) == 0) {	//˵��û��������Ϣ
		log_error("You haven't configed! Please config it.\n");
		return ERROR_NOT_CONFIG_INFORMATION;
	}

    /*1.  ������������±��Ƿ��ID��ͬ;*/
    k = IsSignalControllerParaIdLegal(pSignalControlpara,nPhaseArray,nFollowPhaseArray);
    if(k > 0)
    {
        return k;
    }

 
    /*�ṹ���ڲ��߼��Ϸ��Լ��*/

    /*2.  ������λ����λ�Ƿ����;*/
    /*3.  ������λ���Ƿ���ڲ�����ͻ: ͬһ�����ڵ���λ���ܲ�������λ�źͻ����Ƿ�����λ���Ӧ;*/
    /*�����Ƿ����λ���еĲ�ƥ��*/
     /*������λ�Ƿ����÷����߼�*/
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        sTemp = pSignalControlpara->stPhase[i].nPhaseID;
        if(pSignalControlpara->stPhase[i].nCircleID == 0)
        {
            continue;
        }
        for(j = 0 ;j < NUM_PHASE ; j++)
        {
            sTemp = pSignalControlpara->stPhase[i].byPhaseConcurrency[j];
            sTemp2 = pSignalControlpara->stPhase[i].nPhaseID;
            if(sTemp == 0)
            {
                continue;
            }
            //phase is not exist
            if(IsItemInShortArray(nPhaseArray,NUM_PHASE,sTemp) == 0)
            {
                log_error("%s:%d stConcurrentPhase  phase  %d is not exist \n",__func__,__LINE__,sTemp);
                return ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE;
            }

            //phase is in the same circle
            if(pSignalControlpara->stPhase[sTemp - 1].nCircleID == pSignalControlpara->stPhase[i].nCircleID)
            {
                log_error("%s:%d stConcurrentPhase  nPhaseId %d :  ConcurrentPase  %d is in the same circle   \n",__func__,__LINE__,pSignalControlpara->stPhase[i].nPhaseID,sTemp);
                return ERROR_SAME_CIRCLE_CONCURRENT_PHASE;
            }

            //����λ�Ĳ�����λ��û�и���λ
            if(IsItemInCharArray(pSignalControlpara->stPhase[sTemp - 1].byPhaseConcurrency,NUM_PHASE, sTemp2) == 0)
            {
                log_error("%s:%d stConcurrentPhase[%d]  phase  %d is not in it's child array \n",__func__,__LINE__,i+1,sTemp2);
                return ERROR_FATHER_NOT_EXIST_CONCURRENT_PHASE;
            }

            //����λ����������Ƿ�����      
            if(IsPhaseContinuousInPhaseTurn(pSignalControlpara, pSignalControlpara->stPhase[sTemp - 1].nCircleID, pSignalControlpara->stPhase[i].byPhaseConcurrency, NUM_PHASE) == 0)
            {
                log_error("%s:%d stConcurrentPhase[%d]  phase is not  continuous in it's turn array ,circle Id is %d   %d \n",__func__,__LINE__,i+1,pSignalControlpara->stPhase[i].nCircleID,sTemp);
                return ERROR_CHILD_NOT_CONTINUOUS_CONCURRENT_PHASE;
            }

            //������У�����һ������ű�֮�ͱ������   �ŵ�����ʱ��Ӳ���
            if(IsBarrierGreenSignalRationEqual(pSignalControlpara,pSignalControlpara->stPhase[i].byPhaseConcurrency, NUM_PHASE,pSignalControlpara->stPhase[i].nCircleID) == 1)
            {
                log_error("%s:%d stConcurrentPhase  Barrier GreenSignalRation is error ,please check it again \n",__func__,__LINE__);
                return ERROR_BARRIER_CONCURRENT_PHASE;
             }
        }
    }

    /*4.  ��������λ����Ƿ����;*/
    /*5.  ���ź���λ����Ƿ��Ӧ;*/
    for(i = 0 ; i < NUM_PHASE_TURN ; i++)
    {
        for(k = 0 ; k < NUM_RING_COUNT; k++)
        {
            if(pSignalControlpara->stPhaseTurn[i][k].nCircleID == 0)
            {
                continue;
            }
        
            for(j = 0 ; j < NUM_PHASE ; j++)
            {
                sTemp = pSignalControlpara->stPhaseTurn[i][k].nTurnArray[j];

                if(sTemp == 0)
                {
                    continue;
                }

                if(pSignalControlpara->stPhase[sTemp - 1].nCircleID == 0)
                {
                    log_error("%s:%d  stPhaseTurn nPhaseTurnID[%d]:Circle[%d]   stPhase  %d is not exist \n ",__func__,__LINE__,i+1,k+1,sTemp);
                    return ERROR_NOT_EXIST_PHASE_TURN_PHASE;
                }

                /*���ź���λ����Ƿ��Ӧ;*/
                if(pSignalControlpara->stPhase[sTemp - 1].nCircleID != pSignalControlpara->stPhaseTurn[i][k].nCircleID)
                {
                    log_error("%s:%d  stPhaseTurn  nPhaseTurnID[%d]:Circle[%d]  nPhase %d  nCircleID  is not equal to stPhase \n ",__func__,__LINE__,i+1,k+1,sTemp);
                    return ERROR_NOT_EQUAL_PHASE_TURN_CIRCLE;
                }
            }

        }

        if(checkPhaseTrunInRing(pSignalControlpara,i+1) == FALSE)
        {
            log_error("%s:%d  stPhaseTurn  phase is not  correct in its turn array ",__func__,__LINE__);
            return ERROR_NOT_CORRECT_PHASE_TURN;
        }
    }

    /*6.  ���űȱ���ĳ��λ�����ű�ʱ���Ƿ�ȸ���λ������ʱ�䡢�Ƶ�ʱ�䡢ȫ��ʱ��֮��Ҫ�󣬻��߱��������ʱ�䡢���˷���ʱ��֮�ʹ�;*/
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION ; i++)
    {
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            sTemp = pSignalControlpara->stGreenSignalRation[i][j].nPhaseID;

            if(sTemp == 0)
            {
                continue;
            }
            
            sTemp2 = pSignalControlpara->AscSignalTransTable[sTemp - 1].nGreenLightTime + pSignalControlpara->stPhase[sTemp - 1].nYellowTime +  pSignalControlpara->stPhase[sTemp - 1].nAllRedTime;
            if(pSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationTime < sTemp2)
            {
                log_error("%s:%d  stGreenSignalRation  %d  is lower than sTemp2  Moto  %d\n",__func__,__LINE__,sTemp,sTemp2);
                return ERROR_SPLIT_LOW_MOTO_GREEN_SIGNAL_RATION;
            }
        }
    }

    /*7.  ͨ�����п���Դ�Ƿ����;*/
     /* ����Դ��������*/
    for(i = 0 ; i < NUM_CHANNEL ; i++)
    {
        sTemp = pSignalControlpara->stChannel[i].nControllerID ;

        if(sTemp == 0)
        {
            continue;
        }

        if((pSignalControlpara->stChannel[i].nControllerType == MOTOR ) || (pSignalControlpara->stChannel[i].nControllerType == PEDESTRIAN))
        {
            if(IsItemInShortArray(nPhaseArray,NUM_PHASE,sTemp) == 0)
            {
                log_error("%s:%d  stChannel[%d]   controller id is not exist  \n ",__func__,__LINE__,i+1);
                return ERROR_NOT_EXIST_SOURCE_CHANNEL;
            }
        }
        else if (pSignalControlpara->stChannel[i].nControllerType == FOLLOW)
        {
            if(IsItemInShortArray(nFollowPhaseArray,NUM_FOLLOW_PHASE,sTemp) == 0)
            {
                log_error("%s:%d  stChannel[%d]   controller id is not exist  \n ",__func__,__LINE__,i+1);
                return ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL;
            }

        }
        else if(OTHER == pSignalControlpara->stChannel[i].nControllerType)
        {
            //to do
        }
        else
        {
            log_error("%s:%d  stChannel[%d]   controller nControllerType is not exist  \n ",__func__,__LINE__,i+1);
            return 30;
        }
    }
    
    /*8.  �����������űȺż��������Ƿ����;*/
    for(i = 0 ; i < NUM_SCHEME; i++)
    {
        sTemp = pSignalControlpara->stScheme[i].nSchemeID;

        if(sTemp == 0)
        {
            continue;
        }

        sTemp = pSignalControlpara->stScheme[i].nPhaseTurnID;
        sTemp2 =  pSignalControlpara->stScheme[i].nGreenSignalRatioID;

        if((sTemp <= 0) || (sTemp2 <= 0))
        {
            log_error("%s:%d   stScheme  nPhaseTurnID  %d  or nGreenSignalRatioID  is not exist\n",__func__,__LINE__,sTemp,sTemp2);
            return ERROR_ILLEGAL_SCHME;
        }
        if(pSignalControlpara->stPhaseTurn[sTemp - 1][0].nPhaseTurnID == 0)
        {
            log_error("%s:%d  stScheme[%d]  nPhaseTurnID  %d is not exist  \n  ",__func__,__LINE__,i+1,sTemp);
            return ERROR_NOT_EXIST_PHASE_TURN_SHCEME;
        }
        if(pSignalControlpara->stGreenSignalRation[sTemp2 - 1][0].nGreenSignalRationID == 0)
        {
            log_error("%s:%d  stScheme[%d]  stGreenSignalRation  %d is not exist  \n",__func__,__LINE__,i+1,sTemp2);
            return ERROR_NOT_EXIST_GREEN_SIGNAL_RATION_SCHEME;
        }
        //�������е����ڳ��Ƿ����������е������ű�ʱ��֮��
        if(GetCircleTime(pSignalControlpara,sTemp2,sTemp) != pSignalControlpara->stScheme[i].nCycleTime)
        {
            log_error("%s:%d  stScheme[%d]  cycle time is not correct  ,should be %d\n",__func__,__LINE__,i+1,GetCircleTime(pSignalControlpara,sTemp2,sTemp));
            return ERROR_CIRCLE_TIME_SCHEME;
        }
        
    }

    /*9.  �������з������Ƿ����;*/
    for(i = 0 ; i < NUM_ACTION ; i++)
    {
        sTemp = pSignalControlpara->stAction[i].nActionID;

        if(sTemp == 0)
        {
            continue;
        }

        sTemp2 = pSignalControlpara->stAction[i].nSchemeID;

        if((sTemp2 >= 251) && (sTemp2 <= 255))//���ⷽ����
        {
            continue;
        }

        if((sTemp2 < 1) || (pSignalControlpara->stScheme[sTemp2  -1].nSchemeID == 0))
        {
            log_error("%s:%d  stAction[%d]  nSchemeID  %d is not exist  \n",__func__,__LINE__,i+1,sTemp2);
            return ERROR_NOT_EXIST_SCHEME_ACTION;

        }
    }

    /*10. ʱ�α��ж������Ƿ����;*/
    for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID; j++)
        {
            sTemp = pSignalControlpara->stTimeInterval[i][j].nActionID;

            if((sTemp == 0) || ((sTemp >= 201) && (sTemp <= 204)))
            {
                continue;
            }

            if(pSignalControlpara->stAction[sTemp - 1].nActionID == 0)
            {
                log_error("%s:%d stTimeInterval[%d]:nTimeID[%d]  nActionId   %d  is not exist \n ",__func__,__LINE__,i+1,j+1,sTemp);
                return ERROR_NOT_EXIST_ACTION_TIMEINTERVAL;
            }
        }
    }

    /*11. ���ȱ���ʱ�α���Ƿ����;*/
    for(i = 0 ;  i < NUM_SCHEDULE; i++)
    {
        sTemp = pSignalControlpara->stPlanSchedule[i].nScheduleID;

        if(sTemp == 0)
        {
            continue;
        }

        sTemp2 = pSignalControlpara->stPlanSchedule[i].nTimeIntervalID;

        if((sTemp2 <= 0) || (pSignalControlpara->stTimeInterval[sTemp2 - 1][0].nTimeIntervalID == 0))
        {
            log_error("%s:%d  stPlanSchedule[%d]   nTimeIntervalID  %d is not exist  \n",__func__,__LINE__,i+1,sTemp2);
            return ERROR_NOT_EXIST_TIMEINTERVAL_SCHEDULE;
        }

    }

    /*12. ���ȱ��в�ͬ����֮���Ƿ�����ص�����;*/
    for(i = 0 ; i < NUM_SCHEDULE; i++)
    {
        for(j = i+1 ; j < NUM_SCHEDULE; j++)
        {
            //DEG("~~~~~~~   %p  %#x\n",&pSignalControlpara->stPlanSchedule[i],&pSignalControlpara->stPlanSchedule[j]);
            if(DateTimeCmp(&(pSignalControlpara->stPlanSchedule[i]), &(pSignalControlpara->stPlanSchedule[j]))== 1)
            {
                log_error("%s:%d   stPlanSchedule_%d  and   stPlanSchedule_%d overlap  \n",__func__,__LINE__,i+1,j+1);
                return ERROR_REPEATE_SCHEDULE;
            }
        }
    }

    /*13. ������λ�е�ĸ��λ�Ƿ����;*/
    for(i = 0 ; i < NUM_FOLLOW_PHASE; i++)
    {
        sTemp = pSignalControlpara->stFollowPhase[i].nFollowPhaseID;
        if(sTemp == 0)
        {
            continue;
        }
    
        for(j = 0 ; j < NUM_PHASE ; j++)
        {
            sTemp = pSignalControlpara->stFollowPhase[i].nArrayMotherPhase[j];

            if(sTemp == 0)
            {
                break;
            }
            if(IS_PHASE_INABLE(pSignalControlpara->stPhase[sTemp - 1].wPhaseOptions)== 0)
            {
                log_error("%s:%d   stFollowPhase[%d]  nPhaseID  %d  is not exist  \n ",__func__,__LINE__,i+1,sTemp);
                return ERROR_NOT_EXIST_MOTHER_PHASE_FOLLOW_PHASE;
            }

        }
    }

    //========================>>>>>>>>>>>>>>>  ����1����λ���е���λ��������ڲ�����λ��
    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        sTemp = pSignalControlpara->stPhase[i].nPhaseID;
        if(IS_PHASE_INABLE(pSignalControlpara->stPhase[i].wPhaseOptions)== 1)
        {
            sTemp2 = pSignalControlpara->stPhase[i].nCircleID;
            //����2,��λ���е���λ����������������
            for(j = 0 ; j < NUM_PHASE_TURN; j++)
            {   
                if(pSignalControlpara->stPhaseTurn[j][sTemp2 - 1].nTurnArray[0] == 0)
                {
                    continue;
                }
            
                if(IsItemInCharArray(pSignalControlpara->stPhaseTurn[j][sTemp2 - 1].nTurnArray,NUM_PHASE,sTemp) == 0)
                {
                    log_error("%s:%d phase %d is not in stPhaseTurnArray %d  circleId  %d\n",__func__,__LINE__,sTemp,j,sTemp2);
                    return ERROR_NOT_EXIST_PHASE_TURN_PHASE_2;
                }
            }
            //����3, ��λ�������űȱ���
            for(j = 0 ; j < NUM_GREEN_SIGNAL_RATION; j++)
            {
                if((pSignalControlpara->stGreenSignalRation[i][sTemp - 1].nGreenSignalRationID != 0)&&
                    (pSignalControlpara->stGreenSignalRation[i][sTemp - 1].nPhaseID == 0))
                {
                    log_error("%s:%d phase %d is not in stGreenSignalRation \n",__func__,__LINE__,sTemp);
                    return ERROR_NOT_EXIST_PHASE_GREEN_SIGNAL_RATION;                

                }

            }
        }
    }

    return 0;
}


/*****************************************************************************
 �� �� ��  : LoadDataFromCfg
 ��������  : �������ļ��м����������ݵ��ڴ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : unsigned
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��31��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
Boolean LoadDataFromCfg(SignalControllerPara *pSignalControlpara, const char *path)
{
    char cSection[256];
    int i = 0;
    SignalControllerPara zero;
	
	if ((NULL == pSignalControlpara) || parse_start((path == NULL) ? CFG_NAME : path) == False)
    {
        return FALSE;
    }

	memset(pSignalControlpara, 0, sizeof(SignalControllerPara));

    //���ص�Ԫ���� 
    memset(cSection,0,sizeof(cSection));
    ReadUnitPara(&(pSignalControlpara->stUnitPara));

    //������λ���� 
    ReadPhaseItem(pSignalControlpara->stPhase,pSignalControlpara->AscSignalTransTable,NUM_PHASE);

    //����ͨ������ 
    ReadChannelItem(pSignalControlpara->stChannel, NUM_CHANNEL);

    //�������űȲ��� 
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION ; i++)
    {
        ReadGreenSignalRationItem(pSignalControlpara->stGreenSignalRation[i],i,NUM_PHASE);

    }

    //To Do  �����
    for(i = 0 ; i < NUM_PHASE_TURN ; i++)
    {
        ReadPhaseTurnItem(pSignalControlpara->stPhaseTurn[i],i,4);
    }

    //���ط�������� 
    ReadSchemeItem(pSignalControlpara->stScheme,NUM_SCHEME);

    //���ض��������
    ReadActionItem(pSignalControlpara->stAction,NUM_ACTION);

    //����ʱ�α����
    for(i = 0; i < NUM_TIME_INTERVAL; i++)
    {
        ReadTimeIntervalItem(pSignalControlpara->stTimeInterval[i], i,NUM_TIME_INTERVAL_ID);
    }
    
    //���ص��Ȳ���
    ReadPlanSchedule(pSignalControlpara->stPlanSchedule,NUM_SCHEDULE);

    //���ظ������
    ReadFollowPhaseItem(pSignalControlpara->stFollowPhase,NUM_FOLLOW_PHASE);

    //
    ReadVehicleDetector(pSignalControlpara->AscVehicleDetectorTable,MAX_VEHICLEDETECTOR_COUNT);

    //
    ReadPedestrianDetector(pSignalControlpara->AscPedestrianDetectorTable,MAX_PEDESTRIANDETECTOR_COUNT);

    //��������
    parse_end();

	memset(&zero, 0, sizeof(SignalControllerPara));
	if (memcmp(pSignalControlpara, &zero, sizeof(SignalControllerPara)) == 0) {	//˵��û��������Ϣ
		log_error("config file is not exist or there is no  config information in file . \n");
		return FALSE;
	}	
	
	return TRUE;
}







