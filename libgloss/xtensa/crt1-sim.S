// crt1-sim.S
// For the Xtensa simulator target, this code sets up the C calling context
// and calls main()  (via __clibrary_start).
// Control arrives here at _start from the reset vector or from crt0-app.S.

// Copyright (c) 1998-2012 Tensilica Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <xtensa/config/core-isa.h>
#include <xtensa/corebits.h>
#include <syscalls.h>

// Exports
.global _start

// Imports
//   __clibrary_init	from C library (eg. newlib or uclibc)
//   exit		from C library
//   main		from user application
//   __stack		from linker script (see LSP Ref Manual)

.type	__clibrary_init, @function
.type	main, @function
.type	exit, @function

# define CALL	call4
# define CALLX	callx4
# define ARG1	a6	/* 1st outgoing call argument */
# define ARG2	a7	/* 2nd outgoing call argument */
# define ARG3	a8	/* 3rd outgoing call argument */
# define ARG4	a9	/* 4th outgoing call argument */
# define ARG5	a10	/* 5th outgoing call argument */

		.data
		.weak	_start_envp	// allow overriding
		.align	4
_start_envp:	.word	0		// empty environ

	.text
	.align 4

_start:
	//  _start is typically NOT at the beginning of the text segment --
	//  it is always called from either the reset vector or other code
	//  that does equivalent initialization (such as crt0-app.S).
	//
	//  Assumptions on entry to _start:
	//	- low (level-one) and medium priority interrupts are disabled
	//	  via PS.INTLEVEL and/or INTENABLE (PS.INTLEVEL is expected to
	//	  be zeroed, to potentially enable them, before calling main)
	//	- C calling context not initialized:
	//	  - PS not initialized
	//	  - SP not initialized
	//	- the following are initialized:
	//	  - LITBASE, cache attributes, WindowBase, WindowStart,
	//	    CPENABLE, FP's FCR and FSR, EXCSAVE[n]

	// Keep a0 zero.  It is used to initialize a few things.
	// It is also the return address, where zero indicates
	// that the frame used by _start is the bottommost frame.
	//
	movi	a0, 0		// keep this register zero.

	wsr	a0, INTENABLE	// INTENABLE value is not defined after reset.
				//make sure that interrupts are shut off (*before* we lower PS.INTLEVEL and PS.EXCM!)

	//  Windowed register init, so we can call windowed code (eg. C code).
	movi	a1, 1
	wsr	a1, WINDOWSTART
	//  The processor always clears WINDOWBASE at reset, so no need to clear it here.
	//  It resets WINDOWSTART to 1 starting with LX2.0/X7.0 (RB-2006.0).
	//  However, assuming hard reset is not yet always practical, so do this anyway:
	wsr	a0, WINDOWBASE
	rsync

	// Set VECBASE to use our vectors instead vectors in ROM
	movi	a1, _vector_table
	wsr	a1, VECBASE


	// Run only one core
	// Multi-threading could be supported in future
	rsr.prid a1		// core and multiprocessor ID
	extui	a1, a1, 13, 1	// extract core ID
	beqz	a1, .Lcore0	// goto Lcore0 for core0 only
.Lsuspend:			// other cores are suspended
	waiti	0
	j	.Lsuspend

.Lcore0:
	// Initialize the stack pointer.
	// See the "ABI and Software Conventions" chapter in the
	// Xtensa ISA Reference manual for details.

	// NOTE: Because the _start routine does not use any memory in its
	// stack frame, and because all of its CALL instructions use a
	// window size of 4, the stack frame for _start can be empty.
	movi	sp, __stack

	// reserve stack space for
	//    - argv array
	//    - argument strings
	movi	a2, SYS_argv_size
	simcall		// returns size of argv[] + its strings in a2

	// The stack only needs 16-byte alignment.
	// However, here we round up the argv size further to 128 byte multiples
	// so that in most cases, variations in argv[0]'s path do not result in
	// different stack allocation.  Otherwise, such variations can impact
	// execution timing (eg. due to cache effects etc) for the same code and data.
	// If we have a PIF, it's more likely the extra required space is okay.
	addi	a2, a2, 127
	srli	a2, a2, 7
	slli	a2, a2, 7

	// No need to use MOVSP because we have no caller (we're the
	// base caller); in fact it's better not to use MOVSP in this
	// context, to avoid unnecessary ALLOCA exceptions and copying
	// from undefined memory:
	//   sub     a3, sp, a2
	//   movsp   sp, a3
	sub	sp, sp, a2

	/*
	 *  Now that sp (a1) is set, we can set PS as per the application
	 *  (user vector mode, enable interrupts, enable window exceptions if applicable).
	 */
	movi	a3, PS_UM|PS_WOE	// PS.WOE = 1, PS.UM = 1, PS.EXCM = 0, PS.INTLEVEL = 0
	wsr	a3, PS
	rsync

	#if XCHAL_HAVE_FP || XCHAL_HAVE_DFP
	movi	a3, 1
	wsr	a3, CPENABLE
	#endif

1:

	/*
	 *  Clear the BSS (uninitialized data) segments.
	 *  This code supports multiple zeroed sections (*.bss).
	 *  For speed, we clear memory using an ISS simcall
	 *  (see crt1-boards.S for more generic BSS clearing code).
	 */
	movi	a6, __bss_start
	movi	a7, _end
	bgeu	a6, a7, .Lnobss
.Lbssloop:
	movi	a2, SYS_memset
	l32i	a3, a6, 0	// arg1 = fill start address
	movi	a4, 0		// arg2 = fill pattern
	l32i	a5, a6, 4	// get end address
	addi	a6, a6, 8	// next bss table entry
	sub	a5, a5, a3	// arg3 = fill size in bytes
	simcall			// memset(a3,a4,a5)
	bltu	a6, a7, .Lbssloop	// loop until end of bss table
.Lnobss:

	mov	a3, sp			// tell simcall where to write argv[]
	movi	a2, SYS_argv
	simcall			// write argv[] array at a3

	movi	a2, SYS_argc
	simcall			// put argc in a2

	.weak	_init
	.weak	_fini
	mov	ARG2, sp		// argv
	movi	ARG3, _start_envp	// envp
	movi	ARG4, _init		// _init
	movi	ARG5, _fini		// _fini
	CALL	__clibrary_init

	mov	ARG1, a2		// argc
	mov	ARG2, sp		// argv
	CALL	main

	//  The return value is the same register as the first outgoing argument.
	CALL	exit			// exit with main's return value
	// Does not return here.

	.size	_start, . - _start

