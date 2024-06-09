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
// Application Name     -   SSL Demo
// Application Overview -   This is a sample application demonstrating the
//                          use of secure sockets on a CC3200 device.The
//                          application connects to an AP and
//                          tries to establish a secure connection to the
//                          Google server.
// Application Details  -
// docs\examples\CC32xx_SSL_Demo_Application.pdf
// or
// http://processors.wiki.ti.com/index.php/CC32xx_SSL_Demo_Application
//
//*****************************************************************************
//*****************************************************************************
//
//! \addtogroup ssl
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// Simplelink includes
#include "simplelink.h"
//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_apps_rcm.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"

//Common interface includes
#include "./lib_if/gpio_if.h"
#include "./lib_if/common.h"
#include "./lib_if/uart_if.h"
#include "./lib_if/i2c_if.h"

// Custom includes
#include "./utils/network_utils.h"

//adjustment for IR
#include "systick.h"
#include "pin.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "gpio.h"
#include "spi.h"
#include "hw_nvic.h"
#include "pin_mux_config.h"
#include "hw_common_reg.h"

// OLED
#include "./oled/Adafruit_SSD1351.h"
#include "./oledAdafruit_GFX.h"
#include "./oled/oled_test.h"
#include "./oled/glcdfont.h"




//*****************************************************************************
//macro and variables
//*****************************************************************************

//AWS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                4    /* Current Date */
#define MONTH               6     /* Month 1-12 */
#define YEAR                2024  /* Current year */
#define HOUR                12    /* Time - hours */
#define MINUTE              0    /* Time - minutes */
#define SECOND              0     /* Time - seconds */

#define APPLICATION_NAME      "SSL"
#define APPLICATION_VERSION   "SQ24"
#define SERVER_NAME           "a2m3q4kfchczuu-ats.iot.us-west-1.amazonaws.com" // CHANGE ME
#define GOOGLE_DST_PORT       8443

#define POSTHEADER "POST /things/Tao_CC3200Board/shadow HTTP/1.1\r\n"             // CHANGE ME
#define GETHEADER "GET /things/Tao_CC3200Board/shadow HTTP/1.1\r\n"
#define HOSTHEADER "Host: a2m3q4kfchczuu-ats.iot.us-west-1.amazonaws.com\r\n"  // CHANGE ME
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"
//#define DATA1 "{" \
//            "\"state\": {\r\n"                                              \
//                "\"desired\" : {\r\n"                                       \
//                    "\"message\" :\""                                       \
//                        "0"                             \
//                        "\"\r\n"                                            \
//                "}"                                                         \
//            "}"                                                             \
//        "}\r\n\r\n"

// Uart Communication

#define CONSOLE              UARTA0_BASE
#define UartGetChar()        MAP_UARTCharGet(CONSOLE)
#define UartPutChar(c)       MAP_UARTCharPut(CONSOLE,c)
#define MAX_STRING_LENGTH    80

volatile char uart_buffer[100];
volatile int uart_buffer_ind = 0;



#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
volatile int g_iCounter = 0;
char DATA1[200] = "{" \
        "\"state\": {\r\n"                                              \
            "\"desired\" : {\r\n"                                       \
                "\"message\" :\""                                       \
                    "0"                             \
                    "\"\r\n"                                            \
            "}"                                                         \
        "}"                                                             \
    "}\r\n\r\n";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// IR
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


extern void (* const g_pfnVectors[])(void);

volatile uint64_t ulsystick_delta_us = 0;
volatile uint64_t delta_buffer[50];
volatile int systick_cnt = 0;
volatile int k = 0; // index writing delta_buffer

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//*****************************************************************************
//functions
//*****************************************************************************
static int set_time();
static void BoardInit(void);
static int http_post(int);

// Systick
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
    ulsystick_delta_us = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//IR
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
            if (systick_cnt > 5){
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//SNAKE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define APPLICATION_VERSION     "1.4.0"
#define SPI_IF_BIT_RATE  100000
#define SNAKE_SIZE            8
#define SNAKE_LENGTH          10
#define RIGHT   0
#define LEFT    1
#define UP      2
#define DOWN    3

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

struct ListNode {
    int coordinate[2];
    struct ListNode* next;
    struct ListNode* prev;
};

struct DoublyLinkedlist {
    struct ListNode* head;
    struct ListNode* tail;
    int length;
};

struct Snake {
    struct DoublyLinkedlist* body;
    struct ListNode* head;
    struct ListNode* tail;
    int segSize;
    int initPos;
    int length;
};


//*****************************************************************************
// Doubly Linked List
//*****************************************************************************

struct ListNode* createListNode(int x, int y) {
    struct ListNode* newNode = (struct ListNode*)malloc(sizeof(struct ListNode));
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    newNode->coordinate[0] = x;
    newNode->coordinate[1] = y;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

struct DoublyLinkedlist* createDoublyLinkedlist() {
    struct DoublyLinkedlist* newList = (struct DoublyLinkedlist*)malloc(sizeof(struct DoublyLinkedlist));
    if (newList == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    newList->head = NULL;
    newList->tail = NULL;
    newList->length = 0;
    return newList;
}
void freeDoublyLinkedlist(struct ListNode* head) {
    struct ListNode* current = head;
    struct ListNode* temp;
    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
}

void insertFront(struct DoublyLinkedlist* list, int x, int y) {
    struct ListNode* newNode = createListNode(x, y);
    newNode->prev = NULL;
    if (list->length > 0) {
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
    }
    else {
        newNode->next = NULL;
        list->head = newNode;
        list->tail = newNode;
    }
    list->length++;
}

void deleteTail(struct DoublyLinkedlist* list) {
    struct ListNode* temp = list->tail;
    if (list->length > 1) {
        list->length--;
        list->tail = list->tail->prev;
        list->tail->next = NULL;
    }
    else {
        list->length = 0;
        list->head = NULL;
        list->tail = NULL;
    }
    freeDoublyLinkedlist(temp);
}

void printList(struct DoublyLinkedlist* list) {
    struct ListNode* current = list->head;
    while (current != NULL) {
        printf("x = %d, y = %d\n", current->coordinate[0], current->coordinate[1]);
        current = current->next;
    }
}

//*****************************************************************************
// Snake
//*****************************************************************************
struct Snake* createSnake(int segSize, int length, int initPos) {
    struct Snake* newSnake = (struct Snake*)malloc(sizeof(struct Snake));
    int i;
    newSnake->body = createDoublyLinkedlist();
    newSnake->segSize = segSize;
    newSnake->initPos = initPos;
    newSnake->length = length;
    for (i = length - 1; i >= 0; i--) {
        insertFront(newSnake->body, initPos + i * segSize, initPos);
    }
    newSnake->head = newSnake->body->head;
    newSnake->tail = newSnake->body->tail;
    return newSnake;
}

void freeSnake(struct Snake* snake) {
    struct ListNode* head = snake->head;
    freeDoublyLinkedlist(head);
}

void move(struct Snake* snake, int direction) {
    int x = snake->head->coordinate[0];
    int y = snake->head->coordinate[1];
    if (direction == RIGHT) insertFront(snake->body, x+snake->segSize, y);
    else if (direction == LEFT) insertFront(snake->body, x-snake->segSize, y);
    else if (direction == UP) insertFront(snake->body, x, y+snake->segSize);
    else insertFront(snake->body, x, y-snake->segSize);
    deleteTail(snake->body);
    snake->head = snake->body->head;
    snake->tail = snake->body->tail;
}

int selfCollision() {
    return 0;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//I2C
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write the register address to be read from.
// Stop bit implicitly assumed to be 0.
// Read the specified length of data
int readacc(axis){
    unsigned char DevAddr = 0x18;
    unsigned char XOffSet = 0x3;
    unsigned char YOffSet = 0x5;
    unsigned char out;
    unsigned char RdLen = 1;
    if (axis == 1){
        I2C_IF_Write(DevAddr,&XOffSet,1,0);
        I2C_IF_Read(DevAddr, &out, RdLen);
    }
    else if (axis == 2){
        //
        // Write the register address to be read from.
        // Stop bit implicitly assumed to be 0.
        //Read the specified length of data
        I2C_IF_Write(DevAddr,&YOffSet,1,0);
        I2C_IF_Read(DevAddr, &out, RdLen);
    }

    else{
        return 0;
    }

    return (int)(((signed char)out)*0.1);


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//print
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void print_lost(void) {
    fillScreen(BLACK);
    drawChar(0, HEIGHT/5, 'Y', WHITE, WHITE, 5);
    drawChar(30, HEIGHT/5, 'o', WHITE, WHITE, 5);
    drawChar(55, HEIGHT/5, 'u', WHITE, WHITE, 5);

    drawChar(0, HEIGHT*3/5, 'L', WHITE, WHITE, 5);
    drawChar(30, HEIGHT*3/5, 'o', WHITE, WHITE, 5);
    drawChar(58, HEIGHT*3/5, 's', WHITE, WHITE, 5);
    drawChar(77, HEIGHT*3/5, 't', WHITE, WHITE, 5);
    drawChar(95, HEIGHT*3/5, '!', WHITE, WHITE, 5);
}

void print_win(void) {
    fillScreen(BLACK);
    drawChar(0, HEIGHT/5, 'Y', WHITE, WHITE, 5);
    drawChar(30, HEIGHT/5, 'o', WHITE, WHITE, 5);
    drawChar(55, HEIGHT/5, 'u', WHITE, WHITE, 5);

    drawChar(0, HEIGHT*3/5, 'W', WHITE, WHITE, 5);
    drawChar(30, HEIGHT*3/5, 'i', WHITE, WHITE, 5);
    drawChar(58, HEIGHT*3/5, 'n', WHITE, WHITE, 5);
}

void print_start(void) {
    fillScreen(BLACK);
    drawChar(0, 6, 'P', WHITE, WHITE, 1);
    drawChar(5, 6, 'r', WHITE, WHITE, 1);
    drawChar(10, 6, 'e', WHITE, WHITE, 1);
    drawChar(15, 6, 's', WHITE, WHITE, 1);
    drawChar(20, 6, 's', WHITE, WHITE, 1);

    drawChar(30, 6, 'A', WHITE, WHITE, 1);
    drawChar(35, 6, 'n', WHITE, WHITE, 1);
    drawChar(40, 6, 'y', WHITE, WHITE, 1);

    drawChar(50, 6, 'K', WHITE, WHITE, 1);
    drawChar(55, 6, 'e', WHITE, WHITE, 1);
    drawChar(60, 6, 'y', WHITE, WHITE, 1);

    drawChar(70, 6, 'T', WHITE, WHITE, 1);
    drawChar(75, 6, 'o', WHITE, WHITE, 1);

    drawChar(85, 6, 'S', WHITE, WHITE, 1);
    drawChar(90, 6, 't', WHITE, WHITE, 1);
    drawChar(95, 6, 'a', WHITE, WHITE, 1);
    drawChar(100, 6, 'r', WHITE, WHITE, 1);
    drawChar(105, 6, 't', WHITE, WHITE, 1);
}

void print_restart(void) {
    fillScreen(BLACK);
    drawChar(0, 6, 'P', WHITE, WHITE, 1);
    drawChar(5, 6, 'r', WHITE, WHITE, 1);
    drawChar(10, 6, 'e', WHITE, WHITE, 1);
    drawChar(15, 6, 's', WHITE, WHITE, 1);
    drawChar(20, 6, 's', WHITE, WHITE, 1);

    drawChar(30, 6, 'A', WHITE, WHITE, 1);
    drawChar(35, 6, 'n', WHITE, WHITE, 1);
    drawChar(40, 6, 'y', WHITE, WHITE, 1);

    drawChar(50, 6, 'K', WHITE, WHITE, 1);
    drawChar(55, 6, 'e', WHITE, WHITE, 1);
    drawChar(60, 6, 'y', WHITE, WHITE, 1);

    drawChar(70, 6, 'T', WHITE, WHITE, 1);
    drawChar(75, 6, 'o', WHITE, WHITE, 1);

    drawChar(85, 6, 'R', WHITE, WHITE, 1);
    drawChar(90, 6, 'e', WHITE, WHITE, 1);
    drawChar(95, 6, 's', WHITE, WHITE, 1);
    drawChar(100, 6, 't', WHITE, WHITE, 1);
    drawChar(105, 6, 'a', WHITE, WHITE, 1);
    drawChar(110, 6, 'r', WHITE, WHITE, 1);
    drawChar(115, 6, 't', WHITE, WHITE, 1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//SW
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int sw(num){
    int sw;
    if(num == 2){
        sw = GPIOPinRead(GPIOA1_BASE, 0x20) & 0x20;
    }
    else if (num == 3){
        sw = GPIOPinRead(GPIOA2_BASE, 0x40) & 0x40;
    }
    else {
        sw = 0;
    }
    return sw;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Initial
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


static void BoardInit(void) {
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************
static int set_time() {
    long retVal;
    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;
    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                       SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                       sizeof(SlDateTime),(unsigned char *)(&g_time));
    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}

void json_message(char message) {
    snprintf(DATA1, sizeof(DATA1),
                     "{" \
                         "\"state\": {\r\n"                                              \
                             "\"desired\" : {\r\n"                                       \
                                 "\"message\" :\""                                     \
                                             "%c"                                       \
                                     "\"\r\n"                                            \
                             "}"                                                         \
                         "}"                                                             \
                     "}\r\n\r\n", message
    );
}



//*****************************************************************************
//
//! Main 
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void main() {
    int status;

    //IR parameter
    int t = 0;
    int i = 0; // index reading delta buffer
    int j = 0; // use for avoid first reading
    int speed = 1; //IR output for control speed

    int x_now;
    int y_now;

    //acc & map
    int x_acc = 1;
    int y_acc = 1;
    int x = 1;
    int y = 1;

    //score
    int mark_x;
    int mark_y;
    int score;
    int score_1;
    //int score_2;
    int lRetVal = 0;

    int flag;

    //
    // Initialize board configuration
    //
    BoardInit();

    PinMuxConfig();

    InitTerm();

    SysTickInit();

    ClearTerm();

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

    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);

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

    // Establish internet connection
    UART_PRINT("My terminal works!\n\r");
    // initialize global default app configuration
    g_app_config.host = SERVER_NAME;
    g_app_config.port = GOOGLE_DST_PORT;
    //Connect the CC3200 to the local access point
    lRetVal = connectToAccessPoint();
    //Set time so that encryption can be used
    lRetVal = set_time();
    if(lRetVal < 0) {
        UART_PRINT("Unable to set time in the device");
        LOOP_FOREVER();
    }
    //Connect to the website with TLS encryption
    lRetVal = tls_connect();
    if(lRetVal < 0) {
        ERR_PRINT(lRetVal);
    }



//    sl_Stop(SL_STOP_TIMEOUT);

    while(1) {
        //
        // Fetching the input from the terminal.
        //


        j ++;
        if(j == 3){
            j = 2;
        }
        fillScreen(BLACK);
        MAP_UtilsDelay(4000000);
        print_start();
        while(1){
            status = 0;
            score = 5;
            score_1 = 0;
            flag = 1;


            status = http_get(lRetVal);
            if ((sw(2) || sw(3) )|| status == 0 || status == 3){
                json_message('0');
                http_post(lRetVal);
                break;
            }

        }

        //mark_x = rand() %120 + 4;
        //mark_y = rand() %120 + 4;
        fillScreen(BLACK);
        mark_x = 70;
        mark_y = 70;

        // Start the OLED Display loop
        int direction = 3;
        int lastDirection = 3;

        // Initialize a Snake object then draw it to the OLED screen
        struct Snake* snake = createSnake(SNAKE_SIZE, SNAKE_LENGTH, WIDTH/2);
        struct ListNode* head = snake->head;
        while (head != NULL) {
            fillRect(head->coordinate[0], head->coordinate[1], 10, 10, RED);
            head = head->next;
        }

        while(flag){
            //read IR
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (delta_buffer[i] < 15000 && delta_buffer[i] > 5000 || j == 1){


                //call decode_0 to get case
                speed = decode_0(i);
                i = 0;
                k = 0;
                // if didn't failed, set decode_1_flag reset read and write delta_buffer index
                if(speed != 1000){
                    Report("[%d] ", speed);
                    k = 0;
                    i = 0;
                }
                // if failed, reset timer for detect time difference between two press and reset read and write buffer
                else{
                    speed = 1;
                    k = 0;
                    i = 0;
                }

            }

            //if delta_buffer full before a start signal appear, clear the buffer.
            if (i == 30){
                for (i = 0; i < 30; i++){
                    delta_buffer[i] = 0;
                }
                k = 0;
                i = 0;
            }
            if (delta_buffer[i] != 0){
                i++;
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //


            //read acc
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (t>(100000/(100*speed+1))){

                x_acc = readacc(1);
                y_acc = readacc(2);

                //erase old position
                drawLine(0,128-y ,128 ,128-y, BLACK);
                drawLine(x, 0, x, 128, BLACK);

                //update new position
                x = x + x_acc;
                y = y + y_acc;

                //checking rang
                if (x>=30){
                    x = 30;
                }
                else if (x<=1){
                    x = 1;
                }

                if (y>=30){
                    y = 30;
                }
                else if (y<=1){
                    y = 1;
                }

                //draw new position
                drawLine(0,128-y ,128 ,128-y, WHITE);
                drawLine(x, 0, x, 128, WHITE);
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                if (sw(3)) {
                    if (lastDirection == UP) direction = LEFT;
                    else if (lastDirection == LEFT) direction = DOWN;
                    else if (lastDirection == RIGHT) direction = UP;
                    else direction = RIGHT;
                }

                //            SW2 Pressed
                if (sw(2)) {
                    if (lastDirection == UP) direction = RIGHT;
                    else if (lastDirection == LEFT) direction = UP;
                    else if (lastDirection == RIGHT) direction = DOWN;
                    else direction = LEFT;
                }
                lastDirection = direction;

                // Move the snake and draw new position
                fillRect(snake->tail->coordinate[0], snake->tail->coordinate[1], 10, 10, BLACK);
                move(snake, direction);
                fillRect(snake->head->coordinate[0], snake->head->coordinate[1], 10, 10, RED);

                // get head position of the snake
                x_now = snake->head->coordinate[0];
                y_now = snake->head->coordinate[1];

                drawCircle(mark_x, mark_y, 5, BLACK);
                if((x_now - mark_x) > -10 && (x_now - mark_x < 3) && (y_now - mark_y) > -10 && (y_now - mark_y) < 3){
                    score_1 = score_1 + speed/2 + x/10 + y/10;
                    mark_x = rand() %100 + 10;
                    mark_y = rand() %100 + 10;
                }

                drawCircle(mark_x, mark_y, 5, WHITE);

                // Check if the snake hits the wall
                if (x_now <= x || x_now >= 128 || y_now <= 0 || y_now >= 128-y) {
                    status = 2;
                    print_lost();
                    MAP_UtilsDelay(4000000);
                    print_restart();
                    MAP_UtilsDelay(4000000);
                    while(1){
                        if (sw(2) || sw(3)){
                            json_message('2');
                            http_post(lRetVal);
                            break;
                        }
                    }

                    //wait to confirm other player get state and prepared for restart.
                    status = http_get(lRetVal);
                    while(status != 3){
                        status = http_get(lRetVal);
                    }
                    flag = 0;
                }

                if (score == score_1) {
                    status = 1;
                    print_win();
                    MAP_UtilsDelay(4000000);
                    print_restart();
                    MAP_UtilsDelay(4000000);
                    while(1){
                        if (sw(2) || sw(3)){
                                json_message('1');
                                http_post(lRetVal);
                            break;
                        }
                    }
                    status = http_get(lRetVal);
                    while(status != 3){
                        status = http_get(lRetVal);
                    }
                    flag = 0;
                }

                status = http_get(lRetVal);
                if (status == 1){
                    print_lost();
                    MAP_UtilsDelay(4000000);
                    print_restart();
                    MAP_UtilsDelay(4000000);
                    while(1){
                        if (sw(2) || sw(3)){
                            json_message('3');
                            http_post(lRetVal);
                            break;
                        }
                    }
                    flag = 0;
                }

                if (status == 2){
                    print_win();
                    MAP_UtilsDelay(4000000);
                    print_restart();
                    MAP_UtilsDelay(4000000);
                    while(1){
                        if (sw(2) || sw(3)){
                                json_message('3');
                                http_post(lRetVal);
                            break;
                        }
                    }
                    flag = 0;
                }

                Report("%d", score_1);

                t = 0;

            }
            else{
                t++;
            }
        }
    }
}



//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
//AWS

static int http_post(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;
    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, POSTHEADER);
    pcBufHeaders += strlen(POSTHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");
    int dataLength = strlen(DATA1);
    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);
    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);
    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);
    strcpy(pcBufHeaders, DATA1);
    pcBufHeaders += strlen(DATA1);
    int testDataLength = strlen(pcBufHeaders);
    UART_PRINT(acSendBuff);
    //
    // Send the packet to the server
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }


return 0;
}

static int http_get(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, GETHEADER);
    pcBufHeaders += strlen(GETHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    int dataLength = strlen(DATA1);

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);
    strcpy(pcBufHeaders, DATA1);
    pcBufHeaders += strlen(DATA1);
    int testDataLength = strlen(pcBufHeaders);

    UART_PRINT(acSendBuff);


    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }

    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");

        Report("state received: %d\n", acRecvbuff[299]-'0');
        return acRecvbuff[299]-'0';
    }

    return 0;
}
