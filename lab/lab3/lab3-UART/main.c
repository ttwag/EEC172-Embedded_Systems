
//*****************************************************************************
//
// Application Name     - int_sw
// Application Overview - The objective of this application is to demonstrate
//                          GPIO interrupts using SW2 and SW3.
//                          NOTE: the switches are not debounced!
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup int_sw
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdint.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "systick.h"
#include "pin.h"

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"
#include "uart.h"
#include "pin_mux_config.h"
#include "spi.h"

// OLED
#include "./oled/Adafruit_SSD1351.h"
#include "./oled/Adafruit_GFX.h"
#include "./oled/oled_test.h"
#include "./oled/glcdfont.h"

// some helpful macros for systick

#define SPI_IF_BIT_RATE 100000
// the cc3200's fixed clock frequency of 80 MHz
// note the use of ULL to indicate an unsigned long long constant
#define SYSCLKFREQ 80000000ULL

// systick reload value set to 40ms period
#define SYSTICK_RELOAD_VAL 3200000UL

// macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
        ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
                ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern void (* const g_pfnVectors[])(void);

volatile uint64_t ulsystick_delta_us = 0;
volatile uint64_t delta_buffer[50];
volatile int systick_cnt = 0;
volatile int systick_cnt_1 = 0; // detect time difference between two push
volatile int k = 0; // index writing delta_buffer

// Uart Communication
volatile char uart_buffer[100];
volatile int uart_buffer_ind = 0;



//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
static void BoardInit(void);

//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//*****************************************************************************
/**
 * Reset SysTick Counter
 */
static inline void SysTickReset(void) {
    // any write to the ST_CURRENT register clears it
    // after clearing it automatically gets reset without
    // triggering exception logic
    // see reference manual section 3.2.1
    HWREG(NVIC_ST_CURRENT) = 1;

    // clear the global count variable
    systick_cnt = 0;
}

/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */


// pt2_1
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
    systick_cnt_1 ++; // use for detect time difference between two push
    ulsystick_delta_us = 0;
}

/**
 * GPIO Interrupt Handler (Pin 50)
 *
 * Triggered at every rising edge of Pin 50.
 * Computes the 17 pulse width duration of pulse including and after the start pulse
 * The start pulse is ~12ms
 */
static void GPIOA0IntHandler(void) {    // Pin 50 Handler
    unsigned long ulStatus;
    ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, true);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus);

    if (ulStatus & IR_GPIO_PIN) {
        // read the countdown register and compute elapsed cycles
        uint64_t ulsystick_delta = SYSTICK_RELOAD_VAL - SysTickValueGet();
        //ulsystick_delta_us = TICKS_TO_US(ulsystick_delta);
        delta_buffer[k] = TICKS_TO_US(ulsystick_delta);
        k = k + 1;
        // reset the countdown register
        SysTickReset();

    }
}

// decoder sequence
// this part decode the binary signal input to cases.
int decode_0_lib (int decoder_buffer) {
    if(decoder_buffer == 0xc084){
        return 1;
    }
    if(decoder_buffer == 0xc044){
        return 2;
    }
    if(decoder_buffer == 0xc0c4){
        return 3;
    }
    if(decoder_buffer == 0xc024){
        return 4;
    }
    if(decoder_buffer == 0xc0a4){
        return 5;
    }
    if(decoder_buffer == 0xc064){
        return 6;
    }
    if(decoder_buffer == 0xc0e4){
        return 7;
    }
    if(decoder_buffer == 0xc014){
        return 8;
    }
    if(decoder_buffer == 0xc094){
        return 9;
    }
    if(decoder_buffer == 0xc004){
        return 0;
    }
    if(decoder_buffer == 0xc0a0){
        return 10; //enter
    }
    if(decoder_buffer == 0xc038){
        return 11; //delete
    }
    // if none of them found, return failed
    else
        return 1000;
}


// packaged code in pt3, the signal decoder
// this part decode signal data store in delta_buffer to cases
/////////////////////////////////////////////////////////////////////////////////////////
int decode_0(int ii){
    ii++; //this is the starting index for a 16 bit index +1 at beginning to skip start signal.
    int kk = 0; //this variable count for 16 bit.
    int output;

    // set up a new mask
    unsigned int mask_clear = 0x7fff; // 0111 1111 1111 1111, by and operator it will clear bit with 0
    unsigned int mask_set = 0x8000; // 1000 0000 0000 0000, by or operator it will set bit with 1
    int decoder_buffer = 0x0000; // a clear buffer.

    while (kk < 16){

        //Wait  until a new value get in, if wait too much time, return failed
        while(delta_buffer[ii] == 0){
            if (systick_cnt > 100){
                for (kk = 0; kk < 50; kk++){
                    delta_buffer[kk] = 0;
                }
                k = 0;
                return 1000;
            }
        }

        // if 1 use set mask
        if (delta_buffer[ii] > 1500 && delta_buffer[ii] <2500){
            decoder_buffer = decoder_buffer | mask_set;
        }

        // if 0, use reset mask
        else if (delta_buffer[ii] > 500 && delta_buffer[ii] < 1500 ) {
            decoder_buffer = decoder_buffer & mask_clear;
        }

        // shift musk 1 bit right, fill 1 at MSB in clear mask, fill 0 at MSB in set mask
        mask_clear = (mask_clear >> 1) | 0x8000;
        mask_set = (mask_set >> 1);


        // if count trigger, reset delta_buffer and use library decode binary to case.
        if (kk == 15){

            for (kk = 0; kk < 50; kk++){
                delta_buffer[kk] = 0;
            }

            output = decode_0_lib(decoder_buffer);

            return output;
        }
        ii ++;
        kk ++;
    }
    // if not get return value, return failed
    return 1000;
}
////////////////////////////////////////////////////////////////////////////////////////////

// decode case and count with character.
char decode_1(int input, int count){
    if(input == 1)
        return'1';
    else if(input == 2 && count == 1)
        return'2';
    else if(input == 2 && count == 2)
        return'A';
    else if(input == 2 && count == 3)
        return'B';
    else if(input == 2 && count == 4)
        return'C';
    else if(input == 3 && count == 1)
        return'3';
    else if(input == 3 && count == 2)
        return'D';
    else if(input == 3 && count == 3)
        return'E';
    else if(input == 3 && count == 4)
        return'F';
    else if(input == 4 && count == 1)
        return'4';
    else if(input == 4 && count == 2)
        return'G';
    else if(input == 4 && count == 3)
        return'H';
    else if(input == 4 && count == 4)
        return'I';
    else if(input == 5 && count == 1)
        return'5';
    else if(input == 5 && count == 2)
        return'J';
    else if(input == 5 && count == 3)
        return'K';
    else if(input == 5 && count == 4)
        return'L';
    else if(input == 6 && count == 1)
        return'6';
    else if(input == 6 && count == 2)
        return'M';
    else if(input == 6 && count == 3)
        return'N';
    else if(input == 6 && count == 4)
        return'O';
    else if(input == 7 && count == 1)
        return'7';
    else if(input == 7 && count == 2)
        return'P';
    else if(input == 7 && count == 3)
        return'Q';
    else if(input == 7 && count == 4)
        return'R';
    else if(input == 7 && count == 5)
        return'S';
    else if(input == 8 && count == 1)
        return'8';
    else if(input == 8 && count == 2)
        return'T';
    else if(input == 8 && count == 3)
        return'U';
    else if(input == 8 && count == 4)
        return'V';
    else if(input == 9 && count == 1)
        return'9';
    else if(input == 9 && count == 2)
        return'W';
    else if(input == 9 && count == 3)
        return'X';
    else if(input == 9 && count == 4)
        return'Y';
    else if(input == 9 && count == 5)
        return'Z';
    else if(input == 0 && count == 1)
        return'0';
    else if(input == 0 && count == 2)
        return' ';
    else
        return'_';

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




static void UARTIntHandler(void) {

    int y;
    unsigned long ulStatus;
    ulStatus = MAP_UARTIntStatus(UART_INT, true);
    MAP_UARTIntClear(UART_INT, UART_INT_RX);
    if (ulStatus & UART_INT_RX) {
        uart_buffer_ind = 0;
        while (UARTCharsAvail(UART_INT)) {
            uart_buffer[uart_buffer_ind] = UARTCharGetNonBlocking(UART_INT);
            uart_buffer_ind++;
        }
        // Receive Message

        //oled output
        Report("%s\n", uart_buffer);
        fillScreen(BLACK);
        OutstrI(uart_buffer);

        for (y=0; y<100; y++){
            uart_buffer[y] = '\0';
        }
        y = 0;

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
BoardInit(void) {
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

/**
 * Initializes SysTick Module
 */
static void SysTickInit(void) {

    // configure the reset value for the systick countdown register
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);

    // register interrupts on the systick module
    MAP_SysTickIntRegister(SysTickHandler);

    // enable interrupts on systick
    // (trigger SysTickHandler when countdown reaches 0)
    MAP_SysTickIntEnable();

    // enable the systick module itself
    MAP_SysTickEnable();
}



//****************************************************************************
//
//! Main function
//!
//! \param none
//!
//!
//! \return None.
//
//****************************************************************************
int main() {

    //Init
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // declare
    int i = 0; // index reading delta buffer
    int j = 0; // use for avoid first reading
    int x = 0; // index message buffer
    int y = 0; // index m
    int z = 0;
    int count = 1;
    int current = -1; // -1 Indicate NULL store in current
    int decode_0_out;
    int decode_1_flag = 0; // indicate a decode_0 done, decode_1 shall start
    char message_buffer[100]; // message buffer, store the message will be send

    BoardInit();

    PinMuxConfig();

    InitTerm();

    ClearTerm();

    SysTickInit();

    GPIO_IF_LedConfigure(LED1|LED2|LED3);

    GPIO_IF_LedOff(MCU_ALL_LED_IND);

    // GPIO Interrupt
    // Register the interrupt handlers
    MAP_GPIOIntRegister(IR_GPIO_PORT, GPIOA0IntHandler);
    // Configure interrupts on rising edges
    MAP_GPIOIntTypeSet(IR_GPIO_PORT, IR_GPIO_PIN, GPIO_RISING_EDGE); // read ir_output
    uint64_t ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, false);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus);
    // Enable interrupts on ir_output
    MAP_GPIOIntEnable(IR_GPIO_PORT, IR_GPIO_PIN);

    // UART
    MAP_UARTConfigSetExpClk(UART_INT, MAP_PRCMPeripheralClockGet(UART_PERIPH),
                            UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                    UART_CONFIG_PAR_NONE));
    MAP_UARTEnable(UART_INT);
    MAP_UARTFIFOEnable(UART_INT);
    // UART Interrupt
    MAP_UARTIntRegister(UART_INT, UARTIntHandler);
    //unsigned long ulStatus_uart = MAP_UARTIntStatus(UART_INT, false);
    //MAP_UARTIntClear(UART_INT, ulStatus_uart);
    MAP_UARTIntClear(UART_INT, UART_INT_RX);
    MAP_UARTIntEnable(UART_INT, UART_INT_RX);

    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);


    // Reset the peripheral
    MAP_PRCMPeripheralReset(PRCM_GSPI);
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
    fillScreen(BLACK);


    // clear message buffer to NULL
    for (x=0; x<100; x++){
        message_buffer[x] = '\0';
    }
    x = 0;

    for (y=0; y<100; y++){
        uart_buffer[y] = '\0';
    }
    y = 0;


    //

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    Message("\t\t****************************************************\n\r");
    Message("\t\t\tWaveform\n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");


    while (1) {


        j ++;
        if(j == 3){
            j = 2;
        }

        if (delta_buffer[i] < 15000 && delta_buffer[i] > 5000 || j == 1){
            //call decode_0 to get case
            decode_0_out = decode_0(i);
            i = 0;
            k = 0;
            // if didn't failed, set decode_1_flag reset read and write delta_buffer index
            if(decode_0_out != 1000){
                Report("[%d] ", decode_0_out);
                k = 0;
                i = 0;
                decode_1_flag = 1;
            }
            // if failed, reset timer for detect time difference between two press and reset read and write buffer
            else{
                systick_cnt_1 = 0;
                k = 0;
                i = 0;
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            // different case for different input.
            // 10 = enter, 11 = delete
            if (decode_0_out == 11 && (current != -1 || x == 0)){
                decode_1_flag = 0;
                current = -1;
                count = 1;
                // use for debug
                //Report("delete case1 %d \n", x);
            }

            else if (decode_0_out == 11){
                decode_1_flag = 0;
                x--;
                message_buffer[x] = '\0';
                // use for debug
                //Report("delete case2 %d \n", x);
            }

            else if (decode_0_out == 10 && current != -1){
                decode_1_flag = 0;
                Report("%d %d %c", current, count, decode_1(current,count));
                message_buffer[x] = decode_1(current, count);

                //replace with send

                Report("out %s", message_buffer);
                fillScreen(BLACK);
                OutstrO(message_buffer);
                x = 0;
                while(x<100 && message_buffer[x] != '\0'){
                    for(z=0; z<8; z++){
                        if (message_buffer[x+z] == '\0'){
                            message_buffer[x+z] = ' ';
                        }
                        UARTCharPutNonBlocking(UART_INT, message_buffer[x+z]);
                    }
                    x = x+z;
                }
                MAP_UtilsDelay(8000000);

                //clear buffer, index, current and count.
                current = -1;
                count = 1;
                for (x=0; x<100; x++){
                    message_buffer[x] = '\0';
                }
                x = 0;
                // use for debug
                //Report("print case 1 %d \n",message_flag);
            }

            else if (x != 0 && decode_0_out == 10 && current == -1){
                decode_1_flag = 0;

                //replace with send
                Report("out %s", message_buffer);
                fillScreen(BLACK);
                OutstrO(message_buffer);
                x = 0;
                while(x<100 && message_buffer[x] != '\0'){
                    for(z=0;z<8; z++){
                        if (message_buffer[x+z] == '\0'){
                            message_buffer[x+z] = ' ';
                        }
                        UARTCharPutNonBlocking(UART_INT, message_buffer[x+z]);
                    }
                    x = x+z;
                }
                MAP_UtilsDelay(8000000);

                //clear buffer and index
                for (x=0; x<100; x++){
                    message_buffer[x] = '\0';
                }
                x = 0;
                count = 1;

                // use for debug
                //Report("print case 2 %d \n",message_flag);
            }

            else if (decode_0_out == 10){
                Report("empty message\n");
            }

            else if (decode_1_flag == 1 && decode_0_out == current && systick_cnt_1 < 10) {
                decode_1_flag = 0;
                count ++;

                // check if a number pressed for maximum time, if true decode it immediately and reset count, current.
                if (count >= 5 || (count >= 4 && ((current >= 2 && current <= 6) || current == 8)) || (current == 0 && count == 2) || current == 1){
                    Report("%d %d %c", current, count, decode_1(current,count));
                    message_buffer[x] = decode_1(current, count);
                    count = 0;
                    current = -1;
                    x ++;

                }

                // if not reach maximum press count, add count and reset timer for multiple press
                else{
                    //Report("pos %d count %d \n",x , count);
                    systick_cnt_1 = 0;
                }
            }

            //time out or new number input or last number reach maximum count or delete been applied
            else if (decode_1_flag == 1 && (decode_0_out != current || systick_cnt_1 > 10)){
                decode_1_flag = 0;
                // if there's some thing store in current
                if (current != -1){
                    Report("%d %d %c", current, count, decode_1(current, count));
                    message_buffer[x] = decode_1(current, count); //decode current and count, store
                    x++;
                }
                // update count and current
                count = 1;
                current = decode_0_out;
                // use for debug
                //Report("pos %d count %d \n",x , count);

                //clear timer multiple press
                systick_cnt_1 = 0;
            }
        }



        // if no start signal find, go to next delta_buffer address.
        i++;

        //if delta_buffer full before a start signal appear, clear the buffer.
        if (i == 30){
            for (i = 0; i < 30; i++){
                delta_buffer[i] = 0;
            }
            k = 0;
            i = 0;
        }
        while (delta_buffer[i] == 0){
        }




        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // test
        /*
        if (GPIOPinRead(GPIOA1_BASE, 0x20) & 0x20) {
            // Send Message
            GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
            UARTCharPutNonBlocking(UART_INT, 'h');
        }
         */

        //If interrupt has finished writing the signal pulse width buffer

        // Print the decoded button if it's decoded correctly

            //UARTCharPutNonBlocking(UART_INT, 'd');
            //MAP_UtilsDelay(8000000);


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
