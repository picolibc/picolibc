/*
 * Workaround for limitations on win9x where a file contents are
 * not zero'd out if you seek past the end and then write.
 * Copied from ming local-patch to binutils/bfd/libbfd.c written by
 * Mumit Khan  <khan@xraylith.wisc.edu> 
 */

#include <windows.h>
#include <stdio.h>
#include <io.h>

#ifdef __GNUC__
# define INLINE __inline__
#elif defined _MSC_VER
# define INLINE __inline
#else 
# define INLINE
#endif

#define ZEROBLOCKSIZE 512
static int __mingw_fseek_called;

/* FIXME: put this in startup code and make os_platform_id global?
   Or just get _osver from msvcrt.dll and bitest (_osver & 0x8000)? */

INLINE 
static
int
__mingw_is_win9x (void)
{
  static DWORD os_platform_id =  -1 ;

  if (os_platform_id == -1)
    {
      OSVERSIONINFO os_version_info;
      memset (&os_version_info, 0, sizeof (OSVERSIONINFO));
      os_version_info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      GetVersionEx (&os_version_info);

      os_platform_id = os_version_info.dwPlatformId;
    }
 
  /* Don't even bother to check for Win32s. */
  return os_platform_id == VER_PLATFORM_WIN32_WINDOWS;
}

/* The fseek in Win9x runtime does not zero out the file if seeking past
   the end; if you don't want random stuff from your disk included in your
   output DLL/executable, use this version instead. On WinNT/Win2k, it
   just calls runtime fseek().  
   
   CHECK/FIXME: Does this work for both text and binary modes?? */ 


int
__mingw_fseek (FILE *fp, long offset, int whence)
{
# undef fseek 
  __mingw_fseek_called = 1;
  return fseek (fp, offset, whence);
}

int
__mingw_fwrite (const void *buffer, size_t size, size_t count, FILE *fp)
{
# undef fwrite 
  if ( __mingw_is_win9x () &&  __mingw_fseek_called)
    {
      DWORD actual_length, current_position;
      __mingw_fseek_called = 0;
      fflush (fp);
      actual_length = GetFileSize ((HANDLE) _get_osfhandle (fileno (fp)), 
                                   NULL);
      current_position = SetFilePointer ((HANDLE) _get_osfhandle (fileno (fp)),
                                         0, 0, FILE_CURRENT);
#ifdef DEBUG
      printf ("__mingw_fwrite: current %ld, actual %ld\n", 
	      current_position, actual_length);
#endif /* DEBUG */
      if (current_position > actual_length)
	{
	  static char __mingw_zeros[ZEROBLOCKSIZE];
	  long numleft;

	  SetFilePointer ((HANDLE) _get_osfhandle (fileno (fp)), 
	                  0, 0, FILE_END);
	  numleft = current_position - actual_length;

#ifdef DEBUG
	  printf ("__mingw_fwrite: Seeking %ld bytes past end\n", numleft);
#endif /* DEBUG */
	  while (numleft > 0)
	    {
	      DWORD nzeros = (numleft > ZEROBLOCKSIZE)
	                     ? ZEROBLOCKSIZE : numleft;
	      DWORD written;
	      if (! WriteFile ((HANDLE) _get_osfhandle (fileno (fp)),
	                       __mingw_zeros, nzeros, &written, NULL))
	        {
		  /* Best we can hope for, or at least DJ says so. */
	          SetFilePointer ((HANDLE) _get_osfhandle (fileno (fp)), 
	                          0, 0, FILE_BEGIN);
		  return -1;
		}
	      if (written < nzeros)
	        {
		  /* Likewise. */
	          SetFilePointer ((HANDLE) _get_osfhandle (fileno (fp)), 
	                          0, 0, FILE_BEGIN);
		  return -1;
		}

	      numleft -= written;
	    }
	    FlushFileBuffers ((HANDLE) _get_osfhandle (fileno (fp)));
	}
    }
  return fwrite (buffer, size, count, fp);
}
