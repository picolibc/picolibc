/*
 * hl_api.c -- high-level Hostlink IO API.
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#include <string.h>
#include <stdarg.h>

#include "hl_gw.h"
#include "hl_api.h"

/* Parameter types.  */
#define PAT_CHAR	1
#define PAT_SHORT	2
#define PAT_INT		3
#define PAT_STRING	4
/* For future use.  */
#define PAT_INT64	5

/* Used internally to pass user hostlink parameters to _hl_message().  */
struct _hl_user_info {
  uint32_t vendor_id;
  uint32_t opcode;
  uint32_t result;
};

/*
 * Main function to send a message using hostlink.
 *
 * syscall - one of HL_SYSCALL_* defines from hl_api.h.
 *
 * user - parameters and return value for _user_hostlink implementation.
 *	Packing structure:
 *	 uint32 vendor_id - user-defined vendor ID.  ID 1025 is reserved for
 *			    GNU IO extensions.
 *	 uint32 opcode - operation code for user-defined hostlink.
 *	 char format[] - argument string in the same format as for
 *			 _hl_message() function, see below.
 *
 * format - argument and return values format string [(i4sp)*:?(i4sp)*], where
 *	  characters before ':' is arguments and after is return values.
 *	  Supported format characters:
 *	   i or 4 - uint32 value, pack_int will be used;
 *	   s      - char * data, pack_str will be used;
 *	   p      - void * data, pack_ptr will be used.
 *
 * ap - argument values and pointers to the output values.  Must be in sync
 *      with format string.
 *      For hostlink message argument:
 *	   i or 4 - uint32 value;
 *	   s      - char * pointer to the NUL-teminated string;
 *	   p      - void * pointer to the buffer and uint32 buffer length.
 *      For output values:
 *	   i or 4 - uint32 * pointer to uint32 to return;
 *	   s      - char * pointer to buffer to return, it must have enough
 *		    space to store returned data.
 *		    You can get packed buffer length with _hl_get_ptr_len();
 *	   p      - void * pointer and uint32 * length pointer to store
 *		    returned data along with length.  Buffer must be enough
 *		    to store returned data.
 *		    You can get packed buffer length with _hl_get_ptr_len();
 *
 * return - pointer to the hostlink buffer after output values.
 */
static volatile __uncached char *
_hl_message_va (uint32_t syscall, struct _hl_user_info *user,
		const char *format, va_list ap)
{
  const char *f = format;
  volatile __uncached char *p = _hl_payload ();
  int get_answer = 0;

  p = _hl_pack_int (p, syscall);

  if (syscall == HL_SYSCALL_USER)
    {
      p = _hl_pack_int (p, user->vendor_id);
      p = _hl_pack_int (p, user->opcode);
      p = _hl_pack_str (p, format);
    }

  for (; *f; f++)
    {
      void *ptr;
      uint32_t len;

      if (*f == ':')
	{
	  f++;
	  get_answer = 1;
	  break;
	}

      switch (*f)
	{
	case 'i':
	case '4':
	  p = _hl_pack_int (p, va_arg (ap, uint32_t));
	  break;
	case 's':
	  p = _hl_pack_str (p, va_arg (ap, char *));
	  break;
	case 'p':
	  ptr = va_arg (ap, void *);
	  len = va_arg (ap, uint32_t);
	  p = _hl_pack_ptr (p, ptr, len);
	  break;
	default:
	  return NULL;
	}

      if (p == NULL)
	return NULL;
    }

  _hl_send (p);

  p = _hl_recv ();

  if (syscall == HL_SYSCALL_USER && p)
    p = _hl_unpack_int (p, &user->result);

  if (p && get_answer)
    {
      for (; *f; f++)
	{
	  void *ptr;
	  uint32_t *plen;

	  switch (*f)
	    {
	    case 'i':
	    case '4':
	      p = _hl_unpack_int (p, va_arg (ap, uint32_t *));
	      break;
	    case 's':
	      p = _hl_unpack_str (p, va_arg (ap, char *));
	      break;
	    case 'p':
	      ptr = va_arg (ap, void *);
	      plen = va_arg (ap, uint32_t *);
	      p = _hl_unpack_ptr (p, ptr, plen);
	      break;
	    default:
	      return NULL;
	    }

	  if (p == NULL)
	    return NULL;
	}
    }

  return p;
}

/*
 * Pack integer value (uint32) to provided buffer.
 * Packing structure:
 *     uint16 type  (PAT_INT = 3)
 *     uint16 size  (4)
 *     uint32 value
 */
volatile __uncached char *
_hl_pack_int (volatile __uncached char *p, uint32_t x)
{
  volatile __uncached uint16_t *type = (volatile __uncached uint16_t *) p;
  volatile __uncached uint16_t *size = (volatile __uncached uint16_t *)
				       (p + 2);
  volatile __uncached uint32_t *val = (volatile __uncached uint32_t *)
				      (p + 4);
  const uint32_t payload_used = 8;

  if (_hl_payload_left (p) < payload_used)
    return NULL;

  *type = PAT_INT;
  *size = 4;
  *val = x;

  return p + payload_used;
}

/*
 * Pack data (pointer and legth) to provided buffer.
 * Packing structure:
 *     uint16 type  (PAT_STRING = 4)
 *     uint16 size  (length)
 *     char   buf[length]
 */
volatile __uncached char *
_hl_pack_ptr (volatile __uncached char *p, const void *s, uint16_t len)
{
  volatile __uncached uint16_t *type = (volatile __uncached uint16_t *) p;
  volatile __uncached uint16_t *size = (volatile __uncached uint16_t *)
				       (p + 2);
  volatile __uncached char *buf = p + 4;
  const uint32_t payload_used = 4 + ALIGN (len, 4);

  if (_hl_payload_left (p) < payload_used)
    return NULL;

  *type = PAT_STRING;
  *size = len;

  /* _vdmemcpy(buf, s, len); */
  for (uint16_t i = 0; i < len; i++)
    buf[i] = ((const char *) s)[i];

  return p + payload_used;
}

/*
 * Pack NUL-terminated string to provided buffer.
 * Packing structure:
 *     uint16 type  (PAT_STRING = 4)
 *     uint16 size  (length)
 *     char   buf[length]
 */
volatile __uncached char *
_hl_pack_str (volatile __uncached char *p, const char *s)
{
  return _hl_pack_ptr (p, s, strlen (s) + 1);
}

/* Unpack integer value (uint32_t) from a buffer.  */
volatile __uncached char *
_hl_unpack_int (volatile __uncached char *p, uint32_t *x)
{
  volatile __uncached uint16_t *type = (volatile __uncached uint16_t *) p;
  volatile __uncached uint16_t *size = (volatile __uncached uint16_t *)
		(p + 2);
  volatile __uncached uint32_t *val = (volatile __uncached uint32_t *)
		(p + 4);
  const uint32_t payload_used = 8;

  if (_hl_payload_left (p) < payload_used || *type != PAT_INT || *size != 4)
    return NULL;

  if (x)
    *x = *val;

  return p + payload_used;
}

/* Unpack data from a buffer.  */
volatile __uncached char *
_hl_unpack_ptr (volatile __uncached char *p, void *s, uint32_t *plen)
{
  volatile __uncached uint16_t *type = (volatile __uncached uint16_t *) p;
  volatile __uncached uint16_t *size = (volatile __uncached uint16_t *)
				       (p + 2);
  volatile __uncached char *buf = p + 4;
  uint32_t payload_used;
  uint32_t len;

  if (_hl_payload_left (p) < 4 || *type != PAT_STRING)
    return NULL;

  len = *size;
  payload_used = 4 + ALIGN (len, 4);

  if (_hl_payload_left (p) < payload_used)
    return NULL;

  if (plen)
    *plen = len;

  /* _vsmemcpy(s, buf, len); */
  if (s)
    {
      for (uint32_t i = 0; i < len; i++)
	((char *) s)[i] = buf[i];
    }

  return p + payload_used;
}

/*
 * Unpack data from a buffer.
 *
 * No difference compared to _hl_unpack_ptr, except that this function
 * does not return a length.
 */
volatile __uncached char *
_hl_unpack_str (volatile __uncached char *p, char *s)
{
  return _hl_unpack_ptr (p, s, NULL);
}

/* Return length of packed data (PAT_STRING) if it is on top of the buffer.  */
uint32_t
_hl_get_ptr_len (volatile __uncached char *p)
{
  volatile __uncached uint16_t *type = (volatile __uncached uint16_t *) p;
  volatile __uncached uint16_t *size = (volatile __uncached uint16_t *)
				       (p + 2);

  if (_hl_payload_left (p) < 4 || *type != PAT_STRING)
    return 0;

  return *size;
}

/* Public version of _hl_message_va().  */
volatile __uncached char *
_hl_message (uint32_t syscall, const char *format, ...)
{
  va_list ap;
  volatile __uncached char *p;

  va_start (ap, format);

  p = _hl_message_va (syscall, 0, format, ap);

  va_end (ap);

  return p;
}

/*
 * API to call user-defined hostlink.  See description of user argument in
 * _hl_message_va().
 */
uint32_t
_user_hostlink (uint32_t vendor, uint32_t opcode, const char *format, ...)
{
  va_list ap;
  struct _hl_user_info ui = { .vendor_id = vendor,
			      .opcode = opcode };

  va_start (ap, format);

  _hl_message_va (HL_SYSCALL_USER, &ui, format, ap);

  va_end (ap);

  return ui.result;
}
