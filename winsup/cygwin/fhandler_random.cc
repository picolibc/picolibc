/* fhandler_dev_random.cc: code to access /dev/random

   Copyright 2000 Cygnus Solutions.

   Written by Corinna Vinschen (vinschen@cygnus.com)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include "winsup.h"

#define RANDOM   8
#define URANDOM  9

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

int
fhandler_dev_random::write (const void *, size_t len)
{
  return len;
}

int
fhandler_dev_random::read (void *ptr, size_t len)
{
  if (!len)
    return 0;
  if (!crypt_prov
      && !CryptAcquireContext (&crypt_prov, NULL, MS_DEF_PROV,
                            PROV_RSA_FULL, 0)
      && !CryptAcquireContext (&crypt_prov, NULL, MS_DEF_PROV,
                               PROV_RSA_FULL, CRYPT_NEWKEYSET))
    {
      __seterrno ();
      return -1;
    }
  if (!CryptGenRandom (crypt_prov, len, (BYTE *)ptr))
    {
      __seterrno ();
      return -1;
    }
  return len;
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

