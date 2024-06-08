.module os_cpu_a
.optsdcc -mmcs51 --model-large

.globl _OSStartHighRdy
.globl _OSCtxSw
.globl _OSIntCtxSw
.globl _OSTimer0ISR

ar0 = 0x00
ar1 = 0x01
ar2 = 0x02
ar3 = 0x03
ar4 = 0x04
ar5 = 0x05
ar6 = 0x06
ar7 = 0x07

.area ISEG    (DATA)

.area XSEG    (XDATA)

.area CSEG    (CODE)

; -----------------------------------------------------------
; 宏：所有寄存器入栈
; -----------------------------------------------------------
.macro PSH_ALL_REGISTERS
    push    acc
    push    ie
    clr     ea
    push    dpl
    push    dph
    push    b
    push    ar2
    push    ar3
    push    ar4
    push    ar5
    push    ar6
    push    ar7
    push    ar0
    push    ar1
    push    psw
    clr     psw
    push    _bp
.endm

; -----------------------------------------------------------
; 宏：所有寄存器出栈
; -----------------------------------------------------------
.macro POP_ALL_REGISTERS
    pop     _bp
    pop     psw
    pop     ar1
    pop     ar0
    pop     ar7
    pop     ar6
    pop     ar5
    pop     ar4
    pop     ar3
    pop     ar2
    pop     b
    pop     dph
    pop     dpl
    pop     acc
    clr     ea
    jnb     acc.7, 1000$
    setb    ea
1000$:
    pop     acc
.endm

; -----------------------------------------------------------
; 宏：获取硬件栈和任务堆栈指针
; R0       = __start__stack - 1
; (R2, R3) = OSTCBCur
; (R5, R6) = OSTCBCur -> OSTCBStkPtr
; DPTR     = (R5, R6, R7)
; -----------------------------------------------------------
.macro OBTAIN_STACK_ADDRESS
    ; R0 = __start__stack - 1
    mov     r0, #(__start__stack - 1)
    ; (R2, R3) = OSTCBCur
    mov     dptr, #_OSTCBCur
    movx    a, @dptr
    mov     r2, a
    inc     dptr
    movx    a, @dptr
    mov     r3, a
    ; (R5, R6) = OSTCBCur -> OSTCBStkPtr
    mov     dpl, r2
    mov     dph, r3
    movx    a, @dptr
    mov     r5, a
    inc     dptr
    movx    a, @dptr
    mov     r6, a
    ; DPTR = (R5, R6)
    mov     dpl, r5
    mov     dph, r6
.endm

; -----------------------------------------------------------
; 宏：将任务堆栈内容拷贝到XRAM
; R1 = 任务堆栈的长度
; -----------------------------------------------------------
.macro COPY_STACK_TO_XRAM
    OBTAIN_STACK_ADDRESS
    ; R1 = SP - (__start__stack - 1)
    mov     a, sp
    clr     c
    subb    a, r0
    mov     r1, a
    ; DPTR[0] = R1
    movx    @dptr, a
    ; *(++DPTR) = *(++R0)
2000$:
    inc     r0
    mov     a, @r0
    inc     dptr
    movx    @dptr, a
    djnz    r1, 2000$
.endm

; -----------------------------------------------------------
; 宏：将XRAM内容拷贝到任务堆栈
; R1 = 任务堆栈的长度
; -----------------------------------------------------------
.macro COPY_XRAM_TO_STACK
    OBTAIN_STACK_ADDRESS
    ; R1 = DPTR[0]
    movx    a, @dptr
    mov     r1, a
    ; *(++R0) = *(++DPTR)
3000$:
    inc     dptr
    movx    a, @dptr
    inc     r0
    mov     @r0, a
    djnz    r1, 3000$
    ; SP = R0
    mov     sp, r0
.endm

; -----------------------------------------------------------
; 调用Hook函数OSTaskSwHook()
; OSRunning = TRUE
; 恢复栈：PTR_STK = OSTCBHighRdy->OSTCBStkPtr
; 从栈中恢复寄存器
; -----------------------------------------------------------
_OSStartHighRdy:
    lcall   _OSTaskSwHook
    ; OSRunning = TRUE
    mov     dptr, #_OSRunning
    mov     a, #0x01
    movx    @dptr, a
    COPY_XRAM_TO_STACK
    POP_ALL_REGISTERS
    ret

; -----------------------------------------------------------
; 保存寄存器到栈中
; 保存栈：OSTCBCur -> OSTCBStkPtr = PTR_STK
; 调用Hook函数OSTaskSwHook()
; OSTCBCur  = OSTCBHighRdy
; OSPrioCur = OSPrioHighRdy
; 恢复栈：PTR_STK = OSTCBHighRdy->OSTCBStkPtr
; 从栈中恢复寄存器
; -----------------------------------------------------------
_OSCtxSw:
    PSH_ALL_REGISTERS
    ; 可复用的任务切换代码块
block_switchTaskReally:
    ; OSTCBCur -> OSTCBStkPtr = PTR_STK
    COPY_STACK_TO_XRAM
    ; OSTaskSwHook()
    lcall   _OSTaskSwHook
    ; OSTCBCur = OSTCBHighRdy
    mov     dptr, #_OSTCBHighRdy
    movx    a, @dptr
    mov     r6, a
    inc     dptr
    movx    a, @dptr
    mov     r7, a
    mov     dptr, #_OSTCBCur
    mov     a, r6
    movx    @dptr, a
    mov     a, r7
    inc     dptr
    movx    @dptr, a
    ; OSPrioCur = OSPrioHighRdy
    mov     dptr, #_OSPrioHighRdy
    movx    a, @dptr
    mov     dptr, #_OSPrioCur
    movx    @dptr, a
    ; PTR_STK = _OSTCBCur->OSTCBStkPtr
    COPY_XRAM_TO_STACK
    POP_ALL_REGISTERS
    ret

_OSIntCtxSw:
    ; 调整SP指针，去掉在调用OSIntExit()、OSIntCtxSw()过程中压栈的多余内容
    ; SP = SP - 4
    mov     a, sp
    clr     c
    subb    a, #0x04
    mov     sp, a
    ; 退出中断后，执行任务切换
    mov     dptr, #block_switchTaskReally
    push    dpl
    push    dph
    reti

; -----------------------------------------------------------
; 定时器0中断服务程序，用于实现系统时钟节拍
; -----------------------------------------------------------
_OSTimer0ISR:
    PSH_ALL_REGISTERS
    lcall   _OSIntEnter
    lcall   _OSTimeTick
    lcall   _OSIntExit
    POP_ALL_REGISTERS
    reti
