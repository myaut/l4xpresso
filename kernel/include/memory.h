/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/include/memory.h
Author: myaut

@LICENSE
*/

#ifndef MEMORY_H_
#define MEMORY_H_

#include <config.h>
#include <types.h>
#include <platform/link.h>

/*
 * Each address space is associated with one or more thread
 * AS represented as linked list of fpages, mappings are too
 * */

typedef enum { MAP, GRANT, UNMAP } map_action_t;

struct fpage;

typedef struct {
	uint32_t	as_spaceid;
	struct fpage* 	first;

	struct fpage* 	lru;			/*LRU page, will be mapped as MAP_ALWAYS*/
} as_t;

/*
 * Note that fpage format is not compliant with L4 X2 binary format
 *
 * as_next - next in address space chain
 * map_next - next in mappings chain (cycle list)
 *
 * base - base address of fpage
 * shift - size of fpage == 1 << shift
 * rwx - access bits
 * mpid - id of memory pool
 * flags - flags*/
struct fpage {
	as_t*	as;
	struct fpage* as_next;
	struct fpage* map_next;

	union {
		struct {
			uint32_t base;
			uint32_t mpid : 6;
			uint32_t flags : 6;
			uint32_t shift : 16;
			uint32_t rwx : 4;
		} fpage;
		uint32_t raw[2];
	};
};
typedef struct fpage fpage_t;

typedef struct {
#ifdef CONFIG_DEBUG
	char*	 name;
#endif

	memptr_t start;
	memptr_t end;

	uint32_t flags;
	uint32_t tag;
} mempool_t;

/* Kernel permissions flags */
#define MP_KR		0x0001
#define MP_KW		0x0002
#define MP_KX		0x0004

/* Userspace permissions flags */
#define MP_UR		0x0010
#define MP_UW		0x0020
#define MP_UX		0x0040

/* Fpage type */
#define MP_NO_FPAGE	0x0000		/* Not mappable fpages (Kernel) */
#define MP_SRAM		0x0100		/* Fpage in SRAM: granularity 64 words*/
#define MP_AHB_RAM	0x0200		/* Fpage in AHB SRAM: granularity 64 words, bit bang mappings*/
#define MP_DEVICES	0x0400		/* Fpage in AHB/APB0/AHB0: granularity 16 kB*/
#define MP_MEMPOOL	0x0800		/* Entire mempool is mapped */

/* Map memory from mempool always (for example text is mapped always because without it thread couldn't run)
 * other fpages mapped on request because we limited in MPU resources)*/
#define MP_MAP_ALWAYS 	0x1000

#define MP_FPAGE_MASK	0x0F00

#define MP_USER_PERM(mpflags) ((mpflags & 0xF0) >> 4)

#define FPAGE_ALWAYS	0x1
#define FPAGE_CLONE 	0x2		/*Fpage is mapped from other as*/
#define FPAGE_MAPPED 	0x4		/*Fpage is mapped with MAP (unavailable in original AS)*/

typedef enum {
	MPT_KERNEL_TEXT,
	MPT_KERNEL_DATA,
	MPT_USER_TEXT,
	MPT_USER_DATA,
	MPT_AVAILABLE,
	MPT_DEVICES,
	MPT_UNKNOWN = -1
} mempool_tag_t;

/*If fpage_size = 0, memory is not allocable*/
#ifdef CONFIG_DEBUG
#define DECLARE_MEMPOOL(name_, start_, end_, flags_, tag_) 	\
	{													\
		.name = name_,									\
		.start = (memptr_t) (start_),					\
		.end  = (memptr_t) (end_),						\
		.flags = flags_,  								\
		.tag = tag_										\
	}
#else
#define DECLARE_MEMPOOL(name_, start_, end_, flags_, tag_) 	\
	{													\
		.start = (memptr_t) (start_),					\
		.end  = (memptr_t) (end_),						\
		.flags = flags_,  								\
		.tag = tag_										\
	}
#endif

#define DECLARE_MEMPOOL_2(name, prefix, flags, tag) \
		DECLARE_MEMPOOL(name, &(prefix ## _start), &(prefix ## _end), flags, tag)

typedef enum {
	MPU_DISABLED,
	MPU_ENABLED
} mpu_state_t;

void memory_init();

as_t* as_create(uint32_t as_spaceid);

void create_fpage_chain(memptr_t base, size_t size, as_t* as, int mpid, fpage_t** pfirst, fpage_t** plast);
int create_fpages(as_t* as, memptr_t base, size_t size);
int create_fpages_ext(int mpid, as_t* as, memptr_t base, size_t size, fpage_t** pfirst,
		fpage_t** plast);
void insert_fpage_chain_to_as(as_t* as, fpage_t* first, fpage_t* last);
void insert_fpage_to_as(as_t* as, fpage_t* fpage);
int map_fpage(as_t* src, as_t* dst, fpage_t* fpage, map_action_t action);
int unmap_fpage(as_t* as, fpage_t* fpage);
void as_setup_mpu(as_t* as);
void as_map_user(as_t* as);
void mpu_enable(mpu_state_t i);
int map_area(as_t* src, as_t* dst, memptr_t base, size_t size, map_action_t action, int is_priviliged);

#endif /* MEMORY_H_ */
