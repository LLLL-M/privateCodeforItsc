#pragma once 

#include <array>
#include <bitset>
#include "channel.h"

enum FaultStatus
{   
    NORMAL_STATUS = 0,              //正常
    GREEN_CONFLICT_STATUS = 1,      //绿冲突
    RED_GREEN_CONFLICT_STATUS = 2,  //红绿冲突
    RED_OFF_STATUS = 3,             //红灯熄灭
    YELLOW_FAULT = 4,				//黄灯故障 
};

struct Led;

class FaultCheck
{
private:
	enum { FAULT_COUNT = 4 };
	std::array<int, FAULT_COUNT> normal;
	std::array<int, FAULT_COUNT> error;
	FaultStatus faultStatus;

	bool greenconflict(const Led &green, const Channel &channel);
	bool redgreenconflict(const Led &green, const Led &red, const Channel &channel);
	bool redoff(const Led &red, const Channel &channel);
	bool yellowfault(const Led &yellow, const Channel &channel);

public:
	FaultCheck()
	{
		normal.fill(0);
		error.fill(0);
		faultStatus = NORMAL_STATUS;	//这块可能测国标时要读取故障记录好进行故障恢复
	}

	~FaultCheck() {}

	bool hasfault(const std::bitset<9> &info, bool curCheck, const Channel &channel);
};
