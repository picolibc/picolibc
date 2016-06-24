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

#include <errno.h>
#include <mntent.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLUMN_FS_NAME			0
#define COLUMN_MOUNTPOINT		1
#define COLUMN_FS_TYPE			2
#define COLUMN_OPTS				3
#define COLUMN_DUMP_FREQ		4
#define COLUMN_PASSNO			5

#define BUFFER_SIZE				4096

struct mntent *getmntent_r(FILE *fp, struct mntent *result, char *buffer, int bufsize)
{
	char *token, *buff_ptr;
	char *line = NULL;
	size_t size = 0;
	int i;

	buff_ptr = buffer;

	if (getline(&line, &size, fp) == -1 || feof(fp)) {
		free(line);
		return NULL;
	}

	for (token = strtok(line, " "), i = 0; token != NULL; token = strtok(NULL, " "), ++i) {
		if (((buff_ptr + strlen(token) + 1) - buffer) > bufsize) {
			errno = ENOBUFS;
			free(line);
			return NULL;
		}

		switch (i) {
		case COLUMN_FS_NAME:
			result->mnt_fsname = strcpy(buff_ptr, token);
			break;
		case COLUMN_MOUNTPOINT:
			result->mnt_dir = strcpy(buff_ptr, token);
			break;
		case COLUMN_FS_TYPE:
			result->mnt_type = strcpy(buff_ptr, token);
			break;
		case COLUMN_OPTS:
			result->mnt_opts = strcpy(buff_ptr, token);
			break;
		case COLUMN_DUMP_FREQ:
			result->mnt_freq = atoi(token);
			break;
		case COLUMN_PASSNO:
			result->mnt_passno = atoi(token);
			break;
		default:
			free(line);
			return NULL;
		}

		buff_ptr += strlen(token) + 1;
	}

	free(line);
	return result;
}

struct mntent *getmntent(FILE *fp)
{
	static struct mntent result;
	static char buffer[BUFFER_SIZE];
	memset(&result, 0, sizeof(struct mntent));
	memset(buffer, 0, sizeof(buffer));

	return getmntent_r(fp, &result, buffer, sizeof(buffer));
}
