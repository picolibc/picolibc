/* fhandler_dev_dsp: code to emulate OSS sound model /dev/dsp

   Written by Andy Younger (andy@snoogie.demon.co.uk)
   Extended by Gerd Spalink (Gerd.Spalink@t-online.de)
     to support recording from the audio input

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/soundcard.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "cygwait.h"

/*------------------------------------------------------------------------
  Simple encapsulation of the win32 audio device.

  Implementation Notes
  1. Audio structures are malloced just before the first read or
     write to /dev/dsp. The actual buffer size is determined at that time,
     such that one buffer holds about 125ms of audio data.
     At the time of this writing, 12 buffers are allocated,
     so that up to 1.5 seconds can be buffered within Win32.
     The buffer size can be queried with the ioctl SNDCTL_DSP_GETBLKSIZE,
     but for this implementation only returns meaningful results if
     sampling rate, number of channels and number of bits per sample
     are not changed afterwards.
     The audio structures are freed when the device is reset or closed,
     and they are not passed to exec'ed processes.
     The dev_ member is cleared after a fork. This forces the child
     to reopen the audio device._

  2. Every open call creates a new instance of the handler. After a
     successful open, every subsequent open from the same process
     to the device fails with EBUSY.
     The structures are shared between duped handles, but not with
     children. They only inherit the settings from the parent.
 */

enum { DEFAULT_BLOCKS = 12, MAX_BLOCKS = 256 };

class fhandler_dev_dsp::Audio
{ // This class contains functionality common to Audio_in and Audio_out
 public:
   Audio (fhandler_dev_dsp *my_fh);
   ~Audio ();

  class queue;

  bool isvalid ();
  void setconvert (int format);
  void convert_none (unsigned char *buffer, int size_bytes) { }
  void convert_U8_S8 (unsigned char *buffer, int size_bytes);
  void convert_S16LE_U16LE (unsigned char *buffer, int size_bytes);
  void convert_S16LE_U16BE (unsigned char *buffer, int size_bytes);
  void convert_S16LE_S16BE (unsigned char *buffer, int size_bytes);
  void fillFormat (WAVEFORMATEX * format,
		   int rate, int bits, int channels);
  static unsigned blockSize (int rate, int bits, int channels);
  void (fhandler_dev_dsp::Audio::*convert_)
    (unsigned char *buffer, int size_bytes);

  int bufferIndex_;  // offset into pHdr_->lpData
  WAVEHDR *pHdr_;    // data to be filled by write
  WAVEHDR wavehdr_[MAX_BLOCKS];
  char *bigwavebuffer_; // audio samples only
  // Member variables below must be locked
  queue *Qisr2app_; // blocks passed from wave callback

  fhandler_dev_dsp *fh;
};

class fhandler_dev_dsp::Audio::queue
{ // non-blocking fixed size queues for buffer management
 public:
   queue (int depth = 4);
  ~queue ();

  bool send (WAVEHDR *);  // queue an item, returns true if successful
  bool recv (WAVEHDR **); // retrieve an item, returns true if successful
  void reset ();
  int query (); // return number of items queued
  inline void lock () { EnterCriticalSection (&lock_); }
  inline void unlock () { LeaveCriticalSection (&lock_); }
  inline void dellock () { debug_printf ("Deleting Critical Section"); DeleteCriticalSection (&lock_); }
  bool isvalid () { return storage_; }
 private:
  CRITICAL_SECTION lock_;
  int head_;
  int tail_;
  int depth_;
  WAVEHDR **storage_;
};

static void CALLBACK waveOut_callback (HWAVEOUT hWave, UINT msg,
				       DWORD_PTR instance, DWORD_PTR param1,
				       DWORD_PTR param2);

class fhandler_dev_dsp::Audio_out: public Audio
{
 public:
  Audio_out (fhandler_dev_dsp *my_fh) : Audio (my_fh) {}

  void fork_fixup (HANDLE parent);
  bool query (int rate, int bits, int channels);
  bool start ();
  void stop (bool immediately = false);
  int write (const char *pSampleData, int nBytes);
  void buf_info (audio_buf_info *p, int rate, int bits, int channels);
  static void default_buf_info (audio_buf_info *p, int rate, int bits, int channels);
  void callback_sampledone (WAVEHDR *pHdr);
  bool parsewav (const char *&pData, int &nBytes,
		 int rate, int bits, int channels);

 private:
  void init (unsigned blockSize);
  void waitforallsent ();
  bool waitforspace ();
  bool sendcurrent ();

  HWAVEOUT dev_;     // The wave device
  /* Private copies of audiofreq_, audiobits_, audiochannels_,
     possibly set from wave file */
  int freq_;
  int bits_;
  int channels_;

  friend fhandler_dev_dsp;
};

static void CALLBACK waveIn_callback (HWAVEIN hWave, UINT msg,
				      DWORD_PTR instance, DWORD_PTR param1,
				      DWORD_PTR param2);

class fhandler_dev_dsp::Audio_in: public Audio
{
public:
  Audio_in (fhandler_dev_dsp *my_fh) : Audio (my_fh) {}

  void fork_fixup (HANDLE parent);
  bool query (int rate, int bits, int channels);
  bool start (int rate, int bits, int channels);
  void stop ();
  bool read (char *pSampleData, int &nBytes);
  void buf_info (audio_buf_info *p, int rate, int bits, int channels);
  static void default_buf_info (audio_buf_info *p, int rate, int bits, int channels);
  void callback_blockfull (WAVEHDR *pHdr);

private:
  bool init (unsigned blockSize);
  bool queueblock (WAVEHDR *pHdr);
  bool waitfordata (); // blocks until we have a good pHdr_ unless O_NONBLOCK

  HWAVEIN dev_;
};

/* --------------------------------------------------------------------
   Implementation */

// Simple fixed length FIFO queue implementation for audio buffer management
fhandler_dev_dsp::Audio::queue::queue (int depth)
{
  // allow space for one extra object in the queue
  // so we can distinguish full and empty status
  depth_ = depth;
  storage_ = new WAVEHDR *[depth_ + 1];
}

fhandler_dev_dsp::Audio::queue::~queue ()
{
  delete[] storage_;
}

void
fhandler_dev_dsp::Audio::queue::reset ()
 {
   /* When starting, after reset and after fork */
   head_ = tail_ = 0;
   debug_printf ("InitializeCriticalSection");
   memset (&lock_, 0, sizeof (lock_));
   InitializeCriticalSection (&lock_);
 }

bool
fhandler_dev_dsp::Audio::queue::send (WAVEHDR *x)
{
  bool res = false;
  lock ();
  if (query () == depth_)
    system_printf ("Queue overflow");
  else
    {
      storage_[tail_] = x;
      if (++tail_ > depth_)
	tail_ = 0;
      res = true;
    }
  unlock ();
  return res;
}

bool
fhandler_dev_dsp::Audio::queue::recv (WAVEHDR **x)
{
  bool res = false;
  lock ();
  if (query () != 0)
    {
      *x = storage_[head_];
      if (++head_ > depth_)
	head_ = 0;
      res = true;
    }
  unlock ();
  return res;
}

int
fhandler_dev_dsp::Audio::queue::query ()
{
  int n = tail_ - head_;
  if (n < 0)
    n += depth_ + 1;
  return n;
}

// Audio class implements functionality need for both read and write
fhandler_dev_dsp::Audio::Audio (fhandler_dev_dsp *my_fh)
{
  bigwavebuffer_ = NULL;
  fh = my_fh;
  Qisr2app_ = new queue (fh->fragstotal_);
  convert_ = &fhandler_dev_dsp::Audio::convert_none;
}

fhandler_dev_dsp::Audio::~Audio ()
{
  debug_printf("");
  delete Qisr2app_;
  delete[] bigwavebuffer_;
}

inline bool
fhandler_dev_dsp::Audio::isvalid ()
{
  return bigwavebuffer_ && Qisr2app_ && Qisr2app_->isvalid ();
}

void
fhandler_dev_dsp::Audio::setconvert (int format)
{
  switch (format)
    {
    case AFMT_S8:
      convert_ = &fhandler_dev_dsp::Audio::convert_U8_S8;
      debug_printf ("U8_S8");
      break;
    case AFMT_U16_LE:
      convert_ = &fhandler_dev_dsp::Audio::convert_S16LE_U16LE;
      debug_printf ("S16LE_U16LE");
      break;
    case AFMT_U16_BE:
      convert_ = &fhandler_dev_dsp::Audio::convert_S16LE_U16BE;
      debug_printf ("S16LE_U16BE");
      break;
    case AFMT_S16_BE:
      convert_ = &fhandler_dev_dsp::Audio::convert_S16LE_S16BE;
      debug_printf ("S16LE_S16BE");
      break;
    default:
      convert_ = &fhandler_dev_dsp::Audio::convert_none;
      debug_printf ("none");
    }
}

void
fhandler_dev_dsp::Audio::convert_U8_S8 (unsigned char *buffer,
					int size_bytes)
{
  while (size_bytes-- > 0)
    {
      *buffer ^= (unsigned char)0x80;
      buffer++;
    }
}

void
fhandler_dev_dsp::Audio::convert_S16LE_U16BE (unsigned char *buffer,
					      int size_bytes)
{
  int size_samples = size_bytes / 2;
  unsigned char hi, lo;
  while (size_samples-- > 0)
    {
      hi = buffer[0];
      lo = buffer[1];
      *buffer++ = lo;
      *buffer++ = hi ^ (unsigned char)0x80;
    }
}

void
fhandler_dev_dsp::Audio::convert_S16LE_U16LE (unsigned char *buffer,
					      int size_bytes)
{
  int size_samples = size_bytes / 2;
  while (size_samples-- > 0)
    {
      buffer++;
      *buffer ^= (unsigned char)0x80;
      buffer++;
    }
}

void
fhandler_dev_dsp::Audio::convert_S16LE_S16BE (unsigned char *buffer,
					      int size_bytes)
{
  int size_samples = size_bytes / 2;
  unsigned char hi, lo;
  while (size_samples-- > 0)
    {
      hi = buffer[0];
      lo = buffer[1];
      *buffer++ = lo;
      *buffer++ = hi;
    }
}

void
fhandler_dev_dsp::Audio::fillFormat (WAVEFORMATEX * format,
				     int rate, int bits, int channels)
{
  memset (format, 0, sizeof (*format));
  format->wFormatTag = WAVE_FORMAT_PCM;
  format->wBitsPerSample = bits;
  format->nChannels = channels;
  format->nSamplesPerSec = rate;
  format->nAvgBytesPerSec = format->nSamplesPerSec * format->nChannels
    * (bits / 8);
  format->nBlockAlign = format->nChannels * (bits / 8);
}

// calculate a good block size
unsigned
fhandler_dev_dsp::Audio::blockSize (int rate, int bits, int channels)
{
  unsigned blockSize;
  blockSize = ((bits / 8) * channels * rate) / 8; // approx 125ms per block
  // round up to multiple of 64
  blockSize +=  0x3f;
  blockSize &= ~0x3f;
  return blockSize;
}

//=======================================================================
void
fhandler_dev_dsp::Audio_out::fork_fixup (HANDLE parent)
{
  /* Null dev_.
     It will be necessary to reset the queue, open the device
     and create a lock when writing */
  debug_printf ("parent=%p", parent);
  dev_ = NULL;
}


bool
fhandler_dev_dsp::Audio_out::query (int rate, int bits, int channels)
{
  WAVEFORMATEX format;
  MMRESULT rc;

  fillFormat (&format, rate, bits, channels);
  rc = waveOutOpen (NULL, WAVE_MAPPER, &format, 0L, 0L, WAVE_FORMAT_QUERY);
  debug_printf ("%u = waveOutOpen(freq=%d bits=%d channels=%d)", rc, rate, bits, channels);
  return (rc == MMSYSERR_NOERROR);
}

bool
fhandler_dev_dsp::Audio_out::start ()
{
  WAVEFORMATEX format;
  MMRESULT rc;

  if (dev_)
    return true;

  /* In case of fork bigwavebuffer may already exist */
  if (!bigwavebuffer_)
    bigwavebuffer_ = new char[fh->fragstotal_ * fh->fragsize_];

  if (!isvalid ())
    return false;

  fillFormat (&format, freq_, bits_, channels_);
  rc = waveOutOpen (&dev_, WAVE_MAPPER, &format, (DWORD_PTR) waveOut_callback,
		     (DWORD_PTR) this, CALLBACK_FUNCTION);
  if (rc == MMSYSERR_NOERROR)
    init (fh->fragsize_);

  debug_printf ("%u = waveOutOpen(freq=%d bits=%d channels=%d)", rc, freq_, bits_, channels_);

  return (rc == MMSYSERR_NOERROR);
}

void
fhandler_dev_dsp::Audio_out::stop (bool immediately)
{
  MMRESULT rc;
  WAVEHDR *pHdr;

  debug_printf ("dev_=%p", dev_);
  if (dev_)
    {
      if (!immediately)
	{
	  sendcurrent ();	// force out last block whatever size..
	  waitforallsent ();	// block till finished..
	}

      rc = waveOutReset (dev_);
      debug_printf ("%u = waveOutReset()", rc);
      while (Qisr2app_->recv (&pHdr))
	{
	  rc = waveOutUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR));
	  debug_printf ("%u = waveOutUnprepareHeader(%p)", rc, pHdr);
	}

      no_thread_exit_protect for_now (true);
      rc = waveOutClose (dev_);
      debug_printf ("%u = waveOutClose()", rc);

      Qisr2app_->dellock ();
    }
}

void
fhandler_dev_dsp::Audio_out::init (unsigned blockSize)
{
  int i;

  // internally queue all of our buffer for later use by write
  Qisr2app_->reset ();
  for (i = 0; i < fh->fragstotal_; i++)
    {
      wavehdr_[i].lpData = &bigwavebuffer_[i * blockSize];
      wavehdr_[i].dwUser = (int) blockSize;
      wavehdr_[i].dwFlags = 0;
      if (!Qisr2app_->send (&wavehdr_[i]))
	{
	  system_printf ("Internal Error i=%d", i);
	  break; // should not happen
	}
    }
  pHdr_ = NULL;
}

int
fhandler_dev_dsp::Audio_out::write (const char *pSampleData, int nBytes)
{
  int bytes_to_write = nBytes;
  while (bytes_to_write != 0)
    { // Block if all blocks used until at least one is free
      if (!waitforspace ())
	{
	  if (bytes_to_write != nBytes)
	    break;
	  return -1;
	}

      int sizeleft = (int)pHdr_->dwUser - bufferIndex_;
      if (bytes_to_write < sizeleft)
	{ // all data fits into the current block, with some space left
	  memcpy (&pHdr_->lpData[bufferIndex_], pSampleData, bytes_to_write);
	  bufferIndex_ += bytes_to_write;
	  bytes_to_write = 0;
	  break;
	}
      else
	{ // data will fill up the current block
	  memcpy (&pHdr_->lpData[bufferIndex_], pSampleData, sizeleft);
	  bufferIndex_ += sizeleft;
	  sendcurrent ();
	  pSampleData += sizeleft;
	  bytes_to_write -= sizeleft;
	}
    }
  return nBytes - bytes_to_write;
}

void
fhandler_dev_dsp::Audio_out::buf_info (audio_buf_info *p,
				       int rate, int bits, int channels)
{
  if (dev_)
    {
      /* If the device is running we use the internal values,
	 possibly set from the wave file. */
      p->fragstotal = fh->fragstotal_;
      p->fragsize = fh->fragsize_;
      p->fragments = Qisr2app_->query ();
      if (pHdr_ != NULL)
	p->bytes = (int)pHdr_->dwUser - bufferIndex_
	  + p->fragsize * p->fragments;
      else
	p->bytes = p->fragsize * p->fragments;
    }
  else
    {
      default_buf_info(p, rate, bits, channels);
    }
}

void fhandler_dev_dsp::Audio_out::default_buf_info (audio_buf_info *p,
                                                int rate, int bits, int channels)
{
  p->fragstotal = DEFAULT_BLOCKS;
  p->fragsize = blockSize (rate, bits, channels);
  p->fragments = p->fragstotal;
  p->bytes = p->fragsize * p->fragments;
}

/* This is called on an interupt so use locking.. Note Qisr2app_
   is used so we should wrap all references to it in locks. */
inline void
fhandler_dev_dsp::Audio_out::callback_sampledone (WAVEHDR *pHdr)
{
  Qisr2app_->send (pHdr);
}

bool
fhandler_dev_dsp::Audio_out::waitforspace ()
{
  WAVEHDR *pHdr;
  MMRESULT rc = WAVERR_STILLPLAYING;

  if (pHdr_ != NULL)
    return true;
  while (!Qisr2app_->recv (&pHdr))
    {
      if (fh->is_nonblocking ())
	{
	  set_errno (EAGAIN);
	  return false;
	}
      debug_printf ("1ms");
      switch (cygwait (1))
	{
	case WAIT_SIGNALED:
	  if (!_my_tls.call_signal_handler ())
	    {
	      set_errno (EINTR);
	      return false;
	    }
	  break;
	case WAIT_CANCELED:
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	default:
	  break;
	}
    }
  if (pHdr->dwFlags)
    {
      /* Errors are ignored here. They will probbaly cause a failure
	 in the subsequent PrepareHeader */
      rc = waveOutUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR));
      debug_printf ("%u = waveOutUnprepareHeader(%p)", rc, pHdr);
    }
  pHdr_ = pHdr;
  bufferIndex_ = 0;
  return true;
}

void
fhandler_dev_dsp::Audio_out::waitforallsent ()
{
  while (Qisr2app_->query () != fh->fragstotal_)
    {
      debug_printf ("%d blocks in Qisr2app", Qisr2app_->query ());
      Sleep (100);
    }
}

// send the block described by pHdr_ and bufferIndex_ to wave device
bool
fhandler_dev_dsp::Audio_out::sendcurrent ()
{
  WAVEHDR *pHdr = pHdr_;
  MMRESULT rc;
  debug_printf ("pHdr=%p bytes=%d", pHdr, bufferIndex_);

  if (pHdr_ == NULL)
    return false;
  pHdr_ = NULL;

  // Sample buffer conversion
  (this->*convert_) ((unsigned char *)pHdr->lpData, bufferIndex_);

  // Send internal buffer out to the soundcard
  pHdr->dwBufferLength = bufferIndex_;
  rc = waveOutPrepareHeader (dev_, pHdr, sizeof (WAVEHDR));
  debug_printf ("%u = waveOutPrepareHeader(%p)", rc, pHdr);
  if (rc == MMSYSERR_NOERROR)
    {
      rc = waveOutWrite (dev_, pHdr, sizeof (WAVEHDR));
      debug_printf ("%u = waveOutWrite(%p)", rc, pHdr);
    }
  if (rc == MMSYSERR_NOERROR)
    return true;

  /* FIXME: Should we return an error instead ?*/
  pHdr->dwFlags = 0; /* avoid calling UnprepareHeader again */
  Qisr2app_->send (pHdr);
  return false;
}

//------------------------------------------------------------------------
// Call back routine
static void CALLBACK
waveOut_callback (HWAVEOUT hWave, UINT msg, DWORD_PTR instance,
		  DWORD_PTR param1, DWORD_PTR param2)
{
  if (msg == WOM_DONE)
    {
      fhandler_dev_dsp::Audio_out *ptr =
	(fhandler_dev_dsp::Audio_out *) instance;
      ptr->callback_sampledone ((WAVEHDR *) param1);
    }
}

//------------------------------------------------------------------------
// wav file detection..
#pragma pack(1)
struct wavchunk
{
  char id[4];
  unsigned int len;
};
struct wavformat
{
  unsigned short wFormatTag;
  unsigned short wChannels;
  unsigned int dwSamplesPerSec;
  unsigned int dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSample;
};
#pragma pack()

bool
fhandler_dev_dsp::Audio_out::parsewav (const char * &pData, int &nBytes,
				       int dev_freq, int dev_bits, int dev_channels)
{
  int len;
  const char *end = pData + nBytes;
  const char *pDat;
  int skip = 0;

  /* Start with default values from the device handler */
  freq_ = dev_freq;
  bits_ = dev_bits;
  channels_ = dev_channels;
  setconvert (bits_ == 8 ? AFMT_U8 : AFMT_S16_LE);

  // Check alignment first: A lot of the code below depends on it
  if (((uintptr_t)pData & 0x3) != 0)
    return false;
  if (!(pData[0] == 'R' && pData[1] == 'I'
	&& pData[2] == 'F' && pData[3] == 'F'))
    return false;
  if (!(pData[8] == 'W' && pData[9] == 'A'
	&& pData[10] == 'V' && pData[11] == 'E'))
    return false;

  len = *(int *) &pData[4];
  len -= 12;
  pDat = pData + 12;
  skip = 12;
  while ((len > 0) && (pDat + sizeof (wavchunk) < end))
    { /* We recognize two kinds of wavchunk:
	 "fmt " for the PCM parameters (only PCM supported here)
	 "data" for the start of PCM data */
      wavchunk * pChunk = (wavchunk *) pDat;
      int blklen = pChunk-> len;
      if (pChunk->id[0] == 'f' && pChunk->id[1] == 'm'
	  && pChunk->id[2] == 't' && pChunk->id[3] == ' ')
	{
	  wavformat *format = (wavformat *) (pChunk + 1);
	  if ((char *) (format + 1) >= end)
	    return false;
	  // We have found the parameter chunk
	  if (format->wFormatTag == 0x0001)
	    { // Micr*s*ft PCM; check if parameters work with our device
	      if (query (format->dwSamplesPerSec, format->wBitsPerSample,
			 format->wChannels))
		{ // return the parameters we found
		  freq_ = format->dwSamplesPerSec;
		  bits_ = format->wBitsPerSample;
		  channels_ = format->wChannels;
		}
	    }
	}
      else
	{
	  if (pChunk->id[0] == 'd' && pChunk->id[1] == 'a'
	      && pChunk->id[2] == 't' && pChunk->id[3] == 'a')
	    { // throw away all the header & not output it to the soundcard.
	      skip += sizeof (wavchunk);
	      debug_printf ("Discard %d bytes wave header", skip);
	      pData += skip;
	      nBytes -= skip;
	      setconvert (bits_ == 8 ? AFMT_U8 : AFMT_S16_LE);
	      return true;
	    }
	}
      pDat += blklen + sizeof (wavchunk);
      skip += blklen + sizeof (wavchunk);
      len -= blklen + sizeof (wavchunk);
    }
  return false;
}

/* ========================================================================
   Buffering concept for Audio_in:
   On the first read, we queue all blocks of our bigwavebuffer
   for reception and start the wave-in device.
   We manage queues of pointers to WAVEHDR
   When a block has been filled, the callback puts the corresponding
   WAVEHDR pointer into a queue.
   The function read() blocks (polled, sigh) until at least one good buffer
   has arrived, then the data is copied into the buffer provided to read().
   After a buffer has been fully used by read(), it is queued again
   to the wave-in device immediately.
   The function read() iterates until all data requested has been
   received, there is no way to interrupt it */

void
fhandler_dev_dsp::Audio_in::fork_fixup (HANDLE parent)
{
  /* Null dev_.
     It will be necessary to reset the queue, open the device
     and create a lock when reading */
  debug_printf ("parent=%p", parent);
  dev_ = NULL;
}

bool
fhandler_dev_dsp::Audio_in::query (int rate, int bits, int channels)
{
  WAVEFORMATEX format;
  MMRESULT rc;

  fillFormat (&format, rate, bits, channels);
  rc = waveInOpen (NULL, WAVE_MAPPER, &format, 0L, 0L, WAVE_FORMAT_QUERY);
  debug_printf ("%u = waveInOpen(freq=%d bits=%d channels=%d)", rc, rate, bits, channels);
  return (rc == MMSYSERR_NOERROR);
}

bool
fhandler_dev_dsp::Audio_in::start (int rate, int bits, int channels)
{
  WAVEFORMATEX format;
  MMRESULT rc;

  if (dev_)
    return true;

  /* In case of fork bigwavebuffer may already exist */
  if (!bigwavebuffer_)
    bigwavebuffer_ = new char[fh->fragstotal_ * fh->fragsize_];

  if (!isvalid ())
    return false;

  fillFormat (&format, rate, bits, channels);
  rc = waveInOpen (&dev_, WAVE_MAPPER, &format, (DWORD_PTR) waveIn_callback,
		   (DWORD_PTR) this, CALLBACK_FUNCTION);
  debug_printf ("%u = waveInOpen(rate=%d bits=%d channels=%d)", rc, rate, bits, channels);

  if (rc == MMSYSERR_NOERROR)
    {
      if (!init (fh->fragsize_))
	return false;
    }
  return (rc == MMSYSERR_NOERROR);
}

void
fhandler_dev_dsp::Audio_in::stop ()
{
  MMRESULT rc;
  WAVEHDR *pHdr;

  debug_printf ("dev_=%p", dev_);
  if (dev_)
    {
      /* Note that waveInReset calls our callback for all incomplete buffers.
	 Since all the win32 wave functions appear to use a common lock,
	 we must not call into the wave API from the callback.
	 Otherwise we end up in a deadlock. */
      rc = waveInReset (dev_);
      debug_printf ("%u = waveInReset()", rc);

      while (Qisr2app_->recv (&pHdr))
	{
	  rc = waveInUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR));
	  debug_printf ("%u = waveInUnprepareHeader(%p)", rc, pHdr);
	}

      no_thread_exit_protect for_now (true);
      rc = waveInClose (dev_);
      debug_printf ("%u = waveInClose()", rc);

      Qisr2app_->dellock ();
    }
}

bool
fhandler_dev_dsp::Audio_in::queueblock (WAVEHDR *pHdr)
{
  MMRESULT rc;
  rc = waveInPrepareHeader (dev_, pHdr, sizeof (WAVEHDR));
  debug_printf ("%u = waveInPrepareHeader(%p)", rc, pHdr);
  if (rc == MMSYSERR_NOERROR)
    {
      rc = waveInAddBuffer (dev_, pHdr, sizeof (WAVEHDR));
      debug_printf ("%u = waveInAddBuffer(%p)", rc, pHdr);
    }
  if (rc == MMSYSERR_NOERROR)
    return true;

  /* FIXME: Should the calling function return an error instead ?*/
  pHdr->dwFlags = 0;  /* avoid calling UnprepareHeader again */
  pHdr->dwBytesRecorded = 0;  /* no data will have been read */
  Qisr2app_->send (pHdr);
  return false;
}

bool
fhandler_dev_dsp::Audio_in::init (unsigned blockSize)
{
  MMRESULT rc;
  int i;

  // try to queue all of our buffer for reception
  Qisr2app_->reset ();
  for (i = 0; i < fh->fragstotal_; i++)
    {
      wavehdr_[i].lpData = &bigwavebuffer_[i * blockSize];
      wavehdr_[i].dwBufferLength = blockSize;
      wavehdr_[i].dwFlags = 0;
      if (!queueblock (&wavehdr_[i]))
	break;
    }
  pHdr_ = NULL;
  rc = waveInStart (dev_);
  debug_printf ("%u = waveInStart(), queued=%d", rc, i);
  return (rc == MMSYSERR_NOERROR);
}

bool
fhandler_dev_dsp::Audio_in::read (char *pSampleData, int &nBytes)
{
  int bytes_to_read = nBytes;
  nBytes = 0;
  debug_printf ("pSampleData=%p nBytes=%d", pSampleData, bytes_to_read);
  while (bytes_to_read != 0)
    { // Block till next sound has been read
      if (!waitfordata ())
	{
	  if (nBytes)
	    return true;
	  nBytes = -1;
	  return false;
	}

      // Handle gathering our blocks into smaller or larger buffer
      int sizeleft = pHdr_->dwBytesRecorded - bufferIndex_;
      if (bytes_to_read < sizeleft)
	{ // The current buffer holds more data than requested
	  memcpy (pSampleData, &pHdr_->lpData[bufferIndex_], bytes_to_read);
	  (this->*convert_) ((unsigned char *)pSampleData, bytes_to_read);
	  nBytes += bytes_to_read;
	  bufferIndex_ += bytes_to_read;
	  debug_printf ("got %d", bytes_to_read);
	  break; // done; use remaining data in next call to read
	}
      else
	{ // not enough or exact amount in the current buffer
	  if (sizeleft)
	    { // use up what we have
	      memcpy (pSampleData, &pHdr_->lpData[bufferIndex_], sizeleft);
	      (this->*convert_) ((unsigned char *)pSampleData, sizeleft);
	      nBytes += sizeleft;
	      bytes_to_read -= sizeleft;
	      pSampleData += sizeleft;
	      debug_printf ("got %d", sizeleft);
	    }
	  queueblock (pHdr_); // re-queue this block to ISR
	  pHdr_ = NULL;       // need to wait for a new block
	  // if more samples are needed, we need a new block now
	}
    }
  debug_printf ("end nBytes=%d", nBytes);
  return true;
}

bool
fhandler_dev_dsp::Audio_in::waitfordata ()
{
  WAVEHDR *pHdr;
  MMRESULT rc;

  if (pHdr_ != NULL)
    return true;
  while (!Qisr2app_->recv (&pHdr))
    {
      if (fh->is_nonblocking ())
	{
	  set_errno (EAGAIN);
	  return false;
	}
      debug_printf ("100ms");
      switch (cygwait (100))
	{
	case WAIT_SIGNALED:
	  if (!_my_tls.call_signal_handler ())
	    {
	      set_errno (EINTR);
	      return false;
	    }
	  break;
	case WAIT_CANCELED:
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	default:
	  break;
	}
    }
  if (pHdr->dwFlags) /* Zero if queued following error in queueblock */
    {
      /* Errors are ignored here. They will probbaly cause a failure
	 in the subsequent PrepareHeader */
      rc = waveInUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR));
      debug_printf ("%u = waveInUnprepareHeader(%p)", rc, pHdr);
    }
  pHdr_ = pHdr;
  bufferIndex_ = 0;
  return true;
}

void fhandler_dev_dsp::Audio_in::default_buf_info (audio_buf_info *p,
                                                int rate, int bits, int channels)
{
  p->fragstotal = DEFAULT_BLOCKS;
  p->fragsize = blockSize (rate, bits, channels);
  p->fragments = 0;
  p->bytes = 0;
}

void
fhandler_dev_dsp::Audio_in::buf_info (audio_buf_info *p,
				      int rate, int bits, int channels)
{
  if (dev_)
    {
      p->fragstotal = fh->fragstotal_;
      p->fragsize = fh->fragsize_;
      p->fragments = Qisr2app_->query ();
      if (pHdr_ != NULL)
	p->bytes = pHdr_->dwBytesRecorded - bufferIndex_
	  + p->fragsize * p->fragments;
      else
	p->bytes = p->fragsize * p->fragments;
    }
  else
    {
      default_buf_info(p, rate, bits, channels);
    }
}

inline void
fhandler_dev_dsp::Audio_in::callback_blockfull (WAVEHDR *pHdr)
{
  Qisr2app_->send (pHdr);
}

static void CALLBACK
waveIn_callback (HWAVEIN hWave, UINT msg, DWORD_PTR instance, DWORD_PTR param1,
		 DWORD_PTR param2)
{
  if (msg == WIM_DATA)
    {
      fhandler_dev_dsp::Audio_in *ptr =
	(fhandler_dev_dsp::Audio_in *) instance;
      ptr->callback_blockfull ((WAVEHDR *) param1);
    }
}


/* ------------------------------------------------------------------------
   /dev/dsp handler
   ------------------------------------------------------------------------ */
fhandler_dev_dsp::fhandler_dev_dsp ():
  fhandler_base ()
{
  audio_in_ = NULL;
  audio_out_ = NULL;
  dev ().parse (FH_OSS_DSP);
}

ssize_t
fhandler_dev_dsp::write (const void *ptr, size_t len)
{
  return base ()->_write (ptr, len);
}

void
fhandler_dev_dsp::read (void *ptr, size_t& len)
{
  return base ()->_read (ptr, len);
}

int
fhandler_dev_dsp::ioctl (unsigned int cmd, void *buf)
{
  return base ()->_ioctl (cmd, buf);
}

int
fhandler_dev_dsp::fcntl (int cmd, intptr_t arg)
{
  return base ()->_fcntl (cmd, arg);
}

void
fhandler_dev_dsp::fixup_after_fork (HANDLE parent)
{
  base ()->_fixup_after_fork (parent);
}

void
fhandler_dev_dsp::fixup_after_exec ()
{
  base ()->_fixup_after_exec ();
}


int
fhandler_dev_dsp::open (int flags, mode_t)
{
  int ret = 0, err = 0;
  UINT num_in = 0, num_out = 0;
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  // Work out initial sample format & frequency, /dev/dsp defaults
  audioformat_ = AFMT_U8;
  audiofreq_ = 8000;
  audiobits_ = 8;
  audiochannels_ = 1;
  fragstotal_ = DEFAULT_BLOCKS;
  fragment_has_been_set = false;
  switch (flags & O_ACCMODE)
    {
    case O_RDWR:
      if ((num_in = waveInGetNumDevs ()) == 0)
	err = ENXIO;
      fallthrough;
    case O_WRONLY:
      if ((num_out = waveOutGetNumDevs ()) == 0)
	err = ENXIO;
      break;
    case O_RDONLY:
      if ((num_in = waveInGetNumDevs ()) == 0)
	err = ENXIO;
      break;
    default:
      err = EINVAL;
    }

  if (err)
    set_errno (err);
  else
    ret = open_null (flags);

  debug_printf ("ACCMODE=%y audio_in=%d audio_out=%d, err=%d, ret=%d",
		flags & O_ACCMODE, num_in, num_out, err, ret);
  if (ret)
    being_closed = false;
  return ret;
}

#define IS_WRITE() ((get_flags() & O_ACCMODE) != O_RDONLY)
#define IS_READ() ((get_flags() & O_ACCMODE) != O_WRONLY)

ssize_t
fhandler_dev_dsp::_write (const void *ptr, size_t len)
{
  debug_printf ("ptr=%p len=%ld", ptr, len);
  int len_s = len;
  const char *ptr_s = static_cast <const char *> (ptr);

  if (being_closed)
    {
      set_errno (EBADF);
      return -1;
    }

  if (audio_out_)
    /* nothing to do */;
  else if (IS_WRITE ())
    {
      if (!fragment_has_been_set)
	fragsize_ = Audio::blockSize (audiofreq_, audiobits_, audiochannels_);
      debug_printf ("Allocating");
      if (!(audio_out_ = new Audio_out (this)))
	return -1;

      /* check for wave file & get parameters & skip header if possible. */

      if (audio_out_->parsewav (ptr_s, len_s,
				audiofreq_, audiobits_, audiochannels_))
	debug_printf ("=> ptr_s=%p len_s=%d", ptr_s, len_s);
    }
  else
    {
      set_errno (EBADF); // device was opened for read?
      return -1;
    }

  /* Open audio device properly with callbacks.
     Private parameters were set in call to parsewav.
     This is a no-op when there are successive writes in the same process */
  if (!audio_out_->start ())
    {
      set_errno (EIO);
      return -1;
    }

  int written = audio_out_->write (ptr_s, len_s);
  if (written < 0)
    {
      if (len - len_s > 0)
	return len - len_s;
      return -1;
    }
  return len - len_s + written;
}

void
fhandler_dev_dsp::_read (void *ptr, size_t& len)
{
  debug_printf ("ptr=%p len=%ld", ptr, len);

  if (audio_in_)
    /* nothing to do */;
  else if (IS_READ ())
    {
      if (!fragment_has_been_set)
	fragsize_ = Audio::blockSize (audiofreq_, audiobits_, audiochannels_);
      debug_printf ("Allocating");
      if (!(audio_in_ = new Audio_in (this)))
	{
	  len = (size_t)-1;
	  return;
	}
      audio_in_->setconvert (audioformat_);
    }
  else
    {
      len = (size_t)-1;
      set_errno (EBADF); // device was opened for write?
      return;
    }

  /* Open audio device properly with callbacks.
     This is a noop when there are successive reads in the same process */
  if (!audio_in_->start (audiofreq_, audiobits_, audiochannels_))
    {
      len = (size_t)-1;
      set_errno (EIO);
      return;
    }

  audio_in_->read ((char *)ptr, (int&)len);
}

void
fhandler_dev_dsp::close_audio_in ()
{
  if (audio_in_)
    {
      audio_in_->stop ();
      delete audio_in_;
      audio_in_ = NULL;
    }
}

void
fhandler_dev_dsp::close_audio_out (bool immediately)
{
  if (audio_out_)
    {
      audio_out_->stop (immediately);
      delete audio_out_;
      audio_out_ = NULL;
    }
}

int
fhandler_dev_dsp::close ()
{
  debug_printf ("audio_in=%p audio_out=%p", audio_in_, audio_out_);
  being_closed = true;
  close_audio_in ();
  close_audio_out ();
  return fhandler_base::close ();
}

int
fhandler_dev_dsp::_ioctl (unsigned int cmd, void *buf)
{
  debug_printf ("audio_in=%p audio_out=%p", audio_in_, audio_out_);
  int *intbuf = (int *) buf;
  switch (cmd)
    {
#define CASE(a) case a : debug_printf ("/dev/dsp: ioctl %s", #a);

      CASE (SNDCTL_DSP_RESET)
	close_audio_in ();
	close_audio_out (true);
	return 0;
	break;

      CASE (SNDCTL_DSP_GETBLKSIZE)
	if (!fragment_has_been_set)
	  fragsize_ = Audio::blockSize (audiofreq_, audiobits_, audiochannels_);
	*intbuf = fragsize_;
	return 0;

      CASE (SNDCTL_DSP_SETFMT)
      {
	int nBits;
	switch (*intbuf)
	  {
	  case AFMT_QUERY:
	    *intbuf = audioformat_;
	    return 0;
	    break;
	  case AFMT_U16_BE:
	  case AFMT_U16_LE:
	  case AFMT_S16_BE:
	  case AFMT_S16_LE:
	    nBits = 16;
	    break;
	  case AFMT_U8:
	  case AFMT_S8:
	    nBits = 8;
	    break;
	  default:
	    nBits = 0;
	  }
	if (nBits && IS_WRITE ())
	  {
	    close_audio_out ();
	    if (audio_out_->query (audiofreq_, nBits, audiochannels_))
	      {
		audiobits_ = nBits;
		audioformat_ = *intbuf;
	      }
	    else
	      {
		*intbuf = audiobits_;
		return -1;
	      }
	  }
	if (nBits && IS_READ ())
	  {
	    close_audio_in ();
	    if (audio_in_->query (audiofreq_, nBits, audiochannels_))
	      {
		audiobits_ = nBits;
		audioformat_ = *intbuf;
	      }
	    else
	      {
		*intbuf = audiobits_;
		return -1;
	      }
	  }
	return 0;
      }

      CASE (SNDCTL_DSP_SPEED)
	if (IS_WRITE ())
	  {
	    close_audio_out ();
	    if (audio_out_->query (*intbuf, audiobits_, audiochannels_))
	      audiofreq_ = *intbuf;
	    else
	      {
		*intbuf = audiofreq_;
		return -1;
	      }
	  }
	if (IS_READ ())
	  {
	    close_audio_in ();
	    if (audio_in_->query (*intbuf, audiobits_, audiochannels_))
	      audiofreq_ = *intbuf;
	    else
	      {
		*intbuf = audiofreq_;
		return -1;
	      }
	  }
	return 0;

      CASE (SNDCTL_DSP_STEREO)
      {
	int nChannels = *intbuf + 1;
	int res = _ioctl (SNDCTL_DSP_CHANNELS, &nChannels);
	*intbuf = nChannels - 1;
	return res;
      }

      CASE (SNDCTL_DSP_CHANNELS)
      {
	int nChannels = *intbuf;

	if (IS_WRITE ())
	  {
	    close_audio_out ();
	    if (audio_out_->query (audiofreq_, audiobits_, nChannels))
	      audiochannels_ = nChannels;
	    else
	      {
		*intbuf = audiochannels_;
		return -1;
	      }
	  }
	if (IS_READ ())
	  {
	    close_audio_in ();
	    if (audio_in_->query (audiofreq_, audiobits_, nChannels))
	      audiochannels_ = nChannels;
	    else
	      {
		*intbuf = audiochannels_;
		return -1;
	      }
	  }
	return 0;
      }

      CASE (SNDCTL_DSP_GETOSPACE)
      {
	if (!IS_WRITE ())
	  {
	    set_errno(EBADF);
	    return -1;
	  }
	audio_buf_info *p = (audio_buf_info *) buf;
	if (audio_out_)
	  audio_out_->buf_info (p, audiofreq_, audiobits_, audiochannels_);
	else if (fragment_has_been_set)
	  {
	    p->bytes = fragsize_ * fragstotal_;
	    p->fragsize = fragsize_;
	    p->fragstotal = fragstotal_;
	    p->fragments = fragstotal_;
	  }
	else
	  Audio_out::default_buf_info(p, audiofreq_, audiobits_, audiochannels_);
        debug_printf ("buf=%p frags=%d fragsize=%d bytes=%d",
                      buf, p->fragments, p->fragsize, p->bytes);
	return 0;
      }

      CASE (SNDCTL_DSP_GETISPACE)
      {
	if (!IS_READ ())
	  {
	    set_errno(EBADF);
	    return -1;
	  }
	audio_buf_info *p = (audio_buf_info *) buf;
	if (audio_in_)
	  audio_in_->buf_info (p, audiofreq_, audiobits_, audiochannels_);
	else if (fragment_has_been_set)
	  {
	    p->bytes = 0;
	    p->fragsize = fragsize_;
	    p->fragstotal = fragstotal_;
	    p->fragments = 0;
	  }
	else
	  Audio_in::default_buf_info(p, audiofreq_, audiobits_, audiochannels_);
        debug_printf ("buf=%p frags=%d fragsize=%d bytes=%d",
                      buf, p->fragments, p->fragsize, p->bytes);
	return 0;
      }

      CASE (SNDCTL_DSP_SETFRAGMENT)
      {
	if (audio_out_ || audio_in_)
	  return 0; /* Too late to set fragment. Ignore. */
	int *p = (int *) buf;
	fragstotal_ = min (*p >> 16, MAX_BLOCKS);
	fragsize_ = 1 << (*p & 0xffff);
	fragment_has_been_set = true;
	return 0;
      }

      CASE (SNDCTL_DSP_GETFMTS)
	*intbuf = AFMT_S16_LE | AFMT_U8; // only native formats returned here
	return 0;

      CASE (SNDCTL_DSP_GETCAPS)
	*intbuf = DSP_CAP_BATCH | DSP_CAP_DUPLEX;
	return 0;

      CASE (SNDCTL_DSP_POST)
	if (audio_out_)
	  audio_out_->sendcurrent (); // force out last block whatever size..
	return 0;

      CASE (SNDCTL_DSP_SYNC)
	if (audio_out_)
	  {
	    audio_out_->sendcurrent (); // force out last block whatever size..
	    audio_out_->waitforallsent (); // block till finished..
	  }
	return 0;

    default:
      return fhandler_base::ioctl (cmd, buf);
      break;

#undef CASE
    }
}

int
fhandler_dev_dsp::_fcntl (int cmd, intptr_t arg)
{
  return fhandler_base::fcntl(cmd, arg);
}

void
fhandler_dev_dsp::_fixup_after_fork (HANDLE parent)
{ // called from new child process
  debug_printf ("audio_in=%p audio_out=%p",
		audio_in_, audio_out_);

  fhandler_base::fixup_after_fork (parent);
  if (audio_in_)
    audio_in_->fork_fixup (parent);
  if (audio_out_)
    audio_out_->fork_fixup (parent);
}

void
fhandler_dev_dsp::_fixup_after_exec ()
{
  debug_printf ("audio_in=%p audio_out=%p, close_on_exec %d",
		audio_in_, audio_out_, close_on_exec ());
  if (!close_on_exec ())
    {
      audio_in_ = NULL;
      audio_out_ = NULL;
    }
}
