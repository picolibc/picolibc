#ifndef _SYS_VFS_H_
#define _SYS_VFS_H_

struct statfs {
   long    f_type;     /* type of filesystem (see below) */
   long    f_bsize;    /* optimal transfer block size */
   long    f_blocks;   /* total data blocks in file system */
   long    f_bfree;    /* free blocks in fs */
   long    f_bavail;   /* free blocks avail to non-superuser */
   long    f_files;    /* total file nodes in file system */
   long    f_ffree;    /* free file nodes in fs */
   long    f_fsid;     /* file system id */
   long    f_namelen;  /* maximum length of filenames */
   long    f_spare[6]; /* spare for later */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int statfs (const char *__path, struct statfs *__buf);
int fstatfs (int __fd, struct statfs *__buf);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /*_SYS_VFS_H_*/
