/* bsd_mutex.h: BSD Mutex helper

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
#ifndef _BSD_MUTEX_H
#define _BSD_MUTEX_H

#define MTX_DEF 0

#define MA_OWNED 1
#define MA_NOTOWNED 2

#define PZERO	  (0x20)
#define PRIO_MASK (0x1f)
#define PDROP  0x1000
#define PCATCH 0x2000
#define PLOCK  0x3000

struct mtx {
  HANDLE h;
  const char *name;
  DWORD owner;
  unsigned long cnt;
};

/* Some BSD kernel global mutex. */
extern struct mtx Giant;

void mtx_init (mtx *, const char *, const void *, int);
void _mtx_lock (mtx *, DWORD winpid, const char *, int);
#define mtx_lock(m) _mtx_lock((m), (td->ipcblk->winpid), __FILE__, __LINE__)
int mtx_owned (mtx *, DWORD);
void _mtx_assert(mtx *, int, DWORD winpid, const char *, int);
#define mtx_assert(m,w,p) _mtx_assert((m),(w),(p),__FILE__,__LINE__)
void _mtx_unlock (mtx *, const char *, int);
#define mtx_unlock(m) _mtx_unlock((m),__FILE__,__LINE__)

void mtx_destroy (mtx *);

void msleep_init (void);
int _msleep (void *, struct mtx *, int, const char *, int, struct thread *);
#define msleep(i,m,p,w,t) _msleep((i),(m),(p),(w),(t),(td))
#define tsleep(i,p,w,t)   _msleep((i),NULL,(p),(w),(t),(td))
int wakeup (void *);
void wakeup_all (void);

#endif /* _BSD_MUTEX_H */
