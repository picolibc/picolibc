/* minires.c.  Stub synchronous resolver for Cygwin.

   Written by Pierre A. Humblet <Pierre.Humblet@ieee.org>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "minires.h"

/***********************************************************************

Utilities

***********************************************************************/
/***********************************************************************

dprintf
***********************************************************************/
void minires_dprintf(const char * format, ...)
{
  va_list args;

  va_start(args, format);
  fprintf(stderr, "Resolv: ");
  vfprintf(stderr, format, args);
  va_end(args);
}

/***********************************************************************

scanline
Put pointers in list[] to the beginning of each space or comma delimited
word in "in", and put the lengths in sizes[] (counting the final 0).
Return the number of words found
***********************************************************************/
static int scanline(char * in, char **list, int * sizes, int maxnum)
{
  int i;
  char * startp;
  for (i = 0; i < maxnum; i++) {
    while((*in) && (isspace((unsigned char)*in) || *in == ',')) in++;
    if (*in == 0)
      break;
    startp = in++;
    while((*in) && !isspace((unsigned char)*in) && *in != ',') in++;
    list[i] = startp;
    sizes[i] = in - startp + 1;
    if (*in)
      *in++ = 0;
  }
  return i;
}

/***********************************************************************

Read the search string.

***********************************************************************/
void minires_get_search(char * string, res_state statp)
{
  char * words[MAXDNSRCH+1], * ptr;
  int sizes[MAXDNSRCH+1];
  int i, j, debug = statp->options & RES_DEBUG;

  i = scanline(string, words, sizes, MAXDNSRCH+1);
  ptr = statp->defdname;
  for (j = 0; j < i; j++) {
    if (j < MAXDNSRCH
	&& ptr + sizes[j] < &statp->defdname[DIM(statp->defdname)]) {
      statp->dnsrch[j] = strcpy(ptr, words[j]);
      statp->dnsrch[j+1] = NULL;
      ptr += sizes[j];
      DPRINTF(debug, "registry search \"%s\"\n", words[j]);
    }
    else if (j < MAXDNSRCH + 1)
      DPRINTF(debug, "no space for \"%s\"\n", words[j]);
  }
}

/***********************************************************************

Read options


***********************************************************************/
static void get_options(res_state statp, int n, char **words)
{
  char *ptr;
  int i, value;

  for (i = 0;  i < n;  ++i) {
    if (!strcasecmp("debug", words[i])) {
      statp->options |= RES_DEBUG;
      DPRINTF(statp->options & RES_DEBUG, "%s: 1\n", words[i]);
      continue;
    }
    if (!strcasecmp("inet6", words[i])) {
      statp->options |= RES_USE_INET6;
      DPRINTF(statp->options & RES_DEBUG, "%s: 1\n", words[i]);
      continue;
    }
    if (!strcasecmp("osquery", words[i])) {
      statp->use_os = 1;
      DPRINTF(statp->options & RES_DEBUG, "%s: 1\n", words[i]);
      continue;
    }

    if ((ptr = strchr(words[i], ':'))) {
      *ptr++ = 0;
      value = atoi(ptr);
      /* Not supported
	 if (!strcasecmp("ndots", words[i])) {
	 statp->ndots = value;
	 continue;
	 }
      */
      if (!strcasecmp("retry", words[i])
	  || !strcasecmp("attempts", words[i])) {
	if (value < 1)
	  value = 1;
	else if (value > RES_MAXRETRY)
	  value = RES_MAXRETRY;
	statp->retry = value;
	DPRINTF(statp->options & RES_DEBUG, "%s: %d\n", words[i], value);
	continue;
      }
      if (!strcasecmp("retrans", words[i])
	  || !strcasecmp("timeout", words[i])) {
	if (value < 1)
	  value = 1;
	else if (value > RES_MAXRETRANS)
	  value = RES_MAXRETRANS;
	statp->retrans = value;
	DPRINTF(statp->options & RES_DEBUG, "%s: %d\n", words[i], value);
	continue;
      }
    }
    DPRINTF(statp->options & RES_DEBUG, "unknown option: \"%s\"\n", words[i]);
  }
}

/***********************************************************************

Read the resolv.conf file.
We only look for nameserver, domain, search and options

***********************************************************************/
#if MAXNS > MAXDNSRCH + 1
#define MAXSIZE MAXNS
#else
#define MAXSIZE MAXDNSRCH + 1 /* Make unused one visible */
#endif
static void get_resolv(res_state statp)
{
  FILE * fd;
  char *words[MAXSIZE + 1], line[4096], *ptr;
  int sizes[DIM(words)];
  int i, j, ns = 0, have_search, have_address, debug = statp->options & RES_DEBUG;

  fd = fopen(_PATH_RESCONF, "r");
  DPRINTF(debug, _PATH_RESCONF ": %s\n", fd?"OK":strerror(errno));
  if (fd == NULL)
    return;

  statp->use_os = 0; /* Do not use os_query, except if allowed by "options" */
  have_search = (statp->dnsrch[0] != NULL);
  have_address = (statp->nscount != 0);

  while ( fgets(line, sizeof(line), fd) != 0) {
    DPRINTF(debug, _PATH_RESCONF " line: %s", line);
    if ((i = scanline(line, words, sizes, DIM(words))) > 0) {
      if (!have_address
	  && !strncasecmp("nameserver", words[0], sizes[0])) {
	for ( j = 1; j < i ; j++) {
	  in_addr_t address;
	  address = cygwin_inet_addr(words[j]);
	  if (address == INADDR_NONE) {
	    DPRINTF(debug, "invalid server \"%s\"\n", words[j]);
	  }
	  else if (ns >= MAXNS) {
	    DPRINTF(debug, "no space for server \"%s\"\n", words[j]);
	  }
	  else {
	    statp->nsaddr_list[ns++].sin_addr.s_addr = address;
	    statp->nscount++;
	    DPRINTF(debug, "nameserver \"%s\"\n", words[j]);
	  }
	}
      }
      else if (!have_search
	       && (!strncasecmp("search", words[0], sizes[0])
		   || !strncasecmp("domain", words[0], sizes[0]))) {
	ptr = statp->defdname;
	for (j = 0; j + 1 < i; j++) {
	  if (j < MAXDNSRCH
	      && ptr + sizes[j + 1] < &statp->defdname[DIM(statp->defdname)]) {
	    statp->dnsrch[j] = strcpy(ptr, words[j+1]);
	    statp->dnsrch[j+1] = 0;
	    ptr += sizes[j+1];
	    DPRINTF(debug, "domain|search \"%s\"\n", statp->dnsrch[j]);
	  }
	  else {
	    DPRINTF(debug, "no space for \"%s\"\n", words[j+1]);
	  }
	}
      }
      /* Options line */
      else if (!strncasecmp("options", words[0], sizes[0])) {
	get_options(statp, i - 1, &words[1]);
	debug = statp->options & RES_DEBUG;
      }
    }
  }
  fclose(fd);
  return;
}

/****************************************************************************/
/*
   open_sock()
   Create a datagram socket and call bind.

****************************************************************************/

static int open_sock(struct sockaddr_in *CliAddr, int debug)
{
  int fd;

  DPRINTF(debug, "opening UDP socket\n");

  /* Create a datagram socket */
  if ((fd = cygwin_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    DPRINTF(debug, "socket(UDP): %s\n", strerror(errno));
    return -1;
  }
  /* Set non-blocking */
  if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)  {
    DPRINTF(debug, "fcntl: %s\n", strerror(errno));
    return -1;
  }
  /* Set close on exec flag */
  if (fcntl(fd, F_SETFD, 1) == -1) {
    DPRINTF(debug, "fcntl: %s\n", strerror(errno));
    return -1;
  }

  CliAddr->sin_family = AF_INET;
  CliAddr->sin_addr.s_addr = htonl(INADDR_ANY);
  CliAddr->sin_port = htons(0);
  bzero(CliAddr->sin_zero, sizeof(CliAddr->sin_zero));
  /* Get a port */
  if (cygwin_bind(fd, (struct sockaddr *) CliAddr, sizeof(*CliAddr)) < 0) {
    DPRINTF(debug, "bind: %s\n", strerror(errno));
    return -1;
  }

  DPRINTF(debug, "UDP socket sockfd %d\n", fd);
  return fd;
}

/*****************************************************************
 *
 __res_state()
 Undocumented but public. Accessed through _res

 *****************************************************************/
static struct __res_state res;
struct __res_state *__res_state(void)
{
  return & res;
}

/*****************************************************************
 *
 res_init()

 *****************************************************************/
int res_ninit(res_state statp)
{
  int i;

  statp->res_h_errno = NETDB_SUCCESS;
   /* Only debug may be set before calling init */
  statp->options &= RES_DEBUG;
  statp->options |= RES_INIT | RES_DEFAULT;
  statp->nscount = 0;
  statp->os_query = NULL;
  statp->retrans = RES_TIMEOUT; /* timeout in seconds */
  statp->retry = RES_MAXRETRY;  /* max number of retries */
  statp->use_os = 1;            /* use os_query if available and allowed by get_resolv */
  statp->mypid = -1;
  statp->sockfd = -1;
  /* Use the pid and the ppid for random seed, from the point of view of an outsider.
     Mix the upper and lower bits as they are not used equally */
  i = getpid();
  statp->id = (ushort) (getppid() ^ (i << 8) ^ (i >> 8));
  for (i = 0; i < (int) DIM(statp->dnsrch); i++)  statp->dnsrch[i] = 0;

  /* resolv.conf (dns servers & search list)*/
  get_resolv(statp);
  /* Get dns servers and search list from an os-specific routine, set os_query */
  get_dns_info(statp);

  if (statp->nscount == 0 && !statp->os_query) {
    errno = ENONET;
    statp->res_h_errno = NETDB_INTERNAL;
    DPRINTF(statp->options & RES_DEBUG, "no dns server found\n");
    return -1;
  }
  for (i = 0; i < statp->nscount; i++) {
    statp->nsaddr_list[i].sin_family = AF_INET;
    statp->nsaddr_list[i].sin_port = htons(NAMESERVER_PORT);
    bzero(statp->nsaddr_list[i].sin_zero, sizeof(statp->nsaddr_list[i].sin_zero));
  }
  return 0;
}

int res_init()
{
  int r = res_ninit(& res);
  h_errno = res.res_h_errno;
  return r;
}

/*****************************************************************
 *
 res_close()

 *****************************************************************/
void res_nclose(res_state statp)
{
  int res;
  if (statp->sockfd != -1) {
    res = close(statp->sockfd);
    DPRINTF(statp->options & RES_DEBUG, "close sockfd %d: %s\n",
	    statp->sockfd, (res == 0)?"OK":strerror(errno));
    statp->sockfd = -1;
  }
}

void res_close()
{
  res_nclose(& res);
}

/*****************************************************************
 *
 get_tcp_buf()

 *****************************************************************/
static int get_tcp_buf(int fd, unsigned char *buf, int size, int debug)
{
  int res;
  while (size > 0) {
    if ((res = read(fd, buf, size)) < 0) {
      DPRINTF(debug, "read(TCP): %s\n", strerror(errno));
      return -1;
    }
    DPRINTF(debug, "read(TCP) %d out of %d\n", res, size);
    size -= res;
    buf += res;
  }
  return 0;
}

/*****************************************************************
 *
 get_tcp()

 *****************************************************************/
static int get_tcp(struct sockaddr_in *CliAddr,
		   const unsigned char * MsgPtr, int MsgLength,
		   unsigned char * AnsPtr, int AnsLength, int debug)
{
  int fd, res = -1;
  unsigned short ans_length;
  union {short len; u_int8_t buf[sizeof(short)];} len_buf;

  DPRINTF(debug, "retrying with TCP\n");

  /* Create a tcp socket */
  if ((fd = cygwin_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    DPRINTF(debug, "socket(TCP): %s\n", strerror(errno));
    return -1;
  }

  if (cygwin_connect(fd, (struct sockaddr *)  CliAddr, sizeof(* CliAddr)) < 0) {
    DPRINTF(debug, "connect(TCP): %s\n", strerror(errno));
    goto done;
  }

  /* Send the length then the message */
  len_buf.len = htons(MsgLength);
  if (write(fd, len_buf.buf, sizeof(len_buf)) != sizeof(len_buf)
      || write(fd, MsgPtr, MsgLength) != MsgLength) {
    DPRINTF(debug, "write(TCP): %s\n", strerror(errno));
    goto done;
  }

  /* Read the answer length */
  if (get_tcp_buf(fd, len_buf.buf, sizeof(len_buf), debug))
    goto done;
  ans_length = ntohs(len_buf.len);

  /* Read the answer */
  if (get_tcp_buf(fd, AnsPtr, MIN(ans_length, AnsLength), debug))
    goto done;
  res = ans_length;

 done:
  close (fd);
  return res;
}

/*****************************************************************
 **
 res_send
 Assumes that the message is a query starting with a short id.
 Handles retransmissions until that id is received.

*****************************************************************/
int res_nsend( res_state statp, const unsigned char * MsgPtr,
	       int MsgLength, unsigned char * AnsPtr, int AnsLength)
{
  /* Current server, shared by all tasks */
  static volatile unsigned int SServ = 0XFFFFFFFF;
  int tcp;
  const int debug = statp->options & RES_DEBUG;

  fd_set fdset_read;
  int rslt, addrLen, transNum, wServ;
  struct sockaddr_in mySockAddr, dnsSockAddr;
  struct timeval timeOut;

  statp->res_h_errno = NETDB_SUCCESS;
  if (((statp->options & RES_INIT) == 0) && (res_ninit(statp) != 0))
    return -1;

  /* If a hook exists to a native implementation, use it */
  if (statp->os_query) {
    int len;
    short int Class, Type;
    char DomName[MAXDNAME];
    unsigned char * ptr = (unsigned char *) MsgPtr + HFIXEDSZ;
    len = dn_expand(MsgPtr, MsgPtr + MsgLength, ptr, DomName, sizeof(DomName));
    if (len > 0) {
      ptr += len;
      GETSHORT(Type, ptr);
      GETSHORT(Class, ptr);
      if (AnsLength >= 2)
          memcpy(AnsPtr, MsgPtr, 2);
      return ((os_query_t *) statp->os_query)(statp, DomName, Class, Type, AnsPtr, AnsLength);
    }
    else {
      /* dn_expand sets errno */
      statp->res_h_errno = NETDB_INTERNAL;
      return -1;
    }
  }

  /* Close the socket if it had been opened before a fork.
     Reuse of pid's cannot hurt */
  if ((statp->sockfd != -1) && ((pid_t) statp->mypid != getpid())) {
    res_nclose(statp);
  }

  /* Open a socket for this process */
  if (statp->sockfd == -1) {
    /* Create a non-blocking, close on exec socket and bind it (to any port) */
    statp->sockfd = open_sock(& mySockAddr, debug);
    if (statp->sockfd  < 0 ) {
      statp->res_h_errno = NETDB_INTERNAL;
      return -1;
    }
    statp->mypid = getpid();
    if (SServ == 0XFFFFFFFF) /* Pseudo random */
      SServ =  statp->id % statp->nscount;
  }

  transNum = 0;
  while ( transNum++ < statp->retry) {
    if ((wServ = SServ + 1) >= statp->nscount)
      wServ = 0;
    SServ = wServ;

    /* There exists attacks on DNS where many wrong answers with guessed id's and
       spoofed source address and port are generated at about the time when the
       program is tricked into resolving a name.
       This routine runs through the retry loop for each incorrect answer.
       It is thus extremely likely that such attacks will cause a TRY_AGAIN return,
       probably causing the calling program to retry after a delay.

       Note that valid late or duplicate answers to a previous questions also cause
       a retry, although this is minimized by flushing the socket before sending the
       new question.
    */

    /* Flush duplicate or late answers */
    while ((rslt = cygwin_recvfrom( statp->sockfd, AnsPtr, AnsLength, 0, NULL, NULL)) >= 0) {
      DPRINTF(debug, "Flushed %d bytes\n", rslt);
    }
    DPRINTF(debug && (errno != EWOULDBLOCK),
	    "Unexpected errno for flushing recvfrom: %s", strerror(errno));

    /* Send the message */
    rslt = cygwin_sendto(statp->sockfd, MsgPtr, MsgLength, 0,
			 (struct sockaddr *) &statp->nsaddr_list[wServ],
			 sizeof(struct sockaddr_in));
    DPRINTF(debug, "sendto: server %08X:%hu sockfd %d %s\n",
	    ntohl(statp->nsaddr_list[wServ].sin_addr.s_addr),
	    ntohs(statp->nsaddr_list[wServ].sin_port),
	    statp->sockfd, (rslt == MsgLength)?"OK":strerror(errno));
    if (rslt != MsgLength) {
      statp->res_h_errno = NETDB_INTERNAL;
      return -1;
    };
    /*
      Wait for a reply with select()
    */
    FD_ZERO(&fdset_read);
    FD_SET (statp->sockfd, &fdset_read );
    timeOut.tv_sec = statp->retrans;
    timeOut.tv_usec = 0;
    rslt = cygwin_select(statp->sockfd + 1, &fdset_read, NULL, NULL, &timeOut);
    if ( rslt == 0 ) { /* Timeout */
      DPRINTF(statp->options & RES_DEBUG, "timeout for server %08X:%hu sockfd %d\n",
	      ntohl(statp->nsaddr_list[wServ].sin_addr.s_addr),
	      ntohs(statp->nsaddr_list[wServ].sin_port),
	      statp->sockfd);
      continue;
    }
    else if ((rslt != 1) || (FD_ISSET(statp->sockfd, &fdset_read) == 0)) {
      DPRINTF(debug, "select sockfd %d: %s\n", statp->sockfd, strerror(errno));
      statp->res_h_errno = NETDB_INTERNAL;
      return -1;
    }

    addrLen = sizeof(dnsSockAddr);
    rslt = cygwin_recvfrom(statp->sockfd, AnsPtr, AnsLength, 0,
			   (struct sockaddr *) & dnsSockAddr, & addrLen);
    if (rslt <= 0) {
      DPRINTF(debug, "recvfrom sockfd %d: %s\n", statp->sockfd, strerror(errno));
      statp->res_h_errno = NETDB_INTERNAL;
      return -1;
    }
    DPRINTF(debug, "recvfrom: %d bytes from %08X:%hu sockfd %d\n", rslt,
            ntohl(dnsSockAddr.sin_addr.s_addr),
            ntohs(dnsSockAddr.sin_port),
            statp->sockfd);
    /*
       Prepare to retry with tcp
    */
    for (tcp = 0; tcp < 2; tcp++) {
      /* Check if this is the expected message from the expected server */
      if ((memcmp(& dnsSockAddr, & statp->nsaddr_list[wServ],
		  (char *) & dnsSockAddr.sin_zero[0] - (char *) & dnsSockAddr) == 0)
	  && (rslt >= HFIXEDSZ)
	  && (*MsgPtr == *AnsPtr)     /* Ids match */
	  && (*(MsgPtr + 1) == *(AnsPtr + 1))
	  && ((AnsPtr[2] & QR) != 0)
	  && (AnsPtr[4] == 0)
	  /* We check the question if present.
	     Some servers don't return it on error, in particular
	     when the name in the question is not valid. */
	  && (((AnsPtr[5] == 0)
	       && ((AnsPtr[3] & ERR_MASK) != NOERROR))
	      || ((AnsPtr[5] == 1)
		  && (rslt >= MsgLength)
		  && (memcmp(MsgPtr + HFIXEDSZ, AnsPtr + HFIXEDSZ, MsgLength - HFIXEDSZ) == 0)))) {
	if ((AnsPtr[3] & ERR_MASK) == NOERROR) {
	  if ((AnsPtr[2] & TC) && (tcp == 0) && !(statp->options & RES_IGNTC)) {
	    /* Truncated. Try TCP */
	    rslt = get_tcp(&statp->nsaddr_list[wServ], MsgPtr, MsgLength,
			   AnsPtr, AnsLength, statp->options & RES_DEBUG);
	    continue /* Tcp loop */;
	  }
	  else if ((AnsPtr[6] | AnsPtr[7])!= 0)
	    return rslt;
	  else
	    statp->res_h_errno = NO_DATA;
	}
#if 0
 NETDB_INTERNAL -1 /* see errno */
 NETDB_SUCCESS   0 /* no problem */
 HOST_NOT_FOUND  1 /* Authoritative Answer Host not found */
 TRY_AGAIN       2 /* Non-Authoritive Host not found, or SERVERFAIL */
			 Also seen returned by some servers when the name is too long
 NO_RECOVERY     3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
 NO_DATA         4 /* Valid name, no data record of requested type */
#endif
	else {
	  switch (AnsPtr[3] & ERR_MASK) {
	  /* return HOST_NOT_FOUND even for non-authoritative answers */
	  case NXDOMAIN:
	  case FORMERR:
	    statp->res_h_errno = HOST_NOT_FOUND;
	    break;
	  case SERVFAIL:
	    statp->res_h_errno = TRY_AGAIN;
	    break;
	  default:
	    statp->res_h_errno = NO_RECOVERY;
	  }
	}
	return -1;
      }
      else {
	DPRINTF(debug, "unexpected answer\n");
	break;
      }
    } /* TCP */
  }
  DPRINTF(debug, "too many retries\n");
  statp->res_h_errno = TRY_AGAIN;
  return -1;
}

int res_send( const unsigned char * MsgPtr, int MsgLength,
	      unsigned char * AnsPtr, int AnsLength)
{
  int r = res_nsend(& res, MsgPtr, MsgLength, AnsPtr, AnsLength);
  h_errno = res.res_h_errno;
  return r;
}

/*****************************************************************
 *
 res_mkquery

 Return: packet size
	-1 name format is incorrect
*****************************************************************/
int res_nmkquery (res_state statp,
		  int op, const char * dnameptr, int qclass, int qtype,
		  const unsigned char * dataptr __attribute__ ((unused)),
		  int datalen __attribute__ ((unused)),
		  const unsigned char * newrr __attribute__ ((unused)),
		  unsigned char * buf, int buflen)
{
  int i, len;
  const char * ptr;
  unsigned int id4;

  if (op == QUERY) {
    /* Write the name and verify buffer length */
    len = dn_comp(dnameptr, buf + HFIXEDSZ, buflen - HFIXEDSZ - QFIXEDSZ, NULL, NULL);
    if (len < 0) {
      DPRINTF(statp->options & RES_DEBUG,
	      "\"%s\" invalid or buffer too short\n", dnameptr);
      statp->res_h_errno = NETDB_INTERNAL;
      return -1;
    }

    /* Fill the header */
    PUTSHORT(statp->id, buf);
    PUTSHORT(RD, buf);
    PUTSHORT(1, buf); /* Number of questions */
    for (i = 0; i < 3; i++)
      PUTSHORT(0, buf); /* Number of answers */

    /* Write qtype and qclass */
    buf += len;
    PUTSHORT(qtype, buf);
    PUTSHORT(qclass, buf);

    /* Update id. The current query adds entropy to the next query id */
    for (id4 = qtype, i = 0, ptr = dnameptr; *ptr; ptr++, i += 3)
      id4 ^= *ptr << (i & 0xF);
    i = 1 + statp->id % 15; /* Between 1 and 16 */
    /* id dependent rotation, also brings MSW to LSW */
    id4 = (id4 << i) ^ (id4 >> (16 - i)) ^ (id4 >> (32 - i));
    if ((short) id4)
      statp->id ^= (short) id4;
    else
      statp->id++; /* Force change */

    return len + (HFIXEDSZ + QFIXEDSZ); /* packet size */
  }
  else { /* Not implemented */
    errno = ENOSYS;
    statp->res_h_errno = NETDB_INTERNAL;
    return -1;
  }
}

int res_mkquery (int op, const char * dnameptr, int qclass, int qtype,
		 const unsigned char * dataptr, int datalen,
		 const unsigned char * newrr, unsigned char * buf, int buflen)
{
  int r = res_nmkquery (& res, op, dnameptr, qclass, qtype,
			dataptr, datalen, newrr, buf, buflen);
  h_errno = res.res_h_errno;
  return r;

}

/*****************************************************************
 * res_query()
 *****************************************************************/

int res_nquery( res_state statp, const char * DomName, int Class, int Type,
		unsigned char * AnsPtr, int AnsLength)
{
  u_int8_t packet[PACKETSZ];
  int len;

  DPRINTF(statp->options & RES_DEBUG, "query \"%s\" type %d\n", DomName, Type);
  statp->res_h_errno = NETDB_SUCCESS;

  /* If a hook exists to a native implementation, use it */
  if (statp->os_query) {
    if (AnsLength >= 2)
        memset(AnsPtr, 0/*Id*/, 2);
    return ((os_query_t *) statp->os_query)(statp, DomName, Class, Type, AnsPtr, AnsLength);
  }

  if ((len = res_nmkquery (statp, QUERY, DomName, Class, Type,
			   0, 0, 0, packet, PACKETSZ)) < 0)
    return -1;
  return res_nsend( statp, packet, len, AnsPtr, AnsLength);
}

int res_query( const char * DomName, int Class, int Type, unsigned char * AnsPtr, int AnsLength)
{
  int r = res_nquery(& res, DomName, Class, Type, AnsPtr, AnsLength);
  h_errno = res.res_h_errno;
  return r;
}

/*****************************************************************
 * res_querydomain()
 *****************************************************************/
int res_nquerydomain( res_state statp, const char * Name, const char * DomName,
		      int Class, int Type, unsigned char * AnsPtr, int AnsLength)
{
  char fqdn[MAXDNAME], *ptr;
  size_t nlen;

  DPRINTF(statp->options & RES_DEBUG, "querydomain \"%s\"  \"%s\" type %d\n",
	  Name, DomName, Type);

  if (!DomName)
    ptr = (char *) Name;
  else if ((nlen = strlen(Name)) >= sizeof(fqdn) - 1)
    goto error;
  else {
    strcpy(fqdn, Name);
    ptr = &fqdn[nlen];
    if (nlen && *(ptr - 1) != '.')
      *ptr++ = '.';
    fqdn[sizeof(fqdn) - 1] = 0;
    strncpy(ptr, DomName, sizeof(fqdn) - (ptr - fqdn));
    if (fqdn[sizeof(fqdn) - 1])
      goto error;
    ptr = fqdn;
  }
  return res_nquery(statp, ptr, Class, Type, AnsPtr, AnsLength);

 error:
  DPRINTF(statp->options & RES_DEBUG, "querydomain: name too long\n");
  errno = EINVAL;
  statp->res_h_errno = NETDB_INTERNAL;;
  return -1;
}

int res_querydomain( const char * Name, const char * DomName, int Class,
		     int Type, unsigned char * AnsPtr, int AnsLength)
{
  int r = res_nquerydomain(& res, Name, DomName, Class, Type, AnsPtr,
			   AnsLength);
  h_errno = res.res_h_errno;
  return r;
}

/*****************************************************************
 *
 res_search()

 *****************************************************************/

int res_nsearch( res_state statp, const char * DomName, int Class, int Type,
		 unsigned char * AnsPtr, int AnsLength)
{
  int len, stat, i;
  char fullDomName[MAXDNAME], *ptr, *sptr;

  DPRINTF(statp->options & RES_DEBUG, "search \"%s\" type %d\n", DomName, Type);

  if (((statp->options & RES_INIT) == 0) && (res_ninit(statp) != 0))
    return -1;

  stat = res_nquery( statp, DomName, Class, Type, AnsPtr, AnsLength);

  /* Check if will skip search */
  if (statp->res_h_errno != HOST_NOT_FOUND               /* Success or hard failure */
      || ((ptr = strrchr(DomName, '.')) && (!*(ptr+1)))  /* Final dot */
      || (((statp->options & RES_DNSRCH) == 0)           /* Or no search */
	  && ((ptr != NULL)                              /*  And some dot */
	      || ((statp->options & RES_DEFNAMES) == 0)))/*    or no def domain */
      || (!(sptr = statp->dnsrch[0])))
    return stat;

  len = strlen(DomName);
  if (len >= MAXDNAME - 1) /* Space for next dot */
    goto error;
  strcpy(fullDomName, DomName);
  fullDomName[len++] = '.';
  fullDomName[MAXDNAME - 1] = 0; /* Overflow indicator */
  i = 0;
  do {
    strncpy(fullDomName + len, sptr, MAXDNAME - len);
    if (fullDomName[MAXDNAME - 1])
      goto error;
    stat = res_nquery(statp, fullDomName, Class, Type, AnsPtr, AnsLength);
  } while ((sptr = statp->dnsrch[++i]) != NULL
	   && statp->res_h_errno == HOST_NOT_FOUND
	   && (statp->options & RES_DNSRCH) != 0);

  /* Return last stat */
  return stat;

 error:
  DPRINTF(statp->options & RES_DEBUG, "name too long during search\n");
  errno = EINVAL;
  statp->res_h_errno = NETDB_INTERNAL;
  return -1;
}

int res_search( const char * DomName, int Class, int Type,
		unsigned char * AnsPtr, int AnsLength)
{
  int r = res_nsearch(& res, DomName, Class, Type, AnsPtr, AnsLength);
  h_errno = res.res_h_errno;
  return r;
}

/*****************************************************************
 * dn_expand
 *****************************************************************/

int dn_expand(const unsigned char *msg, const unsigned char *eomorig,
	      const unsigned char *comp_dn, char *exp_dn, int length)
{
  unsigned int len, complen = 0;
  const unsigned char *comp_dn_orig = comp_dn;

  if (comp_dn >= eomorig)
    goto expand_fail;
  if ((len = *comp_dn++) == 0)       /* Weird case */
    exp_dn++;
  else do {
    if (len <= MAXLABEL) {
      if ((length -= (len + 1)) >= 0 /* Need space for final . */
	  && comp_dn + len <= eomorig) {
	do { *exp_dn++ = *comp_dn++; } while (--len != 0);
	*exp_dn++ = '.';
      }
      else
	goto expand_fail;
    }
    else if (len >= (128+64)) {
      if (!complen)   /* Still in the original field? */
	complen = (comp_dn - comp_dn_orig) + 1;
      comp_dn = msg + (((len & ~(128+64)) << 8) + *comp_dn);
      if (comp_dn >= eomorig)
	goto expand_fail;
    }
    else
      goto expand_fail;
  } while ((len = *comp_dn++) != 0);
  /* Replace last . with a 0 */
  *(--exp_dn) = 0;
  if (!complen)
    complen = comp_dn - comp_dn_orig;
/*  fprintf(stderr, "dn_expand %s\n", exp_start); */
  return complen;

expand_fail:
  errno = EINVAL;
  return -1;
}

/*****************************************************************
 *
 dn_comp

 Return -1 in case of overflow, but still fill buffer correctly.
 We do not check the alphabet of the host names
 nor the length of the compressed name and we
 preserve the letter cases.

 *****************************************************************/
int dn_comp(const char * exp_dn, unsigned char * comp_dn, int length,
	    unsigned char ** dnptrs, unsigned char ** lastdnptr)
{
  unsigned char *cptr = comp_dn, *dptr, *lptr, *rptr;
  unsigned int i, len;
  unsigned char * const eptr = comp_dn + length - 1; /* Last valid */

  errno = EINVAL;

  if (*exp_dn == '.' && !*(exp_dn + 1))
    exp_dn++;
  while (1) {
    if (*exp_dn == '.' || cptr > eptr)
      return -1;
    if (*exp_dn == 0) {
      *cptr++ = 0;
      break;
    }
    /* Try to compress */
    if (dnptrs) {
      for (i = 1; dnptrs[i]; i++) {
	dptr = dnptrs[i];
	if (dptr >= comp_dn) /* Handle name.name */
	  continue;
	rptr = (unsigned char *) exp_dn;
	len = *dptr++;
	while (1) {
	  do {
	    if (*dptr++ != *rptr++)
	      goto next_dn;
	  } while (--len);
	  len = *dptr++;
	  if (len == 0) { /* last label */
	    if (!*rptr || (*rptr == '.' && !*(rptr + 1))) { /* Full match */
	      len = (dnptrs[i] - dnptrs[0]) | 0xC000;
	      /* Write pointer */
	      *cptr++ = len >> 8;
	      if (cptr > eptr)
		return -1;
	      *cptr++ = len;
	      goto done;
	    }
	    goto next_dn;
	  }
	  if (*rptr++ != '.')
	    goto next_dn;
	  if (len >= 128 + 64) {
	    dptr = dnptrs[0] + ((len - 128 - 64) << 8) + *dptr;
	    len = *dptr++;
	  }
	}
      next_dn: ;
      }
      /* Record label if asked and if space is available and if not too far off */
      if (lastdnptr && (lastdnptr != &dnptrs[i]) && (cptr - dnptrs[0]) < 0xC000) {
	dnptrs[i] = cptr;
	dnptrs[i+1] = NULL;
      }
    }
    /* Write label */
    lptr = cptr++; /* Length byte */
    rptr = (unsigned char *) exp_dn;
    do {
      if (cptr <= eptr)
	*cptr++ = *rptr;
    } while ((*++rptr != '.') && (*rptr != 0));
    len = rptr - (unsigned char *) exp_dn;
    if (len > MAXLABEL)
      return -1;
    *lptr = len;
    exp_dn = (char *) rptr;
    if (*exp_dn != 0)
      exp_dn++; /* Skip over . */
  }
 done:
  return cptr - comp_dn;
}

/*****************************************************************
 * dn_skipname

 Measures the compressed domain name length and returns it.
 *****************************************************************/
int dn_skipname(const unsigned char *comp_dn, const unsigned char *eom)
{
  int len;
  const unsigned char *comp_dn_orig = comp_dn;

  do {
    len = *comp_dn++;
    if (len >= (128 + 64)) {
      comp_dn++;
      break;
    }
    if (len > MAXLABEL ||
	(comp_dn += len) > eom)
      return -1;
  } while (len != 0);

  return comp_dn - comp_dn_orig;
}

/*****************************************************************
 * dn_length1    For internal use

 Return length of uncompressed name incl final 0.
 *****************************************************************/

int dn_length1(const unsigned char *msg, const unsigned char *eomorig,
	       const unsigned char *comp_dn)
{
  unsigned int len, length = 0;

  errno = EINVAL;
  if (comp_dn >= eomorig)
    goto expand_fail;
  else while ((len = *comp_dn++) != 0) {
    if (len <= MAXLABEL) {
      if ((comp_dn += len) <= eomorig)
	length += len + 1;
      else
	goto expand_fail;
    }
    else if (len >= (128+64)) {
      comp_dn = msg + (((len & ~(128+64)) << 8) + *comp_dn);
      if (comp_dn >= eomorig)
	goto expand_fail;
    }
    else
      goto expand_fail;
  }
  return length;

expand_fail:
  return -1;
}
