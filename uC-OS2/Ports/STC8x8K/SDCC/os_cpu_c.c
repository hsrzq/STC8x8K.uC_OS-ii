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
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
* For      : STC8A8K64D4 / STC8H8K64U
* Toolchain: SDCC
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (OS_VERSION >= 281) && (OS_TMR_EN > 0)
static INT16U OSTmrCtr;
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
void OSInitHookBegin(void)
{
#if OS_VERSION >= 281 && OS_TMR_EN > 0
    OSTmrCtr = 0;
#endif
}

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
void OSInitHookEnd(void)
{
    OSTimerInit();
}

/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
void OSTaskIdleHook(void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskIdleHook();
#endif
}

/*
*********************************************************************************************************
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
void OSTCBInitHook(OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TCBInitHook(ptcb);
#else
    ptcb = ptcb;
#endif
}

/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskCreateHook(OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskCreateHook(ptcb);
#else
    ptcb = ptcb;
#endif
}

/*
*********************************************************************************************************
*                                            TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : ptcb      is a pointer to the task control block of the task that is returning.
*
* Note(s)    : none
*********************************************************************************************************
*/
void OSTaskReturnHook(OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskReturnHook(ptcb);
#else
    ptcb = ptcb;
#endif
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
void OSTaskSwHook(void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskSwHook();
#endif
}

/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_TIME_TICK_HOOK_EN > 0)
void OSTimeTickHook(void)
{
#if OS_APP_HOOKS_EN > 0
    App_TimeTickHook();
#endif

#if OS_VERSION >= 281 && OS_TMR_EN > 0
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif
}
#endif

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0
void OSTaskStatHook(void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskStatHook();
#endif
}
#endif

/*
**********************************************************************************************************
*                                       INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created. This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack. It is assumed that 'ptos' points to the
*                            highest valid address on the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
**********************************************************************************************************
*/
OS_STK *OSTaskStkInit(void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)
{
    opt;
    uint32_t ulAddress;
    OS_STK *stk = ptos;

    ulAddress = (uint32_t)task;             // 任务入口地址
    *(++stk)  = (OS_STK)(ulAddress & 0xFF); // 任务入口地址低字节
    *(++stk)  = (OS_STK)(ulAddress >> 8);   // 任务入口地址高字节
    *(++stk)  = 0x00;                       // ACC
    *(++stk)  = 0x80;                       // IE
    ulAddress = (uint32_t)p_arg;            // 任务参数地址
    *(++stk)  = (OS_STK)(ulAddress & 0xFF); // DPL
    *(++stk)  = (OS_STK)(ulAddress >> 8);   // DPH
    *(++stk)  = (OS_STK)(ulAddress >> 16);  // B
    *(++stk)  = 0x00;                       // R2
    *(++stk)  = 0x00;                       // R3
    *(++stk)  = 0x00;                       // R4
    *(++stk)  = 0x00;                       // R5
    *(++stk)  = 0x00;                       // R6
    *(++stk)  = 0x00;                       // R7
    *(++stk)  = 0x00;                       // R0
    *(++stk)  = 0x00;                       // R1
    *(++stk)  = 0x00;                       // PSW
    *(++stk)  = 0x00;                       // BP
    *ptos     = (OS_STK)(stk - ptos);       // 任务栈长度

    return ptos;
}

void OSTimerInit(void)
{
#if (OS_TICK_TIMER_1T == 1)
    AUXR |= 0x80; // 定时器时钟1T模式
    const uint16_t usTick = OS_CPU_MAIN_CLOCK / OS_TICKS_PER_SEC;
#else
    AUXR &= 0x7F; // 定时器时钟12T模式
    const uint16_t usTick = OS_CPU_MAIN_CLOCK / OS_TICKS_PER_SEC / 12;
#endif
    const uint16_t usInit = 0x10000 - usTick;

    TMR0 = usInit; // 定时器自动重装初值
    TMOD = 0x03;   // 定时器0工作在模式3，该模式下不支持中断嵌套
    ET0  = 1;      // 使能定时器中断
    TR0  = 1;      // 启动定时器
    // EA   = 1;      // 模式3无需打开总中断
}
