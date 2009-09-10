#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <_ansi.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <reent.h>

_syscall2 (int ,link, const char *,old, const char *,new)
