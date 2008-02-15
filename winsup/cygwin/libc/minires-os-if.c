/* minires-os-if.c.  Stub synchronous resolver for Cygwin.

   Copyright 2006, 2007 Red Hat, Inc.

   Written by Pierre A. Humblet <Pierre.Humblet@ieee.org>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "minires.h"

#ifdef __CYGWIN__
/***********************************************************************
 *
 Windows interface code

***********************************************************************/

/* Conflict between Windows definitions and others */
#undef ERROR
#undef NOERROR
#undef DELETE

#include <windows.h>
#include <iphlpapi.h>
#include <windns.h>
#include <sys/cygwin.h>

/***********************************************************************
 * write_record: Translates a Windows DNS record into a compressed record
 ***********************************************************************/

#define PUTDOMAIN(d,p)\
 {int res = dn_comp(d, p, EndPtr - p, dnptrs, lastdnptr); p += res < 0 ? strlen(d) : res; }

static u_char * write_record(unsigned char * ptr, PDNS_RECORD rr, unsigned char * EndPtr,
			   unsigned char ** dnptrs, unsigned char ** lastdnptr, int debug)
{
  u_char * rd_length_ptr;

  PUTDOMAIN(rr->pName, ptr);

  if (ptr + 4 > EndPtr)
    ptr += 4;
  else {
    PUTSHORT(rr->wType, ptr);
    PUTSHORT(ns_c_in, ptr);
  }
  if ((rr->Flags.DW & 0x3) == DnsSectionQuestion)
    return ptr;

  if (ptr + 4 > EndPtr)
    ptr += 4;
  else {
    PUTLONG(rr->dwTtl, ptr);
  }
  rd_length_ptr = ptr;
  ptr += 2; /* Placeholder for RDLENGTH */

  /* The default case uses an undocumented feature of the Windows
     resolver for types greater than 16.
     The DNS_RECORD Data contains the record in wire format. */

  switch(rr->wType) {
  case DNS_TYPE_A:
  {
    u_char * aptr = (u_char *) & rr->Data.A.IpAddress;
    if (ptr + 4 <= EndPtr) {
      ptr[0] = aptr[0];
      ptr[1] = aptr[1];
      ptr[2] = aptr[2];
      ptr[3] = aptr[3];
    }
    ptr += 4;
    break;
  }
  case DNS_TYPE_NS:
  case DNS_TYPE_MD:
  case DNS_TYPE_MF:
  case DNS_TYPE_CNAME:
  case DNS_TYPE_MB:
  case DNS_TYPE_MG:
  case DNS_TYPE_MR:
  case DNS_TYPE_PTR:
    PUTDOMAIN(rr->Data.PTR.pNameHost, ptr);
    break;
  case DNS_TYPE_SOA:
    PUTDOMAIN(rr->Data.SOA.pNamePrimaryServer, ptr);
    PUTDOMAIN(rr->Data.SOA.pNameAdministrator, ptr);
    if (ptr + 20 > EndPtr)
      ptr += 20;
    else {
      PUTLONG(rr->Data.SOA.dwSerialNo, ptr);
      PUTLONG(rr->Data.SOA.dwRefresh, ptr);
      PUTLONG(rr->Data.SOA.dwRetry, ptr);
      PUTLONG(rr->Data.SOA.dwExpire, ptr);
      PUTLONG(rr->Data.SOA.dwDefaultTtl, ptr);
    }
    break;
  case DNS_TYPE_NULL:
    if (ptr +  rr->Data.Null.dwByteCount <= EndPtr)
      memcpy(ptr, rr->Data.Null.Data, rr->Data.Null.dwByteCount);
    ptr += rr->Data.Null.dwByteCount;
    if (rr->Data.Null.dwByteCount == rr->wDataLength - sizeof(DNS_NULL_DATA) + 1)
      DPRINTF(debug, "Null byte count has an unexpected value\n");
    break;
  case DNS_TYPE_WKS:
    if (ptr + rr->wDataLength - sizeof(DNS_WKS_DATA) + 1 + 5 > EndPtr)
      ptr += rr->wDataLength - sizeof(DNS_WKS_DATA) + 1 + 5;
    else {
      PUTLONG(rr->Data.WKS.IpAddress, ptr);
      *ptr++ = rr->Data.WKS.chProtocol;
      memcpy(ptr, rr->Data.WKS.BitMask, rr->wDataLength - sizeof(DNS_WKS_DATA) + 1);
      ptr += rr->wDataLength - sizeof(DNS_WKS_DATA) + 1;
    }
    break;
  case DNS_TYPE_MINFO:
  case DNS_TYPE_RP:
    PUTDOMAIN(rr->Data.MINFO.pNameMailbox, ptr);
    PUTDOMAIN(rr->Data.MINFO.pNameErrorsMailbox, ptr);
    break;
  case DNS_TYPE_MX:
  case DNS_TYPE_AFSDB:
  case DNS_TYPE_RT:
    if (ptr + 2 > EndPtr)
      ptr += 2;
    else
      PUTSHORT(rr->Data.MX.wPreference, ptr);
    PUTDOMAIN(rr->Data.MX.pNameExchange, ptr);
    break;
  case DNS_TYPE_HINFO:
  case DNS_TYPE_ISDN:
  case DNS_TYPE_TEXT:
  case DNS_TYPE_X25:
  {
    unsigned int i, len;
    for (i = 0; i < rr->Data.TXT.dwStringCount; i++) {
      len = strlen(rr->Data.TXT.pStringArray[i]) & 0xFF;
      if (ptr + len + 1 > EndPtr)
	ptr += len + 1;
      else {
	*ptr++ = len;
	memcpy(ptr, rr->Data.TXT.pStringArray[i], len);
	ptr += len;
      }
    }
    break;
  }
  case DNS_TYPE_SRV:
    if (ptr + 6 > EndPtr)
      ptr += 6;
    else {
      PUTSHORT(rr->Data.SRV.wPriority, ptr);
      PUTSHORT(rr->Data.SRV.wWeight, ptr);
      PUTSHORT(rr->Data.SRV.wPort, ptr);
    }
    PUTDOMAIN(rr->Data.SRV.pNameTarget, ptr);
    break;
  default:
  {
    unsigned int len = rr->wDataLength;
    DPRINTF(debug, "No structure for wType %d\n", rr->wType);
    if (ptr + len <= EndPtr)
      memcpy(ptr, (char *) &rr->Data, len);
    ptr += len;
    break;
  }
  }
  if (rd_length_ptr + 2 <= EndPtr)
    PUTSHORT(ptr - rd_length_ptr - 2, rd_length_ptr);
  return ptr;
}

/***********************************************************************
 *
 cygwin_query: implements res_nquery by calling DnsQuery

 ***********************************************************************/
static int cygwin_query(res_state statp, const char * DomName, int Class, int Type,
			unsigned char * AnsPtr, int AnsLength)
{
  DNS_STATUS res;
  PDNS_RECORD pQueryResultsSet, rr;
  int section, len, counts[4] = {0, 0, 0, 0}, debug = statp->options & RES_DEBUG;
  unsigned char * dnptrs[256], * ptr;

  dnptrs[0] = AnsPtr;
  dnptrs[1] = NULL;

  if (Class != ns_c_in) {
    errno = ENOSYS;
    statp->res_h_errno = NETDB_INTERNAL;
    return -1;
  }

  res = DnsQuery_A(DomName, Type, DNS_QUERY_TREAT_AS_FQDN,
		   NULL, &pQueryResultsSet, NULL);
#if 0
#define NETDB_INTERNAL -1 /* see errno */
#define HOST_NOT_FOUND  1 /* Authoritative Answer Host not found */
#define TRY_AGAIN       2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define NO_RECOVERY     3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define NO_DATA		4 /* Valid name, no data record of requested type */
#endif

  DPRINTF(debug, "DnsQuery: %lu (Windows)\n", res);
  if (res) {
    switch (res) {
    case ERROR_INVALID_NAME:
      errno = EINVAL;
      statp->res_h_errno = NETDB_INTERNAL;;
      break;
    case ERROR_TIMEOUT:
      statp->res_h_errno = TRY_AGAIN;
      break;
    case DNS_ERROR_RCODE_NAME_ERROR:
      statp->res_h_errno = HOST_NOT_FOUND;
      break;
    case DNS_ERROR_RCODE_SERVER_FAILURE:
      statp->res_h_errno = TRY_AGAIN;
      break;
    case DNS_ERROR_NO_DNS_SERVERS:
    case DNS_ERROR_RCODE_FORMAT_ERROR:
    case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
    case DNS_ERROR_RCODE_REFUSED:
      statp->res_h_errno = NO_RECOVERY;
      break;
    case DNS_INFO_NO_RECORDS:      /* May be returned even if the host doesn't exist */
      statp->res_h_errno = NO_DATA;
      break;
    default:
      DPRINTF(debug, "Unknown code %lu for %s %d\n", res, DomName, Type);
      statp->res_h_errno = NO_RECOVERY;
      break;
    }
    len = -1;
    goto done;
  }

  ptr = AnsPtr + HFIXEDSZ; /* Skip header */

  rr = pQueryResultsSet;
  section = 0;
  while (rr) {
    if (!counts[0] && (rr->Flags.DW & 0x3)) {
      /* No question. Adopt the first name as the name in the question */
      if ((len = dn_comp(rr->pName, ptr, AnsLength - 4,
			 dnptrs, &dnptrs[DIM(dnptrs) - 1])) < 0) {
	ptr = NULL;
	break;
      }
      ptr += len;
      PUTSHORT(Type, ptr);
      PUTSHORT(ns_c_in, ptr);
      counts[0] = 1;
    }

    DPRINTF(debug, "%s Section %d Type %u Windows Record Length %u\n",
	    rr->pName, rr->Flags.DW & 0x3, rr->wType, rr->wDataLength);

    /* Check the records are in correct section order */
    if ((rr->Flags.DW & 0x3) < section) {
      DPRINTF(debug, "Unexpected section order %s %d\n", DomName, Type);
      continue;
    }
    section =  rr->Flags.DW & 0x3;

    ptr = write_record(ptr, rr, AnsPtr + AnsLength, dnptrs,
		       &dnptrs[DIM(dnptrs) - 1], debug);

    counts[section]++;
    rr = rr->pNext;
  }

  DnsRecordListFree(pQueryResultsSet, DnsFreeRecordList);

  len = ptr - AnsPtr;
done:
  ptr = AnsPtr;
  PUTSHORT(0, ptr); /* Id */
  PUTSHORT((QR << 8) + RA + RD, ptr);
  for (section = 0; section < DIM(counts); section++) {
    PUTSHORT(counts[section], ptr);
  }
  return len;
}

/***********************************************************************
 *
 get_registry_items: returns dns items from the registry

 kHey: Handle to registry key
 KeyValue: key value to read
 what: 0 addresses ; 1 search list

***********************************************************************/
static void get_registry_dns_items(HKEY hKey, LPCTSTR KeyValue,
				   res_state statp, int what)
{
  DWORD size = 0;
  LONG res;
  LPBYTE list;
  int debug = statp->options & RES_DEBUG;

  res = RegQueryValueEx( hKey, KeyValue, NULL, NULL, NULL, &size);
  DPRINTF(debug, "value %s, error %lu (Windows), size %lu\n",
	  KeyValue, res, size);
  if ((res == ERROR_SUCCESS) && (size > 1)) {
    if (!(list = (LPBYTE) alloca(size))) {
      DPRINTF(debug, "alloca: %s\n", strerror(errno));
    }
    else if ((res = RegQueryValueEx( hKey, KeyValue, NULL, NULL, list,
				     &size )) != ERROR_SUCCESS) {
      DPRINTF(debug, "RegQueryValueEx: error %lu (Windows)\n", res);
    }
    else if (what == 0) { /* Get the addresses */
      BYTE *ap, *srch;
      int numAddresses = 0;
      for (ap = list; ap < list + size && *ap; ap = srch) {
	/* The separation character can be 0, ' ', or ','. */
	for (srch = ap; *srch && (isdigit(*srch) || *srch == '.' ); srch++);
	*srch++ = 0;
	if (numAddresses < DIM(statp->nsaddr_list)) {
	  DPRINTF(debug, "server \"%s\"\n", ap);
	  statp->nsaddr_list[numAddresses].sin_addr.s_addr = cygwin_inet_addr(ap);
	  if ( statp->nsaddr_list[numAddresses].sin_addr.s_addr != 0 )
	    numAddresses++;
	}
	else
	  DPRINTF(debug, "no space for server \"%s\"\n", ap);
      }
      statp->nscount = numAddresses;
    }
    else /* Parse the search line */
      minires_get_search(list, statp);
  }
  return;
}

/***********************************************************************
 *
 get_registry_dns:

 Read the registry to get dns server addresses in Network Byte Order,
    and set statp->nscount (for NT <= 4.0)
 Read the registry SearchList

***********************************************************************/

static void get_registry_dns(res_state statp)
{
  HKEY hKey;
  DWORD  res;
  const char *keyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";

  DPRINTF(statp->options & RES_DEBUG, "key %s\n", keyName);
  if ((res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, keyName, 0,
			   KEY_QUERY_VALUE | KEY_READ, &hKey)) != ERROR_SUCCESS) {
    DPRINTF(statp->options & RES_DEBUG, "RegOpenKeyEx: error %lu (Windows)\n", res);
    return;
  }

  if (statp->nscount == 0)
    get_registry_dns_items(hKey, "NameServer", statp, 0);
  if (statp->nscount == 0)
    get_registry_dns_items(hKey, "DhcpNameServer", statp, 0);
  if (statp->dnsrch[0] == NULL)
    get_registry_dns_items(hKey, "SearchList", statp, 1);

  RegCloseKey(hKey);

  return;
}

/***********************************************************************
 *
 get_dns_info: Get the search list or the domain name
	       and the dns server addresses in Network Byte Order
	       Set statp->os_query if DnsQuery is available.

***********************************************************************/
void get_dns_info(res_state statp)
{
#if MAX_HOSTNAME_LEN > MAXHOSTNAMELEN
#define MAX_HOSTNAME_SIZE (MAX_HOSTNAME_LEN + 1)
#else
#define MAX_HOSTNAME_SIZE (MAXHOSTNAMELEN + 1)
#endif
#if MAX_HOSTNAME_SIZE > 256 /* sizeof(defdname) */
#error stap->defdname too short
#endif

  int res, debug = statp->options & RES_DEBUG;

  ULONG ulOutBufLen = 0;
  DWORD dwRetVal;
  IP_ADDR_STRING * pIPAddr;
  FIXED_INFO * pFixedInfo;
  int numAddresses = 0;

  if (statp->use_os
      && ((dwRetVal = DnsQuery_A(NULL, 0, 0, NULL, NULL, NULL)) != ERROR_PROC_NOT_FOUND))
  {
    DPRINTF(debug, "using dnsapi.dll %d\n", dwRetVal);
    statp->os_query = (typeof(statp->os_query)) cygwin_query;
    /* We just need the search list. Avoid loading iphlpapi. */
    statp->nscount = -1;
  }

  if (statp->nscount != 0)
    goto use_registry;

  /* First call to get the buffer length we need */
  dwRetVal = GetNetworkParams((FIXED_INFO *) 0, &ulOutBufLen);
  if (dwRetVal != ERROR_BUFFER_OVERFLOW) {
    DPRINTF(debug, "GetNetworkParams: error %lu (Windows)\n", dwRetVal);
    goto use_registry;
  }
  if ((pFixedInfo = (FIXED_INFO *) alloca(ulOutBufLen)) == 0) {
    DPRINTF(debug, "alloca: %s\n", strerror(errno));
    goto use_registry;
  }
  if ((dwRetVal = GetNetworkParams(pFixedInfo, & ulOutBufLen))) {
    DPRINTF(debug, "GetNetworkParams: error %lu (Windows)\n", dwRetVal);
    goto use_registry;
  }

  DPRINTF(debug, "GetNetworkParams: OK\n");
  /* Record server addresses, up to array size */
  for (pIPAddr = &(pFixedInfo->DnsServerList), numAddresses = 0;
       pIPAddr;
       pIPAddr = pIPAddr->Next) {
    if (numAddresses < DIM(statp->nsaddr_list)) {
	DPRINTF(debug, "server \"%s\"\n", pIPAddr->IpAddress.String);
      statp->nsaddr_list[numAddresses].sin_addr.s_addr = cygwin_inet_addr(pIPAddr->IpAddress.String);
      if (statp->nsaddr_list[numAddresses].sin_addr.s_addr != 0) {
	numAddresses++;
	statp->nscount++;
      }
    }
    else
      DPRINTF(debug, "no space for server \"%s\"\n", pIPAddr->IpAddress.String);
  }

 use_registry:
  get_registry_dns(statp);

  if (!statp->dnsrch[0]) {
    statp->defdname[sizeof(statp->defdname) - 1] = 0;
    if (!(res = getdomainname(statp->defdname, sizeof(statp->defdname)))) {
      if (statp->defdname[0] && !statp->defdname[sizeof(statp->defdname) - 1])
	statp->dnsrch[0] = statp->defdname;
    }
    DPRINTF(debug, "getdomainname \"%s\"\n",
	    (res)? strerror(errno) : statp->defdname);
  }
}

#else
/***********************************************************************
 *
 Default interface code

***********************************************************************/

void get_dns_info(res_state statp)
{
  return;
}

#endif



#if 0
#define DNS_ERROR_RCODE_FORMAT_ERROR 9001L
#define DNS_ERROR_RCODE_SERVER_FAILURE 9002L
#define DNS_ERROR_RCODE_NAME_ERROR 9003L
#define DNS_ERROR_RCODE_NOT_IMPLEMENTED 9004L
#define DNS_ERROR_RCODE_REFUSED 9005L
#define DNS_ERROR_RCODE_YXDOMAIN 9006L
#define DNS_ERROR_RCODE_YXRRSET 9007L
#define DNS_ERROR_RCODE_NXRRSET 9008L
#define DNS_ERROR_RCODE_NOTAUTH 9009L
#define DNS_ERROR_RCODE_NOTZONE 9010L
#define DNS_ERROR_RCODE_BADSIG 9016L
#define DNS_ERROR_RCODE_BADKEY 9017L
#define DNS_ERROR_RCODE_BADTIME 9018L
#define DNS_INFO_NO_RECORDS 9501L
#define DNS_ERROR_BAD_PACKET 9502L
#define DNS_ERROR_NO_PACKET 9503L
#define DNS_ERROR_RCODE 9504L
#define DNS_ERROR_UNSECURE_PACKET 9505L
#define DNS_ERROR_INVALID_TYPE 9551L
#define DNS_ERROR_INVALID_IP_ADDRESS 9552L
#define DNS_ERROR_INVALID_PROPERTY 9553L
#define DNS_ERROR_TRY_AGAIN_LATER 9554L
#define DNS_ERROR_NOT_UNIQUE 9555L
#define DNS_ERROR_NON_RFC_NAME 9556L
#define DNS_STATUS_FQDN 9557L
#define DNS_STATUS_DOTTED_NAME 9558L
#define DNS_STATUS_SINGLE_PART_NAME 9559L
#define DNS_ERROR_INVALID_NAME_CHAR 9560L
#define DNS_ERROR_NUMERIC_NAME 9561L
#define DNS_ERROR_NOT_LALOWED_ON_ROOT_SERVER 9562L
#define DNS_ERROR_NOT_ALLOWED_UNDER_DELEGATION 9563L
#define DNS_ERROR_CANNOT_FIND_ROOT_HINTS 9564L
#define DNS_ERROR_INCONSISTENT_ROOT_HINTS 9565L
#define DNS_ERROR_ZONE_DOES_NOT_EXIST 9601L
#define DNS_ERROR_NO_ZONE_INFO 9602L
#define DNS_ERROR_INVALID_ZONE_OPERATION 9603L
#define DNS_ERROR_ZONE_CONFIGURATION_ERROR 9604L
#define DNS_ERROR_ZONE_HAS_NO_SOA_RECORD 9605L
#define DNS_ERROR_ZONE_HAS_NO_NS_RECORDS 9606L
#define DNS_ERROR_ZONE_LOCKED 9607L
#define DNS_ERROR_ZONE_CREATION_FAILED 9608L
#define DNS_ERROR_ZONE_ALREADY_EXISTS 9609L
#define DNS_ERROR_AUTOZONE_ALREADY_EXISTS 9610L
#define DNS_ERROR_INVALID_ZONE_TYPE 9611L
#define DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP 9612L
#define DNS_ERROR_ZONE_NOT_SECONDARY 9613L
#define DNS_ERROR_NEED_SECONDARY_ADDRESSES 9614L
#define DNS_ERROR_WINS_INIT_FAILED 9615L
#define DNS_ERROR_NEED_WINS_SERVERS 9616L
#define DNS_ERROR_NBSTAT_INIT_FAILED 9617L
#define DNS_ERROR_SOA_DELETE_INVALID 9618L
#define DNS_ERROR_FORWARDER_ALREADY_EXISTS 9619L
#define DNS_ERROR_ZONE_REQUIRES_MASTER_IP 9620L
#define DNS_ERROR_ZONE_IS_SHUTDOWN 9621L
#define DNS_ERROR_PRIMARY_REQUIRES_DATAFILE 9651L
#define DNS_ERROR_INVALID_DATAFILE_NAME 9652L
#define DNS_ERROR_DATAFILE_OPEN_FAILURE 9653L
#define DNS_ERROR_FILE_WRITEBACK_FAILED 9654L
#define DNS_ERROR_DATAFILE_PARSING 9655L
#define DNS_ERROR_RECORD_DOES_NOT_EXIST 9701L
#define DNS_ERROR_RECORD_FORMAT 9702L
#define DNS_ERROR_NODE_CREATION_FAILED 9703L
#define DNS_ERROR_UNKNOWN_RECORD_TYPE 9704L
#define DNS_ERROR_RECORD_TIMED_OUT 9705L
#define DNS_ERROR_NAME_NOT_IN_ZONE 9706L
#define DNS_ERROR_CNAME_LOOP 9707L
#define DNS_ERROR_NODE_IS_CNAME 9708L
#define DNS_ERROR_CNAME_COLLISION 9709L
#define DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT 9710L
#define DNS_ERROR_RECORD_ALREADY_EXISTS 9711L
#define DNS_ERROR_SECONDARY_DATA 9712L
#define DNS_ERROR_NO_CREATE_CACHE_DATA 9713L
#define DNS_ERROR_NAME_DOES_NOT_EXIST 9714L
#define DNS_WARNING_PTR_CREATE_FAILED 9715L
#define DNS_WARNING_DOMAIN_UNDELETED 9716L
#define DNS_ERROR_DS_UNAVAILABLE 9717L
#define DNS_ERROR_DS_ZONE_ALREADY_EXISTS 9718L
#define DNS_ERROR_NO_BOOTFILE_IF_DS_ZONE 9719L
#define DNS_INFO_AXFR_COMPLETE 9751L
#define DNS_ERROR_AXFR 9752L
#define DNS_INFO_ADDED_LOCAL_WINS 9753L
#define DNS_STATUS_CONTINUE_NEEDED 9801L
#define DNS_ERROR_NO_TCPIP 9851L
#define DNS_ERROR_NO_DNS_SERVERS 9852L
#define DNS_ERROR_DP_DOES_NOT_EXIST 9901L
#define DNS_ERROR_DP_ALREADY_EXISTS 9902L
#define DNS_ERROR_DP_NOT_ENLISTED 9903L
#define DNS_ERROR_DP_ALREADY_ENLISTED 9904L
#define DNS_ERROR_DP_NOT_AVAILABLE 9905L
#endif
