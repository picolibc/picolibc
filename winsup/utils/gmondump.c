/*
    gmondump.c
    Displays summary info about given profile data file(s).

    Written by Mark Geisert <mark@maxrnd.com>.

    This file is part of Cygwin.

    This software is a copyrighted work licensed under the terms of the
    Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details.
*/

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cygwin/version.h"

typedef unsigned short ushort;
typedef uint16_t u_int16_t; // Non-standard sized type needed by ancient gmon.h
#include "gmon.h"

FILE       *ofile;
const char *pgm = "gmondump";
int         verbose = 0;

void __attribute__ ((__noreturn__))
usage (FILE *where)
{
  fprintf (where, "\
Usage: %s [OPTIONS] FILENAME...\n\
\n\
Display formatted contents of profile data file(s).\n\
Such files usually have names starting with \"gmon.out\".\n\
OPTIONS are:\n\
\n\
  -h, --help             Display usage information and exit\n\
  -v, --verbose          Display more file details (toggle: default false)\n\
  -V, --version          Display version information and exit\n\
\n", pgm);

  exit (where == stderr ? 1 : 0 );
}

void __attribute__ ((__noreturn__))
usage1 (FILE *where)
{
  fprintf (where, "Usage: %s [OPTIONS] FILENAME...\n", pgm);

  exit (where == stderr ? 1 : 0 );
}

void
note (const char *fmt, ...)
{
  va_list args;
  char    buf[4096];

  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);

  fputs (buf, ofile);
  fflush (ofile);
}

void
warn (int geterrno, const char *fmt, ...)
{
  va_list args;
  char    buf[4096];

  va_start (args, fmt);
  sprintf (buf, "%s: ", pgm);
  vsprintf (strchr (buf, '\0'), fmt, args);
  va_end (args);
  if (geterrno)
    perror (buf);
  else
    {
      fputs (buf, ofile);
      fputs ("\n", ofile);
      fflush (ofile);
    }
}

void __attribute__ ((noreturn))
error (int geterrno, const char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  warn (geterrno, fmt, args);
  va_end (args);

  exit (1);
}

void
gmondump1 (char *filename)
{
  ushort    *bucket = NULL;
  int        fd;
  struct gmonhdr hdr;
  int        hitbuckets;
  int        hitcount;
  int        numbuckets;
  int        numrawarcs;
  struct rawarc *rawarc = NULL;
  int        res;
  struct stat stat;

  fd = open (filename, O_RDONLY | O_BINARY);
  if (fd < 0)
    {
      note ("file%s %s couldn't be opened; continuing\n",
            strchr (filename, '*') ? "s" : "", filename);
      return;
    }

  /* Read and sanity-check what should be a gmon header. */
  res = fstat (fd, &stat);
  if (res < 0)
    goto notgmon;
  if (S_IFREG != (stat.st_mode & S_IFMT))
    goto notgmon;
  res = read (fd, &hdr, sizeof (hdr));
  if (res != sizeof (hdr))
    goto notgmon;
  if (hdr.lpc >= hdr.hpc)
    goto notgmon;
  numbuckets = (hdr.ncnt - sizeof (hdr)) / sizeof (short);
  if (numbuckets != (hdr.hpc - hdr.lpc) / 4)
    goto notgmon;
  numrawarcs = 0;
  if (stat.st_size != hdr.ncnt)
    {
      numrawarcs = stat.st_size - hdr.ncnt;
      if (numrawarcs !=
          (int) sizeof (rawarc) * (numrawarcs / (int) sizeof (rawarc)))
        goto notgmon;
      numrawarcs /= (int) sizeof (rawarc);
    }

  /* Looks good, so read and display the profiling info. */
  bucket = (ushort *) calloc (numbuckets, sizeof (ushort));
  res = read (fd, bucket, hdr.ncnt - sizeof (hdr));
  if (res != hdr.ncnt - (int) sizeof (hdr))
    goto notgmon;
  hitcount = hitbuckets = 0;
  for (res = 0; res < numbuckets; ++bucket, ++res)
    if (*bucket)
      {
        ++hitbuckets;
        hitcount += *bucket;
      }
  bucket -= numbuckets;

  note ("file %s, gmon version 0x%x, sample rate %d\n",
        filename, hdr.version, hdr.profrate);
  note ("  address range %p..%p\n", hdr.lpc, hdr.hpc);
  note ("  numbuckets %d, hitbuckets %d, hitcount %d, numrawarcs %d\n",
        numbuckets, hitbuckets, hitcount, numrawarcs);

  /* If verbose is set, display contents of buckets and rawarcs arrays. */
  if (verbose)
    {
      if (hitbuckets)
        note ("  bucket data follows...\n");
      char *addr = (char *) hdr.lpc;
      int   incr = (hdr.hpc - hdr.lpc) / numbuckets;
      for (res = 0; res < numbuckets; ++bucket, ++res, addr += incr)
        if (*bucket)
          note ("    address %p, hitcount %d\n", addr, *bucket);
      bucket -= numbuckets;

      if (numrawarcs)
        {
          rawarc = (struct rawarc *) calloc (numrawarcs, sizeof (rawarc));
          res = read (fd, rawarc, numrawarcs * (int) sizeof (rawarc));
          if (res != numrawarcs * (int) sizeof (rawarc))
            error (0, "unable to read rawarc data");
          note ("  rawarc data follows...\n");
          for (res = 0; res < numrawarcs; ++rawarc, ++res)
            note ("    from %p, self %p, count %d\n",
                  rawarc->raw_frompc, rawarc->raw_selfpc, rawarc->raw_count);
        }
    }

  note ("\n");
  if (0)
    {
notgmon:
      note ("file %s isn't a profile data file; continuing\n", filename);
    }
  if (rawarc)
    free (rawarc);
  if (bucket)
    free (bucket);
  close (fd);
}

struct option longopts[] = {
  {"help",    no_argument, NULL, 'h'},
  {"verbose", no_argument, NULL, 'v'},
  {"version", no_argument, NULL, 'V'},
  {NULL,      0,           NULL, 0  }
};

const char *const opts = "+hvV";

void __attribute__ ((__noreturn__))
print_version ()
{
  char *year_of_build = strrchr (__DATE__, ' ') + 1;
  printf ("gmondump (cygwin) %d.%d.%d\n"
          "Profiler data file viewer\n"
          "Copyright (C) %s%s Cygwin Authors\n"
          "This is free software; see the source for copying conditions.  "
          "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS "
          "FOR A PARTICULAR PURPOSE.\n",
          CYGWIN_VERSION_DLL_MAJOR / 1000,
          CYGWIN_VERSION_DLL_MAJOR % 1000,
          CYGWIN_VERSION_DLL_MINOR,
          strncmp (year_of_build, "2021", 4) ? "2021 - " : "",
          year_of_build);
  exit (0);
}

int
main(int argc, char **argv)
{
  ofile = stdout;
  int opt;

  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'h':
        /* Print help and exit. */
        usage (ofile);

      case 'v':
        verbose ^= 1;
        break;

      case 'V':
        /* Print version and exit. */
        print_version ();

      default:
        ;
      }

  if (optind >= argc)
    /* Print one-line help and exit. */
    usage1 (ofile);

  for (int i = optind; i < argc; i++)
    gmondump1 (argv[i]);

  return 0;
}
