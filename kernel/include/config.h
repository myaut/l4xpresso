/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/config.h
Author: myaut

@LICENSE
*/

#ifndef CONFIG_H_
#define CONFIG_H_

/*Debug options*/
#define CONFIG_DEBUG
#define CONFIG_KDB

/*Table sizes*/
enum {
	CONFIG_MAX_THREADS	= 32,
	CONFIG_MAX_KT_EVENTS	= 64,
	CONFIG_MAX_ASYNC_EVENTS	= 32
};

#define CONFIG_KTIMER_HEARTBEAT		4096
#define CONFIG_KTIMER_MINTICKS		128

#endif /* CONFIG_H_ */
