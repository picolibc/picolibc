/* fhandler_serial.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"

/**********************************************************************/
/* fhandler_serial */

fhandler_serial::fhandler_serial (const char *name, DWORD devtype, int unit) :
	fhandler_base (devtype, name, unit)
{
  set_cb (sizeof *this);
  vmin_ = 0;
  vtime_ = 0;
  pgrp_ = myself->pgid;
  set_need_fork_fixup ();
}

void
fhandler_serial::overlapped_setup ()
{
  memset (&io_status, 0, sizeof (io_status));
  io_status.hEvent = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  ProtectHandle (io_status.hEvent);
  overlapped_armed = 0;
}

int
fhandler_serial::raw_read (void *ptr, size_t ulen)
{
  int tot;
  DWORD n;
  HANDLE w4[2];
  DWORD minchars = vmin_ ?: ulen;

  w4[0] = io_status.hEvent;
  w4[1] = signal_arrived;

  debug_printf ("ulen %d, vmin_ %d, vtime_ %d, hEvent %p", ulen, vmin_, vtime_,
		io_status.hEvent);
  if (!overlapped_armed)
    {
      (void) SetCommMask (get_handle (), EV_RXCHAR);
      ResetEvent (io_status.hEvent);
    }

  for (n = 0, tot = 0; ulen; ulen -= n, ptr = (char *)ptr + n)
    {
      DWORD ev;
      COMSTAT st;
      DWORD inq = 1;

      n = 0;

      if (!vtime_ && !vmin_)
	inq = ulen;
      else if (vtime_)
	{
	  inq = ulen;	// non-interruptible -- have to use kernel timeouts
			// also note that this is not strictly correct.
			// if vmin > ulen then things won't work right.
	  overlapped_armed = -1;
	}
      if (!overlapped_armed)
	{
	  if (!ClearCommError (get_handle (), &ev, &st))
	    goto err;
	  else if (ev)
	    termios_printf ("error detected %x", ev);
	  else if (st.cbInQue)
	    inq = st.cbInQue;
	  else if ((size_t)tot >= minchars)
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
	      switch (WaitForMultipleObjects (2, w4, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0:
		  if (!GetOverlappedResult (get_handle (), &io_status, &n, FALSE))
		    goto err;
		  debug_printf ("n %d, ev %x", n, ev);
		  break;
		case WAIT_OBJECT_0 + 1:
		  tot = -1;
		  PurgeComm (get_handle (), PURGE_RXABORT);
		  overlapped_armed = 0;
		  set_sig_errno (EINTR);
		  goto out;
		default:
		  goto err;
		}
	    }
	}

      overlapped_armed = 0;
      ResetEvent (io_status.hEvent);
      if (inq > ulen)
	inq = ulen;
      debug_printf ("inq %d", inq);
      if (ReadFile (get_handle(), ptr, min (inq, ulen), &n, &io_status))
	/* Got something */;
      else if (GetLastError () != ERROR_IO_PENDING)
	goto err;
      else if (!GetOverlappedResult (get_handle (), &io_status, &n, TRUE))
	goto err;

      tot += n;
      debug_printf ("vtime_ %d, vmin_ %d, n %d, tot %d", vtime_, vmin_, n, tot);
      if (vtime_ || !vmin_ || !n)
	break;
      continue;

    err:
      PurgeComm (get_handle (), PURGE_RXABORT);
      debug_printf ("err %E");
      if (GetLastError () == ERROR_OPERATION_ABORTED)
	n = 0;
      else
	{
	  tot = -1;
	  __seterrno ();
	  break;
	}
    }

out:
  return tot;
}

/* Cover function to WriteFile to provide Posix interface and semantics
   (as much as possible).  */
int
fhandler_serial::raw_write (const void *ptr, size_t len)
{
  DWORD bytes_written;

  if (overlapped_armed)
    PurgeComm (get_handle (), PURGE_TXABORT | PURGE_RXABORT);
  ResetEvent (io_status.hEvent);

  for (;;)
    {
      overlapped_armed = TRUE;
      if (WriteFile (get_handle(), ptr, len, &bytes_written, &io_status))
	break;

      switch (GetLastError ())
	{
	  case ERROR_OPERATION_ABORTED:
	    continue;
	  case ERROR_IO_PENDING:
	    break;
	  default:
	    goto err;
	}

      if (!GetOverlappedResult (get_handle (), &io_status, &bytes_written, TRUE))
	goto err;

      break;
    }

  overlapped_armed = FALSE;
  return bytes_written;

err:
  __seterrno ();
  return -1;
}

void
fhandler_serial::dump (void)
{
  paranoid_printf ("here");
}

void
fhandler_serial::init (HANDLE f, DWORD flags, mode_t bin)
{
  fhandler_base::init (f, flags, bin);
  (void) open (NULL, flags, bin ? O_BINARY : 0);
}

int
fhandler_serial::open (const char *name, int flags, mode_t mode)
{
  int res;
  COMMTIMEOUTS to;
  extern BOOL reset_com;

  syscall_printf ("fhandler_serial::open (%s, %p, %p)",
			get_name (), flags, mode);

  if (name && !(res = this->fhandler_base::open (flags, mode)))
    return 0;
  else
    res = 1;

  (void) SetCommMask (get_handle (), EV_RXCHAR);

  set_r_no_interrupt (1);	// Handled explicitly in read code

  overlapped_setup ();

  memset (&to, 0, sizeof (to));
  (void) SetCommTimeouts (get_handle (), &to);

  /* Reset serial port to known state of 9600-8-1-no flow control
     on open for better behavior under Win 95.

     FIXME:  This should only be done when explicitly opening the com
     port.  It should not be reset if an fd is inherited.
     Using __progname in this way, to determine how far along in the
     initialization we are, is really a terrible kludge and should
     be fixed ASAP.
  */
  extern char *__progname;
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
  syscall_printf ("%p = fhandler_serial::open (%s, %p, %p)",
			res, get_name (), flags, mode);
  return res;
}

int
fhandler_serial::close ()
{
  (void) ForceCloseHandle (io_status.hEvent);
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
fhandler_serial::tcdrain (void)
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

/* tcflush: POSIX 7.2.2.1 */
int
fhandler_serial::tcflush (int queue)
{
  if (queue == TCOFLUSH || queue == TCIOFLUSH)
    PurgeComm (get_handle (), PURGE_TXABORT | PURGE_TXCLEAR);

  if (queue == TCIFLUSH | queue == TCIOFLUSH)
    /* Input flushing by polling until nothing turns up
       (we stop after 1000 chars anyway) */
    for (int max = 1000; max > 0; max--)
      {
	DWORD ev;
	COMSTAT st;
	if (!PurgeComm (get_handle (), PURGE_RXABORT | PURGE_RXCLEAR))
	  break;
	Sleep (100);
	if (!ClearCommError (get_handle (), &ev, &st) || !st.cbInQue)
	  break;
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

  BOOL dropDTR = FALSE;
  COMMTIMEOUTS to;
  DCB ostate, state;
  unsigned int ovtime = vtime_, ovmin = vmin_;

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
      case B0:	/* drop DTR */
	dropDTR = TRUE;
	state.BaudRate = 0;
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
      default:
	/* Unsupported baud rate! */
	termios_printf ("Invalid t->c_ospeed %d", t->c_ospeed);
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
	termios_printf ("Invalid t->c_cflag byte size %d",
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

  /* -------------- Set state and exit ------------------ */
  if (memcmp (&ostate, &state, sizeof (state)) != 0)
    SetCommState (get_handle (), &state);

  set_r_binary ((t->c_iflag & IGNCR) ? 0 : 1);
  set_w_binary ((t->c_oflag & ONLCR) ? 0 : 1);

  if (dropDTR == TRUE)
    EscapeCommFunction (get_handle (), CLRDTR);
  else
    {
      /* FIXME: Sometimes when CLRDTR is set, setting
      state.fDtrControl = DTR_CONTROL_ENABLE will fail.  This
      is a problem since a program might want to change some
      parameters while DTR is still down. */

      EscapeCommFunction (get_handle (), SETDTR);
    }

  /*
  The following documentation on was taken from "Linux Serial Programming
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
      vmin_ = MAXDWORD;
      vtime_ = 0;
    }
  else
    {
      vtime_ = t->c_cc[VTIME] * 100;
      vmin_ = t->c_cc[VMIN];
    }

  debug_printf ("vtime %d, vmin %d\n", vtime_, vmin_);

  if (ovmin == vmin_ && ovtime == vtime_)
    return 0;

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

  debug_printf ("ReadTotalTimeoutConstant %d, ReadIntervalTimeout %d, ReadTotalTimeoutMultiplier %d",
		to.ReadTotalTimeoutConstant, to.ReadIntervalTimeout, to.ReadTotalTimeoutMultiplier);
  int res = SetCommTimeouts (get_handle (), &to);
  if (!res)
    {
      system_printf ("SetCommTimeout failed, %E");
      __seterrno ();
      return -1;
    }

  return 0;
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

  /* -------------- Baud rate ------------------ */

  switch (state.BaudRate)
    {
      case 0:
	/* FIXME: need to drop DTR */
	t->c_cflag = t->c_ospeed = t->c_ispeed = B0;
	break;
      case CBR_110:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B110;
	break;
      case CBR_300:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B300;
	break;
      case CBR_600:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B600;
	break;
      case CBR_1200:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B1200;
	break;
      case CBR_2400:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B2400;
	break;
      case CBR_4800:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B4800;
	break;
      case CBR_9600:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B9600;
	break;
      case CBR_19200:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B19200;
	break;
      case CBR_38400:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B38400;
	break;
      case CBR_57600:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B57600;
	break;
      case CBR_115200:
	t->c_cflag = t->c_ospeed = t->c_ispeed = B115200;
	break;
      default:
	/* Unsupported baud rate! */
	termios_printf ("Invalid baud rate %d", state.BaudRate);
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
	termios_printf ("Invalid byte size %d", state.ByteSize);
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
  if (state.fParity == TRUE)
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
  if ((state.fRtsControl == RTS_CONTROL_HANDSHAKE) &&
      (state.fOutxCtsFlow == TRUE))
    t->c_cflag |= CRTSCTS;
  if (state.fRtsControl == RTS_CONTROL_HANDSHAKE)
    t->c_cflag |= CRTSXOFF;

  /* -------------- CLOCAL --------------- */
  /* DSR is only lead toggled only by CLOCAL.  Check it to see if
     CLOCAL was called. */
  /* FIXME: If tcsetattr() hasn't been called previously, this may
     give a false CLOCAL. */

  if (state.fDsrSensitivity == FALSE)
    t->c_cflag |= CLOCAL;

  /* FIXME: need to handle IGNCR */
#if 0
  if (!get_r_binary ())
    t->c_iflag |= IGNCR;
#endif

  if (!get_w_binary ())
    t->c_oflag |= ONLCR;

  debug_printf ("vmin_ %d, vtime_ %d", vmin_, vtime_);
  if (vmin_ == MAXDWORD)
    {
      t->c_lflag |= ICANON;
      t->c_cc[VTIME] = t->c_cc[VMIN] = 0;
    }
  else
    {
      t->c_cc[VTIME] = vtime_ / 100;
      t->c_cc[VMIN] = vmin_;
    }

  return 0;
}

void
fhandler_serial::fixup_after_fork (HANDLE parent)
{
  if (get_close_on_exec ())
    this->fhandler_base::fixup_after_fork (parent);
  overlapped_setup ();
  debug_printf ("io_status.hEvent %p", io_status.hEvent);
}

void
fhandler_serial::fixup_after_exec (HANDLE)
{
  overlapped_setup ();
  debug_printf ("io_status.hEvent %p", io_status.hEvent);
  return;
}

int
fhandler_serial::dup (fhandler_base *child)
{
  fhandler_serial *fhc = (fhandler_serial *) child;
  overlapped_setup ();
  fhc->vmin_ = vmin_;
  fhc->vtime_ = vtime_;
  return fhandler_base::dup (child);
}
