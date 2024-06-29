/*
Copyright (C) 1991 DJ Delorie
All rights reserved.

Redistribution, modification, and use in source and binary forms is permitted
provided that the above copyright notice and following paragraph are
duplicated in all such forms.

This file is distributed WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <sys/features.h>
#ifndef __IEEE_BIG_ENDIAN
#ifndef __IEEE_LITTLE_ENDIAN

/* This file can define macros to choose variations of the IEEE float
   format:

   _FLT_LARGEST_EXPONENT_IS_NORMAL

	Defined if the float format uses the largest exponent for finite
	numbers rather than NaN and infinity representations.  Such a
	format cannot represent NaNs or infinities at all, but it's FLT_MAX
	is twice the IEEE value.

   _FLT_NO_DENORMALS

	Defined if the float format does not support IEEE denormals.  Every
	float with a zero exponent is taken to be a zero representation.
 
   ??? At the moment, there are no equivalent macros above for doubles and
   the macros are not fully supported by --enable-newlib-hw-fp.

   __IEEE_BIG_ENDIAN

        Defined if the float format is big endian.  This is mutually exclusive
        with __IEEE_LITTLE_ENDIAN.

   __IEEE_LITTLE_ENDIAN
 
        Defined if the float format is little endian.  This is mutually exclusive
        with __IEEE_BIG_ENDIAN.

   Note that one of __IEEE_BIG_ENDIAN or __IEEE_LITTLE_ENDIAN must be specified for a
   platform or error will occur.

   __IEEE_BYTES_LITTLE_ENDIAN

        This flag is used in conjunction with __IEEE_BIG_ENDIAN to describe a situation 
	whereby multiple words of an IEEE floating point are in big endian order, but the
	words themselves are little endian with respect to the bytes.

   _DOUBLE_IS_32BITS 

        This is used on platforms that support double by using the 32-bit IEEE
        float type.

   _FLOAT_ARG

        This represents what type a float arg is passed as.  It is used when the type is
        not promoted to double.
	

   __OBSOLETE_MATH_DEFAULT

	Default value for __OBSOLETE_MATH if that's not set by the user.
	It should be set here based on predefined feature macros.

   __OBSOLETE_MATH

	If set to 1 then some new math code will be disabled and older libm
	code will be used instead.  This is necessary because the new math
	code does not support all targets, it assumes that the toolchain has
	ISO C99 support (hexfloat literals, standard fenv semantics), the
	target has IEEE-754 conforming binary32 float and binary64 double
	(not mixed endian) representation, standard SNaN representation,
	double and single precision arithmetics has similar latency and it
	has no legacy SVID matherr support, only POSIX errno and fenv
	exception based error handling.
*/

#if !defined(__IEEE_BIG_ENDIAN) && !defined(__IEEE_LITTLE_ENDIAN)
# if defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__FLOAT_WORD_ORDER__)
#  if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#   define __IEEE_BIG_ENDIAN
#  else
#   define __IEEE_LITTLE_ENDIAN
#  endif
# endif
#endif

#if __SIZEOF_DOUBLE__ == __SIZEOF_FLOAT__
# define _DBL_EQ_FLT
#endif

#if __SIZEOF_LONG_DOUBLE == __SIZEOF_DOUBLE__
# define LDBL_EQ_DBL
#endif

#if __SIZEOF_DOUBLE__ == 4
# define _DOUBLE_IS_32BITS
#endif

#if _SIZEOF_LONG_DOUBLE__ == 4
# define _LONG_DOUBLE_IS_32BITS
#endif

#if (defined(__arm__) || defined(__thumb__)) && !defined(__MAVERICK__)
/* arm with hard fp and soft dp cannot use new float code */
# if (__ARM_FP & 4) && !(__ARM_FP & 8)
#  define __OBSOLETE_MATH_DEFAULT_FLOAT 1
# endif
/* ARM traditionally used big-endian words; and within those words the
   byte ordering was big or little endian depending upon the target.
   Modern floating-point formats are naturally ordered; in this case
   __VFP_FP__ will be defined, even if soft-float.  */
#if defined(__VFP_FP__) || defined(__SOFTFP__)
# ifdef __ARMEL__
#  define __IEEE_LITTLE_ENDIAN
# else
#  define __IEEE_BIG_ENDIAN
# endif
#else
# define __IEEE_BIG_ENDIAN
# ifdef __ARMEL__
#  define __IEEE_BYTES_LITTLE_ENDIAN
# endif
#endif
#ifndef __SOFTFP__
# define _SUPPORTS_ERREXCEPT
#endif
/* As per ISO/IEC TS 18661 '__FLT_EVAL_METHOD__' will be defined to 16
   (if compiling with +fp16 support) so it can't be used by math.h to
   define float_t and double_t.  For values of '__FLT_EVAL_METHOD__'
   other than 0, 1, 2 the definition of float_t and double_t is
   implementation-defined.  */
#define __DOUBLE_TYPE double
#define __FLOAT_TYPE float
#endif

#if defined (__aarch64__)
#if defined (__AARCH64EL__)
#define __IEEE_LITTLE_ENDIAN
#else
#define __IEEE_BIG_ENDIAN
#endif
#ifdef __ARM_FP
# define _SUPPORTS_ERREXCEPT
#else
#ifdef __clang__
#include <float.h>
/* Clang accesses FPSR for FLT_ROUNDS with soft float target */
#undef FLT_ROUNDS
#define FLT_ROUNDS 1
#endif
#endif
/* As per ISO/IEC TS 18661 '__FLT_EVAL_METHOD__' will be defined to 16
   (if compiling with +fp16 support) so it can't be used by math.h to
   define float_t and double_t.  For values of '__FLT_EVAL_METHOD__'
   other than 0, 1, 2 the definition of float_t and double_t is
   implementation-defined.  */
#define __DOUBLE_TYPE double
#define __FLOAT_TYPE float
#endif

#ifdef __epiphany__
#define __IEEE_LITTLE_ENDIAN
#define Sudden_Underflow 1
#endif

#ifdef __hppa__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __nds32__
#ifdef __big_endian__
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __SPU__
#define __IEEE_BIG_ENDIAN

#define isfinite(__y) \
	(__extension__ ({int __cy; \
		(sizeof (__y) == sizeof (float))  ? (1) : \
		(__cy = fpclassify(__y)) != FP_INFINITE && __cy != FP_NAN;}))

#define isinf(__x) ((sizeof (__x) == sizeof (float))  ?  (0) : __isinfd(__x))
#define isnan(__x) ((sizeof (__x) == sizeof (float))  ?  (0) : __isnand(__x))

/*
 * Macros for use in ieeefp.h. We can't just define the real ones here
 * (like those above) as we have name space issues when this is *not*
 * included via generic the ieeefp.h.
 */
#define __ieeefp_isnanf(x)	0
#define __ieeefp_isinff(x)	0
#define __ieeefp_finitef(x)	1
#endif

#ifdef __sparc__
#ifdef __LITTLE_ENDIAN_DATA__
#define __IEEE_LITTLE_ENDIAN
#else
#define __IEEE_BIG_ENDIAN
#endif
#endif

#if defined(__m68k__) || defined(__mc68000__)
#define __IEEE_BIG_ENDIAN
#ifdef __HAVE_68881__
# define _SUPPORTS_ERREXCEPT
#endif
#endif

#if defined(__mc68hc11__) || defined(__mc68hc12__) || defined(__mc68hc1x__)
#define __IEEE_BIG_ENDIAN
#ifdef __HAVE_SHORT_DOUBLE__
# define _DOUBLE_IS_32BITS
#endif
#endif

#if defined (__H8300__) || defined (__H8300H__) || defined (__H8300S__) || defined (__H8500__) || defined (__H8300SX__)
#define __IEEE_BIG_ENDIAN
#define _FLOAT_ARG float
#define _DOUBLE_IS_32BITS
#endif

#if defined (__xc16x__) || defined (__xc16xL__) || defined (__xc16xS__)
#define __IEEE_LITTLE_ENDIAN
#define _FLOAT_ARG float
#define _DOUBLE_IS_32BITS
#endif


#ifdef __sh__
#define _IEEE_754_2008_SNAN 0
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define __IEEE_LITTLE_ENDIAN
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define __IEEE_BIG_ENDIAN
#endif
#ifdef __SH_FPU_ANY__
#define _SUPPORTS_ERREXCEPT
#endif
#endif

#ifdef _AM29K
#define __IEEE_BIG_ENDIAN
#endif

#ifdef _WIN32
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __i386__
#define __IEEE_LITTLE_ENDIAN
#ifndef _SOFT_FLOAT
# define _SUPPORTS_ERREXCEPT
#endif
#endif

#ifdef __riscv
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#if defined(__riscv_flen) || defined (__riscv_zfinx)
# define _SUPPORTS_ERREXCEPT
#endif
#endif

#ifdef __powerpc__

#ifdef __LITTLE_ENDIAN__
#define __IEEE_LITTLE_ENDIAN
#else
#define __IEEE_BIG_ENDIAN
#endif

#ifndef _SOFT_FLOAT
# define _SUPPORTS_ERREXCEPT
#endif

#endif

#ifdef __i960__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __lm32__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __M32R__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __nvptx__
#define __IEEE_LITTLE_ENDIAN
#endif

#if defined(_C4x) || defined(_C3x)
#define __IEEE_BIG_ENDIAN
#define _DOUBLE_IS_32BITS
#endif

#ifdef __TMS320C6X__
#ifdef _BIG_ENDIAN
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __TIC80__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __mips__
#define _IEEE_754_2008_SNAN 0
#ifndef __mips_soft_float
#define _SUPPORTS_ERREXCEPT
#endif
#endif

#ifdef __MIPSEL__
#define __IEEE_LITTLE_ENDIAN
#endif
#ifdef __MIPSEB__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __MMIX__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __D30V__
#define __IEEE_BIG_ENDIAN
#endif

/* necv70 was __IEEE_LITTLE_ENDIAN. */

#ifdef __W65__
#define __IEEE_LITTLE_ENDIAN
#define _DOUBLE_IS_32BITS
#endif

#if defined(__Z8001__) || defined(__Z8002__)
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __m88k__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __mn10300__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __mn10200__
#define __IEEE_LITTLE_ENDIAN
#define _DOUBLE_IS_32BITS
#endif

#ifdef __v800
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __v850
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __D10V__
#define __IEEE_BIG_ENDIAN
#if __DOUBLE__ == 32
#define _DOUBLE_IS_32BITS
#endif
#endif

#ifdef __PPC__
#if (defined(_BIG_ENDIAN) && _BIG_ENDIAN) || (defined(_AIX) && _AIX)
#define __IEEE_BIG_ENDIAN
#else
#if (defined(_LITTLE_ENDIAN) && _LITTLE_ENDIAN) || (defined(__sun__) && __sun__) || (defined(_WIN32) && _WIN32)
#define __IEEE_LITTLE_ENDIAN
#endif
#endif
#endif

#ifdef __xstormy16__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __arc__
#ifdef __big_endian__
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __CRX__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __CSKY__
#ifdef __CSKYBE__
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __fr30__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __FT32__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __mcore__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __mt__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __frv__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __moxie__
#ifdef __MOXIE_BIG_ENDIAN__
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __ia64__
#ifdef __BIG_ENDIAN__
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __AVR__
#define __IEEE_LITTLE_ENDIAN
#if !defined(__SIZEOF_DOUBLE__) || __SIZEOF_DOUBLE__ == 4
#define _DOUBLE_IS_32BITS
#endif
#endif

#if defined(__or1k__) || defined(__OR1K__) || defined(__OR1KND__)
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __IP2K__
#define __IEEE_BIG_ENDIAN
#define __SMALL_BITFIELDS
#define _DOUBLE_IS_32BITS
#endif

#ifdef __iq2000__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __MAVERICK__
#ifdef __ARMEL__
#  define __IEEE_LITTLE_ENDIAN
#else  /* must be __ARMEB__ */
#  define __IEEE_BIG_ENDIAN
#endif /* __ARMEL__ */
#endif /* __MAVERICK__ */

#ifdef __m32c__
#define __IEEE_LITTLE_ENDIAN
#define __SMALL_BITFIELDS
#endif

#ifdef __CRIS__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __BFIN__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __x86_64__
#define __IEEE_LITTLE_ENDIAN
#ifndef _SOFT_FLOAT
# define _SUPPORTS_ERREXCEPT
#endif
#endif

#ifdef __mep__
#ifdef __LITTLE_ENDIAN__
#define __IEEE_LITTLE_ENDIAN
#else
#define __IEEE_BIG_ENDIAN
#endif
#endif

#ifdef __MICROBLAZE__
#ifndef __MICROBLAZEEL__
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __MSP430__
#define __IEEE_LITTLE_ENDIAN
#define __SMALL_BITFIELDS	/* 16 Bit INT */
#endif

#ifdef __PRU__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __RL78__
#define __IEEE_LITTLE_ENDIAN
#define __SMALL_BITFIELDS	/* 16 Bit INT */
#ifndef __RL78_64BIT_DOUBLES__
#define _DOUBLE_IS_32BITS
#endif
#endif

#ifdef __RX__

#ifdef __RX_BIG_ENDIAN__
#define __IEEE_BIG_ENDIAN
#else
#define __IEEE_LITTLE_ENDIAN
#endif

#ifndef __RX_64BIT_DOUBLES__
#define _DOUBLE_IS_32BITS
#endif

#ifdef __RX_16BIT_INTS__
#define __SMALL_BITFIELDS
#endif

#endif

#if (defined(__CR16__) || defined(__CR16C__) ||defined(__CR16CP__))
#define __IEEE_LITTLE_ENDIAN
#define __SMALL_BITFIELDS	/* 16 Bit INT */
#endif

#ifdef __NIOS2__
# ifdef __nios2_big_endian__
#  define __IEEE_BIG_ENDIAN
# else
#  define __IEEE_LITTLE_ENDIAN
# endif
#endif

#ifdef __VISIUM__
#define __IEEE_BIG_ENDIAN
#endif

#if (defined(__XTENSA__))
#ifdef __XTENSA_EB__
#define __IEEE_BIG_ENDIAN
#endif
#ifdef __XTENSA_EL__
#define __IEEE_LITTLE_ENDIAN
#endif
#endif

#ifdef __AMDGCN__
#define __IEEE_LITTLE_ENDIAN
#endif

/* New math code requires 64-bit doubles */
#ifdef _DOUBLE_IS_32BITS
#undef __OBSOLETE_MATH
#undef __OBSOLETE_MATH_DOUBLE
#undef __OBSOLETE_MATH_FLOAT
#undef __OBSOLETE_MATH_DEFAULT_FLOAT
#undef __OBSOLETE_MATH_DEFAULT_DOUBLE
#define __OBSOLETE_MATH 1
#endif

#ifdef __XTENSA_EB__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __CYGWIN__
#define __OBSOLETE_MATH_DEFAULT 0
#endif

#ifndef __OBSOLETE_MATH_DEFAULT
#define __OBSOLETE_MATH_DEFAULT 1
#endif

#ifndef __OBSOLETE_MATH
#define __OBSOLETE_MATH __OBSOLETE_MATH_DEFAULT
#endif

#ifndef __OBSOLETE_MATH_FLOAT
#ifdef __OBSOLETE_MATH_DEFAULT_FLOAT
#define __OBSOLETE_MATH_FLOAT __OBSOLETE_MATH_DEFAULT_FLOAT
#else
#define __OBSOLETE_MATH_FLOAT __OBSOLETE_MATH
#endif
#endif

#ifndef __OBSOLETE_MATH_DOUBLE
#ifdef __OBSOLETE_MATH_DEFAULT_DOUBLE
#define __OBSOLETE_MATH_DOUBLE __OBSOLETE_MATH_DEFAULT_DOUBLE
#else
#define __OBSOLETE_MATH_DOUBLE __OBSOLETE_MATH
#endif
#endif

/* Use __FLOAT_WORD_ORDER__ if we don't have
 * more specific platform knowledge
 */
#ifndef __IEEE_BIG_ENDIAN
# ifndef __IEEE_LITTLE_ENDIAN
#  ifdef __FLOAT_WORD_ORDER__
#   if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define __IEEE_LITTLE_ENDIAN
#   endif
#   if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#    define __IEEE_BIG_ENDIAN
#   endif
#  endif
# endif
#endif

#ifndef __IEEE_BIG_ENDIAN
#ifndef __IEEE_LITTLE_ENDIAN
#error Endianess not declared!!
#endif /* not __IEEE_LITTLE_ENDIAN */
#endif /* not __IEEE_BIG_ENDIAN */

#endif /* not __IEEE_LITTLE_ENDIAN */
#endif /* not __IEEE_BIG_ENDIAN */

