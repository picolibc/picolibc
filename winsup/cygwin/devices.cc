
#include "winsup.h"
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "devices.h"
#include "sys/cygwin.h"
#include "tty.h"
#include "pinfo.h"
typedef const device *KR_device_t;


static KR_device_t KR_find_keyword (const char *KR_keyword, int KR_length);



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

const device dev_tty_storage =
{"/dev/tty", FH_TTY, "\\dev\\tty", 0, 0, 0, 0};

const device dev_ttys_storage =
{"/dev/ttys", FH_TTYS, "\\dev\\tty%d", 0, 255, 0, 0};

const device dev_console_storage =
{"/dev/console", FH_CONSOLE, "\\dev\\console", 0, 0, 0, 0};

const device dev_ttym_storage =
{"/dev/ttym", FH_TTYM, "\\dev\\ttym", 0, 255, 0, 0};

const device dev_ptym_storage =
{"/dev/ptmx", FH_PTYM, "\\dev\\ptmx", 0, 0, 0, 0};

const device dev_windows_storage =
{"/dev/windows", FH_WINDOWS, "\\dev\\windows", 0, 0, 0, 0};

const device dev_oss_dsp_storage =
{"/dev/dsp", FH_OSS_DSP, "\\dev\\dsp", 0, 0, 0, 0};

const device dev_conin_storage =
{"/dev/conin", FH_CONIN, "conin", 0, 0, 0, 0};

const device dev_conout_storage =
{"/dev/conout", FH_CONOUT, "conout", 0, 0, 0, 0};

const device dev_null_storage =
{"/dev/null", FH_NULL, "nul", 0, 0, 0, 0};

const device dev_zero_storage =
{"/dev/zero", FH_ZERO, "\\dev\\zero", 0, 0, 0, 0};

const device dev_random_storage =
{"/dev/random", FH_RANDOM, "\\dev\\random", 0, 0, 0, 0};

const device dev_urandom_storage =
{"/dev/urandom", FH_URANDOM, "\\dev\\urandom", 0, 0, 0, 0};

const device dev_mem_storage =
{"/dev/mem", FH_MEM, "\\dev\\mem", 0, 0, 0, 0};

const device dev_kmem_storage =
{"/dev/kmem", FH_KMEM, "\\dev\\mem", 0, 0, 0, 0};

const device dev_clipboard_storage =
{"/dev/clipboard", FH_CLIPBOARD, "\\dev\\clipboard", 0, 0, 0, 0};

const device dev_port_storage =
{"/dev/port", FH_PORT, "\\dev\\port", 0, 0, 0, 0};

const device dev_serial_storage =
{"/dev/com", FH_SERIAL, "\\.\\com%d", 1, 99};

const device dev_ttyS_storage =
{"/dev/ttyS", FH_SERIAL, "\\.\\com%d", 0, 99, -1};

const device dev_pipe_storage =
{"/dev/pipe", FH_PIPE, "\\dev\\pipe", 0, 0, 0, 0};

const device dev_fifo_storage =
{"/dev/fifo", FH_FIFO, "\\dev\\fifo", 0, 0, 0, 0};

const device dev_tape_storage =
{"/dev/st", FH_TAPE, "\\Device\\Tape%d", 0, 127};

const device dev_nst_storage =
{"/dev/nst", FH_NTAPE, "\\Device\\Tape%d", 0, 127};

const device dev_floppy_storage =
{"/dev/fd", FH_FLOPPY, "\\Device\\Floppy%d", 0, 15};

const device dev_cdrom_storage =
{"/dev/scd", FH_CDROM, "\\Device\\CdRom%d", 0, 15};

const device dev_sr_storage =
{"/dev/sr", FH_CDROM, "\\Device\\CdRom%d", 0, 15};

const device dev_hda_storage =
{"/dev/hda", FH_SDA, "\\Device\\Harddisk%d\\Partition%d", 1, 16, -1};

const device dev_hdb_storage =
{"/dev/hdb", FH_SDB, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 15};

const device dev_hdc_storage =
{"/dev/hdc", FH_SDC, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 31};

const device dev_hdd_storage =
{"/dev/hdd", FH_SDD, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 47};

const device dev_hde_storage =
{"/dev/hde", FH_SDE, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 63};

const device dev_hdf_storage =
{"/dev/hdf", FH_SDF, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 79};

const device dev_hdg_storage =
{"/dev/hdg", FH_SDG, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 95};

const device dev_hdh_storage =
{"/dev/hdh", FH_SDH, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 111};

const device dev_hdi_storage =
{"/dev/hdi", FH_SDI, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 127};

const device dev_hdj_storage =
{"/dev/hdj", FH_SDJ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 143};

const device dev_hdk_storage =
{"/dev/hdk", FH_SDK, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 159};

const device dev_hdl_storage =
{"/dev/hdl", FH_SDL, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 175};

const device dev_hdm_storage =
{"/dev/hdm", FH_SDM, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 191};

const device dev_hdn_storage =
{"/dev/hdn", FH_SDN, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 207};

const device dev_hdo_storage =
{"/dev/hdo", FH_SDO, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 223};

const device dev_hdp_storage =
{"/dev/hdp", FH_SDP, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 239};

const device dev_hdq_storage =
{"/dev/hdq", FH_SDQ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 255  /* FIXME 8 bit lunacy */};

const device dev_hdr_storage =
{"/dev/hdr", FH_SDR, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 271};

const device dev_hds_storage =
{"/dev/hds", FH_SDS, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 287};

const device dev_hdt_storage =
{"/dev/hdt", FH_SDT, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 303};

const device dev_hdu_storage =
{"/dev/hdu", FH_SDU, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 319};

const device dev_hdv_storage =
{"/dev/hdv", FH_SDV, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 335};

const device dev_hdw_storage =
{"/dev/hdw", FH_SDW, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 351};

const device dev_hdx_storage =
{"/dev/hdx", FH_SDX, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 367};

const device dev_hdy_storage =
{"/dev/hdy", FH_SDY, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 383};

const device dev_hdz_storage =
{"/dev/hdz", FH_SDZ, "\\Device\\Harddisk%d\\Partition%d", 1, 16, 399};

const device dev_rawdrive_storage =
{"/dev/rawdrive", FH_RAWDRIVE, "\\DosDevices\\%c:", 0, 0, 0, 0};

const device dev_bad_storage =
{":bad:", FH_BAD, ":bad:", 0, 0, 0, 0};
#define bad_dev (&dev_bad_storage)

const device *unit_devices[] =
{
  &dev_cdrom_storage,
  &dev_serial_storage,
  &dev_ttym_storage,
  &dev_ttys_storage,
  &dev_floppy_storage,
  &dev_tcp_storage,
  &dev_rawdrive_storage,
  &dev_tape_storage
};

const device *uniq_devices[] = 
{
  &dev_bad_storage,
  &dev_fs_storage,
  &dev_process_storage,
  &dev_registry_storage,
  &dev_proc_storage,
  &dev_fifo_storage,
  &dev_pipew_storage,
  &dev_piper_storage,
  &dev_pipe_storage,
  &dev_mem_storage,
  &dev_kmem_storage,
  &dev_null_storage,
  &dev_zero_storage,
  &dev_port_storage,
  &dev_random_storage,
  &dev_urandom_storage,
  &dev_tty_storage,
  &dev_console_storage,
  &dev_ptym_storage,
  &dev_conout_storage,
  &dev_conin_storage,
  &dev_clipboard_storage,
  &dev_windows_storage,
  &dev_oss_dsp_storage,
  &dev_serial_storage
};


static KR_device_t KR_find_keyword (const char *KR_keyword, int KR_length)
{

  switch (KR_length)
    {
    case 7:
      switch (KR_keyword [6])
        {
        case 't':
          if (strncmp (KR_keyword, "/dev/st", 7) == 0)
            {
{
return &dev_tape_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'r':
          if (strncmp (KR_keyword, "/dev/sr", 7) == 0)
            {
{
return &dev_sr_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'd':
          if (strncmp (KR_keyword, "/dev/fd", 7) == 0)
            {
{
return &dev_floppy_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 8:
      switch (KR_keyword [7])
        {
        case 'z':
          if (strncmp (KR_keyword, "/dev/hdz", 8) == 0)
            {
{
return &dev_hdz_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'y':
          switch (KR_keyword [5])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/tty", 8) == 0)
                {
{
return &dev_tty_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/hdy", 8) == 0)
                {
{
return &dev_hdy_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'x':
          if (strncmp (KR_keyword, "/dev/hdx", 8) == 0)
            {
{
return &dev_hdx_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'w':
          if (strncmp (KR_keyword, "/dev/hdw", 8) == 0)
            {
{
return &dev_hdw_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'v':
          if (strncmp (KR_keyword, "/dev/hdv", 8) == 0)
            {
{
return &dev_hdv_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'u':
          if (strncmp (KR_keyword, "/dev/hdu", 8) == 0)
            {
{
return &dev_hdu_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 't':
          switch (KR_keyword [5])
            {
            case 'n':
              if (strncmp (KR_keyword, "/dev/nst", 8) == 0)
                {
{
return &dev_nst_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/hdt", 8) == 0)
                {
{
return &dev_hdt_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 's':
          if (strncmp (KR_keyword, "/dev/hds", 8) == 0)
            {
{
return &dev_hds_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'r':
          if (strncmp (KR_keyword, "/dev/hdr", 8) == 0)
            {
{
return &dev_hdr_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'q':
          if (strncmp (KR_keyword, "/dev/hdq", 8) == 0)
            {
{
return &dev_hdq_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'p':
          switch (KR_keyword [5])
            {
            case 'h':
              if (strncmp (KR_keyword, "/dev/hdp", 8) == 0)
                {
{
return &dev_hdp_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/dsp", 8) == 0)
                {
{
return &dev_oss_dsp_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'o':
          if (strncmp (KR_keyword, "/dev/hdo", 8) == 0)
            {
{
return &dev_hdo_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'n':
          if (strncmp (KR_keyword, "/dev/hdn", 8) == 0)
            {
{
return &dev_hdn_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'm':
          switch (KR_keyword [5])
            {
            case 'm':
              if (strncmp (KR_keyword, "/dev/mem", 8) == 0)
                {
{
return &dev_mem_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/hdm", 8) == 0)
                {
{
return &dev_hdm_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/com", 8) == 0)
                {
{
return &dev_serial_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'l':
          if (strncmp (KR_keyword, "/dev/hdl", 8) == 0)
            {
{
return &dev_hdl_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'k':
          if (strncmp (KR_keyword, "/dev/hdk", 8) == 0)
            {
{
return &dev_hdk_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'j':
          if (strncmp (KR_keyword, "/dev/hdj", 8) == 0)
            {
{
return &dev_hdj_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'i':
          if (strncmp (KR_keyword, "/dev/hdi", 8) == 0)
            {
{
return &dev_hdi_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'h':
          if (strncmp (KR_keyword, "/dev/hdh", 8) == 0)
            {
{
return &dev_hdh_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'g':
          if (strncmp (KR_keyword, "/dev/hdg", 8) == 0)
            {
{
return &dev_hdg_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'f':
          if (strncmp (KR_keyword, "/dev/hdf", 8) == 0)
            {
{
return &dev_hdf_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'e':
          if (strncmp (KR_keyword, "/dev/hde", 8) == 0)
            {
{
return &dev_hde_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'd':
          switch (KR_keyword [5])
            {
            case 's':
              if (strncmp (KR_keyword, "/dev/scd", 8) == 0)
                {
{
return &dev_cdrom_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/hdd", 8) == 0)
                {
{
return &dev_hdd_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'c':
          if (strncmp (KR_keyword, "/dev/hdc", 8) == 0)
            {
{
return &dev_hdc_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'b':
          if (strncmp (KR_keyword, "/dev/hdb", 8) == 0)
            {
{
return &dev_hdb_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'a':
          if (strncmp (KR_keyword, "/dev/hda", 8) == 0)
            {
{
return &dev_hda_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 9:
      switch (KR_keyword [8])
        {
        case 'x':
          if (strncmp (KR_keyword, "/dev/ptmx", 9) == 0)
            {
{
return &dev_ptym_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 't':
          if (strncmp (KR_keyword, "/dev/port", 9) == 0)
            {
{
return &dev_port_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 's':
          if (strncmp (KR_keyword, "/dev/ttys", 9) == 0)
            {
{
return &dev_ttys_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'o':
          switch (KR_keyword [5])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/zero", 9) == 0)
                {
{
return &dev_zero_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/fifo", 9) == 0)
                {
{
return &dev_fifo_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'm':
          switch (KR_keyword [5])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/ttym", 9) == 0)
                {
{
return &dev_ttym_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/kmem", 9) == 0)
                {
{
return &dev_kmem_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'l':
          if (strncmp (KR_keyword, "/dev/null", 9) == 0)
            {
{
return &dev_null_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'e':
          if (strncmp (KR_keyword, "/dev/pipe", 9) == 0)
            {
{
return &dev_pipe_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'S':
          if (strncmp (KR_keyword, "/dev/ttyS", 9) == 0)
            {
{
return &dev_ttyS_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 10:
          if (strncmp (KR_keyword, "/dev/conin", 10) == 0)
            {
{
return &dev_conin_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
    case 11:
      switch (KR_keyword [5])
        {
        case 'r':
          if (strncmp (KR_keyword, "/dev/random", 11) == 0)
            {
{
return &dev_random_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'c':
          if (strncmp (KR_keyword, "/dev/conout", 11) == 0)
            {
{
return &dev_conout_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 12:
      switch (KR_keyword [5])
        {
        case 'w':
          if (strncmp (KR_keyword, "/dev/windows", 12) == 0)
            {
{
return &dev_windows_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'u':
          if (strncmp (KR_keyword, "/dev/urandom", 12) == 0)
            {
{
return &dev_urandom_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'c':
          if (strncmp (KR_keyword, "/dev/console", 12) == 0)
            {
{
return &dev_console_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 13:
          if (strncmp (KR_keyword, "/dev/rawdrive", 13) == 0)
            {
{
return &dev_rawdrive_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
    case 14:
          if (strncmp (KR_keyword, "/dev/clipboard", 14) == 0)
            {
{
return &dev_clipboard_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
    default:
{
return	NULL;

}
    }
}






void
device::parse (const char *s)
{
  size_t len = strlen (s);
  const device *dev = KR_find_keyword (s, len);
  unsigned unit = 0;

  if (!dev || !*dev)
    {
      size_t prior_len = len;
      while (len-- > 0 && isdigit (s[len]))
	continue;
      if (++len < prior_len)
	{
	  dev = KR_find_keyword (s, len);
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


