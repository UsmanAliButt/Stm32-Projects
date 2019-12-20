/**

	By: www.emcu.it
	December 2012
	More info are here: www.emcu.it/STMWiFi/tutorial
	Description: Interface to STM WiFi module
	
	*** This SW is used for control the STM WiFi Module
	*
	* HW Connections:
	*	
	*		STM32F0-Discovery				STM WiFi Module (Dongle)
	*			3V	-----------------	J2 UART Pin_1
	*			GND ----------------- J2 UART Pin_2
	*     PA3 ----------------- J2 UART Pin_3
	*			PA2 ----------------- J2 UART Pin_4
	*
	*		On STM32F0-Discovery must be present all jumpers
	*		On STM WiFi Module (dongle) must be present only the jumper J10 (see below)
	*           
	*				o  o-o o J10
	*				o  o o o J9
	*      J4
	*
	* If you connect a led (see schematic below) from PC6 and GND, you have the possibility to monitor 
	*		the waiting from the answer from STM WiFi module.
	*                 __________
	*		PC6____|\|___| 330      |____GND
	*          |/|   |__________|
	*
	* For do the connection just press and release the blue button on the STM32F0-Discovery.
	* When the Led D5 is ON (Led D5 is on the STM WiFi module), the WiFi connection is active. 
	*		For do this it is necessary some seconds, during the set up and connection the Blue Led 
	*		on the STM32F0-Discovery is flashing. 
	* At this point it is also loaded on the STM WiFi the html page named: led.hmtl
	*		This page show the status of the LEDs mounted on the STM32F0-Discovery.
	*
	*	Now open the html page: cgi_demo.html
	*		this page is used to send commands to STM WiFi Module.
	*	For do this you must know the IP address of the STM WiFi module.
	*	To find it I suggest to use the Angry IP Scanner.
	*	Suppose the the STM WiFI IP is: 168.169.0.5
	*	Open your browser and type: 
	*	192.168.0.5/cgi_demo.html
	*	
	*	The custom commands (implemented on STM32F0-Discovery) for control the STM WiFi module are:
	*		lgon  – TurnON the green LED
	*		lgoff – TurnOFF the green LED
	*		lbon  – TurnON the blue LED
	*		lboff – TurnOFF the blue LED
	*		X     – Clear RxBuffer
	*		reset – reset the STM WiFi module, it reload the WiFi configuration received from STM32F0-Discovery 
	*
	* ATTENTION
	*
	* In the main.c there are the strings definitions used for connect the STM WiFi module to your WiFi Router.
	* Remember to configure the parameters in according to your WiFi network.
	* In particular, be sure to specify:
	* 
	* RouterName ______________________________________
	*                                                  |
	* 	uint8_t TxBuffer_RouterName[] = "at+s.ssidtxt=NETGEAR-3G\n\r";
	*
	* RouterPassword _______________________________________________
	*                                                               |   
	*		uint8_t TxBuffer_RouterPW[] = "at+s.scfg=wifi_wpa_psk_text,XYZ\n\r";
	* 
	***
	
	***
	*
	* COMMANDs available on STM32F0-Discovery are:
	*
	* 	BLed_OFF 		// Turn OFF the Blue Led
	* 	BLed_ON 		// Turn ON  the Blue Led
	* 	GLed_OFF 		// Turn OFF the Green Led
	* 	HLed_ON 		// Turn ON  the Green Led
	*
	* The variables below are used in stm32f0xx_it.c for flashing leds
	* 	LBflash=0;	// Led Blue  0==FlashOFF 1==FlashON 
	* 	LGflash=0;	// Led Green 0==FlashOFF 1==FlashON
	*
	* For remember the status of the leds use the variables below
	* 	LedG=0; 		// Led Greem 0==OFF 1==ON
	* 	LedB=0; 		// Led Blue  0==OFF 1==ON
	*
	***
	
	*** The USART1 and USART2 configuration is:
					USART_BaudRate 						= 115200;
					USART_WordLength 					= USART_WordLength_8b;
					USART_StopBits 						= USART_StopBits_1;
					USART_Parity 							= USART_Parity_No;
					USART_HardwareFlowControl = USART_HardwareFlowControl_None;
					USART_Mode 								= USART_Mode_Rx | USART_Mode_Tx;
	
	*** NOTE: the string RxBuffer contain the string received from USART2
	
  ******************************************************************************
  * @file    USART/HyperTerminal_Interrupt/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-May-2012
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
#include "main.h"
#include "eeprom.h"
#include "Definizioni.h"
#include <string.h>

/** @addtogroup STM32F0xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup HyperTerminal_Interrupt
  * @{
  */

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// Define the commands for the Red, Blue Led OFF and ON
#define BLed_OFF	GPIO_ResetBits(GPIOC, GPIO_Pin_8) // Blue LED OFF
#define BLed_ON		GPIO_SetBits(GPIOC, GPIO_Pin_8) 	// Blue LED ON
// Define the commands for the Green Led OFF and ON
#define GLed_OFF	GPIO_ResetBits(GPIOC, GPIO_Pin_9)	// Green LED OFF
#define GLed_ON		GPIO_SetBits(GPIOC, GPIO_Pin_9) 	// Green LED ON
// Define the commands for the Red Led OFF and ON
#define RLed_OFF	GPIO_ResetBits(GPIOC, GPIO_Pin_6)	// Red LED OFF
#define RLed_ON		GPIO_SetBits(GPIOC, GPIO_Pin_6) 	// Red LED ON

#define TXBUFFERSIZE   (countof(TxBuffer_AT) - 1)

// Commands available ******************************************************************
#define RxLBOFF  "lboff"	// Led Blue OFF
#define RxLBON   "lbon"		// Led Blue ON
#define RxLGOFF  "lgoff"	// Led Green OFF
#define RxLGON   "lgon"		// Led Green ON
#define RxClrBuf "X"			// Clear RxBuffer
#define RxReset  "reset"	// Reset the STM WiFi module, it reload the WiFi configuration
													//		received from STM32F0-Discovery.
													//		During the reset the Blue Led is flashing.

// Received strings used for test the status of STM WiFi
#define WiFi_IP		":WiFi Up:"  	// This means that STM WiFi is connected to WiFi Network
#define WiFi_OK		"OK"					// This is the answer at the AT command

/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Private variables ---------------------------------------------------------*/

uint8_t TxBuffer_AT[] = "at\n\r";
uint8_t TxBuffer_FAIL1[] = "+WIND:42:RX_MGMT:";
uint8_t TxBuffer_FAIL2[] = "+WIND:43:RX_DATA:";
uint8_t TxBuffer_FAIL3[] = "+WIND:44:RX_UNK:";
uint8_t TxBuffer_FAIL4[] = "+WIND:34:WiFi Unhandled Event:";
uint8_t TxBuffer_FAIL5[] = "ERROR:";

uint8_t TxBuffer_RouterName[] = "at+s.ssidtxt=pakistan\n\r"; 										// BBMHem
uint8_t TxBuffer_RouterPW[] = "at+s.scfg=wifi_wpa_psk_text,usmanali\n\r";	// enrico321
uint8_t TxBuffer_RouterPotectionMode[] = "at+s.scfg=wifi_priv_mode,2\n\r";			// 2
uint8_t TxBuffer_RouterRadioInSTAMode[] = "at+s.scfg=wifi_mode,1\n\r"; 					// 1
uint8_t TxBuffer_RouterDHCPclient[] = "at+s.scfg=ip_use_dhcp,1\n\r";
uint8_t TxBuffer_RouterSaveSettings[] = "at&w\n\r";
uint8_t TxBuffer_RouterSoftReset[] = "at+cfun=1\r\n";

// HTML Pages
uint8_t TxBuffer_Delete_led_page[] = "at+s.fsd=/led.html\n\r"; // Delete led.html page
uint8_t TxBuffer_Prepare_led_page[] = "at+s.fsc=/led.html,192\n\r";
uint8_t TxBuffer_Prepare_led_page_upload[] = "at+s.fsa=/led.html,192\n\r";
uint8_t TxBuffer_led_pageLVoffLBoff[] = "<html><head><meta content='\22'text/html; charset=ISO-8859-1\0x22 http-equiv=\0x22content-type\0x22><title>Leds.html</title></head><body> <h1>USMAN ALI</h1><br>Green_Led is OFF <br>Blue_Led is OFF<br></body></html>\r\n";
uint8_t TxBuffer_led_pageLVonLBoff[] = "<html><head><meta content='\22'text/html; charset=ISO-8859-1\0x22 http-equiv=\0x22content-type\0x22><title>Leds.html</title></head><body> <h1>USMAN ALI</h1><br>Green_Led is ON  <br>Blue_Led is OFF<br></body></html>\r\n";
uint8_t TxBuffer_led_pageLVonLBon[] = "<html><head><meta content='\22'text/html; charset=ISO-8859-1\0x22 http-equiv=\0x22content-type\0x22><title>Leds.html</title></head><body> <h1>USMAN ALI</h1><br>Green_Led is ON  <br>Blue_Led is ON <br></body></html>\r\n";
uint8_t TxBuffer_led_pageLVoffLBon[] = "<html><head><meta content='\22'text/html; charset=ISO-8859-1\0x22 http-equiv=\0x22content-type\0x22><title>Leds.html</title></head><body> <h1>USMAN ALI</h1><br>Green_Led is OFF <br>Blue_Led is ON <br></body></html>\r\n";

// Initialize the Leds status to OFF
uint8_t LedG=0; 		// Led Greem 0==OFF 1==ON
uint8_t LedB=0; 		// Led Blue  0==OFF 1==ON
// Initialize the Leds flashing to OFF
uint8_t LBflash=0;	// Led Blue  0==FlashOFF 1==FlashON 
uint8_t LGflash=0;	// Led Green 0==FlashOFF 1==FlashON 


uint8_t RxBuffer[RXBUFFERSIZE];
// uint8_t NbrOfDataToTransfer = TXBUFFERSIZE;
uint16_t NbrOfDataToRead = RXBUFFERSIZE;
__IO uint8_t TxCount = 0; 
__IO uint16_t RxCount = 0; 

uint16_t LungBuff=0;
uint16_t Cont=0;
size_t   LungStringa=0;
uint16_t Val = 0;
	
/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[NB_OF_VAR] = {0x5555, 0x6666, 0x7777};
uint16_t VarDataTab[NB_OF_VAR] = {0, 0, 0};
uint16_t VarValue = 0;
uint16_t NumPressBott=0x30;


static __IO uint32_t TimingDelay;
USART_InitTypeDef USART_InitStructure;
// extern uint8_t NbrOfDataToTransfer;
extern uint16_t NbrOfDataToRead;
extern __IO uint8_t TxCount; 
extern __IO uint16_t RxCount; 
uint8_t Tasto=0;
uint8_t MemTasto=0;
uint16_t RxChar=0;
uint16_t RxChar_USART2=0;
uint16_t TLampeggio=0;


/* Private function prototypes -----------------------------------------------*/
void NVIC_Config(void);
void PA0_InFloating(void);
void PC6PC8andPC9output(void);
void Delay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);
void STM_EVAL_COM_2_Init(USART_InitTypeDef* USART_InitStruct);

uint8_t BuffOneCH(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
uint8_t Search_B2inB1(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t Buffer1Length, uint16_t Buffer2Length);
static void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLength);
// uint8_t SearchBuffer2inBuffer1(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t Buffer1Length, uint16_t Buffer2Length);

uint8_t ConfigureWiFi(void);	// it return PASS or FAIL
void TestRxCommand(void);
void LoadAppropite_LedPage(void);
void Clr_RxBuffer(void);

void ResetSTMWiFIModule(void);
void ResetSTMWiFIModule_retainsLEDs(void);

/* Private functions ---------------------------------------------------------*/




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

   /* Unlock the Flash Program Erase controller 
  FLASH_Unlock();
  EEPROM Init 
  EE_Init();		
	*/

	// Configure I/O for reading the BLUE button on STM32F0-Discovery
	PA0_InFloating();
	
	// Configure I/O for LEDs that are present on STM32F0-Discovery + PC6 for RED led
	PC6PC8andPC9output();
	
	/* Setup SysTick Timer for 1 msec interrupts. ***************************************************
	     ------------------------------------------
	    1. The SysTick_Config() function is a CMSIS function which configure:
	       - The SysTick Reload register with value passed as function parameter.
	       - Configure the SysTick IRQ priority to the lowest value (0x0F).
	       - Reset the SysTick Counter register.
	       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
	       - Enable the SysTick Interrupt.
	       - Start the SysTick Counter.

	    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
	       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
	       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
	       inside the misc.c file.

	    3. You can change the SysTick IRQ priority by calling the
	       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function
	       call. The NVIC_SetPriority() is defined inside the core_cm3.h file.

	    4. To adjust the SysTick time base, use the following formula:

	         Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)

	       - Reload Value is the parameter to be passed for SysTick_Config() function
	       - Reload Value should not exceed 0xFFFFFF
	*/
	SysTick_Config(SystemCoreClock / 1000); // 1mS
	
  
  /* NVIC configuration */
  NVIC_Config();
  
  /* USARTx configuration ------------------------------------------------------*/
  /* USARTx configured as follow:
  - BaudRate = 115200 baud  
  - Word Length = 8 Bits
  - One Stop Bit
  - NO parity
  - NO hardware flow control
  - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
	// Conf. USART1 as is COM1
  STM_EVAL_COMInit(COM1, &USART_InitStructure);
	
	// Conf. USART2 as is COM2
  STM_EVAL_COM_2_Init(&USART_InitStructure); 
  
  /* Enable the EVAL_COM1 Receive interrupt: this interrupt is generated when the 
  EVAL_COM1 receive data register is not empty */
  USART_ITConfig(EVAL_COM1, USART_IT_RXNE, ENABLE);
  
  /* Enable the EVAL_COM2 Receive interrupt: this interrupt is generated when the 
  EVAL_COM2 receive data register is not empty */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);


	Tasto=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
	MemTasto = Tasto;

	// Clear RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);

	// Initialize the LED variable
  LedG=0; 		// Led Greem 0==OFF
  LedB=0; 		// Led Blue  0==OFF
	// Initialize the Leds flashing to OFF
	LBflash=0; 	// Led Blue  0==FlashOFF
	LGflash=0;	// Led Green 0==FlashOFF


  /* Infinite loop */
  while (1)
  {
		// Check the BLUE button *********************************************************** 
		Tasto=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
		if (MemTasto != Tasto)
			{
			MemTasto = Tasto;					
			if (Tasto==0)
				{
				GLed_OFF;		// Set Led Green to OFF
				BLed_OFF;		// Set Led Blue to OFF
				LedG=0; 		// Status of Led Greem is 0==OFF
				LedB=0; 		// Status of Led Blue is 0==OFF
				LGflash=0;	// Set Led Green 0==FlashOFF
				LBflash=1; 	// Set Led Blue 1==FlashON		
								
				if (ConfigureWiFi() == PASS)	// Send AT command for configure the STM WiFi Module *******
					{
					LBflash=0; 	// Set Led Blue 0==FlashOFF
					BLed_OFF;		// Set Led Blue to OFF
					}
				else
					LGflash=1; 	// Set Led Green 0==FlashOFF				
				}
			}
	
		// Test if there are commands from the STM WiFi module *****************************
		TestRxCommand();
			
  }
	
}



//
// Test the commands received from STM WiFi module
//		Test the contents of RxBuffer
//
void TestRxCommand(void)
{
	// Test if received fromSTM WiFI: +WIND:42:RX_MGMT: - Unhandled Event: - From network means FAIL
	// if is so I reset the STM WiFI module
	if (Search_B2inB1(RxBuffer, TxBuffer_FAIL1, RXBUFFERSIZE, (countof(TxBuffer_FAIL1) - 1)) != FAIL)		
		{
		GLed_OFF;
		BLed_OFF;
		LBflash=1; // Led Blue   1==FlashON
		LGflash=1; // Led Green  1==FlashON
		ResetSTMWiFIModule_retainsLEDs();
		}
	// *******************************************************************************************	

	// Test if received fromSTM WiFI: +WIND:43:RX_DATA: - Unhandled Event: - From network means FAIL
	// if is so I reset the STM WiFI module
	if (Search_B2inB1(RxBuffer, TxBuffer_FAIL2, RXBUFFERSIZE, (countof(TxBuffer_FAIL2) - 1)) != FAIL)		
		{
		GLed_OFF;
		BLed_OFF;
		LBflash=1; // Led Blue   1==FlashON
		LGflash=1; // Led Green  1==FlashON
		ResetSTMWiFIModule_retainsLEDs();
		}
	// *******************************************************************************************		
	
	// Test if received fromSTM WiFI: +WIND:44:RX_UNK: - Unhandled Event: - From network means FAIL
	// if is so I reset the STM WiFI module
	if (Search_B2inB1(RxBuffer, TxBuffer_FAIL3, RXBUFFERSIZE, (countof(TxBuffer_FAIL3) - 1)) != FAIL)		
		{
		GLed_OFF;
		BLed_OFF;
		LBflash=1; // Led Blue   1==FlashON
		LGflash=1; // Led Green  1==FlashON
		ResetSTMWiFIModule_retainsLEDs();
		}
	// *******************************************************************************************

	// Test if received fromSTM WiFI: +WIND:34:WiFi - Unhandled Event: - From network means FAIL
	// if is so I reset the STM WiFI module
	if (Search_B2inB1(RxBuffer, TxBuffer_FAIL4, RXBUFFERSIZE, (countof(TxBuffer_FAIL4) - 1)) != FAIL)		
		{
		GLed_OFF;
		BLed_OFF;
		LBflash=1; // Led Blue   1==FlashON
		LGflash=1; // Led Green  1==FlashON
		ResetSTMWiFIModule_retainsLEDs();
		}
	// *******************************************************************************************		
		
	// Test if received fromSTM WiFI: ERROR: - From network means FAIL
	// if is so I reset the STM WiFI module
	if (Search_B2inB1(RxBuffer, TxBuffer_FAIL5, RXBUFFERSIZE, (countof(TxBuffer_FAIL5) - 1)) != FAIL)		
		{
		GLed_OFF;
		BLed_OFF;
		LBflash=1; // Led Blue   1==FlashON
		LGflash=1; // Led Green  1==FlashON
		ResetSTMWiFIModule_retainsLEDs();
		}
	// *******************************************************************************************			
		
		
	// Test command: X - Clear RxBuffer **********************************************************
	if (BuffOneCH(RxBuffer, RxClrBuf, RXBUFFERSIZE) != FAIL)
		{
		Clr_RxBuffer(); // Clear the RxBuffer 
		Delay(1000); 		// Dly 1sec
		Clr_RxBuffer(); // Clear the RxBuffer			
		}
	// *******************************************************************************************
		
		
	// *******************************************************************************************	
	// Test command: reset - reset the STM WiFi module, 
	//							 STM WiFi reload the WiFi configuration received from STM32F0-Discovery 
	if (Search_B2inB1(RxBuffer, RxReset, RXBUFFERSIZE, (countof(RxReset) - 1)) != FAIL)		
		{
		GLed_OFF;
		LedG=0;
		BLed_OFF;
		LedB=0;
		LBflash=1; // Led Blue  1==FlashON
		ConfigureWiFi();
		// ResetSTMWiFIModule();
		}
	// *******************************************************************************************


	// ***************** Test lgon lgoff *********************************************************
	//
	// Test command: lgon - Green LED ON
	if (Search_B2inB1(RxBuffer, RxLGON, RXBUFFERSIZE, (countof(RxLGON) - 1)) != FAIL)
		{
		GLed_ON;
		LedG=1;
		Clr_RxBuffer(); // Clear the RxBuffer 
		LoadAppropite_LedPage();
		Delay(1000); 		// Dly 1sec
		Clr_RxBuffer(); // Clear the RxBuffer
		}		
	// Test command: lgoff - Green LED OFF
	if (Search_B2inB1(RxBuffer, RxLGOFF, RXBUFFERSIZE, (countof(RxLGOFF) - 1)) != FAIL)
		{
		GLed_OFF;
		LedG=0;
		Clr_RxBuffer(); // Clear the RxBuffer 
		LoadAppropite_LedPage();
		Delay(1000); 		// Dly 1sec
		Clr_RxBuffer(); // Clear the RxBuffer			
		}	
	// *******************************************************************************************
		
		
	// ***************** Test lbon lboff *********************************************************
	//
	// Test command: lbon - Blue LED ON
	if (Search_B2inB1(RxBuffer, RxLBON, RXBUFFERSIZE, (countof(RxLBON) - 1)) != FAIL)	
		{
		BLed_ON;
		LedB=1;
		Clr_RxBuffer(); // Clear the RxBuffer 
		LoadAppropite_LedPage();
		Delay(1000); 		// Dly 1sec
		Clr_RxBuffer(); // Clear the RxBuffer
		}		
	// Test command: lboff - Blue LED OFF
	if (Search_B2inB1(RxBuffer, RxLBOFF, RXBUFFERSIZE, (countof(RxLBOFF) - 1)) != FAIL)		
		{
		BLed_OFF;
		LedB=0;	
		Clr_RxBuffer(); // Clear the RxBuffer 
		LoadAppropite_LedPage();
		Delay(1000); 		// Dly 1sec
		Clr_RxBuffer(); // Clear the RxBuffer			
		}	
	// *******************************************************************************************	
}



// 
// Reset the STM WiFi Module
//
void ResetSTMWiFIModule(void)
{
	// Send Router Soft Reset *********************************
	LungBuff=(countof(TxBuffer_RouterSoftReset) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterSoftReset[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}	
	Delay(1000); 		// Dly 1sec
	Clr_RxBuffer(); // Clear the RxBuffer
		
	// Start LEDs flashing
	LBflash=0;		
	BLed_OFF;
	LGflash=1; // Green Led flasshing	
	LBflash=1; // Blue Led flasshing
	// test the WiFi Connection -> :WiFi Up:
	//                                |
	while ( Search_B2inB1(RxBuffer, WiFi_IP, RXBUFFERSIZE, (countof(WiFi_IP) - 1)) == FAIL ) 
		{
		Delay(1000);	
		}
	// Stop LEDs flashing
	LBflash=0;		
	BLed_OFF;
	LGflash=0;		
	GLed_OFF;
			
	Clr_RxBuffer(); // Clear the RxBuffer 
	LoadAppropite_LedPage();
	Delay(1000); 		// Dly 1sec
	Clr_RxBuffer(); // Clear the RxBuffer
			
	LBflash=0; // Led Blue  0==FlashOFF
	BLed_OFF;		
}


// 
// Reset the STM WiFi Module but retains the status of the LEDs
//
void ResetSTMWiFIModule_retainsLEDs(void)
{
	uint8_t LedG_Mem = LedG; 	// Memorize the status of Green Led
	uint8_t LedB_Mem = LedB; 	// Memorize the status of Blue Led
	
	ConfigureWiFi();
			
	Delay(2000); 		// Dly 2sec
	Clr_RxBuffer(); // Clear the RxBuffer			
	LBflash=0; 			// Led Blue  0==FlashOFF
	BLed_OFF;
	LedG = LedG_Mem; 	// Restore the status of Green Led
	LedB = LedB_Mem; 	// Restore the status of Blue Led		
	LoadAppropite_LedPage();
	Delay(1000); 		// Dly 1sec
	Clr_RxBuffer(); // Clear the RxBuffer
		
	LGflash=1;			// Green LED flashing - ATTENTION: In the final application, this line, should be REMOVED.
	
}



//
// Clear RxBuffer
//
void Clr_RxBuffer(void)
{
		Fill_Buffer(RxBuffer, RXBUFFERSIZE);
		RxCount=0;
}


//
// Configure the WiFi module
//
uint8_t ConfigureWiFi(void)
{
	
	// Send Router Name **************************************************************************
	LungBuff=(countof(TxBuffer_RouterName) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterName[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************

		
	// Send Router Password **********************************************************************
	LungBuff=(countof(TxBuffer_RouterPW) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterPW[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************
		
	
	// Send Router Potection Mode ****************************************************************
	LungBuff=(countof(TxBuffer_RouterPotectionMode) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterPotectionMode[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************
	
		
	// Send Router Radio in STA Mode *************************************************************
	LungBuff=(countof(TxBuffer_RouterRadioInSTAMode) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterRadioInSTAMode[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}					
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************
	
		
	// Send Router DHCP Client *******************************************************************
	LungBuff=(countof(TxBuffer_RouterDHCPclient) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterDHCPclient[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}	
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;	
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************
	
		
	// Send Router Save Settings *****************************************************************
	LungBuff=(countof(TxBuffer_RouterSaveSettings) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterSaveSettings[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}					
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************
	

	// Send Router Soft Reset ********************************************************************
	// Start LEDs flashing
	LBflash=0;		
	BLed_OFF;
	LGflash=1; // Green Led flasshing	
	LBflash=1; // Blue Led flasshing
	// Send Router Soft Reset *****************************************
	LungBuff=(countof(TxBuffer_RouterSoftReset) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_RouterSoftReset[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}	
	Delay(1000);
	/* Clear the RxBuffer */
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);
	RxCount=0;
		
	// test the WiFi Connection -> :WiFi Up:  *************************
	//                                |
	while ( Search_B2inB1(RxBuffer, WiFi_IP, RXBUFFERSIZE, (countof(WiFi_IP) - 1)) == FAIL ) 
		{
		Delay(1000);	
		}
	LGflash=0; // Green Led NON flasshing	
	GLed_OFF;	
		
	Delay(1000);
	/* Clear the RxBuffer */
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);
	RxCount=0; // ********************************************************************************
	
		
	// LED.HTML page to load on STM WiFi *********************************************************	
		
	// Prepare the HTML page named LED.HTML ******************************************************
	LungBuff=(countof(TxBuffer_Prepare_led_page) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_Prepare_led_page[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************
	
		
	// Prepare to UpLoad the LED.HTML page *******************************************************	
	LungBuff=(countof(TxBuffer_Prepare_led_page_upload) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_Prepare_led_page_upload[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}
	// *******************************************************************************************
		
		
	// UpLoad the LED.HTML page ******************************************************************	
	LungBuff=(countof(TxBuffer_led_pageLVoffLBoff) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_led_pageLVoffLBoff[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}					
	// Test the OK answer ******************************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // ********************************************************************************
	
	/* Clear the RxBuffer */
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);
	RxCount=0;
	
	LGflash=0; // Green Led flasshing OFF	
	LBflash=0; // Blue Led flasshing OFF
	BLed_OFF;
	GLed_OFF;
		
	return PASS;
}

// *******************************************************************************************
//
// Load the appropriate led.html page on STM WiFi in according to the value of
//		LedG and LedB
//
void LoadAppropite_LedPage(void)
{

	// Delete the led.html page ********************************************
	LungBuff=(countof(TxBuffer_Delete_led_page) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_Delete_led_page[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}	
	Delay(1000); // Dly 1sec
	/* Clear the RxBuffer */
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);
	RxCount=0;
	// *********************************************************************
		
		
	// Prepare a blank led.html page ***************************************
	LungBuff=(countof(TxBuffer_Prepare_led_page) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_Prepare_led_page[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}
	// Test the OK answer **************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;		
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // **********************************************************	
	
		
		
	// Prepare to upload the led.html page *********************************
	LungBuff=(countof(TxBuffer_Prepare_led_page_upload) - 1);
	for (Cont=0; Cont<=LungBuff; Cont++)
		{
		USART_SendData(USART2, TxBuffer_Prepare_led_page_upload[Cont]);
		// The software must wait the end of transmission
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			{}	
		}
	// *********************************************************************
		
	
	// *********************************************************************
	// Test the value of LedG and LedB for upload the appropriate led.html page
	if (LedG==1 & LedB==0)
		{
		LungBuff=(countof(TxBuffer_led_pageLVonLBoff) - 1);
		for (Cont=0; Cont<=LungBuff; Cont++)
			{
			USART_SendData(USART2, TxBuffer_led_pageLVonLBoff[Cont]);
			// The software must wait the end of transmission
			while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
				{}	
			}			
		}	
	if (LedG==1 & LedB==1)
		{
		LungBuff=(countof(TxBuffer_led_pageLVonLBon) - 1);
		for (Cont=0; Cont<=LungBuff; Cont++)
			{
			USART_SendData(USART2, TxBuffer_led_pageLVonLBon[Cont]);
			// The software must wait the end of transmission
			while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
				{}	
			}			
		}		
	if (LedG==0 & LedB==1)
		{
		LungBuff=(countof(TxBuffer_led_pageLVoffLBon) - 1);
		for (Cont=0; Cont<=LungBuff; Cont++)
			{
			USART_SendData(USART2, TxBuffer_led_pageLVoffLBon[Cont]);
			// The software must wait the end of transmission
			while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
				{}	
			}			
		}			
	if (LedG==0 & LedB==0)
		{
		LungBuff=(countof(TxBuffer_led_pageLVoffLBoff) - 1);
		for (Cont=0; Cont<=LungBuff; Cont++)
			{
			USART_SendData(USART2, TxBuffer_led_pageLVoffLBoff[Cont]);
			// The software must wait the end of transmission
			while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
				{}	
			}			
		}	
	// Test the OK answer **************************************************
	RLed_ON;
	while ( Search_B2inB1(RxBuffer, WiFi_OK, RXBUFFERSIZE, (countof(WiFi_OK) - 1)) == FAIL ) 
		{
		Delay(1000);
		}		
	RLed_OFF;	
	Delay(DlyBeforeClrRxBuffer); 					// Dly before clear the RxBuffer
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);	// Clear the RxBuffer 
	RxCount=0; // **********************************************************	

		
	Delay(1000); // Dly 1sec
	/* Clear the RxBuffer */
	Fill_Buffer(RxBuffer, RXBUFFERSIZE);
	RxCount=0;
}
// *******************************************************************************************



/**
  * @brief  Fills buffer.
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferLength: size of the buffer to fill
  * @retval None
  */
static void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLength)
{
  uint16_t index = 0;
  
  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++ )
  {
    pBuffer[index] = 0x00;
  }
}


/*
//
// Search pBguffer2 in pBuffer1
//
// pBuffer1 is RxBuffer
// pBuffer2 is string to search in pBuffer1
// Buffer1Length is the length of pBuffer1
// Bugger2Length is the length of pBuffer2
//
uint8_t SearchBuffer2inBuffer1(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t Buffer1Length, uint16_t Buffer2Length)
{
uint16_t ValComp=0;

  while (Buffer1Length--)
  {
    if (*pBuffer1 == *pBuffer2)
    {
      ValComp++; 
			pBuffer1++;
      pBuffer2++;
			if (ValComp == Buffer2Length)
				return PASS;
    }
		else
		{	
			pBuffer1++;
			if (ValComp > 0 )
				return FAIL;
		}
  }
  return FAIL; 
}

*/



uint8_t Search_B2inB1(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t Buffer1Length, uint16_t Buffer2Length)
{
uint16_t ValComp = 0;
uint8_t* Mem_pBuffer2 = pBuffer2;

  while (Buffer1Length--)
  {
    if (*pBuffer1 == *pBuffer2)
    {
      ValComp++; 
			pBuffer1++;
      pBuffer2++;
			if (ValComp == Buffer2Length)
				return PASS;
    }
		else
		{
			ValComp=0;
			pBuffer2=Mem_pBuffer2;
			pBuffer1++;
		}
  }
  return FAIL; 
}







/**
  * @brief  Search, character -> Buffer2, in the Buffer1
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer1 contain pBuffer2
  *         FAILED: pBuffer1 differs from pBuffer2
	*
	* ATTENTION Buffer2 must contain only one character
	*
  */
uint8_t BuffOneCH(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 == *pBuffer2)
    {
      return PASS; 
    }
    pBuffer1++;
    // pBuffer2++;
  }
  
  return FAIL; 
}


/**
  * @brief  Configures COM2 port.
  */
void STM_EVAL_COM_2_Init(USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* Enable USART2 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 

  /* Connect PA2 to USART2_Tx **********************************/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);

  /* Connect PA3 to USART2_Rx **********************************/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);
  
  /* Configure USART2 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  /* Configure USART2 Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* USART2 configuration */
  USART_Init(USART2, USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(USART2, ENABLE);
}



/**
  * @brief  PA0 InPut + Floating (BLUE Key on STM32F0-Discovery)
  * @param  None
  * @retval None
  */
void PA0_InFloating(void)
{
	GPIO_InitTypeDef        GPIO_InitStructure;
	/* GPIOC Periph clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}



/**
  * @brief  Inizialize PC8 and PC9 in OutPut PushPull (LEDs) + PC6 for RED led
  * @param  None
  * @retval None
  */
void PC6PC8andPC9output(void)
{
	GPIO_InitTypeDef        GPIO_InitStructure;
	/* GPIOC Periph clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	/* Configure PC8 and PC9 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EVAL_COM1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	// Enable USART2 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
}



/****************************************************************************************************
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
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
