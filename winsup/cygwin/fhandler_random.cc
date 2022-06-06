/* fhandler_random.cc: code to access /dev/random and /dev/urandom

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <ntsecapi.h>
#include <unistd.h>
#include <sys/param.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "sync.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"

#define RANDOM   8
#define URANDOM  9

/* The system PRNG is reseeded after reading 128K bytes. */
#define RESEED_INTERVAL	(128 * 1024)

#define PSEUDO_MULTIPLIER       (6364136223846793005LL)
#define PSEUDO_SHIFTVAL		(21)

int
fhandler_dev_random::pseudo_write (const void *ptr, size_t len)
{
  /* Use buffer to mess up the pseudo random number generator. */
  for (size_t i = 0; i < len; ++i)
    pseudo = (pseudo + ((unsigned char *)ptr)[i]) * PSEUDO_MULTIPLIER + 1;
  return len;
}

ssize_t
fhandler_dev_random::write (const void *ptr, size_t len)
{
  if (!len)
    return 0;
  if (!ptr)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* Limit len to a value <= 4096 since we don't want to overact.
     Copy to local buffer because RtlGenRandom violates const. */
  size_t limited_len = MIN (len, 4096);
  unsigned char buf[limited_len];

  /* Mess up system entropy source. Return error if device is /dev/random. */
  __try
    {
      memcpy (buf, ptr, limited_len);
      if (!RtlGenRandom (buf, limited_len) && dev () == FH_RANDOM)
	return -1;
      /* Mess up the pseudo random number generator. */
      pseudo_write (buf, limited_len);
    }
  __except (EFAULT)
    {
      len = -1;
    }
  __endtry
  /* Note that we return len, not limited_len.  No reason to confuse the
     caller... */
  return len;
}

int
fhandler_dev_random::pseudo_read (void *ptr, size_t len)
{
  /* Use pseudo random number generator as fallback entropy source.
     This multiplier was obtained from Knuth, D.E., "The Art of
     Computer Programming," Vol 2, Seminumerical Algorithms, Third
     Edition, Addison-Wesley, 1998, p. 106 (line 26) & p. 108 */
  for (size_t i = 0; i < len; ++i)
    {
      pseudo = pseudo * PSEUDO_MULTIPLIER + 1;
      ((unsigned char *)ptr)[i] = (pseudo >> PSEUDO_SHIFTVAL) & UCHAR_MAX;
    }
  return len;
}

void
fhandler_dev_random::read (void *ptr, size_t& len)
{
  if (!len)
    return;

  if (!ptr)
    {
      set_errno (EINVAL);
      len = (size_t) -1;
      return;
    }

  __try
    {
      /* /dev/random has to provide high quality random numbers.  Therefore we
	 re-seed the system PRNG for each block of 512 bytes.  This results in
	 sufficiently random sequences, comparable to the Linux /dev/random. */
      if (dev () == FH_RANDOM)
	{
	  void *dummy = malloc (RESEED_INTERVAL);
	  if (!dummy)
	    {
	      __seterrno ();
	      len = (size_t) -1;
	      return;
	    }
	  for (size_t offset = 0; offset < len; offset += 512)
	    {
	      if (!RtlGenRandom (dummy, RESEED_INTERVAL) ||
		  !RtlGenRandom ((PBYTE) ptr + offset, len - offset))
		{
		  len = (size_t) -1;
		  break;
		}
	    }
	  free (dummy);
	}

      /* If device is /dev/urandom, just use system RNG as is, with our own
	 PRNG as fallback. */
      else if (!RtlGenRandom (ptr, len))
	len = pseudo_read (ptr, len);
    }
  __except (EFAULT)
    {
      len = (size_t) -1;
    }
  __endtry
}
