#ifndef _VH_H
#define _VH_H

/*
BREAK #SWID definition:
0x00 – 0x1F: Free to use
0x20 – 0x1FF: Reserved for EX9
0x0200 – 0x7EFF: Free to use
0x7F00 – 0x7FFF: Reserved for virtual hosting
*/
/* These are #SWID defined for Virtual Hosting. */
#define VH_FOPEN	0x7F00
#define VH_FREOPEN	0x7F01
#define VH_FCLOSE	0x7F02
#define VH_FFLUSH	0x7F03
#define VH_FREAD	0x7F04
#define VH_FWRITE	0x7F05
#define VH_FGETC	0x7F06
#define VH_FGETS	0x7F07
#define VH_FPUTC	0x7F08
#define VH_FPUTS	0x7F09
#define VH_UNGETC	0x7F0A
#define VH_FTELL	0x7F0B
#define VH_FSEEK	0x7F0C
#define VH_REWIND	0x7F0D
#define VH_CLEARERR	0x7F0E
#define VH_FEOF		0x7F0F
#define VH_FERROR	0x7F10
#define VH_REMOVE	0x7F11
#define VH_TMPFILE	0x7F12
/* From here, define Low-level routines. */
#define VH_EXIT		0x7F20
#define VH_OPEN         0x7F21
#define VH_CLOSE        0x7F22
#define VH_READ         0x7F23
#define VH_WRITE        0x7F24
#define VH_LSEEK        0x7F25
#define VH_UNLINK       0x7F26
#define VH_RENAME       0x7F27
#define VH_FSTAT        0x7F28
#define VH_STAT         0x7F29
#define VH_GETTIMEOFDAY 0x7F2A
#define VH_ISATTY       0x7F2B
#define VH_SYSTEM       0x7F2C
#define VH_GETERR       0x7F2D	/* The method we get errno.  */
#define VH_GETPID       0x7F2E
#define VH_KILL         0x7F2F
#define VH_TIMES        0x7F30


/* Define macros that generate assembly output.
   Generate a System Call exception to notify GDB
   to handle this virtual I/O routine.  */

.macro TYPE0 name num
/* If r0 is not NULL(0), set errno.  */
        .text
        .global \name
        .type   \name, @function
        .align  2
\name:
	BREAK	\num		/* Generate_Exception(Breakpoint);  */
	bnez	$r0, 1f		/* Branch if success.
				   r0 value is not NULL(0).  */
	BREAK	VH_GETERR
	l.w	$r15, _impure_ptr
        swi	$r0, [$r15]	/* Set errno.  */
	move	$r0, 0		/* Set return value as 0.  */
1:
        ret
        .size   \name, .-\name
.endm

.macro TYPE1 name num
/* If r0 is EOF(-1), set errno.  */
        .text
        .global \name
        .type   \name, @function
        .align  2
\name:
	BREAK	\num		/* Generate_Exception(Breakpoint);  */
	addi	$r15, $r0, 1
        bnezs8  1f		/* Branch if success.
				   r0 value is EOF(-1).  */
	BREAK	VH_GETERR
	l.w	$r15, _impure_ptr
        swi	$r0, [$r15]	/* Set errno.  */
        move    $r0, -1		/* Set return value as -1.  */
1:
        ret
        .size   \name, .-\name
.endm

.macro TYPE2 name num
/* If r0 is less than r2, set errno.  */
        .text
        .global \name
        .type   \name, @function
        .align  2
\name:
	BREAK	\num		/* Generate_Exception(Breakpoint);  */
	slt	$r15, $r0, $r2	/* If r15 is set, set errno.  */
        beqzs8  1f		/* Branch if success.
				   r15 is zero.  */
	move	$r4, $r0	/* Keep return value r0.  */
	BREAK	VH_GETERR
	l.w	$r15, _impure_ptr
        swi	$r0, [$r15]	/* Set errno.  */
        move    $r0, $r4	/* Restore r0.  */
1:
        ret
        .size   \name, .-\name
.endm

.macro TYPE3 name num
/* No errors are defined.  */
        .text
        .global \name
        .type   \name, @function
        .align  2
\name:
	BREAK	\num		/* Generate_Exception(Breakpoint);  */
        ret
        .size   \name, .-\name
.endm
#endif /* _VH_H */
