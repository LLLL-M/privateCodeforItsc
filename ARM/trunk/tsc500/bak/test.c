#include <stdio.h>

void main()
{
	char buf[20] = {0};	
	
	strcpy(buf,"just do it !!!!");
	printf("--->  %s\n",buf);

	strcpy(buf,"Go !");
	printf("===>   %s\n",buf);

	memcpy(buf,"Let's Go !",15);
	printf("----->  %s\n",buf);
}



