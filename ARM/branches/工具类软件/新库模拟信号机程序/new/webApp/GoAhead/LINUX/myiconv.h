#ifndef __MYICONV_H__
#define	__MYICONV_H__

/********************************************************************
    created:   2007/11/15
    created:   15:11:2007   10:30
    filename: wxb_codeConv.c
    author:       Wang xuebin
    depend:       iconv.lib
    build:     不需要build，被包含到其它源代码中
    purpose:   提供从UTF-8到GB2312的内码转换，以及反向的转换
*********************************************************************/
#include <stdio.h>
#include <iconv.h>
#include <string.h>

#define OUTLEN  1024

/* 代码转换:从一种编码转为另一种编码 */
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
/* UNICODE码转为GB2312码
   成功则返回一个动态分配的char*变量，需要在使用完毕后手动free，失败返回NULL */
static __inline char* u2g(char *inbuf, char *szOut)
{
    return (-1 == code_convert("UTF-8","GBK",inbuf,strlen(inbuf),szOut,OUTLEN)) ? NULL : szOut;
}
/* GB2312码转为UNICODE码
   成功则返回一个动态分配的char*变量，需要在使用完毕后手动free，失败返回NULL */
static __inline char* g2u(char *inbuf, char *szOut)
{
    return (-1 == code_convert("GBK","UTF-8",inbuf,strlen(inbuf),szOut,OUTLEN)) ? NULL : szOut;
}

#endif
