/* ntea.cc: code for manipulating NTEA information

   Copyright 1997, 1998, 2000 Cygnus Solutions.

   Written by Sergey S. Okhapkin (sos@prospect.com.ru)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <stdlib.h>
#include "security.h"

/* Default to not using NTEA information */
BOOL allow_ntea = FALSE;

/*
From Windows NT DDK:

FILE_FULL_EA_INFORMATION provides extended attribute information.
This structure is used primarily by network drivers.

Members

NextEntryOffset
The offset of the next FILE_FULL_EA_INFORMATION-type entry. This member is
zero if no other entries follow this one.

Flags
Can be zero or can be set with FILE_NEED_EA, indicating that the file to which
the EA belongs cannot be interpreted without understanding the associated
extended attributes.

EaNameLength
The length in bytes of the EaName array. This value does not include a
zero-terminator to EaName.

EaValueLength
The length in bytes of each EA value in the array.

EaName
An array of characters naming the EA for this entry.

Comments
This structure is longword-aligned. If a set of FILE_FULL_EA_INFORMATION
entries is buffered, NextEntryOffset value in each entry, except the last,
falls on a longword boundary.
The value(s) associated with each entry follows the EaName array. That is, an
EA's values are located at EaName + (EaNameLength + 1).
*/

typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

/* Functions prototypes */

int NTReadEA (const char *file, const char *attrname, char *buf, int len);
static PFILE_FULL_EA_INFORMATION NTReadEARaw (HANDLE file, int *len);
BOOL NTWriteEA(const char *file, const char *attrname, char *buf, int len);

/*
 * NTReadEA - read file's Extended Attribute.
 *
 * Parameters:
 *	file	- pointer to filename
 *	attrname- pointer to EA name (case insensitivy. EAs are sored in upper
 *		  case).
 *	attrbuf - pointer to buffer to store EA's value.
 *	len	- length of attrbuf.
 * Return value:
 *	0	- if file or attribute "attrname" not found.
 *	N	- number of bytes stored in attrbuf if succes.
 *	-1	- attrbuf too small for EA value.
 */

int __stdcall
NTReadEA (const char *file, const char *attrname, char *attrbuf, int len)
{
    /* return immediately if NTEA usage is turned off */
    if (! allow_ntea)
      return FALSE;

    HANDLE hFileSource;
    int eafound = 0;
    PFILE_FULL_EA_INFORMATION ea, sea;
    int easize;

    hFileSource = CreateFile (file, FILE_READ_EA,
	FILE_SHARE_READ | FILE_SHARE_WRITE,
	&sec_none_nih, // sa
	OPEN_EXISTING,
	FILE_FLAG_BACKUP_SEMANTICS,
	NULL
	);

    if (hFileSource == INVALID_HANDLE_VALUE)
	return 0;

    /* Read in raw array of EAs */
    ea = sea = NTReadEARaw (hFileSource, &easize);

    /* Search for requested attribute */
    while (sea)
      {
	if (strcasematch (ea->EaName, attrname)) /* EA found */
	  {
	    if (ea->EaValueLength > len)
	      {
		eafound = -1;		/* buffer too small */
		break;
	      }
	    memcpy (attrbuf, ea->EaName + (ea->EaNameLength + 1),
		    ea->EaValueLength);
	    eafound = ea->EaValueLength;
	    break;
	  }
	if ((ea->NextEntryOffset == 0) || ((int) ea->NextEntryOffset > easize))
	  break;
	ea = (PFILE_FULL_EA_INFORMATION) ((char *) ea + ea->NextEntryOffset);
      }

    if (sea)
      free (sea);
    CloseHandle (hFileSource);

    return eafound;
}

/*
 * NTReadEARaw - internal routine to read EAs array to malloced buffer. The
 *		 caller should free this buffer after usage.
 * Parameters:
 *	hFileSource - handle to file. This handle should have FILE_READ_EA
 *		      rights.
 *	len	    - pointer to int variable where length of buffer will
 *		      be stored.
 * Return value:
 *	pointer to buffer with file's EAs, or NULL if any error occured.
 */

static
PFILE_FULL_EA_INFORMATION
NTReadEARaw (HANDLE hFileSource, int *len)
{
  WIN32_STREAM_ID StreamId;
  DWORD dwBytesWritten;
  LPVOID lpContext;
  DWORD StreamSize;
  PFILE_FULL_EA_INFORMATION eafound = NULL;

  lpContext = NULL;
  StreamSize = sizeof (WIN32_STREAM_ID) - sizeof (WCHAR**);

  /* Read the WIN32_STREAM_ID in */

  while (BackupRead (hFileSource, (LPBYTE) &StreamId, StreamSize,
		     &dwBytesWritten,
		     FALSE,		// don't abort yet
		     FALSE,		// don't process security
		     &lpContext))
    {
      DWORD sl,sh;

      if (dwBytesWritten == 0) /* No more Stream IDs */
	break;
      /* skip StreamName */
      if (StreamId.dwStreamNameSize)
	{
	  unsigned char *buf;
	  buf = (unsigned char *) malloc (StreamId.dwStreamNameSize);

	  if (buf == NULL)
	    break;

	  if (!BackupRead (hFileSource, buf,  // buffer to read
			   StreamId.dwStreamNameSize,   // num bytes to read
			   &dwBytesWritten,
			   FALSE,		// don't abort yet
			   FALSE,		// don't process security
			   &lpContext))		// Stream name read error
	    {
	      free (buf);
	      break;
	    }
	  free (buf);
	}

	/* Is it EA stream? */
	if (StreamId.dwStreamId == BACKUP_EA_DATA)
	  {
	    unsigned char *buf;
	    buf = (unsigned char *) malloc (StreamId.Size.LowPart);

	    if (buf == NULL)
	      break;
	    if (!BackupRead (hFileSource, buf,	// buffer to read
			     StreamId.Size.LowPart, // num bytes to write
			     &dwBytesWritten,
			     FALSE,		// don't abort yet
			     FALSE,		// don't process security
			     &lpContext))
	      {
		free (buf);	/* EA read error */
		break;
	      }
	    eafound = (PFILE_FULL_EA_INFORMATION) buf;
	    *len = StreamId.Size.LowPart;
	    break;
	}
	/* Skip current stream */
	if (!BackupSeek (hFileSource,
			 StreamId.Size.LowPart,
			 StreamId.Size.HighPart,
			 &sl,
			 &sh,
			 &lpContext))
	  break;
    }

  /* free context */
  BackupRead (
      hFileSource,
      NULL,		// buffer to write
      0,		// number of bytes to write
      &dwBytesWritten,
      TRUE,		// abort
      FALSE,		// don't process security
      &lpContext);

  return eafound;
}

/*
 * NTWriteEA - write file's Extended Attribute.
 *
 * Parameters:
 *	file	- pointer to filename
 *	attrname- pointer to EA name (case insensitivy. EAs are sored in upper
 *		  case).
 *	buf	- pointer to buffer with EA value.
 *	len	- length of buf.
 * Return value:
 *	TRUE if success, FALSE otherwice.
 * Note: if len=0 given EA will be deleted.
 */

BOOL __stdcall
NTWriteEA (const char *file, const char *attrname, char *buf, int len)
{
  /* return immediately if NTEA usage is turned off */
  if (! allow_ntea)
    return TRUE;

  HANDLE hFileSource;
  WIN32_STREAM_ID StreamId;
  DWORD dwBytesWritten;
  LPVOID lpContext;
  DWORD StreamSize, easize;
  BOOL bSuccess=FALSE;
  PFILE_FULL_EA_INFORMATION ea;

  hFileSource = CreateFile (file, FILE_WRITE_EA,
			    FILE_SHARE_READ | FILE_SHARE_WRITE,
			    &sec_none_nih, // sa
			    OPEN_EXISTING,
			    FILE_FLAG_BACKUP_SEMANTICS,
			    NULL);

  if (hFileSource == INVALID_HANDLE_VALUE)
    return FALSE;

  lpContext = NULL;
  StreamSize = sizeof (WIN32_STREAM_ID) - sizeof (WCHAR**);

  /* FILE_FULL_EA_INFORMATION structure is longword-aligned */
  easize = sizeof (*ea) - sizeof (WCHAR**) + strlen (attrname) + 1 + len
      + (sizeof (DWORD) - 1);
  easize &= ~(sizeof (DWORD) - 1);

  if ((ea = (PFILE_FULL_EA_INFORMATION) malloc (easize)) == NULL)
    goto cleanup;

  memset (ea, 0, easize);
  ea->EaNameLength = strlen (attrname);
  ea->EaValueLength = len;
  strcpy (ea->EaName, attrname);
  memcpy (ea->EaName + (ea->EaNameLength + 1), buf, len);

  StreamId.dwStreamId = BACKUP_EA_DATA;
  StreamId.dwStreamAttributes = 0;
  StreamId.Size.HighPart = 0;
  StreamId.Size.LowPart = easize;
  StreamId.dwStreamNameSize = 0;

  if (!BackupWrite (hFileSource, (LPBYTE) &StreamId, StreamSize,
		    &dwBytesWritten,
		    FALSE,		// don't abort yet
		    FALSE,		// don't process security
		    &lpContext))
    goto cleanup;

  if (!BackupWrite (hFileSource, (LPBYTE) ea, easize,
		    &dwBytesWritten,
		    FALSE,		// don't abort yet
		    FALSE,		// don't process security
		    &lpContext))
    goto cleanup;

  bSuccess = TRUE;
  /* free context */

cleanup:
  BackupRead (hFileSource,
	      NULL,			// buffer to write
	      0,			// number of bytes to write
	      &dwBytesWritten,
	      TRUE,			// abort
	      FALSE,			// don't process security
	      &lpContext);

  CloseHandle (hFileSource);
  if (ea)
    free (ea);

  return bSuccess;
}
