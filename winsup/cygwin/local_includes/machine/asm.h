#ifndef _MACHINE_ASM_H
#define _MACHINE_ASM_H

/* Macros for importing FreeBSD/OpenBSD/NetBSD assembler code. */

#ifdef __x86_64__

#define _ALIGN_TEXT		.p2align 4,0x90

/* The NATIVE_ENTRY macro just adds the required gas macros.  It can be
   used stand-alone if the code is so short that it's better to change the
   argument registeres rather than adding the code overhead. */

#define NATIVE_ENTRY(__sym)		 \
		.text			;\
		.p2align 4		;\
		.globl __sym		;\
		.seh_proc __sym		;\
	__sym:				 \

/* The ENTRY macros cover the difference in argument passing between
   MS-ABI and SysV ABI.  Note especially that RSI/RDI are always saved
   in the stack shadow space defined by the MS_ABI, and restored when
   calling ret (see the "ret" macro below). */

#define __ENTRY(__sym)			 \
		NATIVE_ENTRY(__sym)	 \
		movq	%rsi,8(%rsp)	;\
		movq	%rdi,16(%rsp)	;

/* ENTRY1 for functions with 1 arg */
#define ENTRY1(__sym)			 \
		__ENTRY(__sym)		 \
		movq	%rcx,%rdi	;\
		.seh_endprologue	;

/* ENTRY2 for functions with 2 args */
#define ENTRY2(__sym)			 \
		__ENTRY(__sym)		 \
		movq	%rcx,%rdi	;\
		movq	%rdx,%rsi	;\
		.seh_endprologue	;

/* ENTRY3 for functions with 3 args */
#define ENTRY3(__sym)			 \
		__ENTRY(__sym)		 \
		movq	%rcx,%rdi	;\
		movq	%rdx,%rsi	;\
		movq	%r8,%rdx	;\
		.seh_endprologue	;

/* ENTRY4 for functions with 4 args */
#define ENTRY4(__sym)			 \
		__ENTRY(__sym)		 \
		movq	%rcx,%rdi	;\
		movq	%rdx,%rsi	;\
		movq	%r8,%rdx	;\
		movq	%r9,%rcx	;\
		.seh_endprologue	;

#define ret				 \
		movq	8(%rsp),%rsi	;\
		movq	16(%rsp),%rdi	;\
		retq			;\

#define END(__sym)			 \
		.seh_endproc

#endif /* __x86_64__ */

#define __FBSDID(s)			 \
		.ident s

#define STRONG_ALIAS(__a,__s)		 \
		.globl	__a		;\
		__a = __s		;

#endif /* _MACHINE_ASM_H */
