/*
 * usleep
 * Implementation according to:
 * The Open Group Base Specifications Issue 6
 * IEEE Std 1003.1, 2004 Edition
 */

/*
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  Contributed by:
 *  Ramiro Polla <ramiro@lisha.ufsc.br>
 */

#include <sys/types.h>
#include <errno.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int __cdecl usleep(useconds_t useconds)
{
    if(useconds == 0)
        return 0;

    if(useconds >= 1000000)
        return EINVAL;

    Sleep(useconds / 1000);

    return 0;
}
