/*STARTINC
 *
 *           COPYRIGHT (C) 1991, 1992 ARRAY TECHNOLOGY CORPORATION
 *                            All Rights Reserved
 *
 * This software is confidential information which is proprietary to and
 * a trade secret of ARRAY Technology Corporation.  Use, duplication, or
 * disclosure is subject to the terms of a separate license agreement.
 *
 *
 *  NAME:  
 *
 *
 *  DESCRIPTION: 
 *
 *
 *ENDINC
 */

/*	%Q%	%I%	%M%	*/

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/*
 * dbgmon.h -- debugging monitor definitions
 */

/*
 * catch bogus compiles
 */
#if defined(MIPSEB) && defined(MIPSEL)
# include "error -- both MIPSEB and MIPSEL defined"
#endif

#if !defined(MIPSEB) && !defined(MIPSEL)
# include "error -- neither MIPSEB or MIPSEL defined"
#endif

/*
 * PROM_STACK is the address of the first word above the prom stack
 * the prom stack grows downward from the first word less than PROM_STACK
 */
#define	PROM_STACK	0xa0010000

/*
 * register names
 */
#define	R_R0		0
#define	R_R1		1
#define	R_R2		2
#define	R_R3		3
#define	R_R4		4
#define	R_R5		5
#define	R_R6		6
#define	R_R7		7
#define	R_R8		8
#define	R_R9		9
#define	R_R10		10
#define	R_R11		11
#define	R_R12		12
#define	R_R13		13
#define	R_R14		14
#define	R_R15		15
#define	R_R16		16
#define	R_R17		17
#define	R_R18		18
#define	R_R19		19
#define	R_R20		20
#define	R_R21		21
#define	R_R22		22
#define	R_R23		23
#define	R_R24		24
#define	R_R25		25
#define	R_R26		26
#define	R_R27		27
#define	R_R28		28
#define	R_R29		29
#define	R_R30		30
#define	R_R31		31
#define	R_F0		32
#define	R_F1		33
#define	R_F2		34
#define	R_F3		35
#define	R_F4		36
#define	R_F5		37
#define	R_F6		38
#define	R_F7		39
#define	R_F8		40
#define	R_F9		41
#define	R_F10		42
#define	R_F11		43
#define	R_F12		44
#define	R_F13		45
#define	R_F14		46
#define	R_F15		47
#define	R_F16		48
#define	R_F17		49
#define	R_F18		50
#define	R_F19		51
#define	R_F20		52
#define	R_F21		53
#define	R_F22		54
#define	R_F23		55
#define	R_F24		56
#define	R_F25		57
#define	R_F26		58
#define	R_F27		59
#define	R_F28		60
#define	R_F29		61
#define	R_F30		62
#define	R_F31		63
#define	R_EPC		64
#define	R_MDHI		65
#define	R_MDLO		66
#define	R_SR		67
#define	R_CAUSE		68
#define	R_BADVADDR	69
#define R_DCIC          70
#define R_BPC           71
#define R_BDA           72
#define	R_EXCTYPE	73
#define	NREGS		74

/*
 * compiler defined bindings
 */
#define	R_ZERO		R_R0
#define	R_AT		R_R1
#define	R_V0		R_R2
#define	R_V1		R_R3
#define	R_A0		R_R4
#define	R_A1		R_R5
#define	R_A2		R_R6
#define	R_A3		R_R7
#define	R_T0		R_R8
#define	R_T1		R_R9
#define	R_T2		R_R10
#define	R_T3		R_R11
#define	R_T4		R_R12
#define	R_T5		R_R13
#define	R_T6		R_R14
#define	R_T7		R_R15
#define	R_S0		R_R16
#define	R_S1		R_R17
#define	R_S2		R_R18
#define	R_S3		R_R19
#define	R_S4		R_R20
#define	R_S5		R_R21
#define	R_S6		R_R22
#define	R_S7		R_R23
#define	R_T8		R_R24
#define	R_T9		R_R25
#define	R_K0		R_R26
#define	R_K1		R_R27
#define	R_GP		R_R28
#define	R_SP		R_R29
#define	R_FP		R_R30
#define	R_RA		R_R31

/*
 * memory reference widths
 */
#define	SW_BYTE		1
#define	SW_HALFWORD	2
#define	SW_WORD		4

/*
 * Monitor modes
 */
#define	MODE_DBGMON	0	/* debug monitor is executing */
#define	MODE_CLIENT	1	/* client is executing */

/*
 * String constants
 */
#define	DEFAULT_STRLEN	70		/* default max strlen for string cmd */

