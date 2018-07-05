/* syslog.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__
#define USE_SYS_TYPES_FD_SET
#include "winsup.h"
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"
#include "tls_pbuf.h"

#define CYGWIN_LOG_NAME L"Cygwin"

static struct
{
  wchar_t *process_ident;
  int process_logopt;
  int process_facility;
  int process_logmask;
} syslog_globals = { NULL, 0, 0, LOG_UPTO (LOG_DEBUG) };

/* openlog: save the passed args. Don't open the system log or /dev/log yet.  */
extern "C" void
openlog (const char *ident, int logopt, int facility)
{
    wchar_t *new_ident = NULL;

    debug_printf ("openlog called with (%s, %d, %d)",
		  ident ? ident : "<NULL>", logopt, facility);

    if (ident)
      {
	sys_mbstowcs_alloc (&new_ident, HEAP_NOTHEAP, ident);
	if (!new_ident)
	    debug_printf ("failed to allocate memory for "
			  "syslog_globals.process_ident");
	else
	  {
	    wchar_t *old_ident = syslog_globals.process_ident;
	    syslog_globals.process_ident = new_ident;
	    if (old_ident)
	      free (old_ident);
	  }
      }
    syslog_globals.process_logopt = logopt;
    syslog_globals.process_facility = facility;
}

/* setlogmask: set the log priority mask and return previous mask.
   If maskpri is zero, just return previous. */
int
setlogmask (int maskpri)
{
  if (maskpri == 0)
    return syslog_globals.process_logmask;

  int old_mask = syslog_globals.process_logmask;
  syslog_globals.process_logmask = maskpri;

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
extern int get_inet_addr_local (const struct sockaddr *, int,
			  struct sockaddr_storage *, int *,
			  int * = NULL, int * = NULL);

static void
connect_syslogd ()
{
  int fd;
  struct sockaddr_un sun;
  struct sockaddr_storage sst;
  int len, type;

  if (syslogd_inited != not_inited && syslogd_sock >= 0)
    close (syslogd_sock);
  syslogd_inited = inited_failed;
  syslogd_sock = -1;
  sun.sun_family = AF_LOCAL;
  strncpy (sun.sun_path, _PATH_LOG, sizeof sun.sun_path);
  if (get_inet_addr_local ((struct sockaddr *) &sun, sizeof sun,
			   &sst, &len, &type))
    return;
  if ((fd = cygwin_socket (AF_LOCAL, type | SOCK_CLOEXEC, 0)) < 0)
    return;
  if (cygwin_connect (fd, (struct sockaddr *) &sun, sizeof sun) == 0)
    {
      if (type == SOCK_DGRAM)
	{
	  /*
	   * FIXME
	   *
	   * As soon as AF_LOCAL sockets are using pipes, this code has to
	   * got away.
	   */

	  /* connect on a dgram socket always succeeds.  We still don't know
	     if syslogd is actually listening. */
	  tmp_pathbuf tp;
	  PMIB_UDPTABLE tab = (PMIB_UDPTABLE) tp.w_get ();
	  DWORD size = 65536;
	  bool found = false;
	  struct sockaddr_in *sa = (struct sockaddr_in *) &sst;

	  if (GetUdpTable (tab, &size, FALSE) == NO_ERROR)
	    {
	      for (DWORD i = 0; i < tab->dwNumEntries; ++i)
		if (tab->table[i].dwLocalAddr == sa->sin_addr.s_addr
		    && tab->table[i].dwLocalPort == sa->sin_port)
		  {
		    found = true;
		    break;
		  }
	      if (!found)
		{
		  /* No syslogd is listening. */
		  close (fd);
		  return;
		}
	    }
	}
      syslogd_inited = type == SOCK_DGRAM ? inited_dgram : inited_stream;
    }
  else
    {
      close (fd);
      return;
    }
  syslogd_sock = fd;
  debug_printf ("found /dev/log, fd = %d, type = %s",
		fd, syslogd_inited == inited_stream ? "STREAM" : "DGRAM");
  return;
}

static int
try_connect_syslogd (int priority, const char *msg, size_t len)
{
  ssize_t ret = -1;

  try_connect_guard.init ("try_connect_guard")->acquire ();
  if (syslogd_inited == not_inited)
    connect_syslogd ();
  if (syslogd_inited != inited_failed)
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
      if (ret >= 0 || !(syslog_globals.process_logopt & LOG_CONS))
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
  debug_printf ("%y %s", priority, message);
  /* If the priority fails the current mask, reject */
  if ((LOG_MASK (LOG_PRI (priority)) & syslog_globals.process_logmask) == 0)
    {
      debug_printf ("failing message %y due to priority mask %y",
		    priority, syslog_globals.process_logmask);
      return;
    }

  /* Set default facility to LOG_USER if not yet set via openlog. */
  if (!syslog_globals.process_facility)
    syslog_globals.process_facility = LOG_USER;

  /* Add default facility if not in the given priority. */
  if (!(priority & LOG_FACMASK))
    priority |= syslog_globals.process_facility;

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
      if (syslog_globals.process_ident != NULL)
	{
	  if (pass.print ("%ls: ", syslog_globals.process_ident) == -1)
	    return;
	}
      if (syslog_globals.process_logopt & LOG_PID)
	{
	  if (pass.print ("PID %u: ", getpid ()) == -1)
	    return;
	}

      /* Print out the variable part */
      if (pass.print_va (message, ap) == -1)
	return;

    }
  char *total_msg = pass.get_message ();
  size_t len = strlen (total_msg);
  if (len != 0 && (total_msg[len - 1] == '\n'))
    total_msg[--len] = '\0';

  if (syslog_globals.process_logopt & LOG_PERROR)
    {
      write (STDERR_FILENO, total_msg, len);
      write (STDERR_FILENO, "\n", 1);
    }

  int fd;
  if ((fd = try_connect_syslogd (priority, total_msg, len + 1)) < 0)
    {
      /* If syslogd isn't present, open the event log and send the message */
      HANDLE hEventSrc;

      hEventSrc = RegisterEventSourceW (NULL, syslog_globals.process_ident
					      ?: CYGWIN_LOG_NAME);
      if (!hEventSrc)
	debug_printf ("RegisterEventSourceW, %E");
      else
	{
	  wchar_t *msg_strings[1];
	  tmp_pathbuf tp;
	  msg_strings[0] = tp.w_get ();
	  sys_mbstowcs (msg_strings[0], NT_MAX_PATH, total_msg);
	  if (!ReportEventW (hEventSrc, eventType, 0, 0, cygheap->user.sid (),
			     1, 0, (const wchar_t **) msg_strings, NULL))
	    debug_printf ("ReportEventW, %E");
	  DeregisterEventSource (hEventSrc);
	}
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
