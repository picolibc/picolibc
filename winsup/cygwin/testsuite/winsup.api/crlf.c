
typedef enum {
  Nop=100000,	/* 	; do nothing */
  New1,		/*	; reset and begin new  test */
  Open,		/* 	; open test file */
  Read,		/* [askfor] [get] ; read bytes into buffer */
  Write,	/* [expect] [bytestream] ; write to file (expect 0 = ignore)*/
  Compare,	/* [bytestream] ; compare buffer to given bytes */
  Verify,	/* [bytestream] ; compare file to given bytes */
  Seek,		/* [offset] [whence] ; seek in file */
  Tell,		/* [offset] ; compare file positions */
  BufSize,	/* [size] ; change the stdio buffer size */
  Flush,	/*	; flush the stdio stream */
  Text,		/* 	; switch file to text mode */
  Binary,	/* 	; switch file to binary mode */
  Rep,		/* [count] ; repeat 'R' bytes (used in bytestream) */
  Fill,		/* [posn] ; fill 'F' until given byte position */
  Start,	/*	; for Seek */
  Current,	/*	; for Seek */
  End,		/* 	; for Seek, or end of byte stream */
  Max } Opcode;

#define New New1,__LINE__

/* Byte streams are just inserted into the command stream, and must
   end in an End code. */

int commands[] = {
#ifndef __DJGPP__
  New,
  Write, 1605, Rep, 1600, 'h', 'e', 'l', 'l', 'o', End,
  Tell, 1605,

  Open,
  BufSize, 16,
  Read, 1605, 1605,
  Compare, Rep, 1600, 'h', 'e', 'l', 'l', 'o', End,
  Tell, 1605,
  Flush,
  Tell, 1605,
  Seek, 1000, Start,
  Tell, 1000,

  New,
  Text,
  Write, 2, 'x', 10, End,
  Verify, 'x', 13, 10, End,

  New,
  Binary,
  Write, 2, 'x', 10, End,
  Verify, 'x', 10, End,

  BufSize, 16,

  New,
  Binary,
  Write, 0, Fill, 15, 13, 10, 'x', End,
  Text,  Open,
  Read, 17, 17,
  Compare, Fill, 15, 10, 'x', End,
  Tell, 18,

  New,
  Binary,
  Write, 0, Fill, 14, 13, 10, 'x', End,
  Text,  Open,
  Read, 16, 16,
  Compare, Fill, 14, 10, 'x', End,
  Tell, 17,

  New,
  Binary,
  Write, 0, 13, 10, 'a', 'b', End,
  Text, Open,
  Read, 2, 2,
  Compare, 10, 'a', End,
  Tell, 3,

  New,
  Binary,
  Write, 0, 10, 'a', 'b', End,
  Text, Open,
  Read, 2, 2,
  Compare, 10, 'a', End,
  Tell, 2,

  New,
  Binary,
  Write, 0, 13, 'a', 'b', End,
  Text, Open,
  Read, 2, 2,
  Compare, 13, 'a', End,
  Tell, 2,

  New,
  Binary,
  Write, 0, 13, 13, 10, 'a', End,
  Text, Open,
  Read, 2, 2,
  Compare, 13, 10, End,
  Tell, 3,

  New,
  Binary,
  Write, 0, 13, 10, 'a', 13, End,
  Text, Open,
  Read, 2, 2,
  Compare, 10, 'a', End,
  Tell, 3,

  New,
  Binary,
  Write, 0, 13, 10, 'a', 10, End,
  Text, Open,
  Read, 2, 2,
  Compare, 10, 'a', End,
  Tell, 3,

  New,
  Binary,
  Write, 0, 13, 13, 13, 10, 'a', 10, 10, 10, End,
  Text, Open,
  Read, 4, 4,
  Compare, 13, 13, 10, 'a', End,
  Tell, 5,
#endif

  New,
  Binary,
  Write, 0, 13, 13, 13, 10, 'a', 'b', 13, 10, 13, 10, End,
  Text, Open,
  Read, 4, 4,
  Compare, 13, 13, 10, 'a', End,
  Tell, 5,

  };

/*==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif

int errors = 0;

#define num_commands (sizeof(commands)/sizeof(commands[0]))

int pc;
int askfor, get, expect, count, posn, whence, size;

typedef struct {
  unsigned char *bytes;
  int max, count;
} Buffer;

Buffer rw_buf={0,0,0}, cmp_buf={0,0,0}, vfy_buf={0,0,0};

void
expand_buf(Buffer *buf, int len)
{
  if (buf->max < len)
    {
      buf->max = len+20;
      if (buf->bytes)
	buf->bytes = (unsigned char *)realloc(buf->bytes, buf->max);
      else
	buf->bytes = (unsigned char *)malloc(buf->max);
    }
}

void
get_bytestream(Buffer *buf)
{
  int tpc;
  int len = 0, rep, byte;
  unsigned char *bp;

  for (tpc = pc+1; tpc < num_commands && commands[tpc] != End; tpc++)
    {
      switch (commands[tpc])
	{
	case Rep:
	  len += commands[tpc+1];
	  tpc ++;
	  break;
	case Fill:
	  if (len < commands[tpc+1])
	    len = commands[tpc+1];
	  tpc ++;
	  break;
	default:
	  len ++;
	  break;
	}
    }

  expand_buf(buf, len);

  len = 0;
  bp = buf->bytes;

  for (tpc = pc+1; tpc < num_commands && commands[tpc] != End; tpc++)
    {
      switch (commands[tpc])
	{
	case Rep:
	  rep = commands[++tpc];
	  byte = 'R';
	  while (rep--) *bp++ = byte;
	  break;
	case Fill:
	  rep = commands[++tpc];
	  byte = 'F';
	  while (bp-buf->bytes < rep) *bp++ = byte;
	  break;
	default:
	  *bp++ = commands[tpc];
	  break;
	}
    }
  buf->count = bp - buf->bytes;
  pc = tpc;
}

char dataname[] = "crlf.dat";

int verbose=1;
void
v(char *fmt, ...)
{
  va_list ap;
  if (!verbose) return;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  printf("\n");
}

void
vp(char *fmt, ...)
{
  va_list ap;
  if (!verbose) return;
  printf("%08x: ", pc);
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  printf("\n");
}

void
errorq(int use_errno, char *fmt, ...)
{
  va_list ap;
  fprintf(stderr, "crlf: Error at pc=%d: ", pc);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  if (use_errno)
    perror("The error was");
  errors++;
}

void
error(int use_errno, char *fmt, ...)
{
  va_list ap;
  fprintf(stderr, "crlf: Error at pc=%d: ", pc);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  if (use_errno)
    perror("The error was");
  fprintf(stderr, "FAIL\n");
  exit(1);
}

void
display_buf(char *which, Buffer *buf, int ofs)
{
  int i;
  fprintf(stderr, "%s %04x:", which, ofs);
  for (i=0; i<8; i++)
    if (i+ofs < buf->count)
      {
	unsigned char b = buf->bytes[i+ofs];
	fprintf(stderr, " %02x", b);
	if (isgraph(b))
	  fprintf(stderr, " %c ", b);
	else
	  fprintf(stderr, "   ", b);
      }
  fprintf(stderr, "\n");
}

void
compare_bufs(char *name, Buffer *actual, Buffer *expected)
{
  int i, got_one=0;
  for (i=0; i<actual->count && i<expected->count; i++)
    if (actual->bytes[i] != expected->bytes[i])
      {
	errorq(0, "%s: byte mismatch at offset 0x%x", name, i);
	got_one = 1;
	break;
      }
  if (!got_one)
    {
      if (actual->count < expected->count)
	errorq(0, "%s: too few bytes (0x%x vs 0x%x)", name,
	      actual->count, expected->count);
      else if (actual->count > expected->count)
	errorq(0, "%s: too many bytes (0x%x vs 0x%x)", name,
	      actual->count, expected->count);
      else
	return;
    }

  i -= 4;
  if (i<0) i = 0;
  display_buf("Actual  ", actual, i);
  display_buf("Expected", expected, i);
}

int
main(int argc, char **argv)
{
  char *readmode = "rb";
  char *writemode = "wb";
  FILE *file = 0;
  int i, fd;
  struct stat st;
  char *str;

  while (argc > 1 && argv[1][0] == '-')
    {
      if (strcmp(argv[1], "-v") == 0)
	verbose++;
      argc--;
      argv++;
    }

  size = 0;

  for (pc=0; pc<num_commands; pc++)
    {
      switch (commands[pc])
	{
	case Nop:
	  vp("Nop");
	  break;

	case New1:
	  i = commands[++pc];
	  vp("New %d", i);
	  if (file) fclose(file);
	  file = 0;
	  remove(dataname);
	  break;

	case Open:
	  vp("Open");
	  if (file) fclose(file);
	  file = 0;
	  break;

	case Read:
	  if (!file)
	    {
	      file = fopen(dataname, readmode);
	      if (size)
		setvbuf(file, 0, _IOFBF, size);
	    }
	  if (!file)
	    error(1, "cannot open %s, mode %s", dataname, readmode);
	  askfor = commands[++pc];
	  get = commands[++pc];
	  vp("Read %d %d", askfor, get);
	  expand_buf(&rw_buf, askfor);
	  if (askfor == 1)
	    {
	      i = getc(file);
	      rw_buf.bytes[0] = i;
	      i = (i == EOF) ? 0 : 1;
	    }
	  else
	    i = fread(rw_buf.bytes, 1, askfor, file);
	  if (i != get)
	    error(0, "read wrong number of bytes (%d vs %d)", i, get);
	  rw_buf.count = i;
	  break;

	case Write:
	  if (!file)
	    {
	      file = fopen(dataname, writemode);
	      if (size)
		setvbuf(file, 0, _IOFBF, size);
	    }
	  if (!file)
	    error(1, "cannot open %s, mode %s\n", dataname, writemode);
	  expect = commands[++pc];
	  get_bytestream(&rw_buf);
	  vp("Write %d %d", rw_buf.count, expect);
	  if (askfor == 1)
	    {
	      i = putc(rw_buf.bytes[0], file);
	      i = (i == EOF) ? 0 : 1;
	    }
	  else
	    i = fwrite(rw_buf.bytes, 1, rw_buf.count, file);
	  if (expect && (i != expect))
	    error(0, "wrote wrong number of bytes (%d vs %d)", i, expect);
	  break;

	case Compare:
	  get_bytestream(&cmp_buf);
	  vp("Compare %d/%d", rw_buf.count, cmp_buf.count);
	  compare_bufs("Compare", &rw_buf, &cmp_buf);
	  break;

	case Verify:
	  if (file) fclose(file);
	  file = 0;
	  get_bytestream(&cmp_buf);
	  vp("Verify %s", dataname);
	  if (stat(dataname, &st))
	    error(1, "Can't stat %s", dataname);
	  expand_buf(&vfy_buf, st.st_size);
	  i = open(dataname, O_RDONLY|O_BINARY, 0);
	  vfy_buf.count = read(i, vfy_buf.bytes, st.st_size);
	  close(i);
	  compare_bufs("Verify", &vfy_buf, &cmp_buf);
	  break;

	case Seek:
	  posn = commands[++pc];
	  whence = commands[++pc];
	  switch (whence)
	    {
	    case Start:
	      whence = SEEK_SET;
	      str = "Start";
	      break;
	    case Current:
	      whence = SEEK_CUR;
	      str = "Current";
	      break;
	    case End:
	      whence = SEEK_END;
	      str = "End";
	      break;
	    }
	  vp("Seek 0x%x %s", posn, str);
	  i = fseek(file, posn, whence);
	  if (i)
	    error(1, "fseek failed");
	  break;

	case Tell:
	  posn = commands[++pc];
	  vp("Tell 0x%x", posn);
	  i = ftell(file);
	  if (i != posn)
	    error(0, "ftell failed, got 0x%x expected 0x%x", i, posn);
	  break;

	case BufSize:
	  size = commands[++pc];
	  vp("BufSize 0x%x", size);
	  if (file)
	    {
	      fflush(file);
	      setvbuf(file, 0, _IOFBF, size);
	    }
	  break;

	case Flush:
	  vp("Flush");
	  if (file)
	    fflush(file);
	  break;

	case Text:
	  vp("Text");
	  readmode = "rt";
	  writemode = "wt";
	  break;

	case Binary:
	  vp("Binary");
	  readmode = "rb";
	  writemode = "wb";
	  break;

	default:
	  printf("Invalid command code %d at offset %d\n", commands[pc], pc);
	  exit(1);
	}
    }

  if (file) fclose(file);

  if (errors)
    printf("FAIL: %d error%s\n", errors, errors==1?"":"s");
  else
    {
      printf("PASS\n");
      unlink (dataname);
    }
  return errors;
}
