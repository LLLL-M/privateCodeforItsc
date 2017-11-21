#include "LogSystem.h"


int main(void)
{
	int i = 0;
	int j = 0;
	
	InitLogSystem("./Log/",3,1);
	
	while(++j <= 0xfffff)
	{
		while(++i <= 0xff)
		{
			log_debug("美丽中国人，可爱中国梦 ---->  0x%x---0x%x \n",j,i);
		}
		i = 0;
	}
	
	

	return 0;
}
