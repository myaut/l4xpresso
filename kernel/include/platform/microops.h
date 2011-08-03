/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/platform/microops.h
Author: myaut

@LICENSE
*/

#ifndef MICROOPS_H_
#define MICROOPS_H_

#include <types.h>

typedef uint32_t spinlock_t;
typedef	 uint32_t atomic_t;

#define SPINLOCK_INITIALIZER 	0x0

/* Basic spinlock ops for ARMv7m architecture */
int spinlock_trylock(spinlock_t* sl);
void spinlock_lock(spinlock_t* sl);
void spinlock_unlock(spinlock_t* sl);

/* Atomic ops */
void atomic_set(atomic_t* atom, atomic_t newval);
uint32_t atomic_get(atomic_t* atom);

uint32_t test_and_set(uint32_t* atom);

/*IRQ disable/enable*/
void irq_disable(void);
void irq_enable(void);

int irq_number();

#endif /* MICOROPS_H_ */
