/* syscalls.c -- reentrant syscalls for OpenRISC 1000.
 *
 * Copyright (c) 2011, 2014 Authors
 *
 * Contributor Julius Baxter <juliusbaxter@gmail.com>
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <errno.h>
#include <reent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "board.h"

/* Write is actually the only thing we provide. All other are stubs.. */

extern void _or1k_outbyte(char c);

_ssize_t
_write_r(struct _reent * reent, int fd, const void *buf, size_t nbytes)
{
	int i;
	char* b = (char*) buf;

	for (i = 0; i < nbytes; i++) {
		if (*(b + i) == '\n') {
			_or1k_outbyte ('\r');
		}
		_or1k_outbyte (*(b + i));
	}
	return (nbytes);
}

void
_exit(int rc)
{
	_or1k_board_exit();
	while (1) {}
}

int
_close_r(struct _reent *reent, int fildes)
{
	reent->_errno = ENOSYS;
	return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int
_execve_r(struct _reent *reent, const char *name, char * const *argv,
		char * const *env)
{
	reent->_errno = ENOSYS;
	return -1;
}

int
_fork_r(struct _reent *reent)
{
	errno = ENOSYS;
	return -1;
}

int
_fstat_r(struct _reent *reent, int fildes, struct stat *st)
{
	reent->_errno = ENOSYS;
	return -1;
}

int
_getpid_r(struct _reent *reent)
{
	reent->_errno = ENOSYS;
	return -1;
}

int
_gettimeofday(struct _reent *reent, struct timeval  *ptimeval, void *ptimezone)
{
	reent->_errno = ENOSYS;
	return -1;
}

int
_isatty_r(struct _reent *reent, int file)
{
	reent->_errno = ENOSYS;
	return 0;
}

int
_kill_r(struct _reent *reent, int pid, int sig)
{
	reent->_errno = ENOSYS;
	return -1;
}

int
_link_r(struct _reent *reent, const char *existing, const char *new)
{
	reent->_errno = ENOSYS;
	return -1;
}

_off_t
_lseek_r(struct _reent *reent, int file, _off_t ptr, int dir)
{
	errno = ENOSYS;
	return -1;
}

int
_open(struct _reent *reent, char *file, int flags, int mode)
{
	reent->_errno = ENOSYS;
	return -1;
}

_ssize_t
_read_r(struct _reent *reent, int file, void *ptr, size_t len)
{
	reent->_errno = ENOSYS;
	return -1;
}

int
_readlink_r(struct _reent *reent, const char *path, char *buf, size_t bufsize)
{
	reent->_errno = ENOSYS;
	return -1;
}

int
_stat_r(struct _reent *reent, const char *path, struct stat *buf)
{
	reent->_errno = EIO;
	return -1;
}

int
_unlink_r(struct _reent *reent, const char * path)
{
	reent->_errno = EIO;
	return (-1);
}

