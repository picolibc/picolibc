/* fhandler_dev_clipboard: code to access /dev/clipboard

   Written by Charles Wilson (cwilson@ece.gatech.edu)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <wchar.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "sync.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"
#include <sys/clipboard.h>
#include <unistd.h>

/* Opening clipboard immediately after CloseClipboard()
   sometimes fails. Therefore use retry-loop. */
static inline bool
open_clipboard ()
{
  const int max_retry = 10;
  for (int i = 0; i < max_retry; i++)
    {
      if (OpenClipboard (NULL))
	return true;
      Sleep (1);
    }
  return false;
}

static inline bool
close_clipboard ()
{
  return CloseClipboard ();
}

/*
 * Robert Collins:
 * FIXME: should we use GetClipboardSequenceNumber to tell if the clipboard has
 * changed? How does /dev/clipboard operate under (say) linux?
 */

fhandler_dev_clipboard::fhandler_dev_clipboard ()
  : fhandler_base (), pos (0), membuffer (NULL), msize (0)
{
  /* FIXME: check for errors and loop until we can open the clipboard */
  open_clipboard ();
  cygnativeformat = RegisterClipboardFormatW (CYGWIN_NATIVE);
  close_clipboard ();
}

/*
 * Special clipboard dup to duplicate input and output
 * handles.
 */

int
fhandler_dev_clipboard::dup (fhandler_base * child, int flags)
{
  fhandler_dev_clipboard *fhc = (fhandler_dev_clipboard *) child;
  fhc->pos = fhc->msize = 0;
  fhc->membuffer = NULL;
  return fhandler_base::dup (child, flags);
}

int
fhandler_dev_clipboard::set_clipboard (const void *buf, size_t len)
{
  HGLOBAL hmem;
  /* Native CYGWIN format */
  if (open_clipboard ())
    {
      cygcb_t *clipbuf;

      hmem = GlobalAlloc (GMEM_MOVEABLE, sizeof (cygcb_t) + len);
      if (!hmem)
	{
	  __seterrno ();
	  close_clipboard ();
	  return -1;
	}
      clipbuf = (cygcb_t *) GlobalLock (hmem);

      clock_gettime (CLOCK_REALTIME, &clipbuf->ts);
      clipbuf->cb_size = len;
      memcpy (clipbuf->cb_data, buf, len); // append user-supplied data

      GlobalUnlock (hmem);
      EmptyClipboard ();
      HANDLE ret = SetClipboardData (cygnativeformat, hmem);
      close_clipboard ();
      /* According to MSDN, hmem must not be free'd after transferring the
	 data to the clipboard via SetClipboardData. */
      /* GlobalFree (hmem); */
      if (!ret)
	{
	  __seterrno ();
	  return -1;
	}
    }

  /* CF_TEXT/CF_OEMTEXT for copying to wordpad and the like */
  len = sys_mbstowcs (NULL, 0, (const char *) buf, len);
  if (!len)
    {
      set_errno (EILSEQ);
      return -1;
    }
  if (open_clipboard ())
    {
      PWCHAR clipbuf;

      hmem = GlobalAlloc (GMEM_MOVEABLE, (len + 1) * sizeof (WCHAR));
      if (!hmem)
	{
	  __seterrno ();
	  close_clipboard ();
	  return -1;
	}
      clipbuf = (PWCHAR) GlobalLock (hmem);
      sys_mbstowcs (clipbuf, len + 1, (const char *) buf);
      GlobalUnlock (hmem);
      HANDLE ret = SetClipboardData (CF_UNICODETEXT, hmem);
      close_clipboard ();
      /* According to MSDN, hmem must not be free'd after transferring the
	 data to the clipboard via SetClipboardData. */
      /* GlobalFree (hmem); */
      if (!ret)
	{
	  __seterrno ();
	  return -1;
	}
    }
  return 0;
}

/* FIXME: arbitrary seeking is not handled */
ssize_t
fhandler_dev_clipboard::write (const void *buf, size_t len)
{
  /* write to our membuffer */
  size_t cursize = msize;
  void *tempbuffer = realloc (membuffer, cursize + len);
  if (!tempbuffer)
    {
      debug_printf ("Couldn't realloc() clipboard buffer for write");
      return -1;
    }
  membuffer = tempbuffer;
  msize = cursize + len;
  memcpy ((unsigned char *) membuffer + cursize, buf, len);

  /* now pass to windows */
  if (set_clipboard (membuffer, msize))
    {
      /* FIXME: membuffer is now out of sync with pos, but msize
		is used above */
      return -1;
    }

  pos = msize;
  return len;
}

int
fhandler_dev_clipboard::fstat (struct stat *buf)
{
  buf->st_mode = S_IFCHR | STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;
  buf->st_uid = geteuid ();
  buf->st_gid = getegid ();
  buf->st_nlink = 1;
  buf->st_blksize = PREFERRED_IO_BLKSIZE;

  buf->st_ctim.tv_sec = 1164931200L;	/* Arbitrary value: 2006-12-01 */
  buf->st_ctim.tv_nsec = 0L;
  buf->st_birthtim = buf->st_atim = buf->st_mtim = buf->st_ctim;

  if (open_clipboard ())
    {
      UINT formatlist[1] = { cygnativeformat };
      int format;
      HGLOBAL hglb;
      cygcb_t *clipbuf;

      if ((format = GetPriorityClipboardFormat (formatlist, 1)) > 0
	  && (hglb = GetClipboardData (format))
	  && (clipbuf = (cygcb_t *) GlobalLock (hglb)))
	{
	  buf->st_atim = buf->st_mtim = clipbuf->ts;
	  buf->st_size = clipbuf->cb_size;
	  GlobalUnlock (hglb);
	}
      close_clipboard ();
    }

  return 0;
}

void
fhandler_dev_clipboard::read (void *ptr, size_t& len)
{
  HGLOBAL hglb;
  size_t ret = 0;
  UINT formatlist[2];
  UINT format;
  LPVOID cb_data;
  int rach;

  if (!open_clipboard ())
    {
      len = 0;
      return;
    }
  formatlist[0] = cygnativeformat;
  formatlist[1] = CF_UNICODETEXT;
  if ((format = GetPriorityClipboardFormat (formatlist, 2)) <= 0
      || !(hglb = GetClipboardData (format))
      || !(cb_data = GlobalLock (hglb)))
    {
      close_clipboard ();
      len = 0;
      return;
    }
  if (format == cygnativeformat)
    {
      cygcb_t *clipbuf = (cygcb_t *) cb_data;

      if (pos < (off_t) clipbuf->cb_size)
	{
	  ret = (len > (clipbuf->cb_size - pos)) ? clipbuf->cb_size - pos : len;
	  memcpy (ptr, clipbuf->cb_data + pos, ret);
	  pos += ret;
	}
    }
  else if ((rach = get_readahead ()) >= 0)
    {
      /* Deliver from read-ahead buffer. */
      char * out_ptr = (char *) ptr;
      * out_ptr++ = rach;
      ret = 1;
      while (ret < len && (rach = get_readahead ()) >= 0)
	{
	  * out_ptr++ = rach;
	  ret++;
	}
    }
  else
    {
      wchar_t *buf = (wchar_t *) cb_data;

      size_t glen = GlobalSize (hglb) / sizeof (WCHAR) - 1;
      if (pos < (off_t) glen)
	{
	  /* If caller's buffer is too small to hold at least one
	     max-size character, redirect algorithm to local
	     read-ahead buffer, finally fill class read-ahead buffer
	     with result and feed caller from there. */
	  char *conv_ptr = (char *) ptr;
	  size_t conv_len = len;
#define cprabuf_len MB_LEN_MAX	/* max MB_CUR_MAX of all encodings */
	  char cprabuf [cprabuf_len];
	  if (len < cprabuf_len)
	    {
	      conv_ptr = cprabuf;
	      conv_len = cprabuf_len;
	    }

	  /* Comparing apples and oranges here, but the below loop could become
	     extremly slow otherwise.  We rather return a few bytes less than
	     possible instead of being even more slow than usual... */
	  if (glen > pos + conv_len)
	    glen = pos + conv_len;
	  /* This loop is necessary because the number of bytes returned by
	     sys_wcstombs does not indicate the number of wide chars used for
	     it, so we could potentially drop wide chars. */
	  while ((ret = sys_wcstombs (NULL, 0, buf + pos, glen - pos))
		  != (size_t) -1
		 && (ret > conv_len
			/* Skip separated high surrogate: */
		     || ((buf [glen - 1] & 0xFC00) == 0xD800
			 && glen - pos > 1)))
	     --glen;
	  if (ret == (size_t) -1)
	    ret = 0;
	  else
	    {
	      ret = sys_wcstombs ((char *) conv_ptr, (size_t) -1,
				  buf + pos, glen - pos);
	      pos = glen;
	      /* If using read-ahead buffer, copy to class read-ahead buffer
	         and deliver first byte. */
	      if (conv_ptr == cprabuf)
		{
		  puts_readahead (cprabuf, ret);
		  char *out_ptr = (char *) ptr;
		  ret = 0;
		  while (ret < len && (rach = get_readahead ()) >= 0)
		    {
		      * out_ptr++ = rach;
		      ret++;
		    }
		}
	    }
	}
    }
  GlobalUnlock (hglb);
  close_clipboard ();
  len = ret;
}

off_t
fhandler_dev_clipboard::lseek (off_t offset, int whence)
{
  /* On reads we check this at read time, not seek time.
   * On writes we use this to decide how to write - empty and write, or open, copy, empty
   * and write
   */
  pos = offset;
  /* treat seek like rewind */
  if (membuffer)
    {
      free (membuffer);
      membuffer = NULL;
    }
  msize = 0;
  return 0;
}

int
fhandler_dev_clipboard::close ()
{
  if (!have_execed)
    {
      pos = msize = 0;
      if (membuffer)
	{
	  free (membuffer);
	  membuffer = NULL;
	}
    }
  return fhandler_base::close ();
}

void
fhandler_dev_clipboard::fixup_after_exec ()
{
  if (!close_on_exec ())
    {
      pos = msize = 0;
      membuffer = NULL;
    }
}
