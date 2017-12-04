#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int _write (int, void*, unsigned int);

int
_open() {
  return -1;
}

int
_close() {
  return -1;
}

int
_lseek() {
  return 0;
}

int
_read() {
  return 0;
}

int
isatty() {
  return 1;
}

int
_fstat (int file,
       struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

