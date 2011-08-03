/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/kip.c
Author: myaut

@LICENSE
*/

#include <kip.h>
#include <platform/link.h>

kip_t kip __KIP = {
	.kernel_id = 'L4\230K',		/*ITMO*/
	.api_version = 0x8507000,	/*Ver. M.2 Rev. 1*/
	.api_flags	= 0x00000000,	/*Little endian 32-bit*/
};

/*Extra information on KIP*/
char kip_extra[128] __KIP;
kip_mem_desc_t* mem_desc = kip_extra;
