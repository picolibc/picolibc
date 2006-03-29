#ifndef _DOS_H_
#define _DOS_H_

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

#ifdef __cplusplus
extern "C" {
#endif

int bdos(int func, unsigned dx, unsigned al);
int bdosptr(int func, void *dx, unsigned al);
int int86(int ivec, union REGS *in, union REGS *out);
int int86x(int ivec, union REGS *in, union REGS *out, struct SREGS *seg);
int intdos(union REGS *in, union REGS *out);
int intdosx(union REGS *in, union REGS *out, struct SREGS *seg);

#ifdef __cplusplus
}
#endif

#endif


