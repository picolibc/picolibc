/* setlsapwd.cc: Set LSA private data password for current user.

   Copyright 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "shared_info.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "security.h"
#include "cygserver_setpwd.h"
#include "ntdll.h"
#include <ntsecapi.h>
#include <stdlib.h>
#include <wchar.h>

#ifdef USE_SERVER
/*
 * client_request_setpwd Constructor
 */

client_request_setpwd::client_request_setpwd (PUNICODE_STRING passwd)
  : client_request (CYGSERVER_REQUEST_SETPWD, &_parameters, sizeof (_parameters))
{
  memset (_parameters.in.passwd, 0, sizeof _parameters.in.passwd);
  if (passwd->Length > 0 && passwd->Length < 256 * sizeof (WCHAR))
    wcpncpy (_parameters.in.passwd, passwd->Buffer, 255);

  msglen (sizeof (_parameters.in));
}

#endif /* USE_SERVER */

unsigned long
setlsapwd (const char *passwd)
{
  unsigned long ret = (unsigned long) -1;
  HANDLE lsa = INVALID_HANDLE_VALUE;
  WCHAR sid[128];
  WCHAR key_name[128 + wcslen (CYGWIN_LSA_KEY_PREFIX)];
  PWCHAR data_buf = NULL;
  UNICODE_STRING key;
  UNICODE_STRING data;

  wcpcpy (wcpcpy (key_name, CYGWIN_LSA_KEY_PREFIX),
	  cygheap->user.get_windows_id (sid));
  RtlInitUnicodeString (&key, key_name);
  if (!passwd || ! *passwd
      || sys_mbstowcs_alloc (&data_buf, HEAP_NOTHEAP, passwd))
    {
      NTSTATUS status = STATUS_ACCESS_DENIED;

      memset (&data, 0, sizeof data);
      if (data_buf)
	RtlInitUnicodeString (&data, data_buf);
      /* First try it locally.  Works for admin accounts. */
      if ((lsa = open_local_policy (POLICY_CREATE_SECRET))
	  != INVALID_HANDLE_VALUE)
	{
	  status = LsaStorePrivateData (lsa, &key, data.Length ? &data : NULL);
	  if (NT_SUCCESS (status))
	    ret = 0;
	  LsaClose (lsa);
	}
      if (ret)
#ifdef USE_SERVER
	{
	  /* If that fails, ask cygserver. */
	  client_request_setpwd request (&data);
	  if (request.make_request () == -1 || request.error_code ())
	    set_errno (request.error_code ());
	  else
	    ret = 0;
	}
#else
      __seterrno_from_nt_status (status);
#endif
      if (data_buf)
	free (data_buf);
    }
  return ret;
}
