#ifndef _DOS_H_
#define _DOS_H_

#include "pc.h"

union REGS {
  struct {
    unsigned long ax;
    unsigned long bx;
    unsigned long cx;
    unsigned long dx;
    unsigned long si;
    unsigned long di;
    unsigned long cflag;
    unsigned long flags;
  } x;
  struct {
    unsigned char al;
    unsigned char ah;
    unsigned short upper_ax;
    unsigned char bl;
    unsigned char bh;
    unsigned short upper_bx;
    unsigned char cl;
    unsigned char ch;
    unsigned short upper_cx;
    unsigned char dl;
    unsigned char dh;
    unsigned short upper_dx;
  } h;
};

struct SREGS {
  unsigned short cs;
  unsigned short ds;
  unsigned short es;
  unsigned short fs;
  unsigned short gs;
  unsigned short ss;
};

struct ftime {
  unsigned ft_tsec:5;	/* 0-29, double to get real seconds */
  unsigned ft_min:6;	/* 0-59 */
  unsigned ft_hour:5;	/* 0-23 */
  unsigned ft_day:5;	/* 1-31 */
  unsigned ft_month:4;	/* 1-12 */
  unsigned ft_year:7;	/* since 1980 */
};

struct date {
  short da_year;
  char  da_day;
  char  da_mon;
};

struct time {
  unsigned char ti_min;
  unsigned char ti_hour;
  unsigned char ti_hund;
  unsigned char ti_sec;
};

struct dfree {
  unsigned df_avail;
  unsigned df_total;
  unsigned df_bsec;
  unsigned df_sclus;
};

#ifdef __cplusplus
extern "C" {
#endif

int bdos(int func, unsigned dx, unsigned al);
int bdosptr(int func, void *dx, unsigned al);
int int86(int ivec, union REGS *in, union REGS *out);
int int86x(int ivec, union REGS *in, union REGS *out, struct SREGS *seg);
int intdos(union REGS *in, union REGS *out);
int intdosx(union REGS *in, union REGS *out, struct SREGS *seg);

int enable(void);
int disable(void);

int getftime(int handle, struct ftime *ftimep);
int setftime(int handle, struct ftime *ftimep);

int getcbrk(void);
int setcbrk(int new_value);

void getdate(struct date *);
void gettime(struct time *);
void setdate(struct date *);
void settime(struct time *);

void getdfree(unsigned char drive, struct dfree *ptr);

void delay(unsigned msec);
int _get_default_drive(void);
void _fixpath(const char *, char *);

#ifdef __cplusplus
}
#endif

#endif

