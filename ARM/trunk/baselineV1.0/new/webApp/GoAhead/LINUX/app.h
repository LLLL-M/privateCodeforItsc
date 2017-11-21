#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <sys/socket.h>
#include "HikConfig.h"
#include "its.h"
#include "platform.h"
#include "myiconv.h"

#define ITOA(I) ({char _buf_[8] = {0};sprintf(_buf_, "%d", I);_buf_;})
#define XMLVALUE(ND) ({\
		xmlChar *szKey = xmlNodeGetContent(ND);\
		int val = 0;\
		if (szKey != NULL) {\
			val = atoi((const char *)szKey);\
			xmlFree(szKey);\
		}\
		val;})

static inline void GetLocalTSCData(int type, void *buf, int len)
{
	struct sockaddr_in localAddr = 
	{   
		.sin_family = PF_INET,
		.sin_addr = {inet_addr("127.0.0.1")},
		.sin_port = htons(20000),
		.sin_zero = {0},
	};
	socklen_t localLen = sizeof(localAddr);
	int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == socketFd)
	{
		ERR("create socket fail");
		return;
	}
	int sendData[260] = {0x6e6e, type, 0};
	sendto(socketFd, sendData, sizeof(sendData), 0, (struct sockaddr *)&localAddr, localLen);
	recvfrom(socketFd, sendData, sizeof(sendData), 0, (struct sockaddr *)&localAddr, &localLen);
	close(socketFd);
	memcpy(buf, &sendData[2], len);
}

static xmlNodePtr CreateSplitTable(GreenSignalRationItem *item, char (*phaseDesc)[64])
{
	xmlNodePtr splitTableNode, phaseNode; //tableNoNode
	int i, tableNo = 0;
	xmlChar outbuf[OUTLEN];

	splitTableNode = xmlNewNode(NULL, BAD_CAST "splitTable");
	if(splitTableNode == NULL)
	{
		ERR("Fail to create splitTableNode");
		return NULL;
	}
	//tableNoNode = xmlNewChild(splitTableNode, NULL, BAD_CAST "tableNo", BAD_CAST "0");
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (item[i].nGreenSignalRationTime == 0)
			continue;
		tableNo = item[i].nGreenSignalRationID;
		phaseNode = xmlNewChild(splitTableNode, NULL, BAD_CAST "phase", NULL);
		xmlSetProp(phaseNode, BAD_CAST "id", BAD_CAST ITOA(i + 1));
		xmlNewChild(phaseNode, NULL, BAD_CAST "time", BAD_CAST ITOA(item[i].nGreenSignalRationTime));
		g2u(phaseDesc[i], (char *)outbuf);
		xmlNewChild(phaseNode, NULL, BAD_CAST "desc", outbuf);
		INFO("splitTable: %d, phase: %d, time: %d, desc: %s", item[i].nGreenSignalRationID, i + 1, item[i].nGreenSignalRationTime, outbuf);
	}
	if (tableNo == 0)
	{
		xmlFreeNode(splitTableNode);
		return NULL;
	}
	//xmlNodeSetContent(tableNoNode, BAD_CAST ITOA(tableNo));
	return splitTableNode;
}

static void ParseSplitTable(xmlXPathObjectPtr result, UInt8 nSplitId)
{
	xmlNodePtr *nodeSets = result->nodesetval->nodeTab;
	xmlNodePtr phaseNode, curNode;
	xmlChar *prop;
	GreenSignalRationItem *item = gSignalControlpara->stGreenSignalRation[nSplitId - 1];
	int i, j;
	int phaseId = 0, splitTime = 0, cycleTime = 0;

	for (i = 0; i < result->nodesetval->nodeNr; i++) 
	{
		phaseNode = nodeSets[i];
		prop = xmlGetProp(phaseNode, BAD_CAST "id");
		if (prop == NULL)
			continue;
		phaseId = atoi((const char *)prop);
		xmlFree(prop);
		if (phaseId == 0)
			continue;
		for (curNode = phaseNode->xmlChildrenNode; curNode != NULL; curNode = curNode->next) 
		{
			if (xmlStrcmp(curNode->name, BAD_CAST "time") == 0)
			{
				splitTime = XMLVALUE(curNode);
				item[phaseId - 1].nGreenSignalRationTime = splitTime;
				cycleTime += splitTime;
				INFO("phase %d, splitTime %d", phaseId, splitTime);
			}
		}
	}
	if (cycleTime > 0)
	{
		for (j = 0; j < NUM_SCHEME; j++)
		{
			if (gSignalControlpara->stScheme[j].nGreenSignalRatioID == nSplitId)
				gSignalControlpara->stScheme[j].nCycleTime = (UInt8)cycleTime;
		}
	}
}

static Boolean SetSplitTable(const char *xmldata, int len, UInt8 nSplitId)
{
	xmlDocPtr doc;
	xmlXPathContextPtr context;    //XPATH上下文指针
	xmlXPathObjectPtr result;       //XPATH对象指针，用来存储查询结果
	Boolean ret = FALSE;

	if (nSplitId == 0)
	{
		ERR("current use splitid = 0 error");
		return FALSE;
	}
	xmlKeepBlanksDefault(0);
	doc = xmlParseMemory(xmldata, len);
	if (doc == NULL)
	{
		ERR("parse xml memory error");
		return FALSE;
	}
	context = xmlXPathNewContext(doc);     //创建一个XPath上下文指针
	if (context == NULL)
	{
		ERR("create xpath context fail!");
		goto out2;
	}
	result = xmlXPathEvalExpression(BAD_CAST "/split/splitTable/phase", context); //查询XPath表达式，得到一个查询结果
	xmlXPathFreeContext(context);             //释放上下文指针
	if (result == NULL) {
		ERR("xmlXPathEvalExpression return NULL");
		goto out2;
	}
	if (xmlXPathNodeSetIsEmpty(result->nodesetval)) { //检查查询结果是否为空
		ERR("nodeset is empty");
		goto out1;
	}
	ParseSplitTable(result, nSplitId);
	SendSignalControlParams();	//发送设置参数到主控程序
	ret = TRUE;
out1:
	xmlXPathFreeObject(result);
out2:
	xmlFreeDoc(doc);
	return ret;
}

static xmlChar *GetSplitTableXml(UInt8 nSplitId)
{
	xmlDocPtr pdoc;
	xmlNodePtr root;
	xmlChar *outbuf;
	char phaseDesc[16][64];	//相位描述
	int outlen;

	if (nSplitId == 0)
	{
		ERR("current use splitid = 0 error");
		return NULL;
	}
	pdoc = xmlNewDoc(BAD_CAST "1.0");
	if(pdoc == NULL)
	{
		ERR("Fail to create new XML doc");
		return NULL;
	}
	root = xmlNewNode(NULL, BAD_CAST "split");
	if(root == NULL)
	{
		ERR("Fail to create split node");
		xmlFreeDoc(pdoc);
		return NULL;
	}
	xmlDocSetRootElement(pdoc, root);
	GetSignalControlParams();	//从主控程序中获取全部参数
	GetLocalTSCData(0x9b, (char *)phaseDesc, sizeof(phaseDesc));
	xmlAddChild(root, CreateSplitTable(gSignalControlpara->stGreenSignalRation[nSplitId - 1], phaseDesc));
	//xmlSaveFormatFileEnc("/opt/app.xml", pdoc, "utf-8", 1);
	xmlDocDumpFormatMemoryEnc(pdoc, &outbuf, &outlen, "utf-8", 1);
	printf("/************************/\n%s\n/************************/\n", outbuf);
	xmlFreeDoc(pdoc);
	return outbuf;
}

static Boolean SetSpecialControl(const char *xmldata, int len)
{
	xmlDocPtr doc;
	xmlNodePtr root, curNode;
	UInt8 schemeId = 0, stageNo = 0;

	xmlKeepBlanksDefault(0);
	doc = xmlParseMemory(xmldata, len);
	if (doc == NULL)
	{
		ERR("parse xml memory error");
		return FALSE;
	}
	root = xmlDocGetRootElement(doc);
	if (root == NULL)
	{
		ERR("xmlDocGetRootElement fail");
		xmlFreeDoc(doc);
		return FALSE;
	}
	for (curNode = root->xmlChildrenNode; curNode != NULL; curNode = curNode->next) 
	{
		if (xmlStrcmp(curNode->name, BAD_CAST "schemeId") == 0)
			schemeId = (UInt8)XMLVALUE(curNode);
		else if (xmlStrcmp(curNode->name, BAD_CAST "stageNo") == 0)
			stageNo = (UInt8)XMLVALUE(curNode);
	}
	INFO("special control, schemeId = %d, stageNo = %d", schemeId, stageNo);
	ItsCtl(WEB_CONTROL, schemeId, stageNo);
	return TRUE;
}

void splitTable(webs_t wp, char_t *path, char_t *query)
{
	int flags = websGetRequestFlags(wp);
	int reqmethod = JudgingType(flags);
	xmlChar *data = NULL;
	MsgRealtimePattern realtimeinfo;

	GetLocalTSCData(0xd0, &realtimeinfo.nPatternId, sizeof(MsgRealtimePattern) - 8);
	websHeader(wp);
	if (reqmethod == METHOD_GET)
	{
		data = GetSplitTableXml(realtimeinfo.nSplitId);
		if (data == NULL)
			websWrite(wp, T("Can't get split table data"));
		else
		{
			websWrite(wp, T("%s"), data);
			xmlFree(data);
		}
	}
	else if (reqmethod == METHOD_POST)
	{
		INFO("####### query ###########\n%s\n", query);
		INFO("******* postData ********\n%s\n", wp->postData);
		if (SetSplitTable(wp->postData, wp->lenPostData, realtimeinfo.nSplitId))
			websWrite(wp, T("set successful!"));
		else
			websWrite(wp, T("set fail!"));
	}
	else
	{
		ERR("flags %d isn't identified!", flags);
		websWrite(wp, T("unknown request flags %d"), flags);
	}
	websFooter(wp);
	websDone(wp, 200);
}

void specialControl(webs_t wp, char_t *path, char_t *query)
{
	int flags = websGetRequestFlags(wp);
	int reqmethod = JudgingType(flags);

	websHeader(wp);
	if (reqmethod == METHOD_POST)
	{
		if (SetSpecialControl(query, gstrlen(query)))
			//websWrite(wp, T("set successful!"));
			websWrite(wp, T("%s"), query);
		else
			websWrite(wp, T("set fail!"));
	}
	else
	{
		ERR("flags %d isn't identified!", flags);
		websWrite(wp, T("unknown request flags %d"), flags);
	}
	websFooter(wp);
	websDone(wp, 200);
}
