/*
 * Basic startup code for Blackfin processor
 *
 * Copyright (C) 2008 Analog Devices, Inc.
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

// basic startup code which
// - turns the cycle counter on
// - loads up FP & SP (both supervisor and user)
// - initialises the device drivers (FIOCRT)
// - calls monstartup to set up the profiling routines (PROFCRT)
// - calls the C++ startup (CPLUSCRT)
// - initialises argc/argv (FIOCRT/normal)
// - calls _main
// - calls _exit (which calls monexit to dump accumulated prof data (PROFCRT))
// - defines dummy IO routines (!FIOCRT)

#include <sys/platform.h>
#include <cplb.h>
#include <sys/anomaly_macros_rtl.h>

#define IVBh (EVT0 >> 16)
#define IVBl (EVT0 & 0xFFFF)
#define UNASSIGNED_VAL 0
#define UNASSIGNED_FILL 0
// just IVG15
#define INTERRUPT_BITS 0x400
#if defined(_ADI_THREADS) || \
    !defined(__ADSPLPBLACKFIN__) || defined(__ADSPBF561__) || defined(__ADSPBF566__)
#define SET_CLOCK_SPEED 0
#else
#define SET_CLOCK_SPEED 1
#endif

#if SET_CLOCK_SPEED == 1
#include <sys/pll.h>
#define SET_CLK_MSEL 0x16
#define SET_CLK_DF 0
#define SET_CLK_LOCK_COUNT 0x300
#define SET_CLK_CSEL 0
#define SET_CLK_SSEL 5

/*
** CLKIN == 27MHz on the EZ-Kits.
** D==0 means CLKIN is passed to PLL without dividing.
** MSEL==0x16 means VCO==27*0x16 == 594MHz
** CSEL==0 means CCLK==VCO == 594MHz
** SSEL==5 means SCLK==VCO/5 == 118MHz
*/

#endif

#ifdef __ADSPBF561_COREB__
	.section        .b.text,"ax",@progbits
	.align 2;
	.global __coreb_start;
	.type __coreb_start, STT_FUNC;
__coreb_start:
#elif defined(__ADSPBF60x_CORE1__)
	.section        .1.text,"ax",@progbits
	.align 2;
	.global __core1_start;
	.type __core1_start, STT_FUNC;
__core1_start:
#else
	.text;
	.align 2;
	.global __start;
	.type __start, STT_FUNC;
__start:
#endif
#if WA_05000109
	// Avoid Anomaly ID 05000109.
#	define SYSCFG_VALUE 0x30
	R1 = SYSCFG_VALUE;
	SYSCFG = R1;
#endif
#if WA_05000229
   // Avoid Anomaly 05-00-0229: DMA5_CONFIG and SPI_CTL not cleared on reset.
   R1 = 0x400;
#if defined(__ADSPBF538__) || defined(__ADSPBF539__)
   P0.L = SPI0_CTL & 0xFFFF;
   P0.H = SPI0_CTL >> 16;
   W[P0] = R1.L;
#else
   P0.L = SPI_CTL & 0xFFFF;
   P0.H = SPI_CTL >> 16;
   W[P0] = R1.L;
#endif
   P0.L = DMA5_CONFIG & 0xFFFF;
   P0.H = DMA5_CONFIG >> 16;
   R1 = 0;
   W[P0] = R1.L;
#endif
	// Zap loop counters to zero, to make sure that
	// hw loops are disabled - it could be really baffling
	// if the counters and bottom regs are set, and we happen
	// to run into them.
	R7 = 0;
	LC0 = R7;
	LC1 = R7;

	// Clear the DAG Length regs too, so that it's safe to
	// use I-regs without them wrapping around.
	L0 = R7;
	L1 = R7;
	L2 = R7;
	L3 = R7;

	// Zero ITEST_COMMAND and DTEST_COMMAND
	// (in case they have crud in them and
	// does a write somewhere when we enable cache)
	I0.L = (ITEST_COMMAND & 0xFFFF);
	I0.H = (ITEST_COMMAND >> 16);
	I1.L = (DTEST_COMMAND & 0xFFFF);
	I1.H = (DTEST_COMMAND >> 16);
	R7 = 0;
	[I0] = R7;
	[I1] = R7;
	// It seems writing ITEST_COMMAND from SDRAM with icache enabled
	// needs SSYNC.
#ifdef __BFIN_SDRAM
	SSYNC;
#else
	CSYNC;
#endif

	// Initialise the Event Vector table.
	P0.H = IVBh;
	P0.L = IVBl;

	// Install __unknown_exception_occurred in EVT so that
	// there is defined behaviour.
	P0 += 2*4;		// Skip Emulation and Reset
	P1 = 13;
	R1.L = __unknown_exception_occurred;
	R1.H = __unknown_exception_occurred;
	LSETUP (L$ivt,L$ivt) LC0 = P1;
L$ivt:	[P0++] = R1;
	// Set IVG15's handler to be the start of the mode-change
	// code. Then, before we return from the Reset back to user
	// mode, we'll raise IVG15. This will mean we stay in supervisor
	// mode, and continue from the mode-change point., but at a
	// much lower priority.
	P1.H = L$supervisor_mode;
	P1.L = L$supervisor_mode;
	[P0] = P1;

	// Initialise the stack.
	// Note: this points just past the end of the section.
	// First write should be with [--SP].
#ifdef __BFIN_SDRAM
	SP.L = __end + 0x400000 - 12;
	SP.H = __end + 0x400000 - 12;
#else
#ifdef __ADSPBF561_COREB__
	SP.L=__coreb_stack_end - 12;
	SP.H=__coreb_stack_end - 12;
#elif defined(__ADSPBF60x_CORE1__)
	SP.L=__core1_stack_end - 12;
	SP.H=__core1_stack_end - 12;
#else
	SP.L=__stack_end - 12;
	SP.H=__stack_end - 12;
#endif
#endif
	usp = sp;

	// We're still in supervisor mode at the moment, so the FP
	// needs to point to the supervisor stack.
	FP = SP;

	// And make space for incoming "parameters" for functions
	// we call from here:
	SP += -12;

	// Zero out bss section
#ifdef __BFIN_SDRAM
	R0.L = ___bss_start;
	R0.H = ___bss_start;
	R1.L = __end;
	R1.H = __end;
#else
#ifdef __ADSPBF561_COREB__
	R0.L = __coreb_bss_start;
	R0.H = __coreb_bss_start;
	R1.L = __coreb_bss_end;
	R1.H = __coreb_bss_end;
#elif defined(__ADSPBF60x_CORE1__)
	R0.L = __core1_bss_start;
	R0.H = __core1_bss_start;
	R1.L = __core1_bss_end;
	R1.H = __core1_bss_end;
#else
	R0.L = __bss_start;
	R0.H = __bss_start;
	R1.L = __bss_end;
	R1.H = __bss_end;
#endif
#endif
	R2 = R1 - R0;
	R1 = 0;
#ifdef __ADSPBF561_COREB__
	CALL.X __coreb_memset;
#elif defined(__ADSPBF60x_CORE1__)
	CALL.X __core1_memset;
#else
	CALL.X _memset;
#endif

	R0 = INTERRUPT_BITS;
	R0 <<= 5;	// Bits 0-4 not settable.
	// CALL.X __install_default_handlers;
	R4 = R0;		// Save modified list

	R0 = SYSCFG;		// Enable the Cycle counter
	BITSET(R0,1);
	SYSCFG = R0;

#if WA_05000137
	// Avoid anomaly #05000137

	// Set the port preferences of DAG0 and DAG1 to be
	// different; this gives better performance when
	// performing dual-dag operations on SDRAM.
	P0.L = DMEM_CONTROL & 0xFFFF;
	P0.H = DMEM_CONTROL >> 16;
	R0 = [P0];
	BITSET(R0, 12);
	BITCLR(R0, 13);
	[P0] = R0;
	CSYNC;
#endif

	// Reinitialise data areas in RAM from ROM, if MemInit's
	// been used.
	// CALL.X _mi_initialize;

#if defined(__ADSPLPBLACKFIN__)
#if SET_CLOCK_SPEED == 1

#if 0
	// Check if this feature is enabled, i.e. ___clk_ctrl is defined to non-zero
	P0.L = ___clk_ctrl;
	P0.H = ___clk_ctrl;
	R0 = MAX_IN_STARTUP;
	R1 = [P0];
	R0 = R0 - R1;
	CC = R0;
	IF CC JUMP L$clock_is_set;
#endif

	// Investigate whether we are a suitable revision
	// for boosting the system clocks.
	// speed.
	P0.L = DSPID & 0xFFFF;
	P0.H = DSPID >> 16;
	R0 = [P0];
	R0 = R0.L (Z);
	CC = R0 < 2;
	IF CC JUMP L$clock_is_set;

	// Set the internal Voltage-Controlled Oscillator (VCO)
	R0 = SET_CLK_MSEL (Z);
	R1 = SET_CLK_DF (Z);
	R2 = SET_CLK_LOCK_COUNT (Z);
	CALL.X __pll_set_system_vco;

	// Set the Core and System clocks
	R0 = SET_CLK_CSEL (Z);
	R1 = SET_CLK_SSEL (Z);
	CALL.X __pll_set_system_clocks;

L$clock_is_set:
#endif
#endif /* ADSPLPBLACKFIN */

#if defined(__ADSPBF561__) || defined(__ADSPBF566__) || defined(__ADSPBF606__) || defined(__ADSPBF607__) || defined(__ADSPBF608__) || defined(__ADSPBF609__)

	// Initialise the multi-core data tables.
	// A dummy function will be called if we are not linking with
	// -multicore
	// CALL.X __mc_data_initialise;
#endif

#if 0
	// Write the cplb exception handler to the EVT if approprate and
	// initialise the CPLBs if they're needed. couldn't do
	// this before we set up the stacks.
	P2.H = ___cplb_ctrl;
	P2.L = ___cplb_ctrl;
	R0 = CPLB_ENABLE_ANY_CPLBS;
	R6 = [P2];
	R0 = R0 & R6;
	CC = R0;
	IF !CC JUMP L$no_cplbs;
#if !defined(_ADI_THREADS)
	P1.H = __cplb_hdr;
	P1.L = __cplb_hdr;
	P0.H = IVBh;
	P0.L = IVBl;
	[P0+12] = P1;   // write exception handler
#endif /* _ADI_THREADS */
	R0 = R6;
	CALL.X __cplb_init;
#endif
L$no_cplbs:
	//  Enable interrupts
	STI R4;		// Using the mask from default handlers
	RAISE 15;

	// Move the processor into user mode.
	P0.L=L$still_interrupt_in_ipend;
	P0.H=L$still_interrupt_in_ipend;
	RETI=P0;

L$still_interrupt_in_ipend:
	rti;	// keep doing 'rti' until we've 'finished' servicing all
		// interrupts of priority higher than IVG15. Normally one
		// would expect to only have the reset interrupt in IPEND
		// being serviced, but occasionally when debugging this may
		// not be the case - if restart is hit when servicing an
		// interrupt.
		//
		// When we clear all bits from IPEND, we'll enter user mode,
		// then we'll automatically jump to supervisor_mode to start
		// servicing IVG15 (which we will 'service' for the whole
		// program, so that the program is in supervisor mode.
		//
		// Need to do this to 'finish' servicing the reset interupt.

L$supervisor_mode:
	[--SP] = RETI;	// re-enables the interrupt system

	R0.L = UNASSIGNED_VAL;
	R0.H = UNASSIGNED_VAL;
#if UNASSIGNED_FILL
	R2=R0;
	R3=R0;
	R4=R0;
	R5=R0;
	R6=R0;
	R7=R0;
	P0=R0;
	P1=R0;
	P2=R0;
	P3=R0;
	P4=R0;
	P5=R0;
#endif
	// Push a RETS and Old FP onto the stack, for sanity.
	[--SP]=R0;
	[--SP]=R0;
	// Make sure the FP is sensible.
	FP = SP;

	// And leave space for incoming "parameters"
	SP += -12;

#ifdef PROFCRT
	CALL.X monstartup; // initialise profiling routines
#endif  /* PROFCRT */

#if !defined(__ADSPBF561_COREB__) && !defined(__ADSPBF60x_CORE1__)
	CALL.X __init;

	R0.L = __fini;
	R0.H = __fini;
	CALL.X _atexit;
#endif

#if !defined(_ADI_THREADS)
#ifdef FIOCRT
	// FILE IO provides access to real command-line arguments.
	CALL.X __getargv;
	r1.l=__Argv;
	r1.h=__Argv;
#else
	// Default to having no arguments and a null list.
	R0=0;
#ifdef __ADSPBF561_COREB__
	R1.L=L$argv_coreb;
	R1.H=L$argv_coreb;
#elif defined(__ADSPBF60x_CORE1__)
	R1.L=L$argv_core1;
	R1.H=L$argv_core1;
#else
	R1.L=L$argv;
	R1.H=L$argv;
#endif
#endif /* FIOCRT */
#endif /* _ADI_THREADS */

	// At long last, call the application program.
#ifdef __ADSPBF561_COREB__
	CALL.X _coreb_main;
#elif defined(__ADSPBF60x_CORE1__)
	CALL.X _core1_main;
#else
	CALL.X _main;
#endif

#if !defined(_ADI_THREADS)
#if !defined(__ADSPBF561_COREB__) && !defined(__ADSPBF60x_CORE1__)
	CALL.X _exit;	// passing in main's return value
#endif
#endif

#ifdef __ADSPBF561_COREB__
	.size	__coreb_start, .-__coreb_start
#elif defined(__ADSPBF60x_CORE1__)
	.size	__core1_start, .-__core1_start
#else
	.size	__start, .-__start
#endif

	.align 2
	.type __unknown_exception_occurred, STT_FUNC;
__unknown_exception_occurred:
	// This function is invoked by the default exception
	// handler, if it does not recognise the kind of
	// exception that has occurred. In other words, the
	// default handler only handles some of the system's
	// exception types, and it does not expect any others
	// to occur. If your application is going to be using
	// other kinds of exceptions, you must replace the
	// default handler with your own, that handles all the
	// exceptions you will use.
	//
	// Since there's nothing we can do, we just loop here
	// at what we hope is a suitably informative label.
	IDLE;
	CSYNC;
	JUMP __unknown_exception_occurred;
	RTS;
	.size __unknown_exception_occurred, .-__unknown_exception_occurred

#if defined(__ADSPLPBLACKFIN__)
#if SET_CLOCK_SPEED == 1

/*
** CLKIN == 27MHz on the EZ-Kits.
** D==0 means CLKIN is passed to PLL without dividing.
** MSEL==0x16 means VCO==27*0x16 == 594MHz
** CSEL==0 means CCLK==VCO == 594MHz
** SSEL==5 means SCLK==VCO/5 == 118MHz
*/

// int pll_set_system_clocks(int csel, int ssel)
// returns 0 for success, -1 for error.

	.align 2
	.type __pll_set_system_clocks, STT_FUNC;
__pll_set_system_clocks:
	P0.H = PLL_DIV >> 16;
	P0.L = PLL_DIV & 0xFFFF;
	R2 = W[P0] (Z);

	// Plant CSEL and SSEL
	R0 <<= 16;
	R0.L = (4 << 8) | 2;	// 2 bits, at posn 4
	R1 <<= 16;
	R1.L = 4;		// 4 bits, at posn 0
	R2 = DEPOSIT(R2, R0);

#if defined(__WORKAROUND_DREG_COMP_LATENCY)
        // Work around anomaly 05-00-0209 which affects the DEPOSIT
        // instruction (and the EXTRACT, SIGNBITS, and EXPADJ instructions)
        // if the previous instruction created any of its operands
        NOP;
#endif

	R2 = DEPOSIT(R2, R1);

	W[P0] = R2;
	SSYNC;
	RTS;
	.size __pll_set_system_clocks, .-__pll_set_system_clocks

// int pll_set_system_vco(int msel, int df, lockcnt)
	.align 2
	.type __pll_set_system_vco, STT_FUNC;
__pll_set_system_vco:
	P0.H = PLL_CTL >> 16;
	P0.L = PLL_CTL & 0xFFFF;
	R3 = W[P0] (Z);
	P2 = R3;		// Save copy
	R3 >>= 1;		// Drop old DF
        R1 = ROT R1 BY -1;      // Move DF into CC
	R3 = ROT R3 BY 1;	// and into ctl space.
	R0 <<= 16;		// Set up pattern reg
	R0.L = (9<<8) | 6;	// (6 bits at posn 9)
        R1 = P2;                // Get the old version
	R3 = DEPOSIT(R3, R0);
	CC = R1 == R3;		// and if we haven't changed
	IF CC JUMP L$done;	// Anything, return

	CC = R2 == 0;		// Use default lockcount if
	IF CC JUMP L$wakeup;	// user one is zero.
	P2.H = PLL_LOCKCNT >> 16;
	P2.L = PLL_LOCKCNT & 0xFFFF;
	W[P2] = R2;		// Set the lock counter
L$wakeup:
	P2.H = SIC_IWR >> 16;
	P2.L = SIC_IWR & 0xFFFF;
	R2 = [P2];
	BITSET(R2, 0);		// enable PLL Wakeup
	[P2] = R2;

	W[P0] = R3;		// Update PLL_CTL
	SSYNC;

	CLI R2;			// Avoid unnecessary interrupts
	IDLE;			// Wait until PLL has locked
	STI R2;			// Restore interrupts.

L$done:
	RTS;
	.size __pll_set_system_vco, .-__pll_set_system_vco
#endif
#endif /* ADSPLPBLACKFIN */

#if defined(__ADSPBF561_COREB__) || defined(__ADSPBF60x_CORE1__)
#ifdef __ADSPBF561_COREB__
	.section        .b.text,"ax",@progbits
	.type __coreb_memset, STT_FUNC
__coreb_memset:
#else
	.section        .1.text,"ax",@progbits
	.type __core1_memset, STT_FUNC
__core1_memset:
#endif
	P0 = R0 ;              /* P0 = address */
	P2 = R2 ;              /* P2 = count   */
	R3 = R0 + R2;          /* end          */
	CC = R2 <= 7(IU);
	IF CC JUMP  .Ltoo_small;
	R1 = R1.B (Z);         /* R1 = fill char */
	R2 =  3;
	R2 = R0 & R2;          /* addr bottom two bits */
	CC =  R2 == 0;             /* AZ set if zero.	*/
	IF !CC JUMP  .Lforce_align ;  /* Jump if addr not aligned. */

.Laligned:
	P1 = P2 >> 2;          /* count = n/4        */
	R2 = R1 <<  8;         /* create quad filler */
	R2.L = R2.L + R1.L(NS);
	R2.H = R2.L + R1.H(NS);
	P2 = R3;

	LSETUP (.Lquad_loop , .Lquad_loop) LC0=P1;
.Lquad_loop:
	[P0++] = R2;

	CC = P0 == P2;
	IF !CC JUMP .Lbytes_left;
	RTS;

.Lbytes_left:
	R2 = R3;                /* end point */
	R3 = P0;                /* current position */
	R2 = R2 - R3;           /* bytes left */
	P2 = R2;

.Ltoo_small:
	CC = P2 == 0;           /* Check zero count */
	IF CC JUMP .Lfinished;    /* Unusual */

.Lbytes:
	LSETUP (.Lbyte_loop , .Lbyte_loop) LC0=P2;
.Lbyte_loop:
	B[P0++] = R1;

.Lfinished:
	RTS;

.Lforce_align:
	CC = BITTST (R0, 0);  /* odd byte */
	R0 = 4;
	R0 = R0 - R2;
	P1 = R0;
	R0 = P0;		    /* Recover return address */
	IF !CC JUMP .Lskip1;
	B[P0++] = R1;
.Lskip1:
	CC = R2 <= 2;          /* 2 bytes */
	P2 -= P1;              /* reduce count */
	IF !CC JUMP .Laligned;
	B[P0++] = R1;
	B[P0++] = R1;
	JUMP .Laligned;
#ifdef __ADSPBF561_COREB__
.size __coreb_memset,.-__coreb_memset
#else
.size __core1_memset,.-__core1_memset
#endif
#endif

#ifdef __ADSPBF561_COREB__
	.section	.b.bss,"aw",@progbits
	.align 4
	.type	L$argv_coreb, @object
	.size	L$argv_coreb, 4
L$argv_coreb:
	.zero	4
#elif defined(__ADSPBF60x_CORE1__)
	.section	.1.bss,"aw",@progbits
	.align 4
	.type	L$argv_core1, @object
	.size	L$argv_core1, 4
L$argv_core1:
	.zero	4
#else
	.local	L$argv
	.comm	L$argv,4,4
#endif

