/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/ktable.h
Author: myaut

@LICENSE
*/

#ifndef KTABLE_H_
#define KTABLE_H_

#include <platform/armv7m.h>
#include <platform/link.h>
#include <types.h>

struct ktable
{
	char* tname;
	ptr_t bitmap;
	ptr_t data;
	size_t num;
	size_t size;
};

typedef struct ktable ktable_t;

#define DECLARE_KTABLE(type, name, num_)	 				\
	static __BITMAP char kt_ ## name ## _bitmap[num_ / 8];	\
	static __KTABLE type kt_ ## name ## _data[num_];		\
	ktable_t name = {										\
			.tname = #name,									\
			.bitmap = (ptr_t) kt_ ## name ## _bitmap,		\
			.data = (ptr_t) kt_ ## name ## _data,			\
			.num = num_, .size = sizeof(type) 				\
	}

void  ktable_init(ktable_t* kt);
int ktable_is_allocated(ktable_t* kt, int i);
void* ktable_alloc_id(ktable_t* kt, int i);
void* ktable_alloc(ktable_t* kt);
void  ktable_free(ktable_t* kt, void* element);

uint32_t ktable_getid(ktable_t* kt, void* element);

/*
 * Should be used as:
 *
 * type* el;
 * int idx;
 *
 * for_each_in_ktable(el, idx, my_ktable) {
 * 	 ...
 * }
 * */
#define for_each_in_ktable(el, idx, kt)	for(el = (typeof(el)) (kt)->data, idx = 0; idx < (kt)->num; ++idx, ++el) \
		if(((char*) BIT_ADDR((kt)->bitmap))[idx << BIT_SHIFT] == 1)

#endif /* KTABLE_H_ */
