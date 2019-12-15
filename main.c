/**
  ******************************************************************************
  * @file    DAC_SignalsGeneration/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    23-March-2012
  * @brief   Main program body
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
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "stm32f0xx_lp_modes.h"
/** @addtogroup STM32F0_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup DAC_Signals_Generation
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DAC_DHR12R1_ADDRESS      0x40007408
#define DAC_DHR8R1_ADDRESS       0x40007410

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;  
DAC_InitTypeDef            DAC_InitStructure;
DMA_InitTypeDef            DMA_InitStructure;
NVIC_InitTypeDef  NVIC_InitStructure;
TIM_TimeBaseInitTypeDef    T3_TimeBStructure;
const uint16_t Sine12bit[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909, 
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};	  
const uint8_t Escalator8bit[6] = {0x0, 0x33, 0x66, 0x99, 0xCC, 0xFF};
const uint8_t Ttria[6]         = {0x00, 0x66, 0x33, 0xcc, 0x33, 0x66}; //0x00 can be added and tested at the end  
uint8_t Idx = 0;
__IO uint8_t  SelectedWavesForm = 1;
__IO uint8_t WaveChange = 1; 

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void DAC_Config(void);
void SleepMode(void);
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f0xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f0xx.c file
     */ 

  /* Preconfiguration before using DAC----------------------------------------*/
  //DAC_Config();
	
	__IO uint32_t index = 0,i=0;
    STM_EVAL_PBInit(BUTTON_USER,BUTTON_MODE_GPIO);
    
    /* Loop while User button is maintained pressed */
   // while(STM_EVAL_PBGetState(BUTTON_USER) != RESET)
   // {
   // }
    
    /* Enable PWR APB1 Clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    
    /* Allow access to Backup */
    PWR_BackupAccessCmd(ENABLE);
    
    /* Reset RTC Domain */
    RCC_BackupResetCmd(ENABLE);
    RCC_BackupResetCmd(DISABLE);

STM_EVAL_LEDInit(LED4);
	i=0;
	while(i!=4){
	STM_EVAL_LEDToggle(LED4);
	i++;
	for(index = 0; index < 0x7FFFF; index++);
	} 
  
	// TIM2 Periph clock enable 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
  // Time base configuration 
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Period = 0xFF;          
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;       
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // TIM2 TRGO selection 
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
  
  // TIM2 enable counter 
  TIM_Cmd(TIM2, ENABLE);
	
	
	
	//Timer3 configuration
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseStructInit(&T3_TimeBStructure); 
  T3_TimeBStructure.TIM_Period = 2000;          
  T3_TimeBStructure.TIM_Prescaler = 65535;       
  T3_TimeBStructure.TIM_ClockDivision = 0;    
  T3_TimeBStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM3, &T3_TimeBStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //Counter in Overflow mode
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	
  //SleepMode(); //Go to sleep - Wakeup on the TIM3 expiration 	

		
	DAC_Config();
	
	/* Configures Button GPIO and EXTI Line */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);	


  /* Infinite loop */
  while (1)
  {
    /* If the wave form is changed */
    if (WaveChange == 1)
    {  
      /* Switch the selected waves forms according the Button status */
      if (SelectedWavesForm == 1)
      { 
				  DAC_DeInit(); //Stop dac
          SleepMode(); 
				/* The sine wave has been selected */
          /* Sine Wave generator ---------------------------------------------*/
          /* DAC channel1 Configuration */
          DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;                //Timer2 external TRG0 as trigger for dac
          DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;       //Dac output buffer selected
          
          /* DMA channel3 Configuration */
          DMA_DeInit(DMA1_Channel3); 
          DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDRESS;    //DMA pick data from this address
          DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Sine12bit;
          DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
          DMA_InitStructure.DMA_BufferSize = 32;
          DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
          DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
          DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
          DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
          DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
          DMA_InitStructure.DMA_Priority = DMA_Priority_High;
          DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
          DMA_Init(DMA1_Channel3, &DMA_InitStructure);

          /* Enable DMA1 Channel3 */
          DMA_Cmd(DMA1_Channel3, ENABLE);

          /* DAC Channel1 Init */
          DAC_Init(DAC_Channel_1, &DAC_InitStructure);

          /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
             automatically connected to the DAC converter. */
          DAC_Cmd(DAC_Channel_1, ENABLE);

          /* Enable DMA for DAC Channel1 */
          DAC_DMACmd(DAC_Channel_1, ENABLE);
         
      }
			/*Custom wave selected- TriTriangle*/
			else if(SelectedWavesForm == 2) {
				    DAC_DeInit();   
				    SleepMode(); //Go to sleep - Wakeup on the TIM3 expiration 
				    DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
            DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
				 
						DMA_DeInit(DMA1_Channel3);
						DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR8R1_ADDRESS;
						DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Ttria;
						DMA_InitStructure.DMA_BufferSize = 6;
						DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
						DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
						DMA_Init(DMA1_Channel3, &DMA_InitStructure);
				  
						// Enable DMA1 Channel2 
						DMA_Cmd(DMA1_Channel3, ENABLE);
			
						// DAC channel1 Configuration 
						DAC_Init(DAC_Channel_1, &DAC_InitStructure);

						// Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
						// automatically connected to the DAC converter. 
						DAC_Cmd(DAC_Channel_1, ENABLE);

						// Enable DMA for DAC Channel1 
						DAC_DMACmd(DAC_Channel_1, ENABLE);				
			} 
          /* The Escalator wave has been selected */
       else
       {
         DAC_DeInit(); 
				 SleepMode(); //Go to sleep - Wakeup on the TIM3 expiration 
          // Escalator Wave generator -----------------------------------------
           
          // DAC channel1 Configuration 
          DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
          DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
          
  
          // DMA1 channel2 configuration 
          DMA_DeInit(DMA1_Channel3);

          DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR8R1_ADDRESS;
          DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Escalator8bit;
          DMA_InitStructure.DMA_BufferSize = 6;
          DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
          DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
          DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    
          // Enable DMA1 Channel2 
          DMA_Cmd(DMA1_Channel3, ENABLE);
    
          // DAC channel1 Configuration 
          DAC_Init(DAC_Channel_1, &DAC_InitStructure);

          // Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
          // automatically connected to the DAC converter. 
          DAC_Cmd(DAC_Channel_1, ENABLE);

          // Enable DMA for DAC Channel1 
          DAC_DMACmd(DAC_Channel_1, ENABLE);
					SelectedWavesForm = 0;
      }
      WaveChange = !WaveChange;
    }
  }
}


/**
  * @brief  PrecConfiguration: configure PA4 in analog,
  *                           enable DAC clock, enable DMA1 clock
  * @param  None
  * @retval None
  */
void DAC_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* DMA1 clock enable (to be used with DAC) */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  /* Configure PA.04 (DAC_OUT1) as analog */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void SleepMode(void){

//#if defined (SLEEP_MODE)	
#if defined (STOP_MODE)
// Sleep Mode Entry 
	//	- System Running at PLL (48MHz)
	//	- Flash 3 wait state
	//	- Prefetch and Cache enabled
	//	- Code running from Internal FLASH
	//	- All peripherals disabled.
	//	- Wakeup using EXTI Line (User Button PA.00)
	 
//Wake up from sleep mode by pressing the button 
//SleepMode_Measure();
	StopMode_Measure();
#endif

}
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
