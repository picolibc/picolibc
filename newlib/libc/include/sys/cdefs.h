/* libc/sys/linux/sys/cdefs.h - Helper macros for K&R vs. ANSI C compat. */

/* Written 2000 by Werner Almesberger */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Berkeley Software Design, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)cdefs.h	8.8 (Berkeley) 1/9/95
 * $FreeBSD$
 */

#ifndef	_SYS_CDEFS_H_
#define	_SYS_CDEFS_H_

#include <sys/config.h>
#include <machine/_default_types.h>

#define __PMT(args)	args
#define __DOTS    	, ...
#ifdef __cplusplus
#define __THROW throw()
#else
#define __THROW
#endif

#ifdef __GNUC__
# define __ASMNAME(cname)  __XSTRING (__USER_LABEL_PREFIX__) cname
# define _ASMNAME(cname)   __asm__(__ASMNAME(cname))
#else
# define _ASMNAME(cname)
#endif

#define __ptr_t void *
#define __long_double_t  long double

#ifndef __BOUNDED_POINTERS__
# define __bounded      /* nothing */
# define __unbounded    /* nothing */
# define __ptrvalue     /* nothing */
#endif

/*
 * Compiler feature checks.
 */
#ifndef	__has_attribute
#define	__has_attribute(x)	0
#endif
#ifndef	__has_feature
#define	__has_feature(x)	0
#endif
#ifndef	__has_builtin
#define	__has_builtin(x)	0
#endif

/*
 * Attributes.
 *
 * For many of these, the attribute isn't strictly necessary, so they
 * can be safely replaced with nothing when not supported
 */

#if __has_attribute(__pure__)
# define __pure __attribute__((__pure__))
#else
# define __pure
#endif

#if __has_attribute(__format__)
# define __picolibc_format(a,b,c) __attribute__((__format__(a,b,c)))
#else
# define __picolibc_format(a,b,c)
#endif

#if __has_attribute(__nonnull__)
# define __nonnull(params) __attribute__((__nonnull__ params))
#else
# define __nonnull(params)
#endif

#if __has_attribute(__returns_nonnull__)
# define __returns_nonnull __attribute__((__returns_nonnull__))
#else
# define __returns_nonnull
#endif

#if __has_attribute(__alloc_size__)
# define __alloc_size(a) __attribute__ ((__alloc_size__(a)))
# define __alloc_size2(a,b) __attribute__ ((__alloc_size__(a,b)))
#else
# define __alloc_size(a)
# define __alloc_size2(a,b)
#endif

#if __has_attribute(__alloc_align__)
# define __alloc_align(param) __attribute__ ((__alloc_align__(param)))
#else
# define __alloc_align(param)
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
# define __noreturn [[noreturn]]
#elif __has_attribute(__noreturn__)
# define __noreturn __attribute__((__noreturn__))
#else
# define __noreturn
#endif

#if __has_attribute(__const__)
# define __const __attribute__((__const__))
#else
# define __const
#endif

#if __has_attribute(__unused__)
# define __unused __attribute__((__unused__))
#else
# define __unused
#endif

#if __has_attribute(__used__)
# define __used __attribute__((__used__))
#else
# define __used
#endif

#if __has_attribute(__packed__)
# define __packed __attribute__((__packed__))
#else
# define __packed
#endif

#if __has_attribute(__aligned__)
# define __aligned(x) __attribute__((__aligned__(x)))
#else
# define __aligned(x)
#endif

#if __has_attribute(__section__)
# define __section(x) __attribute__((__section__(x)))
#else
# define __section(x)
#endif

#if __has_attribute(__naked__)
# define __naked __attribute__((__naked__))
#else
# define __naked
#endif

#if __has_attribute(__noinline__)
# define __noinline __attribute__((__noinline__))
#else
# define __noinline
#endif

#if __has_attribute(__always_inline__)
# define __always_inline __inline __attribute__((__always_inline__))
#else
# define __always_inline
#endif

#if __has_attribute(__warn_unused_result__)
# define __warn_unused_result __attribute__((__warn_unused_result__))
#else
# define __warn_unused_result
#endif

#if __has_attribute(__weak__)
#define __weak __attribute__((__weak__))
#else
#define __weak
#endif

#if __has_attribute(__nothrow__)
#define __nothrow __attribute__((__nothrow__))
#else
#define __nothrow
#endif

#if __has_attribute(__deprecated__)
# if __GNUC_PREREQ__(4,5) || defined(__clang__)
#  define __deprecated(m) __attribute__((__deprecated__(m)))
# else
#  define __deprecated(m) __attribute__((__deprecated__))
# endif
#else
# define __deprecated(m)
#endif

#if __has_attribute(__no_builtin__)
# define __no_builtin  __attribute__((__no_builtin__))
#elif defined(__HAVE_CC_INHIBIT_LOOP_TO_LIBCALL)
# define __no_builtin __attribute__ ((__optimize__ ("-fno-tree-loop-distribute-patterns")))
#else
# define __no_builtin
#endif

#if __has_attribute(__malloc__)
# if __GNUC_PREREQ__(11, 0)
#  define       __malloc_like_with_free(_f,_a) __attribute__((__malloc__, __malloc__(_f,_a)))
# else
#  define       __malloc_like_with_free(_f,_a) __attribute__((__malloc__))
# endif
#else
# define __malloc_like_with_free(free,free_arg)
#endif

#define	__malloc_like __malloc_like_with_free(free, 1)

#if __has_attribute(always_inline) && __has_attribute(gnu_inline)
/*
 * When this macro is defined, use it to declare inline versions of extern functions.
 */
#define __declare_extern_inline(type) extern __inline type __attribute((gnu_inline, always_inline))
#endif

#if __has_attribute(__alias__)
# define __strong_reference(sym,aliassym) \
    extern __typeof (sym) aliassym __attribute__ ((__alias__ (__STRING(sym))))
# define __strong_reference_dup(sym,aliassym)	\
    extern __typeof (aliassym) aliassym __attribute__ ((__alias__ (__STRING(sym))))
# if __has_attribute(__weak__)
#  define __weak_reference(sym,aliassym)	\
    extern __typeof (sym) aliassym __attribute__ ((__weak__, __alias__ (__STRING(sym))))
# endif
#endif

/*
 * Builtins.
 *
 * When __has_builtin isn't available, these need to be detected
 * during configuration.
 */

#if __has_builtin(__builtin_add_overflow)
#define __HAVE_BUILTIN_ADD_OVERFLOW 1
#endif
#if __has_builtin(__builtin_alloca)
#define __HAVE_BUILTIN_ALLOCA 1
#endif
#if __has_builtin(__builtin_copysign)
#define __HAVE_BUILTIN_COPYSIGN 1
#endif
#if __has_builtin(__builtin_copysignl)
#define __HAVE_BUILTIN_COPYSIGNL 1
#endif
#if __has_builtin(__builtin_ctz)
#define __HAVE_BUILTIN_CTZ 1
#endif
#if __has_builtin(__builtin_ctzl)
#define __HAVE_BUILTIN_CTZL 1
#endif
#if __has_builtin(__builtin_ctzll)
#define __HAVE_BUILTIN_CTZLL 1
#endif
#if __has_builtin(__builtin_ffs)
#define __HAVE_BUILTIN_FFS 1
#endif
#if __has_builtin(__builtin_ffsl)
#define __HAVE_BUILTIN_FFSL 1
#endif
#if __has_builtin(__builtin_ffsll)
#define __HAVE_BUILTIN_FFSLL 1
#endif
#if __has_builtin(__builtin_finitel)
#define __HAVE_BUILTIN_FINITEL 1
#endif
#if __has_builtin(__builtin_isfinite)
#define __HAVE_BUILTIN_ISFINITE 1
#endif
#if __has_builtin(__builtin_isinf)
#define __HAVE_BUILTIN_ISINF 1
#endif
#if __has_builtin(__builtin_isinfl)
#define __HAVE_BUILTIN_ISINFL 1
#endif
#if __has_builtin(__builtin_isnan)
#define __HAVE_BUILTIN_ISNAN 1
#endif
#if __has_builtin(__builtin_isnanl)
#define __HAVE_BUILTIN_ISNANL 1
#endif
#if __has_builtin(__builtin_issignalingl)
#define __HAVE_BUILTIN_ISSIGNALINGL 1
#endif
#if __has_builtin(__builtin_mul_overflow)
#define __HAVE_BUILTIN_MUL_OVERFLOW 1
#endif

#if !__has_builtin(__builtin_expect)
#define __builtin_expect(cond, exp) (cond)
#endif
#if !__has_builtin(__builtin_unreachable)
#define __builtin_unreachable()
#endif

/* Alignment builtins for better type checking and improved code generation. */
/* Provide fallback versions for other compilers (GCC/Clang < 10): */
#if !__has_builtin(__builtin_is_aligned)
#define __builtin_is_aligned(x, align)	\
	(((__uintptr_t)x & ((align) - 1)) == 0)
#endif
#if !__has_builtin(__builtin_align_up)
#define __builtin_align_up(x, align)	\
	((__typeof__(x))(((__uintptr_t)(x)+((align)-1))&(~((align)-1))))
#endif
#if !__has_builtin(__builtin_align_down)
#define __builtin_align_down(x, align)	\
	((__typeof__(x))((x)&(~((align)-1))))
#endif

#define __align_up(x, y) __builtin_align_up(x, y)
#define __align_down(x, y) __builtin_align_down(x, y)
#define __is_aligned(x, y) __builtin_is_aligned(x, y)

/*
 * When the address sanitizer is enabled, we must prevent the library
 * from even reading beyond the end of input data. This happens in
 * many optimized string functions.
 */
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define _PICOLIBC_NO_OUT_OF_BOUNDS_READS
#endif

/*  ISO C++.  */

#ifdef __cplusplus
#ifdef __HAVE_STD_CXX
#define _BEGIN_STD_C namespace std { extern "C" {
#define _END_STD_C  } }
#else
#define _BEGIN_STD_C extern "C" {
#define _END_STD_C  }
#endif
#else
#define _BEGIN_STD_C
#define _END_STD_C
#endif

#if __GNUC_PREREQ (4, 2) || defined(__clang__)
#define __GNUCLIKE_PRAGMA_DIAGNOSTIC 1
#endif

#if defined(__cplusplus)
#define	__inline	inline		/* convert to C++ keyword */
#endif /* !__cplusplus */

/*
 * The __CONCAT macro is used to concatenate parts of symbol names, e.g.
 * with "#define OLD(foo) __CONCAT(old,foo)", OLD(foo) produces oldfoo.
 * The __CONCAT macro is a bit tricky to use if it must work in non-ANSI
 * mode -- there must be no spaces between its arguments, and for nested
 * __CONCAT's, all the __CONCAT's must be at the left.  __CONCAT can also
 * concatenate double-quoted strings produced by the __STRING macro, but
 * this only works with ANSI C.
 *
 * __XSTRING is like __STRING, but it expands any macros in its argument
 * first.  It is only available with ANSI C.
 */
#if defined(__STDC__) || defined(__cplusplus)
#define	__CONCAT1(x,y)	x ## y
#define	__CONCAT(x,y)	__CONCAT1(x,y)
#define	__STRING(x)	#x		/* stringify without expanding x */
#else	/* !(__STDC__ || __cplusplus) */
#define	__CONCAT(x,y)	x/**/y
#define	__STRING(x)	"x"
#endif	/* !(__STDC__ || __cplusplus) */
#define	__XSTRING(x)	__STRING(x)	/* expand x, then stringify */

#if !__GNUC_PREREQ__(2, 95)
#define	__alignof(x)	__offsetof(struct { char __a; x __b; }, __b)
#endif

/*
 * Keywords added in C11.
 */

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L

#if !__has_feature(c_alignas)
#if (defined(__cplusplus) && __cplusplus >= 201103L) || \
    __has_feature(cxx_alignas)
#define	_Alignas(x)		alignas(x)
#else
/* XXX: Only emulates _Alignas(constant-expression); not _Alignas(type-name). */
#define	_Alignas(x)		__aligned(x)
#endif
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
#define	_Alignof(x)		alignof(x)
#else
#define	_Alignof(x)		__alignof(x)
#endif

#if !defined(__cplusplus) && !__has_feature(c_atomic) && \
	!__has_feature(cxx_atomic) && !__GNUC_PREREQ__(4, 7)
/*
 * No native support for _Atomic(). Place object in structure to prevent
 * most forms of direct non-atomic access.
 */
#define	_Atomic(T)		struct { T volatile __val; }
#endif

#if !__has_feature(c_static_assert)
#if (defined(__cplusplus) && __cplusplus >= 201103L) || \
    __has_feature(cxx_static_assert)
#define	_Static_assert(x, y)	static_assert(x, y)
#elif __GNUC_PREREQ__(4,6) && !defined(__cplusplus)
/* Nothing, gcc 4.6 and higher has _Static_assert built-in */
#elif defined(__COUNTER__)
#define	_Static_assert(x, y)	__Static_assert(x, __COUNTER__)
#define	__Static_assert(x, y)	___Static_assert(x, y)
#define	___Static_assert(x, y)	typedef char __assert_ ## y[(x) ? 1 : -1] \
				__unused
#else
#define	_Static_assert(x, y)	struct __hack
#endif
#endif

#endif /* __STDC_VERSION__ || __STDC_VERSION__ < 201112L */

/*
 * Emulation of C11 _Generic().  Unlike the previously defined C11
 * keywords, it is not possible to implement this using exactly the same
 * syntax.  Therefore implement something similar under the name
 * __generic().  Unlike _Generic(), this macro can only distinguish
 * between a single type, so it requires nested invocations to
 * distinguish multiple cases.
 *
 * Note that the comma operator is used to force expr to decay in
 * order to match _Generic().
 */

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || \
    __has_feature(c_generic_selections)
#define	__generic(expr, t, yes, no)					\
	_Generic(expr, t: yes, default: no)
#elif __GNUC_PREREQ__(3, 1) && !defined(__cplusplus)
#define	__generic(expr, t, yes, no)					\
	__builtin_choose_expr(						\
	    __builtin_types_compatible_p(__typeof((0, (expr))), t), yes, no)
#endif

/*
 * C99 Static array indices in function parameter declarations.  Syntax such as:
 * void bar(int myArray[static 10]);
 * is allowed in C99 but not in C++.  Define __min_size appropriately so
 * headers using it can be compiled in either language.  Use like this:
 * void bar(int myArray[__min_size(10)]);
 */
#if !defined(__cplusplus) && \
    (defined(__clang__) || __GNUC_PREREQ__(4, 6)) && \
    (!defined(__STDC_VERSION__) || (__STDC_VERSION__ >= 199901))
#define __min_size(x)	static (x)
#else
#define __min_size(x)	(x)
#endif

/* XXX: should use `#if __STDC_VERSION__ < 199901'. */
#if !__GNUC_PREREQ__(2, 7) && !defined(__COMPCERT__)
#define	__func__	NULL
#endif

/*
 * We use `__restrict' as a way to define the `restrict' type qualifier
 * without disturbing older software that is unaware of C99 keywords.
 * GCC also provides `__restrict' as an extension to support C99-style
 * restricted pointers in other language modes.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901
#define	__restrict	restrict
#elif !__GNUC_PREREQ__(2, 95)
#define	__restrict
#endif

/*
 * Additionally, we allow to use `__restrict_arr' for declaring arrays as
 * non-overlapping per C99.  It's not allowed in C++.
 */
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L && !defined(__cplusplus)
#define __restrict_arr       restrict
#else
#define __restrict_arr
#endif

#define __offsetof(type, field)	offsetof(type, field)
#define	__rangeof(type, start, end) \
	(__offsetof(type, end) - __offsetof(type, start))

/*
 * Given the pointer x to the member m of the struct s, return
 * a pointer to the containing structure.  When using GCC, we first
 * assign pointer x to a local variable, to check that its type is
 * compatible with member m.
 */
#if __GNUC_PREREQ__(3, 1)
#define	__containerof(x, s, m) ({					\
	const volatile __typeof(((s *)0)->m) *__x = (x);		\
	((s *) (uintptr_t) ((const volatile char *)__x - __offsetof(s, m))); \
})
#else
#define	__containerof(x, s, m)						\
        ((s *)(uintptr_t) ((const volatile char *)(x) - __offsetof(s, m)))
#endif

#if defined(__GNUC__)
#ifndef __weak_reference
#ifdef __ELF__
#ifdef __STDC__
#define	__weak_reference(sym,alias)	\
	__asm__(".weak " #alias);	\
	__asm__(".equ "  #alias ", " #sym)
#else
#define	__weak_reference(sym,alias)	\
	__asm__(".weak alias");		\
	__asm__(".equ alias, sym")
#endif
#elif __clang__
#ifdef __MACH__
/* Macos prefixes all C symbols with an underscore, this needs to be done manually in asm */

/* So far I have not been able to create on Macos an exported symbol that itself
 * is weak but aliases a strong symbol. A workaround is to make the original
 * symbol weak and the alias symbol will automatically become weak too. */
/* Hint: use `nm -m obj.o` to check the symbols weak/strong on Mac */
#define __weak_reference(sym,alias) \
	__asm__(".weak_definition _" #sym); \
	__asm__(".globl _" #alias); \
	__asm__(".set _" #alias ", _" #sym)
#elif defined(__STDC__)
#define __weak_reference(sym,alias) \
	__asm__(".weak_reference " #alias); \
	__asm__(".globl " #alias); \
	__asm__(".set " #alias ", " #sym)
#else
#define __weak_reference(sym,alias) \
	__asm__(".weak_reference alias");\
	__asm__(".set alias, sym")
#endif
#else	/* !__ELF__ && !__clang__ */
#ifdef __STDC__
#define	__weak_reference(sym,alias)	\
	__asm__(".stabs \"_" #alias "\",11,0,0,0");	\
	__asm__(".stabs \"_" #sym "\",1,0,0,0")
#else
#define	__weak_reference(sym,alias)	\
	__asm__(".stabs \"_/**/alias\",11,0,0,0");	\
	__asm__(".stabs \"_/**/sym\",1,0,0,0")
#endif
#endif
#endif

#endif	/* __GNUC__ */

/*
 * fall-through case statement annotations
 */
#if __cplusplus >= 201703L || __STDC_VERSION__ > 201710L
/* Standard C++17/C23 attribute */
# define __fallthrough [[fallthrough]]
#elif __has_attribute(__fallthrough__)
/* Non-standard but supported by at least gcc and clang */
# define __fallthrough __attribute__((__fallthrough__))
#else
# define __fallthrough do { } while(0)
#endif

/*  The traditional meaning of 'extern inline' for GCC is not
  to emit the function body unless the address is explicitly
  taken.  However this behaviour is changing to match the C99
  standard, which uses 'extern inline' to indicate that the
  function body *must* be emitted.  Likewise, a function declared
  without either 'extern' or 'static' defaults to extern linkage
  (C99 6.2.2p5), and the compiler may choose whether to use the
  inline version or call the extern linkage version (6.7.4p6).
  If we are using GCC, but do not have the new behaviour, we need
  to use extern inline; if we are using a new GCC with the
  C99-compatible behaviour, or a non-GCC compiler (which we will
  have to hope is C99, since there is no other way to achieve the
  effect of omitting the function if it isn't referenced) we use
  'static inline', which c99 defines to mean more-or-less the same
  as the Gnu C 'extern inline'.  */
#if defined(__GNUC__) && !defined(__GNUC_STDC_INLINE__)
/* We're using GCC, but without the new C99-compatible behaviour.  */
#define __elidable_inline extern __always_inline
#else
/* We're using GCC in C99 mode, or an unknown compiler which
  we just have to hope obeys the C99 semantics of inline.  */
#define __elidable_inline static __inline
#endif

#endif /* !_SYS_CDEFS_H_ */
