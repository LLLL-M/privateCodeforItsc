#include "gbconfig.h"

GbConfig config;

int main(void)
{
	LoadDataFromCfg(&config,"./gbconfig.ini");

	return 0;
}



