/****************************************************************************
 *
 *  Module name: remcom.c $
 *  Revision: 1.34 $
 *  Date: 91/03/09 12:29:49 $
 *  Contributor:     Lake Stevens Instrument Division$
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works on target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *  Modified for SPARC by Stu Grossman, Cygnus Support.
 *
 *  This code has been extensively tested on the Fujitsu SPARClite demo board.
 *
 *  To enable debugger support, two things need to happen.  One, a
 *  call to set_debug_traps() is necessary in order to allow any breakpoints
 *  or error conditions to be properly intercepted and reported to gdb.
 *  Two, a breakpoint needs to be generated to begin communication.  This
 *  is most easily accomplished by a call to breakpoint().  Breakpoint()
 *  simulates a breakpoint by executing a trap #1.
 *
 *************
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 *    bBB..BB	    Set baud rate to BB..BB		   OK or BNN, then sets
 *							   baud rate
 *
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/

#include <string.h>
#include <signal.h>
#include "dbgmon.h"
#include "parser.h"
#include "ctype.h"

/************************************************************************
 *
 * external low-level support routines
 */

extern putchar();   /* write a single character      */
extern getchar();   /* read and return a single char */

/************************************************************************/

/* Stuff for stdio-like gets_debugger_check() */

#define CTRL(x)   ('x'&0x1f)
#define DEL       0x7f
#define INTR      CTRL(C)
#define BELL      0x7
#define PROMPT    "? "

#define BUFSIZE 512                              /* Big enough for register packets */

static int initialized = 0;                      /* !0 means we've been initialized */

static char hexchars[]="0123456789abcdef";

extern unsigned int _regs[];                     /* Saved registers from client    */

/* Convert ch from a hex digit to an int */

static int    
hex(ch)
     unsigned char ch;
{
  if (ch >= 'a' && ch <= 'f')
    return ch-'a'+10;
  if (ch >= '0' && ch <= '9')
    return ch-'0';
  if (ch >= 'A' && ch <= 'F')
    return ch-'A'+10;
  return -1;
}

/* scan for the sequence $<data>#<checksum>     */

static void
getpacket(buffer)
     char *buffer;
{
  unsigned char checksum;
  unsigned char xmitcsum;
  int i;
  int count;
  unsigned char ch;

    /* At this point, the start character ($) has been received through
     * the debug monitor parser. Get the remaining characters and 
     * process them.
     */

    checksum = 0;
    xmitcsum = -1;
    count = 0;

    /* read until a # or end of buffer is found */

    while (count < BUFSIZE)
    {
        ch = getchar();
        if (ch == '#')
            break;
        checksum = checksum + ch;
        buffer[count] = ch;
        count = count + 1;
    }

    if (count >= BUFSIZE)
        buffer[count] = 0;

    if (ch == '#')
    {
        xmitcsum = hex(getchar()) << 4;
        xmitcsum |= hex(getchar());
#if 0
    /* Humans shouldn't have to figure out checksums to type to it. */
        putchar ('+');
        return;
#endif

        if (checksum != xmitcsum)
        {
            putchar('-');                      /* failed checksum      */
            return;                            /* Back to monitor loop */
        }
        else
        {
            putchar('+');                      /* successful transfer */

            /* if a sequence char is present, reply the sequence ID */

            if (buffer[2] == ':')
            {
                putchar(buffer[0]);
                putchar(buffer[1]);

               /* remove sequence chars from buffer */

               count = strlen(buffer);
               for (i=3; i <= count; i++)
                   buffer[i-3] = buffer[i];
            }
            
            /* Buffer command received- go and process it. */


        }
    }
}


/* send the packet in buffer.  */

static void
putpacket(buffer)
     unsigned char *buffer;
{
  unsigned char checksum;
  int count;
  unsigned char ch;

  /*  $<packet info>#<checksum>. */
  do
    {
      putchar('$');
      checksum = 0;
      count = 0;

      while (ch = buffer[count])
	{
	  if (! putchar(ch))
	    return;
	  checksum += ch;
	  count += 1;
	}

      putchar('#');
      putchar(hexchars[checksum >> 4]);
      putchar(hexchars[checksum & 0xf]);

    }
  while (getchar() != '+');
}

static char remcomInBuffer[BUFSIZE];
static char remcomOutBuffer[BUFSIZE];

/* Indicate to caller of mem2hex or hex2mem that there has been an error.  */

static volatile int mem_err = 0;

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null), in case of mem fault,
 * return 0.
 * If MAY_FAULT is non-zero, then we will handle memory faults by returning
 * a 0, else treat a fault like any other fault in the stub.
 */

static unsigned char *
mem2hex(mem, buf, count, may_fault)
     unsigned char *mem;
     unsigned char *buf;
     int count;
     int may_fault;
{
  unsigned char ch;

  while (count-- > 0)
    {
      ch = *mem++;
      if (mem_err)
	return 0;
      *buf++ = hexchars[ch >> 4];
      *buf++ = hexchars[ch & 0xf];
    }

  *buf = 0;

  return buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte written */

static char *
hex2mem(buf, mem, count, may_fault)
     unsigned char *buf;
     unsigned char *mem;
     int count;
     int may_fault;
{
  int i;
  unsigned char ch;

  for (i=0; i<count; i++)
    {
      ch = hex(*buf++) << 4;
      ch |= hex(*buf++);
      *mem++ = ch;
      if (mem_err)
	return 0;
    }

  return mem;
}

/* This table contains the mapping between SPARC hardware trap types, and
   signals, which are primarily what GDB understands.  It also indicates
   which hardware traps we need to commandeer when initializing the stub. */

static struct hard_trap_info
{
  unsigned char tt;       /* Trap type code for SPARClite */
  unsigned char signo;    /* Signal that we map this trap into */
} hard_trap_info[] = {
  {0x06, SIGSEGV},        /* instruction access error */
  {0x0a, SIGILL},         /* privileged instruction */
  {0x0a, SIGILL},         /* illegal instruction */
  {0x0b, SIGEMT},         /* cp disabled */
  {0x07, SIGSEGV},        /* data access exception */
  {0x09, SIGTRAP},        /* ta 1 - normal breakpoint instruction */
  {0, 0}                  /* Must be last */
};

/* Convert the SPARC hardware trap type code to a unix signal number. */

static int
computeSignal(tt)
     int tt;
{
  struct hard_trap_info *ht;

  for (ht = hard_trap_info; ht->tt && ht->signo; ht++)
    if (ht->tt == tt)
      return ht->signo;

  return SIGHUP;		/* default for things we don't know about */
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */

static int
hexToInt(char **ptr, int *intValue)
{
  int numChars = 0;
  int hexValue;

  *intValue = 0;

  while (**ptr)
    {
      hexValue = hex(**ptr);
      if (hexValue < 0)
	break;

      *intValue = (*intValue << 4) | hexValue;
      numChars ++;

      (*ptr)++;
    }

  return (numChars);
}

/* This function lets GDB know that an exception has occured. */

static void
debug_handle_exception ()
{
    int           tt;            /* Trap type */
    int           sigval;
    char          *ptr;

    tt = (_regs[R_CAUSE] >> 2) & 0x0f;

    /* reply to host that an exception has occurred */
    sigval = computeSignal(tt);
    ptr = remcomOutBuffer;
  
    *ptr++ = 'T';
    *ptr++ = hexchars[sigval >> 4];
    *ptr++ = hexchars[sigval & 0xf];
  
    *ptr++ = hexchars[R_EPC >> 4];
    *ptr++ = hexchars[R_EPC & 0xf];
    *ptr++ = ':';
    ptr = mem2hex((char *)&_regs[R_EPC], ptr, 4, 0);
    *ptr++ = ';';
  
    *ptr++ = hexchars[R_FP >> 4];
    *ptr++ = hexchars[R_FP & 0xf];
    *ptr++ = ':';
    ptr = mem2hex((char *)&_regs[R_FP], ptr, 4, 0);
    *ptr++ = ';';
  
    *ptr++ = hexchars[R_SP >> 4];
    *ptr++ = hexchars[R_SP & 0xf];
    *ptr++ = ':';
    ptr = mem2hex((char *)&_regs[R_SP], ptr, 4, 0);
    *ptr++ = ';';
  
    *ptr++ = 0;
  
    putpacket(remcomOutBuffer);
    
    return;
}


void process_packet()
{

    char          *ptr;
    int           length;
    int           addr;
    int           sigval;
    int           tt;            /* Trap type */

    remcomOutBuffer[0] = 0;
    getpacket(remcomInBuffer);
    switch (remcomInBuffer[0])
    {

/* Return Last SIGVAL */

case '?':
        tt = (_regs[R_CAUSE] >> 2) & 0x0f;
        sigval = computeSignal(tt);
        remcomOutBuffer[0] = 'S';
        remcomOutBuffer[1] = hexchars[sigval >> 4];
        remcomOutBuffer[2] = hexchars[sigval & 0xf];
        remcomOutBuffer[3] = 0;
        break;

        /* toggle debug flag */

        case 'd':
            break;

        /* Return the values of the CPU registers */

        case 'g':
            ptr = remcomOutBuffer;
            ptr = mem2hex((char *)_regs, ptr, 32 * 4, 0);        /* General Purpose Registers */
            ptr = mem2hex((char *)&_regs[R_EPC], ptr, 9 * 4, 0); /* CP0 Registers             */
            break;

        /*  set the value of the CPU registers - return OK */

        case 'G':
            ptr = &remcomInBuffer[1];
            hex2mem(ptr, (char *)_regs, 32 * 4, 0);                     /* General Purpose Registers */
            hex2mem(ptr + 32 * 4 * 2, (char *)&_regs[R_EPC], 9 * 4, 0); /* CP0 Registers             */
            strcpy(remcomOutBuffer,"OK");
            break;

        /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */

        case 'm':
            ptr = &remcomInBuffer[1];
            if (hexToInt(&ptr, &addr) && *ptr++ == ',' && hexToInt(&ptr, &length))
            {
                if (mem2hex((char *)addr, remcomOutBuffer, length, 1))
                    break;
                strcpy (remcomOutBuffer, "E03");
            }
            else
                strcpy(remcomOutBuffer,"E01");
            break;

        /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */

        case 'M':
            ptr = &remcomInBuffer[1];
            if (hexToInt(&ptr, &addr) && *ptr++ == ',' && hexToInt(&ptr, &length) && *ptr++ == ':')
            {
                if (hex2mem(ptr, (char *)addr, length, 1))
                    strcpy(remcomOutBuffer, "OK");
                else
                    strcpy(remcomOutBuffer, "E03");
            }
            else
                strcpy(remcomOutBuffer, "E02");
            break;

        /* cAA..AA    Continue at address AA..AA(optional) */

        case 'c':

        /* try to read optional parameter, pc unchanged if no parm */

            ptr = &remcomInBuffer[1];
            if (hexToInt(&ptr, &addr))
            {
                gdb_go ( addr );
            }
            else
            {
                dbg_cont();
            }
            return;

        /* kill the program */

        case 'k':	
            break;

        /* Reset */

        case 'r':
            break;

	/* switch */

    }

    /* Reply to the request */

    putpacket(remcomOutBuffer);
}


/*
 * gets_debugger_check - This is the same as the stdio gets, but we also
 *                       check for a leading $ in the buffer. This so we
 *                       gracefully handle the GDB protocol packets.
 */

char *
gets_debugger_check(buf)
char *buf;
{
    register char c;
    char *bufp;

    bufp = buf;
    for (;;) 
    {
        c = getchar();
        switch (c) 
        {

            /* quote next char */

            case '$':
                if ( buf == bufp )
                    process_packet();
                break;
                 
            case CTRL(V):
                c = getchar();
                if (bufp < &buf[LINESIZE-3]) 
                {
                    rmw_byte (bufp++,c);
                    showchar(c);
                 } 
                 else
                 {
                    putchar(BELL);
                 }
                 break;

            case '\n':
            case '\r':
                putchar('\n');
                rmw_byte (bufp,0);
                return(buf);

            case CTRL(H):
            case DEL:
                if (bufp > buf) 
                {
                    bufp--;
                    putchar(CTRL(H));
                    putchar(' ');
                    putchar(CTRL(H));
                }
                break;

            case CTRL(U):
                if (bufp > buf) 
                {
                    printf("^U\n%s", PROMPT);
                    bufp = buf;
                }
                break;

            case '\t':
                c = ' ';

            default:
                /*
                 * Make sure there's room for this character
                 * plus a trailing \n and 0 byte
                 */
                if (isprint(c) && bufp < &buf[LINESIZE-3]) 
                {
                    rmw_byte ( bufp++, c );
                    putchar(c);
                } 
                else
                {
                    putchar(BELL);
                }
                break;
        }
    }
}
