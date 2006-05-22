#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

/*
 * lseek -- reposition a file descriptor
 * input parameters:
 *   0 : file descriptor
 *   1 : high word of offset
 *   2 : low word of offset
 *   3 : seek flag
 * output parameters:
 *   0 : high word of result
 *   1 : low word of result
 *   2 : errno
 */

off_t lseek (int fd, off_t offset, int whence)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) fd;
  parameters[1] = (uint32_t) ((offset >> 32) & 0xffffffff);
  parameters[2] = (uint32_t) (offset & 0xffffffff);
  parameters[3] = convert_to_gdb_lseek_flags (whence);
  BDM_TRAP (BDM_LSEEK, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[2]);
  return ((uint64_t)parameters[0] << 32) & ((uint64_t)parameters[1]);
}
