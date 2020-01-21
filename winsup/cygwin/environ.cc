/* environ.cc: Cygwin-adopted functions from newlib to manipulate
   process's environment.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <userenv.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>
#include <locale.h>
#include <assert.h>
#include <sys/param.h>
#include <cygwin/version.h>
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
#include "shared_info.h"
#include "ntdll.h"

/* If this is not NULL, it points to memory allocated by us. */
static char **lastenviron;

/* Parse CYGWIN options */

static NO_COPY bool export_settings = false;

enum settings
  {
    isfunc,
    setdword,
    setbool,
    setbit
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

static void
set_proc_retry (const char *buf)
{
  child_info::retry_count = strtoul (buf, NULL, 0);
}

static void
set_winsymlinks (const char *buf)
{
  if (!buf || !*buf)
    allow_winsymlinks = WSYM_lnk;
  else if (ascii_strncasematch (buf, "lnk", 3))
    allow_winsymlinks = WSYM_lnk;
  /* Make sure to try native symlinks only on systems supporting them. */
  else if (ascii_strncasematch (buf, "native", 6))
    allow_winsymlinks = ascii_strcasematch (buf + 6, "strict")
			? WSYM_nativestrict : WSYM_native;
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
  {"dosfilewarning", {&dos_file_warning}, setbool, NULL, {{false}, {true}}},
  {"error_start", {func: error_start_init}, isfunc, NULL, {{0}, {0}}},
  {"export", {&export_settings}, setbool, NULL, {{false}, {true}}},
  {"glob", {func: glob_init}, isfunc, NULL, {{0}, {s: "normal"}}},
  {"pipe_byte", {&pipe_byte}, setbool, NULL, {{false}, {true}}},
  {"proc_retry", {func: set_proc_retry}, isfunc, NULL, {{0}, {5}}},
  {"reset_com", {&reset_com}, setbool, NULL, {{false}, {true}}},
  {"wincmdln", {&wincmdln}, setbool, NULL, {{false}, {true}}},
  {"winsymlinks", {func: set_winsymlinks}, isfunc, NULL, {{0}, {0}}},
  {"disable_pcon", {&disable_pcon}, setbool, NULL, {{false}, {true}}},
  {NULL, {0}, setdword, 0, {{0}, {0}}}
};

/* Return a possibly-quoted token.
   Returns NULL when no more tokens available.  */
static char *
strbrk(char *&buf)
{
  buf += strspn(buf, " \t");
  if (!*buf)
    return NULL;
  char *tok = buf;
  char *sep = buf + strcspn(buf, " \t");
  char *quotestart = strchr(buf, '"');
  if (!quotestart || quotestart > sep)
    {
      buf = sep + !!*sep;	/* Don't point beyond EOS */
      quotestart = NULL;
    }
  else
    {
      char *quote = quotestart;
      sep = NULL;
      while (!sep)
	{
	  char *clquote = strchr (quote + 1, '"');
	  if (!clquote)
	    sep = strchr (quote, '\0');
	  else if (clquote[-1] != '\\')
	    sep = clquote;
	  else
	    {
	      memmove (clquote - 1, clquote, 1 + strchr (clquote, '\0') - clquote);
	      quote = clquote - 1;
	    }
	}
      buf = sep + 1;
      memmove (quotestart, quotestart + 1, sep - quotestart);
      sep--;
    }
  *sep = '\0';
  return tok;
}


/* Parse a string of the form "something=stuff somethingelse=more-stuff",
   silently ignoring unknown "somethings".  */
static void __stdcall
parse_options (const char *inbuf)
{
  int istrue;
  parse_thing *k;

  if (inbuf == NULL)
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

  char *buf = strcpy ((char *) alloca (strlen (inbuf) + 1), inbuf);

  while (char *p = strbrk (buf))
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
		k->setting.func ((!eq || !istrue) ?  k->values[istrue].s : eq);
		debug_printf ("%s (called func)", k->name);
		break;
	      case setdword:
		if (!istrue || !eq)
		  *k->setting.x = k->values[istrue].i;
		else
		  *k->setting.x = strtol (eq, NULL, 0);
		debug_printf ("%s %u", k->name, *k->setting.x);
		break;
	      case setbool:
		if (!istrue || !eq)
		  *k->setting.b = k->values[istrue].i;
		else
		  *k->setting.b = !!strtol (eq, NULL, 0);
		debug_printf ("%s%s", *k->setting.b ? "" : "no", k->name);
		break;
	      case setbit:
		*k->setting.x &= ~k->values[istrue].i;
		if (istrue || (eq && strtol (eq, NULL, 0)))
		  *k->setting.x |= k->values[istrue].i;
		debug_printf ("%s %x", k->name, *k->setting.x);
		break;
	      }

	    int n = 0;
	    if (eq)
	      {
		*--eq = ch;
		n = eq - p;
	      }
	    p = strdup (keyword_here);
	    if (n > 0)
	      p[n] = ':';
	    k->remember = p;
	    break;
	  }
      }
  debug_printf ("returning");
}

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

#define WC ((unsigned char) 1)
/* Note:  You *must* fill in this array setting the ordinal value of the first
   character of the above environment variable names to 1.
   This table is intended to speed up lookup of these variables. */

static const unsigned char conv_start_chars[256] =
  {
    0,        0,        0,        0,        0,        0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0,
/*            A         B         C         D         E         F         G */
    0,        0,        0,        0,        0,        0,        0,        0,
    /*  72 */
/*  H         I         J         K         L         M         N         O */
    WC,       0,        0,        0,        WC,       0,        0,        0,
    /*  80 */
/*  P         Q         R         S         T         U         V         W */
    WC,       0,        0,        0,        WC,       0,        0,        0,
    /*  88 */
/*  x         Y         Z                                                   */
    0,        0,        0,        0,        0,        0,        0,        0,
    /*  96 */
/*            a         b         c         d         e         f         g */
    0,        0,        0,        0,        0,        0,        0,        0,
    /* 104 */
/*  h         i         j         k         l         m         n         o */
    WC,       0,        0,        0,        WC,       0,        0,        0,
    /* 112 */
/*  p         q         r         s         t         u         v         w */
    WC,       0,        0,        0,        WC,       0,        0,        0,
  };

static inline char
match_first_char (const char *s, unsigned char m)
{
  return conv_start_chars[*(unsigned char *)s] & m;
}

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
  posix = (char *) realloc (posix, strlen (in_posix) + 1);
  strcpy (posix, in_posix);
  if (in_native)
    {
      native = (char *) realloc (native, namelen + 1 + strlen (in_native));
      stpcpy (stpcpy (native, name), in_native);
    }
  else
    {
      tmp_pathbuf tp;
      char *buf = tp.c_get ();
      towin32 (in_posix, buf, NT_MAX_PATH);
      native = (char *) realloc (native, namelen + 1 + strlen (buf));
      stpcpy (stpcpy (native, name), buf);
    }
  if (immediate && cygwin_finished_initializing)
    {
      wchar_t s[sys_mbstowcs (NULL, 0, native) + 1];
      sys_mbstowcs (s, sizeof s, native);
      /* Hack. Relies on affected variables only having ASCII names. */
      s[namelen - 1] = L'\0';
      SetEnvironmentVariableW (s, s + namelen);
    }
  debug_printf ("posix %s", posix);
  debug_printf ("native %s", native);
}


/* Check for a "special" environment variable name.  *env is the pointer
  to the beginning of the environment variable name.  *in_posix is any
  known posix value for the environment variable. Returns a pointer to
  the appropriate conversion structure.  */
win_env * __reg3
getwinenv (const char *env, const char *in_posix, win_env *temp)
{
  if (!match_first_char (env, WC))
    return NULL;

  for (int i = 0; conv_envvars[i].name != NULL; i++)
    if (strncmp (env, conv_envvars[i].name, conv_envvars[i].namelen) == 0)
      {
	win_env *we = conv_envvars + i;
	const char *val;
	if (!cur_environ () || !(val = in_posix ?: getenv (we->name)))
	  debug_printf ("can't set native for %s since no environ yet",
			we->name);
	else if (!we->posix || strcmp (val, we->posix) != 0)
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
inline static void
posify_maybe (char **here, const char *value, char *outenv)
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
  if (!conv->toposix (value, newvalue, NT_MAX_PATH - len) || errno != EIDRM)
    conv->add_cache (newvalue, *value != '/' ? value : NULL);
  else
    {
      /* The conversion routine removed elements from a path list so we have
	 to recalculate the windows path to remove elements there, too. */
      tmp_pathbuf tp;
      char *cleanvalue = tp.c_get ();
      conv->towin32 (newvalue, cleanvalue, NT_MAX_PATH);
      conv->add_cache (newvalue, cleanvalue);
    }

  debug_printf ("env var converted to %s", outenv);
  *here = strdup (outenv);
  free (src);
}

/* Returns pointer to value associated with name, if any, else NULL.
  Sets offset to be the offset of the name/value combination in the
  environment array, for use by setenv(3) and unsetenv(3).
  Explicitly removes '=' in argument name.  */

static char *
my_findenv (const char *name, int *offset)
{
  register int len;
  register char **p;
  const char *c;

  if (cur_environ () == NULL)
    return NULL;

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
  return NULL;
}

/* Primitive getenv before the environment is built.  */

static char *
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

static char * (*findenv_func)(const char *, int *) = getearly;

/* Returns ptr to value associated with name, if any, else NULL.  */

extern "C" char *
getenv (const char *name)
{
  int offset;
  return findenv_func (name, &offset);
}

/* This function is required so that newlib uses the same environment
   as Cygwin. */
extern "C" char *
_getenv_r (struct _reent *, const char *name)
{
  int offset;
  return findenv_func (name, &offset);
}

/* Like getenv, but returns NULL if effective and real UID/GIDs do not match */
extern "C" char *
secure_getenv (const char *name)
{
  int offset;
  if (cygheap->user.issetuid ())
    return NULL;
  return findenv_func (name, &offset);
}

/* Return number of environment entries, including terminating NULL. */
static int __stdcall
envsize (const char * const *in_envp)
{
  const char * const *envp;

  if (in_envp == NULL)
    return 0;

  for (envp = in_envp; *envp; envp++)
    continue;
  return 1 + envp - in_envp;
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

      /* If sz == 0, we need two new slots, one for the terminating NULL. */
      int newsz = sz == 0 ? 2 : sz + 1;
      int allocsz = newsz * sizeof (char *);

      offset = newsz - 2;

      /* Allocate space for additional element. */
      if (cur_environ () == lastenviron)
	lastenviron = __cygwin_environ = (char **) realloc (lastenviron,
							    allocsz);
      else if ((lastenviron = (char **) realloc (lastenviron, allocsz)) != NULL)
	__cygwin_environ = (char **) memcpy (lastenviron, __cygwin_environ,
					     sz * sizeof (char *));
      if (!lastenviron)
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
      memcpy (envhere, name, namelen);
      envhere[namelen] = '=';
      strcpy (envhere + namelen + 1, value);
    }

  /* Update cygwin's cache, if appropriate */
  win_env *spenv;
  if ((spenv = getwinenv (envhere)))
    spenv->add_cache (value);
  if (strcmp (name, "CYGWIN") == 0)
    parse_options (value);

  return 0;
}

/* Set an environment variable */
extern "C" int
putenv (char *str)
{
  __try
    {
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
  __except (EFAULT) {}
  __endtry
  return -1;
}

/* Set the value of the environment variable "name" to be
   "value".  If overwrite is set, replace any current value.  */
extern "C" int
setenv (const char *name, const char *value, int overwrite)
{
  __try
    {
      if (!name || !*name || strchr (name, '='))
	{
	  set_errno (EINVAL);
	  __leave;
	}
      return _addenv (name, value, !!overwrite);
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

/* Delete environment variable "name".  */
extern "C" int
unsetenv (const char *name)
{
  register char **e;
  int offset;

  __try
    {
      if (!name || *name == '\0' || strchr (name, '='))
	{
	  set_errno (EINVAL);
	  __leave;
	}

      while (my_findenv (name, &offset))	/* if set multiple times */
	/* Move up the rest of the array */
	for (e = cur_environ () + offset; ; e++)
	  if (!(*e = *(e + 1)))
	    break;

      return 0;
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

/* Clear the environment.  */
extern "C" int
clearenv (void)
{
  __try
    {
      if (cur_environ () == lastenviron)
	{
	  free (lastenviron);
	  lastenviron = NULL;
	}
      __cygwin_environ = NULL;
      update_envptrs ();
      return 0;
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

/* Minimal list of Windows vars which must be converted to uppercase.
   Either for POSIX compatibility of for backward compatibility with
   existing applications. */
static struct renv {
	const char *name;
	const size_t namelen;
} renv_arr[] = {
	{ NL("COMMONPROGRAMFILES=") },		// 0
	{ NL("COMSPEC=") },
	{ NL("PATH=") },			// 2
	{ NL("PROGRAMFILES=") },
	{ NL("SYSTEMDRIVE=") },			// 4
	{ NL("SYSTEMROOT=") },
	{ NL("TEMP=") },			// 6
	{ NL("TMP=") },
	{ NL("WINDIR=") }			// 8
};
#define RENV_SIZE (sizeof (renv_arr) / sizeof (renv_arr[0]))

/* Set of first characters of the above list of variables. */
static const char idx_arr[] = "CPSTW";
/* Index into renv_arr at which the variables with this specific character
   starts. */
static const int start_at[] = { 0, 2, 4, 6, 8 };

/* Turn environment variable part of a=b string into uppercase - for some
   environment variables only. */
static __inline__ void
ucenv (char *p, const char *eq)
{
  /* Hopefully as quickly as possible - only upper case specific set of important
     Windows variables. */
  char first = cyg_toupper (*p);
  const char *idx = strchr (idx_arr, first);
  if (idx)
    for (size_t i = start_at[idx - idx_arr];
	 i < RENV_SIZE && renv_arr[i].name[0] == first;
	 ++i)
      if (strncasematch (p, renv_arr[i].name, renv_arr[i].namelen))
	{
	  strncpy (p, renv_arr[i].name, renv_arr[i].namelen);
	  break;
	}
}

/* Initialize the environ array.  Look for the CYGWIN environment variable and
   set appropriate options from it.  */
void
environ_init (char **envp, int envc)
{
  PWCHAR rawenv;
  char *p;
  bool envp_passed_in;

  __try
    {
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

      rawenv = GetEnvironmentStringsW ();
      if (!rawenv)
	{
	  system_printf ("GetEnvironmentStrings returned NULL, %E");
	  return;
	}
      debug_printf ("GetEnvironmentStrings returned %p", rawenv);

      lastenviron = envp = win32env_to_cygenv (rawenv, true);

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
    }
  __except (NO_ERROR)
    {
      api_fatal ("internal error reading the windows environment "
		 "- too many environment variables?");
    }
  __endtry
}

int sawTERM = 0;

char ** __reg2
win32env_to_cygenv (PWCHAR rawenv, bool posify)
{
  tmp_pathbuf tp;
  char **envp;
  int envc;
  char *newp;
  int i;
  const char cygterm[] = "TERM=cygwin";
  const char xterm[] = "TERM=xterm-256color";
  char *tmpbuf = tp.t_get ();
  PWCHAR w;

  /* Allocate space for environment + trailing NULL + CYGWIN env. */
  envp = (char **) malloc ((4 + (envc = 100)) * sizeof (char *));

  /* Current directory information is recorded as variables of the
     form "=X:=X:\foo\bar; these must be changed into something legal
     (we could just ignore them but maybe an application will
     eventually want to use them).  */
  for (i = 0, w = rawenv; *w != L'\0'; w = wcschr (w, L'\0') + 1, i++)
    {
      sys_wcstombs_alloc_no_path (&newp, HEAP_NOTHEAP, w);
      if (i >= envc)
        envp = (char **) realloc (envp, (4 + (envc += 100)) * sizeof (char *));
      envp[i] = newp;
      if (*newp == '=')
        *newp = '!';
      char *eq = strchrnul (newp, '=');
      ucenv (newp, eq);    /* uppercase env vars which need it */
      if (*newp == 'T' && strncmp (newp, "TERM=", 5) == 0)
        sawTERM = 1;
      else if (*newp == 'C' && strncmp (newp, "CYGWIN=", 7) == 0)
        parse_options (newp + 7);
      if (*eq && posify)
        posify_maybe (envp + i, *++eq ? eq : --eq, tmpbuf);
      debug_printf ("%p: %s", envp[i], envp[i]);
    }

  /* If console has 24 bit color capability, TERM=xterm-256color,
     otherwise, TERM=cygwin */
  if (!sawTERM)
    envp[i++] = strdup (wincap.has_con_24bit_colors () ? xterm : cygterm);

  envp[i] = NULL;
  return envp;
}

/* Function called by qsort to sort environment strings.  */
static int
env_sort (const void *a, const void *b)
{
  const char **p = (const char **) a;
  const char **q = (const char **) b;

  return strcmp (*p, *q);
}

char * __reg3
getwinenveq (const char *name, size_t namelen, int x)
{
  WCHAR name0[namelen - 1];
  WCHAR valbuf[32768]; /* Max size of an env.var including trailing '\0'. */

  name0[sys_mbstowcs (name0, sizeof name0, name, namelen - 1)] = L'\0';
  int totlen = GetEnvironmentVariableW (name0, valbuf, 32768);
  if (totlen > 0)
    {
      totlen = sys_wcstombs_no_path (NULL, 0, valbuf) + 1;
      if (x == HEAP_1_STR)
	totlen += namelen;
      else
	namelen = 0;
      char *p = (char *) cmalloc_abort ((cygheap_types) x, totlen);
      if (namelen)
	strcpy (p, name);
      sys_wcstombs_no_path (p + namelen, totlen, valbuf);
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

  char __reg3 *retrieve (bool, const char * const = NULL);
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

static inline int
raise_envblock (int new_tl, PWCHAR &envblock, PWCHAR &s)
{
  int tl = new_tl + 100;
  PWCHAR new_envblock =
	    (PWCHAR) realloc (envblock, (2 + tl) * sizeof (WCHAR));
  /* If realloc moves the block, move `s' with it. */
  if (new_envblock != envblock)
    {
      s += new_envblock - envblock;
      envblock = new_envblock;
    }
  return tl;
}

#define SPENVS_SIZE (sizeof (spenvs) / sizeof (spenvs[0]))

int
env_compare (const void *key, const void *memb)
{
  const char *k = *(const char **) key;
  const char *m = *(const char **) memb;

  char *ke = strchr (k, '=');
  char *me = strchr (m, '=');
  if (ke == NULL || me == NULL)
    return strcasecmp (k, m);
  int ret = strncasecmp (k, m, MIN (ke - k, me - m));
  if (!ret)
    ret = (ke - k) - (me - m);
  return ret;
}

/* Create a Windows-style environment block, i.e. a typical character buffer
   filled with null terminated strings, terminated by double null characters.
   Converts environment variables noted in conv_envvars into win32 form
   prior to placing them in the string.

   If new_token is set, we're going to switch the user account in
   child_info_spawn::worker.  If so, we're also fetching the Windows default
   environment for the new user, and merge it into the environment we propage
   to the child. */
char ** __reg3
build_env (const char * const *envp, PWCHAR &envblock, int &envc,
	   bool no_envblock, HANDLE new_token)
{
  PWCHAR cwinenv = NULL;
  size_t winnum = 0;
  char **winenv = NULL;

  int len, n;
  const char * const *srcp;
  char **dstp;
  bool saw_spenv[SPENVS_SIZE] = {0};

  static char *const empty_env[] = { NULL };

  debug_printf ("envp %p", envp);

  if (!envp)
    envp = empty_env;

  /* How many elements? */
  for (n = 0; envp[n]; n++)
    continue;

  /* Fetch windows env and convert to POSIX-style env. */
  if (new_token
      && CreateEnvironmentBlock ((LPVOID *) &cwinenv, new_token, FALSE))
    {
      PWCHAR var = cwinenv;
      while (*var)
	{
	  ++winnum;
	  var = wcschr (var, L'\0') + 1;
	}
      winenv = (char **) calloc (winnum + 1, sizeof (char *));
      if (winenv)
	{
	  for (winnum = 0, var = cwinenv;
	       *var;
	       ++winnum, var = wcschr (var, L'\0') + 1)
	    sys_wcstombs_alloc_no_path (&winenv[winnum], HEAP_NOTHEAP, var);
	}
      DestroyEnvironmentBlock (cwinenv);
      /* Eliminate variables which are already available in envp, as well as
	 the small set of crucial variables needing POSIX conversion and
	 potentially collide.  The windows env is sorted, so we can use
	 bsearch.  We're doing this first step, so the following code doesn't
	 allocate too much memory. */
      if (winenv)
	{
	  for (srcp = envp; *srcp; srcp++)
	    {
	      char **elem = (char **) bsearch (srcp, winenv, winnum,
					       sizeof *winenv, env_compare);
	      if (elem)
		{
		  free (*elem);
		  /* Use memmove to keep array sorted.
		     winnum - (elem - winenv) copies all elements following
		     elem, including the trailing NULL pointer. */
		  memmove (elem, elem + 1,
			   (winnum - (elem - winenv)) * sizeof *elem);
		  --winnum;
		}
	    }
	  for (char **elem = winenv; *elem; elem++)
	    {
	      if (match_first_char (*elem, WC))
		for (int i = 0; conv_envvars[i].name != NULL; i++)
		  if (strncmp (*elem, conv_envvars[i].name,
			       conv_envvars[i].namelen) == 0)
		    {
		      free (*elem);
		      memmove (elem, elem + 1,
			       (winnum - (elem - winenv)) * sizeof *elem);
		      --winnum;
		      --elem;
		    }
	    }
	}
    }

  /* Allocate a new "argv-style" environ list with room for extra stuff. */
  char **newenv = (char **) cmalloc_abort (HEAP_1_ARGV, sizeof (char *) *
				     (n + winnum + SPENVS_SIZE + 1));

  int tl = 0;
  char **pass_dstp;
  char **pass_env = (char **) alloca (sizeof (char *)
				      * (n + winnum + SPENVS_SIZE + 1));
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
    if (!saw_spenv[i] && (spenvs[i].force_into_environment
			  || cygheap->user.issetuid ()))
      {
	*dstp = spenvs[i].retrieve (false);
	if (*dstp && *dstp != env_dontadd)
	  {
	    *pass_dstp++ = *dstp;
	    tl += strlen (*dstp) + 1;
	    /* Eliminate from winenv. */
	    if (winenv)
	      {
		char **elem = (char **) bsearch (dstp, winenv, winnum,
						 sizeof *winenv, env_compare);
		if (elem)
		  {
		    free (*elem);
		    memmove (elem, elem + 1,
			     (winnum - (elem - winenv)) * sizeof *elem);
		    --winnum;
		  }
	      }
	    dstp++;
	  }
      }

  /* Fill in any Windows environment vars still missing. */
  if (winenv)
    {
      char **elem;
      for (elem = winenv; *elem; ++elem)
	{
	  *dstp = cstrdup1 (*elem);
	  free (*elem);
	  *pass_dstp++ = *dstp;
	  tl += strlen (*dstp) + 1;
	  ++dstp;
	}
      free (winenv);
    }

  envc = dstp - newenv;		/* Number of entries in newenv */
  assert ((size_t) envc <= (n + winnum + SPENVS_SIZE));
  *dstp = NULL;			/* Terminate */

  size_t pass_envc = pass_dstp - pass_env;
  if (!pass_envc)
    envblock = NULL;
  else
    {
      *pass_dstp = NULL;
      debug_printf ("env count %ld, bytes %d", pass_envc, tl);
      win_env temp;
      temp.reset ();

      /* Windows programs expect the environment block to be sorted.  */
      qsort (pass_env, pass_envc, sizeof (char *), env_sort);

      /* Create an environment block suitable for passing to CreateProcess.  */
      PWCHAR s;
      envblock = (PWCHAR) malloc ((2 + tl) * sizeof (WCHAR));
      int new_tl = 0;
      bool saw_PATH = false;
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
	    {
	      p = conv->native;	/* Use win32 path */
	      /* Does PATH exist in the environment? */
	      if (**srcp == 'P')
		{
		  /* And is it non-empty? */
		  if (!conv->native || !conv->native[0])
		    continue;
		  saw_PATH = true;
		}
	    }
	  else
	    p = *srcp;		/* Don't worry about it */

	  len = sys_mbstowcs (NULL, 0, p);
	  new_tl += len;	/* Keep running total of block length so far */

	  /* See if we need to increase the size of the block. */
	  if (new_tl > tl)
	    tl = raise_envblock (new_tl, envblock, s);

	  len = sys_mbstowcs (s, len, p);

	  /* See if environment variable is "special" in a Windows sense.
	     Under NT, the current directories for visited drives are stored
	     as =C:=\bar.  Cygwin converts the '=' to '!' for hopefully obvious
	     reasons.  We need to convert it back when building the envblock */
	  if (s[0] == L'!' && (iswdrive (s + 1) || (s[1] == L':' && s[2] == L':'))
	      && s[3] == L'=')
	    *s = L'=';
	  s += len + 1;
	}
      /* If PATH doesn't exist in the environment, add a PATH with just
	 Cygwin's bin dir to the Windows env to allow loading system DLLs
	 during execve. */
      if (!saw_PATH)
	{
	  new_tl += cygheap->installation_dir.Length / sizeof (WCHAR) + 5 + 1;
	  if (new_tl > tl)
	    tl = raise_envblock (new_tl, envblock, s);
	  s = wcpcpy (wcpcpy (s, L"PATH="),
		      cygheap->installation_dir.Buffer) + 1;
	}
      *s = L'\0';			/* Two null bytes at the end */
      assert ((s - envblock) <= tl);	/* Detect if we somehow ran over end
					   of buffer */
    }

  debug_printf ("envp %p, envc %d", newenv, envc);
  return newenv;
}

#ifdef __i386__
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
#endif
