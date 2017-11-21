/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : HikConfig.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��2��
  ����޸�   :
  ��������   : ���ļ���ɶ������ļ����ж�ȡ������У��Ľӿ�
  �����б�   :
              ArrayToInt
              checkPhaseTrunInRing
              DateTimeCmp
              find_next_index
              GetCircleTime
              IsArrayRepeat
              IsBarrierGreenSignalRationEqual
              IsItemInCharArray
              IsItemInShortArray
              IsPhaseContinuousInPhaseTurn
              isPhaseInArray
              IsSignalControllerParaIdLegal
              IsSignalControlparaLegal
              LoadDataFromCfg
              log_error
  �޸���ʷ   :
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

  2.��    ��   : 2014��12��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����µĽṹ�壬�޸���ؽӿ�
******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "gbconfig.h"
#include <stdarg.h>
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define COL_DEF "\033[m"
#define COL_RED "\033[0;32;31m"
#define COL_GRE "\033[0;32;32m"
#define COL_BLU "\033[0;32;34m"
#define COL_YEL "\033[1;33m"

//�Ƿ������Ĵ����ӡ
#define LOG_ERR_CHN


#define DEG(fmt,...) fprintf(stdout,"gbConfig library debug : "fmt "\n",##__VA_ARGS__)



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
char *gErrorContentGb = NULL;//��ȫ�ֱ����洢������Ϣ���������߽��з���


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : log_error
 ��������  : ��ӡ������Ϣ�ӿڣ����д������ݻ� ���浽ȫ�ֻ��������ⲿ�ӿڿ�ֱ
             �Ӳ鿴�����ݡ�
 �������  : const char* format  
             ...                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���
  2.��    ��   : 2014��12��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �޸Ļ�������СΪ1024�ֽ�
  3.��    ��   : 2014��12��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����strcat�����ַ�����Ϊһ��sprintf���������ַ���	
*****************************************************************************/
static void log_error(const char* format, ...)
{
#ifndef LOG_ERR_CHN    
	char buff[1024] = {0};
    if(NULL == gErrorContentGb)
    {
        gErrorContentGb = calloc(1024,1);

    }
	sprintf(buff,"%s HikConfig library error: %s <ERROR>  ",COL_BLU,COL_YEL);
	va_list argptr;
	va_start(argptr, format);

    if(NULL != gErrorContentGb)
    {
    	memset(gErrorContentGb,0,1024);
    	vsnprintf(gErrorContentGb,1024,format,argptr);
    }

	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//��buff��λ�������ƶ�strlen(buff)�ֽڣ���ǰ����24�ֽ�������ŵ�ǰʱ�䣬��������2�ֽڵĿո������ָ����ݺ�ʱ��
	va_end(argptr);
	strcat(buff,COL_DEF);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);
#endif
}

/*****************************************************************************
 �� �� ��  : log_error_cn
 ��������  : ��ӡ������Ϣ�ӿڣ����д������ݻ� ���浽ȫ�ֻ��������ⲿ�ӿڿ�ֱ
             �Ӳ鿴�����ݡ�
 �������  : const char* format  
             ...                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���
  2.��    ��   : 2014��12��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �޸Ļ�������СΪ1024�ֽ�
  3.��    ��   : 2014��12��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����strcat�����ַ�����Ϊһ��sprintf���������ַ���	
*****************************************************************************/
static void log_error_cn(const char* format, ...)
{
#ifdef LOG_ERR_CHN
	char buff[1024] = {0};
    if(NULL == gErrorContentGb)
    {
        gErrorContentGb = calloc(1024,1);

    }
	sprintf(buff,"%s HikConfig library error: %s <ERROR>  ",COL_BLU,COL_YEL);
	va_list argptr;
	va_start(argptr, format);

    if(NULL != gErrorContentGb)
    {
    	memset(gErrorContentGb,0,1024);
    	vsnprintf(gErrorContentGb,1024,format,argptr);
    }

	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//��buff��λ�������ƶ�strlen(buff)�ֽڣ���ǰ����24�ֽ�������ŵ�ǰʱ�䣬��������2�ֽڵĿո������ָ����ݺ�ʱ��
	va_end(argptr);
	strcat(buff,COL_DEF);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);
#endif
}


