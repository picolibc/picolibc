/* sys/mman.h

   Copyright 1996, 1997, 1998, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <sys/types.h>

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_FILE 0
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_TYPE 0xF
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_ANON MAP_ANONYMOUS
/* Non-standard flag */
#define MAP_NORESERVE 0x4000	/* Don't reserve swap space for this mapping.
				   Page protection must be set explicitely
				   to access page. Only supported for anonymous
				   private mappings. */
#define MAP_AUTOGROW 0x8000	/* Grow underlying object to mapping size.
				   File must be opened for writing. */

#define MAP_FAILED ((void *)-1)

/*
 * Flags for msync.
 */
#define MS_ASYNC 1
#define MS_SYNC 2
#define MS_INVALIDATE 4

#ifndef __INSIDE_CYGWIN__
extern void *mmap (void *__addr, size_t __len, int __prot, int __flags, int __fd, off_t __off);
#endif
extern int munmap (void *__addr, size_t __len);
extern int mprotect (void *__addr, size_t __len, int __prot);
extern int msync (void *__addr, size_t __len, int __flags);
extern int mlock (const void *__addr, size_t __len);
extern int munlock (const void *__addr, size_t __len);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /*  _SYS_MMAN_H_ */
