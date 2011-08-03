/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/kip.c
Author: myaut

@LICENSE
*/

#include <kip.h>
#include <config.h>
#include <platform/link.h>

kip_t kip __KIP = {
	.kernel_id = 0x00000000,
	.api_version = 0x8507000,	/*Ver. M.2 Rev. 1*/
	.api_flags	= 0x00000000,	/*Little endian 32-bit*/
};

/*Extra information on KIP*/
char __kip_extra[CONFIG_KIP_EXTRA_SIZE] __KIP = "";
char* kip_extra = &__kip_extra[0];

kip_mem_desc_t* mem_desc = NULL;
