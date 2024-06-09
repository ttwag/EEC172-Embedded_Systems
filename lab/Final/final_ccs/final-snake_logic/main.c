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
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "gpio.h"
#include "gpio_if.h"
#include "uart.h"
#include "spi.h"
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
#define SPI_IF_BIT_RATE  100000
#define SNAKE_SIZE            8
#define SNAKE_LENGTH          10
#define RIGHT   0
#define LEFT    1
#define UP      2
#define DOWN    3


#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

static void BoardInit(void);
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

//*****************************************************************************

//*****************************************************************************
//
//! SPI Initialization
//!
//! This function configures SPI modelue as master and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
void SPIInit()
{
    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

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
}

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

    // Reset the peripheral
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // Initialize SPI Communication
    SPIInit();


    // Initialize the OLED
    Adafruit_Init();
    fillScreen(BLACK);

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

    while (1) {
            // Check new direction
//            SW3 Pressed
            if (GPIOPinRead(GPIOA2_BASE, 0x40) & 0x40) {
                if (lastDirection == UP) direction = LEFT;
                else if (lastDirection == LEFT) direction = DOWN;
                else if (lastDirection == RIGHT) direction = UP;
                else direction = RIGHT;
            }

//            SW2 Pressed
            if (GPIOPinRead(GPIOA1_BASE, 0x20) & 0x20) {
                if (lastDirection == UP) direction = RIGHT;
                else if (lastDirection == LEFT) direction = UP;
                else if (lastDirection == RIGHT) direction = DOWN;
                else direction = LEFT;
            }
            lastDirection = direction;

            // Erase last position
            fillRect(snake->tail->coordinate[0], snake->tail->coordinate[1], 10, 10, BLACK);

            // Move the snake and draw new position
            move(snake, direction);

            // Check if the snake hits the wall
            int x = snake->head->coordinate[0];
            int y = snake->head->coordinate[1];

            if (x <= 0 || x >= 128 || y <= 0 || y >= 128) {
                print_lost();
                break;
            }
            fillRect(snake->head->coordinate[0], snake->head->coordinate[1], 10, 10, RED);


            // Fruits


            // Controls the frame rate
            MAP_UtilsDelay(4000000);
        }
     freeSnake(snake);
}

