/* callout.h -- Definitions used by Cygwin callout mechanism

   Copyright 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

extern "C" {
enum cw_callout_t
{
  CO_ENV,
  CO_SPAWN,
  CO_SYMLINK,
  CO_UNAME
};

enum cw_callout_return_t
{
  CO_R_ERROR,
  CO_R_KEEP_GOING,
  CO_R_SHORT_CIRCUIT
};

typedef cw_callout_return_t (*cw_callout_function_t) (cw_callout_t, ...);
};
