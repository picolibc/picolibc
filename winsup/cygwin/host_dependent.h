/* host_dependent.h: host dependent Cygwin header file.

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Portions of the cygwin DLL require special constants whose values
   are dependent on the host system.  Rather than dynamically
   determine those values whenever they are required, initialize these
   values once at process start-up. */

class host_dependent_constants
{
 public:
  void init (void);

  /* Used by fhandler_disk_file::lock which needs a platform-specific
     upper word value for locking entire files. */
  DWORD win32_upper;

  /* fhandler_base::open requires host dependent file sharing
     attributes. */
  int shared;
};

extern host_dependent_constants host_dependent;
