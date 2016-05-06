/* Copyright (c) 2016 Phoenix Systems
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

#ifndef SYSLOG_H
#define SYSLOG_H

#include <stdlib.h>

#define	LOG_EMERG		0		/* System is unusable */
#define	LOG_ALERT		1		/* Action must be taken immediately */
#define	LOG_CRIT		2		/* Critical conditions */
#define	LOG_ERR			3		/* Error conditions */
#define	LOG_WARNING		4		/* Warning conditions */
#define	LOG_NOTICE		5		/* Normal but significant condition */
#define	LOG_INFO		6		/* Informational */
#define	LOG_DEBUG		7		/* Debug-level messages */

#define	LOG_PRIMASK		0x07								/* Mask to extract priority part (internal) */
#define	LOG_PRI(p)	((p) & LOG_PRIMASK)						/* Extract priority */
#define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))

#ifdef SYSLOG_NAMES
#define	INTERNAL_NOPRI	0x10								/* The "no priority" priority */
#define	INTERNAL_MARK	LOG_MAKEPRI(LOG_NFACILITIES, 0)		/* Mark "facility" */
typedef struct _code {
	char *c_name;
	int	c_val;
} CODE;

CODE prioritynames[] = {
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"debug",	LOG_DEBUG,
	"emerg",	LOG_EMERG,
	"err",		LOG_ERR,
	"error",	LOG_ERR,			/* DEPRECATED */
	"info",		LOG_INFO,
	"none",		INTERNAL_NOPRI,		/* INTERNAL */
	"notice",	LOG_NOTICE,
	"panic", 	LOG_EMERG,			/* DEPRECATED */
	"warn",		LOG_WARNING,		/* DEPRECATED */
	"warning",	LOG_WARNING,
	NULL,		-1,
};
#endif

/* Facility codes */
#define	LOG_KERN		(0 << 3)			/* Kernel messages */
#define	LOG_USER		(1 << 3)			/* Random user-level messages */
#define	LOG_MAIL		(2 << 3)			/* Mail system */
#define	LOG_DAEMON		(3 << 3)			/* System daemons */
#define	LOG_AUTH		(4 << 3)			/* Security/authorization messages */
#define	LOG_SYSLOG		(5 << 3)			/* Messages generated internally by syslogd */
#define	LOG_LPR			(6 << 3)			/* Line printer subsystem */
#define	LOG_NEWS		(7 << 3)			/* Network news subsystem */
#define	LOG_UUCP		(8 << 3)			/* UUCP subsystem */
#define	LOG_CRON		(9 << 3)			/* Clock daemon */
#define	LOG_AUTHPRIV	(10 << 3)			/* Security/authorization messages (private) */

/* Other codes through 15 reserved for system use */
#define	LOG_LOCAL0		(16 << 3)			/* Reserved for local use */
#define	LOG_LOCAL1		(17 << 3)			/* Reserved for local use */
#define	LOG_LOCAL2		(18 << 3)			/* Reserved for local use */
#define	LOG_LOCAL3		(19 << 3)			/* Reserved for local use */
#define	LOG_LOCAL4		(20 << 3)			/* Reserved for local use */
#define	LOG_LOCAL5		(21 << 3)			/* Reserved for local use */
#define	LOG_LOCAL6		(22 << 3)			/* Reserved for local use */
#define	LOG_LOCAL7		(23 << 3)			/* Reserved for local use */

#define	LOG_NFACILITIES	24							/* Current number of facilities */
#define	LOG_FACMASK		0x03f8						/* Mask to extract facility part */
#define	LOG_FAC(p)	(((p) & LOG_FACMASK) >> 3)		/* Facility of pri */

#ifdef SYSLOG_NAMES
CODE facilitynames[] = {
	"auth",		LOG_AUTH,
	"authpriv",	LOG_AUTHPRIV,
	"cron", 	LOG_CRON,
	"daemon",	LOG_DAEMON,
	"kern",		LOG_KERN,
	"lpr",		LOG_LPR,
	"mail",		LOG_MAIL,
	"mark", 	INTERNAL_MARK,		/* INTERNAL */
	"news",		LOG_NEWS,
	"security",	LOG_AUTH,			/* DEPRECATED */
	"syslog",	LOG_SYSLOG,
	"user",		LOG_USER,
	"uucp",		LOG_UUCP,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1,
};
#endif

/* Arguments to setlogmask */
#define	LOG_MASK(pri)	(1 << (pri))				/* Mask for one priority */
#define	LOG_UPTO(pri)	((1 << ((pri) + 1)) - 1)	/* All priorities through pri */

/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define	LOG_PID			0x01		/* Log the pid with each message */
#define	LOG_CONS		0x02		/* Log on the console if errors in sending */
#define	LOG_ODELAY		0x04		/* Delay open until first syslog() (default) */
#define	LOG_NDELAY		0x08		/* Don't delay open */
#define	LOG_NOWAIT		0x10		/* Don't wait for console forks: DEPRECATED */
#define	LOG_PERROR		0x20		/* Log to stderr as well */

void openlog(const char *ident, int option, int facility);
void syslog(int priority, const char *format, ...);
void closelog();

#endif
