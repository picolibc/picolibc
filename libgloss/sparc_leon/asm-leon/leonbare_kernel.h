/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef __LEONBARE_KERNEL_H__
#define __LEONBARE_KERNEL_H__

#include <asm-leon/contextswitch.h>
#include <asm-leon/leonbare_debug.h>
#include <asm-leon/leon.h>
#ifndef __ASSEMBLER__
#include <asm-leon/leonbare_kernel_queue.h>
#include <reent.h>
#endif
#include "irq.h"

#define LEONBARE_RUNQ_READY_NR      (2)	/* queue 0-1 for ready */
#define LEONBARE_RUNQ_SUSPENDED_IDX (2)	/* queue 2   for suspended */
#define LEONBARE_RUNQ_PREPARE_IDX   (3)	/* LEONBARE_RUNQ_READY_NR times queues */
#define LEONBARE_RUNQ_KILLED_IDX    (LEONBARE_RUNQ_PREPARE_IDX+LEONBARE_RUNQ_READY_NR)	/* queue 2   for killed threads */
#define LEONBARE_RUNQ_NR            (LEONBARE_RUNQ_KILLED_IDX+1)

#define LEONBARE_RUNQ_ISREADY(idx)   ((idx) >= 0 && (idx) < LEONBARE_RUNQ_READY_NR)
#define LEONBARE_RUNQ_ISPREPARE(idx) ((idx) >= LEONBARE_RUNQ_PREPARE_IDX && (idx) < LEONBARE_RUNQ_PREPARE_IDX+LEONBARE_RUNQ_READY_NR)
#define LEONBARE_RUNQ_ISSUSPEND(idx) ((idx) == LEONBARE_RUNQ_SUSPENDED_IDX)
#define LEONBARE_RUNQ_ISKILLED(idx)  ((idx) == LEONBARE_RUNQ_KILLED_IDX)

#ifndef __ASSEMBLER__

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MACRO_BEGIN     do {
#define MACRO_END       } while (0)

#define optbarrier() __asm__ __volatile__("": : :"memory")

typedef struct leonbare_thread_ctx
{
  unsigned long sf_locals[8];
  unsigned long sf_ins[8];
  unsigned long outs[8];
  unsigned long globals[8];
  unsigned long psr;
  unsigned long wim;
  unsigned long magic;
  unsigned long fpu;
  /* size aligned to 8 */
} leonbare_thread_ctx_t;
#define LEONBARE_THREAD_CTX_SZ sizeof(struct leonbare_thread_ctx)

typedef
LBTAILQ_HEAD (leonbare_mutex_queue, leonbare_mutex) *
  leonbare_mutex_queue_t;

#endif
#define LEONBARE_THREAD_OFFSET_CTX 0
#ifndef __ASSEMBLER__

     struct leonbare_thread_protect
     {
       unsigned int runq;
       unsigned int krp_runq_depth;
       unsigned int krp_k_depth;
       struct leonbare_mutex *krp_m;
       unsigned int krp_m_depth;
       unsigned int krp_flags;;
       unsigned int krp_flags_depth;
     };

#define LEONBARE_INT_DISABLE_DECL  unsigned long _irq_flags = leonbare_disable_traps();
#define LEONBARE_INT_ENABLE_DECL  leonbare_enable_traps(_irq_flags);

#define leonbare_setu32p(a,v) leonbare_leon23_storenocache(a,v)
#define leonbare_setu32(a,v)  leonbare_leon23_storenocache(a,v)
#define leonbare_getu32(a)    leonbare_leon23_loadnocache(a)

#define LEONBARE_KERNEL_UNCACHED
#ifndef LEONBARE_KERNEL_UNCACHED
#define LEONBARE_KERNEL_SETU32P(a,v) (a=v)
#define LEONBARE_KERNEL_SETU32(a,v)  (a=v)	/* uncached version should return v */
#define LEONBARE_KERNEL_GETU32(a)    (a)
#define LEONBARE_KERNEL_GETU32P(a)   (a)
#define LEONBARE_KERNEL_GETI32(a)    (a)
#define LEONBARE_KERNEL_GETU32P_CAST(a,typ) ((typ)(a))
#define LEONBARE_KERNEL_GETU32P_BARE(a) (*(a))	/* uncached: no & */
#define LEONBARE_KERNEL_SETU32P_BARE(a,v) (*(a) = v)	/* uncached: no & */
#else
#define LEONBARE_KERNEL_SETU32P(a,v) (leonbare_setu32p(&a,v))
#define LEONBARE_KERNEL_SETU32(a,v)  (leonbare_setu32p(&a,v))	/* uncached version should return v */
#define LEONBARE_KERNEL_GETU32(a)    (leonbare_getu32(&a))
#define LEONBARE_KERNEL_GETU32P(a)   ((void *)leonbare_getu32(&a))
#define LEONBARE_KERNEL_GETI32(a)    (leonbare_getu32(&a))
#define LEONBARE_KERNEL_GETU32P_CAST(a,typ) ((typ)(LEONBARE_KERNEL_GETU32P(a)))
#define LEONBARE_KERNEL_GETU32P_BARE(a) ((void *)leonbare_getu32(a))	/* uncached: no & */
#define LEONBARE_KERNEL_SETU32P_BARE(a,v) (leonbare_setu32p(a,v))	/* uncached: no & */
#endif


#define LEONBARE_SMP_SPINLOCK_AQUIRE(l)
#define LEONBARE_SMP_SPINLOCK_RELEASE(l)

#define LEONBARE_ISQ_ISDISABLED ((leon23_getpsr() & SPARC_PSR_PIL_MASK) == SPARC_PSR_PIL_MASK)

#define _LEONBARE_PROTECT_IRQ_START					\
    if (LEONBARE_KR_CURRENT->th_prot.krp_flags_depth++) {			\
        LBPASSERT((LEONBARE_ISQ_ISDISABLED),"Internal error: Recursiv IRQ protection with irq's enabled",0); \
    } else {								\
	LEONBARE_KR_CURRENT->th_prot.krp_flags = leonbare_disable_traps(); \
    }

#define _LEONBARE_PROTECT_IRQ_END					\
    if (--LEONBARE_KR_CURRENT->th_prot.krp_flags_depth) {			\
        LBPASSERT((LEONBARE_ISQ_ISDISABLED),"Internal error: Recursiv IRQ protection with irq's enabled",0); \
    } else {								\
	leonbare_enable_traps(LEONBARE_KR_CURRENT->th_prot.krp_flags);	\
    }

#define _LEONBARE_PROTECT_MUTEXSTRUCT_START(m)				\
    if (LEONBARE_KR_CURRENT->th_prot.krp_m_depth++) {			\
        LBPASSERT((LEONBARE_KR_CURRENT->th_prot.krp_m == m),"Mutex protection only allowed for one mutex at a time",0);	\
    } else {								\
        LEONBARE_SMP_SPINLOCK_AQUIRE(m->smp_lock);			\
	LEONBARE_KR_CURRENT->th_prot.krp_m = m;				\
    }

#define _LEONBARE_PROTECT_MUTEXSTRUCT_END(m)				\
    LBPASSERT((LEONBARE_KR_CURRENT->th_prot.krp_m == m),"Mutex protection only allowed for one mutex at a time",0); \
    if ((--LEONBARE_KR_CURRENT->th_prot.krp_m_depth) == 0) {		\
        LEONBARE_SMP_SPINLOCK_RELEASE(m->smp_lock);			\
    }

#define _LEONBARE_PROTECT_KERNEL_START				\
    if (LEONBARE_KR_CURRENT->th_prot.krp_k_depth++ == 0) {			\
        LEONBARE_SMP_SPINLOCK_AQUIRE(LEONBARE_KR_LOCK);			\
    }

#define _LEONBARE_PROTECT_KERNEL_END				\
    if ((--LEONBARE_KR_CURRENT->th_prot.krp_k_depth) == 0) {		\
        LEONBARE_SMP_SPINLOCK_RELEASE(LEONBARE_KR_LOCK);			\
    }


#define LEONBARE_PROTECT_MUTEXSTRUCT_START(m)	\
    _LEONBARE_PROTECT_IRQ_START;		\
    _LEONBARE_PROTECT_MUTEXSTRUCT_START(m)

#define LEONBARE_PROTECT_MUTEXSTRUCT_END(m)	\
    _LEONBARE_PROTECT_MUTEXSTRUCT_END(m)	\
    _LEONBARE_PROTECT_IRQ_END;


#define LEONBARE_PROTECT_KERNEL_START()		\
    _LEONBARE_PROTECT_IRQ_START;		\
    _LEONBARE_PROTECT_KERNEL_START;

#define LEONBARE_PROTECT_KERNEL_END()		\
    _LEONBARE_PROTECT_KERNEL_END;		\
    _LEONBARE_PROTECT_IRQ_END;

     typedef struct leonbare_thread
     {
       struct leonbare_thread_ctx th_ctx;
       unsigned int th_flags;

       int th_account;		/* how many ticks the thread stays in the readyqueue for one round */
       int th_caccount;		/* current value of th_account, updated on reinsertion */
       unsigned int th_pri_idx;	/* ready queue index */
       unsigned int th_runq_idx;	/* ready queue index index */
       unsigned int th_runq_which;	/* 0: ready queue, 1: ready prepare queue */

       char *th_name;
       int th_result;
       int (*th_func) (void *);
       void *th_arg;
       char *th_stack_base;
       unsigned int th_stack_size;
       struct _reent th_reent;	/* reentrant structure for newlib */
       struct _reent *th_reentp;	/* pointer to eather pt_reent or global reent */

       struct leonbare_thread_protect th_prot;

         LBTAILQ_ENTRY (leonbare_thread) th_runq;
         LBTAILQ_ENTRY (leonbare_thread) th_allq;
         LBTAILQ_ENTRY (leonbare_thread) th_mutex;
       struct leonbare_mutex_queue th_mutex_locked;

     } *leonbare_thread_t __attribute__ ((aligned (8)));

#define LEONBARE_TH_FLAGS_get(c)      LEONBARE_KERNEL_GETU32((c)->th_flags)
#define LEONBARE_TH_ACCOUNT_get(c)    LEONBARE_KERNEL_GETI32((c)->th_account)
#define LEONBARE_TH_CACCOUNT_get(c)   LEONBARE_KERNEL_GETI32((c)->th_caccount)

#define LEONBARE_TH_PRI_IDX_get(c)    LEONBARE_KERNEL_GETU32((c)->th_pri_idx)
#define LEONBARE_TH_RUNQ_IDX_get(c)   LEONBARE_KERNEL_GETU32((c)->th_runq_idx)
#define LEONBARE_TH_RUNQ_WHICH_get(c) LEONBARE_KERNEL_GETU32((c)->th_runq_which)

#define LEONBARE_TH_NAME_get(c)       LEONBARE_KERNEL_GETU32P((c)->th_name)
#define LEONBARE_TH_RESULT_get(c)     LEONBARE_KERNEL_GETI32((c)->th_result)
#define LEONBARE_TH_FUNC_get(c)       LEONBARE_KERNEL_GETU32((c)->th_func)
#define LEONBARE_TH_ARG_get(c)        LEONBARE_KERNEL_GETU32((c)->th_arg)
#define LEONBARE_TH_STACK_BASE_get(c) LEONBARE_KERNEL_GETU32P((c)->th_stack_base)
#define LEONBARE_TH_STACK_SIZE_get(c) LEONBARE_KERNEL_GETU32((c)->th_stack_size)
#define LEONBARE_TH_REENTP_get(c)     LEONBARE_KERNEL_GETU32P((c)->th_reentp)




#define LEONBARE_TH_NAME(c) (c->th_name)
#define LEONBARE_TH_NAME_DBG(c) (LEONBARE_TH_NAME(c) ? LEONBARE_TH_NAME(c) : "<unknown>")

#define LEONBARE_REENT_SET(p) ((_impure_ptr=(p)->th_reentp)==_impure_ptr)

#define LEONBARE_TH_READY        (1<<0)
#define LEONBARE_TH_SUSPENDED    (1<<1)
#define LEONBARE_TH_TERMINATED   (1<<2)
#define LEONBARE_TH_FINISHED     (1<<3)

#define LEONBARE_TH_SATEMASK     (LEONBARE_TH_READY | \
				  LEONBARE_TH_SUSPENDED | \
				  LEONBARE_TH_TERMINATED | \
				  LEONBARE_TH_FINISHED)

#define LEONBARE_TH_SETSTATE(c,f) c->th_flags = ((c->th_flags & ~LEONBARE_TH_SATEMASK) | (f & LEONBARE_TH_SATEMASK))
#define LEONBARE_TH_ORSTATE(c,f) c->th_flags |= (f & LEONBARE_TH_SATEMASK)

     typedef LBTAILQ_HEAD (leonbare_thread_queue,
			   leonbare_thread) * leonbare_thread_queue_t;

     extern struct leonbare_kernel leonbare_kernel;
#define KERNEL_GLOBAL leonbare_kernel
     typedef struct leonbare_kernel
     {
       leonbare_thread_t kr_cur, kr_next;
       struct leonbare_thread_queue kr_runq[LEONBARE_RUNQ_NR];
       struct leonbare_thread_queue kr_allq;
       struct leonbare_mutex_queue kr_allm;
       int kr_is_inkernel, kr_need_schedule, kr_is_preemption, kr_runq_which;
       int kr_protect_flags;
     } leonbare_kernel_t __attribute__ ((aligned (8)));
#define LEONBARE_KR_CURRENT       (KERNEL_GLOBAL.kr_cur)
#define LEONBARE_KR_NEXT          (KERNEL_GLOBAL.kr_next)
#define LEONBARE_KR_RUNQ(i)     (&(KERNEL_GLOBAL.kr_runq[i]))
#define LEONBARE_KR_RUNQ_WHICH    (KERNEL_GLOBAL.kr_runq_which)
#define LEONBARE_KR_ALLQ        (&(KERNEL_GLOBAL.kr_allq))
#define LEONBARE_KR_ALLM        (&(KERNEL_GLOBAL.kr_allm))
#define LEONBARE_KR_IS_IN_KERNEL  (KERNEL_GLOBAL.kr_is_inkernel)
#define LEONBARE_KR_IS_PREEMPTION (KERNEL_GLOBAL.kr_is_preemption)

#define LEONBARE_KR_NEED_SCHEDULE (LEONBARE_KR_CURRENT != LEONBARE_KR_NEXT)

#define LEONBARE_STACKALIGN(sp) ((((unsigned int)sp) + 7) & ~7)

/* context switching macros, implemented via setjmp/longjmp plus saving errno */
#define SAVE_CONTEXT(t) ( _leonbare_kernel_savecontext((t), 0) )
#define RESTORE_CONTEXT(t) _leonbare_kernel_switchto((t), 1)

#define KERNEL_SCHEDULE(f,retval) \
  MACRO_BEGIN \
    LEONBARE_KR_IS_IN_KERNEL--; \
    if (LEONBARE_KR_IS_IN_KERNEL == 0 && LEONBARE_KR_NEED_SCHEDULE) {	\
      LEONBARE_KR_IS_IN_KERNEL++; \
      if ((f) && (SAVE_CONTEXT(LEONBARE_KR_CURRENT) == 0)) {	\
        leonbare_sched(); \
      } \
      optbarrier(); \
      LEONBARE_KR_IS_IN_KERNEL--; \
    } \
  MACRO_END

#define KERNEL_ENTER LEONBARE_KR_IS_IN_KERNEL++;
#define KERNEL_EXIT(f,ret) KERNEL_SCHEDULE(f,ret)

     int leonbare_thread_init ();
     int leonbare_thread_create (struct leonbare_thread *thread, char *stack,
				 int stacksize);
     int leonbare_sched_update ();
     leonbare_thread_t leonbare_sched_paytime ();
     void leonbare_sched_insert (struct leonbare_thread *thread, int head,
				 int prepare);
     unsigned int leonbare_sched ();
     unsigned int reschedule ();
     unsigned int _leonbare_kernel_switchto (struct leonbare_thread *old,
					     struct leonbare_thread *new);

#define LEONBARE_STACK_DEFINE(n,size) unsigned char n[size] __attribute__((aligned(8)));
#define LEONBARE_STACK_SIZE_DEFAULT 1024*20

     typedef struct leonbare_mutex
     {
       unsigned int mx_owner_cnt;
       leonbare_thread_t mx_owner;
       struct leonbare_thread_queue mx_threads;
         LBTAILQ_ENTRY (leonbare_mutex) mx_allm;
         LBTAILQ_ENTRY (leonbare_mutex) mx_locked;

     } *leonbare_mutex_t;

#define LEONBARE_MUTEX_OWNER_GET(m) LEONBARE_KERNEL_GETU32(m->mx_owner)
#define LEONBARE_MUTEX_OWNER_SET(m,o) LEONBARE_KERNEL_SETU32(m->mx_owner,o)
#define LEONBARE_MUTEX_OWNER_CNT_GET(m) LEONBARE_KERNEL_GETU32(m->mx_owner_cnt)
#define LEONBARE_MUTEX_OWNER_CNT_SET(m,o) LEONBARE_KERNEL_SETU32(m->mx_owner_cnt,o)

#define LEONBARE_MUTEX_LOCK_TIMEOUT -1
#define LEONBARE_MUTEX_LOCK_OK       0
#define LEONBARE_MUTEX_LOCK_ERROR    1

#define LEONBARE_MUTEX_UNLOCK_OK     0
#define LEONBARE_MUTEX_UNLOCK_ERROR  1


#define LEONBARE_PROTECT_DECL(flags)    unsigned long flags;
#define LEONBARE_PROTECT_KERNEL(flags)    flags = leonbare_disable_traps();
#define LEONBARE_UNPROTECT_KERNEL(flags)  leonbare_enable_traps(flags);

#define LEONBARE_PROTECT_MUTEX(flags,m)    flags = leonbare_disable_traps();
#define LEONBARE_UNPROTECT_MUTEX(flags,m)  leonbare_enable_traps(flags);

#else

#define LEONBARE_THREAD_CTX_STORE_LOCALS(base_reg) \
        std     %l0, [%base_reg + LEONBARE_THREAD_CTX_STACK_L0]; \
        std     %l2, [%base_reg + LEONBARE_THREAD_CTX_STACK_L2]; \
        std     %l4, [%base_reg + LEONBARE_THREAD_CTX_STACK_L4]; \
        std     %l6, [%base_reg + LEONBARE_THREAD_CTX_STACK_L6];

#define LEONBARE_THREAD_CTX_STORE_INS(base_reg) \
        std     %i0, [%base_reg + LEONBARE_THREAD_CTX_STACK_I0]; \
        std     %i2, [%base_reg + LEONBARE_THREAD_CTX_STACK_I2]; \
        std     %i4, [%base_reg + LEONBARE_THREAD_CTX_STACK_I4]; \
        std     %i6, [%base_reg + LEONBARE_THREAD_CTX_STACK_I6];

#define LEONBARE_THREAD_CTX_STORE_OUTS(base_reg) \
        std     %o0, [%base_reg + LEONBARE_THREAD_CTX_STACK_O0]; \
        std     %o2, [%base_reg + LEONBARE_THREAD_CTX_STACK_O2]; \
        std     %o4, [%base_reg + LEONBARE_THREAD_CTX_STACK_O4]; \
        std     %o6, [%base_reg + LEONBARE_THREAD_CTX_STACK_O6];

#define LEONBARE_THREAD_CTX_STORE_GLOBALS(base_reg) \
        st      %g1, [%base_reg + LEONBARE_THREAD_CTX_STACK_G1]; \
        std     %g2, [%base_reg + LEONBARE_THREAD_CTX_STACK_G2]; \
        std     %g4, [%base_reg + LEONBARE_THREAD_CTX_STACK_G4]; \
        std     %g6, [%base_reg + LEONBARE_THREAD_CTX_STACK_G6];


#define LEONBARE_THREAD_CTX_LOAD_LOCALS(base_reg) \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_L0], %l0; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_L2], %l2; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_L4], %l4; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_L6], %l6;

#define LEONBARE_THREAD_CTX_LOAD_INS(base_reg) \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_I0], %i0; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_I2], %i2; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_I4], %i4; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_I6], %i6;

#define LEONBARE_THREAD_CTX_LOAD_OUTS(base_reg) \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_O0], %o0; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_O2], %o2; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_O4], %o4; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_O6], %o6;

#define LEONBARE_THREAD_CTX_LOAD_GLOBALS(base_reg) \
        ld      [%base_reg + LEONBARE_THREAD_CTX_STACK_G1], %g1; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_G2], %g2; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_G4], %g4; \
        ldd     [%base_reg + LEONBARE_THREAD_CTX_STACK_G6], %g6;


#define LEONBARE_THREAD_CTX_STACK_L0 (0*8*4)
#define LEONBARE_THREAD_CTX_STACK_L2 (LEONBARE_THREAD_CTX_STACK_L0+(2*4))
#define LEONBARE_THREAD_CTX_STACK_L4 (LEONBARE_THREAD_CTX_STACK_L0+(4*4))
#define LEONBARE_THREAD_CTX_STACK_L6 (LEONBARE_THREAD_CTX_STACK_L0+(6*4))

#define LEONBARE_THREAD_CTX_STACK_I0 (1*8*4)
#define LEONBARE_THREAD_CTX_STACK_I2 (LEONBARE_THREAD_CTX_STACK_I0+(2*4))
#define LEONBARE_THREAD_CTX_STACK_I4 (LEONBARE_THREAD_CTX_STACK_I0+(4*4))
#define LEONBARE_THREAD_CTX_STACK_I6 (LEONBARE_THREAD_CTX_STACK_I0+(6*4))

#define LEONBARE_THREAD_CTX_STACK_O0 (2*8*4)
#define LEONBARE_THREAD_CTX_STACK_O2 (LEONBARE_THREAD_CTX_STACK_O0+(2*4))
#define LEONBARE_THREAD_CTX_STACK_O4 (LEONBARE_THREAD_CTX_STACK_O0+(4*4))
#define LEONBARE_THREAD_CTX_STACK_O6 (LEONBARE_THREAD_CTX_STACK_O0+(6*4))

#define LEONBARE_THREAD_CTX_STACK_G0 (3*8*4)
#define LEONBARE_THREAD_CTX_STACK_G1 (LEONBARE_THREAD_CTX_STACK_G0+(1*4))
#define LEONBARE_THREAD_CTX_STACK_G2 (LEONBARE_THREAD_CTX_STACK_G0+(2*4))
#define LEONBARE_THREAD_CTX_STACK_G4 (LEONBARE_THREAD_CTX_STACK_G0+(4*4))
#define LEONBARE_THREAD_CTX_STACK_G6 (LEONBARE_THREAD_CTX_STACK_G0+(6*4))

#define LEONBARE_THREAD_CTX_STACK_PSR    (4*8*4)
#define LEONBARE_THREAD_CTX_STACK_WIM    (LEONBARE_THREAD_CTX_STACK_PSR+4)
#define LEONBARE_THREAD_CTX_STACK_MAGIC  (LEONBARE_THREAD_CTX_STACK_PSR+8)
#define LEONBARE_THREAD_CTX_STACK_FPU  (LEONBARE_THREAD_CTX_STACK_PSR+12)

#define LEONBARE_THREAD_CTX_SZ (LEONBARE_THREAD_CTX_STACK_PSR+16)

#endif /* __ASSEMBLER__ */

# define LEONBARE_STOPALL													\
    LBDEBUG_HEADER_PRINTF(LBDEBUG_ALWAYS_NR,"Stopped at %s(%d), possibly not implemented yet\n",__FUNCTION__,__LINE__);	\
    _leonbare_Stop();

#define LEONBARE_THREAD_CTX_MAGIC 0x1234

#ifdef LBDEBUG_DO_ASSERT
#define LEONBARE_VERIFYIRQDISABLED() LBPASSERT(((leon23_getpsr() & SPARC_PSR_PIL_MASK) == SPARC_PSR_PIL_MASK),"Irq must be disabled (pil==0xf)\n",0)
#define LEONBARE_VERIFYSCHED() leonbare_sched_verify()
#else
#define LEONBARE_VERIFYIRQDISABLED()
#define LEONBARE_VERIFYSCHED()
#endif
#define LEONBARE_PRINTQUEUES() if (PDEBUG_FLAGS_CHECK(LBDEBUG_QUEUE_NR)) { leonbare_sched_printqueue(); }

#endif
