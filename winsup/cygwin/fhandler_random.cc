/* fhandler_dev_random.cc: code to access /dev/random and /dev/urandom

   Copyright 2000 Cygnus Solutions.

   Written by Corinna Vinschen (vinschen@cygnus.com)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <limits.h>
#include "cygerrno.h"
#include "fhandler.h"

#define RANDOM   8
#define URANDOM  9

#define PSEUDO_MULTIPLIER       (6364136223846793005LL)
#define PSEUDO_SHIFTVAL		(21)

fhandler_dev_random::fhandler_dev_random (const char *name, int nunit)
  : fhandler_base (FH_RANDOM, name),
    unit(nunit),
    crypt_prov((HCRYPTPROV)NULL)
{
  set_cb (sizeof *this);
}

int
fhandler_dev_random::open (const char *, int flags, mode_t)
{
  set_flags (flags);
  return 1;
}

BOOL
fhandler_dev_random::crypt_gen_random (void *ptr, size_t len)
{
  if (!crypt_prov
      && !CryptAcquireContext (&crypt_prov, NULL, MS_DEF_PROV, PROV_RSA_FULL,
			       CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)
      && !CryptAcquireContext (&crypt_prov, NULL, MS_DEF_PROV, PROV_RSA_FULL,
			       CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET
			       | CRYPT_NEWKEYSET))
    {
      debug_printf ("%E = CryptAquireContext()");
      return FALSE;
    }
  if (!CryptGenRandom (crypt_prov, len, (BYTE *)ptr))
    {
      debug_printf ("%E = CryptGenRandom()");
      return FALSE;
    }
  return TRUE;
}

int
fhandler_dev_random::pseudo_write (const void *ptr, size_t len)
{
  /* Use buffer to mess up the pseudo random number generator. */
  for (size_t i = 0; i < len; ++i)
    pseudo = (pseudo + ((unsigned char *)ptr)[i]) * PSEUDO_MULTIPLIER + 1;
  return len;
}

int
fhandler_dev_random::write (const void *ptr, size_t len)
{
  if (!len)
    return 0;
  if (!ptr)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* Limit len to a value <= 512 since we don't want to overact.
     Copy to local buffer because CryptGenRandom violates const. */
  unsigned char buf[512];
  size_t limited_len = len <= 512 ? len : 512;
  memcpy (buf, ptr, limited_len);

  /* Mess up system entropy source. Return error if device is /dev/random. */
  if (!crypt_gen_random (buf, limited_len) && unit == RANDOM)
    {
      __seterrno ();
      return -1;
    }
  /* Mess up the pseudo random number generator. */
  pseudo_write (buf, limited_len);
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

int
fhandler_dev_random::read (void *ptr, size_t len)
{
  if (!len)
    return 0;
  if (!ptr)
    {
      set_errno (EINVAL);
      return -1;
    }

  if (crypt_gen_random (ptr, len))
    return len;
  /* If device is /dev/urandom, use pseudo number generator as fallback.
     Don't do this for /dev/random since it's intended for uses that need
     very high quality randomness. */
  if (unit == URANDOM)
    return pseudo_read (ptr, len);

  __seterrno ();
  return -1;
}

off_t
fhandler_dev_random::lseek (off_t, int)
{
  return 0;
}

int
fhandler_dev_random::close (void)
{
  if (crypt_prov)
    while (!CryptReleaseContext (crypt_prov, 0)
	   && GetLastError () == ERROR_BUSY)
      Sleep(10);
  return 0;
}

int
fhandler_dev_random::dup (fhandler_base *child)
{
  fhandler_dev_random *fhr = (fhandler_dev_random *) child;
  fhr->unit = unit;
  fhr->crypt_prov = (HCRYPTPROV)NULL;
  return 0;
}

void
fhandler_dev_random::dump ()
{
  paranoid_printf("here, fhandler_dev_random");
}

