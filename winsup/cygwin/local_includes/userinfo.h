/* userinfo.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

enum fetch_user_arg_type_t {
  SID_arg,
  NAME_arg,
  ID_arg,
  FULL_acc_arg,
};

#ifdef __INSIDE_CYGWIN__

struct fetch_acc_t {
  cygpsid sid;
  PUNICODE_STRING name;
  PUNICODE_STRING dom;
  SID_NAME_USE acc_type;
};

struct fetch_user_arg_t
{
  fetch_user_arg_type_t type;
  union {
    cygpsid *sid;
    const char *name;
    uint32_t id;
    fetch_acc_t *full_acc;
  };
  /* Only used in fetch_account_from_file/line. */
  size_t len;
};

#endif
