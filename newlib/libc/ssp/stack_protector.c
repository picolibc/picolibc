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

#ifdef __THREAD_LOCAL_STORAGE_STACK_GUARD
#include "machine/_ssp_tls.h"
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
#ifdef __TINY_STDIO
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
