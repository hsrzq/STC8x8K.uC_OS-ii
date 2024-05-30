#include "includes.h"

#define TASK_STK_SIZE 128

__sfr __at(0x93) P0M1;
__sfr __at(0x94) P0M0;

// ==== 实时任务堆栈定义 ============
OS_STK Task_STK_0[TASK_STK_SIZE];
OS_STK Task_STK_1[TASK_STK_SIZE];
OS_STK Task_STK_2[TASK_STK_SIZE];

void Port0Init(void)
{
    /* 设置P0为强推挽输出 */
    P0M0 = 0xff;
    P0M1 = 0x00;
    P0   = 0x00;
}

void FLOPCheck0(void *p_arg)
{
    p_arg;
    while (1) {
        OSTimeDly(500);
        P0_0 = !P0_0;
    }
}

void FLOPCheck1(void *p_arg)
{
    p_arg;
    while (1) {
        OSTimeDly(999);
        P0_1 = 1;
        OSTimeDly(1);
        P0_1 = 0;
    }
}

void FLOPCheck2(void *p_arg)
{
    p_arg;
    while (1) {
        OSTimeDly(4500);
        P0_2 = 1;
        OSTimeDly(500);
        P0_2 = 0;
    }
}

void main(void)
{
    Port0Init();

    OSInit();
    OSTaskCreate(FLOPCheck0, (void *)0, &Task_STK_0[0], 2);
    OSTaskCreate(FLOPCheck1, (void *)0, &Task_STK_1[0], 3);
    OSTaskCreate(FLOPCheck2, (void *)0, &Task_STK_2[0], 4);
    OSStart();
}
