/* fhandler_virtual.h: Header for virtual fhandlers

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

struct virt_tab_t {
  const char *name;
  size_t name_len;
  fh_devices fhandler;
  virtual_ftype_t type;
  off_t (*format_func)(void *data, char *&);
};

#define _VN(s)	s, sizeof (s) - 1

extern virt_tab_t *virt_tab_search (const char *, bool, const virt_tab_t *,
				    size_t);

static inline unsigned char
virt_ftype_to_dtype (virtual_ftype_t type)
{
  unsigned char d_type;

  switch (type) 
    {
    case virt_directory:
      d_type = DT_DIR;
      break;
    case virt_symlink:
      d_type = DT_LNK;
      break;
    case virt_file:
      d_type = DT_REG;
      break;
    default:
      d_type = DT_UNKNOWN;
      break;
    }
  return d_type;
}
