#include <unistd.h>
int ftruncate(int __fd, off_t __length)
{
  return _chsize (__fd, __length);
}
