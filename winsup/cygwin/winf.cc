/* winf.cc

   Copyright 2003, 2004, 2005, 2006 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "sync.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "winf.h"
#include "sys/cygwin.h"

void
linebuf::finish (bool cmdlenoverflow_ok)
{
  if (!ix)
    add ("", 1);
  else
    {
      if (ix-- > MAXCYGWINCMDLEN && cmdlenoverflow_ok)
	ix = MAXCYGWINCMDLEN - 1;
      buf[ix] = '\0';
    }
}

void
linebuf::add (const char *what, int len)
{
  size_t newix = ix + len;
  if (newix >= alloced || !buf)
    {
      alloced += LINE_BUF_CHUNK + newix;
      buf = (char *) realloc (buf, alloced + 1);
    }
  memcpy (buf + ix, what, len);
  ix = newix;
  buf[ix] = '\0';
}

void
linebuf::prepend (const char *what, int len)
{
  int buflen;
  size_t newix;
  if ((newix = ix + len) >= alloced)
    {
      alloced += LINE_BUF_CHUNK + newix;
      buf = (char *) realloc (buf, alloced + 1);
      buf[ix] = '\0';
    }
  if ((buflen = strlen (buf)))
      memmove (buf + len, buf, buflen + 1);
  else
      buf[newix] = '\0';
  memcpy (buf, what, len);
  ix = newix;
}

bool
linebuf::fromargv (av& newargv, char *real_path, bool cmdlenoverflow_ok)
{
  bool success = true;
  for (int i = 0; i < newargv.argc; i++)
    {
      char *p = NULL;
      const char *a;

      newargv.dup_maybe (i);
      a = i ? newargv[i] : (char *) real_path;
      int len = strlen (a);
      if (len != 0 && !strpbrk (a, " \t\n\r\""))
	add (a, len);
      else
	{
	  add ("\"", 1);
	  /* Handle embedded special characters " and \.
	     A " is always preceded by a \.
	     A \ is not special unless it precedes a ".  If it does,
	     then all preceding \'s must be doubled to avoid having
	     the Windows command line parser interpret the \ as quoting
	     the ".  This rule applies to a string of \'s before the end
	     of the string, since cygwin/windows uses a " to delimit the
	     argument. */
	  for (; (p = strpbrk (a, "\"\\")); a = ++p)
	    {
	      add (a, p - a);
	      /* Find length of string of backslashes */
	      int n = strspn (p, "\\");
	      if (!n)
		add ("\\\"", 2);	/* No backslashes, so it must be a ".
					       The " has to be protected with a backslash. */
	      else
		{
		  add (p, n);	/* Add the run of backslashes */
		  /* Need to double up all of the preceding
		     backslashes if they precede a quote or EOS. */
		  if (!p[n] || p[n] == '"')
		    add (p, n);
		  p += n - 1;		/* Point to last backslash */
		}
	    }
	  if (*a)
	    add (a);
	  add ("\"", 1);
	}
      add (" ", 1);
    }

  finish (cmdlenoverflow_ok);

  if (ix >= MAXWINCMDLEN)
    {
      debug_printf ("command line too long (>32K), return E2BIG");
      set_errno (E2BIG);
      success = false;
    }

  return success;
}

int
av::unshift (const char *what, int conv)
{
  char **av;
  av = (char **) crealloc (argv, (argc + 2) * sizeof (char *));
  if (!av)
    return 0;

  argv = av;
  memmove (argv + 1, argv, (argc + 1) * sizeof (char *));
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  if (conv)
    {
      cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_RELATIVE, what, buf,
			NT_MAX_PATH);
      char *p = strchr (buf, '\0') - 4;
      if (p > buf && ascii_strcasematch (p, ".exe"))
	*p = '\0';
      what = buf;
    }
  *argv = cstrdup1 (what);
  calloced++;
  argc++;
  return 1;
}
