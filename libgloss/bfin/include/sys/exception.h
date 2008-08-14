/*
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#pragma once
#ifndef __NO_BUILTIN
#pragma system_header /* exception.h */
#endif
/************************************************************************
 *
 * exception.h
 *
 * Copyright (C) 2008 Analog Devices, Inc.
 *
 ************************************************************************/

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_5_7)
#pragma diag(suppress:misra_rule_6_3)
#pragma diag(suppress:misra_rule_19_4)
#pragma diag(suppress:misra_rule_19_7)
#pragma diag(suppress:misra_rule_19_10)
#pragma diag(suppress:misra_rule_19_13)
#endif /* _MISRA_RULES */



/*
** Definitions for user-friendly interrupt handling.
*/

/*
** Memory-Mapped Registers (MMRs) - these record what causes address
** exceptions.
*/

#define EX_DATA_FAULT_STATUS	0xFFE00008
#define EX_DATA_FAULT_ADDR	0xFFE0000C
#define EX_CODE_FAULT_STATUS	0xFFE01008
#define EX_CODE_FAULT_ADDR	0xFFE0100C

/*
** Event Vector Table
*/

#define EX_EVENT_VECTOR_TABLE	0xFFE02000

/*
** Meaning of the various bits in EXCAUSE field in SEQSTAT register.
*/

#define EX_BITS		0x3F	/* All EXCAUSE bits */
#define EX_TYPE		0x30	/* The bits which define the type */
#define EX_DEBUG	0x10	/* If set, is a debug exception type */
#define EX_SYS		0x20	/* If set, is a system exception type */
				/* If neither set, is from EXCPT instr */

#define EX_IS_DEBUG_EXCEPTION(E)	(((E)&EX_TYPE)==EX_DEBUG)
#define EX_IS_SYSTEM_EXCEPTION(E)	(((E)&EX_TYPE)==EX_SYS)
#define EX_IS_USER_EXCEPTION(E)		(((E)&EX_TYPE)==0)

/*
** Service exceptions continue from the instruction after the one
** that raised the exception.
** Error exceptions restart the instruction that raised the exception.
*/

#define EX_IS_SERVICE_EXCEPTION(E)	(!EX_IS_SYSTEM_EXCEPTION(E))
#define EX_IS_ERROR_EXCEPTION(E)	(EX_IS_SYSTEM_EXCEPTION(E))

#define EX_DB_SINGLE_STEP 0x10	/* Processor is single-stepping */
#define EX_DB_EMTRCOVRFLW 0x11	/* Emulation Trace buffer overflowed */

#define EX_SYS_UNDEFINSTR 0x21	/* Undefined instruction */
#define EX_SYS_ILLINSTRC  0x22	/* Illegal instruction combination */
#define EX_SYS_DCPLBPROT  0x23	/* Data CPLB Protection violation */
#define EX_SYS_DALIGN     0x24	/* Data access misaligned address violation */
#define EX_SYS_UNRECEVT   0x25	/* Unrecoverable event */
#define EX_SYS_DCPLBMISS  0x26	/* Data access CPLB Miss */
#define EX_SYS_DCPLBMHIT  0x27	/* Data access CPLB Multiple Hits */
#define EX_SYS_EMWATCHPT  0x28	/* Emulation watch point match */
#define EX_SYS_CACCESSEX  0x29	/* Code fetch access exception */
#define EX_SYS_CALIGN     0x2A	/* Attempted misaligned instr cache fetch */
#define EX_SYS_CCPLBPROT  0x2B	/* Code fetch CPLB Protection */
#define EX_SYS_CCPLBMISS  0x2C	/* CPLB miss on an instruction fetch */
#define EX_SYS_CCPLBMHIT  0x2D	/* Code fetch CPLB Multiple Hits */
#define EX_SYS_ILLUSESUP  0x2E	/* Illegal use of Supervisor Resource */

/*
** Meaning of the various bits in HWERRCAUSE in SEQSTAT
*/

#define EX_HWBITS (0x1F<<14)	/* bits 18:14 */

#if !defined(__ADSPLPBLACKFIN__)
#define EX_HW_NOMEM1	(0x16<<14)
#define EX_HW_NOMEM2	(0x17<<14)
#else
#define EX_HW_SYSMMR	(0x02<<14)
#define EX_HW_EXTMEM	(0x03<<14)
#endif
#define EX_HW_DMAHIT	(0x01<<14)
#define EX_HW_PERFMON (0x12<<14)
#define EX_HW_RAISE	(0x18<<14)

/*
** Meaning of the bits in DATA_FAULT_STATUS and CODE_FAULT_STATUS
*/

#define EX_DATA_FAULT_ILLADDR   (1<<19)	/* non-existent memory */
#define EX_DATA_FAULT_DAG       (1<<18)	/* 0=>DAG0, 1=>DAG1 */
#define EX_DATA_FAULT_USERSUPV  (1<<17)	/* 0=>user mode, 1=> supervisor */
#define EX_DATA_FAULT_READWRITE (1<<16)	/* 0=>read, 1=>write */
#define EX_DATA_FAULT_CPLB       0xFFFF	/* 0=>CPLB0, 1=>CPLB1, etc */

#define EX_CODE_FAULT_ILLADDR   (1<<19)	/* non-existent memory */
#define EX_CODE_FAULT_USERSUPV  (1<<17)	/* 0=>user mode, 1=> supervisor */
#define EX_CODE_FAULT_CPLB       0xFFFF	/* 0=>CPLB0, 1=>CPLB1, etc */

/*
** The kinds of interrupt that can occur. These are also the
** indices into the Event Vector Table.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ik_err=-1,
  ik_emulation,
  ik_reset,
  ik_nmi,
  ik_exception,
  ik_global_int_enable,
  ik_hardware_err,
  ik_timer,
  ik_ivg7,
  ik_ivg8,
  ik_ivg9,
  ik_ivg10,
  ik_ivg11,
  ik_ivg12,
  ik_ivg13,
  ik_ivg14,
  ik_ivg15,
  num_interrupt_kind
} interrupt_kind;

/*
** Structure for recording details of an exception or interrupt
** that has occurred.
*/

typedef struct {
  interrupt_kind kind;		/* whether interrupt, exception, etc. */
  int value;			/* interrupt number, exception type, etc. */
  void *pc;			/* PC at point where exception occurred */
  void *addr;			/* if an address faulted, which one. */
  unsigned status;		/* if an address faulted, why. */
} interrupt_info;

/*
** Macro for defining an interrupt routine
*/

typedef void (*ex_handler_fn)();

#define EX_HANDLER(KIND,NAME) \
_Pragma(#KIND) \
void NAME ()

#define EX_HANDLER_PROTO(KIND, NAME) EX_HANDLER(KIND, NAME)

#define EX_INTERRUPT_HANDLER(NAME)	EX_HANDLER(interrupt,NAME)
#define EX_EXCEPTION_HANDLER(NAME)	EX_HANDLER(exception,NAME)
#define EX_NMI_HANDLER(NAME)		EX_HANDLER(nmi,NAME)
#define EX_REENTRANT_HANDLER(NAME) \
_Pragma("interrupt_reentrant") \
EX_HANDLER(interrupt,NAME)

/*
** A convenience function for setting up the interrupt_info contents.
** Must be called from immediately with the interrupt handler.
*/

void get_interrupt_info(interrupt_kind int_kind, interrupt_info *int_info);

/*
** Diagnostics function for reporting unexpected events.
*/

void _ex_report_event(interrupt_info *int_info);

/*
** Register an interrupt handler within the EVT.
** Return previous value if there was one.
*/
ex_handler_fn register_handler(interrupt_kind int_kind, ex_handler_fn handler);

/*
** Some magic values for registering default and null handlers.
*/

#define EX_INT_DEFAULT ((ex_handler_fn)-1)
#define EX_INT_IGNORE  ((ex_handler_fn)0)

/*
** Extended function to register an interrupt handler within the EVT.
** Returns the old handler.
**
** If enabled == EX_INT_ALWAYS_ENABLE, install fn (if fn != EX_INT_IGNORE
** and fn != EX_INT_DISABLE), and then enable the interrupt in IMASK then
** return
**
** If fn == EX_INT_IGNORE, disable the interrupt
** If fn == EX_INT_DEFAULT, delete the handler entry in the EVT and disable
**   the interrupt in IMASK
** Otherwise, install the new interrupt handler.  Then,
**  If enabled == EX_INT_DISABLE, disable the interrupt in IMASK
**  If enabled == EX_INT_ENABLE, enable the interrupt in IMASK
**  otherwise leave the interrupt status alone.
*/
ex_handler_fn register_handler_ex(interrupt_kind kind, ex_handler_fn fn,
                                  int enable);

/* Constants for the enabled parameter of register_handler_ex */
#define EX_INT_DISABLE 0
#define EX_INT_ENABLE 1
#define EX_INT_KEEP_IMASK -1
#define EX_INT_ALWAYS_ENABLE 2

/*
** Allow the user to raise exceptions from C.
*/

int raise_interrupt(interrupt_kind kind, int which,
   int cmd, int arg1, int arg2);

#ifdef __cplusplus
  } /* extern "C" */
#endif

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _EXCEPTION_H */
