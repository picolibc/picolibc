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
#include <phoenix/socket_args.h>
#include <phoenix/sockios.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = syscall3(int, SYS_GETSOCKNAME, sockfd, addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
	struct sockopt_args args;
	args.level = level;
	args.optname = optname;
	args.optval = optval;
	args.optlen = *optlen;

	int ret = ioctl(sockfd, IOC_GETSOCKOPT, &args);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	*optlen = args.optlen;
	return ret;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
	struct sockopt_args args;
	args.level = level;
	args.optname = optname;
	args.optval = (void *) optval;
	args.optlen = optlen;

	int ret = ioctl(sockfd, IOC_SETSOCKOPT, &args);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}
