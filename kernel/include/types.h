/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/include/types.h
Author: myaut

@LICENSE
*/

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <lpc/LPC17xx.h>

typedef uint32_t   ptr_t;
typedef uintptr_t memptr_t;

typedef uint32_t l4_thread_t;

#define NULL ((ptr_t) 0x0)

#endif /* TYPES_H_ */
