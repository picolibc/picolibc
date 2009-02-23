/* Common defines for ttyname.c and ttyname_r.c */
 
#include <dirent.h>	/* For MAXNAMLEN */
#include <paths.h>	/* For _PATH_DEV */

#define TTYNAME_BUFSIZE	(sizeof (_PATH_DEV) + MAXNAMLEN)
