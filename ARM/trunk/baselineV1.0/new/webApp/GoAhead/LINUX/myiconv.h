#ifndef __MYICONV_H__
#define	__MYICONV_H__

/********************************************************************
    created:   2007/11/15
    created:   15:11:2007   10:30
    filename: wxb_codeConv.c
    author:       Wang xuebin
    depend:       iconv.lib
    build:     ����Ҫbuild��������������Դ������
    purpose:   �ṩ��UTF-8��GB2312������ת�����Լ������ת��
*********************************************************************/
#include <stdio.h>
#include <iconv.h>
#include <string.h>

#define OUTLEN  1024

/* ����ת��:��һ�ֱ���תΪ��һ�ֱ��� */
static int code_convert(char* from_charset, char* to_charset, char* inbuf,
                        int inlen, char* outbuf, int outlen)
{
    int ret = 0;
    iconv_t cd;
    char** pin = &inbuf;
    char** pout = &outbuf;
    cd = iconv_open(to_charset,from_charset);
    if(cd == (iconv_t)-1)
	{
		perror("iconv_open fail");
        return -1;
	}
    memset(outbuf,0,outlen);
    if(iconv(cd,(char**)pin,(size_t *)&inlen,pout,(size_t *)&outlen) == -1)
	{
		perror("iconv convert fail");
        ret = -1;
	}
    iconv_close(cd);
    return ret;
}
/* UNICODE��תΪGB2312��
   �ɹ��򷵻�һ����̬�����char*��������Ҫ��ʹ����Ϻ��ֶ�free��ʧ�ܷ���NULL */
static __inline char* u2g(char *inbuf, char *szOut)
{
    return (-1 == code_convert("UTF-8","GBK",inbuf,strlen(inbuf),szOut,OUTLEN)) ? NULL : szOut;
}
/* GB2312��תΪUNICODE��
   �ɹ��򷵻�һ����̬�����char*��������Ҫ��ʹ����Ϻ��ֶ�free��ʧ�ܷ���NULL */
static __inline char* g2u(char *inbuf, char *szOut)
{
    return (-1 == code_convert("GBK","UTF-8",inbuf,strlen(inbuf),szOut,OUTLEN)) ? NULL : szOut;
}

#endif
