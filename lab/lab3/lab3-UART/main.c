
//*****************************************************************************
//
// Application Name     - int_sw
// Application Overview - The objective of this application is to demonstrate
//							GPIO interrupts using SW2 and SW3.
//							NOTE: the switches are not debounced!
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
volatile int count = 0;
volatile uint64_t delta_buffer[20];
volatile int start = 0;
volatile int readReady = 0;

// Uart Communication
volatile int receiveReady = 0;

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
//    expired_num++;
}

/**
 * GPIO Interrupt Handler (Pin 50)
 *
 * Triggered at every rising edge of Pin 50.
 * Computes the 17 pulse width duration of pulse including and after the start pulse
 * The start pulse is ~12ms
 */
static void GPIOA0IntHandler(void) {	// Pin 50 Handler
    unsigned long ulStatus;
    ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, true);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus);

    if (ulStatus & IR_GPIO_PIN) {

        // read the countdown register and compute elapsed cycles
        uint64_t ulsystick_delta = SYSTICK_RELOAD_VAL - SysTickValueGet();
        uint64_t time = TICKS_TO_US(ulsystick_delta);

        // Captures the start pulse only after ~12ms pulse
        if (10000 < time && time < 13500) start = 1;
        if (start) {
            delta_buffer[count] = time;
            count++;
            if (count >= 17) {
                start = 0;
                count = 0;
                // Set readReady high so main can print it
                readReady = 1;
            }
        }
        // reset the countdown register
        SysTickReset();
    }
}

static void UARTIntHandler(void) {
    unsigned long ulStatus;
    ulStatus = MAP_UARTIntStatus(UART_INT, true);
    //MAP_UARTIntClear(UART_INT, ulStatus);
    MAP_UARTIntClear(UART_INT, UART_INT_RX);
    if (ulStatus & UART_INT_RX) {
        receiveReady = 1;
//        while (uart_available) {
//            buffer[i] = uartcharget();
//            i++;
//        }
//        uartReady = 1;
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

//*****************************************************************************
//
//! The decoder decodes the signal pulse width time stored in the delta_buffer
//! into a single controller button number
//!
//! \param  None
//!
//! \return Controller button number
//
//*****************************************************************************
int decoder() {
    int sum = 0x0;
    int i;
    for (i = 9; i < 17; i++) {
        sum = sum << 1;
        if (delta_buffer[i] > 2000) {
            sum = sum | 0x1;
        }
    }
    if (sum == 132) {
        return 1;
    } else if (sum == 68) {
        return 2;
    } else if (sum == 196) {
        return 3;
    } else if (sum == 36) {
        return 4;
    } else if (sum == 164) {
        return 5;
    } else if (sum == 100) {
        return 6;
    } else if (sum == 228) {
        return 7;
    } else if (sum == 20) {
        return 8;
    } else if (sum == 148) {
        return 9;
    } else if (sum == 4) {
        return 0;
    } else if (sum == 56) {
        return 100; // Mute
    } else if (sum == 160) {
        return 200; // Last
    }
    else return -1;
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

    // UART Interrupt
    MAP_UARTConfigSetExpClk(UART_INT, MAP_PRCMPeripheralClockGet(UART_PERIPH),
                            UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE));
    MAP_UARTIntRegister(UART_INT, UARTIntHandler);
    //unsigned long ulStatus_uart = MAP_UARTIntStatus(UART_INT, false);
    //MAP_UARTIntClear(UART_INT, ulStatus_uart);
    MAP_UARTIntClear(UART_INT, UART_INT_RX);
    MAP_UARTIntEnable(UART_INT, UART_INT_RX);


    Message("\t\t****************************************************\n\r");
    Message("\t\t\tWaveform\n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");

    while (1) {
        // SW
        if (GPIOPinRead(GPIOA1_BASE, 0x20) & 0x20) {
            // Send Message
            GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
            UARTCharPut(UART_INT, 'H');
        }
        if (receiveReady) {
            // LED
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            receiveReady = 0;
        }

        // If interrupt has finished writing the signal pulse width buffer
//        if (readReady) {
//            int num = decoder();
//            // Print the decoded button if it's decoded correctly
//            if (num != -1) {
//                Report("%d\n", num);
//            }
//            // Prevent printing a number multiple times
//            readReady = 0;
//        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
