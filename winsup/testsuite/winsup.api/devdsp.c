/* devdsp.c: Device tests for /dev/dsp

   Copyright 2004 Red Hat, Inc

   Written by Gerd Spalink (Gerd.Spalink@t-online.de)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Conventions used here:
   We use the libltp framework
   1. Any unexpected behaviour leads to an exit with nonzero exit status
   2. Unexpected behaviour from /dev/dsp results in an exit status of TFAIL
   3. Unexpected behaviour from OS (malloc, fork, waitpid...) result
      in an exit status of TBROK */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/soundcard.h>
#include <math.h>
#include <errno.h>
#include "test.h" /* use libltp framework */

/* Controls if a child can open the device after the parent */
#define CHILD_EXPECT 0 /* 0 or 1 */

static const char wavfile_okay[] =
  {
#include "devdsp_okay.h" /* a sound sample */
  };

/* Globals required by libltp */
const char *TCID = "devdsp";   /* set test case identifier */
int TST_TOTAL = 37;

/* Prototypes */
void sinegen (void *wave, int rate, int bits, int len, int stride);
void sinegenw (int freq, int samprate, short *value, int len, int stride);
void sinegenb (int freq, int samprate, unsigned char *value, int len,
	       int stride);
void playtest (int fd, int rate, int stereo, int bits);
void rectest (int fd, int rate, int stereo, int bits);
void rwtest (int fd, int rate, int stereo, int bits);
void setpars (int fd, int rate, int stereo, int bits);
void forkplaytest (void);
void forkrectest (void);
void recordingtest (void);
void playbacktest (void);
void monitortest (void);
void ioctltest (void);
void abortplaytest (void);
void playwavtest (void);
void syncwithchild (pid_t pid, int expected_exit_status);
void cleanup (void);
void dup_test (void);

static int expect_child_failure = 0;

/* Sampling rates we want to test */
static const int rates[] = { 44100, 22050, 8000 };

/* Combinations of stereo/bits we want to test */
struct sb
{
  int stereo;
  int bits;
};
static const struct sb sblut[] = { {0, 8}, {0, 16}, {1, 8}, {1, 16} };

int
main (int argc, char *argv[])
{
  /*  tst_brkm(TBROK, cleanup, "see if it breaks all right"); */
  ioctltest ();
  playbacktest ();
  recordingtest ();
  monitortest ();
  forkplaytest ();
  forkrectest ();
  abortplaytest ();
  playwavtest ();
  dup_test ();
  tst_exit ();
  /* NOTREACHED */
  return 0;
}

/* test some extra ioctls */
void
ioctltest (void)
{
  int audio1;
  int ioctl_par;
  int channels;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  audio1 = open ("/dev/dsp", O_WRONLY);
  if (audio1 < 0)
    {
      tst_brkm (TFAIL, cleanup, "open W: %s", strerror (errno));
    }
  setpars (audio1, 44100, 1, 16);

  channels = ioctl_par = 1;
  while (ioctl (audio1, SNDCTL_DSP_CHANNELS, &ioctl_par) == 0)
    {
      channels++;
      ioctl_par = channels;
    }
  if (channels == ioctl_par)
    tst_resm (TFAIL, "Max channels=%d failed", ioctl_par);
  else
    tst_resm (TPASS, "Max channels=%d failed, OK=%d", channels, ioctl_par);

  /* Note: block size may depend on parameters */
  if (ioctl (audio1, SNDCTL_DSP_GETBLKSIZE, &ioctl_par) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl GETBLKSIZE: %s", strerror (errno));
    }
  tst_resm (TPASS, "ioctl get buffer size=%d", ioctl_par);
  if (ioctl (audio1, SNDCTL_DSP_GETFMTS, &ioctl_par) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl GETFMTS: %s", strerror (errno));
    }
  tst_resm (TPASS, "ioctl get formats=%08x", ioctl_par);
  if (ioctl (audio1, SNDCTL_DSP_GETCAPS, &ioctl_par) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl GETCAPS: %s", strerror (errno));
    }
  tst_resm (TPASS, "ioctl get caps=%08x", ioctl_par);
  if (close (audio1) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
}

/* test write / play */
void
playbacktest (void)
{
  int audio1, audio2;
  int rate, k;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  audio1 = open ("/dev/dsp", O_WRONLY);
  if (audio1 < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp W: %s",
		strerror (errno));
    }
  audio2 = open ("/dev/dsp", O_WRONLY);
  if (audio2 >= 0)
    {
      tst_brkm (TFAIL, cleanup,
		"Second open /dev/dsp W succeeded, but is expected to fail");
    }
  else if (errno != EBUSY)
    {
      tst_brkm (TFAIL, cleanup, "Expected EBUSY here, exit: %s",
		strerror (errno));
    }
  for (rate = 0; rate < sizeof (rates) / sizeof (int); rate++)
    for (k = 0; k < sizeof (sblut) / sizeof (struct sb); k++)
      {
	playtest (audio1, rates[rate], sblut[k].stereo, sblut[k].bits);
	tst_resm (TPASS, "Play bits=%2d stereo=%d rate=%5d",
		  sblut[k].bits, sblut[k].stereo, rates[rate]);
      }
  if (close (audio1) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
}

/* test read / record */
void
recordingtest (void)
{
  int audio1, audio2;
  int rate, k;

  /* test read / record */
  tst_resm (TINFO, "Running %s", __FUNCTION__);
  audio1 = open ("/dev/dsp", O_RDONLY);
  if (audio1 < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp R: %s",
		strerror (errno));
    }
  audio2 = open ("/dev/dsp", O_RDONLY);
  if (audio2 >= 0)
    {
      tst_brkm (TFAIL, cleanup,
		"Second open /dev/dsp R succeeded, but is expected to fail");
    }
  else if (errno != EBUSY)
    {
      tst_brkm (TFAIL, cleanup, "Expected EBUSY here, exit: %s",
		strerror (errno));
    }
  for (rate = 0; rate < sizeof (rates) / sizeof (int); rate++)
    for (k = 0; k < sizeof (sblut) / sizeof (struct sb); k++)
      {
	rectest (audio1, rates[rate], sblut[k].stereo, sblut[k].bits);
	tst_resm (TPASS, "Record bits=%2d stereo=%d rate=%5d",
		  sblut[k].bits, sblut[k].stereo, rates[rate]);
      }
  if (close (audio1) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
}

/* simultaneous read/write */
void
monitortest (void)
{
  int fd;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  fd = open ("/dev/dsp", O_RDWR);
  if (fd < 0)
    {
      tst_brkm (TFAIL, cleanup, "open RW: %s", strerror (errno));
    }
  rwtest (fd, 44100, 1, 16);
  tst_resm (TPASS, "Record+Play rate=44100, stereo, 16 bits");
  if (close (fd) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
}

void
forkrectest (void)
{
  int pid;
  int fd;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  fd = open ("/dev/dsp", O_RDONLY);
  if (fd < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp R: %s",
		strerror (errno));
    }
  pid = fork ();
  if (pid < 0)
    {
      tst_brkm (TBROK, cleanup, "Fork failed: %s", strerror (errno));
    }
  if (pid)
    {
      tst_resm (TINFO, "forked, child PID=%d", pid);
      syncwithchild (pid, 0);
      tst_resm (TINFO, "parent records..");
      rectest (fd, 22050, 1, 16);
      tst_resm (TINFO, "parent done");
    }
  else
    {				/* child */
      tst_resm (TINFO, "child records..");
      rectest (fd, 44100, 1, 16);
      tst_resm (TINFO, "child done");
      fflush (stdout);
      exit (0);			/* implicit close */
    }
  tst_resm (TPASS, "child records after fork");
  /* fork again, but now we have done a read before,
   * so the child is expected to fail
   */
  pid = fork ();
  if (pid < 0)
    {
      tst_brkm (TBROK, cleanup, "Fork failed: %s", strerror (errno));
    }
  if (pid)
    {
      tst_resm (TINFO, "forked, child PID=%d", pid);
      syncwithchild (pid, CHILD_EXPECT?TFAIL:0);   /* expecting error exit */
      tst_resm (TINFO, "parent records again ..");
      rectest (fd, 22050, 1, 16);
      tst_resm (TINFO, "parent done");
    }
  else
    {				/* child */
      expect_child_failure = CHILD_EXPECT;
      tst_resm (TINFO, "child trying to record %s",
		CHILD_EXPECT?"(should fail)..":"");
      rectest (fd, 44100, 1, 16);
      /* NOTREACHED */
      tst_resm (TINFO, "child done");
      fflush (stdout);
      exit (0);			/* implicit close */
    }
  if (close (fd) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
  tst_resm (TPASS, "child tries to record while parent is already recording");
}

void
forkplaytest (void)
{
  int pid;
  int fd;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  fd = open ("/dev/dsp", O_WRONLY);
  if (fd < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp R: %s",
		strerror (errno));
    }
  pid = fork ();
  if (pid < 0)
    {
      tst_brkm (TBROK, cleanup, "Fork failed: %s", strerror (errno));
    }
  if (pid)
    {
      tst_resm (TINFO, "forked, child PID=%d", pid);
      syncwithchild (pid, 0);
      tst_resm (TINFO, "parent plays..");
      playtest (fd, 22050, 0, 8);
      tst_resm (TINFO, "parent done");
    }
  else
    {				/* child */
      tst_resm (TINFO, "child plays..");
      playtest (fd, 44100, 1, 16);
      tst_resm (TINFO, "child done");
      fflush (stdout);
      exit (0);			/* implicit close */
    }
  tst_resm (TPASS, "child plays after fork");
  /* fork again, but now we have done a write before,
   * so the child is expected to fail
   */
  pid = fork ();
  if (pid < 0)
    {
      tst_brkm (TBROK, cleanup, "Fork failed");
    }
  if (pid)
    {
      tst_resm (TINFO, "forked, child PID=%d", pid);
      syncwithchild (pid, CHILD_EXPECT?TFAIL:0);  /* expected failure */
      tst_resm (TINFO, "parent plays again..");
      playtest (fd, 22050, 0, 8);
      tst_resm (TINFO, "parent done");
    }
  else
    {				/* child */
      expect_child_failure = CHILD_EXPECT;
      tst_resm (TINFO, "child trying to play %s",
		CHILD_EXPECT?"(should fail)..":"");
      playtest (fd, 44100, 1, 16);
      /* NOTREACHED */
      tst_resm (TINFO, "child done");
      fflush (stdout);
      exit (0);			/* implicit close */
    }
  if (close (fd) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
  tst_resm (TPASS, "child tries to play while parent is already playing");
}

void
playtest (int fd, int rate, int stereo, int bits)
{				/* Play sine waves, always 0.25 sec */
  void *wave;
  int n, c, b;
  int size;
  if (stereo)
    c = 2;
  else
    c = 1;
  if (bits == 8)
    b = 1;
  else
    b = 2;
  size = rate / 4 * c * b;

  wave = malloc (size);
  if (wave == NULL)
    {
      tst_brkm (TBROK, cleanup, "Malloc failed, exit");
    }
  setpars (fd, rate, stereo, bits);
  sinegen (wave, rate, bits, rate / 4, c);

  if ((n = write (fd, wave, size)) < 0)
    {
      tst_brkm (TFAIL, cleanup, "write: %s", strerror (errno));
    }
  if (n != size)
    {
      tst_brkm (TFAIL, cleanup, "Wrote %d, expected %d; exit", n, size);
    }
  free (wave);
}

void
rectest (int fd, int rate, int stereo, int bits)
{
  void *wave;
  int n, c, b;
  int size;
  if (stereo)
    c = 2;
  else
    c = 1;
  if (bits == 8)
    b = 1;
  else
    b = 2;
  size = rate / 4 * c * b;

  wave = malloc (size);
  if (wave == NULL)
    {
      tst_brkm (TBROK, cleanup, "Malloc failed, exit");
    }
  setpars (fd, rate, stereo, bits);
  if ((n = read (fd, wave, size)) < 0)
    {
      tst_brkm (TFAIL, cleanup, "read: %s", strerror (errno));
    }
  if (n != size)
    {
      tst_brkm (TFAIL, cleanup, "Read n=%d (%d expected); exit", n, size);
    }
  if ((n = read (fd, wave, size)) < 0)
    {
      tst_brkm (TFAIL, cleanup, "read: %s", strerror (errno));
    }
  if (n != size)
    {
      tst_brkm (TFAIL, cleanup, "Read n=%d (%d expected); exit", n, size);
    }
  free (wave);
}

void
rwtest (int fd, int rate, int stereo, int bits)
{
  int pid;
  void *wave;
  int n, c, b;
  int size;
  if (stereo)
    c = 2;
  else
    c = 1;
  if (bits == 8)
    b = 1;
  else
    b = 2;
  size = rate / 4 * c * b;

  wave = malloc (size);
  if (wave == NULL)
    {
      tst_brkm (TBROK, cleanup, "Malloc failed, exit");
    }
  setpars (fd, rate, stereo, bits);
  pid = fork ();
  if (pid < 0)
    {
      tst_brkm (TBROK, cleanup, "Fork failed: %s", strerror (errno));
    }
  if (pid)
    {
      tst_resm (TINFO, "forked, child PID=%d parent records", pid);
      if ((n = read (fd, wave, size)) < 0)
	{
	  tst_brkm (TFAIL, cleanup, "read: %s", strerror (errno));
	}
      if (n != size)
	{
	  tst_brkm (TFAIL, cleanup, "Read n=%d (%d expected)", n, size);
	}
      free (wave);
      syncwithchild (pid, 0);
    }
  else
    {				/* child */
      tst_resm (TINFO, "child plays");
      sinegen (wave, rate, bits, rate / 4, c);
      if ((n = write (fd, wave, size)) < 0)
	{
	  tst_brkm (TFAIL, cleanup, "child write: %s", strerror (errno));
	}
      if (n != size)
	{
	  tst_brkm (TFAIL, cleanup, "child write n=%d OK (%d expected)", n,
		    size);
	}
      free (wave);
      exit (0);
    }
}

void
setpars (int fd, int rate, int stereo, int bits)
{
  int ioctl_par = 0;

  if (ioctl (fd, SNDCTL_DSP_SAMPLESIZE, &bits) < 0)
    {
      if (expect_child_failure)
	{			/* Note: Don't print this to stderr because we expect failures here
				 *       for the some cases after fork()
				 */
	  tst_resm (TINFO, "ioctl SNDCTL_DSP_SAMPLESIZE: %s",
		    strerror (errno));
	  exit (TFAIL);
	}
      else
	{
	  tst_brkm (TFAIL, cleanup, "ioctl SNDCTL_DSP_SAMPLESIZE: %s",
		    strerror (errno));
	}
    }
  if (ioctl (fd, SNDCTL_DSP_STEREO, &stereo) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl SNDCTL_DSP_STEREO: %s",
		strerror (errno));
    }
  if (ioctl (fd, SNDCTL_DSP_SPEED, &rate) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl SNDCTL_DSP_SPEED: %s",
		strerror (errno));
    }
  if (ioctl (fd, SNDCTL_DSP_SYNC, &ioctl_par) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl SNDCTL_DSP_SYNC: %s",
		strerror (errno));
    }
}

void
syncwithchild (pid_t pid, int expected_exit_status)
{
  int status;

  if (waitpid (pid, &status, 0) != pid)
    {
      tst_brkm (TBROK, cleanup, "Wait for child: %s", strerror (errno));
    }
  if (!WIFEXITED (status))
    {
      tst_brkm (TBROK, cleanup, "Child had abnormal exit");
    }
  if (WEXITSTATUS (status) != expected_exit_status)
    {
      tst_brkm (TFAIL, cleanup, "Child had exit status %d != %d",
		WEXITSTATUS (status), expected_exit_status);
    }
}

void
sinegen (void *wave, int rate, int bits, int len, int stride)
{
  if (bits == 8)
    {
      sinegenb (1000, rate, (unsigned char *) wave, len, stride);
      if (stride == 2)
	sinegenb (800, rate, (unsigned char *) wave + 1, len, stride);
    }
  else
    {
      sinegenw (1000, rate, (short *) wave, len, stride);
      if (stride == 2)
	sinegenw (800, rate, (short *) wave + 1, len, stride);
    }
}

void
sinegenw (int freq, int samprate, short *value, int len, int stride)
{
  double phase, incr;

  phase = 0.0;
  incr = M_PI * 2.0 * (double) freq / (double) samprate;
  while (len-- > 0)
    {
      *value = (short) floor (0.5 + 6553 * sin (phase));
      value += stride;
      phase += incr;
    }
}

void
sinegenb (int freq, int samprate, unsigned char *value, int len, int stride)
{
  double phase, incr;

  phase = 0.0;
  incr = M_PI * 2.0 * (double) freq / (double) samprate;
  while (len-- > 0)
    {
      *value = (unsigned char) floor (128.5 + 26 * sin (phase));
      value += stride;
      phase += incr;
    }
}

void
abortplaytest (void)
{
  int audio;
  int size = sizeof (wavfile_okay);
  int n;
  int ioctl_par = 0;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  audio = open ("/dev/dsp", O_WRONLY);
  if (audio < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp W: %s",
		strerror (errno));
    }
  if ((n = write (audio, wavfile_okay, size)) < 0)
    {
      tst_brkm (TFAIL, cleanup, "write: %s", strerror (errno));
    }
  if (n != size)
    {
      tst_brkm (TFAIL, cleanup, "Wrote %d, expected %d; exit", n, size);
    }
  if (ioctl (audio, SNDCTL_DSP_RESET, &ioctl_par) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl DSP_RESET: %s", strerror (errno));
    }
  if (close (audio) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
  tst_resm (TPASS, "Playwav + ioctl DSP_RESET=%d", ioctl_par);
}

void
playwavtest (void)
{
  int audio;
  int size = sizeof (wavfile_okay);
  int n;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  audio = open ("/dev/dsp", O_WRONLY);
  if (audio < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp W: %s",
		strerror (errno));
    }
  if ((n = write (audio, wavfile_okay, size)) < 0)
    {
      tst_brkm (TFAIL, cleanup, "write: %s", strerror (errno));
    }
  if (n != size)
    {
      tst_brkm (TFAIL, cleanup, "Wrote %d, expected %d; exit", n, size);
    }
  if (close (audio) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
  tst_resm (TPASS, "Set parameters from wave file header");
}

void dup_test (void)
{
  int audio, fd, n;
  int bits1, bits2;
  int size = sizeof (wavfile_okay);
  int header = 44;
  const char *okay = wavfile_okay + header;

  tst_resm (TINFO, "Running %s", __FUNCTION__);
  audio = open ("/dev/dsp", O_WRONLY);
  if (audio < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp W: %s",
		strerror (errno));
    }
  /* write header once to set parameters correctly */
  n = write (audio, wavfile_okay, header);
  if (n != header)
    {
      tst_brkm (TFAIL, cleanup, "Wrote %d, expected %d; exit", n, header);
    }
  size = size - header;
  /* dup / close */
  for (fd = audio+1; fd <= audio+5; fd++)
    if (dup2 (fd-1, fd) != -1)
      {
	if (fd-2 >= audio)
	  if (close (fd-2) < 0)
	    {
	      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
	    }
	if ((n = write (fd, okay, size)) < 0)
	  {
	    tst_brkm (TFAIL, cleanup, "write: %s", strerror (errno));
	  }
	if (n != size)
	  {
	    tst_brkm (TFAIL, cleanup, "Wrote %d, expected %d; exit", n, size);
	  }
      }
    else
      tst_brkm (TFAIL, cleanup, "dup: %s", strerror (errno));

  for (fd = audio+4; fd <= audio+5; fd++)
    if (close (fd) < 0)
      {
	tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
      }
  tst_resm (TPASS, "Write to duped fd");

  audio = open ("/dev/dsp", O_WRONLY);
  if (audio < 0)
    {
      tst_brkm (TFAIL, cleanup, "Error open /dev/dsp W: %s",
		strerror (errno));
    }
  fd = audio + 1;
  if (dup2 (audio, fd) == -1)
    {
      tst_brkm (TFAIL, cleanup, "dup: %s", strerror (errno));
    }
  bits1 = AFMT_U8;
  if (ioctl (audio, SNDCTL_DSP_SAMPLESIZE, &bits1) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl: %s", strerror (errno));
    }
  bits1 = AFMT_S16_LE;
  if (ioctl (fd, SNDCTL_DSP_SAMPLESIZE, &bits1) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl: %s", strerror (errno));
    }
  bits1 = AFMT_QUERY;
  if (ioctl (audio, SNDCTL_DSP_SAMPLESIZE, &bits1) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl: %s", strerror (errno));
    }
  bits2 = AFMT_QUERY;
  if (ioctl (fd, SNDCTL_DSP_SAMPLESIZE, &bits2) < 0)
    {
      tst_brkm (TFAIL, cleanup, "ioctl: %s", strerror (errno));
    }
  if (bits1 != AFMT_S16_LE || bits2 != AFMT_S16_LE)
    {
      tst_brkm (TFAIL, cleanup, "Inconsistent state of duped fd: %d %d %d",
		AFMT_S16_LE,bits1,bits2);
    }
  if (close (audio) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
  if (close (fd) < 0)
    {
      tst_brkm (TFAIL, cleanup, "Close audio: %s", strerror (errno));
    }
  tst_resm (TPASS, "Parameter change to duped fd");
}
void
cleanup (void)
{
}
