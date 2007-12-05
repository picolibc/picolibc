/* netdb.cc: network database related routines.

   Copyright 2002, 2003, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <windows.h>
#include <sys/cygwin.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

/* Locate and open a system network database file.  relative_path
 should be one of the following values:
 "protocol"
 "services"
 "networks"
 "hosts"

 It is the callers responsibility to close the file.  */
static FILE *
open_system_file (const char *relative_path)
{
  /* system dir path is never longer. */
  char win32_name[MAX_PATH];

  if (!GetSystemDirectory (win32_name, MAX_PATH))
    return NULL;
  strcat (win32_name, "\\drivers\\etc\\");
  strcat (win32_name, relative_path);
  debug_printf ("netdb file to open %s", win32_name);
  FILE *result = fopen (win32_name, "rt");
  debug_printf ("handle to netdb file %p", result);
  return result;
}

inline static FILE *
open_protocol_file ()
{
  return open_system_file ("protocol");
}

/* Wrapper for open_system_file(), fixing the constant name
"services".  Returns the open file. */
inline static FILE *
open_services_file ()
{
  return open_system_file ("services");
}

/* Read an entire line up till the next \n character.  Memory for the
line is dynamically allocated, and the caller must call free() to
deallocate it.  When the end of file is reached, NULL is returned.  */
static char *
get_entire_line (FILE *fd)
{
  static const int BUFF_SIZE = 1024;
  struct line_fragment
  {
    char buffer[BUFF_SIZE];
    line_fragment *next;
  };

  line_fragment *fragment_list_head = NULL;
  line_fragment *fragment = NULL;
  int fragment_count = 0;
  char *result;

  do
    {
      line_fragment *new_fragment = (line_fragment *) malloc (sizeof (line_fragment));
      paranoid_printf ("line fragment allocated %p", new_fragment);
      if (!fragment_list_head)
	fragment_list_head = new_fragment;
      if (fragment)
	fragment->next = new_fragment;
      fragment = new_fragment;
      fragment->next = NULL;
      *fragment->buffer = '\0';
      result = fgets (fragment->buffer, BUFF_SIZE, fd);
      ++fragment_count;
    }
  while (result && !strchr (fragment->buffer, '\n'));

  if (*fragment_list_head->buffer != '\0')
    {
      char *concatenated_line = (char *) calloc (fragment_count * BUFF_SIZE , sizeof (char));
      paranoid_printf ("concatenated line allocated %p", concatenated_line);
      *concatenated_line = '\0';
      fragment = fragment_list_head;
      while (fragment != NULL)
	{
	  line_fragment *previous = fragment;
	  strcat (concatenated_line, fragment->buffer);
	  fragment = fragment->next;
	  free (previous);
	}
      return concatenated_line;
    }
  else
    {
      fragment = fragment_list_head;
      while (fragment != NULL)
	{
	  line_fragment *previous = fragment;
	  fragment = fragment->next;
	  free (previous);
	}
      return NULL;
    }
}

/* Characters representing whitespace.  Used by parse_* routines to
delimit tokens.  */
static const NO_COPY char *SPACE = " \t\n\r\f";

/* Parse a list aliases from a network database file.  Returns a
char** structure terminated by a NULL. */
static void
parse_alias_list (char ***aliases, char **lasts)
{
  struct alias_t
  {
    char *alias_name;
    alias_t *next;
  };
  alias_t *alias_list_head = NULL, *alias_list_tail = NULL;
  char *alias;
  int alias_count = 0;
  alias = strtok_r (NULL, SPACE, lasts);

  while (alias)
    {
      ++alias_count;
      alias_t *new_alias = (alias_t *) malloc (sizeof (alias_t));
      paranoid_printf ("new alias alloc %p", new_alias);
      if (!alias_list_head)
	alias_list_head = new_alias;
      if (alias_list_tail)
	alias_list_tail->next = new_alias;
      new_alias->next = NULL;
      new_alias->alias_name = alias;
      alias_list_tail = new_alias;
      alias = strtok_r (NULL, SPACE, lasts);
    }

  *aliases = (char**) calloc (alias_count + 1, sizeof (char *));
  paranoid_printf ("aliases alloc %p", *aliases);

  char **current_entry = *aliases;
  while (alias_list_head)
    {
      alias_t *previous = alias_list_head;
      *current_entry = strdup (alias_list_head->alias_name);
      paranoid_printf ("*current entry strdup %p", *current_entry);
      alias_list_head = alias_list_head->next;
      free (previous);
      ++current_entry;
    }

  *current_entry = NULL;
}

/* Read the next line from svc_file, and parse it into the structure
pointed to by sep.  sep can point to stack or static data, but it's
members will be overwritten with pointers to dynamically allocated
heap data accommodating parsed data.  It is the responsibility of the
caller to free up the allocated structures. The function returns true
to indicate that a line was successfully read and parsed.  False is
used to indicate that no more lines can be read and parsed.  This
should also interpreted as end of file. */
static bool
parse_services_line (FILE *svc_file, struct servent *sep)
{
  char *line;
  while ((line = get_entire_line (svc_file)))
    {
      char *name, *port, *protocol, *lasts;

      line[strcspn (line, "#")] = '\0'; // truncate at comment marker.
      name = strtok_r (line, SPACE, &lasts);
      if (!name)
	{
	  free (line);
	  continue;
	}
      port = strtok_r (NULL, SPACE, &lasts);
      protocol = strchr (port, '/');
      *protocol++ = '\0';
      sep->s_name = strdup (name);
      paranoid_printf ("sep->s_name strdup %p", sep->s_name);
      sep->s_port = atoi (port);
      sep->s_proto = strdup (protocol);
      paranoid_printf ("sep->s_proto strdup %p", sep->s_proto);
      /* parse_alias_list relies on side effects.  Read the comments
	 for that function.*/
      parse_alias_list (& sep->s_aliases, &lasts);
      free (line);
      return true;
    }
  return false;
}

static FILE *svc_file = NULL;
static long int svc_read_pos = 0;
static struct servent current_servent;

/* Steps through a struct servent, and frees all of the internal
structures.*/
static void
free_servent (struct servent *sep)
{
  free (sep->s_name);
  free (sep->s_proto);
  char ** current = sep->s_aliases;
  while (current && *current)
    {
      free (*current);
      ++current;
    }
  free (sep->s_aliases);
  sep->s_name = NULL;
  sep->s_port = 0;
  sep->s_proto = NULL;
  sep->s_aliases = NULL;
}

extern "C" void
cygwin_setservent (int stay_open)
{
  if (svc_file)
    fclose (svc_file);
  if (stay_open)
    svc_file = open_services_file ();
  free_servent (&current_servent);
  svc_read_pos = 0;
  syscall_printf ("setservent (%d)", stay_open);
}

extern "C" struct servent *
cygwin_getservent (void)
{
  FILE *fd;
  if (svc_file)
    fd = svc_file;
  else
    {
      fd = open_services_file ();
      if (!fd)
	{
	  syscall_printf ("%p = getservent()", NULL);
	  return NULL;
	}
      fseek (fd, svc_read_pos, SEEK_SET);
    }
  free_servent (&current_servent);
  bool found = parse_services_line (fd, &current_servent);
  if (!svc_file)
    {
      svc_read_pos = ftell (fd);
      fclose (fd);
    }
  struct servent *result;
  if (found)
    result = &current_servent;
  else
    result = NULL;
  syscall_printf ("%p = getservent()", result);
  return result;
}

extern "C" void
cygwin_endservent (void)
{
  if (svc_file)
    {
      fclose (svc_file);
      svc_file = NULL;
    }
  free_servent (&current_servent);
  svc_read_pos = 0;
  syscall_printf ("endservent ()");
}

/* Read the next line from proto_file, and parse it into the structure
pointed to by pep.  pep can point to stack or static data, but it's
members will be overwritten with pointers to dynamically allocated
heap data accommodating parsed data.  It is the responsibility of the
caller to free up the allocated structures. The function returns true
to indicate that a line was successfully read and parsed.  False is
used to indicate that no more lines can be read and parsed.  This
should also interpreted as end of file. */
static bool
parse_protocol_line (FILE *proto_file, struct protoent *pep)
{
  char *line;
  while ((line = get_entire_line (proto_file)))
    {
      char *name, *protocol, *lasts;

      line[strcspn (line, "#")] = '\0'; // truncate at comment marker.
      name = strtok_r (line, SPACE, &lasts);
      if (!name)
	{
	  free (line);
	  continue;
	}
      protocol = strtok_r (NULL, SPACE, &lasts);
      pep->p_name = strdup (name);
      paranoid_printf ("pep->p_name strdup %p", pep->p_name);
      pep->p_proto = atoi (protocol);
      /* parse_alias_list relies on side effects.  Read the comments
	 for that function.*/
      parse_alias_list (& pep->p_aliases, &lasts);
      free (line);
      return true;
    }
  return false;
}

static FILE *proto_file = NULL;
static long int proto_read_pos = 0;
static struct protoent current_protoent;

/* Steps through a struct protoent, and frees all the internal
structures.  */
static void
free_protoent (struct protoent *pep)
{
  free (pep->p_name);
  char ** current = pep->p_aliases;
  while (current && *current)
    {
      free (*current);
      ++current;
    }
  free (pep->p_aliases);
  pep->p_name = NULL;
  pep->p_proto = 0;
  pep->p_aliases = NULL;
}

extern "C" void
cygwin_setprotoent (int stay_open)
{
  if (proto_file)
    fclose (proto_file);

  if (stay_open)
    proto_file = open_protocol_file ();

  free_protoent (&current_protoent);
  proto_read_pos = 0;
  syscall_printf ("setprotoent (%d)", stay_open);
}

extern "C" struct protoent *
cygwin_getprotoent (void)
{
  FILE *fd;

  if (proto_file)
    fd = proto_file;
  else
    {
      fd = open_protocol_file ();
      if (!fd)
	{
	  syscall_printf ("%p = getprotoent()", NULL);
	  return NULL;
	}
      fseek (fd, proto_read_pos, SEEK_SET);
    }
  free_protoent (&current_protoent);

  bool found = parse_protocol_line (fd, &current_protoent);
  if (!proto_file)
    {
      proto_read_pos = ftell (fd);
      fclose (fd);
    }

  struct protoent *result;
  if (found)
    result =  &current_protoent;
  else
    result =  NULL;

  syscall_printf ("%p = getprotoent()", result);
  return result;
}

extern "C" void
cygwin_endprotoent (void)
{
  if (proto_file)
    {
      fclose (proto_file);
      proto_file = NULL;
    }

  free_protoent (&current_protoent);
  proto_read_pos = 0;
  syscall_printf ("endprotoent ()");
}
