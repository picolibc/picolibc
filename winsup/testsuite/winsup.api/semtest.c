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
 * Obtained from: $NetBSD: semtest.c,v 1.4 2002/07/20 08:36:25 grant Exp $
 * $FreeBSD: /repoman/r/ncvs/src/tools/regression/sysvsem/semtest.c,v 1.1 2002/08/15 06:34:37 alfred Exp $
 */

/*
 * Test the SVID-compatible Semaphore facility.
 */

/*
 * CV, 2003-11-17: Add to Cygwin testsuite.
 */
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/sem.h>
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

const char *TCID = "semtest";   /* Test program identifier. */
int TST_TOTAL = 54;             /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */

int	main (int, char *[]);
void	print_semid_ds (struct semid_ds *, mode_t);
void	sigsys_handler (int);
void	sigchld_handler(int);
void	cleanup (void);
void	waiter (void);

int	sender_semid = -1;
pid_t	child_pid;
int	child_count;
int	signal_was_sigchld;

key_t	semkey;

/*
 * This is the original semun union used by the sysvsem utility.
 * It is deliberately kept here under #if 0'ed condition for future
 * reference. PLEASE DO NOT REMOVE.  The {SET,GET}ALL in FreeBSD
 * are signed values, so the default version in sys/sem.h suffices.
 */
#if 1 /*0*/
union semun {
	int	val;		/* value for SETVAL */
	struct	semid_ds *buf;	/* buffer for IPC_{STAT,SET} */
	u_short	*array;		/* array for GETALL & SETALL */
};
#endif

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	union semun sun;
	struct semid_ds s_ds;
	sigset_t sigmask;
	int i;

	Tst_count = 0;

	/*
	 * Install a SIGSYS handler so that we can exit gracefully if
	 * System V Semaphore support isn't in the kernel.
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

	semkey = ftok("/", 4160);
	tst_resm (semkey == (key_t)-1 ? TFAIL : TPASS,
		  "ftok(\"/\") returns valid value");

	/*
	 * Initialize child_pid to ourselves to that the cleanup function
	 * works before we create the receiver.
	 */
	child_pid = getpid();

	sender_semid = semget(semkey, 1, IPC_CREAT | 0640);
	tst_resm (sender_semid == -1 ? TFAIL : TPASS, "sender calls semget");
	
	sun.buf = &s_ds;
	tst_resm (semctl(sender_semid, 0, IPC_STAT, sun) == -1 ? TFAIL : TPASS, 
		  "semctl IPC_STAT");

	print_semid_ds(&s_ds, 0640);

	s_ds.sem_perm.mode = (s_ds.sem_perm.mode & ~0777) | 0600;

	sun.buf = &s_ds;
	tst_resm (semctl(sender_semid, 0, IPC_SET, sun) == -1 ? TFAIL : TPASS, 
		  "semctl IPC_SET");

	memset(&s_ds, 0, sizeof(s_ds));

	sun.buf = &s_ds;
	tst_resm (semctl(sender_semid, 0, IPC_STAT, sun) == -1 ? TFAIL : TPASS, 
		  "semctl IPC_STAT");

	tst_resm ((s_ds.sem_perm.mode & 0777) != 0600 ? TFAIL : TPASS, 
		  "IPC_SET of mode holds");

	print_semid_ds(&s_ds, 0600);

	for (child_count = 0; child_count < 5; child_count++) {
		switch ((child_pid = fork())) {
		case -1:
			tst_brkm (TBROK, cleanup, "fork");
			/* NOTREACHED */

		case 0:
			Tst_count += 22 + child_count * 4;
			tst_resm (TPASS, "fork");
			waiter();
			break;

		default:
			break;
		}
	}

	/*
	 * Wait for all of the waiters to be attempting to acquire the
	 * semaphore.
	 */
	for (;;) {
		i = semctl(sender_semid, 0, GETNCNT);
		if (i == -1)
			tst_brkm (TBROK, cleanup, "semctl GETNCNT");
		if (i == 5)
			break;
	}

	/*
	 * Now set the thundering herd in motion by initializing the
	 * semaphore to the value 1.
	 */
	sun.val = 1;
	tst_resm (semctl(sender_semid, 0, SETVAL, sun) == -1 ? TFAIL : TPASS,
		  "sender: semctl SETVAL to 1");

	/*
	 * Suspend forever; when we get SIGCHLD, the handler will exit.
	 */
	sigemptyset(&sigmask);
	for (;;) {
		(void) sigsuspend(&sigmask);
		if (signal_was_sigchld)
			signal_was_sigchld = 0;
		else
			break;
	}

	/*
	 * ...and any other signal is an unexpected error.
	 */

	tst_brkm (TBROK, cleanup, "sender: received unexpected signal");
	exit (1);
}

void
sigsys_handler(int signo)
{

	tst_brkm (TBROK, cleanup,
		  "System V Semaphore support is not present in the kernel");
}

void
sigchld_handler(int signo)
{
	union semun sun;
	struct semid_ds s_ds;
	int cstatus;

	/*
	 * Reap the child; if it exited successfully, then we're on the
	 * right track!
	 */
	if (wait(&cstatus) == -1)
		tst_brkm (TBROK, cleanup, "wait");

	if (WIFEXITED(cstatus) == 0)
		tst_brkm (TBROK, cleanup, "receiver exited abnormally");

	if (WEXITSTATUS(cstatus) != 0)
		tst_brkm (TBROK, cleanup, "receiver exited with status %d",
			  WEXITSTATUS(cstatus));

	/*
	 * If we get here, the child has exited normally, and we should
	 * decrement the child count.  If the child_count reaches 0, we
	 * should exit.
	 */

	sun.buf = &s_ds;
	tst_resm (semctl(sender_semid, 0, IPC_STAT, sun) == -1 ? TFAIL : TPASS,
		  "semctl IPC_STAT");

	print_semid_ds(&s_ds, 0600);

	if (--child_count != 0) {
		signal_was_sigchld = 1;
		return;
	}

	cleanup ();
}

void
cleanup()
{

	/*
	 * If we're the sender, and it exists, remove the message queue.
	 */
	if (child_pid != 0 && sender_semid != -1) {
		tst_resm (semctl(sender_semid, 0, IPC_RMID) == -1
			  ? TFAIL : TPASS, "semctl IPC_RMID");
	}
	tst_exit ();
}

void
print_semid_ds(struct semid_ds *sp, mode_t mode)
{
	uid_t uid = geteuid();
	gid_t gid = getegid();

	printf("PERM: uid %d, gid %d, cuid %d, cgid %d, mode 0%o\n",
	    (int)sp->sem_perm.uid, (int)sp->sem_perm.gid,
	    (int)sp->sem_perm.cuid, (int)sp->sem_perm.cgid,
	    sp->sem_perm.mode & 0777);

	printf("nsems %u\n", sp->sem_nsems);

	printf("otime: %s", ctime(&sp->sem_otime));
	printf("ctime: %s", ctime(&sp->sem_ctime));

	/*
	 * Sanity check a few things.
	 */

	tst_resm (sp->sem_perm.uid != uid || sp->sem_perm.cuid != uid
		  ? TFAIL : TPASS, "uid matches");

	tst_resm (sp->sem_perm.gid != gid || sp->sem_perm.cgid != gid
		  ? TFAIL : TPASS, "gid matches");

	tst_resm ((sp->sem_perm.mode & 0777) != mode
		  ? TFAIL : TPASS, "mode matches");
}

void
waiter()
{
	struct sembuf s;
	int semid;

	tst_resm ((semid = semget(semkey, 1, 0)) == -1 ? TFAIL : TPASS,
		  "waiter: semget");

	/*
	 * Attempt to acquire the semaphore.
	 */
	s.sem_num = 0;
	s.sem_op = -1;
	s.sem_flg = SEM_UNDO;

	tst_resm (semop(semid, &s, 1) == -1 ? TFAIL : TPASS,
		  "waiter: semop -1");

	printf("WOO!  GOT THE SEMAPHORE!\n");
	sleep(1);

	/*
	 * Release the semaphore and exit.
	 */
	s.sem_num = 0;
	s.sem_op = 1;
	s.sem_flg = SEM_UNDO;

	tst_resm (semop(semid, &s, 1) == -1 ? TFAIL : TPASS,
		  "waiter: semop +1");

	/* Allow parent to receive message before getting SIGCHLD. */
	sleep (1);
	exit(0);
}
