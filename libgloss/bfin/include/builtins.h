/*
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

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_2_4)
#pragma diag(suppress:misra_rule_5_3)
#pragma diag(suppress:misra_rule_6_3)
#pragma diag(suppress:misra_rule_8_1)
#pragma diag(suppress:misra_rule_8_8)
#pragma diag(suppress:misra_rule_8_5)
#pragma diag(suppress:misra_rule_19_7)
#pragma diag(suppress:misra_rule_19_15)
#pragma diag(suppress:misra_rule_20_2)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__NO_BUILTIN)

/* VDSP -> GCC glue */
#define __builtin_NOP()            __asm__ __volatile__ ("NOP;")
#define __builtin_cli()            ({ unsigned int __rval; __asm__ __volatile__ ("cli %0;" : "=r"(__rval)); __rval; })
#define __builtin_sti(x)           __asm__ __volatile__ ("sti %0;" : : "r"(x))
#define __builtin_idle()           __asm__ __volatile__ ("IDLE;")
#define __builtin_raise(x)         __asm__ __volatile__ ("raise %0;" : : "n"(x))
#define __builtin_excpt(x)         __asm__ __volatile__ ("excpt %0;" : : "n"(x))
#define __builtin_prefetch(x)      __asm__ __volatile__ ("PREFETCH[%0];" : : "p"(x))
#define __builtin_prefetchmodup(x) ({ void *__p = &(x); __asm__ __volatile__ ("PREFETCH[%0++];" : "+p"(__p)); __p; })
#define __builtin_flushinv(x)      __asm__ __volatile__ ("FLUSHINV[%0];" : : "p"(x))
#define __builtin_flushinvmodup(x) ({ void *__p = &(x); __asm__ __volatile__ ("FLUSHINV[%0++];" : "+p"(__p)); __p; })
#define __builtin_flush(x)         __asm__ __volatile__ ("FLUSH[%0];" : : "p"(x))
#define __builtin_flushmodup(x)    ({ void *__p = &(x); __asm__ __volatile__ ("FLUSH[%0++];" : "+p"(__p)); __p; })
#define __builtin_iflush(x)        __asm__ __volatile__ ("IFLUSH[%0];" : : "p"(x))
#define __builtin_iflushmodup(x)   ({ void *__p = &(x); __asm__ __volatile__ ("IFLUSH[%0++];" : "+p"(__p)); __p; })
#define __builtin_csync()          __builtin_bfin_csync()
#define __builtin_ssync()          __builtin_bfin_ssync()

#endif /* __NO_BUILTIN */


#if !defined(__NO_BUILTIN) && !defined(__NO_SHORTNAMES)

#if (!defined(__DEFINED_NOP) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_NOP)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_NOP))))

#define __DEFINED_NOP

/* Insert a normal 16 bit NOP, which is treated as volatile.
*/

#pragma inline
#pragma always_inline
static void  NOP(void) {
  __builtin_NOP();
}

#endif /* __DEFINED_NOP */

#if (!defined(__DEFINED_CLI) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_CLI)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_CLI))))

#define __DEFINED_CLI

#pragma inline
#pragma always_inline
static unsigned int  cli(void) {
  unsigned int  __rval = __builtin_cli();
  return __rval;
}

#endif /* __DEFINED_CLI */

#if (!defined(__DEFINED_STI) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_STI)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_STI))))

#define __DEFINED_STI

#pragma inline
#pragma always_inline
static void  sti(unsigned int  __a) {
  __builtin_sti(__a);
}

#endif /* __DEFINED_STI */

#if (!defined(__DEFINED_IDLE) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_IDLE)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_IDLE))))

#define __DEFINED_IDLE

#pragma inline
#pragma always_inline
static void  idle(void) {
  __builtin_idle();
}

#endif /* __DEFINED_IDLE */

#if (!defined(__DEFINED_RAISE_INTR) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_RAISE_INTR)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_RAISE_INTR))))

#define __DEFINED_RAISE_INTR

#define raise_intr(A) (__builtin_raise((A)))

#endif /* __DEFINED_RAISE_INTR */

#if (!defined(__DEFINED_EXCPT) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_EXCPT)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_EXCPT))))

#define __DEFINED_EXCPT

#define excpt(A) (__builtin_excpt((A)))

#endif /* __DEFINED_EXCPT */

#if (!defined(__DEFINED_PREFETCH) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_PREFETCH)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_PREFETCH))))

#define __DEFINED_PREFETCH

#pragma inline
#pragma always_inline
static void  prefetch(void * __a) {
  __builtin_prefetch(__a);
}

#endif /* __DEFINED_PREFETCH */

#if (!defined(__DEFINED_PREFETCHMODUP) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_PREFETCHMODUP)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_PREFETCHMODUP))))

#define __DEFINED_PREFETCHMODUP

#pragma inline
#pragma always_inline
static void * prefetchmodup(void * __a) {
  void * __rval = __builtin_prefetchmodup(__a);
  return __rval;
}

#endif /* __DEFINED_PREFETCHMODUP */

#if (!defined(__DEFINED_FLUSHINV) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_FLUSHINV)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_FLUSHINV))))

#define __DEFINED_FLUSHINV

#pragma inline
#pragma always_inline
static void  flushinv(void * __a) {
  __builtin_flushinv(__a);
}

#endif /* __DEFINED_FLUSHINV */

#if (!defined(__DEFINED_FLUSHINVMODUP) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_FLUSHINVMODUP)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_FLUSHINVMODUP))))

#define __DEFINED_FLUSHINVMODUP

#pragma inline
#pragma always_inline
static void * flushinvmodup(void * __a) {
  void * __rval = __builtin_flushinvmodup(__a);
  return __rval;
}

#endif /* __DEFINED_FLUSHINVMODUP */

#if (!defined(__DEFINED_FLUSH) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_FLUSH)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_FLUSH))))

#define __DEFINED_FLUSH

#pragma inline
#pragma always_inline
static void  flush(void * __a) {
  __builtin_flush(__a);
}

#endif /* __DEFINED_FLUSH */

#if (!defined(__DEFINED_FLUSHMODUP) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_FLUSHMODUP)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_FLUSHMODUP))))

#define __DEFINED_FLUSHMODUP

#pragma inline
#pragma always_inline
static void * flushmodup(void * __a) {
  void * __rval = __builtin_flushmodup(__a);
  return __rval;
}

#endif /* __DEFINED_FLUSHMODUP */

#if (!defined(__DEFINED_IFLUSH) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_IFLUSH)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_IFLUSH))))

#define __DEFINED_IFLUSH

#pragma inline
#pragma always_inline
static void  iflush(void * __a) {
  __builtin_iflush(__a);
}

#endif /* __DEFINED_IFLUSH */

#if (!defined(__DEFINED_IFLUSHMODUP) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_IFLUSHMODUP)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_IFLUSHMODUP))))

#define __DEFINED_IFLUSHMODUP

#pragma inline
#pragma always_inline
static void * iflushmodup(void * __a) {
  void * __rval = __builtin_iflushmodup(__a);
  return __rval;
}

#endif /* __DEFINED_IFLUSHMODUP */

#if (!defined(__DEFINED_CSYNC) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_CSYNC)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_CSYNC))))

#define __DEFINED_CSYNC

/* generate a csync instruction protected by CLI/STI for anomaly 05-00-0312;
** you can generate an unprotected csync by using csync_int
*/

#pragma inline
#pragma always_inline
static void csync(void) {
  __builtin_csync();
}

#endif /* __DEFINED_CSYNC */

#if (!defined(__DEFINED_SSYNC) && \
     ((defined(__SPECIFIC_NAMES) && defined(__ENABLE_SSYNC)) || \
       (!defined(__SPECIFIC_NAMES) && !defined(__DISABLE_SSYNC))))

#define __DEFINED_SSYNC

/* generate a ssync instruction protected by CLI/STI for anomaly 05-00-0312;
** you can generate an unprotected ssync by using ssync_int
*/

#pragma inline
#pragma always_inline
static void ssync(void) {
  __builtin_ssync();
}

#endif /* __DEFINED_SSYNC */

#endif /* __NO_BUILTIN */

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif

#ifdef __cplusplus
}
#endif
