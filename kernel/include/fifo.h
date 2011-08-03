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

#define		FIFO_OK			0x0
#define		FIFO_OVERFLOW	0x1
#define		FIFO_EMPTY		0x2

/**
* Очередь
*/
struct fifo_t {
	uint8_t* q_data;	/*!< Указатель на первый байт очереди в xdata (область 256 байт) */
	uint32_t q_top;			/*!< Голова очереди*/
	uint32_t q_end;			/*!< Конец очереди*/
	uint32_t q_length;		/*!< Длина очереди*/
	size_t   q_size;		/*!< Размер очереди*/
};

//При достижении конца массива переполняется и переходит на начало.

/**
* Инициализирует очередь и устанавливает указатель
* @return Возвращает FIFO_OK (всегда)
* @param queue служебная структура очереди
* @param addr указатель на 256-байтную область в xdata для данных очереди
*/
uint32_t fifo_init(struct fifo_t* queue, uint8_t* addr, size_t size);

/**
* Помещает в очередь элемент
* @return Возвращает FIFO_OK в случае успеха или FIFO_OVERFLOW если очередь переполнена
* @param queue служебная структура очереди
* @param el Значение элемента
*/
uint32_t fifo_push(struct fifo_t* queue, uint8_t el);

/**
* Возвращает состояние очереди
* @return Возвращает FIFO_OVERFLOW если очередь переполнена, FIFO_EMPTY - если пуста
* или FIFO_OK во всех остальных случаях.
* @param queue служебная структура очереди
*/
uint32_t fifo_state(struct fifo_t* queue);

/**
* Извлекает элемент из очереди
* @return Возвращает FIFO_OK в случае успеха или FIFO_EMPTY если очередь пуста
* @param queue служебная структура очереди
* @param el куда помещать элемент
*/
uint32_t fifo_pop(struct fifo_t* queue, uint8_t* el);

/**
* Определяет количество элементов в очереди
* @return Возвращает количество элементов в очереди
* @param queue служебная структура очереди
*/
uint32_t fifo_length(struct fifo_t* queue);

#endif /* FIFO_H_ */
