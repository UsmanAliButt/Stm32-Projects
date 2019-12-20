/**
  @page Data_Exchange CEC Data Exchange using Interrupt example
  
  @verbatim
  ******************** (C) COPYRIGHT 2012 STMicroelectronics *******************
  * @file    CEC/Data_Exchange/readme.txt 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-May-2012
  * @brief   Description of the CEC Data Exchange using Interrupt example.
  ******************************************************************************
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
   @endverbatim

@par Example Description 

This example provides a basic communication between two HDMI-CEC devices using 
interrupts. 
The first and second CEC device send a command to the CEC other device. 
The data received by the first and second CEC device are stored respectively in 
ReceiveBuffer. In both boards, the data transfers is managed using CEC_IRQHandler
in stm32F0xx_it.c file.

The example illustrates the use of the CEC communication between two devices 
(2 x STM320518-EVAL boards).Each device can send a frame to the other device by 
pressing the Joystick button on the EVAL board. 

- Hardware Description

To use this example, you need to load it on two STM320518-EVAL boards (let's call them 
Device_1 and Device_2) then connect these two boards through CEC lines(PB10 or HDMI 
connectors) and GND.
In the firmware example, uncomment the dedicated line in the main.h file  to use
the CEC peripheral as STM32 device_1 or as STM32 device_2.

@verbatim
*------------------------------------------------------------------------------*
|           STM320518-EVAL                         STM320518-EVAL              |
|         Device Address :0x01                    Device Address :0x03         |
|         ____________________                   ____________________          |
|        |                    |                 |                    |         |
|        |                    |                 |                    |         | 
|        |     __________     |                 |     __________     |         |
|        |    |   CEC    |____|____CECLine______|____|   CEC    |    |         |
|        |    | Device_1 |    |                 |    | Device_2 |    |         |
|        |    |__________|    |                 |    |__________|    |         |
|        |                    |                 |                    |         |
|        |  O LD1             |                 |  O LD1             |         |
|        |  O LD2    Joystick |                 |  O LD2    Joystick |         |
|        |  O LD3        _    |                 |  O LD3        _    |         |
|        |  O LD4       |_|   |                 |  O LD4       |_|   |         |
|        |                    |                 |                    |         |
|        |             GND O--|-----------------|--O GND             |         |
|        |____________________|                 |____________________|         |
|                                                                              |
|                                                                              |
*------------------------------------------------------------------------------*
@endverbatim

- Software Description

At each joystick buttons press:
- The CEC device sends the specific command (Volume Up, Volume Down, OSD Name or
  CEC Version) to the other device and waits for the ACK command.

- The CEC device sends the defined NumberOfByte from TransmitBuffer to the other 
  CEC device which are received into ReceiveBuffer.

- The sender device can send a frame to the other device by pressing the JoyStick 
  button on the EVAL board. Sending status is signaled by lightening and though 
  as follow:
   - If Joystick RIGHT, LEFT, Up or Down is pressed and the data transmission is 
     succeeded ==> Green Led(LD1) is ON.
   - If Joystick RIGHT, LEFT, Up or Down is pressed and the data transmission is 
     failed ==> Red Led(LD3) is ON.    
 
- The Receiver device compares the NumberOfByte received data with the defined ones and 
  check the ACK command, Received data correctness is signaled by LED lightening 
  and though as follow:
   - OSD Name   : Joystick RIGHT and data correctly received ==> LD1, LD2 ON and LD3,LD4 OFF 
   - CEC Version: Joystick LEFT and data correctly received  ==> LD3, LD4 ON and LD1,LD2 OFF
   - Volume Up  : Joystick UP and data correctly received    ==> LD1, LD2, LD3 and LD4 are ON
   - Volume Down: Joystick DOWN and data correctly received  ==> LD1, LD2, LD3 and LD4 are OFF
   - If an error occurred during reception ==> Only LD3(Red LED) is ON.

@par Directory contents 

  - CEC/Data_Exchange/stm32f0xx_conf.h    Library Configuration file
  - CEC/Data_Exchange/stm32f0xx_it.c      Interrupt handlers
  - CEC/Data_Exchange/stm32f0xx_it.h      Interrupt handlers header file
  - CEC/Data_Exchange/main.c              Main program
  - CEC/Data_Exchange/main.h              Header for main.c module
  - CEC/Data_Exchange/system_stm32f0xx.c  STM32F0xx system source file
  
@note The "system_stm32f0xx.c" is generated by an automatic clock configuration 
      tool and can be easily customized to meet user application requirements. 
      To select different clock setup, use the "STM32F0xx_Clock_Configuration_VX.Y.Z.xls" 
      provided with the AN4055 package available on <a href="http://www.st.com/internet/mcu/class/1734.jsp">  ST Microcontrollers </a>
         
@par Hardware and Software environment

  - This example runs on STM32F0xx Devices.
  
  - This example has been tested with STMicroelectronics STM320518-EVAL (STM32F0xx)
    evaluation board and can be easily tailored to any other supported device 
    and development board.

  - STM320518-EVAL Set-up
     Connect the boards by using one of the two following alternatives:
    - A HDMI Cables between all boards HDMI-CEC connectors (CN3 or CN4) on 
      STM320518-EVAL . 
    - Use a simple wire between all devices CEC Lines (PB.10), in this case don't 
      forget to connect all boards grounds together.

@par How to use it ? 

In order to make the program work, you must do the following :
 - Copy all source files from this example folder to the template folder under
   Project\STM32F0xx_StdPeriph_Templates
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the example

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
