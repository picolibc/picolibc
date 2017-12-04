/* ctermid */

#include <stdio.h>
#include <string.h>

static char devname[] = "/dev/tty";

char *
ctermid (char *buf)
{
  if (buf == NULL)
    return devname;

  return strcpy (buf, "/dev/tty");
}
