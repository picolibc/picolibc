/* libc/sys/linux/crt0.c - Run-time initialization */

/* FIXME: This should be rewritten in assembler and
          placed in a subdirectory specific to a platform.
          There should also be calls to run constructors. */

/* Written 2000 by Werner Almesberger */


#include <stdlib.h>


extern char **environ;

extern int main(int argc,char **argv,char **envp);


void _start(int args)
{
    /*
     * The argument block begins above the current stack frame, because we
     * have no return address. The calculation assumes that sizeof(int) ==
     * sizeof(void *). This is okay for i386 user space, but may be invalid in
     * other cases.
     */
    int *params = &args-1;
    int argc = *params;
    char **argv = (char **) (params+1);

    environ = argv+argc+1;
    exit(main(argc,argv,environ));
}
