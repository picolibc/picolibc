/* cygsuba.c: Minimal subauthentication functionality to support
              logon without password.

   Copyright 2001 Red Hat, Inc.

Written by Corinna Vinschen <vinschen@redhat.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>
#include <subauth.h>
#include <ntsecapi.h>

NTSTATUS NTAPI
Msv1_0SubAuthenticationRoutine (NETLOGON_LOGON_INFO_CLASS logon_level,
				VOID *logon_inf,
				ULONG flags,
				USER_ALL_INFORMATION *usr_inf,
				ULONG *which,
				ULONG *usr_flags,
				BOOLEAN *auth,
				LARGE_INTEGER *logoff,
				LARGE_INTEGER *kickoff)
{
  ULONG valid_account = USER_NORMAL_ACCOUNT;
  if (!(flags & MSV1_0_PASSTHRU))
    valid_account |= USER_TEMP_DUPLICATE_ACCOUNT;

  *which = *usr_flags = 0;

  /* Not a Network logon? 
     TODO: How do I manage an interactive logon using a subauthentication
     package??? The logon_level "interactive" is available but I never
     got it working. I assume that's the reason I don't get a legal
     logon session so that I can connect to network drives. */
  if (logon_level != NetlogonNetworkInformation)
    {
      *auth = TRUE;
      return STATUS_INVALID_INFO_CLASS;
    }

  /* Account type ok? */
  if (!(usr_inf->UserAccountControl & valid_account))
    {
      *auth = FALSE;
      return STATUS_NO_SUCH_USER;
    }

  /* Guest logon? */
  if (flags & MSV1_0_GUEST_LOGON)
    *usr_flags = LOGON_GUEST;

#if defined (SSHD)
  /* The same code could be used to allow the DLL checking for
     SSH RSA/DSA keys. For that purpose, SSH would need it's
     own implementation with the below field used to transport
     the keys which have to be checked. This could be used to
     allow secure logon with RSA/DSA instead of passwords.
     Of course that needs lots of additions to the code... */
  {
    PNETLOGON_NETWORK_INFO nw_inf = (PNETLOGON_NETWORK_INFO) logon_inf;

    /*
        nw_inf->LmChallenge.data <=>
			MSV1_0_LM20_LOGON::ChallengeToClient
        nw_inf->NtChallengeResponse <=>
			MSV1_0_LM20_LOGON::CaseSensitiveChallengeResponse
        nw_inf->LmChallengeResponse <=>
			MSV1_0_LM20_LOGON::CaseInsensitiveChallengeResponse
    */
    if (authentication_failed)
      {
        *auth = (usr_inf->UserAccountControl & USER_ACCOUNT_DISABLED) ?
		         FALSE : TRUE;
        return STATUS_WRONG_PASSWORD;
      }
  }
#endif

  /* All accounts except for the local admin are checked for being
     locked out or disabled or expired. */
  if (usr_inf->UserId != DOMAIN_USER_RID_ADMIN)
    {
      SYSTEMTIME CurrentTime;
      LARGE_INTEGER LogonTime;

      /* Account locked out? */
      if (usr_inf->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED)
	{
	  *auth = (usr_inf->UserAccountControl & USER_ACCOUNT_DISABLED) ?
			   FALSE : TRUE;
	  return STATUS_ACCOUNT_LOCKED_OUT;
	}

      /* Account disabled? */
      if (usr_inf->UserAccountControl & USER_ACCOUNT_DISABLED)
        {
          *auth = FALSE;
          return STATUS_ACCOUNT_DISABLED;
        }

      /* Account expired? */
      GetSystemTime (&CurrentTime);
      SystemTimeToFileTime(&CurrentTime, (LPFILETIME) &LogonTime);
      if (usr_inf->AccountExpires.QuadPart &&
          LogonTime.QuadPart >= usr_inf->AccountExpires.QuadPart)
	{
          *auth = TRUE;
          return STATUS_ACCOUNT_EXPIRED;
        }
    }

  /* Don't force logout. */
  logoff->HighPart = 0x7FFFFFFF;
  logoff->LowPart = 0xFFFFFFFF;
  kickoff->HighPart = 0x7FFFFFFF;
  kickoff->LowPart = 0xFFFFFFFF;

  *auth = TRUE;
  return STATUS_SUCCESS;
}

NTSTATUS NTAPI
Msv1_0SubAuthenticationFilter (NETLOGON_LOGON_INFO_CLASS logon_level,
			       VOID *logon_inf,
			       ULONG flags,
			       USER_ALL_INFORMATION *usr_inf,
			       ULONG *which,
			       ULONG *usr_flags,
			       BOOLEAN *auth,
			       LARGE_INTEGER *logoff,
			       LARGE_INTEGER *kickoff)
{
  return Msv1_0SubAuthenticationRoutine (logon_level, logon_inf, flags,
  					 usr_inf, which, usr_flags,
					 auth, logoff, kickoff);
}
