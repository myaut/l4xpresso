/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/fifo.h
Author: myaut

@LICENSE
*/

#ifndef FIFO_H_
#define FIFO_H_

#include <types.h>

#define FIFO(type_, size_) 			\
	struct {						\
		uint32_t top;				\
		uint32_t end;				\
		uint32_t length;			\
		type_ data[size_];			\
	}

#define FIFO_PUT(q, el, size) 	 	\
	if(q.length < size) { 			\
		++q.length; 				\
		q.data[++q.end] = el; 		\
	}

#define FIFO_GET(el, q, size) 	 	\
		if(q.length > 0) { 			\
			--q.length; 			\
			el = q.data[++q.top]; 	\
		}

#endif /* FIFO_H_ */
