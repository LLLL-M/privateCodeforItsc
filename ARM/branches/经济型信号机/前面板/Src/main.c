/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��9��
  ����޸�   :
  ��������   : ����ǰ����MCU������Ҫ�����ǽ������Եƿذ��CAN��Ϣ������
               ǰ����ϵ�ģ��·�ں��̵�
  �����б�   :
              assert_failed
              CanGetKeyStatus
              CanGetLedStatus
              CanSendKeyStatus
              Can_Get_MailBox
              Can_Send_Recv_Data
              Cfg_Can
              Cfg_IWDG
              Cfg_Uart
              Error_Handler
              EXTI_IRQHandler_Config
              fputc
              HAL_CAN_RxCpltCallback
              HAL_CAN_TxCpltCallback
              HAL_GPIO_EXTI_Callback
              InitGPIOA
              InitGPIOB
              InitGPIOC
              InitGPIOD
              IsRecvEnd
              main
              SetGPIOInputOrOutput
              SetGroupsLedStatus
              SetKeyLight
              SleepNoEnd
              SystemClock_Config
  �޸���ʷ   :
  1.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���һЩע����Ϣ
******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "main.h"
#include <stdio.h>

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void Cfg_Can();
static void EXTI_IRQHandler_Config(void);

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
CAN_HandleTypeDef    g_CanHandle;
UART_HandleTypeDef g_UartHandle;
IWDG_HandleTypeDef g_IwdgHandle;

//nLedStatus 0x01 Green, 0x02 Red, 0x04 Yellow
typedef enum
{
    GREEN = 0x01,
    RED = 0x02,
    YELLOW = 0x04,
}LEDSTATUS;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static uint8_t g_nArrayLedGroupStatus[4][4];//�ֱ��ʾ�ƿذ�1-4�ĵ���1-4�ĵ�״̬
static uint8_t g_nArrayRecvedGroup[4];//�ѽ��յ��ƿ���Ϣ������
static uint8_t g_nMaxGroup = 1;//���ֵӦ�ø���CAN��Ϣ�õ�����·����������22·�����Ϊ2�������44·�������4;
static uint32_t g_CountRecved = 0;//��������������������Ƶ��


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 48000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PREDIV                         = 1
  *            PLLMUL                         = 6
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  */
static void SystemClock_Config_HSE(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable HSE Oscillator and Activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    Error_Handler(); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)!= HAL_OK)
  {
    Error_Handler(); 
  }

  //����ʱ�Ӱ�ȫϵͳ
  HAL_RCC_EnableCSS();
}


/*****************************************************************************
 �� �� ��  : SystemClock_Config
 ��������  : ϵͳʱ������
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Select HSI48 Oscillator as PLL source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI48;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_OK != HAL_RCC_OscConfig(&RCC_OscInitStruct))
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK and PCLK1 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  
  if (HAL_OK != HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1))
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */

}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*****************************************************************************
 �� �� ��  : SleepNoEnd
 ��������  : ��ѭ������ֹ������ֹ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SleepNoEnd(void)
{
    while(1)
    {
        HAL_Delay(500);
    }

}

/*****************************************************************************
 �� �� ��  : SetGPIOInputOrOutput
 ��������  : ɨ�谴������ʱӦ�ð�GPIOA0-4����Ϊ����ģʽ����������������LED��
             ʱӦ�øĳ��������ģʽ
 �������  : uint8_t mode  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetGPIOInputOrOutput(uint8_t mode)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    if(0 == mode)//input
    {
    	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;//GPIO_MODE_AF_OD
    	//gGPIO_InitStruct.Pull  = GPIO_NOPULL;
    }
    else if(1 == mode)//output
    {
    	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;//GPIO_MODE_AF_OD
    	//gGPIO_InitStruct.Pull  = GPIO_PULLDOWN;
    }
    
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;

	HAL_GPIO_DeInit(GPIOA,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
}


//�ֱ����ֶ����Զ���������ȫ�졢����������״̬
#define KEY0	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)
#define KEY1	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1)
#define KEY2	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2)
#define KEY3	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3)
#define KEY4	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)

/*****************************************************************************
 �� �� ��  : SetKeyLight
 ��������  : ���ݰ���������Ӧ�ĵ�
 �������  : uint8_t num        
             uint8_t keyStatus  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetKeyLight(uint8_t num,uint8_t keyStatus)
{
    uint16_t GPIO_pin;
    
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);		   //�ȵͺ�ߣ�������

    switch (num) {
        case 1: GPIO_pin = GPIO_PIN_0; break;
        case 2: GPIO_pin = GPIO_PIN_1; break;
        case 3: GPIO_pin = GPIO_PIN_2; break;
        case 4: GPIO_pin = GPIO_PIN_3; break;
        case 5: GPIO_pin = GPIO_PIN_4; break;
    }
    printf("+++>  key   %d   Pin  %d\r\n",keyStatus,GPIO_pin);
    HAL_GPIO_WritePin(GPIOA,GPIO_pin, keyStatus ? GPIO_PIN_RESET : GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);	
    //HAL_Delay(10);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);		   //�ȵͺ�ߣ�������

}

/*****************************************************************************
 �� �� ��  : Cfg_Uart
 ��������  : ��������
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���жϷ���ֵ��Ϊ������!
*****************************************************************************/
void Cfg_Uart(void)
{
  g_UartHandle.Instance        = USARTx;

  g_UartHandle.Init.BaudRate   = 9600;
  g_UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  g_UartHandle.Init.StopBits   = UART_STOPBITS_1;
  g_UartHandle.Init.Parity     = UART_PARITY_NONE;
  g_UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  g_UartHandle.Init.Mode       = UART_MODE_TX_RX;
  g_UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT; 

  
  if(HAL_OK != HAL_UART_DeInit(&g_UartHandle))
  {
    Error_Handler();
  }  
  if(HAL_OK != HAL_UART_Init(&g_UartHandle))
  {
    Error_Handler();
  }
}

/*****************************************************************************
 �� �� ��  : Cfg_Can
 ��������  : ��CAN�Ĺ���ģʽ�������ʵȽ�������
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : ����CAN��ʼ��ʧ�ܴ���
*****************************************************************************/
void Cfg_Can(void)
{
    CAN_FilterConfTypeDef  sFilterConfig;
    static CanTxMsgTypeDef        TxMessage;
    static CanRxMsgTypeDef        RxMessage;
    uint8_t err_time = 0;//�������
    
    /*##-1- Configure the CAN peripheral #######################################*/
    g_CanHandle.Instance = CAN;
    g_CanHandle.pTxMsg = &TxMessage;
    g_CanHandle.pRxMsg = &RxMessage;

    g_CanHandle.Init.TTCM = DISABLE;//��ֹʱ�䴥��ͨ��ģʽ
    g_CanHandle.Init.ABOM = DISABLE;//�����CAN_MCR�Ĵ�����INRQλ������1�����0��һ��Ӳ����⵽128��11λ����������λ�����˳�����״̬��
    g_CanHandle.Init.AWUM = DISABLE;//˯��ģʽͨ�����CAN_MCR�Ĵ�����SLEEPλ�����������
    g_CanHandle.Init.NART = DISABLE;//CAN����ֻ������1�Σ����ܷ��͵Ľ����Σ��ɹ���������ٲö�ʧ��
    g_CanHandle.Init.RFLM = DISABLE;//�ڽ������ʱFIFOδ��������������FIFO�ı���δ����������һ���յ��ı��ĻḲ��ԭ�еı���
    g_CanHandle.Init.TXFP = DISABLE;//����FIFO���ȼ��ɱ��ĵı�ʶ��������
    g_CanHandle.Init.Mode = CAN_MODE_NORMAL;//��������ʱ���������
    //g_CanHandle.Init.Mode = CAN_MODE_LOOPBACK;//����ʱ�����������
    g_CanHandle.Init.SJW = CAN_SJW_1TQ;//����ͬ����Ծ���1��ʱ�䵥λ
    g_CanHandle.Init.BS1 = CAN_BS1_13TQ;//ʱ���1Ϊ8��ʱ�䵥λ
    g_CanHandle.Init.BS2 = CAN_BS2_2TQ;//ʱ���2Ϊ7��ʱ�䵥λ
    g_CanHandle.Init.Prescaler = 6;//���������úͺ��İ���ͬ��Ϊ500K,48M/6/(1+13+2);!!!!!!!!!!!!!!!!!!

    while(HAL_OK != HAL_CAN_Init(&g_CanHandle))
    {
        err_time++;
        if(255 == err_time)
        {
            printf("Can Init Error \r\n");
            return;
        }
    }

    /*##-2- Configure the CAN Filter ###########################################*/
    sFilterConfig.FilterNumber = 0;//ָ���˴���ʼ���Ĺ�����0
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;//ָ���˹�����������ʼ������ģʽΪ��ʶ������λģʽ
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;//�����˹�����λ��1��32λ������
    sFilterConfig.FilterIdHigh = 0x0000;//�����趨��������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ����
    sFilterConfig.FilterIdLow = 0x0000;//�����趨��������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
    sFilterConfig.FilterMaskIdHigh = 0x0000;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ��
    sFilterConfig.FilterMaskIdLow = 0x0000;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
    sFilterConfig.FilterFIFOAssignment = CAN_FIFO0;//�趨��ָ���������FIFO0
    sFilterConfig.FilterActivation = ENABLE;//ʹ�ܹ�����		
    sFilterConfig.BankNumber = 14;

    if (HAL_OK != HAL_CAN_ConfigFilter(&g_CanHandle, &sFilterConfig))
    {
        Error_Handler();
    }
}

/*****************************************************************************
 �� �� ��  : Cfg_IWDG
 ��������  : ��ʼ���������Ź������Ź���ι��������2��
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2015��1��6��
    ��    ��   : xiaowh
    �޸�����   : ��ι��ʱ���Ϊ1s
*****************************************************************************/
void Cfg_IWDG(void)
{
    g_IwdgHandle.Instance = IWDG;

    g_IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;
    //g_IwdgHandle.Init.Reload    = 2500;//2500 / 1.25 == 2000ms
    g_IwdgHandle.Init.Reload    = 1250;//1250 / 1.25 = 1000ms
    g_IwdgHandle.Init.Window    = IWDG_WINDOW_DISABLE;

    if (HAL_OK != HAL_IWDG_Init(&g_IwdgHandle))
    {
        /* Initialization Error */
        Error_Handler();
    }
}


/*****************************************************************************
 �� �� ��  : CalLedOutputCode
 ��������  : ���ݵ�ɫ����Ӧ�ø��Ĵ�����ֵ
 �������  : LEDSTATUS nLedStatus  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��1��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �ٴ��޸��ĸ������˳��
*****************************************************************************/
uint32_t CalLedOutputCode(LEDSTATUS nLedStatus)
{
    uint8_t i = 0;//i ����ѭ�����������ĸ�����
    uint8_t j = 0;//j ����ѭ����ת��ֱ�С���ת�������ĸ����̵�
    uint32_t tmp = 0;

    for(i = 0;i < 4 ; i++)
    {
        for(j = 0; j < 4 ; j++)
        {
            if(nLedStatus != (g_nArrayLedGroupStatus[i][j]&nLedStatus))//�ж��̵ơ���ơ��Ƶ��Ƿ���Ҫ������Ĭ���ǵ�����
            {
                if(3 == j)//��������ˣ���˳��ֱ���GB13 GB14 GB15 GB16
                {
                    if(nLedStatus == YELLOW)//����ǻƵƵĻ���ֱ��continue���������м���
                    {
                        continue;
                    }
                    tmp |= ((uint16_t)0x01 << (12 + i)) ;
                    continue;
                }
            
                if(i < 3)//���������������ͬ���ְ�ΪGPIO 7 8 9, 4 5 6, 1 2 3, 10 11 12
                {
                    tmp |= ((uint16_t)0x01 << (6 + j - i*3)) ;
                }
                else
                {
                    tmp |= ((uint16_t)0x01 << (9 + j)) ;
                }
            }
        }
    }

    return tmp;
}

/*****************************************************************************
 �� �� ��  : SetGroupsLedStatus
 ��������  : ��CAN��Ϣ�������֮�󣬸��ݸ���Ƶ�״̬�����ε�ơ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : Ӳ����������ƵƸı�󣬴�������Ӧ������

  3.��    ��   : 2015��1��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���ն���ͨ��1234������ͨ��5678������ͨ��9 10 11 12������ͨ
                 ��13 14 15 16��˳�����µ��
*****************************************************************************/
void SetGroupsLedStatus(void)
{
    //green
    GPIOB->ODR = CalLedOutputCode(GREEN); 
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);   
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET); 

    //red
    GPIOB->ODR = CalLedOutputCode(RED); 
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);   
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);      

    //yellow
    GPIOC->ODR = CalLedOutputCode(YELLOW);
}


/*****************************************************************************
 �� �� ��  : CanGetLedStatus
 ��������  : �������Եƿذ�ĵ����Ϣ
 �������  : CAN_HandleTypeDef *CanHandle  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2015��1��7��
    ��    ��   : xiaowh
    �޸�����   : �ж�22·ʱ�ķ�����1��ʾ44·��2��ʾ22·��Ӧ��ֱ���жϲ�Ӧ���жϵ�һλ��ֵ
*****************************************************************************/
void CanGetLedStatus(CAN_HandleTypeDef *CanHandle)
{
    uint8_t nGroupId;//�ƿذ���ϵ��£�������ƣ�ǰ�����ǻ��������к���̵ı仯������4��Ϊ���˵ƣ�ֻ�к��̵ı仯��
    
    switch(CanHandle->pRxMsg->StdId)
    {
        case 0x301: nGroupId = 0;break;
        case 0x302: nGroupId = 1;break;
        case 0x303: nGroupId = 2;break;
        case 0x304: nGroupId = 3;break;
        default:    nGroupId = 0;break;
    }

    g_nArrayLedGroupStatus[nGroupId][0] = CanHandle->pRxMsg->Data[1]&0x07;
    g_nArrayLedGroupStatus[nGroupId][1] = CanHandle->pRxMsg->Data[2]&0x07;
    g_nArrayLedGroupStatus[nGroupId][2] = CanHandle->pRxMsg->Data[3]&0x07;
    g_nArrayLedGroupStatus[nGroupId][3] = CanHandle->pRxMsg->Data[4]&0x07;

    g_nArrayRecvedGroup[nGroupId] = 1;//�ж��յ���������

    if(1 == (CanHandle->pRxMsg->Data[0]))//44·�������
    {
        g_nMaxGroup = 4;
    }
    else if(2 == (CanHandle->pRxMsg->Data[0]))//22·������� ��ʱҪ�Ѷ��Ϸ����״̬��Ϣ���Ƶ���������ȥ��
    {
        g_nMaxGroup = 2;
        //printf("===>   StdId  0x%x   GroupId  %d  \r\n",CanHandle->pRxMsg->StdId,nGroupId);
        memcpy(g_nArrayLedGroupStatus[2],g_nArrayLedGroupStatus[0],4);
        memcpy(g_nArrayLedGroupStatus[3],g_nArrayLedGroupStatus[1],4);
    }
}

/*****************************************************************************
 �� �� ��  : CanGetKeyStatus
 ��������  : �������Ժ��İ�İ���״̬��Ϣ
 �������  : CAN_HandleTypeDef *CanHandle  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void CanGetKeyStatus(CAN_HandleTypeDef *CanHandle)
{
    if(0x110 != CanHandle->pRxMsg->StdId)
    {
        return ;
    }

    GPIO_InitTypeDef   GPIO_InitStructure;

    uint8_t i = 0;

    //����GPIOAΪ���ģʽ
    SetGPIOInputOrOutput(1);
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_15_IRQn); 
    
    for(i = 0; i < 5 ; i++)
    {
        SetKeyLight(i+1,((CanHandle->pRxMsg->Data[0]>>i) & 0x01));
    }

    EXTI_IRQHandler_Config();
}

/*****************************************************************************
 �� �� ��  : IsRecvEnd
 ��������  : �ж��Ƿ������ɣ�ֻ�е�ȫ�����ܽ������ͳһ���е�Ʋ�����
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t IsRecvEnd(void)
{
    uint8_t i ; 
    uint8_t nRecvNum = 0;
    
    for(i = 0 ; i < 4; i++)
    {
        if(1 == g_nArrayRecvedGroup[i])
        {
            nRecvNum++;
        }
    }

    if(nRecvNum == g_nMaxGroup)
    {
        memset(g_nArrayRecvedGroup,0,sizeof(g_nArrayRecvedGroup));
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************
 �� �� ��  : Can_Get_MailBox
 ��������  : ��ÿ��е�CAN������
 �������  : CAN_HandleTypeDef* hcan  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t Can_Get_MailBox(CAN_HandleTypeDef* hcan)
{
    uint8_t transmitmailbox = CAN_TXSTATUS_NOMAILBOX;

    if(CAN_TSR_TME0 == (hcan->Instance->TSR&CAN_TSR_TME0))
    {
      transmitmailbox = 0;
    }
    else if(CAN_TSR_TME1 == (hcan->Instance->TSR&CAN_TSR_TME1))
    {
      transmitmailbox = 1;
    }
    else if(CAN_TSR_TME2 == (hcan->Instance->TSR&CAN_TSR_TME2))
    {
      transmitmailbox = 2;
    }

    return transmitmailbox;
}


/*****************************************************************************
 �� �� ��  : Can_Send_Recv_Data
 ��������  : ������װһ��CAN���ͺͽ��յĺ�������������CAN��Ϣʧ�ܵľ���ԭ
             ��
 �������  : CAN_HandleTypeDef *CanHandle  CAN���
             uint8_t nSendOrRecv           0 ��ʾ������Ϣ, 1 ��ʾ������Ϣ
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t Can_Send_Recv_Data(CAN_HandleTypeDef *CanHandle,uint8_t nSendOrRecv)
{
    uint8_t err_time = 0;//�������
    uint8_t err_code = 0;
    
    if(1 == nSendOrRecv)//����CAN��Ϣ�ж�
    {
        while(HAL_OK != (err_code = HAL_CAN_Receive_IT(CanHandle, CAN_FIFO0)))
        {
            err_time++;

            printf("#######Can  Recv Failed ,State  0x%x  Locked  0x%x ,err_code 0x%x\r\n",CanHandle->State,CanHandle->Lock,err_code);
            if(10 == err_time)
            {
                printf("Can Recv Error \r\n");                
                return 0;
            }
        }
    }
    else if(0 == nSendOrRecv)//����CAN��Ϣ�ж�
    {
        while(HAL_OK != (err_code = HAL_CAN_Transmit(CanHandle, 10)))//Cube���е����⣬�жϷ���ʱ���ܻ��������CAN�����Ȳ���ֱ�ӷ��ͷ�ʽ
        //while(HAL_CAN_Transmit_IT(CanHandle) != HAL_OK)//CAN�����޸ĺ���ʹ���жϷ�ʽ
        {
            err_time++;

            printf("#######Can  Send Failed , State  0x%x  Locked  0x%x transmitmailbox  0x%x err_code 0x%x\r\n",CanHandle->State,CanHandle->Lock,Can_Get_MailBox(CanHandle),err_code);
            if(10 == err_time)
            {
                printf("Can Send Error \r\n");
                return 0;
            }
        }
    }

    return 1;
}

/*****************************************************************************
 �� �� ��  : HAL_CAN_RxCpltCallback
 ��������  : CAN������ɻص�����
 �������  : CAN_HandleTypeDef *CanHandle  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2015��1��7��
    ��    ��   : xiaowh
    �޸�����   : �޸�ι��ʱ��������Ƶ��
*****************************************************************************/
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *CanHandle)
{
    if((0x110 == CanHandle->pRxMsg->StdId)&&(1 == CanHandle->pRxMsg->DLC))//���ذ��ǰ��巢�͵İ�������״̬��Ϣ
    {
        HAL_IWDG_Refresh(&g_IwdgHandle);
        printf("StdID :  %d , Data[0]  :  0x%x \r\n",CanHandle->pRxMsg->StdId,CanHandle->pRxMsg->Data[0]);
        CanGetKeyStatus(CanHandle);
		g_CountRecved++;
        //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_5);
    }
    else if((5 == CanHandle->pRxMsg->DLC)&&((0x301 == CanHandle->pRxMsg->StdId) || (0x302 == CanHandle->pRxMsg->StdId) || (0x303 == CanHandle->pRxMsg->StdId) || (0x304 == CanHandle->pRxMsg->StdId)))
    {
        HAL_IWDG_Refresh(&g_IwdgHandle);
        CanGetLedStatus(CanHandle);

        //if(IsRecvEnd() == 1)
        {
            SetGroupsLedStatus();
        }            
		g_CountRecved++;
    }

    if(5 == g_CountRecved)
    {
        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_8);
        g_CountRecved = 0;
    }
    Can_Send_Recv_Data(CanHandle,1);//�����µ�CAN��Ϣ
}

/*****************************************************************************
 �� �� ��  : HAL_CAN_TxCpltCallback
 ��������  : CAN��Ϣ������ɻص�������
 �������  : CAN_HandleTypeDef* hcan  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2015��1��7��
    ��    ��   : xiaowh
    �޸�����   : ������ɺ���ι��
*****************************************************************************/
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
    //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_4);
    
    //HAL_IWDG_Refresh(&g_IwdgHandle);
}

/*****************************************************************************
 �� �� ��  : HAL_CAN_ErrorCallback
 ��������  : CANͨѶ���ϻص�����
 �������  : CAN_HandleTypeDef *hcan  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 20115��1��6��
    ��    ��   : xiaowh
    �޸�����   : ��CAN�����������û�н������ǰ��ֱ������MCU����
*****************************************************************************/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    printf("Can Communication Error , Error Code is : 0x%x  , State Code is : 0x%x\r\n",hcan->ErrorCode,hcan->State);
    printf("Software will restart ....\r\n");
    //HAL_Delay(1200);//��ʱ1.2sʹwatchdog ��Ч�����������
//    NVIC_SystemReset();//����ϵͳ
}

/*****************************************************************************
 �� �� ��  : EXTI_IRQHandler_Config
 ��������  : ���ð����ж�
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void EXTI_IRQHandler_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Configure PC.13 pin as input floating */
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

/*****************************************************************************
 �� �� ��  : CanSendKeyStatus
 ��������  : ���а�������ʱ��ǰ��������ذ巢�����а�����״̬��Ϣ�����ߵ���
             �İ�������ʱ����������Ϣ��
 �������  : uint8_t nKeyStatus  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void CanSendKeyStatus(uint8_t nKeyStatus)
{
    g_CanHandle.pTxMsg->StdId = 0x401;
    g_CanHandle.pTxMsg->ExtId = 0;
    g_CanHandle.pTxMsg->RTR = CAN_RTR_DATA;
    g_CanHandle.pTxMsg->IDE = CAN_ID_STD;
    g_CanHandle.pTxMsg->DLC = 1;
    g_CanHandle.pTxMsg->Data[0] = nKeyStatus;

    Can_Send_Recv_Data(&g_CanHandle,0);
}

/*****************************************************************************
 �� �� ��  : HAL_GPIO_EXTI_Callback
 ��������  : �����жϻص�����
 �������  : uint16_t GPIO_Pin  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t nKeyStatus = 0;//����LED�Ƶ�״̬����5λ��Ч���ֱ��ʾ�Զ����ֶ���������ȫ�졢������״̬��1��ʾ���£�0��ʾδ���¡�

    if ((GPIO_Pin == GPIO_PIN_0)&&(!KEY0))
    {
        nKeyStatus |= 0x01;
    }
    else if((GPIO_Pin == GPIO_PIN_1)&&(!KEY1))
    {
        nKeyStatus |= (0x01 << 0x01);
    }
    else if((GPIO_Pin == GPIO_PIN_2)&&(!KEY2))
    {
        nKeyStatus |= (0x01 << 0x02);
    }
    else if((GPIO_Pin == GPIO_PIN_3)&&(!KEY3))
    {
        nKeyStatus |= (0x01 << 0x03);
    }
    else if((GPIO_Pin == GPIO_PIN_4)&&(!KEY4))
    {
        nKeyStatus |= (0x01 << 0x04);
    }
    printf("===>  HAL_GPIO_EXTI_Callback , Pin  %d   nKeyStatus  %d\r\n",GPIO_Pin,nKeyStatus);
    if(nKeyStatus != 0)//��ֹ�����
    {
        //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_3);
        printf("Send Key Status :  0x%x \r\n",nKeyStatus);
        CanSendKeyStatus(nKeyStatus);
    }
}

/*****************************************************************************
 �� �� ��  : InitGPIOA
 ��������  : ��ʼ��GPIOA
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void InitGPIOA(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//����Ч��Ĭ����Ҫ���͵�
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin = GPIO_PIN_5;//����ָʾ��ʹ�ܽ�
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//����Ч��Ĭ����Ҫ���ߵ�
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_15;//�ⲿ���������LEDʹ�ܽ�  / �ⲿ���������ʱ�� /CAN�źŽ���ָʾ��
                                                                                // �ⲿ��������ư���ʹ�ܽ�
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    


    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);	 //����PA5�ߵ�ƽ��ʹ�ܺ���ָʾ��

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);//����PA6�͵�ƽ��ʹ�ܰ�������LED

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);//

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);//Ĭ��״̬�£�Ҫ��CAN����ָʾ���������ʼ��ֻ�Ǹ�ODR�Ĵ���һ��RESETֵ�������RESETֵ������
                                                        //0�����ԣ�Init������CANָʾ�ƻ�����.
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);  //ʹ���ⲿ����ֵ����

    //CANͨ������PA11 PA12��ר��CAN��ʼ����������ɳ�ʼ����
}

/*****************************************************************************
 �� �� ��  : InitGPIOB
 ��������  : ��ʼ��GPIOB
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void InitGPIOB(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    __GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Pull = GPIO_PULLUP;//��ʼ����
    GPIO_InitStruct.Pin = GPIO_PIN_All;//
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  //����ָʾ������

    GPIOB->ODR = 0xffff;
}

/*****************************************************************************
 �� �� ��  : InitGPIOC
 ��������  : ��ʼ��GPIOC
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void InitGPIOC(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    __GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Pull = GPIO_PULLUP;//��ʼ����
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|
                            GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|
                            GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;//
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);  //��ָʾ�ƿ�����

    GPIOC->ODR |= 0x1fff;
}

/*****************************************************************************
 �� �� ��  : InitGPIOD
 ��������  : ��ʼ��GPIOD
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void InitGPIOD(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    __GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//
    GPIO_InitStruct.Pull = GPIO_PULLUP;//��ʼ����
    GPIO_InitStruct.Pin = GPIO_PIN_2;//
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);  //��ָʾ�ƿ�����

    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
}

/*****************************************************************************
 �� �� ��  : fputc
 ��������  : �ض���fputc���� ,�����Ϳ���ʹ��printfL
 �������  : int ch   
             FILE *f  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��12��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : ��ʽ��ȥ����ӡ��Ϣ��
*****************************************************************************/
int fputc(int ch, FILE *f)
{   
    //return ch;
	while((USART1->ISR&0X40)==0);//ѭ������,ֱ���������   
        USART1->TDR = (uint8_t) ch;      

	return ch;
}

/*****************************************************************************
 �� �� ��  : TestLedIndex
 ��������  : ���Ժ�������������ʵ��LED�Ƶ�˳��
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��1��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void TestLedIndex()
{
    uint8_t i = 0;
    uint8_t j = 0;

    while(1)
    {

        GPIOB->ODR = 0;

        for(i = 0 ; i < 16 ; i++)
        {
            GPIOB->ODR |= (1 << i);
            
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);   
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);         

            HAL_Delay(1000);
        }

    }



    return;
    for(i = 0 ; i < 4; i++)
    {
        for(j = 0 ; j < 4; j++)
        {
            g_nArrayLedGroupStatus[i][j] = GREEN;
            HAL_Delay(400);
            SetGroupsLedStatus();
        }
    }
    HAL_Delay(50);
    memset(g_nArrayLedGroupStatus,0,sizeof(g_nArrayLedGroupStatus));
    for(i = 0 ; i < 4; i++)
    {
        for(j = 0 ; j < 4; j++)
        {
            g_nArrayLedGroupStatus[i][j] = RED;
            HAL_Delay(400);
            SetGroupsLedStatus();
        }
    }
    memset(g_nArrayLedGroupStatus,0,sizeof(g_nArrayLedGroupStatus));

    HAL_Delay(50);
    for(i = 0 ; i < 4; i++)
    {
        for(j = 0 ; j < 4; j++)
        {
            g_nArrayLedGroupStatus[i][j] = YELLOW;
            HAL_Delay(400);
            SetGroupsLedStatus();
        }
    }
    
}


/*****************************************************************************
 �� �� ��  : main
 ��������  : ǰ������������
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���
  2.��    ��   : 2015��1��7��
    ��    ��   : xiaowh
    �޸�����   : ����ѭ�����Ѳ���ι����ֻ���յ�CAN��Ϣ���ι��������ͻ����������������Ź�������CAN�����ж�
	
*****************************************************************************/
int main(void)
{
    HAL_Init();
    
    SystemClock_Config();//Configure the system clock to 48 MHz

    if (0 != __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        HAL_Delay(1000);
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    else
    {
        //��1s��ʱ����֤ʱ���ȶ�
        HAL_Delay(1000);
    }

    InitGPIOA();
    InitGPIOB();
    InitGPIOC();
    InitGPIOD();
    Cfg_Uart();
    Cfg_Can();
    Cfg_IWDG();
    //TestLedIndex();
    if(HAL_OK != HAL_IWDG_Start(&g_IwdgHandle))// Start the IWDG
    {
        Error_Handler();
    }    
	
    //���жϷ�ʽ���հ�������
    EXTI_IRQHandler_Config();


    if(HAL_OK != HAL_CAN_Receive_IT(&g_CanHandle, CAN_FIFO0))
    {
        Error_Handler();
    }

    printf("==> System Restart ... \r\n");

    while (1)
    {
        HAL_Delay(500);
#if 1
        if (HAL_OK != HAL_IWDG_Refresh(&g_IwdgHandle))//ι��
        {
          Error_Handler();
        }
#endif
    }
}


