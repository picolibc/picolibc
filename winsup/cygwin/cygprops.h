/* cygprops.h: Cygwin DLL properties

   Copyright 2009, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once
/* DLL properties data. */
struct cygwin_props_t
{
  char magic[68];
  ULONG size;
  ULONG disable_key;
};

#define CYGWIN_PROPS_MAGIC \
  "Fortunately, I keep my feathers numbered for just such an emergency"
