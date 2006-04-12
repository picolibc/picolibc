/* winf.h

   Copyright 2003, 2004, 2005 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _WINF_H
#define _WINF_H

#define MAXWINCMDLEN 32767
#define LINE_BUF_CHUNK (CYG_MAX_PATH * 2)

class av
{
  char **argv;
  int calloced;
 public:
  int argc;
  bool win16_exe;
  bool iscui;
  av (): argv (NULL), iscui (false) {}
  av (int ac_in, const char * const *av_in) : calloced (0), argc (ac_in), win16_exe (false), iscui (false)
  {
    argv = (char **) cmalloc (HEAP_1_ARGV, (argc + 5) * sizeof (char *));
    memcpy (argv, av_in, (argc + 1) * sizeof (char *));
  }
  void *operator new (size_t, void *p) __attribute__ ((nothrow)) {return p;}
  void set (int ac_in, const char * const *av_in) {new (this) av (ac_in, av_in);}
  ~av ()
  {
    if (argv)
      {
	for (int i = 0; i < calloced; i++)
	  if (argv[i])
	    cfree (argv[i]);
	cfree (argv);
      }
  }
  int unshift (const char *what, int conv = 0);
  operator char **() {return argv;}
  void all_calloced () {calloced = argc;}
  void replace0_maybe (const char *arg0)
  {
    /* Note: Assumes that argv array has not yet been "unshifted" */
    if (!calloced)
      {
	argv[0] = cstrdup1 (arg0);
	calloced = true;
      }
  }
  void dup_maybe (int i)
  {
    if (i >= calloced)
      argv[i] = cstrdup1 (argv[i]);
  }
  void dup_all ()
  {
    for (int i = calloced; i < argc; i++)
      argv[i] = cstrdup1 (argv[i]);
  }
  int fixup (const char *, path_conv&, const char *);
};

class linebuf
{
 public:
  size_t ix;
  char *buf;
  size_t alloced;
  linebuf () : ix (0), buf (NULL), alloced (0) {}
  ~linebuf () {if (buf) free (buf);}
  void add (const char *what, int len) __attribute__ ((regparm (3)));
  void add (const char *what) {add (what, strlen (what));}
  void prepend (const char *what, int len);
  void finish () __attribute__ ((regparm (1)));
  bool fromargv(av&, char *);
  operator char *() {return buf;}
};

#endif /*_WINF_H*/
