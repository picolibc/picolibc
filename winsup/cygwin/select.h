/* select.h

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Winsock select() types and macros */

/*
 * Use this struct to interface to
 * the system provided select.
 */
typedef struct winsock_fd_set
{
  unsigned int fd_count;
  HANDLE fd_array[1024]; /* Dynamically allocated. */
} winsock_fd_set;

/*
 * Define the Win32 winsock definitions to have a prefix WINSOCK_
 * so we can be explicit when we are using them.
 */
#define WINSOCK_FD_ISSET(fd, set) __WSAFDIsSet ((SOCKET)fd, (fd_set *)set)
#define WINSOCK_FD_SET(fd, set) do { \
	       (set)->fd_array[(set)->fd_count++]=fd;\
} while(0)
#define WINSOCK_FD_ZERO(set) ((set)->fd_count = 0)
#define WINSOCK_FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < (set)->fd_count ; __i++) { \
	if ((set)->fd_array[__i] == fd) { \
	    while (__i < (set)->fd_count-1) { \
		(set)->fd_array[__i] = \
		    (set)->fd_array[__i+1]; \
		__i++; \
	    } \
	    (set)->fd_count--; \
	    break; \
	} \
    } \
} while(0)

extern "C" int PASCAL __WSAFDIsSet(SOCKET, fd_set*);
extern "C" int PASCAL win32_select(int, fd_set*, fd_set*, fd_set*, const struct timeval*);

/*
 * call to winsock's select() -
 * type coercion need to appease confused prototypes
 */
#define WINSOCK_SELECT(nfd, rd, wr, ex, timeo) \
    win32_select (nfd, (fd_set *)rd, (fd_set *)wr, (fd_set *)ex, timeo)

