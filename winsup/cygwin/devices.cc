/* ANSI-C code produced by gperf version 2.7.2 */
/* Command-line: gperf -c --key-positions=5-8,1-2,9-10 -r -t -C -E -T -L ANSI-C -Hdevhash -Ndevice::lookup -Z devstring -7 -G /cygnus/src/uberbaum/winsup/cygwin/devices.gperf  */
#include "winsup.h"
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "devices.h"
#include "sys/cygwin.h"
#include "tty.h"
#include "pinfo.h"
#undef __GNUC__
static unsigned int devhash (const char *, unsigned)
  __attribute__ ((regparm (2)));
enum
  {
    TOTAL_KEYWORDS = 54,
    MIN_WORD_LENGTH = 5,
    MAX_WORD_LENGTH = 14,
    MIN_HASH_VALUE = 131,
    MAX_HASH_VALUE = 358
  };

/* maximum key range = 228, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
devhash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      359, 359, 359, 359, 359, 359, 359, 359, 359, 359,
      359, 359, 359, 359, 359, 359, 359, 359, 359, 359,
      359, 359, 359, 359, 359, 359, 359, 359, 359, 359,
      359, 359, 359, 359, 359, 359, 359, 359, 359, 359,
      359, 359, 359, 359, 359, 359, 359,  60, 359, 359,
      359, 359, 359, 359, 359, 359, 359, 359,  62, 359,
      359, 359, 359, 359, 359, 359, 359, 359, 359, 359,
      359, 359, 359, 359, 359, 359, 359, 359, 359, 359,
      359, 359, 359,  15, 359, 359, 359, 359, 359, 359,
      359, 359, 359, 359, 359, 359, 359,   0,   2,  46,
       42,  50,   3,   4,  36,  15,  10,  41,  61,  13,
       26,  30,  52,  57,  45,  27,  59,  39,  51,  38,
       32,  18,  14, 359, 359, 359, 359, 359
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 10:
        hval += asso_values[(unsigned) cyg_tolower (str[9])];
      case 9:
        hval += asso_values[(unsigned) cyg_tolower (str[8])];
      case 8:
        hval += asso_values[(unsigned) cyg_tolower (str[7])];
      case 7:
        hval += asso_values[(unsigned) cyg_tolower (str[6])];
      case 6:
        hval += asso_values[(unsigned) cyg_tolower (str[5])];
      case 5:
        hval += asso_values[(unsigned) cyg_tolower (str[4])];
      case 4:
      case 3:
      case 2:
        hval += asso_values[(unsigned) cyg_tolower (str[1])];
      case 1:
        hval += asso_values[(unsigned) cyg_tolower (str[0])];
        break;
    }
  return hval;
}

static NO_COPY const struct device wordlist[] =
  {
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""},
    {":bad:", FH_BAD, ":bad:", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
    {"/dev/fd", FH_FLOPPY, "\\Device\\Floppy%d", 0, 15},
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/fifo", FH_FIFO, "\\dev\\fifo", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/sr", FH_CDROM, "\\Device\\CdRom%d", 0, 15},
    {""}, {""}, {""}, {""},
    {"/dev/mem", FH_MEM, "\\dev\\mem", 0, 0, 0, 0},
    {""},
    {"/dev/hda", FH_SDA, "\\Device\\Harddisk%d\\Partition%d", 1, 16, -1},
    {""},
    {"/dev/hdb", FH_SDB, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 15},
    {"/dev/hdf", FH_SDF, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 79},
    {"/dev/hdg", FH_SDG, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 95},
    {""}, {""},
    {"/dev/st", FH_TAPE, "\\Device\\Tape%d", 0, 127},
    {""}, {""},
    {"/dev/hdj", FH_SDJ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 143},
    {"/dev/com", FH_SERIAL, "\\.\\com%d", 1, 99},
    {""},
    {"/dev/hdm", FH_SDM, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 191},
    {"/dev/hdz", FH_SDZ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 399},
    {"/dev/hdi", FH_SDI, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 127},
    {""}, {""},
    {"/dev/hdy", FH_SDY, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 383},
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/hdn", FH_SDN, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 207},
    {"/dev/hds", FH_SDS, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 287},
    {""}, {""},
    {"/dev/hdo", FH_SDO, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 223},
    {""},
    {"/dev/hdx", FH_SDX, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 367},
    {""},
    {"/dev/nst", FH_NTAPE, "\\Device\\Tape%d", 0, 127},
    {""},
    {"/dev/hdh", FH_SDH, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 111},
    {"/dev/scd", FH_CDROM, "\\Device\\CdRom%d", 0, 15},
    {"/dev/hdw", FH_SDW, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 351},
    {"/dev/hdu", FH_SDU, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 319},
    {"/dev/kmem", FH_KMEM, "\\dev\\mem", 0, 0, 0, 0},
    {"/dev/hdk", FH_SDK, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 159},
    {"/dev/hdd", FH_SDD, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 47},
    {"/dev/dsp", FH_OSS_DSP, "\\dev\\dsp", 0, 0, 0, 0},
    {""},
    {"/dev/hdr", FH_SDR, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 271},
    {"/dev/hdc", FH_SDC, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 31},
    {""}, {""}, {""},
    {"/dev/hde", FH_SDE, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 63},
    {"/dev/hdv", FH_SDV, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 335},
    {"/dev/hdp", FH_SDP, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 239},
    {""}, {""}, {""}, {""},
    {"/dev/hdq", FH_SDQ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 255  /* FIXME 8 bit lunacy */},
    {"/dev/tty", FH_TTY, "\\dev\\tty", 0, 0, 0, 0},
    {"/dev/hdt", FH_SDT, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 303},
    {""},
    {"/dev/hdl", FH_SDL, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 175},
    {"/dev/zero", FH_ZERO, "\\dev\\zero", 0, 0, 0, 0},
    {""}, {""}, {""}, {""},
    {"/dev/conin", FH_CONIN, "conin", 0, 0, 0, 0},
    {"/dev/random", FH_RANDOM, "\\dev\\random", 0, 0, 0, 0},
    {""}, {""}, {""},
    {"/dev/ttym", FH_TTYM, "\\dev\\ttym", 0, 255, 0, 0},
    {""},
    {"/dev/ttyS", FH_SERIAL, "\\.\\com%d", 0, 99, -1},
    {""}, {""},
    {"/dev/windows", FH_WINDOWS, "\\dev\\windows", 0, 0, 0, 0},
    {"/dev/urandom", FH_URANDOM, "\\dev\\urandom", 0, 0, 0, 0},
    {"/dev/ptmx", FH_PTYM, "\\dev\\ptmx", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""},
    {"/dev/console", FH_CONSOLE, "\\dev\\console", 0, 0, 0, 0},
    {"/dev/ttys", FH_TTYS, "\\dev\\tty%d", 0, 255, 0, 0},
    {""}, {""}, {""}, {""}, {""},
    {"/dev/pipe", FH_PIPE, "\\dev\\pipe", 0, 0, 0, 0},
    {""}, {""}, {""},
    {"/dev/conout", FH_CONOUT, "conout", 0, 0, 0, 0},
    {"/dev/rawdrive", FH_RAWDRIVE, "\\DosDevices\\%c:", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/clipboard", FH_CLIPBOARD, "\\dev\\clipboard", 0, 0, 0, 0},
    {""}, {""}, {""}, {""},
    {"/dev/port", FH_PORT, "\\dev\\port", 0, 0, 0, 0},
    {"/dev/null", FH_NULL, "nul", 0, 0, 0, 0}
  };

static const device cygdrive_dev_storage =
  {"/cygdrive", FH_CYGDRIVE, "/cygdrive", 0, 0, 0, 0};

static const device fs_dev_storage =
  {"", FH_FS, "", 0, 0, 0, 0};

static const device proc_dev_storage =
  {"", FH_PROC, "", 0, 0, 0, 0};

static const device registry_dev_storage =
  {"", FH_REGISTRY, "", 0, 0, 0, 0};

static const device process_dev_storage =
  {"", FH_PROCESS, "", 0, 0, 0, 0};

static const device tcp_dev_storage =
  {"/dev/inet/tcp", FH_TCP, "", 0, 0, 0, 0};

static const device udp_dev_storage =
  {"/dev/inet/udp", FH_UCP, "", 0, 0, 0, 0};

static const device icmp_dev_storage =
  {"/dev/inet/icmp", FH_ICMP, "", 0, 0, 0, 0};

static const device unix_dev_storage =
  {"/dev/inet/unix", FH_UNIX, "", 0, 0, 0, 0};

static const device stream_dev_storage =
  {"/dev/inet/stream", FH_STREAM, "", 0, 0, 0, 0};

static const device dgram_dev_storage =
  {"/dev/inet/dgram", FH_DGRAM, "", 0, 0, 0, 0};

static const device piper_dev_storage =
  {"", FH_PIPER, "", 0, 0, 0, 0};

static const device pipew_dev_storage =
  {"", FH_PIPEW, "", 0, 0, 0, 0};

static const device dev_fs =
  {"", FH_FS, "", 0, 0, 0, 0};
const device *bad_dev = wordlist + 131;
const device *cdrom_dev = wordlist + 241;
const device *cdrom_dev1 = wordlist + 285;
const device *clipboard_dev = wordlist + 352;
const device *conin_dev = wordlist + 315;
const device *conout_dev = wordlist + 344;
const device *console_dev = wordlist + 333;
const device *cygdrive_dev = &cygdrive_dev_storage;
const device *dgram_dev = &dgram_dev_storage;
const device *fifo_dev = wordlist + 222;
const device *floppy_dev = wordlist + 214;
const device *fs_dev = &fs_dev_storage;
const device *icmp_dev = &icmp_dev_storage;
const device *kmem_dev = wordlist + 288;
const device *mem_dev = wordlist + 246;
const device *ntape_dev = wordlist + 282;
const device *null_dev = wordlist + 358;
const device *oss_dsp_dev = wordlist + 291;
const device *pipe_dev = wordlist + 340;
const device *piper_dev = &piper_dev_storage;
const device *pipew_dev = &pipew_dev_storage;
const device *port_dev = wordlist + 357;
const device *proc_dev = &proc_dev_storage;
const device *process_dev = &process_dev_storage;
const device *ptym_dev = wordlist + 327;
const device *random_dev = wordlist + 316;
const device *rawdrive_dev = wordlist + 345;
const device *registry_dev = &registry_dev_storage;
const device *sda_dev = wordlist + 248;
const device *sdb_dev = wordlist + 250;
const device *sdc_dev = wordlist + 294;
const device *sdd_dev = wordlist + 290;
const device *sde_dev = wordlist + 298;
const device *sdf_dev = wordlist + 251;
const device *sdg_dev = wordlist + 252;
const device *sdh_dev = wordlist + 284;
const device *sdi_dev = wordlist + 263;
const device *sdj_dev = wordlist + 258;
const device *sdk_dev = wordlist + 289;
const device *sdl_dev = wordlist + 309;
const device *sdm_dev = wordlist + 261;
const device *sdn_dev = wordlist + 274;
const device *sdo_dev = wordlist + 278;
const device *sdp_dev = wordlist + 300;
const device *sdq_dev = wordlist + 305;
const device *sdr_dev = wordlist + 293;
const device *sds_dev = wordlist + 275;
const device *sdt_dev = wordlist + 307;
const device *sdu_dev = wordlist + 287;
const device *sdv_dev = wordlist + 299;
const device *sdw_dev = wordlist + 286;
const device *sdx_dev = wordlist + 280;
const device *sdy_dev = wordlist + 266;
const device *sdz_dev = wordlist + 262;
const device *serial_dev = wordlist + 259;
const device *serial_dev1 = wordlist + 322;
const device *stream_dev = &stream_dev_storage;
const device *tape_dev = wordlist + 255;
const device *tcp_dev = &tcp_dev_storage;
const device *tty_dev = wordlist + 306;
const device *ttym_dev = wordlist + 320;
const device *ttys_dev = wordlist + 334;
const device *udp_dev = &udp_dev_storage;
const device *unix_dev = &unix_dev_storage;
const device *urandom_dev = wordlist + 326;
const device *windows_dev = wordlist + 325;
const device *zero_dev = wordlist + 310;

const device *unit_devices[] =
{
  wordlist + 241 /* cdrom */,
  wordlist + 259 /* serial */,
  wordlist + 320 /* ttym */,
  wordlist + 334 /* ttys */,
  wordlist + 214 /* floppy */,
  &tcp_dev_storage /* tcp */,
  wordlist + 345 /* rawdrive */,
  wordlist + 255 /* tape */
};

const device *uniq_devices[] = 
{
  wordlist + 131 /* bad */,
  &fs_dev_storage /* fs */,
  &process_dev_storage /* process */,
  &registry_dev_storage /* registry */,
  &proc_dev_storage /* proc */,
  wordlist + 222 /* fifo */,
  &pipew_dev_storage /* pipew */,
  &piper_dev_storage /* piper */,
  wordlist + 340 /* pipe */,
  wordlist + 246 /* mem */,
  wordlist + 288 /* kmem */,
  wordlist + 358 /* null */,
  wordlist + 310 /* zero */,
  wordlist + 357 /* port */,
  wordlist + 316 /* random */,
  wordlist + 326 /* urandom */,
  wordlist + 306 /* tty */,
  wordlist + 333 /* console */,
  wordlist + 327 /* ptym */,
  wordlist + 344 /* conout */,
  wordlist + 315 /* conin */,
  wordlist + 352 /* clipboard */,
  wordlist + 325 /* windows */,
  wordlist + 291 /* oss_dsp */,
  wordlist + 259 /* serial */
};

#ifdef __GNUC__
__inline
#endif
const struct device *
device::lookup (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = devhash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (strncasematch (str, s, len))
            return &wordlist[key];
        }
    }
  return 0;
}
void
device::parse (const char *s)
{
  size_t len = strlen (s);
  const device *dev = lookup (s, len);
  unsigned unit = 0;

  if (!dev || !*dev)
    {
      size_t prior_len = len;
      while (len-- > 0 && isdigit (s[len]))
	continue;
      if (++len < prior_len)
	{
	  dev = lookup (s, len);
	  if (!dev || (!dev->upper && !dev->devn == FH_TTY))
	    dev = NULL;
	  else
	    {
	      unsigned n = atoi (s + len);
	      if (dev->devn == FH_TTY)
		dev = ttys_dev;         // SIGH
	      if (n >= dev->lower && n <= dev->upper)
		unit = n;
	    }
	}
    }

  if (!dev || !*dev)
    *this = *fs_dev;
  else if (dev->devn == FH_TTY)
    tty_to_real_device ();
  else
    {
      *this = *dev;
      if (!setunit (unit))
	devn = 0;
    }
}

void
device::init ()
{
  /* nothing to do... yet */
}

void
device::parse (_major_t major, _minor_t minor)
{
  _dev_t dev = FHDEV (major, 0);

  devn = 0;

  unsigned i;
  for (i = 0; i < (sizeof (unit_devices) / sizeof (unit_devices[0])); i++)
    if (unit_devices[i]->devn == dev)
      {
	*this = *unit_devices[i];
	this->setunit (minor);
	goto out;
      }

  dev = FHDEV (major, minor);
  for (i = 0; i < (sizeof (uniq_devices) / sizeof (uniq_devices[0])); i++)
    if (uniq_devices[i]->devn == dev)
      {
	*this = *uniq_devices[i];
	break;
      }
  
out:
  if (!*this)
    devn = FHDEV (major, minor);
  return;
}

void
device::parse (_dev_t dev)
{
  parse (_major (dev), _minor (dev));
}

void
device::tty_to_real_device ()
{
  if (!real_tty_attached (myself))
    *this = myself->ctty < 0 ? *bad_dev : *console_dev;
  else
    {
      *this = *ttys_dev;
      setunit (myself->ctty);
    }
}
