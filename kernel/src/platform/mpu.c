/*
 * mpu.c
 *
 *  Created on: 25.09.2011
 *      Author: myaut
 */

#include <fpage.h>
#include <memory.h>
#include <error.h>
#include <platform/mpu.h>

void mpu_setup_region(int n, fpage_t* fp) {
	static uint32_t* mpu_base = (uint32_t*) MPU_BASE_ADDR;
	static uint32_t* mpu_attr = (uint32_t*) MPU_ATTR_ADDR;

	*mpu_base = (FPAGE_BASE(fp) & MPU_REGION_MASK) | 0x10 | (n & 0xF);
	*mpu_attr = ((mempool_getbyid(fp->fpage.mpid)->flags & MP_UX)? 0 : (1 << 28)) |	/*XN bit*/
			0x3  << 24 /*Full access*/ | ((fp->fpage.shift - 1) << 1) /*Region*/ | 1 /*Enable*/;
}

void mpu_enable(mpu_state_t i) {
	static uint32_t* mpu_ctrl = (uint32_t*) MPU_CTRL_ADDR;

	*mpu_ctrl = i | MPU_PRIVDEFENA;
}

void memmanage_handler(void) {
	uint32_t mmsr = *((uint32_t*) MPU_FAULT_STATUS_ADDR);
	uint32_t mmar = *((uint32_t*) MPU_FAULT_ADDRESS_ADDR);
	fpage_t* fp = thread_current()->as->first;

	if(mmsr & MPU_MEM_FAULT) {
		while(fp) {
			fp = fp->as_next;

			if(addr_in_fpage(mmar, fp)) {
				thread_current()->as->lru = fp;
				as_setup_mpu(thread_current()->as);

				return;
			}
		}
	}

	panic("Memory fault\n");
}
