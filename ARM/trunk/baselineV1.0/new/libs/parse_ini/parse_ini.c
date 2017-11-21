/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : parse_ini.c
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年12月1日
  最近修改   :
  功能描述   : 1.该文件主要定义了用来解析ini配置文件的相关接口，可以实现ini
               的获取、添加、以及删除的功能
               2.该文件是基于标准c实现的，因此可以在多个平台进行移植
               3.该文件支持对用'#'注释的ini配置文件进行解析，#[section]可以注释掉一个section
               4.该文件支持在添加修改或是删除配置时删除配置文件中多余重复的配置
  函数列表   :
              add_key_string
              add_more_key
              add_one_key
              delete_profile_section
              del_key
              del_section
              find_key
              get_key_string
              get_more_value
              get_one_value
              is_notes
              parse_end
              parse_start
              print_info
              read_profile_int
              read_profile_string
              write_profile_string
  修改历史   :
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "parse_ini.h"
#include <errno.h>

#ifdef __linux__
#include <unistd.h>
#include <pthread.h>
#endif
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#if defined(__GNUC__)   //linux gcc compiler
#define COL_DEF "\033[m"
#define COL_RED "\033[0;32;31m"
#define COL_GRE "\033[0;32;32m"
#define COL_BLU "\033[0;32;34m"
#define COL_YEL "\033[1;33m"
#define ERR(fmt, ...) fprintf(stderr, COL_BLU "parse library error: "\
		COL_YEL fmt COL_DEF "\n", ##__VA_ARGS__)
#elif defined(_MSC_VER) //windows VS--cl.exe compiler
#define ERR(fmt, ...) fprintf(stderr, "parse library error: " fmt "\n", ##__VA_ARGS__)
#endif

#define NO_VALUE	0
#define USED		1

#define INI_FLAG_NOWRITE	0
#define INI_FLAG_WRITE		1

#define MAX_SECTION_LEN		128

#define MIN_BUFFER_SIZE		(2 * 4096)	//8k

#define	KEY_IS_NULL		(-4)	//key==NULL
#define SECTION_TOO_LONG (-3)	//section长度太长
#define SECTION_IS_NULL	(-2)	//section==NULL
#define NO_INII			(-1)	//未初始化
#define FINDED			0		//找到key
#define NO_SECTION		1		//未找到section
#define NO_KEY			2		//未找到key

#define SUCC	({parse_end(); True;})
#define FAIL	({parse_end(); False;})
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
static int ini_flag = INI_FLAG_NOWRITE;
static FILE *ini_fp = NULL;
static char *ini_buf = NULL;
static size_t ini_bufsize = 0;
static char ini_filename[64] = {0};

#ifdef __linux__
static pthread_mutex_t ini_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*****************************************************************************
 函 数 名  : parse_start
 功能描述  : 解析文件的初始化函数，函数首先会以只读方式打开文件，
             若是打开失败就创建一个新的，随后申请一块内存把文件中的内容拷贝到其中
             若是linux系统初始化成功还会有加锁的操作
 输入参数  : const char *filename  文件名
 返 回 值  : 初始化成功返回True并同时记录下文件名,反之返回False
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
Bool parse_start(const char *filename)
{
	size_t filesize = 0;
	int count;
	
	if (filename == NULL)
	{
		ERR("the filename is NULL");
		return False;
	}
#ifdef __linux__
	pthread_mutex_lock(&ini_mutex);
#endif
	ini_fp = fopen(filename, "r");
	if ((ini_fp == NULL) && (errno != ENOENT)) 
	{    //如果文件是只读方式打开失败并且不是因为文件不存在
		ERR("open file %s error: %s\n", filename, strerror(errno));		
#ifdef __linux__
		pthread_mutex_unlock(&ini_mutex);
#endif
		return False;
	}

	if (ini_fp != NULL)
	{
		(void) fseek(ini_fp, 0, SEEK_END);
		filesize = ftell(ini_fp);
		rewind(ini_fp);
	}
	count = 1 + (filesize / MIN_BUFFER_SIZE);
	ini_buf = calloc(count, MIN_BUFFER_SIZE);
	if (ini_buf == NULL) 
	{
		ERR("calloc buffer fail: %s", strerror(errno));
		if (ini_fp != NULL)
		{
		    fclose(ini_fp);
		}
#ifdef __linux__
		pthread_mutex_unlock(&ini_mutex);
#endif
		return False;
	}
	ini_bufsize = count * MIN_BUFFER_SIZE;
	
	if (ini_fp != NULL) 
	{	
		fread(ini_buf, filesize, 1, ini_fp);
		fclose(ini_fp);
	}
	
	strcpy(ini_filename, filename);
	return True;
}

/*****************************************************************************
 函 数 名  : parse_end
 功能描述  : 解析结束时调用的函数，主要用来把内存中修改的配置再次写入到文件
             中，随后同步文件，最后释放内存，若是linux系统还会有解锁的操作
             只有ini_flag==INI_FLAG_WRITE的内存才会把数据再次写入到文件，也就说
             只有配置信息被改变了才会写入，若是只是读取那么在这一步是不会有写入操作的
 输入参数  : 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void parse_end(void)
{
	if (ini_buf != NULL) 
	{
		if (ini_flag == INI_FLAG_WRITE) 
		{
			ini_fp = fopen(ini_filename, "w");
			fwrite(ini_buf, strlen(ini_buf), 1, ini_fp);
			fflush(ini_fp);
#ifdef __linux__
			fsync(fileno(ini_fp));
#endif
			fclose(ini_fp);
			ini_flag = INI_FLAG_NOWRITE;
		}
		free(ini_buf);
		ini_buf = NULL;
#ifdef __linux__
		pthread_mutex_unlock(&ini_mutex);
#endif
	}
}

/*****************************************************************************
 函 数 名  : is_notes
 功能描述  : 判断此行是否为注释
 输入参数  : char *p  当前所在行的字符串指针
 返 回 值  : 如果此行是注释就返回True，反之返回False
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline Bool is_notes(char *p)
{
	while ((*p != '\n') && (p != ini_buf)) 
	{
		p--;	
	}
	while (isspace(*p)) 
	{
		p++;
	}
	return (*p == '#') ? True : False;
}

/*****************************************************************************
 函 数 名  : find_key
 功能描述  : 通过section和key查找关键字所在的行，通过最后一个参数返回找寻的
             位置。程序首先会在存放配置文件的内存中找寻section，接着找寻section中的key
 输入参数  : const char *section  ini配置文件的项
             const char *key      ini配置文件项中的关键字
             char **line          一个二级指针，用以返回查询的位置
 返 回 值  : 若是找不到section则返回NO_SECTION，同时返回存放配置文件内存的尾部
             若是找不到key则返回NO_KEY，同时返回section的首部
             若是找到key则返回FINDED，同时返回key这一行所在的位置
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static int find_key(const char *section, const char *key, char **line)
{
	char str[MAX_SECTION_LEN] = {0};
	char *start = NULL, *end = NULL;
	char *h = ini_buf, *p;
	
	if (ini_buf == NULL)
	{
		ERR("You haven't called 'parse_start', please call it!");
		return NO_INII;
	}
	if (section == NULL)
	{
		ERR("Parameter error: section == NULL");
		return SECTION_IS_NULL;
	}
	if (strlen(section) > MAX_SECTION_LEN - 3)	//因为'[' ']' '\0'占用了3个字符
	{
		ERR("The length of section is too long!\nmax_section_len = %d, and section is %s\n", MAX_SECTION_LEN - 3, section);
		return SECTION_TOO_LONG;
	}
	sprintf(str, "[%s]", section);
	while (1) 
	{	//循环查找配置文件中是否含有[section]
		if((start = strstr(h, str)) == NULL) 
		{
			*line = ini_buf + strlen(ini_buf);	//如果找不到就在就把指针移到文件末尾传递给上层函数
			return NO_SECTION;
		}
		if (!is_notes(start)) 
		{	//找到[section]若不是注释就退出循环
			break;		
		}
		h = strchr(start, '\n');
	}
	
	if (key == NULL)
	{
		*line = start;
		return KEY_IS_NULL;
	}
	start += strlen(str);
	while (1) 
	{	//循环查找[section]中是否含有key
		end = strchr(start, '[');
		p = strstr(start, key);
						/*防止在其他section中找到key*/
		if ((p == NULL) || (end != NULL && p > end)) 
		{	//如果未找到key，就把指针位置移到section的尾部
			p = (end == NULL) ? (start + strlen(start) - 1) : (end - 1);
			while (isspace(*p) || *p == '#')
			{
				p--;
			}
			*line = strchr(p, '\n') + 1;
			return NO_KEY;
		}
		if (is_notes(p)) 
		{	//找到key若是注释就继续循环
			start = strchr(p, '\n');
			continue;
		}
		//提取配置文件中找到的key与参数key进行完整匹配，防止出现"abc"与"abc1"会匹配的bug
		memset(str, 0, sizeof(str));
		while (isspace(*p)) 
		{
		    p++;	//跳过空格或是制表符
		}
		sscanf(p, "%[^= \t]", str);
		if (strcmp(key, str) == 0) 
		{ //若完全匹配则退出循环
			break;
		}
		start = strchr(p, '\n');
	}

	*line = p;
	return FINDED;
}

/*****************************************************************************
 函 数 名  : print_info
 功能描述  : 打印查找结果的信息，主要用来调试使用
 输入参数  : int ret              find_key函数返回的结果
             const char *section  ini配置文件的项
             const char *key      ini配置文件项中的关键字
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
UNUSEDATTR static void print_info(int ret, const char *section, const char *key)
{
	switch(ret) 
	{
		case NO_SECTION:
			ERR("no this section[%s] be configed", section);
			break;
		
		case NO_KEY:
			ERR("no this key '%s' be configed in section[%s]", key, section);
			break;
		
		default: break;
	}
}

/*****************************************************************************
 函 数 名  : get_one_value
 功能描述  : 获取一个键值，一对一关系，例如size=12，所以key所对应的值必须是整数才能调用
 输入参数  : const char *section  
             const char *key      
 返 回 值  : 若是找到返回key对应的整数值，反之返回0
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
int get_one_value(const char *section, const char *key)
{
	char value[64];
	int ret;
	char *p = NULL;

	ret = find_key(section, key, &p);
	if (ret != FINDED) 
	{
		//print_info(ret, section, key);
		return NO_VALUE;
	}
	
	if (1 == sscanf(p, "%*[^=]=%[^\r\n]", value)) 
	{
		return (int)strtoul(value, NULL, 10);
	} 
	else 
	{
		return NO_VALUE;
	}
}

/*****************************************************************************
 函 数 名  : get_key_string
 功能描述  : 获取key对应的字符串
 输入参数  : const char *section  
             const char *key      
             char *buf            调用提供存放结果的buffer指针，不能为NULL
 返 回 值  : 若是找到则把key对应的字符串拷贝到buf中，并返回buf指针，反之返回NULL
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
char *get_key_string(const char *section, const char *key, char *buf)
{
	int ret;
	char *p = NULL;
	
	if (buf == NULL) 
	{
		ERR("the argument 'buf' can't be NULL");
		return NULL;
	}
	ret = find_key(section, key, &p);
	if (ret != FINDED) 
	{
		//print_info(ret, section, key);
		return NULL;
	}
	
	if (1 == sscanf(p, "%*[^=]=%[^\r\n]", buf)) 
	{
		return buf;
	} 
	else 
	{
		return NULL;
	}
}

/*****************************************************************************
 函 数 名  : get_more_value
 功能描述  : 获取多个键值对，一对多关系，例如week=1,2,3,4,5,6,7，
 输入参数  : const char *section   
             const char *key       
             unsigned char *array  调用者提供的数组指针
             int num               调用提供的数组大小
 返 回 值  : 若是找到则把key对应的值以','拆分成整数填充到array中并返回True，反之返回False
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
Bool get_more_value(const char *section, const char *key, unsigned char *array, int num)
{
	int value, i, ret;
	char line[256] = {0};
	char *p = NULL;
	
	memset(array, 0, num);
	ret = find_key(section, key, &p);
	if (ret != FINDED) 
	{
		//print_info(ret, section, key);
		return False;
	}
	
	sscanf(p, "%[^\r\n]", line);
	p = strchr(line, '=');
	for (i = 0; i < num; i++) 
	{
		if (p == NULL) 
		{
			break;
		}
		p += 1;
		if (1 != sscanf(p, "%d,%*s", &value)) 
		{
			return (i == 0) ? False : True;
		}
		array[i] = (unsigned char)value;
		p = strchr(p, ',');
	}
	
	return True;
}

static inline char *find_enough_memory(int addsize, char *p)
{
	char *newbuf;
	int offset;
	
	if (strlen(ini_buf) + addsize < ini_bufsize)
		return p;
	offset = (int)(p - ini_buf);
	newbuf = realloc(ini_buf, ini_bufsize + MIN_BUFFER_SIZE);
	if (newbuf == NULL)
	{
		ERR("realloc memory isn't enough!");
		return NULL;
	}
	else
	{
		//puts("realloc successful!");
		ini_buf = newbuf;
		memset(ini_buf + ini_bufsize, 0, MIN_BUFFER_SIZE);
		ini_bufsize += MIN_BUFFER_SIZE;
		return ini_buf + offset;
	}
}
/*****************************************************************************
 函 数 名  : add_key_string
 功能描述  : 添加一个key=string键值对，若是key存在就修改key的值，不存在则添加
             添加完成之后会把ini_flag置为INI_FLAG_WRITE表明配置被改变了，需要回写
 输入参数  : const char *section  
             const char *key      
             const char *string   
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void add_key_string(const char *section, const char *key, const char *string)
{
	char *p = NULL;
	char buf[256] = {0};
	int len;
	int ret;
	
	if (ini_buf == NULL)
	{
		ERR("You haven't called 'parse_start', please call it!");
		return;
	}
	ret = find_key(section, key, &p);
	switch(ret) 
	{
		case NO_SECTION:
			if (p == ini_buf) 
			{	//如果在开头就无需添加空行
				sprintf(p, "[%s]\n%s=%s\n", section, key, string);
			} 
			else 
			{ //每个section之间空两行
				sprintf(buf, "\n\n[%s]\n%s=%s\n", section, key, string);
				len = strlen(buf);
				p = find_enough_memory(len, p);
				if (p == NULL)
					break;
				memcpy(p, buf, len);
			}
			break;
		
		case NO_KEY:
			sprintf(buf, "%s=%s\n", key, string);
			len = strlen(buf);
			p = find_enough_memory(len, p);
			if (p == NULL)
				break;
			memmove(p + len, p, strlen(p));
			memcpy(p, buf, len);
			break;
			
		case FINDED:
			del_key(section, key);
			sprintf(buf, "%s=%s\n", key, string);
			len = strlen(buf);
			p = find_enough_memory(len, p);
			if (p == NULL)
				break;
			memmove(p + len, p, strlen(p));
			memcpy(p, buf, len);
			break;
		
		default:
			return;
	}
	ini_flag = INI_FLAG_WRITE;
}

/*****************************************************************************
 函 数 名  : add_one_key
 功能描述  : 添加一个键值对，一对一关系，例如key=5，显然key对应的值是个整数时才调用
 输入参数  : const char *section  
             const char *key      
             int value            
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void add_one_key(const char *section, const char *key, int value)
{
	char str[128] = {0};
	sprintf(str, "%d", value);
	add_key_string(section, key, str);
}

/*****************************************************************************
 函 数 名  : add_more_key
 功能描述  : 添加多个键值对，一对多关系，例如week=1,2,3,4
 输入参数  : const char *section  
             const char *key      
             int *array           
             int num              
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void add_more_key(const char *section, const char *key, unsigned char *array, int num)
{
	char str[1024] = {0};
	char *p = str;
	int i;
	
	if (array == NULL || num <= 0) {
		return;
	}
	for (i = 0; i < num - 1; i++) 
	{
		sprintf(p, "%d,", array[i]);
		p = strchr(p, ',') + 1;
	}
	sprintf(p, "%d", array[num - 1]);
	add_key_string(section, key, str);	
}

/*****************************************************************************
 函 数 名  : del_key
 功能描述  : 删除一个键值对
 输入参数  : const char *section  
             const char *key      
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void del_key(const char *section, const char *key)
{
	char *p = NULL, *tmp = NULL;
	int ret;
	size_t len;
	
	while (1)	//循环为了可以删除重复的配置
	{
		ret = find_key(section, key, &p);
		if (ret != FINDED) 
		{
			return;
		}
		tmp = strchr(p, '\n') + 1;
		len = strlen(tmp);
		memmove(p, tmp, len);
		p += len;
		memset(p, 0, strlen(p));
	}
}

/*****************************************************************************
 函 数 名  : del_section
 功能描述  : 删除一项
 输入参数  : const char *section  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void del_section(const char *section)
{
	char *start = NULL, *end = NULL, *p = NULL;
	size_t len;
	int ret;
	
	while (1)	//循环是为了可以删除重复的配置
	{
		ret = find_key(section, NULL, &start);
		if (ret != KEY_IS_NULL)
			return;
		end = strchr(start + 1, '[');	//找出紧接着的下一项起始位置
		if (end == NULL) 
		{ // it indicate that it's the last section
			memset(start, '\0', strlen(start));
			return;
		} 
		else 
		{
			if (is_notes(end))
			{	//如果后面紧接着的一项被注释了，那就把end倒退到注释的位置为止
				while (*end != '#')
					end--;
			}
			len = strlen(end);
			memmove(start, end, len);
			p = start + len;
			memset(p, '\0', strlen(p));
		}
	}
}

/*****************************************************************************
 函 数 名  : read_profile_string
 功能描述  : 读取key对应的字符串，此函数可以被使用者单独调用，即不需要调用
             parse_start和parse_end函数
 输入参数  : const char *section        
             const char *key            
             char *value                存放读取结果的buffer指针
             int size                   buffer大小
             const char *default_value  默认返回值
             const char *file           文件名
 返 回 值  : 成功返回True，失败返回False
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
int read_profile_string(const char *section, const char *key,char *value, 
						int size, const char *default_value, const char *file)
{
	int ret;
	char *p = NULL;
	
	memset(value, 0, size);
	if (parse_start(file) == False) 
	{
		if (default_value != NULL)
		{
			memcpy(value, default_value, size);
		}
		return FAIL;
	}
	
	ret = find_key(section, key, &p);
	if (ret != FINDED) 
	{
		//print_info(ret, section, key);
		if (default_value != NULL)
		{
			memcpy(value, default_value, size);
		}
		return FAIL;
	}
	
	if (1 == sscanf(p, "%*[^=]=%[^\r\n]", value)) 
	{
		return SUCC;
	} 
	else 
	{
		if (default_value != NULL)
		{
			memcpy(value, default_value, size);
		}
		return FAIL;
	}
}

/*****************************************************************************
 函 数 名  : read_profile_string
 功能描述  : 读取key对应的整数值，此函数可以被使用者单独调用，即不需要调用
             parse_start和parse_end函数
 输入参数  : const char *section        
             const char *key            
             const char *default_value  默认返回值
             const char *file           文件名
 返 回 值  : 成功返回key对应的整数值，失败返回参数默认值default_value
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
int read_profile_int( const char *section, const char *key,int default_value, 
						const char *file)
{
	char value[64];
	
	if (read_profile_string(section,key,value, sizeof(value),"0",file) == SUCC) 
	{
		return (int)strtoul(value, NULL, 10);
	} 
	else 
	{
		return default_value;
	}
}

/*****************************************************************************
 函 数 名  : write_profile_string
 功能描述  : 写入一个键值对，此函数可以被使用者单独调用，即不需要调用
             parse_start和parse_end函数
 输入参数  : const char *section  
             const char *key      
             const char *value    
             const char *file     
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
int write_profile_string(const char *section, const char *key,const char *value, const char *file)
{
	if (parse_start(file) == False) 
	{
		return FAIL;
	}
	add_key_string(section, key, value);
	return SUCC;
}

/*****************************************************************************
 函 数 名  : delete_profile_string
 功能描述  : 删除一项或是一个键值对，当key!=NULL时删除[section]中的key，反之
             删除[section]
 输入参数  : const char *section  
             const char *key      
             const char *file     
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void delete_profile_string(const char *section, const char *key, const char *file)
{
	if (parse_start(file) == False)
	{
		return;
	}
	if (key != NULL) 
	{
	    del_key(section, key);
	} 
	else 
	{
	    del_section(section);
	}
	parse_end();
}
