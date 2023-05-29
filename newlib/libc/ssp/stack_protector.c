/* Copyright (c) 2017 Yaakov Selkowitz <yselkowi@redhat.com> */
#include <sys/cdefs.h>
#include <sys/param.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#if defined(__AMDGCN__) || defined(__nvptx__)
/* Global constructors not supported on this target, yet.  */
uintptr_t __stack_chk_guard = 0x00000aff; /* 0, 0, '\n', 255  */

#else
uintptr_t __stack_chk_guard = 0;

int     getentropy (void *, size_t) _ATTRIBUTE((__weak__));

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

void __stack_chk_fail (void) __attribute__((__noreturn__));

#define STACK_CHK_MSG "*** stack smashing detected ***: terminated"

void
__attribute__((__noreturn__))
__stack_chk_fail_weak (void)
{
#ifdef TINY_STDIO
  puts(STACK_CHK_MSG);
#else
  static const char msg[] = STACK_CHK_MSG "\n";
  write (2, msg, sizeof(msg)-1);
#endif
  raise (SIGABRT);
  _exit (127);
}
__weak_reference(__stack_chk_fail_weak, __stack_chk_fail);

#ifdef __ELF__
void
__attribute__((visibility ("hidden")))
__stack_chk_fail_local (void)
{
	__stack_chk_fail();
}
#endif
