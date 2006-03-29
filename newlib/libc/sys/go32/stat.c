/*
  (c) Copyright 1992 Eric Backus

  This software may be used freely so long as this copyright notice is
  left intact.  There is no warrantee on this software.
*/

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "dos.h"
#include <errno.h>
#include <stdio.h>

extern int	_stat_assist(const char *, struct stat *);
extern void	_fixpath(const char *, char *);

struct path_list
{
    struct path_list	*next;
    char		*path;
    int			inode;
};

static int
fixinode(const char *path, struct stat *buf)
{
    static struct path_list	*path_list[1256];
    /* Start the inode count at three, since root path should be two */
    static int			inode_count = 3;

    struct path_list		*path_ptr, *prev_ptr;
    const char			*p;
    int				hash;

    /* Skip over device and leading '/' */
    if (path[1] == ':' && path[2] == '/') path += 3;

    /* We could probably use a better hash than this */
    p = path;
    hash = 0;
    while (*p != '\0') hash += *p++;
    hash = hash & 0xff;

    /* Have we seen this string? */
    path_ptr = path_list[hash];
    prev_ptr = path_ptr;
    while (path_ptr)
    {
	if (strcmp(path, path_ptr->path) == 0) break;
	prev_ptr = path_ptr;
	path_ptr = path_ptr->next;
    }

    if (path_ptr)
	/* Same string, so same inode */
	buf->st_ino = path_ptr->inode;
    else
    {
	/* New string with same hash code */
	path_ptr = malloc(sizeof *path_ptr);
	if (path_ptr == NULL)
	  {
	    errno = ENOMEM;
	    return -1;
	  }
	path_ptr->next = NULL;
	path_ptr->path = strdup(path);
	if (path_ptr->path == NULL)
	  {
	    errno = ENOMEM;
	    return -1;
	  }
	path_ptr->inode = inode_count;
	if (prev_ptr)
	    prev_ptr->next = path_ptr;
	else
	    path_list[hash] = path_ptr;
	buf->st_ino = inode_count;
	inode_count++;
    }
    return 0;
}

int
stat(const char *path, struct stat *buf)
{
    static int	stat_called_before = 0;
    char	p[1090];	/* Should be p[PATH_MAX+1] */
    int		status;

    /* Normalize the path */
    _fixpath(path, p);

    /* Work around strange bug with stat and time */
    if (!stat_called_before)
    {
	stat_called_before = 1;
	(void) time((time_t *) 0);
    }

    /* Check for root path */
    if (strcmp(p, "/") == 0 || strcmp(p + 1, ":/") == 0)
    {
	/* Handle root path as special case, stat_assist doesn't like
	   the root directory. */
	if (p[1] == ':')
	{
	    if (p[0] >= 'a' && p[0] <= 'z')
		buf->st_dev = p[0] - 'a';
	    else
		buf->st_dev = p[0] - 'A';
	}
	else
	    buf->st_dev = -1;	/* No device? */
	buf->st_ino = 2;	/* Root path always inode 2 */
	buf->st_mode = S_IFDIR | S_IREAD | S_IWRITE | S_IEXEC;
	buf->st_nlink = 1;
	buf->st_uid = getuid();
	buf->st_gid = getgid();
	buf->st_rdev = buf->st_dev;
	buf->st_size = 0;
	buf->st_atime = 0;
	buf->st_mtime = 0;
	buf->st_ctime = 0;
	buf->st_blksize = 512;	/* Not always correct? */
	status = 0;
    }
    else
    {
	status = _stat_assist(p, buf);

	/* Make inode numbers unique */
	if (status == 0) status = fixinode(p, buf);

	/* The stat_assist does something weird with st_dev, but sets
	   st_rdev to the drive number.  Fix st_dev. */
	buf->st_dev = buf->st_rdev;

	/* Make all files owned by ourself. */
	buf->st_uid = getuid();
	buf->st_gid = getgid();

	/* Make all directories writable.  They always are in DOS, but
	   stat_assist doesn't think so. */
	if (S_ISDIR(buf->st_mode)) buf->st_mode |= S_IWRITE;
    }

    return status;
}
