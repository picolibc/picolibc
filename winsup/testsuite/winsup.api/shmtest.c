/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Obtained from: $NetBSD: shmtest.c,v 1.3 2002/07/20 08:36:26 grant Exp $
 * $FreeBSD: /repoman/r/ncvs/src/tools/regression/sysvshm/shmtest.c,v 1.1 2002/08/15 06:34:37 alfred Exp $
 */

/*
 * Test the SVID-compatible Shared Memory facility.
 */

#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "test.h"
#include "usctest.h"

const char *TCID = "shmtest";   /* Test program identifier. */
int TST_TOTAL = 22;             /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */

int	main __P((int, char *[]));
void	print_shmid_ds __P((struct shmid_ds *, mode_t));
void	sigsys_handler __P((int));
void	sigchld_handler __P((int));
void	cleanup __P((void));
void	receiver __P((void));

const char *m_str = "The quick brown fox jumped over the lazy dog.";

int	sender_shmid = -1;
pid_t	child_pid;

key_t	shmkey;

size_t	pgsize;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct sigaction sa;
	struct shmid_ds s_ds;
	sigset_t sigmask;
	char *shm_buf;

	Tst_count = 0;

	/*
	 * Install a SIGSYS handler so that we can exit gracefully if
	 * System V Shared Memory support isn't in the kernel.
	 */
	sa.sa_handler = sigsys_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGSYS, &sa, NULL) == -1)
		tst_brkm (TBROK, cleanup, "sigaction SIGSYS");

	/*
	 * Install and SIGCHLD handler to deal with all possible exit
	 * conditions of the receiver.
	 */
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		tst_brkm (TBROK, cleanup, "sigaction SIGCHLD");

	pgsize = sysconf(_SC_PAGESIZE);

	shmkey = ftok("/", 4160);
	tst_resm (shmkey == (key_t)-1 ? TFAIL : TPASS,
		  "ftok(\"/\") returns valid value");

	/*
	 * Initialize child_pid to ourselves to that the cleanup function
	 * works before we create the receiver.
	 */
	child_pid = getpid();

	sender_shmid = shmget(shmkey, pgsize, IPC_CREAT | 0640);
	tst_resm (sender_shmid == -1 ? TFAIL : TPASS, "sender calls shmget");

	tst_resm (shmctl(sender_shmid, IPC_STAT, &s_ds) == -1 ? TFAIL : TPASS,
		  "shmctl IPC_STAT");

	print_shmid_ds(&s_ds, 0640);

	s_ds.shm_perm.mode = (s_ds.shm_perm.mode & ~0777) | 0600;

	tst_resm (shmctl(sender_shmid, IPC_SET, &s_ds) == -1 ? TFAIL : TPASS,
		  "shmctl IPC_SET");

	memset(&s_ds, 0, sizeof(s_ds));

	tst_resm (shmctl(sender_shmid, IPC_STAT, &s_ds) == -1 ? TFAIL : TPASS,
		  "shmctl IPC_STAT");

	tst_resm ((s_ds.shm_perm.mode & 0777) != 0600 ? TFAIL : TPASS,
		  "IPC_SET of mode holds");

	print_shmid_ds(&s_ds, 0600);

	tst_resm ((shm_buf = shmat(sender_shmid, NULL, 0)) == (void *) -1
		  ? TFAIL : TPASS, "sender: shmat");

	/*
	 * Write the test pattern into the shared memory buffer.
	 */
	strcpy(shm_buf, m_str);

	switch ((child_pid = fork())) {
	case -1:
		tst_brkm (TBROK, cleanup, "fork");
		/* NOTREACHED */

	case 0:
		tst_resm (TPASS, "fork");
		receiver();
		break;

	default:
		Tst_count += 4;
		break;
	}

	/*
	 * Suspend forever; when we get SIGCHLD, the handler will exit.
	 */
	sigemptyset(&sigmask);
	(void) sigsuspend(&sigmask);

	/*
	 * ...and any other signal is an unexpected error.
	 */
	tst_brkm (TBROK, cleanup, "sender: received unexpected signal");
	exit (1);
}

void
sigsys_handler(signo)
	int signo;
{

	tst_brkm (TBROK, cleanup,
		"System V Shared Memory support is not present in the kernel");
}

void
sigchld_handler(signo)
	int signo;
{
	struct shmid_ds s_ds;
	int cstatus;

	/*
	 * Reap the child; if it exited successfully, then the test passed!
	 */
	if (waitpid(child_pid, &cstatus, 0) != child_pid)
		tst_brkm (TBROK, cleanup, "waitpid");

	if (WIFEXITED(cstatus) == 0)
		tst_brkm (TBROK, cleanup, "receiver exited abnormally");

	if (WEXITSTATUS(cstatus) != 0)
		tst_brkm (TBROK, cleanup, "receiver exited with status %d",
			  WEXITSTATUS(cstatus));

	/*
	 * If we get here, the child has exited normally, and thus
	 * we should exit normally too.  First, tho, we print out
	 * the final stats for the message queue.
	 */

	tst_resm (shmctl(sender_shmid, IPC_STAT, &s_ds) == -1 ? TFAIL : TPASS,
		  "shmctl IPC_STAT");

	print_shmid_ds(&s_ds, 0600);

	cleanup ();
}

void
cleanup()
{

	/*
	 * If we're the sender, and it exists, remove the shared memory area.
	 */
	if (child_pid != 0 && sender_shmid != -1) {
		tst_resm (shmctl(sender_shmid, IPC_RMID, NULL) == -1
			  ? TFAIL : TPASS, "shmctl IPC_RMID");
	}
	tst_exit ();
}

void
print_shmid_ds(sp, mode)
	struct shmid_ds *sp;
	mode_t mode;
{
	uid_t uid = geteuid();
	gid_t gid = getegid();

	printf("PERM: uid %d, gid %d, cuid %d, cgid %d, mode 0%o\n",
	    (int)sp->shm_perm.uid, (int)sp->shm_perm.gid,
	    (int)sp->shm_perm.cuid, (int)sp->shm_perm.cgid,
	    sp->shm_perm.mode & 0777);

	printf("segsz %lu, lpid %d, cpid %d, nattch %u\n",
	    (u_long)sp->shm_segsz, sp->shm_lpid, sp->shm_cpid,
	    sp->shm_nattch);

	printf("atime: %s", ctime(&sp->shm_atime));
	printf("dtime: %s", ctime(&sp->shm_dtime));
	printf("ctime: %s", ctime(&sp->shm_ctime));

	/*
	 * Sanity check a few things.
	 */

	tst_resm (sp->shm_perm.uid != uid || sp->shm_perm.cuid != uid
		  ? TFAIL : TPASS, "uid matches");

	tst_resm (sp->shm_perm.gid != gid || sp->shm_perm.cgid != gid
		  ? TFAIL : TPASS, "gid matches");

	tst_resm ((sp->shm_perm.mode & 0777) != mode ? TFAIL : TPASS,
		  "mode matches");
}

void
receiver()
{
	int shmid;
	void *shm_buf;

	tst_resm ((shmid = shmget(shmkey, pgsize, 0)) == -1 ? TFAIL : TPASS,
		  "receiver: shmget");

	tst_resm ((shm_buf = shmat(shmid, NULL, 0)) == (void *) -1
		  ? TFAIL : TPASS, "receiver: shmat");

	printf("%s\n", (const char *)shm_buf);
	tst_resm (strcmp((const char *)shm_buf, m_str) != 0 ? TFAIL : TPASS,
		  "receiver: data is correct");

	exit(0);
}
