/* The interrupt table.
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

	.text

	.globl __interrupt_vector
__interrupt_vector:
	.long __stack 				/* 0 */
	.long __reset				/* 1 */
	.long __access_error			/* 2 */
	.long __address_error			/* 3 */
	.long __illegal_instruction		/* 4 */
	.long __divide_by_zero			/* 5 */
	.long __other_interrupt			/* 6 */
	.long __other_interrupt			/* 7 */
	.long __privilege_violation		/* 8 */
	.long __trace				/* 9 */
	.long __unimplemented_line_a_opcode	/* 10 */
	.long __unimplemented_line_f_opcode	/* 11 */
	.long __non_pc_breakpoint_debug_interrupt /* 12 */
	.long __pc_breakpoint_debug_interrupt	/* 13 */
	.long __format_error			/* 14 */
	.rept 24 - 15
	.long __other_interrupt			/* [15,24) */
	.endr
	.long __spurious_interrupt		/* 24 */
	.rept 32 - 25
	.long __other_interrupt			/* [25,32) */
	.endr
	.long __trap0				/* 32 */
	.long __trap1				/* 33 */
	.long __trap2				/* 34 */
	.long __trap3				/* 35 */
	.long __trap4				/* 36 */
	.long __trap5				/* 37 */
	.long __trap6				/* 38 */
	.long __trap7				/* 39 */
	.long __trap8				/* 40 */
	.long __trap9				/* 41 */
	.long __trap10				/* 42 */
	.long __trap11				/* 43 */
	.long __trap12				/* 44 */
	.long __trap13				/* 45 */
	.long __trap14				/* 46 */
	.long __trap15				/* 47 */
	.long __fp_branch_unordered		/* 48 */
	.long __fp_inexact_result		/* 49 */
	.long __fp_divide_by_zero		/* 50 */
	.long __fp_underflow			/* 51 */
	.long __fp_operand_error		/* 52 */
	.long __fp_overflow			/* 53 */
	.long __fp_input_not_a_number		/* 54 */
	.long __fp_input_denormalized_number	/* 55 */
	.rept 61 - 56
	.long __other_interrupt			/* [56,61) */
	.endr
	.long __unsupported_instruction		/* 61 */
	.rept 64 - 62
	.long __other_interrupt			/* [62,64) */
	.endr
	.rept 256-64
	.long __other_interrupt			/* [64,256) */
	.endr
