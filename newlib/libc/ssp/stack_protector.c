/* Copyright (c) 2017 Yaakov Selkowitz <yselkowi@redhat.com> */
#include <sys/param.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#if defined(__AMDGCN__) || defined(__nvptx__)
/* Global constructors not supported on this target, yet.  */
uintptr_t __stack_chk_guard = 0x00000aff; /* 0, 0, '\n', 255  */

#else
#ifdef __PPC__
/*
 * PowerPC uses a thread-local variable, but located *below* the
 * regular TLS block. There are two register-sized values. This will
 * end up allocated in the .data segment to make sure there is space
 * for it, then we do some pointer arithmetic using a TLS symbol
 * created by the linker that points at the start of the .tdata
 * segment.
 */
struct __stack_chk_guard {
    uintptr_t v[2];
}  __stack_chk_guard __section(".stack_chk");

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wanalyzer-out-of-bounds"
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

extern _Thread_local char __tls_first[];

#define __stack_chk_guard       (((struct __stack_chk_guard *) __tls_first)[-1].v[0])
#else
uintptr_t __stack_chk_guard = 0;
#endif

int     getentropy (void *, size_t) __weak;

void __stack_chk_init (void) __attribute__((__constructor__));

void
__attribute__((__constructor__))
__stack_chk_init (void)
{
  if (__stack_chk_guard != 0)
    return;

  if (getentropy) {
    /* Use getentropy if available */
    getentropy(&__stack_chk_guard, sizeof(__stack_chk_guard));
  } else {
    /* If getentropy is not available, use the "terminator canary". */
    ((unsigned char *)&__stack_chk_guard)[0] = 0;
    ((unsigned char *)&__stack_chk_guard)[1] = 0;
#if __SIZEOF_POINTER__ > 2
    ((unsigned char *)&__stack_chk_guard)[2] = '\n';
    ((unsigned char *)&__stack_chk_guard)[3] = 255;
#endif
  }
}
#endif

__noreturn void __stack_chk_fail (void);

#define STACK_CHK_MSG "*** stack smashing detected ***: terminated"

__typeof(__stack_chk_fail) __stack_chk_fail_weak;

__noreturn void
__stack_chk_fail_weak (void)
{
#ifdef TINY_STDIO
  puts(STACK_CHK_MSG);
#else
  static const char msg[] = STACK_CHK_MSG "\n";
  write (2, msg, sizeof(msg)-1);
#endif
  abort();
}
__weak_reference(__stack_chk_fail_weak, __stack_chk_fail);

#ifdef __ELF__

__typeof(__stack_chk_fail) __stack_chk_fail_local;

void
__attribute__((visibility ("hidden")))
__stack_chk_fail_local (void)
{
	__stack_chk_fail();
}
#endif
