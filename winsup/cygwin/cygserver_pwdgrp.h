/* cygserver_pwdgrp.h: Request account information

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_PWDGRP_H__
#define __CYGSERVER_PWDGRP_H__

#include <sys/types.h>
#include "cygserver.h"
#include "userinfo.h"

class transport_layer_base;
class process_cache;

class client_request_pwdgrp : public client_request
{
  friend class client_request;

private:
  union _pwdgrp_param_t
  {
    struct _pwdgrp_in_t
    {
      bool group;
      fetch_user_arg_type_t type;
      union
      {
      	BYTE sid[40];
	char name[UNLEN + 1];
	uint32_t id;
      } arg;
    } in;

    struct
    {
      char line[1024];
    } out;
  } _parameters;

#ifndef __INSIDE_CYGWIN__
  client_request_pwdgrp ();
  virtual void serve (transport_layer_base *, process_cache *);
  void pwd_serve ();
  void grp_serve ();
#endif

public:

#ifdef __INSIDE_CYGWIN__
  client_request_pwdgrp (fetch_user_arg_t &arg, bool group);
#endif

  const char *line () const { return (msglen () > 0) ? _parameters.out.line
						     : NULL; }
};

#endif /* __CYGSERVER_PWDGRP_H__ */
