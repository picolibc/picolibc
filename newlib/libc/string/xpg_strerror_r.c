/* Copyright (c) 2011 Eric Blake <eblake@redhat.com> */
/* POSIX variant of strerror_r. */
#define _POSIX_C_SOURCE 200809
#include <errno.h>
#include <string.h>

__typeof(strerror_r) __xpg_strerror_r;

int
__xpg_strerror_r (int errnum,
	char *buffer,
	size_t n)
{
  char *error;
  int result = 0;

  if (!n)
    return ERANGE;
  error = _strerror_r (errnum, 1, &result);
  if (strlen (error) >= n)
    {
      memcpy (buffer, error, n - 1);
      buffer[n - 1] = '\0';
      return ERANGE;
    }
  strcpy (buffer, error);
  return (result || *error) ? result : EINVAL;
}
