/* Operating system stubs, set up for the MRI simulator */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>

int _DEFUN(_read,(file, ptr, len),
	   int file _AND
	   char *ptr _AND
	   int len)
{
  return 0;
}


int _DEFUN(_lseek,(file, ptr, dir),
	  int file _AND
	  int ptr _AND
	  int dir)
{
  return 0;
}

int _DEFUN(_close,(file),
	  int file)
{
  return -1;
}

int isatty(file)
     int file;
{
  return 1;
}

int _DEFUN(_fstat,(file, st),
	  int file _AND
	  struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

int
_open (path, flags)
     const char *path;
     int flags;
{
  return 0;
}
