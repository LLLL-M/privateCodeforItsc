#ifndef _MXMLRESOLVE_H_
#define __MXMLRESOLVE_H
#include "mxml.h"

#define MAX_MXML_TEXT_SIZE	1024*15	//max buffer size for xml text
#define MXML_NODE_TYPE_NUM	5


typedef enum
{
	MXML_SUCCESS=0,
	MXML_FAILURE=1
}eXmlRetValue;
typedef enum
{
	MXML_INVALID_TYPE=0,
	MXML_SIMPLE_VAR=1,		//var of char, short, integer
	MXML_SIMPLE_STR=2,		//string
	MXML_SIMPLE_ARR=3,		//array
	MXML_ONE_DIMENSIONAL_ARR=4,	//one-dimensional array of a struct
	MXML_TWO_DIMENSIONAL_ARR=5	//two-dimensional array of a struct 
}eLabelType;

typedef struct mxml_node
{
	mxml_node_t *top;	//top node
	mxml_node_t *cur;	//current node 
	int id1;			//id1 of struct-array or id of array
	int id2;			//id2 of struct-array
	char ob[32];		//object
	char type;			//label type
	struct mxml_node *sub;	//sub-node, NULL if no
}mxmlNode;

void mxmlGenerateStart(char *typename, char *op);
char *mxmlGenerateFinished(void);
void mxmlAddNodeVar(mxml_node_t *top, char *ob, int value);
void mxmlAddNodeStr(mxml_node_t *top, char *ob, char *value, int id);
void mxmlAddNodeArr(mxml_node_t *top, char *ob, char *str, int id);
//void mxmlAddNodeArr1(mxml_node_t *top, char *ob, char *value, int len);
mxml_node_t* mxmlAddArr1(mxml_node_t *top, char *ob, int id1);
mxml_node_t* mxmlAddArr2(mxml_node_t *top, char *ob, int id1, int id2);


char mxmlParseStart(char *str);
void mxmlParseEnd(void);
char *mxmlGetMsgOp(void);
char *mxmlGetMsgType(void);
char* mxmlGetNode(char type, char *ob, int id1, int id2, char subType, char *subOb, int subId1, int subId2);
mxmlNode *mxmlGetRequest(void);

//char* getArray(char *str,char *val);
unsigned int  mxmlStr2Array(char *str,char *val);
char* mxmlArray2Str(char *val, int len,  char *str);
#endif
