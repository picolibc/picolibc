/* fhandler.cc.  See console.cc for fhandler_console functions.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
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
#include "sigproc.h"
#include "pinfo.h"
#include <assert.h>

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
fhandler_base::puts_readahead (const char *s, size_t len = (size_t) -1)
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
fhandler_base::set_readahead_valid (int val, int ch = -1)
{
  if (!val)
    ralen = raixget = raixput = 0;
  if (ch != -1)
    put_readahead(ch);
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
   be too long (e.g. devices or some such).  */
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
      win32_path_name = (char *) cmalloc (HEAP_STR, strlen(fmt) + 16);
      __small_sprintf (win32_path_name, fmt, unit);
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
    unix_path_name = cstrdup (unix_path);
  else
    {
      unix_path_name = cstrdup (win32_path_name);
      for (char *p = unix_path_name; (p = strchr (p, '\\')); p++)
	*p = '/';
    }

  if (unix_path_name == NULL)
    {
      system_printf ("fatal error. strdup failed");
      exit (ENOMEM);
    }
  namehash = hash_path_name (0, win32_path_name);
}

void
fhandler_base::reset_unix_path_name (const char *unix_path)
{
  cfree (unix_path_name);
  unix_path_name = cstrdup (unix_path);
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

/* Normal file i/o handlers.  */

/* Cover function to ReadFile to achieve (as much as possible) Posix style
   semantics and use of errno.  */
int
fhandler_base::raw_read (void *ptr, size_t ulen)
{
  DWORD bytes_read;

  if (!ReadFile (get_handle(), ptr, ulen, &bytes_read, 0))
    {
      int errcode;

      /* Some errors are not really errors.  Detect such cases here.  */

      errcode = GetLastError ();
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
	    return 0;
	case ERROR_INVALID_FUNCTION:
	case ERROR_INVALID_PARAMETER:
	  if (openflags & O_DIROPEN)
	    {
	      set_errno (EISDIR);
	      return -1;
	    }
	default:
	  syscall_printf ("ReadFile %s failed, %E", unix_path_name);
	  __seterrno_from_win_error (errcode);
	  return -1;
	  break;
	}
    }

  return bytes_read;
}

/* Cover function to WriteFile to provide Posix interface and semantics
   (as much as possible).  */
int
fhandler_base::raw_write (const void *ptr, size_t len)
{
  DWORD bytes_written;

  if (!WriteFile (get_handle(), ptr, len, &bytes_written, 0))
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
  if (perfile_table)
    {
      size_t nlen = strlen (get_name ());
      unsigned accflags = ACCFLAGS (flags);
      for (__cygwin_perfile *pf = perfile_table; pf->name; pf++)
	if (!*pf->name && ACCFLAGS (pf->flags) == accflags)
	  return pf->flags & ~(O_RDONLY | O_WRONLY | O_RDWR);
	else
	  {
	    size_t pflen = strlen (pf->name);
	    const char *stem = get_name () + nlen - pflen;
	    if (pflen > nlen || (stem != get_name () && !isdirsep (stem[-1])))
	      continue;
	    else if (ACCFLAGS (pf->flags) == accflags && strcasematch (stem, pf->name))
	      return pf->flags & ~(O_RDONLY | O_WRONLY | O_RDWR);
	  }
    }
  return __fmode;
}

/* Open system call handler function. */
int
fhandler_base::open (path_conv *, int flags, mode_t mode)
{
  int res = 0;
  HANDLE x;
  int file_attributes;
  int shared;
  int creation_distribution;
  SECURITY_ATTRIBUTES sa = sec_none;

  syscall_printf ("(%s, %p)", get_win32_name (), flags);

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
    set_append_p();

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
  if (get_query_open () &&
      isremote () &&
      creation_distribution == OPEN_EXISTING &&
      GetFileAttributes (get_win32_name ()) == (DWORD) -1)
    {
      set_errno (ENOENT);
      goto done;
    }

  /* If the file should actually be created and ntsec is on,
     set files attributes. */
  if (flags & O_CREAT && get_device () == FH_DISK && allow_ntsec && has_acls ())
    set_security_attribute (mode, &sa, alloca (4096), 4096);

  x = CreateFile (get_win32_name (), access, shared, &sa, creation_distribution,
		  file_attributes, 0);

  syscall_printf ("%p = CreateFileA (%s, %p, %p, %p, %p, %p, 0)",
		  x, get_win32_name (), access, shared, &sa,
		  creation_distribution, file_attributes);

  if (x == INVALID_HANDLE_VALUE)
    {
      if (GetLastError () == ERROR_INVALID_HANDLE)
	set_errno (ENOENT);
      else
	__seterrno ();
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
  int bin;
  int fmode;
  if ((bin = flags & (O_BINARY | O_TEXT)))
    /* nothing to do */;
  else if ((fmode = get_default_fmode (flags)) & O_BINARY)
    bin = O_BINARY;
  else if (fmode & O_TEXT)
    bin = O_TEXT;
  else if (get_device () == FH_DISK)
    bin = get_w_binary () || get_r_binary ();
  else
    bin = (binmode == O_BINARY) || get_w_binary () || get_r_binary ();

  if (bin & O_TEXT)
    bin = 0;

  set_flags (flags | (bin ? O_BINARY : O_TEXT));

  set_r_binary (bin);
  set_w_binary (bin);
  syscall_printf ("filemode set to %s", bin ? "binary" : "text");

  if (get_device () != FH_TAPE
      && get_device () != FH_FLOPPY
      && get_device () != FH_SERIAL)
    {
      if (flags & O_APPEND)
	SetFilePointer (get_handle(), 0, 0, FILE_END);
      else
	SetFilePointer (get_handle(), 0, 0, FILE_BEGIN);
    }

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
int
fhandler_base::read (void *in_ptr, size_t in_len)
{
  int len = (int) in_len;
  char *ptr = (char *) in_ptr;

  int c;
  int copied_chars = 0;

  while (len)
    if ((c = get_readahead ()) < 0)
      break;
    else
      {
	ptr[copied_chars++] = (unsigned char) (c & 0xff);
	len--;
      }

  if (copied_chars && is_slow ())
    return copied_chars;

  if (len)
    {
      int readlen = raw_read (ptr + copied_chars, len);
      if (copied_chars == 0)
	copied_chars = readlen;		/* Propagate error or EOF */
      else if (readlen > 0)		/* FIXME: should flag EOF for next read */
	copied_chars += readlen;
    }

  if (copied_chars <= 0)
    return copied_chars;
  if (get_r_binary ())
    {
      debug_printf ("returning %d chars, binary mode", copied_chars);
      return copied_chars;
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
  register char *src = (char *) ptr;
  register char *dst = (char *) ptr;
  register char *end = src + copied_chars - 1;

  /* Read up to the last but one char - the last char needs special handling */
  while (src < end)
    {
      *dst = *src++;
      if (*dst != '\r' || *src != '\n')
	dst++;
    }

  c = *src;
  /* if last char is a '\r' then read one more to see if we should
     translate this one too */
  if (c == '\r')
    {
      char c1 = 0;
      len = raw_read (&c1, 1);
      if (len <= 0)
	/* nothing */;
      else if (c1 == '\n')
	c = '\n';
      else
	set_readahead_valid (1, c1);
    }

  *dst++ = c;
  copied_chars = dst - (char *) ptr;

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

  debug_printf ("returning %d chars, text mode", copied_chars);
  return copied_chars;
}

int
fhandler_base::write (const void *ptr, size_t len)
{
  int res;

  if (get_append_p ())
    SetFilePointer (get_handle(), 0, 0, FILE_END);
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
	  memset(zeros, 0, 512);
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

off_t
fhandler_base::lseek (off_t offset, int whence)
{
  off_t res;

  /* Seeks on text files is tough, we rewind and read till we get to the
     right place.  */

  if (whence != SEEK_CUR || offset != 0)
    {
      if (whence == SEEK_CUR)
	offset -= ralen - raixget;
      set_readahead_valid (0);
    }

  debug_printf ("lseek (%s, %d, %d)", unix_path_name, offset, whence);

#if 0	/* lseek has no business messing about with text-mode stuff */

  if (!get_r_binary ())
    {
      int newplace;

      if (whence == 0)
	{
	  newplace = offset;
	}
      else if (whence ==1)
	{
	  newplace = rpos +  offset;
	}
      else
	{
	  /* Seek from the end of a file.. */
	  if (rsize == -1)
	    {
	      /* Find the size of the file by reading till the end */

	      char b[CHUNK_SIZE];
	      while (read (b, sizeof (b)) > 0)
		;
	      rsize = rpos;
	    }
	  newplace = rsize + offset;
	}

      if (rpos > newplace)
	{
	  SetFilePointer (handle, 0, 0, 0);
	  rpos = 0;
	}

      /* You can never shrink something more than 50% by turning CRLF into LF,
	 so we binary chop looking for the right place */

      while (rpos < newplace)
	{
	  char b[CHUNK_SIZE];
	  size_t span = (newplace - rpos) / 2;
	  if (span == 0)
	    span = 1;
	  if (span > sizeof (b))
	    span = sizeof (b);

	  debug_printf ("lseek (%s, %d, %d) span %d, rpos %d newplace %d",
		       name, offset, whence,span,rpos, newplace);
	  read (b, span);
	}

      debug_printf ("Returning %d", newplace);
      return newplace;
    }
#endif	/* end of deleted code dealing with text mode */

  DWORD win32_whence = whence == SEEK_SET ? FILE_BEGIN
		       : (whence == SEEK_CUR ? FILE_CURRENT : FILE_END);

  res = SetFilePointer (get_handle(), offset, 0, win32_whence);
  if (res == -1)
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

  syscall_printf ("closing '%s' handle %p", get_name (), get_handle());
  if (CloseHandle (get_handle()))
    res = 0;
  else
    {
      paranoid_printf ("CloseHandle (%d <%s>) failed", get_handle(),
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
  set_errno (ENOSYS);
  return -1;
}

extern "C" char * __stdcall
rootdir(char *full_path)
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
fhandler_base::fstat (struct stat *buf, path_conv *)
{
  switch (get_device ())
    {
    case FH_PIPEW:
      buf->st_mode = STD_WBITS | S_IWGRP | S_IWOTH;
      break;
    case FH_PIPER:
      buf->st_mode = STD_RBITS;
      break;
    default:
      buf->st_mode = STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;
      break;
    }

  buf->st_mode |= get_device () == FH_FLOPPY ? S_IFBLK : S_IFCHR;
  buf->st_nlink = 1;
  buf->st_blksize = S_BLKSIZE;
  buf->st_dev = buf->st_rdev = FHDEVN (get_device ()) << 8 | (get_unit () & 0xff);
  buf->st_ino = get_namehash ();
  buf->st_atime = buf->st_mtime = buf->st_ctime = time (NULL) - 1;
  return 0;
}

void
fhandler_base::init (HANDLE f, DWORD a, mode_t bin)
{
  set_io_handle (f);
  set_r_binary (bin);
  set_w_binary (bin);
  access = a;
  a &= GENERIC_READ | GENERIC_WRITE;
  if (a == GENERIC_READ)
    set_flags (O_RDONLY);
  if (a == GENERIC_WRITE)
    set_flags (O_WRONLY);
  if (a == (GENERIC_READ | GENERIC_WRITE))
    set_flags (O_RDWR);
  set_open_status ();
  debug_printf ("created new fhandler_base for handle %p", f);
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
  if (get_nohandle ())
    nh = NULL;
  else if (!DuplicateHandle (hMainProc, get_handle(), hMainProc, &nh, 0, TRUE,
			DUPLICATE_SAME_ACCESS))
    {
      system_printf ("dup(%s) failed, handle %x, %E",
		     get_name (), get_handle());
      __seterrno ();
      return -1;
    }

  child->set_io_handle (nh);
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
  open_status (0)
{
  int bin = __fmode & O_TEXT ? 0 : 1;
  if (status != FH_DISK && status != FH_CONSOLE)
    {
      if (!get_r_binset ())
	set_r_binary (bin);
      if (!get_w_binset ())
	set_w_binary (bin);
    }
}

/* Normal I/O destructor */
fhandler_base::~fhandler_base (void)
{
  if (unix_path_name != NULL)
    cfree (unix_path_name);
  if (win32_path_name != NULL)
    cfree (win32_path_name);
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
#ifdef DEBUGGING
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
#ifdef DEBUGGING
  if (h)
    setclexec_pid (oh, h, not_inheriting);
#endif
}

void
fhandler_base::fork_fixup (HANDLE parent, HANDLE &h, const char *name)
{
#ifdef DEBUGGING
  HANDLE oh = h;
#endif
  if (/* !is_socket () && */ !get_close_on_exec ())
    debug_printf ("handle %p already opened", h);
  else if (!DuplicateHandle (parent, h, hMainProc, &h, 0, !get_close_on_exec (),
			     DUPLICATE_SAME_ACCESS))
    system_printf ("%s - %E, handle %s<%p>", get_name (), name, h);
#ifdef DEBUGGING
  else
    {
      debug_printf ("%s success - oldh %p, h %p", get_name (), oh, h);
      // someday, maybe ProtectHandle2 (h, name);
      setclexec_pid (oh, h, !get_close_on_exec ());
    }
#endif
}

void
fhandler_base::set_close_on_exec (int val)
{
  set_inheritance (io_handle, val);
  set_close_on_exec_flag (val);
  debug_printf ("set close_on_exec for %s to %d", get_name (), val);
}

void
fhandler_base::fixup_after_fork (HANDLE parent)
{
  debug_printf ("inheriting '%s' from parent", get_name ());
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

off_t
fhandler_base::telldir (DIR *)
{
  set_errno (ENOTDIR);
  return -1;
}

void
fhandler_base::seekdir (DIR *, off_t)
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
