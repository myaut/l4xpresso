/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/platform/irq.c
Author: myaut

@LICENSE
*/

#include <platform/armv7m.h>
#include <platform/irq.h>

volatile uint32_t irq_stack_pointer;
volatile uint32_t irq_exc_return;
volatile uint32_t irq_user_sp;
volatile uint32_t irq_window[8];
