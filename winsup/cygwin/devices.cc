
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
  {"/dev/inet/udp", FH_UDP, "", 0, 0, 0, 0};

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

const device dev_sda_storage =
{"/dev/sda", FH_SDA, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 0};

const device dev_sdb_storage =
{"/dev/sdb", FH_SDB, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 16};

const device dev_sdc_storage =
{"/dev/sdc", FH_SDC, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 32};

const device dev_sdd_storage =
{"/dev/sdd", FH_SDD, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 48};

const device dev_sde_storage =
{"/dev/sde", FH_SDE, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 64};

const device dev_sdf_storage =
{"/dev/sdf", FH_SDF, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 80};

const device dev_sdg_storage =
{"/dev/sdg", FH_SDG, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 96};

const device dev_sdh_storage =
{"/dev/sdh", FH_SDH, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 112};

const device dev_sdi_storage =
{"/dev/sdi", FH_SDI, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 128};

const device dev_sdj_storage =
{"/dev/sdj", FH_SDJ, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 144};

const device dev_sdk_storage =
{"/dev/sdk", FH_SDK, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 160};

const device dev_sdl_storage =
{"/dev/sdl", FH_SDL, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 176};

const device dev_sdm_storage =
{"/dev/sdm", FH_SDM, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 192};

const device dev_sdn_storage =
{"/dev/sdn", FH_SDN, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 208};

const device dev_sdo_storage =
{"/dev/sdo", FH_SDO, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 224};

const device dev_sdp_storage =
{"/dev/sdp", FH_SDP, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 240};

const device dev_sdq_storage =
{"/dev/sdq", FH_SDQ, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 256};

const device dev_sdr_storage =
{"/dev/sdr", FH_SDR, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 272};

const device dev_sds_storage =
{"/dev/sds", FH_SDS, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 288};

const device dev_sdt_storage =
{"/dev/sdt", FH_SDT, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 304};

const device dev_sdu_storage =
{"/dev/sdu", FH_SDU, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 320};

const device dev_sdv_storage =
{"/dev/sdv", FH_SDV, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 336};

const device dev_sdw_storage =
{"/dev/sdw", FH_SDW, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 352};

const device dev_sdx_storage =
{"/dev/sdx", FH_SDX, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 368};

const device dev_sdy_storage =
{"/dev/sdy", FH_SDY, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 384};

const device dev_sdz_storage =
{"/dev/sdz", FH_SDZ, "\\Device\\Harddisk%d\\Partition%d", 1, 15, 400};

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
          if (strncmp (KR_keyword, "/dev/sdz", 8) == 0)
            {
{
return &dev_sdz_storage;

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
            case 's':
              if (strncmp (KR_keyword, "/dev/sdy", 8) == 0)
                {
{
return &dev_sdy_storage;

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
          if (strncmp (KR_keyword, "/dev/sdx", 8) == 0)
            {
{
return &dev_sdx_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'w':
          if (strncmp (KR_keyword, "/dev/sdw", 8) == 0)
            {
{
return &dev_sdw_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'v':
          if (strncmp (KR_keyword, "/dev/sdv", 8) == 0)
            {
{
return &dev_sdv_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'u':
          if (strncmp (KR_keyword, "/dev/sdu", 8) == 0)
            {
{
return &dev_sdu_storage;

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
            case 's':
              if (strncmp (KR_keyword, "/dev/sdt", 8) == 0)
                {
{
return &dev_sdt_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
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
            default:
{
return	NULL;

}
            }
        case 's':
          if (strncmp (KR_keyword, "/dev/sds", 8) == 0)
            {
{
return &dev_sds_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'r':
          if (strncmp (KR_keyword, "/dev/sdr", 8) == 0)
            {
{
return &dev_sdr_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'q':
          if (strncmp (KR_keyword, "/dev/sdq", 8) == 0)
            {
{
return &dev_sdq_storage;

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
            case 's':
              if (strncmp (KR_keyword, "/dev/sdp", 8) == 0)
                {
{
return &dev_sdp_storage;

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
          if (strncmp (KR_keyword, "/dev/sdo", 8) == 0)
            {
{
return &dev_sdo_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'n':
          if (strncmp (KR_keyword, "/dev/sdn", 8) == 0)
            {
{
return &dev_sdn_storage;

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
            case 's':
              if (strncmp (KR_keyword, "/dev/sdm", 8) == 0)
                {
{
return &dev_sdm_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
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
          if (strncmp (KR_keyword, "/dev/sdl", 8) == 0)
            {
{
return &dev_sdl_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'k':
          if (strncmp (KR_keyword, "/dev/sdk", 8) == 0)
            {
{
return &dev_sdk_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'j':
          if (strncmp (KR_keyword, "/dev/sdj", 8) == 0)
            {
{
return &dev_sdj_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'i':
          if (strncmp (KR_keyword, "/dev/sdi", 8) == 0)
            {
{
return &dev_sdi_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'h':
          if (strncmp (KR_keyword, "/dev/sdh", 8) == 0)
            {
{
return &dev_sdh_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'g':
          if (strncmp (KR_keyword, "/dev/sdg", 8) == 0)
            {
{
return &dev_sdg_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'f':
          if (strncmp (KR_keyword, "/dev/sdf", 8) == 0)
            {
{
return &dev_sdf_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'e':
          if (strncmp (KR_keyword, "/dev/sde", 8) == 0)
            {
{
return &dev_sde_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'd':
          switch (KR_keyword [6])
            {
            case 'd':
              if (strncmp (KR_keyword, "/dev/sdd", 8) == 0)
                {
{
return &dev_sdd_storage;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'c':
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
            default:
{
return	NULL;

}
            }
        case 'c':
          if (strncmp (KR_keyword, "/dev/sdc", 8) == 0)
            {
{
return &dev_sdc_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'b':
          if (strncmp (KR_keyword, "/dev/sdb", 8) == 0)
            {
{
return &dev_sdb_storage;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'a':
          if (strncmp (KR_keyword, "/dev/sda", 8) == 0)
            {
{
return &dev_sda_storage;

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
	  if (!dev || (!dev->upper && dev->devn != FH_TTY))
	    dev = NULL;
	  else
	    {
	      unsigned n = atoi (s + len);
	      if (dev->devn == FH_TTY)
		dev = ttys_dev;         // SIGH
	      if (n >= dev->lower && n <= dev->upper)
		unit = n;
	      else
	        dev = NULL;
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


