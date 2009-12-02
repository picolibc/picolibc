/* syslog.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/un.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"

#define CYGWIN_LOG_NAME "Cygwin"

/* openlog: save the passed args. Don't open the system log or /dev/log yet.  */
extern "C" void
openlog (const char *ident, int logopt, int facility)
{
    debug_printf ("openlog called with (%s, %d, %d)",
		       ident ? ident : "<NULL>", logopt, facility);

    if (_my_tls.locals.process_ident != NULL)
      {
	free (_my_tls.locals.process_ident);
	_my_tls.locals.process_ident = NULL;
      }
    if (ident)
      {
	_my_tls.locals.process_ident = (char *) malloc (strlen (ident) + 1);
	if (!_my_tls.locals.process_ident)
	  {
	    debug_printf ("failed to allocate memory for _my_tls.locals.process_ident");
	    return;
	  }
	strcpy (_my_tls.locals.process_ident, ident);
      }
    _my_tls.locals.process_logopt = logopt;
    _my_tls.locals.process_facility = facility;
}

/* setlogmask: set the log priority mask and return previous mask.
   If maskpri is zero, just return previous. */
int
setlogmask (int maskpri)
{
  if (maskpri == 0)
    return _my_tls.locals.process_logmask;

  int old_mask = _my_tls.locals.process_logmask;
  _my_tls.locals.process_logmask = maskpri;

  return old_mask;
}

/* Private class used to handle formatting of syslog message
   It is named pass_handler because it does a two-pass handling of log
   strings.  The first pass counts the length of the string, and the second
   one builds the string. */

class pass_handler
{
  private:
    FILE *fp_;
    char *message_;
    int total_len_;

    void shutdown ();

    /* Explicitly disallow copies */
    pass_handler (const pass_handler &);
    pass_handler & operator = (const pass_handler &);

  public:
    pass_handler ();
    ~pass_handler ();

    int initialize (int);

    int print (const char *,...);
    int print_va (const char *, va_list);
    char *get_message () const { return message_; }
    void set_message (char *s) { message_ = s; *message_ = '\0'; }
};

pass_handler::pass_handler () : fp_ (0), message_ (0), total_len_ (0)
{
  ;
}

pass_handler::~pass_handler ()
{
  shutdown ();
}

void
pass_handler::shutdown ()
{
  if (fp_ != NULL)
    {
      fclose (fp_);
      fp_ = 0;
    }
}

int
pass_handler::initialize (int pass_number)
{
    shutdown ();
    if (pass_number)
      return total_len_ + 1;

    fp_ = fopen ("/dev/null", "wb");
    setbuf (fp_, NULL);
    if (fp_ == NULL)
      {
	debug_printf ("failed to open /dev/null");
	return -1;
      }
    total_len_ = 0;
    return 0;
}

int
pass_handler::print (const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    int ret = print_va (fmt, ap);
    va_end (ap);
    return ret;
}

int
pass_handler::print_va (const char *fmt, va_list list)
{
    if (fp_ != NULL)
      {
	int len = vfprintf (fp_, fmt, list);
	if (len < 0)
	  return -1;
	total_len_ += len;
	return 0;
      }
    else if (message_ != NULL)
      {
	char *printpos = &message_[strlen (message_)];
	vsprintf (printpos, fmt, list);
	return 0;
      }
    debug_printf ("FAILURE ! fp_ and message_ both 0!! ");
    return -1;
}

static NO_COPY muto try_connect_guard;
static enum {
  not_inited,
  inited_failed,
  inited_dgram,
  inited_stream
} syslogd_inited;
static int syslogd_sock = -1;
extern "C" int cygwin_socket (int, int, int);
extern "C" int cygwin_connect (int, const struct sockaddr *, int);

static void
connect_syslogd ()
{
  struct __stat64 st;
  int fd;
  struct sockaddr_un sun;

  if (syslogd_inited != not_inited && syslogd_sock >= 0)
    close (syslogd_sock);
  syslogd_inited = inited_failed;
  syslogd_sock = -1;
  if (stat64 (_PATH_LOG, &st) || !S_ISSOCK (st.st_mode))
    return;
  if ((fd = cygwin_socket (AF_LOCAL, SOCK_DGRAM, 0)) < 0)
    return;
  sun.sun_family = AF_LOCAL;
  strncpy (sun.sun_path, _PATH_LOG, sizeof sun.sun_path);
  if (cygwin_connect (fd, (struct sockaddr *) &sun, sizeof sun))
    {
      if (get_errno () != EPROTOTYPE)
	{
	  close (fd);
	  return;
	}
      /* Retry with SOCK_STREAM. */
      if ((fd = cygwin_socket (AF_LOCAL, SOCK_STREAM, 0)) < 0)
	return;
      if (cygwin_connect (fd, (struct sockaddr *) &sun, sizeof sun))
	{
	  close (fd);
	  return;
	}
      syslogd_inited = inited_stream;
    }
  else
    syslogd_inited = inited_dgram;
  syslogd_sock = fd;
  fcntl64 (syslogd_sock, F_SETFD, FD_CLOEXEC);
  return;
}

static int
try_connect_syslogd (int priority, const char *msg, int len)
{
  ssize_t ret = -1;

  try_connect_guard.init ("try_connect_guard")->acquire ();
  if (syslogd_inited == not_inited)
    connect_syslogd ();
  if (syslogd_sock >= 0)
    {
      char pribuf[16];
      sprintf (pribuf, "<%d>", priority);
      struct iovec iv[2] =
      {
	{ pribuf, strlen (pribuf) },
	{ (char *) msg, len }
      };

      ret = writev (syslogd_sock, iv, 2);
      /* If the syslog daemon has been restarted and /dev/log was
	 a stream socket, the connection is broken.  In this case,
	 try to reopen the socket and try again. */
      if (ret < 0 && syslogd_inited == inited_stream)
	{
	  connect_syslogd ();
	  if (syslogd_sock >= 0)
	    ret = writev (syslogd_sock, iv, 2);
	}
      /* If write fails and LOG_CONS is set, return failure to vsyslog so
	 it falls back to the usual logging method for this OS. */
      if (ret >= 0 || !(_my_tls.locals.process_logopt & LOG_CONS))
	ret = syslogd_sock;
    }
  try_connect_guard.release ();
  return ret;
}

/* syslog: creates the log message and writes to /dev/log, or to the
   NT system log if /dev/log isn't available.

   FIXME. WinNT system log messages don't look pretty, but in order to
   fix this we have to embed resources in the code and tell the NT
   registry where we are, blech (what happens if we move ?).  We could,
   however, add the resources in Cygwin and always point to that. */

extern "C" void
vsyslog (int priority, const char *message, va_list ap)
{
  debug_printf ("%x %s", priority, message);
  /* If the priority fails the current mask, reject */
  if ((LOG_MASK (LOG_PRI (priority)) & _my_tls.locals.process_logmask) == 0)
    {
      debug_printf ("failing message %x due to priority mask %x",
		    priority, _my_tls.locals.process_logmask);
      return;
    }

  /* Set default facility to LOG_USER if not yet set via openlog. */
  if (!_my_tls.locals.process_facility)
    _my_tls.locals.process_facility = LOG_USER;

  /* Add default facility if not in the given priority. */
  if (!(priority & LOG_FACMASK))
    priority |= _my_tls.locals.process_facility;

  /* Translate %m in the message to error text */
  char *errtext = strerror (get_errno ());
  int errlen = strlen (errtext);
  int numfound = 0;

  for (const char *cp = message; *cp; cp++)
    if (*cp == '%' && cp[1] == 'm')
      numfound++;

  char *newmessage = (char *) alloca (strlen (message) +
				      (errlen * numfound) + 1);

  if (newmessage == NULL)
    {
      debug_printf ("failed to allocate newmessage");
      return;
    }

  char *dst = newmessage;
  for (const char *cp2 = message; *cp2; cp2++)
    if (*cp2 == '%' && cp2[1] == 'm')
      {
	cp2++;
	strcpy (dst, errtext);
	while (*dst)
	  dst++;
      }
    else
      *dst++ = *cp2;

  *dst = '\0';
  message = newmessage;

  /* Work out the priority type - we ignore the facility for now.. */
  WORD eventType;
  switch (LOG_PRI (priority))
    {
    case LOG_EMERG:
    case LOG_ALERT:
    case LOG_CRIT:
    case LOG_ERR:
      eventType = EVENTLOG_ERROR_TYPE;
      break;
    case LOG_WARNING:
      eventType = EVENTLOG_WARNING_TYPE;
      break;
    case LOG_NOTICE:
    case LOG_INFO:
    case LOG_DEBUG:
      eventType = EVENTLOG_INFORMATION_TYPE;
      break;
    default:
      eventType = EVENTLOG_ERROR_TYPE;
      break;
    }

  /* We need to know how long the buffer needs to be.
     The only legal way I can see of doing this is to
     do a vfprintf to /dev/null, and count the bytes
     output, then do it again to a malloc'ed string. This
     is ugly, slow, but prevents core dumps :-).
   */
  pass_handler pass;
  for (int pass_number = 0; pass_number < 2; ++pass_number)
    {
      int n = pass.initialize (pass_number);
      if (n == -1)
	return;
      else if (n > 0)
	pass.set_message ((char *) alloca (n));

      /* Deal with ident_string */
      if (_my_tls.locals.process_ident != NULL)
	{
	  if (pass.print ("%s: ", _my_tls.locals.process_ident) == -1)
	    return;
	}
      if (_my_tls.locals.process_logopt & LOG_PID)
	{
	  if (pass.print ("PID %u: ", getpid ()) == -1)
	    return;
	}

      /* Print out the variable part */
      if (pass.print_va (message, ap) == -1)
	return;

    }
  const char *msg_strings[1];
  char *total_msg = pass.get_message ();
  int len = strlen (total_msg);
  if (len != 0 && (total_msg[len - 1] == '\n'))
    total_msg[--len] = '\0';

  msg_strings[0] = total_msg;

  if (_my_tls.locals.process_logopt & LOG_PERROR)
    {
      write (STDERR_FILENO, total_msg, len);
      write (STDERR_FILENO, "\n", 1);
    }

  int fd;
  if ((fd = try_connect_syslogd (priority, total_msg, len + 1)) < 0)
    {
      /* If syslogd isn't present, open the event log and send the message */
      HANDLE hEventSrc = RegisterEventSourceA (NULL, (_my_tls.locals.process_ident != NULL) ?
				       _my_tls.locals.process_ident : CYGWIN_LOG_NAME);
      if (hEventSrc == NULL)
	{
	  debug_printf ("RegisterEventSourceA failed with %E");
	  return;
	}
      if (!ReportEventA (hEventSrc, eventType, 0, 0,
			 cygheap->user.sid (), 1, 0, msg_strings, NULL))
	debug_printf ("ReportEventA failed with %E");
      DeregisterEventSource (hEventSrc);
    }
}

extern "C" void
syslog (int priority, const char *message, ...)
{
  va_list ap;
  va_start (ap, message);
  vsyslog (priority, message, ap);
  va_end (ap);
}

static NO_COPY muto klog_guard;
fhandler_mailslot *dev_kmsg;

extern "C" void
vklog (int priority, const char *message, va_list ap)
{
  /* TODO: kernel messages are under our control entirely and they should
     be quick.  No playing with /dev/null, but a fixed upper size for now. */
  char buf[2060];	/* 2048 + a prority */
  if (!(priority & ~LOG_PRIMASK))
    priority = LOG_KERN | LOG_PRI (priority);
  __small_sprintf (buf, "<%d>", priority);
  __small_vsprintf (buf + strlen (buf), message, ap);
  klog_guard.init ("klog_guard")->acquire ();
  if (!dev_kmsg)
    dev_kmsg = (fhandler_mailslot *) build_fh_name ("/dev/kmsg");
  if (dev_kmsg && !dev_kmsg->get_handle ())
    dev_kmsg->open (O_WRONLY, 0);
  if (dev_kmsg && dev_kmsg->get_handle ())
    dev_kmsg->write (buf, strlen (buf) + 1);
  klog_guard.release ();
}

extern "C" void
klog (int priority, const char *message, ...)
{
  va_list ap;
  va_start (ap, message);
  vklog (priority, message, ap);
  va_end (ap);
}

extern "C" void
closelog (void)
{
  try_connect_guard.init ("try_connect_guard")->acquire ();
  if (syslogd_inited != not_inited && syslogd_sock >= 0)
    {
      close (syslogd_sock);
      syslogd_sock = -1;
      syslogd_inited = not_inited;
    }
  try_connect_guard.release ();
}
