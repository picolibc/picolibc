/* Copyright (c) 2016 Phoenix Systems
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

#include "syscall.h"

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/termios.h>

int _isatty(int fd)
{
	int ret = syscall1(int, SYS_ISATTY, fd);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int tcgetattr(int fd, struct termios *termios_p)
{
	int ret = ioctl(fd, TCGETS, termios_p);
	if (ret < 0) {
		errno = -ret;
		return -1; 
	}

	return ret;
}

int tcsetattr(int fd, int mode, const struct termios *termios_p)
{
	int cmd;
	switch (mode) {
		case TCSANOW:
			cmd = TCSETS;
			break;
		case TCSADRAIN:
			cmd = TCSETSW;
			break;
		case TCSAFLUSH:
			cmd = TCSETSF;
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	int ret = ioctl(fd, cmd, termios_p);
	if (ret < 0) {
		errno = -ret;
		return -1; 
	}

	return ret;
}

int tcsendbreak(int fd, int duration)
{
	int ret = ioctl(fd, TCSBRK, duration);
	if (ret < 0) {
		errno = -ret;
		return -1; 
	}

	return ret;
}

int tcflush(int fd, int queue_selector)
{
	int ret = ioctl(fd, TCFLSH, queue_selector);
	if (ret < 0) {
		errno = -ret;
		return -1; 
	}

	return ret;
}

int tcdrain(int fd)
{
	int ret = ioctl(fd, TCDRAIN, 0);
	if (ret < 0) {
		errno = -ret;
		return -1; 
	}

	return ret;
}

pid_t tcgetsid(int fd)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}

int tcsetpgrp(int fd, pid_t pgrp)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}
