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
#define bad_dev (&dev_bad_storage)

enum
  {
    TOTAL_KEYWORDS = 54,
    MIN_WORD_LENGTH = 5,
    MAX_WORD_LENGTH = 14,
    MIN_HASH_VALUE = 106,
    MAX_HASH_VALUE = 288
  };

/* maximum key range = 183, duplicates = 0 */

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
      289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
      289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
      289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
      289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
      289, 289, 289, 289, 289, 289, 289,   2, 289, 289,
      289, 289, 289, 289, 289, 289, 289, 289,  62, 289,
      289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
      289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
      289, 289, 289,  27, 289, 289, 289, 289, 289, 289,
      289, 289, 289, 289, 289, 289, 289,  29,  28,  12,
       45,  59,  32,  39,  24,  11,  19,  53,  55,   2,
       57,  35,  21,  54,  62,   9,  52,   8,  37,  10,
       16,  33,  43, 289, 289, 289, 289, 289
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
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/com", FH_SERIAL, "\\.\\com%d", 1, 99},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
    {"/dev/st", FH_TAPE, "\\Device\\Tape%d", 0, 127},
    {""}, {""},
    {"/dev/mem", FH_MEM, "\\dev\\mem", 0, 0, 0, 0},
    {""}, {""},
    {"/dev/scd", FH_CDROM, "\\Device\\CdRom%d", 0, 15},
    {""}, {""}, {""},
    {"/dev/sr", FH_CDROM, "\\Device\\CdRom%d", 0, 15},
    {"/dev/hdm", FH_SDM, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 191},
    {""}, {""}, {""},
    {"/dev/dsp", FH_OSS_DSP, "\\dev\\dsp", 0, 0, 0, 0},
    {"/dev/fd", FH_FLOPPY, "\\Device\\Floppy%d", 0, 15},
    {"/dev/hdu", FH_SDU, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 319},
    {"/dev/hds", FH_SDS, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 287},
    {"/dev/hdw", FH_SDW, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 351},
    {"/dev/hdi", FH_SDI, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 127},
    {"/dev/hdc", FH_SDC, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 31},
    {""}, {""}, {""},
    {"/dev/hdx", FH_SDX, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 367},
    {""}, {""},
    {"/dev/hdj", FH_SDJ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 143},
    {""},
    {"/dev/hdp", FH_SDP, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 239},
    {""},
    {"/dev/ptmx", FH_PTYM, "\\dev\\ptmx", 0, 0, 0, 0},
    {"/dev/hdh", FH_SDH, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 111},
    {""}, {""}, {""},
    {"/dev/hdb", FH_SDB, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 15},
    {"/dev/hda", FH_SDA, "\\Device\\Harddisk%d\\Partition%d", 1, 16, -1},
    {""},
    {":bad:", FH_BAD, ":bad:", 0, 0, 0, 0},
    {"/dev/hdf", FH_SDF, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 79},
    {"/dev/hdy", FH_SDY, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 383},
    {""},
    {"/dev/hdo", FH_SDO, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 223},
    {""},
    {"/dev/hdv", FH_SDV, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 335},
    {""},
    {"/dev/hdg", FH_SDG, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 95},
    {""}, {""},
    {"/dev/fifo", FH_FIFO, "\\dev\\fifo", 0, 0, 0, 0},
    {"/dev/hdz", FH_SDZ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 399},
    {"/dev/pipe", FH_PIPE, "\\dev\\pipe", 0, 0, 0, 0},
    {"/dev/hdd", FH_SDD, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 47},
    {""}, {""},
    {"/dev/kmem", FH_KMEM, "\\dev\\mem", 0, 0, 0, 0},
    {"/dev/nst", FH_NTAPE, "\\Device\\Tape%d", 0, 127},
    {""}, {""},
    {"/dev/hdt", FH_SDT, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 303},
    {"/dev/hdk", FH_SDK, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 159},
    {"/dev/hdq", FH_SDQ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 255  /* FIXME 8 bit lunacy */},
    {"/dev/hdl", FH_SDL, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 175},
    {""},
    {"/dev/hdn", FH_SDN, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 207},
    {""},
    {"/dev/hde", FH_SDE, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 63},
    {""}, {""},
    {"/dev/hdr", FH_SDR, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 271},
    {""},
    {"/dev/clipboard", FH_CLIPBOARD, "\\dev\\clipboard", 0, 0, 0, 0},
    {""}, {""}, {""},
    {"/dev/tty", FH_TTY, "\\dev\\tty", 0, 0, 0, 0},
    {""}, {""},
    {"/dev/ttym", FH_TTYM, "\\dev\\ttym", 0, 255, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/ttys", FH_TTYS, "\\dev\\tty%d", 0, 255, 0, 0},
    {""}, {""},
    {"/dev/conout", FH_CONOUT, "conout", 0, 0, 0, 0},
    {""},
    {"/dev/console", FH_CONSOLE, "\\dev\\console", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/windows", FH_WINDOWS, "\\dev\\windows", 0, 0, 0, 0},
    {""}, {""},
    {"/dev/ttyS", FH_SERIAL, "\\.\\com%d", 0, 99, -1},
    {""}, {""}, {""}, {""}, {""},
    {"/dev/port", FH_PORT, "\\dev\\port", 0, 0, 0, 0},
    {""}, {""},
    {"/dev/conin", FH_CONIN, "conin", 0, 0, 0, 0},
    {""},
    {"/dev/null", FH_NULL, "nul", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""},
    {"/dev/zero", FH_ZERO, "\\dev\\zero", 0, 0, 0, 0},
    {""}, {""}, {""}, {""},
    {"/dev/urandom", FH_URANDOM, "\\dev\\urandom", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/rawdrive", FH_RAWDRIVE, "\\DosDevices\\%c:", 0, 0, 0, 0},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {"/dev/random", FH_RANDOM, "\\dev\\random", 0, 0, 0, 0}
  };

const device dev_cygdrive_storage =
  {"/cygdrive", FH_CYGDRIVE, "/cygdrive", 0, 0, 0, 0};

const device dev_fs_storage =
  {"", FH_FS, "", 0, 0, 0, 0};

const device dev_proc_storage =
  {"", FH_PROC, "", 0, 0, 0, 0};

const device dev_registry_storage =
  {"", FH_REGISTRY, "", 0, 0, 0, 0};

const device dev_process_storage =
  {"", FH_PROCESS, "", 0, 0, 0, 0};

const device dev_tcp_storage =
  {"/dev/inet/tcp", FH_TCP, "", 0, 0, 0, 0};

const device dev_udp_storage =
  {"/dev/inet/udp", FH_UCP, "", 0, 0, 0, 0};

const device dev_icmp_storage =
  {"/dev/inet/icmp", FH_ICMP, "", 0, 0, 0, 0};

const device dev_unix_storage =
  {"/dev/inet/unix", FH_UNIX, "", 0, 0, 0, 0};

const device dev_stream_storage =
  {"/dev/inet/stream", FH_STREAM, "", 0, 0, 0, 0};

const device dev_dgram_storage =
  {"/dev/inet/dgram", FH_DGRAM, "", 0, 0, 0, 0};

const device dev_piper_storage =
  {"", FH_PIPER, "", 0, 0, 0, 0};

const device dev_pipew_storage =
  {"", FH_PIPEW, "", 0, 0, 0, 0};

const device dev_fs =
  {"", FH_FS, "", 0, 0, 0, 0};
const device dev_bad_storage = wordlist[157];
const device dev_cdrom_storage = wordlist[123];
const device dev_cdrom1_storage = wordlist[127];
const device dev_clipboard_storage = wordlist[190];
const device dev_conin_storage = wordlist[231];
const device dev_conout_storage = wordlist[207];
const device dev_console_storage = wordlist[209];
const device dev_fifo_storage = wordlist[168];
const device dev_floppy_storage = wordlist[133];
const device dev_kmem_storage = wordlist[174];
const device dev_mem_storage = wordlist[120];
const device dev_ntape_storage = wordlist[175];
const device dev_null_storage = wordlist[233];
const device dev_oss_dsp_storage = wordlist[132];
const device dev_pipe_storage = wordlist[170];
const device dev_port_storage = wordlist[228];
const device dev_ptym_storage = wordlist[149];
const device dev_random_storage = wordlist[288];
const device dev_rawdrive_storage = wordlist[270];
const device dev_sda_storage = wordlist[155];
const device dev_sdb_storage = wordlist[154];
const device dev_sdc_storage = wordlist[138];
const device dev_sdd_storage = wordlist[171];
const device dev_sde_storage = wordlist[185];
const device dev_sdf_storage = wordlist[158];
const device dev_sdg_storage = wordlist[165];
const device dev_sdh_storage = wordlist[150];
const device dev_sdi_storage = wordlist[137];
const device dev_sdj_storage = wordlist[145];
const device dev_sdk_storage = wordlist[179];
const device dev_sdl_storage = wordlist[181];
const device dev_sdm_storage = wordlist[128];
const device dev_sdn_storage = wordlist[183];
const device dev_sdo_storage = wordlist[161];
const device dev_sdp_storage = wordlist[147];
const device dev_sdq_storage = wordlist[180];
const device dev_sdr_storage = wordlist[188];
const device dev_sds_storage = wordlist[135];
const device dev_sdt_storage = wordlist[178];
const device dev_sdu_storage = wordlist[134];
const device dev_sdv_storage = wordlist[163];
const device dev_sdw_storage = wordlist[136];
const device dev_sdx_storage = wordlist[142];
const device dev_sdy_storage = wordlist[159];
const device dev_sdz_storage = wordlist[169];
const device dev_serial_storage = wordlist[106];
const device dev_serial1_storage = wordlist[222];
const device dev_tape_storage = wordlist[117];
const device dev_tty_storage = wordlist[194];
const device dev_ttym_storage = wordlist[197];
const device dev_ttys_storage = wordlist[204];
const device dev_urandom_storage = wordlist[262];
const device dev_windows_storage = wordlist[219];
const device dev_zero_storage = wordlist[257];

const device *unit_devices[] =
{
  &wordlist[123] /* cdrom */,
  &wordlist[106] /* serial */,
  &wordlist[197] /* ttym */,
  &wordlist[204] /* ttys */,
  &wordlist[133] /* floppy */,
  &dev_tcp_storage /* tcp */,
  &wordlist[270] /* rawdrive */,
  &wordlist[117] /* tape */
};

const device *uniq_devices[] = 
{
  &wordlist[157] /* bad */,
  &dev_fs_storage /* fs */,
  &dev_process_storage /* process */,
  &dev_registry_storage /* registry */,
  &dev_proc_storage /* proc */,
  &wordlist[168] /* fifo */,
  &dev_pipew_storage /* pipew */,
  &dev_piper_storage /* piper */,
  &wordlist[170] /* pipe */,
  &wordlist[120] /* mem */,
  &wordlist[174] /* kmem */,
  &wordlist[233] /* null */,
  &wordlist[257] /* zero */,
  &wordlist[228] /* port */,
  &wordlist[288] /* random */,
  &wordlist[262] /* urandom */,
  &wordlist[194] /* tty */,
  &wordlist[209] /* console */,
  &wordlist[149] /* ptym */,
  &wordlist[207] /* conout */,
  &wordlist[231] /* conin */,
  &wordlist[190] /* clipboard */,
  &wordlist[219] /* windows */,
  &wordlist[132] /* oss_dsp */,
  &wordlist[106] /* serial */
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
