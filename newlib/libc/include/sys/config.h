#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__

/* exceptions first */
/* ??? Why is much of this stuff duplicated with machine/ieeefp.h?  */
#if defined(__H8300__) || defined(__H8500__) || defined (__H8300H__) ||  defined(__W65__) || defined (__H8300S__)
#define _FLOAT_ARG float
#define __SMALL_BITFIELDS
#define _DOUBLE_IS_32BITS
#define __IEEE_BIG_ENDIAN
/* ???  This conditional is true for the h8500 and the w65, defining H8300
   in those cases probably isn't the right thing to do.  */
#define H8300 1
#endif

#ifdef __W65__
#define _DOUBLE_IS_32BITS
#define __SMALL_BITFIELDS
#define __IEEE_BIG_ENDIAN
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX 32767
#define UINT_MAX 65535

#endif

/* 16 bit integer machines */
#if defined(__Z8001__) || defined(__Z8002__) || defined(__H8300__) || defined(__H8500__) || defined(__W65__) || defined (__H8300H__) || defined (__H8300S__) || defined (__mn10200__)

#undef INT_MAX
#undef UINT_MAX
#define INT_MAX 32767
#define UINT_MAX 65535
#endif

#if defined(__D10V__)
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX __INT_MAX__
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#define _DOUBLE_IS_32BITS
#define _POINTER_INT short
#define __IEEE_BIG_ENDIAN
#endif

#ifdef ___AM29K__
#define _FLOAT_RET double
#endif

#ifdef __i386__
#ifndef __unix__
/* in other words, go32 */
#define _FLOAT_RET double
#endif
#endif

#ifdef __M32R__
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __m68k__
/* This is defined in machine/ieeefp.h; need to check is it redundant here? */
#define __IEEE_BIG_ENDIAN
#endif

#ifdef __mn10300__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __mn10200__
#define _DOUBLE_IS_32BITS
#define __SMALL_BITFIELDS
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __TIC80__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __v850
#define __IEEE_LITTLE_ENDIAN
#define __ATTRIBUTE_IMPURE_PTR__ __attribute__((__sda__))
#endif

#ifdef __D30V__
#define __IEEE_BIG_ENDIAN
#endif

/* For the PowerPC eabi, force the _impure_ptr to be in .sdata */
#if defined(__PPC__) && defined(_CALL_SYSV)
#define __ATTRIBUTE_IMPURE_PTR__ __attribute__((__section__(".sdata")))
#endif

#ifdef __sparc__
#ifdef __LITTLE_ENDIAN_DATA__
#define __IEEE_LITTLE_ENDIAN
#else
#define __IEEE_BIG_ENDIAN
#endif
#endif

#if INT_MAX == 32767
typedef long int __int32_t;
typedef unsigned long int __uint32_t;
#else
typedef int __int32_t;
typedef unsigned int __uint32_t;
#endif

#ifndef _POINTER_INT
#define _POINTER_INT long
#endif

#undef __RAND_MAX
#if INT_MAX == 32767
#define __RAND_MAX 32767
#else
#define __RAND_MAX 0x7fffffff
#endif

#endif /* __SYS_CONFIG_H__ */
