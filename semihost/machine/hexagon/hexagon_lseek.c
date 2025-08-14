#include "hexagon_semihost.h"
#include <errno.h>
#include <unistd.h>

off_t
lseek(int fd, off_t offset, int whence)
{
    off_t file_offset = 0;
    switch (whence) {
    case SEEK_CUR:
        file_offset = hexagon_ftell(fd);
        break;
    case SEEK_END:
        file_offset = flen(fd);
        break;
    case SEEK_SET:
        break;
    default:;
        errno = EINVAL;
        return -1;
    }
    /* file offset is invalid */
    if (file_offset < 0) {
        /* errno already set in above switch case */
        return -1;
    }

    offset += file_offset;

    int args[] = { fd, offset };
    int retval = hexagon_semihost(SYS_SEEK, args);
    return retval;
}
