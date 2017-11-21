/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : LampController.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��18��
  ����޸�   :
  ��������   : ���ﶨ�����ⲿ�ӿں�����SetLampStatus������ͨ���Ƶ���ɫ
  �����б�   :
              SetLampArray
              SetLampStatus
  �޸���ʷ   :
  1.��    ��   : 2014��7��18��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#include "LampController.h"


static unsigned short g_nLampStatus[8] = {0};

#define N 32
#define M(X) (1u<<(X))
static void __attribute__((unused)) print_data(unsigned int c)    
{ 
	int i;    
	for (i = N - 1; i >= 0; i--)	{         
	   putchar(((c & M(i)) == 0) ? '0':'1');
	   if(i % 8 == 0) printf(" ");
	}       
	printf("\n");
}


static int HardflashDogFlag = 0;

/*********************************************************************************
*
*	дӲ�����źš�
*
***********************************************************************************/

void HardflashDogOutput(void)
{
	if (HardflashDogFlag == 1)
	{
		HardflashDogFlag = 0;
	}
	else
	{
		HardflashDogFlag = 1;
	}
#ifdef ARM_PLATFORM
	
	HardflashDogCtrl(HardflashDogFlag);
#endif
	return;
}




/*********************************************************************************
*
*  д�ƿذ��źŵơ�
*
***********************************************************************************/
static int PhaseLampOutput(int boardNum, unsigned short outLamp)
{

	//printf("=====>   %s   ===>INFO 100130:%d:0x%x|",__func__,boardNum, outLamp); 
	//print_data(outLamp);
	
	if (boardNum<1 || boardNum>8)
	{
		//printf("boardNum error:%d\n", boardNum);
		return -1;
	}

	g_nLampStatus[boardNum-1] = outLamp;
#ifdef ARM_PLATFORM
	
	if (boardNum == 8)
	{
		i_can_its_send_led_request(boardNum, g_nLampStatus);
		//���ذ�����ָʾ��
		Hiktsc_Running_Status();
		//Ӳ�����Ź�ι������
        //
		//HardwareWatchdogKeepAlive(boardNum);
	}
#endif	
	
	return 0;
}



/*****************************************************************************
 �� �� ��  : SetLampArray
 ��������  : ����ͨ���ż���Ҫ���õĵ�ɫ���õ����Ե���CAN�ӿڵ�16λֵ
 �������  : unsigned short *nOutLampArray  
             unsigned short val             
             LampColor type                 
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��18��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetLampArray(unsigned short *nOutLampArray,unsigned short val,LampColor type)
{
    unsigned short nOutLampIndex = 0;
    
    if(nOutLampArray)
    {
        
        if(val <= 4)
        {
            nOutLampIndex = 1;
            //printf("type  :  0x%x   val  :  0x%x  nOutLampArray[n] 0x%x \n",type,val,nOutLampArray[nOutLampIndex - 1]);
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 8) && (val >= 5))
        {
            nOutLampIndex = 2;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 12) && (val >= 9))
        {
            nOutLampIndex = 3;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 16) && (val >= 13))
        {
            nOutLampIndex = 4;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 20) && (val >= 17))
        {
            nOutLampIndex = 5;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 24) && (val >= 21))
        {
            nOutLampIndex = 6;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 28) && (val >= 25))
        {
            nOutLampIndex = 7;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }
        if((val <= 32) && (val >= 29))
        {
            nOutLampIndex = 8;
            nOutLampArray[nOutLampIndex - 1] |= type << ((val - 1 - 4*(nOutLampIndex - 1))*3);
        }

       // printf("type  0x%x   val  0x%x nOutLampArray[nOutLampIndex - 1]  0x%x   \n",type,val,nOutLampArray[nOutLampIndex - 1]);
    }

}


/*****************************************************************************
 �� �� ��  : SetLampStatus
 ��������  : ����ͨ����ɫ
 �������  : unsigned short nTotalControllerLampNum     ���źŻ��ڸ�ģʽ����Ҫ������ͨ������  ���磬�źŻ�������8��ͨ��
             unsigned short *nTotalControllerLampArray  ���źŻ��ڸ�ģʽ�¹�����Ҫ�ľ���ͨ������
             unsigned short nNum                      ������Ҫͬʱ����������ͨ���ĸ���        ���磬�ڵ�ǰ��λ�£���Ҫ����2��ͨ��  
             unsigned short *nArray                    ������Ҫͬʱ����������ͨ������ָ��      ���磬��ǰ���е�ͨ����Ϊ1 4����С��1����1��ʼ���� 
             LampColor type                            ��Ҫ�����ľ���������ǻƵƻ����̵ƺ�� 
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��18��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetLampStatus(unsigned short nTotalControllerLampNum,unsigned short *nTotalControllerLampArray,
                        unsigned short nNum,unsigned short *nArray,LampColor type)
{
    if(nNum > 32 || nTotalControllerLampNum > 32)
    {
        return ;
    }

    int i = 0;
    unsigned short nOutLampArray[8] = {0};

    for(i = 0 ;i < 8 ; i++)
    {
        nOutLampArray[i] = 0x5000;
    }



    //���ȸ���Ҫֱ�ӵ�Ƶ�ͨ��ͨ����ֵ��

    if(type != Lamp_Off)
    {
        for(i=0; i < nNum; i++ )
        {
            SetLampArray(nOutLampArray,nArray[i],type);

        }
    }

    unsigned short nLeftArrayLen = 0;
    unsigned short nLeftArray[32] = {0};
    int j = 0;
    int flag = 0;

    //�ҵ�ʣ���ͨ��
    for(i=0; i < nTotalControllerLampNum; i++)
    {
        for(j=0; j < nNum; j++)
        {
            if(nTotalControllerLampArray[i] == nArray[j])
            {
                flag = 1;
                break;
            }
        }

        if(flag == 0)
        {
            nLeftArray[nLeftArrayLen] = nTotalControllerLampArray[i];
            nLeftArrayLen++;
        }

        flag = 0;
    }

    //
    for(i=0; i < nLeftArrayLen; i++)
    {
        SetLampArray(nOutLampArray, nLeftArray[i], Lamp_Red);
    }

    for(i=1; i <= 8; i++)
    {
        //���������CANͨ�Ŵ��룬�������͵������i_can_its_send_led_request
        //printf("1 ====>   0x%x\n",nOutLampArray[i-1]);
        PhaseLampOutput(i, nOutLampArray[i-1]);

    }
}

/*****************************************************************************
 �� �� ��  : ControlLampLight
 ��������  : ����������ⲿ���õĽӿں����������źŵƵ���ɫ������
 �������  : unsigned short nTotalControllerLampNum     ���źŻ��ڸ�ģʽ����Ҫ������ͨ������  ���磬�źŻ�������8��ͨ��
             unsigned short *nTotalControllerLampArray  ���źŻ��ڸ�ģʽ�¹�����Ҫ�ľ���ͨ������
             unsigned short nNum                      ������Ҫͬʱ����������ͨ���ĸ���        ���磬�ڵ�ǰ��λ�£���Ҫ����2��ͨ��  
             unsigned short *nArray                    ������Ҫͬʱ����������ͨ������ָ��      ���磬��ǰ���е�ͨ����Ϊ1 4����С��1����1��ʼ���� 
             LampColor type                            ��Ҫ�����ľ���������ǻƵƻ����̵ƺ�� 
             short nLightTime                          �õƳ�������״̬��ʱ��             
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��19��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void ControlLampLight(unsigned short nTotalControllerLampNum,unsigned short *nTotalControllerLampArray,
                        unsigned short nNum,unsigned short *nArray,LampColor type,short nLightTime)
{

    if(!nArray || !nTotalControllerLampArray || nLightTime <= 0)
    {
        return;
    }
    nLightTime++;
    do
    {
        if((type == Lamp_Green) || (type == Lamp_Yellow) || (type == Lamp_Red))
        {
           SetLampStatus(nTotalControllerLampNum,nTotalControllerLampArray,nNum,nArray,type);

           sleep(1);

           //HardflashDogOutput();
          // continue;
        }        

        if((type == Lamp_Green_Light) || (type == Lamp_Yellow_Light) || (type == Lamp_Red_Light))
        {
           // printf("Red\n");
           SetLampStatus(nTotalControllerLampNum,nTotalControllerLampArray,nNum,nArray,type-4);//�ȱ��ָ������ɫ
           //sleep(1);//�ȴ�1����
           usleep(500*1000);
          // nLightTime--;
          // printf("Off\n");
           SetLampStatus(nTotalControllerLampNum,nTotalControllerLampArray,nNum,nArray,Lamp_Off);//�ѵ�ǰָ�������
           //sleep(1);
           usleep(500*1000);
           //HardflashDogOutput();
           //continue;
        } 


    }while(--nLightTime > 0);




}


void InitLampController()
{
#ifdef ARM_PLATFORM
	//canͨ�ų�ʼ��
	i_can_its_init();
	
	//CPUͨ��CPLD�����IO�ڳ�ʼ��
	CPLD_IO_Init();
#endif
}

#if 0

int main(void)
{
    unsigned short nTotalControllerLampArray[] = {1,2,3,4,5,6,7,8,9};
    unsigned short nArrayLamp[] = {1,3,5,7};



    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Red,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Green,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Yellow,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Red_Light,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Green_Light,3);

    ControlLampLight(9,nTotalControllerLampArray,4, nArrayLamp, Lamp_Yellow_Light,3);
    


    return 0;
    while(1)
    {
        SetLampStatus(9,nTotalControllerLampArray,0, nArrayLamp, Lamp_Green);
        sleep(1);
        SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);
    }
    printf("All Red   :\n");



    printf("1 7 Green\n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);
    
    printf("1 7 Yellow \n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Yellow);

    printf("2 8 Green\n");
    nArrayLamp[0] = 2;
    nArrayLamp[1] = 8;
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);

    printf("2 8 Yellow\n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Yellow);

    printf("3 9 Green\n");
    nArrayLamp[0] = 3;
    nArrayLamp[1] = 9;
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Green);

    printf("3 9 Yellow\n");
    SetLampStatus(9,nTotalControllerLampArray,2, nArrayLamp, Lamp_Yellow);

    return 0;

    SetLampStatus(8,nTotalControllerLampArray,0, nArrayLamp, Lamp_Green);
    SetLampStatus(8,nTotalControllerLampArray,1, nArrayLamp, Lamp_Yellow);

    printf("\n===============\n");
    unsigned short nArrayLamp2[] = {2};
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp2, Lamp_Green);
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp2, Lamp_Yellow);

    printf("\n===============\n");
    unsigned short nArrayLamp3[] = {3};
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp3, Lamp_Green);
    SetLampStatus(6,nTotalControllerLampArray,1, nArrayLamp3, Lamp_Yellow);



    return 0;
}
#endif
