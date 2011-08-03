/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/memory.c
Author: myaut

@LICENSE
*/

#include <memory.h>
#include <config.h>
#include <debug.h>
#include <ktable.h>

static mempool_t memmap[] = {
	DECLARE_MEMPOOL_2("KTEXT", kernel_text, MP_KR | MP_KX | MP_NO_FPAGE),
	DECLARE_MEMPOOL_2("UTEXT", user_text, MP_UR | MP_UX | MP_MEMPOOL | MP_MAP_ALWAYS),
	DECLARE_MEMPOOL_2("KDATA", kernel_data, MP_KR | MP_KW | MP_NO_FPAGE),
	DECLARE_MEMPOOL_2("KBSS", kernel_bss, MP_KR | MP_KW | MP_NO_FPAGE),
	DECLARE_MEMPOOL_2("UDATA", user_data, MP_UR | MP_UW | MP_MEMPOOL | MP_MAP_ALWAYS),
	DECLARE_MEMPOOL_2("UBSS", user_bss, MP_UR | MP_UW | MP_MEMPOOL  | MP_MAP_ALWAYS),
	DECLARE_MEMPOOL  ("MEM0", &user_bss_end, 0x10008000, MP_UR | MP_UW | MP_SRAM),
	DECLARE_MEMPOOL_2("KBITMAP", kernel_ahb, MP_KR | MP_KW | MP_NO_FPAGE),
	DECLARE_MEMPOOL  ("MEM1", &kernel_ahb_end, 0x20084000, MP_UR | MP_UW | MP_AHB_RAM),
	DECLARE_MEMPOOL  ("APBDEV", 0x40000000, 0x40100000, MP_UR | MP_UW | MP_DEVICES),
	DECLARE_MEMPOOL  ("AHBDEV", 0x50000000, 0x50200000, MP_UR | MP_UW | MP_DEVICES)
};

DECLARE_KTABLE(as_t, as_table, CONFIG_MAX_ADRESS_SPACES);
DECLARE_KTABLE(fpage_t, fpage_table, CONFIG_MAX_FPAGES);

void memory_init() {
	ktable_init(&as_table);
	ktable_init(&fpage_table);
}

memptr_t addr_align(memptr_t addr, uint32_t size) {
	if(addr & (size - 1))
		return (addr & ~(size - 1)) + size;
	else
		return (addr & ~(size - 1));
}

int fp_addr_log2(memptr_t addr) {
	int shift = 0;

	while ((addr <<= 1) != 0) ++shift;

	return 31 - shift;
}

/*
 * NOTE: as and fpage chain couldn't overlap
 * if they do, use insert_fpage_to_as which checks every fpage
 * */
void insert_fpage_chain_to_as(as_t* as, fpage_t* first, fpage_t* last) {
	fpage_t* p = as->first;

	if(!p) {
		as->first = first;
		return;
	}

	if(last->fpage.base < p->fpage.base) {
		/*Map into beginning*/
		last->as_next = as->first;
		as->first = first;
	}
	else {
		while(p->as_next != NULL) {
			if(last->fpage.base < p->as_next->fpage.base) {
				last->as_next = p->as_next;
				break;
			}

			p = p->as_next;
		}
	}

	p->as_next = first;
}

void insert_fpage_to_as(as_t* as, fpage_t* fpage) {
	insert_fpage_chain_to_as(as, fpage, fpage);
}

fpage_t* create_fpage(mempool_id_t mpid, as_t* as, memptr_t base, size_t shift) {
	fpage_t* fpage = (fpage_t*) ktable_alloc(&fpage_table);

	fpage->as = as;
	fpage->as_next = NULL;
	fpage->map_next = fpage; 	/*That is first fpage in mapping*/
	fpage->fpage.mpid = mpid;
	fpage->fpage.flags = 0;
	fpage->fpage.rwx = MP_USER_PERM(memmap[mpid].flags);

	fpage->fpage.base = base;
	fpage->fpage.shift = shift;

	if(memmap[mpid].flags & MP_MAP_ALWAYS)
		fpage->fpage.flags |= FPAGE_ALWAYS;

	return fpage;
}

int create_fpages(mempool_id_t mpid, as_t* as, memptr_t base, memptr_t size) {
	int i, shift, bshift, sshift;
	fpage_t *fpage = NULL, *first = NULL, *last = NULL;

	/*if mpid is unknown, search using base addr*/
	if(mpid == MP_UNKNOWN) {
		for(i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
			if(memmap[i].start <= base && memmap[i].end >= (base + size) ) {
				mpid = i;
				break;
			}
		}
		if(mpid == MP_UNKNOWN) {
			/* Cannot find appropriate mempool, return NULL*/
			return -1;
		}
	}

	switch(memmap[mpid].flags & MP_FPAGE_MASK) {
	case MP_MEMPOOL:
		base = addr_align((memmap[mpid].start), CONFIG_LEAST_FPAGE_SIZE);
		size = addr_align((memmap[mpid].end - memmap[mpid].start), CONFIG_LEAST_FPAGE_SIZE);
		break;
	case MP_SRAM:
	case MP_AHB_RAM:
		base = addr_align(base, CONFIG_LEAST_FPAGE_SIZE);
		size = addr_align(size, CONFIG_LEAST_FPAGE_SIZE);
		break;
	case MP_DEVICES:
		base = base & 0xFFFC0000;
		size = size & 0xFFFC0000;
		break;
	}

	if(size == 0)
		return -2;

	/* Create chain of fpages
	 *
	 * MPU in Cortex-M3 support only regions of 2 ^ n size, so if we want create page of 96 bytes for example,
	 * we should split it into smaller ones
	 * */

	dbg_printf(DL_MEMORY, "MEM: fpage chain %s [b:%p, sz:%p] as %p\n", memmap[mpid].name, base, size, as);

	while(size) {
		/* Select lease of log2(base),log2(size). Needed to make regions with correct align*/
		bshift = fp_addr_log2(base);
		sshift = fp_addr_log2(size);

		shift = ((1 << bshift) > size)? sshift : bshift;

		if(!first) {
			first = create_fpage(mpid, as, base, shift);
			fpage = first;
		}
		else {
			fpage->as_next = create_fpage(mpid, as, base, shift);
			last = fpage;
			fpage = fpage->as_next;
		}
		size -= (1 << shift);
		base += (1 << shift);
	}

	if(last)
		insert_fpage_chain_to_as(as, first, last);
	else
		insert_fpage_to_as(as, first);

	return 0;
}

/*
 * Should be platform functions
 */
void mpu_setup_region(int n, fpage_t* fp) {
	static uint32_t* mpu_base = (uint32_t*) 0xE000ED9C;
	static uint32_t* mpu_attr = (uint32_t*) 0xE000EDA0;

	*mpu_base = (fp->fpage.base & 0xFFFFFFE0) | 0x10 | (n & 0xF);
	*mpu_attr = ((memmap[fp->fpage.mpid].flags & MP_UX)? 0 : (1 << 28)) |	/*XN bit*/
			0x3  << 24 /*Full access*/ | ((fp->fpage.shift - 1) << 1) /*Region*/ | 1 /*Enable*/;
}

void mpu_enable(mpu_state_t i) {
	static uint32_t* mpu_ctrl = (uint32_t*) 0xE000ED94;

	*mpu_ctrl = i;
}

void as_setup_mpu(as_t* as) {
	fpage_t* mpu[8];
	char lru = 0;		/*Is fpage lru?*/
	fpage_t* fp = as->first;
	int i = 0, j = 7, k = 0;

	/*
	 * We walk thru fpage list
	 * [0:k] are always-mapped fpages
	 * [k:7] are LRU fpages
	 *
	 * We clear all lru bits in fpages list
	 * When memfault occurs, it sets lru flag in fpage flags
	 * */

	while(fp != NULL) {
		if(fp->fpage.flags & FPAGE_ALWAYS) {
			if(k < 8)
				mpu[k++] = fp;
		}
		else {
			if(j >= k) {
				mpu[j--] = fp;
				if(fp->fpage.flags & FPAGE_LRU)
					lru |= 1 << j;
			}
			else if(fp->fpage.flags & FPAGE_LRU) {
				/*Slots exhausted, search first non-LRU fpage and replace it*/
				for(i = k; i < 8; ++i) {
					if(!(lru & (1 << i))) {
						lru |= 1 << i;
						mpu[i] = fp;
					}
				}
			}
		} /*Non-always fpage mapping*/

		fp->fpage.flags &= ~FPAGE_LRU;
		fp = fp->as_next;
	}

	for(i = 0; i < k; ++i) {
		mpu_setup_region(i, mpu[i]);
	}

	for(i = 7; i > j; --i) {
		mpu_setup_region(i, mpu[i]);
	}
}

as_t* create_as(uint32_t as_spaceid) {
	as_t* as = (as_t*) ktable_alloc(&as_table);
	int i;

	/*assert as == NULL*/

	as->as_spaceid = as_spaceid;
	as->first = NULL;

	if(as_spaceid == 0) {
		/* Create entire space */

		for(i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
			if(memmap[i].flags & MP_UR) {
				/* For all user readable memory pools create fpage*/
				create_fpages(i, as, memmap[i].start, (memmap[i].end - memmap[i].start));
			}
		}
	}

	return as;
}

int map_fpage(as_t* as, fpage_t* fpage, map_action_t action) {
	/*NOT IMPLEMENTED*/
	return -1;
}

#ifdef CONFIG_KDB

char* kdb_mempool_prop(mempool_t* mp) {
	static char mempool[10] = "--- --- -";
	mempool[0] = (mp->flags & MP_KR)? 'r' : '-';
	mempool[1] = (mp->flags & MP_KW)? 'w' : '-';
	mempool[2] = (mp->flags & MP_KX)? 'x' : '-';

	mempool[4] = (mp->flags & MP_UR)? 'r' : '-';
	mempool[5] = (mp->flags & MP_UW)? 'w' : '-';
	mempool[6] = (mp->flags & MP_UX)? 'x' : '-';

	mempool[8] = (mp->flags & MP_DEVICES)? 'D' :
					(mp->flags & MP_MEMPOOL)? 'M' :
					(mp->flags & MP_AHB_RAM)? 'A' :
					(mp->flags & MP_SRAM)? 'S' : 'N';
	return mempool;
}

void kdb_dump_mempool() {
	int i = 0;

	dbg_printf(DL_KDB, "%8s %8s [%8s:%8s] %10s\n", "NAME", "SIZE", "START", "END", "FLAGS");

	for(i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		dbg_printf(DL_KDB, "%8s %8d [%p:%p] %10s\n", memmap[i].name, (memmap[i].end - memmap[i].start),
					memmap[i].start, memmap[i].end, kdb_mempool_prop(&(memmap[i])));
	}
}

void kdb_dump_as() {
	int idx = 0, nl = 0;
	as_t* as = NULL;
	fpage_t* fpage = NULL;

	for_each_in_ktable(as, idx, &as_table) {
		fpage = as->first;
		dbg_printf(DL_KDB, "Address Space %p\n", as->as_spaceid);

		while(fpage) {
			dbg_printf(DL_MEMORY, "MEM: fpage %s [b:%p, sz:2**%d]\n", memmap[fpage->fpage.mpid].name,
						fpage->fpage.base, fpage->fpage.shift);
			fpage = fpage->as_next;
			++nl;

			if(nl == 12) {
				dbg_puts("Press any key...\n");
				while(dbg_getchar() == 0);
				nl = 0;
			}
		}
	}
}

#endif
