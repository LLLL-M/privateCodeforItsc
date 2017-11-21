/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : WebsCallback.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年11月25日
  最近修改   :
  功能描述   : initweb的回调函数文件，这些函数直接响应页面地址，根据地址的不
               同返回不同的数据。
  函数列表   :
              actionDownload
              channelTable
              clearAllPara
              coordinate
              faultConfig
              faultDetectionSet
              getXMLValue
              greenRatio
              loginTest
              overlapping
              pedestrianDetector
              PhaseTable
              programTable
              resetAllPara
              ringAndPhase
              saveAllPara
              scheduling
              SendMsgToBoard
              sequenceTable
              sqliteCallback
              timeBasedActionTable
              timeInterval
              TreeDynamicParameter
              unitParams
              upldForm
              vehicleDetector
  修改历史   :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#include "WebsCallback.h"
#include "hikConfig.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern SignalControllerPara *gSignalControlpara;//全局指针，存储了信号机运行需要的所有变量
extern char *gErrorContent;//libhikcofig.a中供外部调用的错误内容缓冲区。
extern IPINFO IpInfo_eth0; //
extern IPINFO IpInfo_eth1;//
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/


/*****************************************************************************
 函 数 名  : StringToIntegerArray
 功能描述  : 从字符串中提取出整型数组
 输入参数  : unsigned short *array  
             unsigned short arrlen  
             char *str              
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void StringToIntegerArray(unsigned short *array,unsigned short arrlen,char *str)
{
    if((!str) || (!array))
    {
        return;
    }
    int i = 0;
    char *p = NULL;
    
    if(strlen(str) > 0)
    {
        array[i++] = atoi(strtok(str,","));

        while((p = strtok(NULL,",")))
        {
            if(i < arrlen)
            {
                array[i++] = atoi(p);
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : IntegerArrayToString
 功能描述  : 把short型数组按指定分隔符组合成字符串
 输入参数  : unsigned short *array    
             unsigned short arrayLen  
             char *str                
             unsigned short strLen    
             char *separator          
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void IntegerArrayToString(unsigned short *array,unsigned short arrayLen,char *str,unsigned short strLen,char *separator)
{
    int i = 0;
    memset(str,0,strLen);

    if((!str) || (!separator))
    {
        return;
    }

    for(i = 0 ; i < arrayLen ; i++)
    {
        if(0 != array[i])
        {
            if(strlen(str)+2 > strLen)
            {
                break;
            }

            sprintf(str+strlen(str),"%d%s",array[i],separator);
        }
    }

}



/*****************************************************************************
 函 数 名  : JudgingType
 功能描述  : 判断HTTP请求类型
 输入参数  : int flags  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int  JudgingType(int flags)
{
	int reqmethod;
	if(flags&WEBS_PUT_REQUEST)
	{
		reqmethod = METHOD_PUT;
	}
	else if(flags&WEBS_DELETE_REQUEST)
	{
		reqmethod = METHOD_DELETE;
	}
	else if(flags&WEBS_POST_REQUEST)
	{
		reqmethod = METHOD_POST;
	}
	else
	{
		reqmethod = METHOD_GET;
	}
	return reqmethod;
}

/*****************************************************************************
 函 数 名  : loginTest
 功能描述  : 用户登录合法性验证(暂时取消验证，登录永久合法)
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void loginTest(webs_t wp, char_t *path, char_t *query)
{
	char usernameStr[16] = {0};
	char passwordStr[16] = {0};
	char *username = usernameStr;
	char *password = passwordStr;
	int result = 0;
	//printf("query:%s\n",query);
	getXMLValue(query,"Login","username",username);
	getXMLValue(query,"Login","password",password);
	result = getLoginInfo(usernameStr,passwordStr);
	result = 1;
    log_debug("%s   username  %s  passwd  %s  result  %d\n",__func__,username,password,result);
	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<Login>\r\n"));
	websWrite(wp, T("<username>%s</username>\r\n"), username);
	websWrite(wp, T("<password>%s</password>\r\n"), password);
	websWrite(wp, T("<loginresult>%d</loginresult>\r\n"), result);
	websWrite(wp, T("</Login>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}



/*****************************************************************************
 函 数 名  : PhaseTable
 功能描述  : 相位表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void PhaseTable(webs_t wp, char_t *path, char_t *query)
{	char tmpStr[48] = {0};
	int account = 0;
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	stPhaseTable gPhaseTable;
	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{
		getXMLValue(query,"PhaseNo","PhaseAccount",tmpStr);
		account = atoi(tmpStr) ;
		result = getPhaseTableInfo(&gPhaseTable,account);
	}
	else if(reqmethod == METHOD_POST)
	{
		memset(&gPhaseTable,0,sizeof(stPhaseTable));
		getXMLValue(query,"PhaseNo","PhaseAccount",tmpStr);
		account = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","CircleAccount",tmpStr);
		gPhaseTable.nCircleNo = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","MinimumGreen",tmpStr);
		gPhaseTable.iMinimumGreen = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","MaximumGreenOne",tmpStr);
		gPhaseTable.iMaximumGreenOne = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","MaximumGreenTwo",tmpStr);
		gPhaseTable.iMaximumGreenTwo = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","ExtensionGreen",tmpStr);
		gPhaseTable.iExtensionGreen = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","MaximumRestrict",tmpStr);
		gPhaseTable.iMaximumRestrict = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","DynamicStep",tmpStr);
		gPhaseTable.iDynamicStep = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","YellowLightTime",tmpStr);
		gPhaseTable.iYellowLightTime = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","AllRedTime",tmpStr);
		gPhaseTable.iAllRedTime = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","GreenLightTime",tmpStr);
		gPhaseTable.nGreenLightTime = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","RedLightProtect",tmpStr);
		gPhaseTable.iRedLightProtect = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","PedestrianRelease",tmpStr);
		gPhaseTable.iPedestrianRelease = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","PedestrianCleaned",tmpStr);
		gPhaseTable.iPedestrianCleaned = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","KeepPedestrianRelease",tmpStr);
		gPhaseTable.iKeepPedestrianRelease = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","NoLockDetentionRequest",tmpStr);
		gPhaseTable.iNoLockDetentionRequest = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","DoubleEntrancePhase",tmpStr);
		gPhaseTable.iDoubleEntrancePhase = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","GuaranteeFluxDensityExtensionGreen",tmpStr);
		gPhaseTable.iGuaranteeFluxDensityExtensionGreen = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","ConditionalServiceValid",tmpStr);
		gPhaseTable.iConditionalServiceValid = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","MeanwhileEmptyLoseEfficacy",tmpStr);
		gPhaseTable.iMeanwhileEmptyLoseEfficacy = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","Enable",tmpStr);
		gPhaseTable.nIsEnable = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","Initialize",tmpStr);
		gPhaseTable.iInitialize = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","NonInduction",tmpStr);
		gPhaseTable.iNonInduction = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","VehicleAutomaticRequest",tmpStr);
		gPhaseTable.iVehicleAutomaticRequest = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","PedestrianAutomaticRequest",tmpStr);
		gPhaseTable.iPedestrianAutomaticRequest = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","AutomaticFlashInto",tmpStr);
		gPhaseTable.iAutomaticFlashInto = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","AutomaticFlashExit",tmpStr);
		gPhaseTable.iAutomaticFlashExit = atoi(tmpStr);

		getXMLValue(query,"PhaseNo","AutoPedestrianPass",tmpStr);
		gPhaseTable.nAutoPedestrianPass = atoi(tmpStr);

        //log_debug("circle   %d  Enable %d\n",gPhaseTable.nCircleNo,gPhaseTable.nIsEnable);
		result = setPhaseTableInfo(&gPhaseTable, account);
		}
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<PhaseNo>\r\n"));
		websWrite(wp, T("<CircleAccount>%d</CircleAccount>\r\n"), gPhaseTable.nCircleNo);//added by xiaowh
		websWrite(wp, T("<MinimumGreen>%d</MinimumGreen>\r\n"), gPhaseTable.iMinimumGreen);
		websWrite(wp, T("<MaximumGreenOne>%d</MaximumGreenOne>\r\n"), gPhaseTable.iMaximumGreenOne);
		websWrite(wp, T("<MaximumGreenTwo>%d</MaximumGreenTwo>\r\n"), gPhaseTable.iMaximumGreenTwo);
		websWrite(wp, T("<ExtensionGreen>%d</ExtensionGreen>\r\n"), gPhaseTable.iExtensionGreen);
		websWrite(wp, T("<MaximumRestrict>%d</MaximumRestrict>\r\n"), gPhaseTable.iMaximumRestrict);
		websWrite(wp, T("<DynamicStep>%d</DynamicStep>\r\n"), gPhaseTable.iDynamicStep);
		websWrite(wp, T("<YellowLightTime>%d</YellowLightTime>\r\n"), gPhaseTable.iYellowLightTime);
		websWrite(wp, T("<AllRedTime>%d</AllRedTime>\r\n"), gPhaseTable.iAllRedTime);
		websWrite(wp, T("<GreenLightTime>%d</GreenLightTime>\r\n"), gPhaseTable.nGreenLightTime);	///added by xiaowh
		websWrite(wp, T("<RedLightProtect>%d</RedLightProtect>\r\n"), gPhaseTable.iRedLightProtect);
		websWrite(wp, T("<PedestrianRelease>%d</PedestrianRelease>\r\n"), gPhaseTable.iPedestrianRelease);
		websWrite(wp, T("<PedestrianCleaned>%d</PedestrianCleaned>\r\n"), gPhaseTable.iPedestrianCleaned);
		websWrite(wp, T("<KeepPedestrianRelease>%d</KeepPedestrianRelease>\r\n"), gPhaseTable.iKeepPedestrianRelease);
		websWrite(wp, T("<NoLockDetentionRequest>%d</NoLockDetentionRequest>\r\n"), gPhaseTable.iNoLockDetentionRequest);
		websWrite(wp, T("<DoubleEntrancePhase>%d</DoubleEntrancePhase>\r\n"), gPhaseTable.iDoubleEntrancePhase);
		websWrite(wp, T("<GuaranteeFluxDensityExtensionGreen>%d</GuaranteeFluxDensityExtensionGreen>\r\n"), gPhaseTable.iGuaranteeFluxDensityExtensionGreen);
		websWrite(wp, T("<ConditionalServiceValid>%d</ConditionalServiceValid>\r\n"), gPhaseTable.iConditionalServiceValid);
		websWrite(wp, T("<MeanwhileEmptyLoseEfficacy>%d</MeanwhileEmptyLoseEfficacy>\r\n"), gPhaseTable.iMeanwhileEmptyLoseEfficacy);
		websWrite(wp, T("<Enable>%d</Enable>\r\n"), gPhaseTable.nIsEnable);//added by xiaowh
		websWrite(wp, T("<Initialize>%d</Initialize>\r\n"), gPhaseTable.iInitialize);
		websWrite(wp, T("<NonInduction>%d</NonInduction>\r\n"), gPhaseTable.iNonInduction);
		websWrite(wp, T("<VehicleAutomaticRequest>%d</VehicleAutomaticRequest>\r\n"), gPhaseTable.iVehicleAutomaticRequest);
		websWrite(wp, T("<PedestrianAutomaticRequest>%d</PedestrianAutomaticRequest>\r\n"), gPhaseTable.iPedestrianAutomaticRequest);
		websWrite(wp, T("<AutomaticFlashInto>%d</AutomaticFlashInto>\r\n"), gPhaseTable.iAutomaticFlashInto);
		websWrite(wp, T("<AutomaticFlashExit>%d</AutomaticFlashExit>\r\n"), gPhaseTable.iAutomaticFlashExit);
		websWrite(wp, T("<AutoPedestrianPass>%d</AutoPedestrianPass>\r\n"), gPhaseTable.nAutoPedestrianPass);	//added by xiaowh
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</PhaseNo>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
}


/*****************************************************************************
 函 数 名  : ringAndPhase
 功能描述  : 环和并发相位回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ringAndPhase(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[1024] = {0};
	int phaseNum;
//	char phaseNo[12] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
    unsigned short nPhaseArray[NUM_PHASE] = {0};
	//log_debug("%s  query:   %s\n",__func__,query);

    int i = 0;
    int j = 0;
    char buf[256] = {0};
    int nTotalNum = 0;
    char *p = null;

	ConcurrentPhaseItem stConcurrentPhase[NUM_PHASE];//并发相位
    memset(stConcurrentPhase,0,NUM_PHASE*sizeof(ConcurrentPhaseItem));

	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{
		result = getringAndPhaseInfo(stConcurrentPhase);

	}
	else if(reqmethod == METHOD_POST)
	{
	    memset(tmpStr,0,sizeof(tmpStr));
		getXMLValue(query,"RingAndPhase","TotalPhaseCount",tmpStr);
		nTotalNum = atoi(tmpStr);

        getXMLValue(query,"RingAndPhase","PhaseArray",tmpStr);
        StringToIntegerArray(nPhaseArray,NUM_PHASE,tmpStr);
        
		for(i = 0 ; i < nTotalNum ; i++)
		{
		    memset(buf,0,sizeof(buf));
		    memset(tmpStr,0,sizeof(tmpStr));
		    sprintf(buf,"RingForPhase%d",nPhaseArray[i]);
            getXMLValue(query,"RingAndPhase",buf,tmpStr);
            stConcurrentPhase[nPhaseArray[i]-1].nCircleID = atoi(tmpStr);

            memset(buf,0,sizeof(buf));
            memset(tmpStr,0,sizeof(tmpStr));
		    sprintf(buf,"SamePhase%d",nPhaseArray[i]);
            getXMLValue(query,"RingAndPhase",buf,tmpStr);

            stConcurrentPhase[nPhaseArray[i]-1].nPhaseID = nPhaseArray[i];
            //printf("   %d  :      %s\n",nPhaseArray[i],tmpStr);
            if(strlen(tmpStr) > 0 )
            {
                j = 0;
                stConcurrentPhase[nPhaseArray[i]-1].nArrayConcurrentPase[j] = atoi(strtok(tmpStr,","));
                printf("===>   %d ",stConcurrentPhase[nPhaseArray[i]-1].nArrayConcurrentPase[j]);
                while((p = strtok(NULL,",")))
                {
                   j++;
                   stConcurrentPhase[nPhaseArray[i]-1].nArrayConcurrentPase[j] = atoi(p);
                   printf("  %d",stConcurrentPhase[nPhaseArray[i]-1].nArrayConcurrentPase[j]);
                }
                printf("\n");
            }

		}
        result = setringAndPhaseInfo(stConcurrentPhase);
	}

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<RingAndPhase>\r\n"));

    for(i=0; i < NUM_PHASE ; i++)
    {
        if(stConcurrentPhase[i].nPhaseID == 0)
        {
            continue;
        }

        phaseNum = stConcurrentPhase[i].nPhaseID;
        memset(buf,0,sizeof(buf));

        for(j = 0 ; j < NUM_PHASE ; j++)
        {
            if(stConcurrentPhase[i].nArrayConcurrentPase[j] == 0)
            {
                break;
            }

            sprintf(buf+strlen(buf),"%d,",stConcurrentPhase[i].nArrayConcurrentPase[j]);
        }

        if(strlen(buf) != 0)
        {
            buf[strlen(buf) - 1] = '\0';

        }


		websWrite(wp, T("<RingForPhase%d>%d</RingForPhase%d>\r\n"), phaseNum,stConcurrentPhase[i].nCircleID,phaseNum);


		websWrite(wp, T("<SamePhase%d>%s</SamePhase%d>\r\n"), phaseNum,buf,phaseNum);

		//log_debug("%s===>  phase  %d  array  %s\n",__func__,phaseNum,buf);

    }

	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</RingAndPhase>\r\n"));
	websFooter(wp);
	websDone(wp, 200);

}

/*****************************************************************************
 函 数 名  : channelTable
 功能描述  : 通道表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void channelTable(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	int account = 0;
	//log_debug("%s   query:%s\n",__func__,query);
	reqmethod = JudgingType(flags);
	stChannelTable gChannelTable;

	if (reqmethod == METHOD_GET)
	{
		getXMLValue(query,"ChannelTable","channelNum",tmpStr);
		account = atoi(tmpStr);
		result = getChannelTableInfo(&gChannelTable, account);
	}
	else if(reqmethod == METHOD_POST)
	{
		memset(&gChannelTable,0,sizeof(gChannelTable));
		getXMLValue(query,"ChannelTable","channelNum",tmpStr);
		account = atoi(tmpStr);

		getXMLValue(query,"ChannelTable","controlSource",tmpStr);
		gChannelTable.iControlSource = atoi(tmpStr);

		getXMLValue(query,"ChannelTable","controlType",tmpStr);
		gChannelTable.iControlType = atoi(tmpStr);

		getXMLValue(query,"ChannelTable","flashMode",tmpStr);
		gChannelTable.iFlashMode = atoi(tmpStr);

		getXMLValue(query,"ChannelTable","brightMode",tmpStr);
		gChannelTable.iBrightMode = atoi(tmpStr);

        //log_debug("account  %d  source  %d\n",account,gChannelTable.iControlSource);
		//把单元参数保存到指定ini配置文件中
		result = setChannelTableInfo(&gChannelTable, account);
	}

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<ChannelTable>\r\n"));
	websWrite(wp, T("<controlSource>%d</controlSource>\r\n"), gChannelTable.iControlSource);
	websWrite(wp, T("<controlType>%d</controlType>\r\n"), gChannelTable.iControlType);
	websWrite(wp, T("<flashMode>%d</flashMode>\r\n"), gChannelTable.iFlashMode);
	websWrite(wp, T("<brightMode>%d</brightMode>\r\n"), gChannelTable.iBrightMode);
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</ChannelTable>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : greenRatio
 功能描述  : 绿信比表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void greenRatio(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[1024] = {0};
	char phaseNo[48] = {0};
	unsigned short nPhaseArray[NUM_PHASE] = {0};
	int phaseNum;
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	int account = 0;
	//log_debug("%s    \n",__func__);
	reqmethod = JudgingType(flags);
    stGreenRatio gGreenRatio;

    int nTotalPhaseNum = 0;

	if (reqmethod == METHOD_GET)
	{		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		getXMLValue(query,"GreenRatio","splitNo",tmpStr);
		account = atoi(tmpStr);
		result = getGreenRatioInfo(&gGreenRatio,account);
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gGreenRatio,0,sizeof(gGreenRatio));
		getXMLValue(query,"GreenRatio","splitNo",tmpStr);
		account = atoi(tmpStr);
		gGreenRatio.iSplitNo= account;

		getXMLValue(query,"GreenRatio","TotalPhaseCount",tmpStr);//相位总数
		nTotalPhaseNum = atoi(tmpStr);

		getXMLValue(query,"GreenRatio","PhaseArray",tmpStr);//相位数组
		StringToIntegerArray(nPhaseArray,NUM_PHASE,tmpStr);
        
		for(phaseNum=1;phaseNum<=nTotalPhaseNum;phaseNum++)
		{
			sprintf(phaseNo,"%s%d","splitForPhase",nPhaseArray[phaseNum-1]);
			getXMLValue(query,"GreenRatio",phaseNo,tmpStr);
			gGreenRatio.iSplitForPhase[nPhaseArray[phaseNum-1]-1] = atoi(tmpStr);

			sprintf(phaseNo,"%s%d","splitMode",nPhaseArray[phaseNum-1]);
			getXMLValue(query,"GreenRatio",phaseNo,tmpStr);
			gGreenRatio.iModeForPhase[nPhaseArray[phaseNum-1]-1] = atoi(tmpStr);

			sprintf(phaseNo,"%s%d","coordinate",nPhaseArray[phaseNum-1]);
			getXMLValue(query,"GreenRatio",phaseNo,tmpStr);
			gGreenRatio.iCoordinatePhase[nPhaseArray[phaseNum-1]-1] = atoi(tmpStr);

			gGreenRatio.nPhaseId[nPhaseArray[phaseNum-1] - 1] = nPhaseArray[phaseNum-1];
		}

		//把单元参数保存到指定ini配置文件中
		result = setGreenRatioInfo(&gGreenRatio,account);
	}

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<GreenRatio>\r\n"));
	for(phaseNum=1;phaseNum<NUM_PHASE;phaseNum++)
	{
	    if(gGreenRatio.nPhaseId[phaseNum - 1] == 0)
	    {
            continue;
	    }
		websWrite(wp, T("<splitForPhase%d>%d</splitForPhase%d>\r\n"), phaseNum,gGreenRatio.iSplitForPhase[phaseNum-1],phaseNum);
		websWrite(wp, T("<splitMode%d>%d</splitMode%d>\r\n"), phaseNum,gGreenRatio.iModeForPhase[phaseNum-1],phaseNum);
		websWrite(wp, T("<coordinate%d>%d</coordinate%d>\r\n"), phaseNum,gGreenRatio.iCoordinatePhase[phaseNum-1],phaseNum);
	}

	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</GreenRatio>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : faultDetectionSet
 功能描述  : 故障检测回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void faultDetectionSet(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int ChanNum;
	char strChan0[12], strChan1[12], strChan2[12];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	printf("query:%s\n",query);
	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		result = getFaultDetectionSetInfo(&gFaultDetectionSet,"/home/login.ini");
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gFaultDetectionSet,0,sizeof(stFaultDetectionSet));
		getXMLValue(query,"FaultDetectionSet","VoltageDetectionTimes",tmpStr);
		gFaultDetectionSet.VoltageDetectionTimes = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","RedLightDetectionTimes",tmpStr);
		gFaultDetectionSet.RedLightDetectionTimes = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","ConflictDetectionAttempts",tmpStr);
		gFaultDetectionSet.ConflictDetectionAttempts = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","ManualPanelKeyNumber",tmpStr);
		gFaultDetectionSet.ManualPanelKeyNumber = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","RemoteControlKeyNumber",tmpStr);
		gFaultDetectionSet.RemoteControlKeyNumber = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","SenseSwitch",tmpStr);
		gFaultDetectionSet.SenseSwitch = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","DynamicStep",tmpStr);
		gFaultDetectionSet.DynamicStep = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","CurrentFaultDetection",tmpStr);
		gFaultDetectionSet.CurrentFaultDetection = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","AlarmAndFaultCurrent",tmpStr);
		gFaultDetectionSet.AlarmAndFaultCurrent = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","AlarmAndFaultVoltage",tmpStr);
		gFaultDetectionSet.AlarmAndFaultVoltage = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","EnableWatchdog",tmpStr);
		gFaultDetectionSet.EnableWatchdog = atoi(tmpStr);
		getXMLValue(query,"FaultDetectionSet","EnableGPS",tmpStr);
		gFaultDetectionSet.EnableGPS = atoi(tmpStr);
		for(ChanNum=0;ChanNum<=31;ChanNum++)
		{
			sprintf(strChan0,"%s%d_0","CNum",ChanNum+1);
			sprintf(strChan1,"%s%d_1","CNum",ChanNum+1);
			sprintf(strChan2,"%s%d_2","CNum",ChanNum+1);
			getXMLValue(query,"FaultDetectionSet",strChan0,tmpStr);
			gFaultDetectionSet.CNum[ChanNum][0] = atoi(tmpStr);
			getXMLValue(query,"FaultDetectionSet",strChan1,tmpStr);
			gFaultDetectionSet.CNum[ChanNum][1] = atoi(tmpStr);
			getXMLValue(query,"FaultDetectionSet",strChan2,tmpStr);
			gFaultDetectionSet.CNum[ChanNum][2] = atoi(tmpStr);

		}
		//把单元参数保存到指定ini配置文件中
		result = saveFaultDetectionSet2Ini(gFaultDetectionSet,"/home/login.ini");
	}
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<FaultDetectionSet>\r\n"));
		websWrite(wp, T("<VoltageDetectionTimes>%d</VoltageDetectionTimes>\r\n"), gFaultDetectionSet.VoltageDetectionTimes);
		websWrite(wp, T("<RedLightDetectionTimes>%d</RedLightDetectionTimes>\r\n"), gFaultDetectionSet.RedLightDetectionTimes);
		websWrite(wp, T("<ConflictDetectionAttempts>%d</ConflictDetectionAttempts>\r\n"), gFaultDetectionSet.ConflictDetectionAttempts);
		websWrite(wp, T("<ManualPanelKeyNumber>%d</ManualPanelKeyNumber>\r\n"), gFaultDetectionSet.ManualPanelKeyNumber);
		websWrite(wp, T("<RemoteControlKeyNumber>%d</RemoteControlKeyNumber>\r\n"), gFaultDetectionSet.RemoteControlKeyNumber);
		websWrite(wp, T("<SenseSwitch>%d</SenseSwitch>\r\n"), gFaultDetectionSet.SenseSwitch);
		websWrite(wp, T("<DynamicStep>%d</DynamicStep>\r\n"), gFaultDetectionSet.DynamicStep);
		websWrite(wp, T("<CurrentFaultDetection>%d</CurrentFaultDetection>\r\n"), gFaultDetectionSet.CurrentFaultDetection);
		websWrite(wp, T("<AlarmAndFaultCurrent>%d</AlarmAndFaultCurrent>\r\n"), gFaultDetectionSet.AlarmAndFaultCurrent);
		websWrite(wp, T("<AlarmAndFaultVoltage>%d</AlarmAndFaultVoltage>\r\n"), gFaultDetectionSet.AlarmAndFaultVoltage);
		websWrite(wp, T("<EnableWatchdog>%d</EnableWatchdog>\r\n"), gFaultDetectionSet.EnableWatchdog);
		websWrite(wp, T("<EnableGPS>%d</EnableGPS>\r\n"), gFaultDetectionSet.EnableGPS);
		for(ChanNum=0;ChanNum<=31;ChanNum++)
		{
			websWrite(wp, T("<CNum%d_0>%d</CNum%d_0>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][0],ChanNum+1);
			websWrite(wp, T("<CNum%d_1>%d</CNum%d_1>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][1],ChanNum+1);
			websWrite(wp, T("<CNum%d_2>%d</CNum%d_2>\r\n"), ChanNum+1,gFaultDetectionSet.CNum[ChanNum][2],ChanNum+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</FaultDetectionSet>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : sequenceTable
 功能描述  : 相序表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void sequenceTable(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
//	int ChanNum;
//	char seqNum1[12], seqNum2[12], seqNum3[12],seqNum4[12];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	reqmethod = JudgingType(flags);
	int account = 0;
	int i = 0;
	int j = 0;

	stSequenceTable gSequenceTable;
    char buf[256];


	if (reqmethod == METHOD_GET)
	{		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端

		getXMLValue(query,"SequenceTable","nPhaseTurnId",tmpStr);
		//log_debug("===>   tempStr   %s\n",tmpStr);
		account = atoi(tmpStr);

		//log_debug("===>  pHaseTurnId   %d\n",account);

		result = getSequenceTableInfo(&gSequenceTable,account);
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gSequenceTable,0,sizeof(stSequenceTable));
		getXMLValue(query,"SequenceTable","nPhaseTurnId",tmpStr);
		gSequenceTable.SequenceTableNo = atoi(tmpStr);

        //log_debug("==>  no  %d\n",gSequenceTable.SequenceTableNo);
        for(i = 0 ; i < 4  ;  i++)
        {
            for(j = 0 ; j < NUM_PHASE ; j++)
            {
                memset(buf,0,sizeof(buf));
                memset(tmpStr,0,sizeof(tmpStr));
                sprintf(buf,"Circle_%d_%d",i+1,j+1);
    			getXMLValue(query,"SequenceTable",buf,tmpStr);
    			gSequenceTable.SNum[i][j] = atoi(tmpStr);

            }
        }

		//把单元参数保存到指定ini配置文件中
		result = setSequenceTableInfo(&gSequenceTable,gSequenceTable.SequenceTableNo);
	}

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<SequenceTable>\r\n"));
	websWrite(wp, T("<SequenceTableNo>%d</SequenceTableNo>\r\n"), gSequenceTable.SequenceTableNo);

	for(i = 0 ; i < 4 ; i++)
	{
	    for(j = 0 ; j < NUM_PHASE; j++)
	    {
			websWrite(wp, T("<Circle_%d_%d>%d</Circle_%d_%d>\r\n"),i+1, j+1,gSequenceTable.SNum[i][j],i+1,j+1);
            //log_debug("<Circle_%d_%d>%d</Circle_%d_%d>\n",i+1, j+1,gSequenceTable.SNum[i][j],i+1,j+1);
	    }

	}
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</SequenceTable>\r\n"));
	websFooter(wp);
	websDone(wp, 200);



}

/*****************************************************************************
 函 数 名  : programTable
 功能描述  : 方案表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void programTable(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
//	char seqNum1[12], seqNum2[12], seqNum3[12],seqNum4[12];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	//int ChanNum;
	int account = 0;
	//log_debug("%s   query:%s\n",__func__,query);
	reqmethod = JudgingType(flags);

	stProgramTable gProgramTable;

	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		getXMLValue(query,"ProgramTable","ProgramTableNo",tmpStr);
		account = atoi(tmpStr);

		result = getProgramTableInfo(&gProgramTable, account);
	}
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		getXMLValue(query,"ProgramTable","ProgramTableNo",tmpStr);
		gProgramTable.nSchemeID = atoi(tmpStr);

		getXMLValue(query,"ProgramTable","nCycleTime",tmpStr);
		gProgramTable.nCycleTime = atoi(tmpStr);

		getXMLValue(query,"ProgramTable","nGreenSignalRatioID",tmpStr);
		gProgramTable.nGreenSignalRatioID = atoi(tmpStr);

		getXMLValue(query,"ProgramTable","nOffset",tmpStr);
		gProgramTable.nOffset = atoi(tmpStr);

		getXMLValue(query,"ProgramTable","nPhaseTurnID",tmpStr);
		gProgramTable.nPhaseTurnID = atoi(tmpStr);

		result = setProgramTableInfo(&gProgramTable,gProgramTable.nSchemeID);

	}
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<ProgramTable>\r\n"));

        websWrite(wp, T("<nCycleTime>%d</nCycleTime>\r\n"), gProgramTable.nCycleTime);
        websWrite(wp, T("<nGreenSignalRatioID>%d</nGreenSignalRatioID>\r\n"), gProgramTable.nGreenSignalRatioID);
        websWrite(wp, T("<nOffset>%d</nOffset>\r\n"), gProgramTable.nOffset);
        websWrite(wp, T("<nPhaseTurnID>%d</nPhaseTurnID>\r\n"), gProgramTable.nPhaseTurnID);
        websWrite(wp, T("<nSchemeID>%d</nSchemeID>\r\n"), gProgramTable.nSchemeID);

		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</ProgramTable>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : timeBasedActionTable
 功能描述  : 动作表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void timeBasedActionTable(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int ChanNum;
	char seqNum[20];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	int account = 0;

	//log_debug("%s   query:%s\n",__func__,query);
	reqmethod = JudgingType(flags);

	stTimeBasedActionTable gTimeBasedActionTable;

	if (reqmethod == METHOD_GET)
	{
		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		getXMLValue(query,"TimeBasedActionTable","ActionTable",tmpStr);
		account = atoi(tmpStr);

       // log_debug("====>   account  %d\n",account);
		result = getTimeBasedActionTableInfo(&gTimeBasedActionTable,account);
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gTimeBasedActionTable,0,sizeof(stTimeBasedActionTable));
		getXMLValue(query,"TimeBasedActionTable","ActionTable",tmpStr);
		gTimeBasedActionTable.ActionTable = atoi(tmpStr);

		getXMLValue(query,"TimeBasedActionTable","ProgramNo",tmpStr);
		gTimeBasedActionTable.ProgramNo = atoi(tmpStr);

		for(ChanNum=0;ChanNum<3;ChanNum++)
		{
			sprintf(seqNum,"%s%d","AssistFunction",ChanNum+1);
			getXMLValue(query,"TimeBasedActionTable",seqNum,tmpStr);
			gTimeBasedActionTable.AssistFunction[ChanNum] = atoi(tmpStr);
		}
		for(ChanNum=0;ChanNum<7;ChanNum++)
		{
			sprintf(seqNum,"%s%d","SpecialFunction",ChanNum+1);
			getXMLValue(query,"TimeBasedActionTable",seqNum,tmpStr);
			gTimeBasedActionTable.SpecialFunction[ChanNum] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = setTimeBasedActionTableInfo(&gTimeBasedActionTable,gTimeBasedActionTable.ActionTable);
	}


	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<TimeBasedActionTable>\r\n"));
	//websWrite(wp, T("<ActionTable>%d</ActionTable>\r\n"), gTimeBasedActionTable.ActionTable);
	websWrite(wp, T("<ProgramNo>%d</ProgramNo>\r\n"), gTimeBasedActionTable.ProgramNo);
	log_debug("%s-----ProgramNO   %d\n",__func__,gTimeBasedActionTable.ProgramNo);
	for(ChanNum=0;ChanNum<3;ChanNum++)
	{
		websWrite(wp, T("<AssistFunction%d>%d</AssistFunction%d>\r\n"), ChanNum+1,gTimeBasedActionTable.AssistFunction[ChanNum],ChanNum+1);
	}
	for(ChanNum=0;ChanNum<= 7;ChanNum++)
	{
		websWrite(wp, T("<SpecialFunction%d>%d</SpecialFunction%d>\r\n"), ChanNum+1,gTimeBasedActionTable.SpecialFunction[ChanNum],ChanNum+1);
	}
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</TimeBasedActionTable>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : timeInterval
 功能描述  : 时段表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void timeInterval(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	//int Tcount;
//	char timeNum1[20],timeNum2[20],timeNum3[20];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	int account = 0;
	int i  = 0 ;

	//log_debug("%s    \n",__func__);
	reqmethod = JudgingType(flags);

    TimeIntervalItem item[NUM_TIME_INTERVAL_ID];
    memset(item,0,NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem));

    char buf[256];
	int nTotalNum = 0;

	if (reqmethod == METHOD_GET)
	{		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		getXMLValue(query,"TimeInterval","TimeIntervalID",tmpStr);
		account = atoi(tmpStr);

       // log_debug("===> count   %d",account);

		result = getTimeIntervalInfo(item,account);
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回

        getXMLValue(query,"TimeInterval","TimeIntervalID",tmpStr);
        account = atoi(tmpStr);

        getXMLValue(query,"TimeInterval","TotalTimeCount",tmpStr);
        nTotalNum = atoi(tmpStr);

        for(i = 0 ; i < nTotalNum; i++)
        {
            memset(buf,0,sizeof(buf));
            sprintf(buf,"Hour_%d",i+1);
            getXMLValue(query,"TimeInterval",buf,tmpStr);
            item[i].cStartTimeHour = atoi(tmpStr);

            memset(buf,0,sizeof(buf));
            sprintf(buf,"Minute_%d",i+1);
            getXMLValue(query,"TimeInterval",buf,tmpStr);
            item[i].cStartTimeMinute = atoi(tmpStr);

            memset(buf,0,sizeof(buf));
            sprintf(buf,"ActionId_%d",i+1);
            getXMLValue(query,"TimeInterval",buf,tmpStr);
            item[i].nActionID = atoi(tmpStr);

            item[i].nTimeID = i+1;
            item[i].nTimeIntervalID = account;

        }

        result = setTimeIntervalInfo(item,account);

	}

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<TimeInterval>\r\n"));

    for(i = 0 ; i < NUM_TIME_INTERVAL_ID; i++)
    {
        if(item[i].nActionID != 0)
        {
    		websWrite(wp, T("<Hour_%d>%d</Hour_%d>\r\n"), i+1,item[i].cStartTimeHour,i+1);
    		websWrite(wp, T("<Minute_%d>%d</Minute_%d>\r\n"), i+1,item[i].cStartTimeMinute,i+1);
            websWrite(wp, T("<ActionId_%d>%d</ActionId_%d>\r\n"), i+1,item[i].nActionID,i+1);

            log_debug("Hour  %d   Minute  %d  ActionId  %d\n",item[i].cStartTimeHour,item[i].cStartTimeMinute,item[i].nActionID);
           // log_debug("<Hour_%d>%d</Hour_%d>\n", i+1,item[i].cStartTimeHour,i+1);
        }
    }

	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</TimeInterval>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : scheduling
 功能描述  : 调度表回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void scheduling(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int Tcount;
	char MDW1[20],MDW2[20],MDW3[20];                    //MDW:Month,Day,WeekDay
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
    int account = 0;
    stScheduling gScheduling;

	//log_debug("%s    :%s\n",__func__,query);
	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{
		getXMLValue(query,"Scheduling","SchedulingNo",tmpStr);
		account = atoi(tmpStr);

       // log_debug("account   %d\n",account);

		result = getSchedulingInfo(&gScheduling,account);
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gScheduling,0,sizeof(stScheduling));
		getXMLValue(query,"Scheduling","SchedulingNo",tmpStr);
		gScheduling.SchedulingNo = atoi(tmpStr);

		getXMLValue(query,"Scheduling","TimeIntervalNum",tmpStr);
		gScheduling.TimeIntervalNum = atoi(tmpStr);
		for(Tcount=0;Tcount<12;Tcount++)
		{
			sprintf(MDW1,"%s%d","Month",Tcount+1);
			getXMLValue(query,"Scheduling",MDW1,tmpStr);
			gScheduling.Month[Tcount] = atoi(tmpStr);
		}
		for(Tcount=0;Tcount<31;Tcount++)
		{
			sprintf(MDW2,"%s%d","Day",Tcount+1);
			getXMLValue(query,"Scheduling",MDW2,tmpStr);
			gScheduling.Day[Tcount] = atoi(tmpStr);
			//printf("==>   %d\n",gScheduling.Day[Tcount]);
		}
		for(Tcount=0;Tcount<7;Tcount++)
		{
			sprintf(MDW3,"%s%d","WeekDay",Tcount+1);
			getXMLValue(query,"Scheduling",MDW3,tmpStr);
			gScheduling.WeekDay[Tcount] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = setSchedulingInfo(&gScheduling,gScheduling.SchedulingNo);
	}
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Scheduling>\r\n"));
		websWrite(wp, T("<SchedulingNo>%d</SchedulingNo>\r\n"), gScheduling.SchedulingNo);
		websWrite(wp, T("<TimeIntervalNum>%d</TimeIntervalNum>\r\n"), gScheduling.TimeIntervalNum);
		for(Tcount=0;Tcount<12;Tcount++)
		{
			websWrite(wp, T("<Month%d>%d</Month%d>\r\n"), Tcount+1,gScheduling.Month[Tcount],Tcount+1);
		}
		for(Tcount=0;Tcount<31;Tcount++)
		{
			websWrite(wp, T("<Day%d>%d</Day%d>\r\n"), Tcount+1,gScheduling.Day[Tcount],Tcount+1);
		}
		for(Tcount=0;Tcount<7;Tcount++)
		{
			websWrite(wp, T("<WeekDay%d>%d</WeekDay%d>\r\n"), Tcount+1,gScheduling.WeekDay[Tcount],Tcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Scheduling>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : overlapping
 功能描述  : 跟随相位回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void overlapping(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int Overcount;
	char PPhase[20];
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
    int account = 0;

	//log_debug("%s       query:%s\n",__func__,query);
    stOverlapping gOverlapping;


	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		getXMLValue(query,"Overlapping","FollowPhase",tmpStr);
		account = atoi(tmpStr);

		result = getOverlappingInfo(&gOverlapping,account);
	}
	else if(reqmethod == METHOD_POST)
	{
		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		getXMLValue(query,"Overlapping","FollowPhase",tmpStr);
		gOverlapping.FollowPhase = atoi(tmpStr);

		getXMLValue(query,"Overlapping","GreenLight",tmpStr);
		gOverlapping.GreenLight = atoi(tmpStr);

		getXMLValue(query,"Overlapping","RedLight",tmpStr);
		gOverlapping.RedLight = atoi(tmpStr);

		getXMLValue(query,"Overlapping","YellowLight",tmpStr);
		gOverlapping.YellowLight = atoi(tmpStr);

		getXMLValue(query,"Overlapping","GreenFlash",tmpStr);
		gOverlapping.GreenFlash = atoi(tmpStr);

		getXMLValue(query,"Overlapping","ModifiedPhase",tmpStr);
		gOverlapping.ModifiedPhase = atoi(tmpStr);

		for(Overcount=0;Overcount<NUM_PHASE;Overcount++)
		{
			sprintf(PPhase,"%s%d","ParentPhase",Overcount+1);
			getXMLValue(query,"Overlapping",PPhase,tmpStr);
			gOverlapping.ParentPhase[Overcount] = atoi(tmpStr);
		}
		//把单元参数保存到指定ini配置文件中
		result = setOverlappingInfo(&gOverlapping,gOverlapping.FollowPhase);
	}
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Overlapping>\r\n"));
		websWrite(wp, T("<FollowPhase>%d</FollowPhase>\r\n"), gOverlapping.FollowPhase);
		websWrite(wp, T("<GreenLight>%d</GreenLight>\r\n"), gOverlapping.GreenLight);
		websWrite(wp, T("<RedLight>%d</RedLight>\r\n"), gOverlapping.RedLight);
		websWrite(wp, T("<YellowLight>%d</YellowLight>\r\n"), gOverlapping.YellowLight);
		websWrite(wp, T("<GreenFlash>%d</GreenFlash>\r\n"), gOverlapping.GreenFlash);
		websWrite(wp, T("<ModifiedPhase>%d</ModifiedPhase>\r\n"), gOverlapping.ModifiedPhase);
		for(Overcount=0;Overcount<NUM_PHASE;Overcount++)
		{
			websWrite(wp, T("<ParentPhase%d>%d</ParentPhase%d>\r\n"), Overcount+1,gOverlapping.ParentPhase[Overcount],Overcount+1);

			//log_debug("<ParentPhase%d>%d</ParentPhase%d>\n",Overcount+1,gOverlapping.ParentPhase[Overcount],Overcount+1);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</Overlapping>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : coordinate
 功能描述  : 协调相位回调函数(未更新)
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void coordinate(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	printf("query:%s\n",query);
	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		result = getCoordinateInfo(&gCoordinate,"/home/login.ini");
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gCoordinate,0,sizeof(stCoordinate));
		getXMLValue(query,"Coordinate","ControlModel",tmpStr);
		gCoordinate.ControlModel = atoi(tmpStr);
		getXMLValue(query,"Coordinate","ManualMethod",tmpStr);
		gCoordinate.ManualMethod = atoi(tmpStr);
		getXMLValue(query,"Coordinate","CoordinationMode",tmpStr);
		gCoordinate.CoordinationMode = atoi(tmpStr);
		getXMLValue(query,"Coordinate","CoordinateMaxMode",tmpStr);
		gCoordinate.CoordinateMaxMode = atoi(tmpStr);
		getXMLValue(query,"Coordinate","CoordinateForceMode",tmpStr);
		gCoordinate.CoordinateForceMode = atoi(tmpStr);
		//把单元参数保存到指定ini配置文件中
		result = saveCoordinate2Ini(gCoordinate,"/home/login.ini");
	}
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<Coordinate>\r\n"));
		websWrite(wp, T("<ControlModel>%d</ControlModel>\r\n"), gCoordinate.ControlModel);
		websWrite(wp, T("<ManualMethod>%d</ManualMethod>\r\n"), gCoordinate.ManualMethod);
		websWrite(wp, T("<CoordinationMode>%d</CoordinationMode>\r\n"), gCoordinate.CoordinationMode);
		websWrite(wp, T("<CoordinateMaxMode>%d</CoordinateMaxMode>\r\n"), gCoordinate.CoordinateMaxMode);
		websWrite(wp, T("<CoordinateForceMode>%d</CoordinateForceMode>\r\n"), gCoordinate.CoordinateForceMode);
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);		websWrite(wp, T("</Coordinate>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
}



/*****************************************************************************
 函 数 名  : unitParams
 功能描述  : 单元参数回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void unitParams(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	reqmethod = JudgingType(flags);
    stUnitParams gUnitParams;

	if (reqmethod == METHOD_GET)
	{
		result = getUnitParamsInfo(&gUnitParams);
	}
	else if(reqmethod == METHOD_POST)
	{
		memset(&gUnitParams,0,sizeof(stUnitParams));
		getXMLValue(query,"UNITPARAMS","StartFlashingYellowTime",tmpStr);
		gUnitParams.iStartFlashingYellowTime = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","StartAllRedTime",tmpStr);
		gUnitParams.iStartAllRedTime = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","DegradationTime",tmpStr);
		gUnitParams.iDegradationTime = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","SpeedFactor",tmpStr);
		gUnitParams.iSpeedFactor = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","MinimumRedLightTime",tmpStr);
		gUnitParams.iMinimumRedLightTime = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","CommunicationTimeout",tmpStr);
		gUnitParams.iCommunicationTimeout = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","FlashingFrequency",tmpStr);
		gUnitParams.iFlashingFrequency = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","TwiceCrossingTimeInterval",tmpStr);
		gUnitParams.iTwiceCrossingTimeInterval = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","TwiceCrossingReverseTimeInterval",tmpStr);
		gUnitParams.iTwiceCrossingReverseTimeInterval = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","SmoothTransitionPeriod",tmpStr);
		gUnitParams.iSmoothTransitionPeriod = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","FlowCollectionPeriod",tmpStr);
		gUnitParams.iFlowCollectionPeriod = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","CollectUnit",tmpStr);
		gUnitParams.iCollectUnit = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","AutoPedestrianEmpty",tmpStr);
		gUnitParams.iAutoPedestrianEmpty = atoi(tmpStr);

		getXMLValue(query,"UNITPARAMS","OverpressureDetection",tmpStr);
		gUnitParams.iOverpressureDetection = atoi(tmpStr);

		result = setUnitParamsInfo(&gUnitParams);
	}
	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<UNITPARAMS>\r\n"));
	websWrite(wp, T("<StartFlashingYellowTime>%d</StartFlashingYellowTime>\r\n"), gUnitParams.iStartFlashingYellowTime);
	websWrite(wp, T("<StartAllRedTime>%d</StartAllRedTime>\r\n"), gUnitParams.iStartAllRedTime);
	websWrite(wp, T("<DegradationTime>%d</DegradationTime>\r\n"), gUnitParams.iDegradationTime);
	websWrite(wp, T("<SpeedFactor>%d</SpeedFactor>\r\n"), gUnitParams.iSpeedFactor);
	websWrite(wp, T("<MinimumRedLightTime>%d</MinimumRedLightTime>\r\n"), gUnitParams.iMinimumRedLightTime);
	websWrite(wp, T("<CommunicationTimeout>%d</CommunicationTimeout>\r\n"), gUnitParams.iCommunicationTimeout);
	websWrite(wp, T("<FlashingFrequency>%d</FlashingFrequency>\r\n"), gUnitParams.iFlashingFrequency);
	websWrite(wp, T("<TwiceCrossingTimeInterval>%d</TwiceCrossingTimeInterval>\r\n"), gUnitParams.iTwiceCrossingTimeInterval);
	websWrite(wp, T("<TwiceCrossingReverseTimeInterval>%d</TwiceCrossingReverseTimeInterval>\r\n"), gUnitParams.iTwiceCrossingReverseTimeInterval);
	websWrite(wp, T("<SmoothTransitionPeriod>%d</SmoothTransitionPeriod>\r\n"), gUnitParams.iSmoothTransitionPeriod);
	websWrite(wp, T("<FlowCollectionPeriod>%d</FlowCollectionPeriod>\r\n"), gUnitParams.iFlowCollectionPeriod);
	websWrite(wp, T("<CollectUnit>%d</CollectUnit>\r\n"), gUnitParams.iCollectUnit);
	websWrite(wp, T("<AutoPedestrianEmpty>%d</AutoPedestrianEmpty>\r\n"), gUnitParams.iAutoPedestrianEmpty);
	websWrite(wp, T("<OverpressureDetection>%d</OverpressureDetection>\r\n"), gUnitParams.iOverpressureDetection);
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</UNITPARAMS>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : SendMsgToBoard
 功能描述  : 通过socket将变更标志发送到HikTSC程序
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int SendMsgToBoard()
{
    int sock = 0;
    int ret = 0;

    if(CreateSocket(&sock, 0,null,0,0,0) == 1)
    {
        ret = SendDataNonBlock(sock,(void *)gSignalControlpara,sizeof(SignalControllerPara),IpInfo_eth0.cIp,IPPORT);//默认使用eth0来发送数据
    }

    close(sock);
    
    return ret;
}

/*****************************************************************************
 函 数 名  : saveAllPara
 功能描述  : 把所有参数保存到本地，同时检查合法性，如果合法则保存成真正的配
             置文件，否则仅仅保存为临时文件
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void saveAllPara(webs_t wp, char_t *path, char_t *query)
{
	int result = 0;
	unlink("./config_tmp.txt");
	
	WriteConfigFile(gSignalControlpara, "./config_tmp.txt");
    result = IsSignalControlparaLegal(gSignalControlpara);
    if(result == 0)
    {
        //log_debug("---- %d  %d\n",gSignalControlpara->stChannel[0].nChannelID,gSignalControlpara->stChannel[0].nControllerID);

        //send msg to board to reload cfg
        //if(SendMsgToBoard() > 0)
        if(SendSignalControlParams() == 1)
        {
            log_debug("succeed to send msg to board \n");
        }
        else
        {
            log_error("error to send msg to board \n");
        }
    }

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"GBK\" ?>\n"));
	websWrite(wp, T("<SaveAllPara>\r\n"));
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);

	if(result != 0)//如果校验有误，则添加一个结点，显示错误内容
	{
        websWrite(wp, T("<ErrorContent>%s</ErrorContent>\r\n"), gErrorContent);
	}
	websWrite(wp, T("</SaveAllPara>\r\n"));
	websFooter(wp);
	websDone(wp, 200);

    log_debug("%s   query:%s   result  %d \n",__func__,query,result);

}

/*****************************************************************************
 函 数 名  : resetAllPara
 功能描述  : 重新读取配置文件，清空当前配置
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void resetAllPara(webs_t wp, char_t *path, char_t *query)
{
	int result = 0;

    memset(gSignalControlpara,0,sizeof(SignalControllerPara));

   // result = LoadDataFromCfg(gSignalControlpara, NULL);
    result = GetSignalControlParams();

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<ResetAllPara>\r\n"));
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</ResetAllPara>\r\n"));
	websFooter(wp);
	websDone(wp, 200);

    log_debug("%s   query:%s   result  %d \n",__func__,query,result);
}

/*****************************************************************************
 函 数 名  : clearAllPara
 功能描述  : 清空配置表，只是将全局变量清零，并不写入文件
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void clearAllPara(webs_t wp, char_t *path, char_t *query)
{
	int result = 0;

    memset(gSignalControlpara,0,sizeof(SignalControllerPara));

    result = 1;


	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<ClearAllPara>\r\n"));
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</ClearAllPara>\r\n"));
	websFooter(wp);
	websDone(wp, 200);

    log_debug("%s   query:%s   result  %d \n",__func__,query,result);
}
/*****************************************************************************
 函 数 名  : getLibsInfo
 功能描述  : 获取本源文件使用的库版本号
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void getLibsInfo(webs_t wp, char_t *path, char_t *query)
{

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<GetLibsInfo>\r\n"));
	//websWrite(wp, T("<IniInfo>%s</IniInfo>\r\n"), "V1.1.1");
	//websWrite(wp, T("<HikCfgInfo>%s</HikCfgInfo>\r\n"), "V2.1.1.1");
	websWrite(wp, T("<WebAppInfo>%s</WebAppInfo>\r\n"), VERSION_WEB_APP);
	
	websWrite(wp, T("<IniInfo>%s</IniInfo>\r\n"), PARSEINI_LIB_VERSION);
	websWrite(wp, T("<HikCfgInfo>%s</HikCfgInfo>\r\n"), HIKCONFIG_LIB_VERSION);
	
	websWrite(wp, T("</GetLibsInfo>\r\n"));
	websFooter(wp);
	websDone(wp, 200);

    log_debug("%s  \n",__func__);

}


/*****************************************************************************
 函 数 名  : vehicleDetector
 功能描述  : 车检器设置回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void vehicleDetector(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
    int account = 0;
    
	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{		
		getXMLValue(query,"VehicleDetector","DetectorNo",tmpStr);
		account = atoi(tmpStr) ;
	
		result = getVehicleDetectorInfo(&gVehicleDetector,account);
	//	log_debug("account %d -- %d  result  %d \n",account,gVehicleDetector.RequestPhase,result);
	}
	else if(reqmethod == METHOD_POST)
	{		
		memset(&gVehicleDetector,0,sizeof(stVehicleDetector));
		getXMLValue(query,"VehicleDetector","DetectorNo",tmpStr);
		gVehicleDetector.DetectorNo = atoi(tmpStr);
        log_debug("%s  DetectorNo  %d\n",__func__,gVehicleDetector.DetectorNo);
		
		getXMLValue(query,"VehicleDetector","RequestPhase",tmpStr);
		gVehicleDetector.RequestPhase = atoi(tmpStr);
		log_debug("%s  RequestPhase  %d\n",__func__,gVehicleDetector.RequestPhase);
		
		getXMLValue(query,"VehicleDetector","SwitchPhase",tmpStr);
		gVehicleDetector.SwitchPhase = atoi(tmpStr);
		log_debug("%s  SwitchPhase  %d\n",__func__,gVehicleDetector.SwitchPhase);
		
		getXMLValue(query,"VehicleDetector","Delay",tmpStr);
		gVehicleDetector.Delay = atoi(tmpStr);
		//log_debug("%s  Delay  %d\n",__func__,gVehicleDetector.Delay);
		
		getXMLValue(query,"VehicleDetector","FailureTime",tmpStr);
		gVehicleDetector.FailureTime = atoi(tmpStr);
		//log_debug("%s  FailureTime  %d\n",__func__,gVehicleDetector.FailureTime);
		
		getXMLValue(query,"VehicleDetector","QueueLimit",tmpStr);
		gVehicleDetector.QueueLimit = atoi(tmpStr);
		//log_debug("%s  QueueLimit  %d\n",__func__,gVehicleDetector.QueueLimit);
		
		getXMLValue(query,"VehicleDetector","NoResponseTime",tmpStr);
		gVehicleDetector.NoResponseTime = atoi(tmpStr);
		log_debug("%s  NoResponseTime  %d\n",__func__,gVehicleDetector.NoResponseTime);
		
		getXMLValue(query,"VehicleDetector","MaxDuration",tmpStr);
		gVehicleDetector.MaxDuration = atoi(tmpStr);
		log_debug("%s  MaxDuration  %d\n",__func__,gVehicleDetector.MaxDuration);
		
		getXMLValue(query,"VehicleDetector","Extend",tmpStr);
		gVehicleDetector.Extend = atoi(tmpStr);
		log_debug("%s  Extend  %d\n",__func__,gVehicleDetector.Extend);
		
		getXMLValue(query,"VehicleDetector","MaxVehicle",tmpStr);
		gVehicleDetector.MaxVehicle = atoi(tmpStr);
		log_debug("%s  MaxVehicle  %d\n",__func__,gVehicleDetector.MaxVehicle);
		
		getXMLValue(query,"VehicleDetector","Flow",tmpStr);
		gVehicleDetector.Flow = atoi(tmpStr);
		//log_debug("%s  Flow  %d\n",__func__,gVehicleDetector.Flow);
		
		getXMLValue(query,"VehicleDetector","Occupancy",tmpStr);
		gVehicleDetector.Occupancy = atoi(tmpStr);
		//log_debug("%s  Occupancy  %d\n",__func__,gVehicleDetector.Occupancy);
		
		getXMLValue(query,"VehicleDetector","ProlongGreen",tmpStr);
		gVehicleDetector.ProlongGreen = atoi(tmpStr);
		//log_debug("%s  ProlongGreen  %d\n",__func__,gVehicleDetector.ProlongGreen);
		
		getXMLValue(query,"VehicleDetector","AccumulateInitial",tmpStr);
		gVehicleDetector.AccumulateInitial = atoi(tmpStr);
		//log_debug("%s  AccumulateInitial  %d\n",__func__,gVehicleDetector.AccumulateInitial);
		
		getXMLValue(query,"VehicleDetector","Queue",tmpStr);
		gVehicleDetector.Queue = atoi(tmpStr);
		//log_debug("%s  Queue  %d\n",__func__,gVehicleDetector.Queue);
		
		getXMLValue(query,"VehicleDetector","Request",tmpStr);
		gVehicleDetector.Request = atoi(tmpStr);
		//log_debug("%s  Request  %d\n",__func__,gVehicleDetector.Request);
		
		getXMLValue(query,"VehicleDetector","RedInterval",tmpStr);
		gVehicleDetector.RedInterval = atoi(tmpStr);
		//log_debug("%s  RedInterval  %d\n",__func__,gVehicleDetector.RedInterval);
		
		getXMLValue(query,"VehicleDetector","YellowInterval",tmpStr);
		gVehicleDetector.YellowInterval = atoi(tmpStr);
		//log_debug("%s  YellowInterval  %d\n",__func__,gVehicleDetector.YellowInterval);
		
		//把单元参数保存到指定ini配置文件中
		result = setVehicleDetectorInfo(&gVehicleDetector,gVehicleDetector.DetectorNo);
	}

	
	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<VehicleDetector>\r\n"));
	websWrite(wp, T("<DetectorNo>%d</DetectorNo>\r\n"), gVehicleDetector.DetectorNo);
	websWrite(wp, T("<RequestPhase>%d</RequestPhase>\r\n"), gVehicleDetector.RequestPhase);
	websWrite(wp, T("<SwitchPhase>%d</SwitchPhase>\r\n"), gVehicleDetector.SwitchPhase);
	websWrite(wp, T("<Delay>%d</Delay>\r\n"), gVehicleDetector.Delay);
	websWrite(wp, T("<FailureTime>%d</FailureTime>\r\n"), gVehicleDetector.FailureTime);
	websWrite(wp, T("<QueueLimit>%d</QueueLimit>\r\n"), gVehicleDetector.QueueLimit);
	websWrite(wp, T("<NoResponseTime>%d</NoResponseTime>\r\n"), gVehicleDetector.NoResponseTime);
	websWrite(wp, T("<MaxDuration>%d</MaxDuration>\r\n"), gVehicleDetector.MaxDuration);
	websWrite(wp, T("<Extend>%d</Extend>\r\n"), gVehicleDetector.Extend);
	websWrite(wp, T("<MaxVehicle>%d</MaxVehicle>\r\n"), gVehicleDetector.MaxVehicle);
	websWrite(wp, T("<Flow>%d</Flow>\r\n"), gVehicleDetector.Flow);
	websWrite(wp, T("<Occupancy>%d</Occupancy>\r\n"), gVehicleDetector.Occupancy);
	websWrite(wp, T("<ProlongGreen>%d</ProlongGreen>\r\n"), gVehicleDetector.ProlongGreen);
	websWrite(wp, T("<AccumulateInitial>%d</AccumulateInitial>\r\n"), gVehicleDetector.AccumulateInitial);
	websWrite(wp, T("<Queue>%d</Queue>\r\n"), gVehicleDetector.Queue);
	websWrite(wp, T("<Request>%d</Request>\r\n"), gVehicleDetector.Request);
	websWrite(wp, T("<RedInterval>%d</RedInterval>\r\n"), gVehicleDetector.RedInterval);
	websWrite(wp, T("<YellowInterval>%d</YellowInterval>\r\n"), gVehicleDetector.YellowInterval);
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);		
	websWrite(wp, T("</VehicleDetector>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}


/*****************************************************************************
 函 数 名  : pedestrianDetector
 功能描述  : 行人检测器回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void pedestrianDetector(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
    int account = 0;

	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{	
		getXMLValue(query,"PedestrianDetector","DetectorNo",tmpStr);
		account = atoi(tmpStr) ;
	
		result = getPedestrianInfo(&gPedestrian,account);
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gPedestrian,0,sizeof(stPedestrian));
		getXMLValue(query,"PedestrianDetector","DetectorNo",tmpStr);
		gPedestrian.DetectorNo = atoi(tmpStr);
        log_debug("%s DetectorNo  %d \n",__func__,gPedestrian.DetectorNo);
		
		getXMLValue(query,"PedestrianDetector","RequestPhase",tmpStr);
		gPedestrian.RequestPhase = atoi(tmpStr);
        log_debug("%s RequestPhase  %d \n",__func__,gPedestrian.RequestPhase);
		
		getXMLValue(query,"PedestrianDetector","NoResponseTime",tmpStr);
		gPedestrian.NoResponseTime = atoi(tmpStr);
		log_debug("%s NoResponseTime  %d \n",__func__,gPedestrian.NoResponseTime);
		
		getXMLValue(query,"PedestrianDetector","MaxDuration",tmpStr);
		gPedestrian.MaxDuration = atoi(tmpStr);
		log_debug("%s MaxDuration  %d \n",__func__,gPedestrian.MaxDuration);
		
		getXMLValue(query,"PedestrianDetector","InductionNumber",tmpStr);
		gPedestrian.InductionNumber = atoi(tmpStr);
		log_debug("%s InductionNumber  %d \n",__func__,gPedestrian.InductionNumber);
		
		//把单元参数保存到指定ini配置文件中
		result = setPedestrianInfo(&gPedestrian,gPedestrian.DetectorNo);
	}
	
	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<PedestrianDetector>\r\n"));
	websWrite(wp, T("<DetectorNo>%d</DetectorNo>\r\n"), gPedestrian.DetectorNo);
	websWrite(wp, T("<RequestPhase>%d</RequestPhase>\r\n"), gPedestrian.RequestPhase);
	websWrite(wp, T("<NoResponseTime>%d</NoResponseTime>\r\n"), gPedestrian.NoResponseTime);
	websWrite(wp, T("<MaxDuration>%d</MaxDuration>\r\n"), gPedestrian.MaxDuration);
	websWrite(wp, T("<InductionNumber>%d</InductionNumber>\r\n"), gPedestrian.InductionNumber);
	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</PedestrianDetector>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : sqliteCallback
 功能描述  : 故障配置中sqlite回调函数
 输入参数  : void *para           
             int n_column         
             char **column_value  
             char **column_name   
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int sqliteCallback(void *para, int n_column, char **column_value, char **column_name)
{
#if 0
	int i;
	char *p = para;

	for (i = 0; i < n_column - 1; i++) {
		p += strlen(p);
		sprintf(p, "%s|", column_value[i]);
	}
	p += strlen(p);
	sprintf(p, "%s;", column_value[i]);

	return SQLITE_OK;
#endif
    return 0;
}


/*****************************************************************************
 函 数 名  : faultConfig
 功能描述  : 故障日志回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void faultConfig(webs_t wp, char_t *path, char_t *query)
{
#if 0
	char tmpStr[48] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	sqlite3 *db = NULL;
	char *errmsg = NULL;
//	int startNum, i;
	char cmd[128] = {0};
	char msg[1024] = {0};

	reqmethod = JudgingType(flags);
	printf("mode: %s, query:%s\n", ((reqmethod == METHOD_GET) ? "GET" : "POST"), query);
	if (reqmethod == METHOD_GET) {
	//added by Jicky
		getXMLValue(query,"FaultConfig","RecordMessage", cmd);
		if (cmd[0] != '\0') {
			result = sqlite3_open("/home/hikTSC.db", &db);
			if(result != SQLITE_OK) {
				printf("open database fail\n");
			} else {
				result = sqlite3_exec(db, cmd, sqliteCallback, msg, &errmsg);
				if (result != SQLITE_OK) {
					log_error("sqlite3_exec called fail, error msg: %s\n", errmsg);
				} else {
					if (strlen(msg) > 0) {
						msg[strlen(msg) - 1] = '\0';	//去除最后结尾的那个;这样web分割字符串时便不会多一个空字串
					}
				}
				sqlite3_close(db);
			}
		} else {
			//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
			result = getFaultConfigInfo(&gFaultConfig,"/home/login.ini");
		}
	}
	else if(reqmethod == METHOD_POST) {
		//if (strncmp(getXMLValue(query,"FaultConfig","DeleteCommand", cmd), "error", 5) != 0) {
		getXMLValue(query,"FaultConfig","DeleteCommand", cmd);
		if (cmd[0] != '\0') {
			result = sqlite3_open("/home/hikTSC.db", &db);
			if(result != SQLITE_OK) {
				printf("open database fail\n");
			} else {
				result = sqlite3_exec(db, cmd, NULL, NULL, &errmsg);
				if (result != SQLITE_OK) {
					log_error("sqlite3_exec called fail, error msg: %s\n", errmsg);
				}
				sqlite3_close(db);
				result = 1;
			}
		} else {

			//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
			memset(&gFaultConfig,0,sizeof(stFaultConfig));
			getXMLValue(query,"FaultConfig","ControlRecord",tmpStr);
			gFaultConfig.ControlRecord = atoi(tmpStr);
			getXMLValue(query,"FaultConfig","LogRecord",tmpStr);
			gFaultConfig.LogRecord = atoi(tmpStr);
#if 0
			getXMLValue(query,"FaultConfig","CommunicatRecord",tmpStr);
			gFaultConfig.CommunicatRecord = atoi(tmpStr);
			getXMLValue(query,"FaultConfig","DetectorRecord",tmpStr);
			gFaultConfig.DetectorRecord = atoi(tmpStr);
#endif
			//把单元参数保存到指定ini配置文件中
			result = saveFaultConfig2Ini(gFaultConfig,"/home/login.ini");
		}
	}
		websHeader(wp);
		websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		websWrite(wp, T("<FaultConfig>\r\n"));
		websWrite(wp, T("<ControlRecord>%d</ControlRecord>\r\n"), gFaultConfig.ControlRecord);
		websWrite(wp, T("<LogRecord>%d</LogRecord>\r\n"), gFaultConfig.LogRecord);
#if 0
		websWrite(wp, T("<CommunicatRecord>%d</CommunicatRecord>\r\n"), gFaultConfig.CommunicatRecord);
		websWrite(wp, T("<DetectorRecord>%d</DetectorRecord>\r\n"), gFaultConfig.DetectorRecord);
#endif
		if (reqmethod == METHOD_GET) {
			websWrite(wp, T("<RecordMessage>%s</RecordMessage>\r\n"), msg);
		}
		websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
		websWrite(wp, T("</FaultConfig>\r\n"));
		websFooter(wp);
		websDone(wp, 200);
#endif
}

/*****************************************************************************
 函 数 名  : actionDownload
 功能描述  : 把本地配置文件上传到WEB
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void actionDownload(webs_t wp, char_t *path, char_t *query)
{
/*	char buffer[10240];
	const char * file = CFG_NAME;
	int result = 0;
	FILE *stream = NULL;

    stream = fopen(file, "r");
	result = fread(buffer,1,10240,stream);
    fclose(stream);
	buffer[result] = '\0';
*/
	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n"));
	websWrite(wp, T("<SaveParams>\r\n"));
	websWrite(wp, T("<download>%s</download>\r\n"), gSignalControlpara);
	websWrite(wp, T("<statusCode>1</statusCode>\r\n"));
	websWrite(wp, T("</SaveParams>\r\n"));
	websFooter(wp);
	websDone(wp, 200);

	log_debug("%s  fileSize  %d\n",__func__,sizeof(SignalControllerPara));
}

/*****************************************************************************
 函 数 名  : TreeDynamicParameter
 功能描述  : 动态参数树回调函数
 输入参数  : webs_t wp      
             char_t *path   
             char_t *query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年2月2日
    作    者   : 肖文虎
    修改内容   : 添加对车辆检测器及行人检测器的支持
*****************************************************************************/
void TreeDynamicParameter(webs_t wp, char_t *path, char_t *query)
{
	char tmpStr[20480] = {0};
	int reqmethod = METHOD_GET;
	int flags = websGetRequestFlags(wp);
	int result = 0;
	int i = 0;
    int j = 0 ;
    char *p = NULL;
	reqmethod = JudgingType(flags);
	if (reqmethod == METHOD_GET)
	{		//如果是GET或者POST命令，则把配置文件中获取的参数发给网页客户端
		//result = getTreeDynamicParameter(&gTreeDynamicPara);
		//GetSignalControlParams();
		result = 1;
	}
	else if(reqmethod == METHOD_POST)
	{		//如果是PUT命令，则从客户端发送过来的xml进行解析并保存到配置文件中，并增加设置是否成功的结果一起返回
		memset(&gTreeDynamicPara,0,sizeof(stTreeDynamicPara));
		getXMLValue(query,"TreeDynamicParameter","addCount",tmpStr);
		gTreeDynamicPara.addCount = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","addChannel",tmpStr);
		gTreeDynamicPara.addChannel = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","addProgram",tmpStr);
		gTreeDynamicPara.addProgram = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","addSplit",tmpStr);
		gTreeDynamicPara.addSplit = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","nPhaseTurnCount",tmpStr);
		gTreeDynamicPara.nPhaseTurnCount = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","nActionCount",tmpStr);
		gTreeDynamicPara.nActionCount = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","nTimeInterval",tmpStr);
		gTreeDynamicPara.nTimeInterval = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","nScheduleCount",tmpStr);
		gTreeDynamicPara.nScheduleCount = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","nFollowPhaseCount",tmpStr);
		gTreeDynamicPara.nFollowPhaseCount = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","nVehicleDetectorCount",tmpStr);
		gTreeDynamicPara.nVehicleDetectorCount = atoi(tmpStr);

		getXMLValue(query,"TreeDynamicParameter","nPedestrianDetectorCount",tmpStr);
		gTreeDynamicPara.nPedestrianDetectorCount = atoi(tmpStr);


        //arry list
        getXMLValue(query,"TreeDynamicParameter","PhaseArray",tmpStr);
        i = 0;
        while(i++ < NUM_PHASE)
        {
            gSignalControlpara->stPhase[i-1].nPhaseID = 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  PhaseArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_PHASE))
            {
                gSignalControlpara->stPhase[i - 1].nPhaseID = i;
            }
            //log_debug("Phase  ===>  %d\n",i);
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_PHASE))
                {
                    gSignalControlpara->stPhase[i - 1].nPhaseID = i;//然后再将WEB传过来的数据填充到数组中
                    //log_debug("Phase  ===>  %d\n",i);
                }
            }

        }

        //channel arry
        getXMLValue(query,"TreeDynamicParameter","channelArray",tmpStr);
        i = 0;
        while(i++ < NUM_CHANNEL)
        {
            gSignalControlpara->stChannel[i-1].nChannelID = 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  ChannelArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_CHANNEL))
            {
                gSignalControlpara->stChannel[i - 1].nChannelID = i;
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_CHANNEL))
                {
                    gSignalControlpara->stChannel[i - 1].nChannelID = i;//然后再将WEB传过来的数据填充到数组中
                }
            }
        }        

        //方案表
        getXMLValue(query,"TreeDynamicParameter","programArray",tmpStr);
        i = 0;
        while(i++ < NUM_SCHEME)
        {
            gSignalControlpara->stScheme[i-1].nSchemeID= 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  SchemeArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_SCHEME))
            {
                gSignalControlpara->stScheme[i - 1].nSchemeID = i;
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_SCHEME))
                {
                    gSignalControlpara->stScheme[i - 1].nSchemeID = i;//然后再将WEB传过来的数据填充到数组中
                }
            }
        }

        //绿信比表
        getXMLValue(query,"TreeDynamicParameter","splitArray",tmpStr);
        i = 0;
        while(i++ < NUM_GREEN_SIGNAL_RATION)
        {   
            j = 0;
            while(j++ < NUM_PHASE)
            {
                gSignalControlpara->stGreenSignalRation[i - 1][j-1].nGreenSignalRationID= 0;//先将所有ID设置为0，
            }
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  GreenSignalRationArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_GREEN_SIGNAL_RATION))
            {
                j = 0;
                while(j++ < NUM_PHASE)
                {
                    gSignalControlpara->stGreenSignalRation[i - 1][j-1].nGreenSignalRationID = i;
                }
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_GREEN_SIGNAL_RATION))
                {
                    j = 0;
                    while(j++ < NUM_PHASE)//然后再将WEB传过来的数据填充到数组中
                    {
                        gSignalControlpara->stGreenSignalRation[i - 1][j-1].nGreenSignalRationID = i;
                    }
                }
            }
        }
        //相序表
        getXMLValue(query,"TreeDynamicParameter","phaseTurnArray",tmpStr);
        i = 0;
        while(i++ < NUM_PHASE_TURN)
        {
            j = 0;
            while(j++ < 4)
            {
                gSignalControlpara->stPhaseTurn[i-1][j-1].nPhaseTurnID= 0;//先将所有ID设置为0，
            }
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  PhaseTurn : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_PHASE_TURN))
            {
                j = 0;
                while(j++ < 4)
                {
                    gSignalControlpara->stPhaseTurn[i-1][j-1].nPhaseTurnID= i;//先将所有ID设置为0，
                }

            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_PHASE_TURN))
                {
                    j = 0;
                    while(j++ < 4)
                    {
                        gSignalControlpara->stPhaseTurn[i-1][j-1].nPhaseTurnID= i;//先将所有ID设置为0，
                    }

                }
            }
        }

        //动作表
        getXMLValue(query,"TreeDynamicParameter","ActionArray",tmpStr);
        i = 0;
        while(i++ < NUM_ACTION)
        {
            gSignalControlpara->stAction[i-1].nActionID= 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  ActionArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_ACTION))
            {
                gSignalControlpara->stAction[i - 1].nActionID = i;
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_ACTION))
                {
                    gSignalControlpara->stAction[i - 1].nActionID = i;//然后再将WEB传过来的数据填充到数组中
                }
            }
        }

        //时段表
        getXMLValue(query,"TreeDynamicParameter","TimeIntervalArray",tmpStr);
        i = 0;
        while(i++ < NUM_TIME_INTERVAL)
        {
            j = 0;
            while(j++ < NUM_TIME_INTERVAL_ID)
            {
                gSignalControlpara->stTimeInterval[i-1][j-1].nTimeIntervalID = 0;//先将所有ID设置为0，
            }
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  TimeIntervalArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            j = 0;
            
            if((i >= 1) && (i <= NUM_TIME_INTERVAL))
            {
                while(j++ < NUM_TIME_INTERVAL_ID)
                {
                    gSignalControlpara->stTimeInterval[i - 1][j-1].nTimeIntervalID = i;
                }
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_TIME_INTERVAL))
                {
                    j = 0;
                    while(j++ < NUM_TIME_INTERVAL_ID)
                    {
                        gSignalControlpara->stTimeInterval[i - 1][j-1].nTimeIntervalID = i;//然后再将WEB传过来的数据填充到数组中
                    }
                }
            }
        }

        //跟随相位表
        getXMLValue(query,"TreeDynamicParameter","FollowPhaseArray",tmpStr);
        i = 0;
        while(i++ < NUM_FOLLOW_PHASE)
        {
            gSignalControlpara->stFollowPhase[i-1].nFollowPhaseID = 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  FollowPhaseArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_FOLLOW_PHASE))
            {
                gSignalControlpara->stFollowPhase[i - 1].nFollowPhaseID = i;
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_FOLLOW_PHASE))
                {
                    gSignalControlpara->stFollowPhase[i - 1].nFollowPhaseID = i;//然后再将WEB传过来的数据填充到数组中
                }
            }
        }

        //调度表
        getXMLValue(query,"TreeDynamicParameter","ScheduleArray",tmpStr);
        i = 0;
        while(i++ < NUM_SCHEDULE)
        {
            gSignalControlpara->stPlanSchedule[i-1].nScheduleID = 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  ScheduleArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= NUM_SCHEDULE))
            {
                gSignalControlpara->stPlanSchedule[i - 1].nScheduleID = i;
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_SCHEDULE))
                {
                    gSignalControlpara->stPlanSchedule[i - 1].nScheduleID = i;//然后再将WEB传过来的数据填充到数组中
                }
            }
        }

        // 车辆检测器
        getXMLValue(query,"TreeDynamicParameter","VehicleDetectorArray",tmpStr);
        i = 0;
        while(i++ < MAX_VEHICLEDETECTOR_COUNT)
        {
            gSignalControlpara->AscVehicleDetectorTable[i-1].byVehicleDetectorNumber = 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  VehicleDetectorArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= MAX_VEHICLEDETECTOR_COUNT))
            {
                gSignalControlpara->AscVehicleDetectorTable[i-1].byVehicleDetectorNumber = i;
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_SCHEDULE))
                {
                    gSignalControlpara->AscVehicleDetectorTable[i-1].byVehicleDetectorNumber = i;//然后再将WEB传过来的数据填充到数组中
                }
            }
        }

        //行人检测器
        getXMLValue(query,"TreeDynamicParameter","PedestrianDetectorArray",tmpStr);
        i = 0;
        while(i++ < MAX_PEDESTRIANDETECTOR_COUNT)
        {
            gSignalControlpara->AscPedestrianDetectorTable[i-1].byPedestrianDetectorNumber = 0;//先将所有ID设置为0，
        }        
        if(strlen(tmpStr) > 0)
        {   
            log_debug("%s  PedestrianDetetorArray : %s\n",__func__,tmpStr);
            i = atoi(strtok(tmpStr,","));
            if((i >= 1) && (i <= MAX_PEDESTRIANDETECTOR_COUNT))
            {
                gSignalControlpara->AscPedestrianDetectorTable[i-1].byPedestrianDetectorNumber = i;
            }
            while((p = strtok(NULL,",")))
            {
                i = atoi(p);
                if((i >= 1) && (i <= NUM_SCHEDULE))
                {
                    gSignalControlpara->AscPedestrianDetectorTable[i-1].byPedestrianDetectorNumber = i;//然后再将WEB传过来的数据填充到数组中
                }
            }
        }


		//log_debug("nPhaseTurnCount  %d  %d \n",gTreeDynamicPara.nPhaseTurnCount,gTreeDynamicPara.addSplit);
		//把单元参数保存到指定ini配置文件中
		//result = saveTreeDynamicParameter(&gTreeDynamicPara);
		result = 1;
	}

	websHeader(wp);
	websWrite(wp, T("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
	websWrite(wp, T("<TreeDynamicParameter>\r\n"));

    //相位表总数
    memset(&gTreeDynamicPara,0,sizeof(stTreeDynamicPara));
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_PHASE ; i++)
    {
        //if(gSignalControlpara->stPhase[i].nPhaseID != 0)
        if(IS_PHASE_INABLE(gSignalControlpara->stPhase[i].wPhaseOptions) == 1)
        {
            gTreeDynamicPara.addCount++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stPhase[i].nPhaseID);
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<addCount>%d</addCount>\r\n"), gTreeDynamicPara.addCount);//相位表总数
    websWrite(wp, T("<PhaseArray>%s</PhaseArray>\r\n"), tmpStr);//相位表ID数组
    //log_debug("%s  send phase array   %s    count  %d\n",__func__,tmpStr,gTreeDynamicPara.addCount);

    //通道表总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_CHANNEL ; i++)
    {
        if(gSignalControlpara->stChannel[i].nControllerID != 0)
        {
            gTreeDynamicPara.addChannel++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stChannel[i].nChannelID);
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<addChannel>%d</addChannel>\r\n"), gTreeDynamicPara.addChannel);
    websWrite(wp, T("<ChannelArray>%s</ChannelArray>\r\n"), tmpStr);

    //绿信比表总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_GREEN_SIGNAL_RATION ; i++)
    {
        for(j = 0 ; j <  NUM_PHASE; j++)
        {
            if(gSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationID != 0)
            {
                gTreeDynamicPara.addSplit++;
                sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stGreenSignalRation[i][j].nGreenSignalRationID);
                break;
            }
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<addSplit>%d</addSplit>\r\n"), gTreeDynamicPara.addSplit);
    websWrite(wp, T("<GreenSignalRationArray>%s</GreenSignalRationArray>\r\n"),tmpStr);
    

    //方案表总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_SCHEME; i++)
    {
        if(gSignalControlpara->stScheme[i].nSchemeID != 0)
        {
            gTreeDynamicPara.addProgram++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stScheme[i].nSchemeID);
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<addProgram>%d</addProgram>\r\n"), gTreeDynamicPara.addProgram);
    websWrite(wp, T("<SchemeArray>%s</SchemeArray>\r\n"),tmpStr);

    //相序表总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ;  i < NUM_PHASE_TURN ; i++)
    {
        for(j = 0 ; j < 4 ; j++)
        {
            if(gSignalControlpara->stPhaseTurn[i][j].nTurnArray[0] != 0)
            {
                gTreeDynamicPara.nPhaseTurnCount++;
                sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stPhaseTurn[i][j].nPhaseTurnID);
                break;
            }
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<nPhaseTurnCount>%d</nPhaseTurnCount>\r\n"), gTreeDynamicPara.nPhaseTurnCount);
    websWrite(wp, T("<PhaseTurnArray>%s</PhaseTurnArray>\r\n"), tmpStr);
    
    //动作表总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_ACTION ; i++)
    {
        if((gSignalControlpara->stAction[i].nActionID != 0) 
            && (gSignalControlpara->stAction[i].nActionID < 115) )
        {
            gTreeDynamicPara.nActionCount++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stAction[i].nActionID);
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<nActionCount>%d</nActionCount>\r\n"), gTreeDynamicPara.nActionCount);
    websWrite(wp, T("<ActionArray>%s</ActionArray>\r\n"),tmpStr);

    //时段表总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID ; j++)
        {
            if(gSignalControlpara->stTimeInterval[i][j].nTimeIntervalID != 0)
            {
                gTreeDynamicPara.nTimeInterval++;
                sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stTimeInterval[i][j].nTimeIntervalID);
                break;
            }
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<nTimeInterval>%d</nTimeInterval>\r\n"), gTreeDynamicPara.nTimeInterval);
    websWrite(wp, T("<TimeIntervalArray>%s</TimeIntervalArray>\r\n"), tmpStr);

    //调度计划表总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_SCHEDULE ; i++)
    {
        if(gSignalControlpara->stPlanSchedule[i].nScheduleID != 0)
        {
            gTreeDynamicPara.nScheduleCount++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stPlanSchedule[i].nScheduleID);
        }
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<nScheduleCount>%d</nScheduleCount>\r\n"), gTreeDynamicPara.nScheduleCount);
    websWrite(wp, T("<ScheduleArray>%s</ScheduleArray>\r\n"), tmpStr);

    //跟随相位总数
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < NUM_FOLLOW_PHASE; i++)
    {
        if(gSignalControlpara->stFollowPhase[i].nArrayMotherPhase[0] != 0)
        {
            gTreeDynamicPara.nFollowPhaseCount++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->stFollowPhase[i].nFollowPhaseID);
        }

    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<nFollowPhaseCount>%d</nFollowPhaseCount>\r\n"), gTreeDynamicPara.nFollowPhaseCount);
    websWrite(wp, T("<FollowPhaseArray>%s</FollowPhaseArray>\r\n"),tmpStr);

    
    //车辆检测器
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < MAX_VEHICLEDETECTOR_COUNT; i++)
    {
        if(gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase != 0)
        {
            gTreeDynamicPara.nVehicleDetectorCount++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorNumber);
        }

    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<nVehicleDetectorCount>%d</nVehicleDetectorCount>\r\n"), gTreeDynamicPara.nVehicleDetectorCount);
    websWrite(wp, T("<VehicleDetectorArray>%s</VehicleDetectorArray>\r\n"),tmpStr);
    
    //行人检测器
    memset(tmpStr,0,sizeof(tmpStr));
    for(i = 0 ; i < MAX_PEDESTRIANDETECTOR_COUNT; i++)
    {
        if(gSignalControlpara->AscPedestrianDetectorTable[i].byPedestrianDetectorNumber != 0)
        {
            gTreeDynamicPara.nPedestrianDetectorCount++;
            sprintf(tmpStr+strlen(tmpStr),"%d,",gSignalControlpara->AscPedestrianDetectorTable[i].byPedestrianDetectorNumber);
        }

    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    websWrite(wp, T("<nPedestrianDetectorCount>%d</nPedestrianDetectorCount>\r\n"), gTreeDynamicPara.nPedestrianDetectorCount);
    websWrite(wp, T("<PedestrianDetectorArray>%s</PedestrianDetectorArray>\r\n"),tmpStr);

	websWrite(wp, T("<statusCode>%d</statusCode>\r\n"), result);
	websWrite(wp, T("</TreeDynamicParameter>\r\n"));
	websFooter(wp);
	websDone(wp, 200);
}

/*****************************************************************************
 函 数 名  : upldForm
 功能描述  : 把WEB上传来的配置文件保存在本地
 输入参数  : webs_t wp       
             char_t * path   
             char_t * query  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void upldForm(webs_t wp, char_t * path, char_t * query)
{
    FILE *       fp;
    char_t *     fn;
    char_t *     bn = NULL;
    int          locWrite;
    int          numLeft;
    int          numWrite = 0;

    a_assert(websValid(wp));
    websHeader(wp);

    fn = websGetVar(wp, T("filename"), T(""));
    if (fn != NULL && *fn != '\0')
	{
        if ((int)(bn = gstrrchr(fn, '/') + 1) == 1)
		{
            if ((int)(bn = gstrrchr(fn, '\\') + 1) == 1)
			{
                bn = fn;
            }
        }
    }

    websWrite(wp, T("Filename = %s<br>Size = %d bytes<br>"), bn, wp->lenPostData);

    if ((fp = fopen((bn == NULL ? CFG_NAME : bn), "w+b")) == NULL)
	{
        websWrite(wp, T("File open failed!<br>"));
    }
	else
	{
        locWrite = 0;
        numLeft = wp->lenPostData;
        while (numLeft > 0)
		{
            numWrite = fwrite(&(wp->postData[locWrite]), sizeof(*(wp->postData)), numLeft, fp);
            if (numWrite < numLeft)
			{
                websWrite(wp, T("File write failed.<br>  ferror=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), ferror(fp), locWrite, numLeft, numWrite, wp->lenPostData);
            	break;
            }
            locWrite += numWrite;
            numLeft -= numWrite;
        }

        if (numLeft == 0)
		{
            if (fclose(fp) != 0)
			{
                websWrite(wp, T("File close failed.<br>  errno=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), errno, locWrite, numLeft, numWrite, wp->lenPostData);
            }
			else
			{
                websWrite(wp, T("File Size Written = %d bytes<br>"), wp->lenPostData);
            }
        }
		else
		{
            websWrite(wp, T("numLeft=%d locWrite=%d Size=%d bytes<br>"), numLeft, locWrite, wp->lenPostData);
        }
    }

    websFooter(wp);
    websDone(wp, 200);

    log_debug("%s    \n",__func__);
}


/*****************************************************************************
 函 数 名  : getXMLValue
 功能描述  : 解析XML数据
 输入参数  : char * XMLStr       
             char * rootTag      
             char * rootTagName  
             char * returnValue  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年11月25日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
char * getXMLValue(char * XMLStr,char * rootTag,char * rootTagName,char * returnValue)
{
	int len;
//    char *pRes;
    char *XMLSTR_init;				//xml有效信息
    UTIL_XML_REQ req;
    UTIL_XML_TAG *tag;

    XMLSTR_init = strstr(XMLStr,"<?xml version='1.0' encoding='utf-8'?>");//只取有效信息
	if (XMLSTR_init == NULL) {
		return "error";
	}
	util_xml_init(&req);
    //if((len=util_xml_validate(&req, XMLStr, strlen(XMLStr))) < 0)
    if((len=util_xml_validate(&req, XMLSTR_init, strlen(XMLSTR_init))) < 0)
    {
        util_xml_cleanup(&req);
        return "error";
    }
    tag = req.root_tag;
    if(!tag || util_xml_matchtag(rootTag, tag->name) != 0)
    {
        util_xml_cleanup(&req);
        return "error";
    }
	tag = tag->first_child;
    if(!tag)
    {
        util_xml_cleanup(&req);
        return "error";
    }
	while(tag)
	{
		if(strcmp((const char *)rootTagName, tag->name) ==0)
		{
			if(tag->value)
			{
				strcpy(returnValue,tag->value);
			}
			else
			{

				strcpy(returnValue,"");
			}
			break;
		}
		tag = tag->next;
	}
    	util_xml_cleanup(&req);
	return returnValue;
}



