/* fhandler_dev_mixer: code to emulate OSS sound model /dev/mixer

   Written by Takashi Yano <takashi.yano@nifty.ne.jp>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/soundcard.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include <mmddk.h>

ssize_t
fhandler_dev_mixer::write (const void *ptr, size_t len)
{
  set_errno (EINVAL);
  return -1;
}

void
fhandler_dev_mixer::read (void *ptr, size_t& len)
{
  len = -1;
  set_errno (EINVAL);
}

int
fhandler_dev_mixer::open (int flags, mode_t)
{
  int ret = -1, err = 0;
  switch (flags & O_ACCMODE)
    {
    case O_RDWR:
    case O_WRONLY:
    case O_RDONLY:
      if (waveInGetNumDevs () == 0 && waveOutGetNumDevs () == 0)
	err = ENXIO;
      break;
    default:
      err = EINVAL;
    }

  if (err)
    set_errno (err);
  else
    {
      ret = open_null (flags);
      rec_source = SOUND_MIXER_MIC;
    }

  return ret;
}

static DWORD
volume_oss_to_winmm (int vol_oss)
{
  unsigned int vol_l, vol_r;
  DWORD vol_winmm;

  vol_l = ((unsigned int) vol_oss) & 0xff;
  vol_r = ((unsigned int) vol_oss) >> 8;
  vol_l = min ((vol_l * 65535 + 50) / 100, 65535);
  vol_r = min ((vol_r * 65535 + 50) / 100, 65535);
  vol_winmm = (vol_r << 16) | vol_l;

  return vol_winmm;
}

static int
volume_winmm_to_oss (DWORD vol_winmm)
{
  int vol_l, vol_r;

  vol_l = vol_winmm & 0xffff;
  vol_r = vol_winmm >> 16;
  vol_l = min ((vol_l * 100 + 32768) / 65535, 100);
  vol_r = min ((vol_r * 100 + 32768) / 65535, 100);
  return (vol_r << 8) | vol_l;
}

int
fhandler_dev_mixer::ioctl (unsigned int cmd, void *buf)
{
  int ret = 0;
  DWORD id, flag;
  DWORD vol;
  WAVEOUTCAPS woc;
  switch (cmd)
    {
    case SOUND_MIXER_READ_DEVMASK:
      *(int *) buf = SOUND_MASK_VOLUME | SOUND_MASK_MIC | SOUND_MASK_LINE;
      break;
    case SOUND_MIXER_READ_RECMASK:
      *(int *) buf = SOUND_MASK_MIC | SOUND_MASK_LINE;
      break;
    case SOUND_MIXER_READ_STEREODEVS:
      *(int *) buf = SOUND_MASK_VOLUME | SOUND_MASK_LINE;
      break;
    case SOUND_MIXER_READ_CAPS:
      *(int *) buf = SOUND_CAP_EXCL_INPUT;
      break;
    case MIXER_WRITE (SOUND_MIXER_RECSRC):
      /* Dummy implementation */
      if (*(int *) buf == 0 || ((*(int *) buf) & SOUND_MASK_MIC))
	rec_source = SOUND_MIXER_MIC;
      else if ((*(int *) buf) & SOUND_MASK_LINE)
	rec_source = SOUND_MIXER_LINE;
      break;
    case MIXER_READ (SOUND_MIXER_RECSRC):
      /* Dummy implementation */
      *(int *) buf = 1 << rec_source;
      break;
    case MIXER_WRITE (SOUND_MIXER_VOLUME):
      waveOutMessage ((HWAVEOUT)WAVE_MAPPER, DRVM_MAPPER_PREFERRED_GET,
		      (DWORD_PTR)&id, (DWORD_PTR)&flag);
      waveOutGetDevCaps ((UINT)id, &woc, sizeof (woc));
      vol = volume_oss_to_winmm (*(int *) buf);
      if (!(woc.dwSupport & WAVECAPS_LRVOLUME))
	vol = max(vol & 0xffff, (vol >> 16) & 0xffff);
      if (waveOutSetVolume ((HWAVEOUT)WAVE_MAPPER, vol) != MMSYSERR_NOERROR)
	{
	  set_errno (EINVAL);
	  ret = -1;
	}
      break;
    case MIXER_READ (SOUND_MIXER_VOLUME):
      waveOutMessage ((HWAVEOUT)WAVE_MAPPER, DRVM_MAPPER_PREFERRED_GET,
		      (DWORD_PTR)&id, (DWORD_PTR)&flag);
      waveOutGetDevCaps ((UINT)id, &woc, sizeof (woc));
      if (waveOutGetVolume ((HWAVEOUT)WAVE_MAPPER, &vol) != MMSYSERR_NOERROR)
	{
	  set_errno (EINVAL);
	  ret = -1;
	  break;
	}
      if (!(woc.dwSupport & WAVECAPS_LRVOLUME))
	vol |= (vol & 0xffff) << 16;
      *(int *) buf = volume_winmm_to_oss (vol);
      break;
    default:
      for (int i = 0; i < SOUND_MIXER_NRDEVICES; i++)
	{
	  if (cmd == (unsigned int) MIXER_WRITE (i))
	    goto out;
	  if (cmd == (unsigned int) MIXER_READ (i))
	    {
	      *(int *) buf = 0;
	      goto out;
	    }
	}
      set_errno (EINVAL);
      ret = -1;
      break;
    }
out:
  return ret;
}
