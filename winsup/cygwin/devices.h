/* devices.h

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

typedef unsigned short _major_t;
typedef unsigned short _minor_t;
typedef mode_t _mode_t;
typedef __dev32_t _dev_t;

#define FHDEV(maj, min) ((((unsigned) (maj)) << (sizeof (_major_t) * 8)) | (unsigned) (min))
#define _minor(dev) ((dev) & ((1 << (sizeof (_minor_t) * 8)) - 1))
#define _major(dev) ((dev) >> (sizeof (_major_t) * 8))

enum fh_devices
{
  FH_TTY     = FHDEV (5, 0),
  FH_CONSOLE = FHDEV (5, 1),
  FH_PTYM    = FHDEV (5, 2),	/* /dev/ptmx */
  FH_CONIN   = FHDEV (5, 255),
  FH_CONOUT  = FHDEV (5, 254),

  DEV_TTYM_MAJOR = 128,
  FH_TTYM    = FHDEV (DEV_TTYM_MAJOR, 0),
  FH_TTYM_MAX= FHDEV (DEV_TTYM_MAJOR, 255),

  DEV_TTYS_MAJOR = 136,
  FH_TTYS    = FHDEV (DEV_TTYS_MAJOR, 0),	/* FIXME: Should separate ttys and ptys */
  FH_TTYS_MAX= FHDEV (DEV_TTYS_MAJOR, 255),	/* FIXME: Should separate ttys and ptys */

  DEV_SERIAL_MAJOR = 117,
  FH_SERIAL  = FHDEV (117, 0),	/* /dev/ttyS? */

  FH_WINDOWS = FHDEV (13, 255),
  FH_CLIPBOARD=FHDEV (13, 254),

  FH_PIPE    = FHDEV (0, 255),
  FH_PIPER   = FHDEV (0, 254),
  FH_PIPEW   = FHDEV (0, 253),
  FH_FIFO    = FHDEV (0, 252),
  FH_SOCKET  = FHDEV (0, 251),
  FH_PROC    = FHDEV (0, 250),
  FH_REGISTRY= FHDEV (0, 249),
  FH_PROCESS = FHDEV (0, 248),

  FH_FS      = FHDEV (0, 247),	/* filesystem based device */

  DEV_FLOPPY_MAJOR = 2,
  FH_FLOPPY  = FHDEV (DEV_FLOPPY_MAJOR, 0),

  DEV_CDROM_MAJOR = 11,
  FH_CDROM   = FHDEV (DEV_CDROM_MAJOR, 0),

  DEV_TAPE_MAJOR = 9,
  FH_TAPE    = FHDEV (DEV_TAPE_MAJOR, 0),
  FH_NTAPE   = FHDEV (DEV_TAPE_MAJOR, 128),
  FH_MAXNTAPE= FHDEV (DEV_TAPE_MAJOR, 255),

  DEV_SD_MAJOR = 8,
  FH_SD      = FHDEV (8, 0),

  FH_MEM     = FHDEV (1, 1),
  FH_KMEM    = FHDEV (1, 2),	/* not implemented yet */
  FH_NULL    = FHDEV (1, 3),
  FH_ZERO    = FHDEV (1, 4),
  FH_PORT    = FHDEV (1, 5),
  FH_RANDOM  = FHDEV (1, 8),
  FH_URANDOM = FHDEV (1, 9),
  FH_OSS_DSP = FHDEV (14, 3),

  DEV_CYGDRIVE_MAJOR = 30,
  FH_CYGDRIVE= FHDEV (DEV_CYGDRIVE_MAJOR, 0),
  FH_CYGDRIVE_A= FHDEV (DEV_CYGDRIVE_MAJOR, 'a'),
  FH_CYGDRIVE_Z= FHDEV (DEV_CYGDRIVE_MAJOR, 'z'),

  DEV_RAWDRIVE_MAJOR = 65,
  FH_RAWDRIVE= FHDEV (DEV_RAWDRIVE_MAJOR, 0),

  FH_BAD     = 0
};

struct device
{
  const char *name;
  union
  {
    _dev_t devn;
    struct
    {
      _minor_t minor;
      _major_t major;
    };
  };
  const char *fmt;
  unsigned lower, upper;
  int adjust;
  unsigned mul;
  _mode_t mode;
  bool dev_on_fs;
  static const device *lookup (const char *, unsigned int = 0xffffffff);
  void parse (const char *);
  void parse (_major_t major, _minor_t minor);
  void parse (_dev_t dev);
  inline bool setunit (unsigned n)
  {
    if (mul && n > mul)
      return false;
    minor += (n + adjust) * (mul ?: 1);
    return true;
  }
  static void init ();
  void tty_to_real_device ();
  inline operator int () const {return devn;}
  inline void setfs (bool x) {dev_on_fs = x;}
  inline bool isfs () const {return dev_on_fs;}
};

extern const device *console_dev;
extern const device *piper_dev;
extern const device *pipew_dev;
extern const device *socket_dev;
extern const device *ttym_dev;
extern const device *ttys_dev;
extern const device *urandom_dev;
