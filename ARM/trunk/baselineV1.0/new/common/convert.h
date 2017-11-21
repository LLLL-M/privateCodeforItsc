#ifndef __CONVERT_H__
#define __CONVERT_H__

#include <arpa/inet.h>
#include "HikConfig.h"

typedef enum
{
	INVALID_BIT = 0,
	UNIT_BIT = 1,
	PHASE_BIT = 2,
	PHASETURN_BIT = 3,
	SPLIT_BIT = 4,
	VEHDETECTOR_BIT = 5,
	PEDDETECTOR_BIT = 6,
	CHANNEL_BIT = 7,
	SCHEME_BIT = 8,
	ACTION_BIT = 9,
	TIMEINTERVAL_BIT = 10,
	SCHEDULE_BIT = 11,
	
	FOLLOWPHASE_BIT = 13,
	
	STAGE_TIMING_BIT = 17,
	CONFLICTPHASE_BIT = 18,
} ConvertBit;	//转换比特位

#ifdef USE_GB_PROTOCOL
#include "gb.h"
#define BIT_CLEAR(val, bit)	do {\
	val &= ~(1 << (bit));\
} while (0)
	
#define BIT_SET(val, bit)	do {\
	val |= (1 << (bit));\
} while (0)

#define GB_BIG_ENDIAN //默认使用大端模式，如果使用小端模式把这个宏注释掉即可
#ifdef GB_BIG_ENDIAN
#define LITTLE16_TO_BIG16(v)	htons(v)
#define LITTLE32_TO_BIG32(v)	htonl(v)
#define BIG16_TO_LITTLE16(v)	ntohs(v)
#define BIG32_TO_LITTLE32(v)	ntohl(v)
#else
#define LITTLE16_TO_BIG16(v)	(v)
#define LITTLE32_TO_BIG32(v)	(v)
#define BIG16_TO_LITTLE16(v)	(v)
#define BIG32_TO_LITTLE32(v)	(v)
#endif

typedef void (*ConvertFunc)(GbConfig *gb, SignalControllerPara *ntcip);

typedef struct
{
	ConvertBit bit;
	ConvertFunc func;
} ConvertStruct;
#endif

#endif
