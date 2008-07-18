/* environ.cc: Cygwin-adopted functions from newlib to manipulate
   process's environment.

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>
#include <assert.h>
#include <cygwin/version.h>
#include <winnls.h>
#include "pinfo.h"
#include "perprocess.h"
#include "path.h"
#include "cygerrno.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "registry.h"
#include "environ.h"
#include "child_info.h"

extern bool dos_file_warning;
extern bool allow_glob;
extern bool ignore_case_with_glob;
extern bool allow_winsymlinks;
extern bool strip_title_path;
bool reset_com = false;
static bool envcache = true;
#ifdef USE_SERVER
extern bool allow_server;
#endif

static char **lastenviron;

/* Helper functions for the below environment variables which have to
   be converted Win32<->POSIX. */
extern "C" ssize_t env_PATH_to_posix (const void *, void *, size_t);

ssize_t
env_plist_to_posix (const void *win32, void *posix, size_t size)
{
  return cygwin_conv_path_list (CCP_WIN_A_TO_POSIX | CCP_RELATIVE, win32,
				posix, size);
}

ssize_t
env_plist_to_win32 (const void *posix, void *win32, size_t size)
{
  return cygwin_conv_path_list (CCP_POSIX_TO_WIN_A | CCP_RELATIVE, posix,
				win32, size);
}

ssize_t
env_path_to_posix (const void *win32, void *posix, size_t size)
{
  return cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, win32,
			   posix, size);
}

ssize_t
env_path_to_win32 (const void *posix, void *win32, size_t size)
{
  return cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_ABSOLUTE, posix,
			   win32, size);
}

#define ENVMALLOC \
  (CYGWIN_VERSION_DLL_MAKE_COMBINED (user_data->api_major, user_data->api_minor) \
	  <= CYGWIN_VERSION_DLL_MALLOC_ENV)

#define NL(x) x, (sizeof (x) - 1)
/* List of names which are converted from dos to unix
   on the way in and back again on the way out.

   PATH needs to be here because CreateProcess uses it and gdb uses
   CreateProcess.  HOME is here because most shells use it and would be
   confused by Windows style path names.  */
static win_env conv_envvars[] =
  {
    {NL ("PATH="), NULL, NULL, env_PATH_to_posix, env_plist_to_win32, true},
    {NL ("HOME="), NULL, NULL, env_path_to_posix, env_path_to_win32, false},
    {NL ("LD_LIBRARY_PATH="), NULL, NULL,
			       env_plist_to_posix, env_plist_to_win32, true},
    {NL ("TMPDIR="), NULL, NULL, env_path_to_posix, env_path_to_win32, false},
    {NL ("TMP="), NULL, NULL, env_path_to_posix, env_path_to_win32, false},
    {NL ("TEMP="), NULL, NULL, env_path_to_posix, env_path_to_win32, false},
    {NULL, 0, NULL, NULL, 0, 0}
  };

static unsigned char conv_start_chars[256] = {0};

struct win_env&
win_env::operator = (struct win_env& x)
{
  name = x.name;
  namelen = x.namelen;
  toposix = x.toposix;
  towin32 = x.towin32;
  immediate = false;
  return *this;
}

win_env::~win_env ()
{
  if (posix)
    free (posix);
  if (native)
    free (native);
}

void
win_env::add_cache (const char *in_posix, const char *in_native)
{
  MALLOC_CHECK;
  posix = (char *) realloc (posix, strlen (in_posix) + 1);
  strcpy (posix, in_posix);
  if (in_native)
    {
      native = (char *) realloc (native, namelen + 1 + strlen (in_native));
      strcpy (native, name);
      strcpy (native + namelen, in_native);
    }
  else
    {
      tmp_pathbuf tp;
      char *buf = tp.c_get ();
      strcpy (buf, name + namelen);
      towin32 (in_posix, buf, NT_MAX_PATH);
      native = (char *) realloc (native, namelen + 1 + strlen (buf));
      strcpy (native, name);
      strcpy (native + namelen, buf);
    }
  MALLOC_CHECK;
  if (immediate && cygwin_finished_initializing)
    {
      char s[namelen];
      size_t n = namelen - 1;
      memcpy (s, name, n);
      s[n] = '\0';
      SetEnvironmentVariable (s, native + namelen);
    }
  debug_printf ("posix %s", posix);
  debug_printf ("native %s", native);
}


/* Check for a "special" environment variable name.  *env is the pointer
  to the beginning of the environment variable name.  *in_posix is any
  known posix value for the environment variable. Returns a pointer to
  the appropriate conversion structure.  */
win_env * __stdcall
getwinenv (const char *env, const char *in_posix, win_env *temp)
{
  if (!conv_start_chars[(unsigned char)*env])
    return NULL;

  for (int i = 0; conv_envvars[i].name != NULL; i++)
    if (strncmp (env, conv_envvars[i].name, conv_envvars[i].namelen) == 0)
      {
	win_env *we = conv_envvars + i;
	const char *val;
	if (!cur_environ () || !(val = in_posix ?: getenv (we->name)))
	  debug_printf ("can't set native for %s since no environ yet",
			we->name);
	else if (!envcache || !we->posix || strcmp (val, we->posix) != 0)
	  {
	    if (temp)
	      {
		*temp = *we;
		we = temp;
	      }
	    we->add_cache (val);
	  }
	return we;
      }
  return NULL;
}

/* Convert windows path specs to POSIX, if appropriate.
 */
static void __stdcall
posify (char **here, const char *value, char *outenv)
{
  char *src = *here;
  win_env *conv;

  if (!(conv = getwinenv (src)))
    return;

  int len = strcspn (src, "=") + 1;

  /* Turn all the items from c:<foo>;<bar> into their
     mounted equivalents - if there is one.  */

  memcpy (outenv, src, len);
  char *newvalue = outenv + len;
  if (!conv->toposix (value, newvalue, NT_MAX_PATH - len)
      || _impure_ptr->_errno != EIDRM)
    conv->add_cache (newvalue, *value != '/' ? value : NULL);
  else
    {
      /* The conversion routine removed elements from a path list so we have
	 to recalculate the windows path to remove elements there, too. */
      char cleanvalue[strlen (value) + 1];
      conv->towin32 (newvalue, cleanvalue, sizeof cleanvalue);
      conv->add_cache (newvalue, cleanvalue);
    }

  debug_printf ("env var converted to %s", outenv);
  *here = strdup (outenv);
  free (src);
  MALLOC_CHECK;
}

/* Returns pointer to value associated with name, if any, else NULL.
  Sets offset to be the offset of the name/value combination in the
  environment array, for use by setenv(3) and unsetenv(3).
  Explicitly removes '=' in argument name.  */

static char * __stdcall
my_findenv (const char *name, int *offset)
{
  register int len;
  register char **p;
  const char *c;

  c = name;
  len = 0;
  while (*c && *c != '=')
    {
      c++;
      len++;
    }

  for (p = cur_environ (); *p; ++p)
    if (!strncmp (*p, name, len))
      if (*(c = *p + len) == '=')
	{
	  *offset = p - cur_environ ();
	  return (char *) (++c);
	}
  MALLOC_CHECK;
  return NULL;
}

/* Primitive getenv before the environment is built.  */

static char __stdcall *
getearly (const char * name, int *)
{
  char *ret;
  char **ptr;
  int len;

  if (spawn_info && (ptr = spawn_info->moreinfo->envp))
    {
      len = strlen (name);
      for (; *ptr; ptr++)
	if (strncasematch (name, *ptr, len) && (*ptr)[len] == '=')
	  return *ptr + len + 1;
    }
  else if ((len = GetEnvironmentVariableA (name, NULL, 0))
	   && (ret = (char *) cmalloc_abort (HEAP_2_STR, len))
	   && GetEnvironmentVariableA (name, ret, len))
    return ret;

  return NULL;
}

static char * (*findenv_func)(const char *, int *) = (char * (*)(const char *, int *)) getearly;

/* Returns ptr to value associated with name, if any, else NULL.  */

extern "C" char *
getenv (const char *name)
{
  int offset;
  return findenv_func (name, &offset);
}

static int __stdcall
envsize (const char * const *in_envp)
{
  const char * const *envp;
  for (envp = in_envp; *envp; envp++)
    continue;
  return (1 + envp - in_envp) * sizeof (const char *);
}

/* Takes similar arguments to setenv except that overwrite is
   either -1, 0, or 1.  0 or 1 signify that the function should
   perform similarly to setenv.  Otherwise putenv is assumed. */
static int __stdcall
_addenv (const char *name, const char *value, int overwrite)
{
  int issetenv = overwrite >= 0;
  int offset;
  char *p;

  unsigned int valuelen = strlen (value);
  if ((p = my_findenv (name, &offset)))
    {				/* Already exists. */
      if (!overwrite)		/* Ok to overwrite? */
	return 0;		/* No.  Wanted to add new value.  FIXME: Right return value? */

      /* We've found the offset into environ.  If this is a setenv call and
	 there is room in the current environment entry then just overwrite it.
	 Otherwise handle this case below. */
      if (issetenv && strlen (p) >= valuelen)
	{
	  strcpy (p, value);
	  return 0;
	}
    }
  else
    {				/* Create new slot. */
      int sz = envsize (cur_environ ());
      int allocsz = sz + (2 * sizeof (char *));

      offset = (sz - 1) / sizeof (char *);

      /* Allocate space for additional element plus terminating NULL. */
      if (cur_environ () == lastenviron)
	lastenviron = __cygwin_environ = (char **) realloc (cur_environ (),
							    allocsz);
      else if ((lastenviron = (char **) malloc (allocsz)) != NULL)
	__cygwin_environ = (char **) memcpy ((char **) lastenviron,
					     __cygwin_environ, sz);

      if (!__cygwin_environ)
	{
#ifdef DEBUGGING
	  try_to_debug ();
#endif
	  return -1;				/* Oops.  No more memory. */
	}

      __cygwin_environ[offset + 1] = NULL;	/* NULL terminate. */
      update_envptrs ();	/* Update any local copies of 'environ'. */
    }

  char *envhere;
  if (!issetenv)
    /* Not setenv. Just overwrite existing. */
    envhere = cur_environ ()[offset] = (char *) (ENVMALLOC ? strdup (name) : name);
  else
    {				/* setenv */
      /* Look for an '=' in the name and ignore anything after that if found. */
      for (p = (char *) name; *p && *p != '='; p++)
	continue;

      int namelen = p - name;	/* Length of name. */
      /* Allocate enough space for name + '=' + value + '\0' */
      envhere = cur_environ ()[offset] = (char *) malloc (namelen + valuelen + 2);
      if (!envhere)
	return -1;		/* Oops.  No more memory. */

      /* Put name '=' value into current slot. */
      strncpy (envhere, name, namelen);
      envhere[namelen] = '=';
      strcpy (envhere + namelen + 1, value);
    }

  /* Update cygwin's cache, if appropriate */
  win_env *spenv;
  if ((spenv = getwinenv (envhere)))
    spenv->add_cache (value);

  MALLOC_CHECK;
  return 0;
}

/* Set an environment variable */
extern "C" int
putenv (char *str)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (*str)
    {
      char *eq = strchr (str, '=');
      if (eq)
	return _addenv (str, eq + 1, -1);

      /* Remove str from the environment. */
      unsetenv (str);
    }
  return 0;
}

/* Set the value of the environment variable "name" to be
   "value".  If overwrite is set, replace any current value.  */
extern "C" int
setenv (const char *name, const char *value, int overwrite)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (!*name)
    return 0;
  if (*value == '=')
    value++;
  return _addenv (name, value, !!overwrite);
}

/* Delete environment variable "name".  */
extern "C" int
unsetenv (const char *name)
{
  register char **e;
  int offset;
  myfault efault;
  if (efault.faulted () || *name == '\0' || strchr (name, '='))
    {
      set_errno (EINVAL);
      return -1;
    }

  while (my_findenv (name, &offset))	/* if set multiple times */
    /* Move up the rest of the array */
    for (e = cur_environ () + offset; ; e++)
      if (!(*e = *(e + 1)))
	break;

  return 0;
}

/* Turn environment variable part of a=b string into uppercase. */
static __inline__ void
ucenv (char *p, char *eq)
{
  /* Amazingly, NT has a case sensitive environment name list,
     but only sometimes.
     It's normal to have NT set your "Path" to something.
     Later, you set "PATH" to something else.  This alters "Path".
     But if you try and do a naive getenv on "PATH" you'll get nothing.

     So we upper case the labels here to prevent confusion later but
     we only do it for the first process in a session group. */
  for (; p < eq; p++)
    if (islower (*p))
      *p = cyg_toupper (*p);
}

/* Parse CYGWIN options */

static NO_COPY bool export_settings = false;

enum settings
  {
    justset,
    isfunc,
    setbit,
    set_process_state,
  };

/* When BUF is:
   null or empty: disables globbing
   "ignorecase": enables case-insensitive globbing
   anything else: enables case-sensitive globbing */
static void
glob_init (const char *buf)
{
  if (!buf || !*buf)
    {
      allow_glob = false;
      ignore_case_with_glob = false;
    }
  else if (ascii_strncasematch (buf, "ignorecase", 10))
    {
      allow_glob = true;
      ignore_case_with_glob = true;
    }
  else
    {
      allow_glob = true;
      ignore_case_with_glob = false;
    }
}

void
set_file_api_mode (codepage_type cp)
{
  if (cp == oem_cp)
    {
      SetFileApisToOEM ();
      debug_printf ("File APIs set to OEM");
    }
  else
    {
      SetFileApisToANSI ();
      debug_printf ("File APIs set to ANSI");
    }
}

void
codepage_init (const char *buf)
{
  if (!buf)
    buf = "ansi";

  if (ascii_strcasematch (buf, "oem"))
    {
      current_codepage = oem_cp;
      active_codepage = GetOEMCP ();
    }
  else if (ascii_strcasematch (buf, "utf8"))
    {
      current_codepage = utf8_cp;
      active_codepage = CP_UTF8;
    }
  else
    {
      if (!ascii_strcasematch (buf, "ansi"))
	debug_printf ("Wrong codepage name: %s", buf);
      /* Fallback to ANSI */
      current_codepage = ansi_cp;
      active_codepage = GetACP ();
    }
  set_file_api_mode (current_codepage);
}

static void
set_chunksize (const char *buf)
{
  wincap.set_chunksize (strtoul (buf, NULL, 0));
}

static void
set_proc_retry (const char *buf)
{
  child_info::retry_count = strtoul (buf, NULL, 0);
}

/* The structure below is used to set up an array which is used to
   parse the CYGWIN environment variable or, if enabled, options from
   the registry.  */
static struct parse_thing
  {
    const char *name;
    union parse_setting
      {
	bool *b;
	DWORD *x;
	int *i;
	void (*func)(const char *);
      } setting;

    enum settings disposition;
    char *remember;
    union parse_values
      {
	DWORD i;
	const char *s;
      } values[2];
  } known[] NO_COPY =
{
  {"codepage", {func: &codepage_init}, isfunc, NULL, {{0}, {0}}},
  {"dosfilewarning", {&dos_file_warning}, justset, NULL, {{false}, {true}}},
  {"envcache", {&envcache}, justset, NULL, {{true}, {false}}},
  {"error_start", {func: &error_start_init}, isfunc, NULL, {{0}, {0}}},
  {"export", {&export_settings}, justset, NULL, {{false}, {true}}},
  {"forkchunk", {func: set_chunksize}, isfunc, NULL, {{0}, {0}}},
  {"glob", {func: &glob_init}, isfunc, NULL, {{0}, {s: "normal"}}},
  {"proc_retry", {func: set_proc_retry}, isfunc, NULL, {{0}, {5}}},
  {"reset_com", {&reset_com}, justset, NULL, {{false}, {true}}},
#ifdef USE_SERVER
  {"server", {&allow_server}, justset, NULL, {{false}, {true}}},
#endif
  {"strip_title", {&strip_title_path}, justset, NULL, {{false}, {true}}},
  {"title", {&display_title}, justset, NULL, {{false}, {true}}},
  {"tty", {NULL}, set_process_state, NULL, {{0}, {PID_USETTY}}},
  {"winsymlinks", {&allow_winsymlinks}, justset, NULL, {{false}, {true}}},
  {NULL, {0}, justset, 0, {{0}, {0}}}
};

/* Parse a string of the form "something=stuff somethingelse=more-stuff",
   silently ignoring unknown "somethings".  */
static void __stdcall
parse_options (char *buf)
{
  int istrue;
  char *p, *lasts;
  parse_thing *k;

  if (buf == NULL)
    {
      tmp_pathbuf tp;
      char *newbuf = tp.c_get ();
      newbuf[0] = '\0';
      for (k = known; k->name != NULL; k++)
	if (k->remember)
	  {
	    strcat (strcat (newbuf, " "), k->remember);
	    free (k->remember);
	    k->remember = NULL;
	  }

      if (export_settings)
	{
	  debug_printf ("%s", newbuf + 1);
	  setenv ("CYGWIN", newbuf + 1, 1);
	}
      return;
    }

  buf = strcpy ((char *) alloca (strlen (buf) + 1), buf);
  for (p = strtok_r (buf, " \t", &lasts);
       p != NULL;
       p = strtok_r (NULL, " \t", &lasts))
    {
      char *keyword_here = p;
      if (!(istrue = !ascii_strncasematch (p, "no", 2)))
	p += 2;
      else if (!(istrue = *p != '-'))
	p++;

      char ch, *eq;
      if ((eq = strchr (p, '=')) != NULL || (eq = strchr (p, ':')) != NULL)
	ch = *eq, *eq++ = '\0';
      else
	ch = 0;

      for (parse_thing *k = known; k->name != NULL; k++)
	if (ascii_strcasematch (p, k->name))
	  {
	    switch (k->disposition)
	      {
	      case isfunc:
		k->setting.func ((!eq || !istrue) ?
		  k->values[istrue].s : eq);
		debug_printf ("%s (called func)", k->name);
		break;
	      case justset:
		if (!istrue || !eq)
		  *k->setting.x = k->values[istrue].i;
		else
		  *k->setting.x = strtol (eq, NULL, 0);
		debug_printf ("%s %d", k->name, *k->setting.x);
		break;
	      case set_process_state:
		k->setting.x = &myself->process_state;
		/* fall through */
	      case setbit:
		*k->setting.x &= ~k->values[istrue].i;
		if (istrue || (eq && strtol (eq, NULL, 0)))
		  *k->setting.x |= k->values[istrue].i;
		debug_printf ("%s %x", k->name, *k->setting.x);
		break;
	      }

	    if (eq)
	      *--eq = ch;

	    int n = eq - p;
	    p = strdup (keyword_here);
	    if (n > 0)
	      p[n] = ':';
	    k->remember = p;
	    break;
	  }
      }
  debug_printf ("returning");
}

/* Set options from the registry. */
static bool __stdcall
regopt (const char *name, char *buf)
{
  bool parsed_something = false;
  char lname[strlen (name) + 1];
  strlwr (strcpy (lname, name));

  for (int i = 0; i < 2; i++)
    {
      reg_key r (i, KEY_READ, CYGWIN_INFO_PROGRAM_OPTIONS_NAME, NULL);

      if (r.get_string (lname, buf, NT_MAX_PATH, "") == ERROR_SUCCESS)
	{
	  parse_options (buf);
	  parsed_something = true;
	  break;
	}
    }

  MALLOC_CHECK;
  return parsed_something;
}

/* Initialize the environ array.  Look for the CYGWIN environment
   environment variable and set appropriate options from it.  */
void
environ_init (char **envp, int envc)
{
  PWCHAR rawenv, w;
  int i;
  char *p;
  char *newp;
  int sawTERM = 0;
  bool envp_passed_in;
  bool got_something_from_registry;
  static char NO_COPY cygterm[] = "TERM=cygwin";
  myfault efault;
  tmp_pathbuf tp;

  if (efault.faulted ())
    api_fatal ("internal error reading the windows environment - too many environment variables?");

  if (!conv_start_chars[0])
    for (int i = 0; conv_envvars[i].name != NULL; i++)
      {
	conv_start_chars[(int) cyg_tolower (conv_envvars[i].name[0])] = 1;
	conv_start_chars[(int) cyg_toupper (conv_envvars[i].name[0])] = 1;
      }

  char *tmpbuf = tp.t_get ();
  got_something_from_registry = regopt ("default", tmpbuf);
  if (myself->progname[0])
    got_something_from_registry = regopt (myself->progname, tmpbuf)
				  || got_something_from_registry;

  if (!envp)
    envp_passed_in = 0;
  else
    {
      envc++;
      envc *= sizeof (char *);
      char **newenv = (char **) malloc (envc);
      memcpy (newenv, envp, envc);
      cfree (envp);

      /* Older applications relied on the fact that cygwin malloced elements of the
	 environment list.  */
      envp = newenv;
      if (ENVMALLOC)
	for (char **e = newenv; *e; e++)
	  {
	    char *p = *e;
	    *e = strdup (p);
	    cfree (p);
	  }
      envp_passed_in = 1;
      goto out;
    }

  /* Allocate space for environment + trailing NULL + CYGWIN env. */
  lastenviron = envp = (char **) malloc ((4 + (envc = 100)) * sizeof (char *));

  /* We need the CYGWIN variable content before we can loop through
     the whole environment, so that the wide-char to multibyte conversion
     can be done according to the "codepage" setting. */
  if ((i = GetEnvironmentVariableA ("CYGWIN", NULL, 0)))
    {
      char *buf = (char *) alloca (i);
      GetEnvironmentVariableA ("CYGWIN", buf, i);
      parse_options (buf);
    }

  rawenv = GetEnvironmentStringsW ();
  if (!rawenv)
    {
      system_printf ("GetEnvironmentStrings returned NULL, %E");
      return;
    }
  debug_printf ("GetEnvironmentStrings returned %p", rawenv);

  /* Current directory information is recorded as variables of the
     form "=X:=X:\foo\bar; these must be changed into something legal
     (we could just ignore them but maybe an application will
     eventually want to use them).  */
  for (i = 0, w = rawenv; *w != L'\0'; w = wcschr (w, L'\0') + 1, i++)
    {
      sys_wcstombs_alloc (&newp, HEAP_NOTHEAP, w);
      if (i >= envc)
	envp = (char **) realloc (envp, (4 + (envc += 100)) * sizeof (char *));
      envp[i] = newp;
      if (*newp == '=')
	*newp = '!';
      char *eq = strechr (newp, '=');
      if (!child_proc_info)
	ucenv (newp, eq);
      if (*newp == 'T' && strncmp (newp, "TERM=", 5) == 0)
	sawTERM = 1;
      if (*newp == 'C' && strncmp (newp, "CYGWIN=", sizeof ("CYGWIN=") - 1) == 0)
	parse_options (newp + sizeof ("CYGWIN=") - 1);
      if (*eq && conv_start_chars[(unsigned char)envp[i][0]])
	posify (envp + i, *++eq ? eq : --eq, tmpbuf);
      debug_printf ("%p: %s", envp[i], envp[i]);
    }

  if (!sawTERM)
    envp[i++] = strdup (cygterm);
  envp[i] = NULL;
  FreeEnvironmentStringsW (rawenv);

out:
  findenv_func = (char * (*)(const char*, int*)) my_findenv;
  __cygwin_environ = envp;
  update_envptrs ();
  if (envp_passed_in)
    {
      p = getenv ("CYGWIN");
      if (p)
	parse_options (p);
    }

  if (got_something_from_registry)
    parse_options (NULL);	/* possibly export registry settings to
				   environment */
  MALLOC_CHECK;
}

/* Function called by qsort to sort environment strings.  */
static int
env_sort (const void *a, const void *b)
{
  const char **p = (const char **) a;
  const char **q = (const char **) b;

  return strcmp (*p, *q);
}

char * __stdcall
getwinenveq (const char *name, size_t namelen, int x)
{
  WCHAR name0[namelen - 1];
  WCHAR valbuf[32768]; /* Max size of an env.var including trailing '\0'. */

  name0[sys_mbstowcs (name0, sizeof name0, name, namelen - 1)] = L'\0';
  int totlen = GetEnvironmentVariableW (name0, valbuf, 32768);
  if (totlen > 0)
    {
      totlen = sys_wcstombs (NULL, 0, valbuf);
      if (x == HEAP_1_STR)
	totlen += namelen;
      else
	namelen = 0;
      char *p = (char *) cmalloc_abort ((cygheap_types) x, totlen);
      if (namelen)
	strcpy (p, name);
      sys_wcstombs (p + namelen, totlen, valbuf);
      debug_printf ("using value from GetEnvironmentVariable for '%W'", name0);
      return p;
    }

  debug_printf ("warning: %s not present in environment", name);
  return NULL;
}

struct spenv
{
  const char *name;
  size_t namelen;
  bool force_into_environment;	/* If true, always add to env if missing */
  bool add_if_exists;		/* if true, retrieve value from cache */
  const char * (cygheap_user::*from_cygheap) (const char *, size_t);

  char *retrieve (bool, const char * const = NULL)
    __attribute__ ((regparm (3)));
};

#define env_dontadd almost_null

/* Keep this list in upper case and sorted */
static NO_COPY spenv spenvs[] =
{
#ifdef DEBUGGING
  {NL ("CYGWIN_DEBUG="), false, true, NULL},
#endif
  {NL ("HOMEDRIVE="), false, false, &cygheap_user::env_homedrive},
  {NL ("HOMEPATH="), false, false, &cygheap_user::env_homepath},
  {NL ("LOGONSERVER="), false, false, &cygheap_user::env_logsrv},
  {NL ("PATH="), false, true, NULL},
  {NL ("SYSTEMDRIVE="), false, true, NULL},
  {NL ("SYSTEMROOT="), true, true, &cygheap_user::env_systemroot},
  {NL ("USERDOMAIN="), false, false, &cygheap_user::env_domain},
  {NL ("USERNAME="), false, false, &cygheap_user::env_name},
  {NL ("USERPROFILE="), false, false, &cygheap_user::env_userprofile},
  {NL ("WINDIR="), true, true, &cygheap_user::env_systemroot}
};

char *
spenv::retrieve (bool no_envblock, const char *const env)
{
  if (env && !ascii_strncasematch (env, name, namelen))
    return NULL;

  debug_printf ("no_envblock %d", no_envblock);

  if (from_cygheap)
    {
      const char *p;
      if (env && !cygheap->user.issetuid ())
	{
	  debug_printf ("duping existing value for '%s'", name);
	  /* Don't really care what it's set to if we're calling a cygwin program */
	  return cstrdup1 (env);
	}

      /* Calculate (potentially) value for given environment variable.  */
      p = (cygheap->user.*from_cygheap) (name, namelen);
      if (!p || (no_envblock && !env) || (p == env_dontadd))
	return env_dontadd;
      char *s = (char *) cmalloc_abort (HEAP_1_STR, namelen + strlen (p) + 1);
      strcpy (s, name);
      strcpy (s + namelen, p);
      debug_printf ("using computed value for '%s'", name);
      return s;
    }

  if (env)
    return cstrdup1 (env);

  return getwinenveq (name, namelen, HEAP_1_STR);
}

#define SPENVS_SIZE (sizeof (spenvs) / sizeof (spenvs[0]))

/* Create a Windows-style environment block, i.e. a typical character buffer
   filled with null terminated strings, terminated by double null characters.
   Converts environment variables noted in conv_envvars into win32 form
   prior to placing them in the string.  */
char ** __stdcall
build_env (const char * const *envp, PWCHAR &envblock, int &envc,
	   bool no_envblock)
{
  int len, n;
  const char * const *srcp;
  char **dstp;
  bool saw_spenv[SPENVS_SIZE] = {0};

  debug_printf ("envp %p", envp);

  /* How many elements? */
  for (n = 0; envp[n]; n++)
    continue;

  /* Allocate a new "argv-style" environ list with room for extra stuff. */
  char **newenv = (char **) cmalloc_abort (HEAP_1_ARGV, sizeof (char *) *
				     (n + SPENVS_SIZE + 1));

  int tl = 0;
  char **pass_dstp;
  char **pass_env = (char **) alloca (sizeof (char *) * (n + SPENVS_SIZE + 1));
  /* Iterate over input list, generating a new environment list and refreshing
     "special" entries, if necessary. */
  for (srcp = envp, dstp = newenv, pass_dstp = pass_env; *srcp; srcp++)
    {
      bool calc_tl = !no_envblock;
      /* Look for entries that require special attention */
      for (unsigned i = 0; i < SPENVS_SIZE; i++)
	if (!saw_spenv[i] && (*dstp = spenvs[i].retrieve (no_envblock, *srcp)))
	  {
	    saw_spenv[i] = 1;
	    if (*dstp == env_dontadd)
	      goto next1;
	    if (spenvs[i].add_if_exists)
	      calc_tl = true;
	    goto  next0;
	  }

      /* Add entry to new environment */
      *dstp = cstrdup1 (*srcp);

    next0:
      if (calc_tl)
	{
	  *pass_dstp++ = *dstp;
	  tl += strlen (*dstp) + 1;
	}
      dstp++;
    next1:
      continue;
    }

  assert ((srcp - envp) == n);
  /* Fill in any required-but-missing environment variables. */
  for (unsigned i = 0; i < SPENVS_SIZE; i++)
    if (!saw_spenv[i] && (spenvs[i].force_into_environment || cygheap->user.issetuid ()))
      {
	  *dstp = spenvs[i].retrieve (false);
	  if (*dstp && *dstp != env_dontadd)
	    {
	      *pass_dstp++ = *dstp;
	      tl += strlen (*dstp) + 1;
	      dstp++;
	    }
	}

  envc = dstp - newenv;		/* Number of entries in newenv */
  assert ((size_t) envc <= (n + SPENVS_SIZE));
  *dstp = NULL;			/* Terminate */

  size_t pass_envc = pass_dstp - pass_env;
  if (!pass_envc)
    envblock = NULL;
  else
    {
      *pass_dstp = NULL;
      debug_printf ("env count %d, bytes %d", pass_envc, tl);
      win_env temp;
      temp.reset ();

      /* Windows programs expect the environment block to be sorted.  */
      qsort (pass_env, pass_envc, sizeof (char *), env_sort);

      /* Create an environment block suitable for passing to CreateProcess.  */
      PWCHAR s;
      envblock = (PWCHAR) malloc ((2 + tl) * sizeof (WCHAR));
      int new_tl = 0;
      for (srcp = pass_env, s = envblock; *srcp; srcp++)
	{
	  const char *p;
	  win_env *conv;
	  len = strcspn (*srcp, "=") + 1;
	  const char *rest = *srcp + len;

	  /* Check for a bad entry.  This is necessary to get rid of empty
	     strings, induced by putenv and changing the string afterwards.
	     Note that this doesn't stop invalid strings without '=' in it
	     etc., but we're opting for speed here for now.  Adding complete
	     checking would be pretty expensive. */
	  if (len == 1 || !*rest)
	    continue;

	  /* See if this entry requires posix->win32 conversion. */
	  conv = getwinenv (*srcp, rest, &temp);
	  if (conv)
	    p = conv->native;	/* Use win32 path */
	  else
	    p = *srcp;		/* Don't worry about it */

	  len = strlen (p) + 1;
	  new_tl += len;	/* Keep running total of block length so far */

	  /* See if we need to increase the size of the block. */
	  if (new_tl > tl)
	    {
	      tl = new_tl + 100;
	      PWCHAR new_envblock =
			(PWCHAR) realloc (envblock, (2 + tl) * sizeof (WCHAR));
	      /* If realloc moves the block, move `s' with it. */
	      if (new_envblock != envblock)
		{
		  s += new_envblock - envblock;
		  envblock = new_envblock;
		}
	    }

	  int slen = sys_mbstowcs (s, len, p, len);

	  /* See if environment variable is "special" in a Windows sense.
	     Under NT, the current directories for visited drives are stored
	     as =C:=\bar.  Cygwin converts the '=' to '!' for hopefully obvious
	     reasons.  We need to convert it back when building the envblock */
	  if (s[0] == L'!' && (iswdrive (s + 1) || (s[1] == L':' && s[2] == L':'))
	      && s[3] == L'=')
	    *s = L'=';
	  s += slen + 1;
	}
      *s = L'\0';			/* Two null bytes at the end */
      assert ((s - envblock) <= tl);	/* Detect if we somehow ran over end
					   of buffer */
    }

  debug_printf ("envp %p, envc %d", newenv, envc);
  return newenv;
}

/* This idiocy is necessary because the early implementers of cygwin
   did not seem to know about importing data variables from the DLL.
   So, we have to synchronize cygwin's idea of the environment with the
   main program's with each reference to the environment. */
extern "C" char ** __stdcall
cur_environ ()
{
  if (*main_environ != __cygwin_environ)
    {
      __cygwin_environ = *main_environ;
      update_envptrs ();
    }

  return __cygwin_environ;
}
