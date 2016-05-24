/* pwdgrp.cc: Request account information

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/cygwin.h>

#include "cygserver.h"
#include "process.h"
#include "transport.h"

#include "cygserver_pwdgrp.h"

#include <sddl.h>

client_request_pwdgrp::client_request_pwdgrp ()
  : client_request (CYGSERVER_REQUEST_PWDGRP,
		    &_parameters, sizeof (_parameters))
{ 
}

void
client_request_pwdgrp::pwd_serve ()
{
  struct passwd *pwd = NULL;

  switch (_parameters.in.type)
    {
    case SID_arg:
      pwd = (struct passwd *) cygwin_internal (CW_GETPWSID, 0,
					      &_parameters.in.arg.sid);
      break;
    case NAME_arg:
      pwd = getpwnam (_parameters.in.arg.name);
      break;
    case ID_arg:
      pwd = getpwuid (_parameters.in.arg.id);
      break;
    default:
      break;
    }
  if (pwd)
    msglen (snprintf (_parameters.out.line, sizeof _parameters.out.line,
		      "%s:%s:%u:%u:%s:%s:%s",
		      pwd->pw_name ?: "",
		      pwd->pw_passwd ?: "",
		      (uint32_t) pwd->pw_uid,
		      (uint32_t) pwd->pw_gid,
		      pwd->pw_gecos ?: "",
		      pwd->pw_dir ?: "",
		      pwd->pw_shell ?: "") + 1);
  else
    {
      switch (_parameters.in.type)
	{
	case SID_arg:
	  {
	    char *str;
	    if (ConvertSidToStringSid (&_parameters.in.arg.sid, &str))
	      {
		debug_printf ("User <%s> failed", str);
		LocalFree (str);
	      }
	  }
	  break;
	case NAME_arg:
	  debug_printf ("User <%s> failed", _parameters.in.arg.name);
	  break;
	case ID_arg:
	  debug_printf ("User <%u> failed", _parameters.in.arg.id);
	  break;
	default:
	  break;
	}
      _parameters.out.line[0] = '\0';
      msglen (0);
      error_code (ENOENT);
    }
}

void
client_request_pwdgrp::grp_serve ()
{
  struct group *grp = NULL;

  switch (_parameters.in.type)
    {
    case SID_arg:
      grp = (struct group *) cygwin_internal (CW_GETGRSID, 0,
					      &_parameters.in.arg.sid);
      break;
    case NAME_arg:
      grp = getgrnam (_parameters.in.arg.name);
      break;
    case ID_arg:
      grp = getgrgid (_parameters.in.arg.id);
      break;
    default:
      break;
    }
  if (grp)
    msglen (snprintf (_parameters.out.line, sizeof _parameters.out.line,
		      "%s:%s:%u:",
		      grp->gr_name ?: "",
		      grp->gr_passwd ?: "",
		      (uint32_t) grp->gr_gid) + 1);
  else
    {
      switch (_parameters.in.type)
	{
	case SID_arg:
	  {
	    char *str;
	    if (ConvertSidToStringSid (&_parameters.in.arg.sid, &str))
	      {
		debug_printf ("Group <%s> failed", str);
		LocalFree (str);
	      }
	  }
	  break;
	case NAME_arg:
	  debug_printf ("Group <%s> failed", _parameters.in.arg.name);
	  break;
	case ID_arg:
	  debug_printf ("Group <%u> failed", _parameters.in.arg.id);
	  break;
	default:
	  break;
	}
      _parameters.out.line[0] = '\0';
      msglen (0);
      error_code (ENOENT);
    }
}

void
client_request_pwdgrp::serve (transport_layer_base *const conn,
			      process_cache *const cache)
{
  debug_printf ("Request account information");
  if (msglen () < __builtin_offsetof (struct _pwdgrp_param_t::_pwdgrp_in_t, arg)
		  + sizeof (uint32_t)
      || msglen () > sizeof (_parameters.in))
    {
      syscall_printf ("bad request body length: got %lu", msglen ());
      error_code (EINVAL);
      msglen (0);
      return;
    }
  error_code (0);
  if (_parameters.in.group)
    grp_serve ();
  else
    pwd_serve ();
  debug_printf ("Request account information returns <%s> error %d", _parameters.out.line, error_code ());
}
#endif /* __OUTSIDE_CYGWIN__ */
