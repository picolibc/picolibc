#include <signal.h>
#include <string.h>
#include <unistd.h>

void
__attribute__((__noreturn__))
__chk_fail(void)
{
  char msg[] = "*** buffer overflow detected ***: terminated\n";
  write (2, msg, strlen (msg));
  raise (SIGABRT);
  _exit (127);
}
