/* setlsapwd.cc: Set LSA private data password for current user.

   Copyright 2008, 2009, 2011, 2014 Red Hat, Inc.

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

unsigned long
setlsapwd (const char *passwd, const char *username)
{
  unsigned long ret = (unsigned long) -1;
  HANDLE lsa;
  WCHAR sid[128];
  WCHAR key_name[128 + wcslen (CYGWIN_LSA_KEY_PREFIX)];
  PWCHAR data_buf = NULL;
  UNICODE_STRING key;
  UNICODE_STRING data;

  if (username)
    {
      cygsid psid;
      struct passwd *pw = internal_getpwnam (username);

      if (!pw || !psid.getfrompw (pw))
	{
	  set_errno (ENOENT);
	  return ret;
	}
      wcpcpy (wcpcpy (key_name, CYGWIN_LSA_KEY_PREFIX), psid.string (sid));
    }
  else
    wcpcpy (wcpcpy (key_name, CYGWIN_LSA_KEY_PREFIX),
	    cygheap->user.get_windows_id (sid));
  RtlInitUnicodeString (&key, key_name);
  if (!passwd || ! *passwd
      || sys_mbstowcs_alloc (&data_buf, HEAP_NOTHEAP, passwd))
    {
      memset (&data, 0, sizeof data);
      if (data_buf)
	RtlInitUnicodeString (&data, data_buf);
      /* First try it locally.  Works for admin accounts. */
      if ((lsa = lsa_open_policy (NULL, POLICY_CREATE_SECRET)))
	{
	  NTSTATUS status = LsaStorePrivateData (lsa, &key,
						 data.Length ? &data : NULL);
	  /* Success or we're trying to remove a password entry which doesn't
	     exist. */
	  if (NT_SUCCESS (status)
	      || (data.Length == 0 && status == STATUS_OBJECT_NAME_NOT_FOUND))
	    ret = 0;
	  else
	    __seterrno_from_nt_status (status);
	  lsa_close_policy (lsa);
	}
      else if (ret && !username)
	{
	  client_request_setpwd request (&data);
	  if (request.make_request () == -1 || request.error_code ())
	    set_errno (request.error_code ());
	  else
	    ret = 0;
	}
      if (data_buf)
	{
	  RtlSecureZeroMemory (data.Buffer, data.Length);
	  free (data_buf);
	}
    }
  return ret;
}
