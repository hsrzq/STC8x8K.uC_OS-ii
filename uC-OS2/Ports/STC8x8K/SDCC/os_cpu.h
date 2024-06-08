/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                       MCS-51 STC8x8K Specific code
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
* For      : STC8A8K64D4 / STC8H8K64U
* Toolchain: SDCC
*********************************************************************************************************
*/

#ifdef OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT extern
#endif

#include "STC8x8K.h"

/*
**********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
**********************************************************************************************************
*/

typedef unsigned char BOOLEAN;     /* 8-bit  boolean or logical true/false (TRUE/FALSE)   */
typedef unsigned char INT8U;       /* Unsigned  8 bit quantity                            */
typedef signed char INT8S;         /* Signed    8 bit quantity                            */
typedef unsigned int INT16U;       /* Unsigned 16 bit quantity                            */
typedef signed int INT16S;         /* Signed   16 bit quantity                            */
typedef unsigned long INT32U;      /* Unsigned 32 bit quantity                            */
typedef signed long INT32S;        /* Signed   32 bit quantity                            */
typedef unsigned long long INT64U; /* Unsigned 64 bit quantity                            */
typedef signed long long INT64S;   /* Signed   64 bit quantity                            */
typedef float FP32;                /* Single precision floating point                     */

typedef unsigned char OS_STK;      /* Each stack entry is 8-bit wide                      */
typedef unsigned char OS_CPU_SR;   /* Define size of CPU status register (PSW = 8 bits)   */

/*
*********************************************************************************************************
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts. In other words, if
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section. The IAR compiler does not support inline assembly so I'm
*             using the _OPC() intrinsic function. Here are the instructions:
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/

#define OS_CRITICAL_METHOD 1

#if OS_CRITICAL_METHOD == 1
#define OS_ENTER_CRITICAL() \
    do {                    \
        EA = 0;             \
    } while (0)
#define OS_EXIT_CRITICAL() \
    do {                   \
        EA = 1;            \
    } while (0)
#elif OS_CRITICAL_METHOD == 2
#error "OS_CRITICAL_METHOD == 2 is NOT supported yet."
#elif OS_CRITICAL_METHOD == 3
#error "OS_CRITICAL_METHOD == 3 is NOT supported yet."
#endif

/*
*********************************************************************************************************
*                                            Miscellaneous
*********************************************************************************************************
*/

#define OS_STK_GROWTH 0 /* Stack grows from LOW to HIGH memory on MCS-51 */
#define OS_TASK_SW()  OSCtxSw()

/*
**********************************************************************************************************
*                                         Function Prototypes
**********************************************************************************************************
*/

void OSStartHighRdy(void) __naked;
void OSCtxSw(void) __naked;
void OSIntCtxSw(void) __naked;

/*
**********************************************************************************************************
*                                         OS Timer Functions
**********************************************************************************************************
*/

void OSTimerInit(void);
void OSTimer0ISR(void) __interrupt(1) __naked;
