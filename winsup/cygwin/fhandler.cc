/* fhandler.cc.  See console.cc for fhandler_console functions.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <sys/uio.h>
#include <signal.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "cygwin/version.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "pinfo.h"
#include <assert.h>
#include <limits.h>

static NO_COPY const int CHUNK_SIZE = 1024; /* Used for crlf conversions */

struct __cygwin_perfile *perfile_table;

DWORD binmode;

inline fhandler_base&
fhandler_base::operator =(fhandler_base &x)
{
  memcpy (this, &x, sizeof *this);
  unix_path_name = x.unix_path_name ? cstrdup (x.unix_path_name) : NULL;
  win32_path_name = x.win32_path_name ? cstrdup (x.win32_path_name) : NULL;
  rabuf = NULL;
  ralen = 0;
  raixget = 0;
  raixput = 0;
  rabuflen = 0;
  return *this;
}

int
fhandler_base::puts_readahead (const char *s, size_t len)
{
  int success = 1;
  while ((*s || (len != (size_t) -1 && len--))
	 && (success = put_readahead (*s++) > 0))
    continue;
  return success;
}

int
fhandler_base::put_readahead (char value)
{
  char *newrabuf;
  if (raixput < rabuflen)
    /* Nothing to do */;
  else if ((newrabuf = (char *) realloc (rabuf, rabuflen += 32)))
    rabuf = newrabuf;
  else
    return 0;

  rabuf[raixput++] = value;
  ralen++;
  return 1;
}

int
fhandler_base::get_readahead ()
{
  int chret = -1;
  if (raixget < ralen)
    chret = ((unsigned char) rabuf[raixget++]) & 0xff;
  /* FIXME - not thread safe */
  if (raixget >= ralen)
    raixget = raixput = ralen = 0;
  return chret;
}

int
fhandler_base::peek_readahead (int queryput)
{
  int chret = -1;
  if (!queryput && raixget < ralen)
    chret = ((unsigned char) rabuf[raixget]) & 0xff;
  else if (queryput && raixput > 0)
    chret = ((unsigned char) rabuf[raixput - 1]) & 0xff;
  return chret;
}

void
fhandler_base::set_readahead_valid (int val, int ch)
{
  if (!val)
    ralen = raixget = raixput = 0;
  if (ch != -1)
    put_readahead (ch);
}

int
fhandler_base::eat_readahead (int n)
{
  int oralen = ralen;
  if (n < 0)
    n = ralen;
  if (n > 0 && ralen)
    {
      if ((int) (ralen -= n) < 0)
	ralen = 0;

      if (raixget >= ralen)
	raixget = raixput = ralen = 0;
      else if (raixput > ralen)
	raixput = ralen;
    }

  return oralen;
}

int
fhandler_base::get_readahead_into_buffer (char *buf, size_t buflen)
{
  int ch;
  int copied_chars = 0;

  while (buflen)
    if ((ch = get_readahead ()) < 0)
      break;
    else
      {
	buf[copied_chars++] = (unsigned char)(ch & 0xff);
	buflen--;
      }

  return copied_chars;
}

/* Record the file name.
   Filenames are used mostly for debugging messages, and it's hoped that
   in cases where the name is really required, the filename wouldn't ever
   be too long (e.g. devices or some such).
   The unix_path_name is also used by virtual fhandlers.  */
void
fhandler_base::set_name (const char *unix_path, const char *win32_path, int unit)
{
  if (unix_path == NULL || !*unix_path)
    return;

  if (win32_path)
    win32_path_name = cstrdup (win32_path);
  else
    {
      const char *fmt = get_native_name ();
      char *w =  (char *) cmalloc (HEAP_STR, strlen (fmt) + 16);
      __small_sprintf (w, fmt, unit);
      win32_path_name = w;
    }

  if (win32_path_name == NULL)
    {
      system_printf ("fatal error. strdup failed");
      exit (ENOMEM);
    }

  assert (unix_path_name == NULL);
  /* FIXME: This isn't really right.  It ignores the first argument if we're
     building names for a device and just converts the device name from the
     win32 name since it has theoretically been previously detected by
     path_conv. Ideally, we should pass in a format string and build the
     unix_path, too. */
  if (!is_device () || *win32_path_name != '\\')
    unix_path_name = unix_path;
  else
    {
      char *p = cstrdup (win32_path_name);
      unix_path_name = p;
      while ((p = strchr (p, '\\')) != NULL)
	*p++ = '/';
      if (unix_path)
	cfree ((void *) unix_path);
    }

  if (unix_path_name == NULL)
    {
      system_printf ("fatal error. strdup failed");
      exit (ENOMEM);
    }
  namehash = hash_path_name (0, win32_path_name);
}

/* Detect if we are sitting at EOF for conditions where Windows
   returns an error but UNIX doesn't.  */
static int __stdcall
is_at_eof (HANDLE h, DWORD err)
{
  DWORD size, upper1, curr;

  size = GetFileSize (h, &upper1);
  if (upper1 != 0xffffffff || GetLastError () == NO_ERROR)
    {
      LONG upper2 = 0;
      curr = SetFilePointer (h, 0, &upper2, FILE_CURRENT);
      if (curr == size && upper1 == (DWORD) upper2)
	return 1;
    }

  SetLastError (err);
  return 0;
}

void
fhandler_base::set_flags (int flags, int supplied_bin)
{
  int bin;
  int fmode;
  debug_printf ("flags %p, supplied_bin %p", flags, supplied_bin);
  if ((bin = flags & (O_BINARY | O_TEXT)))
    debug_printf ("O_TEXT/O_BINARY set in flags %p", bin);
  else if (get_r_binset () && get_w_binset ())
    bin = get_r_binary () ? O_BINARY : O_TEXT;	// FIXME: Not quite right
  else if ((fmode = get_default_fmode (flags)) & O_BINARY)
    bin = O_BINARY;
  else if (fmode & O_TEXT)
    bin = O_TEXT;
  else if (supplied_bin)
    bin = supplied_bin;
  else
    bin = get_w_binary () || get_r_binary () || (binmode != O_TEXT)
	  ? O_BINARY : O_TEXT;

  openflags = flags | bin;

  bin &= O_BINARY;
  set_r_binary (bin);
  set_w_binary (bin);
  syscall_printf ("filemode set to %s", bin ? "binary" : "text");
}

/* Normal file i/o handlers.  */

/* Cover function to ReadFile to achieve (as much as possible) Posix style
   semantics and use of errno.  */
void
fhandler_base::raw_read (void *ptr, size_t& ulen)
{
#define bytes_read ((ssize_t) ulen)

  DWORD len = ulen;
  (ssize_t) ulen = -1;
  if (read_state)
    SetEvent (read_state);
  BOOL res = ReadFile (get_handle (), ptr, len, (DWORD *) &ulen, 0);
  if (read_state)
    SetEvent (read_state);
  if (!res)
    {
      /* Some errors are not really errors.  Detect such cases here.  */

      DWORD  errcode = GetLastError ();
      switch (errcode)
	{
	case ERROR_BROKEN_PIPE:
	  /* This is really EOF.  */
	  bytes_read = 0;
	  break;
	case ERROR_MORE_DATA:
	  /* `bytes_read' is supposedly valid.  */
	  break;
	case ERROR_NOACCESS:
	  if (is_at_eof (get_handle (), errcode))
	    {
	      bytes_read = 0;
	      break;
	    }
	case ERROR_INVALID_FUNCTION:
	case ERROR_INVALID_PARAMETER:
	case ERROR_INVALID_HANDLE:
	  if (openflags & O_DIROPEN)
	    {
	      set_errno (EISDIR);
	      bytes_read = -1;
	      break;
	    }
	default:
	  syscall_printf ("ReadFile %s failed, %E", unix_path_name);
	  __seterrno_from_win_error (errcode);
	  bytes_read = -1;
	  break;
	}
    }
#undef bytes_read
}

/* Cover function to WriteFile to provide Posix interface and semantics
   (as much as possible).  */
int
fhandler_base::raw_write (const void *ptr, size_t len)
{
  DWORD bytes_written;

  if (!WriteFile (get_handle (), ptr, len, &bytes_written, 0))
    {
      if (GetLastError () == ERROR_DISK_FULL && bytes_written > 0)
	return bytes_written;
      __seterrno ();
      if (get_errno () == EPIPE)
	raise (SIGPIPE);
      return -1;
    }
  return bytes_written;
}

#define ACCFLAGS(x) (x & (O_RDONLY | O_WRONLY | O_RDWR))
int
fhandler_base::get_default_fmode (int flags)
{
  int fmode = __fmode;
  if (perfile_table)
    {
      size_t nlen = strlen (get_name ());
      unsigned accflags = ACCFLAGS (flags);
      for (__cygwin_perfile *pf = perfile_table; pf->name; pf++)
	if (!*pf->name && ACCFLAGS (pf->flags) == accflags)
	  {
	    fmode = pf->flags & ~(O_RDONLY | O_WRONLY | O_RDWR);
	    break;
	  }
	else
	  {
	    size_t pflen = strlen (pf->name);
	    const char *stem = get_name () + nlen - pflen;
	    if (pflen > nlen || (stem != get_name () && !isdirsep (stem[-1])))
	      continue;
	    else if (ACCFLAGS (pf->flags) == accflags && strcasematch (stem, pf->name))
	      {
		fmode = pf->flags & ~(O_RDONLY | O_WRONLY | O_RDWR);
		break;
	      }
	  }
    }
  return fmode;
}

/* Open system call handler function. */
int
fhandler_base::open (path_conv *pc, int flags, mode_t mode)
{
  int res = 0;
  HANDLE x;
  int file_attributes;
  int shared;
  int creation_distribution;
  SECURITY_ATTRIBUTES sa = sec_none;

  syscall_printf ("(%s, %p) query_open %d", get_win32_name (), flags, get_query_open ());

  if (get_win32_name () == NULL)
    {
      set_errno (ENOENT);
      goto done;
    }

  if (get_query_open ())
    access = 0;
  else if (get_device () == FH_TAPE)
    access = GENERIC_READ | GENERIC_WRITE;
  else if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_RDONLY)
    access = GENERIC_READ;
  else if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_WRONLY)
    access = GENERIC_WRITE;
  else
    access = GENERIC_READ | GENERIC_WRITE;

  /* Allow reliable lseek on disk devices. */
  if (get_device () == FH_FLOPPY)
    access |= GENERIC_READ;

  /* FIXME: O_EXCL handling?  */

  if ((flags & O_TRUNC) && ((flags & O_ACCMODE) != O_RDONLY))
    {
      if (flags & O_CREAT)
	creation_distribution = CREATE_ALWAYS;
      else
	creation_distribution = TRUNCATE_EXISTING;
    }
  else if (flags & O_CREAT)
    creation_distribution = OPEN_ALWAYS;
  else
    creation_distribution = OPEN_EXISTING;

  if ((flags & O_EXCL) && (flags & O_CREAT))
    creation_distribution = CREATE_NEW;

  if (flags & O_APPEND)
    set_append_p ();

  /* These flags are host dependent. */
  shared = wincap.shared ();

  file_attributes = FILE_ATTRIBUTE_NORMAL;
  if (flags & O_DIROPEN)
    file_attributes |= FILE_FLAG_BACKUP_SEMANTICS;
  if (get_device () == FH_SERIAL)
    file_attributes |= FILE_FLAG_OVERLAPPED;

#ifdef HIDDEN_DOT_FILES
  if (flags & O_CREAT && get_device () == FH_DISK)
    {
      char *c = strrchr (get_win32_name (), '\\');
      if ((c && c[1] == '.') || *get_win32_name () == '.')
	file_attributes |= FILE_ATTRIBUTE_HIDDEN;
    }
#endif

  /* CreateFile() with dwDesiredAccess == 0 when called on remote
     share returns some handle, even if file doesn't exist. This code
     works around this bug. */
  if (get_query_open () && isremote () &&
      creation_distribution == OPEN_EXISTING && pc && !pc->exists ())
    {
      set_errno (ENOENT);
      goto done;
    }

  /* If mode has no write bits set, we set the R/O attribute. */
  if (!(mode & (S_IWUSR | S_IWGRP | S_IWOTH)))
    file_attributes |= FILE_ATTRIBUTE_READONLY;

  /* If the file should actually be created and ntsec is on,
     set files attributes. */
  if (flags & O_CREAT && get_device () == FH_DISK && allow_ntsec && has_acls ())
    set_security_attribute (mode, &sa, alloca (4096), 4096);

  x = CreateFile (get_win32_name (), access, shared, &sa, creation_distribution,
		  file_attributes, 0);

  syscall_printf ("%p = CreateFile (%s, %p, %p, %p, %p, %p, 0)",
		  x, get_win32_name (), access, shared, &sa,
		  creation_distribution, file_attributes);

  if (x == INVALID_HANDLE_VALUE)
    {
      if (!wincap.can_open_directories () && pc && pc->isdir ())
	{
	  if (mode & (O_CREAT | O_EXCL) == (O_CREAT | O_EXCL))
	    set_errno (EEXIST);
	  else if (mode & (O_WRONLY | O_RDWR))
	    set_errno (EISDIR);
	  else
	    set_nohandle (true);
	}
      else if (GetLastError () == ERROR_INVALID_HANDLE)
	set_errno (ENOENT);
      else
	__seterrno ();
      if (!get_nohandle ())
	goto done;
    }

  /* Attributes may be set only if a file is _really_ created.
     This code is now only used for ntea here since the files
     security attributes are set in CreateFile () now. */
  if (flags & O_CREAT && get_device () == FH_DISK
      && GetLastError () != ERROR_ALREADY_EXISTS
      && !allow_ntsec && allow_ntea)
    set_file_attribute (has_acls (), get_win32_name (), mode);

  set_io_handle (x);
  set_flags (flags, pc ? pc->binmode () : 0);

  res = 1;
  set_open_status ();
done:
  syscall_printf ("%d = fhandler_base::open (%s, %p)", res, get_win32_name (),
		  flags);
  return res;
}

/* states:
   open buffer in binary mode?  Just do the read.

   open buffer in text mode?  Scan buffer for control zs and handle
   the first one found.  Then scan buffer, converting every \r\n into
   an \n.  If last char is an \r, look ahead one more char, if \n then
   modify \r, if not, remember char.
*/
void
fhandler_base::read (void *in_ptr, size_t& len)
{
  size_t in_len = len;
  char *ptr = (char *) in_ptr;
  ssize_t copied_chars = 0;
  int c;

  while (len)
    if ((c = get_readahead ()) < 0)
      break;
    else
      {
	ptr[copied_chars++] = (unsigned char) (c & 0xff);
	len--;
      }

  if (copied_chars && is_slow ())
    {
      len = (size_t) copied_chars;
      return;
    }

  if (len)
    {
      raw_read (ptr + copied_chars, len);
      if (!copied_chars)
	/* nothing */;
      else if ((ssize_t) len > 0)
	len += copied_chars;
      else
	len = copied_chars;
    }
  else if (copied_chars <= 0)
    {
      len = (size_t) copied_chars;
      return;
    }

  if (get_r_binary ())
    {
      debug_printf ("returning %d chars, binary mode", len);
      return;
    }

#if 0
  char *ctrlzpos;
  /* Scan buffer for a control-z and shorten the buffer to that length */

  ctrlzpos = (char *) memchr ((char *) ptr, 0x1a, copied_chars);
  if (ctrlzpos)
    {
      lseek ((ctrlzpos - ((char *) ptr + copied_chars)), SEEK_CUR);
      copied_chars = ctrlzpos - (char *) ptr;
    }

  if (copied_chars == 0)
    {
      debug_printf ("returning 0 chars, text mode, CTRL-Z found");
      return 0;
    }
#endif

  /* Scan buffer and turn \r\n into \n */
  char *src = (char *) ptr;
  char *dst = (char *) ptr;
  char *end = src + len - 1;

  /* Read up to the last but one char - the last char needs special handling */
  while (src < end)
    {
      if (*src == '\r' && src[1] == '\n')
	src++;
      *dst++ = *src++;
    }

  len = dst - (char *) ptr;

  /* if last char is a '\r' then read one more to see if we should
     translate this one too */
  if (len < in_len && *src == '\r')
    {
      size_t clen = 1;
      raw_read (&c, clen);
      if (clen <= 0)
	/* nothing */;
      else if (c != '\n')
	set_readahead_valid (1, c);
      else
	{
	  *dst++ = '\n';
	  len++;
	}
    }


#ifndef NOSTRACE
  if (strace.active)
    {
      char buf[16 * 6 + 1];
      char *p = buf;

      for (int i = 0; i < copied_chars && i < 16; ++i)
	{
	  unsigned char c = ((unsigned char *) ptr)[i];
	  /* >= 33 so space prints in hex */
	  __small_sprintf (p, c >= 33 && c <= 127 ? " %c" : " %p", c);
	  p += strlen (p);
	}
      debug_printf ("read %d bytes (%s%s)", copied_chars, buf,
		    copied_chars > 16 ? " ..." : "");
    }
#endif

  debug_printf ("returning %d chars, text mode", len);
  return;
}

int
fhandler_base::write (const void *ptr, size_t len)
{
  int res;

  if (get_append_p ())
    SetFilePointer (get_handle (), 0, 0, FILE_END);
  else if (wincap.has_lseek_bug () && get_check_win95_lseek_bug ())
    {
      /* Note: this bug doesn't happen on NT4, even though the documentation
	 for WriteFile() says that it *may* happen on any OS. */
      int actual_length, current_position;
      set_check_win95_lseek_bug (0); /* don't do it again */
      actual_length = GetFileSize (get_handle (), NULL);
      current_position = SetFilePointer (get_handle (), 0, 0, FILE_CURRENT);
      if (current_position > actual_length)
	{
	  /* Oops, this is the bug case - Win95 uses whatever is on the disk
	     instead of some known (safe) value, so we must seek back and
	     fill in the gap with zeros. - DJ */
	  char zeros[512];
	  int number_of_zeros_to_write = current_position - actual_length;
	  memset (zeros, 0, 512);
	  SetFilePointer (get_handle (), 0, 0, FILE_END);
	  while (number_of_zeros_to_write > 0)
	    {
	      DWORD zeros_this_time = (number_of_zeros_to_write > 512
				     ? 512 : number_of_zeros_to_write);
	      DWORD written;
	      if (!WriteFile (get_handle (), zeros, zeros_this_time, &written,
			      NULL))
		{
		  __seterrno ();
		  if (get_errno () == EPIPE)
		    raise (SIGPIPE);
		  /* This might fail, but it's the best we can hope for */
		  SetFilePointer (get_handle (), current_position, 0, FILE_BEGIN);
		  return -1;

		}
	      if (written < zeros_this_time) /* just in case */
		{
		  set_errno (ENOSPC);
		  /* This might fail, but it's the best we can hope for */
		  SetFilePointer (get_handle (), current_position, 0, FILE_BEGIN);
		  return -1;
		}
	      number_of_zeros_to_write -= written;
	    }
	}
    }

  if (get_w_binary ())
    {
      debug_printf ("binary write");
      res = raw_write (ptr, len);
    }
  else
    {
      debug_printf ("text write");
      /* This is the Microsoft/DJGPP way.  Still not ideal, but it's
	 compatible.
	 Modified slightly by CGF 2000-10-07 */

      int left_in_data = len;
      char *data = (char *)ptr;
      res = 0;

      while (left_in_data > 0)
	{
	  char buf[CHUNK_SIZE + 1], *buf_ptr = buf;
	  int left_in_buf = CHUNK_SIZE;

	  while (left_in_buf > 0 && left_in_data > 0)
	    {
	      char ch = *data++;
	      if (ch == '\n')
		{
		  *buf_ptr++ = '\r';
		  left_in_buf--;
		}
	      *buf_ptr++ = ch;
	      left_in_buf--;
	      left_in_data--;
	      if (left_in_data > 0 && ch == '\r' && *data == '\n')
		{
		  *buf_ptr++ = *data++;
		  left_in_buf--;
		  left_in_data--;
		}
	    }

	  /* We've got a buffer-full, or we're out of data.  Write it out */
	  int nbytes;
	  int want = buf_ptr - buf;
	  if ((nbytes = raw_write (buf, want)) == want)
	    {
	      /* Keep track of how much written not counting additional \r's */
	      res = data - (char *)ptr;
	      continue;
	    }

	  if (nbytes == -1)
	    res = -1;		/* Error */
	  else
	    res += nbytes;	/* Partial write.  Return total bytes written. */
	  break;		/* All done */
	}
    }

  debug_printf ("%d = write (%p, %d)", res, ptr, len);
  return res;
}

ssize_t
fhandler_base::readv (const struct iovec *const iov, const int iovcnt,
		      ssize_t tot)
{
  assert (iov);
  assert (iovcnt >= 1);

  if (iovcnt == 1)
    {
      size_t len = iov->iov_len;
      read (iov->iov_base, len);
      return len;
    }

  if (tot == -1)		// i.e. if not pre-calculated by the caller.
    {
      tot = 0;
      const struct iovec *iovptr = iov + iovcnt;
      do
	{
	  iovptr -= 1;
	  tot += iovptr->iov_len;
	}
      while (iovptr != iov);
    }

  assert (tot >= 0);

  if (tot == 0)
    return 0;

  char *buf = (char *) alloca (tot);

  if (!buf)
    {
      set_errno (ENOMEM);
      return -1;
    }

  read (buf, (size_t) tot);

  const struct iovec *iovptr = iov;
  int nbytes = tot;

  while (nbytes > 0)
    {
      const int frag = min (nbytes, (ssize_t) iovptr->iov_len);
      memcpy (iovptr->iov_base, buf, frag);
      buf += frag;
      iovptr += 1;
      nbytes -= frag;
    }

  return tot;
}

ssize_t
fhandler_base::writev (const struct iovec *const iov, const int iovcnt,
		       ssize_t tot)
{
  assert (iov);
  assert (iovcnt >= 1);

  if (iovcnt == 1)
    return write (iov->iov_base, iov->iov_len);

  if (tot == -1)		// i.e. if not pre-calculated by the caller.
    {
      tot = 0;
      const struct iovec *iovptr = iov + iovcnt;
      do
	{
	  iovptr -= 1;
	  tot += iovptr->iov_len;
	}
      while (iovptr != iov);
    }

  assert (tot >= 0);

  if (tot == 0)
    return 0;

  char *const buf = (char *) alloca (tot);

  if (!buf)
    {
      set_errno (ENOMEM);
      return -1;
    }

  char *bufptr = buf;
  const struct iovec *iovptr = iov;
  int nbytes = tot;

  while (nbytes != 0)
    {
      const int frag = min (nbytes, (ssize_t) iovptr->iov_len);
      memcpy (bufptr, iovptr->iov_base, frag);
      bufptr += frag;
      iovptr += 1;
      nbytes -= frag;
    }

  return write (buf, tot);
}

__off64_t
fhandler_base::lseek (__off64_t offset, int whence)
{
  __off64_t res;

  /* 9x/Me doesn't support 64bit offsets.  We trap that here and return
     EINVAL.  It doesn't make sense to simulate bigger offsets by a
     SetFilePointer sequence since FAT and FAT32 don't support file
     size >= 4GB anyway. */
  if (!wincap.has_64bit_file_access ()
      && (offset < LONG_MIN || offset > LONG_MAX))
    {
      debug_printf ("Win9x, offset not 32 bit.");
      set_errno (EINVAL);
      return (__off64_t)-1;
    }

  /* Seeks on text files is tough, we rewind and read till we get to the
     right place.  */

  if (whence != SEEK_CUR || offset != 0)
    {
      if (whence == SEEK_CUR)
	offset -= ralen - raixget;
      set_readahead_valid (0);
    }

  debug_printf ("lseek (%s, %D, %d)", unix_path_name, offset, whence);

  DWORD win32_whence = whence == SEEK_SET ? FILE_BEGIN
		       : (whence == SEEK_CUR ? FILE_CURRENT : FILE_END);

  LONG off_low = offset & 0xffffffff;
  LONG *poff_high, off_high;
  if (!wincap.has_64bit_file_access ())
    poff_high = NULL;
  else
    {
      off_high =  offset >> 32;
      poff_high = &off_high;
    }

  debug_printf ("setting file pointer to %u (high), %u (low)", off_high, off_low);
  res = SetFilePointer (get_handle (), off_low, poff_high, win32_whence);
  if (res == INVALID_SET_FILE_POINTER && GetLastError ())
    {
      __seterrno ();
    }
  else
    {
      /* When next we write(), we will check to see if *this* seek went beyond
	 the end of the file, and back-seek and fill with zeros if so - DJ */
      set_check_win95_lseek_bug ();

      /* If this was a SEEK_CUR with offset 0, we still might have
	 readahead that we have to take into account when calculating
	 the actual position for the application.  */
      if (whence == SEEK_CUR)
	res -= ralen - raixget;
    }

  return res;
}

int
fhandler_base::close ()
{
  int res = -1;

  syscall_printf ("closing '%s' handle %p", get_name (), get_handle ());
  if (get_nohandle () || CloseHandle (get_handle ()))
    res = 0;
  else
    {
      paranoid_printf ("CloseHandle (%d <%s>) failed", get_handle (),
		       get_name ());

      __seterrno ();
    }
  return res;
}

int
fhandler_base::ioctl (unsigned int cmd, void *buf)
{
  if (cmd == FIONBIO)
    syscall_printf ("ioctl (FIONBIO, %p)", buf);
  else
    syscall_printf ("ioctl (%x, %p)", cmd, buf);

  set_errno (EINVAL);
  return -1;
}

int
fhandler_base::lock (int, struct flock *)
{
  set_errno (EINVAL);
  return -1;
}

extern "C" char * __stdcall
rootdir (char *full_path)
{
  /* Possible choices:
   * d:... -> d:/
   * \\server\share... -> \\server\share\
   * else current drive.
   */
  char *root = full_path;

  if (full_path[1] == ':')
    strcpy (full_path + 2, "\\");
  else if (full_path[0] == '\\' && full_path[1] == '\\')
    {
      char *cp = full_path + 2;
      while (*cp && *cp != '\\')
	cp++;
      if (!*cp)
	{
	  set_errno (ENOTDIR);
	  return NULL;
	}
      cp++;
      while (*cp && *cp != '\\')
	cp++;
      strcpy (cp, "\\");
    }
  else
    root = NULL;

  return root;
}

int __stdcall
fhandler_base::fstat (struct __stat64 *buf, path_conv *)
{
  debug_printf ("here");
  switch (get_device ())
    {
    case FH_PIPE:
      buf->st_mode = S_IFIFO | STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;
      break;
    case FH_PIPEW:
      buf->st_mode = S_IFIFO | STD_WBITS | S_IWGRP | S_IWOTH;
      break;
    case FH_PIPER:
      buf->st_mode = S_IFIFO | STD_RBITS;
      break;
    case FH_FLOPPY:
      buf->st_mode = S_IFBLK | STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;
      break;
    default:
      buf->st_mode = S_IFCHR | STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;
      break;
    }

  buf->st_nlink = 1;
  buf->st_blksize = S_BLKSIZE;
  time_as_timestruc_t (&buf->st_ctim);
  buf->st_atim = buf->st_mtim = buf->st_ctim;
  return 0;
}

void
fhandler_base::init (HANDLE f, DWORD a, mode_t bin)
{
  set_io_handle (f);
  access = a;
  a &= GENERIC_READ | GENERIC_WRITE;
  int flags = 0;
  if (a == GENERIC_READ)
    flags = O_RDONLY;
  else if (a == GENERIC_WRITE)
    flags = O_WRONLY;
  else if (a == (GENERIC_READ | GENERIC_WRITE))
    flags = O_RDWR;
  set_flags (flags | bin);
  set_open_status ();
  debug_printf ("created new fhandler_base for handle %p, bin %d", f, get_r_binary ());
}

void
fhandler_base::dump (void)
{
  paranoid_printf ("here");
}

int
fhandler_base::dup (fhandler_base *child)
{
  debug_printf ("in fhandler_base dup");

  HANDLE nh;
  if (!get_nohandle ())
    {
      if (!DuplicateHandle (hMainProc, get_handle (), hMainProc, &nh, 0, TRUE,
			    DUPLICATE_SAME_ACCESS))
	{
	  system_printf ("dup(%s) failed, handle %x, %E",
			 get_name (), get_handle ());
	  __seterrno ();
	  return -1;
	}

      child->set_io_handle (nh);
    }
  return 0;
}

int fhandler_base::fcntl (int cmd, void *arg)
{
  int res;

  switch (cmd)
    {
    case F_GETFD:
      res = get_close_on_exec () ? FD_CLOEXEC : 0;
      break;
    case F_SETFD:
      set_close_on_exec ((int) arg);
      res = 0;
      break;
    case F_GETFL:
      res = get_flags ();
      debug_printf ("GETFL: %d", res);
      break;
    case F_SETFL:
      {
	/*
	 * Only O_APPEND, O_ASYNC and O_NONBLOCK/O_NDELAY are allowed.
	 * Each other flag will be ignored.
	 * Since O_ASYNC isn't defined in fcntl.h it's currently
	 * ignored as well.
	 */
	const int allowed_flags = O_APPEND | O_NONBLOCK_MASK;
	int new_flags = (int) arg & allowed_flags;
	/* Carefully test for the O_NONBLOCK or deprecated OLD_O_NDELAY flag.
	   Set only the flag that has been passed in.  If both are set, just
	   record O_NONBLOCK.   */
	if ((new_flags & OLD_O_NDELAY) && (new_flags & O_NONBLOCK))
	  new_flags = O_NONBLOCK;
	set_flags ((get_flags () & ~allowed_flags) | new_flags);
      }
      res = 0;
      break;
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      res = lock (cmd, (struct flock *) arg);
      break;
    default:
      set_errno (EINVAL);
      res = -1;
      break;
    }
  return res;
}

/* Base terminal handlers.  These just return errors.  */

int
fhandler_base::tcflush (int)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcsendbreak (int)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcdrain (void)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcflow (int)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcsetattr (int, const struct termios *)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcgetattr (struct termios *)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcsetpgrp (const pid_t)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcgetpgrp (void)
{
  set_errno (ENOTTY);
  return -1;
}

void
fhandler_base::operator delete (void *p)
{
  cfree (p);
  return;
}

/* Normal I/O constructor */
fhandler_base::fhandler_base (DWORD devtype, int unit):
  status (devtype),
  access (0),
  io_handle (NULL),
  namehash (0),
  openflags (0),
  rabuf (NULL),
  ralen (0),
  raixget (0),
  raixput (0),
  rabuflen (0),
  unix_path_name (NULL),
  win32_path_name (NULL),
  open_status (0),
  read_state (NULL)
{
}

/* Normal I/O destructor */
fhandler_base::~fhandler_base (void)
{
  if (unix_path_name != NULL)
    cfree ((void *) unix_path_name);
  if (win32_path_name != NULL)
    cfree ((void *) win32_path_name);
  if (rabuf)
    free (rabuf);
  unix_path_name = win32_path_name = NULL;
}

/**********************************************************************/
/* /dev/null */

fhandler_dev_null::fhandler_dev_null () :
	fhandler_base (FH_NULL)
{
}

void
fhandler_dev_null::dump (void)
{
  paranoid_printf ("here");
}

void
fhandler_base::set_inheritance (HANDLE &h, int not_inheriting)
{
#ifdef DEBUGGING_AND_FDS_PROTECTED
  HANDLE oh = h;
#endif
  /* Note that we could use SetHandleInformation here but it is not available
     on all platforms.  Test cases seem to indicate that using DuplicateHandle
     in this fashion does not actually close the original handle, which is
     what we want.  If this changes in the future, we may be forced to use
     SetHandleInformation on newer OS's */
  if (!DuplicateHandle (hMainProc, h, hMainProc, &h, 0, !not_inheriting,
			     DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
    debug_printf ("DuplicateHandle failed, %E");
#ifdef DEBUGGING_AND_FDS_PROTECTED
  if (h)
    setclexec (oh, h, not_inheriting);
#endif
}

void
fhandler_base::fork_fixup (HANDLE parent, HANDLE &h, const char *name)
{
  if (/* !is_socket () && */ !get_close_on_exec ())
    debug_printf ("handle %p already opened", h);
  else if (!DuplicateHandle (parent, h, hMainProc, &h, 0, !get_close_on_exec (),
			     DUPLICATE_SAME_ACCESS))
    system_printf ("%s - %E, handle %s<%p>", get_name (), name, h);
#ifdef DEBUGGING_AND_FDS_PROTECTED
  else if (get_close_on_exec ())
    ProtectHandle (h);	/* would have to be fancier than this */
  else
    /* ProtectHandleINH (h) */;	/* Should already be protected */
#endif
}

void
fhandler_base::set_close_on_exec (int val)
{
  if (!get_nohandle ())
    set_inheritance (io_handle, val);
  set_close_on_exec_flag (val);
  debug_printf ("set close_on_exec for %s to %d", get_name (), val);
}

void
fhandler_base::fixup_after_fork (HANDLE parent)
{
  debug_printf ("inheriting '%s' from parent", get_name ());
  if (!get_nohandle ())
    fork_fixup (parent, io_handle, "io_handle");
}

bool
fhandler_base::is_nonblocking ()
{
  return (openflags & O_NONBLOCK_MASK) != 0;
}

void
fhandler_base::set_nonblocking (int yes)
{
  int current = openflags & O_NONBLOCK_MASK;
  int new_flags = yes ? (!current ? O_NONBLOCK : current) : 0;
  openflags = (openflags & ~O_NONBLOCK_MASK) | new_flags;
}

DIR *
fhandler_base::opendir (path_conv&)
{
  set_errno (ENOTDIR);
  return NULL;
}

struct dirent *
fhandler_base::readdir (DIR *)
{
  set_errno (ENOTDIR);
  return NULL;
}

__off64_t
fhandler_base::telldir (DIR *)
{
  set_errno (ENOTDIR);
  return -1;
}

void
fhandler_base::seekdir (DIR *, __off64_t)
{
  set_errno (ENOTDIR);
  return;
}

void
fhandler_base::rewinddir (DIR *)
{
  set_errno (ENOTDIR);
  return;
}

int
fhandler_base::closedir (DIR *)
{
  set_errno (ENOTDIR);
  return -1;
}
