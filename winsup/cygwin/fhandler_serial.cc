/* fhandler_serial.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <sys/param.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "sigproc.h"
#include "pinfo.h"
#include <asm/socket.h>
#include <devioctl.h>
#include <ntddser.h>
#include "cygwait.h"

/**********************************************************************/
/* fhandler_serial */

fhandler_serial::fhandler_serial ()
  : fhandler_base (), vmin_ (0), vtime_ (0), pgrp_ (myself->pgid)
{
  need_fork_fixup (true);
}

void
fhandler_serial::overlapped_setup ()
{
  memset (&io_status, 0, sizeof (io_status));
  io_status.hEvent = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  ProtectHandle (io_status.hEvent);
  overlapped_armed = 0;
}

void __reg3
fhandler_serial::raw_read (void *ptr, size_t& ulen)
{
  int tot;
  DWORD n;

  size_t minchars = vmin_ ? MIN (vmin_, ulen) : ulen;

  debug_printf ("ulen %ld, vmin_ %ld, vtime_ %u, hEvent %p",
		ulen, vmin_, vtime_, io_status.hEvent);
  if (!overlapped_armed)
    {
      SetCommMask (get_handle (), EV_RXCHAR);
      ResetEvent (io_status.hEvent);
    }

  for (n = 0, tot = 0; ulen; ulen -= n, ptr = (char *) ptr + n)
    {
      COMSTAT st;
      DWORD inq = vmin_ ? minchars : vtime_ ? ulen : 1;

      n = 0;

      if (vtime_) // non-interruptible -- have to use kernel timeouts
	overlapped_armed = -1;

      if (!ClearCommError (get_handle (), &ev, &st))
	goto err;
      else if (ev)
	termios_printf ("error detected %x", ev);
      else if (st.cbInQue && !vtime_)
	inq = st.cbInQue;
      else if (!is_nonblocking () && !overlapped_armed)
	{
	  if ((size_t) tot >= minchars)
	    break;
	  else if (WaitCommEvent (get_handle (), &ev, &io_status))
	    {
	      debug_printf ("WaitCommEvent succeeded: ev %x", ev);
	      if (!ev)
		continue;
	    }
	  else if (GetLastError () != ERROR_IO_PENDING)
	    goto err;
	  else
	    {
	      overlapped_armed = 1;
	      switch (cygwait (io_status.hEvent))
		{
		case WAIT_OBJECT_0:
		  if (!GetOverlappedResult (get_handle (), &io_status, &n,
					    FALSE))
		    goto err;
		  debug_printf ("n %u, ev %x", n, ev);
		  break;
		case WAIT_SIGNALED:
		  tot = -1;
		  PurgeComm (get_handle (), PURGE_RXABORT);
		  overlapped_armed = 0;
		  set_sig_errno (EINTR);
		  goto out;
		case WAIT_CANCELED:
		  PurgeComm (get_handle (), PURGE_RXABORT);
		  overlapped_armed = 0;
		  pthread::static_cancel_self ();
		  /*NOTREACHED*/
		default:
		  goto err;
		}
	    }
	}

      overlapped_armed = 0;
      ResetEvent (io_status.hEvent);
      if (inq > ulen)
	inq = ulen;
      debug_printf ("inq %u", inq);
      if (ReadFile (get_handle (), ptr, inq, &n, &io_status))
	/* Got something */;
      else if (GetLastError () != ERROR_IO_PENDING)
	goto err;
      else if (is_nonblocking ())
	{
	  /* Use CancelIo rather than PurgeComm (PURGE_RXABORT) since
	     PurgeComm apparently discards in-flight bytes while CancelIo
	     only stops the overlapped IO routine. */
	  CancelIo (get_handle ());
	  if (GetOverlappedResult (get_handle (), &io_status, &n, FALSE))
	    tot = n;
	  else if (GetLastError () != ERROR_OPERATION_ABORTED)
	    goto err;
	  if (tot == 0)
	    {
	      tot = -1;
	      set_errno (EAGAIN);
	    }
	  goto out;
	}
      else if (!GetOverlappedResult (get_handle (), &io_status, &n, TRUE))
	goto err;

      tot += n;
      debug_printf ("vtime_ %u, vmin_ %lu, n %u, tot %d", vtime_, vmin_, n, tot);
      if (vtime_ || !vmin_ || !n)
	break;
      continue;

    err:
      debug_printf ("err %E");
      if (GetLastError () != ERROR_OPERATION_ABORTED)
	{
	  PurgeComm (get_handle (), PURGE_RXABORT);
	  tot = -1;
	  __seterrno ();
	  break;
	}

      n = 0;
    }

out:
  ulen = tot;
}

/* Cover function to WriteFile to provide Posix interface and semantics
   (as much as possible).  */
ssize_t __reg3
fhandler_serial::raw_write (const void *ptr, size_t len)
{
  DWORD bytes_written;
  OVERLAPPED write_status;

  memset (&write_status, 0, sizeof (write_status));
  write_status.hEvent = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  ProtectHandle (write_status.hEvent);

  for (;;)
    {
      if (WriteFile (get_handle (), ptr, len, &bytes_written, &write_status))
	break;

      switch (GetLastError ())
	{
	case ERROR_OPERATION_ABORTED:
	  DWORD ev;
	  if (!ClearCommError (get_handle (), &ev, NULL))
	    goto err;
	  if (ev)
	    termios_printf ("error detected %x", ev);
	  continue;
	case ERROR_IO_PENDING:
	  break;
	default:
	  goto err;
	}

      if (!is_nonblocking ())
	{
	  switch (cygwait (write_status.hEvent))
	    {
	    case WAIT_OBJECT_0:
	      break;
	    case WAIT_SIGNALED:
	      PurgeComm (get_handle (), PURGE_TXABORT);
	      set_sig_errno (EINTR);
	      ForceCloseHandle (write_status.hEvent);
	      return -1;
	    case WAIT_CANCELED:
	      PurgeComm (get_handle (), PURGE_TXABORT);
	      pthread::static_cancel_self ();
	      /*NOTREACHED*/
	    default:
	      goto err;
	    }
	}
      if (!GetOverlappedResult (get_handle (), &write_status, &bytes_written, TRUE))
	goto err;

      break;
    }

  ForceCloseHandle (write_status.hEvent);

  return bytes_written;

err:
  __seterrno ();
  ForceCloseHandle (write_status.hEvent);
  return -1;
}

int
fhandler_serial::init (HANDLE f, DWORD flags, mode_t bin)
{
  return open (flags, bin & (O_BINARY | O_TEXT));
}

int
fhandler_serial::open (int flags, mode_t mode)
{
  int res;
  COMMTIMEOUTS to;

  syscall_printf ("fhandler_serial::open (%s, %y, 0%o)",
		  get_name (), flags, mode);

  if (!fhandler_base::open (flags, mode))
    return 0;

  res = 1;

  SetCommMask (get_handle (), EV_RXCHAR);

  overlapped_setup ();

  memset (&to, 0, sizeof (to));
  SetCommTimeouts (get_handle (), &to);

  /* Reset serial port to known state of 9600-8-1-no flow control
     on open for better behavior under Win 95.

     FIXME:  This should only be done when explicitly opening the com
     port.  It should not be reset if an fd is inherited.
     Using __progname in this way, to determine how far along in the
     initialization we are, is really a terrible kludge and should
     be fixed ASAP.
  */
  if (reset_com && __progname)
    {
      DCB state;
      GetCommState (get_handle (), &state);
      syscall_printf ("setting initial state on %s (reset_com %d)",
		      get_name (), reset_com);
      state.BaudRate = CBR_9600;
      state.ByteSize = 8;
      state.StopBits = ONESTOPBIT;
      state.Parity = NOPARITY; /* FIXME: correct default? */
      state.fBinary = TRUE; /* binary xfer */
      state.EofChar = 0; /* no end-of-data in binary mode */
      state.fNull = FALSE; /* don't discard nulls in binary mode */
      state.fParity = FALSE; /* ignore parity errors */
      state.fErrorChar = FALSE;
      state.fTXContinueOnXoff = TRUE; /* separate TX and RX flow control */
      state.fOutX = FALSE; /* disable transmission flow control */
      state.fInX = FALSE; /* disable reception flow control */
      state.XonChar = 0x11;
      state.XoffChar = 0x13;
      state.fOutxDsrFlow = FALSE; /* disable DSR flow control */
      state.fRtsControl = RTS_CONTROL_ENABLE; /* ignore lead control except
						  DTR */
      state.fOutxCtsFlow = FALSE; /* disable output flow control */
      state.fDtrControl = DTR_CONTROL_ENABLE; /* assert DTR */
      state.fDsrSensitivity = FALSE; /* don't assert DSR */
      state.fAbortOnError = TRUE;
      if (!SetCommState (get_handle (), &state))
	system_printf ("couldn't set initial state for %s, %E", get_name ());
    }

  SetCommMask (get_handle (), EV_RXCHAR);
  set_open_status ();
  syscall_printf ("%p = fhandler_serial::open (%s, %y, 0%o)",
		  res, get_name (), flags, mode);
  return res;
}

int
fhandler_serial::close ()
{
  ForceCloseHandle (io_status.hEvent);
  return fhandler_base::close ();
}

/* tcsendbreak: POSIX 7.2.2.1 */
/* Break for 250-500 milliseconds if duration == 0 */
/* Otherwise, units for duration are undefined */
int
fhandler_serial::tcsendbreak (int duration)
{
  unsigned int sleeptime = 300000;

  if (duration > 0)
    sleeptime *= duration;

  if (SetCommBreak (get_handle ()) == 0)
    return -1;

  /* FIXME: need to send zero bits during duration */
  usleep (sleeptime);

  if (ClearCommBreak (get_handle ()) == 0)
    return -1;

  syscall_printf ("0 = fhandler_serial:tcsendbreak (%d)", duration);

  return 0;
}

/* tcdrain: POSIX 7.2.2.1 */
int
fhandler_serial::tcdrain ()
{
  if (FlushFileBuffers (get_handle ()) == 0)
    return -1;

  return 0;
}

/* tcflow: POSIX 7.2.2.1 */
int
fhandler_serial::tcflow (int action)
{
  DWORD win32action = 0;
  DCB dcb;
  char xchar;

  termios_printf ("action %d", action);

  switch (action)
    {
    case TCOOFF:
      win32action = SETXOFF;
      break;
    case TCOON:
      win32action = SETXON;
      break;
    case TCION:
    case TCIOFF:
      if (GetCommState (get_handle (), &dcb) == 0)
	return -1;
      if (action == TCION)
	xchar = (dcb.XonChar ? dcb.XonChar : 0x11);
      else
	xchar = (dcb.XoffChar ? dcb.XoffChar : 0x13);
      if (TransmitCommChar (get_handle (), xchar) == 0)
	return -1;
      return 0;
      break;
    default:
      return -1;
      break;
    }

  if (EscapeCommFunction (get_handle (), win32action) == 0)
    return -1;

  return 0;
}


/* switch_modem_lines: set or clear RTS and/or DTR */
int
fhandler_serial::switch_modem_lines (int set, int clr)
{
  int res = 0;

  if (set & TIOCM_RTS)
    {
      if (EscapeCommFunction (get_handle (), SETRTS))
	rts = TIOCM_RTS;
      else
	{
	  __seterrno ();
	  res = -1;
	}
    }
  else if (clr & TIOCM_RTS)
    {
      if (EscapeCommFunction (get_handle (), CLRRTS))
	rts = 0;
      else
	{
	  __seterrno ();
	  res = -1;
	}
    }
  if (set & TIOCM_DTR)
    {
      if (EscapeCommFunction (get_handle (), SETDTR))
	rts = TIOCM_DTR;
      else
	{
	  __seterrno ();
	  res = -1;
	}
    }
  else if (clr & TIOCM_DTR)
    {
      if (EscapeCommFunction (get_handle (), CLRDTR))
	rts = 0;
      else
	{
	  __seterrno ();
	  res = -1;
	}
    }

  return res;
}

/* ioctl: */
int
fhandler_serial::ioctl (unsigned int cmd, void *buf)
{
  int res = 0;

# define ibuf ((int) (intptr_t) buf)
# define ipbuf (*(int *) buf)

  DWORD ev;
  COMSTAT st;
  if (!ClearCommError (get_handle (), &ev, &st))
    {
      __seterrno ();
      res = -1;
    }
  else
    switch (cmd)
      {
      case TCFLSH:
	res = tcflush (ibuf);
	break;
      case TIOCMGET:
	DWORD modem_lines;
	if (!GetCommModemStatus (get_handle (), &modem_lines))
	  {
	    __seterrno ();
	    res = -1;
	  }
	else
	  {
	    ipbuf = 0;
	    if (modem_lines & MS_CTS_ON)
	      ipbuf |= TIOCM_CTS;
	    if (modem_lines & MS_DSR_ON)
	      ipbuf |= TIOCM_DSR;
	    if (modem_lines & MS_RING_ON)
	      ipbuf |= TIOCM_RI;
	    if (modem_lines & MS_RLSD_ON)
	      ipbuf |= TIOCM_CD;

	    DWORD cb;
	    DWORD mcr;
	    if (!DeviceIoControl (get_handle (), IOCTL_SERIAL_GET_DTRRTS,
				  NULL, 0, &mcr, 4, &cb, 0) || cb != 4)
	      ipbuf |= rts | dtr;
	    else
	      {
		if (mcr & 2)
		  ipbuf |= TIOCM_RTS;
		if (mcr & 1)
		  ipbuf |= TIOCM_DTR;
	      }
	  }
	break;
      case TIOCMSET:
	if (switch_modem_lines (ipbuf, ~ipbuf))
	  res = -1;
	break;
      case TIOCMBIS:
	if (switch_modem_lines (ipbuf, 0))
	  res = -1;
	break;
      case TIOCMBIC:
	if (switch_modem_lines (0, ipbuf))
	  res = -1;
	break;
      case TIOCCBRK:
	if (ClearCommBreak (get_handle ()) == 0)
	  {
	    __seterrno ();
	    res = -1;
	  }
	break;
      case TIOCSBRK:
	if (SetCommBreak (get_handle ()) == 0)
	  {
	    __seterrno ();
	    res = -1;
	  }
	break;
     case TIOCINQ:
       if (ev & CE_FRAME || ev & CE_IOE || ev & CE_OVERRUN || ev & CE_RXOVER
	   || ev & CE_RXPARITY)
	 {
	   set_errno (EINVAL);	/* FIXME: Use correct errno */
	   res = -1;
	 }
       else
	 ipbuf = st.cbInQue;
       break;
     case TIOCGWINSZ:
       ((struct winsize *) buf)->ws_row = 0;
       ((struct winsize *) buf)->ws_col = 0;
       break;
     case FIONREAD:
       set_errno (ENOTSUP);
       res = -1;
       break;
     default:
       res = fhandler_base::ioctl (cmd, buf);
       break;
     }

  termios_printf ("%d = ioctl(%x, %p)", res, cmd, buf);
# undef ibuf
# undef ipbuf
  return res;
}

/* tcflush: POSIX 7.2.2.1 */
int
fhandler_serial::tcflush (int queue)
{
  DWORD flags;

  switch (queue)
    {
    case TCOFLUSH:
      flags = PURGE_TXABORT | PURGE_TXCLEAR;
      break;
    case TCIFLUSH:
      flags = PURGE_RXABORT | PURGE_RXCLEAR;
      break;
    case TCIOFLUSH:
      flags = PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR;
      break;
    default:
      termios_printf ("Invalid tcflush queue %d", queue);
      set_errno (EINVAL);
      return -1;
    }

  if (!PurgeComm (get_handle (), flags))
    {
      __seterrno ();
      return -1;
    }

  return 0;
}

/* tcsetattr: POSIX 7.2.1.1 */
int
fhandler_serial::tcsetattr (int action, const struct termios *t)
{
  /* Possible actions:
    TCSANOW:   immediately change attributes.
    TCSADRAIN: flush output, then change attributes.
    TCSAFLUSH: flush output and discard input, then change attributes.
  */

  bool dropDTR = false;
  COMMTIMEOUTS to;
  DCB ostate, state;
  unsigned int ovtime = vtime_, ovmin = vmin_;
  int tmpDtr, tmpRts, res;
  res = tmpDtr = tmpRts = 0;

  termios_printf ("action %d", action);
  if ((action == TCSADRAIN) || (action == TCSAFLUSH))
    {
      FlushFileBuffers (get_handle ());
      termios_printf ("flushed file buffers");
    }
  if (action == TCSAFLUSH)
    PurgeComm (get_handle (), (PURGE_RXABORT | PURGE_RXCLEAR));

  /* get default/last comm state */
  if (!GetCommState (get_handle (), &ostate))
    return -1;

  state = ostate;

  /* -------------- Set baud rate ------------------ */
  /* FIXME: WIN32 also has 14400, 56000, 128000, and 256000.
     Unix also has 230400. */

  switch (t->c_ospeed)
    {
    case B0:
      /* Drop DTR - but leave DCB-resident bitrate as-is since
	 0 is an invalid bitrate in Win32 */
      dropDTR = true;
      break;
    case B110:
      state.BaudRate = CBR_110;
      break;
    case B300:
      state.BaudRate = CBR_300;
      break;
    case B600:
      state.BaudRate = CBR_600;
      break;
    case B1200:
      state.BaudRate = CBR_1200;
      break;
    case B2400:
      state.BaudRate = CBR_2400;
      break;
    case B4800:
      state.BaudRate = CBR_4800;
      break;
    case B9600:
      state.BaudRate = CBR_9600;
      break;
    case B19200:
      state.BaudRate = CBR_19200;
      break;
    case B38400:
      state.BaudRate = CBR_38400;
      break;
    case B57600:
      state.BaudRate = CBR_57600;
      break;
    case B115200:
      state.BaudRate = CBR_115200;
      break;
    case B128000:
      state.BaudRate = CBR_128000;
      break;
    case B230400:
      state.BaudRate = 230400 /* CBR_230400 - not defined */;
      break;
    case B256000:
      state.BaudRate = CBR_256000;
      break;
    case B460800:
      state.BaudRate = 460800 /* CBR_460800 - not defined */;
      break;
    case B500000:
      state.BaudRate = 500000 /* CBR_500000 - not defined */;
      break;
    case B576000:
      state.BaudRate = 576000 /* CBR_576000 - not defined */;
      break;
    case B921600:
      state.BaudRate = 921600 /* CBR_921600 - not defined */;
      break;
    case B1000000:
      state.BaudRate = 1000000 /* CBR_1000000 - not defined */;
      break;
    case B1152000:
      state.BaudRate = 1152000 /* CBR_1152000 - not defined */;
      break;
    case B1500000:
      state.BaudRate = 1500000 /* CBR_1500000 - not defined */;
      break;
    case B2000000:
      state.BaudRate = 2000000 /* CBR_2000000 - not defined */;
      break;
    case B2500000:
      state.BaudRate = 2500000 /* CBR_2500000 - not defined */;
      break;
    case B3000000:
      state.BaudRate = 3000000 /* CBR_3000000 - not defined */;
      break;
    default:
      /* Unsupported baud rate! */
      termios_printf ("Invalid t->c_ospeed %u", t->c_ospeed);
      set_errno (EINVAL);
      return -1;
    }

  /* -------------- Set byte size ------------------ */

  switch (t->c_cflag & CSIZE)
    {
    case CS5:
      state.ByteSize = 5;
      break;
    case CS6:
      state.ByteSize = 6;
      break;
    case CS7:
      state.ByteSize = 7;
      break;
    case CS8:
      state.ByteSize = 8;
      break;
    default:
      /* Unsupported byte size! */
      termios_printf ("Invalid t->c_cflag byte size %u",
		      t->c_cflag & CSIZE);
      set_errno (EINVAL);
      return -1;
    }

  /* -------------- Set stop bits ------------------ */

  if (t->c_cflag & CSTOPB)
    state.StopBits = TWOSTOPBITS;
  else
    state.StopBits = ONESTOPBIT;

  /* -------------- Set parity ------------------ */

  if (t->c_cflag & PARENB)
    state.Parity = (t->c_cflag & PARODD) ? ODDPARITY : EVENPARITY;
  else
    state.Parity = NOPARITY;

  state.fBinary = TRUE;     /* Binary transfer */
  state.EofChar = 0;	    /* No end-of-data in binary mode */
  state.fNull = FALSE;      /* Don't discard nulls in binary mode */

  /* -------------- Parity errors ------------------ */
  /* fParity combines the function of INPCK and NOT IGNPAR */

  if ((t->c_iflag & INPCK) && !(t->c_iflag & IGNPAR))
    state.fParity = TRUE;   /* detect parity errors */
  else
    state.fParity = FALSE;  /* ignore parity errors */

  /* Only present in Win32, Unix has no equivalent */
  state.fErrorChar = FALSE;
  state.ErrorChar = 0;

  /* -------------- Set software flow control ------------------ */
  /* Set fTXContinueOnXoff to FALSE.  This prevents the triggering of a
     premature XON when the remote device interprets a received character
     as XON (same as IXANY on the remote side).  Otherwise, a TRUE
     value separates the TX and RX functions. */

  state.fTXContinueOnXoff = TRUE;     /* separate TX and RX flow control */

  /* Transmission flow control */
  if (t->c_iflag & IXON)
    state.fOutX = TRUE;   /* enable */
  else
    state.fOutX = FALSE;  /* disable */

  /* Reception flow control */
  if (t->c_iflag & IXOFF)
    state.fInX = TRUE;    /* enable */
  else
    state.fInX = FALSE;   /* disable */

  /* XoffLim and XonLim are left at default values */

  state.XonChar = (t->c_cc[VSTART] ? t->c_cc[VSTART] : 0x11);
  state.XoffChar = (t->c_cc[VSTOP] ? t->c_cc[VSTOP] : 0x13);

  /* -------------- Set hardware flow control ------------------ */

  /* Disable DSR flow control */
  state.fOutxDsrFlow = FALSE;

  /* Some old flavors of Unix automatically enabled hardware flow
     control when software flow control was not enabled.  Since newer
     Unices tend to require explicit setting of hardware flow-control,
     this is what we do. */

  /* RTS/CTS flow control */
  if (t->c_cflag & CRTSCTS)
    {							/* enable */
      state.fOutxCtsFlow = TRUE;
      state.fRtsControl = RTS_CONTROL_HANDSHAKE;
    }
  else
    {							/* disable */
      state.fRtsControl = RTS_CONTROL_ENABLE;
      state.fOutxCtsFlow = FALSE;
      tmpRts = TIOCM_RTS;
    }

  if (t->c_cflag & CRTSXOFF)
    state.fRtsControl = RTS_CONTROL_HANDSHAKE;

  /* -------------- DTR ------------------ */
  /* Assert DTR on device open */

  state.fDtrControl = DTR_CONTROL_ENABLE;

  /* -------------- DSR ------------------ */
  /* Assert DSR at the device? */

  if (t->c_cflag & CLOCAL)
    state.fDsrSensitivity = FALSE;  /* no */
  else
    state.fDsrSensitivity = TRUE;   /* yes */

  /* -------------- Error handling ------------------ */
  /* Since read/write operations terminate upon error, we
     will use ClearCommError() to resume. */

  state.fAbortOnError = TRUE;

  if ((memcmp (&ostate, &state, sizeof (state)) != 0)
      && !SetCommState (get_handle (), &state))
    {
      /* SetCommState() failed, usually due to invalid DCB param.
	 Keep track of this so we can set errno to EINVAL later
	 and return failure */
      termios_printf ("SetCommState() failed, %E");
      __seterrno ();
      res = -1;
    }

  rbinary ((t->c_iflag & IGNCR) ? false : true);
  wbinary ((t->c_oflag & ONLCR) ? false : true);

  if (dropDTR)
    {
      EscapeCommFunction (get_handle (), CLRDTR);
      tmpDtr = 0;
    }
  else
    {
      /* FIXME: Sometimes when CLRDTR is set, setting
      state.fDtrControl = DTR_CONTROL_ENABLE will fail.  This
      is a problem since a program might want to change some
      parameters while DTR is still down. */

      EscapeCommFunction (get_handle (), SETDTR);
      tmpDtr = TIOCM_DTR;
    }

  rts = tmpRts;
  dtr = tmpDtr;

  /* The following documentation on was taken from "Linux Serial Programming
  HOWTO".  It explains how MIN (t->c_cc[VMIN] || vmin_) and TIME
  (t->c_cc[VTIME] || vtime_) is to be used.

  In non-canonical input processing mode, input is not assembled into
  lines and input processing (erase, kill, delete, etc.) does not
  occur. Two parameters control the behavior of this mode: c_cc[VTIME]
  sets the character timer, and c_cc[VMIN] sets the minimum number of
  characters to receive before satisfying the read.

  If MIN > 0 and TIME = 0, MIN sets the number of characters to receive
  before the read is satisfied. As TIME is zero, the timer is not used.

  If MIN = 0 and TIME > 0, TIME serves as a timeout value. The read will
  be satisfied if a single character is read, or TIME is exceeded (t =
  TIME *0.1 s). If TIME is exceeded, no character will be returned.

  If MIN > 0 and TIME > 0, TIME serves as an inter-character timer. The
  read will be satisfied if MIN characters are received, or the time
  between two characters exceeds TIME. The timer is restarted every time
  a character is received and only becomes active after the first
  character has been received.

  If MIN = 0 and TIME = 0, read will be satisfied immediately. The
  number of characters currently available, or the number of characters
  requested will be returned. According to Antonino (see contributions),
  you could issue a fcntl(fd, F_SETFL, FNDELAY); before reading to get
  the same result.
  */

  if (t->c_lflag & ICANON)
    {
      vmin_ = 0;
      vtime_ = 0;
    }
  else
    {
      vtime_ = t->c_cc[VTIME] * 100;
      vmin_ = t->c_cc[VMIN];
    }

  debug_printf ("vtime %d, vmin %ld", vtime_, vmin_);

  if (ovmin != vmin_ || ovtime != vtime_)
  {
    memset (&to, 0, sizeof (to));

    if ((vmin_ > 0) && (vtime_ == 0))
      {
	/* Returns immediately with whatever is in buffer on a ReadFile();
	   or blocks if nothing found.  We will keep calling ReadFile(); until
	   vmin_ characters are read */
	to.ReadIntervalTimeout = to.ReadTotalTimeoutMultiplier = MAXDWORD;
	to.ReadTotalTimeoutConstant = MAXDWORD - 1;
      }
    else if ((vmin_ == 0) && (vtime_ > 0))
      {
	/* set timeoout constant appropriately and we will only try to
	   read one character in ReadFile() */
	to.ReadTotalTimeoutConstant = vtime_;
	to.ReadIntervalTimeout = to.ReadTotalTimeoutMultiplier = MAXDWORD;
      }
    else if ((vmin_ > 0) && (vtime_ > 0))
      {
	/* time applies to the interval time for this case */
	to.ReadIntervalTimeout = vtime_;
      }
    else if ((vmin_ == 0) && (vtime_ == 0))
      {
	/* returns immediately with whatever is in buffer as per
	   Time-Outs docs in Win32 SDK API docs */
	to.ReadIntervalTimeout = MAXDWORD;
      }

    debug_printf ("ReadTotalTimeoutConstant %u, ReadIntervalTimeout %u, ReadTotalTimeoutMultiplier %u",
		  to.ReadTotalTimeoutConstant, to.ReadIntervalTimeout, to.ReadTotalTimeoutMultiplier);

    if (!SetCommTimeouts(get_handle (), &to))
      {
	/* SetCommTimeouts() failed. Keep track of this so we
	   can set errno to EINVAL later and return failure */
	termios_printf ("SetCommTimeouts() failed, %E");
	__seterrno ();
	res = -1;
      }
  }

  return res;
}

/* tcgetattr: POSIX 7.2.1.1 */
int
fhandler_serial::tcgetattr (struct termios *t)
{
  DCB state;

  /* Get current Win32 comm state */
  if (GetCommState (get_handle (), &state) == 0)
    return -1;

  /* for safety */
  memset (t, 0, sizeof (*t));

  t->c_cflag = 0;
  /* -------------- Baud rate ------------------ */
  switch (state.BaudRate)
    {
    case CBR_110:
	t->c_ospeed = t->c_ispeed = B110;
	break;
    case CBR_300:
	t->c_ospeed = t->c_ispeed = B300;
	break;
    case CBR_600:
	t->c_ospeed = t->c_ispeed = B600;
	break;
    case CBR_1200:
	t->c_ospeed = t->c_ispeed = B1200;
	break;
    case CBR_2400:
	t->c_ospeed = t->c_ispeed = B2400;
	break;
    case CBR_4800:
	t->c_ospeed = t->c_ispeed = B4800;
	break;
    case CBR_9600:
	t->c_ospeed = t->c_ispeed = B9600;
	break;
    case CBR_19200:
	t->c_ospeed = t->c_ispeed = B19200;
	break;
    case CBR_38400:
	t->c_ospeed = t->c_ispeed = B38400;
	break;
    case CBR_57600:
	t->c_ospeed = t->c_ispeed = B57600;
	break;
    case CBR_115200:
	t->c_ospeed = t->c_ispeed = B115200;
	break;
    case CBR_128000:
	t->c_ospeed = t->c_ispeed = B128000;
	break;
    case 230400: /* CBR_230400 - not defined */
	t->c_ospeed = t->c_ispeed = B230400;
	break;
    case CBR_256000:
	t->c_ospeed = t->c_ispeed = B256000;
	break;
    case 460800: /* CBR_460000 - not defined */
	t->c_ospeed = t->c_ispeed = B460800;
	break;
    case 500000: /* CBR_500000 - not defined */
	t->c_ospeed = t->c_ispeed = B500000;
	break;
    case 576000: /* CBR_576000 - not defined */
	t->c_ospeed = t->c_ispeed = B576000;
	break;
    case 921600: /* CBR_921600 - not defined */
	t->c_ospeed = t->c_ispeed = B921600;
	break;
    case 1000000: /* CBR_1000000 - not defined */
	t->c_ospeed = t->c_ispeed = B1000000;
	break;
    case 1152000: /* CBR_1152000 - not defined */
	t->c_ospeed = t->c_ispeed = B1152000;
	break;
    case 1500000: /* CBR_1500000 - not defined */
	t->c_ospeed = t->c_ispeed = B1500000;
	break;
    case 2000000: /* CBR_2000000 - not defined */
	t->c_ospeed = t->c_ispeed = B2000000;
	break;
    case 2500000: /* CBR_2500000 - not defined */
	t->c_ospeed = t->c_ispeed = B2500000;
	break;
    case 3000000: /* CBR_3000000 - not defined */
	t->c_ospeed = t->c_ispeed = B3000000;
	break;
    default:
	/* Unsupported baud rate! */
	termios_printf ("Invalid baud rate %u", state.BaudRate);
	set_errno (EINVAL);
	return -1;
    }

  /* -------------- Byte size ------------------ */

  switch (state.ByteSize)
    {
    case 5:
      t->c_cflag |= CS5;
      break;
    case 6:
      t->c_cflag |= CS6;
      break;
    case 7:
      t->c_cflag |= CS7;
      break;
    case 8:
      t->c_cflag |= CS8;
      break;
    default:
      /* Unsupported byte size! */
      termios_printf ("Invalid byte size %u", state.ByteSize);
      set_errno (EINVAL);
      return -1;
    }

  /* -------------- Stop bits ------------------ */

  if (state.StopBits == TWOSTOPBITS)
    t->c_cflag |= CSTOPB;

  /* -------------- Parity ------------------ */

  if (state.Parity == ODDPARITY)
    t->c_cflag |= (PARENB | PARODD);
  if (state.Parity == EVENPARITY)
    t->c_cflag |= PARENB;

  /* -------------- Parity errors ------------------ */

  /* fParity combines the function of INPCK and NOT IGNPAR */
  if (state.fParity)
    t->c_iflag |= INPCK;
  else
    t->c_iflag |= IGNPAR;	/* not necessarily! */

  /* -------------- Software flow control ------------------ */

  /* transmission flow control */
  if (state.fOutX)
    t->c_iflag |= IXON;

  /* reception flow control */
  if (state.fInX)
    t->c_iflag |= IXOFF;

  t->c_cc[VSTART] = (state.XonChar ? state.XonChar : 0x11);
  t->c_cc[VSTOP] = (state.XoffChar ? state.XoffChar : 0x13);

  /* -------------- Hardware flow control ------------------ */
  /* Some old flavors of Unix automatically enabled hardware flow
     control when software flow control was not enabled.  Since newer
     Unices tend to require explicit setting of hardware flow-control,
     this is what we do. */

  /* Input flow-control */
  if ((state.fRtsControl == RTS_CONTROL_HANDSHAKE) && state.fOutxCtsFlow)
    t->c_cflag |= CRTSCTS;
  if (state.fRtsControl == RTS_CONTROL_HANDSHAKE)
    t->c_cflag |= CRTSXOFF;

  /* -------------- CLOCAL --------------- */
  /* DSR is only lead toggled only by CLOCAL.  Check it to see if
     CLOCAL was called. */
  /* FIXME: If tcsetattr() hasn't been called previously, this may
     give a false CLOCAL. */

  if (!state.fDsrSensitivity)
    t->c_cflag |= CLOCAL;

  /* FIXME: need to handle IGNCR */
#if 0
  if (!rbinary ())
    t->c_iflag |= IGNCR;
#endif

  if (!wbinary ())
    t->c_oflag |= ONLCR;

  t->c_cc[VTIME] = vtime_ / 100;
  t->c_cc[VMIN] = vmin_;

  debug_printf ("vmin_ %lu, vtime_ %u", vmin_, vtime_);

  return 0;
}

void
fhandler_serial::fixup_after_fork (HANDLE parent)
{
  if (close_on_exec ())
    fhandler_base::fixup_after_fork (parent);
  overlapped_setup ();
  debug_printf ("io_status.hEvent %p", io_status.hEvent);
}

void
fhandler_serial::fixup_after_exec ()
{
  if (!close_on_exec ())
    overlapped_setup ();
  debug_printf ("io_status.hEvent %p, close_on_exec %d", io_status.hEvent, close_on_exec ());
}

int
fhandler_serial::dup (fhandler_base *child, int flags)
{
  fhandler_serial *fhc = (fhandler_serial *) child;
  fhc->overlapped_setup ();
  return fhandler_base::dup (child, flags);
}
