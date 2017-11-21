#!/bin/awk -f 

#开头，只调用一次，用来做初始化
BEGIN{
	-F ":"
	printf "----------------------------------------------------------------\n"
	nTotalNum[2] = 0		#总共请求包数目
	nMin[i] = 0			#最小耗时
	nMax[i] = 0			#最大耗时
	nSum[i] = 0			#耗时总和
	nFlagIsFirstTime[2] = 0		
	nLess200[2] = 0			#小于200ms的个数
	nBetween200And500[2] = 0	#大于200小于500的个数
	nBetween500And1000[2] = 0	#大于500小于1000的个数
	nBetween1000And2000[2] = 0	#大于1000小于2000的个数
	nMore2000[2] = 0			#大于2000的个数
	
}

#计算最小，最大值，及分布区间
function GetNumInRange(i)
{
	costTime = (i == 0) ? $6 : $10
	
	if(costTime ~ /^[0-9]+$/)
	{
		if(costTime <= 200)
		{
			nLess200[i]++
		}
		else if(costTime > 200 && costTime <= 500)
		{
			nBetween200And500[i]++
		}
		else if(costTime >500 && costTime <= 1000)
		{
			nBetween500And1000[i]++
		}
		else if(costTime > 1000 && costTime <= 2000)
		{
			nBetween1000And2000[i]++
		}
		else if(costTime > 2000)
		{
			nMore2000[i]++
		}
		
		if(nFlagIsFirstTime[i] == 0)
		{
			nFlagIsFirstTime[i] = 1
			nMin[i] = costTime
		}
		
		if(costTime > nMax[i])
		{
			nMax[i] = costTime
		}
		
		if(costTime < nMin[i])
		{
			nMin[i] = costTime
		}
		
		
		nSum[i] += costTime
		nTotalNum[i]++	
	}

}

#真正的开始部分
{
	GetNumInRange(0);
	GetNumInRange(1);
}

#所有行数都调用结束后得到的总结
END{
	-F ":"
	
	for(i = 0; i <= 1; i++)
	{
		if(i == 0)
		{
			printf("总消耗时间统计: \n")
		}
		else
		{
			printf("\n前端设备消耗时间统计: \n")
		}
		printf("样本总数: %d 个\n",nTotalNum[i])
		printf("最小耗时: %d ms, 最大耗时: %d ms, 平均耗时: %d ms\n\n",nMin[i],nMax[i],nSum[i]/nTotalNum[i])
		printf("   耗时区间\t\t总数\t\t百分占比\n");
		printf("[   0ms, 200ms]\t\t%4d\t\t%2.3f%%\n", nLess200[i],100*nLess200[i]/nTotalNum[i])
		printf("[ 200ms, 500ms]\t\t%4d\t\t%2.3f%%\n", nBetween200And500[i],100*nBetween200And500[i]/nTotalNum[i])
		printf("[ 500ms,1000ms]\t\t%4d\t\t%2.3f%%\n", nBetween500And1000[i],100*nBetween500And1000[i]/nTotalNum[i])
		printf("[1000ms,2000ms]\t\t%4d\t\t%2.3f%%\n", nBetween1000And2000[i],100*nBetween1000And2000[i]/nTotalNum[i])
		printf("[2000ms,^   ms]\t\t%4d\t\t%2.3f%%\n", nMore2000[i],100*nMore2000[i]/nTotalNum[i])	
	}



	printf "\n----------------------------------------------------------------\n"	
}


