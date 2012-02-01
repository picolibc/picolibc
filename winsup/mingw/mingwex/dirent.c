/*
 * dirent.c
 *
 * This file has no copyright assigned and is placed in the Public Domain.
 *
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
 * Further significantly revised for improved memory utilisation,
 * efficiency in operation, and better POSIX standards compliance
 * by Keith Marshall <keithmarshall@users.sourceforge.net>
 *	
 */
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <dirent.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

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

#define DIRENT_OPEN(D)	\
    ((D).dd_handle = _tfindfirst((D).dd_name, &((D).dd_dta)))

#define DIRENT_UPDATE(D)  \
    _tfindnext( (D).dd_handle, &(D).dd_dta )


/*****
 *
 * opendir()
 *
 * Returns a pointer to a DIR structure appropriately filled in
 * to begin searching a directory.
 *
 */
_TDIR * 
_topendir( const _TCHAR *path_name )
{
  _TDIR *nd;
  _TCHAR abs_path[MAX_PATH];
	
  /* Reject any request which passes a NULL or an empty path name;
   * note that POSIX doesn't specify the handling for the NULL case,
   * and some implementations may simply fail with a segmentation
   * fault.  We will fail more gracefully.  Previous versions used
   * EFAULT here, but EINVAL seems more appropriate; however, POSIX
   * specifies neither of these for any opendir() failure.
   */
  DIRENT_REJECT( (path_name == NULL), EINVAL, (_TDIR *)(NULL) );
  /*
   * Conversely, POSIX *does* specify ENOENT for the empty path
   * name case, where we previously had ENOTDIR; here, we correct
   * this previous anomaly.
   */
  DIRENT_REJECT( (*path_name == _T('\0')), ENOENT, (_TDIR *)(NULL) );

  /* Identify the absolute path name corresponding to the (maybe
   * relative) path name we are to process; (this ensures that we
   * may always refer back to this same path name, e.g. to rewind
   * the "directory stream", even after an intervening change of
   * current working directory).
   */
  _tfullpath( abs_path, path_name, MAX_PATH );

  /* Ensure that the generated absolute path name ends with a
   * directory separator (backslash) character, so that we may
   * correctly append a wild-card matching pattern which will
   * cause _findfirst() and _findnext() to return every entry
   * in the specified directory; (note that, for now we may
   * simply assume that abs_path refers to a directory;
   * we will verify that when we call _findfirst() on it).
   */
  if( *abs_path != _T('\0') )
    {
      size_t offset = _tcslen( abs_path ) - 1;
      if( (abs_path[offset] != _T('/')) && (abs_path[offset] != _T('\\')) )
	_tcscat( abs_path, _T("\\") );
    }

  /* Now append the "match everything" wild-card pattern.
   */
  _tcscat( abs_path, _T("*") );

  /* Allocate space to store DIR structure.  The size MUST be
   * adjusted to accommodate the complete absolute path name for
   * the specified directory, extended to include the wild-card
   * matching pattern, as above; (note that we DO NOT need any
   * special provision for the terminating NUL on the path name,
   * since the base size of the DIR structure includes it).
   */
  nd = (_TDIR *)(malloc(
	 sizeof( _TDIR ) + (_tcslen( abs_path ) * sizeof( _TCHAR ))
       ));

  /* Bail out, if insufficient memory.
   */ 
  DIRENT_REJECT( (nd == NULL), ENOMEM, (_TDIR *)(NULL) );

  /* Copy the extended absolute path name string into place
   * within the allocated space for the DIR structure.
   */
  _tcscpy( nd->dd_private.dd_name, abs_path );

  /* Initialize the "directory stream", by calling _findfirst() on it;
   * this leaves the data for the first directory entry in the internal
   * dirent buffer, ready to be retrieved by readdir().
   */
  if( DIRENT_OPEN( nd->dd_private ) == (intptr_t)(-1) )
    {
      /* The _findfirst() call, (implied by DIRENT_OPEN), failed;
       * _findfirst() sets EINVAL where POSIX mandates ENOTDIR...
       */
      if( errno == EINVAL )
	errno = ENOTDIR;

      /* ...otherwise, while it may not be strictly POSIX conformant,
       * just accept whatever value _findfirst() assigned to errno.  In
       * any event, prepare to return the NULL "directory stream"; since
       * this implies that we will lose our reference pointer to the
       * block of memory we allocated for the stream, free that
       * before we bail out.
       */
      free( nd );
      return (_TDIR *)(NULL);
    }

  /* Initialize the status, (i.e. the location index), so that
   * readdir() will simply return the first directory entry, which
   * has already been fetched by _findfirst(), without performing
   * an intervening _findnext() call.
   */
  nd->dd_private.dd_stat = 0;

  /* The d_ino and d_reclen fields have no relevance in MS-Windows;
   * initialize them to zero, as a one-time assignment for this DIR
   * instance, and henceforth forget them; (users should simply
   * ignore them).
   */
  nd->dd_dir.d_ino = 0;
  nd->dd_dir.d_reclen = 0;

  /* We've now completely initialized an instance of a DIR structure,
   * representing the requested "directory stream"; return a pointer
   * via which the caller may access it.
   */
  return nd;
}


/*****
 *
 * readdir()
 *
 * Return a pointer to a dirent structure filled in with information
 * on the next available entry, (if any), in the "directory stream".
 */
struct _tdirent *
_treaddir( _TDIR *dirp )
{
  /* Check for a valid DIR stream reference; (we can't really
   * be certain until we try to read from it, except in the case
   * of a NULL pointer reference).  Where we lack a valid reference,
   * POSIX mandates reporting EBADF; we previously had EFAULT, so
   * this version corrects the former anomaly.
   */ 
  DIRENT_REJECT( (dirp == NULL), EBADF, (struct _tdirent *)(NULL) );

  /* Okay to proceed.  If this is the first readdir() request
   * following an opendir(), or a rewinddir(), then we already
   * have the requisite return information...
   */
  if( dirp->dd_private.dd_stat++ > 0 )
    {
      /* Otherwise...
       *
       * Get the next search entry.  POSIX mandates that this must
       * return NULL after the last entry has been read, but that it
       * MUST NOT change errno in this case.  MS-Windows _findnext()
       * DOES change errno (to ENOENT) after the last entry has been
       * read, so we must be prepared to restore it to its previous
       * value, when no actual error has occurred.
       */
      int prev_errno = errno;
      if( DIRENT_UPDATE( dirp->dd_private ) != 0 )
	{
	  /* May be an error, or just the case described above...
	   */
	  if( GetLastError() == ERROR_NO_MORE_FILES )
	    /*
	     * ...which requires us to reset errno.
	     */
	    errno = prev_errno;	

	  /* In either case, there is no valid data to return.
	   */
	  return (struct _tdirent *)(NULL);
	}
    }

  /* Successfully got an entry.  Everything about the file is
   * already appropriately filled in, except for the length of
   * the file name in the d_namlen field...
   */
  dirp->dd_dir.d_namlen = _tcslen( dirp->dd_dir.d_name );
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


/*****
 *
 * closedir()
 *
 * Frees up resources allocated by opendir().
 *
 */
int
_tclosedir( _TDIR * dirp )
{
  /* Attempting to reference a directory stream via a NULL pointer
   * would cause a segmentation fault; evade this.  Since NULL can
   * never represent an open directory stream, set the EBADF errno
   * status, as mandated by POSIX, once again correcting previous
   * anomalous use of EFAULT in this context.
   */
  DIRENT_REJECT(
      ((dirp == NULL) || (_findclose( dirp->dd_private.dd_handle ) != 0)),
	EBADF, -1
    );

  /* If we didn't bail out above, we have a valid DIR structure
   * with which we have finished; release the memory allocated
   * to it, before returning "success".
   */
  free( dirp );
  return 0;
}


/*****
 *
 * rewinddir()
 *
 * Return to the beginning of the directory "stream".  We simply call
 * _findclose(), to clear prior context, then _findfirst() to restart
 * the directory search, resetting the location index appropriately,
 * as it would be left by opendir().
 *
 */
void
_trewinddir( _TDIR * dirp )
{
  /* This is an XSI extension to POSIX, which specifies no formal
   * error conditions; we will continue to check for and evade the
   * potential segmentation fault which would result from passing a
   * NULL reference pointer.  For consistency with the core functions
   * implemented above, we will again report this as EBADF, rather
   * than the EFAULT of previous versions.
   */
  DIRENT_REJECT(
      ((dirp == NULL) || (_findclose( dirp->dd_private.dd_handle ) != 0)),
	EBADF, DIRENT_RETURN_NOTHING
    );
  
  /* We successfully closed the prior search context; reopen...
   */
  if( DIRENT_OPEN( dirp->dd_private ) != (intptr_t)(-1) )
    /*
     * ...and, on success, reset the location index.
     */
    dirp->dd_private.dd_stat = 0;
}


/*****
 *
 * telldir()
 *
 * Returns the "position" in the "directory stream" which can then
 * be passed to seekdir(), to return back to a previous entry.  We
 * simply return the current location index from the dd_stat field.
 *
 */
long
_ttelldir( _TDIR * dirp )
{
  /* This too is a POSIX-XSI extension, with no mandatory error
   * conditions.  Once again, evade a potential segmentation fault
   * on passing a NULL reference pointer, again reporting it as
   * EBADF in preference to the EFAULT of previous versions.
   */
  DIRENT_REJECT( (dirp == NULL), EBADF, -1 );

  /* We didn't bail out; just assume dirp is valid, and return
   * the location index from the dd_stat field.
   */
  return dirp->dd_private.dd_stat;
}


/*****
 *
 * seekdir()
 *
 * Seek to an entry previously returned by telldir().  We rewind
 * the "directory stream", then repeatedly call _findnext() while
 * incrementing its internal location index until it matches the
 * position requested, or we reach the end of the stream.  This is
 * not perfect, in that the directory may have changed while we
 * weren't looking, but it is the best we can achieve, and may
 * likely reproduce the behaviour of other implementations.
 *
 */
void
_tseekdir( _TDIR * dirp, long loc )
{
  /* Another POSIX-XSI extension, with no specified mandatory
   * error conditions; we require a seek location of zero or
   * greater, and will reject less than zero as EINVAL...
   */
  DIRENT_REJECT( (loc < 0L), EINVAL, DIRENT_RETURN_NOTHING );

  /* Other than this, we simply accept any error condition
   * which arises as we "rewind" the "directory stream"...
   */
  _trewinddir( dirp );

  /* ...and, if this is successful...
   */
  if( (loc > 0) && (dirp->dd_private.dd_handle != (intptr_t)(-1)) )
    /*
     * ...seek forward until the location index within
     * the DIR structure matches the requested location.
     */
    while( (++dirp->dd_private.dd_stat < loc)
      &&   (DIRENT_UPDATE( dirp->dd_private ) == 0)  )
      ;
}

/* $RCSfile$: end of file */
