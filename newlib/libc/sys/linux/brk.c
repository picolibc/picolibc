/* libc/sys/linux/brk.c - Change data segment size */

/* Written 2000 by Werner Almesberger */


#include <stddef.h> /* for NULL */
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/syscall.h>


static char *curr_brk = NULL;


#define __NR___brk __NR_brk /* Linux brk ain't no brk(2) */

static _syscall1(void *,__brk,void *,end_data_segment)


int brk(void *end_data_segment)
{
     char *new_brk;

     new_brk = __brk(end_data_segment);
     if (new_brk != end_data_segment) return -1;
     curr_brk = new_brk;
     return 0;
}


void *sbrk(size_t increment) /* SHOULD be ptrdiff_t */
{
    char *old_brk,*new_brk;

    if (!curr_brk) curr_brk = __brk(NULL);
    new_brk = __brk(curr_brk+increment);
    if (new_brk != curr_brk+increment) return (void *) -1;
    old_brk = curr_brk;
    curr_brk = new_brk;
    return old_brk;
}
