#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
int
main (int argc, char **argv)
{
  char *cwd = getcwd (NULL, 256);
  if (cwd == NULL)
    {
      fprintf (stderr, "%s: getcwd returns NULL\n", argv[0]);
      exit (1);
    }

  exit (0);
}

