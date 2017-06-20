/*	$NetBSD: sysv_shm.c,v 1.23 1994/07/04 23:25:12 glass Exp $	*/
/*
 * Copyright (c) 1994 Adam Glass and Charles Hannum.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Adam Glass and Charles
 *	Hannum.
 * 4. The names of the authors may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file is heavily changed to become part of Cygwin's cygserver.
 */

#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#include <sys/cdefs.h>
#ifndef __FBSDID
#define __FBSDID(s)	const char version[] = (s)
#endif
__FBSDID("$FreeBSD: /repoman/r/ncvs/src/sys/kern/sysv_shm.c,v 1.89 2003/11/07 04:47:14 rwatson Exp $");
/* CV, 2006-01-09: Inspected upstream up to version 1.104. */

#define _KERNEL 1
#define __BSD_VISIBLE 1
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/shm.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysproto.h>

#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "cygserver.h"
#include "process.h"
#include "cygserver_ipc.h"

#ifdef __CYGWIN__
#ifndef PAGE_SIZE
#define PAGE_SIZE (getpagesize ())
#endif
#ifndef PAGE_MASK
#define PAGE_MASK (PAGE_SIZE - 1)
#endif
#define btoc(b)	(((b) + PAGE_MASK) / PAGE_SIZE)
#define round_page(p) ((((unsigned long)(p)) + PAGE_MASK) & ~(PAGE_MASK))
#ifdef __CYGWIN__
#define GIANT_REQUIRED
#else
#define GIANT_REQUIRED mtx_assert(&Giant, MA_OWNED)
#endif
#define KERN_SUCCESS 0
#define VM_PROT_READ	PROT_READ
#define VM_PROT_WRITE	PROT_WRITE
#define VM_INHERIT_SHARE 0
#define OBJT_PHYS  0
#define OBJT_SWAP  0
#define VM_PROT_DEFAULT 0
#define VM_OBJECT_LOCK(a)
#define vm_object_clear_flag(a,b)
#define vm_object_set_flag(a,b)
#define VM_OBJECT_UNLOCK(a)
#define vm_map_remove(a,b,c) KERN_SUCCESS
typedef int vm_prot_t;
#endif /* __CYGWIN__ */

#ifndef __CYGWIN__
static MALLOC_DEFINE(M_SHM, "shm", "SVID compatible shared memory segments");

struct oshmctl_args;
static int oshmctl(struct thread *td, struct oshmctl_args *uap);
#endif /* __CYGWIN__ */

static int shmget_allocate_segment(struct thread *td,
    struct shmget_args *uap, int mode);
static int shmget_existing(struct thread *td, struct shmget_args *uap,
    int mode, int segnum);

#ifndef __CYGWIN__
/* XXX casting to (sy_call_t *) is bogus, as usual. */
static sy_call_t *shmcalls[] = {
	(sy_call_t *)shmat, (sy_call_t *)oshmctl,
	(sy_call_t *)shmdt, (sy_call_t *)shmget,
	(sy_call_t *)shmctl
};
#endif /* __CYGWIN__ */

#define	SHMSEG_FREE     	0x0200
#define	SHMSEG_REMOVED  	0x0400
#define	SHMSEG_ALLOCATED	0x0800
#define	SHMSEG_WANTED		0x1000

static int shm_last_free, shm_nused, shm_committed, shmalloced, shm_nattch;
static struct shmid_ds	*shmsegs;

struct shm_handle {
	/* vm_offset_t kva; */
	vm_object_t shm_object;
};

struct shmmap_state {
	vm_offset_t va;
	int shmid;
};

static void shm_deallocate_segment(struct shmid_ds *);
static int shm_find_segment_by_key(key_t);
static struct shmid_ds *shm_find_segment_by_shmid(int);
static struct shmid_ds *shm_find_segment_by_shmidx(int);
static int shm_delete_mapping(struct vmspace *vm, struct shmmap_state *);
static void shmrealloc(void);

/*
 * Tuneable values.
 */
#ifndef SHMMAXPGS
#define	SHMMAXPGS	8192	/* Note: sysv shared memory is swap backed. */
#endif
#ifndef SHMMAX
#define	SHMMAX	(SHMMAXPGS*PAGE_SIZE)
#endif
#ifndef SHMMIN
#define	SHMMIN	1
#endif
#ifndef SHMMNI
#define	SHMMNI	192
#endif
#ifndef SHMSEG
#define	SHMSEG	128
#endif
#ifndef SHMALL
#define	SHMALL	(SHMMAXPGS)
#endif

struct	shminfo shminfo = {
	SHMMAX,
	SHMMIN,
	SHMMNI,
	SHMSEG,
	SHMALL
};

#ifndef __CYGWIN__
static int shm_use_phys;
#else
static long shm_use_phys;
static long shm_allow_removed;
#endif /* __CYGWIN__ */

#ifndef __CYGWIN__
struct shm_info shm_info;
#endif /* __CYGWIN__ */

#ifndef __CYGWIN__
SYSCTL_DECL(_kern_ipc);
SYSCTL_INT(_kern_ipc, OID_AUTO, shmmax, CTLFLAG_RW, &shminfo.shmmax, 0, "");
SYSCTL_INT(_kern_ipc, OID_AUTO, shmmin, CTLFLAG_RW, &shminfo.shmmin, 0, "");
SYSCTL_INT(_kern_ipc, OID_AUTO, shmmni, CTLFLAG_RDTUN, &shminfo.shmmni, 0, "");
SYSCTL_INT(_kern_ipc, OID_AUTO, shmseg, CTLFLAG_RDTUN, &shminfo.shmseg, 0, "");
SYSCTL_INT(_kern_ipc, OID_AUTO, shmall, CTLFLAG_RW, &shminfo.shmall, 0, "");
SYSCTL_INT(_kern_ipc, OID_AUTO, shm_use_phys, CTLFLAG_RW,
    &shm_use_phys, 0, "");
SYSCTL_INT(_kern_ipc, OID_AUTO, shm_allow_removed, CTLFLAG_RW,
    &shm_allow_removed, 0, "");
SYSCTL_PROC(_kern_ipc, OID_AUTO, shmsegs, CTLFLAG_RD,
    NULL, 0, sysctl_shmsegs, "", "");
#endif /* __CYGWIN__ */

static int
shm_find_segment_by_key(key_t key)
{
	int i;

	for (i = 0; i < shmalloced; i++)
		if ((shmsegs[i].shm_perm.mode & SHMSEG_ALLOCATED) &&
		    shmsegs[i].shm_perm.key == key)
			return (i);
	return (-1);
}

static struct shmid_ds *
shm_find_segment_by_shmid(int shmid)
{
	int segnum;
	struct shmid_ds *shmseg;

	segnum = IPCID_TO_IX(shmid);
	if (segnum < 0 || segnum >= shmalloced)
		return (NULL);
	shmseg = &shmsegs[segnum];
	if ((shmseg->shm_perm.mode & SHMSEG_ALLOCATED) == 0 ||
	    (!shm_allow_removed &&
	     (shmseg->shm_perm.mode & SHMSEG_REMOVED) != 0) ||
	    shmseg->shm_perm.seq != IPCID_TO_SEQ(shmid))
		return (NULL);
	return (shmseg);
}

static struct shmid_ds *
shm_find_segment_by_shmidx(int segnum)
{
	struct shmid_ds *shmseg;

	if (segnum < 0 || segnum >= shmalloced)
		return (NULL);
	shmseg = &shmsegs[segnum];
	if ((shmseg->shm_perm.mode & SHMSEG_ALLOCATED) == 0 ||
	    (!shm_allow_removed &&
	     (shmseg->shm_perm.mode & SHMSEG_REMOVED) != 0))
		return (NULL);
	return (shmseg);
}

static void
shm_deallocate_segment(struct shmid_ds *shmseg)
{
	struct shm_handle *shm_handle;
	size_t size;

	GIANT_REQUIRED;

	shm_handle = shmseg->shm_internal;
	vm_object_deallocate(shm_handle->shm_object);
	sys_free(shm_handle, M_SHM);
	shmseg->shm_internal = NULL;
	size = round_page(shmseg->shm_segsz);
	shm_committed -= btoc(size);
	shm_nused--;
	shmseg->shm_perm.mode = SHMSEG_FREE;
}

static int
shm_delete_mapping(struct vmspace *vm, struct shmmap_state *shmmap_s)
{
	struct shmid_ds *shmseg;
	int segnum, result;
	size_t size __attribute__ ((unused));

	GIANT_REQUIRED;

	segnum = IPCID_TO_IX(shmmap_s->shmid);
	shmseg = &shmsegs[segnum];
	size = round_page(shmseg->shm_segsz);
	result = vm_map_remove(&vm->vm_map, shmmap_s->va, shmmap_s->va + size);
	if (result != KERN_SUCCESS)
		return (EINVAL);
	shmmap_s->shmid = -1;
	shmseg->shm_dtime = time (NULL);
	--shm_nattch;
	if ((--shmseg->shm_nattch <= 0) &&
	    (shmseg->shm_perm.mode & SHMSEG_REMOVED)) {
		shm_deallocate_segment(shmseg);
		shm_last_free = segnum;
	}
	return (0);
}

#ifndef _SYS_SYSPROTO_H_
struct shmdt_args {
	const void *shmaddr;
};
#endif

/*
 * MPSAFE
 */
int
shmdt(struct thread *td, struct shmdt_args *uap)
{
	struct proc *p = td->td_proc;
	struct shmmap_state *shmmap_s;
	int i;
	int error = 0;

	if (!jail_sysvipc_allowed && jailed(td->td_ucred))
		return (ENOSYS);
	mtx_lock(&Giant);
	shmmap_s = p->p_vmspace->vm_shm;
 	if (shmmap_s == NULL) {
		error = EINVAL;
		goto done2;
	}
	for (i = 0; i < shminfo.shmseg; i++, shmmap_s++) {
		if (shmmap_s->shmid != -1 &&
		    shmmap_s->va == (vm_offset_t)uap->shmaddr) {
			break;
		}
	}
	if (i == shminfo.shmseg) {
		error = EINVAL;
		goto done2;
	}
	error = shm_delete_mapping(p->p_vmspace, shmmap_s);
done2:
	mtx_unlock(&Giant);
	return (error);
}

#ifndef _SYS_SYSPROTO_H_
struct shmat_args {
	int shmid;
	const void *shmaddr;
	int shmflg;
};
#endif

/*
 * MPSAFE
 */
int
kern_shmat(struct thread *td, int shmid, const void *shmaddr, int shmflg)
{
	struct proc *p = td->td_proc;
	int i, flags __attribute__ ((unused));
	struct shmid_ds *shmseg;
	struct shmmap_state *shmmap_s = NULL;
#ifndef __CYGWIN__
	struct shm_handle *shm_handle;
#endif
	vm_offset_t attach_va;
	vm_prot_t prot;
	vm_size_t size;
#ifndef __CYGWIN__
	int rv;
#endif
	int error = 0;

	if (!jail_sysvipc_allowed && jailed(td->td_ucred))
		return (ENOSYS);
	mtx_lock(&Giant);
	shmmap_s = p->p_vmspace->vm_shm;
	if (shmmap_s == NULL) {
		size = shminfo.shmseg * sizeof(struct shmmap_state);
		shmmap_s = (struct shmmap_state *) sys_malloc(size, M_SHM, M_WAITOK);
		for (i = 0; i < shminfo.shmseg; i++)
			shmmap_s[i].shmid = -1;
		p->p_vmspace->vm_shm = shmmap_s;
	}
	shmseg = shm_find_segment_by_shmid(shmid);
	if (shmseg == NULL) {
		error = EINVAL;
		goto done2;
	}
	error = ipcperm(td, &shmseg->shm_perm,
	    (shmflg & SHM_RDONLY) ? IPC_R : IPC_R|IPC_W);
	if (error)
		goto done2;
	for (i = 0; i < shminfo.shmseg; i++) {
		if (shmmap_s->shmid == -1)
			break;
		shmmap_s++;
	}
	if (i >= shminfo.shmseg) {
		error = EMFILE;
		goto done2;
	}
	size = round_page(shmseg->shm_segsz);
#ifdef VM_PROT_READ_IS_EXEC
	prot = VM_PROT_READ | VM_PROT_EXECUTE;
#else
	prot = VM_PROT_READ;
#endif
	if ((shmflg & SHM_RDONLY) == 0)
		prot |= VM_PROT_WRITE;
	flags = MAP_ANON | MAP_SHARED;
	debug_printf ("shmaddr: %x, shmflg: %x", shmaddr, shmflg);
#ifdef __CYGWIN__
	/* The alignment checks have already been made in the Cygwin DLL
	   and shmat's only job is to keep record of the attached mem.
	   These checks break shm on 9x since MapViewOfFileEx apparently
	   returns memory which isn't aligned to SHMLBA.  Go figure!  */
	attach_va = (vm_offset_t)shmaddr;
#else
	if (shmaddr) {
		flags |= MAP_FIXED;
		if (shmflg & SHM_RND) {
			attach_va = (vm_offset_t)shmaddr & ~(SHMLBA-1);
		} else if (((vm_offset_t)shmaddr & (SHMLBA-1)) == 0) {
			attach_va = (vm_offset_t)shmaddr;
		} else {
			error = EINVAL;
			goto done2;
		}
	} else {
		/*
		 * This is just a hint to vm_map_find() about where to
		 * put it.
		 */
		attach_va = round_page((vm_offset_t)p->p_vmspace->vm_taddr
		    + maxtsiz + maxdsiz);
	}

	shm_handle = shmseg->shm_internal;
	vm_object_reference(shm_handle->shm_object);
	rv = vm_map_find(&p->p_vmspace->vm_map, shm_handle->shm_object,
		0, &attach_va, size, (flags & MAP_FIXED)?0:1, prot, prot, 0);
	if (rv != KERN_SUCCESS) {
		error = ENOMEM;
		goto done2;
	}
	vm_map_inherit(&p->p_vmspace->vm_map,
		attach_va, attach_va + size, VM_INHERIT_SHARE);
#endif

	shmmap_s->va = attach_va;
	shmmap_s->shmid = shmid;
	shmseg->shm_lpid = p->p_pid;
	shmseg->shm_atime = time (NULL);
	shmseg->shm_nattch++;
	shm_nattch++;
	td->td_retval[0] = attach_va;
done2:
	mtx_unlock(&Giant);
	return (error);
}

int
shmat(struct thread *td, struct shmat_args *uap)
{
	return kern_shmat(td, uap->shmid, uap->shmaddr, uap->shmflg);
}

#ifndef __CYGWIN__
struct oshmid_ds {
	struct	ipc_perm shm_perm;	/* operation perms */
	int	shm_segsz;		/* size of segment (bytes) */
	u_short	shm_cpid;		/* pid, creator */
	u_short	shm_lpid;		/* pid, last operation */
	short	shm_nattch;		/* no. of current attaches */
	time_t	shm_atime;		/* last attach time */
	time_t	shm_dtime;		/* last detach time */
	time_t	shm_ctime;		/* last change time */
	void	*shm_handle;		/* internal handle for shm segment */
};

struct oshmctl_args {
	int shmid;
	int cmd;
	struct oshmid_ds *ubuf;
};

/*
 * MPSAFE
 */
static int
oshmctl(struct thread *td, struct oshmctl_args *uap)
{
#ifdef COMPAT_43
	int error = 0;
	struct shmid_ds *shmseg;
	struct oshmid_ds outbuf;

	if (!jail_sysvipc_allowed && jailed(td->td_ucred))
		return (ENOSYS);
	mtx_lock(&Giant);
	shmseg = shm_find_segment_by_shmid(uap->shmid);
	if (shmseg == NULL) {
		error = EINVAL;
		goto done2;
	}
	switch (uap->cmd) {
	case IPC_STAT:
		error = ipcperm(td, &shmseg->shm_perm, IPC_R);
		if (error)
			goto done2;
		outbuf.shm_perm = shmseg->shm_perm;
		outbuf.shm_segsz = shmseg->shm_segsz;
		outbuf.shm_cpid = shmseg->shm_cpid;
		outbuf.shm_lpid = shmseg->shm_lpid;
		outbuf.shm_nattch = shmseg->shm_nattch;
		outbuf.shm_atime = shmseg->shm_atime;
		outbuf.shm_dtime = shmseg->shm_dtime;
		outbuf.shm_ctime = shmseg->shm_ctime;
		outbuf.shm_handle = shmseg->shm_internal;
		error = copyout(&outbuf, uap->ubuf, sizeof(outbuf));
		if (error)
			goto done2;
		break;
	default:
		/* XXX casting to (sy_call_t *) is bogus, as usual. */
		error = ((sy_call_t *)shmctl)(td, uap);
		break;
	}
done2:
	mtx_unlock(&Giant);
	return (error);
#else
	return (EINVAL);
#endif
}
#endif /* !__CYGWIN__ */

#ifndef _SYS_SYSPROTO_H_
struct shmctl_args {
	int shmid;
	int cmd;
	struct shmid_ds *buf;
};
#endif

/*
 * MPSAFE
 */
int
kern_shmctl(struct thread *td, int shmid, int cmd, void *buf, size_t *bufsz)
{
	int error = 0;
	struct shmid_ds *shmseg;

	if (!jail_sysvipc_allowed && jailed(td->td_ucred))
		return (ENOSYS);

	mtx_lock(&Giant);
	switch (cmd) {
	case IPC_INFO:
		memcpy(buf, &shminfo, sizeof(shminfo));
		if (bufsz)
			*bufsz = sizeof(shminfo);
		td->td_retval[0] = shmalloced;
		goto done2;
	case SHM_INFO: {
		struct shm_info shm_info;
		shm_info.used_ids = shm_nused;
		shm_info.shm_tot = shm_committed * PAGE_SIZE;
#ifdef __CYGWIN__
		shm_info.shm_atts = shm_nattch;
#else
		shm_info.shm_rss = 0;	/*XXX where to get from ? */
		shm_info.shm_swp = 0;	/*XXX where to get from ? */
		shm_info.swap_attempts = 0;	/*XXX where to get from ? */
		shm_info.swap_successes = 0;	/*XXX where to get from ? */
#endif /* __CYGWIN__ */
		memcpy(buf, &shm_info, sizeof(shm_info));
		if (bufsz)
			*bufsz = sizeof(shm_info);
		td->td_retval[0] = shmalloced;
		goto done2;
	}
	}
	if (cmd == SHM_STAT)
		shmseg = shm_find_segment_by_shmidx(shmid);
	else
		shmseg = shm_find_segment_by_shmid(shmid);
	if (shmseg == NULL) {
		error = EINVAL;
		goto done2;
	}
	switch (cmd) {
	case SHM_STAT:
	case IPC_STAT:
		error = ipcperm(td, &shmseg->shm_perm, IPC_R);
		if (error)
			goto done2;
		memcpy(buf, shmseg, sizeof(struct shmid_ds));
		if (bufsz)
			*bufsz = sizeof(struct shmid_ds);
		if (cmd == SHM_STAT)
			td->td_retval[0] = IXSEQ_TO_IPCID(shmid, shmseg->shm_perm);
		break;
	case IPC_SET: {
		struct shmid_ds *shmid;

		shmid = (struct shmid_ds *)buf;
		error = ipcperm(td, &shmseg->shm_perm, IPC_M);
		if (error)
			goto done2;
		shmseg->shm_perm.uid = shmid->shm_perm.uid;
		shmseg->shm_perm.gid = shmid->shm_perm.gid;
		shmseg->shm_perm.mode =
		    (shmseg->shm_perm.mode & ~ACCESSPERMS) |
		    (shmid->shm_perm.mode & ACCESSPERMS);
		shmseg->shm_ctime = time (NULL);
		break;
	}
	case IPC_RMID:
		error = ipcperm(td, &shmseg->shm_perm, IPC_M);
		if (error)
			goto done2;
		shmseg->shm_perm.key = IPC_PRIVATE;
		shmseg->shm_perm.mode |= SHMSEG_REMOVED;
		if (shmseg->shm_nattch <= 0) {
			shm_deallocate_segment(shmseg);
			shm_last_free = IPCID_TO_IX(shmid);
		}
		break;
#if 0
	case SHM_LOCK:
	case SHM_UNLOCK:
#endif
	default:
		error = EINVAL;
		break;
	}
done2:
	mtx_unlock(&Giant);
	return (error);
}

int
shmctl(struct thread *td, struct shmctl_args *uap)
{
	int error = 0;
	struct shmid_ds buf;
	size_t bufsz;
	
	/* IPC_SET needs to copyin the buffer before calling kern_shmctl */
	if (uap->cmd == IPC_SET) {
		if ((error = copyin(uap->buf, &buf, sizeof(struct shmid_ds))))
			goto done;
	}
#ifdef __CYGWIN__
	if (uap->cmd == IPC_INFO && uap->shmid > 0) {
		/* Can't use the default kern_shmctl interface. */
		int shmid = uap->shmid;
		if (shmid > shminfo.shmmni)
			shmid = shminfo.shmmni;
		error = copyout(shmsegs, uap->buf,
				shmid * sizeof(struct shmid_ds));
		td->td_retval[0] = error ? -1 : 0;
		return (error);
	}
#endif /* __CYGWIN__ */
	
	error = kern_shmctl(td, uap->shmid, uap->cmd, (void *)&buf, &bufsz);
	if (error)
		goto done;
	
	/* Cases in which we need to copyout */
	switch (uap->cmd) {
	case IPC_INFO:
	case SHM_INFO:
	case SHM_STAT:
	case IPC_STAT:
		error = copyout(&buf, uap->buf, bufsz);
		break;
	}

done:
	if (error) {
		/* Invalidate the return value */
		td->td_retval[0] = -1;
	}
	return (error);
}


#ifndef _SYS_SYSPROTO_H_
struct shmget_args {
	key_t key;
	size_t size;
	int shmflg;
};
#endif

static int
shmget_existing(struct thread *td, struct shmget_args *uap, int mode, int segnum)
{
	struct shmid_ds *shmseg;
	int error;

	shmseg = &shmsegs[segnum];
	if (shmseg->shm_perm.mode & SHMSEG_REMOVED) {
		/*
		 * This segment is in the process of being allocated.  Wait
		 * until it's done, and look the key up again (in case the
		 * allocation failed or it was freed).
		 */
		shmseg->shm_perm.mode |= SHMSEG_WANTED;
		error = tsleep(shmseg, PLOCK | PCATCH, "shmget", 0);
		if (error)
			return (error);
		return (EAGAIN);
	}
	if ((uap->shmflg & (IPC_CREAT | IPC_EXCL)) == (IPC_CREAT | IPC_EXCL))
		return (EEXIST);
	error = ipcperm(td, &shmseg->shm_perm, mode);
	if (error)
		return (error);
	if (uap->size && uap->size > shmseg->shm_segsz)
		return (EINVAL);
	td->td_retval[0] = IXSEQ_TO_IPCID(segnum, shmseg->shm_perm);
#ifdef __CYGWIN__
	td->td_retval[1] =
		vm_object_duplicate(td, shmseg->shm_internal->shm_object);
#endif /* __CYGWIN__ */
	return (0);
}

static int
shmget_allocate_segment(struct thread *td, struct shmget_args *uap, int mode)
{
	int i, segnum, shmid, size;
#ifndef __CYGWIN__
	struct ucred *cred = td->td_ucred;
#endif /* __CYGWIN__ */
	struct shmid_ds *shmseg;
	struct shm_handle *shm_handle;

	GIANT_REQUIRED;

	if (uap->size < (unsigned long) shminfo.shmmin ||
	    uap->size > (unsigned long) shminfo.shmmax)
		return (EINVAL);
	if (shm_nused >= shminfo.shmmni) /* Any shmids left? */
		return (ENOSPC);
	size = round_page(uap->size);
	if (shm_committed + btoc(size) > shminfo.shmall)
		return (ENOMEM);
	if (shm_last_free < 0) {
		shmrealloc();	/* Maybe expand the shmsegs[] array. */
		for (i = 0; i < shmalloced; i++)
			if (shmsegs[i].shm_perm.mode & SHMSEG_FREE)
				break;
		if (i == shmalloced)
			return (ENOSPC);
		segnum = i;
	} else  {
		segnum = shm_last_free;
		shm_last_free = -1;
	}
	shmseg = &shmsegs[segnum];
	/*
	 * In case we sleep in malloc(), mark the segment present but deleted
	 * so that noone else tries to create the same key.
	 */
	shmseg->shm_perm.mode = SHMSEG_ALLOCATED | SHMSEG_REMOVED;
	shmseg->shm_perm.key = uap->key;
	shmseg->shm_perm.seq = (shmseg->shm_perm.seq + 1) & 0x7fff;
	shm_handle = (struct shm_handle *)
	    sys_malloc(sizeof(struct shm_handle), M_SHM, M_WAITOK);
	shmid = IXSEQ_TO_IPCID(segnum, shmseg->shm_perm);
	
	/*
	 * We make sure that we have allocated a pager before we need
	 * to.
	 */
	if (shm_use_phys) {
		shm_handle->shm_object =
		    vm_pager_allocate(OBJT_PHYS, 0, size, VM_PROT_DEFAULT, 0);
	} else {
		shm_handle->shm_object =
		    vm_pager_allocate(OBJT_SWAP, 0, size, VM_PROT_DEFAULT, 0);
	}
	VM_OBJECT_LOCK(shm_handle->shm_object);
	vm_object_clear_flag(shm_handle->shm_object, OBJ_ONEMAPPING);
	vm_object_set_flag(shm_handle->shm_object, OBJ_NOSPLIT);
	VM_OBJECT_UNLOCK(shm_handle->shm_object);

	shmseg->shm_internal = shm_handle;
#ifdef __CYGWIN__
	shmseg->shm_perm.cuid = shmseg->shm_perm.uid = td->ipcblk->uid;
	shmseg->shm_perm.cgid = shmseg->shm_perm.gid = td->ipcblk->gid;
#else
	shmseg->shm_perm.cuid = shmseg->shm_perm.uid = cred->cr_uid;
	shmseg->shm_perm.cgid = shmseg->shm_perm.gid = cred->cr_gid;
#endif /* __CYGWIN__ */
	shmseg->shm_perm.mode = (shmseg->shm_perm.mode & SHMSEG_WANTED) |
	    (mode & ACCESSPERMS) | SHMSEG_ALLOCATED;
	shmseg->shm_segsz = uap->size;
	shmseg->shm_cpid = td->td_proc->p_pid;
	shmseg->shm_lpid = shmseg->shm_nattch = 0;
	shmseg->shm_atime = shmseg->shm_dtime = 0;
	shmseg->shm_ctime = time (NULL);
	shm_committed += btoc(size);
	shm_nused++;
	if (shmseg->shm_perm.mode & SHMSEG_WANTED) {
		/*
		 * Somebody else wanted this key while we were asleep.  Wake
		 * them up now.
		 */
		shmseg->shm_perm.mode &= ~SHMSEG_WANTED;
		wakeup(shmseg);
	}
	td->td_retval[0] = shmid;
#ifdef __CYGWIN__
	td->td_retval[1] =
		vm_object_duplicate(td, shmseg->shm_internal->shm_object);
#endif /* __CYGWIN__ */
	return (0);
}

/*
 * MPSAFE
 */
int
shmget(struct thread *td, struct shmget_args *uap)
{
	int segnum, mode;
	int error;

	if (!jail_sysvipc_allowed && jailed(td->td_ucred))
		return (ENOSYS);
	mtx_lock(&Giant);
	mode = uap->shmflg & ACCESSPERMS;
	if (uap->key != IPC_PRIVATE) {
	again:
#ifdef __CYGWIN__
		if (uap->shmflg & IPC_KEY_IS_SHMID)
		  segnum = shm_find_segment_by_shmid ((int) uap->key) ?
			   IPCID_TO_IX((int) uap->key) : -1;
		else
#endif
		segnum = shm_find_segment_by_key(uap->key);
		if (segnum >= 0) {
			error = shmget_existing(td, uap, mode, segnum);
			if (error == EAGAIN)
				goto again;
			goto done2;
		}
		if ((uap->shmflg & IPC_CREAT) == 0) {
			error = ENOENT;
			goto done2;
		}
	}
	error = shmget_allocate_segment(td, uap, mode);
done2:
#ifdef __CYGWIN__
	if (!error)
		ipcexit_creat_hookthread (td);
	else
		td->td_retval[0] = -1;
#endif
	mtx_unlock(&Giant);
	return (error);
}

#ifndef __CYGWIN__
/*
 * MPSAFE
 */
int
shmsys(td, uap)
	struct thread *td;
	/* XXX actually varargs. */
	struct shmsys_args /* {
		int	which;
		int	a2;
		int	a3;
		int	a4;
	} */ *uap;
{
	int error;

	if (!jail_sysvipc_allowed && jailed(td->td_ucred))
		return (ENOSYS);
	if (uap->which < 0 ||
	    uap->which >= sizeof(shmcalls)/sizeof(shmcalls[0]))
		return (EINVAL);
	mtx_lock(&Giant);
	error = (*shmcalls[uap->which])(td, &uap->a2);
	mtx_unlock(&Giant);
	return (error);
}
#endif /* __CYGWIN__ */

static void
shmfork_myhook(struct proc *p1, struct proc *p2)
{
	struct shmmap_state *shmmap_s;
	size_t size;
	int i;

	size = shminfo.shmseg * sizeof(struct shmmap_state);
	shmmap_s = (struct shmmap_state *) sys_malloc(size, M_SHM, M_WAITOK);
	bcopy(p1->p_vmspace->vm_shm, shmmap_s, size);
	p2->p_vmspace->vm_shm = shmmap_s;
	for (i = 0; i < shminfo.shmseg; i++, shmmap_s++)
		if (shmmap_s->shmid != -1) {
			shm_nattch++;
			shmsegs[IPCID_TO_IX(shmmap_s->shmid)].shm_nattch++;
		}
}

#ifdef __CYGWIN__
int cygwin_shmfork_myhook (struct thread *td, struct proc *parent)
{
  ipcexit_creat_hookthread (td);
  ipc_p_vmspace (td->ipcblk);
  ipc_p_vmspace (parent);
  shmfork_myhook (parent, td->ipcblk);
  return 0;
}
#endif

void
shmexit_myhook(struct vmspace *vm)
{
	struct shmmap_state *base, *shm;
	int i;

	GIANT_REQUIRED;

	if ((base = vm->vm_shm) != NULL) {
		vm->vm_shm = NULL;
		for (i = 0, shm = base; i < shminfo.shmseg; i++, shm++) {
			if (shm->shmid != -1)
				shm_delete_mapping(vm, shm);
		}
		sys_free(base, M_SHM);
	}
}

static void
shmrealloc(void)
{
	int i;
	struct shmid_ds *newsegs;

	if (shmalloced >= shminfo.shmmni)
		return;

	newsegs = (struct shmid_ds *) sys_malloc(shminfo.shmmni * sizeof(*newsegs), M_SHM, M_WAITOK);
	if (newsegs == NULL)
		return;
	for (i = 0; i < shmalloced; i++)
		bcopy(&shmsegs[i], &newsegs[i], sizeof(newsegs[0]));
	for (; i < shminfo.shmmni; i++) {
		shmsegs[i].shm_perm.mode = SHMSEG_FREE;
		shmsegs[i].shm_perm.seq = 0;
	}
	sys_free(shmsegs, M_SHM);
	shmsegs = newsegs;
	shmalloced = shminfo.shmmni;
}

void
shminit(void)
{
	int i;
	tun_bool_t shm_ar;

	TUNABLE_INT_FETCH("kern.ipc.shmmaxpgs", &shminfo.shmall);
	for (i = PAGE_SIZE; i > 0; i--) {
		shminfo.shmmax = shminfo.shmall * i;
		if (shminfo.shmmax >= shminfo.shmall)
			break;
	}
	TUNABLE_INT_FETCH("kern.ipc.shmmin", &shminfo.shmmin);
	TUNABLE_INT_FETCH("kern.ipc.shmmni", &shminfo.shmmni);
	TUNABLE_INT_FETCH("kern.ipc.shmseg", &shminfo.shmseg);
	TUNABLE_BOOL_FETCH("kern.ipc.shm_allow_removed", &shm_ar);
	if (shm_ar == TUN_TRUE)
	  shm_allow_removed = 1;
	shmalloced = shminfo.shmmni;
	shmsegs = (struct shmid_ds *) sys_malloc(shmalloced * sizeof(shmsegs[0]), M_SHM, M_WAITOK);
	if (shmsegs == NULL)
		panic("cannot allocate initial memory for sysvshm");
	for (i = 0; i < shmalloced; i++) {
		shmsegs[i].shm_perm.mode = SHMSEG_FREE;
		shmsegs[i].shm_perm.seq = 0;
	}
	shm_last_free = 0;
	shm_nused = 0;
	shm_committed = 0;
#ifndef __CYGWIN__
	shmexit_hook = &shmexit_myhook;
	shmfork_hook = &shmfork_myhook;
#endif /* __CYGWIN__ */
}

int
shmunload(void)
{

	if (shm_nused > 0)
		return (EBUSY);

	sys_free(shmsegs, M_SHM);
#ifndef __CYGWIN__
	shmexit_hook = NULL;
	shmfork_hook = NULL;
#endif /* __CYGWIN__ */
	return (0);
}

#ifndef __CYGWIN__
static int
sysctl_shmsegs(SYSCTL_HANDLER_ARGS)
{

	return (SYSCTL_OUT(req, shmsegs, shmalloced * sizeof(shmsegs[0])));
}

static int
sysvshm_modload(struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch (cmd) {
	case MOD_LOAD:
		shminit();
		break;
	case MOD_UNLOAD:
		error = shmunload();
		break;
	case MOD_SHUTDOWN:
		break;
	default:
		error = EINVAL;
		break;
	}
	return (error);
}

static moduledata_t sysvshm_mod = {
	"sysvshm",
	&sysvshm_modload,
	NULL
};

SYSCALL_MODULE_HELPER(shmsys);
SYSCALL_MODULE_HELPER(shmat);
SYSCALL_MODULE_HELPER(shmctl);
SYSCALL_MODULE_HELPER(shmdt);
SYSCALL_MODULE_HELPER(shmget);

DECLARE_MODULE(sysvshm, sysvshm_mod,
	SI_SUB_SYSV_SHM, SI_ORDER_FIRST);
MODULE_VERSION(sysvshm, 1);
#endif /* __CYGWIN__ */
#endif /* __OUTSIDE_CYGWIN__ */
