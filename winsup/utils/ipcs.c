/*
 * Copyright (c) 1994 SigmaSoft, Th. Lockert <tholo@sigmasoft.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef __FBSDID
#define __FBSDID(s)	const char version[] = (s)
#endif
__FBSDID("$FreeBSD: /repoman/r/ncvs/src/usr.bin/ipcs/ipcs.c,v 1.22 2003/10/30 16:52:14 iwasaki Exp $");

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <grp.h>
#ifndef __CYGWIN__
#include <kvm.h>
#include <nlist.h>
#endif /* __CYGWIN__ */
#include <limits.h>
#include <paths.h>
#include <pwd.h>
#ifdef __CYGWIN__
#include <grp.h>
#endif /* __CYGWIN__ */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#ifndef __CYGWIN__
#include <sys/proc.h>
#include <sys/sysctl.h>
#endif /* __CYGWIN__ */
#define _KERNEL
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>

/* SysCtlGatherStruct structure. */
struct scgs_vector {
	const char *sysctl;
	off_t offset;
	size_t size;
};

int	use_sysctl = 1;
struct semid_ds	*sema;
struct seminfo	seminfo;
struct msginfo	msginfo;
struct msqid_ds	*msqids;
struct shminfo	shminfo;
struct shmid_ds	*shmsegs;

char   *fmt_perm(u_short);
void	cvt_time(time_t, char *);
void	sysctlgatherstruct(void *addr, size_t size, struct scgs_vector *vec);
void	kget(int idx, void *addr, size_t size);
void	usage(void);
#ifdef __CYGWIN__
void help() __attribute__ ((noreturn));
#endif /* __CYGWIN__ */

#ifdef __CYGWIN__
#define _POSIX2_LINE_MAX 255
typedef int kvm_t;
#define kvm_openfiles(a,b,c,d,e) NULL
#define kvm_nlist(a,b) -1
#define kvm_close(a)
#define kvm_read(a,b,c,d) 0
#define kvm_geterr(a) ""
#define sysctlbyname(a,b,c,d,e) -1

struct nlist {
  const char *n_name;
  int n_type;
  int n_value;
};
#endif /* __CYGWIN__ */

static struct nlist symbols[] = {
	{"sema"},
#define X_SEMA		0
	{"seminfo"},
#define X_SEMINFO	1
	{"msginfo"},
#define X_MSGINFO	2
	{"msqids"},
#define X_MSQIDS	3
	{"shminfo"},
#define X_SHMINFO	4
	{"shmsegs"},
#define X_SHMSEGS	5
#ifdef __CYGWIN__
	{"msgstat"},
#define X_MSGSTAT	6
	{"semstat"},
#define X_SEMSTAT	7
	{"shmstat"},
#define X_SHMSTAT	8
#endif /* __CYGWIN__ */
	{NULL}
};

#define	SHMINFO_XVEC				\
X(shmmax, sizeof(int))				\
X(shmmin, sizeof(int))				\
X(shmmni, sizeof(int))				\
X(shmseg, sizeof(int))				\
X(shmall, sizeof(int))

#define	SEMINFO_XVEC				\
X(semmap, sizeof(int))				\
X(semmni, sizeof(int))				\
X(semmns, sizeof(int))				\
X(semmnu, sizeof(int))				\
X(semmsl, sizeof(int))				\
X(semopm, sizeof(int))				\
X(semume, sizeof(int))				\
X(semusz, sizeof(int))				\
X(semvmx, sizeof(int))				\
X(semaem, sizeof(int))

#define	MSGINFO_XVEC				\
X(msgmax, sizeof(int))				\
X(msgmni, sizeof(int))				\
X(msgmnb, sizeof(int))				\
X(msgtql, sizeof(int))				\
X(msgssz, sizeof(int))				\
X(msgseg, sizeof(int))

#define	X(a, b)	{ "kern.ipc." #a, offsetof(TYPEC, a), (b) },
#define	TYPEC	struct shminfo
struct scgs_vector shminfo_scgsv[] = { SHMINFO_XVEC { NULL } };
#undef	TYPEC
#define	TYPEC	struct seminfo
struct scgs_vector seminfo_scgsv[] = { SEMINFO_XVEC { NULL } };
#undef	TYPEC
#define	TYPEC	struct msginfo
struct scgs_vector msginfo_scgsv[] = { MSGINFO_XVEC { NULL } };
#undef	TYPEC
#undef	X

static kvm_t *kd;

#ifdef __CYGWIN__
const char *
user_from_uid (uid_t uid, uid_t dummy)
{
  struct passwd *pwd = getpwuid (uid);
  return pwd ? pwd->pw_name : "???";
}

const char *
group_from_gid (gid_t gid, gid_t dummy)
{
  struct group *grp = getgrgid (gid);
  return grp ? grp->gr_name : "???";
}
#endif /* __CYGWIN__ */

char   *
fmt_perm(mode)
	u_short mode;
{
	static char buffer[100];

	buffer[0] = '-';
	buffer[1] = '-';
	buffer[2] = ((mode & 0400) ? 'r' : '-');
	buffer[3] = ((mode & 0200) ? 'w' : '-');
	buffer[4] = ((mode & 0100) ? 'a' : '-');
	buffer[5] = ((mode & 0040) ? 'r' : '-');
	buffer[6] = ((mode & 0020) ? 'w' : '-');
	buffer[7] = ((mode & 0010) ? 'a' : '-');
	buffer[8] = ((mode & 0004) ? 'r' : '-');
	buffer[9] = ((mode & 0002) ? 'w' : '-');
	buffer[10] = ((mode & 0001) ? 'a' : '-');
	buffer[11] = '\0';
	return (&buffer[0]);
}

void
cvt_time(t, buf)
	time_t  t;
	char   *buf;
{
	struct tm *tm;

	if (t == 0) {
		strcpy(buf, "no-entry");
	} else {
		tm = localtime(&t);
		sprintf(buf, "%2d:%02d:%02d",
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
}
#define	SHMINFO		1
#define	SHMTOTAL	2
#define	MSGINFO		4
#define	MSGTOTAL	8
#define	SEMINFO		16
#define	SEMTOTAL	32

#define BIGGEST		1
#define CREATOR		2
#define OUTSTANDING	4
#define PID		8
#define TIME		16
#ifdef __CYGWIN__
#define STATUS		32
#endif /* __CYGWIN__ */

int
main(argc, argv)
	int     argc;
	char   *argv[];
{
	int     display = SHMINFO | MSGINFO | SEMINFO;
	int     option = 0;
	char   *core = NULL, *namelist = NULL;
	char	kvmoferr[_POSIX2_LINE_MAX];  /* Error buf for kvm_openfiles. */
	int     i;

#ifdef __CYGWIN__
	while ((i = getopt(argc, argv, "MmQqSsabchoptTu")) != -1)
#else
	while ((i = getopt(argc, argv, "MmQqSsabC:cN:optTy")) != -1)
#endif /* __CYGWIN__ */
		switch (i) {
		case 'M':
			display = SHMTOTAL;
			break;
		case 'm':
			display = SHMINFO;
			break;
		case 'Q':
			display = MSGTOTAL;
			break;
		case 'q':
			display = MSGINFO;
			break;
		case 'S':
			display = SEMTOTAL;
			break;
		case 's':
			display = SEMINFO;
			break;
		case 'T':
			display = SHMTOTAL | MSGTOTAL | SEMTOTAL;
			break;
		case 'a':
			option |= BIGGEST | CREATOR | OUTSTANDING | PID | TIME;
			break;
		case 'b':
			option |= BIGGEST;
			break;
#ifndef __CYGWIN__
		case 'C':
			core = optarg;
			break;
#endif /* __CYGWIN__ */
		case 'c':
			option |= CREATOR;
			break;
#ifndef __CYGWIN__
		case 'N':
			namelist = optarg;
			break;
#endif /* __CYGWIN__ */
		case 'o':
			option |= OUTSTANDING;
			break;
		case 'p':
			option |= PID;
			break;
		case 't':
			option |= TIME;
			break;
#ifdef __CYGWIN__
		case 'h':
			help();
		case 'u':
			option |= STATUS;
			break;
#endif /* __CYGWIN__ */
#ifndef __CYGWIN__
		case 'y':
			use_sysctl = 0;
			break;
#endif /* __CYGWIN__ */
		default:
			usage();
		}

	/*
	 * If paths to the exec file or core file were specified, we
	 * aren't operating on the running kernel, so we can't use
	 * sysctl.
	 */
	if (namelist != NULL || core != NULL)
		use_sysctl = 0;

	if (!use_sysctl) {
		kd = kvm_openfiles(namelist, core, NULL, O_RDONLY, kvmoferr);
		if (kd == NULL)
			errx(1, "kvm_openfiles: %s", kvmoferr);
		switch (kvm_nlist(kd, symbols)) {
		case 0:
			break;
		case -1:
			errx(1, "unable to read kernel symbol table");
		default:
#ifdef notdef		/* they'll be told more civilly later */
			warnx("nlist failed");
			for (i = 0; symbols[i].n_name != NULL; i++)
				if (symbols[i].n_value == 0)
					warnx("symbol %s not found",
					    symbols[i].n_name);
#endif
			break;
		}
	}

	kget(X_MSGINFO, &msginfo, sizeof(msginfo));
	if ((display & (MSGINFO | MSGTOTAL))) {
		if (display & MSGTOTAL) {
			printf("msginfo:\n");
			printf("\tmsgmax: %8ld\t(max characters in a message)\n",
			    msginfo.msgmax);
			printf("\tmsgmni: %8ld\t(# of message queues)\n",
			    msginfo.msgmni);
			printf("\tmsgmnb: %8ld\t(max characters in a message queue)\n",
			    msginfo.msgmnb);
			printf("\tmsgtql: %8ld\t(max # of messages in system)\n",
			    msginfo.msgtql);
			printf("\tmsgssz: %8ld\t(size of a message segment)\n",
			    msginfo.msgssz);
			printf("\tmsgseg: %8ld\t(# of message segments in system)\n\n",
			    msginfo.msgseg);
		}
		if (display & MSGINFO) {
			struct msqid_ds *xmsqids;
			size_t xmsqids_len;


			xmsqids_len = sizeof(struct msqid_ds) * msginfo.msgmni;
			xmsqids = malloc(xmsqids_len);
			kget(X_MSQIDS, xmsqids, xmsqids_len);

			printf("Message Queues:\n");
			printf("T     ID               KEY        MODE       OWNER    GROUP");
			if (option & CREATOR)
				printf("  CREATOR   CGROUP");
			if (option & OUTSTANDING)
				printf(" CBYTES   QNUM");
			if (option & BIGGEST)
				printf(" QBYTES");
			if (option & PID)
				printf("  LSPID  LRPID");
			if (option & TIME)
				printf("    STIME    RTIME    CTIME");
			printf("\n");
			for (i = 0; i < msginfo.msgmni; i += 1) {
				if (xmsqids[i].msg_qbytes != 0) {
					char    stime_buf[100], rtime_buf[100],
					        ctime_buf[100];
					struct msqid_ds *msqptr = &xmsqids[i];

					cvt_time(msqptr->msg_stime, stime_buf);
					cvt_time(msqptr->msg_rtime, rtime_buf);
					cvt_time(msqptr->msg_ctime, ctime_buf);

					printf("q %6d %20llu %s %8s %8s",
					    IXSEQ_TO_IPCID(i, msqptr->msg_perm),
					    msqptr->msg_perm.key,
					    fmt_perm(msqptr->msg_perm.mode),
					    user_from_uid(msqptr->msg_perm.uid, 0),
					    group_from_gid(msqptr->msg_perm.gid, 0));

					if (option & CREATOR)
						printf(" %8s %8s",
						    user_from_uid(msqptr->msg_perm.cuid, 0),
						    group_from_gid(msqptr->msg_perm.cgid, 0));

					if (option & OUTSTANDING)
						printf(" %6lu %6lu",
						    msqptr->msg_cbytes,
						    msqptr->msg_qnum);

					if (option & BIGGEST)
						printf(" %6lu",
						    msqptr->msg_qbytes);

					if (option & PID)
						printf(" %6d %6d",
						    msqptr->msg_lspid,
						    msqptr->msg_lrpid);

					if (option & TIME)
						printf(" %s %s %s",
						    stime_buf,
						    rtime_buf,
						    ctime_buf);

					printf("\n");
				}
			}
			printf("\n");
		}
#ifdef __CYGWIN__
		if (option & STATUS) {
			struct msg_info msg_info;
			kget(X_MSGSTAT, &msg_info, sizeof msg_info);
			printf("Message Queue Usage:\n");
			printf("\tmsg_ids: %7ld\t(no. of allocated queues)\n",
			    msg_info.msg_ids);
			printf("\tmsg_num: %7ld\t(no. of messages in system)\n",
			    msg_info.msg_num);
			printf("\tmsg_tot: %7ld\t(size in bytes of messages in system)\n",
			    msg_info.msg_tot);
			printf("\n");
		}
#endif /* __CYGWIN__ */
	} else
		if (display & (MSGINFO | MSGTOTAL)) {
			fprintf(stderr,
			    "SVID messages facility not configured in the system\n");
		}

	kget(X_SHMINFO, &shminfo, sizeof(shminfo));
	if ((display & (SHMINFO | SHMTOTAL))) {
		if (display & SHMTOTAL) {
			printf("shminfo:\n");
			printf("\tshmmax: %8ld\t(max shared memory segment size)\n",
			    shminfo.shmmax);
			printf("\tshmmin: %8ld\t(min shared memory segment size)\n",
			    shminfo.shmmin);
			printf("\tshmmni: %8ld\t(max number of shared memory identifiers)\n",
			    shminfo.shmmni);
			printf("\tshmseg: %8ld\t(max shared memory segments per process)\n",
			    shminfo.shmseg);
			printf("\tshmall: %8ld\t(max amount of shared memory in pages)\n\n",
			    shminfo.shmall);
		}
		if (display & SHMINFO) {
			struct shmid_ds *xshmids;
			size_t xshmids_len;

			xshmids_len = sizeof(struct shmid_ds) * shminfo.shmmni;
			xshmids = malloc(xshmids_len);
			kget(X_SHMSEGS, xshmids, xshmids_len);

			printf("Shared Memory:\n");
			printf("T     ID               KEY        MODE       OWNER    GROUP");
			if (option & CREATOR)
				printf("  CREATOR   CGROUP");
			if (option & OUTSTANDING)
				printf(" NATTCH");
			if (option & BIGGEST)
				printf("  SEGSZ");
			if (option & PID)
				printf("  CPID  LPID");
			if (option & TIME)
				printf("    ATIME    DTIME    CTIME");
			printf("\n");
			for (i = 0; i < shminfo.shmmni; i += 1) {
				if (xshmids[i].shm_perm.mode & 0x0800) {
					char    atime_buf[100], dtime_buf[100],
					        ctime_buf[100];
					struct shmid_ds *shmptr = &xshmids[i];

					cvt_time(shmptr->shm_atime, atime_buf);
					cvt_time(shmptr->shm_dtime, dtime_buf);
					cvt_time(shmptr->shm_ctime, ctime_buf);

					printf("m %6d %20llu %s %8s %8s",
					    IXSEQ_TO_IPCID(i, shmptr->shm_perm),
					    shmptr->shm_perm.key,
					    fmt_perm(shmptr->shm_perm.mode),
					    user_from_uid(shmptr->shm_perm.uid, 0),
					    group_from_gid(shmptr->shm_perm.gid, 0));

					if (option & CREATOR)
						printf(" %8s %8s",
						    user_from_uid(shmptr->shm_perm.cuid, 0),
						    group_from_gid(shmptr->shm_perm.cgid, 0));

					if (option & OUTSTANDING)
						printf(" %6d",
						    shmptr->shm_nattch);

					if (option & BIGGEST)
						printf(" %6d",
						    shmptr->shm_segsz);

					if (option & PID)
						printf(" %6d %6d",
						    shmptr->shm_cpid,
						    shmptr->shm_lpid);

					if (option & TIME)
						printf(" %s %s %s",
						    atime_buf,
						    dtime_buf,
						    ctime_buf);

					printf("\n");
				}
			}
			printf("\n");
		}
#ifdef __CYGWIN__
		if (option & STATUS) {
			struct shm_info shm_info;
			kget(X_SHMSTAT, &shm_info, sizeof shm_info);
			printf("Shared Memory Usage:\n");
			printf("\tshm_ids:  %7ld\t(no. of allocated segments)\n",
			    shm_info.shm_ids);
			printf("\tshm_tot:  %7ld\t(size in bytes of allocated segments)\n",
			    shm_info.shm_tot);
			printf("\tshm_atts: %7ld\t(no. of attached segments in system)\n",
			    shm_info.shm_atts);
			printf("\n");
		}
#endif /* __CYGWIN__ */
	} else
		if (display & (SHMINFO | SHMTOTAL)) {
			fprintf(stderr,
			    "SVID shared memory facility not configured in the system\n");
		}

	kget(X_SEMINFO, &seminfo, sizeof(seminfo));
	if ((display & (SEMINFO | SEMTOTAL))) {
		struct semid_ds *xsema;
		size_t xsema_len;

		if (display & SEMTOTAL) {
			printf("seminfo:\n");
			printf("\tsemmap: %8ld\t(# of entries in semaphore map)\n",
			    seminfo.semmap);
			printf("\tsemmni: %8ld\t(# of semaphore identifiers)\n",
			    seminfo.semmni);
			printf("\tsemmns: %8ld\t(# of semaphores in system)\n",
			    seminfo.semmns);
			printf("\tsemmnu: %8ld\t(# of undo structures in system)\n",
			    seminfo.semmnu);
			printf("\tsemmsl: %8ld\t(max # of semaphores per id)\n",
			    seminfo.semmsl);
			printf("\tsemopm: %8ld\t(max # of operations per semop call)\n",
			    seminfo.semopm);
			printf("\tsemume: %8ld\t(max # of undo entries per process)\n",
			    seminfo.semume);
			printf("\tsemusz: %8ld\t(size in bytes of undo structure)\n",
			    seminfo.semusz);
			printf("\tsemvmx: %8ld\t(semaphore maximum value)\n",
			    seminfo.semvmx);
			printf("\tsemaem: %8ld\t(adjust on exit max value)\n\n",
			    seminfo.semaem);
		}
		if (display & SEMINFO) {
			xsema_len = sizeof(struct semid_ds) * seminfo.semmni;
			xsema = malloc(xsema_len);
			kget(X_SEMA, xsema, xsema_len);

			printf("Semaphores:\n");
			printf("T     ID               KEY        MODE       OWNER    GROUP");
			if (option & CREATOR)
				printf("  CREATOR   CGROUP");
			if (option & BIGGEST)
				printf(" NSEMS");
			if (option & TIME)
				printf("    OTIME    CTIME");
			printf("\n");
			for (i = 0; i < seminfo.semmni; i += 1) {
				if ((xsema[i].sem_perm.mode & SEM_ALLOC) != 0) {
					char    ctime_buf[100], otime_buf[100];
					struct semid_ds *semaptr = &xsema[i];

					cvt_time(semaptr->sem_otime, otime_buf);
					cvt_time(semaptr->sem_ctime, ctime_buf);

					printf("s %6d %20llu %s %8s %8s",
					    IXSEQ_TO_IPCID(i, semaptr->sem_perm),
					    semaptr->sem_perm.key,
					    fmt_perm(semaptr->sem_perm.mode),
					    user_from_uid(semaptr->sem_perm.uid, 0),
					    group_from_gid(semaptr->sem_perm.gid, 0));

					if (option & CREATOR)
						printf(" %8s %8s",
						    user_from_uid(semaptr->sem_perm.cuid, 0),
						    group_from_gid(semaptr->sem_perm.cgid, 0));

					if (option & BIGGEST)
						printf(" %6d",
						    semaptr->sem_nsems);

					if (option & TIME)
						printf(" %s %s",
						    otime_buf,
						    ctime_buf);

					printf("\n");
				}
			}

			printf("\n");
		}
#ifdef __CYGWIN__
		if (option & STATUS) {
			struct sem_info sem_info;
			kget(X_SEMSTAT, &sem_info, sizeof sem_info);
			printf("Semaphore Usage:\n");
			printf("\tsem_ids: %7ld\t(no. of allocated semaphore sets)\n",
			    sem_info.sem_ids);
			printf("\tsem_num: %7ld\t(no. of allocated semaphores)\n",
			    sem_info.sem_num);
			printf("\n");
		}
#endif /* __CYGWIN__ */
	} else
		if (display & (SEMINFO | SEMTOTAL)) {
			fprintf(stderr, "SVID semaphores facility not configured in the system\n");
		}
	if (!use_sysctl)
		kvm_close(kd);

	exit(0);
}

void
sysctlgatherstruct(addr, size, vecarr)
	void *addr;
	size_t size;
	struct scgs_vector *vecarr;
{
	struct scgs_vector *xp;
	size_t tsiz;
	int rv;

	for (xp = vecarr; xp->sysctl != NULL; xp++) {
		assert(xp->offset <= size);
		tsiz = xp->size;
		rv = sysctlbyname(xp->sysctl, (char *)addr + xp->offset,
		    &tsiz, NULL, 0);
		if (rv == -1)
			errx(1, "sysctlbyname: %s", xp->sysctl);
		if (tsiz != xp->size)
			errx(1, "%s size mismatch (expected %d, got %d)",
			    xp->sysctl, xp->size, tsiz);
	}
}

void
kget(idx, addr, size)
	int idx;
	void *addr;
	size_t size;
{
	const char *symn;			/* symbol name */
	size_t tsiz;
	int rv;
	unsigned long kaddr;
	const char *sym2sysctl[] = {	/* symbol to sysctl name table */
		"kern.ipc.sema",
		"kern.ipc.seminfo",
		"kern.ipc.msginfo",
		"kern.ipc.msqids",
		"kern.ipc.shminfo",
		"kern.ipc.shmsegs" };

#ifndef __CYGWIN__
	assert((unsigned)idx <= sizeof(sym2sysctl) / sizeof(*sym2sysctl));
#endif /* __CYGWIN__ */
#ifdef __CYGWIN__
        if (1) {
		union semun su;
		switch (idx) {
		case X_MSGINFO:
			if (msgctl(0, IPC_INFO, (struct msqid_ds *)addr) < 0)
			  err(1, "msgctl");
			break;
		case X_SEMINFO:
			su.buf = (struct semid_ds *)addr;
			if (semctl(0, 0, IPC_INFO, su) < 0)
			  err(1, "semctl");
			break;
			break;
		case X_SHMINFO:
			if (shmctl(0, IPC_INFO, (struct shmid_ds *)addr) < 0)
			  err(1, "shmctl");
			break;
		case X_MSQIDS:
			if (msgctl(size / sizeof(struct msqid_ds), IPC_INFO,
				   (struct msqid_ds *)addr) < 0)
			  err(1, "msgctl");
			break;
		case X_SEMA:
			su.buf = (struct semid_ds *)addr;
			if (semctl(size / sizeof(struct semid_ds), 0,
				   IPC_INFO, su) < 0)
			  err(1, "semctl");
			break;
		case X_SHMSEGS:
			if (shmctl(size / sizeof(struct shmid_ds), IPC_INFO,
				   (struct shmid_ds *)addr) < 0)
			  err(1, "shmctl");
			break;
		case X_MSGSTAT:
			if (msgctl(0, MSG_INFO, (struct msqid_ds *)addr) < 0)
			  err(1, "msgctl");
			break;
		case X_SEMSTAT:
			su.buf = (struct semid_ds *)addr;
			if (semctl(0, 0, SEM_INFO, su) < 0)
			  err(1, "semctl");
			break;
			break;
		case X_SHMSTAT:
			if (shmctl(0, SHM_INFO, (struct shmid_ds *)addr) < 0)
			  err(1, "shmctl");
			break;
		}
	} else
#endif /* __CYGWIN__ */
	if (!use_sysctl) {
		symn = symbols[idx].n_name;
		if (*symn == '_')
			symn++;
		if (symbols[idx].n_type == 0 || symbols[idx].n_value == 0)
			errx(1, "symbol %s undefined", symn);
		/*
		 * For some symbols, the value we retreieve is
		 * actually a pointer; since we want the actual value,
		 * we have to manually dereference it.
		 */
		switch (idx) {
		case X_MSQIDS:
			tsiz = sizeof(msqids);
			rv = kvm_read(kd, symbols[idx].n_value,
			    &msqids, tsiz);
			kaddr = (u_long)msqids;
			break;
		case X_SHMSEGS:
			tsiz = sizeof(shmsegs);
			rv = kvm_read(kd, symbols[idx].n_value,
			    &shmsegs, tsiz);
			kaddr = (u_long)shmsegs;
			break;
		case X_SEMA:
			tsiz = sizeof(sema);
			rv = kvm_read(kd, symbols[idx].n_value,
			    &sema, tsiz);
			kaddr = (u_long)sema;
			break;
		default:
			rv = tsiz = 0;
			kaddr = symbols[idx].n_value;
			break;
		}
		if ((unsigned)rv != tsiz)
			errx(1, "%s: %s", symn, kvm_geterr(kd));
		if ((unsigned)kvm_read(kd, kaddr, addr, size) != size)
			errx(1, "%s: %s", symn, kvm_geterr(kd));
	} else {
		switch (idx) {
		case X_SHMINFO:
			sysctlgatherstruct(addr, size, shminfo_scgsv);
			break;
		case X_SEMINFO:
			sysctlgatherstruct(addr, size, seminfo_scgsv);
			break;
		case X_MSGINFO:
			sysctlgatherstruct(addr, size, msginfo_scgsv);
			break;
		default:
			tsiz = size;
			rv = sysctlbyname(sym2sysctl[idx], addr, &tsiz,
			    NULL, 0);
			if (rv == -1)
				err(1, "sysctlbyname: %s", sym2sysctl[idx]);
			if (tsiz != size)
				errx(1, "%s size mismatch "
				    "(expected %d, got %d)",
				    sym2sysctl[idx], size, tsiz);
			break;
		}
	}
}

void
usage()
{

	fprintf(stderr,
#ifdef __CYGWIN__
	    "usage: ipcs [-abchmopqstuMQST]\n"
	    "Try `ipcs -h' for more information.\n");
#else
	    "usage: ipcs [-abcmopqstyMQST] [-C corefile] [-N namelist]\n");
#endif /* __CYGWIN__ */
	exit(1);
}

#ifdef __CYGWIN__
void
help()
{	puts(
"usage: ipcs [-abchmopqstuMQST]\n"
"ipcs prints information for IPC resources for which you have read access.\n"
"  -a   Show the maximum amount of information possible when displaying\n"
"       active semaphores, message queues and shared memory segments\n"
"       (This is shorthand for specifying -bcopt).\n"
"  -b   Show the maximum allowed sizes for active semaphores, message queues\n"
"       and shared memory segments.  The maximum allowed size is the maximum\n"
"       number of bytes in a message on a message queue, the size of a shared\n"
"       memory segment, or the number of semaphores in a set of semaphores.\n"
"  -c   Show the creator's name and group for active semaphores, message\n"
"       queues, and shared memory segments.\n"
"  -h   This help.\n"
"  -m   Display information about active shared memory segments.\n"
"  -o   Show outstanding usage for active message queues and shared memory\n"
"       segments.  The outstanding usage is the number of messages in a\n"
"       message queue, or the number of processes attached to a shared memory\n"
"       segment.\n"
"  -p   Show the process ID information for active semaphores, message queues\n"
"       and shared memory segments.  The process ID information is the last\n"
"       process to send a message to or receive a message from a message\n"
"       queue, the process that created a semaphore, or the last process to\n"
"       attach or detach a shared memory segment.\n"
"  -q   Display information about active message queues.\n"
"  -s   Display information about active semaphores.\n"
"  -t   Show access times for active semaphores, message queues and shared\n"
"  -u   Show system wide usage of IPC resources.  This information is added\n"
"       as a list after the appropriate shared memory, messages queue or\n"
"       semaphore information output\n"
"  -M   Display system information about shared memory.\n"
"  -Q   Display system information about messages queues.\n"
"  -S   Display system information about semaphores.\n"
"  -T   Display system information about shared memory, message queues and\n"
"       semaphores.\n");
	exit(0);
}
#endif /* __CYGWIN__ */
