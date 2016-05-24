/* setpwd.cc: Set LSA private data password for current user.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include <ntsecapi.h>
#include <ntdef.h>
#include "ntdll.h"

#include "cygserver.h"
#include "process.h"
#include "transport.h"

#include "cygserver_setpwd.h"

client_request_setpwd::client_request_setpwd ()
  : client_request (CYGSERVER_REQUEST_SETPWD,
		    &_parameters, sizeof (_parameters))
{ 
}

void
client_request_setpwd::serve (transport_layer_base *const conn,
			      process_cache *const cache)
{
  HANDLE tok;
  PTOKEN_USER user;
  WCHAR sidbuf[128], key_name [128 + wcslen (CYGWIN_LSA_KEY_PREFIX)];
  UNICODE_STRING sid, key, data;

  syscall_printf ("Request to set private data");
  if (msglen () != sizeof (_parameters.in))
    {
      syscall_printf ("bad request body length: expecting %lu bytes, got %lu",
		      sizeof (_parameters), msglen ());
      error_code (EINVAL);
      msglen (0);
      return;
    }
  msglen (0);
  if (!conn->impersonate_client ())
    {
      error_code (EACCES);
      return;
    }
  if (!OpenThreadToken (GetCurrentThread (), TOKEN_READ, TRUE, &tok))
    {
      conn->revert_to_self ();
      error_code (EACCES);
      return;
    }
  /* Get uid from user SID in token. */
  user = (PTOKEN_USER) get_token_info (tok, TokenUser);
  CloseHandle (tok);
  conn->revert_to_self ();
  if (!user)
    {
      error_code (EACCES);
      return;
    }
  LSA_OBJECT_ATTRIBUTES oa = { 0, 0, 0, 0, 0, 0 };
  HANDLE lsa;
  NTSTATUS status = LsaOpenPolicy (NULL, &oa, POLICY_CREATE_SECRET, &lsa);
  if (!NT_SUCCESS (status))
    {
      error_code (LsaNtStatusToWinError (status));
      return;
    }
  RtlInitEmptyUnicodeString (&sid, sidbuf, sizeof sidbuf);
  RtlConvertSidToUnicodeString (&sid, user->User.Sid, FALSE);
  free (user);
  RtlInitEmptyUnicodeString (&key, key_name, sizeof key_name);
  RtlAppendUnicodeToString (&key, CYGWIN_LSA_KEY_PREFIX);
  RtlAppendUnicodeStringToString (&key, &sid);
  RtlInitUnicodeString (&data, _parameters.in.passwd);
  status = LsaStorePrivateData (lsa, &key, data.Length ? &data : NULL);
  if (data.Length)
    RtlSecureZeroMemory (data.Buffer, data.Length);
  /* Success or we're trying to remove a password entry which doesn't exist. */
  if (NT_SUCCESS (status)
      || (data.Length == 0 && status == STATUS_OBJECT_NAME_NOT_FOUND))
    error_code (0);
  else
    error_code (LsaNtStatusToWinError (status));
  syscall_printf ("Request to set private data returns error %d", error_code ());
  LsaClose (lsa);
}
#endif /* __OUTSIDE_CYGWIN__ */
