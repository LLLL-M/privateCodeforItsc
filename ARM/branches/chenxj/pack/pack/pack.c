/********************************************/


/*********************************************/

#include "pack.h"

#define PACK_ARGV (6)	//命令行个数
#define DEVICE_CLASS 123 //暂时先定义为123
#define OEM_CODE 0xffffffff

//------------version----------------
#define MAJOR_SHIFT 16
#define MINOR_SHIFT	8
#define SOFT_MAJOR_VERSION 1
#define SOFT_MINOR_VERSION 0
#define SOFT_BUILD_VERSION 0
//-----------------------------------








//-----------------------------
//功能：按字节计算校验和
//输入：pdata--校验缓冲区的指针
//		len----缓冲区的长度
//返回值：32位的校验和
//-----------------------------

unsigned int check_byte_sum(char *pdata, int len) //pdata需要校验的数据缓冲区的指针，len需检验的长度，以字节为单位。
{
	unsigned int sum = 0;
	int i;

	for(i=0; i < len; i++)
	{
		sum += ((unsigned int)pdata[i]) & 0xff;			//越界！！！？？？

	}

	return sum;
}


//-----------------------------------------------------------
//功能：利用简单的异或进行数据变换，用于升级文件的打包和解包
//输入：src--源文件
//		dst--目的文件
//		len--文件大小
//返回值：0--正常
//		 -1--异常
//-----------------------------------------------------------

int convertData(char *src, char *dst, int len)
{
	/* 固定的幻数，用于异或变换 */
	char magic[] = {0xba, 0xcd, 0xbc, 0xfe, 0xd6, 0xca, 0xdd, 0xd3,
					0xba, 0xb9, 0xa3, 0xab, 0xbf, 0xcb, 0xb5, 0xbe};  
	int i, j;
	int magiclen, startmagic;

	if(src==NULL || dst==NULL)
	{	
		/*对于%p一般以十六进制整数方式输出指针的值，附加前缀0x*/
		printf("Invalid input param: src = 0x%p, dst = 0x%p\n", src, dst);  
		return -1;
	}

	magiclen = sizeof(magic); 

	for(i=0, startmagic = 0; i < len; startmagic = (startmagic+1)%magiclen) 
	{
		/* 用startmagic控制每次内循环magic的起始位置 */
		for(j=0; j<magiclen && i<len; j++, i++)
		{
			/* 进行异或变换 */
			*dst++ = *src++ ^ magic[(startmagic+j)%magiclen];
		}
	}

	return 0;
}

//-----------------------------------------------------------
//功能：获得软件版本信息
//输入：如果输出不为NULL，打印版本信息，并存入buf
//返回值：软件版本信息
//-----------------------------------------------------------

unsigned int  mkSoftVersion(char *buf)
{
	if(buf)
	{
		sprintf(buf, "V%d.%d.%d", SOFT_MAJOR_VERSION, SOFT_MINOR_VERSION, SOFT_BUILD_VERSION);
	}
	
	return ( ((SOFT_MAJOR_VERSION) << (MAJOR_SHIFT)) | ((SOFT_MINOR_VERSION) << (MINOR_SHIFT)) | SOFT_BUILD_VERSION);
}


//-----------------------------------------------------------
//功能：检测命令行输入信息，并生成update_flag信息
//输入：命令行输入信息结构体
//返回值：0--正常
//		 -1--异常
//-----------------------------------------------------------

int info_conv(char *update_flag, IFNO_ARGV *info)
{
	char update_flag_conv[19] = {0};
		
	/*平台检测*/
	printf("platform  = %s\n", info->platform );
	if(strcmp(info->platform , "davinci") == 0)
	{		
		memcpy(&update_flag_conv[0], "111", 3);
	}
	else	
	{
		printf("platform error.\n");
		return -1;
	}
	
	/*flash检测*/	
	printf("flash_size = %s\n", info->flash_size);
	if(strcmp(info->flash_size, "4M") == 0)
	{
		memcpy(&update_flag_conv[3], "001", 3); 
	}
	else	
	{
		printf("flashSize is not match.\n");
		return -1;
	}
	
	/*memory检测*/
	printf("mem_size = %s\n", info->mem_size);
	if(strcmp(info->mem_size, "64M") == 0)
	{
		memcpy(&update_flag_conv[6], "001", 3);
	}
	else	
	{
		printf("mem_size is not match.\n");
		return -1;
	}
	
	//目前暂定这两个为系统默认、
	/*majortype检测*/
	printf("majorType = %s\n", info->major_type);
	if(strcmp(info->major_type, "IPC") == 0)
	{
		memcpy(&update_flag_conv[9], "111", 3);
	}
	else	
	{
		printf("majorType is not match.\n");
		return -1;
	}

	/*minor_type*/
	printf("minorType = %s\n", info->minor_type);
	memcpy(&update_flag_conv[12], info->minor_type, 3);    
	
	/*语言检测*/
	printf("language = %s\n", info->lang);
	if(strcmp(info->lang , "CN") == 0)
	{
		info->language = 2;
		memcpy(&update_flag_conv[15], "002", 3);
	}
	else if (strcmp(info->lang , "EN") == 0)
	{
		info->language = 1;
		memcpy(&update_flag_conv[15], "001", 3);
	}
	else
	{
		printf("language error, not CN or EN.\n");
		return -1;
	}
	
	/*构造升级信息*/
//	memcpy(fheader->res,update_flag,18);
	printf("update flag=%s\n",update_flag_conv);

//	update_flag = update_flag_conv;
	strcpy(update_flag, update_flag_conv);
	puts("ceshi");

	return 0;
}



int create_dig(int argc, char *argv[], FIRMWARE_HEADER *fheader, UPGRADE_FILE_HEADER *uheader, IFNO_ARGV *info)
{
	int err;
	int fdout,fdin;
	int fdin_len;
	unsigned int check_sum;
	int i,j;
	int offset = 0;

	/*最终文件名，digicap.dav*/
	char file_name[30] = {0};

//	memset(file_name, 0, sizeof(file_name));
	strcpy(file_name, UPFILE_NAME);

	/*这里O_TRUNC，每次进行升级打包，都会将上次的截断*/	
	fdout = open(file_name, O_RDWR|O_CREAT|O_TRUNC, 0777);
	if(fdout < 0) 																			     
	{
		printf("create file %s error\n", file_name);
		return -1;
	}

	
	/*存入文件内容,同时填入升级文件的头结构信息*/
	for(i = PACK_ARGV+1, j = 0; i < argc; i++,j++)
	{
		char f_name[50];
		char *tmp;

		/*这里注意，文件的存放地址为当前文件夹中，统一下*/
		sprintf(f_name, "%s", argv[i]);		


		if((fdin = open(f_name, O_RDWR))== -1)
		{
			printf("open file %s error\n", argv[i]);
			break;
		}

		/*获取文件大小*/
		struct stat filestat;
		fstat(fdin, &filestat);
		fdin_len = filestat.st_size;
		printf("fdin_len%d\n",fdin_len);

		//经过测试，可行
//		f_len = lseek(fdin, 0, SEEK_END);


		/*读取文件*/
		tmp = (char*)calloc(1, fdin_len);
		lseek(fdin, 0, SEEK_SET);
		read(fdin, tmp, fdin_len);

		/*写入fdout*/
		//其实这里不需要定位了，因为每次都是从文件尾加入
	//	lseek(fdout, header_len+offset, SEEK_SET); 		测试！！！
		write(fdout, tmp, fdin_len);
		

		/*对升级文件进行校验和*/
		check_sum = check_byte_sum(tmp, fdin_len);		

		/*将升级文件的信息放入升级文件头*/
		//注意这里结构体的多个用[I],在空间地址上；？？？？
		uheader[j].startOffset = info->header_len + offset;//注意这里是uheader_len！	
		uheader[j].fileLen = fdin_len;
		uheader[j].checkSum = check_sum;
		strcpy(uheader[j].fileName, argv[i]);//注意这里是升级文件名，不是

		/*文件偏移量*/
		offset += fdin_len;

		close(fdin);
		free(tmp);

		/*print info for test*/
		printf("Packing file %s:\n", argv[i]);
		printf("startOffset=0x%x\n", uheader[j].startOffset);
		printf("fileLen=%d\n", uheader[j].fileLen);
		printf("checkSum=0x%x\n", uheader[j].checkSum);
		printf("...ok.\n\n"); 

	}



	/*填写固件头信息*/
	fheader->magic_number = CFG_MAGIC;
	fheader->header_length = info->header_len; //注意看下这里是否是完整长度
	fheader->file_nums = argc-PACK_ARGV-1;
	fheader->language = info->language;
	fheader->device_class = DEVICE_CLASS;
	fheader->oemCode = OEM_CODE;
	fheader->softwareVersion = mkSoftVersion(NULL);
//	fheader->res = update_flag;  //注意下，这里与前面info——conv里面功能重合
	//memcpy(fheader->res,update_flag,18);   注意下/测试
	memset(fheader->res_feature, 1, 12);
	/*
	for(i=0; i < 12, i++)
	{
		fheader->res_feature[i] = 0xff; 
	}
	*/
	/*获取固件头校验和,这里去除前3个字节来校验和*/
	fheader->header_check_sum = check_byte_sum((char *)fheader+12, info->fheader_len-12);

	/* print info for test */
		printf("magic_number:0x%x\n", fheader->magic_number);
		printf("header_length:%d\n", fheader->header_length);
		printf("file_nums:%d\n", fheader->file_nums);
		printf("language:%d\n", fheader->language);
		printf("device_class:%d\n", fheader->device_class);
		printf("oemCode:0x%x\n", fheader->oemCode);
		printf("header_checksum:0x%x\n", fheader->header_check_sum);



	/*将固件头、升级文件头放入digicap.dav*/
	lseek(fdout, 0, SEEK_SET);
	write(fdout, fheader, info->header_len);

//	write(fdout, uheader, uheader_len);
	
	//注意这里还需要进行加密头文件操作

	close(fdout);
	return 0;
}








int main(int argc, char *argv[])
{
	
	unsigned int device_class;//程序还没有写接收其的部分
	unsigned int fheader_len, uheader_len, header_len;

	char update_flag[19] = {0};


	UPGRADE_FILE_HEADER *uheader;
	FIRMWARE_HEADER *fheader;
	IFNO_ARGV *info;

	/*命令行检验，并取出命令行信息*/
	if (argc < PACK_ARGV+2)
	{
		printf("usage: ./pack platform flash_size mem_size major_type minor_type language filename...\n");
		exit(0);
	}


	/*分配内存*/
	fheader_len = sizeof(FIRMWARE_HEADER);
	uheader_len = (argc - 1-PACK_ARGV) * sizeof(UPGRADE_FILE_HEADER);
	header_len = fheader_len + uheader_len;

	fheader = (FIRMWARE_HEADER *)calloc(1, header_len);
//	memset((char *)fheader, 0, header_len);		注意下这里初始化
//	uheader = (UPGRADE_FILE_HEADER *)realloc((fheader+fheader_len+5),uheader_len);
	//这里的错误很明显
//	uheader = (UPGRADE_FILE_HEADER *)malloc(fheader+1);			//???
	uheader = (UPGRADE_FILE_HEADER *)(fheader+1);
	memset(uheader, 0, uheader_len);

	//这里出错了sizeof
	info = (IFNO_ARGV *)malloc(sizeof(IFNO_ARGV));//+=+
	memset(info, 0 ,sizeof(IFNO_ARGV));



	/*
	info->platform = argv[1];
	info->flash_size = argv[2];
	info->mem_size = argv[3];
	info->major_type = argv[4]; 
	info->minor_type = argv[5];
	info->lang = argv[6];
	*/
	info->header_len = header_len;
	info->fheader_len = fheader_len;
	info->uheader_len = uheader_len;
	strcpy(info->platform, argv[1]);
	strcpy(info->flash_size, argv[2]);
	strcpy(info->mem_size, argv[3]);
	strcpy(info->major_type, argv[4]);
	strcpy(info->minor_type, argv[5]);
	strcpy(info->lang, argv[6]);




	/*对输入进行信息转换，存取*/
	info_conv(update_flag, info);
	memcpy(fheader->res,update_flag,18);

	/* 创建升级包文件(digicap.dav) */
	create_dig(argc, argv, fheader, uheader, info);

	

	return 0;
}





















