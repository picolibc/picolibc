// crt1-boards.S
//
// For most hardware / boards, this code sets up the C calling context
// (setting up stack, PS, and clearing BSS) and jumps to __clibrary_start
// which sets up the C library, calls constructors and registers destructors,
// and calls main().
//
// Control arrives here at _start from the reset vector or from crt0-app.S.

// Copyright (c) 1998-2013 Tensilica Inc.
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

#include <xtensa/corebits.h>


// Exports
.global _start

// Imports
//   __clibrary_init		from C library (eg. newlib or uclibc)
//   exit			from C library
//   main			from user application
//   board_init			board-specific (uart/mingloss/tinygloss.c)
//   xthal_dcache_all_writeback	from HAL library
//   __stack			from linker script (see LSP Ref Manual)
//   __bss_start		from linker script (see LSP Ref Manual)
//   _end			from linker script (see LSP Ref Manual)

.type	main, @function

# define CALL	call4
# define CALLX	callx4
# define ARG1	a6	/* 1st outgoing call argument */
# define ARG2	a7	/* 2nd outgoing call argument */
# define ARG3	a8	/* 3rd outgoing call argument */
# define ARG4	a9	/* 4th outgoing call argument */
# define ARG5	a10	/* 5th outgoing call argument */


/**************************************************************************/

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

	// Initialize the stack pointer.
	// See the "ABI and Software Conventions" chapter in the
	// Xtensa ISA Reference manual for details.

	// NOTE: Because the _start routine does not use any memory in its
	// stack frame, and because all of its CALL instructions use a
	// window size of 4 (or zero), the stack frame for _start can be empty.

	movi	sp, __stack

	/*
	 *  Now that sp (a1) is set, we can set PS as per the application
	 *  (user vector mode, enable interrupts, enable window exceptions if applicable).
	 */
	movi	a3, PS_UM|PS_WOE
	wsr	a3, PS
	rsync


	/*
	 *  Do any initialization that affects the memory map, such as
	 *  setting up TLB entries, that needs to be done before we can
	 *  successfully clear BSS (e.g. if some BSS segments are in
	 *  remapped areas).
	 *
	 *  NOTE:  This hook works where the reset vector does not unpack
	 *  segments (see "ROM packing" in the LSP manual), or where
	 *  unpacking of segments is not affected by memory remapping.
	 *  If ROM unpacking is affected, TLB setup must be done in
	 *  assembler from the reset vector.
	 *
	 *  The __memmap_init() routine can be a C function, however it
	 *  does not have BSS initialized!  In particular, __memmap_init()
	 *  cannot set BSS variables, i.e. uninitialized global variables
	 *  (they'll be wiped out by the following BSS clear), nor can it
	 *  assume they are yet initialized to zero.
	 *
	 *  The __memmap_init() function is optional.  It is marked as a
	 *  weak symbol, so that it gets valued zero if not defined.
	 */
	.weak	__memmap_init
	movi	a4, __memmap_init
	beqz	a4, 1f
	CALLX	a4
1:

	/*
	 *  Clear the BSS (uninitialized data) segments.
	 *  This code supports multiple zeroed sections (*.bss).
	 *
	 */
	movi	a6, __bss_start
	movi	a8, _end
	sub	a8, a8, a6
	movi	a7, 0
	CALL	memset

	/* init semihosting if has function */
	.weak __semihosting_init
	movi	a4, __semihosting_init
	beqz	a4, 2f
	CALLX	a4
2:

	//  We can now call C code, the C calling environment has been initialized.
	//
	//  From this point on, we use ABI-specific macros to refer to registers a0 .. a15
	//  (ARG#).


	.type	board_init, @function
	.type	__clibrary_init, @function
	.type	exit, @function


	//  Initialize the board (eg. UART, etc).
	CALL	board_init

	/*
	 *  Call __clibrary_init to initialize the C library:
	 *
	 *  void __clibrary_init(int argc, char ** argv, char ** environ,
	 *		void(*init_func)(void), void(*fini_func)(void));
	 */

	//  Pass an empty argv array, with an empty string as the program name.

	.weak _init
	.weak _fini
	movi	ARG1, _start_argc	// argc address
	movi	ARG2, _start_argv	// argv = ["", 0]
	movi	ARG3, _start_envp	// envp = [0]
	movi 	ARG4, _init		// function that calls constructors
	movi	ARG5, _fini		// function that calls destructors
	l32i	ARG1, ARG1, 0		// argc = 1
	CALL	__clibrary_init

	//  Call:   int main(int argc, char ** argv, char ** environ);
	movi	ARG1, _start_argc	// argc address
	movi	ARG2, _start_argv	// argv = ["", 0]
	movi	ARG3, _start_envp	// envp = [0]
	l32i	ARG1, ARG1, 0		// argc = 1
	CALL	main
	//  The return value is the same register as the first outgoing argument.
	CALL	exit			// exit with main's return value
	// Does not return here.

	.data
	//  Mark argc/argv/envp parameters as weak so that an external
	//  object file can override them.
	.weak	_start_argc, _start_argv, _start_envp
	.align	4
_start_argv:
	.word	_start_null	// empty program name
_start_null:
_start_envp:
	.word	0		// end of argv array, empty string, empty environ
_start_argc:
	.word	1		// one argument (program name)
	.text

	.size	_start, . - _start

