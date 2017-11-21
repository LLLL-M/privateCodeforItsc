#ifndef __COMMON_H_
#define __COMMON_H_

#define GET_BIT(v, bit) ((v >> bit) & 0x01)			//获取v的某一bit位
#define SET_BIT(v, bit) 	({(v) |= (1 << (bit));})	//设置v的某一bit位


int executeCMD(const char *cmd, char *result, int buffsize);
int open_port(int comport);
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
#endif