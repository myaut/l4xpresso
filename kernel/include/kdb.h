/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/kdb.h
Author: myaut

@LICENSE
*/

#ifndef KDB_H_
#define KDB_H_

#include <ktable.h>

/*Simple kernel debugger*/
int kdb_handler(char c);

void kdb_print_menu();

void kdb_register_ktable(ktable_t* kt);
void kdb_dump_ktable();

#endif /* KDB_H_ */
