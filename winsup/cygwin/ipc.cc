/* ipc.cc: Single unix specification IPC interface for Cygwin

   Copyright 2001, 2002, 2003 Red Hat, Inc.

   Originally written by Robert Collins <robert.collins@hotmail.com>
   Updated to 64 bit key_t by Charles Wilson <cygwin@cwilson.fastmail.fm>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <sys/stat.h>

/* Notes: we return a valid key even if id's low order 8 bits are 0. */
extern "C" key_t
ftok (const char *path, int id)
{
  struct __stat64 statbuf;
  key_t tmp;
  if (stat64 (path, &statbuf))
    {
      /* stat set the appropriate errno for us */
      return (key_t) -1;
    }

  /* Since __CYGWIN_USE_BIG_TYPES__,
     dev_t is 32bits for cygwin
     ino_t is 64bits for cygwin
     and we need 8 bits for the id.
     thus key_t needs 104 bits total -- but we only have 64 (long long)
     We will have to alias; leaving open the possibility that the same
     key will be returned for multiple files.  This possibility exists
     also on Linux; the question is, how to minimize this possibility.

     How to solve?  Well, based on C. Vinschen's research, the nFileIndex*
     words vary as follows, on a partition with > 110,000 files
     nFileIndexHigh:    564 values between 0x00010000 -- 0xffff0000
     nFileIndexLow : 103812 values between 0x00000000 -- 0x0003ffff
     R. Collins suggests that these may represent a tree path,
     and that it would require ~2.9M files to force the tree depth
     to increase and reveal more bit usage.

     Implementation details: dev_t is 32bits, but is formed by
	device(32bits) << 16 | unit(32bits)
     But device is ACTUALLY == status & FH_DEVMASK, where FH_DEVMASK
     is 0x00000fff --> 12 bits

     As it happens, the maximum number of devices is actually
     FH_NDEV, not FH_DEVMASK, where FH_NDEV is currently 0x0000001d.
     However, FH_NDEV grows as new device types are added.  So
     currently the device number needs 5 bits, but later?  Let's
     take a cue from Linux, and use the lower 8 bits (instead of the
     lower 12 or 16) for the device (major?) number.

     Similarly, while 'units' is an int (32bits), it is unclear
     how many of these are significant. For most devices, it seems that
     'units' is equivalent to 'minor'.  For FH_TAPE, it's obvious that
     only 8 bits are important.  However, for FH_SOCKET...it might be
     as high as 16 significant bits.

     Let's assume that we only need 8 bits from device (major) and
     only 8 bits from unit (minor). (On linux, only 8 bits of minor
     are used, and none from major).
     ---> so, we only need 0x00ff00ff (16 bits) of dev_t

     ---> we MUST have all 8 bits of id.

     ---> So, we only have 64 - 8 - 16 = 40 bits for ino_t.  But, we
     need 0xffff0000 for nFileIndexHigh and 0x0003ffff for nFileIndexLow
     minimum, or 16 + 18 = 34 bits.  Lucky us - we have 6 more bits
     to distribute.

     For lack of a better idea, we'll allocate 2 of the extra bits to
     nFileIndexHigh and 4 to nFileIndexLow.  */

  /* get 8 bits from dev_t (major), put into 0xff00000000000000L */
  tmp  = (((key_t) statbuf.st_dev) & 0x0000000000ff0000LL) << 40;
  /* get 8 bits from dev_t (minor), put into 0x00ff000000000000L */
  tmp |= (((key_t) statbuf.st_dev) & 0x00000000000000ffLL) << 48;
  /* get upper 16+2 bits from nFileInfoHigh, put into 0x0000ffffc0000000L
     shift down first, then mask, to avoid sign extension on rightshift  */
  tmp |= (((key_t) statbuf.st_ino) & 0xffffc00000000000LL) >> 16;
  /* get lower 18+4 bits from nFileInfoLow, put into  0x000000003fffff00L  */
  tmp |= (((key_t) statbuf.st_ino) & 0x00000000003fffffLL) << 8;
  /* use all 8 bits of id, and put into 0x00000000000000ffL */
  tmp |= (id & 0x00ff);
  return tmp;
}
