/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : binfile.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��8��3��
  ����޸�   :
  ��������   : ��Դ���װ�˶Զ����������ļ���д�ĳ��ýӿڣ�����ֻ��Ҫʹ��Re-
               adBinCfgInfo��WriteBinCfgInfo����ʵ�ֶԶ�����������Ϣ�Ķ�д��
               ��Դ�ļ�һ������ͨ������������Ҫ�ٴθ��ģ�������Ҫ�ṩ�����д
               ����ʱ��
  �����б�   :
              CompressBackupCfg
              GetCompressPackageName
              IsFileEmpty
              ReadBackupBinCfgInfo
              ReadBinCfgInfo
              UnCompressBackupCfg
              WriteBinCfgInfo
  �޸���ʷ   :
  1.��    ��   : 2015��8��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "binfile.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define GET_RELATIVE_PATH(path) strrchr((path),'/')+1

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

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*****************************************************************************
 �� �� ��  : IsFileEmpty
 ��������  : �ж��ļ���С�Ƿ�Ϊ�գ����Ϊ���򷵻�0�����򷵻�ʵ�ʴ�С .0:
             empty, 1: not empty
 �������  : char *pFileName  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static UINT32 IsFileEmpty(char *pFileName)
{
	struct stat buf;

	if(pFileName == NULL)
	{
		return 0;	
	}
	
	if(0 == stat(pFileName,&buf))
	{
		return buf.st_size; 	
	}
	else
	{
//		ERR("test file %s size failed : %s.\n",pFileName,strerror(errno));
		return 0;
	}
}

/*****************************************************************************
 �� �� ��  : GetCompressPackageName
 ��������  : ��ñ���ѹ�����ľ���·����
 �������  : char *pFileName     
             char *pPackageName  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void GetCompressPackageName(char *pFileName,char *pPackageName)
{
	char *p = NULL;
	UINT8 nOffset = 0;
	strcpy(pPackageName,pFileName);
	if((p = strrchr(pPackageName,'.')) == NULL)
	{
		return;
	}
	nOffset = p - pPackageName;
	sprintf(pPackageName+nOffset,".tar.gz");
}

/*****************************************************************************
 �� �� ��  : UnCompressBackupCfg
 ��������  : ��ѹ�������ļ�
 �������  : char *pFileName  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void UnCompressBackupCfg(char *pFileName)
{
	char cCmd[512] = {0};
	char pPackageName[256] = {0};
	
	GetCompressPackageName(pFileName,pPackageName);	
	
	if(IsFileEmpty(pPackageName) != 0)
	{
		sprintf(cCmd,"cd /home/ && tar zxvf %s >> /dev/null && cd - >> /dev/null",GET_RELATIVE_PATH(pPackageName));
		//uncompress tar.gz package
		system(cCmd);
	}
}

/*****************************************************************************
 �� �� ��  : CompressBackupCfg
 ��������  : //ѹ�������ļ� , ��pFileName���ļ����.tar.gz�ı��ݣ�����cIsCh-
             eck�Ƿ����1�������Ƿ���б��ݣ������0���������ݣ������1����
             ��
             //�жϱ����ļ������ڻ򱸷��ļ������ʱ�Ž��б���
 �������  : char *pFileName  
             UINT8 cIsCheck   
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void CompressBackupCfg(char *pFileName,UINT8 cIsCheck)
{
	char cCmd[512] = {0};
	char pPackageName[256] = {0};
	
	GetCompressPackageName(pFileName,pPackageName);	

	if(cIsCheck == 1)
    {
        if(IsFileEmpty(pPackageName) != 0)
        {
            return;
        }
    }
    
	sprintf(cCmd,"cd /home/ && tar zcvf %s %s >> /dev/null && cd - >> /dev/null",GET_RELATIVE_PATH(pPackageName),GET_RELATIVE_PATH(pFileName));
	system(cCmd);
}

/*****************************************************************************
 �� �� ��  : ReadBackupBinCfgInfo
 ��������  : �ӱ����ļ��ж�ȡ������Ϣ����������Ĭ�ϵ������ļ���ѹ����.tar.gz
             ����ļ�
 �������  : char *pFileName  
             void *pBuf       
             UINT32 nBufSize  
             UINT32 nOffset   
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
UINT32 ReadBackupBinCfgInfo(char *pFileName, void *pBuf, UINT32 nBufSize, UINT32 nOffset)
{
	if( pFileName == NULL || pBuf == NULL )
	{
		return 0;	
	}
	UnCompressBackupCfg(pFileName);
	//read file
	return ReadBinCfgInfo(pFileName,pBuf,nBufSize,nOffset,0);
}

/*****************************************************************************
 �� �� ��  : ReadBinCfgInfo
 ��������  : ��������������Ϣ���ȿ��Զ�ȡ�����ṹ�壬Ҳ���Զ�ȡĳһ���ȫ
             ����size��offset�����������øýӿڽ����ݱ����ļ��Ƿ�����������Ƿ���Ҫ���б���
 �������  : char *pFileName      
             void *pBuf           
             UINT32 nBufSize      
             UINT32 nOffset       
             UINT8 nIsReadBackup  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��31��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
UINT32 ReadBinCfgInfo(char *pFileName, void *pBuf, UINT32 nBufSize, UINT32 nOffset, UINT8 nIsReadBackup)
{
	int fd = 0;
	int ret = 0;
	
	if( pFileName == NULL || pBuf == NULL )
	{
		return 0;	
	}
	
	if((IsFileEmpty(pFileName) == 0) && (nIsReadBackup == 1 ))
	{
		return ReadBackupBinCfgInfo(pFileName,pBuf,nBufSize,nOffset);	
	}

	fd = open(pFileName,O_RDONLY);
	if(-1 == fd)
	{
		ERR("open %s failed : %s .\n",pFileName,strerror(errno));
		return 0;
	}
	
	if(-1 == lseek(fd,nOffset,SEEK_SET))
	{
		ERR("lseek %s offset %d failed : %s .\n",pFileName,nOffset,strerror(errno));
		close(fd);
		return 0;
	}

	ret = read(fd,pBuf,nBufSize);
	if(ret < 0)
	{
		ERR("read %s failed : %s .\n",pFileName,strerror(errno));
		close(fd);
		return 0;
	}

	close(fd);
	CompressBackupCfg(pFileName,1);
	return ret; 
	
}

/*****************************************************************************
 �� �� ��  : WriteBinCfgInfo
 ��������  : д�����������ļ�,ֻ�ṩд�������ṹ�壬���ṩ����д��ṹ���е�
             ĳһ��,���øýӿں󣬽�ͬ���������ñ���
 �������  : char *pFileName  
             void *pBuf       
             UINT32 nBufSize  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��31��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
UINT32 WriteBinCfgInfo(char *pFileName, void *pBuf, UINT32 nBufSize)
{
	int fd = 0;
	int ret = 0;
	
	if( pFileName == NULL || pBuf == NULL )
	{
		return 0;	
	}
	
	fd = open(pFileName,O_RDWR|O_TRUNC|O_CREAT);
	if(-1 == fd)
	{
		ERR("open %s failed : %s .\n",pFileName,strerror(errno));
		return 0;
	}
	ret = write(fd,pBuf,nBufSize);
	if(ret < 0)
	{
		ERR("write %s failed : %s .\n",pFileName,strerror(errno));
		close(fd);
		return 0;
	}

	close(fd);
	CompressBackupCfg(pFileName,0);
	return ret; 
}

