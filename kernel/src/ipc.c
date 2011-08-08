/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/kernel/src/ipc.c
Author: myaut

@LICENSE
*/

#include <platform/armv7m.h>
#include <debug.h>
#include <types.h>
#include <thread.h>
#include <memory.h>
#include <config.h>
#include <ipc.h>

extern tcb_t* caller;

/* Imports from thread.c */
extern tcb_t* thread_map[CONFIG_MAX_THREADS];
extern int thread_count;

uint32_t ipc_read_mr(tcb_t* from, int i) {
	if(i >= 8) return from->utcb->mr[i - 8];
	else return from->ctx.regs[i];
}

void ipc_write_mr(tcb_t* to, int i, uint32_t data) {
	if(i >= 8) to->utcb->mr[i - 8] = data;
	else to->ctx.regs[i] = data;
}

void ipc(tcb_t* from, tcb_t* to) {
	ipc_msg_tag_t tag;
	ipc_typed_item	ti;
	int i, t, j;
	uint32_t tm;		/* typed item extra word*/

	tag.raw = from->ctx.regs[0];

	to->ctx.regs[0] = tag.s.label << 16;

	/* First 8 MRs are located on r4-r11,
	 * last 8 MRs - in UTCB
	 * */

	/*Copy untyped words*/
	for(i = 1; i < tag.s.n_untyped && i < 15; ++i) {
		ipc_write_mr(to, i, ipc_read_mr(from, i));
	}

	j = -1;
	/* Copy typed words
	 * FSM: j - number of byte*/
	for(t = i; t < tag.s.n_typed && i < 15; ++t, ++i) {
		if(j == -1) {
			/*If j == -1 - read tag*/
			ti.raw = ipc_read_mr(from, i);
			++j;
		}
		else {
			/* MapItem / GrantItem have 1xxx in header */
			if(ti.s.header & IPC_TI_MAP_GRANT) {
				tm = ipc_read_mr(from, i);

				map_area(from->as, to->as, ti.raw & 0xFFFFFFC0, tm & 0xFFFFFFC0,
							(ti.s.header & IPC_TI_GRANT)? GRANT : MAP, thread_ispriviliged(from));
			}
		}
	}

	to->utcb->sender = from->t_globalid;

	to->state = T_RUNNABLE;
	to->ipc_from = L4_NILTHREAD;
	from->state = T_RUNNABLE;

	dbg_printf(DL_IPC, "IPC: %t to %t\n", caller->t_globalid, to->t_globalid);
}

void sys_ipc(uint32_t* param1) {
	/*TODO: Checking of recv-mask*/
	tcb_t *to_thr = NULL;
	l4_thread_t to = param1[REG_R0], from = param1[REG_R1];

	if(to != L4_NILTHREAD) {
		to_thr =  thread_by_globalid(to);

		if((to_thr && to_thr->state == T_RECV_BLOCKED)
				|| to == caller->t_globalid ) {
			/* To thread who waiting us or
			 * send myself*/
			ipc(caller, to_thr);
		}
		else if(to_thr && to_thr->state == T_INACTIVE &&
				GLOBALID_TO_TID(to_thr->utcb->t_pager) == GLOBALID_TO_TID(caller->t_globalid)) {
			/* That is thread start protocol */

			dbg_printf(DL_IPC, "IPC: %t thread start\n", to);

			thread_init_ctx((void*) ipc_read_mr(caller, 2),
							(void*) ipc_read_mr(caller, 1), to_thr);
			caller->state = T_RUNNABLE;
		}
		else  {
			/*No waiting, block myself*/
			caller->state = T_SEND_BLOCKED;
			caller->utcb->intended_receiver = to;

			dbg_printf(DL_IPC, "IPC: %t sending\n", caller->t_globalid);
		}
	}

	if(from != L4_NILTHREAD) {
		/*Only receive phases, simply lock myself*/
		caller->state = T_RECV_BLOCKED;
		caller->ipc_from = from;

		dbg_printf(DL_IPC, "IPC: %t receiving\n", caller->t_globalid);
	}
}

uint32_t ipc_deliver(void* data) {
	tcb_t* thr = NULL, *from_thr = NULL;
	int i;

	for(i = 1; i < thread_count; ++i) {
		thr = thread_map[i];

		if(thr->state == T_RECV_BLOCKED && thr->ipc_from != L4_NILTHREAD) {
			from_thr = thread_by_globalid(thr->ipc_from);

			ipc(from_thr, thr);
		}
	}

	return 4096;
}
