#include <string.h>
#include <stdlib.h>
#include "mxmlresolve.h"
#include "hik.h"

static mxml_node_t *mxmlpTree=NULL;
static mxml_node_t *mxmlpMsg=NULL;
static char *mxmlLabels[]={"var","str", "arr", "d1", "d2"};

/************************parse xml file or string***********************/
char mxmlParseStart(char *str)
{
	char ret=0;
	FILE *fp=NULL;

	if(str == NULL)
		return 0;

	if(strstr(str,".xml")!=NULL && strlen(str)<32)//str is a xml file name
	{
		fp = fopen(str, "r");
		if(fp == NULL)
		{
			ERR("Can't open file: %s\n", str);
			return 0;
		}
		mxmlpTree = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
		fclose(fp);
	}
	else// str is a xml text
	{
		mxmlpTree = mxmlLoadString(NULL, str, MXML_OPAQUE_CALLBACK);
	}
	if(mxmlpTree)
		mxmlpMsg = mxmlFindElement(mxmlpTree, mxmlpTree, "MessageEx", NULL, NULL, MXML_DESCEND);
	if(mxmlpMsg)
		ret = 1;
	return ret;
}
char *mxmlGetMsgOp(void)
{
	char *op=NULL;
	op = (char*)mxmlElementGetAttr(mxmlpMsg, "op");
	//INFO(" op: %s \n", op);
	return op;
}
char *mxmlGetMsgType(void)
{
	char *type=NULL;
	type = (char*)mxmlElementGetAttr(mxmlpMsg, "type");
	//INFO(" type: %s \n", type);
	return type;
}
static char *mxmlGetSimpleNode(mxmlNode *info)
{
	char *ret=NULL;
	mxml_node_t *tmp=NULL;
	if(info == NULL || info->sub != NULL)	
	{
		ERR("Param can't be NULL or parent!");
		return NULL;
	}
	if(info->type == MXML_SIMPLE_ARR && info->id1 != 0)
	{
		mxml_node_t *node=NULL;
		char id1[4]={0};
		sprintf(id1,"%d", info->id1);
		for(node=mxmlFindElement(info->cur, info->top, mxmlLabels[info->type-1], "id", id1, MXML_DESCEND);
			node!=NULL;node=mxmlFindElement(node, info->cur, mxmlLabels[info->type-1], "id", id1, MXML_DESCEND))
		{
			if(strcmp(info->ob, mxmlElementGetAttr(node,"ob")) == 0)
			{
				if(node->child != NULL)
				{		
					ret = node->child->value.opaque;
				}
				else
					ERR("Child of node is NULL!");
				break;
			}
		}
	}
	else
	{	
		tmp = mxmlFindElement(info->cur,info->top,mxmlLabels[info->type-1],"ob",info->ob,MXML_DESCEND);
		if(tmp != NULL)
		{
			if(tmp->child!=NULL)
				ret = tmp->child->value.opaque;
			else
				ERR("Child of node is NULL!");
		}
	}
	return ret;
}
static char *mxmlGetValue(mxmlNode *info)
{
//	mxml_node_t *tmp=NULL;
	char *ret=NULL;
	if(info == NULL)
	{
		ERR("mxmlGetValue: info can't be NULL!\n");
		return NULL;
	}
	if(info->type <= MXML_SIMPLE_ARR)	//single label
	{
		ret = mxmlGetSimpleNode(info);
	}
	else
	{
		mxml_node_t *node=NULL;
		int id2=0;
		char id1[4]={0};
		sprintf(id1,"%d", info->id1);
//		printf("type:%d, id1:%s, id2:%d, ob:%s\n", info->dlab->type, id1, info->cArr->id2, info->cArr->ob);
		//tmp = mxmlFindElement(info->cur, info->top, "d2", );	
		for(node=mxmlFindElement(info->cur, info->top, mxmlLabels[info->type-1], "id1", id1, MXML_DESCEND);
			node!=NULL;node=mxmlFindElement(node, info->cur, mxmlLabels[info->type-1], "id1", id1, MXML_DESCEND))
		{
			if(strcmp(info->ob, mxmlElementGetAttr(node,"ob")) == 0)
			{
				if(info->type == MXML_TWO_DIMENSIONAL_ARR)
				{
					id2=atoi(mxmlElementGetAttr(node,"id2"));		
					if(id2 == info->id2)
						break;
				}
				else
					break;	
			}
		//	printf("id1:%s,id2:%s, ob:%s\n",mxmlElementGetAttr(node,"id1"),
		//			mxmlElementGetAttr(node, "id2"),mxmlElementGetAttr(node,"ob"));
		}
		if(node != NULL)	
		{
			info->sub->top = info->cur;
			info->sub->cur = node;
			ret = mxmlGetSimpleNode(info->sub);
		}
	}
	return ret;
}
/*
type: label type -> refer to eLabelType
ob:    ob attribute of the label
id1:  the first index of the struct-arry or the index of the array 
id2:  the second index of the struct-array
(the following 4 params is only for the struct-array, for other var, they can be filled with 0 or NULL)
subType: the label type of the sub node --> refer to eLabelType
subOb: 	ob attribute of the sub node label
sudId1:   the first index of the struct-arry or the index of the array
subId2:   the second index of the struct-array
*/
char* mxmlGetNode(char type, char *ob, int id1, int id2, char subType, char *subOb, int subId1, int subId2)
{
	char *p=NULL;
	mxmlNode tmp;
	mxmlNode sub;
	memset(&tmp, 0, sizeof(tmp));
	memset(&sub, 0, sizeof(sub));
	
	if(mxmlpTree == NULL || mxmlpMsg == NULL)
	{
		ERR("mxmlGetNode: No MXML text loaded...\n");
		return NULL;
	}

	if(type < MXML_SIMPLE_VAR || type > MXML_TWO_DIMENSIONAL_ARR)
	{
		ERR("mxmlGetNode: Not supported xml node type: %d...\n", type);
		return NULL;
	}
	tmp.top = mxmlpTree;
	tmp.cur = mxmlpMsg;
	tmp.type = type; 
	strncpy(tmp.ob, ob, strlen(ob));
	tmp.id1 = id1;
	tmp.id2 = id2;

	if(type > MXML_SIMPLE_ARR)
	{
		tmp.sub = &sub;	
		sub.cur = NULL;
		sub.top = mxmlpMsg;
		sub.type = subType;
		sub.id1 = subId1;
		sub.id2 = subId2;
		strncpy(sub.ob, subOb, strlen(subOb));
	}
	p = mxmlGetValue(&tmp);
	//printf("result: %s=%s\n",ob,p);
	return p;
}
static char mxmlGetNodeType(mxml_node_t *node)
{
	int i;
	char ret=MXML_INVALID_TYPE;

	if(node == NULL)
		return ret;
	for(i=0; i<MXML_NODE_TYPE_NUM; i++)	
	{
		if(strcmp(node->value.opaque, mxmlLabels[i]) == 0)
			break;
	}
	if(i!=MXML_NODE_TYPE_NUM)
		ret = i+1;

	return ret;
}
	
static mxmlNode gReqNode;
static mxmlNode lSubNode;
mxmlNode *mxmlGetRequest(void)
{
	static mxml_node_t *node=NULL;
	static mxml_node_t *lastNode=NULL;
	static mxml_node_t *subNode=NULL;
	static char flag = 1;
	mxmlNode *ret=NULL;
	char *p = NULL;

	switch(flag)
	{
		case 1://for nodes without subnode
			memset(&gReqNode, 0, sizeof(gReqNode));
			if(node == NULL)
				node=mxmlFindElement(mxmlpMsg,mxmlpTree,NULL,NULL,NULL,MXML_DESCEND);
			else
				node=mxmlFindElement(node,mxmlpMsg,NULL,NULL,NULL,MXML_DESCEND);

			if(node!=NULL)
			{
//				printf("1---------->type:%s, ob:%s\n", node->value.opaque, mxmlElementGetAttr(node,"ob"));
				gReqNode.type = mxmlGetNodeType(node);
				strcpy(gReqNode.ob, mxmlElementGetAttr(node,"ob"));
//				printf("2---------->type: %d, ob: %s\n", gReqNode.type, gReqNode.ob);
				if((p=(char *)mxmlElementGetAttr(node, "id")) != NULL)
					gReqNode.id1 = atoi(p);
				if(gReqNode.type > MXML_SIMPLE_ARR)
				{
//					memset(&lSubNode, 0, sizeof(lSubNode));		
					gReqNode.id1 = atoi(mxmlElementGetAttr(node, "id1"));
					if(gReqNode.type == MXML_TWO_DIMENSIONAL_ARR)
						gReqNode.id2 = atoi(mxmlElementGetAttr(node, "id2"));

//					strcpy(gReqLabel.ob, gReqNode.ob);

					flag = 2;
					mxmlGetRequest();
				}
				ret = &gReqNode;
			}
			break;
		case 2://for nodes with parent
			memset(&lSubNode, 0, sizeof(lSubNode));		
			if(subNode ==  NULL)
				subNode = node;
			if((subNode=mxmlFindElement(subNode,node, NULL, NULL, NULL, MXML_DESCEND))!=NULL)
			{
//				gReqNode.dlab = &gReqLabel;		
				lSubNode.type = mxmlGetNodeType(subNode);
//				gReqNode.sub->type = mxmlGetNodeType(subNode);		
				strcpy(lSubNode.ob, mxmlElementGetAttr(subNode, "ob"));
				gReqNode.sub = &lSubNode;
				lastNode = subNode;
				ret = &gReqNode;
			}
			else
			{
				flag = 1;
				if(lastNode != NULL)
				{
//					memset(&gReqNode, 0, sizeof(gReqNode));
					node = lastNode;
					ret = mxmlGetRequest();
				}
				else
				{
					INFO("No node in D label...\n");//means get all
				}
			}
			break;
		default:
			break;
	}
	
	return ret;
}
//only for char
unsigned int mxmlStr2Array(char *str,char *val)
{
	char *q=NULL;
	int i;
	if(str==NULL || val == NULL)
		return 0;
	for(i=0,q=strtok(str,","); q!=NULL; q=strtok(NULL,","),i++)
	{
		val[i]=atoi(q);	
	}	
	//INFO("Str2Array: len:%d\n", i);
	return i;
}
char* mxmlArray2Str(char *val, int len,  char *buf)
{
	int i;
	char *ret = buf;
	
	if(buf==NULL || val==NULL)
		return NULL;	
	if(len > 1)	
	{
		sprintf(buf, "%d", val[0]);
		for(i=1; i<len; i++)
			sprintf(buf, "%s,%d", buf, val[i]);
	}
//	printf("Array2Str: %s\n", buf);
	return ret;
}
void mxmlParseEnd(void)
{
	if(mxmlpTree!=NULL)
	{
		mxmlDelete(mxmlpTree);
		mxmlpTree=NULL;
		mxmlpMsg=NULL;
	}
}
/************************end of parse***************************/

/************************generate xml string***********************/
static mxml_node_t *mxmlgTree=NULL;
static mxml_node_t *mxmlgMsg=NULL;
static char mxmlbuff[MAX_MXML_TEXT_SIZE]={0};
void mxmlGenerateStart(char *typename, char *op)
{
	if(typename == NULL || op == NULL)
	{
		ERR("mxmlGenerateStart: param typename and op can't be NULL!\n");
		return;
	}
	mxmlgTree = mxmlNewXML("1.0");
	mxmlgMsg = mxmlNewElement(mxmlgTree, "MessageEx");
	mxmlElementSetAttr(mxmlgMsg, "type", typename);
	mxmlElementSetAttr(mxmlgMsg, "op", op);
}

void mxmlAddNodeVar(mxml_node_t *top, char *ob, int value)
{
	char buf[16]={0};//
	mxml_node_t *node=NULL;
	mxml_node_t *ptree=top;

	if(ob == NULL)
	{
		ERR("mxmlAddNodeVar: param ob can't be NULL!\n");
		return;
	}
	if(top == NULL)
		ptree=mxmlgMsg;
	
	if(mxmlFindElement(ptree, ptree, mxmlLabels[0], "ob", ob, MXML_DESCEND)==NULL)
	{
		node=mxmlNewElement(ptree, mxmlLabels[0]);
		mxmlElementSetAttr(node, "ob", ob);	
		sprintf(buf, "%d", value);
		mxmlNewText(node, 0, buf);	
	}
//	mxmlNewInteger(node, value);
}
void mxmlAddNodeStr(mxml_node_t *top, char *ob, char *value)
{
	mxml_node_t *node=NULL;
	mxml_node_t *ptree=top;

	if(ob == NULL)
	{
		ERR("mxmlAddNodeStr: param ob can't be NULL!\n");
		return;
	}
	if(top == NULL)
		ptree=mxmlgMsg;
	if(mxmlFindElement(ptree, ptree, mxmlLabels[1], "ob", ob, MXML_DESCEND)==NULL)
	{
		node=mxmlNewElement(ptree, mxmlLabels[1]);
		mxmlElementSetAttr(node, "ob", ob);	
		mxmlNewText(node, 0, value);	
	}
}
void mxmlAddNodeArr(mxml_node_t *top, char *ob, char *str, int id)
{
	mxml_node_t *node=NULL;
	mxml_node_t *ptree=top;
	mxml_node_t *n1,*n2;
	char buf[4]={0};

	if(ob == NULL|| str == NULL)
	{
		ERR("mxmlAddNodeArr: param ob and str can't be NULL!\n");
		return;
	}
	if(top == NULL)
		ptree=mxmlgMsg;
	if(id != 0)
	{
		sprintf(buf, "%d", id);
		for(n1=mxmlFindElement(ptree,ptree, mxmlLabels[2], "ob", ob, MXML_DESCEND);n1!=NULL;
				n1=mxmlFindElement(n1,ptree, mxmlLabels[2], "ob", ob, MXML_DESCEND))
		{
			for(n2=mxmlFindElement(ptree,ptree, mxmlLabels[2], "id", buf, MXML_DESCEND);n2!=NULL;
					n2=mxmlFindElement(n2,ptree, mxmlLabels[2], "id", buf, MXML_DESCEND))
				if(n1 == n2)
					break;
			if(n2 != NULL)
				break;
		}
	}
	else
		n1 = mxmlFindElement(ptree, ptree, mxmlLabels[2], "ob", ob, MXML_DESCEND);

	if(n1 == NULL)
	{
		node=mxmlNewElement(ptree, mxmlLabels[2]);
		mxmlElementSetAttr(node, "ob", ob);	
		if(id != 0)
			mxmlElementSetAttr(node, "id", buf);
		mxmlNewText(node, 0, str);
	}
}
/*
void mxmlAddNodeArr1(mxml_node_t *top, char *ob, char *value, int len)
{
	char buf[256]={0};
	int i;
	mxml_node_t *node=NULL;
	mxml_node_t *ptree=top;

	if(ob == NULL)
	{
		printf("mxmlAddNodeArr: param ob can't be NULL!\n");
		return;
	}
	if(top == NULL)
		ptree=mxmlgMsg;
	if(mxmlFindElement(ptree, ptree, mxmlLabels[2], "ob", ob, MMXML_DESCEND)==NULL)
	{
		node=mxmlNewElement(ptree, mxmlLabels[2]);
		mxmlElementSetAttr(node, "ob", ob);	
		if(len>0)
		{
			sprintf(buf, "%d", value[0]);
			for(i=1; i<len; i++)	
			{
				sprintf(buf,"%s,%d",buf,value[i]);	
			}
			mxmlNewText(node, 0, buf);	
		}
	}
}
*/
mxml_node_t* mxmlAddArr1(mxml_node_t *top, char *ob, int id1)
{
	char buf[16]={0};//
	mxml_node_t *node=NULL;
	mxml_node_t *n1=NULL;
	mxml_node_t *n2=NULL;
	mxml_node_t *ptree=top;

	if(ob == NULL)
	{
		ERR("mxmlAddArr1: param ob can't be NULL!\n");
		return NULL;
	}
	if(top == NULL)
		ptree=mxmlgMsg;

	sprintf(buf, "%d", id1);
	for(n1 = mxmlFindElement(ptree, ptree, mxmlLabels[3], "ob", ob, MXML_DESCEND); n1!=NULL;
			n1 = mxmlFindElement(n1,ptree, mxmlLabels[3],"ob", ob, MXML_DESCEND))
	{
		for(n2 = mxmlFindElement(ptree, ptree, mxmlLabels[3], "id1", buf, MXML_DESCEND);n2!=NULL;
				n2 = mxmlFindElement(n2, ptree, mxmlLabels[3], "id1", buf, MXML_DESCEND))
			if(n1 == n2)
				break;
		if(n2!=NULL)
			break;
	}
	if(n1!=NULL)
		node = n1;
	else
	{
		node=mxmlNewElement(ptree, mxmlLabels[3]);
		mxmlElementSetAttr(node, "ob", ob);	
		sprintf(buf, "%d", id1);
		mxmlElementSetAttr(node, "id1", buf);	
	}
	return node;
}
mxml_node_t* mxmlAddArr2(mxml_node_t *top, char *ob, int id1, int id2)
{
	char buf1[16]={0};//
	char buf2[16]={0};
	mxml_node_t *node=NULL;
	mxml_node_t *n1=NULL;
	mxml_node_t *n2=NULL;
	mxml_node_t *n3=NULL;
	mxml_node_t *ptree=top;

	if(ob == NULL)
	{
		ERR("mxmlAddArr2: param ob can't be NULL!\n");
		return NULL;
	}
	if(top == NULL)
		ptree=mxmlgMsg;

	sprintf(buf1, "%d", id1);
	sprintf(buf2, "%d", id2);

	for(n1 = mxmlFindElement(ptree, ptree, mxmlLabels[4], "id1", buf1, MXML_DESCEND);n1!=NULL;
			n1 = mxmlFindElement(n1, ptree, mxmlLabels[4], "id1", buf1, MXML_DESCEND))
	{
		for(n2 = mxmlFindElement(ptree, ptree, mxmlLabels[4], "id2", buf2, MXML_DESCEND);n2!=NULL;
				n2 = mxmlFindElement(n2, ptree, mxmlLabels[4], "id2", buf2, MXML_DESCEND))
		{
			for(n3 = mxmlFindElement(ptree, ptree, mxmlLabels[4], "ob", ob, MXML_DESCEND);n3!=NULL;
					n3 = mxmlFindElement(n3, ptree, mxmlLabels[4], "ob", ob, MXML_DESCEND))
					if(n1 == n2 && n2 == n3)
						break;
			if(n2 != NULL)
				break;
		}
		if(n1 != NULL)
			break;
	}

	if(n1!=NULL)
		node = n1;
	else
	{
		node=mxmlNewElement(ptree, mxmlLabels[4]);
		mxmlElementSetAttr(node, "ob", ob);	
		mxmlElementSetAttr(node, "id1", buf1);	
		mxmlElementSetAttr(node, "id2", buf2);	
	}
	return node;
}
char *mxmlGenerateFinished(void)
{
	memset(mxmlbuff, 0, sizeof(mxmlbuff));
	mxmlSaveString(mxmlgTree, mxmlbuff, sizeof(mxmlbuff), MXML_NO_CALLBACK);
	return mxmlbuff;
}
/************************end of generate***************************/
