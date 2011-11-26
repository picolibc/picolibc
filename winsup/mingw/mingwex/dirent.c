/*
 * dirent.c
 *
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Derived from DIRLIB.C by Matt J. Weinstein 
 * This note appears in the DIRLIB.H
 * DIRLIB.H by M. J. Weinstein   Released to public domain 1-Jan-89
 *
 * Updated by Jeremy Bettis <jeremy@hksys.com>
 * Significantly revised and rewinddir, seekdir and telldir added
 * by Colin Peters <colin@fu.is.saga-u.ac.jp>
 *	
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <dirent.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h> /* for GetFileAttributes */

#include <tchar.h>
#define SUFFIX	_T("*")
#define SLASH	_T("\\")

#define DIRENT_RETURN_NOTHING
#define DIRENT_REJECT( chk, err, rtn )	\
  do { if( chk ){ errno = (err); return rtn; }} while(0)

union __dirstream_t
{
  /* Actual (private) declaration for opaque data type "DIR". */

  /* dirent struct to return from dir (NOTE: this makes this thread
   * safe as long as only one thread uses a particular DIR struct at
   * a time) */
  struct dirent	dd_dir;

  struct __dirstream_private_t
  {
    /* Three padding fields, matching the head of dd_dir...
     */
    long		dd_ino;		/* Always zero. */
    unsigned short	dd_reclen;	/* Always zero. */
    unsigned short	dd_namlen;	/* Length of name in d_name. */

    /* ...to keep the start of this disk transfer area for this dir
     * aligned at the offset of the dd_dir.d_type field
     */
    struct _finddata_t	dd_dta;

    /* _findnext handle */
    intptr_t		dd_handle;

    /* Status of search:
     *   (type is now int -- was short in older versions).
     *   0 = not started yet (next entry to read is first entry)
     *  -1 = off the end
     *   positive = 0 based index of next entry
     */
    int			dd_stat;

    /* given path for dir with search pattern (struct is extended) */
    char		dd_name[1];

  } dd_private;
};

union __wdirstream_t
{
  /* Actual (private) declaration for opaque data type "_WDIR". */

  /* dirent struct to return from dir (NOTE: this makes this thread
   * safe as long as only one thread uses a particular DIR struct at
   * a time) */
  struct _wdirent	dd_dir;

  struct __wdirstream_private_t
  {
    /* Three padding fields, matching the head of dd_dir...
     */
    long		dd_ino;		/* Always zero. */
    unsigned short	dd_reclen;	/* Always zero. */
    unsigned short	dd_namlen;	/* Length of name in d_name. */

    /* ...to keep the start of this disk transfer area for this dir
     * aligned at the offset of the dd_dir.d_type field
     */
    struct _wfinddata_t	dd_dta;

    /* _findnext handle */
    intptr_t		dd_handle;

    /* Status of search:
     *   0 = not started yet (next entry to read is first entry)
     *  -1 = off the end
     *   positive = 0 based index of next entry
     */
    int			dd_stat;

    /* given path for dir with search pattern (struct is extended) */
    wchar_t		dd_name[1];

  } dd_private;
};

/* We map the BSD d_type field in the returned dirent structure
 * from the Microsoft _finddata_t dd_dta.attrib bits, which are:
 *
 *   _A_NORMAL	(0x0000)	normal file: best fit for DT_REG
 *   _A_RDONLY	(0x0001)	read-only: no BSD d_type equivalent
 *   _A_HIDDEN	(0x0002)	hidden entity: no BSD equivalent
 *   _A_SYSTEM	(0x0004)	system entity: no BSD equivalent
 *   _A_VOLID	(0x0008)	volume label: no BSD equivalent
 *   _A_SUBDIR	(0x0010)	directory: best fit for DT_DIR
 *   _A_ARCH	(0x0020)	"dirty": no BSD equivalent
 *
 * Of these, _A_RDONLY, _A_HIDDEN, _A_SYSTEM, and _A_ARCH are
 * modifier bits, rather than true entity type specifiers; we
 * will ignore them in the mapping, by applying this mask:
 */
#define DT_IGNORED	(_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH)

/* Helper for opendir().  */
static inline unsigned _tGetFileAttributes (const _TCHAR * tPath)
{
#ifdef _UNICODE
  /* GetFileAttributesW does not work on W9x, so convert to ANSI */
  if (_osver & 0x8000)
    {
      char aPath [MAX_PATH];
      WideCharToMultiByte (CP_ACP, 0, tPath, -1, aPath, MAX_PATH, NULL,
			   NULL);
      return GetFileAttributesA (aPath);
    }
  return GetFileAttributesW (tPath);
#else
  return GetFileAttributesA (tPath);
#endif
}

/*
 * opendir
 *
 * Returns a pointer to a DIR structure appropriately filled in to begin
 * searching a directory.
 */
_TDIR * 
_topendir (const _TCHAR *szPath)
{
  _TDIR *nd;
  unsigned int rc;
  _TCHAR szFullPath[MAX_PATH];
	
  /* Reject any request which passes a NULL or an empty path name;
   * note that POSIX doesn't specify the handling for the NULL case,
   * and some implementations may simply fail with a segmentation
   * fault.  We will fail more gracefully; however, the choice of
   * EFAULT is not specified by POSIX, for any opendir failure.
   */
  DIRENT_REJECT( (!szPath), EFAULT, (_TDIR *)(0) );
  /*
   * Conversely, POSIX *does* specify ENOENT for the empty path
   * name case, where we previously had ENOTDIR; here, we correct
   * this previous anomaly.
   */
  DIRENT_REJECT( (*szPath == _T('\0')), ENOENT, (_TDIR *)(0) );

  /* Attempt to determine if the given path really is a directory.
   * On error, user may call GetLastError() for more error info
   */
  DIRENT_REJECT( ((rc = _tGetFileAttributes (szPath)) == (unsigned int)(-1)),
      ENOENT, (_TDIR *)(0)
    );

  /* Error, entry exists but not a directory.
   */
  DIRENT_REJECT( (!(rc & FILE_ATTRIBUTE_DIRECTORY)), ENOTDIR, (_TDIR *)(0) );

  /* Make an absolute pathname.  */
  _tfullpath (szFullPath, szPath, MAX_PATH);

  /* Allocate enough space to store DIR structure and the complete
   * directory path given. */
  nd = (_TDIR *)(malloc( sizeof( _TDIR )
	+ (_tcslen( szFullPath ) + _tcslen( SLASH ) + _tcslen( SUFFIX ) + 1)
	    * sizeof( _TCHAR )
       ));

  /* Error, out of memory.
   */ 
  DIRENT_REJECT( (!nd), ENOMEM, (_TDIR *)(0) );

  /* Create the search expression. */
  _tcscpy (nd->dd_private.dd_name, szFullPath);

  /* Add on a slash if the path does not end with one. */
  if (nd->dd_private.dd_name[0] != _T('\0')
      && _tcsrchr (nd->dd_private.dd_name, _T('/')) != nd->dd_private.dd_name
					    + _tcslen (nd->dd_private.dd_name) - 1
      && _tcsrchr (nd->dd_private.dd_name, _T('\\')) != nd->dd_private.dd_name
      					     + _tcslen (nd->dd_private.dd_name) - 1)
    {
      _tcscat (nd->dd_private.dd_name, SLASH);
    }

  /* Add on the search pattern */
  _tcscat (nd->dd_private.dd_name, SUFFIX);

  /* Initialize handle to -1 so that a premature closedir doesn't try
   * to call _findclose on it. */
  nd->dd_private.dd_handle = -1;

  /* Initialize the status. */
  nd->dd_private.dd_stat = 0;

  /* Initialize the dirent structure. ino and reclen are invalid under
   * Win32, and name simply points at the appropriate part of the
   * findfirst_t structure. */
  nd->dd_dir.d_ino = 0;
  nd->dd_dir.d_reclen = 0;
  nd->dd_dir.d_namlen = 0;
  memset (nd->dd_dir.d_name, 0, FILENAME_MAX);

  return nd;
}


/*
 * readdir
 *
 * Return a pointer to a dirent structure filled with the information on the
 * next entry in the directory.
 */
struct _tdirent *
_treaddir (_TDIR *dirp)
{
  /* Check for a valid DIR stream reference; (we can't really
   * be certain until we try to read from it, except in the case
   * of a NULL pointer reference).  Where we lack a valid reference,
   * POSIX mandates reporting EBADF; we previously had EFAULT, so
   * this version corrects the former anomaly.
   */ 
  DIRENT_REJECT( (!dirp), EBADF, (struct _tdirent *)(0) );

  if (dirp->dd_private.dd_stat < 0)
    {
      /* We have already returned all files in the directory
       * (or the structure has an invalid dd_stat). */
      return (struct _tdirent *) 0;
    }
  else if (dirp->dd_private.dd_stat == 0)
    {
      /* We haven't started the search yet. */
      /* Start the search */
      dirp->dd_private.dd_handle = _tfindfirst (dirp->dd_private.dd_name, &(dirp->dd_private.dd_dta));

      if (dirp->dd_private.dd_handle == -1)
	{
	  /* Oops! Seems there are no files in that
	   * directory. */
	  dirp->dd_private.dd_stat = -1;
	}
      else
	{
	  dirp->dd_private.dd_stat = 1;
	}
    }
  else
    {
      /* Get the next search entry.  POSIX mandates that this must
       * return NULL after the last entry has been read, but that it
       * MUST NOT change errno in this case.  MS-Windows _findnext()
       * DOES change errno (to ENOENT) after the last entry has been
       * read, so we must be prepared to restore it to its previous
       * value, when no actual error has occurred.
       */
      int prev_errno = errno;
      if (_tfindnext (dirp->dd_private.dd_handle, &(dirp->dd_private.dd_dta)))
	{
	  /* May be an error, or just the case described above...
	   */
	  if( GetLastError() == ERROR_NO_MORE_FILES )
	    /*
	     * ...which requires us to reset errno.
	     */
	    errno = prev_errno;	

	  /* FIXME: this is just wrong: we should NOT close the DIR
	   * handle here; it is the responsibility of closedir().
	   */
	  _findclose (dirp->dd_private.dd_handle);
	  dirp->dd_private.dd_handle = -1;
	  dirp->dd_private.dd_stat = -1;
	}
      else
	{
	  /* Update the status to indicate the correct
	   * number. */
	  dirp->dd_private.dd_stat++;
	}
    }

  if (dirp->dd_private.dd_stat > 0)
    {
      /* Successfully got an entry. Everything about the file is
       * already appropriately filled in except the length of the
       * file name...
       */
      dirp->dd_dir.d_namlen = _tcslen (dirp->dd_dir.d_name);
      /*
       * ...and the attributes returned in the dd_dta.attrib field;
       * these require adjustment to their BSD equivalents, which are
       * returned via the union with the dd_dir.d_type field:
       */
      switch( dirp->dd_dir.d_type &= ~DT_IGNORED )
	{
	  case DT_REG:
	  case DT_DIR:
	    /* After stripping out the modifier bits in DT_IGNORED,
	     * (which we ALWAYS ignore), this pair require no further
	     * adjustment...
	     */
	    break;

	  default:
	    /* ...while nothing else has an appropriate equivalent
	     * in the BSD d_type identification model.
	     */
	    dirp->dd_dir.d_type = DT_UNKNOWN;
	}
      return &dirp->dd_dir;
    }

  return (struct _tdirent *) 0;
}


/*
 * closedir
 *
 * Frees up resources allocated by opendir.
 */
int
_tclosedir (_TDIR * dirp)
{
  int rc = 0;

  /* Attempting to reference a directory stream via a NULL pointer
   * would cause a segmentation fault; evade this.  Since NULL can
   * never represent an open directory stream, set the EBADF errno
   * status, as mandated by POSIX, once again correcting previous
   * anomalous use of EFAULT in this context.
   */
  DIRENT_REJECT( (!dirp), EBADF, -1 );

  if (dirp->dd_private.dd_handle != -1)
    {
      rc = _findclose (dirp->dd_private.dd_handle);
    }

  /* Delete the dir structure. */
  free (dirp);

  return rc;
}

/*
 * rewinddir
 *
 * Return to the beginning of the directory "stream". We simply call findclose
 * and then reset things like an opendir.
 */
void
_trewinddir (_TDIR * dirp)
{
  /* Once again, evade a potential segmentation fault on passing
   * a NULL reference pointer, and again correct previous anomalous
   * use of EFAULT, where POSIX mandates EBADF for errno reporting.
   */
  DIRENT_REJECT( (!dirp), EBADF, DIRENT_RETURN_NOTHING );

  if (dirp->dd_private.dd_handle != -1)
    {
      _findclose (dirp->dd_private.dd_handle);
    }

  dirp->dd_private.dd_handle = -1;
  dirp->dd_private.dd_stat = 0;
}

/*
 * telldir
 *
 * Returns the "position" in the "directory stream" which can be used with
 * seekdir to go back to an old entry. We simply return the value in stat.
 */
long
_ttelldir (_TDIR * dirp)
{
  /* Once again, evade a potential segmentation fault on passing
   * a NULL reference pointer, and again correct previous anomalous
   * use of EFAULT, where POSIX mandates EBADF for errno reporting.
   */
  DIRENT_REJECT( (!dirp), EBADF, -1 );
  return dirp->dd_private.dd_stat;
}

/*
 * seekdir
 *
 * Seek to an entry previously returned by telldir. We rewind the directory
 * and call readdir repeatedly until either dd_stat is the position number
 * or -1 (off the end). This is not perfect, in that the directory may
 * have changed while we weren't looking. But that is probably the case with
 * any such system.
 */
void
_tseekdir (_TDIR * dirp, long lPos)
{
  /* Once again, evade a potential segmentation fault on passing
   * a NULL reference pointer, and again correct previous anomalous
   * use of EFAULT, where POSIX mandates EBADF for errno reporting.
   */
  DIRENT_REJECT( (!dirp), EBADF, DIRENT_RETURN_NOTHING );

  /* Seeking to an invalid position.
   */
  DIRENT_REJECT( (lPos < -1), EINVAL, DIRENT_RETURN_NOTHING );

  if (lPos == -1)
    {
      /* Seek past end. */
      if (dirp->dd_private.dd_handle != -1)
	{
	  _findclose (dirp->dd_private.dd_handle);
	}
      dirp->dd_private.dd_handle = -1;
      dirp->dd_private.dd_stat = -1;
    }
  else
    {
      /* Rewind and read forward to the appropriate index. */
      _trewinddir (dirp);

      while ((dirp->dd_private.dd_stat < lPos) && _treaddir (dirp))
	;
    }
}
