/* fhandler_dev_dsp: code to emulate OSS sound model /dev/dsp

   Copyright 2001, 2002, 2003, 2004 Red Hat, Inc

   Written by Andy Younger (andy@snoogie.demon.co.uk)
   Extended by Gerd Spalink (Gerd.Spalink@t-online.de)
     to support recording from the audio input

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <windows.h>
#include <sys/soundcard.h>
#include <mmsystem.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"

/*------------------------------------------------------------------------
  Simple encapsulation of the win32 audio device.

  Implementation Notes
  1. Audio buffers are created dynamically just before the first read or
     write to /dev/dsp. The actual buffer size is determined at that time,
     such that one buffer holds about 125ms of audio data.
     At the time of this writing, 12 buffers are allocated,
     so that up to 1.5 seconds can be buffered within Win32.
     The buffer size can be queried with the ioctl SNDCTL_DSP_GETBLKSIZE,
     but for this implementation only returns meaningful results if
     sampling rate, number of channels and number of bits per sample
     are not changed afterwards.

  2. Every open call creates a new instance of the handler. To cope
     with the fact that only a single wave device exists, the static
     variable open_count tracks opens for one process. After a
     successful open, every subsequent open from the same process
     to the device fails with EBUSY.
     If different processes open the audio device simultaneously,
     the results are unpredictable - usually the first one wins.
    
  3. The wave device is reserved within a process from the time that
     the first read or write call has been successful until /dev/dsp
     has been closed by that process. During this reservation period
     child processes that use the same file descriptor cannot
     do read, write or ioctls that change the device properties.
     This means that a parent can open the device, do some ioctl,
     spawn children, and any one of them can do the data read/write
 */

class fhandler_dev_dsp::Audio
{ // This class contains functionality common to Audio_in and Audio_out
 public:
   Audio ();
  ~Audio ();

  class queue;

  bool denyAccess ();
  void fork_fixup (HANDLE parent);
  inline DWORD getOwner () { return owner_; }
  void setOwner () { owner_ = GetCurrentProcessId (); }
  inline void clearOwner () { owner_ = 0L; }
  void setformat (int format);
  void convert_none (unsigned char *buffer, int size_bytes) { }
  void convert_U8_S8 (unsigned char *buffer, int size_bytes);
  void convert_S16LE_U16LE (unsigned char *buffer, int size_bytes);
  void convert_S16LE_U16BE (unsigned char *buffer, int size_bytes);
  void convert_S16LE_S16BE (unsigned char *buffer, int size_bytes);
  void fillFormat (WAVEFORMATEX * format,
		   int rate, int bits, int channels);
  unsigned blockSize (int rate, int bits, int channels);

  void (fhandler_dev_dsp::Audio::*convert_)
    (unsigned char *buffer, int size_bytes);
  inline void lock () { EnterCriticalSection (&lock_); }
  inline void unlock () { LeaveCriticalSection (&lock_); }
 private:
  DWORD owner_; /* Process ID when wave operation started, else 0 */
  CRITICAL_SECTION lock_;
};

class fhandler_dev_dsp::Audio::queue
{ // non-blocking fixed size queues for buffer management
 public:
   queue (int depth = 4);
  ~queue ();

  bool send (WAVEHDR *);  // queue an item, returns true if successful
  bool recv (WAVEHDR **); // retrieve an item, returns true if successful
  int query ();           // return number of items queued

 private:
  int head_;
  int tail_;
  int depth_, depth1_;
  WAVEHDR **storage_;
};

static void CALLBACK waveOut_callback (HWAVEOUT hWave, UINT msg, DWORD instance,
				       DWORD param1, DWORD param2);

class fhandler_dev_dsp::Audio_out: public Audio
{
 public:
   Audio_out ();
  ~Audio_out ();

  bool query (int rate, int bits, int channels);
  bool start (int rate, int bits, int channels);
  void stop ();
  bool write (const char *pSampleData, int nBytes);
  void buf_info (audio_buf_info *p, int rate, int bits, int channels);
  void callback_sampledone (WAVEHDR *pHdr);
  bool parsewav (const char *&pData, int &nBytes,
		 int &rate, int &bits, int &channels);

 private:
  void init (unsigned blockSize);
  void waitforallsent ();
  void waitforspace ();
  bool sendcurrent ();
  int emptyblocks ();

  enum { MAX_BLOCKS = 12 };
  queue *Qapp2app_;  // empty and unprepared blocks
  HWAVEOUT dev_;     // The wave device
  int bufferIndex_;  // offset into pHdr_->lpData
  WAVEHDR *pHdr_;    // data to be filled by write  
  WAVEHDR wavehdr_[MAX_BLOCKS];
  char *bigwavebuffer_; // audio samples only

  // Member variables below must be locked
  queue *Qisr2app_; // empty blocks passed from wave callback
};

static void CALLBACK waveIn_callback (HWAVEIN hWave, UINT msg, DWORD instance,
				      DWORD param1, DWORD param2);

class fhandler_dev_dsp::Audio_in: public Audio
{
public:
   Audio_in ();
  ~Audio_in ();

  bool query (int rate, int bits, int channels);
  bool start (int rate, int bits, int channels);
  void stop ();
  bool read (char *pSampleData, int &nBytes);
  void callback_blockfull (WAVEHDR *pHdr);

private:
  bool init (unsigned blockSize);
  bool queueblock (WAVEHDR *pHdr);
  void waitfordata (); // blocks until we have a good pHdr_

  enum { MAX_BLOCKS = 12 }; // read ahead of 1.5 seconds
  queue *Qapp2app_;    // filled and unprepared blocks
  HWAVEIN dev_;
  int bufferIndex_;    // offset into pHdr_->lpData
  WAVEHDR *pHdr_;      // successfully recorded data
  WAVEHDR wavehdr_[MAX_BLOCKS];
  char *bigwavebuffer_; // audio samples

  // Member variables below must be locked
  queue *Qisr2app_; // filled blocks passed from wave callback
};

/* --------------------------------------------------------------------
   Implementation */

// Simple fixed length FIFO queue implementation for audio buffer management
fhandler_dev_dsp::Audio::queue::queue (int depth)
{
  head_ = 0;
  tail_ = 0;
  depth_ = depth;
  depth1_ = depth + 1;
  // allow space for one extra object in the queue
  // so we can distinguish full and empty status
  storage_ = new WAVEHDR *[depth1_];
}

fhandler_dev_dsp::Audio::queue::~queue ()
{
  delete[] storage_;
}

bool
fhandler_dev_dsp::Audio::queue::send (WAVEHDR *x)
{
  if (query () == depth_)
    return false;
  storage_[tail_] = x;
  tail_++;
  if (tail_ == depth1_)
    tail_ = 0;
  return true;
}

bool
fhandler_dev_dsp::Audio::queue::recv (WAVEHDR **x)
{
  if (query () == 0)
    return false;
  *x = storage_[head_];
  head_++;
  if (head_ == depth1_)
    head_ = 0;
  return true;
}

int
fhandler_dev_dsp::Audio::queue::query ()
{
  int n = tail_ - head_;
  if (n < 0)
    n += depth1_;
  return n;
}

// Audio class implements functionality need for both read and write
fhandler_dev_dsp::Audio::Audio ()
{
  InitializeCriticalSection (&lock_);
  convert_ = &fhandler_dev_dsp::Audio::convert_none;
  owner_ = 0L;
}

fhandler_dev_dsp::Audio::~Audio ()
{
  DeleteCriticalSection (&lock_);
}

void
fhandler_dev_dsp::Audio::fork_fixup (HANDLE parent)
{
  debug_printf ("parent=0x%08x", parent);
  InitializeCriticalSection (&lock_);
}

bool
fhandler_dev_dsp::Audio::denyAccess ()
{
  if (owner_ == 0L)
    return false;
  return (GetCurrentProcessId () != owner_);
}

void
fhandler_dev_dsp::Audio::setformat (int format)
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
fhandler_dev_dsp::Audio_out::Audio_out (): Audio ()
{
  bigwavebuffer_ = NULL;
  Qisr2app_ = new queue (MAX_BLOCKS);
  Qapp2app_ = new queue (MAX_BLOCKS);
}

fhandler_dev_dsp::Audio_out::~Audio_out ()
{
  stop ();
  delete Qapp2app_;
  delete Qisr2app_;
}

bool
fhandler_dev_dsp::Audio_out::query (int rate, int bits, int channels)
{
  WAVEFORMATEX format;
  MMRESULT rc;

  fillFormat (&format, rate, bits, channels);
  rc = waveOutOpen (NULL, WAVE_MAPPER, &format, 0L, 0L, WAVE_FORMAT_QUERY);
  debug_printf ("freq=%d bits=%d channels=%d %s", rate, bits, channels,
		(rc != MMSYSERR_NOERROR) ? "FAIL" : "OK");
  return (rc == MMSYSERR_NOERROR);
}

bool
fhandler_dev_dsp::Audio_out::start (int rate, int bits, int channels)
{
  WAVEFORMATEX format;
  MMRESULT rc;
  unsigned bSize = blockSize (rate, bits, channels);
  bigwavebuffer_ = new char[MAX_BLOCKS * bSize];
  if (bigwavebuffer_ == NULL)
    return false;

  int nDevices = waveOutGetNumDevs ();
  debug_printf ("number devices=%d, blocksize=%d", nDevices, bSize);
  if (nDevices <= 0)
    return false;

  fillFormat (&format, rate, bits, channels);
  rc = waveOutOpen (&dev_, WAVE_MAPPER, &format, (DWORD) waveOut_callback,
		     (DWORD) this, CALLBACK_FUNCTION);
  if (rc == MMSYSERR_NOERROR)
    {
      setOwner ();
      init (bSize);
    }

  debug_printf ("freq=%d bits=%d channels=%d %s", rate, bits, channels,
		(rc != MMSYSERR_NOERROR) ? "FAIL" : "OK");
  
  return (rc == MMSYSERR_NOERROR);
}

void
fhandler_dev_dsp::Audio_out::stop ()
{
  MMRESULT rc;
  WAVEHDR *pHdr;
  bool gotblock;

  debug_printf ("dev_=%08x pid=%d owner=%d", (int)dev_,
		GetCurrentProcessId (), getOwner ());
  if (getOwner () && !denyAccess ())
    {
      sendcurrent ();		// force out last block whatever size..
      waitforallsent ();        // block till finished..

      rc = waveOutReset (dev_);
      debug_printf ("waveOutReset rc=%d", rc);
      do
	{
	  lock ();
	  gotblock = Qisr2app_->recv (&pHdr);
	  unlock ();
	  if (gotblock)
	    {
	      rc = waveOutUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR));
	      debug_printf ("waveOutUnprepareHeader Block 0x%08x %s", pHdr,
			    (rc != MMSYSERR_NOERROR) ? "FAIL" : "OK");
	    }
	}
      while (gotblock);
      while (Qapp2app_->recv (&pHdr))
	/* flush queue */;

      rc = waveOutClose (dev_);
      debug_printf ("waveOutClose rc=%d", rc);

      clearOwner ();

      if (bigwavebuffer_)
	{
	  delete[] bigwavebuffer_;
	  bigwavebuffer_ = NULL;
	}
    }
}

void
fhandler_dev_dsp::Audio_out::init (unsigned blockSize)
{
  int i;

  // internally queue all of our buffer for later use by write
  for (i = 0; i < MAX_BLOCKS; i++)
    {
      wavehdr_[i].lpData = &bigwavebuffer_[i * blockSize];
      (int)wavehdr_[i].dwUser = blockSize;
      if (!Qapp2app_->send (&wavehdr_[i]))
	{
	  debug_printf ("Internal Error i=%d", i);
	  break; // should not happen
	}
    }
  pHdr_ = NULL;
}

bool
fhandler_dev_dsp::Audio_out::write (const char *pSampleData, int nBytes)
{
  while (nBytes != 0)
    { // Block if all blocks used until at least one is free
      waitforspace ();

      int sizeleft = (int)pHdr_->dwUser - bufferIndex_;
      if (nBytes < sizeleft)
	{ // all data fits into the current block, with some space left
	  memcpy (&pHdr_->lpData[bufferIndex_], pSampleData, nBytes);
	  bufferIndex_ += nBytes;
	  break;
	}
      else
	{ // data will fill up the current block
	  memcpy (&pHdr_->lpData[bufferIndex_], pSampleData, sizeleft);
	  bufferIndex_ += sizeleft;
	  sendcurrent ();
	  pSampleData += sizeleft;
	  nBytes -= sizeleft;
	}
    }
  return true;
}

// return number of (partially) empty blocks back.
int
fhandler_dev_dsp::Audio_out::emptyblocks ()
{
  int n;
  lock ();
  n = Qisr2app_->query ();
  unlock ();
  n += Qapp2app_->query ();
  if (pHdr_ != NULL)
    n++;
  return n;
}

void
fhandler_dev_dsp::Audio_out::buf_info (audio_buf_info *p,
				       int rate, int bits, int channels)
{
  p->fragstotal = MAX_BLOCKS;
  p->fragsize = blockSize (rate, bits, channels);
  if (getOwner ())
    {
      p->fragments = emptyblocks ();
      if (pHdr_ != NULL)
	p->bytes = p->fragsize - bufferIndex_ +
	  p->fragsize * (p->fragments - 1);
      else
	p->bytes = p->fragsize * p->fragments;
    }
  else
    {
      p->fragments = MAX_BLOCKS;
      p->bytes = p->fragsize * p->fragments;
    }
}

/* This is called on an interupt so use locking.. Note Qisr2app_
   is used so we should wrap all references to it in locks. */
void
fhandler_dev_dsp::Audio_out::callback_sampledone (WAVEHDR *pHdr)
{
  lock ();
  Qisr2app_->send (pHdr);
  unlock ();
}

void
fhandler_dev_dsp::Audio_out::waitforspace ()
{
  WAVEHDR *pHdr;
  bool gotblock;
  MMRESULT rc = WAVERR_STILLPLAYING;

  if (pHdr_ != NULL)
    return;
  while (Qapp2app_->recv (&pHdr) == false)
    {
      lock ();
      gotblock = Qisr2app_->recv (&pHdr);
      unlock ();
      if (gotblock)
	{
	  if ((pHdr->dwFlags & WHDR_DONE)
	      && ((rc = waveOutUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR)))
	       == MMSYSERR_NOERROR))
	    {
	      Qapp2app_->send (pHdr);
	    }
	  else
	    {
	      debug_printf ("error UnprepareHeader 0x%08x, rc=%d, 100ms",
			    pHdr, rc);
	      lock ();
	      Qisr2app_->send (pHdr);
	      unlock ();
	      Sleep (100);
	    }
	}
      else
	{
	  debug_printf ("100ms");
	  Sleep (100);
	}
    }
  pHdr_ = pHdr;
  bufferIndex_ = 0;
}

void
fhandler_dev_dsp::Audio_out::waitforallsent ()
{
  while (emptyblocks () != MAX_BLOCKS)
    {
      debug_printf ("100ms Qisr=%d Qapp=%d",
		    Qisr2app_->query (), Qapp2app_->query ());
      Sleep (100);
    }
}

// send the block described by pHdr_ and bufferIndex_ to wave device
bool
fhandler_dev_dsp::Audio_out::sendcurrent ()
{
  WAVEHDR *pHdr = pHdr_;
  if (pHdr_ == NULL)
    return false;
  pHdr_ = NULL;

  // Sample buffer conversion
  (this->*convert_) ((unsigned char *)pHdr->lpData, bufferIndex_);

  // Send internal buffer out to the soundcard
  pHdr->dwBufferLength = bufferIndex_;
  pHdr->dwFlags = 0;
  if (waveOutPrepareHeader (dev_, pHdr, sizeof (WAVEHDR)) == MMSYSERR_NOERROR)
    {
      if (waveOutWrite (dev_, pHdr, sizeof (WAVEHDR)) == MMSYSERR_NOERROR)
	{
	  debug_printf ("waveOutWrite bytes=%d", bufferIndex_);
	  return true;
	}
      else
	{
	  debug_printf ("waveOutWrite failed");
	  lock ();
	  Qisr2app_->send (pHdr);
	  unlock ();
	}
    }
  else
    {
      debug_printf ("waveOutPrepareHeader failed");
      Qapp2app_->send (pHdr);
    }
  return false;
}

//------------------------------------------------------------------------
// Call back routine
static void CALLBACK
waveOut_callback (HWAVEOUT hWave, UINT msg, DWORD instance, DWORD param1,
		  DWORD param2)
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
				       int &rate, int &bits, int &channels)
{
  int len;
  const char *end = pData + nBytes;
  const char *pDat;
  int skip = 0;
  
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
		  rate = format->dwSamplesPerSec;
		  bits = format->wBitsPerSample;
		  channels = format->wChannels;
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
   WAVEHDR pointer into a queue. We need a second queue to distinguish
   blocks with data from blocks that have been unprepared and are ready
   to be used by read().
   The function read() blocks (polled, sigh) until at least one good buffer
   has arrived, then the data is copied into the buffer provided to read().
   After a buffer has been fully used by read(), it is queued again
   to the wave-in device immediately.
   The function read() iterates until all data requested has been
   received, there is no way to interrupt it */

fhandler_dev_dsp::Audio_in::Audio_in () : Audio ()
{
  bigwavebuffer_ = NULL;
  Qisr2app_ = new queue (MAX_BLOCKS);
  Qapp2app_ = new queue (MAX_BLOCKS);
}

fhandler_dev_dsp::Audio_in::~Audio_in ()
{
  stop ();
  delete Qapp2app_;
  delete Qisr2app_;
}

bool
fhandler_dev_dsp::Audio_in::query (int rate, int bits, int channels)
{
  WAVEFORMATEX format;
  MMRESULT rc;

  fillFormat (&format, rate, bits, channels);
  rc = waveInOpen (NULL, WAVE_MAPPER, &format, 0L, 0L, WAVE_FORMAT_QUERY);
  debug_printf ("freq=%d bits=%d channels=%d %s", rate, bits, channels,
		(rc != MMSYSERR_NOERROR) ? "FAIL" : "OK");
  return (rc == MMSYSERR_NOERROR);
}

bool
fhandler_dev_dsp::Audio_in::start (int rate, int bits, int channels)
{
  WAVEFORMATEX format;
  MMRESULT rc;
  unsigned bSize = blockSize (rate, bits, channels);
  bigwavebuffer_ = new char[MAX_BLOCKS * bSize];
  if (bigwavebuffer_ == NULL)
    return false;

  int nDevices = waveInGetNumDevs ();
  debug_printf ("number devices=%d, blocksize=%d", nDevices, bSize);
  if (nDevices <= 0)
    return false;

  fillFormat (&format, rate, bits, channels);
  rc = waveInOpen (&dev_, WAVE_MAPPER, &format, (DWORD) waveIn_callback,
		   (DWORD) this, CALLBACK_FUNCTION);
  if (rc == MMSYSERR_NOERROR)
    {
      setOwner ();
      if (!init (bSize))
	{
	  stop ();
	  return false;
	}
    }

  debug_printf ("freq=%d bits=%d channels=%d %s", rate, bits, channels,
		(rc != MMSYSERR_NOERROR) ? "FAIL" : "OK");
  
  return (rc == MMSYSERR_NOERROR);
}

void
fhandler_dev_dsp::Audio_in::stop ()
{
  MMRESULT rc;
  WAVEHDR *pHdr;
  bool gotblock;

  debug_printf ("dev_=%08x pid=%d owner=%d", (int)dev_,
		GetCurrentProcessId (), getOwner ());
  if (getOwner () && !denyAccess ())
    {
      rc = waveInReset (dev_);
      /* Note that waveInReset calls our callback for all incomplete buffers.
	 Since all the win32 wave functions appear to use a common lock,
	 we must not call into the wave API from the callback.
	 Otherwise we end up in a deadlock. */
      debug_printf ("waveInReset rc=%d", rc);

      do
	{
	  lock ();
	  gotblock = Qisr2app_->recv (&pHdr);
	  unlock ();
	  if (gotblock)
	    {
	      rc = waveInUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR));
	      debug_printf ("waveInUnprepareHeader Block 0x%08x %s", pHdr,
			    (rc != MMSYSERR_NOERROR) ? "FAIL" : "OK");
	    }
	}
      while (gotblock);
      while (Qapp2app_->recv (&pHdr))
	/* flush queue */;

      rc = waveInClose (dev_);
      debug_printf ("waveInClose rc=%d", rc);

      clearOwner ();

      if (bigwavebuffer_)
	{
	  delete[] bigwavebuffer_;
	  bigwavebuffer_ = NULL;
	}
    }
}

bool
fhandler_dev_dsp::Audio_in::queueblock (WAVEHDR *pHdr)
{
  MMRESULT rc;
  pHdr->dwFlags = 0;
  rc = waveInPrepareHeader (dev_, pHdr, sizeof (WAVEHDR));
  if (rc == MMSYSERR_NOERROR)
    rc = waveInAddBuffer (dev_, pHdr, sizeof (WAVEHDR));
  debug_printf ("waveInAddBuffer Block 0x%08x %s", pHdr,
		(rc != MMSYSERR_NOERROR) ? "FAIL" : "OK");
  return (rc == MMSYSERR_NOERROR);
}

bool
fhandler_dev_dsp::Audio_in::init (unsigned blockSize)
{
  MMRESULT rc;
  int i;

  // try to queue all of our buffer for reception
  for (i = 0; i < MAX_BLOCKS; i++)
    {
      wavehdr_[i].lpData = &bigwavebuffer_[i * blockSize];
      wavehdr_[i].dwBufferLength = blockSize;
      if (!queueblock (&wavehdr_[i]))
	break;
    }
  pHdr_ = NULL;
  rc = waveInStart (dev_);
  debug_printf ("waveInStart=%d %s queued=%d",
		rc, (rc != MMSYSERR_NOERROR) ? "FAIL" : "OK", i);
  return (rc == MMSYSERR_NOERROR);
}

bool
fhandler_dev_dsp::Audio_in::read (char *pSampleData, int &nBytes)
{
  int bytes_to_read = nBytes;
  nBytes = 0;
  debug_printf ("pSampleData=%08x nBytes=%d", pSampleData, bytes_to_read);
  while (bytes_to_read != 0)
    { // Block till next sound has been read
      waitfordata ();

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

void
fhandler_dev_dsp::Audio_in::waitfordata ()
{
  WAVEHDR *pHdr;
  bool gotblock;
  MMRESULT rc;

  if (pHdr_ != NULL)
    return;
  while (Qapp2app_->recv (&pHdr) == false)
    {
      lock ();
      gotblock = Qisr2app_->recv (&pHdr);
      unlock ();
      if (gotblock)
	{
	  rc = waveInUnprepareHeader (dev_, pHdr, sizeof (WAVEHDR));
	  if (rc == MMSYSERR_NOERROR)
	    Qapp2app_->send (pHdr);
	  else
	    debug_printf ("error UnprepareHeader 0x%08x", pHdr);
	}
      else
	{
	  debug_printf ("100ms");
	  Sleep (100);
	}
    }
  pHdr_ = pHdr;
  bufferIndex_ = 0; 
}

// This is called on an interrupt so use locking..
void
fhandler_dev_dsp::Audio_in::callback_blockfull (WAVEHDR *pHdr)
{
  lock ();
  Qisr2app_->send (pHdr);
  unlock ();
}

static void CALLBACK
waveIn_callback (HWAVEIN hWave, UINT msg, DWORD instance, DWORD param1,
		 DWORD param2)
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
   ------------------------------------------------------------------------
   instances of the handler statics */
int fhandler_dev_dsp::open_count = 0;

fhandler_dev_dsp::fhandler_dev_dsp ():
  fhandler_base ()
{
  debug_printf ("0x%08x", (int)this);
  audio_in_ = NULL;
  audio_out_ = NULL;
}

fhandler_dev_dsp::~fhandler_dev_dsp ()
{
  close ();
  debug_printf ("0x%08x end", (int)this);
}

int
fhandler_dev_dsp::open (int flags, mode_t mode)
{
  open_count++;
  if (open_count > 1)
    {
      set_errno (EBUSY);
      return 0;
    }
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  // Work out initial sample format & frequency, /dev/dsp defaults
  audioformat_ = AFMT_U8;
  audiofreq_ = 8000;
  audiobits_ = 8;
  audiochannels_ = 1;
  switch (flags & O_ACCMODE)
    {
    case O_WRONLY:
      audio_out_ = new Audio_out;
      if (!audio_out_->query (audiofreq_, audiobits_, audiochannels_))
	{
	  delete audio_out_;
	  audio_out_ = NULL;
	}
      break;
    case O_RDONLY:
      audio_in_ = new Audio_in;
      if (!audio_in_->query (audiofreq_, audiobits_, audiochannels_)) 
	{
	  delete audio_in_;
	  audio_in_ = NULL;
	}
      break;
    case O_RDWR:
      audio_out_ = new Audio_out;
      if (audio_out_->query (audiofreq_, audiobits_, audiochannels_))
	{
	  audio_in_ = new Audio_in;
	  if (!audio_in_->query (audiofreq_, audiobits_, audiochannels_))
	    {
	      delete audio_in_;
	      audio_in_ = NULL;
	      audio_out_->stop ();
	      delete audio_out_;
	      audio_out_ = NULL;
	    }
	}
      else
	{
	  delete audio_out_;
	  audio_out_ = NULL;
	}
      break;
    default:
      set_errno (EINVAL);
      return 0;
    } // switch (flags & O_ACCMODE)
  int rc;
  if (audio_in_ || audio_out_)
    { /* All tried query () succeeded */
      rc = 1;
      set_open_status ();
      set_need_fork_fixup ();
      set_close_on_exec_flag (1);
    }
  else
    { /* One of the tried query () failed */
      rc = 0;
      set_errno (EIO);
    }
  debug_printf ("ACCMODE=0x%08x audio_in=%08x audio_out=%08x, rc=%d",
                flags & O_ACCMODE, (int)audio_in_, (int)audio_out_, rc);
  return rc;
}

#define RETURN_ERROR_WHEN_BUSY(audio)\
  if ((audio)->denyAccess ())    \
    {\
      set_errno (EBUSY);\
      return -1;\
    }

int
fhandler_dev_dsp::write (const void *ptr, size_t len)
{
  int len_s = len; 
  debug_printf ("ptr=%08x len=%d", ptr, len);
  if (!audio_out_)
    {
      set_errno (EACCES); // device was opened for read?
      return -1;
    }
  RETURN_ERROR_WHEN_BUSY (audio_out_);
  if (audio_out_->getOwner () == 0L)
    { // No owner yet, lets do it
      // check for wave file & get parameters & skip header if possible.
      if (audio_out_->parsewav ((const char *) ptr, len_s,
				audiofreq_, audiobits_, audiochannels_))
	{ // update our format conversion
	  audioformat_ = ((audiobits_ == 8) ? AFMT_U8 : AFMT_S16_LE);
	  audio_out_->setformat (audioformat_);
	}
      // Open audio device properly with callbacks.
      if (!audio_out_->start (audiofreq_, audiobits_, audiochannels_))
	{
	  set_errno (EIO);
	  return -1;
	}
    }

  audio_out_->write ((char *)ptr, len_s);
  return len;
}

void __stdcall
fhandler_dev_dsp::read (void *ptr, size_t& len)
{
  debug_printf ("ptr=%08x len=%d", ptr, len);
  if (!audio_in_)
    {
      len = (size_t)-1;
      set_errno (EACCES); // device was opened for write?
      return;
    }
  if (audio_in_->denyAccess ())
    {
      len = (size_t)-1;
      set_errno (EBUSY);
      return;
    }
  if (audio_in_->getOwner () == 0L)
    { // No owner yet. Let's take it
      // Open audio device properly with callbacks.
      if (!audio_in_->start (audiofreq_, audiobits_, audiochannels_))
	{
	  len = (size_t)-1;
	  set_errno (EIO);
	  return;
	}
    }
  audio_in_->read ((char *)ptr, (int&)len);
  return;
}

_off64_t
fhandler_dev_dsp::lseek (_off64_t offset, int whence)
{
  return 0;
}

int
fhandler_dev_dsp::close (void)
{
  debug_printf ("audio_in=%08x audio_out=%08x",
		(int)audio_in_, (int)audio_out_);
  if (audio_in_)
    {
      delete audio_in_;
      audio_in_ = NULL;
    }
  if (audio_out_)
    {
      delete audio_out_;
      audio_out_ = NULL;
    }
  if (open_count > 0)
    open_count--;
  return 0;
}

int
fhandler_dev_dsp::dup (fhandler_base * child)
{
  debug_printf ("");
  fhandler_dev_dsp *fhc = (fhandler_dev_dsp *) child;

  fhc->set_flags (get_flags ());
  fhc->audiochannels_ = audiochannels_;
  fhc->audiobits_ = audiobits_;
  fhc->audiofreq_ = audiofreq_;
  fhc->audioformat_ = audioformat_;
  return 0;
}

int
fhandler_dev_dsp::ioctl (unsigned int cmd, void *ptr)
{
  int *intptr = (int *) ptr;
  debug_printf ("audio_in=%08x audio_out=%08x",
		(int)audio_in_, (int)audio_out_);
  switch (cmd)
    {
#define CASE(a) case a : debug_printf ("/dev/dsp: ioctl %s", #a);

      CASE (SNDCTL_DSP_RESET)
	if (audio_out_)
	  {
	    RETURN_ERROR_WHEN_BUSY (audio_out_);
	    audioformat_ = AFMT_U8;
	    audiofreq_ = 8000;
	    audiobits_ = 8;
	    audiochannels_ = 1;
	    audio_out_->stop ();
	    audio_out_->setformat (audioformat_);
	  }
	if (audio_in_)
	  {
	    RETURN_ERROR_WHEN_BUSY (audio_in_);
	    audioformat_ = AFMT_U8;
	    audiofreq_ = 8000;
	    audiobits_ = 8;
	    audiochannels_ = 1;
	    audio_in_->stop ();
	    audio_in_->setformat (audioformat_);
	  }
	return 0;
	break;

      CASE (SNDCTL_DSP_GETBLKSIZE)
	if (audio_out_)
	  {
	    *intptr = audio_out_->blockSize (audiofreq_,
					     audiobits_,
					     audiochannels_);
	  }
	else
	  { // I am very sure that audio_in_ is valid
	    *intptr = audio_in_->blockSize (audiofreq_,
					    audiobits_,
					    audiochannels_);
	  }
	return 0;
	break;

      CASE (SNDCTL_DSP_SETFMT)
      {
	int nBits;
	switch (*intptr)
	  {
	  case AFMT_QUERY:
	    *intptr = audioformat_;
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
	if (nBits && audio_out_)
	  {
	    RETURN_ERROR_WHEN_BUSY (audio_out_);
	    audio_out_->stop ();
	    audio_out_->setformat (*intptr);
	    if (audio_out_->query (audiofreq_, nBits, audiochannels_))
	      {
		audiobits_ = nBits;
		audioformat_ = *intptr;
	      }
	    else
	      {
		*intptr = audiobits_;
		return -1;
	      }
	  }
	if (nBits && audio_in_)
	  {
	    RETURN_ERROR_WHEN_BUSY (audio_in_);
	    audio_in_->stop ();
	    audio_in_->setformat (*intptr);
	    if (audio_in_->query (audiofreq_, nBits, audiochannels_))
	      {
		audiobits_ = nBits;
		audioformat_ = *intptr;
	      }
	    else
	      {
		*intptr = audiobits_;
		return -1;
	      }
	  }
	return 0;
      }
      break;

      CASE (SNDCTL_DSP_SPEED)
      {
	if (audio_out_)
	  {	    
	    RETURN_ERROR_WHEN_BUSY (audio_out_);
	    audio_out_->stop ();
	    if (audio_out_->query (*intptr, audiobits_, audiochannels_))
	      audiofreq_ = *intptr;
	    else
	      {
		*intptr = audiofreq_;
		return -1;
	      }
	  }
	if (audio_in_)
	  {
	    RETURN_ERROR_WHEN_BUSY (audio_in_);
	    audio_in_->stop ();
	    if (audio_in_->query (*intptr, audiobits_, audiochannels_))
	      audiofreq_ = *intptr;
	    else
	      {
		*intptr = audiofreq_;
		return -1;
	      }
	  }
	return 0;
      }
      break;

      CASE (SNDCTL_DSP_STEREO)
      {
	int nChannels = *intptr + 1;

	if (audio_out_)
	  {	    
	    RETURN_ERROR_WHEN_BUSY (audio_out_);
	    audio_out_->stop ();
	    if (audio_out_->query (audiofreq_, audiobits_, nChannels))
	      audiochannels_ = nChannels;
	    else
	      {
		*intptr = audiochannels_ - 1;
		return -1;
	      }
	  }
	if (audio_in_)
	  {
	    RETURN_ERROR_WHEN_BUSY (audio_in_);
	    audio_in_->stop ();
	    if (audio_in_->query (audiofreq_, audiobits_, nChannels))
	      audiochannels_ = nChannels;
	    else
	      {
		*intptr = audiochannels_ - 1;
		return -1;
	      }
	  }
	return 0;
      }
      break;

      CASE (SNDCTL_DSP_GETOSPACE)
      {
	audio_buf_info *p = (audio_buf_info *) ptr;
	if (audio_out_)
	  {
	    RETURN_ERROR_WHEN_BUSY (audio_out_);
	    audio_out_->buf_info (p, audiofreq_, audiobits_, audiochannels_);
	    debug_printf ("ptr=%p frags=%d fragsize=%d bytes=%d",
			  ptr, p->fragments, p->fragsize, p->bytes);
	  }
	return 0;
      }
      break;

      CASE (SNDCTL_DSP_SETFRAGMENT)
      {
	// Fake!! esound & mikmod require this on non PowerPC platforms.
	//
	return 0;
      }
      break;

      CASE (SNDCTL_DSP_GETFMTS)
      {
	*intptr = AFMT_S16_LE | AFMT_U8; // only native formats returned here
	return 0;
      }
      break;

      CASE (SNDCTL_DSP_POST)
      CASE (SNDCTL_DSP_SYNC)
      {
	if (audio_out_)
	  {
	    // Stop audio out device
	    RETURN_ERROR_WHEN_BUSY (audio_out_);
	    audio_out_->stop ();
	  }
	if (audio_in_)
	  {
	    // Stop audio in device
	    RETURN_ERROR_WHEN_BUSY (audio_in_);
	    audio_in_->stop ();
	  }
	return 0;
      }
      break;

    default:
      debug_printf ("/dev/dsp: ioctl 0x%08x not handled yet! FIXME:", cmd);
      break;

#undef CASE
    };
  set_errno (EINVAL);
  return -1;
}

void
fhandler_dev_dsp::dump ()
{
  paranoid_printf ("here");
}

void
fhandler_dev_dsp::fixup_after_fork (HANDLE parent)
{ // called from new child process
  debug_printf ("audio_in=%08x audio_out=%08x",
		(int)audio_in_, (int)audio_out_);
  if (audio_in_ )
    audio_in_ ->fork_fixup (parent);
  if (audio_out_)
    audio_out_->fork_fixup (parent);
}

void
fhandler_dev_dsp::fixup_after_exec ()
{
  debug_printf ("audio_in=%08x audio_out=%08x",
		(int)audio_in_, (int)audio_out_);
}


