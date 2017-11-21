/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : countDown_LaiSi.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��4��2��
  ����޸�   :
  ��������   : ��Դ�ļ�ʵ������˹Э�鵹��ʱ�����ṩ�˶�ȡ���޸ġ�������˹��
               ���ļ��Ľӿڣ�ͬʱ�ṩ������˹Э��Ľӿڡ�
  �����б�   :
              ConvertData
              countDownByLaiSiProtocol
              GetNextControllerId
              GetRuningPhaseId
              PrintData
              ReadLaiSiCfgFromIni
              SetCountDownValueLaiSi
              SetCountValue
              UpdateLaiSiCfg
              WriteLaiSiCfgToIni
  �޸���ʷ   :
  1.��    ��   : 2015��4��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "countDown_LaiSi.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

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
//��˹Э���Ӧ��ת��
static unsigned char  gLaiSiConvertArray[16] = {0x02, 0x9e, 0x24, 0x0c,  0x98, 0x48, 0x40, 0x1e, 0x00, 0x08, 0x10, 0x0c0, 0x62, 0x84, 0x60, 0x70};

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static CountDownCfgLaiSi        g_CountDownCfgLaiSi;                       //ȫ�ֲ�������ŵ�����˹Э����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
static CountDownParamsLaiSi     g_CountDownParamsLaiSi[4][MAX_NUM_PHASE];    //�ֱ������������͡������������ˡ������浹��ʱ�����������Ҫÿ����и���,��λID��Ϊ�����±ꡣ

/*****************************************************************************
 �� �� ��  : PrintData
 ��������  : �����ã���ӡ���ͻ��������ݡ�
 �������  : unsigned char *nArray  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void PrintData(unsigned char *nArray)
{
    int i = 0;

    for(i = 1; i <= 48; i++)
    {
        fprintf(stderr,"0x%x ",nArray[i - 1]);
        if((i%12 == 0) && (i != 0))
        {
            fprintf(stderr,"\n");
        }
    }
    
    fprintf(stderr,"\n\n\n");
}


/*****************************************************************************
 �� �� ��  : ConvertData
 ��������  : ���ݵ���ʱʱ�䣬��ʮ���Ƶ���ʱʱ�䣬������˹Э��ת���ɷ���Ҫ��
             ��ʮλ������λ����
 �������  : unsigned char iTempPassTime[2]     �ֱ���ת���ĸ�λ��ʮλ
             unsigned char cPhaseCountDownTime  ת��ǰ�ĵ���ʱʱ�䡣
             unsigned char cPhaseColor          ��ǰ��ɫ
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void ConvertData(unsigned char iTempPassTime[2],unsigned char cPhaseCountDownTime,unsigned char cPhaseColor)
{
    iTempPassTime[0] = cPhaseCountDownTime%10;//��λ
    iTempPassTime[1] = cPhaseCountDownTime/10;//ʮλ

    //ת��
    if((iTempPassTime[0] >= 0) && (iTempPassTime[0] <= 15) && (iTempPassTime[1] >= 0) && (iTempPassTime[1] <= 15)) 
    {
        iTempPassTime[0] = gLaiSiConvertArray[iTempPassTime[0]];
        iTempPassTime[1] = gLaiSiConvertArray[iTempPassTime[1]];
    }

    if((GREEN == cPhaseColor) || (GREEN_BLINK == cPhaseColor))//1 0
    {
        iTempPassTime[0] |= 0x00; 
        iTempPassTime[1] |= 0x01; 
    }
    else if(RED == cPhaseColor)//0 1
    {
        iTempPassTime[0] |= 0x01;
        iTempPassTime[1] |= 0x00;
    }
    else if((YELLOW == cPhaseColor) || (YELLOW_BLINK == cPhaseColor))//0 0 
    {
        iTempPassTime[0] |= 0x00; 
        iTempPassTime[1] |= 0x00; 
    }
}

/*****************************************************************************
 �� �� ��  : GetNextControllerId
 ��������  : ���㵱ǰ���е���λID����һ����λ
 �������  : unsigned char cDeviceId  
             unsigned char cNowId     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static unsigned char GetNextControllerId(unsigned char cDeviceId,unsigned char cNowId)
{
    int i = 0;
    unsigned char cIndex = 0;
    
    for(i = 0; i < MAX_NUM_PHASE; i++)
    {
        if(cNowId == g_CountDownCfgLaiSi.cControllerID[cDeviceId][i])
        {
            cIndex = i;
        }
    }
    //�����ǰ����λ��������һ����������һ��ֵ��0����ô��Ҫ���ص�һ������ԴID
    if((cIndex == (MAX_NUM_PHASE - 1)) || (g_CountDownCfgLaiSi.cControllerID[cDeviceId][cIndex+1] == 0))
    {
        return g_CountDownCfgLaiSi.cControllerID[cDeviceId][0];
    }
    else
    {
        return g_CountDownCfgLaiSi.cControllerID[cDeviceId][cIndex+1];
    }
}

/*****************************************************************************
 �� �� ��  : GetRuningPhaseId
 ��������  : ����ĳ���豸��ǰ��Ҫ���е���λ��
 �������  : unsigned char cDeviceId  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static unsigned char GetRuningPhaseId(unsigned char cDeviceId)
{
    int i = 0;
    unsigned char cControllerID = 0;
    unsigned char cControllerType = g_CountDownCfgLaiSi.cControllerType[cDeviceId];

    static unsigned char cLastControllerId[MAX_NUM_COUNTDOWN] = {0};

    for(i = 0; i < MAX_NUM_PHASE; i++)
    {
        cControllerID = g_CountDownCfgLaiSi.cControllerID[cDeviceId][i];

        if(cControllerID == 0)
        {
            break;
        }

        if(g_CountDownParamsLaiSi[cControllerType-1][cControllerID - 1].cColor == GREEN)
        {
            cLastControllerId[cDeviceId] = cControllerID;

            return cControllerID;
        }
    }

    cControllerID = cLastControllerId[cDeviceId];
    //�������λ��ǰ�ǻƵƣ��һƵƵĵ���ʱ��1����ô���´θ÷���ĵ���ʱҪ��ʾ��һ����λ
    if((g_CountDownParamsLaiSi[cControllerType-1][cControllerID - 1].cColor == YELLOW)&&
        (g_CountDownParamsLaiSi[cControllerType-1][cControllerID - 1].cTime == 1))
    {
        cLastControllerId[cDeviceId] = GetNextControllerId(cDeviceId,cControllerID);
    }
    
    return cControllerID;
}
#define GET_COLOR(val)    (((val) == 1) ? "��" : ((val == 2) ? "��" : ((val == 3) ? "��" : "")))

/*****************************************************************************
 �� �� ��  : SetCountValue
 ��������  : ��������豸ID�ĵ���ʱ����ɫ��Ϣ��
 �������  : unsigned char iTempPassTime[2]  
             unsigned char cDeviceId         
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetCountValue(unsigned char iTempPassTime[2],unsigned char cDeviceId)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;
    unsigned char cPhaseId = 0;

    cPhaseId = GetRuningPhaseId(cDeviceId);
    
    cPhaseCountDownTime = g_CountDownParamsLaiSi[g_CountDownCfgLaiSi.cControllerType[cDeviceId]-1][cPhaseId - 1].cTime;
    cPhaseColor = g_CountDownParamsLaiSi[g_CountDownCfgLaiSi.cControllerType[cDeviceId]-1][cPhaseId - 1].cColor;

    //fprintf(stderr,"SetCountValue  --->cDeviceId %d, cPhaseId %d,  countdown %d  color %s\n",cDeviceId,cPhaseId,cPhaseCountDownTime,GET_COLOR(cPhaseColor));

    ConvertData(iTempPassTime,cPhaseCountDownTime,cPhaseColor);
}


/*****************************************************************************
 �� �� ��  : countDownByLaiSiProtocol
 ��������  : ��˹Э�����ʵ��
 �� �� ֵ  :                      
 �޸���ʷ  
  1.��    ��   : 2015��3��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���
����˼·:
1. ����˵��Ҫ���� a��ĺ��ʱ�䣬a��ʮλ������B,��λ������C
2. ��B��C����Լ����ת��������ת��:��0x02, 0x9e, 0x24, 0x0c,  0x98, 0x48, 0x40, 0x1e, 0x00, 0x08, 0x10, 0x0c0, 0x62, 0x84, 0x60, 0x70���ֱ��Ӧ����0 ~ 15 ���߶��롣
3. ����˵a=24s�Ļ�����ôB����2��C����4�����Ӧ��ת��ֱ���0x24 0x98,��ɫ��01
4. ʮλ�͸�λ�ֱ����ɫ����λ�����������0x24 | 0x00 , 0x98 | 0x01���������0x24 0x99
5. ��ô��������ʱ�Ƶ����ݾ���0x24 0x99
6. 0x50 0x51 0x52 0x53��ʵ�ֱ��Ӧ��IDΪ0 1 2 3�ĵ���ʱ�豸,��Щ�豸��û�ж�������֮�֣���Ҫ����ʵ���������
7. 0x60 0x70 0x80�������16λ����Ҫ��0x50�������������ͬ��ͬ��0x61 ... ֻ������������ʱ����ΪУ����ȷ��
*****************************************************************************/
void countDownByLaiSiProtocol(unsigned char *pBuf,unsigned char *pLen)
{
    int i =0;
    unsigned char iTempPassTime[2] = {0};//[0]��λ��[1]ʮλ
    unsigned char tempBuf[48] ={0x50,0xff,0xff,0x51,0xff,0xff,0x52,0xff,0xff,0x53,0xff,0xff,
                                0x60,0xff,0xff,0x61,0xff,0xff,0x62,0xff,0xff,0x63,0xff,0xff,
                                0x70,0xff,0xff,0x71,0xff,0xff,0x72,0xff,0xff,0x73,0xff,0xff,
                                0x80,0xff,0xff,0x81,0xff,0xff,0x82,0xff,0xff,0x83,0xff,0xff};

    if((NULL == pBuf) || (NULL == pLen))
    {
        return;
    }

    //�������ڳ��ҵĽ��ܣ�0x60 0x70 0x80��0x50��������ͬ����������
    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(g_CountDownCfgLaiSi.cControllerID[i][0] == 0)//���ĳ������ʱû�б����ã���ֱ�ӷ��ء�
        {
            continue;
        }
    
        SetCountValue(iTempPassTime,g_CountDownCfgLaiSi.cDeviceId[i]);
        
        tempBuf[3*i+1] = iTempPassTime[1];
        tempBuf[3*i+2] = iTempPassTime[0];//0x50

        tempBuf[3*i+13] = iTempPassTime[1];
        tempBuf[3*i+14] = iTempPassTime[0];//0x60

        tempBuf[3*i+25] = iTempPassTime[1];
        tempBuf[3*i+26] = iTempPassTime[0];//0x70

        tempBuf[3*i+37] = iTempPassTime[1];
        tempBuf[3*i+38] = iTempPassTime[0];//0x80
    }
   // PrintData(tempBuf);
    memcpy(pBuf,tempBuf,48);
    *pLen = 48;//ʹ����˹Э��Ļ�����Ҫͨ��485����48�ֽڵ�����
}


/*****************************************************************************
 �� �� ��  : SetCountDownValueLaiSi
 ��������  : ����˹Э�鵹��ʱ������ֵ�������ڵ���ʱ�ӿ��е��á�
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetCountDownValueLaiSi(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend)
{
    if(pCountDownParamsSend == NULL)
    {
        return;
    }

    int i = 0;
    
    for(i = 0; i < 16; i++)
    {
        //����������ʱ
        g_CountDownParamsLaiSi[MOTOR-1][i].cControllerID = i+1;
        g_CountDownParamsLaiSi[MOTOR-1][i].cColor = pCountDownParamsSend->stVehPhaseCountingDown[i][0];
        g_CountDownParamsLaiSi[MOTOR-1][i].cTime = pCountDownParamsSend->stVehPhaseCountingDown[i][1];

        //���˵���ʱ
        g_CountDownParamsLaiSi[PEDESTRIAN-1][i].cControllerID = i+1;
        g_CountDownParamsLaiSi[PEDESTRIAN-1][i].cColor = pCountDownParamsSend->stPedPhaseCountingDown[i][0];
        g_CountDownParamsLaiSi[PEDESTRIAN-1][i].cTime = pCountDownParamsSend->stPedPhaseCountingDown[i][1];

        //������λ����ʱ
        g_CountDownParamsLaiSi[FOLLOW-1][i].cControllerID = i+1;
        g_CountDownParamsLaiSi[FOLLOW-1][i].cColor = pCountDownParamsSend->ucOverlap[i][0];
        g_CountDownParamsLaiSi[FOLLOW-1][i].cTime = pCountDownParamsSend->ucOverlap[i][1];

       // if(i == 0)
        //ERR("%d ,  %s,  %d\n",g_CountDownParamsLaiSi[MOTOR-1][i].cControllerID,
          //                      GET_COLOR(g_CountDownParamsLaiSi[MOTOR-1][i].cColor),
            //                    g_CountDownParamsLaiSi[MOTOR-1][i].cTime);
    }
}


/*****************************************************************************
 �� �� ��  : ReadLaiSiCfgFromIni
 ��������  : �������ļ��ж�ȡ��˹����ʱ�Ƶ�������Ϣ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadLaiSiCfgFromIni()
{
	parse_start(CFG_NAME_LAISI);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        sprintf(tmpstr,"LaiSiDevice_%d",i);

        g_CountDownCfgLaiSi.cDeviceId[i] = i;
        get_more_value(tmpstr,"cControllerID",g_CountDownCfgLaiSi.cControllerID[i],MAX_NUM_PHASE);
        g_CountDownCfgLaiSi.cControllerType[i] = get_one_value(tmpstr,"cControllerType");

        ERR("===>  id %d, cControllerId  %d, type %d \n",g_CountDownCfgLaiSi.cDeviceId[i],g_CountDownCfgLaiSi.cControllerID[i][0],g_CountDownCfgLaiSi.cControllerType[i]);
    }

    parse_end();
}

/*****************************************************************************
 �� �� ��  : WriteLaiSiCfgToIni
 ��������  : ����˹����ʱ��Ϣ���浽�����ļ���
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteLaiSiCfgToIni()
{
	parse_start(CFG_NAME_LAISI);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        sprintf(tmpstr,"LaiSiDevice_%d",i);

        add_one_key(tmpstr,"cControllerType",g_CountDownCfgLaiSi.cControllerType[i]);
        add_more_key(tmpstr,"cControllerID",g_CountDownCfgLaiSi.cControllerID[i],MAX_NUM_PHASE);
    }

    parse_end();
}

/*****************************************************************************
 �� �� ��  : UpdateLaiSiCfg
 ��������  : ������˹������Ϣ
 �������  : CountDownCfgLaiSi *pData  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void UpdateLaiSiCfg(CountDownCfgLaiSi *pData)
{
    if(!pData)
    {
        return;
    }

    memcpy(&g_CountDownCfgLaiSi,pData,sizeof(g_CountDownCfgLaiSi));
}

