/**
  ******************************************************************************
  * @file    ADC/ADC_Sequencer/Inc/main.h
  * @author  MCD Application Team
  * @version V1.1.0RC2
  * @date    01-Aug-2014
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx_nucleo.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor ADCx instance under use and associated
   resources */

/* ## Definition of ADC related resources ################################### */
/* Definition of ADCx clock resources */
#define ADCx                            ADC1
#define ADCx_CLK_ENABLE()               __ADC1_CLK_ENABLE()

#define ADCx_FORCE_RESET()              __ADC1_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __ADC1_RELEASE_RESET()

/* Definition of ADCx channels */
#define ADCx_CHANNELa                   ADC_CHANNEL_4
#define ADCx_CHANNELb                   ADC_CHANNEL_TEMPSENSOR

/* Definition of ADCx channels pins */
#define ADCx_CHANNELa_GPIO_CLK_ENABLE() __GPIOA_CLK_ENABLE()
#define ADCx_CHANNELa_GPIO_PORT         GPIOA
#define ADCx_CHANNELa_PIN               GPIO_PIN_4

/* Definition of ADCx DMA resources */
#define ADCx_DMA_CLK_ENABLE()           __DMA1_CLK_ENABLE()
#define ADCx_DMA                        DMA1_Channel1

#define ADCx_DMA_IRQn                   DMA1_Ch1_IRQn
#define ADCx_DMA_IRQHandler             DMA1_Ch1_IRQHandler

/* Definition of ADCx NVIC resources */
#define ADCx_IRQn                       ADC1_COMP_IRQn
#define ADCx_IRQHandler                 ADC1_COMP_IRQHandler

/* ## Definition of DAC related resources ################################### */
/* Definition of DACx clock resources */
#define DACx                            DAC
#define DACx_CLK_ENABLE()               __DAC1_CLK_ENABLE()
#define DACx_CHANNEL_GPIO_CLK_ENABLE()  __GPIOA_CLK_ENABLE()

#define DACx_FORCE_RESET()              __DAC1_FORCE_RESET()
#define DACx_RELEASE_RESET()            __DAC1_RELEASE_RESET()

/* Definition of DACx channels */
#define DACx_CHANNEL_TO_ADCx_CHANNELa            DAC_CHANNEL_1

/* Definition of DACx channels pins */
#define DACx_CHANNEL_TO_ADCx_CHANNELa_PIN        GPIO_PIN_4
#define DACx_CHANNEL_TO_ADCx_CHANNELa_GPIO_PORT  GPIOA

//一下是对CAN的配置

#define CANx                            CAN
#define CANx_CLK_ENABLE()               __CAN_CLK_ENABLE()
#define CANx_GPIO_CLK_ENABLE()          __GPIOA_CLK_ENABLE()

#define CANx_FORCE_RESET()              __CAN_FORCE_RESET()
#define CANx_RELEASE_RESET()            __CAN_RELEASE_RESET()

/* Definition for USARTx Pins */
#define CANx_TX_PIN                    GPIO_PIN_12
#define CANx_TX_GPIO_PORT              GPIOA
#define CANx_TX_AF                     GPIO_AF4_CAN
#define CANx_RX_PIN                    GPIO_PIN_11
#define CANx_RX_GPIO_PORT              GPIOA
#define CANx_RX_AF                     GPIO_AF4_CAN

/* Definition for USARTx's NVIC */
#define CANx_RX_IRQn                   CEC_CAN_IRQn
#define CANx_RX_IRQHandler             CEC_CAN_IRQHandler

#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

//#define __GPIOD_CLK_ENABLE()         (RCC->AHBENR |= (RCC_AHBENR_GPIODEN))

/* Definition for USARTx clock resources */
#define USARTx                           USART1
#define USARTx_CLK_ENABLE()              __USART1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_9
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF1_USART1
#define USARTx_RX_PIN                    GPIO_PIN_10
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF1_USART1

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART1_IRQn
#define USARTx_IRQHandler                USART1_IRQHandler

/* Size of Trasmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                      TXBUFFERSIZE

/* Definition of TIM instance */
#define TIMx                              TIM14

/* Definition for TIMx clock resources */
#define TIMx_CLK_ENABLE()                 __TIM14_CLK_ENABLE()

/* Definition for TIMx's NVIC */
#define TIMx_IRQn                         TIM14_IRQn

/* Definition for TIMx remap */
#define TIMx_REMAP                        TIM_TIM14_RTC

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
