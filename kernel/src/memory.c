/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/kernel/src/memory.c
Author: myaut

@LICENSE
*/

#include <memory.h>
#include <config.h>
#include <debug.h>
#include <ktable.h>

#include <kip.h>

/**
 * @file    memory.c
 * @brief   Memory management subsystem.
 *
 * Unlike traditional L4 kernels, built for "large systems" we work with small MCU, so:
 * 		- We don't have virtual memory and pages
 * 		- RAM is small, but physical address space is big (32-bit), and it include
 * 		  devices, flash, bit-bang regions
 * 		- We have memory protection unit with only 8 regions
 *
 * Memory management is split into three conceptions:
 * 		- Memory pool (mempool_t), which represent area of PAS with specific attributes
 * 		  (hardcoded in memmap table),
 * 		- Flexible page (fpage_t) - unlike traditional fpages in L4, they represent MPU region
 * 		- Address space (as_t) - sorted list of fpages bound to specific thread(s)
 *
 * MPU in Cortex-M3 support only regions of 2 ^ n size, so if we want create page of 96 bytes for example,
 * we should split it into smaller ones, and create fpage chain consisting of 32 byte and 64 byte fpages
 * That why make code very complicated
 *
 * Obvious way is to use regions of standard size (e.g. 128 bytes), but it is very wasteful
 * in terms of memory faults (we have only 8 regions for MPU, so for large ASes, we will often
 * receive memmanage faults), and fpage table.
 * */

/**
 * Memory map of MPU.
 * Translated into memdesc array in KIP by memory_init
 * */
static mempool_t memmap[] = {
	DECLARE_MEMPOOL_2("KTEXT", kernel_text, MP_KR | MP_KX | MP_NO_FPAGE, MPT_KERNEL_TEXT),
	DECLARE_MEMPOOL_2("UTEXT", user_text, MP_UR | MP_UX | MP_MEMPOOL | MP_MAP_ALWAYS, MPT_USER_TEXT),
	DECLARE_MEMPOOL_2("KIP", kip, MP_KR | MP_KW | MP_UR | MP_SRAM, MPT_KERNEL_DATA),
	DECLARE_MEMPOOL  ("KDATA", &kip_end, &kernel_data_end, MP_KR | MP_KW | MP_NO_FPAGE, MPT_KERNEL_DATA),
	DECLARE_MEMPOOL_2("KBSS",  kernel_bss, MP_KR | MP_KW | MP_NO_FPAGE, MPT_KERNEL_DATA),
	DECLARE_MEMPOOL_2("UDATA", user_data, MP_UR | MP_UW | MP_MEMPOOL | MP_MAP_ALWAYS, MPT_USER_DATA),
	DECLARE_MEMPOOL_2("UBSS",  user_bss, MP_UR | MP_UW | MP_MEMPOOL  | MP_MAP_ALWAYS, MPT_USER_DATA),
	DECLARE_MEMPOOL  ("MEM0",  &user_bss_end, 0x10008000, MP_UR | MP_UW | MP_SRAM, MPT_AVAILABLE),
	DECLARE_MEMPOOL_2("KBITMAP", kernel_ahb, MP_KR | MP_KW | MP_NO_FPAGE, MPT_KERNEL_DATA),
	DECLARE_MEMPOOL  ("MEM1",   &kernel_ahb_end, 0x20084000, MP_UR | MP_UW | MP_AHB_RAM, MPT_AVAILABLE),
	DECLARE_MEMPOOL  ("APBDEV", 0x40000000, 0x40100000, MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
	DECLARE_MEMPOOL  ("AHBDEV", 0x50000000, 0x50200000, MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES)
};

DECLARE_KTABLE(as_t, as_table, CONFIG_MAX_ADRESS_SPACES);
DECLARE_KTABLE(fpage_t, fpage_table, CONFIG_MAX_FPAGES);

extern kip_mem_desc_t* mem_desc;
extern char* kip_extra;

/* -------------------------------------
 * Some helper functions
 * ------------------------------------- */

memptr_t addr_align(memptr_t addr, size_t size) {
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

int addr_in_fpage(memptr_t addr, fpage_t* fpage) {
	return (addr >= fpage->fpage.base &&
			 addr <= fpage->fpage.base + (1 << fpage->fpage.shift));
}

memptr_t mempool_align(int mpid, memptr_t addr) {
	switch(memmap[mpid].flags & MP_FPAGE_MASK) {
	case MP_MEMPOOL:
	case MP_SRAM:
	case MP_AHB_RAM:
		return addr_align(addr, CONFIG_LEAST_FPAGE_SIZE);
	case MP_DEVICES:
		return addr & 0xFFFC0000;
	}

	return addr_align(addr, CONFIG_LEAST_FPAGE_SIZE);
}

void memory_init() {
	int i = 0, j = 0;

	ktable_init(&as_table);
	ktable_init(&fpage_table);

	mem_desc = (kip_mem_desc_t*) kip_extra;

	/* Initialize mempool table in KIP */
	for(i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		switch(memmap[i].tag) {
		case MPT_USER_DATA:
		case MPT_USER_TEXT:
		case MPT_DEVICES:
		case MPT_AVAILABLE:
			mem_desc[j++].base = addr_align((memmap[i].start), CONFIG_LEAST_FPAGE_SIZE) | i;
			mem_desc[j++].size = addr_align((memmap[i].end - memmap[i].start), CONFIG_LEAST_FPAGE_SIZE) |
					memmap[i].tag;
			j++;
		}
	}

	kip.memory_info.s.memory_desc_ptr = (uint32_t) mem_desc;
	kip.memory_info.s.n 			  = j;
}

/* -------------------------------------
 * Fpage && fpage chain functions
 * ------------------------------------- */

/**
 * Insert chain of fpages into address space
 * @param first,last (first, last) - fpage chain
 * @param as address space
 * 
 * NOTE: as and fpage chain couldn't overlap
 * if they do, use insert_fpage_to_as which checks every fpage
 * */
void insert_fpage_chain_to_as(as_t* as, fpage_t* first, fpage_t* last) {
	fpage_t* p = as->first;

	if(!p) {
		/*First chain in AS*/
		as->first = first;
		return;
	}

	if(last->fpage.base < p->fpage.base) {
		/*Add chain into beginning*/
		last->as_next = as->first;
		as->first = first;
	}
	else {
		/*Search for chain in the middle*/
		while(p->as_next != NULL) {
			if(last->fpage.base < p->as_next->fpage.base) {
				last->as_next = p->as_next;
				break;
			}

			p = p->as_next;
		}

		p->as_next = first;
	}
}

void insert_fpage_to_as(as_t* as, fpage_t* fpage) {
	insert_fpage_chain_to_as(as, fpage, fpage);
}

void remove_fpage_from_as(as_t* as, fpage_t* fp) {
	fpage_t* fpprev = as->first;

	while(fpprev->as_next != fp) {
		/*Fpage from wrong AS*/
		if(fpprev->as_next == NULL)
			return;

		fpprev = fpprev->as_next;
	}

	/*Remove from chain*/
	fpprev->as_next = fp->as_next;
}

/* FIXME: Support for bit-bang regions! */

fpage_t* create_fpage(memptr_t base, size_t shift, as_t* as, int mpid) {
	fpage_t* fpage = (fpage_t*) ktable_alloc(&fpage_table);

	/*FIXME: check for fpage == NULL*/
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

void create_fpage_chain(memptr_t base, size_t size, as_t* as, int mpid, fpage_t** pfirst, fpage_t** plast) {
	int shift, sshift, bshift;
	fpage_t* fpage = NULL;

	while(size) {
		/* Select lease of log2(base),log2(size). Needed to make regions with correct align*/
		bshift = fp_addr_log2(base);
		sshift = fp_addr_log2(size);

		shift = ((1 << bshift) > size)? sshift : bshift;

		if(!*pfirst) {
			/*Create first page*/
			fpage = create_fpage(base, shift, as, mpid);
			*pfirst = fpage;
			*plast = fpage;
		}
		else {
			/*Build chain*/
			fpage->as_next = create_fpage(base, shift, as, mpid);
			*plast = fpage;
			fpage = fpage->as_next;
		}

		size -= (1 << shift);
		base += (1 << shift);
	}
}

fpage_t* split_fpage(as_t* as, fpage_t* fpage, memptr_t split, int rl) {
	memptr_t base = fpage->fpage.base,
			 end = fpage->fpage.base + (1 << fpage->fpage.shift);
	fpage_t *lfirst, *llast, *rfirst, *rlast;
	split = mempool_align(fpage->fpage.mpid, split);

	if(!as)
		return NULL;

	/*Check if we can split something*/
	if(split == base || split == end) {
		return fpage;
	}

	if(fpage->map_next != fpage) {
		/*Splitting not supported for mapped pages*/
		/*UNIMPLIMENTED*/
		return NULL;
	}

	/*Split fpage into two chains of fpages*/
	create_fpage_chain(base, (split - base), as, fpage->fpage.mpid, &lfirst, &llast);
	create_fpage_chain(split,(end - split),  as, fpage->fpage.mpid, &rfirst, &rlast);

	remove_fpage_from_as(as, fpage);
	ktable_free(&fpage_table, fpage);

	insert_fpage_chain_to_as(as, lfirst, llast);
	insert_fpage_chain_to_as(as, rfirst, rlast);

	if(rl == 0) return lfirst;
	else return llast;
}

int mempool_search(memptr_t base, size_t size) {
	int i;

	for(i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		if(memmap[i].start <= base && memmap[i].end >= (base + size) ) {
			return i;
		}
	}
	return -1;
}

int create_fpages_ext(int mpid, as_t* as, memptr_t base, size_t size, fpage_t** pfirst,
		fpage_t** plast) {

	if(size == 0)
		return -2;

	/*if mpid is unknown, search using base addr*/
	if(mpid == -1) {
		if((mpid = mempool_search(base, size)) == -1) {
			/* Cannot find appropriate mempool, return error*/
			return -1;
		}
	}

	dbg_printf(DL_MEMORY, "MEM: fpage chain %s [b:%p, sz:%p] as %p\n", memmap[mpid].name, base, size, as);

	create_fpage_chain(mempool_align(mpid, base), mempool_align(mpid, size), as, mpid, pfirst, plast);

	if(as)
		insert_fpage_chain_to_as(as, *pfirst, *plast);

	return 0;
}

int create_fpages(as_t* as, memptr_t base, size_t size) {
	fpage_t *first = NULL, *last = NULL;
	return create_fpages_ext(-1, as, base, size, &first, &last);
}

/*
 * Should be platform-specific functions
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

	/*0x4 is PRIVDEFENA bit that allows access from kernel*/
	*mpu_ctrl = i | 0x4;
}

/* -------------------------------------
 * AS functions
 * ------------------------------------- */

void as_setup_mpu(as_t* as) {
	fpage_t* mpu[8];
	fpage_t* fp = as->first;
	int i = 0, j = 7, k = 0;

	/*
	 * We walk thru fpage list
	 * [0:k] are always-mapped fpages + LRU page
	 * [k:7] are others
	 * */

	while(fp != NULL) {
		if(fp->fpage.flags & FPAGE_ALWAYS ||
				fp == as->lru) {
			if(k < 8)
				mpu[k++] = fp;
		}
		else {
			if(j >= k)
				mpu[j--] = fp;
		} /*Non-always fpage mapping*/
		fp = fp->as_next;
	}

	as->lru = NULL;

	for(i = 0; i < k; ++i) {
		mpu_setup_region(i, mpu[i]);
	}

	for(i = 7; i > j; --i) {
		mpu_setup_region(i, mpu[i]);
	}
}

void as_map_user(as_t* as) {
	int i;

	for(i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		switch(memmap[i].tag) {
		case MPT_USER_DATA:
		case MPT_USER_TEXT:
			/* Create fpages only for user text and user data*/
			create_fpages(as, memmap[i].start, (memmap[i].end - memmap[i].start));
		}
	}
}

as_t* as_create(uint32_t as_spaceid) {
	as_t* as = (as_t*) ktable_alloc(&as_table);

	/*assert as == NULL*/

	as->as_spaceid = as_spaceid;
	as->first = NULL;

	return as;
}

int map_fpage(as_t* src, as_t* dst, fpage_t* fpage, map_action_t action) {
	fpage_t* fpmap = (fpage_t*) ktable_alloc(&fpage_table);

	/*FIXME: check for fpmap == NULL*/
	fpmap->as_next = NULL;
	fpmap->as = dst;

	/*Copy fpage description*/
	fpmap->raw[0] = fpage->raw[0];
	fpmap->raw[1] = fpage->raw[1];

	/*Set flags correctly*/
	if(action == MAP)
		fpage->fpage.flags |= FPAGE_MAPPED;
	fpmap->fpage.flags = FPAGE_CLONE;

	/*Insert into mapee list*/
	fpmap->map_next = fpage->map_next;
	fpage->map_next = fpmap;

	/*Insert into AS*/
	insert_fpage_to_as(dst, fpmap);

	dbg_printf(DL_MEMORY, "MEM: %s fpage %p from %p to %p\n", (action == MAP)? "mapped" : "granted", fpage, src, dst);

	return 0;
}

int unmap_fpage(as_t* as, fpage_t* fpage) {
	fpage_t* fpprev = fpage;

	dbg_printf(DL_MEMORY, "MEM: unmapped fpage %p from %p\n", fpage, as);

	/* Fpages that are not mapped or granted
	 * are destroyed with it's AS */
	if(!(fpage->fpage.flags & FPAGE_CLONE))
		return -1;

	while(fpprev->map_next != fpage) fpprev = fpprev->map_next;
	/*Clear flags*/
	fpprev->fpage.flags &= ~FPAGE_MAPPED;

	fpprev->map_next = fpage->map_next;
	remove_fpage_from_as(as, fpage);

	if(as->lru == fpage)
		as->lru = NULL;

	ktable_free(&fpage_table, fpage);

	return 0;
}

int map_area(as_t* src, as_t* dst, memptr_t base, size_t size, map_action_t action, int is_priviliged) {
	/*Most complicated part of mapping subsystem*/
	memptr_t end = base + size, probe = base;
	fpage_t *fp = src->first, *first = NULL, *last = NULL;
	int last_invalid = 0;

	/*FIXME: reverse mappings (i.e. thread 1 maps 0x1000 to thread 2, than thread 2 does
	 * the same to thread 1).*/

	/* For priviliged thread (ROOT), we use shadowed mapping,
	 * so first we will check if that fpages are exist
	 * than create them*/

	/* FIXME: checking existence of fpages*/

	if(is_priviliged) {
		create_fpages_ext(-1, src, base, size, &first, &last);
		if(src == dst) {
			/*Maps to itself, ignore other actions*/
			return 0;
		}
	}
	else {
		/* We should determine first and last fpage we will map to:
		 *
		 * +----------+    +----------+    +----------+
		 * |   first  | -> |          | -> |  last    |
		 * +----------+    +----------+    +----------+
		 *     ^base		^    +size      =    ^end
		 *                  |probe
		 *
		 * probe checks that addresses in fpage are sequental
		 * */
		while(fp) {
			if(!first && addr_in_fpage(base, fp)) {
				first = fp;
			}

			if(!last && addr_in_fpage(end, fp)) {
				last = fp;
				break;
			}

			if(first) {
				/*Check if addresses in fpage list are sequental*/
				if(!addr_in_fpage(probe, fp))
					return -1;

				probe += (1 << fp->fpage.shift);
			}

			fp = fp->as_next;
		}
	}

	if(!last || !first) {
		/* Not in address space or error */
		return -1;
	}

	if(first == last)
		last_invalid = 1;

	/* That is a problem because we should split
	 * fpages into two (and split all mappings too) */

	first = split_fpage(src, first, base, 1);

	/* If first and last were same pages, after first split,
	* last fpage will be invalidated, so we search it again*/
	if(last_invalid) {
		fp = first;
		while(fp) {
			if(addr_in_fpage(end, fp)) {
					last = fp;
					break;
			}
			fp = fp->as_next;
		}
	}

	last  = split_fpage(src, last, end, 0);

	/*Map chain of fpages*/

	fp = first;
	do {
		map_fpage(src, dst, fp, action);
		fp = fp->as_next;
	} while(fp != last);

	return 0;
}

void memmanage_handler(void) {
	uint32_t* mmsr = (uint32_t*) 0xE000ED28;
	uint32_t* mmar = (uint32_t*) 0xE000ED34;

	dbg_panic_puts("Memory fault\n");

	while(1);
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
			dbg_printf(DL_KDB, "MEM: fpage %s [b:%p, sz:2**%d]\n", memmap[fpage->fpage.mpid].name,
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
