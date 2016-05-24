/* cygserver_setpwd.h: Set LSA private data password for current user.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_SETPWD_H__
#define __CYGSERVER_SETPWD_H__

#include <sys/types.h>
#include "cygserver.h"

#define CYGWIN_LSA_KEY_PREFIX	L"L$CYGWIN_"

#ifndef __INSIDE_CYGWIN__
class transport_layer_base;
class process_cache;
#endif

class client_request_setpwd : public client_request
{
  friend class client_request;

private:
  union
  {
    struct
    {
      WCHAR passwd[256];
    } in;
  } _parameters;

#ifndef __INSIDE_CYGWIN__
  client_request_setpwd ();
  virtual void serve (transport_layer_base *, process_cache *);
#endif

public:

#ifdef __INSIDE_CYGWIN__
  client_request_setpwd (PUNICODE_STRING);
#endif
};

#ifdef __INSIDE_CYGWIN__
unsigned long setlsapwd (const char *passwd, const char *username);
#endif

#endif /* __CYGSERVER_SETPWD_H__ */
