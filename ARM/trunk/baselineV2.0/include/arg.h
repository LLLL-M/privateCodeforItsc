#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include "hik.h"

typedef enum
{
	NO_ARGUMENT = 0,		//û�в���
	REQUIRED_ARGUMENT = 1,	//��Ҫ����
	OPTIONAL_ARGUMENT = 2,	//������ѡ
} ArgAttr;

typedef void (*ArgFunc)(char *argval);
typedef struct
{
	const char *name;		//������,����:-a,--name
	ArgAttr attr;			//��������,�ο�ArgAttrö��
	const char *doc;		//������ʹ��˵���ĵ�
	ArgFunc func;			//�����Ĵ�����
} ArgInfo;

#define IS_STRING_VALID(string) ({\
	const char *_str = string;\
	int _retval = 0;\
	if (_str[0] == '-') {\
		_str++;\
		if (*_str != '-') {\
			if (isalpha(*_str) > 0 && _str[1] == '\0') \
				_retval = 1;\
		}\
		else if (*_str == '-') {\
			_str++;\
			_retval = 2;\
			do {\
				if (isalpha(*_str++) == 0) {\
					_retval = 0;\
					break;\
				}\
			} while (*_str != 0);\
		}\
	}\
	_retval;})


__attribute__((unused)) static void ArgDeal(int argc, char **argv, ArgInfo *args, int num)
{
	char optstring[128], *str;				//���getopt_long�ĵ���������
	struct option options[128], *op;	//���getopt_long�ĵ��ĸ�����
	ArgFunc argDealFunc[256];
	int i, ret = 0;
	int retval = 0, optval = 128;	//�̲����ͳ�����getopt_long�ķ���ֵ
	
	if (args == NULL || num == 0)
		return;
	memset(optstring, 0, sizeof(optstring));
	memset(options, 0, sizeof(options));
	memset(argDealFunc, 0, sizeof(argDealFunc));
	//�Ѳ�����Ϣ��ĳ��̲������ֿ����ֱ�����optstring��option�У�Ϊ�������getopt_long��׼��
	str = optstring;
	op = options;
	for (i = 0; i < num; i++)
	{
		ret = IS_STRING_VALID(args[i].name);
		if (ret == 0)	//��Ч�Ĳ�����
		{
			ERR("arg name[%s] is invalid!", args[i].name);
			continue;
		}
		else if (ret == 1)	//�ԡ�-����ͷ�Ķ̲���
		{
			*str++ = args[i].name[1];
			if (args[i].attr == REQUIRED_ARGUMENT)
				*str++ = ':';
			else if (args[i].attr == OPTIONAL_ARGUMENT)
			{
				*str++ = ':';
				*str++ = ':';
			}
			retval = args[i].name[1];
			argDealFunc[retval] = args[i].func;
		}
		else	//�ԡ�--����ͷ�ĳ�����
		{
			op->name = args[i].name + 2;
			op->has_arg = args[i].attr;
			op->flag = NULL;
			op->val = optval;
			argDealFunc[optval++] = args[i].func;
			op++;
		}
	}
	//���һ�����-h��--help
	*str = 'h';
	op->name = "help";
	op->has_arg = NO_ARGUMENT;
	op->flag = NULL;
	op->val = 1;	//Ĭ�ϰ�1��Ϊ--helpѡ��ķ���ֵ
	
	while ((optval = getopt_long(argc, argv, optstring, options, NULL)) != -1)
	{
		if (optval == 'h' || optval == 1)
		{
			INFO("The hikTSC help information as follows:");
			for (i = 0; i < num; i++)
			{
				if (args[i].doc)
					fprintf(stderr, "%s: %s\n", args[i].name, args[i].doc);
			}
			_exit(0);
		}
		if (argDealFunc[optval] != NULL)
			argDealFunc[optval](optarg);
	}
}
