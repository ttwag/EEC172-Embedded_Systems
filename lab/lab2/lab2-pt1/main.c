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
// Application Name     - SPI Demo
// Application Overview - The demo application focuses on showing the required 
//                        initialization sequence to enable the CC3200 SPI 
//                        module in full duplex 4-wire master and slave mode(s).
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup SPI_Demo
//! @{
//
//*****************************************************************************

// Standard includes
#include <string.h>
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "gpio.h"
#include "gpio_if.h"
#include "uart.h"
#include "interrupt.h"

// Common interface includes
#include "uart_if.h"
#include "pin_mux_config.h"

// OLED
#include "./oled/Adafruit_SSD1351.h"
#include "./oled/Adafruit_GFX.h"
#include "./oled/oled_test.h"
#include "./oled/glcdfont.h"


#define APPLICATION_VERSION     "1.4.0"
//*****************************************************************************
//
// Application Master/Slave mode selector macro
//
// MASTER_MODE = 1 : Application in master mode
// MASTER_MODE = 0 : Application in slave mode
//
//*****************************************************************************
#define MASTER_MODE      0

#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100

#define MASTER_MSG       "This is CC3200 SPI Master Application\n\r"
#define SLAVE_MSG        "This is CC3200 SPI Slave Application\n\r"


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
//
//! Print the string, "Hello World!" onto the Adafruit OlED Display
//
//*****************************************************************************

void print_hello_world(void) {
    fillScreen(BLACK);
    drawChar(0, HEIGHT/5, 'H', WHITE, WHITE, 5);
    drawChar(30, HEIGHT/5, 'e', WHITE, WHITE, 5);
    drawChar(55, HEIGHT/5, 'l', WHITE, WHITE, 5);
    drawChar(75, HEIGHT/5, 'l', WHITE, WHITE, 5);
    drawChar(100, HEIGHT/5, 'o', WHITE, WHITE, 5);

    drawChar(0, HEIGHT*3/5, 'W', WHITE, WHITE, 5);
    drawChar(30, HEIGHT*3/5, 'o', WHITE, WHITE, 5);
    drawChar(58, HEIGHT*3/5, 'r', WHITE, WHITE, 5);
    drawChar(75, HEIGHT*3/5, 'l', WHITE, WHITE, 5);
    drawChar(95, HEIGHT*3/5, 'd', WHITE, WHITE, 5);
    drawChar(113, HEIGHT*3/5, '!', WHITE, WHITE, 5);
}

//*****************************************************************************
//
//! Print the full character set from the  glcdfont.h onto the center of
//! Adafruit OlED Display
//
//*****************************************************************************

void print_font(void) {
    fillScreen(BLACK);
    unsigned char myFont = 0x00;
    int i;
    for (i = 0; i < 256; i++) {
        drawChar(WIDTH/2, HEIGHT/2, myFont, WHITE, WHITE, 5);
        MAP_UtilsDelay(8000000);
        drawChar(WIDTH/2, HEIGHT/2, myFont, BLACK, BLACK, 5);
        myFont += 0x01;
    }
}

//*****************************************************************************
//
//! Print 8 horizontal lines onto the Adafruit OlED Display
//
//*****************************************************************************

void printHzLine(void) {
    fillScreen(BLACK);
    drawLine(0, HEIGHT/9, WIDTH, HEIGHT/9, BLACK);
    drawLine(0, HEIGHT*2/9, WIDTH, HEIGHT*2/9, BLUE);
    drawLine(0, HEIGHT*3/9, WIDTH, HEIGHT*3/9, GREEN);
    drawLine(0, HEIGHT*4/9, WIDTH, HEIGHT*4/9, CYAN);
    drawLine(0, HEIGHT*5/9, WIDTH, HEIGHT*5/9, RED);
    drawLine(0, HEIGHT*6/9, WIDTH, HEIGHT*6/9, MAGENTA);
    drawLine(0, HEIGHT*7/9, WIDTH, HEIGHT*7/9, YELLOW);
    drawLine(0, HEIGHT*8/9, WIDTH, HEIGHT*8/9, WHITE);
}

//*****************************************************************************
//
//! Print 8 vertical lines onto the Adafruit OlED Display
//
//*****************************************************************************

void printVtLine(void) {
    fillScreen(BLACK);
    drawLine(WIDTH/9, 0 ,WIDTH/9, HEIGHT, BLACK);
    drawLine(WIDTH*2/9, 0 ,WIDTH*2/9, HEIGHT, BLUE);
    drawLine(WIDTH*3/9, 0 ,WIDTH*3/9, HEIGHT, GREEN);
    drawLine(WIDTH*4/9, 0 ,WIDTH*4/9, HEIGHT, CYAN);
    drawLine(WIDTH*5/9, 0 ,WIDTH*5/9, HEIGHT, RED);
    drawLine(WIDTH*6/9, 0 ,WIDTH*6/9, HEIGHT, MAGENTA);
    drawLine(WIDTH*7/9, 0 ,WIDTH*7/9, HEIGHT, YELLOW);
    drawLine(WIDTH*8/9, 0 ,WIDTH*8/9, HEIGHT, WHITE);
}

//*****************************************************************************
//
//! SPI Master mode main loop
//!
//! This function configures SPI modelue as master and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
void MasterMain()
{
    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_3PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);


    // Disable OlEDCS
    GPIOPinWrite(GPIOA1_BASE, 0x1, 0x1);

    // Initialize the OLED
    Adafruit_Init();


    while (1) {
        MAP_UtilsDelay(8000000);
        printHzLine();

        MAP_UtilsDelay(8000000);
        printVtLine();

        MAP_UtilsDelay(8000000);
        print_hello_world();
        
        MAP_UtilsDelay(8000000);
        print_font();
        
        MAP_UtilsDelay(8000000);
        testlines(RED);
        
        MAP_UtilsDelay(8000000);
        testfastlines(RED, BLUE);

        MAP_UtilsDelay(8000000);
        testdrawrects(MAGENTA);

        MAP_UtilsDelay(8000000);
        testfillrects(YELLOW, WHITE);

        MAP_UtilsDelay(8000000);
        testfillcircles(20, RED);

        MAP_UtilsDelay(8000000);
        testdrawcircles(30, BLUE);

        MAP_UtilsDelay(8000000);
        testroundrects();

        MAP_UtilsDelay(8000000);
        testtriangles();
    }
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
//! Main function for spi demo application
//!
//! \param none
//!
//! \return None.
//
//*****************************************************************************
void main()
{
    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Muxing UART and SPI lines.
    //
    PinMuxConfig();

    GPIO_IF_LedConfigure(LED1|LED2|LED3);
    GPIO_IF_LedOff(MCU_ALL_LED_IND);
    GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    // Reset the peripheral
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // Start the OLED Display loop
    MasterMain();
}

