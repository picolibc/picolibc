/* devices.h

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Change this if we use another size for devices */
#define FHDEV(maj, min) ((((unsigned) (maj)) << 16) | (unsigned) (min))

typedef unsigned short _major_t;
typedef unsigned short _minor_t;
typedef unsigned char _devtype_t;
typedef __dev32_t _dev_t;

enum fh_devices
{
  /* "Slow" devices */
  FH_TTY     = FHDEV (5, 0),
  FH_CONSOLE = FHDEV (5, 1),
  FH_CONIN   = FHDEV (5, 512),
  FH_CONOUT  = FHDEV (5, 513),
  FH_PTYM    = FHDEV (5, 2),	/* /dev/ptmx */

  DEV_TTYM_MAJOR = 128,
  FH_TTYM    = FHDEV (128, 0),
  FH_TTYM_MAX= FHDEV (128, 255),

  DEV_TTYS_MAJOR = 136,
  FH_TTYS    = FHDEV (DEV_TTYS_MAJOR, 0),	/* FIXME: Should separate ttys and ptys */
  FH_TTYS_MAX= FHDEV (DEV_TTYS_MAJOR, 255),	/* FIXME: Should separate ttys and ptys */

  DEV_SERIAL_MAJOR = 117,
  FH_SERIAL  = FHDEV (117, 0),	/* /dev/ttyS? */

  FH_PIPE    = FHDEV (0, 512),
  FH_PIPER   = FHDEV (0, 513),
  FH_PIPEW   = FHDEV (0, 514),
  FH_FIFO    = FHDEV (0, 515),
  FH_SOCKET  = FHDEV (0, 516),
  FH_WINDOWS = FHDEV (13, 512),

  /* Fast devices */
  FH_FS      = FHDEV (0, 517),	/* filesystem based device */

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

  FH_NULL    = FHDEV (1, 3),
  FH_ZERO    = FHDEV (1, 4),
  FH_PORT    = FHDEV (1, 5),
  FH_RANDOM  = FHDEV (1, 8),
  FH_URANDOM = FHDEV (1, 9),
  FH_MEM     = FHDEV (1, 1),
  FH_CLIPBOARD=FHDEV (13, 513),
  FH_OSS_DSP = FHDEV (14, 3),

  DEV_CYGDRIVE_MAJOR = 30,
  FH_CYGDRIVE= FHDEV (DEV_CYGDRIVE_MAJOR, 0),
  FH_CYGDRIVE_A= FHDEV (DEV_CYGDRIVE_MAJOR, 'a'),
  FH_CYGDRIVE_Z= FHDEV (DEV_CYGDRIVE_MAJOR, 'z'),

  FH_PROC    = FHDEV (0, 519),
  FH_REGISTRY= FHDEV (0, 520),
  FH_PROCESS = FHDEV (0, 521),

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
  _devtype_t type;
  static const device *lookup (const char *, unsigned int = 0xffffffff);
  void parse (const char *);
  inline bool setunit (unsigned n)
  {
    if (mul && n > mul)
      return false;
    minor += (n + adjust) * (mul ?: 1);
    return true;
  }
  static void init ();
  inline operator int () const {return devn;}
};

extern const device *console_dev;
extern const device *piper_dev;
extern const device *pipew_dev;
extern const device *socket_dev;
extern const device *ttym_dev;
extern const device *ttys_dev;
