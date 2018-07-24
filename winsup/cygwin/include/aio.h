/* aio.h: Support for Posix asynchronous i/o routines.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _AIO_H
#define _AIO_H

#include <sys/features.h>
#include <sys/queue.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <limits.h> // for AIO_LISTIO_MAX, AIO_MAX, and AIO_PRIO_DELTA_MAX

/* defines for return value of aio_cancel() */
#define AIO_ALLDONE     0
#define AIO_CANCELED    1
#define AIO_NOTCANCELED 2

/* defines for 'mode' argument of lio_listio() */
#define LIO_NOWAIT      0
#define LIO_WAIT        1

/* defines for 'aio_lio_opcode' element of struct aiocb */
#define LIO_NOP         0
#define LIO_READ        1
#define LIO_WRITE       2

#ifdef __cplusplus
extern "C" {
#endif

/* struct __liocb is Cygwin-specific */
struct __liocb {
    volatile uint32_t    lio_count;
    struct sigevent     *lio_sigevent;
};

/* struct __wincb is Cygwin-specific */
struct __wincb {
    int32_t              status; /* These two fields must be first... */
    uintptr_t            info;   /* ...and must be adjacent, in this order */
    void                *event;
};

/* struct aiocb is defined by Posix */
struct aiocb {
    /* these elements of aiocb are Cygwin-specific */
    TAILQ_ENTRY(aiocb)   aio_chain;
    struct __liocb      *aio_liocb;
    struct __wincb       aio_wincb;
    ssize_t              aio_rbytes;
    int                  aio_errno;
    /* the remaining elements of aiocb are defined by Posix */
    int                  aio_lio_opcode;
    int                  aio_reqprio; /* Not yet implemented; must be zero */
    int                  aio_fildes;
    volatile void       *aio_buf;
    size_t               aio_nbytes;
    off_t                aio_offset;
    struct sigevent      aio_sigevent;
};

/* function prototypes as defined by Posix */
int     aio_cancel  (int, struct aiocb *);
int     aio_error   (const struct aiocb *);
int     aio_fsync   (int, struct aiocb *);
int     aio_read    (struct aiocb *);
ssize_t aio_return  (struct aiocb *);
int     aio_suspend (const struct aiocb *const [], int,
                        const struct timespec *);
int     aio_write   (struct aiocb *);
int     lio_listio  (int, struct aiocb *__restrict const [__restrict], int,
                        struct sigevent *__restrict);

#ifdef __cplusplus
}
#endif
#endif /* _AIO_H */
