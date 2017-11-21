#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "common.h"

/*****************************************************************************
 函 数 名  : ThreadTestUdpSeverMsgCoundown
 功能描述  : 主要用来模拟平台，每秒钟向20000端口请求倒计时信息，判断倒计时信
             息是否有误
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月15日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void  *TestUdpSeverMsgCoundown()
{
    int cSocket = 0;
    STRU_UDP_INFO sMsg;
    struct sockaddr_in sToAddr;

    cSocket = socket(AF_INET,SOCK_DGRAM,0);

    memset(&sToAddr, 0, sizeof(sToAddr));
    sToAddr.sin_family = AF_INET;
    sToAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sToAddr.sin_port = htons (20000);

    sMsg.iHead = 0x6e6e;
    sMsg.iType = 0x9e;

    printf("TestUdpSeverMsgCoundown start ....\n");

    while(1)
    {
        if(-1 == sendto(cSocket,(void *)&sMsg,sizeof(sMsg),0,(struct sockaddr *)&sToAddr,sizeof(sToAddr)))
        {
            printf("ThreadTestUdpSeverMsgCoundown  error to send msg \n");
        }

        sleep(1);
    }

    return NULL;
}

int ThreadTestCountdown()
{
	int result = 0;
	pthread_t thread_id;
	result = pthread_create(&thread_id,NULL,(void *) TestUdpSeverMsgCoundown,NULL);	
	if(result != 0 )
	{
		printf("ThreadTestCountdown  error!\n");
		return 0;
	}
	return 1;

}










