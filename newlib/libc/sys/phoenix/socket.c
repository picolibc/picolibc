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
#include <sys/socket.h>

int socket(int domain, int type, int protocol)
{
	int ret = syscall3(int, SYS_SOCKET, domain, type, protocol);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret = syscall3(int, SYS_BIND, sockfd, addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int listen(int sockfd, int backlog)
{
	int ret = syscall2(int, SYS_LISTEN, sockfd, backlog);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = syscall3(int, SYS_ACCEPT, sockfd, addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret = syscall3(int, SYS_CONNECT, sockfd, addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	struct sendto_args args = { sockfd, buf, len, flags, dest_addr, addrlen };

	int ret = syscall1(int, SYS_SENDTO, &args);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int send(int sockfd, const void *buf, size_t len, int flags)
{
    return sendto(sockfd, buf, len, flags, NULL, 0);
}

int recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	struct recvfrom_args args = { sockfd, buf, len, flags, src_addr, addrlen };

	int ret = syscall1(int, SYS_RECVFROM, &args);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}

int recv(int sockfd, void *buf, size_t len, int flags)
{
	return recvfrom(sockfd, buf, len, flags, NULL, NULL);
}

int shutdown(int sockfd, int how)
{
	/* TODO: implement. */
	errno = ENOSYS;
	return -1;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = syscall3(int, SYS_GETPEERNAME, sockfd, addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return ret;
}
