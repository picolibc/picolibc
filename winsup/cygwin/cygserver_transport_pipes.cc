/* cygserver_transport_pipes.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "wincap.h"
#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_pipes.h"

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifndef __OUTSIDE_CYGWIN__
#include "winsup.h"
#else
#define DEBUG 0
#define debug_printf if (DEBUG) printf
#endif

transport_layer_pipes::transport_layer_pipes (HANDLE new_pipe)
{
  pipe = new_pipe;
  if (inited != true)
    init_security();
};

transport_layer_pipes::transport_layer_pipes () 
{
  pipe = NULL;
  strcpy(pipe_name, "\\\\.\\pipe\\cygwin_lpc");
  if (inited != true)
    init_security();
}


void
transport_layer_pipes::init_security()
{
  /* FIXME: pthread_once or equivalent needed */
  InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);
  SetSecurityDescriptorDacl (&sd, TRUE, 0, FALSE);

  sec_none_nih.nLength = sec_all_nih.nLength = sizeof (SECURITY_ATTRIBUTES);
  sec_none_nih.bInheritHandle = sec_all_nih.bInheritHandle = FALSE;
  sec_none_nih.lpSecurityDescriptor = NULL;
  sec_all_nih.lpSecurityDescriptor = &sd;
  inited = true;
}

void
transport_layer_pipes::listen ()
{
  /* no-op */
}

class transport_layer_pipes *
transport_layer_pipes::accept ()
{
  if (pipe)
    {
      debug_printf ("Already have a pipe in this %p\n",this);
      return NULL;
    }

  pipe = CreateNamedPipe (pipe_name,
                          PIPE_ACCESS_DUPLEX,
                          PIPE_TYPE_BYTE | PIPE_WAIT,
                          PIPE_UNLIMITED_INSTANCES,
                          0, 0, 1000,
                          &sec_all_nih );
  if (pipe == INVALID_HANDLE_VALUE)
    {
      debug_printf ("error creating pipe (%lu)\n.", GetLastError ());
      return NULL;
    }

  if ( !ConnectNamedPipe ( pipe, NULL ) &&
     GetLastError () != ERROR_PIPE_CONNECTED)
    {
      printf ("error connecting to pipe (%lu)\n.", GetLastError ());
      CloseHandle (pipe);
      pipe = NULL;
      return NULL;
    }
  
  transport_layer_pipes *new_conn = new transport_layer_pipes (pipe);
  pipe = NULL;

  return new_conn;  
}

void
transport_layer_pipes::close()
{
  debug_printf ("closing pipe %p\n", pipe);
  if (pipe && pipe != INVALID_HANDLE_VALUE)
    {
      FlushFileBuffers (pipe);
      DisconnectNamedPipe (pipe);
      CloseHandle (pipe);
    }
}

ssize_t
transport_layer_pipes::read (char *buf, size_t len)
{
  debug_printf ("reading from pipe %p\n", pipe);
  if (!pipe || pipe == INVALID_HANDLE_VALUE)
    return -1;

  DWORD bytes_read;
  DWORD rc = ReadFile (pipe, buf, len, &bytes_read, NULL);
  if (!rc)
    {
      debug_printf ("error reading from pipe (%lu)\n", GetLastError ());
      return -1;
    }
  return bytes_read;
}

ssize_t
transport_layer_pipes::write (char *buf, size_t len)
{
  debug_printf ("writing to pipe %p\n", pipe);
  DWORD bytes_written, rc;
  if (!pipe || pipe == INVALID_HANDLE_VALUE)
    return -1;

  rc = WriteFile (pipe, buf, len, &bytes_written, NULL);
  if (!rc)
    {
      debug_printf ("error writing to pipe (%lu)\n", GetLastError ());
      return -1;
    }
  return bytes_written;
}

bool
transport_layer_pipes::connect ()
{
  if (pipe && pipe != INVALID_HANDLE_VALUE)
    {
      debug_printf ("Already have a pipe in this %p\n",this);
      return false;
    }

  while (1)
    {
      pipe = CreateFile (pipe_name,
		         GENERIC_READ | GENERIC_WRITE,
		         FILE_SHARE_READ | FILE_SHARE_WRITE,
		         &sec_all_nih,
		         OPEN_EXISTING,
		         0, NULL);

      if (pipe != INVALID_HANDLE_VALUE)
	/* got the pipe */
        return true;

      if (GetLastError () != ERROR_PIPE_BUSY)
        {
          debug_printf ("Error opening the pipe (%lu)\n", GetLastError ());
          pipe = NULL;
          return false;
        }
      if (!WaitNamedPipe (pipe_name, 20000))
        debug_printf ( "error connecting to server pipe after 20 seconds (%lu)\n", GetLastError () );
      /* We loop here, because the pipe exists but is busy. If it doesn't exist
       * the != ERROR_PIPE_BUSY will catch it.
       */ 
    }
}

void
transport_layer_pipes::impersonate_client ()
{
  debug_printf ("impersonating pipe %p\n", pipe);
  if (pipe && pipe != INVALID_HANDLE_VALUE)
    {
      BOOL rv = ImpersonateNamedPipeClient (pipe);
      if (!rv)
	debug_printf ("Failed to Impersonate the client, (%lu)\n", GetLastError ());
    }
  debug_printf("I am who you are\n");
}

void
transport_layer_pipes::revert_to_self ()
{
  RevertToSelf (); 
  debug_printf("I am who I yam\n");
}

