#ifndef _DIR_H_
#define _DIR_H_

struct ffblk {
  char ff_reserved[21];
  char ff_attrib;
  short ff_ftime;
  short ff_fdate;
  short ff_filler;
  long ff_fsize;
  char ff_name[16];
};

#define FA_RDONLY       1
#define FA_HIDDEN       2
#define FA_SYSTEM       4
#define FA_LABEL        8
#define FA_DIREC        16
#define FA_ARCH         32

/* for fnmerge/fnsplit */
#define MAXPATH   80
#define MAXDRIVE  3
#define MAXDIR	  66
#define MAXFILE   9
#define MAXEXT	  5

#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME  0x04
#define DIRECTORY 0x08
#define DRIVE	  0x10

#ifdef __cplusplus
extern "C" {
#endif

int findfirst(const char *pathname, struct ffblk *ffblk, int attrib);
int findnext(struct ffblk *ffblk);

void fnmerge (char *path, const char *drive, const char *dir,
				 const char *name, const char *ext);
int fnsplit (const char *path, char *drive, char *dir, 
				 char *name, char *ext);

int getdisk(void);
int setdisk(int drive);

#ifdef __cplusplus
}
#endif

#endif
