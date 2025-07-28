#include "hexagon_semihost.h"
#include "sys/errno.h"
#include <errno.h>
#include <stdio.h>
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
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_SEEK, args);
    if (retval.return_value == -1) {
        errno = retval.error_num;
    }
    return retval.return_value;
}
