/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : parse_ini.c
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��12��1��
  ����޸�   :
  ��������   : 1.���ļ���Ҫ��������������ini�����ļ�����ؽӿڣ�����ʵ��ini
               �Ļ�ȡ����ӡ��Լ�ɾ���Ĺ���
               2.���ļ��ǻ��ڱ�׼cʵ�ֵģ���˿����ڶ��ƽ̨������ֲ
               3.���ļ�֧�ֶ���'#'ע�͵�ini�����ļ����н�����#[section]����ע�͵�һ��section
               4.���ļ�֧��������޸Ļ���ɾ������ʱɾ�������ļ��ж����ظ�������
  �����б�   :
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
  �޸���ʷ   :
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "parse_ini.h"
#include <errno.h>

#ifdef __linux__
#include <unistd.h>
#include <pthread.h>
#endif
/*----------------------------------------------*
 * �궨��                                       *
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
#define SECTION_TOO_LONG (-3)	//section����̫��
#define SECTION_IS_NULL	(-2)	//section==NULL
#define NO_INII			(-1)	//δ��ʼ��
#define FINDED			0		//�ҵ�key
#define NO_SECTION		1		//δ�ҵ�section
#define NO_KEY			2		//δ�ҵ�key

#define SUCC	({parse_end(); True;})
#define FAIL	({parse_end(); False;})
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
static int ini_flag = INI_FLAG_NOWRITE;
static FILE *ini_fp = NULL;
static char *ini_buf = NULL;
static size_t ini_bufsize = 0;
static char ini_filename[64] = {0};

#ifdef __linux__
static pthread_mutex_t ini_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*****************************************************************************
 �� �� ��  : parse_start
 ��������  : �����ļ��ĳ�ʼ���������������Ȼ���ֻ����ʽ���ļ���
             ���Ǵ�ʧ�ܾʹ���һ���µģ��������һ���ڴ���ļ��е����ݿ���������
             ����linuxϵͳ��ʼ���ɹ������м����Ĳ���
 �������  : const char *filename  �ļ���
 �� �� ֵ  : ��ʼ���ɹ�����True��ͬʱ��¼���ļ���,��֮����False
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
	{    //����ļ���ֻ����ʽ��ʧ�ܲ��Ҳ�����Ϊ�ļ�������
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
 �� �� ��  : parse_end
 ��������  : ��������ʱ���õĺ�������Ҫ�������ڴ����޸ĵ������ٴ�д�뵽�ļ�
             �У����ͬ���ļ�������ͷ��ڴ棬����linuxϵͳ�����н����Ĳ���
             ֻ��ini_flag==INI_FLAG_WRITE���ڴ�Ż�������ٴ�д�뵽�ļ���Ҳ��˵
             ֻ��������Ϣ���ı��˲Ż�д�룬����ֻ�Ƕ�ȡ��ô����һ���ǲ�����д�������
 �������  : 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : is_notes
 ��������  : �жϴ����Ƿ�Ϊע��
 �������  : char *p  ��ǰ�����е��ַ���ָ��
 �� �� ֵ  : ���������ע�;ͷ���True����֮����False
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : find_key
 ��������  : ͨ��section��key���ҹؼ������ڵ��У�ͨ�����һ������������Ѱ��
             λ�á��������Ȼ��ڴ�������ļ����ڴ�����Ѱsection��������Ѱsection�е�key
 �������  : const char *section  ini�����ļ�����
             const char *key      ini�����ļ����еĹؼ���
             char **line          һ������ָ�룬���Է��ز�ѯ��λ��
 �� �� ֵ  : �����Ҳ���section�򷵻�NO_SECTION��ͬʱ���ش�������ļ��ڴ��β��
             �����Ҳ���key�򷵻�NO_KEY��ͬʱ����section���ײ�
             �����ҵ�key�򷵻�FINDED��ͬʱ����key��һ�����ڵ�λ��
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
	if (strlen(section) > MAX_SECTION_LEN - 3)	//��Ϊ'[' ']' '\0'ռ����3���ַ�
	{
		ERR("The length of section is too long!\nmax_section_len = %d, and section is %s\n", MAX_SECTION_LEN - 3, section);
		return SECTION_TOO_LONG;
	}
	sprintf(str, "[%s]", section);
	while (1) 
	{	//ѭ�����������ļ����Ƿ���[section]
		if((start = strstr(h, str)) == NULL) 
		{
			*line = ini_buf + strlen(ini_buf);	//����Ҳ������ھͰ�ָ���Ƶ��ļ�ĩβ���ݸ��ϲ㺯��
			return NO_SECTION;
		}
		if (!is_notes(start)) 
		{	//�ҵ�[section]������ע�;��˳�ѭ��
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
	{	//ѭ������[section]���Ƿ���key
		end = strchr(start, '[');
		p = strstr(start, key);
						/*��ֹ������section���ҵ�key*/
		if ((p == NULL) || (end != NULL && p > end)) 
		{	//���δ�ҵ�key���Ͱ�ָ��λ���Ƶ�section��β��
			p = (end == NULL) ? (start + strlen(start) - 1) : (end - 1);
			while (isspace(*p) || *p == '#')
			{
				p--;
			}
			*line = strchr(p, '\n') + 1;
			return NO_KEY;
		}
		if (is_notes(p)) 
		{	//�ҵ�key����ע�;ͼ���ѭ��
			start = strchr(p, '\n');
			continue;
		}
		//��ȡ�����ļ����ҵ���key�����key��������ƥ�䣬��ֹ����"abc"��"abc1"��ƥ���bug
		memset(str, 0, sizeof(str));
		while (isspace(*p)) 
		{
		    p++;	//�����ո�����Ʊ��
		}
		sscanf(p, "%[^= \t]", str);
		if (strcmp(key, str) == 0) 
		{ //����ȫƥ�����˳�ѭ��
			break;
		}
		start = strchr(p, '\n');
	}

	*line = p;
	return FINDED;
}

/*****************************************************************************
 �� �� ��  : print_info
 ��������  : ��ӡ���ҽ������Ϣ����Ҫ��������ʹ��
 �������  : int ret              find_key�������صĽ��
             const char *section  ini�����ļ�����
             const char *key      ini�����ļ����еĹؼ���
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : get_one_value
 ��������  : ��ȡһ����ֵ��һ��һ��ϵ������size=12������key����Ӧ��ֵ�������������ܵ���
 �������  : const char *section  
             const char *key      
 �� �� ֵ  : �����ҵ�����key��Ӧ������ֵ����֮����0
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : get_key_string
 ��������  : ��ȡkey��Ӧ���ַ���
 �������  : const char *section  
             const char *key      
             char *buf            �����ṩ��Ž����bufferָ�룬����ΪNULL
 �� �� ֵ  : �����ҵ����key��Ӧ���ַ���������buf�У�������bufָ�룬��֮����NULL
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : get_more_value
 ��������  : ��ȡ�����ֵ�ԣ�һ�Զ��ϵ������week=1,2,3,4,5,6,7��
 �������  : const char *section   
             const char *key       
             unsigned char *array  �������ṩ������ָ��
             int num               �����ṩ�������С
 �� �� ֵ  : �����ҵ����key��Ӧ��ֵ��','��ֳ�������䵽array�в�����True����֮����False
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : add_key_string
 ��������  : ���һ��key=string��ֵ�ԣ�����key���ھ��޸�key��ֵ�������������
             ������֮����ini_flag��ΪINI_FLAG_WRITE�������ñ��ı��ˣ���Ҫ��д
 �������  : const char *section  
             const char *key      
             const char *string   
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
			{	//����ڿ�ͷ��������ӿ���
				sprintf(p, "[%s]\n%s=%s\n", section, key, string);
			} 
			else 
			{ //ÿ��section֮�������
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
 �� �� ��  : add_one_key
 ��������  : ���һ����ֵ�ԣ�һ��һ��ϵ������key=5����Ȼkey��Ӧ��ֵ�Ǹ�����ʱ�ŵ���
 �������  : const char *section  
             const char *key      
             int value            
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void add_one_key(const char *section, const char *key, int value)
{
	char str[128] = {0};
	sprintf(str, "%d", value);
	add_key_string(section, key, str);
}

/*****************************************************************************
 �� �� ��  : add_more_key
 ��������  : ��Ӷ����ֵ�ԣ�һ�Զ��ϵ������week=1,2,3,4
 �������  : const char *section  
             const char *key      
             int *array           
             int num              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : del_key
 ��������  : ɾ��һ����ֵ��
 �������  : const char *section  
             const char *key      
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void del_key(const char *section, const char *key)
{
	char *p = NULL, *tmp = NULL;
	int ret;
	size_t len;
	
	while (1)	//ѭ��Ϊ�˿���ɾ���ظ�������
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
 �� �� ��  : del_section
 ��������  : ɾ��һ��
 �������  : const char *section  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void del_section(const char *section)
{
	char *start = NULL, *end = NULL, *p = NULL;
	size_t len;
	int ret;
	
	while (1)	//ѭ����Ϊ�˿���ɾ���ظ�������
	{
		ret = find_key(section, NULL, &start);
		if (ret != KEY_IS_NULL)
			return;
		end = strchr(start + 1, '[');	//�ҳ������ŵ���һ����ʼλ��
		if (end == NULL) 
		{ // it indicate that it's the last section
			memset(start, '\0', strlen(start));
			return;
		} 
		else 
		{
			if (is_notes(end))
			{	//�����������ŵ�һ�ע���ˣ��ǾͰ�end���˵�ע�͵�λ��Ϊֹ
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
 �� �� ��  : read_profile_string
 ��������  : ��ȡkey��Ӧ���ַ������˺������Ա�ʹ���ߵ������ã�������Ҫ����
             parse_start��parse_end����
 �������  : const char *section        
             const char *key            
             char *value                ��Ŷ�ȡ�����bufferָ��
             int size                   buffer��С
             const char *default_value  Ĭ�Ϸ���ֵ
             const char *file           �ļ���
 �� �� ֵ  : �ɹ�����True��ʧ�ܷ���False
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : read_profile_string
 ��������  : ��ȡkey��Ӧ������ֵ���˺������Ա�ʹ���ߵ������ã�������Ҫ����
             parse_start��parse_end����
 �������  : const char *section        
             const char *key            
             const char *default_value  Ĭ�Ϸ���ֵ
             const char *file           �ļ���
 �� �� ֵ  : �ɹ�����key��Ӧ������ֵ��ʧ�ܷ��ز���Ĭ��ֵdefault_value
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : write_profile_string
 ��������  : д��һ����ֵ�ԣ��˺������Ա�ʹ���ߵ������ã�������Ҫ����
             parse_start��parse_end����
 �������  : const char *section  
             const char *key      
             const char *value    
             const char *file     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
 �� �� ��  : delete_profile_string
 ��������  : ɾ��һ�����һ����ֵ�ԣ���key!=NULLʱɾ��[section]�е�key����֮
             ɾ��[section]
 �������  : const char *section  
             const char *key      
             const char *file     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
