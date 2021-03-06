/**
  ******************************************************************************
  * @file    USART/HyperTerminal_Interrupt/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-May-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "main.h"
#include "Definizioni.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// extern #define RXBUFFERSIZE 

/* Private macro -------------------------------------------------------------*/

// External variables
extern uint16_t 	RxChar;
extern uint16_t 	RxChar_USART2;
extern uint16_t		TLampeggio;

extern uint16_t 	RxCount;
extern uint8_t 		RxBuffer[];

extern uint8_t LBflash;
extern uint8_t LGflash;

/* Private variables ---------------------------------------------------------
uint8_t TxBuffer[] = "by E.M. ";
uint8_t RxBuffer[RXBUFFERSIZE];
uint8_t NbrOfDataToTransfer = TXBUFFERSIZE;
uint8_t NbrOfDataToRead = RXBUFFERSIZE;
__IO uint8_t TxCount = 0; 
__IO uint16_t RxCount = 0; 
*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern void TimingDelay_Decrement(void);

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	TimingDelay_Decrement();	// Used by Delay(nnnn);
	
	// Routine for Flasing LED ****************************************************
	TLampeggio++;
	if (TLampeggio >= rifTLampeggio)
	{		
		if (LBflash==1) // Toggle  BLUE	LED
			GPIOC->ODR ^= GPIO_Pin_8;	
		if (LGflash==1) // Toggle  GREEN	LED
			GPIOC->ODR ^= GPIO_Pin_9;
		
		TLampeggio=0;
	}
		
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @brief  This function handles USART2 global interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {
		
		// RX data
		RxChar_USART2 = USART_ReceiveData( USART2 ); 
		RxBuffer[RxCount++] = RxChar_USART2;
		if (RxCount == RXBUFFERSIZE)
			RxCount=0;
  }

  if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
  {   
    /* Write one byte to the transmit data register 
    USART_SendData(USART2, TxBuffer[TxCount++]);

    
		if(TxCount == NbrOfDataToTransfer)
    {
      // Disable the EVAL_COM1 Transmit interrupt 
      USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    }
		*/
  }
}



/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
  if(USART_GetITStatus(EVAL_COM1, USART_IT_RXNE) != RESET)
  {
		
		// RX data
		RxChar = USART_ReceiveData( USART1 );  
		
  }

  if(USART_GetITStatus(EVAL_COM1, USART_IT_TXE) != RESET)
  {   
    /* Write one byte to the transmit data register 
    USART_SendData(EVAL_COM1, TxBuffer[TxCount++]);

    
		if(TxCount == NbrOfDataToTransfer)
    {
      // Disable the EVAL_COM1 Transmit interrupt 
      USART_ITConfig(EVAL_COM1, USART_IT_TXE, DISABLE);
    }
		*/
  }
}



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
