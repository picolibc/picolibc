/* sys/syslog.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_LOG_H
#define	_SYS_LOG_H

#include <sys/cdefs.h>
#include <stdarg.h>

#define _PATH_LOG	"/dev/log"
#define _PATH_KLOG	"/dev/kmsg"

#define	LOG_EMERG	0
#define	LOG_ALERT	1
#define	LOG_CRIT	2
#define	LOG_ERR		3
#define	LOG_WARNING	4
#define	LOG_NOTICE	5
#define	LOG_INFO	6
#define	LOG_DEBUG	7

#define	LOG_PRIMASK	0x07

#define	LOG_PRI(p)	((p) & LOG_PRIMASK)
#define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))

#define	LOG_KERN	(0<<3)
#define	LOG_USER	(1<<3)
#define	LOG_MAIL	(2<<3)
#define	LOG_DAEMON	(3<<3)
#define	LOG_AUTH	(4<<3)
#define	LOG_SYSLOG	(5<<3)
#define	LOG_LPR		(6<<3)
#define	LOG_NEWS	(7<<3)
#define	LOG_UUCP	(8<<3)
#define	LOG_CRON	(9<<3)
#define	LOG_AUTHPRIV	(10<<3)
#define LOG_FTP		(11<<3)

/* Codes through 15 are reserved for system use */
#define LOG_LOCAL0	(16<<3)
#define LOG_LOCAL1	(17<<3)
#define LOG_LOCAL2	(18<<3)
#define LOG_LOCAL3	(19<<3)
#define LOG_LOCAL4	(20<<3)
#define LOG_LOCAL5	(21<<3)
#define LOG_LOCAL6	(22<<3)
#define LOG_LOCAL7	(23<<3)

#define	LOG_NFACILITIES	24
#define	LOG_FACMASK	0x03f8
#define	LOG_FAC(p)	(((p) & LOG_FACMASK) >> 3)

#ifdef SYSLOG_NAMES

#define INTERNAL_NOPRI	0x10				/* Maps to "none" */
#define INTERNAL_MARK	LOG_MAKEPRI(LOG_NFACILITIES, 0)	/* Maps to "mark" */

typedef struct _code {
  char *c_name;
  int   c_val;
} CODE;

CODE prioritynames[] = {
  { "alert",	LOG_ALERT },
  { "crit",	LOG_CRIT },
  { "debug",	LOG_DEBUG },
  { "emerg",	LOG_EMERG },
  { "err",	LOG_ERR },
  { "error",	LOG_ERR },		/* Deprecated */
  { "info",	LOG_INFO },
  { "none",	INTERNAL_NOPRI },
  { "notice",	LOG_NOTICE },
  { "panic",	LOG_EMERG },		/* Deprecated */
  { "warn",	LOG_WARNING },		/* Deprecated */
  { "warning",	LOG_WARNING },
  { NULL, -1 }
};

CODE facilitynames[] = {
  { "auth",	LOG_AUTH },
  { "authpriv",	LOG_AUTHPRIV },
  { "cron",	LOG_CRON },
  { "daemon",	LOG_DAEMON },
  { "ftp",	LOG_FTP },
  { "kern",	LOG_KERN },
  { "lpr",	LOG_LPR },
  { "mail",	LOG_MAIL },
  { "mark",	INTERNAL_MARK },
  { "news",	LOG_NEWS },
  { "security",	LOG_AUTH },		/* Deprecated */
  { "syslog",	LOG_SYSLOG },
  { "user",	LOG_USER },
  { "uucp",	LOG_UUCP },
  { "local0",	LOG_LOCAL0 },
  { "local1",	LOG_LOCAL1 },
  { "local2",	LOG_LOCAL2 },
  { "local3",	LOG_LOCAL3 },
  { "local4",	LOG_LOCAL4 },
  { "local5",	LOG_LOCAL5 },
  { "local6",	LOG_LOCAL6 },
  { "local7",	LOG_LOCAL7 },
  { NULL,	-1 }
};

#endif /* SYSLOG_NAMES */

#define	LOG_MASK(pri)	(1 << (pri))
#define	LOG_UPTO(pri)	((1 << ((pri)+1)) - 1)

/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define LOG_PID         0x01    /* log the pid with each message */
#define LOG_CONS        0x02    /* log on the console if errors in sending */
#define LOG_ODELAY      0x04    /* delay open until first syslog() (default) */
#define LOG_NDELAY      0x08    /* don't delay open */
#define LOG_NOWAIT      0x10    /* don't wait for console forks: DEPRECATED */
#define LOG_PERROR      0x20    /* log to stderr as well */

__BEGIN_DECLS


void	closelog (void);
void	openlog (const char *, int, int);
int	setlogmask (int);
void	syslog (int, const char *, ...);
void	vsyslog (int, const char *, va_list ap);

__END_DECLS


#endif /* _SYS_LOG_H */
