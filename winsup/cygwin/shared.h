/* shared.h: shared info for cygwin

   Copyright 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/******** Functions declarations for use in methods below ********/

/* Printf type functions */
extern "C" void __api_fatal (const char *, ...) __attribute__ ((noreturn));
extern "C" int __small_sprintf (char *dst, const char *fmt, ...);
extern "C" int __small_vsprintf (char *dst, const char *fmt, va_list ap);

/******** Deletion Queue Class ********/

/* First pass at a file deletion queue structure.

   We can't keep this list in the per-process info, since
   one process may open a file, and outlive a process which
   wanted to unlink the file - and the data would go away.

   Perhaps the FILE_FLAG_DELETE_ON_CLOSE would be ok,
   but brief experimentation didn't get too far.
*/

#define MAX_DELQUEUES_PENDING 100

class delqueue_list
{
  char name[MAX_DELQUEUES_PENDING][MAX_PATH];
  char inuse[MAX_DELQUEUES_PENDING];
  int empty;

public:
  void init ();
  void queue_file (const char *dosname);
  void process_queue ();
};

/* non-NULL if this process is a child of a cygwin process */
extern HANDLE parent_alive;

/******** Registry Access ********/

class reg_key
{
private:

  HKEY key;
  LONG key_is_invalid;

public:

  reg_key (HKEY toplev, REGSAM access, ...);
  reg_key (REGSAM access, ...);
  reg_key (REGSAM access = KEY_ALL_ACCESS);

  void *operator new (size_t, void *p) {return p;}
  void build_reg (HKEY key, REGSAM access, va_list av);

  int error () {return key == (HKEY) INVALID_HANDLE_VALUE;}

  int kill (const char *child);
  int killvalue (const char *name);

  HKEY get_key ();
  int get_int (const char *,int def);
  int get_string (const char *, char *buf, size_t len, const char *def);
  int set_string (const char *,const char *);
  int set_int (const char *, int val);

  ~reg_key ();
};

/* Evaluates path to the directory of the local user registry hive */
char *__stdcall get_registry_hive_path (const PSID psid, char *path);
void __stdcall load_registry_hive (PSID psid);


/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__stdcall sec_user (PVOID sa_buf, PSID sid2 = NULL, BOOL inherit = TRUE);
extern SECURITY_ATTRIBUTES *__stdcall sec_user_nih (PVOID sa_buf, PSID sid2 = NULL);

extern int __stdcall set_console_state_for_spawn ();

extern "C" {
/* This is for programs that want to access the shared data. */
class shared_info *cygwin_getshared (void);

struct cygwin_version_info
{
  unsigned short api_major;
  unsigned short api_minor;
  unsigned short dll_major;
  unsigned short dll_minor;
  unsigned short shared_data;
  unsigned short mount_registry;
  const char *dll_build_date;
  char shared_id[sizeof (CYGWIN_VERSION_DLL_IDENTIFIER) + 64];
};
}

extern cygwin_version_info cygwin_version;
extern const char *cygwin_version_strings;
