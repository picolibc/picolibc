/* fhandler_dev_dsp: code to emulate OSS sound model /dev/dsp

   Copyright 2001 Red Hat, Inc

   Written by Andy Younger (andy@snoogie.demon.co.uk)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <errno.h>
#include "cygerrno.h"
#include "fhandler.h"
#include <windows.h>
#include <sys/soundcard.h>
#include <sys/fcntl.h>
#include <mmsystem.h>

//------------------------------------------------------------------------
// Simple encapsulation of the win32 audio device.
// 
static void CALLBACK wave_callback(HWAVE hWave, UINT msg, DWORD instance, 
			           DWORD param1, DWORD param2);
class Audio
{
public:
  enum { MAX_BLOCKS = 8, BLOCK_SIZE = 16384 };

  Audio ();
  ~Audio ();
  
  bool open (int rate, int bits, int channels, bool bCallback = false);
  void close ();
  int getvolume ();
  void setvolume (int newVolume);
  bool write (const void *pSampleData, int nBytes);
  int blocks ();
  void callback_sampledone (void *pData);
  
  int numbytesoutput ();
  
private:
  char *initialisebuffer ();
  void waitforcallback ();
  bool flush ();
   
  HWAVEOUT dev_;
  volatile int nBlocksInQue_;
  int nBytesWritten_;
  char *buffer_;
  int bufferIndex_;
  CRITICAL_SECTION lock_;
  char *freeblocks_[MAX_BLOCKS];

  char bigwavebuffer_[MAX_BLOCKS * BLOCK_SIZE];
};

Audio::Audio()
{
  int size = BLOCK_SIZE + sizeof(WAVEHDR);
  
  InitializeCriticalSection(&lock_);
  memset(freeblocks_, 0, sizeof(freeblocks_));
  for (int i = 0; i < MAX_BLOCKS; i++)
    {
      char *pBuffer = &bigwavebuffer_[ i * size ];
      memset(pBuffer, 0, size);
      freeblocks_[i] = pBuffer;
    }
}

Audio::~Audio()
{
  if (dev_)
    close();
  DeleteCriticalSection(&lock_);
}

bool 
Audio::open(int rate, int bits, int channels, bool bCallback = false)
{
  WAVEFORMATEX format;
  int nDevices = waveOutGetNumDevs();
  
  nBytesWritten_ = 0L;
  bufferIndex_ = 0;
  buffer_ = 0L;
  debug_printf("number devices %d\n", nDevices); 
  if (nDevices <= 0)
    return false;
  
  debug_printf("trying to map device freq %d, bits %d, "
	       "channels %d, callback %d\n", rate, bits, channels, 
	       bCallback); 

  int bytesperSample = bits / 8;
  
  memset(&format, 0, sizeof(format));
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.wBitsPerSample = bits;
  format.nChannels = channels;
  format.nSamplesPerSec = rate;
  format.nAvgBytesPerSec = format.nSamplesPerSec * format.nChannels * 
			   bytesperSample;
  format.nBlockAlign = format.nChannels * bytesperSample;  

  nBlocksInQue_ = 0;
  HRESULT res = waveOutOpen(&dev_, WAVE_MAPPER, &format, (DWORD)wave_callback,
			    (DWORD)this, bCallback ? CALLBACK_FUNCTION : 0);
  if (res == S_OK)
    {
      debug_printf("Sucessfully opened!");
      return true;
    }
  else
    {
      debug_printf("failed to open");
      return false;
    }
}

void 
Audio::close()
{
  if (dev_)
    {	
      flush();		// force out last block whatever size..	

      while (blocks())	// block till finished..
	waitforcallback();
      
      waveOutReset(dev_);
      waveOutClose(dev_);
      dev_ = 0L;
    }
  nBytesWritten_ = 0L;
}

int
Audio::numbytesoutput()
{
  return nBytesWritten_;
}

int 
Audio::getvolume()
{
  DWORD volume;
  
  waveOutGetVolume(dev_, &volume);
  return ((volume >> 16) + (volume & 0xffff)) >> 1;    
}

void 
Audio::setvolume(int newVolume)
{
  waveOutSetVolume(dev_, (newVolume<<16)|newVolume);    
}

char *
Audio::initialisebuffer()
{
  EnterCriticalSection(&lock_);
  WAVEHDR *pHeader = 0L;
  for (int i = 0; i < MAX_BLOCKS; i++)
    {
      char *pData = freeblocks_[i];
      if (pData)
	{
	  pHeader = (WAVEHDR *)pData;    
	  if (pHeader->dwFlags & WHDR_DONE)
	    {
	      waveOutUnprepareHeader(dev_, pHeader, sizeof(WAVEHDR));   
	    }	
	  freeblocks_[i] = 0L;
	  break;
	}
    }
  LeaveCriticalSection(&lock_); 

  if (pHeader)
    {
      memset(pHeader, 0, sizeof(WAVEHDR));
      pHeader->dwBufferLength = BLOCK_SIZE;
      pHeader->lpData = (LPSTR)(&pHeader[1]);
      return (char *)pHeader->lpData;
    }
  return 0L;
}

bool 
Audio::write(const void *pSampleData, int nBytes)
{
  // split up big blocks into smaller BLOCK_SIZE chunks
  while (nBytes > BLOCK_SIZE)
    {
      write(pSampleData, BLOCK_SIZE);
      nBytes -= BLOCK_SIZE;
      pSampleData = (void *)((char *)pSampleData + BLOCK_SIZE);
    }

  // Block till next sound is flushed
  if (blocks() == MAX_BLOCKS)
      waitforcallback();

  // Allocate new wave buffer if necessary
  if (buffer_ == 0L)
    {
      buffer_ = initialisebuffer();
      if (buffer_ == 0L)
	return false;
    }
  

  // Handle gathering blocks into larger buffer
  int sizeleft = BLOCK_SIZE - bufferIndex_;
  if (nBytes < sizeleft)
    {
      memcpy(&buffer_[bufferIndex_], pSampleData, nBytes);
      bufferIndex_ += nBytes;
      nBytesWritten_ += nBytes;
      return true;
    }
  
  // flushing when we reach our limit of BLOCK_SIZE 
  memcpy(&buffer_[bufferIndex_], pSampleData, sizeleft);
  bufferIndex_ += sizeleft;
  nBytesWritten_ += sizeleft;	
  flush();
      
  // change pointer to rest of sample, and size accordingly
  pSampleData = (void *)((char *)pSampleData + sizeleft);
  nBytes -= sizeleft;	

  // if we still have some sample left over write it out
  if (nBytes)
    return write(pSampleData, nBytes);
  
  return true;
}

// return number of blocks back.
int 
Audio::blocks()
{
  EnterCriticalSection(&lock_);
  int ret = nBlocksInQue_;
  LeaveCriticalSection(&lock_);
  return ret;
}

// This is called on an interupt so use locking.. Note nBlocksInQue_ is 
// modified by it so we should wrap all references to it in locks.
void 
Audio::callback_sampledone(void *pData)
{
  EnterCriticalSection(&lock_);
  
  nBlocksInQue_--; 
  for (int i = 0; i < MAX_BLOCKS; i++)
    if (!freeblocks_[i])
      {
	freeblocks_[i] = (char *)pData;
	break;
      }
  
  LeaveCriticalSection(&lock_);  
}

void
Audio::waitforcallback()
{
  int n = blocks();
  if (!n)
      return;
  do
    {
      Sleep(250);
    }
  while (n == blocks());
}

bool
Audio::flush()
{
  if (!buffer_)
      return false;
  
  // Send internal buffer out to the soundcard
  WAVEHDR *pHeader = ((WAVEHDR *)buffer_) - 1;
  pHeader->dwBufferLength = bufferIndex_;
  if (waveOutPrepareHeader(dev_, pHeader, sizeof(WAVEHDR)) == S_OK &&
      waveOutWrite(dev_, pHeader, sizeof (WAVEHDR)) == S_OK)
    {	    
      EnterCriticalSection(&lock_);
      nBlocksInQue_++;
      LeaveCriticalSection(&lock_);
      bufferIndex_ = 0;
      buffer_ = 0L;
      return true;
    }
  else
    {
      EnterCriticalSection(&lock_);
      for (int i = 0; i < MAX_BLOCKS; i++)
	if (!freeblocks_[i])
	{
	  freeblocks_[i] = (char *)pHeader;
	  break;
        }
      LeaveCriticalSection(&lock_);
    }
  return false;
}

//------------------------------------------------------------------------
// Call back routine
static void CALLBACK 
wave_callback(HWAVE hWave, UINT msg, DWORD instance, DWORD param1, DWORD param2)
{
  if (msg == WOM_DONE)
    {
      Audio *ptr = (Audio *)instance;
      ptr->callback_sampledone((void *)param1);
    }
}
  
//------------------------------------------------------------------------
// /dev/dsp handler
static Audio s_audio;	      // static instance of the Audio handler

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
fhandler_dev_dsp::setupwav(const char *pData, int nBytes)
{
  int len;
  const char *end = pData + nBytes;

  if (!(pData[0] == 'R' && pData[1] == 'I' && 
	pData[2] == 'F' && pData[3] == 'F'))
    return false;
  if (!(pData[8]  == 'W' && pData[9]  == 'A' && 
        pData[10] == 'V' && pData[11] == 'E'))
    return false;
  
  len = *(int *)&pData[4];  
  pData += 12;
  while (len && pData < end)
    { 
      wavchunk *pChunk = (wavchunk *)pData;
      int blklen = pChunk->len;	      
      if (pChunk->id[0] == 'f' && pChunk->id[1] == 'm' && 
	  pChunk->id[2] == 't' && pChunk->id[3] == ' ')
	{
	  wavformat *format = (wavformat *)(pChunk+1);
	  if ((char *)(format+1) > end)
	    return false;
      
	  // Open up audio device with correct frequency for wav file
	  // 
	  // FIXME: should through away all the header & not output
	  // it to the soundcard.
	  s_audio.close();
	  if (s_audio.open(format->dwSamplesPerSec, format->wBitsPerSample,
			   format->wChannels) == false)
	    {
	      s_audio.open(audiofreq_, audiobits_, audiochannels_);
	    }
	  else
	    {
	      audiofreq_ = format->dwSamplesPerSec;
	      audiobits_ = format->wBitsPerSample;
	      audiochannels_ = format->wChannels;
	    }
	  return true;
	}

      pData += blklen + sizeof(wavchunk);
    }
  return false;
}

//------------------------------------------------------------------------
fhandler_dev_dsp::fhandler_dev_dsp (const char *name)
  : fhandler_base (FH_OSS_DSP, name)
{
  set_cb (sizeof *this);     
}

fhandler_dev_dsp::~fhandler_dev_dsp()
{
}

int
fhandler_dev_dsp::open (const char *path, int flags, mode_t mode = 0)
{
  // currently we only support writing    
  if ((flags & (O_WRONLY|O_RDONLY|O_RDWR)) != O_WRONLY)
    return 0;
  
  set_flags(flags);

  // Work out initial sample format & frequency
  if (strcmp(path, "/dev/dsp") == 0L)
    {
      // dev/dsp defaults
      audioformat_ = AFMT_S8;
      audiofreq_ = 8000;
      audiobits_ = 8;
      audiochannels_ = 1;
    }

  if (s_audio.open(audiofreq_, audiobits_, audiochannels_))
    debug_printf("/dev/dsp: successfully opened\n"); 
  else
    debug_printf("/dev/dsp: failed to open\n");   
  return 1;
}

int
fhandler_dev_dsp::write (const void *ptr, size_t len)
{
  if (s_audio.numbytesoutput() == 0)
    {
      // check for wave file & setup frequencys properly if possible.
      setupwav((const char *)ptr, len);
 
      // Open audio device properly with callbacks.
      s_audio.close();
      if (!s_audio.open(audiofreq_, audiobits_, audiochannels_, true))
        return 0;
    }
  
  s_audio.write(ptr, len);
  return len;
}

int
fhandler_dev_dsp::read (void *ptr, size_t len)
{
  return len;
}

off_t
fhandler_dev_dsp::lseek (off_t offset, int whence)
{
  return 0;
}

int
fhandler_dev_dsp::close (void)
{
  s_audio.close();
  return 0;
}

int
fhandler_dev_dsp::dup (fhandler_base * child)
{
  fhandler_dev_dsp *fhc = (fhandler_dev_dsp *)child;

  fhc->set_flags(get_flags());
  fhc->audiochannels_ = audiochannels_;
  fhc->audiobits_ = audiobits_;
  fhc->audiofreq_ = audiofreq_;
  fhc->audioformat_ = audioformat_;
  return 0;
}

int 
fhandler_dev_dsp::ioctl(unsigned int cmd, void *ptr)
{
  int *intptr = (int *)ptr;
  switch (cmd)
    {
      #define CASE(a) case a : debug_printf("/dev/dsp: ioctl %s\n", #a); 
      
      CASE(SNDCTL_DSP_RESET)
	  audioformat_ = AFMT_S8;
	  audiofreq_ = 8000;
	  audiobits_ = 8;
	  audiochannels_ = 1;
	  return 1;
      
      CASE(SNDCTL_DSP_GETBLKSIZE)
	  *intptr = Audio::BLOCK_SIZE;
	  break;

      CASE(SNDCTL_DSP_SETFMT)	 
	{
	  int nBits = 0;
	  if (*intptr == AFMT_S16_LE)
	      nBits = 16;
	  else if (*intptr == AFMT_S8)
	      nBits = 8;	  
	  if (nBits)
	    {
	      s_audio.close();
	      if (s_audio.open(audiofreq_, nBits, audiochannels_) == true)
		{
		  audiobits_ = nBits;
		  return 1;
		}
	      else
		{
		  s_audio.open(audiofreq_, audiobits_, audiochannels_);
		  return -1;
		}
	    }
	}  break;
      
      CASE(SNDCTL_DSP_SPEED)
	  s_audio.close();
	  if (s_audio.open(*intptr, audiobits_, audiochannels_) == true)
	    {
	      audiofreq_ = *intptr;
	      return 1;
	    }
	  else
	    {
	      s_audio.open(audiofreq_, audiobits_, audiochannels_);
	      return -1;
	    }
	  break;
      
      CASE(SNDCTL_DSP_STEREO)
	{
	  int nChannels = *intptr + 1;
	  
	  s_audio.close();
	  if (s_audio.open(audiofreq_, audiobits_, nChannels) == true)
	    {
	      audiochannels_ = nChannels;
	      return 1;
	    }
	  else
	    {
	      s_audio.open(audiofreq_, audiobits_, audiochannels_);
	      return -1;
	    }
	} break;

      CASE(SNDCTL_DSP_GETOSPACE)
	{
	  audio_buf_info *p = (audio_buf_info *)ptr;

	  int nBlocks = s_audio.blocks();
	  int leftblocks = ((Audio::MAX_BLOCKS - nBlocks)-1);
	  if (leftblocks < 0) leftblocks = 0;
	  if (leftblocks > 1)
	    leftblocks = 1;
	  int left = leftblocks * Audio::BLOCK_SIZE;
	 
	  p->fragments = leftblocks;
	  p->fragstotal = Audio::MAX_BLOCKS;
	  p->fragsize = Audio::BLOCK_SIZE;
	  p->bytes = left;

	  debug_printf("ptr: %p "
		       "nblocks: %d "
		       "leftblocks: %d "
		       "left bytes: %d ", ptr, nBlocks, leftblocks, left);
	  
	  
	  return 1;
	} break;
      
      default:
	debug_printf("/dev/dsp: ioctl not handled yet! FIXME:\n"); 
	break;

      #undef CASE
    };
  return -1;
}

void
fhandler_dev_dsp::dump ()
{
  paranoid_printf("here, fhandler_dev_dsp");
}

