#ifndef __DEBUG_H__
#define __DEBUG_H__


#define COL_DEF "\033[m"
#define COL_RED "\033[0;32;31m"
#define COL_GRE "\033[0;32;32m"
#define COL_BLU "\033[0;32;34m"
#define COL_YEL "\033[1;33m"

#ifdef __KERNEL__	//for linux driver
	#define ERR(fmt, ...) printk(KERN_ERR COL_RED "driver error function[%s]:"\
				COL_YEL fmt COL_DEF "\n", __func__, ##__VA_ARGS__)
	#define INFO(fmt, ...) printk(KERN_INFO COL_GRE "driver information:"\
				COL_YEL	fmt COL_DEF "\n", ##__VA_ARGS__)

	#ifdef DEBUG
	#define DBG(fmt, ...) printk(KERN_DEBUG COL_BLU "debug function[%s]:"\
				COL_DEF	fmt, __func__, ##__VA_ARGS__)
	#else
	#define DBG(fmt, ...) ({0;})
	#endif
#else		//for linux application
	#define ERR(fmt, ...) fprintf(stderr, COL_RED "error function[%s]:"\
				COL_YEL fmt COL_DEF "\n", __func__, ##__VA_ARGS__)
	#define INFO(fmt, ...) fprintf(stderr, COL_GRE "information:"\
				COL_YEL	fmt COL_DEF "\n", ##__VA_ARGS__)

	#ifdef DEBUG
	#define DBG(fmt, ...) printf(COL_BLU "debug function[%s]:"\
				COL_DEF	fmt, __func__, ##__VA_ARGS__)
	#else
	#define DBG(fmt, ...) ({0;})
	#endif
#endif


#endif

