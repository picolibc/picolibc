/* cygwin_version.h: shared info for cygwin

   Copyright 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <cygwin/version.h>

extern "C" {
/* This is for programs that want to access the shared data. */
class shared_info *cygwin_getshared (void);

struct cygwin_version_info
{
  unsigned short api_major;
  unsigned short api_minor;
  unsigned short dll_major;
  unsigned short dll_minor;
  unsigned short shared_data;
  unsigned short mount_registry;
  const char *dll_build_date;
  char shared_id[sizeof (CYGWIN_VERSION_DLL_IDENTIFIER) + 64];
};

extern cygwin_version_info cygwin_version;
extern const char *cygwin_version_strings;
}
