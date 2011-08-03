/*
 L4Xpresso
 Copyright (c) 2011, Sergey Klyaus

 File: /leo4-mcu/kernel/src/thread.c
 Author: myaut

 @LICENSE
 */

#include <thread.h>
#include <ktable.h>
#include <config.h>
#include <debug.h>
#include <platform/irq.h>
#include <platform/armv7m.h>

/*
 * Main thread dispatcher
 *
 * Each thread has it's own Thread Control Block (struct tcb_t) and addressed by
 * it's global id. However, globalid are very wasteful - we reserve 3 global ids for
 * in-kernel purposes (IDLE, KERNEL and ROOT), 240 for NVIC's interrupt thread
 * and 256 for user threads, which gives at least 4  * 512 = 2Kb.
 *
 * On the other hand, we don't need so much threads, so we use thread_map -
 * array of pointers to tcb_t sorted by global id and ktable of tcb_t
 * For each search operation we use binary search
 *
 * See also Abi Nourai, A Physically addressed L4 Kernel
 *
 * Also dispatcher is responsible for switching contexts (but not scheduling)
 */

DECLARE_KTABLE(tcb_t, thread_table, CONFIG_MAX_THREADS)
;

/* Always sorted, so we can use binary search on it */
tcb_t* thread_map[CONFIG_MAX_THREADS];
int thread_count;

/**
 * current are always points to TCB which was on processor before we had fallen
 * into interrupt handler. irq_save saves sp and r4-r11 (other are saved automatically
 * on stack). When handler finishes, it calls thread_ctx_switch, which chooses
 * next executable thread and returns it's context
 *
 * irq_return recover's thread's context
 *
 * See also platform/irq.h
 */

volatile tcb_t* current; // Currently on CPU
volatile tcb_t* kernel; // Kernel thread
volatile tcb_t* next_current; // Next dispatched on CPU

/*KIP declarations*/
fpage_t *kip_fpage, *kip_extra_fpage;
extern kip_t kip;
extern char* kip_extra;

void thread_init() {
	fpage_t* last = NULL;

	ktable_init(&thread_table);

	/* Pre-allocate system threads and set kernel as dispatched*/
	kernel = thread_create(TID_TO_GLOBALID(THREAD_KERNEL), NULL);
	/* Set EXC_RETURN and CONTROL reg for kernel */
	kernel->ctx.ret = 0xFFFFFFF9;
	kernel->ctx.ctl = 0x0;

	current = kernel;

	kip.thread_info.s.system_base = THREAD_SYS;
	kip.thread_info.s.user_base = THREAD_USER;

	/* Create KIP fpages
	 * last is ignored, because kip fpages is aligned*/
	create_fpages_ext(-1, NULL, (memptr_t) &kip, sizeof(kip_t), &kip_fpage, &last);
	create_fpages_ext(-1, NULL, (memptr_t) kip_extra, CONFIG_KIP_EXTRA_SIZE, &kip_extra_fpage, &last);
}

extern tcb_t* caller;

/*
 * Return upper_bound with binary search
 * */
int thread_map_search(l4_thread_t globalid, int from, int to) {
	int mid = 0;
	int tid = GLOBALID_TO_TID(globalid);

	/* Upper bound if beginning of array */
	if (to == from || GLOBALID_TO_TID(thread_map[from]->t_globalid) >= tid)
		return from;

	while (from <= to) {
		mid = from + (to - from) / 2;

		if ((to - from) <= 1)
			return to;

		if (GLOBALID_TO_TID(thread_map[mid]->t_globalid) > tid)
			to = mid;
		else if (GLOBALID_TO_TID(thread_map[mid]->t_globalid) < tid)
			from = mid;
		else
			return mid;
	}

	/*NOT REACHED*/
	return -1;
}

/*
 * Insert thread into thread map
 */
void thread_map_insert(l4_thread_t globalid, tcb_t* thr) {
	if (thread_count == 0) {
		thread_map[thread_count++] = thr;
	} else {
		int i = thread_map_search(globalid, 0, thread_count), j;

		/* Move forward
		 * Don't check if count is out of range,
		 * because we will fail on ktable_alloc */

		for (; j > i; --j)
			thread_map[j] = thread_map[j - 1];

		thread_map[i] = thr;
		++thread_count;
	}
}

/**
 * Create thread
 */
tcb_t* thread_create(l4_thread_t globalid, utcb_t* utcb) {
	tcb_t *thr;
	int id;

	id = GLOBALID_TO_TID(globalid);
	thr = (tcb_t*) ktable_alloc(&thread_table);

	if (!thr)
		return NULL;

	thread_map_insert(globalid, thr);
	thr->t_localid = 0x0;

	thr->t_child = NULL;
	thr->t_parent = NULL;
	thr->t_sibling = NULL;

	thr->t_globalid = globalid;
	if(utcb)
		utcb->t_globalid = globalid;

	thr->as = NULL;
	thr->utcb = utcb;
	thr->kip = NULL;
	thr->state = T_INACTIVE;

	dbg_printf(DL_THREAD, "T: New thread: %t @[%p] \n", globalid, thr);

	if (caller != NULL) {
		/* Called from user thread */
		if (id < THREAD_SYS)
			return NULL;

		thr->t_parent = caller;
		if (caller->t_child) {
			tcb_t* t = caller->t_child;

			while (t->t_sibling != 0)
				t = t->t_sibling;
			t->t_sibling = thr;

			thr->t_localid = t->t_localid + (1 << 6);
		} else {
			/*That is first thread in child chain*/
			caller->t_child = thr;

			thr->t_localid = (1 << 6);
		}
	}

	return thr;
}

void thread_space(tcb_t* thr, l4_thread_t spaceid, utcb_t* utcb) {
	/* If spaceid == dest than create new address space
	 * else share address space between threads */
	if (GLOBALID_TO_TID(thr->t_globalid) == GLOBALID_TO_TID(spaceid)) {
		thr->as = as_create(thr->t_globalid);

		/*Grant kip_fpage & kip_ext_fpage only to new AS*/
		map_fpage(NULL, thr->as, kip_fpage, GRANT);
		map_fpage(NULL, thr->as, kip_extra_fpage, GRANT);

		dbg_printf(DL_THREAD, "\tNew space: as: %p, utcb: %p \n", thr->as, utcb);
	} else {
		tcb_t* space = thread_by_globalid(spaceid);

		thr->as = space->as;
	}

	/* If no caller, than it is mapping from kernel to root thread
	 * (some special case for root_utcb) */
	if(caller)
		map_area(caller->as, thr->as, (memptr_t) utcb,
				sizeof(utcb_t), GRANT, thread_ispriviliged(caller));
	else
		map_area(thr->as, thr->as, (memptr_t) utcb, sizeof(utcb_t), GRANT, 1);
}

void thread_start(void* sp, void* pc, uint32_t xpsr, tcb_t *thr) {
	/* Reserve 8 words for fake context */
	sp -= 8 * sizeof(uint32_t);
	thr->ctx.sp = (uint32_t) sp;

	/*Set EXC_RETURN and CONTROL for user thread*/
	thr->ctx.ret = 0xFFFFFFFD;
	thr->ctx.ctl = 0x3;

	/* Stack allocated, now use fake pc, lr, xpsr */
	((uint32_t*) sp)[REG_R0] = (uint32_t) &kip;
	((uint32_t*) sp)[REG_R1] = (uint32_t) thr->utcb;
	((uint32_t*) sp)[REG_R2] = 0x0;
	((uint32_t*) sp)[REG_R3] = 0x0;
	((uint32_t*) sp)[REG_R12] = 0x0;
	((uint32_t*) sp)[REG_LR] = 0xFFFFFFFF;
	((uint32_t*) sp)[REG_PC] = (uint32_t) pc;
	((uint32_t*) sp)[REG_xPSR] = xpsr | 0x1000000; /* Thumb bit on*/

	thr->state = T_RUNNABLE;
}

/*
 * Search thread by it's global id
 */
tcb_t* thread_by_globalid(l4_thread_t globalid) {
	int idx = thread_map_search(globalid, 0, thread_count);

	if (GLOBALID_TO_TID(thread_map[idx]->t_globalid)
			!= GLOBALID_TO_TID(globalid))
		return NULL;
	else
		return thread_map[idx];
}

uint32_t thread_isdispatched() {
	return (next_current != NULL);
}

int thread_isrunnable(tcb_t* thr) {
	return thr->state == T_RUNNABLE;
}

void thread_dispatch(tcb_t* thr) {
	if (thr->state == T_RUNNABLE) {
		dbg_printf(DL_SCHEDULE, "TCB: dispatched %t\n", thr->t_globalid);
		next_current = (volatile tcb_t*) thr;
	}
}

tcb_t* thread_current() {
	return (tcb_t*) current;
}

int thread_ispriviliged(tcb_t* thread) {
	return GLOBALID_TO_TID(thread->t_globalid) == THREAD_ROOT;
}

/* Switch context
 * */
void thread_switch() {
	if (!softirq_isscheduled()) {
		/*No SOFTIRQs is scheduled, we can go to userspace now*/

		/*	Check if current is still runnable
		 *	(not blocked and timeslice not exhausted)
		 *
		 *	kernel is always inactive, so if we in kernel now
		 *	we will try to switch context too*/
		if (!thread_isrunnable((tcb_t*) current)) {
			/*Try to reschedule if not yet done*/
			if (!next_current)
				schedule();

			if (next_current) {
				/*We already scheduled a thread, simply switch to it*/
				dbg_printf(DL_THREAD, "TCB: switch %p -> %p\n", current,
						next_current);
				current = next_current;
				next_current = NULL;

				if (current->as)
					as_setup_mpu(current->as);
			} else {
				/* Fall into kernel, than to IDLE state (WFI)*/
				current = kernel;
				dbg_printf(DL_THREAD, "TCB: switch %p -> IDLE\n", current);
			}
		}
	} else {
		/*Jump to kernel*/
		current = kernel;
		dbg_printf(DL_THREAD, "TCB: switch %p -> kernel\n", current);
	}
}

#if 0
void deliver_ipc() {
	tcb_t* thr = NULL, *from_thr = NULL;
	int idx;

	for_each_in_ktable(thr, idx, (&thread_table)) {
		if(thr->state == T_RECV_BLOCKED) {
			/*Check if somebody sending us*/
			if(thr->t_from != L4_NILTHREAD) {
				from_thr = thread_by_globalid(thr->t_from);

				dbg_printf(DL_IPC, "IPC: %t to %t (sched)\n", thr->t_from, thr->t_globalid);

				from_thr->state = T_RUNNABLE;
				thr->state = T_RUNNABLE;
				thr->t_from = L4_NILTHREAD;
			}
		}
	}
}
#endif

int schedule() {
	tcb_t* root = THREAD_BY_TID(THREAD_ROOT);

	if (!thread_isdispatched() && root->state == T_RUNNABLE)
		thread_dispatch(root);

	return 0;
}

#ifdef CONFIG_KDB

char* kdb_get_thread_type(tcb_t* thr) {
	int id = GLOBALID_TO_TID(thr->t_globalid);

	if (id == THREAD_IDLE)
		return "IDLE";
	else if (id == THREAD_KERNEL)
		return "KERN";
	else if (id == THREAD_ROOT)
		return "ROOT";
	else if (id >= THREAD_SYS)
		return "[SYS]";
	else if (id >= THREAD_USER)
		return "[USR]";

	return "???";
}

void kdb_dump_threads() {
	tcb_t* thr;
	int idx;

	char* state[] = { "", "FREE", "RUN", "SVC", "SEND", "RECV" };

	dbg_printf(DL_KDB, "%5s %8s %8s %6s %s\n", "type", "global", "local",
			"state", "parent");

	for_each_in_ktable(thr, idx, (&thread_table))
		{
			dbg_printf(DL_KDB, "%5s %t %t %6s %t\n", kdb_get_thread_type(thr),
					thr->t_globalid, thr->t_localid, state[thr->state],
					(thr->t_parent) ? thr->t_parent->t_globalid : 0);
		}
}

#endif
