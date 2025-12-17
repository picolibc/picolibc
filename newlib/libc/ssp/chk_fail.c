/* Copyright 2017 Yaakov Selkowitz <yselkowi@redhat.com> */
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ssp/ssp.h>
#undef puts

static void (*fortify_handler)(int sig);

#define CHK_FAIL_MSG "*** overflow detected ***: terminated"

__noreturn void
__chk_fail(void)
{
    puts(CHK_FAIL_MSG);
    if (fortify_handler)
        (*fortify_handler)(SIGABRT);
    abort();
}

void
set_fortify_handler(void (*handler)(int sig))
{
    fortify_handler = handler;
}
