#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "go32.h"
#include "dpmi.h"
/*#include "process.h"*/
#if 1
#define P_WAIT		1
#define P_NOWAIT	2	/* always generates error */
#define P_OVERLAY	3
#endif
extern const char **environ;
#define environ ((const char **)environ)

#define scan_ptr() \
	char const **ptr; \
	for (ptr = &argv0; *ptr; ptr++); \
	ptr = (char const **)(*++ptr);

int execl(const char *path, const char *argv0, ...)
{
  return spawnve(P_OVERLAY, path, &argv0, environ);
}

int execle(const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnve(P_OVERLAY, path, &argv0, ptr);
}

int execlp(const char *path, const char *argv0, ...)
{
  return spawnvpe(P_OVERLAY, path, &argv0, environ);
}

int execlpe(const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnvpe(P_OVERLAY, path, &argv0, ptr);
}

/*-------------------------------------------------*/

int execv(const char *path, const char **argv)
{
  return spawnve(P_OVERLAY, path, argv, environ);
}

int execve(const char *path, const char **argv, const char **envp)
{
  return spawnve(P_OVERLAY, path, argv, envp);
}

int execvp(const char *path, const char **argv)
{
  return spawnvpe(P_OVERLAY, path, argv, environ);
}

int execvpe(const char *path, const char **argv, const char **envp)
{
  return spawnvpe(P_OVERLAY, path, argv, envp);
}

/*-------------------------------------------------*/

int spawnl(int mode, const char *path, const char *argv0, ...)
{
  return spawnve(mode, path, &argv0, environ);
}

int spawnle(int mode, const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnve(mode, path, &argv0, ptr);
}

int spawnlp(int mode, const char *path, const char *argv0, ...)
{
  return spawnvpe(mode, path, &argv0, environ);
}

int spawnlpe(int mode, const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnvpe(mode, path, &argv0, ptr);
}

/*-------------------------------------------------*/

typedef struct {
  u_short eseg;
  u_short argoff;
  u_short argseg;
  u_short fcb1_off;
  u_short fcb1_seg;
  u_short fcb2_off;
  u_short fcb2_seg;
} Execp;

static Execp parm;

static u_long tbuf;

static u_long talloc(size_t amt)
{
  u_long rv = tbuf;
  tbuf += amt;
  return rv;
}

static int direct_exec_tail(const char *program, const char *args, const char **envp)
{
  _go32_dpmi_registers r;
  u_long program_la;
  u_long arg_la;
  u_long parm_la;
  u_long env_la, env_e_la;
  char arg_header[3];
  int i;

  program_la = talloc(strlen(program)+1);
  arg_la = talloc(strlen(args)+3);
  parm_la = talloc(sizeof(Execp));

  dosmemput(program, strlen(program)+1, program_la);
  
  arg_header[0] = strlen(args);
  arg_header[1] = '\r';
  dosmemput(arg_header, 1, arg_la);
  dosmemput(args, strlen(args), arg_la+1);
  dosmemput(arg_header+1, 1, arg_la+1+strlen(args));

  do {
    env_la = talloc(1);
  } while (env_la & 15);
  talloc(-1);
  for (i=0; envp[i]; i++)
  {
    env_e_la = talloc(strlen(envp[i])+1);
    dosmemput(envp[i], strlen(envp[i])+1, env_e_la);
  }
  arg_header[0] = 0;
  arg_header[1] = 1;
  arg_header[2] = 0;
  dosmemput(arg_header, 3, talloc(3));
  env_e_la = talloc(strlen(program)+1);
  dosmemput(program, strlen(program)+1, env_e_la);

  parm.eseg = env_la / 16;
  parm.argseg = arg_la / 16;
  parm.argoff = arg_la & 15;
  dosmemput(&parm, sizeof(parm), parm_la);

  memset(&r, 0, sizeof(r));
  r.x.ax = 0x4b00;
  r.x.ds = program_la / 16;
  r.x.dx = program_la & 15;
  r.x.es = parm_la / 16;
  r.x.bx = parm_la & 15;
  _go32_dpmi_simulate_int(0x21, &r);
  if (r.x.flags & 1)
  {
    errno = r.x.ax;
    return -1;
  }
  
  memset(&r, 0, sizeof(r));
  r.h.ah = 0x4d;
  _go32_dpmi_simulate_int(0x21, &r);
  
  if (r.x.flags & 1)
  {
    errno = r.x.ax;
    return -1;
  }
  return r.x.ax;
}

static int direct_exec(const char *program, const char **argv, const char **envp)
{
  int i, arglen;
  char *args, *argp;

  tbuf = _go32_info_block.linear_address_of_transfer_buffer;

  arglen = 0;
  for (i=1; argv[i]; i++)
    arglen += strlen(argv[i]) + 1;
  args = (char *)malloc(arglen+1);
  argp = args;
  for (i=1; argv[i]; i++)
  {
    const char *p = argv[i];
    if (argp - args > 125)
      break;
    *argp++ = ' ';
    while (*p)
    {
      if (argp - args > 125)
        break;
      *argp++ = *p++;
    }
  }
  *argp = 0;
  
  return direct_exec_tail(program, args, envp);
}

typedef struct {
  char magic[16];
  int struct_length;
  char go32[16];
} StubInfo;
#define STUB_INFO_MAGIC "StubInfoMagic!!"

static int go32_exec(const char *program, const char **argv, const char **envp)
{
  int is_stubbed = 0;
  int found_si = 0;
  StubInfo si;
  unsigned short header[3];
  int pf, has_dot, i;
  char *go32, *sip;
  const char *pp, *pe;
  char rpath[80], *rp;
  int stub_offset, argc;

  int si_la, rm_la, rm_seg;
  short *rm_argv;
  char cmdline[34];
  
  pf = open(program, O_RDONLY|O_BINARY);

  read(pf, header, sizeof(header));
  if (header[0] == 0x010b || header[0] == 0x014c)
  {
    is_stubbed = 1;
  }
  else if (header[0] == 0x5a4d)
  {
    int header_offset = (long)header[2]*512L;
    if (header[1])
      header_offset += (long)header[1] - 512L;
    lseek(pf, header_offset - 4, 0);
    read(pf, &stub_offset, 4);
    header[0] = 0;
    read(pf, header, sizeof(header));
    if (header[0] == 0x010b)
      is_stubbed = 1;
    if (header[0] == 0x014c)
      is_stubbed = 1;
    lseek(pf, stub_offset, 0);
    read(pf, &si, sizeof(si));
    if (memcmp(STUB_INFO_MAGIC, si.magic, 16) == 0)
      found_si = 1;
  }
  if (!is_stubbed)
  {
    close(pf);
    return direct_exec(program, argv, envp);
  }

  if (found_si)
    go32 = si.go32;
  else
    go32 = "go32.exe";
  has_dot = 0;
  for (i=0; go32[i]; i++)
    if (go32[i] == '.')
      has_dot = 1;
  if (!has_dot)
    strcpy(go32+i, ".exe");
  for (i=0; envp[i]; i++)
    if (strncmp(envp[i], "PATH=", 5) == 0)
      pp = envp[i]+5;
  strcpy(rpath, go32);
  while (access(rpath, 0))
  {
    char *ptr;
    rp = rpath;
    for (pe=pp; *pe && *pe != ';'; pe++)
      *rp++ = *pe;
    pp = pe+1;
    if (rp > rpath && rp[-1] != '/' && rp[-1] != '\\' && rp[-1] != ':')
      *rp++ = '/';
    for (ptr = go32; *ptr; ptr++)
      *rp++ = *ptr;
    *rp = 0;
    if (access(rpath, 0) == 0)
      break;
    if (*pe == 0)
      return direct_exec(program, argv, envp); /* give up and just run it */
  }

  if (found_si)
  {  
    lseek(pf, stub_offset, 0);
    sip = (char *)malloc(si.struct_length);
    read(pf, sip, si.struct_length);
  }
  close(pf);

  argv[0] = program; /* since that's where we really found it */

  tbuf = _go32_info_block.linear_address_of_transfer_buffer;

  if (found_si)
  {
    si_la = talloc(si.struct_length);
    dosmemput(sip, si.struct_length, si_la);
    free(sip);
  }
  
  for (argc=0; argv[argc]; argc++);
  rm_la = talloc(2*(argc+1));
  rm_seg = (_go32_info_block.linear_address_of_transfer_buffer >> 4) & 0xffff;
  rm_argv = (short *)malloc((argc+1) * sizeof(short));
  for (i=0; i<argc; i++)
  {
    int sl = strlen(argv[i]) + 1;
    int q = talloc(sl);
    dosmemput(argv[i], sl, q);
    rm_argv[i] = (q - (rm_seg<<4)) & 0xffff;
  }
  rm_argv[i] = 0;
  dosmemput(rm_argv, 2*(argc+1), rm_la);
  
  sprintf(cmdline, " !proxy %04x %04x %04x %04x %04x",
    argc, rm_seg, (rm_la - (rm_seg<<4))&0xffff,
    rm_seg, (si_la - (rm_seg<<4))&0xffff);
  if (!found_si)
    cmdline[22] = 0; /* remove stub information */

  return direct_exec_tail(rpath, cmdline, envp);
}

static int command_exec(const char *program, const char **argv, const char **envp)
{
  const char *comspec=0;
  char *cmdline;
  char *newargs[3];
  int cmdlen;
  int i;
  
  cmdlen = strlen(program) + 4;
  for (i=0; argv[i]; i++)
    cmdlen += strlen(argv[i]) + 1;
  cmdline = (char *)malloc(cmdlen);
  
  strcpy(cmdline, "/c ");
  for (i=0; program[i]; i++)
  {
    if (program[i] == '/')
      cmdline[i+3] = '\\';
    else
      cmdline[i+3] = program[i];
  }
  cmdline[i+3] = 0;
  for (i=1; argv[i]; i++)
  {
    strcat(cmdline, " ");
    strcat(cmdline, argv[i]);
  }
  for (i=0; envp[i]; i++)
    if (strncmp(envp[i], "COMSPEC=", 8) == 0)
      comspec = envp[i]+8;
  if (!comspec)
    for (i=0; environ[i]; i++)
      if (strncmp(environ[i], "COMSPEC=", 8) == 0)
        comspec = environ[i]+8;
  if (!comspec)
    comspec = "c:/command.com";
  newargs[0] = comspec;
  newargs[1] = cmdline;
  newargs[2] = 0;
  i = direct_exec(comspec, (const char **)newargs, envp);
  free(cmdline);
  return i;
}

static int script_exec(const char *program, const char **argv, const char **envp)
{
  return go32_exec(program, argv, envp);
}

static struct {
  char *extension;
  int (*interp)(const char *, const char **, const char **);
} interpreters[] = {
  { ".com", direct_exec },
  { ".exe", go32_exec },
  { ".bat", command_exec },
  { 0,      script_exec }
};
#define INTERP_NO_EXT 3

int spawnv(int mode, const char *path, const char **argv)
{
  return spawnve(mode, path, argv, environ);
}

int spawnve(int mode, const char *path, const char **argv, const char **envp)
{
  /* This is the one that does the work! */
  int i = -1;
  char rpath[80], *rp, *rd=0;
  fflush(stdout); /* just in case */
  for (rp=rpath; *path; *rp++ = *path++)
  {
    if (*path == '.')
      rd = rp;
    if (*path == '\\' || *path == '/')
      rd = 0;
  }
  *rp = 0;
  if (rd)
  {
    for (i=0; interpreters[i].extension; i++)
      if (strcasecmp(rd, interpreters[i].extension) == 0)
        break;
  }
  while (access(rpath, 0))
  {
    i++;
    if (interpreters[i].extension == 0 || rd)
    {
      errno = ENOENT;
      return -1;
    }
    strcpy(rp, interpreters[i].extension);
  }
  if (i == -1)
    i = INTERP_NO_EXT;
  i = interpreters[i].interp(rpath, argv, envp);
  if (mode == P_OVERLAY)
    exit(i);
  return i;
}

int spawnvp(int mode, const char *path, const char **argv)
{
  return spawnvpe(mode, path, argv, environ);
}

int spawnvpe(int mode, const char *path, const char **argv, const char **envp)
{
  const char *pp, *pe, *ptr;
  char rpath[80], *rp, *rd;
  int hasdot = 0, i, tried_dot = 0;

  for (ptr=path; *ptr; ptr++)
  {
    if (*ptr == '.')
      hasdot = 1;
    if (*ptr == '/' || *ptr == '\\' || *ptr == ':')
      return spawnve(mode, path, argv, envp);
  }

  pp = 0;
  for (i=0; envp[i]; i++)
    if (strncmp(envp[i], "PATH=", 5) == 0)
      pp = envp[i] + 5;
  if (pp == 0)
    return spawnve(mode, path, argv, envp);

  while (1)
  {
    if (!tried_dot)
    {
      rp = rpath;
      pe = pp;
      tried_dot = 1;
    }
    else
    {
      rp = rpath;
      for (pe = pp; *pe && *pe != ';'; pe++)
        *rp++ = *pe;
      pp = pe+1;
      if (rp > rpath && rp[-1] != '/' && rp[-1] != '\\' && rp[-1] != ':')
        *rp++ = '/';
    }
    for (ptr = path; *ptr; ptr++)
      *rp++ = *ptr;
    *rp = 0;
    
    if (hasdot)
    {
      if (access(rpath, 0) == 0)
        return spawnve(mode, rpath, argv, envp);
    }
    else
    {
      for (i=0; interpreters[i].extension; i++)
      {
        strcpy(rp, interpreters[i].extension);
        if (access(rpath, 0) == 0)
          return spawnve(mode, rpath, argv, envp);
      }
    }
    if (*pe == 0)
    {
      errno = ENOENT;
      return -1;
    }
  }
}
