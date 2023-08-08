/*
 * Copyright (C) 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <unistd.h>
#include <syscalls.h>
#include <sys/stat.h>
#include <soc/uart.h>

#if defined (OPENOCD_SEMIHOSTING) || (QEMU_SEMIHOSTING)
# define WITH_SEMIHOSTING
# define __WEAK_FUNCTION_ATTR__
#else
# define __WEAK_FUNCTION_ATTR__ __attribute__((__weak__))
#endif


/* __semihosting_call is a function in case semihosting usage, macro (-1) otherwise */
#ifdef WITH_SEMIHOSTING

static inline int
__attribute__ ((always_inline))
__semihosting_call(int id, int arg1, int arg2, int arg3, int arg4)
{
# ifdef OPENOCD_SEMIHOSTING
    register long a2 asm("a2") = id;
    long args[] = {arg1, arg2, arg3, arg4};
    register long a3 asm("a3") = (long)&args;

    /* OpenOCD has different semihosting api for sys_exit on 32-bit and 64-bit */
    if (id == SYS_exit && sizeof(void *) != 8) {
        a3 = ADP_STOPPED_APPLICATION_EXIT;
    }
    __asm__  __volatile__ (
        "break 1, 14\n"
        : "+r"(a2): "r"(a3)
        : "memory");

    // return code is placed in a2 register, so return it to the caller
    return a2;
# else // OPENOCD_SEMIHOSTING
    extern int __sim_call(int id, int arg1, int arg2, int arg3, int arg4);
    return __sim_call(id, arg1, arg2, arg3, arg4);
# endif // OPENOCD_SEMIHOSTING
}

# ifdef OPENOCD_SEMIHOSTING

int
__semihosting_init (void)
{
    struct {
        int version;
    } ver_info = { 2 };
    __semihosting_call(ESP_SEMIHOSTING_SYS_DRV_INFO, (long) &ver_info, sizeof(ver_info), 0, 0);
}

# endif // OPENOCD_SEMIHOSTING

#else // !WITH_SEMIHOSTING
# define __semihosting_call(...) (-1)
#endif // WITH_SEMIHOSTING


void
__WEAK_FUNCTION_ATTR__
__attribute__ ((noreturn))
_exit (int status)
{
    __semihosting_call(SYS_exit, status, 0, 0, 0);

    for (;;) {
        ;
    }
}


int
__WEAK_FUNCTION_ATTR__
_open (const char *file, int flags, int mode)
{
    return __semihosting_call(SYS_open, (int) file, flags, mode, 0);
}


int
__WEAK_FUNCTION_ATTR__
_lseek (int fd, _off_t off, int whence)
{
    return __semihosting_call(SYS_lseek, fd, off, whence, 0);
}


int
__WEAK_FUNCTION_ATTR__
_close (int fd)
{
    return __semihosting_call(SYS_close, fd, 0, 0, 0);
}


_ssize_t
__WEAK_FUNCTION_ATTR__
_write (int fd, const char *buf, size_t cnt)
{
    int ret = 0;
#ifdef WITH_SEMIHOSTING
    ret = __semihosting_call(SYS_write, fd, (int) buf, cnt, 0);
# ifdef OPENOCD_SEMIHOSTING
    /* ret - number of bytes that are NOT written. Calculate written */
    ret = cnt - ret;
# endif // OPENOCD_SEMIHOSTING
#else // !WITH_SEMIHOSTING
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
        return -1;
    }

    for (uint32_t i = 0; i < cnt; i++) {
        board_uart_write_char(buf[i]);
    }
    ret = cnt;
#endif // WITH_SEMIHOSTING
    return ret;
}


/* Do not compile functions with common implementation
 * if building semihosting library
 */
#ifndef WITH_SEMIHOSTING

struct _reent*
__WEAK_FUNCTION_ATTR__
__getreent(void)
{
    return _GLOBAL_REENT;
}


int
__WEAK_FUNCTION_ATTR__
_fstat (int fd, struct stat *pstat)
{

    if (fd < STDERR_FILENO)
    {
        pstat->st_mode = S_IFCHR;
        return  0;
    }
    return  -1;
}


_ssize_t
__WEAK_FUNCTION_ATTR__
_read (int fd, char *buf, size_t cnt)
{
    return -1;
}


int
__WEAK_FUNCTION_ATTR__
_getpid (void)
{
    return -1;
}


int
__WEAK_FUNCTION_ATTR__
_kill (int sig)
{
    return -1;
}


void *
__WEAK_FUNCTION_ATTR__
_sbrk (int incr)
{
    extern char   end; /* Set by linker.  */
    static char * heap_end;
    char *        prev_heap_end;

    if (heap_end == 0) {
        heap_end = & end;
    }

    prev_heap_end = heap_end;
    heap_end += incr;

    return (void *) prev_heap_end;
}


int
__WEAK_FUNCTION_ATTR__
pthread_setcancelstate (int state, int *oldstate)
{
    return 0;
}

#endif // WITH_SEMIHOSTING
