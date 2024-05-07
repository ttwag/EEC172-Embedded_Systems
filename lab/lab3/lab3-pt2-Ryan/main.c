
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

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"

#include "pin_mux_config.h"

// some helpful macros for systick

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
volatile int systick_cnt = 0;
volatile int systick_expired = 0;
volatile int flag = 0;
volatile int count = 0;
volatile uint64_t delta_buffer[100];
volatile int clear_buffer;


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

    systick_expired = 0;
}

/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
    systick_expired = 1;
    ulsystick_delta_us = 0;
}

static void GPIOA0IntHandler(void) {    // Pin 50 Handler
    unsigned long ulStatus;
    ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, true);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus);

    if (ulStatus & IR_GPIO_PIN) {
        if (systick_expired) {
            systick_expired = 0;
        }
        else {

            // read the countdown register and compute elapsed cycles
            uint64_t ulsystick_delta = SYSTICK_RELOAD_VAL - SysTickValueGet();
            //ulsystick_delta_us = TICKS_TO_US(ulsystick_delta);
            delta_buffer[count] = TICKS_TO_US(ulsystick_delta);
            flag = 1;

            count = count + 1;
            // reset the countdown register
            SysTickReset();
        }
    }
}

// decoder sequence
char* decode (int decoder_buffer) {
    if(decoder_buffer == 0xc084){
        return "1";
    }
    if(decoder_buffer == 0xc044){
        return "2";
    }
    if(decoder_buffer == 0xc0c4){
        return "3";
    }
    if(decoder_buffer == 0xc024){
        return "4";
    }
    if(decoder_buffer == 0xc0a4){
        return "5";
    }
    if(decoder_buffer == 0xc064){
        return "6";
    }
    if(decoder_buffer == 0xc0e4){
        return "7";
    }
    if(decoder_buffer == 0xc014){
        return "8";
    }
    if(decoder_buffer == 0xc094){
        return "9";
    }
    if(decoder_buffer == 0xc004){
        return "0";
    }
    if(decoder_buffer == 0xc0a0){
        return "mute";
    }
    if(decoder_buffer == 0xc038){
        return "delete";
    }
    else
        return NULL;
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
    int i = 0;
    int j = 0;
    int k = 0;

    BoardInit();
    
    PinMuxConfig();
    
    InitTerm();

    ClearTerm();

    SysTickInit();

    GPIO_IF_LedConfigure(LED1|LED2|LED3);

    GPIO_IF_LedOff(MCU_ALL_LED_IND);


    // Register the interrupt handlers
    MAP_GPIOIntRegister(IR_GPIO_PORT, GPIOA0IntHandler);

    // Configure interrupts on rising edges
    MAP_GPIOIntTypeSet(IR_GPIO_PORT, IR_GPIO_PIN, GPIO_RISING_EDGE); // read ir_output
    uint64_t ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, false);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus);

    // Enable interrupts on ir_output
    MAP_GPIOIntEnable(IR_GPIO_PORT, IR_GPIO_PIN);


    Message("\t\t****************************************************\n\r");
    Message("\t\t\tWaveform\n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");

    unsigned int mask_clear = 0x7fff;
    unsigned int mask_set = 0x8000;
    int decoder_buffer = 0x0000;

    while (1) {
        //while (!flag){;}
        //uint64_t tmp_delta_us = ulsystick_delta_us;
//        if (flag) {
//            GPIO_IF_LedToggle(MCU_RED_LED_GPIO);
//            Report("measured pulse width: %llu us\n", tmp_delta_us);
//            Report("measured pulse width: %d us\n", ulsystick_delta_us);
//            Report("Count: %d\n", count);
//            flag = 0;
//        }


        //setup mask
        mask_clear = 0x7fff;
        mask_set = 0x8000;
        decoder_buffer = 0x0000;
        k = 0;
        j ++;
        if(j == 3){
            j = 2;
        }


        if (delta_buffer[i] < 15000 && delta_buffer[i] > 5000 || j == 1){
            i++;
            while (k < 16){

                //get result to binary
                while(delta_buffer[i] == 0){
                }

                if (delta_buffer[i] > 1500 && delta_buffer[i] <2500){
                    decoder_buffer = decoder_buffer | mask_set;
                }

                else if (delta_buffer[i] > 500 && delta_buffer[i] < 1500 ) {
                    decoder_buffer = decoder_buffer & mask_clear;
                }

                mask_clear = (mask_clear >> 1) | 0x8000;
                mask_set = (mask_set >> 1);



                if (k == 15){

                    char* output = decode(decoder_buffer);

                    if (output != NULL) {
                        Report("Decoded value: %s\n", output);

                        for (k = 0; k < 100; k++){
                            delta_buffer[k] = 0;
                        }
                        count = 0;
                        i = 0;
                        mask_clear = 0x7fff;
                        mask_set = 0x8000;
                        decoder_buffer = 0x0000;


                    }

                    else {
//                        Report("NA");
                        i = 0;
                        for (k = 0; k < 100; k++){
                            delta_buffer[k] = 0;
                        }
                        count = 0;
                        mask_clear = 0x7fff;
                        mask_set = 0x8000;
                        decoder_buffer = 0x0000;
                    }
                }
                i ++;
                k ++;


                if (systick_cnt > 10000000){
                    for (k = 0; k < 100; k++){
                        delta_buffer[k] = 0;
                    }
                    count = 0;
                    k = 20;
                    i = 0;
                }

            }
        }
        i++;
        if (i == 70){
            for (k = 0; k < 100; k++){
                delta_buffer[k] = 0;
            }
            count = 0;
            i = 0;
        }
        while (delta_buffer[i] == 0){

        }

    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
