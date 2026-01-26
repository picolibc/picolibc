/*
 * Macros for asm code.  AArch64 version.
 *
 * Copyright (c) 2019-2023, Arm Limited.
 * SPDX-License-Identifier: MIT 
 */

#ifndef _ASMDEFS_H
#define _ASMDEFS_H

/* Branch Target Identitication support.  */
#ifdef __ARM_FEATURE_BTI_DEFAULT
#define BTI_C		hint	34
#define BTI_J		hint	36
#define FEATURE_1_BTI   1
#else
#define BTI_C
#define BTI_J
#define FEATURE_1_BTI   0
#endif

/* Return address signing support (pac-ret).  */
#ifdef __ARM_FEATURE_PAC_DEFAULT
#define PACIASP		hint	25;
#define AUTIASP		hint	29;
#define FEATURE_1_PAC   2
#else
#define PACIASP
#define AUTIASP
#define FEATURE_1_PAC   0
#endif

/* GNU_PROPERTY_AARCH64_* macros from elf.h.  */
#define FEATURE_1_AND 0xc0000000

/* Add a NT_GNU_PROPERTY_TYPE_0 note.  */
#ifdef __ILP32__
#define GNU_PROPERTY(type, value)	\
  .section .note.gnu.property, "a";	\
  .p2align 2;				\
  .word 4;				\
  .word 12;				\
  .word 5;				\
  .asciz "GNU";				\
  .word type;				\
  .word 4;				\
  .word value;				\
  .text
#else
#define GNU_PROPERTY(type, value)	\
  .section .note.gnu.property, "a";	\
  .p2align 3;				\
  .word 4;				\
  .word 16;				\
  .word 5;				\
  .asciz "GNU";				\
  .word type;				\
  .word 4;				\
  .word value;				\
  .word 0;				\
  .text
#endif

/* If set then the GNU Property Note section will be added to
   mark objects to support BTI and PAC-RET.  */
#ifndef WANT_GNU_PROPERTY
#define WANT_GNU_PROPERTY (FEATURE_1_BTI|FEATURE_1_PAC)
#endif

#if WANT_GNU_PROPERTY
/* Add property note with supported features to all asm files.  */
GNU_PROPERTY (FEATURE_1_AND, FEATURE_1_BTI|FEATURE_1_PAC)
#endif

#define ENTRY_ALIGN(name, alignment)	\
  .global name;		\
  .type name,%function;	\
  .align alignment;		\
  name:			\
  BTI_C;

#define ENTRY(name)	ENTRY_ALIGN(name, 6)

#define ENTRY_ALIAS(name)	\
  .global name;		\
  .type name,%function;	\
  name:

#define END(name)	\
  .size name, .-name;

#define L(l) .L ## l

#ifdef __ILP32__
  /* Sanitize padding bits of pointer arguments as per aapcs64 */
#define PTR_ARG(n)  mov w##n, w##n
#else
#define PTR_ARG(n)
#endif

#ifdef __ILP32__
  /* Sanitize padding bits of size arguments as per aapcs64 */
#define SIZE_ARG(n)  mov w##n, w##n
#else
#define SIZE_ARG(n)
#endif

/* Compiler supports SVE instructions  */
#ifndef HAVE_SVE
# if __aarch64__ && (__GNUC__ >= 8 || __clang_major__ >= 5)
#   define HAVE_SVE 1
# else
#   define HAVE_SVE 0
# endif
#endif

#endif
