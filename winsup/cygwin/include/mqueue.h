/* mqueue.h: POSIX message queue interface

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/cdefs.h>

#ifndef _MQUEUE_H
#define _MQUEUE_H

__BEGIN_DECLS

typedef intptr_t mqd_t;

struct mq_attr {
  long  mq_flags;	/* Message queue flags */
  long  mq_maxmsg;	/* Max number of messages in queue */
  long  mq_msgsize;	/* Max message size */
  long  mq_curmsgs;	/* Current number of messages in queue */
};

int     mq_close (mqd_t);
int     mq_getattr (mqd_t, struct mq_attr *);
int     mq_notify (mqd_t, const struct sigevent *);
mqd_t   mq_open (const char *, int, ...);
ssize_t mq_receive (mqd_t, char *, size_t, unsigned int *);
int     mq_send (mqd_t, const char *, size_t, unsigned int);
int     mq_setattr (mqd_t, const struct mq_attr *, struct mq_attr *);
ssize_t mq_timedreceive (mqd_t, char *, size_t, unsigned int *,
			 const struct timespec *);
int     mq_timedsend (mqd_t, const char *, size_t, unsigned int,
		      const struct timespec *);
int     mq_unlink (const char *name);

__END_DECLS

#endif /* _MQUEUE_H */
