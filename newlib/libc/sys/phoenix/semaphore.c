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
#include <fcntl.h>
#include <malloc.h>
#include <semaphore.h>
#include <stdarg.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <unistd.h>

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	errno = ENOSYS;
	return -1;
}

int sem_destroy(sem_t *sem)
{
	errno = ENOSYS;
	return -1;
}

sem_t *sem_open(const char *name, int oflag, ...)
{
	sem_t *sem = (sem_t *) malloc(sizeof(sem_t));
	if(sem == NULL) {
		errno = ENOMEM;
		return SEM_FAILED;
	}

	if (*name == '/')
		++name;

	char sem_name[PATH_MAX_SIZE + 24] = "/dev/ipc/sem_";
	strncpy(sem_name + 13, name, PATH_MAX_SIZE + 10);

	int mode = S_IRWXUGO;
	int initval = 1;
	if (oflag & O_CREAT) {
		va_list ap;
		va_start(ap, oflag);
		mode = va_arg(ap, int);
		initval = va_arg(ap, int);
		va_end(ap);
	}

	sem->fd = open(sem_name, oflag, mode);
	if (sem->fd != -1) {
		int flags = fcntl(sem->fd, F_GETFD, 0);
		if (flags >= 0) {
			flags |= FD_CLOEXEC;
			flags = fcntl (sem->fd, F_SETFD, flags);
		}

		if (flags == -1) {
			close (sem->fd);
			free(sem);
			return SEM_FAILED;
		}
	}
	else {
		free(sem);
		return SEM_FAILED;
	}

	if (sem != SEM_FAILED && (oflag | O_CREAT)) {
		if (ioctl(sem->fd, IPC_SEM_INIT, initval) < 0) {
			free(sem);
			return SEM_FAILED;
		}
	}

	return sem;
}

int sem_close(sem_t *sem)
{
	if (sem == NULL || sem->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	close(sem->fd);
	free(sem);

	return 0;
}

int sem_unlink(const char *name)
{
	return unlink(name);
}

int sem_wait(sem_t *sem)
{
	if(sem == NULL || sem->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return ioctl(sem->fd, IPC_SEM_DOWN, 0);
}

int sem_timedwait(sem_t * sem, const struct timespec *abs_timeout)
{
	if(sem == NULL || sem->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return ioctl(sem->fd, IPC_SEM_DOWN_TIMEOUT, 0);
}

int sem_trywait(sem_t *sem)
{
	if(sem == NULL || sem->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return ioctl(sem->fd, IPC_SEM_TRYDOWN, 0);
}

int sem_post(sem_t *sem)
{
	if(sem == NULL || sem->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return ioctl(sem->fd, IPC_SEM_UP, 0);
}

int sem_getvalue (sem_t *sem, int *sval)
{
	if(sem == NULL || sem->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return ioctl(sem->fd, IPC_SEM_GETVAL, (unsigned long) sval);
}
