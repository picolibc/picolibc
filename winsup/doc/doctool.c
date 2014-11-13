/* doctool.c

   Copyright 1998,1999,2000,2001,2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>

/* Building native in a cross-built directory is tricky.  Be careful,
and beware that you don't have the full portability stuff available to
you (like libiberty) */

/*****************************************************************************/

/* The list of extensions that may contain SGML snippets.  We check
   both cases in case the file system isn't case sensitive enough. */

struct {
  char *upper;
  char *lower;
  int is_sgml;
} extensions[] = {
  { ".C",	".c",		0 },
  { ".CC",	".cc",		0 },
  { ".H",	".h",		0 },
  { ".SGML",	".sgml",	1 },
  { 0, 0, 0 }
};

/*****************************************************************************/

void
show_help()
{
  printf("Usage: doctool [-m] [-i] [-d dir] [-o outfile] [-s prefix] \\\n");
  printf("                [-b book_id] infile\n");
  printf("  -m   means to adjust Makefile to include new dependencies\n");
  printf("  -i   means to include internal snippets\n");
  printf("  -d   means to recursively scan directory for snippets\n");
  printf("  -o   means to output to file (else stdout)\n");
  printf("  -s   means to suppress source dir prefix\n");
  printf("  -b   means to change the <book id=\"book_id\">\n");
  printf("\n");
  printf("doctool looks for DOCTOOL-START and DOCTOOL-END lines in source,\n");
  printf("saves <foo id=\"bar\"> blocks, and looks for DOCTOOL-INSERT-bar\n");
  printf("commands to insert selected sections.  IDs starting with int-\n");
  printf("are internal only, add- are added at the end of relevant sections\n");
  printf("or add-int- for both.  Inserted sections are chosen by prefix,\n");
  printf("and sorted when inserted.\n");
  exit(1);
}

/*****************************************************************************/

typedef struct Section {
  struct Section *next;
  struct OneFile *file;
  char *name;
  char internal;
  char addend;
  char used;
  char **lines;
  int num_lines;
  int max_lines;
} Section;

typedef struct OneFile {
  struct OneFile *next;
  char *filename;
  int enable_scan;
  int used;
  Section *sections;
} OneFile;

OneFile *file_list = 0;

char *output_name = 0;
FILE *output_file = 0;

char *source_dir_prefix = "";
char *book_id = 0;

int internal_flag = 0;

/*****************************************************************************/

char *
has_string(char *line, char *string)
{
  int i;
  while (*line)
  {
    for (i=0; line[i]; i++)
    {
      if (!string[i])
	return line;
      if (line[i] != string[i])
	break;
    }
    line++;
  }
  return 0;
}

int
starts_with(char *line, char *string)
{
  int i=0;
  while (1)
  {
    if (!string[i])
      return 1;
    if (!line[i] || line[i] != string[i])
      return 0;
    i++;
  }
}

/*****************************************************************************/

#ifdef S_ISLNK
#define STAT lstat
#else
#define STAT stat
#endif

void
scan_directory(dirname)
  char *dirname;
{
  struct stat st;
  char *name;
  struct dirent *de;
  DIR *dir = opendir(dirname);
  if (!dir)
    return;
  while (de = readdir(dir))
  {
    if (strcmp(de->d_name, ".") == 0
	|| strcmp(de->d_name, "..") == 0)
      continue;

    name = (char *)malloc(strlen(dirname)+strlen(de->d_name)+3);
    strcpy(name, dirname);
    strcat(name, "/");
    strcat(name, de->d_name);

    STAT(name, &st);

    if (S_ISDIR(st.st_mode) && strcmp(de->d_name, "CVS") != 0)
    {
      scan_directory(name);
    }

    else if (S_ISREG(st.st_mode))
    {
      char *dot = strrchr(de->d_name, '.');
      int i;

      if (dot)
      {
	for (i=0; extensions[i].upper; i++)
	  if (strcmp(dot, extensions[i].upper) == 0
	      || strcmp(dot, extensions[i].lower) == 0)
	  {
	    OneFile *one = (OneFile *)malloc(sizeof(OneFile));
	    one->next = file_list;
	    file_list = one;
	    one->filename = name;
	    one->enable_scan = ! extensions[i].is_sgml;
	    one->used = 0;
	    one->sections = 0;
	  }
      }
    }
  }
  closedir (dir);
}

/*****************************************************************************/

void
scan_file(OneFile *one)
{
  FILE *f = fopen(one->filename, "r");
  int enabled = ! one->enable_scan;
  char line[1000], *tag=0, *id=0, *tmp;
  int taglen = 0;
  Section *section = 0;
  Section **prev_section_ptr = &(one->sections);

  if (!f)
  {
    perror(one->filename);
    return;
  }

  while (fgets(line, 1000, f))
  {
    if (one->enable_scan)
    {
      /* source files have comment-embedded docs, check for them */
      if (has_string(line, "DOCTOOL-START"))
	enabled = 1;
      if (has_string(line, "DOCTOOL-END"))
	enabled = 0;
    }
    if (!enabled)
      continue;

    /* DOCTOOL-START

<sect1 id="dt-tags">
this is the doctool tags section.
</sect1>

       DOCTOOL-END */

    if (!tag && line[0] == '<')
    {
      tag = (char *)malloc(strlen(line)+1);
      id = (char *)malloc(strlen(line)+1);
      if (sscanf(line, "<%s id=\"%[^\"]\">", tag, id) == 2)
      {
	if (strcmp(tag, "book") == 0 || strcmp(tag, "BOOK") == 0)
	{
	  /* Don't want to "scan" these */
	  return;
	}
	taglen = strlen(tag);
	section = (Section *)malloc(sizeof(Section));
	/* We want chunks within single files to appear in that order */
	section->next = 0;
	section->file = one;
	*prev_section_ptr = section;
	prev_section_ptr = &(section->next);
	section->internal = 0;
	section->addend = 0;
	section->used = 0;
	section->name = id;
	if (starts_with(section->name, "add-"))
	{
	  section->addend = 1;
	  section->name += 4;
	}
	if (starts_with(section->name, "int-"))
	{
	  section->internal = 1;
	  section->name += 4;
	}
	section->lines = (char **)malloc(10*sizeof(char *));
	section->num_lines = 0;
	section->max_lines = 10;
      }
      else
      {
	free(tag);
	free(id);
	tag = id = 0;
      }
    }

    if (tag && section)
    {
      if (section->num_lines >= section->max_lines)
      {
	section->max_lines += 10;
	section->lines = (char **)realloc(section->lines,
					  section->max_lines * sizeof (char *));
      }
      section->lines[section->num_lines] = (char *)malloc(strlen(line)+1);
      strcpy(section->lines[section->num_lines], line);
      section->num_lines++;

      if (line[0] == '<' && line[1] == '/'
	  && memcmp(line+2, tag, taglen) == 0
	  && (isspace(line[2+taglen]) || line[2+taglen] == '>'))
      {
	/* last line! */
	tag = 0;
      }
    }
  }
  fclose(f);
}

/*****************************************************************************/

Section **
enumerate_matching_sections(char *name_prefix, int internal, int addend, int *count_ret)
{
  Section **rv = (Section **)malloc(12*sizeof(Section *));
  int count = 0, max=10, prefix_len = strlen(name_prefix);
  OneFile *one;
  int wildcard = 0;

  if (name_prefix[strlen(name_prefix)-1] == '-')
    wildcard = 1;

  for (one=file_list; one; one=one->next)
  {
    Section *s;
    for (s=one->sections; s; s=s->next)
    {
      int matches = 0;
      if (wildcard)
      {
	if (starts_with(s->name, name_prefix))
	  matches = 1;
      }
      else
      {
	if (strcmp(s->name, name_prefix) == 0)
	  matches = 1;
      }
      if (s->internal <= internal
	  && s->addend == addend
	  && matches
	  && ! s->used)
      {
	s->used = 1;
	if (count >= max)
	{
	  max += 10;
	  rv = (Section **)realloc(rv, max*sizeof(Section *));
	}
	rv[count++] = s;
	rv[count] = 0;
      }
    }
  }
  if (count_ret)
    *count_ret = count;
  return rv;
}

/*****************************************************************************/

#define ID_CHARS "~@$%&()_-+[]{}:."

void include_section(char *name, int addend);

char *
unprefix(char *fn)
{
  int l = strlen(source_dir_prefix);
  if (memcmp(fn, source_dir_prefix, l) == 0)
  {
    fn += l;
    while (*fn == '/' || *fn == '\\')
      fn++;
    return fn;
  }
  return fn;
}

void
parse_line(char *line, char *filename)
{
  char *cmd = has_string(line, "DOCTOOL-INSERT-");
  char *sname, *send, *id, *save;
  if (!cmd)
  {
    if (book_id
	&& (starts_with(line, "<book") || starts_with(line, "<BOOK")))
    {
      cmd = strchr(line, '>');
      if (cmd)
      {
	cmd++;
	fprintf(output_file, "<book id=\"%s\">", book_id);
	fputs(cmd, output_file);
	return;
      }
    }
    fputs(line, output_file);
    return;
  }
  if (cmd != line)
    fwrite(line, cmd-line, 1, output_file);
  save = (char *)malloc(strlen(line)+1);
  strcpy(save, line);
  line = save;

  sname = cmd + 15; /* strlen("DOCTOOL-INSERT-") */
  for (send = sname;
       *send && isalnum(*send) || strchr(ID_CHARS, *send);
       send++);
  id = (char *)malloc(send-sname+2);
  memcpy(id, sname, send-sname);
  id[send-sname] = 0;
  include_section(id, 0);

  fprintf(output_file, "<!-- %s -->\n", unprefix(filename));

  fputs(send, output_file);
  free(save);
}

int
section_sort(const void *va, const void *vb)
{
  Section *a = *(Section **)va;
  Section *b = *(Section **)vb;
  int rv = strcmp(a->name, b->name);
  if (rv)
    return rv;
  return a->internal - b->internal;
}

void
include_section(char *name, int addend)
{
  Section **sections, *s;
  int count, i, l;

  sections = enumerate_matching_sections(name, internal_flag, addend, &count);

  qsort(sections, count, sizeof(sections[0]), section_sort);
  for (i=0; i<count; i++)
  {
    s = sections[i];
    s->file->used = 1;
    fprintf(output_file, "<!-- %s -->\n", unprefix(s->file->filename));
    for (l=addend; l<s->num_lines-1; l++)
      parse_line(s->lines[l], s->file->filename);
    if (!addend)
    {
      include_section(s->name, 1);
      parse_line(s->lines[l], s->file->filename);
    }
  }

  free(sections);
}

void
parse_sgml(FILE *in, char *input_name)
{
  static char line[1000];
  while (fgets(line, 1000, in))
  {
    parse_line(line, input_name);
  }
}

/*****************************************************************************/

void
fix_makefile(char *output_name)
{
  FILE *in, *out;
  char line[1000];
  int oname_len = strlen(output_name);
  OneFile *one;
  int used_something = 0;
  struct stat st;
  struct utimbuf times;

  stat("Makefile", &st);

  in = fopen("Makefile", "r");
  if (!in)
  {
    perror("Makefile");
    return;
  }

  out = fopen("Makefile.new", "w");
  if (!out)
  {
    perror("Makefile.new");
    return;
  }

  while (fgets(line, 1000, in))
  {
    if (starts_with(line, output_name)
	&& strcmp(line+oname_len, ": \\\n") == 0)
    {
      /* this is the old dependency */
      while (fgets(line, 1000, in))
      {
	if (strcmp(line+strlen(line)-2, "\\\n"))
	  break;
      }
    }
    else
      fputs(line, out);
  }
  fclose(in);

  for (one=file_list; one; one=one->next)
    if (one->used)
    {
      used_something = 1;
      break;
    }

  if (used_something)
  {
    fprintf(out, "%s:", output_name);
    for (one=file_list; one; one=one->next)
      if (one->used)
	fprintf(out, " \\\n\t%s", one->filename);
    fprintf(out, "\n");
  }

  fclose(out);

  times.actime = st.st_atime;
  times.modtime = st.st_mtime;
  utime("Makefile.new", &times);

  if (rename("Makefile", "Makefile.old"))
    return;
  if (rename("Makefile.new", "Makefile"))
    rename("Makefile.old", "Makefile");
}

/*****************************************************************************/

int
main(argc, argv)
  int argc;
  char **argv;
{
  int i;
  OneFile *one;
  FILE *input_file;
  int fix_makefile_flag = 0;

  while (argc > 1 && argv[1][0] == '-')
  {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
      show_help();
    }
    else if (strcmp(argv[1], "-i") == 0)
    {
      internal_flag = 1;
    }
    else if (strcmp(argv[1], "-m") == 0)
    {
      fix_makefile_flag = 1;
    }
    else if (strcmp(argv[1], "-d") == 0 && argc > 2)
    {
      scan_directory(argv[2]);
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-o") == 0 && argc > 2)
    {
      output_name = argv[2];
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-s") == 0 && argc > 2)
    {
      source_dir_prefix = argv[2];
      argc--;
      argv++;
    }
    else if (strcmp(argv[1], "-b") == 0 && argc > 2)
    {
      book_id = argv[2];
      argc--;
      argv++;
    }

    argc--;
    argv++;
  }

  for (one=file_list; one; one=one->next)
  {
    scan_file(one);
  }

  input_file = fopen(argv[1], "r");
  if (!input_file)
  {
    perror(argv[1]);
    return 1;
  }

  if (output_name)
  {
    output_file = fopen(output_name, "w");
    if (!output_file)
    {
      perror(output_name);
      return 1;
    }
  }
  else
  {
    output_file = stdout;
    output_name = "<stdout>";
  }

  parse_sgml(input_file, argv[1]);

  if (output_file != stdout)
    fclose(output_file);

  if (fix_makefile_flag)
    fix_makefile(output_name);

  return 0;
}
