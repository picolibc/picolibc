/* winlean.h - Standard "lean" windows include

   Copyright 2010  Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _WINLEAN_H
#define _WINLEAN_H 1
#define WIN32_LEAN_AND_MEAN 1
#define _WINGDI_H
#define _WINUSER_H
#define _WINNLS_H
#define _WINVER_H
#define _WINNETWK_H
#define _WINSVC_H
#include <windows.h>
#include <wincrypt.h>
#include <lmcons.h>
#include <ntdef.h>
#undef _WINGDI_H
#undef _WINUSER_H
#undef _WINNLS_H
#undef _WINVER_H
#undef _WINNETWK_H
#undef _WINSVC_H
#endif /*_WINLEAN_H*/
