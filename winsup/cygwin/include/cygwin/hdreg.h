/* cygwin/hdreg.h

   Copyright 2002 Red Hat Inc.
   Written by Chris January <chris@atomice.net>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_HDREG_H_
#define _CYGWIN_HDREG_H_

struct hd_geometry {
  unsigned char heads;
  unsigned char sectors;
  unsigned short cylinders;
  unsigned long start;
};

#define HDIO_GETGEO                     0x301

#endif
