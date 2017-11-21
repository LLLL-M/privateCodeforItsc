#include "ykconfig.h"

YK_Config config;

int main(void)
{
	LoadDataFromCfg(&config,"./gbconfig.ini");

	return 0;
}



