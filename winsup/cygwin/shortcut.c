/* shortcut.c: Read shortcuts. This part of the code must be in C because
               the C++ interface to COM doesn't work without -fvtable-thunk
	       which is too dangerous to use.

   Copyright 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <shlobj.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>
#include "shortcut.h"

/* This is needed to avoid including path.h which is a pure C++ header. */
#define PATH_SYMLINK MOUNT_SYMLINK

char shortcut_header[SHORTCUT_HDR_SIZE];
BOOL shortcut_initalized = FALSE;

void
create_shortcut_header (void)
{
  if (!shortcut_initalized)
    {
      shortcut_header[0] = 'L';
      shortcut_header[4] = '\001';
      shortcut_header[5] = '\024';
      shortcut_header[6] = '\002';
      shortcut_header[12] = '\300';
      shortcut_header[19] = 'F';
      shortcut_header[20] = '\f';
      shortcut_header[60] = '\001';
      shortcut_initalized = TRUE;
    }
}

static BOOL
cmp_shortcut_header (const char *file_header)
{
  create_shortcut_header ();
  return memcmp (shortcut_header, file_header, SHORTCUT_HDR_SIZE);
}

int
check_shortcut (const char *path, DWORD fileattr, HANDLE h,
		char *contents, int *error, unsigned *pflags)
{
  HRESULT hres;
  IShellLink *psl = NULL;
  IPersistFile *ppf = NULL;
  WCHAR wc_path[MAX_PATH];
  char full_path[MAX_PATH];
  WIN32_FIND_DATA wfd;
  DWORD len = 0;
  int res = 0;

  /* Initialize COM library. */
  CoInitialize (NULL);

  /* Get a pointer to the IShellLink interface. */
  hres = CoCreateInstance (&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			   &IID_IShellLink, (void **)&psl);
  if (FAILED (hres))
    goto close_it;
  /* Get a pointer to the IPersistFile interface. */
  hres = psl->lpVtbl->QueryInterface (psl, &IID_IPersistFile, (void **)&ppf);
  if (FAILED (hres))
    goto close_it;
  /* Load the shortcut. */
  MultiByteToWideChar(CP_ACP, 0, path, -1, wc_path, MAX_PATH);
  hres = ppf->lpVtbl->Load (ppf, wc_path, STGM_READ);
  if (FAILED (hres))
    goto close_it;
  /* Try the description (containing a POSIX path) first. */
  if (fileattr & FILE_ATTRIBUTE_READONLY)
    {
      /* An additional check is needed to prove if it's a shortcut
         really created by Cygwin or U/WIN. */
      char file_header[SHORTCUT_HDR_SIZE];
      DWORD got;

      if (! ReadFile (h, file_header, SHORTCUT_HDR_SIZE, &got, 0))
	{
          *error = EIO;
	  goto close_it;
	}
      if (got == SHORTCUT_HDR_SIZE && !cmp_shortcut_header (file_header))
        {
	  hres = psl->lpVtbl->GetDescription (psl, contents, MAX_PATH);
	  if (FAILED (hres))
	    goto close_it;
	  len = strlen (contents);
	}
    }
  /* No description or not R/O: Check the "official" path. */
  if (len == 0)
    {
      /* Convert to full path (easy way) */
      if ((path[0] == '\\' && path[1] == '\\')
	  || (_toupper (path[0]) >= 'A' && _toupper (path[0]) <= 'Z'
	      && path[1] == ':'))
	len = 0;
      else
	{
	  len = GetCurrentDirectory (MAX_PATH, full_path);
	  if (path[0] == '\\')
	    len = 2;
	  else if (full_path[len - 1] != '\\')
	    strcpy (full_path + len++, "\\");
	}
      strcpy (full_path + len, path);
      /* Set relative path inside of IShellLink interface. */
      hres = psl->lpVtbl->SetRelativePath (psl, full_path, 0);
      if (FAILED (hres))
	goto close_it;
      /* Get the path to the shortcut target. */
      hres = psl->lpVtbl->GetPath (psl, contents, MAX_PATH, &wfd, 0);
      if (FAILED(hres))
	goto close_it;
    }
  res = strlen (contents);
  if (res) /* It's a symlink.  */
    *pflags = PATH_SYMLINK;

close_it:
  /* Release the pointer to IPersistFile. */
  if (ppf)
    ppf->lpVtbl->Release(ppf);
  /* Release the pointer to IShellLink. */
  if (psl)
    psl->lpVtbl->Release(psl);
  /* Uninitialize COM library. */
  CoUninitialize ();

  return res;
}


