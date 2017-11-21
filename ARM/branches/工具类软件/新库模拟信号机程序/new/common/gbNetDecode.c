#include "gbNetDecode.h"

extern GbConfig *gGbconfig;

extern GbObjectIdentify gObjectArray[73];

extern UInt32	gbToNtcipFlag;					//国标转NTPIP标志


UInt8 nErrorMsg[3] = {0};
UInt8 nIsSendErrorMsg = 0;
int gGBMsgLen = 0;//存储的是国标协议，每次传过来数据的长度，暂时不传参，使用全局变量的方式
/*****************************************************************************
 函 数 名  : SetErrorMsg
 功能描述  : 设置错误消息的内容
 输入参数  : UInt8 nErrorStatus  
             UInt8 nErrorIndex   
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : SendErrorMsg
 功能描述  : 发送错误消息
 输入参数  : int socketFd               
             struct sockaddr_in toAddr  
             UInt8 MsgTypeField         
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : GBNetDataObjectCheckout
 功能描述  : 对数据的对象、索引、子对象进行合法性校验
 输入参数  : UInt8 nMsgType             消息类型
             UInt8 nObjectId            对象标识
             UInt8 nIndexNum            索引个数
             UInt8 nChildObject         子对象
             UInt8 *nIndexArray         索引数组
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年6月26日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline UInt8 GBNetDataObjectCheckout(UInt8 nMsgType,UInt32 nObjectId,UInt32 nIndexNum,UInt32 nChildObject,UInt8 *nIndexArray)
{
    int i = 0;
    nIsSendErrorMsg = 1;
    
    //对象不属于0x81 -- 0xc9之间的值
    if(nObjectId < 0x81 || nObjectId > 0xc9)
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }
    
    //索引字段或子对象字段不正确，对索引个数无法判断，应加一个对索引个数的判断
    if((nIndexArray[0] > gObjectArray[nObjectId - 0x81].maxIndex) || ((nIndexArray[0]*nIndexArray[1]) > gObjectArray[nObjectId - 0x81].maxIndex))
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }
    if((nIndexNum == 1) && (nIndexArray[0] == 0))
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }
    //如果索引个数大于0，但是索引值等于0，也是错误的
    if((nIndexNum == 2) && (nIndexArray[0] == 0))
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }   
    if(nChildObject > gObjectArray[nObjectId - 0x81].childObjectNum)
    {
        return (nMsgType == GB_SET_NO_REPONSE) ? MSG_IGNORE :  MSG_TYPE_ERR;
    }

    if(nMsgType >= 3)//如果消息类型不是查询、设置、及设置无应答，则忽略
    {
        return MSG_IGNORE;
    }
    nIsSendErrorMsg = 0;
    
    return MSG_OK;
}

/*****************************************************************************
 函 数 名  : ConvertByteToInt
 功能描述  : 将一块内存地址，转换成指定长度的整型
 输入参数  : UInt8 *addr  
             UInt8 bytes  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : IsDataEmpty
 功能描述  : 判断一片内存区域是否为空
 输入参数  : UInt8 *addr  
             UInt8 bytes  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : IsDataValid
 功能描述  : 校验数据是否正确，如果不正确的话，直接填充错误消息
 输入参数  : UInt8 *pAddr      
             UInt32 nOffset    
             UInt32 size       
             UInt8 nIsCheck    
             UInt32 nMinValue  
             UInt32 nMaxValue  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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

//判断是否包含对象值域，我们这里姑且认为，如果第一个字节是0x81-0xc9之间的值，那么就不包含对象值域，否则就包含
//包含对象值域返回1，不包含对象值域返回0
static inline int IsObjectValueField(UInt8 val)
{
	if((val >= 0x81) && (val <= 0xc9))
	{
		return 0;
	}
	return 1;
}
/*****************************************************************************
 函 数 名  : SetObjectField
 功能描述  : 设置对象域，该域包括对象标识、索引个数、子对象及索引
 输入参数  : UInt32 nObjectId       
             UInt32 nChildObject    
             UInt32 nIndexNum       
             UInt8 *nIndexArray     
             UInt8 *pSendPackage    
             UInt32 *pIndexSendBuf  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
    if((nIndexArray[1] != 0) || (nIndexNum > 1))//如果索引2值不为0或者索引数大于1
    {
        pSendPackageData[*pIndexSendBuf] = nIndexArray[1];
        *pIndexSendBuf += 1;
    }
}

/*****************************************************************************
 函 数 名  : ObjectDealFun
 功能描述  : 调用对象处理函数，包括设置消息是，更新bit位
 输入参数  : GbObjectIdentify *pObjectIdentify  
             GbOperateType flag                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : DealSimpleObjectValueField
 功能描述  : 对简单对象的值域处理，
 输入参数  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 nFlagIsQueryCmd              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline int DealSimpleObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    
    pAddr += pObjectIdentify->object.baseOffset;
    size = pObjectIdentify->object.size;

    if(nFlagIsQueryCmd == 1)//如果是查询请求，则将全局结构体中的数据，拷贝到发送缓冲区
    {
        ObjectDealFun(pObjectIdentify, GB_QUERY_REQ);
        memcpy(pSendPackage+*pIndexSendBuf,pAddr,size);
        *pIndexSendBuf += size;
    }
    else//如果是设置请求，则将接受缓冲区的内容拷贝到全局结构体中，并进行合法性校验，校验失败返回0
    {
        if(gGBMsgLen < size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }
        memcpy(pAddr+nTemp,pRecvPackage,size);//赋值
        if(0xffff != IsDataValid(pAddr,0,size,pObjectIdentify->object.isCheck,pObjectIdentify->object.minValue,pObjectIdentify->object.maxValue))
        {
            return 0;
        }
        ObjectDealFun(pObjectIdentify, GB_SET_REQ);
    }

    return size;
}
/*****************************************************************************
 函 数 名  : DealSimpleTotalListObjectValueField
 功能描述  : 对表对象的整个表值域处理
 输入参数  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 nFlagIsQueryCmd              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
        nTemp = *pIndexSendBuf;//用来保存对象行数的
        *pIndexSendBuf += 1;

        
        for(i = 0; i < pObjectIdentify->maxIndex; i++)
        {
			//if(i == 15)
				//INFO("%x %x %x \n",*(pAddr+i*size+1),*(pAddr+i*size+2),*(pAddr+i*size+3));
            if(IsDataEmpty(pAddr+i*size+1,size-1) == 0)//只传递有效的数据
            {
                val++;
                
                memcpy(pSendPackage+*pIndexSendBuf,pAddr+i*size,size);
                pSendPackage[*pIndexSendBuf] = (i+1);//这里保存时，必须保证是按顺序的，传递行号
                *pIndexSendBuf += size;  
            }
        }
        pSendPackage[nTemp] = val;//对象总行数
        return size*val;//如果是整表的话，第一个字节总是表的行数
    }    
    else
    {
        nTotalLineNum = ConvertByteToInt(pRecvPackageData,1);
        pRecvPackageData++;//整表的话，第一个字节是行数，要忽略该行

        if((gGBMsgLen - 1) < nTotalLineNum*size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }

        for(i = 0; i < nTotalLineNum; i++)
        {
            val = ConvertByteToInt(pRecvPackageData,1);//找到行号
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
 函 数 名  : DealComplexTotalListObjectValueField
 功能描述  : 对复杂表对象的整个表值域处理
 输入参数  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 nFlagIsQueryCmd              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
        nTemp = *pIndexSendBuf;//用来保存对象行数M和N的
        *pIndexSendBuf += 2;

        for(i = 0; i < pObjectIdentify->childObject[0].maxValue; i++)
        {
            for(j = 0; j < pObjectIdentify->childObject[1].maxValue; j++)
            {
                if(IsDataEmpty(pAddr+(i*pObjectIdentify->childObject[1].maxValue+j)*size+2,size-2) == 0)//只传递有效的数据
                {
                    val++;
                    if(i == 0)
                    {
                        lineNum++;
                    }
                    memcpy(pSendPackage+*pIndexSendBuf,pAddr+(i*pObjectIdentify->childObject[1].maxValue+j)*size,size);
                    pSendPackage[*pIndexSendBuf] = (i+1);//这里保存时，必须保证是按顺序的，传递行号
                    pSendPackage[*pIndexSendBuf+1] = (j+1);//M
                    *pIndexSendBuf += size;  
                }
            }

        }
        pSendPackage[nTemp] = val;//对象总行数N
        pSendPackage[nTemp+1] = lineNum;//对象总行数M
        return size*val;//如果是整表的话，第一个字节总是表的行数
    }    
    else
    {
        nTotalLineNum = ConvertByteToInt(pRecvPackageData,1);
        lineNum = ConvertByteToInt(pRecvPackageData+1,1);
        pRecvPackageData += 2;//二维表的话，第一个字节是N，第二个字节是M

        if((gGBMsgLen - 2) < nTotalLineNum*lineNum*size)
        {
            SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
            return 0;
        }

        for(i = 0; i < nTotalLineNum; i++)
        {
            for(j = 0; j < lineNum; j++)
            {
                nIndex1 = ConvertByteToInt(pRecvPackageData,1);//找到行号
                nIndex2 = ConvertByteToInt(pRecvPackageData+1,1);//找到行号
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
 函 数 名  : DealSimpleListCertainLineObjectValueField
 功能描述  : 对简单表对象的特定某一行表的对象值域进行处理
 输入参数  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 *nIndexArray                 
             UInt8 nFlagIsQueryCmd              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline int DealSimpleListCertainLineObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 *nIndexArray,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//如果是设置请求，则需要将该指针的内容拷贝到pAddr结构体指针中
    UInt8 *pSendPackageData = pSendPackage;//如果是查询请求，则需要将pAddr结构体指针的内容拷贝到该指针中

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
 函 数 名  : DealComPlexListCertainLineObjectValueField
 功能描述  : 对复杂二维表的某一行表的对象值域的处理
 输入参数  : GbObjectIdentify *pObjectIdentify  
             UInt8 *pRecvPackage                
             UInt8 *pSendPackage                
             UInt32 *pIndexSendBuf              
             UInt8 *nIndexArray                 
             UInt8 nFlagIsQueryCmd              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline int DealComPlexListCertainLineObjectValueField(GbObjectIdentify *pObjectIdentify,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf,UInt8 *nIndexArray,UInt8 nFlagIsQueryCmd)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//如果是设置请求，则需要将该指针的内容拷贝到pAddr结构体指针中
    UInt8 *pSendPackageData = pSendPackage;//如果是查询请求，则需要将pAddr结构体指针的内容拷贝到该指针中
#if 0
    UInt8 nTotalLineNum = (pObjectIdentify->objectId == 0x8e) ? 48 : ((pObjectIdentify->objectId == 0xc1) ? 16 : 255);//如果是时段表，那么N最大是48，否则最大是16
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
 函 数 名  : DealSimpleListCertainLineItemObjectValueField
 功能描述  : 简单对象表的某一个元素
 输入参数  : GbObjectIdentify * pObjectIdentify  
             UInt8 * pRecvPackage                
             UInt8 * pSendPackage                
             UInt32 * pIndexSendBuf              
             UInt8 * nIndexArray                 
             UInt8 nFlagIsQueryCmd               
             UInt32 nChildObject                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline int DealSimpleListCertainLineItemObjectValueField(GbObjectIdentify * pObjectIdentify, UInt8 * pRecvPackage, UInt8 * pSendPackage, UInt32 * pIndexSendBuf, UInt8 * nIndexArray, UInt8 nFlagIsQueryCmd,UInt32 nChildObject)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//如果是设置请求，则需要将该指针的内容拷贝到pAddr结构体指针中
    UInt8 *pSendPackageData = pSendPackage;//如果是查询请求，则需要将pAddr结构体指针的内容拷贝到该指针中

    size = pObjectIdentify->object.size;//整表的大小
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
 函 数 名  : DealComPlexListCertainLineItemObjectValueField
 功能描述  : 对复杂对象某一行的特定子对象进行处理
 输入参数  : GbObjectIdentify * pObjectIdentify  
             UInt8 * pRecvPackage                
             UInt8 * pSendPackage                
             UInt32 * pIndexSendBuf              
             UInt8 * nIndexArray                 
             UInt8 nFlagIsQueryCmd               
             UInt32 nChildObject                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline int DealComPlexListCertainLineItemObjectValueField(GbObjectIdentify * pObjectIdentify, UInt8 * pRecvPackage, UInt8 * pSendPackage, UInt32 * pIndexSendBuf, UInt8 * nIndexArray, UInt8 nFlagIsQueryCmd,UInt32 nChildObject)
{
    UInt32 size = 0;
    UInt32 nTemp = 0;
    int i = 0;
    int j = 0;
    UInt32 val = 0;
    UInt8 *pAddr =  (UInt8 *)gGbconfig;
    UInt8 *pRecvPackageData = pRecvPackage;//如果是设置请求，则需要将该指针的内容拷贝到pAddr结构体指针中
    UInt8 *pSendPackageData = pSendPackage;//如果是查询请求，则需要将pAddr结构体指针的内容拷贝到该指针中

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
 函 数 名  : DealObjectValueField
 功能描述  : //根据包的对象，确定需要保存对象的起始地址及大小
             //成功返回实际读取字节个数，失败返回0
 输入参数  : UInt8 nMsgType         
             UInt32 nObjectId       
             UInt32 nChildObject    
             UInt32 nIndexNum       
             UInt8 *nIndexArray     
             UInt8 *pRecvPackage    
             UInt8 *pSendPackage    
             UInt32 *pIndexSendBuf  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline int DealObjectValueField(UInt8 nMsgType,UInt32 nObjectId,UInt32 nChildObject,UInt32 nIndexNum,UInt8 *nIndexArray,UInt8 *pRecvPackage,UInt8 *pSendPackage,UInt32 *pIndexSendBuf)
{
    GbObjectIdentify *pObjectIdentify = NULL;
    UInt8 nFlagIsQueryCmd = 0;
    
    pObjectIdentify = &gObjectArray[nObjectId - 0x81];
    nFlagIsQueryCmd = ((nMsgType == GB_QUERY_REQ) ? 1 : 0);//只有是查询请求时，才进行发送包的填充
    
    SetObjectField(nObjectId,nChildObject,nIndexNum,nIndexArray,pSendPackage,pIndexSendBuf);

    //如果索引数maxIndex = 0或子对象数childObjectNum = 0的话，那么该对象就是单个对象，大小是单个对象的大小
    if((pObjectIdentify->childObjectNum == 0) || (pObjectIdentify->maxIndex == 0))
    {
        return DealSimpleObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nFlagIsQueryCmd);
    }

    //单个对象的情况已经在上个if里面做过计算，下面的这些就是针对表对象的了
    //收到的数据包中索引数0，子对象值为0，那么该对象是表对象的整个对象，数据大小是整个表的大小
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
        //收到的数据包中索引数大于0，子对象值为0，那么该对象是表对象的某行整对象,数据大小是整个单个表的大小
        if(nChildObject == 0)
        {
            if(nIndexNum == 1)//一维表
            {
                return DealSimpleListCertainLineObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd);
            }
            else//二维表
            {
                return DealComPlexListCertainLineObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd);
            }
        }
        //收到的数据包中索引数大于0，子对象值大于0，那么该对象是表对象的某行对象的某个单对象，数据大小是该单对象的大小
        if(nChildObject > 0)
        {
            if(nIndexNum == 1)//一维表
            {   
                return DealSimpleListCertainLineItemObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd,nChildObject);
            }
            else//二维表
            {
                return DealComPlexListCertainLineItemObjectValueField(pObjectIdentify,pRecvPackage,pSendPackage,pIndexSendBuf,nIndexArray,nFlagIsQueryCmd,nChildObject);
            }
        }
    }

    //解析失败
    return 0;
}

/*****************************************************************************
 函 数 名  : GBNetDataDecode
 功能描述  : 对数据包进行解析，成功是1，失败是0
 输入参数  : int socketFd               
             struct sockaddr_in toAddr  
             unsigned char *cNetData    
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int GBNetDataDecode(int socketFd,struct sockaddr_in toAddr,unsigned char *cNetData)
{
    int i = 0;
    int j = 0;
    int nCheckoutRes = 0;       //校验参数结果
    int nDataIndex = 0;         //遍历数据包的当前位置
    UInt8 *pData = NULL;        //保存收到的数据包指针
    UInt32  nObjectId = 0;       //对象标识
    UInt32 nIndexNum = 0;        //索引个数
    UInt32 nChildObject = 0;     //子对象
    UInt8 nIndexArray[3] = {0};//索引数组
    GbMsgTypeField sMsgType;
    UInt8 nArraySendBuf[1024*16] = {0};//注意这里分配的空间仍然可能不够，特别是遇到有多个模块表的情况下。
    UInt32 nIndexSend = 1;//第一个字节先保留，用来填充消息类型域
    
    pData = cNetData;

    //先解析消息类型域，获得对象数及消息类型
    sMsgType = *(GbMsgTypeField *)&pData[nDataIndex++];
    //再根据对象数，遍历数据包，找到每个对象的具体内容
    for(i = 0; i <= sMsgType.objectNum; i++)
    {
        memset(nIndexArray,0,sizeof(nIndexArray));
        //先得到对象标识
        nObjectId = pData[nDataIndex++];

        //再得到其索引个数、子对象及索引
        nIndexNum = ((pData[nDataIndex] >> 6) & 0x3);//索引数
        nChildObject = (pData[nDataIndex] & 0x3f);  //子对象
        nDataIndex++;                               

        //如果索引数目不为0，则解析其索引值
        for(j = 0; j < nIndexNum; j++)
        {
            nIndexArray[j] = pData[nDataIndex++];
        }

        //对消息类型及对象进行校验
        nCheckoutRes = GBNetDataObjectCheckout(sMsgType.operateType,nObjectId, nIndexNum, nChildObject,nIndexArray);
        if(nCheckoutRes != MSG_OK)
        {
            SetErrorMsg(nCheckoutRes,0);
            return 0;
        }

#if 0		
		//如果是设置请求，但是数据包却不包含对象值域，要返回消息长度太小ERROR
		if(IsObjectValueField(*(pData+nDataIndex)) == 0)
		{
			if(sMsgType.operateType == GB_SET_REQ)
            {
				SetErrorMsg(MSG_LENGTH_TOO_SHORT,0);
                nIsSendErrorMsg = 1;
                //发送错误信息
                return 0;
            }
            else if(sMsgType.operateType == GB_SET_NO_REPONSE)
            {
                //直接退出，不发错误消息
                nIsSendErrorMsg = 0;
                return 0;
            }
		
		}
#endif
        gGBMsgLen -= nDataIndex;//判断值域长度时，应该去掉前面的填充部分。
        //对消息对象值域的处理，包括查询时，将结构体的值填充到数据包；设置时将数据包解析出来后，赋值到结构体中
        nCheckoutRes = DealObjectValueField(sMsgType.operateType,nObjectId,nChildObject,nIndexNum,nIndexArray,pData+nDataIndex,nArraySendBuf,&nIndexSend);

        if(nCheckoutRes == 0)//表明解析失败
        {
            if(sMsgType.operateType == GB_SET_REQ)
            {
                nIsSendErrorMsg = 1;
                //发送错误信息
                return 0;
            }
            else if(sMsgType.operateType == GB_SET_NO_REPONSE)
            {
                //直接退出，不发错误消息
                nIsSendErrorMsg = 0;
                return 0;
            }
        }
        else
        {
            //如果是查询请求，且数据包有对象值域时，也要使nDataIndex+1，这样才能正确解析下一个对象
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
    //设置消息类型域字节
    if(sMsgType.operateType == GB_QUERY_REQ)
    {
        sMsgType.operateType = GB_QUERY_REPONSE;
    }
    else if(sMsgType.operateType == GB_SET_REQ)
    {
        sMsgType.operateType = GB_SET_REPONSE;
    }
    nArraySendBuf[0] = *(UInt8 *)&sMsgType;
    //发送响应消息
    if(sendto(socketFd,nArraySendBuf, nIndexSend, 0, (struct sockaddr*)&toAddr, sizeof(toAddr)) <= 0)
    {
       printf("send data faild .\n");
    }

    return 1;
}


