/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/kernel/src/platform/microops.c
Author: myaut

@LICENSE
*/

#include <platform/armv7m.h>
#include <platform/microops.h>
#include <config.h>

#ifdef CONFIG_SMP

/*For future use :-) */

/* Basic spinlock ops for ARMv7m architecture */
int spinlock_trylock(spinlock_t* sl) {
	int result = 0;	/* Assuming lock is busy*/

	__ASM volatile ("mov r1, #1");
	__ASM volatile ("mov r7, %0" : : "r" (sl) );
	__ASM volatile ("ldrex r0, [r7]" : );
	__ASM volatile ("cmp r0, #0");

	/* Lock isn't busy, trying to get it*/
	__ASM volatile ("itt eq\n"
					"strexeq r0, r1, [r7]\n"
					"moveq %0, r0"  : "=r"(result));

	return result;
}

void spinlock_lock(spinlock_t* sl) {
	while(spinlock_trylock(sl) != 1);
}

void spinlock_unlock(spinlock_t* sl) {
	__ASM volatile ("mov r1, #0");
	__ASM volatile ("spinlock_try: ldrex r0, [%0]\n"
					"strex r0, r1, [%0]\n"
					"cmp r0, #0"
						:
						: "r" (sl));
	__ASM volatile ("bne spinlock_try");
}

/* Atomic ops */
void atomic_set(atomic_t* atom, atomic_t newval) {
	__ASM volatile ("mov r1, %0" : : "r" (newval));
	__ASM volatile ("atomic_try: ldrex r0, [%0]\n"
					"strex r0, r1, [%0]\n"
					"cmp r0, #0"
					:
					: "r" (atom));
	__ASM volatile ("bne atomic_try");
}

uint32_t atomic_get(atomic_t* atom) {
	atomic_t result;

	__ASM volatile ("ldrex r0, [%0]"
					:
					: "r" (atom));
	__ASM volatile ("clrex");
	__ASM volatile ("mov %0, r0" : "=r" (result) );

	return result;
}

#else

void atomic_set(atomic_t* atom, atomic_t newval) {
	*atom = newval;
}

uint32_t atomic_get(atomic_t* atom) {
	return *atom;
}

#endif

/* returns zero on success*/
uint32_t test_and_set(uint32_t* atom) {
	int result = 1;

	__ASM volatile (
		"mov r1, #1\n"
		"mov r2, %0\n"
		"ldrex r0, [r2]\n"
		"cmp r0, #0\n"

		/* Lock isn't busy, trying to get it*/
		"itt eq\n"
		"strexeq r0, r1, [r2]\n"
		"moveq %1, r0\n"
			: "=r"(result)
			: "r"(atom)
			: "r0", "r1", "r2");

	return result;
}

void irq_disable(void) {
	__ASM volatile ("cpsid i");
}

void irq_enable(void) {
	__ASM volatile ("cpsie i");
}

int irq_number() {
	int irqno;

	__ASM volatile ( "mrs r0, ipsr\n"
					  "mov r0, %0" : "=r" (irqno) : : "r0");

	return irqno;
}
