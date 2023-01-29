/* cygcheck.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

struct pkgver
{
  char *name;
  char *ver;
};

extern pkgver *get_installed_packages (char **, size_t * = NULL);
