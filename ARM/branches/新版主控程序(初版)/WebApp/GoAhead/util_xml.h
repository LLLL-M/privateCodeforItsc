/*onvif_struct.h
********************************************************************************
<PRE>
ģ����       : onvif
�ļ���       : onvif_struct.h
����ļ�     :
�ļ�ʵ�ֹ��� : 
����         : menghong
�汾         : 
--------------------------------------------------------------------------------
��ע         : N/A
--------------------------------------------------------------------------------
�޸ļ�¼ : 
�� ��        �汾     �޸���              �޸�����
YYYY/MM/DD   X.Y      <���߻��޸�����>    <�޸�����>
</PRE>
*******************************************************************************/
#ifndef _HIK_UTIL_XML_H_
#define _HIK_UTIL_XML_H_


#ifdef __cplusplus
extern "C" {
#endif

#define TAG_MARKER 0x54414754
#define XMLVERSION      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"

/* each element in the inbound xml data is assigned a tag_t_xml structure */
typedef struct tag_t_xml
{
    unsigned long   marker;
    unsigned long   count;
    int     empty;      // empty flag

    char        *name;
    char        *value;
    char        *attr;

    char        *start_name;
    char        *value_name;
    char        *end_name;
    char        *attr_name;

    int     start_len;
    int     value_len;
    int     end_len;
    int     attr_len;

    struct  tag_t_xml   *first_child;
    struct  tag_t_xml   *next;
    struct  tag_t_xml   *parent;
}UTIL_XML_TAG, *pUTIL_XML_TAG;

typedef struct xml_req_tag
{
    /* request fields */    
    char  *query_str;   /*�ͻ��˴��͵�xml����*/
    pUTIL_XML_TAG prolog_tag;
    pUTIL_XML_TAG root_tag;
    int              tag_count;
    
    /* response */
    char        *buf;                /*���ػ�����*/
    int           buflen;             /*���ػ���������*/
    int           currlen;            /*��ǰ���ػ����������ݳ���*/

    #define ERR_STR_SIZE    256
    char        error_string[ERR_STR_SIZE];
}UTIL_XML_REQ, *pUTIL_XML_REQ;

void util_xml_cleanup(pUTIL_XML_REQ req);
int util_xml_init(pUTIL_XML_REQ req);
int util_xml_validate(pUTIL_XML_REQ req, char *xstr, int len);
int util_xml_matchtag(const char *std_tag, const char *recive_tag);
void util_xadd_stag_attr(pUTIL_XML_REQ dstbuf, const char *tag, const char *fmt, ...);
void util_xadd_stag(pUTIL_XML_REQ dstbuf, const char *tag);
void util_xadd_etag(pUTIL_XML_REQ dstbuf, const char *tag);
void util_xadd_elem(pUTIL_XML_REQ dstbuf, const char *tag, const char *val);
void util_xadd_elem_ex(pUTIL_XML_REQ dstbuf, const char *tag, const char *val);
void util_xml_append(pUTIL_XML_REQ dstbuf, const char *fmt, ...);
void util_xml_append_ex(pUTIL_XML_REQ dstbuf, const char *fmt, ...);
void util_xadd_int_elem(pUTIL_XML_REQ pstruDstBuf,const char *szTag,int iVal);
void util_xadd_float_elem(pUTIL_XML_REQ pstruDstBuf,const char *szTag,float fVal);
void util_xadd_elem_attr(pUTIL_XML_REQ dstbuf, const char *tag, const char *val, const char *fmt, ...);

#ifdef __cplusplus
}
#endif


#endif

