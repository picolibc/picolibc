/* fhandler.cc.  See console.cc for fhandler_console functions.

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "cygheap.h"
#include <sys/cygwin.h>
#include <signal.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "path.h"
#include "shared_info.h"
#include "host_dependent.h"
#include "perprocess.h"
#include "security.h"

static NO_COPY const int CHUNK_SIZE = 1024; /* Used for crlf conversions */

static char fhandler_disk_dummy_name[] = "some disk file";

struct __cygwin_perfile *perfile_table = NULL;

DWORD binmode = 0;

inline fhandler_base&
fhandler_base::operator =(fhandler_base &x)
{
  memcpy (this, &x, sizeof *this);
  unix_path_name_ = x.unix_path_name_ ? cstrdup (x.unix_path_name_) : NULL;
  win32_path_name_ = x.win32_path_name_ ? cstrdup (x.win32_path_name_) : NULL;
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
    chret = ((unsigned char)rabuf[raixget++]) & 0xff;
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
   be too long (e.g. devices or some such).
*/

void
fhandler_base::set_name (const char *unix_path, const char *win32_path, int unit)
{
  if (!no_free_names ())
    {
      if (unix_path_name_ != NULL && unix_path_name_ != fhandler_disk_dummy_name)
	cfree (unix_path_name_);
      if (win32_path_name_ != NULL && unix_path_name_ != fhandler_disk_dummy_name)
	cfree (win32_path_name_);
    }

  unix_path_name_ = win32_path_name_ = NULL;
  if (unix_path == NULL || !*unix_path)
    return;

  unix_path_name_ = cstrdup (unix_path);
  if (unix_path_name_ == NULL)
    {
      system_printf ("fatal error. strdup failed");
      exit (ENOMEM);
    }

  if (win32_path)
    win32_path_name_ = cstrdup (win32_path);
  else
    {
      const char *fmt = get_native_name ();
      win32_path_name_ = (char *) cmalloc (HEAP_STR, strlen(fmt) + 16);
      __small_sprintf (win32_path_name_, fmt, unit);
    }

  if (win32_path_name_ == NULL)
    {
      system_printf ("fatal error. strdup failed");
      exit (ENOMEM);
    }
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
	default:
	  syscall_printf ("ReadFile %s failed, %E", unix_path_name_);
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

/* Open system call handler function.
   Path is now already checked for symlinks */
int
fhandler_base::open (int flags, mode_t mode)
{
  int res = 0;
  HANDLE x;
  int file_attributes;
  int shared;
  int creation_distribution;

  syscall_printf ("(%s, %p)", get_win32_name (), flags);

  if (get_win32_name () == NULL)
    {
      set_errno (ENOENT);
      goto done;
    }

  if (get_device () == FH_TAPE)
    {
      access_ = GENERIC_READ | GENERIC_WRITE;
    }
  else if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_RDONLY)
    {
      access_ = GENERIC_READ;
    }
  else if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_WRONLY)
    {
      access_ = GENERIC_WRITE;
    }
  else
    {
      access_ = GENERIC_READ | GENERIC_WRITE;
    }

  /* FIXME: O_EXCL handling?  */

  if ((flags & O_TRUNC) && ((flags & O_ACCMODE) != O_RDONLY))
    {
      if (flags & O_CREAT)
	{
	  creation_distribution = CREATE_ALWAYS;
	}
      else
	{
	  creation_distribution = TRUNCATE_EXISTING;
	}
    }
  else if (flags & O_CREAT)
    creation_distribution = OPEN_ALWAYS;
  else
    creation_distribution = OPEN_EXISTING;

  if ((flags & O_EXCL) && (flags & O_CREAT))
    {
      creation_distribution = CREATE_NEW;
    }

  if (flags & O_APPEND)
    set_append_p();

  /* These flags are host dependent. */
  shared = host_dependent.shared;

  file_attributes = FILE_ATTRIBUTE_NORMAL;
  if (flags & O_DIROPEN)
    file_attributes |= FILE_FLAG_BACKUP_SEMANTICS;
  if (get_device () == FH_SERIAL)
    file_attributes |= FILE_FLAG_OVERLAPPED;

  x = CreateFileA (get_win32_name (), access_, shared,
		   &sec_none, creation_distribution,
		   file_attributes,
		   0);

  syscall_printf ("%d = CreateFileA (%s, %p, %p, %p, %p, %p, 0)",
		  x,
		  get_win32_name (), access_, shared,
		  &sec_none, creation_distribution,
		  file_attributes);

  if (x == INVALID_HANDLE_VALUE)
    {
      if (GetLastError () == ERROR_INVALID_HANDLE)
	set_errno (ENOENT);
      else
	__seterrno ();
      goto done;
    }

  // Attributes may be set only if a file is _really_ created.
  if (flags & O_CREAT && get_device () == FH_DISK
      && GetLastError () != ERROR_ALREADY_EXISTS)
    set_file_attribute (has_acls (), get_win32_name (), mode);

  namehash_ = hash_path_name (0, get_win32_name ());
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
  char *ctrlzpos;
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
  else if (os_being_run != winNT && get_check_win95_lseek_bug ())
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

  debug_printf ("lseek (%s, %d, %d)", unix_path_name_, offset, whence);

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
fhandler_base::close (void)
{
  int res = -1;

  syscall_printf ("handle %p", get_handle());
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

int
fhandler_disk_file::fstat (struct stat *buf)
{
  int res = 0;	// avoid a compiler warning
  BY_HANDLE_FILE_INFORMATION local;
  save_errno saved_errno;

  memset (buf, 0, sizeof (*buf));

  if (is_device ())
    return stat_dev (get_device (), get_unit (), get_namehash (), buf);

  /* NT 3.51 seems to have a bug when attempting to get vol serial
     numbers.  This loop gets around this. */
  for (int i = 0; i < 2; i++)
    {
      if (!(res = GetFileInformationByHandle (get_handle (), &local)))
	break;
      if (local.dwVolumeSerialNumber && (long) local.dwVolumeSerialNumber != -1)
	break;
    }
  debug_printf ("%d = GetFileInformationByHandle (%s, %d)",
		res, get_win32_name (), get_handle ());
  if (res == 0)
    {
      /* GetFileInformationByHandle will fail if it's given stdin/out/err
	 or a pipe*/
      DWORD lsize, hsize;

      if (GetFileType (get_handle ()) != FILE_TYPE_DISK)
	buf->st_mode = S_IFCHR;

      lsize = GetFileSize (get_handle (), &hsize);
      if (lsize == 0xffffffff && GetLastError () != NO_ERROR)
	buf->st_mode = S_IFCHR;
      else
	buf->st_size = lsize;
      /* We expect these to fail! */
      buf->st_mode |= STD_RBITS | STD_WBITS;
      buf->st_blksize = S_BLKSIZE;
      buf->st_ino = get_namehash ();
      syscall_printf ("0 = fstat (, %p)",  buf);
      return 0;
    }

  if (!get_win32_name ())
    {
      saved_errno.set (ENOENT);
      return -1;
    }

  buf->st_atime   = to_time_t (&local.ftLastAccessTime);
  buf->st_mtime   = to_time_t (&local.ftLastWriteTime);
  buf->st_ctime   = to_time_t (&local.ftCreationTime);
  buf->st_nlink   = local.nNumberOfLinks;
  buf->st_dev     = local.dwVolumeSerialNumber;
  buf->st_size    = local.nFileSizeLow;

  /* This is for FAT filesystems, which don't support atime/ctime */
  if (buf->st_atime == 0)
    buf->st_atime = buf->st_mtime;
  if (buf->st_ctime == 0)
    buf->st_ctime = buf->st_mtime;

  /* Allocate some place to determine the root directory. Need to allocate
     enough so that rootdir can add a trailing slash if path starts with \\. */
  char root[strlen (get_win32_name ()) + 3];
  strcpy (root, get_win32_name ());

  /* Assume that if a drive has ACL support it MAY have valid "inodes".
     It definitely does not have valid inodes if it does not have ACL
     support. */
  switch (has_acls () ? GetDriveType (rootdir (root)) : DRIVE_UNKNOWN)
    {
    case DRIVE_FIXED:
    case DRIVE_REMOVABLE:
    case DRIVE_CDROM:
    case DRIVE_RAMDISK:
      /* Although the documentation indicates otherwise, it seems like
	 "inodes" on these devices are persistent, at least across reboots. */
      buf->st_ino = local.nFileIndexHigh | local.nFileIndexLow;
      break;
    default:
      /* Either the nFileIndex* fields are unreliable or unavailable.  Use the
	 next best alternative. */
      buf->st_ino = get_namehash ();
      break;
    }

  buf->st_blksize = S_BLKSIZE;
  buf->st_blocks  = ((unsigned long) buf->st_size + S_BLKSIZE-1) / S_BLKSIZE;

  /* Using a side effect: get_file_attibutes checks for
     directory. This is used, to set S_ISVTX, if needed.  */
  if (local.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    buf->st_mode |= S_IFDIR;
  if (!get_file_attribute (has_acls (),
			   get_win32_name (),
			   &buf->st_mode,
			   &buf->st_uid,
			   &buf->st_gid))
    {
      /* If read-only attribute is set, modify ntsec return value */
      if (local.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
	buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);

      buf->st_mode &= ~S_IFMT;
      if (get_symlink_p ())
	buf->st_mode |= S_IFLNK;
      else if (get_socket_p ())
	buf->st_mode |= S_IFSOCK;
      else if (local.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	buf->st_mode |= S_IFDIR;
      else
	buf->st_mode |= S_IFREG;
    }
  else
    {
      buf->st_mode = 0;
      buf->st_mode |= STD_RBITS;

      if (!(local.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
	buf->st_mode |= STD_WBITS;
      /* | S_IWGRP | S_IWOTH; we don't give write to group etc */

      if (get_symlink_p ())
	buf->st_mode |= S_IFLNK;
      else if (get_socket_p ())
	buf->st_mode |= S_IFSOCK;
      else
	switch (GetFileType (get_handle ()))
	  {
	  case FILE_TYPE_CHAR:
	  case FILE_TYPE_UNKNOWN:
	    buf->st_mode |= S_IFCHR;
	    break;
	  case FILE_TYPE_DISK:
	   if (local.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	     buf->st_mode |= S_IFDIR | STD_XBITS;
	   else
	     {
	       buf->st_mode |= S_IFREG;
		if (get_execable_p ())
		  buf->st_mode |= STD_XBITS;
	     }
	    break;
	  case FILE_TYPE_PIPE:
	    buf->st_mode |= S_IFSOCK;
	    break;
	  }
    }

  syscall_printf ("0 = fstat (, %p) st_atime=%x st_size=%d, st_mode=%p, st_ino=%d, sizeof=%d",
		 buf, buf->st_atime, buf->st_size, buf->st_mode,
		 (int) buf->st_ino, sizeof (*buf));

  return 0;
}

void
fhandler_base::init (HANDLE f, DWORD a, mode_t bin)
{
  set_io_handle (f);
  set_r_binary (bin);
  set_w_binary (bin);
  access_ = a;
  a &= GENERIC_READ | GENERIC_WRITE;
  if (a == GENERIC_READ)
    set_flags (O_RDONLY);
  if (a == GENERIC_WRITE)
    set_flags (O_WRONLY);
  if (a == (GENERIC_READ | GENERIC_WRITE))
    set_flags (O_RDWR);
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
  if (!DuplicateHandle (hMainProc, get_handle(), hMainProc, &nh, 0, TRUE,
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
        const int allowed_flags = O_APPEND | O_NONBLOCK | OLD_O_NDELAY;

        /* Care for the old O_NDELAY flag. If one of the flags is set,
           both flags are set. */
	int new_flags = (int) arg;
        if (new_flags & (O_NONBLOCK | OLD_O_NDELAY))
          new_flags |= O_NONBLOCK | OLD_O_NDELAY;

        int flags = get_flags () & ~allowed_flags;
        set_flags (flags | (new_flags & allowed_flags));
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
fhandler_base::fhandler_base (DWORD devtype, const char *name, int unit):
  access_ (0),
  io_handle (NULL),
  namehash_ (0),
  openflags_ (0),
  rabuf (NULL),
  ralen (0),
  raixget (0),
  raixput (0),
  rabuflen (0)
{
  status = devtype;
  int bin = __fmode & O_TEXT ? 0 : 1;
  if (status != FH_DISK && status != FH_CONSOLE)
    {
      if (!get_r_binset ())
	set_r_binary (bin);
      if (!get_w_binset ())
	set_w_binary (bin);
    }
  unix_path_name_  = win32_path_name_  = NULL;
  set_name (name, NULL, unit);
}

/* Normal I/O destructor */
fhandler_base::~fhandler_base (void)
{
  if (!no_free_names ())
    {
      if (unix_path_name_ != NULL && unix_path_name_ != fhandler_disk_dummy_name)
	cfree (unix_path_name_);
      if (win32_path_name_ != NULL && win32_path_name_ != fhandler_disk_dummy_name)
	cfree (win32_path_name_);
    }
  if (rabuf)
    free (rabuf);
  unix_path_name_ = win32_path_name_ = NULL;
}

/**********************************************************************/
/* fhandler_disk_file */

fhandler_disk_file::fhandler_disk_file (const char *name) :
	fhandler_base (FH_DISK, name)
{
  set_cb (sizeof *this);
  set_no_free_names ();
  unix_path_name_ = win32_path_name_ = fhandler_disk_dummy_name;
}

int
fhandler_disk_file::open (const char *path, int flags, mode_t mode)
{
  syscall_printf ("(%s, %p)", path, flags);

  /* O_NOSYMLINK is an internal flag for implementing lstat, nothing more. */
  path_conv real_path (path, (flags & O_NOSYMLINK) ?
			     PC_SYM_NOFOLLOW : PC_SYM_FOLLOW);

  if (real_path.error &&
      (flags & O_NOSYMLINK || real_path.error != ENOENT || !(flags & O_CREAT)))
    {
      set_errno (real_path.error);
      syscall_printf ("0 = fhandler_disk_file::open (%s, %p)", path, flags);
      return 0;
    }

  set_name (path, real_path.get_win32 ());
  set_no_free_names (0);
  return open (real_path, flags, mode);
}

int
fhandler_disk_file::open (path_conv& real_path, int flags, mode_t mode)
{
  if (get_win32_name () == fhandler_disk_dummy_name)
    {
      win32_path_name_ = real_path.get_win32 ();
      set_no_free_names ();
    }
  /* If necessary, do various other things to see if path is a program.  */
  if (!real_path.isexec ())
    real_path.set_exec (check_execable_p (get_win32_name ()));

  if (real_path.isbinary ())
    {
      set_r_binary (1);
      set_w_binary (1);
    }

  set_has_acls (real_path.has_acls ());

  if (real_path.file_attributes () != (DWORD)-1
      && (real_path.file_attributes () & FILE_ATTRIBUTE_DIRECTORY))
    flags |= O_DIROPEN;

  int res = this->fhandler_base::open (flags, mode);

  if (!res)
    goto out;

  extern BOOL allow_ntea;
  extern BOOL allow_ntsec;

  if (!real_path.isexec () && !allow_ntea
      && (!allow_ntsec || !real_path.has_acls ())
      && GetFileType (get_handle ()) == FILE_TYPE_DISK)
    {
      DWORD done;
      char magic[3];
      /* FIXME should we use /etc/magic ? */
      magic[0] = magic[1] = magic[2] = '\0';
      ReadFile (get_handle (), magic, 3, &done, 0);
      if (has_exec_chars (magic, done))
	real_path.set_exec ();
      if (!(flags & O_APPEND))
	SetFilePointer (get_handle(), 0, 0, FILE_BEGIN);
    }

  if (flags & O_APPEND)
    SetFilePointer (get_handle(), 0, 0, FILE_END);

  set_symlink_p (real_path.issymlink ());
  set_execable_p (real_path.isexec ());
  set_socket_p (real_path.issocket ());

out:
  syscall_printf ("%d = fhandler_disk_file::open (%s, %p)", res,
		  get_win32_name (), flags);
  return res;
}

int
fhandler_disk_file::close ()
{
  int res;
  if ((res = this->fhandler_base::close ()) == 0)
    cygwin_shared->delqueue.process_queue ();
  return res;
}

/*
 * FIXME !!!
 * The correct way to do this to get POSIX locking
 * semantics is to keep a linked list of posix lock
 * requests and map them into Win32 locks. The problem
 * is that Win32 does not deal correctly with overlapping
 * lock requests. Also another pain is that Win95 doesn't do
 * non-blocking or non exclusive locks at all. For '95 just
 * convert all lock requests into blocking,exclusive locks.
 * This shouldn't break many apps but denying all locking
 * would.
 * For now just convert to Win32 locks and hope for the best.
 */

int
fhandler_disk_file::lock (int cmd, struct flock *fl)
{
  int win32_start;
  int win32_len;
  DWORD win32_upper;
  DWORD startpos;

  /*
   * We don't do getlck calls yet.
   */

  if (cmd == F_GETLK)
    {
      set_errno (ENOSYS);
      return -1;
    }

  /*
   * Calculate where in the file to start from,
   * then adjust this by fl->l_start.
   */

  switch (fl->l_whence)
    {
      case SEEK_SET:
	startpos = 0;
	break;
      case SEEK_CUR:
	if ((off_t) (startpos = lseek (0, SEEK_CUR)) == (off_t)-1)
	  return -1;
	break;
      case SEEK_END:
	{
	  BY_HANDLE_FILE_INFORMATION finfo;
	  if (GetFileInformationByHandle (get_handle(), &finfo) == 0)
	    {
	      __seterrno ();
	      return -1;
	    }
	  startpos = finfo.nFileSizeLow; /* Nowhere to keep high word */
	  break;
	}
      default:
	set_errno (EINVAL);
	return -1;
    }

  /*
   * Now the fun starts. Adjust the start and length
   *  fields until they make sense.
   */

  win32_start = startpos + fl->l_start;
  if (fl->l_len < 0)
    {
      win32_start -= fl->l_len;
      win32_len = -fl->l_len;
    }
  else
    win32_len = fl->l_len;

  if (win32_start < 0)
    {
      /* watch the signs! */
      win32_len -= -win32_start;
      if (win32_len <= 0)
	{
	  /* Failure ! */
	  set_errno (EINVAL);
	  return -1;
	}
      win32_start = 0;
    }

  /*
   * Special case if len == 0 for POSIX means lock
   * to the end of the entire file (and all future extensions).
   */
  if (win32_len == 0)
    {
      win32_len = 0xffffffff;
      win32_upper = host_dependent.win32_upper;
    }
  else
    win32_upper = 0;

  BOOL res;

  if (os_being_run == winNT)
    {
      DWORD lock_flags = (cmd == F_SETLK) ? LOCKFILE_FAIL_IMMEDIATELY : 0;
      lock_flags |= (fl->l_type == F_WRLCK) ? LOCKFILE_EXCLUSIVE_LOCK : 0;

      OVERLAPPED ov;

      ov.Internal = 0;
      ov.InternalHigh = 0;
      ov.Offset = (DWORD)win32_start;
      ov.OffsetHigh = 0;
      ov.hEvent = (HANDLE) 0;

      if (fl->l_type == F_UNLCK)
	{
	  res = UnlockFileEx (get_handle (), 0, (DWORD)win32_len, win32_upper, &ov);
	}
      else
	{
	  res = LockFileEx (get_handle (), lock_flags, 0, (DWORD)win32_len,
							win32_upper, &ov);
	  /* Deal with the fail immediately case. */
	  /*
	   * FIXME !! I think this is the right error to check for
	   * but I must admit I haven't checked....
	   */
	  if ((res == 0) && (lock_flags & LOCKFILE_FAIL_IMMEDIATELY) &&
			    (GetLastError () == ERROR_LOCK_FAILED))
	    {
	      set_errno (EAGAIN);
	      return -1;
	    }
	}
    }
  else
    {
      /* Windows 95 -- use primitive lock call */
      if (fl->l_type == F_UNLCK)
	res = UnlockFile (get_handle (), (DWORD)win32_start, 0, (DWORD)win32_len,
							win32_upper);
      else
	res = LockFile (get_handle (), (DWORD)win32_start, 0, (DWORD)win32_len, win32_upper);
    }

  if (res == 0)
    {
      __seterrno ();
      return -1;
    }

  return 0;
}

/* Perform various heuristics on PATH to see if it's a program. */

int
fhandler_disk_file::check_execable_p (const char *path)
{
  int len = strlen (path);
  const char *ch = path + (len > 4 ? len - 4 : len);

  if (strcasematch (".exe", ch)
      || strcasematch (".bat", ch)
      || strcasematch (".com", ch))
    return 1;
  return 0;
}

/**********************************************************************/
/* /dev/null */

fhandler_dev_null::fhandler_dev_null (const char *name) :
	fhandler_base (FH_NULL, name)
{
  set_cb (sizeof *this);
}

void
fhandler_dev_null::dump (void)
{
  paranoid_printf ("here");
}

/**********************************************************************/
/* fhandler_pipe */

fhandler_pipe::fhandler_pipe (const char *name, DWORD devtype) :
	fhandler_base (devtype, name)
{
  set_cb (sizeof *this);
}

off_t
fhandler_pipe::lseek (off_t offset, int whence)
{
  debug_printf ("(%d, %d)", offset, whence);
  set_errno (ESPIPE);
  return -1;
}

#ifdef DEBUGGING
#define nameparm name
#else
#define nameparm
#endif

void
fhandler_base::set_inheritance (HANDLE &h, int not_inheriting, const char *nameparm)
#undef nameparm
{
  HANDLE newh;

  if (!DuplicateHandle (hMainProc, h, hMainProc, &newh, 0, !not_inheriting,
			DUPLICATE_SAME_ACCESS))
    debug_printf ("DuplicateHandle %E");
#ifndef DEBUGGING
  else
    {
      hclose (h);
      h = newh;
    }
#else
  else if (!name)
    {
      hclose (h);
      h = newh;
    }
  else
  /* FIXME: This won't work with sockets */
    {
      ForceCloseHandle2 (h, name);
      h = newh;
      ProtectHandle2 (h, name);
    }
#endif
}

void
fhandler_base::fork_fixup (HANDLE parent, HANDLE &h, const char *name)
{
  if (!DuplicateHandle (parent, h, hMainProc, &h, 0, !get_close_on_exec (),
			DUPLICATE_SAME_ACCESS))
    system_printf ("%s - %E, handle %s<%p>", get_name (), name, h);
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
