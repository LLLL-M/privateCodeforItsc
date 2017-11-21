
#include "Unit.h"


PUnitPara InitUnit()
{
    PUnitPara pUnit = (PUnitPara)malloc(sizeof(UnitPara));

    if(!pUnit)
    {
        return NULL;
    }


    memset(pUnit,0,sizeof(UnitPara));

    log_debug("%s succeed \n",__func__);


    return pUnit;

}



int DestroyUnit(PUnitPara pData)
{
    if(!pData)
    {
        return 0;

    }

    free(pData);

    pData = NULL;
    
    return 1;
}


int LoadDefaultUnitPara(PUnitPara pData)
{
    if(!pData)
    {
        return 0;
    }

    pData->nBootYellowLightTime = 6;
    pData->cIsPedestrianAutoClear = 0;
    pData->nDemotionTime = 0;
    pData->nMinRedTime = 0;
    pData->nLightFreq = 60;
    pData->nGatherCycle = 120;
    pData->nBootAllRedTime = 6;
    pData->nTransitCycle = 2;

   // log_debug("%p  %p  %p\n",&pData->nBootYellowLightTime,&pData->cIsPedestrianAutoClear,&pData->nDemotionTime);

  //  log_debug("%p   %p   %p\n",&pData->nMinRedTime,&pData->nLightFreq,&pData->nGatherCycle);

  //  log_debug("%p   %p   %p\n",&pData->nBootAllRedTime,&pData->nTransitCycle,pData);
    
    
    return 1;
}





