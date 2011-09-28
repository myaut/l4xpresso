/*
 * fpage.h
 *
 *  Created on: 25.09.2011
 *      Author: myaut
 */

#ifndef FPAGE_H_
#define FPAGE_H_

#include <memory.h>
#include <types.h>

#define FPAGE_ALWAYS    0x1     /*!Fpage is always mapped in mpu*/
#define FPAGE_CLONE     0x2     /*!Fpage is mapped from other as*/
#define FPAGE_MAPPED    0x4     /*!Fpage is mapped with MAP (unavailable in original AS)*/

/**
 * Flexible page (fpage_t)
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

#define FPAGE_BASE(fp) 	(fp)->fpage.base
#define FPAGE_SIZE(fp)  (1 << (fp)->fpage.shift)
#define FPAGE_END(fp)	(FPAGE_BASE(fp) + FPAGE_SIZE(fp))

#endif /* FPAGE_H_ */
