#include <sys/cdefs.h>
#include <sys/param.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__AMDGCN__)
/* GCN does not support constructors, yet.  */
uintptr_t __stack_chk_guard = 0x00000aff; /* 0, 0, '\n', 255  */

#else
uintptr_t __stack_chk_guard = 0;

void
__attribute__((__constructor__))
__stack_chk_init (void)
{
  if (__stack_chk_guard != 0)
    return;

#if defined(__CYGWIN__) || defined(__rtems__)
  arc4random_buf(&__stack_chk_guard, sizeof(__stack_chk_guard));
#else
  /* If getentropy is not available, use the "terminator canary". */
  ((unsigned char *)&__stack_chk_guard)[0] = 0;
  ((unsigned char *)&__stack_chk_guard)[1] = 0;
  ((unsigned char *)&__stack_chk_guard)[2] = '\n';
  ((unsigned char *)&__stack_chk_guard)[3] = 255;
#endif
}
#endif

void
__attribute__((__noreturn__))
__stack_chk_fail (void)
{
  char msg[] = "*** stack smashing detected ***: terminated\n";
  write (2, msg, strlen (msg));
  raise (SIGABRT);
  _exit (127);
}

#ifdef __ELF__
void
__attribute__((visibility ("hidden")))
__stack_chk_fail_local (void)
{
	__stack_chk_fail();
}
#endif
