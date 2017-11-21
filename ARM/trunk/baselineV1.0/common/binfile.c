/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : binfile.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年8月3日
  最近修改   :
  功能描述   : 该源码封装了对二进制配置文件读写的常用接口，对外只需要使用Re-
               adBinCfgInfo及WriteBinCfgInfo即可实现对二进制配置信息的读写。
               该源文件一旦测试通过，基本不需要再次更改，除非需要提供特殊读写
               需求时。
  函数列表   :
              CompressBackupCfg
              GetCompressPackageName
              IsFileEmpty
              ReadBackupBinCfgInfo
              ReadBinCfgInfo
              UnCompressBackupCfg
              WriteBinCfgInfo
  修改历史   :
  1.日    期   : 2015年8月3日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "binfile.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define GET_RELATIVE_PATH(path) strrchr((path),'/')+1

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

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


/*****************************************************************************
 函 数 名  : IsFileEmpty
 功能描述  : 判断文件大小是否为空，如果为空则返回0，否则返回实际大小 .0:
             empty, 1: not empty
 输入参数  : char *pFileName  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月3日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : GetCompressPackageName
 功能描述  : 获得备份压缩包的绝对路径名
 输入参数  : char *pFileName     
             char *pPackageName  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月3日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : UnCompressBackupCfg
 功能描述  : 解压缩备份文件
 输入参数  : char *pFileName  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月3日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : CompressBackupCfg
 功能描述  : //压缩备份文件 , 将pFileName的文件添加.tar.gz的备份，根据cIsCh-
             eck是否等于1来决定是否进行备份，如果是0则立即备份，如果是1，则
             先
             //判断备份文件不存在或备份文件被清空时才进行备份
 输入参数  : char *pFileName  
             UINT8 cIsCheck   
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月3日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : ReadBackupBinCfgInfo
 功能描述  : 从备份文件中读取配置信息，我们现在默认的配置文件是压缩成.tar.gz
             后的文件
 输入参数  : char *pFileName  
             void *pBuf       
             UINT32 nBufSize  
             UINT32 nOffset   
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月3日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : ReadBinCfgInfo
 功能描述  : 读二进制配置信息，既可以读取整个结构体，也可以读取某一项，完全
             根据size和offset来决定，调用该接口将根据备份文件是否存在来决定是否需要进行备份
 输入参数  : char *pFileName      
             void *pBuf           
             UINT32 nBufSize      
             UINT32 nOffset       
             UINT8 nIsReadBackup  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月31日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : WriteBinCfgInfo
 功能描述  : 写二进制配置文件,只提供写入整个结构体，不提供单独写入结构体中的
             某一项,调用该接口后，将同步进行配置备份
 输入参数  : char *pFileName  
             void *pBuf       
             UINT32 nBufSize  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月31日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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

