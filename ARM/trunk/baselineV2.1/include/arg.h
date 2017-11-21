#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include "hik.h"

typedef enum
{
	NO_ARGUMENT = 0,		//没有参数
	REQUIRED_ARGUMENT = 1,	//需要参数
	OPTIONAL_ARGUMENT = 2,	//参数可选
} ArgAttr;

typedef void (*ArgFunc)(char *argval);
typedef struct
{
	const char *name;		//参数名,例如:-a,--name
	ArgAttr attr;			//参数属性,参考ArgAttr枚举
	const char *doc;		//参数的使用说明文档
	ArgFunc func;			//参数的处理函数
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
	char optstring[128], *str;				//存放getopt_long的第三个参数
	struct option options[128], *op;	//存放getopt_long的第四个参数
	ArgFunc argDealFunc[256];
	int i, ret = 0;
	int retval = 0, optval = 128;	//短参数和长参数getopt_long的返回值
	
	if (args == NULL || num == 0)
		return;
	memset(optstring, 0, sizeof(optstring));
	memset(options, 0, sizeof(options));
	memset(argDealFunc, 0, sizeof(argDealFunc));
	//把参数信息里的长短参数区分开，分别存放于optstring和option中，为后面调用getopt_long做准备
	str = optstring;
	op = options;
	for (i = 0; i < num; i++)
	{
		ret = IS_STRING_VALID(args[i].name);
		if (ret == 0)	//无效的参数名
		{
			ERR("arg name[%s] is invalid!", args[i].name);
			continue;
		}
		else if (ret == 1)	//以‘-’开头的短参数
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
		else	//以‘--’开头的长参数
		{
			op->name = args[i].name + 2;
			op->has_arg = args[i].attr;
			op->flag = NULL;
			op->val = optval;
			argDealFunc[optval++] = args[i].func;
			op++;
		}
	}
	//最后一个存放-h和--help
	*str = 'h';
	op->name = "help";
	op->has_arg = NO_ARGUMENT;
	op->flag = NULL;
	op->val = 1;	//默认把1作为--help选项的返回值
	
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
