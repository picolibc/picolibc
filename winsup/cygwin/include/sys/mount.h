#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

enum
  {
    /* MOUNT_SYMLINK = 1, place holder.  Do not use it. */
    MOUNT_BINARY =  2,	/* "binary" format read/writes */
    MOUNT_SYSTEM =  8,	/* mount point came from system table */
    MOUNT_EXEC   = 16,	/* Any file in the mounted directory gets 'x' bit */
    MOUNT_AUTO   = 32	/* mount point refers to auto device mount */
  };

int mount (const char *, const char *, unsigned __flags);
int umount (const char *);
int cygwin_umount (const char *__path, unsigned __flags);

#ifdef __cplusplus
};
#endif

#endif /* _SYS_MOUNT_H */
