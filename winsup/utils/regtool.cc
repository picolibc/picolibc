/* regtool.cc

   Copyright 2000 Red Hat Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <windows.h>

enum {
  KT_AUTO, KT_INT, KT_STRING, KT_EXPAND, KT_MULTI
} key_type = KT_AUTO;

int verbose = 0;
int quiet = 0;
char **argv;

HKEY key;
char *value;

const char *usage_msg[] =
{
  "Regtool Copyright (c) 2000 Red Hat Inc",
  " regtool -h  - print this message",
  " regtool [-v] list [key]  - list subkeys and values",
  " regtool [-v] add [key\\subkey]  - add new subkey",
  " regtool [-v] remove [key]  - remove key",
  " regtool [-v|-q] check [key]  - exit 0 if key exists, 1 if not",
  " regtool [-i|-s|-e|-m] set [key\\value] [data ...]  - set value",
  "     -i=integer -s=string -e=expand-string -m=multi-string",
  " regtool [-v] unset [key\\value]  - removes value from key",
  " regtool [-q] get [key\\value]  - prints value to stdout",
  "     -q=quiet, no error msg, just return nonzero exit if key/value missing",
  " keys are like \\prefix\\key\\key\\key\\value, where prefix is any of:",
  "   root     HKCR  HKEY_CLASSES_ROOT",
  "   config   HKCC  HKEY_CURRENT_CONFIG",
  "   user     HKCU  HKEY_CURRENT_USER",
  "   machine  HKLM  HKEY_LOCAL_MACHINE",
  "   users    HKU   HKEY_USERS",
  " example: \\user\\software\\Microsoft\\Clock\\iFormat",
  0
};

void
usage(void)
{
  int i;
  for (i=0; usage_msg[i]; i++)
    fprintf(stderr, "%s\n", usage_msg[i]);
  exit(1);
}

void
Fail(DWORD rv)
{
  char *buf;
  if (!quiet)
    {
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
		    | FORMAT_MESSAGE_FROM_SYSTEM,
		    0, rv, 0, (CHAR *)&buf, 0, 0);
      fprintf(stderr, "Error: %s\n", buf);
    }
  exit(1);
}

struct {
  const char *string;
  HKEY key;
} wkprefixes[] = {
  { "root", HKEY_CLASSES_ROOT },
  { "HKCR", HKEY_CLASSES_ROOT },
  { "HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT },
  { "config", HKEY_CURRENT_CONFIG },
  { "HKCC", HKEY_CURRENT_CONFIG },
  { "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG },
  { "user", HKEY_CURRENT_USER },
  { "HKCU", HKEY_CURRENT_USER },
  { "HKEY_CURRENT_USER", HKEY_CURRENT_USER },
  { "machine", HKEY_LOCAL_MACHINE },
  { "HKLM", HKEY_LOCAL_MACHINE },
  { "HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE },
  { "users", HKEY_USERS },
  { "HKU", HKEY_USERS },
  { "HKEY_USERS", HKEY_USERS },
  { 0, 0 }
};

void translate(char *key)
{
#define isodigit(c) (strchr("01234567", c))
#define tooct(c)    ((c)-'0')
#define tohex(c)    (strchr(_hs,tolower(c))-_hs)
  static char _hs[] = "0123456789abcdef";

  char *d = key;
  char *s = key;
  char c;

  while (*s)
    {
      if (*s == '\\')
        switch (*++s)
          {
          case 'a':
            *d++ = '\007';
            break;
          case 'b':
            *d++ = '\b';
            break;
          case 'e':
            *d++ = '\033';
            break;
          case 'f':
            *d++ = '\f';
            break;
          case 'n':
            *d++ = '\n';
            break;
          case 'r':
            *d++ = '\r';
            break;
          case 't':
            *d++ = '\t';
            break;
          case 'v':
            *d++ = '\v';
            break;
          case '0': case '1': case '2': case '3':
          case '4': case '5': case '6': case '7':
            c = tooct(*s);
            if (isodigit(s[1]))
              {
                c = (c << 3) | tooct(*++s);
                if (isodigit(s[1]))
                  c = (c << 3) | tooct(*++s);
              }
            *d++ = c;
            break;
          case 'x':
            if (isxdigit(s[1]))
              {
                c = tohex(*++s);
                if (isxdigit(s[1]))
                  c = (c << 4) | tohex(*++s);
              }
            *d++ = c;
            break;
          default:     /* before non-special char: just add the char */
            *d++ = *s;
            break;
          }
      else if (*s == '/')
        *d++ = '\\';
      else
        *d++ = *s;
      ++s;
    }
  *d = '\0';
}

void
find_key(int howmanyparts, REGSAM access)
{
  char *n = argv[0], *e, c;
  int i;
  if (*n == '/')
    translate(n);
  while (*n == '\\')
    n++;
  for (e=n; *e && *e != '\\'; e++);
  c = *e;
  *e = 0;
  for (i=0; wkprefixes[i].string; i++)
    if (strcmp(wkprefixes[i].string, n) == 0)
      break;
  if (!wkprefixes[i].string)
    {
      fprintf(stderr, "Unknown key prefix.  Valid prefixes are:\n");
      for (i=0; wkprefixes[i].string; i++)
	fprintf(stderr, "\t%s\n", wkprefixes[i].string);
      exit(1);
    }

  n = e;
  *e = c;
  while (*n && *n == '\\')
    n++;
  e = n+strlen(n);
  if (howmanyparts > 1)
    {
      while (n < e && *e != '\\')
	e--;
      if (*e != '\\')
	{
	  fprintf(stderr, "Invalid key\n");
	  exit(1);
	}
      *e = 0;
      value = e+1;
    }
  if (n[0] == 0)
    {
      key = wkprefixes[i].key;
      return;
    }
  int rv = RegOpenKeyEx(wkprefixes[i].key, n, 0, access, &key);
  if (rv != ERROR_SUCCESS)
    Fail(rv);
  //printf("key `%s' value `%s'\n", n, value);
}


int
cmd_list()
{
  DWORD num_subkeys, maxsubkeylen, num_values, maxvalnamelen, maxvaluelen;
  DWORD maxclasslen;
  char *subkey_name, *value_name, *class_name;
  unsigned char *value_data, *vd;
  DWORD i, j, m, n, t;
  int v;

  find_key(1, KEY_READ);
  RegQueryInfoKey(key, 0, 0, 0, &num_subkeys, &maxsubkeylen, &maxclasslen,
		  &num_values, &maxvalnamelen, &maxvaluelen, 0, 0);

  subkey_name = (char *)malloc(maxsubkeylen+1);
  class_name = (char *)malloc(maxclasslen+1);
  value_name = (char *)malloc(maxvalnamelen+1);
  value_data = (unsigned char *)malloc(maxvaluelen+1);

  for (i=0; i<num_subkeys; i++)
    {
      m = maxsubkeylen+1;
      n = maxclasslen+1;
      RegEnumKeyEx(key, i, subkey_name, &m, 0, class_name, &n, 0);
      if (verbose)
	printf("%s\\ (%s)\n", subkey_name, class_name);
      else
	printf("%s\n", subkey_name);
    }

  for (i=0; i<num_values; i++)
    {
      m = maxvalnamelen+1;
      n = maxvaluelen+1;
      RegEnumValue(key, i, value_name, &m, 0, &t, (BYTE *)value_data, &n);
      if (!verbose)
	printf("%s\n", value_name);
      else
	{
	  printf("%s = ", value_name);
	  switch (t)
	    {
	    case REG_BINARY:
	      for (j=0; j<8 && j<n; j++)
		printf("%02x ", value_data[j]);
	      printf("\n");
	      break;
	    case REG_DWORD:
	      printf("0x%08lx (%lu)\n", *(DWORD *)value_data,
		     *(DWORD *)value_data);
	      break;
	    case REG_DWORD_BIG_ENDIAN:
	      v = ((value_data[0] << 24)
		   | (value_data[1] << 16)
		   | (value_data[2] << 8)
		   | (value_data[3]));
	      printf("0x%08x (%d)\n", v, v);
	      break;
	    case REG_EXPAND_SZ:
	    case REG_SZ:
	      printf("\"%s\"\n", value_data);
	      break;
	    case REG_MULTI_SZ:
	      vd = value_data;
	      while (vd && *vd)
		{
		  printf("\"%s\"", vd);
		  vd = vd+strlen((const char *)vd) + 1;
		  if (*vd)
		    printf(", ");
		}
	      printf("\n");
	      break;
	    default:
	      printf("? (type %d)\n", (int)t);
	    }
	}
    }
  return 0;
}

int
cmd_add()
{
  find_key(2, KEY_ALL_ACCESS);
  HKEY newkey;
  DWORD newtype;
  int rv = RegCreateKeyEx(key, value, 0, (char *)"", REG_OPTION_NON_VOLATILE,
			  KEY_ALL_ACCESS, 0, &newkey, &newtype);
  if (rv != ERROR_SUCCESS)
    Fail(rv);

  if (verbose)
    {
      if (newtype == REG_OPENED_EXISTING_KEY)
	printf("Key %s already exists\n", value);
      else
	printf("Key %s created\n", value);
    }
  return 0;
}

int
cmd_remove()
{
  find_key(2, KEY_ALL_ACCESS);
  DWORD rv = RegDeleteKey(key, value);
  if (rv != ERROR_SUCCESS)
    Fail(rv);
  if (verbose)
    printf("subkey %s deleted\n", value);
  return 0;
}

int
cmd_check()
{
  find_key(1, KEY_READ);
  if (verbose)
    printf("key %s exists\n", argv[0]);
  return 0;
}

int
cmd_set()
{
  int i, n;
  DWORD v, rv;
  char *a = argv[1], *data;
  find_key(2, KEY_ALL_ACCESS);

  if (key_type == KT_AUTO)
    {
      char *e;
      strtoul(a, &e, 0);
      if (a[0] == '%')
	key_type = KT_EXPAND;
      else if (a[0] && !*e)
	key_type = KT_INT;
      else if (argv[2])
	key_type = KT_MULTI;
      else
	key_type = KT_STRING;
    }

  switch (key_type)
    {
    case KT_INT:
      v = strtoul(a, 0, 0);
      rv = RegSetValueEx(key, value, 0, REG_DWORD, (const BYTE *)&v, sizeof(v));
      break;
    case KT_STRING:
      rv = RegSetValueEx(key, value, 0, REG_SZ, (const BYTE *)a, strlen(a));
      break;
    case KT_EXPAND:
      rv = RegSetValueEx(key, value, 0, REG_EXPAND_SZ, (const BYTE *)a, strlen(a));
      break;
    case KT_MULTI:
      for (i=1, n=1; argv[i]; i++)
	n += strlen(argv[i])+1;
      data = (char *)malloc(n);
      for (i=1, n=0; argv[i]; i++)
	{
	  strcpy(data+n, argv[i]);
	  n += strlen(argv[i])+1;
	}
      data[n] = 0;
      rv = RegSetValueEx(key, value, 0, REG_MULTI_SZ, (const BYTE *)data, n+1);
      break;
    case KT_AUTO:
      break;
    }

  if (rv != ERROR_SUCCESS)
    Fail(rv);

  return 0;
}

int
cmd_unset()
{
  find_key(2, KEY_ALL_ACCESS);
  DWORD rv = RegDeleteValue(key, value);
  if (rv != ERROR_SUCCESS)
    Fail(rv);
  if (verbose)
    printf("value %s deleted\n", value);
  return 0;
}

int
cmd_get()
{
  find_key(2, KEY_READ);
  DWORD vtype, dsize, rv;
  char *data, *vd;
  rv = RegQueryValueEx(key, value, 0, &vtype, 0, &dsize);
  if (rv != ERROR_SUCCESS)
    Fail(rv);
  dsize++;
  data = (char *)malloc(dsize);
  rv = RegQueryValueEx(key, value, 0, &vtype, (BYTE *)data, &dsize);
  if (rv != ERROR_SUCCESS)
    Fail(rv);
  switch (vtype)
    {
    case REG_BINARY:
      fwrite(data, dsize, 0, stdout);
      break;
    case REG_DWORD:
      printf("%lu\n", *(DWORD *)data);
      break;
    case REG_SZ:
      printf("%s\n", data);
      break;
    case REG_EXPAND_SZ:
      if (key_type == KT_EXPAND) // hack
	{
	  char *buf;
	  DWORD bufsize;
	  bufsize = ExpandEnvironmentStrings(data, 0, 0);
	  buf = (char *)malloc(bufsize+1);
	  ExpandEnvironmentStrings(data, buf, bufsize+1);
	  data = buf;
	}
      printf("%s\n", data);
      break;
    case REG_MULTI_SZ:
      vd = data;
      while (vd && *vd)
	{
	  printf("%s\n", vd);
	  vd = vd+strlen((const char *)vd) + 1;
	}
      break;
    }
  return 0;
}

struct {
  const char *name;
  int (*func)();
} commands[] = {
  { "list", cmd_list },
  { "add", cmd_add },
  { "remove", cmd_remove },
  { "check", cmd_check },
  { "set", cmd_set },
  { "unset", cmd_unset },
  { "get", cmd_get },
  { 0, 0 }
};

int
main(int argc, char **_argv)
{
  while (1)
    {
      int g = getopt (argc, _argv, "hvqisem");
      if (g == -1)
	break;
      switch (g)
      {
      case 'v':
	verbose ++;
	break;
      case 'q':
	quiet ++;
	break;

      case 'i':
	key_type = KT_INT;
	break;
      case 's':
	key_type = KT_STRING;
	break;
      case 'e':
	key_type = KT_EXPAND;
	break;
      case 'm':
	key_type = KT_MULTI;
	break;

      case '?':
      case 'h':
	usage();
      }
    }
  if (_argv[optind] == NULL)
    usage();

  argv = _argv+optind;
  int i;
  for (i=0; commands[i].name; i++)
    if (strcmp(commands[i].name, argv[0]) == 0)
      {
	argv++;
	return commands[i].func();
      }
  usage();

  return 0;
}
