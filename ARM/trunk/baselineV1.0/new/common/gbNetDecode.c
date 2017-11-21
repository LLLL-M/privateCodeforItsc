#include "gbNetDecode.h"

extern GbConfig *gGbconfig;

extern GbObjectIdentify gObjectArray[73];

extern UInt32	gbToNtcipFlag;					//����תNTPIP��־


UInt8 nErrorMsg[3] = {0};
UInt8 nIsSendErrorMsg = 0;
int gGBMsgLen = 0;//�洢���ǹ���Э�飬ÿ�δ��������ݵĳ��ȣ���ʱ�����Σ�ʹ��ȫ�ֱ����ķ�ʽ
/*****************************************************************************
 �� �� ��  : SetErrorMsg
 ��������  : ���ô�����Ϣ������
 �������  : UInt8 nErrorStatus  
             UInt8 nErrorIndex   
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetErrorMsg(UInt8 nErrorStatus,UInt8 nErrorIndex)
{
    if(nErrorStatus <= 5)
    {
        //nErrorMsg[0] = 0x86;
        nErrorMsg[1] = nErrorStatus;
        nErrorMsg[2] = nErrorIndex;
    }
}

/*****************************************************************************
 �� �� ��  : SendErrorMsg
 ��������  : ���ʹ�����Ϣ
 �������  : int socketFd               
             struct sockaddr_in toAddr  
             UInt8 MsgTypeField         
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void SendErrorMsg(int socketFd,struct sockaddr_in toAddr,UInt8 MsgTypeField)
{
    if(nIsSendErrorMsg == 1)
    {
        nErrorMsg[0] = ((MsgTypeField & 0xf0) | 0x06);
        if(sendto(socketFd,nErrorMsg, sizeof(nErrorMsg), 0, (struct sockaddr*)&toAddr, sizeof(toAddr)) <= 0)
        {
            printf("SendErrorMsg failed to send msg \n");
        }
    }
}



/*****************************************************************************
 �� �� ��  : GBNetDataObjectCheckout
 ��������  : �����ݵĶ����������Ӷ�����кϷ���У��
 �������  : UInt8 nMsgType             ��Ϣ����
             UInt8 nObjectId            �����ʶ
             UInt8 nIndexNum            ��������
             UInt8 nChildObject         �Ӷ���
             UInt8 *nIndexArray         ��������
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��6��26��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline UInt8 GBNetDataObjectCheckout(UInt8 nMsgType,UInt32 nObjectId,UInt32 nIndexNum,UInt32 nChildObject,UInt8 *nIndexArray)
{
    int i = 0;
    nIsSendErrorMsg = 1;
    
    //��������0x81 -- 0xc9֮���ֵ
    if(nObjectId < 0x81 || nObjectId > 0xc9)
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }
    
    //�����ֶλ��Ӷ����ֶβ���ȷ�������������޷��жϣ�Ӧ��һ���������������ж�
    if((nIndexArray[0] > gObjectArray[nObjectId - 0x81].maxIndex) || ((nIndexArray[0]*nIndexArray[1]) > gObjectArray[nObjectId - 0x81].maxIndex))
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }
    if((nIndexNum == 1) && (nIndexArray[0] == 0))
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }
    //���������������0����������ֵ����0��Ҳ�Ǵ����
    if((nIndexNum == 2) && (nIndexArray[0] == 0))
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }   
    if(nChildObject > gObjectArray[nObjectId - 0x81].childObjectNum)
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }

    if(nMsgType >= 3)//�����Ϣ���Ͳ��ǲ�ѯ�����á���������Ӧ�������
    {
        return MSG_IGNORE;
    }
    nIsSendErrorMsg = 0;
    
    return MSG_OK;
}

/*****************************************************************************
 �� �� ��  : ConvertByteToInt
 ��������  : ��һ���ڴ��ַ��ת����ָ�����ȵ�����
 �������  : UInt8 *addr  
             UInt8 bytes  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline Int32 ConvertByteToInt(UInt8 *addr,UInt8 bytes)
{
    int i = 0;
    UInt32 ret = 0;

    for(i = 0; i < bytes; i++)
    {
        ret <<= 8;
        ret |= addr[i]; 
    }

    return ret;
}

/*****************************************************************************
 �� �� ��  : IsDataEmpty
 ��������  : �ж�һƬ�ڴ������Ƿ�Ϊ��
 �������  : UInt8 *addr  
             UInt8 bytes  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline UInt8 IsDataEmpty(UInt8 *addr,UInt8 bytes)
{
    int i = 0;

    for(i = 0; i < bytes; i++)
    {
        if(addr[i] != 0)
        {
            return 0;
        }
    }

    return 1;
}


/*****************************************************************************
 �� �� ��  : IsDataValid
 ��������  : У�������Ƿ���ȷ���������ȷ�Ļ���ֱ����������Ϣ
 �������  : UInt8 *pAddr      
             UInt32 nOffset    
             UInt32 size       
             UInt8 nIsCheck    
             UInt32 nMinValue  
             UInt32 nMaxValue  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline UInt32 IsDataValid(UInt8 *pAddr,UInt32 nOffset,UInt32 size,UInt8 nIsCheck,UInt32 nMinValue,UInt32 nMaxValue)
{
    UInt32 val = 0;

    if(nIsCheck == 0)
    {
        return 0xffff;
    }
    
    val = ConvertByteToInt(pAddr+nOffset,size);

    if((val < nMinValue) || (val > nMaxValue))
    {
        SetErrorMsg(SETTING_OBJECT_ERR, nOffset);
        return nOffset;
    }

    return 0xffff;
}

//�ж��Ƿ��������ֵ���������������Ϊ�������һ���ֽ���0x81-0xc9֮���ֵ����ô�Ͳ���������ֵ�򣬷���Ͱ���
//��������ֵ�򷵻�1������������ֵ�򷵻�0
static inline int IsObjectValueField(UInt8 val)
{
	if((val >= 0x81) && (val <= 0xc9))
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 �� �� ��  : SetObjectField
 ��������  : ���ö����򣬸�����������ʶ�������������Ӷ�������
 �������  : UInt32 nObjectId       
             UInt32 nChildObject    
             UInt32 nIndexNum       
             UInt8 *nIndexArray     
             UInt8 *pSendPackage    
             UInt32 *pIndexSendBuf  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void SetObjectField(UInt32 nObjectId,UInt32 nChildObject,UInt32 nIndexNum,UInt8 *nIndexArray,UInt8 *pSendPackage,UInt32 *pIndexSendBuf)
{
    UInt8 *pSendPackageData = pSendPackage;

    pSendPackageData[*pIndexSendBuf] = nObjectId;
    *pIndexSendBuf += 1;
    pSendPackageData[*pIndexSendBuf] = nChildObject;
    pSendPackageData[*pIndexSendBuf] |= (nIndexNum << 6);
    *pIndexSendBuf += 1;
    if(nIndexArray[0] != 0)
    {
        pSendPackageData[*pIndexSendBuf] = nIndexArray[0];
        *pIndexSendBuf += 1;
    }
    if((nIndexArray[1] != 0) || (nIndexNum > 1))//�������2ֵ��Ϊ0��������������1
    {
        pSendPackageData[*pIndexSendBuf] = nIndexArray[1];
        *pIndexSendBuf += 1;
    }
}

/*****************************************************************************
 �� �� ��  : ObjectDealFun
 ��������  : ���ö�������������������Ϣ�ǣ�����bitλ
 �������  : GbObjectIdentify *pObjectIdentify  
             GbOperateType flag                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void ObjectDealFun(GbObjectIdentify *pObjectIdentify,GbOperateType flag)
{
    if(pObjectIdentify->func != NULL)
    {
        pObjectIdentify->func(flag);
    }

    if(flag == GB_SET_REQ)
    {
        gbToNtcipFlag |= (1 << pObjectIdentify->ntcipUpdateBit);
    }
}

/*****************************************************************************
 �� �� ��  : DealSimpleObjectValueField
 ��������  : �Լ򵥶����ֵ����
 �������  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 nFlagIsQueryCmd              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealSimpleObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    
    pAddr += pObjectIdentify->object.baseOffset;
    size = pObjectIdentify->object.size;

    if(nFlagIsQueryCmd == 1)//����ǲ�ѯ������ȫ�ֽṹ���е����ݣ����������ͻ�����
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);
        memcpy(pSendPackage+*pIndexSendBuf,pAddr,size);
        *pIndexSendBuf += size;
    }
    else//��������������򽫽��ܻ����������ݿ�����ȫ�ֽṹ���У������кϷ���У�飬У��ʧ�ܷ���0
    {
        if(gGBMsgLen < size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }
        memcpy(pAddr+nTemp,pRecvPackage,size);//��ֵ
        if(0xffff != IsDataValid(pAddr,0,size,pObjectIdentify->object.isCheck,pObjectIdentify->object.minValue,pObjectIdentify->object.maxValue))
        {
            return 0;
        }
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
    }

    return size;
}
/*****************************************************************************
 �� �� ��  : DealSimpleTotalListObjectValueField
 ��������  : �Ա�����������ֵ����
 �������  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 nFlagIsQueryCmd              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealSimpleTotalListObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;
    UInt8 nTotalLineNum = 0;
    size = pObjectIdentify->object.size;
    pAddr += pObjectIdentify->object.baseOffset;
    if(nFlagIsQueryCmd == 1)
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);
        nTemp = *pIndexSendBuf;//�����������������
        *pIndexSendBuf += 1;

        
        for(i = 0; i < pObjectIdentify->maxIndex; i++)
        {
			//if(i == 15)
				//INFO("%x %x %x \n",*(pAddr+i*size+1),*(pAddr+i*size+2),*(pAddr+i*size+3));
            if(IsDataEmpty(pAddr+i*size+1,size-1) == 0)//ֻ������Ч������
            {
                val++;
                
                memcpy(pSendPackage+*pIndexSendBuf,pAddr+i*size,size);
                pSendPackage[*pIndexSendBuf] = (i+1);//���ﱣ��ʱ�����뱣֤�ǰ�˳��ģ������к�
                *pIndexSendBuf += size;  
            }
        }
        pSendPackage[nTemp] = val;//����������
        return size*val;//���������Ļ�����һ���ֽ����Ǳ������
    }    
    else
    {
        nTotalLineNum = ConvertByteToInt(pRecvPackageData,1);
        pRecvPackageData++;//����Ļ�����һ���ֽ���������Ҫ���Ը���

        if((gGBMsgLen - 1) < nTotalLineNum*size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }

        for(i = 0; i < nTotalLineNum; i++)
        {
            val = ConvertByteToInt(pRecvPackageData,1);//�ҵ��к�
            memcpy(pAddr+(val - 1)*pObjectIdentify->object.size,pRecvPackageData,size);
            
            for(j = 0; j < pObjectIdentify->childObjectNum; j++)
            {
                nTemp = (val - 1)*pObjectIdentify->object.size+pObjectIdentify->childObject[j].baseOffset;
                if(0xffff != IsDataValid(pAddr,nTemp,pObjectIdentify->childObject[j].size,pObjectIdentify->childObject[j].isCheck,pObjectIdentify->childObject[j].minValue,pObjectIdentify->childObject[j].maxValue))
                {
                    return 0;
                }
            }
            pRecvPackageData += size;
        }
        gbToNtcipFlag |= (1 << pObjectIdentify->ntcipUpdateBit);
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
        return (pRecvPackageData - pRecvPackage);
    }
}

/*****************************************************************************
 �� �� ��  : DealComplexTotalListObjectValueField
 ��������  : �Ը��ӱ�����������ֵ����
 �������  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 nFlagIsQueryCmd              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealComplexTotalListObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int lineNum = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;
    UInt8 nTotalLineNum = 0;
    UInt32 nIndex1 = 0;
    UInt32 nIndex2 = 0;
    
    size = pObjectIdentify->object.size;
    pAddr += pObjectIdentify->object.baseOffset;
    if(nFlagIsQueryCmd == 1)
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);
        nTemp = *pIndexSendBuf;//���������������M��N��
        *pIndexSendBuf += 2;

        for(i = 0; i < pObjectIdentify->childObject[0].maxValue; i++)
        {
            for(j = 0; j < pObjectIdentify->childObject[1].maxValue; j++)
            {
                if(IsDataEmpty(pAddr+(i*pObjectIdentify->childObject[1].maxValue+j)*size+2,size-2) == 0)//ֻ������Ч������
                {
                    val++;
                    if(i == 0)
                    {
                        lineNum++;
                    }
                    memcpy(pSendPackage+*pIndexSendBuf,pAddr+(i*pObjectIdentify->childObject[1].maxValue+j)*size,size);
                    pSendPackage[*pIndexSendBuf] = (i+1);//���ﱣ��ʱ�����뱣֤�ǰ�˳��ģ������к�
                    pSendPackage[*pIndexSendBuf+1] = (j+1);//M
                    *pIndexSendBuf += size;  
                }
            }

        }
        pSendPackage[nTemp] = val;//����������N
        pSendPackage[nTemp+1] = lineNum;//����������M
        return size*val;//���������Ļ�����һ���ֽ����Ǳ������
    }    
    else
    {
        nTotalLineNum = ConvertByteToInt(pRecvPackageData,1);
        lineNum = ConvertByteToInt(pRecvPackageData+1,1);
        pRecvPackageData += 2;//��ά��Ļ�����һ���ֽ���N���ڶ����ֽ���M

        if((gGBMsgLen - 2) < nTotalLineNum*lineNum*size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }

        for(i = 0; i < nTotalLineNum; i++)
        {
            for(j = 0; j < lineNum; j++)
            {
                nIndex1 = ConvertByteToInt(pRecvPackageData,1);//�ҵ��к�
                nIndex2 = ConvertByteToInt(pRecvPackageData+1,1);//�ҵ��к�
                memcpy(pAddr+((nIndex1 - 1)*pObjectIdentify->childObject[1].maxValue+nIndex2 - 1)*size,pRecvPackageData,size);
                
                for(k = 0; k < pObjectIdentify->childObjectNum; k++)
                {
                    nTemp = ((nIndex1 - 1)*pObjectIdentify->childObject[1].maxValue+nIndex2 - 1)*size+pObjectIdentify->childObject[k].baseOffset;
                    if(0xffff != IsDataValid(pAddr,nTemp,pObjectIdentify->childObject[k].size,pObjectIdentify->childObject[k].isCheck,pObjectIdentify->childObject[k].minValue,pObjectIdentify->childObject[k].maxValue))
                    {
                        return 0;
                    }
                }
                pRecvPackageData += size;
            }
        }
        gbToNtcipFlag |= (1 << pObjectIdentify->ntcipUpdateBit);
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
        return (pRecvPackageData - pRecvPackage);
    }
}

/*****************************************************************************
 �� �� ��  : DealSimpleListCertainLineObjectValueField
 ��������  : �Լ򵥱������ض�ĳһ�б�Ķ���ֵ����д���
 �������  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 *nIndexArray                 
             UInt8 nFlagIsQueryCmd              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealSimpleListCertainLineObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 *nIndexArray,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//�����������������Ҫ����ָ������ݿ�����pAddr�ṹ��ָ����
    UInt8 *pSendPackageData = pSendPackage;//����ǲ�ѯ��������Ҫ��pAddr�ṹ��ָ������ݿ�������ָ����

    size = pObjectIdentify->object.size;
    pAddr += pObjectIdentify->object.baseOffset;
    if(nFlagIsQueryCmd == 1)
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);
        memcpy(pSendPackageData+*pIndexSendBuf,pAddr+(nIndexArray[0] - 1)*size,size);
        *pIndexSendBuf += size;
    }
    else
    {
        if(gGBMsgLen < size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }
    
        memcpy(pAddr+(nIndexArray[0] - 1)*pObjectIdentify->object.size,pRecvPackageData,size);
        for(j = 0; j < pObjectIdentify->childObjectNum; j++)
        {
            nTemp = (nIndexArray[0] - 1)*pObjectIdentify->object.size+pObjectIdentify->childObject[j].baseOffset;
            if(0xffff != IsDataValid(pAddr,nTemp,pObjectIdentify->childObject[j].size,pObjectIdentify->childObject[j].isCheck,pObjectIdentify->childObject[j].minValue,pObjectIdentify->childObject[j].maxValue))
            {
                return 0;
            }
        }  
        gbToNtcipFlag |= (1 << pObjectIdentify->ntcipUpdateBit);
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
    }

    return size;
}
/*****************************************************************************
 �� �� ��  : DealComPlexListCertainLineObjectValueField
 ��������  : �Ը��Ӷ�ά���ĳһ�б�Ķ���ֵ��Ĵ���
 �������  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 *nIndexArray                 
             UInt8 nFlagIsQueryCmd              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealComPlexListCertainLineObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 *nIndexArray,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//�����������������Ҫ����ָ������ݿ�����pAddr�ṹ��ָ����
    UInt8 *pSendPackageData = pSendPackage;//����ǲ�ѯ��������Ҫ��pAddr�ṹ��ָ������ݿ�������ָ����
#if 0
    UInt8 nTotalLineNum = (pObjectIdentify->objectId == 0x8e) ? 48 : ((pObjectIdentify->objectId == 0xc1) ? 16 : 255);//�����ʱ�α���ôN�����48�����������16
#else
	UInt8 nTotalLineNum = pObjectIdentify->childObject[1].maxValue;
#endif
    size = pObjectIdentify->object.size;
    pAddr += pObjectIdentify->object.baseOffset;
    if(nFlagIsQueryCmd == 1)
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);

		if(nIndexArray[1] == 0)
		{
			for(i = 0; i < nTotalLineNum; i++)
			{
				if(IsDataEmpty(pAddr+((nIndexArray[0] - 1)*pObjectIdentify->childObject[1].maxValue + i)*size + 2,size-2) == 0)
				{				
					pSendPackageData[*pIndexSendBuf] = nIndexArray[0];
					*pIndexSendBuf += 1;//i

					pSendPackageData[*pIndexSendBuf] = i+1;
					*pIndexSendBuf += 1;//j
					
					memcpy(pSendPackageData+*pIndexSendBuf,pAddr+((nIndexArray[0] - 1)*pObjectIdentify->childObject[1].maxValue + i)*size + 2,size -2);
					*pIndexSendBuf += (size - 2);								
				}
	
			}
		}
		else
		{
			pSendPackageData[*pIndexSendBuf] = nIndexArray[0];
			*pIndexSendBuf += 1;//i

			pSendPackageData[*pIndexSendBuf] = nIndexArray[1];
			*pIndexSendBuf += 1;//j
			
			memcpy(pSendPackageData+*pIndexSendBuf,pAddr+((nIndexArray[0] - 1)*pObjectIdentify->childObject[1].maxValue + nIndexArray[1] - 1)*size + 2,size -2);
			*pIndexSendBuf += (size - 2);		
		}

    }
    else
    {
        if(gGBMsgLen < size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }
        
        nTemp = ((nIndexArray[0] - 1)*pObjectIdentify->childObject[1].maxValue + nIndexArray[1] - 1)*size;
        memcpy(pAddr+nTemp,pRecvPackageData,size);
        for(j = 0; j < pObjectIdentify->childObjectNum; j++)
        {
            nTemp += pObjectIdentify->childObject[j].baseOffset;
            if(0xffff != IsDataValid(pAddr,nTemp,pObjectIdentify->childObject[j].size,pObjectIdentify->childObject[j].isCheck,pObjectIdentify->childObject[j].minValue, pObjectIdentify->childObject[j].maxValue))
            {
                return 0;
            }
        }
        gbToNtcipFlag |= (1 << pObjectIdentify->ntcipUpdateBit);
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
    }  

    return size;
}
/*****************************************************************************
 �� �� ��  : DealSimpleListCertainLineItemObjectValueField
 ��������  : �򵥶�����ĳһ��Ԫ��
 �������  : GbObjectIdentify * pObjectIdentify  
             UInt8 * pRecvPackage                
             UInt8 * pSendPackage                
             UInt32 * pIndexSendBuf              
             UInt8 * nIndexArray                 
             UInt8 nFlagIsQueryCmd               
             UInt32 nChildObject                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealSimpleListCertainLineItemObjectValueField(GbObjectIdentify * pObjectIdentify, UInt8 * pRecvPackage, UInt8 * pSendPackage, UInt32 * pIndexSendBuf, UInt8 * nIndexArray, UInt8 nFlagIsQueryCmd,UInt32 nChildObject)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//�����������������Ҫ����ָ������ݿ�����pAddr�ṹ��ָ����
    UInt8 *pSendPackageData = pSendPackage;//����ǲ�ѯ��������Ҫ��pAddr�ṹ��ָ������ݿ�������ָ����

    size = pObjectIdentify->object.size;//����Ĵ�С
    pAddr += pObjectIdentify->object.baseOffset;
    nTemp = ((nIndexArray[0] - 1)*size  + pObjectIdentify->childObject[nChildObject - 1].baseOffset);
    if(nFlagIsQueryCmd == 1)
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);
        memcpy(pSendPackageData+*pIndexSendBuf,pAddr+nTemp,pObjectIdentify->childObject[nChildObject - 1].size);
        *pIndexSendBuf += pObjectIdentify->childObject[nChildObject - 1].size;
    }
    else
    {
        if(gGBMsgLen < pObjectIdentify->childObject[nChildObject - 1].size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }
        
        memcpy(pAddr+nTemp,pRecvPackageData,pObjectIdentify->childObject[nChildObject - 1].size);
        if(0xffff != IsDataValid(pAddr,nTemp,pObjectIdentify->childObject[nChildObject - 1].size,pObjectIdentify->childObject[nChildObject - 1].isCheck,pObjectIdentify->childObject[nChildObject - 1].minValue,pObjectIdentify->childObject[nChildObject - 1].maxValue))
        {
            return 0;
        } 
        gbToNtcipFlag |= (1 << pObjectIdentify->ntcipUpdateBit);
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
    }

    return pObjectIdentify->childObject[nChildObject - 1].size;
}

/*****************************************************************************
 �� �� ��  : DealComPlexListCertainLineItemObjectValueField
 ��������  : �Ը��Ӷ���ĳһ�е��ض��Ӷ�����д���
 �������  : GbObjectIdentify * pObjectIdentify  
             UInt8 * pRecvPackage                
             UInt8 * pSendPackage                
             UInt32 * pIndexSendBuf              
             UInt8 * nIndexArray                 
             UInt8 nFlagIsQueryCmd               
             UInt32 nChildObject                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealComPlexListCertainLineItemObjectValueField(GbObjectIdentify * pObjectIdentify, UInt8 * pRecvPackage, UInt8 * pSendPackage, UInt32 * pIndexSendBuf, UInt8 * nIndexArray, UInt8 nFlagIsQueryCmd,UInt32 nChildObject)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//�����������������Ҫ����ָ������ݿ�����pAddr�ṹ��ָ����
    UInt8 *pSendPackageData = pSendPackage;//����ǲ�ѯ��������Ҫ��pAddr�ṹ��ָ������ݿ�������ָ����

    size = pObjectIdentify->object.size;
    pAddr += pObjectIdentify->object.baseOffset;
    nTemp = (((nIndexArray[0] - 1)*pObjectIdentify->childObject[1].maxValue + nIndexArray[1] - 1)*size  + pObjectIdentify->childObject[nChildObject - 1].baseOffset);
    if(nFlagIsQueryCmd == 1)
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);
        memcpy(pSendPackageData+*pIndexSendBuf,pAddr+nTemp,pObjectIdentify->childObject[nChildObject - 1].size);
        *pIndexSendBuf += pObjectIdentify->childObject[nChildObject - 1].size;
    }
    else
    {
        if(gGBMsgLen < pObjectIdentify->childObject[nChildObject - 1].size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }
        
        memcpy(pAddr+nTemp,pRecvPackageData,pObjectIdentify->childObject[nChildObject - 1].size);
        if(0xffff != IsDataValid(pAddr,nTemp,pObjectIdentify->childObject[nChildObject - 1].size,pObjectIdentify->childObject[nChildObject - 1].isCheck,pObjectIdentify->childObject[nChildObject - 1].minValue,pObjectIdentify->childObject[nChildObject - 1].maxValue))
        {
            return 0;
        }
        gbToNtcipFlag |= (1 << pObjectIdentify->ntcipUpdateBit);
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
    }
    
    return pObjectIdentify->childObject[nChildObject - 1].size;
}

/*****************************************************************************
 �� �� ��  : DealObjectValueField
 ��������  : //���ݰ��Ķ���ȷ����Ҫ����������ʼ��ַ����С
             //�ɹ�����ʵ�ʶ�ȡ�ֽڸ�����ʧ�ܷ���0
 �������  : UInt8 nMsgType         
             UInt32 nObjectId       
             UInt32 nChildObject    
             UInt32 nIndexNum       
             UInt8 *nIndexArray     
             UInt8 *pRecvPackage    
             UInt8 *pSendPackage    
             UInt32 *pIndexSendBuf  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int DealObjectValueField(UInt8 nMsgType,UInt32 nObjectId,UInt32 nChildObject,UInt32 nIndexNum,UInt8 *nIndexArray,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf)
{
    GbObjectIdentify *pObjectIdentify = NULL;
    UInt8 nFlagIsQueryCmd = 0;
    
    pObjectIdentify = &gObjectArray[nObjectId - 0x81];
    nFlagIsQueryCmd = ((nMsgType == GB_QUERY_REQ) ? 1 : 0);//ֻ���ǲ�ѯ����ʱ���Ž��з��Ͱ������
    
    SetObjectField(nObjectId,nChildObject,nIndexNum,nIndexArray,pSendPackage,pIndexSendBuf);

    //���������maxIndex = 0���Ӷ�����childObjectNum = 0�Ļ�����ô�ö�����ǵ������󣬴�С�ǵ�������Ĵ�С
    if((pObjectIdentify->childObjectNum == 0) || (pObjectIdentify->maxIndex == 0))
    {
        return DealSimpleObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nFlagIsQueryCmd);
    }

    //�������������Ѿ����ϸ�if�����������㣬�������Щ������Ա�������
    //�յ������ݰ���������0���Ӷ���ֵΪ0����ô�ö����Ǳ����������������ݴ�С��������Ĵ�С
    if((nIndexNum == 0) && (nChildObject == 0))
    {
        if((nObjectId == 0xc1) || (nObjectId == 0x8e) || (nObjectId == 0x92))
        {
            return DealComplexTotalListObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nFlagIsQueryCmd);
        }
        else
        {
            return DealSimpleTotalListObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nFlagIsQueryCmd);
        }
    }

    if(nIndexNum > 0)
    {
        //�յ������ݰ�������������0���Ӷ���ֵΪ0����ô�ö����Ǳ�����ĳ��������,���ݴ�С������������Ĵ�С
        if(nChildObject == 0)
        {
            if(nIndexNum == 1)//һά��
            {
                return DealSimpleListCertainLineObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd);
            }
            else//��ά��
            {
                return DealComPlexListCertainLineObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd);
            }
        }
        //�յ������ݰ�������������0���Ӷ���ֵ����0����ô�ö����Ǳ�����ĳ�ж����ĳ�����������ݴ�С�Ǹõ�����Ĵ�С
        if(nChildObject > 0)
        {
            if(nIndexNum == 1)//һά��
            {   
                return DealSimpleListCertainLineItemObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd,nChildObject);
            }
            else//��ά��
            {
                return DealComPlexListCertainLineItemObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd,nChildObject);
            }
        }
    }

    //����ʧ��
    return 0;
}

/*****************************************************************************
 �� �� ��  : GBNetDataDecode
 ��������  : �����ݰ����н������ɹ���1��ʧ����0
 �������  : int socketFd               
             struct sockaddr_in toAddr  
             unsigned char *cNetData    
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int GBNetDataDecode(int socketFd,struct sockaddr_in toAddr,unsigned char *cNetData)
{
    int i = 0;
    int j = 0;
    int nCheckoutRes = 0;       //У��������
    int nDataIndex = 0;         //�������ݰ��ĵ�ǰλ��
    UInt8 *pData = NULL;        //�����յ������ݰ�ָ��
    UInt32  nObjectId = 0;       //�����ʶ
    UInt32 nIndexNum = 0;        //��������
    UInt32 nChildObject = 0;     //�Ӷ���
    UInt8 nIndexArray[3] = {0};//��������
    GbMsgTypeField sMsgType;
    UInt8 nArraySendBuf[1024*16] = {0};//ע���������Ŀռ���Ȼ���ܲ������ر��������ж��ģ��������¡�
    UInt32 nIndexSend = 1;//��һ���ֽ��ȱ��������������Ϣ������
    
    pData = cNetData;

    //�Ƚ�����Ϣ�����򣬻�ö���������Ϣ����
    sMsgType = *(GbMsgTypeField *)&pData[nDataIndex++];
    //�ٸ��ݶ��������������ݰ����ҵ�ÿ������ľ�������
    for(i = 0; i <= sMsgType.objectNum; i++)
    {
        memset(nIndexArray,0,sizeof(nIndexArray));
        //�ȵõ������ʶ
        nObjectId = pData[nDataIndex++];

        //�ٵõ��������������Ӷ�������
        nIndexNum = ((pData[nDataIndex] >> 6) & 0x3);//������
        nChildObject = (pData[nDataIndex] & 0x3f);  //�Ӷ���
        nDataIndex++;                               

        //���������Ŀ��Ϊ0�������������ֵ
        for(j = 0; j < nIndexNum; j++)
        {
            nIndexArray[j] = pData[nDataIndex++];
        }

        //����Ϣ���ͼ��������У��
        nCheckoutRes = GBNetDataObjectCheckout(sMsgType.operateType,nObjectId, nIndexNum, nChildObject,nIndexArray);
        if(nCheckoutRes != MSG_OK)
        {
            SetErrorMsg(nCheckoutRes,0);
            return 0;
        }

#if 0		
		//������������󣬵������ݰ�ȴ����������ֵ��Ҫ������Ϣ����̫СERROR
		if(IsObjectValueField(*(pData+nDataIndex)) == 0)
		{
			if(sMsgType.operateType == GB_SET_REQ)
            {
				SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
                nIsSendErrorMsg = 1;
                //���ʹ�����Ϣ
                return 0;
            }
            else if(sMsgType.operateType == GB_SET_NO_REPONSE)
            {
                //ֱ���˳�������������Ϣ
                nIsSendErrorMsg = 0;
                return 0;
            }
		
		}
#endif
        gGBMsgLen -= nDataIndex;//�ж�ֵ�򳤶�ʱ��Ӧ��ȥ��ǰ�����䲿�֡�
        //����Ϣ����ֵ��Ĵ���������ѯʱ�����ṹ���ֵ��䵽���ݰ�������ʱ�����ݰ����������󣬸�ֵ���ṹ����
        nCheckoutRes = DealObjectValueField(sMsgType.operateType,nObjectId,nChildObject,nIndexNum,nIndexArray,pData+nDataIndex,nArraySendBuf,&nIndexSend);

        if(nCheckoutRes == 0)//��������ʧ��
        {
            if(sMsgType.operateType == GB_SET_REQ)
            {
                nIsSendErrorMsg = 1;
                //���ʹ�����Ϣ
                return 0;
            }
            else if(sMsgType.operateType == GB_SET_NO_REPONSE)
            {
                //ֱ���˳�������������Ϣ
                nIsSendErrorMsg = 0;
                return 0;
            }
        }
        else
        {
            //����ǲ�ѯ���������ݰ��ж���ֵ��ʱ��ҲҪʹnDataIndex+1������������ȷ������һ������
            if(sMsgType.operateType == GB_QUERY_REQ)
            {
				if(IsObjectValueField(*(pData+nDataIndex)) == 1)
                	nDataIndex += nCheckoutRes;
            }
			else
			{
                nDataIndex += nCheckoutRes;
            }
        }
    }
    //������Ϣ�������ֽ�
    if(sMsgType.operateType == GB_QUERY_REQ)
    {
        sMsgType.operateType = GB_QUERY_REPONSE;
    }
    else if(sMsgType.operateType == GB_SET_REQ)
    {
        sMsgType.operateType = GB_SET_REPONSE;
    }
    nArraySendBuf[0] = *(UInt8 *)&sMsgType;
    //������Ӧ��Ϣ
    if(sendto(socketFd,nArraySendBuf, nIndexSend, 0, (struct sockaddr*)&toAddr, sizeof(toAddr)) <= 0)
    {
       printf("send data faild .\n");
    }

    return 1;
}


