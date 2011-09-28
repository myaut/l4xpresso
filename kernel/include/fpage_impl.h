/*
 * fpage_impl.h
 *
 *  Created on: 29.09.2011
 *      Author: myaut
 */

#ifndef FPAGE_IMPL_H_
#define FPAGE_IMPL_H_

void fpages_init(void);

void create_fpage_chain(memptr_t base, size_t size, int mpid, fpage_t** pfirst, fpage_t** plast);

void insert_fpage_chain_to_as(as_t* as, fpage_t* first, fpage_t* last);
void insert_fpage_to_as(as_t* as, fpage_t* fpage);

void remove_fpage_from_as(as_t* as, fpage_t* fp);
fpage_t* split_fpage(as_t* as, fpage_t* fpage, memptr_t split, int rl);

int assign_fpages(as_t* as, memptr_t base, size_t size);
int assign_fpages_ext(int mpid, as_t* as, memptr_t base, size_t size, fpage_t** pfirst,
		fpage_t** plast);

int map_fpage(as_t* src, as_t* dst, fpage_t* fpage, map_action_t action);
int unmap_fpage(as_t* as, fpage_t* fpage);


#endif /* FPAGE_IMPL_H_ */
