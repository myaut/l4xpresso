/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/include/memory.h
Author: myaut

@LICENSE
*/

#ifndef MEMORY_H_
#define MEMORY_H_

#include <types.h>
#include <platform/link.h>

typedef struct {


} memseg_t;

typedef struct {
	char*	 name;

	memptr_t base;
	memptr_t size;

	memseg_t* fpage;
	uint32_t fpage_size;
};

/*If fpage_size = 0, memory is not allocable*/
#define DECLARE_MEMPOOL(pool_name, start, end, fpsz) 	\
	{													\
		.name = pool_name,								\
		.base = (memptr_t) (start),						\
		.size = (memptr_t) (end - start),				\
		.fpage = NULL,									\
		.fpage_size = 0									\
	}

#define DECLARE_MEMPOOL_2(pool_name, prefix, fpsz) DECLARE_MEMPOOL(pool_name, &(prefix ## _start), &(prefix ## _end), fpsz)

/*
 * Memory areas
 */

#endif /* MEMORY_H_ */
