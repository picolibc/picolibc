/* environ.cc: Cygwin-adopted functions from newlib to manipulate
   process's environment.

   Copyright 1997, 1998, 1999, 2000 Cygnus Solutions.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <fcntl.h>
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "fhandler.h"
#include "path.h"
#include "cygerrno.h"

extern BOOL allow_glob;
extern BOOL allow_ntea;
extern BOOL strip_title_path;
extern DWORD chunksize;
BOOL reset_com = TRUE;
static BOOL envcache = TRUE;

/* List of names which are converted from dos to unix
 * on the way in and back again on the way out.
 *
 * PATH needs to be here because CreateProcess uses it and gdb uses
 * CreateProcess.  HOME is here because most shells use it and would be
 * confused by Windows style path names.
 */
static int return_MAX_PATH (const char *) {return MAX_PATH;}
static win_env conv_envvars[] =
  {
    {"PATH=", 5, NULL, NULL, cygwin_win32_to_posix_path_list,
     cygwin_posix_to_win32_path_list,
     cygwin_win32_to_posix_path_list_buf_size,
     cygwin_posix_to_win32_path_list_buf_size},
    {"HOME=", 5, NULL, NULL, cygwin_conv_to_full_posix_path, cygwin_conv_to_full_win32_path,
     return_MAX_PATH, return_MAX_PATH},
    {"LD_LIBRARY_PATH=", 16, NULL, NULL, cygwin_conv_to_full_posix_path,
     cygwin_conv_to_full_win32_path, return_MAX_PATH, return_MAX_PATH},
    {"TMPDIR=", 7, NULL, NULL, cygwin_conv_to_full_posix_path, cygwin_conv_to_full_win32_path,
     return_MAX_PATH, return_MAX_PATH},
    {"TMP=", 4, NULL, NULL, cygwin_conv_to_full_posix_path, cygwin_conv_to_full_win32_path,
     return_MAX_PATH, return_MAX_PATH},
    {"TEMP=", 5, NULL, NULL, cygwin_conv_to_full_posix_path, cygwin_conv_to_full_win32_path,
     return_MAX_PATH, return_MAX_PATH},
    {NULL, 0, NULL, NULL, NULL, NULL, 0, 0}
  };

void
win_env::add_cache (const char *in_posix, const char *in_native)
{
  posix = (char *) realloc (posix, strlen (in_posix) + 1);
  strcpy (posix, in_posix);
  if (in_native)
    {
      native = (char *) realloc (native, namelen + 1 + strlen (in_native));
      (void) strcpy (native, name);
      (void) strcpy (native + namelen, in_native);
    }
  else
    {
      native = (char *) realloc (native, namelen + 1 + win32_len (in_posix));
      (void) strcpy (native, name);
      towin32 (in_posix, native + namelen);
    }
  debug_printf ("posix %s", posix);
  debug_printf ("native %s", native);
}


/* Check for a "special" environment variable name.  *env is the pointer
 * to the beginning of the environment variable name.  n is the length
 * of the name including a mandatory '='.  Returns a pointer to the
 * appropriate conversion structure.
 */
win_env * __stdcall
getwinenv (const char *env, const char *in_posix)
{
  for (int i = 0; conv_envvars[i].name != NULL; i++)
    if (strncasematch (env, conv_envvars[i].name, conv_envvars[i].namelen))
      {
	win_env *we = conv_envvars + i;
	const char *val;
	if (!cur_environ () || !(val = in_posix ?: getenv(we->name)))
	  debug_printf ("can't set native for %s since no environ yet",
			we->name);
	else if (!envcache || !we->posix || strcmp (val, we->posix))
	      we->add_cache (val);
	return we;
      }
  return NULL;
}

/* Convert windows path specs to POSIX, if appropriate.
 */
static void __stdcall
posify (int already_posix, char **here, const char *value)
{
  char *src = *here;
  win_env *conv;
  int len = strcspn (src, "=") + 1;

  if (!(conv = getwinenv (src)))
    return;

  if (already_posix)
    conv->add_cache (value, NULL);
  else
    {
      /* Turn all the items from c:<foo>;<bar> into their
	 mounted equivalents - if there is one.  */

      char *outenv = (char *) malloc (1 + len + conv->posix_len (value));
      memcpy (outenv, src, len);
      conv->toposix (value, outenv + len);
      conv->add_cache (outenv + len, value);

      debug_printf ("env var converted to %s", outenv);
      *here = outenv;
      free (src);
    }
}

/*
 * my_findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environment array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 */

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
  return NULL;
}

/*
 * getenv --
 *	Returns ptr to value associated with name, if any, else NULL.
 */

extern "C" char *
getenv (const char *name)
{
  int offset;

  return my_findenv (name, &offset);
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
      char **env;

      /* Search for the end of the environment. */
      for (env = cur_environ (); *env; env++)
	continue;

      offset = env - cur_environ ();	/* Number of elements currently in environ. */

      /* Allocate space for additional element plus terminating NULL. */
      __cygwin_environ = (char **) realloc (cur_environ (), (sizeof (char *) *
						     (offset + 2)));
      if (!__cygwin_environ)
	return -1;		/* Oops.  No more memory. */

      __cygwin_environ[offset + 1] = NULL;	/* NULL terminate. */
      update_envptrs ();	/* Update any local copies of 'environ'. */
    }

  char *envhere;
  if (!issetenv)
    envhere = cur_environ ()[offset] = (char *) name;	/* Not setenv. Just
						   overwrite existing. */
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

  return 0;
}

/* putenv --
 *	Sets an environment variable
 */

extern "C" int
putenv (const char *str)
{
  int res;
  if ((res = check_null_empty_path (str)))
    {
      if (res == ENOENT)
        return 0;
      set_errno (res);
      return  -1;
    }
  char *eq = strchr (str, '=');
  if (eq)
    return _addenv (str, eq + 1, -1);

  /* Remove str from the environment. */
  unsetenv (str);
  return 0;
}

/*
 * setenv --
 *	Set the value of the environment variable "name" to be
 *	"value".  If overwrite is set, replace any current value.
 */

extern "C" int
setenv (const char *name, const char *value, int overwrite)
{
  int res;
  if ((res = check_null_empty_path (value)) == EFAULT)
    {
      set_errno (res);
      return  -1;
    }
  if ((res = check_null_empty_path (name)))
    {
      if (res == ENOENT)
        return 0;
      set_errno (res);
      return  -1;
    }
  if (*value == '=')
    value++;
  return _addenv (name, value, !!overwrite);
}

/*
 * unsetenv(name) --
 *	Delete environment variable "name".
 */

extern "C" void
unsetenv (const char *name)
{
  register char **e;
  int offset;

  while (my_findenv (name, &offset))	/* if set multiple times */
    /* Move up the rest of the array */
    for (e = cur_environ () + offset; ; e++)
      if (!(*e = *(e + 1)))
	break;
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
      *p = toupper (*p);
}

/* Parse CYGWIN options */

static NO_COPY BOOL export_settings = FALSE;

enum settings
  {
    justset,
    isfunc,
    setbit,
    set_process_state,
  };

/* The structure below is used to set up an array which is used to
 * parse the CYGWIN environment variable or, if enabled, options from
 * the registry.
 */
struct parse_thing
  {
    const char *name;
    union parse_setting
      {
	BOOL *b;
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
  } known[] =
{
  {"binmode", {x: &binmode}, justset, NULL, {{0}, {O_BINARY}}},
  {"envcache", {&envcache}, justset, NULL, {{TRUE}, {FALSE}}},
  {"error_start", {func: &error_start_init}, isfunc, NULL, {{0}, {0}}},
  {"export", {&export_settings}, justset, NULL, {{FALSE}, {TRUE}}},
  {"forkchunk", {x: &chunksize}, justset, NULL, {{8192}, {0}}},
  {"glob", {&allow_glob}, justset, NULL, {{FALSE}, {TRUE}}},
  {"ntea", {&allow_ntea}, justset, NULL, {{FALSE}, {TRUE}}},
  {"ntsec", {&allow_ntsec}, justset, NULL, {{FALSE}, {TRUE}}},
  {"reset_com", {&reset_com}, justset, NULL, {{FALSE}, {TRUE}}},
  {"strip_title", {&strip_title_path}, justset, NULL, {{FALSE}, {TRUE}}},
  {"title", {&display_title}, justset, NULL, {{FALSE}, {TRUE}}},
  {"tty", {NULL}, set_process_state, NULL, {{0}, {PID_USETTY}}},
  {NULL, {0}, justset, 0, {{0}, {0}}}
};

/* Parse a string of the form "something=stuff somethingelse=more-stuff",
 * silently ignoring unknown "somethings".
 */
static void __stdcall
parse_options (char *buf)
{
  int istrue;
  char *p;
  parse_thing *k;

  if (buf == NULL)
    {
      char newbuf[MAX_PATH + 7] = "CYGWIN";
      for (k = known; k->name != NULL; k++)
	if (k->remember)
	  {
	    strcat (strcat (newbuf, " "), k->remember);
	    free (k->remember);
	    k->remember = NULL;
	  }
      if (!export_settings)
	return;
      newbuf[sizeof ("CYGWIN") - 1] = '=';
      debug_printf ("%s", newbuf);
      putenv (newbuf);
      return;
    }

  buf = strcpy ((char *) alloca (strlen (buf) + 1), buf);
  for (p = strtok (buf, " \t"); p != NULL; p = strtok (NULL, " \t"))
    {
      if (!(istrue = !strncasematch (p, "no", 2)))
	p += 2;
      else if (!(istrue = *p != '-'))
	p++;

      char ch, *eq;
      if ((eq = strchr (p, '=')) != NULL || (eq = strchr (p, ':')) != NULL)
	ch = *eq, *eq++ = '\0';
      else
	ch = 0;

      for (parse_thing *k = known; k->name != NULL; k++)
	if (strcasematch (p, k->name))
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
	    p = strdup (p);
	    if (n > 0)
	      p[n] = ':';
	    k->remember = p;
	    break;
	  }
      }
  debug_printf ("returning");
  return;
}

/* Set options from the registry. */

static void __stdcall
regopt (const char *name)
{
  MALLOC_CHECK;
  /* FIXME: should not be under mount */
  reg_key r (KEY_READ, CYGWIN_INFO_PROGRAM_OPTIONS_NAME, NULL);
  char buf[MAX_PATH];
  char lname[strlen(name) + 1];
  strlwr (strcpy (lname, name));
  MALLOC_CHECK;
  if (r.get_string (lname, buf, sizeof (buf) - 1, "") == ERROR_SUCCESS)
    parse_options (buf);
  else
    {
      reg_key r1 (HKEY_LOCAL_MACHINE, KEY_READ, CYGWIN_INFO_PROGRAM_OPTIONS_NAME, NULL);
      if (r1.get_string (lname, buf, sizeof (buf) - 1, "") == ERROR_SUCCESS)
	parse_options (buf);
    }
  MALLOC_CHECK;
}

/* Initialize the environ array.  Look for the CYGWIN environment
 * environment variable and set appropriate options from it.
 */
void
environ_init (int already_posix)
{
  char *rawenv = GetEnvironmentStrings ();
  int envsize, i;
  char *p;
  char *newp, **envp;
  int sawTERM = 0;
  static char cygterm[] = "TERM=cygwin";

  /* Allocate space for environment + trailing NULL + CYGWIN env. */
  envp = (char **) malloc ((4 + (envsize = 100)) * sizeof (char *));

  regopt ("default");
  if (myself->progname[0])
    regopt (myself->progname);

#ifdef NTSEC_ON_BY_DEFAULT
  /* Set ntsec explicit as default, if NT is running */
  if (os_being_run == winNT)
    allow_ntsec = TRUE;
#endif

  /* Current directory information is recorded as variables of the
     form "=X:=X:\foo\bar; these must be changed into something legal
     (we could just ignore them but maybe an application will
     eventually want to use them).  */
  for (i = 0, p = rawenv; *p != '\0'; p = strchr (p, '\0') + 1, i++)
    {
      newp = strdup (p);
      if (i >= envsize)
	envp = (char **) realloc (envp, (4 + (envsize += 100)) *
					    sizeof (char *));
      envp[i] = newp;
      if (*newp == '=')
	*newp = '!';
      char *eq;
      if ((eq = strchr (newp, '=')) == NULL)
	eq = strchr (newp, '\0');
      if (!parent_alive)
	ucenv (newp, eq);
      if (strncmp (newp, "TERM=", 5) == 0)
	sawTERM = 1;
      if (strncmp (newp, "CYGWIN=", sizeof("CYGWIN=") - 1) == 0)
	parse_options (newp + sizeof("CYGWIN=") - 1);
      if (*eq)
	posify (already_posix, envp + i, *++eq ? eq : --eq);
      debug_printf ("%s", envp[i]);
    }

  if (!sawTERM)
    envp[i++] = cygterm;
  envp[i] = NULL;
  __cygwin_environ = envp;
  update_envptrs ();
  parse_options (NULL);
  MALLOC_CHECK;
}

/* Function called by qsort to sort environment strings.
 */
static int
env_sort (const void *a, const void *b)
{
  const char **p = (const char **) a;
  const char **q = (const char **) b;

  return strcmp (*p, *q);
}

/* Create a Windows-style environment block, i.e. a typical character buffer
 * filled with null terminated strings, terminated by double null characters.
 * Converts environment variables noted in conv_envvars into win32 form
 * prior to placing them in the string.
 */
char * __stdcall
winenv (const char * const *envp, int keep_posix)
{
  int len, n, tl;
  const char * const *srcp;
  const char * *dstp;

  for (n = 0; envp[n]; n++)
    continue;

  const char *newenvp[n + 1];

  debug_printf ("envp %p, keep_posix %d", envp, keep_posix);

  for (tl = 0, srcp = envp, dstp = newenvp; *srcp; srcp++, dstp++)
    {
      len = strcspn (*srcp, "=") + 1;
      win_env *conv;

      if (!keep_posix && (conv = getwinenv (*srcp, *srcp + len)))
	*dstp = conv->native;
      else
	*dstp = *srcp;
      tl += strlen (*dstp) + 1;
      if ((*dstp)[0] == '!' && isdrive ((*dstp) + 1) && (*dstp)[3] == '=')
	{
	  char *p = (char *) alloca (strlen (*dstp) + 1);
	  strcpy (p, *dstp);
	  *p = '=';
	  *dstp = p;
	}
    }

  *dstp = NULL;		/* Terminate */

  int envlen = dstp - newenvp;
  debug_printf ("env count %d, bytes %d", envlen, tl);

  /* Windows programs expect the environment block to be sorted.  */
  qsort (newenvp, envlen, sizeof (char *), env_sort);

  /* Create an environment block suitable for passing to CreateProcess.  */
  char *ptr, *envblock;
  envblock = (char *) malloc (tl + 2);
  for (srcp = newenvp, ptr = envblock; *srcp; srcp++)
    {
      len = strlen (*srcp);
      memcpy (ptr, *srcp, len + 1);
      ptr += len + 1;
    }
  *ptr = '\0';		/* Two null bytes at the end */

  return envblock;
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
