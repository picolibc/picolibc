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

#define FA_RDONLY	1
#define FA_HIDDEN	2
#define FA_SYSTEM	4
#define FA_LABEL	8
#define FA_DIREC	16
#define FA_ARCH		32

#ifdef __cplusplus
extern "C" {
#endif

int findfirst(const char *pathname, struct ffblk *ffblk, int attrib);
int findnext(struct ffblk *ffblk);

#ifdef __cplusplus
}
#endif

#endif

