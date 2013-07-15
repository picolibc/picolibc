/* devices.h

   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012 Red
   Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

typedef unsigned short _major_t;
typedef unsigned short _minor_t;
typedef mode_t _mode_t;

#define FHDEV(maj, min) ((((unsigned) (maj)) << (sizeof (_major_t) * 8)) | (unsigned) (min))
#define _minor(dev) ((dev) & ((1 << (sizeof (_minor_t) * 8)) - 1))
#define _major(dev) ((dev) >> (sizeof (_major_t) * 8))

#include <sys/stat.h>
#include <dirent.h>

#define MAX_CONSOLES 63
enum fh_devices
{
  DEV_TTY_MAJOR = 5,
  FH_TTY     = FHDEV (DEV_TTY_MAJOR, 0),
  FH_CONSOLE = FHDEV (DEV_TTY_MAJOR, 1),
  FH_PTMX    = FHDEV (DEV_TTY_MAJOR, 2),
  FH_CONIN   = FHDEV (DEV_TTY_MAJOR, 255),
  FH_CONOUT  = FHDEV (DEV_TTY_MAJOR, 254),

  DEV_CONS_MAJOR = 3,
  FH_CONS     = FHDEV (DEV_CONS_MAJOR, 0),
  FH_CONS_MAX = FHDEV (DEV_CONS_MAJOR, MAX_CONSOLES),

  DEV_PTYM_MAJOR = 128,
  FH_PTYM    = FHDEV (DEV_PTYM_MAJOR, 0),
  FH_PTYM_MAX= FHDEV (DEV_PTYM_MAJOR, 255),

  DEV_PTYS_MAJOR = 136,
  FH_PTYS    = FHDEV (DEV_PTYS_MAJOR, 0),	/* FIXME: Should separate ttys and ptys */
  FH_PTYS_MAX= FHDEV (DEV_PTYS_MAJOR, 255),	/* FIXME: Should separate ttys and ptys */

  DEV_SERIAL_MAJOR = 117,
  FH_SERIAL  = FHDEV (117, 0),	/* /dev/ttyS? */

  DEV_MISC_MAJOR = 13,
  FH_WINDOWS = FHDEV (DEV_MISC_MAJOR, 255),
  FH_CLIPBOARD=FHDEV (DEV_MISC_MAJOR, 254),

  /* begin /proc directories */

  DEV_VIRTFS_MAJOR = 0,
  FH_PROC_MIN_MINOR = FHDEV (DEV_VIRTFS_MAJOR, 200),
  FH_PROCSYSVIPC = FHDEV (DEV_VIRTFS_MAJOR, 249),
  FH_PROCSYS = FHDEV (DEV_VIRTFS_MAJOR, 250),
  FH_PROCESSFD = FHDEV (DEV_VIRTFS_MAJOR, 251),
  FH_PROCNET = FHDEV (DEV_VIRTFS_MAJOR, 252),
  FH_REGISTRY= FHDEV (DEV_VIRTFS_MAJOR, 253),
  FH_PROCESS = FHDEV (DEV_VIRTFS_MAJOR, 254),
  FH_PROC    = FHDEV (DEV_VIRTFS_MAJOR, 255),
  FH_PROC_MAX_MINOR = FHDEV (DEV_VIRTFS_MAJOR, 255),

  /* end /proc directories */

  FH_PIPE    = FHDEV (DEV_VIRTFS_MAJOR, 199),
  FH_PIPER   = FHDEV (DEV_VIRTFS_MAJOR, 198),
  FH_PIPEW   = FHDEV (DEV_VIRTFS_MAJOR, 197),
  FH_FIFO    = FHDEV (DEV_VIRTFS_MAJOR, 196),
  FH_FS      = FHDEV (DEV_VIRTFS_MAJOR, 195),  /* filesystem based device */
  FH_NETDRIVE= FHDEV (DEV_VIRTFS_MAJOR, 194),
  FH_DEV     = FHDEV (DEV_VIRTFS_MAJOR, 193),
  FH_CYGDRIVE= FHDEV (DEV_VIRTFS_MAJOR, 192),

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
  DEV_SD2_MAJOR = 66,
  DEV_SD3_MAJOR = 67,
  DEV_SD4_MAJOR = 68,
  DEV_SD5_MAJOR = 69,
  DEV_SD6_MAJOR = 70,
  DEV_SD7_MAJOR = 71,
  FH_SD      = FHDEV (DEV_SD_MAJOR, 0),
  FH_SD1     = FHDEV (DEV_SD1_MAJOR, 0),
  FH_SD2     = FHDEV (DEV_SD2_MAJOR, 0),
  FH_SD3     = FHDEV (DEV_SD3_MAJOR, 0),
  FH_SD4     = FHDEV (DEV_SD4_MAJOR, 0),
  FH_SD5     = FHDEV (DEV_SD5_MAJOR, 0),
  FH_SD6     = FHDEV (DEV_SD6_MAJOR, 0),
  FH_SD7     = FHDEV (DEV_SD7_MAJOR, 0),
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
  FH_SDAA    = FHDEV (DEV_SD1_MAJOR, 160),
  FH_SDAB    = FHDEV (DEV_SD1_MAJOR, 176),
  FH_SDAC    = FHDEV (DEV_SD1_MAJOR, 192),
  FH_SDAD    = FHDEV (DEV_SD1_MAJOR, 208),
  FH_SDAE    = FHDEV (DEV_SD1_MAJOR, 224),
  FH_SDAF    = FHDEV (DEV_SD1_MAJOR, 240),
  FH_SDAG    = FHDEV (DEV_SD2_MAJOR, 0),
  FH_SDAH    = FHDEV (DEV_SD2_MAJOR, 16),
  FH_SDAI    = FHDEV (DEV_SD2_MAJOR, 32),
  FH_SDAJ    = FHDEV (DEV_SD2_MAJOR, 48),
  FH_SDAK    = FHDEV (DEV_SD2_MAJOR, 64),
  FH_SDAL    = FHDEV (DEV_SD2_MAJOR, 80),
  FH_SDAM    = FHDEV (DEV_SD2_MAJOR, 96),
  FH_SDAN    = FHDEV (DEV_SD2_MAJOR, 112),
  FH_SDAO    = FHDEV (DEV_SD2_MAJOR, 128),
  FH_SDAP    = FHDEV (DEV_SD2_MAJOR, 144),
  FH_SDAQ    = FHDEV (DEV_SD2_MAJOR, 160),
  FH_SDAR    = FHDEV (DEV_SD2_MAJOR, 176),
  FH_SDAS    = FHDEV (DEV_SD2_MAJOR, 192),
  FH_SDAT    = FHDEV (DEV_SD2_MAJOR, 208),
  FH_SDAU    = FHDEV (DEV_SD2_MAJOR, 224),
  FH_SDAV    = FHDEV (DEV_SD2_MAJOR, 240),
  FH_SDAW    = FHDEV (DEV_SD3_MAJOR, 0),
  FH_SDAX    = FHDEV (DEV_SD3_MAJOR, 16),
  FH_SDAY    = FHDEV (DEV_SD3_MAJOR, 32),
  FH_SDAZ    = FHDEV (DEV_SD3_MAJOR, 48),
  FH_SDBA    = FHDEV (DEV_SD3_MAJOR, 64),
  FH_SDBB    = FHDEV (DEV_SD3_MAJOR, 80),
  FH_SDBC    = FHDEV (DEV_SD3_MAJOR, 96),
  FH_SDBD    = FHDEV (DEV_SD3_MAJOR, 112),
  FH_SDBE    = FHDEV (DEV_SD3_MAJOR, 128),
  FH_SDBF    = FHDEV (DEV_SD3_MAJOR, 144),
  FH_SDBG    = FHDEV (DEV_SD3_MAJOR, 160),
  FH_SDBH    = FHDEV (DEV_SD3_MAJOR, 176),
  FH_SDBI    = FHDEV (DEV_SD3_MAJOR, 192),
  FH_SDBJ    = FHDEV (DEV_SD3_MAJOR, 208),
  FH_SDBK    = FHDEV (DEV_SD3_MAJOR, 224),
  FH_SDBL    = FHDEV (DEV_SD3_MAJOR, 240),
  FH_SDBM    = FHDEV (DEV_SD4_MAJOR, 0),
  FH_SDBN    = FHDEV (DEV_SD4_MAJOR, 16),
  FH_SDBO    = FHDEV (DEV_SD4_MAJOR, 32),
  FH_SDBP    = FHDEV (DEV_SD4_MAJOR, 48),
  FH_SDBQ    = FHDEV (DEV_SD4_MAJOR, 64),
  FH_SDBR    = FHDEV (DEV_SD4_MAJOR, 80),
  FH_SDBS    = FHDEV (DEV_SD4_MAJOR, 96),
  FH_SDBT    = FHDEV (DEV_SD4_MAJOR, 112),
  FH_SDBU    = FHDEV (DEV_SD4_MAJOR, 128),
  FH_SDBV    = FHDEV (DEV_SD4_MAJOR, 144),
  FH_SDBW    = FHDEV (DEV_SD4_MAJOR, 160),
  FH_SDBX    = FHDEV (DEV_SD4_MAJOR, 176),
  FH_SDBY    = FHDEV (DEV_SD4_MAJOR, 192),
  FH_SDBZ    = FHDEV (DEV_SD4_MAJOR, 208),
  FH_SDCA    = FHDEV (DEV_SD4_MAJOR, 224),
  FH_SDCB    = FHDEV (DEV_SD4_MAJOR, 240),
  FH_SDCC    = FHDEV (DEV_SD5_MAJOR, 0),
  FH_SDCD    = FHDEV (DEV_SD5_MAJOR, 16),
  FH_SDCE    = FHDEV (DEV_SD5_MAJOR, 32),
  FH_SDCF    = FHDEV (DEV_SD5_MAJOR, 48),
  FH_SDCG    = FHDEV (DEV_SD5_MAJOR, 64),
  FH_SDCH    = FHDEV (DEV_SD5_MAJOR, 80),
  FH_SDCI    = FHDEV (DEV_SD5_MAJOR, 96),
  FH_SDCJ    = FHDEV (DEV_SD5_MAJOR, 112),
  FH_SDCK    = FHDEV (DEV_SD5_MAJOR, 128),
  FH_SDCL    = FHDEV (DEV_SD5_MAJOR, 144),
  FH_SDCM    = FHDEV (DEV_SD5_MAJOR, 160),
  FH_SDCN    = FHDEV (DEV_SD5_MAJOR, 176),
  FH_SDCO    = FHDEV (DEV_SD5_MAJOR, 192),
  FH_SDCP    = FHDEV (DEV_SD5_MAJOR, 208),
  FH_SDCQ    = FHDEV (DEV_SD5_MAJOR, 224),
  FH_SDCR    = FHDEV (DEV_SD5_MAJOR, 240),
  FH_SDCS    = FHDEV (DEV_SD6_MAJOR, 0),
  FH_SDCT    = FHDEV (DEV_SD6_MAJOR, 16),
  FH_SDCU    = FHDEV (DEV_SD6_MAJOR, 32),
  FH_SDCV    = FHDEV (DEV_SD6_MAJOR, 48),
  FH_SDCW    = FHDEV (DEV_SD6_MAJOR, 64),
  FH_SDCX    = FHDEV (DEV_SD6_MAJOR, 80),
  FH_SDCY    = FHDEV (DEV_SD6_MAJOR, 96),
  FH_SDCZ    = FHDEV (DEV_SD6_MAJOR, 112),
  FH_SDDA    = FHDEV (DEV_SD6_MAJOR, 128),
  FH_SDDB    = FHDEV (DEV_SD6_MAJOR, 144),
  FH_SDDC    = FHDEV (DEV_SD6_MAJOR, 160),
  FH_SDDD    = FHDEV (DEV_SD6_MAJOR, 176),
  FH_SDDE    = FHDEV (DEV_SD6_MAJOR, 192),
  FH_SDDF    = FHDEV (DEV_SD6_MAJOR, 208),
  FH_SDDG    = FHDEV (DEV_SD6_MAJOR, 224),
  FH_SDDH    = FHDEV (DEV_SD6_MAJOR, 240),
  FH_SDDI    = FHDEV (DEV_SD7_MAJOR, 0),
  FH_SDDJ    = FHDEV (DEV_SD7_MAJOR, 16),
  FH_SDDK    = FHDEV (DEV_SD7_MAJOR, 32),
  FH_SDDL    = FHDEV (DEV_SD7_MAJOR, 48),
  FH_SDDM    = FHDEV (DEV_SD7_MAJOR, 64),
  FH_SDDN    = FHDEV (DEV_SD7_MAJOR, 80),
  FH_SDDO    = FHDEV (DEV_SD7_MAJOR, 96),
  FH_SDDP    = FHDEV (DEV_SD7_MAJOR, 112),
  FH_SDDQ    = FHDEV (DEV_SD7_MAJOR, 128),
  FH_SDDR    = FHDEV (DEV_SD7_MAJOR, 144),
  FH_SDDS    = FHDEV (DEV_SD7_MAJOR, 160),
  FH_SDDT    = FHDEV (DEV_SD7_MAJOR, 176),
  FH_SDDU    = FHDEV (DEV_SD7_MAJOR, 192),
  FH_SDDV    = FHDEV (DEV_SD7_MAJOR, 208),
  FH_SDDW    = FHDEV (DEV_SD7_MAJOR, 224),
  FH_SDDX    = FHDEV (DEV_SD7_MAJOR, 240),

  DEV_MEM_MAJOR = 1,
  FH_NULL    = FHDEV (DEV_MEM_MAJOR, 3),
  FH_ZERO    = FHDEV (DEV_MEM_MAJOR, 5),
  FH_FULL    = FHDEV (DEV_MEM_MAJOR, 7),
  FH_RANDOM  = FHDEV (DEV_MEM_MAJOR, 8),
  FH_URANDOM = FHDEV (DEV_MEM_MAJOR, 9),
  FH_KMSG    = FHDEV (DEV_MEM_MAJOR, 11),

  DEV_SOUND_MAJOR = 14,
  FH_OSS_DSP = FHDEV (DEV_SOUND_MAJOR, 3),

  DEV_TCP_MAJOR = 30,
  FH_TCP = FHDEV (DEV_TCP_MAJOR, 36),
  FH_UDP = FHDEV (DEV_TCP_MAJOR, 39),
  FH_ICMP = FHDEV (DEV_TCP_MAJOR, 33),
  FH_UNIX = FHDEV (DEV_TCP_MAJOR, 120),
  FH_STREAM = FHDEV (DEV_TCP_MAJOR, 121),
  FH_DGRAM = FHDEV (DEV_TCP_MAJOR, 122),

  FH_NADA     = FHDEV (0, 0),
  FH_ERROR   = FHDEV (255, 255)	/* Set by fh constructor when error detected */
};

struct device
{
  const char *name;
  union __cygwin_dev
  {
    dev_t devn;
    DWORD devn_dword;
    int devn_int;
    fh_devices devn_fh_devices;
    struct
    {
      _minor_t minor;
      _major_t major;
    };
  } d;
  const char *native;
  int (*exists_func) (const device&);
  _mode_t mode;
  bool lives_in_dev:4;
  bool dev_on_fs:4;
  static const device *lookup (const char *, unsigned int = UINT32_MAX);
  void parse (const char *);
  void parse (_major_t major, _minor_t minor);
  void parse (dev_t dev);
  void parsedisk (int, int);
  inline bool setunit (unsigned n)
  {
    d.minor = n;
    return true;
  }
  static void init ();

  static _major_t major (dev_t n)
  {
    __cygwin_dev d;
    d.devn = n;
    return d.major;
  }
  static _minor_t minor (dev_t n)
  {
    __cygwin_dev d;
    d.devn = n;
    return d.minor;
  }
  static _major_t major (int n) {return major ((dev_t) n);}
  static _minor_t minor (int n) {return minor ((dev_t) n);}

  bool is_device (dev_t n) const {return n == d.devn; }
  bool not_device (dev_t n) const {return d.devn && n != d.devn; }

  _minor_t get_minor () const {return d.minor;}
  _major_t get_major () const {return d.major;}
  dev_t   get_device () const {return d.devn;}

  inline operator fh_devices () {return d.devn_fh_devices;}
  inline operator bool () {return !!d.devn_int;}
  inline operator dev_t& () {return d.devn;}
  fh_devices operator = (fh_devices n) {return d.devn_fh_devices = n;}
  inline void setfs (bool x) {dev_on_fs = x;}
  inline bool isfs () const {return dev_on_fs || d.devn == FH_FS;}
  inline bool is_fs_special () const {return dev_on_fs && d.devn != FH_FS;}
  inline bool is_dev_resident () const {return lives_in_dev;}
  inline int exists () const {return exists_func (*this);}
  unsigned char type () const
  {
    if (S_ISBLK (mode))
      return DT_BLK;
    return mode >> 12;
  }
};

extern const device dev_storage[];
extern const device *dev_storage_end;

extern const device *console_dev;
extern const device *ptmx_dev;
extern const device *ptys_dev;
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
extern const device dev_dev_storage;
#define dev_dev (&dev_dev_storage)
extern const device dev_netdrive_storage;
#define netdrive_dev (&dev_netdrive_storage)
extern const device dev_cygdrive_storage;
#define cygdrive_dev (&dev_cygdrive_storage)
extern const device dev_fh_storage;
#define fh_dev (&dev_fh_storage)
extern const device dev_fs_storage;
#define fs_dev (&dev_fs_storage)

#define isproc_dev(devn) \
  (devn >= FH_PROC_MIN_MINOR && devn <= FH_PROC_MAX_MINOR)

#define iscygdrive_dev(devn) (devn == FH_CYGDRIVE)

#define isdev_dev(devn) (devn == FH_DEV)

#define isprocsys_dev(devn) (devn == FH_PROCSYS)

#define isvirtual_dev(devn) \
  (isproc_dev (devn) || devn == FH_CYGDRIVE || devn == FH_NETDRIVE)

#define iscons_dev(n) \
  ((device::major ((dev_t) (n)) == DEV_CONS_MAJOR) \
   || (((dev_t) n) == FH_CONSOLE) \
   || (((dev_t) n) == FH_CONIN) \
   || (((dev_t) n) == FH_CONOUT))

#define istty_slave_dev(n) (device::major (n) == DEV_PTYS_MAJOR)
