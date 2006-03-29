/*
 * bdm-isv.c -- 
 *
 * Copyright (c) 2006 CodeSourcery Inc
 *
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

/* This file contains default interrupt handlers and initialization
   code for the interrupt vector.  All but one of the interrupts are
   user replaceable.

   User trap BDM_TRAP (15) is used for semi hosting support.
   If you replace this one, semihosting will cease to function. */

#include "bdm-semihost.h"

#define NUM_VECTORS 256

#define ISR_DECLARE(NAME) void __attribute__((interrupt_handler)) NAME (void)

ISR_DECLARE (__other_interrupt);
ISR_DECLARE (__reset);
ISR_DECLARE (__access_error);
ISR_DECLARE (__address_error);
ISR_DECLARE (__illegal_instruction);
ISR_DECLARE (__divide_by_zero);
ISR_DECLARE (__privilege_violation);
ISR_DECLARE (__trace);
ISR_DECLARE (__unimplemented_line_a_opcode);
ISR_DECLARE (__unimplemented_line_f_opcode);
ISR_DECLARE (__non_pc_breakpoint_debug_interrupt);
ISR_DECLARE (__pc_breakpoint_debug_interrupt);
ISR_DECLARE (__format_error);
ISR_DECLARE (__spurious_interrupt);
ISR_DECLARE (__trap0);
ISR_DECLARE (__trap1);
ISR_DECLARE (__trap2);
ISR_DECLARE (__trap3);
ISR_DECLARE (__trap4);
ISR_DECLARE (__trap5);
ISR_DECLARE (__trap6);
ISR_DECLARE (__trap7);
ISR_DECLARE (__trap8);
ISR_DECLARE (__trap9);
ISR_DECLARE (__trap10);
ISR_DECLARE (__trap11);
ISR_DECLARE (__trap12);
ISR_DECLARE (__trap13);
ISR_DECLARE (__trap14);
ISR_DECLARE (__trap15);
ISR_DECLARE (__fp_branch_unordered);
ISR_DECLARE (__fp_inexact_result);
ISR_DECLARE (__fp_divide_by_zero);
ISR_DECLARE (__fp_underflow);
ISR_DECLARE (__fp_operand_error);
ISR_DECLARE (__fp_overflow);
ISR_DECLARE (__fp_input_not_a_number);
ISR_DECLARE (__fp_input_denormalized_number);
ISR_DECLARE (__unsupported_instruction);

/* The trap used for semihosting by the debugger.  This must have a
   particular assembly signature, so we don't generate it with the
   compiler.   */
ISR_DECLARE (__bdm_semihosting);

/* The interrupt vector itself must be provided by the linker script
   as it requires 1MB alignment. */
extern void (*__interrupt_vector[NUM_VECTORS])(void);

/* The linker script sets the stack pointer too.  */
extern int  __attribute__ ((weak)) __stack;

/* This hook is called during crt startup and installs and initializes
   the vector table.  It is overridable by a user provided routine.
   If the user routine fails to install the __bdm_semihosting routine,
   semihosting will cease to function.   */

void  software_init_hook (void)
{
  unsigned ix;

  /* Don't set it if it's -1 (zero is a valid value) */
  if ((long)&__interrupt_vector == -1)
    return;
  
  for (ix = 0; ix != NUM_VECTORS; ix++)
    __interrupt_vector[ix] = &__other_interrupt;
  
  /* Set the VBR. */
  __asm__ __volatile__ ("movec.l %0,%/vbr" :: "r" (&__interrupt_vector));

  /* Set an initial stack and reset vector, in case we unexpectedly get
     reset.  */
  __interrupt_vector[0] = (&__stack ? (void (*)(void))&__stack
			   : (void (*)(void))&__interrupt_vector[NUM_VECTORS]);
  __interrupt_vector[1] = &__reset;
  
  /* Store the known interrupt vectors */
  __interrupt_vector[2] = &__access_error;
  __interrupt_vector[3] = &__address_error;
  __interrupt_vector[4] = &__illegal_instruction;
  __interrupt_vector[5] = &__divide_by_zero;
  __interrupt_vector[8] = &__privilege_violation;
  __interrupt_vector[9] = &__trace;
  __interrupt_vector[10] = &__unimplemented_line_a_opcode;
  __interrupt_vector[11] = &__unimplemented_line_f_opcode;
  __interrupt_vector[12] = &__non_pc_breakpoint_debug_interrupt;
  __interrupt_vector[13] = &__pc_breakpoint_debug_interrupt;
  __interrupt_vector[14] = &__format_error;
  __interrupt_vector[24] = &__spurious_interrupt;
  __interrupt_vector[32] = &__trap0;
  __interrupt_vector[33] = &__trap1;
  __interrupt_vector[34] = &__trap2;
  __interrupt_vector[35] = &__trap3;
  __interrupt_vector[36] = &__trap4;
  __interrupt_vector[37] = &__trap5;
  __interrupt_vector[38] = &__trap6;
  __interrupt_vector[39] = &__trap7;
  __interrupt_vector[40] = &__trap8;
  __interrupt_vector[41] = &__trap9;
  __interrupt_vector[42] = &__trap10;
  __interrupt_vector[43] = &__trap11;
  __interrupt_vector[44] = &__trap12;
  __interrupt_vector[45] = &__trap13;
  __interrupt_vector[46] = &__trap14;
  __interrupt_vector[47] = &__trap15;
  __interrupt_vector[48] = &__fp_branch_unordered;
  __interrupt_vector[49] = &__fp_inexact_result;
  __interrupt_vector[50] = &__fp_divide_by_zero;
  __interrupt_vector[51] = &__fp_underflow;
  __interrupt_vector[52] = &__fp_operand_error;
  __interrupt_vector[53] = &__fp_overflow;
  __interrupt_vector[54] = &__fp_input_not_a_number;
  __interrupt_vector[55] = &__fp_input_denormalized_number;
  __interrupt_vector[61] = &__unsupported_instruction;

  /* Install the special handler. */
  __interrupt_vector[0x20 + BDM_TRAP] = &__bdm_semihosting;
}
