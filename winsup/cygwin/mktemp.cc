/* mktemp.cc: mktemp functions

This file is adapted for Cygwin from FreeBSD.

See the copyright at the bottom of this file. */

#include "winsup.h"
#include "cygerrno.h"
#include <fcntl.h>
#include <unistd.h>

#define link_warning(symbol, msg) \
  asm (".stabs \"" msg "\",30,0,0,0\n\t" \
  ".stabs \"_" #symbol "\",1,0,0,0\n");


static int _gettemp(char *, int *, int);
static uint32_t arc4random ();

static const char padchar[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

extern "C" int
mkstemp(char *path)
{
  int fd;
  return _gettemp(path, &fd, 0) ? fd : -1;
}

extern "C" char *
mkdtemp(char *path)
{
  return _gettemp(path, NULL, 1) ? path : NULL;
}

link_warning (mktemp, "the use of `mktemp' is dangerous, better use `mkstemp'")
link_warning (_imp__mktemp, "the use of `mktemp' is dangerous, better use `mkstemp'")

extern "C" char *
mktemp(char *path)
{
  return _gettemp(path, NULL, 0) ? path : (char *) NULL;
}

static int
_gettemp(char *path, int *doopen, int domkdir)
{
  char *start, *trv, *suffp;
  char *pad;

  if (doopen && domkdir)
    {
      set_errno (EINVAL);
      return 0;
    }

  suffp = trv = strchr (path, '\0');
  if (--trv < path)
    {
      set_errno (EINVAL);
      return 0;
    }

  /* Fill space with random characters */
  while (trv >= path && *trv == 'X')
    {
      uint32_t rand = arc4random () % (sizeof (padchar) - 1);
      *trv-- = padchar[rand];
    }
  start = trv + 1;

  /*
   * check the target directory.
   */
  struct __stat64 sbuf;
  if (doopen != NULL || domkdir)
    {
      for (; trv > path; trv--)
	{
	  if (*trv == '/')
	    {
	      *trv = '\0';
	      int rval = stat64 (path, &sbuf);
	      *trv = '/';
	      if (rval != 0)
		return 0;
	      if (!S_ISDIR (sbuf.st_mode))
		{
		  set_errno (ENOTDIR);
		  return 0;
		}
	      break;
	    }
	}
    }

  for (;;)
    {
      if (doopen)
	{
	  if ((*doopen = open (path, O_CREAT | O_EXCL | O_RDWR | O_BINARY,
			       S_IRUSR | S_IWUSR)) >= 0)
	    return 1;
	  if (errno != EEXIST)
	    return 0;
	}
      else if (domkdir)
	{
	  if (mkdir (path, 0700) == 0)
	    return 1;
	  if (errno != EEXIST)
	    return 0;
	  }
      else if (lstat64 (path, &sbuf))
	return errno == ENOENT;

      /* If we have a collision, cycle through the space of filenames */
      for (trv = start;;)
	{
	  if (*trv == '\0' || trv == suffp)
	    return 0;
	  pad = strchr (padchar, *trv);
	  if (pad == NULL || *++pad == '\0')
	    *trv++ = padchar[0];
	  else
	    {
	      *trv++ = *pad;
	      break;
	    }
	}
    }
  /*NOTREACHED*/
}

static uint32_t
arc4random ()
{
  union
  {
    uint32_t rand;
    char buf[sizeof (int) / 8];
  } r;
  int fd = open ("/dev/urandom", O_RDONLY);
  read (fd, r.buf, 4);
  close (fd);
  return r.rand;
}

/*
* Copyright (c) 1987, 1993
*	The Regents of the University of California.  All rights reserved.
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
*	This product includes software developed by the University of
*	California, Berkeley and its contributors.
* 4. Neither the name of the University nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/
