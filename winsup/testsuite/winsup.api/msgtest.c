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
 * Obtained from: $NetBSD: msgtest.c,v 1.7 2002/07/20 08:36:25 grant Exp $
 * $FreeBSD: /repoman/r/ncvs/src/tools/regression/sysvmsg/msgtest.c,v 1.1 2002/08/15 06:34:37 alfred Exp $
 */

/*
 * Test the SVID-compatible Message Queue facility.
 */

/*
 * CV, 2003-11-17: Add to Cygwin testsuite.
 */
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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

const char *TCID = "msgtest";	/* Test program identifier. */
int TST_TOTAL = 31;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

void	print_msqid_ds (struct msqid_ds *, mode_t);
void	sigsys_handler(int);
void	sigchld_handler (int);
void	cleanup (void);
void	receiver (void);

#define	MESSAGE_TEXT_LEN	255

/*
 * Define it as test_mymsg because we already have struct mymsg and we dont
 * want to conflict with it.  Also, regression fails when the default mymsg
 * struct is used, because mtext[] array is '1', so the passed string cannot
 * be processed.
 */
struct test_mymsg {
	long	mtype;
	char	mtext[MESSAGE_TEXT_LEN];
};

const char *m1_str = "California is overrated.";
const char *m2_str = "The quick brown fox jumped over the lazy dog.";

#define	MTYPE_1		1
#define	MTYPE_1_ACK	2

#define	MTYPE_2		3
#define	MTYPE_2_ACK	4

int	sender_msqid = -1;
pid_t	child_pid;

key_t	msgkey;

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	struct msqid_ds m_ds;
	struct test_mymsg m;
	sigset_t sigmask;

	Tst_count = 0;

	/*
	 * Install a SIGSYS handler so that we can exit gracefully if
	 * System V Message Queue support isn't in the kernel.
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

	msgkey = ftok("/", 4160);
	tst_resm (msgkey == (key_t)-1 ? TFAIL : TPASS,
		  "ftok(\"/\") returns valid value");

	/*
	 * Initialize child_pid to ourselves to that the cleanup function
	 * works before we create the receiver.
	 */
	child_pid = getpid();

	sender_msqid = msgget(msgkey, IPC_CREAT | 0640);
	tst_resm (sender_msqid == -1 ? TFAIL : TPASS, "sender calls msgget");

	tst_resm (msgctl(sender_msqid, IPC_STAT, &m_ds) == -1 ? TFAIL : TPASS,
		  "msgctl IPC_STAT");

	print_msqid_ds(&m_ds, 0640);

	m_ds.msg_perm.mode = (m_ds.msg_perm.mode & ~0777) | 0600;

	tst_resm (msgctl(sender_msqid, IPC_SET, &m_ds) == -1 ? TFAIL : TPASS,
		  "msgctl IPC_SET");

	bzero(&m_ds, sizeof m_ds);

	tst_resm (msgctl(sender_msqid, IPC_STAT, &m_ds) == -1 ? TFAIL : TPASS,
		  "msgctl IPC_STAT");

	tst_resm ((m_ds.msg_perm.mode & 0777) != 0600 ? TFAIL : TPASS,
		  "IPC_SET of mode holds");

	print_msqid_ds(&m_ds, 0600);

	switch ((child_pid = fork())) {
	case -1:
		tst_brkm (TBROK, cleanup, "fork");
		/* NOTREACHED */

	case 0:
		tst_resm (TPASS, "fork");
		receiver();
		break;

	default:
		Tst_count += 8;
		break;
	}

	/*
	 * Send the first message to the receiver and wait for the ACK.
	 */
	m.mtype = MTYPE_1;
	strcpy(m.mtext, m1_str);
	tst_resm (msgsnd(sender_msqid, &m, sizeof(m), 0) == -1 ? TFAIL : TPASS,
		  "sender: msgsnd 1");

	tst_resm (msgrcv(sender_msqid, &m, sizeof(m), MTYPE_1_ACK, 0)
	          != sizeof(m) ? TFAIL : TPASS, "sender: msgrcv 1 ack");

	print_msqid_ds(&m_ds, 0600);

	/*
	 * Send the second message to the receiver and wait for the ACK.
	 */
	m.mtype = MTYPE_2;
	strcpy(m.mtext, m2_str);
	tst_resm (msgsnd(sender_msqid, &m, sizeof(m), 0) == -1 ? TFAIL : TPASS,
		  "sender: msgsnd 2");

	tst_resm (msgrcv(sender_msqid, &m, sizeof(m), MTYPE_2_ACK, 0)
		  != sizeof(m) ? TFAIL : TPASS, "sender: msgrcv 2 ack");

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
sigsys_handler(int signo)
{

	tst_brkm (TBROK, cleanup,
		"System V Message Queue support is not present in the kernel");
}

void
sigchld_handler(int signo)
{
	struct msqid_ds m_ds;
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

	tst_resm (msgctl(sender_msqid, IPC_STAT, &m_ds) == -1 ? TFAIL : TPASS,
		  "msgctl IPC_STAT");

	print_msqid_ds(&m_ds, 0600);

	cleanup ();
}

void
cleanup()
{

	/*
	 * If we're the sender, and it exists, remove the message queue.
	 */
	if (child_pid != 0 && sender_msqid != -1) {
		tst_resm (msgctl(sender_msqid, IPC_RMID, NULL) == -1
			  ? TFAIL : TPASS, "msgctl IPC_RMID");
	}
	tst_exit ();
}

void
print_msqid_ds(struct msqid_ds *mp, mode_t mode)
{
	uid_t uid = geteuid();
	gid_t gid = getegid();

	printf("PERM: uid %d, gid %d, cuid %d, cgid %d, mode 0%o\n",
	    (int)mp->msg_perm.uid, (int)mp->msg_perm.gid,
	    (int)mp->msg_perm.cuid, (int)mp->msg_perm.cgid,
	    mp->msg_perm.mode & 0777);

	printf("qnum %lu, qbytes %lu, lspid %d, lrpid %d\n",
	    mp->msg_qnum, (u_long)mp->msg_qbytes, mp->msg_lspid,
	    mp->msg_lrpid);

	printf("stime: %s", ctime(&mp->msg_stime));
	printf("rtime: %s", ctime(&mp->msg_rtime));
	printf("ctime: %s", ctime(&mp->msg_ctime));

	/*
	 * Sanity check a few things.
	 */

	tst_resm (mp->msg_perm.uid != uid || mp->msg_perm.cuid != uid
		  ? TFAIL : TPASS, "uid matches");

	tst_resm (mp->msg_perm.gid != gid || mp->msg_perm.cgid != gid
		  ? TFAIL : TPASS, "gid matches");

	tst_resm ((mp->msg_perm.mode & 0777) != mode ? TFAIL : TPASS,
		  "mode matches");
}

void
receiver()
{
	struct test_mymsg m;
	int msqid;

	tst_resm ((msqid = msgget(msgkey, 0)) == -1 ? TFAIL : TPASS,
		  "receiver: msgget");

	/*
	 * Receive the first message, print it, and send an ACK.
	 */

	tst_resm (msgrcv(msqid, &m, sizeof(m), MTYPE_1, 0) != sizeof(m)
		   ? TFAIL : TPASS, "receiver: msgrcv 1");

	printf("%s\n", m.mtext);
	tst_resm (strcmp(m.mtext, m1_str) != 0 ? TFAIL : TPASS, 
		  "receiver: message 1 data is correct");

	m.mtype = MTYPE_1_ACK;

	tst_resm (msgsnd(msqid, &m, sizeof(m), 0) == -1 ? TFAIL : TPASS, 
		  "receiver: msgsnd ack 1");

	/*
	 * Receive the second message, print it, and send an ACK.
	 */

	tst_resm (msgrcv(msqid, &m, sizeof(m), MTYPE_2, 0) != sizeof(m)
		  ? TFAIL : TPASS, "receiver: msgrcv 2");

	printf("%s\n", m.mtext);
	tst_resm (strcmp(m.mtext, m2_str) != 0 ? TFAIL : TPASS, 
		  "receiver: message 2 data is correct");

	m.mtype = MTYPE_2_ACK;

	tst_resm (msgsnd(msqid, &m, sizeof(m), 0) == -1 ? TFAIL : TPASS, 
		  "receiver: msgsnd ack 2");

	/* Allow parent to receive message before getting SIGCHLD. */
	sleep (1);
	exit(0);
}
