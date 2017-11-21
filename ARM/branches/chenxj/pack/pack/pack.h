#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>



#define UPFILE_NAME			"digicap.dav"	
#define MAX_FILE_NUM		32
//#define FLASH_SECTOR_SIZE	(64*1024)  /*64KB*/

#define IGNORE_VALUE8		0xff
#define IGNORE_VALUE32		0xffffffff
#define CFG_MAGIC			0x484b5753 
//#define RESERVED_FEA_NUMS	16

typedef struct{				/* 44 bytes */
	char 	fileName[32];	/* 文件名 */
	int	    startOffset;	/* 起始位置 */  //4bytes
	int	    fileLen;		/* 文件长度 */  //4bytes
	int  	checkSum;		/* 校验和 */	//4bytes
}UPGRADE_FILE_HEADER;

typedef struct {	/* 64 bytes */
	unsigned int	magic_number;									/* 0x484b5753 */
	unsigned int	header_check_sum;								/* 文件头校验和 */
	unsigned int	header_length;									/* 文件头长度 */
	unsigned int	file_nums;										/* 文件个数 */
	unsigned int	language;										/* 语言 */
	unsigned int	device_class;									/* 1 – DS9000 DVR */
	unsigned int	oemCode;										/* 1 – hikvision  */
	unsigned int	softwareVersion;								/* 软件版本信息*/
	unsigned char	res_feature[12];								/* 保留字段 */
	unsigned char	res[20];										/* 保留字段*/
//	UPGRADE_FILE_HEADER  fileHeader[0];
}FIRMWARE_HEADER;


typedef struct
{
	char	platform[10]; 
	char	flash_size[10]; 
	char	mem_size[10];
	char	major_type[10];
	char	minor_type[10];
	char	lang[10];
	unsigned int 	header_len;
	unsigned int 	fheader_len;
	unsigned int 	uheader_len;
	unsigned int 	language;
}IFNO_ARGV;









