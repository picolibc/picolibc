/* sys/termios.h

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* sys/termios.h */

#ifndef	_SYS_TERMIOS_H
#define _SYS_TERMIOS_H

#define	TIOCMGET	0x5415
#define	TIOCMBIS	0x5416
#define	TIOCMBIC	0x5417
#define	TIOCMSET	0x5418
#define	TIOCINQ		0x541B

/* TIOCINQ is utilized instead of FIONREAD which has been
accupied for other purposes under CYGWIN.
Other UNIX ioctl requests has been omited because
effects of their work one can achive by standard
POSIX commands */

#define TIOCSBRK	0x5427
#define TIOCCBRK	0x5428

#define	TIOCM_DTR	0x002
#define	TIOCM_RTS	0x004
#define	TIOCM_CTS	0x020
#define	TIOCM_CAR	0x040
#define	TIOCM_RNG	0x080
#define	TIOCM_DSR	0x100
#define	TIOCM_CD	TIOCM_CAR
#define	TIOCM_RI	TIOCM_RNG

#define TCOOFF		0
#define TCOON		1
#define TCIOFF		2
#define TCION		3

#define TCGETA	5
#define TCSETA	6
#define TCSETAW		7
#define TCSETAF		8

#define TCIFLUSH	0
#define TCOFLUSH	1
#define TCIOFLUSH	2
#define TCFLSH          3

#define TCSAFLUSH	1
#define TCSANOW		2
#define TCSADRAIN	3
#define TCSADFLUSH	4

#define TIOCPKT		6

#define TIOCPKT_DATA		 0
#define TIOCPKT_FLUSHREAD	 1
#define TIOCPKT_FLUSHWRITE	 2
#define TIOCPKT_STOP		 4
#define TIOCPKT_START		 8
#define TIOCPKT_NOSTOP		16
#define TIOCPKT_DOSTOP		32

#define FIONBIO 0x8004667e /* To be compatible with socket version */

#define CTRL(ch)	((ch)&0x1F)

#define CNUL	0
#define CDEL	0x0007f
#define CESC	'\\'
#define CINTR	CTRL('C')
#define CQUIT	0x0001c
#define CERASE	CTRL('H')
#define CKILL	CTRL('U')
#define CEOT	CTRL('D')
#define CEOL	0
#define CEOL2	0
#define CEOF	CTRL('D')
#define CSTART	CTRL('Q')
#define CSTOP	CTRL('S')
#define CSWTCH	0x0001a
#define NSWTCH	0
#define CSUSP	CTRL('Z')
#define CDSUSP	CTRL('Y')
#define CRPRNT	CTRL('R')
#define CFLUSH	CTRL('O')
#define CWERASE	CTRL('W')
#define CLNEXT	CTRL('V')

/* iflag bits */
#define IGNBRK	0x00001
#define BRKINT	0x00002
#define IGNPAR	0x00004
#define IMAXBEL	0x00008
#define INPCK	0x00010
#define ISTRIP	0x00020
#define INLCR	0x00040
#define IGNCR	0x00080
#define ICRNL	0x00100
#define IXON	0x00400
#define IXOFF	0x01000
#define IUCLC	0x04000
#define IXANY	0x08000
#define PARMRK	0x10000

/* oflag bits */

#define OPOST	0x00001
#define OLCUC	0x00002
#define OCRNL	0x00004
#define ONLCR	0x00008
#define ONOCR	0x00010
#define ONLRET	0x00020
#define OFILL	0x00040
#define CRDLY	0x00180
#define CR0	0x00000
#define CR1	0x00080
#define CR2	0x00100
#define CR3	0x00180
#define NLDLY	0x00200
#define NL0	0x00000
#define NL1	0x00200
#define BSDLY	0x00400
#define BS0	0x00000
#define BS1	0x00400
#define TABDLY	0x01800
#define TAB0	0x00000
#define TAB1	0x00800
#define TAB2	0x01000
#define TAB3	0x01800
#define XTABS	0x01800
#define VTDLY	0x02000
#define VT0	0x00000
#define VT1	0x02000
#define FFDLY	0x04000
#define FF0	0x00000
#define FF1	0x04000
#define OFDEL	0x08000

/* cflag bits */

/* Baud rate values.  These must fit in speed_t, which is unsigned
   char.  See also the extended baud rates below.  These baud rates
   set an additional bit. */
#define CBAUD	 0x0100f
#define B0	 0x00000
#define B50	 0x00001
#define B75	 0x00002
#define B110	 0x00003
#define B134	 0x00004
#define B150	 0x00005
#define B200	 0x00006
#define B300	 0x00007
#define B600	 0x00008
#define B1200	 0x00009
#define B1800	 0x0000a
#define B2400	 0x0000b
#define B4800	 0x0000c
#define B9600	 0x0000d
#define B19200	 0x0000e
#define B38400	 0x0000f

#define CSIZE	 0x00030
#define CS5	 0x00000
#define CS6	 0x00010
#define CS7	 0x00020
#define CS8	 0x00030
#define CSTOPB	 0x00040
#define CREAD	 0x00080
#define PARENB	 0x00100
#define PARODD	 0x00200
#define HUPCL	 0x00400
#define CLOCAL	 0x00800

/* Extended baud rates above 37K. */
#define CBAUDEX	 0x0100f
#define B57600	 0x01001
#define B115200	 0x01002
#define B128000	 0x01003
#define B230400  0x01004
#define B256000	 0x01005
#define B460800  0x01006
#define B500000  0x01007
#define B576000  0x01008
#define B921600  0x01009
#define B1000000 0x0100a
#define B1152000 0x0100b
#define B1500000 0x0100c
#define B2000000 0x0100d
#define B2500000 0x0100e
#define B3000000 0x0100f

#define CRTSXOFF 0x04000
#define CRTSCTS	 0x08000

/* lflag bits */
#define ISIG	0x0001
#define ICANON	0x0002
#define ECHO	0x0004
#define ECHOE	0x0008
#define ECHOK	0x0010
#define ECHONL	0x0020
#define NOFLSH	0x0040
#define TOSTOP	0x0080
#define IEXTEN	0x0100
#define FLUSHO	0x0200
#define ECHOKE	0x0400
#define ECHOCTL	0x0800

#define VDISCARD	1
#define VEOL		2
#define VEOL2		3
#define VEOF		4
#define VERASE		5
#define VINTR		6
#define VKILL		7
#define VLNEXT		8
#define VMIN		9
#define VQUIT		10
#define VREPRINT	11
#define VSTART		12
#define VSTOP		13
#define VSUSP		14
#define VSWTC		15
#define VTIME		16
#define VWERASE	17

#define NCCS		18

/* `c_cc' member of 'struct termios' structure can be disabled by
   using the value _POSIX_VDISABLE.  */
#define	_POSIX_VDISABLE	'\0'

/* Compare a character C to a value VAL from the `c_cc' array in a
   `struct termios'.  If VAL is _POSIX_VDISABLE, no character can match it.  */
#define CCEQ(val, c)	((c) == (val) && (val) != _POSIX_VDISABLE)

typedef unsigned char cc_t;
typedef unsigned int  tcflag_t;
typedef unsigned int  speed_t;
typedef unsigned short otcflag_t;
typedef unsigned char ospeed_t;

struct __oldtermios
{
  otcflag_t	c_iflag;
  otcflag_t	c_oflag;
  otcflag_t	c_cflag;
  otcflag_t	c_lflag;
  char		c_line;
  cc_t		c_cc[NCCS];
  ospeed_t	c_ispeed;
  ospeed_t	c_ospeed;
};

struct termios
{
  tcflag_t	c_iflag;
  tcflag_t	c_oflag;
  tcflag_t	c_cflag;
  tcflag_t	c_lflag;
  char		c_line;
  cc_t		c_cc[NCCS];
  speed_t	c_ispeed;
  speed_t	c_ospeed;
};

#ifdef CYGWIN_VERSION_DLL_IS_OLD_TERMIOS
#ifdef __GNUC__
# define __tonew_termios(ti) \
  ({ \
    struct termios *__newti; \
   \
    if (!CYGWIN_VERSION_DLL_IS_OLD_TERMIOS) \
      __newti = (struct termios *) ti; \
    else \
      { \
	__newti = (struct termios *) alloca(sizeof(struct termios)); \
	__newti->c_iflag = ((struct __oldtermios *)ti)->c_iflag; \
	__newti->c_oflag = ((struct __oldtermios *)ti)->c_oflag; \
	__newti->c_cflag = ((struct __oldtermios *)ti)->c_cflag; \
	__newti->c_lflag = ((struct __oldtermios *)ti)->c_lflag; \
	__newti->c_line = ((struct __oldtermios *)ti)->c_line; \
	__newti->c_ispeed = ((struct __oldtermios *)ti)->c_ispeed; \
	__newti->c_ospeed = ((struct __oldtermios *)ti)->c_ospeed; \
	memcpy (__newti->c_cc, ((struct __oldtermios *)ti)->c_cc, sizeof(__newti->c_cc)); \
      } \
    __newti; \
  })

# define __makenew_termios(ti) \
  (CYGWIN_VERSION_DLL_IS_OLD_TERMIOS ? \
   (struct termios *) alloca (sizeof (struct termios)) : (ti))

# define __toapp_termios(toti, fromti) \
  ({ \
    if (!CYGWIN_VERSION_DLL_IS_OLD_TERMIOS) \
      toti = fromti; \
    else \
      { \
	((struct __oldtermios *)toti)->c_iflag = fromti->c_iflag; \
	((struct __oldtermios *)toti)->c_oflag = fromti->c_oflag; \
	((struct __oldtermios *)toti)->c_cflag = fromti->c_cflag; \
	((struct __oldtermios *)toti)->c_lflag = fromti->c_lflag; \
	((struct __oldtermios *)toti)->c_line = fromti->c_line; \
	((struct __oldtermios *)toti)->c_ispeed = fromti->c_ispeed; \
	((struct __oldtermios *)toti)->c_ospeed = fromti->c_ospeed; \
	memcpy (((struct __oldtermios*)toti)->c_cc, fromti->c_cc, sizeof(fromti->c_cc)); \
      } \
    toti; \
  })
#endif /*__GNUC__*/
#endif

#define termio termios

#define cfgetospeed(tp)		((tp)->c_ospeed)
#define cfgetispeed(tp)		((tp)->c_ispeed)

#ifdef __cplusplus
extern "C" {
#endif

int tcgetattr (int, struct termios *);
int tcsetattr (int, int, const struct termios *);
int tcsendbreak (int, int);
int tcdrain (int);
int tcflush (int, int);
int tcflow (int, int);
int cfsetispeed (struct termios *, speed_t);
int cfsetospeed (struct termios *, speed_t);

#ifdef __cplusplus
}
#endif

/* Extra stuff to make porting stuff easier.  */
struct winsize
{
  unsigned short ws_row, ws_col;
  unsigned short ws_xpixel, ws_ypixel;
};

#define TIOCGWINSZ (('T' << 8) | 1)
#define TIOCSWINSZ (('T' << 8) | 2)
#define TIOCLINUX  (('T' << 8) | 3)

#endif	/* _SYS_TERMIOS_H */
