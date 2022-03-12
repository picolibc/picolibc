/* Copyright 2017 Yaakov Selkowitz <yselkowi@redhat.com> */
#include <signal.h>
#include <string.h>
#include <unistd.h>

static void (*fortify_handler)(int sig);

void
__attribute__((__noreturn__))
__chk_fail(void)
{
  char msg[] = "*** buffer overflow detected ***: terminated\n";
  write (2, msg, strlen (msg));
  if (fortify_handler)
      (*fortify_handler)(SIGABRT);
  raise (SIGABRT);
  _exit (127);
}

void
set_fortify_handler (void (*handler) (int sig))
{
    fortify_handler = handler;
}
