#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <sys/cdefs.h>
#include <sys/_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INO_T_DECLARED
typedef	__ino_t ino_t;
#define	_INO_T_DECLARED
#endif

struct dirent {
    ino_t d_ino;            /* file number */
    __uint8_t d_type;       /* file type, see below */
    char d_name[255 + 1];   /* zero-terminated file name */
};

typedef struct {
    __uint16_t dd_vfs_idx; /*!< VFS index, not to be used by applications */
    __uint16_t dd_rsv;     /*!< field reserved for future extension */
    /* remaining fields are defined by VFS implementation */
} DIR;

#if __BSD_VISIBLE
#define MAXNAMLEN	 255

/* File types */
#define	DT_UNKNOWN	 0
#define	DT_REG		 1
#define	DT_DIR		 2

/* File types which are not defined for ESP.
 * But must be present to build libstd++ */
#define	DT_CHR		 4
#define	DT_BLK		 6
#define	DT_FIFO		 8
#define	DT_LNK		10
#define	DT_SOCK		12
#define	DT_WHT		14
#endif

#ifdef __cplusplus
}
#endif
#endif
