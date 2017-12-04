/* POSIX variant of strerror_r. */
#undef __STRICT_ANSI__
#include <errno.h>
#include <string.h>

int
__xpg_strerror_r (int errnum,
	char *buffer,
	size_t n)
{
  char *error;
  int result = 0;

  if (!n)
    return ERANGE;
  error = _strerror_r (_REENT, errnum, 1, &result);
  if (strlen (error) >= n)
    {
      memcpy (buffer, error, n - 1);
      buffer[n - 1] = '\0';
      return ERANGE;
    }
  strcpy (buffer, error);
  return (result || *error) ? result : EINVAL;
}
