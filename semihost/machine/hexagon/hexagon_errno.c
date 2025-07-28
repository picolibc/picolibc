#include "hexagon_semihost.h"
#include <errno.h>

#define E(N)                                                                   \
    case HEX_E##N:                                                             \
        errno = E##N;                                                          \
        break

void
hexagon_semihost_errno(int err)
{
    switch (err) {
        E(PERM);
        E(NOENT);
        E(INTR);
        E(IO);
        E(NXIO);
        E(BADF);
        E(AGAIN);
        E(NOMEM);
        E(ACCES);
        E(FAULT);
        E(BUSY);
        E(EXIST);
        E(XDEV);
        E(NODEV);
        E(NOTDIR);
        E(ISDIR);
        E(INVAL);
        E(NFILE);
        E(MFILE);
        E(NOTTY);
        E(TXTBSY);
        E(FBIG);
        E(NOSPC);
        E(SPIPE);
        E(ROFS);
        E(MLINK);
        E(PIPE);
        E(RANGE);
        E(NAMETOOLONG);
        E(NOSYS);
        E(LOOP);
        E(OVERFLOW);
    default:
        errno = EINVAL;
        break;
    }
}
