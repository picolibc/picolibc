/* Linux version of pwrite so we can have a weak alias */

#include <_ansi.h>
#include <unistd.h>
#include <reent.h>
#include <machine/weakalias.h>

ssize_t
_pwrite_r (struct _reent *rptr,
     int fd,
     const void *buf,
     size_t n,
     off_t off)
{
  off_t cur_pos;
  _READ_WRITE_RETURN_TYPE num_written;
  
  if ((cur_pos = _lseek_r (rptr, fd, 0, SEEK_CUR)) == (off_t)-1)
    return -1;

  if (_lseek_r (rptr, fd, off, SEEK_SET) == (off_t)-1)
    return -1;

  num_written = _write_r (rptr, fd, buf, n);

  if (_lseek_r (rptr, fd, cur_pos, SEEK_SET) == (off_t)-1)
    return -1;

  return (ssize_t)num_written;
}

#ifndef _REENT_ONLY

ssize_t
__libc_pwrite (int fd,
     const void *buf,
     size_t n,
     off_t off)
{
  return _pwrite_r (_REENT, fd, buf, n, off);
}
weak_alias(__libc_pwrite,pwrite)

#endif
