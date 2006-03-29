#include <string.h>

#include "sys/dir.h"
#include "sys/dirent.h"
#include <errno.h>

DIR *opendir(char *name)
{
  int length;
  DIR *dir = (DIR *)malloc(sizeof(DIR));
  dir->num_read = 0;
  dir->name = (char *)malloc(strlen(name)+6);
  strcpy(dir->name, name);

  /* Append a "." if we got only the device name */
  if (dir->name[1] == ':' && strlen(dir->name) == 2)
      strcat(dir->name, ".");

  /* Strip trailing slashes, so we can append "/*.*" */
  while (1)
  {
      length = strlen(dir->name);
      if (length == 0) break;
      if (dir->name[length - 1] == '/' ||
	  dir->name[length - 1] == '\\')
	  dir->name[length - 1] = '\0';
      else
	  break;
  }

  strcat(dir->name, "/*.*");
  return dir;
}



static char *strlwr(char *s)
{
  char *p = s;
  while (*s)
  {
    if ((*s >= 'A') && (*s <= 'Z'))
      *s += 'a'-'A';
    s++;
  }
  return p;
}

struct dirent *readdir(DIR *dir)
{
  int done;
  int oerrno = errno;
  if (dir->num_read)
    done = findnext(&dir->ff);
  else
    done = findfirst(dir->name, &dir->ff,
		     FA_ARCH|FA_RDONLY|FA_DIREC|FA_HIDDEN|FA_SYSTEM);
  if (done)
  {
    if (errno == ENMFILE)
      errno = oerrno;
    return 0;
  }
  dir->num_read ++;
  dir->de.d_namlen = strlen(dir->ff.ff_name);
  strcpy(dir->de.d_name,dir->ff.ff_name);
  strlwr(dir->de.d_name);
  return &dir->de;
}

long telldir(DIR *dir)
{
  return dir->num_read;
}

void seekdir(DIR *dir, long loc)
{
  int i;
  rewinddir(dir);
  for (i=0; i<loc; i++)
    readdir(dir);
}

void rewinddir(DIR *dir)
{
  dir->num_read = 0;
}

int closedir(DIR *dir)
{
  free(dir->name);
  free(dir);
  return 0;
}

