/* devices.h

   Copyright 2002, 2003 Red Hat, Inc.

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
  DEV_SD1_MAJOR = 65,
  FH_SD      = FHDEV (DEV_SD_MAJOR, 0),
  FH_SD1     = FHDEV (DEV_SD1_MAJOR, 0),
  FH_SDA     = FHDEV (DEV_SD_MAJOR, 0),
  FH_SDB     = FHDEV (DEV_SD_MAJOR, 16),
  FH_SDC     = FHDEV (DEV_SD_MAJOR, 32),
  FH_SDD     = FHDEV (DEV_SD_MAJOR, 48),
  FH_SDE     = FHDEV (DEV_SD_MAJOR, 64),
  FH_SDF     = FHDEV (DEV_SD_MAJOR, 80),
  FH_SDG     = FHDEV (DEV_SD_MAJOR, 96),
  FH_SDH     = FHDEV (DEV_SD_MAJOR, 112),
  FH_SDI     = FHDEV (DEV_SD_MAJOR, 128),
  FH_SDJ     = FHDEV (DEV_SD_MAJOR, 144),
  FH_SDK     = FHDEV (DEV_SD_MAJOR, 160),
  FH_SDL     = FHDEV (DEV_SD_MAJOR, 176),
  FH_SDM     = FHDEV (DEV_SD_MAJOR, 192),
  FH_SDN     = FHDEV (DEV_SD_MAJOR, 208),
  FH_SDO     = FHDEV (DEV_SD_MAJOR, 224),
  FH_SDP     = FHDEV (DEV_SD_MAJOR, 240),
  FH_SDQ     = FHDEV (DEV_SD1_MAJOR, 0),
  FH_SDR     = FHDEV (DEV_SD1_MAJOR, 16),
  FH_SDS     = FHDEV (DEV_SD1_MAJOR, 32),
  FH_SDT     = FHDEV (DEV_SD1_MAJOR, 48),
  FH_SDU     = FHDEV (DEV_SD1_MAJOR, 64),
  FH_SDV     = FHDEV (DEV_SD1_MAJOR, 80),
  FH_SDW     = FHDEV (DEV_SD1_MAJOR, 96),
  FH_SDX     = FHDEV (DEV_SD1_MAJOR, 112),
  FH_SDY     = FHDEV (DEV_SD1_MAJOR, 128),
  FH_SDZ     = FHDEV (DEV_SD1_MAJOR, 144),

  FH_MEM     = FHDEV (1, 1),
  FH_KMEM    = FHDEV (1, 2),	/* not implemented yet */
  FH_NULL    = FHDEV (1, 3),
  FH_ZERO    = FHDEV (1, 4),
  FH_PORT    = FHDEV (1, 5),
  FH_RANDOM  = FHDEV (1, 8),
  FH_URANDOM = FHDEV (1, 9),
  FH_OSS_DSP = FHDEV (14, 3),

  DEV_CYGDRIVE_MAJOR = 98,
  FH_CYGDRIVE= FHDEV (DEV_CYGDRIVE_MAJOR, 0),
  FH_CYGDRIVE_A= FHDEV (DEV_CYGDRIVE_MAJOR, 'a'),
  FH_CYGDRIVE_Z= FHDEV (DEV_CYGDRIVE_MAJOR, 'z'),

  DEV_TCP_MAJOR = 30,
  FH_TCP = FHDEV (DEV_TCP_MAJOR, 36),
  FH_UDP = FHDEV (DEV_TCP_MAJOR, 39),
  FH_ICMP = FHDEV (DEV_TCP_MAJOR, 33),
  FH_UNIX = FHDEV (DEV_TCP_MAJOR, 120),
  FH_STREAM = FHDEV (DEV_TCP_MAJOR, 121),
  FH_DGRAM = FHDEV (DEV_TCP_MAJOR, 122),

  FH_BAD     = FHDEV (0, 0)
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
  const char *native;
  _mode_t mode;
  bool dev_on_fs;
  static const device *lookup (const char *, unsigned int = 0xffffffff);
  void parse (const char *);
  void parse (_major_t major, _minor_t minor);
  void parse (_dev_t dev);
  inline bool setunit (unsigned n)
  {
    minor = n;
    return true;
  }
  static void init ();
  void tty_to_real_device ();
  inline operator int () const {return devn;}
  inline void setfs (bool x) {dev_on_fs = x;}
  inline bool isfs () const {return dev_on_fs;}
};

extern const device *console_dev;
extern const device *ttym_dev;
extern const device *ttys_dev;
extern const device *urandom_dev;

extern const device dev_dgram_storage;
#define dgram_dev (&dev_dgram_storage)
extern const device dev_stream_storage;
#define stream_dev (&dev_stream_storage)
extern const device dev_tcp_storage;
#define tcp_dev (&dev_tcp_storage)
extern const device dev_udp_storage;
#define udp_dev (&dev_udp_storage)

extern const device dev_piper_storage;
#define piper_dev (&dev_piper_storage)
extern const device dev_pipew_storage;
#define pipew_dev (&dev_pipew_storage)
extern const device dev_proc_storage;
#define proc_dev (&dev_proc_storage)
extern const device dev_cygdrive_storage;
#define cygdrive_dev (&dev_cygdrive_storage)
extern const device dev_fh_storage;
#define fh_dev (&dev_fh_storage)
extern const device dev_fs_storage;
#define fs_dev (&dev_fs_storage)
