//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - UART Demo
// Application Overview - The objective of this application is to showcase the 
//                        use of UART. The use case includes getting input from 
//                        the user and display information on the terminal. This 
//                        example take a string as input and display the same 
//                        when enter is received.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup uart_demo
//! @{
//
//*****************************************************************************

#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "uart.h"
#include "pin_mux_config.h"
#include "gpio.h"
#include "utils.h"

// Common interface include
#include "uart_if.h"
#include "gpio_if.h" // For lighting the LED

//*****************************************************************************
//                          MACROS                                  
//*****************************************************************************
#define APPLICATION_VERSION  "1.4.0"
#define APP_NAME             "UART Echo"
#define CONSOLE              UARTA0_BASE
#define UartGetChar()        MAP_UARTCharGet(CONSOLE)
#define UartPutChar(c)       MAP_UARTCharPut(CONSOLE,c)
#define MAX_STRING_LENGTH    80


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile int g_iCounter = 0;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//*****************************************************************************
//                      LOCAL DEFINITION                                   
//*****************************************************************************

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t        CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! Turn the LED corresponding to the binary representation of the input number ON.
//! The LED display range goes from 000 to 111
//!
//! \param lednum is the 3-bits input number that the LED represents
//!
//! \return None
//
//*****************************************************************************
void num2LED(int lednum) {
    switch (lednum) {
        case 0: {
            GPIO_IF_LedOff(MCU_ALL_LED_IND);
            break;
        }
        case 1: {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            break;
        }
        case 2: {
            GPIO_IF_LedOff(MCU_RED_LED_GPIO);
            GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
            break;
        }
        case 3: {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            break;
        }
        case 4: {
            GPIO_IF_LedOff(MCU_RED_LED_GPIO);
            GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
            GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
            break;
        }
        case 5: {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            break;
        }
        case 6: {
            GPIO_IF_LedOff(MCU_RED_LED_GPIO);
            GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
            break;
        }
        case 7: {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            break;
        }
        default: {
            GPIO_IF_LedOff(MCU_ALL_LED_IND);
        }
    }
}


//*****************************************************************************
//
//! Main function constantly polls SW2 and SW3. When SW3 is pressed, a binary
//! counting sequence from 000 to 111 occurs on the LED. Pin 18 is also set to LOW.
//! The message "SW3 pressed" is printed to the terminal once.
//! On the other hand, when SW2 is pressed, the LED turn ON and OFF in unison,
//! and Pin 18 is set to HIGH. The message "SW3 pressed" is printed to the terminal once.
//!
//! \param  None
//!
//! \return None
//! 
//*****************************************************************************
void main()
{
    //char cString[MAX_STRING_LENGTH+1];
//    char cCharacter;
//    int iStringLength = 0;
    //
    // Initailizing the board
    //
    BoardInit();
    //
    // Muxing for Enabling UART_TX and UART_RX.
    //
    PinMuxConfig();
    //
    // Initializing the LED
    //
    GPIO_IF_LedConfigure(LED1|LED2|LED3);
    GPIO_IF_LedOff(MCU_ALL_LED_IND);
    //
    // Initializing the Terminal.
    //
    InitTerm();
    //
    // Clearing the Terminal.
    //
    ClearTerm();
    DisplayBanner(APP_NAME);
    Message("\t\t****************************************************\n\r");
    Message("\t\t\t        CC3200 UART Echo Usage        \n\r");
    Message("\t\t Type in a string of alphanumeric characters and  \n\r");
    Message("\t\t pressenter, the string will be echoed. \n\r") ;
    Message("\t\t Note: if string length reaches 80 character it will \n\r");
    Message("\t\t echo the string without waiting for enter command \n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");
    int SW3Flag = 1, SW2Flag = 1;

    while(1)
    {
        //
        // When SW3 (GPIOPort13, Pin 4) s pressed, start a binary counting sequence
        // print "SW3 pressed" to the terminal
        // set GPIO_28 (P18) Low
        //
        GPIO_IF_LedOff(MCU_ALL_LED_IND);
        int count = 0;
        while (GPIOPinRead(GPIOA1_BASE, 0x20) & 0x20) {
            GPIOPinWrite(GPIOA3_BASE, 0x10, 0);
            if (SW3Flag) {
                Message("SW3 pressed\n");
                SW3Flag = 0;
            }
            if (count == 8) count = 0;
            SW2Flag = 1;
            // LED of the corresponding number light up
            num2LED(count);
            count++;
            MAP_UtilsDelay(8000000);

        }

        //
        // If SW2 (GPIOPort22, Pin 15) is pressed, blink the LEDs ON and OFF in unison
        // print "SW2 pressed" to the terminal
        // set GPIO_28 (P18) High
        //
        GPIO_IF_LedOff(MCU_ALL_LED_IND);
        while (GPIOPinRead(GPIOA2_BASE, 0x40) & 0x40) {
            GPIOPinWrite(GPIOA3_BASE, 0x10, 255);
            if (SW2Flag) {
                Message("SW2 pressed\n");
                SW2Flag = 0;
            }
            SW3Flag = 1;
            GPIO_IF_LedToggle(MCU_RED_LED_GPIO);
            GPIO_IF_LedToggle(MCU_ORANGE_LED_GPIO);
            GPIO_IF_LedToggle(MCU_GREEN_LED_GPIO);
            MAP_UtilsDelay(8000000);
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

    

