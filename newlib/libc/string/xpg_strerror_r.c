/* POSIX variant of strerror_r. */
#undef __STRICT_ANSI__
#include <errno.h>
#include <string.h>

int
_DEFUN (__xpg_strerror_r, (errnum, buffer, n),
	int errnum _AND
	char *buffer _AND
	size_t n)
{
  char *error;

  if (!n)
    return ERANGE;
  error = strerror (errnum);
  if (strlen (error) >= n)
    {
      memcpy (buffer, error, n - 1);
      buffer[n - 1] = '\0';
      return ERANGE;
    }
  strcpy (buffer, error);
  return *error ? 0 : EINVAL;
}
