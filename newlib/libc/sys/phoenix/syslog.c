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

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>

#define BUFFER_SIZE		1024
#define LOG_MAX_SIZE	(3 * 512)

struct {
	int socket;
	struct sockaddr_in addr;
	const char *ident;
	int option;
	int facility;
	int tried;
} client = {
	.ident = NULL,
	.socket = -1,
	.option = 0,
	.facility = 0,
};

void openlog(const char *ident, int option, int facility)
{
	if (client.socket != -1)
		return;

	client.ident = ident;
	client.option = option;
	client.facility = facility;

	if (client.ident == NULL)
		client.ident = "app";

	if ((client.socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf("Socket failed: %s\n", strerror(errno));
		return;
	}

	memset(&(client.addr), 0, sizeof(client.addr));
	client.addr.sin_family = AF_INET;
	client.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client.addr.sin_port = htons(31000);
}

void syslog(int priority, const char *format, ...)
{
	char buffer[BUFFER_SIZE];
	char msg[LOG_MAX_SIZE];
	memset(buffer, 0, sizeof(buffer));
	memset(msg, 0, sizeof(msg));
	
	if (LOG_FAC(priority) == 0)
		priority |= client.facility;
	
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	
	snprintf(msg, sizeof(msg), "<%d> %s[%d]: %s", priority, client.ident, getpid(), buffer);

	if (sendto(client.socket, msg, strlen(msg), 0, (struct sockaddr *) &(client.addr), sizeof(client.addr)) == -1) {
		printf("Send failed: %s\n", strerror(errno));
		return;
	}
}

void closelog()
{
	if (client.socket == -1)
		return;

	close(client.socket);
	client.ident = NULL;
	client.socket = -1;
}
