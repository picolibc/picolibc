/*
 * This file is part of the Mingw32 package.
 *
 * unistd.h maps (roughly) to io.h
 */

#ifndef _UNISTD_H
#define _UNISTD_H

#include <io.h>
#include <process.h>

#define __UNISTD_GETOPT__
#include <getopt.h>
#undef __UNISTD_GETOPT__

#endif /* _UNISTD_H */
