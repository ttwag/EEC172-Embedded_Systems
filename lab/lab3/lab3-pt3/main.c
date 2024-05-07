
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
#include "spi.h"
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

// OLED
#include "./oled/Adafruit_SSD1351.h"
#include "./oled/Adafruit_GFX.h"
#include "./oled/oled_test.h"
#include "./oled/glcdfont.h"

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

#define SPI_IF_BIT_RATE  100000

// State definition of the texting state machine
typedef enum {
    STATE_INPUT,
    STATE_WAITING
} State;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern void (* const g_pfnVectors[])(void);

volatile uint64_t ulsystick_delta_us = 0;
volatile int systick_cnt = 0;
volatile int systick_expired = 0;
volatile int bufferInd = 0;
volatile uint64_t delta_buffer[20];
volatile int start = 0;
volatile int readReady = 0;

int oled_x = 0;
int oled_y = 0;
uint8_t lastButtonPressed = 101;
int buttonPressedNum = 0;

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
            delta_buffer[bufferInd] = time;
            bufferInd++;
            if (bufferInd >= 17) {
                start = 0;
                bufferInd = 0;
                // Set readReady high so main can print it
                readReady = 1;
            }
        }
        // reset the countdown register
        SysTickReset();
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


void oledInit() {
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
    fillScreen(WHITE);
}

void oledPrint(int x, int y, unsigned char oledChar) {
    drawChar(x, y, oledChar, WHITE, WHITE, 2);
}

void oledErase(int x, int y) {
    fillRect(x, y, 10, 14, BLACK);
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
uint8_t decoder() {
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
    else return 255;
}

//*****************************************************************************
//
//! The getTVButton indefinitely wait for the pressed TV button then returns the
//! pressed number
//!
//! \param  None
//!
//! \return Pressed TV button number from decoder
//
//*****************************************************************************
char getTVButton() {
    while (1) {
        // If interrupt has finished writing the signal pulse width buffer
        if (readReady) {
            uint8_t num = decoder();
            // Print the decoded button if it's decoded correctly
            if (num != 255) {
                GPIO_IF_LedOn(MCU_RED_LED_GPIO);
                return num;
            }
            // Prevent printing a number multiple times
            readReady = 0;
        }
    }
}

//*****************************************************************************
//
//! The TVbutton2Letter returns the character associated with the button after 
//! n clicks
//!
//! \param  None
//!
//! \return Controller button number
//
//*****************************************************************************
unsigned char TVbutton2Letter(uint8_t button, int n) {
    if (button == 100) return 100;
    else if (button == 200) return 200;
    else if (button == 0) return 0;
    //else if (button == 1) return 1;
    else {
        int select;
        switch(button) {
            case 2:
                select = n % 3;
                if (select == 0) return 'A';
                else if (select == 1) return 'B';
                else return 'C';
            case 3:
                select = n % 3;
                if (select == 0) return 'D';
                else if (select == 1) return 'E';
                else return 'F';
            case 4:
                select = n % 3;
                if (select == 0) return 'G';
                else if (select == 1) return 'H';
                else return 'I';
            case 5:
                select = n % 3;
                if (select == 0) return 'J';
                else if (select == 1) return 'K';
                else return 'L';
            case 6:
                select = n % 3;
                if (select == 0) return 'M';
                else if (select == 1) return 'N';
                else return 'O';
            case 7:
                select = n % 4;
                if (select == 0) return 'P';
                else if (select == 1) return 'Q';
                else if (select == 2) return 'R';
                else return 'S';
            case 8:
                select = n % 3;
                if (select == 0) return 'T';
                else if (select == 1) return 'U';
                else return 'V';
            case 9:
                select = n % 4;
                if (select == 0) return 'W';
                else if (select == 1) return 'X';
                else if (select == 2) return 'Y';
                else return 'Z';
            default:
                return 100;
        }
    }
}

State handleEvent(State currentState) {
    switch(currentState) {
        case STATE_INPUT:
            oledPrint(oled_x, oled_y, '_');
            uint8_t buttonPressed = getTVButton();
            unsigned char buttonVal = TVbutton2Letter(buttonPressed, buttonPressedNum);
            buttonPressedNum++;
            lastButtonPressed = buttonPressed;
            if (buttonVal == 200 && oled_x != 0) {
                // Backspace
                oled_x -= 13;
                return currentState;
            }
            else if (buttonVal == 1) {
                // Function to be added
                return currentState;
            }
            else if (buttonVal == 100) {
                // Enter
                return currentState;
            }
            else if (buttonVal == 0) {
                // Space
                oled_x += 13;
                return currentState;
            }
            else {
                oledErase(oled_x, oled_y);
                oledPrint(oled_x, oled_y, buttonVal);
                return STATE_WAITING;
            }
        case STATE_WAITING:
            // Waiting Logic
//            for (int i = 0; i < 80; i += 1) {
//                MAP_UtilsDelay(100000);
//                uint8_t buttonPressed = getTVButton(); // if (readReady)
//                unsigned char buttonVal = TVbutton2Letter(buttonPressed, buttonPressedNum);
//                buttonPressedNum++;
//                if (buttonVal == 200) {
//                    oledErase(oled_x, oled_y);
//                    return STATE_INPUT;
//                }
//                else if (buttonVal >= 65 && buttonVal <= 90) {
//                    if (buttonPressed == lastButtonPressed) {
//                        // Toggle
//                        buttonPressedNum++;
//                        oledErase(oled_x, oled_y);
//                        //oledPrint(oled_x, oled_y, )
//                        return STATE_WAITING;
//                    }
//                    else {
//                        // Finish
//                        lastButtonPressed = buttonPressed;
//                        buttonPressedNum = 1;
//                        oled_x+=13;
//                        oledPrint(oled_x, oled_y, buttonVal);
//                        return STATE_WAITING;
//                    }
//                }
//            }

            oled_x += 13;
            return STATE_INPUT;
    }
    return currentState;
}


// x space of ~13 between characters
// y space of ~18 between characters
// Delay of MAP_UtilsDelay(10000000);

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


    // Register the interrupt handlers
    MAP_GPIOIntRegister(IR_GPIO_PORT, GPIOA0IntHandler);

    // Configure interrupts on rising edges
    MAP_GPIOIntTypeSet(IR_GPIO_PORT, IR_GPIO_PIN, GPIO_RISING_EDGE); // read ir_output
    uint64_t ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, false);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus);

    // Enable interrupts on ir_output
    MAP_GPIOIntEnable(IR_GPIO_PORT, IR_GPIO_PIN);

    // Enable OLED
    oledInit();

    // Begin the user input text state machine
    while (1) {
        // If interrupt has finished writing the signal pulse width buffer
        if (readReady) {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            uint8_t num = decoder();
            // Print the decoded button if it's decoded correctly
            if (num != 255) {
//                    return num;
            }
            // Prevent printing a number multiple times
            readReady = 0;
         }
//        oledPrint(oled_x, oled_y, '_');
//        uint8_t buttonNum = getTVButton();
//        Report("%d", buttonNum);
//        oledErase(oled_x, oled_y);
//        unsigned char letter = TVbutton2Letter(buttonNum, 0);
//
//        oledPrint(oled_x, oled_y, letter);
//        oled_x += 13;
        // Test TV print
        // Test waiting state logic
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
