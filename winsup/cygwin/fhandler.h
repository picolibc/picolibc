/* fhandler.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once
#include "pinfo.h"

#include "tty.h"
#include <cygwin/_socketflags.h>
#include <cygwin/_ucred.h>
#include <sys/un.h>

/* fcntl flags used only internaly. */
#define O_NOSYMLINK	0x080000
#define O_DIROPEN	0x100000

/* newlib used to define O_NDELAY differently from O_NONBLOCK.  Now it
   properly defines both to be the same.  Unfortunately, we have to
   behave properly the old version, too, to accommodate older executables. */
#define OLD_O_NDELAY	(CYGWIN_VERSION_CHECK_FOR_OLD_O_NONBLOCK ? 4 : 0)

/* Care for the old O_NDELAY flag. If one of the flags is set,
   both flags are set. */
#define O_NONBLOCK_MASK (O_NONBLOCK | OLD_O_NDELAY)

/* It appears that 64K is the block size used for buffered I/O on NT.
   Using this blocksize in read/write calls in the application results
   in a much better performance than using smaller values. */
#define PREFERRED_IO_BLKSIZE ((blksize_t) 65536)

/* It also appears that this may be the only acceptable block size for
   atomic writes to a pipe.  It is a shame that we have to make this
   so small.  http://cygwin.com/ml/cygwin/2011-03/msg00541.html  */
#define DEFAULT_PIPEBUFSIZE PREFERRED_IO_BLKSIZE

/* Used for fhandler_pipe::create.  Use an available flag which will
   never be used in Cygwin for this function. */
#define PIPE_ADD_PID	FILE_FLAG_FIRST_PIPE_INSTANCE

#define O_TMPFILE_FILE_ATTRS (FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN)

/* Buffer size for ReadConsoleInput() and PeekConsoleInput(). */
/* Per MSDN, max size of buffer required is below 64K. */
/* (65536 / sizeof (INPUT_RECORD)) is 3276, however,
   ERROR_NOT_ENOUGH_MEMORY occurs in win7 if this value is used. */
#define INREC_SIZE 2048

extern const char *windows_device_names[];
extern struct __cygwin_perfile *perfile_table;
#define __fmode (*(user_data->fmode_ptr))
extern const char proc[];
extern const size_t proc_len;
extern const char procsys[];
extern const size_t procsys_len;

class select_record;
class select_stuff;
class fhandler_disk_file;
class inode_t;
typedef struct __DIR DIR;
struct dirent;
struct iovec;
struct acl;
struct __acl_t;

enum dirent_states
{
  dirent_ok		= 0x0000,
  dirent_saw_dot	= 0x0001,
  dirent_saw_dot_dot	= 0x0002,
  dirent_saw_eof	= 0x0004,
  dirent_isroot		= 0x0008,
  dirent_set_d_ino	= 0x0010,
  dirent_get_d_ino	= 0x0020,
  dirent_nfs_d_ino	= 0x0040,

  /* Global flags which must not be deleted on rewinddir or seekdir. */
  dirent_info_mask	= 0x0078
};

enum bind_state
{
  unbound = 0,
  bind_pending = 1,
  bound = 2
};

enum conn_state
{
  unconnected = 0,
  connect_pending = 1,
  connected = 2,
  listener = 3,
  connect_failed = 4	/* FIXME: Do we really need this?  It's basically
				  the same thing as unconnected. */
};

enum line_edit_status
{
  line_edit_ok = 0,
  line_edit_input_done = 1,
  line_edit_signalled = 2,
  line_edit_error = 3,
  line_edit_pipe_full = 4
};

enum bg_check_types
{
  bg_error = -1,
  bg_eof = 0,
  bg_ok = 1,
  bg_signalled = 2
};

enum query_state {
  no_query = 0,
  query_read_control = 1,
  query_read_attributes = 2,
  query_write_control = 3,
  query_write_dac = 4,
  query_write_attributes = 5
};

enum del_lock_called_from {
  on_close,
  after_fork,
  after_exec
};

enum virtual_ftype_t {
  virt_none	 = 0x0000,	/* Invalid, Error */
  virt_file	 = 0x0001,	/* Regular file */
  virt_symlink	 = 0x0002,	/* Symlink */
  virt_pipe	 = 0x0003,	/* Pipe */
  virt_socket	 = 0x0004,	/* Socket */
  virt_chr	 = 0x0005,	/* Character special */
  virt_blk	 = 0x0006,	/* Block special */
  virt_fdsymlink = 0x0007,	/* Fd symlink (e.g. /proc/<PID>/fd/0) */
  virt_fsfile	 = 0x0008,	/* FS-based file via /proc/sys */
  virt_dir_type	 = 0x1000,
  virt_directory = 0x1001,	/* Directory */
  virt_rootdir	 = 0x1002,	/* Root directory of virtual FS */
  virt_fsdir	 = 0x1003,	/* FS-based directory via /proc/sys */
};

static inline bool
virt_ftype_isfile (virtual_ftype_t _f)
{
  return _f != virt_none && !(_f & virt_dir_type);
}

static inline bool
virt_ftype_isdir (virtual_ftype_t _f)
{
  return _f & virt_dir_type;
}

class fhandler_base
{
  friend class dtable;
  friend void close_all_files (bool);

  struct status_flags
  {
    unsigned rbinary		: 1; /* binary read mode */
    unsigned rbinset	    	: 1; /* binary read mode explicitly set */
    unsigned wbinary		: 1; /* binary write mode */
    unsigned wbinset		: 1; /* binary write mode explicitly set */
    unsigned nohandle		: 1; /* No handle associated with fhandler. */
    unsigned did_lseek		: 1; /* set when lseek is called as a flag that
					_write should check if we've moved
					beyond EOF, zero filling or making
					file sparse if so. */
    unsigned query_open		: 3; /* open file without requesting either
					read or write access */
    unsigned close_on_exec      : 1; /* close-on-exec */
    unsigned need_fork_fixup    : 1; /* Set if need to fixup after fork. */
    unsigned isclosed		: 1; /* Set when fhandler is closed. */
    unsigned mandatory_locking	: 1; /* Windows mandatory locking */
    unsigned was_nonblocking	: 1; /* Set when setting O_NONBLOCK.  Never
					reset.  This is for the sake of
					fhandler_base_overlapped::close. */

   public:
    status_flags () :
      rbinary (0), rbinset (0), wbinary (0), wbinset (0), nohandle (0),
      did_lseek (0), query_open (no_query), close_on_exec (0),
      need_fork_fixup (0), isclosed (0), mandatory_locking (0),
      was_nonblocking (0)
      {}
  } status, open_status;

 private:
  ACCESS_MASK access;
  ULONG options;

  HANDLE io_handle;

  ino_t ino;	/* file ID or hashed filename, depends on FS. */
  LONG _refcnt;

 protected:
  /* File open flags from open () and fcntl () calls */
  int openflags;

  char *rabuf;		/* used for crlf conversion in text files */
  size_t ralen;
  size_t raixget;
  size_t raixput;
  size_t rabuflen;

  /* Used for advisory file locking.  See flock.cc.  */
  int64_t unique_id;
  void del_my_locks (del_lock_called_from);
  void set_ino (ino_t i) { ino = i; }

  HANDLE read_state;

 public:
  LONG inc_refcnt () {return InterlockedIncrement (&_refcnt);}
  LONG dec_refcnt () {return InterlockedDecrement (&_refcnt);}
  class fhandler_base *archetype;
  int usecount;

  path_conv pc;

  void reset (const fhandler_base *);
  virtual bool use_archetype () const {return false;}
  virtual void set_name (path_conv &pc);
  virtual void set_name (const char *s)
  {
    pc.set_posix (s);
    pc.set_path (s);
  }
  int error () const {return pc.error;}
  void set_error (int error) {pc.error = error;}
  bool exists () const {return pc.exists ();}
  int pc_binmode () const {return pc.binmode ();}
  device& dev () {return pc.dev;}
  operator DWORD& () {return (DWORD&) pc;}
  fhandler_base ();
  virtual ~fhandler_base ();

  /* Non-virtual simple accessor functions. */
  void set_handle (HANDLE x) { io_handle = x; }

  dev_t& get_device () { return dev (); }
  _major_t get_major () { return dev ().get_major (); }
  _minor_t get_minor () { return dev ().get_minor (); }

  ACCESS_MASK get_access () const { return access; }
  void set_access (ACCESS_MASK x) { access = x; }

  ULONG get_options () const { return options; }
  void set_options (ULONG x) { options = x; }

  int get_flags () { return openflags; }
  void set_flags (int x, int supplied_bin = 0);

  bool is_nonblocking ();
  void set_nonblocking (int);

  bool wbinary () const { return status.wbinset ? status.wbinary : 1; }
  bool rbinary () const { return status.rbinset ? status.rbinary : 1; }

  void wbinary (bool b) {status.wbinary = b; status.wbinset = 1;}
  void rbinary (bool b) {status.rbinary = b; status.rbinset = 1;}

  void set_open_status () {open_status = status;}
  void reset_to_open_binmode ()
  {
    set_flags ((get_flags () & ~(O_TEXT | O_BINARY))
	       | ((open_status.wbinary || open_status.rbinary)
		   ? O_BINARY : O_TEXT));
  }

  IMPLEMENT_STATUS_FLAG (bool, wbinset)
  IMPLEMENT_STATUS_FLAG (bool, rbinset)
  IMPLEMENT_STATUS_FLAG (bool, nohandle)
  IMPLEMENT_STATUS_FLAG (bool, did_lseek)
  IMPLEMENT_STATUS_FLAG (query_state, query_open)
  IMPLEMENT_STATUS_FLAG (bool, close_on_exec)
  IMPLEMENT_STATUS_FLAG (bool, need_fork_fixup)
  IMPLEMENT_STATUS_FLAG (bool, isclosed)
  IMPLEMENT_STATUS_FLAG (bool, mandatory_locking)
  IMPLEMENT_STATUS_FLAG (bool, was_nonblocking)

  int get_default_fmode (int flags);

  virtual void set_close_on_exec (bool val);

  LPSECURITY_ATTRIBUTES get_inheritance (bool all = 0)
  {
    if (all)
      return close_on_exec () ? &sec_all_nih : &sec_all;
    else
      return close_on_exec () ? &sec_none_nih : &sec_none;
  }

  virtual int fixup_before_fork_exec (DWORD) { return 0; }
  virtual void fixup_after_fork (HANDLE);
  virtual void fixup_after_exec ();
  void create_read_state (LONG n)
  {
    read_state = CreateSemaphore (&sec_none_nih, 0, n, NULL);
    ProtectHandle (read_state);
  }

  void signal_read_state (LONG n)
  {
    ReleaseSemaphore (read_state, n, NULL);
  }

  bool get_readahead_valid () { return raixget < ralen; }
  int puts_readahead (const char *s, size_t len = (size_t) -1);
  int put_readahead (char value);

  int get_readahead ();
  int peek_readahead (int queryput = 0);

  void set_readahead_valid (int val, int ch = -1);

  int get_readahead_into_buffer (char *buf, size_t buflen);

  bool has_acls () const { return pc.has_acls (); }

  bool isremote () { return pc.isremote (); }

  bool has_attribute (DWORD x) const {return pc.has_attribute (x);}
  const char *get_name () const { return pc.get_posix (); }
  const char *get_win32_name () { return pc.get_win32 (); }
  virtual dev_t get_dev () { return get_device (); }
  /* Use get_plain_ino if the caller needs to avoid hashing if ino is 0. */
  ino_t get_plain_ino () { return ino; }
  ino_t get_ino () { return ino ?: ino = hash_path_name (0, pc.get_nt_native_path ()); }
  int64_t get_unique_id () const { return unique_id; }
  /* Returns name used for /proc/<pid>/fd in buf. */
  virtual char *get_proc_fd_name (char *buf);

  virtual void set_no_inheritance (HANDLE &, bool);

  /* fixup fd possibly non-inherited handles after fork */
  bool fork_fixup (HANDLE, HANDLE &, const char *);
  virtual bool need_fixup_before () const {return false;}

  int open_with_arch (int, mode_t = 0);
  int open_null (int flags);
  virtual int open (int, mode_t);
  virtual fhandler_base *fd_reopen (int, mode_t);
  virtual void open_setup (int flags);
  void set_unique_id (int64_t u) { unique_id = u; }
  void set_unique_id () { NtAllocateLocallyUniqueId ((PLUID) &unique_id); }

  int close_with_arch ();
  virtual int close ();
  virtual void cleanup ();
  int _archetype_usecount (const char *fn, int ln, int n)
  {
    if (!archetype)
      return 0;
    archetype->usecount += n;
    if (strace.active ())
      strace.prntf (_STRACE_ALL, fn, "line %d:  %s<%p> usecount + %d = %d", ln, get_name (), archetype, n, archetype->usecount);
    return archetype->usecount;
  }

  int open_fs (int, mode_t = 0);
# define archetype_usecount(n) _archetype_usecount (__PRETTY_FUNCTION__, __LINE__, (n))
  int close_fs () { return fhandler_base::close (); }
  virtual int __reg2 fstat (struct stat *buf);
  void __reg2 stat_fixup (struct stat *buf);
  int __reg2 fstat_fs (struct stat *buf);
private:
  int __reg2 fstat_helper (struct stat *buf);
  int __reg2 fstat_by_nfs_ea (struct stat *buf);
  int __reg2 fstat_by_handle (struct stat *buf);
  int __reg2 fstat_by_name (struct stat *buf);
public:
  virtual int __reg2 fstatvfs (struct statvfs *buf);
  int __reg2 utimens_fs (const struct timespec *);
  virtual int __reg1 fchmod (mode_t mode);
  virtual int __reg2 fchown (uid_t uid, gid_t gid);
  virtual int __reg3 facl (int, int, struct acl *);
  virtual struct __acl_t * __reg2 acl_get (uint32_t);
  virtual int __reg3 acl_set (struct __acl_t *, uint32_t);
  virtual ssize_t __reg3 fgetxattr (const char *, void *, size_t);
  virtual int __reg3 fsetxattr (const char *, const void *, size_t, int);
  virtual int __reg3 fadvise (off_t, off_t, int);
  virtual int __reg3 ftruncate (off_t, bool);
  virtual int __reg2 link (const char *);
  virtual int __reg2 utimens (const struct timespec *);
  virtual int __reg1 fsync ();
  virtual int ioctl (unsigned int cmd, void *);
  virtual int fcntl (int cmd, intptr_t);
  virtual char const *ttyname () { return get_name (); }
  virtual void __reg3 read (void *ptr, size_t& len);
  virtual ssize_t __stdcall write (const void *ptr, size_t len);
  virtual ssize_t __stdcall readv (const struct iovec *, int iovcnt, ssize_t tot = -1);
  virtual ssize_t __stdcall writev (const struct iovec *, int iovcnt, ssize_t tot = -1);
  virtual ssize_t __reg3 pread (void *, size_t, off_t, void *aio = NULL);
  virtual ssize_t __reg3 pwrite (void *, size_t, off_t, void *aio = NULL);
  virtual off_t lseek (off_t offset, int whence);
  virtual int lock (int, struct flock *);
  virtual int mand_lock (int, struct flock *);
  virtual int dup (fhandler_base *child, int flags);
  virtual int fpathconf (int);

  virtual HANDLE mmap (caddr_t *addr, size_t len, int prot,
		       int flags, off_t off);
  virtual int munmap (HANDLE h, caddr_t addr, size_t len);
  virtual int msync (HANDLE h, caddr_t addr, size_t len, int flags);
  virtual bool fixup_mmap_after_fork (HANDLE h, int prot, int flags,
				      off_t offset, SIZE_T size,
				      void *address);

  void *operator new (size_t, void *p) __attribute__ ((nothrow)) {return p;}

  virtual int init (HANDLE, DWORD, mode_t);

  virtual int tcflush (int);
  virtual int tcsendbreak (int);
  virtual int tcdrain ();
  virtual int tcflow (int);
  virtual int tcsetattr (int a, const struct termios *t);
  virtual int tcgetattr (struct termios *t);
  virtual int tcsetpgrp (const pid_t pid);
  virtual int tcgetpgrp ();
  virtual pid_t tcgetsid ();
  virtual bool is_tty () const { return false; }
  virtual bool ispipe () const { return false; }
  virtual pid_t get_popen_pid () const {return 0;}
  virtual bool isfifo () const { return false; }
  virtual int ptsname_r (char *, size_t);
  virtual class fhandler_socket *is_socket () { return NULL; }
  virtual class fhandler_socket_wsock *is_wsock_socket () { return NULL; }
  virtual class fhandler_console *is_console () { return 0; }
  virtual class fhandler_signalfd *is_signalfd () { return NULL; }
  virtual class fhandler_timerfd *is_timerfd () { return NULL; }
  virtual int is_windows () {return 0; }

  virtual void __reg3 raw_read (void *ptr, size_t& ulen);
  virtual ssize_t __reg3 raw_write (const void *ptr, size_t ulen);

  /* Virtual accessor functions to hide the fact
     that some fd's have two handles. */
  virtual HANDLE& get_handle () { return io_handle; }
  virtual HANDLE& get_handle_cyg () { return io_handle; }
  virtual HANDLE& get_output_handle () { return io_handle; }
  virtual HANDLE& get_output_handle_cyg () { return io_handle; }
  virtual HANDLE get_stat_handle () { return pc.handle () ?: io_handle; }
  virtual HANDLE get_echo_handle () const { return NULL; }
  virtual bool hit_eof () {return false;}
  virtual select_record *select_read (select_stuff *);
  virtual select_record *select_write (select_stuff *);
  virtual select_record *select_except (select_stuff *);
  virtual const char *get_native_name ()
  {
    return dev ().native ();
  }
  virtual bg_check_types bg_check (int, bool = false) {return bg_ok;}
  virtual void clear_readahead ()
  {
    raixput = raixget = ralen = rabuflen = 0;
    rabuf = NULL;
  }
  void operator delete (void *p) {cfree (p);}
  virtual void set_eof () {}
  virtual int mkdir (mode_t mode);
  virtual int rmdir ();
  virtual __reg2 DIR *opendir (int fd);
  virtual __reg3 int readdir (DIR *, dirent *);
  virtual long telldir (DIR *);
  virtual void seekdir (DIR *, long);
  virtual void rewinddir (DIR *);
  virtual int closedir (DIR *);
  bool is_fs_special () {return pc.is_fs_special ();}
  bool issymlink () {return pc.issymlink ();}
  bool __reg2 device_access_denied (int);
  int __reg3 fhaccess (int flags, bool);
  virtual bool __reg1 has_ongoing_io ()  {return false;}

  fhandler_base (void *) {}

  virtual void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_base *> (x) = *this;
    x->reset (this);
  }

  virtual fhandler_base *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_base));
    fhandler_base *fh = new (ptr) fhandler_base (ptr);
    copyto (fh);
    return fh;
  }
};

struct wsa_event
{
  LONG serial_number;
  long events;
  int  connect_errorcode;
  pid_t owner;
};

class fhandler_socket: public fhandler_base
{
 private:
   /* permission fake following Linux rules */
   uid_t uid;
   uid_t gid;
   mode_t mode;

 protected:
  int addr_family;
  int type;
  inline int get_socket_flags ()
  {
    int ret = 0;
    if (is_nonblocking ())
      ret |= SOCK_NONBLOCK;
    if (close_on_exec ())
      ret |= SOCK_CLOEXEC;
    return ret;
  }

 protected:
  int	    _rmem;
  int	    _wmem;
 public:
  int &rmem () { return _rmem; }
  int &wmem () { return _wmem; }
  void rmem (int nrmem) { _rmem = nrmem; }
  void wmem (int nwmem) { _wmem = nwmem; }

 protected:
  DWORD _rcvtimeo; /* msecs */
  DWORD _sndtimeo; /* msecs */
 public:
  DWORD &rcvtimeo () { return _rcvtimeo; }
  DWORD &sndtimeo () { return _sndtimeo; }

 public:
  fhandler_socket ();
  ~fhandler_socket ();
  fhandler_socket *is_socket () { return this; }

  char *get_proc_fd_name (char *buf);

  virtual int socket (int af, int type, int protocol, int flags) = 0;
  virtual int socketpair (int af, int type, int protocol, int flags,
			  fhandler_socket *fh_out) = 0;
  virtual int bind (const struct sockaddr *name, int namelen) = 0;
  virtual int listen (int backlog) = 0;
  virtual int accept4 (struct sockaddr *peer, int *len, int flags) = 0;
  virtual int connect (const struct sockaddr *name, int namelen) = 0;
  virtual int getsockname (struct sockaddr *name, int *namelen) = 0;
  virtual int getpeername (struct sockaddr *name, int *namelen) = 0;
  virtual int shutdown (int how) = 0;
  virtual int close () = 0;
  virtual int getpeereid (pid_t *pid, uid_t *euid, gid_t *egid);
  virtual ssize_t recvfrom (void *ptr, size_t len, int flags,
			    struct sockaddr *from, int *fromlen) = 0;
  virtual ssize_t recvmsg (struct msghdr *msg, int flags) = 0;
  virtual void __reg3 read (void *ptr, size_t& len) = 0;
  virtual ssize_t __stdcall readv (const struct iovec *, int iovcnt,
				   ssize_t tot = -1) = 0;

  virtual ssize_t sendto (const void *ptr, size_t len, int flags,
	      const struct sockaddr *to, int tolen) = 0;
  virtual ssize_t sendmsg (const struct msghdr *msg, int flags) = 0;
  virtual ssize_t __stdcall write (const void *ptr, size_t len) = 0;
  virtual ssize_t __stdcall writev (const struct iovec *, int iovcnt, ssize_t tot = -1) = 0;
  virtual int setsockopt (int level, int optname, const void *optval,
			  __socklen_t optlen) = 0;
  virtual int getsockopt (int level, int optname, const void *optval,
			  __socklen_t *optlen) = 0;

  virtual int ioctl (unsigned int cmd, void *);
  virtual int fcntl (int cmd, intptr_t);

  int open (int flags, mode_t mode = 0);
  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  int __reg1 fchmod (mode_t newmode);
  int __reg2 fchown (uid_t newuid, gid_t newgid);
  int __reg3 facl (int, int, struct acl *);
  int __reg2 link (const char *);
  off_t lseek (off_t, int)
  { 
    set_errno (ESPIPE);
    return -1;
  }

  void set_addr_family (int af) {addr_family = af;}
  int get_addr_family () {return addr_family;}
  virtual void set_socket_type (int st) { type = st;}
  virtual int get_socket_type () {return type;}

  /* select.cc */
  virtual select_record *select_read (select_stuff *) = 0;
  virtual select_record *select_write (select_stuff *) = 0;
  virtual select_record *select_except (select_stuff *) = 0;
};

/* Encapsulate wsock-based socket classes fhandler_socket_inet and
   fhandler_socket_local during development of fhandler_socket_unix.
   TODO: Perhaps we should keep it that way, under the assumption that
   the Windows 10 AF_UNIX class will eventually get useful at one point. */
class fhandler_socket_wsock: public fhandler_socket
{
 protected:
  virtual int af_local_connect () = 0;

 protected:
  wsa_event *wsock_events;
  HANDLE wsock_mtx;
  HANDLE wsock_evt;
  bool init_events ();
  int wait_for_events (const long event_mask, const DWORD flags);
  void release_events ();
 public:
  const HANDLE wsock_event () const { return wsock_evt; }
  int evaluate_events (const long event_mask, long &events, const bool erase);
  const LONG serial_number () const { return wsock_events->serial_number; }

 protected:
  struct status_flags
  {
    unsigned async_io		   : 1; /* async I/O */
    unsigned saw_shutdown_read     : 1; /* Socket saw a SHUT_RD */
    unsigned saw_shutdown_write    : 1; /* Socket saw a SHUT_WR */
    unsigned saw_reuseaddr	   : 1; /* Socket saw SO_REUSEADDR call */
    unsigned connect_state	   : 3;
   public:
    status_flags () :
      async_io (0), saw_shutdown_read (0), saw_shutdown_write (0),
      saw_reuseaddr (0), connect_state (unconnected)
      {}
  } status;
 public:
  IMPLEMENT_STATUS_FLAG (bool, async_io)
  IMPLEMENT_STATUS_FLAG (bool, saw_shutdown_read)
  IMPLEMENT_STATUS_FLAG (bool, saw_shutdown_write)
  IMPLEMENT_STATUS_FLAG (bool, saw_reuseaddr)
  IMPLEMENT_STATUS_FLAG (conn_state, connect_state)

 protected:
  struct _WSAPROTOCOL_INFOW *prot_info_ptr;
 public:
  bool need_fixup_before () const {return prot_info_ptr != NULL;}
  void set_close_on_exec (bool val);
  void init_fixup_before ();
  int fixup_before_fork_exec (DWORD);
  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();
  int dup (fhandler_base *child, int);

#ifdef __INSIDE_CYGWIN_NET__
 protected:
  int set_socket_handle (SOCKET sock, int af, int type, int flags);
 public:
  /* Originally get_socket returned an int, which is not a good idea
     to cast a handle to on 64 bit.  The right type here is very certainly
     SOCKET instead.  On the other hand, we don't want to have to include
     winsock.h just to build fhandler.h.  Therefore we define get_socket
     now only when building network related code. */
  SOCKET get_socket () { return (SOCKET) get_handle(); }
#endif

 protected:
  virtual ssize_t recv_internal (struct _WSAMSG *wsamsg, bool use_recvmsg) = 0;
  ssize_t send_internal (struct _WSAMSG *wsamsg, int flags);

 public:
  fhandler_socket_wsock ();
  ~fhandler_socket_wsock ();

  fhandler_socket_wsock *is_wsock_socket () { return this; }

  ssize_t recvfrom (void *ptr, size_t len, int flags,
		    struct sockaddr *from, int *fromlen);
  ssize_t recvmsg (struct msghdr *msg, int flags);
  void __reg3 read (void *ptr, size_t& len);
  ssize_t __stdcall readv (const struct iovec *, int iovcnt, ssize_t tot = -1);
  ssize_t __stdcall write (const void *ptr, size_t len);
  ssize_t __stdcall writev (const struct iovec *, int iovcnt, ssize_t tot = -1);
  int shutdown (int how);
  int close ();

  int ioctl (unsigned int cmd, void *);
  int fcntl (int cmd, intptr_t);

  /* select.cc */
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
};

class fhandler_socket_inet: public fhandler_socket_wsock
{
 private:
  bool oobinline; /* True if option SO_OOBINLINE is set */
 protected:
  int af_local_connect () { return 0; }

 protected:
  ssize_t recv_internal (struct _WSAMSG *wsamsg, bool use_recvmsg);

 public:
  fhandler_socket_inet ();
  ~fhandler_socket_inet ();

  int socket (int af, int type, int protocol, int flags);
  int socketpair (int af, int type, int protocol, int flags,
		  fhandler_socket *fh_out);
  int bind (const struct sockaddr *name, int namelen);
  int listen (int backlog);
  int accept4 (struct sockaddr *peer, int *len, int flags);
  int connect (const struct sockaddr *name, int namelen);
  int getsockname (struct sockaddr *name, int *namelen);
  int getpeername (struct sockaddr *name, int *namelen);
  ssize_t sendto (const void *ptr, size_t len, int flags,
	      const struct sockaddr *to, int tolen);
  ssize_t sendmsg (const struct msghdr *msg, int flags);
  int setsockopt (int level, int optname, const void *optval,
		  __socklen_t optlen);
  int getsockopt (int level, int optname, const void *optval,
		  __socklen_t *optlen);

  /* from here on: CLONING */
  fhandler_socket_inet (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_socket_inet *> (x) = *this;
    x->reset (this);
  }

  fhandler_socket_inet *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_socket_inet));
    fhandler_socket_inet *fh = new (ptr) fhandler_socket_inet (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_socket_local: public fhandler_socket_wsock
{
 protected:
  char *sun_path;
  char *peer_sun_path;
  void set_sun_path (const char *path);
  char *get_sun_path () {return sun_path;}
  void set_peer_sun_path (const char *path);
  char *get_peer_sun_path () {return peer_sun_path;}

 protected:
  int connect_secret[4];
  pid_t sec_pid;
  uid_t sec_uid;
  gid_t sec_gid;
  pid_t sec_peer_pid;
  uid_t sec_peer_uid;
  gid_t sec_peer_gid;
  void af_local_set_secret (char *);
  void af_local_setblocking (bool &, bool &);
  void af_local_unsetblocking (bool, bool);
  void af_local_set_cred ();
  void af_local_copy (fhandler_socket_local *);
  bool af_local_recv_secret ();
  bool af_local_send_secret ();
  bool af_local_recv_cred ();
  bool af_local_send_cred ();
  int af_local_accept ();
  int af_local_connect ();
  int af_local_set_no_getpeereid ();
  void af_local_set_sockpair_cred ();

 protected:
  ssize_t recv_internal (struct _WSAMSG *wsamsg, bool use_recvmsg);

 protected:
  struct status_flags
  {
    unsigned no_getpeereid	   : 1;
   public:
    status_flags () : no_getpeereid (0) {}
  } status;
 public:
  IMPLEMENT_STATUS_FLAG (bool, no_getpeereid)

 public:
  fhandler_socket_local ();
  ~fhandler_socket_local ();

  int dup (fhandler_base *child, int);

  int socket (int af, int type, int protocol, int flags);
  int socketpair (int af, int type, int protocol, int flags,
		  fhandler_socket *fh_out);
  int bind (const struct sockaddr *name, int namelen);
  int listen (int backlog);
  int accept4 (struct sockaddr *peer, int *len, int flags);
  int connect (const struct sockaddr *name, int namelen);
  int getsockname (struct sockaddr *name, int *namelen);
  int getpeername (struct sockaddr *name, int *namelen);
  int getpeereid (pid_t *pid, uid_t *euid, gid_t *egid);
  ssize_t sendto (const void *ptr, size_t len, int flags,
	      const struct sockaddr *to, int tolen);
  ssize_t sendmsg (const struct msghdr *msg, int flags);
  int setsockopt (int level, int optname, const void *optval,
		  __socklen_t optlen);
  int getsockopt (int level, int optname, const void *optval,
		  __socklen_t *optlen);

  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  int __reg1 fchmod (mode_t newmode);
  int __reg2 fchown (uid_t newuid, gid_t newgid);
  int __reg3 facl (int, int, struct acl *);
  int __reg2 link (const char *);

  /* from here on: CLONING */
  fhandler_socket_local (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_socket_local *> (x) = *this;
    x->reset (this);
  }

  fhandler_socket_local *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_socket_local));
    fhandler_socket_local *fh = new (ptr) fhandler_socket_local (ptr);
    copyto (fh);
    return fh;
  }
};

/* Sharable spinlock with low CPU profile.  These locks are NOT recursive! */
class af_unix_spinlock_t
{
  LONG  locked;          /* 0 or 1 */

public:
  af_unix_spinlock_t () : locked (0) {}
  void lock ()
  {
    LONG ret = InterlockedExchange (&locked, 1);
    if (ret)
      {
	/* This loop counts the ms Sleep up from 0 to 45 in loop, 15ms steps,
	   with 256 iterations each, . */
        for (uint16_t i = 0; ret; i += 64)
          {
            Sleep (15 * (i >> 14));
            ret = InterlockedExchange (&locked, 1);
          }
      }
  }
  void unlock ()
  {
    InterlockedExchange (&locked, 0);
  }
};

/* Internal representation of shutdown states */
enum shut_state {
  _SHUT_NONE	= 0,
  _SHUT_RECV	= 1,
  _SHUT_SEND	= 2,
  _SHUT_MASK	= 3
};

class sun_name_t
{
 public:
  __socklen_t un_len;
  union
    {
      struct sockaddr_un un;
      /* Allows 108 bytes sun_path plus trailing NUL */
      char _nul[sizeof (struct sockaddr_un) + 1];
    };
  sun_name_t () { set (NULL, 0); }
  sun_name_t (const struct sockaddr *name, __socklen_t namelen)
    { set ((const struct sockaddr_un *) name, namelen); }
  void set (const struct sockaddr_un *name, __socklen_t namelen);
};

/* For each AF_UNIX socket, we need to maintain socket-wide data,
   regardless of the number of descriptors.  The shmem region gets created
   in socket, socketpair or accept4 and reopened by dup, fork or exec. */
class af_unix_shmem_t
{
  /* Don't use SRWLOCKs here.  They are not sharable.  If you must lock
     multiple locks at the same time, always lock in the order bind ->
     conn -> state -> io and unlock io -> state -> conn -> bind to avoid
     deadlocks. */
  af_unix_spinlock_t _bind_lock;
  af_unix_spinlock_t _conn_lock;
  af_unix_spinlock_t _state_lock;
  af_unix_spinlock_t _io_lock;
  LONG _connection_state;	/* conn_state */
  LONG _binding_state;		/* bind_state */
  LONG _shutdown;		/* shut_state */
  LONG _so_error;		/* SO_ERROR */
  LONG _so_passcred;		/* SO_PASSCRED */
  LONG _reuseaddr;		/* dummy */
  int  _type;			/* socket type */
  sun_name_t _sun_path;
  sun_name_t _peer_sun_path;
  struct ucred _sock_cred;	/* filled at listen time */
  struct ucred _peer_cred;	/* filled at connect time */

 public:
  void bind_lock () { _bind_lock.lock (); }
  void bind_unlock () { _bind_lock.unlock (); }
  void conn_lock () { _conn_lock.lock (); }
  void conn_unlock () { _conn_lock.unlock (); }
  void state_lock () { _state_lock.lock (); }
  void state_unlock () { _state_lock.unlock (); }
  void io_lock () { _io_lock.lock (); }
  void io_unlock () { _io_lock.unlock (); }

  conn_state connect_state (conn_state val)
    { return (conn_state) InterlockedExchange (&_connection_state, val); }
  conn_state connect_state () const { return (conn_state) _connection_state; }

  bind_state binding_state (bind_state val)
    { return (bind_state) InterlockedExchange (&_binding_state, val); }
  bind_state binding_state () const { return (bind_state) _binding_state; }

  int shutdown (int shut)
    { return (int) InterlockedExchange (&_shutdown, shut); }
  int shutdown () const { return (int) _shutdown; }

  int so_error (int err) { return (int) InterlockedExchange (&_so_error, err); }
  int so_error () const { return _so_error; }

  bool so_passcred (bool pc)
    { return (bool) InterlockedExchange (&_so_passcred, pc); }
  bool so_passcred () const { return _so_passcred; }

  int reuseaddr (int val)
    { return (int) InterlockedExchange (&_reuseaddr, val); }
  int reuseaddr () const { return _reuseaddr; }

  void set_socket_type (int val) { _type = val; }
  int get_socket_type () const { return _type; }

  void sun_path (struct sockaddr_un *un, __socklen_t unlen)
    { _sun_path.set (un, unlen); }
  void peer_sun_path (struct sockaddr_un *un, __socklen_t unlen)
    { _peer_sun_path.set (un, unlen); }
  sun_name_t *sun_path () {return &_sun_path;}
  sun_name_t *peer_sun_path () {return &_peer_sun_path;}

  void sock_cred (struct ucred *uc) { _sock_cred = *uc; }
  struct ucred *sock_cred () { return &_sock_cred; }
  void peer_cred (struct ucred *uc) { _peer_cred = *uc; }
  struct ucred *peer_cred () { return &_peer_cred; }
};

#ifdef __WITH_AF_UNIX

class fhandler_socket_unix : public fhandler_socket
{
 protected:
  HANDLE shmem_handle;		/* Shared memory region used to share
				   socket-wide state. */
  af_unix_shmem_t *shmem;
  HANDLE backing_file_handle;	/* Either NT symlink or INVALID_HANDLE_VALUE,
				   if the socket is backed by a file in the
				   file system (actually a reparse point) */
  HANDLE connect_wait_thr;
  HANDLE cwt_termination_evt;
  PVOID cwt_param;

  void bind_lock () { shmem->bind_lock (); }
  void bind_unlock () { shmem->bind_unlock (); }
  void conn_lock () { shmem->conn_lock (); }
  void conn_unlock () { shmem->conn_unlock (); }
  void state_lock () { shmem->state_lock (); }
  void state_unlock () { shmem->state_unlock (); }
  void io_lock () { shmem->io_lock (); }
  void io_unlock () { shmem->io_unlock (); }
  conn_state connect_state (conn_state val)
    { return shmem->connect_state (val); }
  conn_state connect_state () const { return shmem->connect_state (); }
  bind_state binding_state (bind_state val)
    { return shmem->binding_state (val); }
  bind_state binding_state () const { return shmem->binding_state (); }
  int saw_shutdown (int shut) { return shmem->shutdown (shut); }
  int saw_shutdown () const { return shmem->shutdown (); }
  int so_error (int err) { return shmem->so_error (err); }
  int so_error () const { return shmem->so_error (); }
  bool so_passcred (bool pc) { return shmem->so_passcred (pc); }
  bool so_passcred () const { return shmem->so_passcred (); }
  int reuseaddr (int err) { return shmem->reuseaddr (err); }
  int reuseaddr () const { return shmem->reuseaddr (); }
  void set_socket_type (int val) { shmem->set_socket_type (val); }
  int get_socket_type () const { return shmem->get_socket_type (); }

  int create_shmem ();
  int reopen_shmem ();
  void gen_pipe_name ();
  static HANDLE create_abstract_link (const sun_name_t *sun,
				      PUNICODE_STRING pipe_name);
  static HANDLE create_reparse_point (const sun_name_t *sun,
				      PUNICODE_STRING pipe_name);
  HANDLE create_file (const sun_name_t *sun);
  static int open_abstract_link (sun_name_t *sun, PUNICODE_STRING pipe_name);
  static int open_reparse_point (sun_name_t *sun, PUNICODE_STRING pipe_name);
  static int open_file (sun_name_t *sun, int &type, PUNICODE_STRING pipe_name);
  HANDLE autobind (sun_name_t *sun);
  wchar_t get_type_char ();
  void set_pipe_non_blocking (bool nonblocking);
  int send_sock_info (bool from_bind);
  int grab_admin_pkg ();
  int recv_peer_info ();
  static NTSTATUS npfs_handle (HANDLE &nph);
  HANDLE create_pipe (bool single_instance);
  HANDLE create_pipe_instance ();
  NTSTATUS open_pipe (PUNICODE_STRING pipe_name, bool xchg_sock_info);
  int wait_pipe (PUNICODE_STRING pipe_name);
  int connect_pipe (PUNICODE_STRING pipe_name);
  int listen_pipe ();
  ULONG peek_pipe (PFILE_PIPE_PEEK_BUFFER pbuf, ULONG psize, HANDLE evt);
  int disconnect_pipe (HANDLE ph);
  /* The NULL pointer check is required for FS methods like fstat.  When
     called via stat or lstat, there's no shared memory, just a path in pc. */
  sun_name_t *sun_path () {return shmem ? shmem->sun_path () : NULL;}
  sun_name_t *peer_sun_path () {return shmem->peer_sun_path ();}
  void sun_path (struct sockaddr_un *un, __socklen_t unlen)
    { shmem->sun_path (un, unlen); }
  void sun_path (sun_name_t *snt)
    { snt ? sun_path (&snt->un, snt->un_len) : sun_path (NULL, 0); }
  void peer_sun_path (struct sockaddr_un *un, __socklen_t unlen)
    { shmem->peer_sun_path (un, unlen); }
  void peer_sun_path (sun_name_t *snt)
    { snt ? peer_sun_path (&snt->un, snt->un_len)
	  : peer_sun_path (NULL, 0); }
  void init_cred ();
  void set_cred ();
  void sock_cred (struct ucred *uc) { shmem->sock_cred (uc); }
  struct ucred *sock_cred () { return shmem->sock_cred (); }
  void peer_cred (struct ucred *uc) { shmem->peer_cred (uc); }
  struct ucred *peer_cred () { return shmem->peer_cred (); }
  void fixup_after_fork (HANDLE parent);
  void fixup_after_exec ();
  void set_close_on_exec (bool val);

 public:
  fhandler_socket_unix ();
  ~fhandler_socket_unix ();

  int dup (fhandler_base *child, int);

  DWORD wait_pipe_thread (PUNICODE_STRING pipe_name);

  int socket (int af, int type, int protocol, int flags);
  int socketpair (int af, int type, int protocol, int flags,
		  fhandler_socket *fh_out);
  int bind (const struct sockaddr *name, int namelen);
  int listen (int backlog);
  int accept4 (struct sockaddr *peer, int *len, int flags);
  int connect (const struct sockaddr *name, int namelen);
  int getsockname (struct sockaddr *name, int *namelen);
  int getpeername (struct sockaddr *name, int *namelen);
  int shutdown (int how);
  int close ();
  int getpeereid (pid_t *pid, uid_t *euid, gid_t *egid);
  ssize_t recvmsg (struct msghdr *msg, int flags);
  ssize_t recvfrom (void *ptr, size_t len, int flags,
		    struct sockaddr *from, int *fromlen);
  void __reg3 read (void *ptr, size_t& len);
  ssize_t __stdcall readv (const struct iovec *const iov, int iovcnt,
			   ssize_t tot = -1);

  ssize_t sendmsg (const struct msghdr *msg, int flags);
  ssize_t sendto (const void *ptr, size_t len, int flags,
		  const struct sockaddr *to, int tolen);
  ssize_t __stdcall write (const void *ptr, size_t len);
  ssize_t __stdcall writev (const struct iovec *const iov, int iovcnt,
			    ssize_t tot = -1);
  int setsockopt (int level, int optname, const void *optval,
		  __socklen_t optlen);
  int getsockopt (int level, int optname, const void *optval,
		  __socklen_t *optlen);

  virtual int ioctl (unsigned int cmd, void *);
  virtual int fcntl (int cmd, intptr_t);

  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  int __reg1 fchmod (mode_t newmode);
  int __reg2 fchown (uid_t newuid, gid_t newgid);
  int __reg3 facl (int, int, struct acl *);
  int __reg2 link (const char *);

  /* select.cc */
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  /* from here on: CLONING */
  fhandler_socket_unix (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_socket_unix *> (x) = *this;
    x->reset (this);
  }

  fhandler_socket_unix *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_socket_unix));
    fhandler_socket_unix *fh = new (ptr) fhandler_socket_unix (ptr);
    copyto (fh);
    return fh;
  }
};

#endif /* __WITH_AF_UNIX */

class fhandler_base_overlapped: public fhandler_base
{
  static HANDLE asio_done;
  static LONG asio_close_counter;
protected:
  enum wait_return
  {
    overlapped_unknown = 0,
    overlapped_success,
    overlapped_nonblocking_no_data,
    overlapped_nullread,
    overlapped_error
  };
  bool io_pending;
  OVERLAPPED io_status;
  OVERLAPPED *overlapped;
  size_t max_atomic_write;
  void *atomic_write_buf;
public:
  wait_return __reg3 wait_overlapped (bool, bool, DWORD *, bool, DWORD = 0);
  int __reg1 setup_overlapped ();
  void __reg1 destroy_overlapped ();
  virtual void __reg3 raw_read (void *ptr, size_t& len);
  virtual ssize_t __reg3 raw_write (const void *ptr, size_t len);
  OVERLAPPED *&get_overlapped () {return overlapped;}
  OVERLAPPED *get_overlapped_buffer () {return &io_status;}
  void set_overlapped (OVERLAPPED *ov) {overlapped = ov;}
  fhandler_base_overlapped (): io_pending (false), overlapped (NULL), max_atomic_write (0), atomic_write_buf (NULL)
  {
    memset (&io_status, 0, sizeof io_status);
  }
  bool __reg1 has_ongoing_io ();

  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();

  int close ();
  int dup (fhandler_base *child, int);

  void check_later ();
  static void __reg1 flush_all_async_io ();;

  fhandler_base_overlapped (void *) {}
  ~fhandler_base_overlapped ()
  {
    if (atomic_write_buf)
      cfree (atomic_write_buf);
  }

  virtual void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_base_overlapped *> (x) = *this;
    reinterpret_cast<fhandler_base_overlapped *> (x)->atomic_write_buf = NULL;
    x->reset (this);
  }

  virtual fhandler_base_overlapped *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_base_overlapped));
    fhandler_base_overlapped *fh = new (ptr) fhandler_base_overlapped (ptr);
    copyto (fh);
    return fh;
  }

  friend DWORD WINAPI flush_async_io (void *);
};

class fhandler_pipe: public fhandler_base_overlapped
{
private:
  pid_t popen_pid;
public:
  fhandler_pipe ();


  bool ispipe() const { return true; }

  void set_popen_pid (pid_t pid) {popen_pid = pid;}
  pid_t get_popen_pid () const {return popen_pid;}
  off_t lseek (off_t offset, int whence);
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
  char *get_proc_fd_name (char *buf);
  int open (int flags, mode_t mode = 0);
  int dup (fhandler_base *child, int);
  int ioctl (unsigned int cmd, void *);
  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  int __reg3 fadvise (off_t, off_t, int);
  int __reg3 ftruncate (off_t, bool);
  int init (HANDLE, DWORD, mode_t, int64_t);
  static int create (fhandler_pipe *[2], unsigned, int);
  static DWORD create (LPSECURITY_ATTRIBUTES, HANDLE *, HANDLE *, DWORD,
		       const char *, DWORD, int64_t *unique_id = NULL);
  fhandler_pipe (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_pipe *> (x) = *this;
    reinterpret_cast<fhandler_pipe *> (x)->atomic_write_buf = NULL;
    x->reset (this);
  }

  fhandler_pipe *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pipe));
    fhandler_pipe *fh = new (ptr) fhandler_pipe (ptr);
    copyto (fh);
    return fh;
  }
};

#define CYGWIN_FIFO_PIPE_NAME_LEN     47
#define MAX_CLIENTS 64

enum fifo_client_connect_state
{
  fc_unknown,
  fc_connected,
  fc_invalid
};

enum
{
  FILE_PIPE_INPUT_AVAILABLE_STATE = 5
};

struct fifo_client_handler
{
  fhandler_base *fh;
  fifo_client_connect_state state;
  fifo_client_handler () : fh (NULL), state (fc_unknown) {}
  int close ();
/* Returns FILE_PIPE_DISCONNECTED_STATE, FILE_PIPE_LISTENING_STATE,
   FILE_PIPE_CONNECTED_STATE, FILE_PIPE_CLOSING_STATE,
   FILE_PIPE_INPUT_AVAILABLE_STATE, or -1 on error. */
  int pipe_state ();
};

class fhandler_fifo: public fhandler_base
{
  HANDLE read_ready;
  HANDLE write_ready;
  HANDLE listen_client_thr;
  HANDLE lct_termination_evt;
  UNICODE_STRING pipe_name;
  WCHAR pipe_name_buf[CYGWIN_FIFO_PIPE_NAME_LEN + 1];
  fifo_client_handler fc_handler[MAX_CLIENTS];
  int nhandlers, nconnected;
  af_unix_spinlock_t _fifo_client_lock;
  bool reader, writer, duplexer;
  size_t max_atomic_write;
  bool __reg2 wait (HANDLE);
  static NTSTATUS npfs_handle (HANDLE &);
  HANDLE create_pipe_instance (bool);
  NTSTATUS open_pipe (HANDLE&);
  int add_client_handler ();
  int delete_client_handler (int);
  bool listen_client ();
  int stop_listen_client ();
  int check_listen_client_thread ();
  void record_connection (fifo_client_handler&);
public:
  fhandler_fifo ();
  bool hit_eof ();
  int get_nhandlers () const { return nhandlers; }
  HANDLE get_fc_handle (int i) const
  { return fc_handler[i].fh->get_handle (); }
  bool is_connected (int i) const
  { return fc_handler[i].state == fc_connected; }
  PUNICODE_STRING get_pipe_name ();
  DWORD listen_client_thread ();
  void fifo_client_lock () { _fifo_client_lock.lock (); }
  void fifo_client_unlock () { _fifo_client_lock.unlock (); }
  int open (int, mode_t);
  off_t lseek (off_t offset, int whence);
  int close ();
  int fcntl (int cmd, intptr_t);
  int dup (fhandler_base *child, int);
  bool isfifo () const { return true; }
  void set_close_on_exec (bool val);
  void __reg3 raw_read (void *ptr, size_t& ulen);
  ssize_t __reg3 raw_write (const void *ptr, size_t ulen);
  bool arm (HANDLE h);
  bool need_fixup_before () const { return reader; }
  int fixup_before_fork_exec (DWORD) { return stop_listen_client (); }
  void init_fixup_before ();
  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();
  int __reg2 fstatvfs (struct statvfs *buf);
  void clear_readahead ()
  {
    fhandler_base::clear_readahead ();
    for (int i = 0; i < nhandlers; i++)
      fc_handler[i].fh->clear_readahead ();
  }
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_fifo (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_fifo *> (x) = *this;
    x->reset (this);
  }

  fhandler_fifo *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_fifo));
    fhandler_fifo *fhf = new (ptr) fhandler_fifo (ptr);
    /* We don't want our client list to change any more. */
    stop_listen_client ();
    copyto (fhf);
    /* fhf->pipe_name_buf is a *copy* of this->pipe_name_buf, but
       fhf->pipe_name.Buffer == this->pipe_name_buf. */
    fhf->pipe_name.Buffer = fhf->pipe_name_buf;
    for (int i = 0; i < nhandlers; i++)
      fhf->fc_handler[i].fh = fc_handler[i].fh->clone ();
    return fhf;
  }
};

class fhandler_dev_raw: public fhandler_base
{
 protected:
  char *devbufalloc;
  char *devbuf;
  DWORD devbufalign;
  DWORD devbufsiz;
  DWORD devbufstart;
  DWORD devbufend;
  struct status_flags
  {
    unsigned lastblk_to_read : 1;
   public:
    status_flags () : lastblk_to_read (0) {}
  } status;

  IMPLEMENT_STATUS_FLAG (bool, lastblk_to_read)

  fhandler_dev_raw ();

 public:
  ~fhandler_dev_raw ();

  int open (int flags, mode_t mode = 0);

  int __reg2 fstat (struct stat *buf);

  int dup (fhandler_base *child, int);
  int ioctl (unsigned int cmd, void *buf);

  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();

  fhandler_dev_raw (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_raw *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_raw *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_raw));
    fhandler_dev_raw *fh = new (ptr) fhandler_dev_raw (ptr);
    copyto (fh);
    return fh;
  }
};

#define MAX_PARTITIONS 15

struct part_t
{
  LONG refcnt;
  HANDLE hdl[MAX_PARTITIONS];
};

class fhandler_dev_floppy: public fhandler_dev_raw
{
 private:
  off_t drive_size;
  part_t *partitions;
  struct status_flags
  {
    unsigned eom_detected    : 1;
   public:
    status_flags () : eom_detected (0) {}
  } status;

  IMPLEMENT_STATUS_FLAG (bool, eom_detected)

  inline off_t get_current_position ();
  int get_drive_info (struct hd_geometry *geo);

  int lock_partition (DWORD to_write);

  BOOL write_file (const void *buf, DWORD to_write, DWORD *written, int *err);
  BOOL read_file (void *buf, DWORD to_read, DWORD *read, int *err);

 public:
  fhandler_dev_floppy ();

  int open (int flags, mode_t mode = 0);
  int close ();
  int dup (fhandler_base *child, int);
  void __reg3 raw_read (void *ptr, size_t& ulen);
  ssize_t __reg3 raw_write (const void *ptr, size_t ulen);
  off_t lseek (off_t offset, int whence);
  int ioctl (unsigned int cmd, void *buf);

  fhandler_dev_floppy (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_floppy *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_floppy *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_floppy));
    fhandler_dev_floppy *fh = new (ptr) fhandler_dev_floppy (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_dev_tape: public fhandler_dev_raw
{
  HANDLE mt_mtx;
  OVERLAPPED ov;

  bool is_rewind_device () { return get_minor () < 128; }
  unsigned int driveno () { return (unsigned int) get_minor () & 0x7f; }
  void drive_init ();

  inline bool _lock (bool);
  inline int unlock (int ret = 0);

 public:
  fhandler_dev_tape ();

  int open (int flags, mode_t mode = 0);
  virtual int close ();

  void __reg3 raw_read (void *ptr, size_t& ulen);
  ssize_t __reg3 raw_write (const void *ptr, size_t ulen);

  virtual off_t lseek (off_t offset, int whence);

  virtual int __reg2 fstat (struct stat *buf);

  virtual int dup (fhandler_base *child, int);
  virtual void fixup_after_fork (HANDLE parent);
  virtual void set_close_on_exec (bool val);
  virtual int ioctl (unsigned int cmd, void *buf);

  fhandler_dev_tape (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_tape *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_tape *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_tape));
    fhandler_dev_tape *fh = new (ptr) fhandler_dev_tape (ptr);
    copyto (fh);
    return fh;
  }
};

/* Standard disk file */

class fhandler_disk_file: public fhandler_base
{
  HANDLE prw_handle;
  bool prw_handle_isasync;
  int __reg3 readdir_helper (DIR *, dirent *, DWORD, DWORD, PUNICODE_STRING fname);

  int prw_open (bool, void *);
  uint64_t fs_ioc_getflags ();
  int fs_ioc_setflags (uint64_t);

 public:
  fhandler_disk_file ();
  fhandler_disk_file (path_conv &pc);

  int open (int flags, mode_t mode);
  int close ();
  int fcntl (int cmd, intptr_t);
  int dup (fhandler_base *child, int);
  void fixup_after_fork (HANDLE parent);
  int mand_lock (int, struct flock *);
  int __reg2 fstat (struct stat *buf);
  int __reg1 fchmod (mode_t mode);
  int __reg2 fchown (uid_t uid, gid_t gid);
  int __reg3 facl (int, int, struct acl *);
  struct __acl_t * __reg2 acl_get (uint32_t);
  int __reg3 acl_set (struct __acl_t *, uint32_t);
  ssize_t __reg3 fgetxattr (const char *, void *, size_t);
  int __reg3 fsetxattr (const char *, const void *, size_t, int);
  int __reg3 fadvise (off_t, off_t, int);
  int __reg3 ftruncate (off_t, bool);
  int __reg2 link (const char *);
  int __reg2 utimens (const struct timespec *);
  int __reg2 fstatvfs (struct statvfs *buf);
  int ioctl (unsigned int cmd, void *buf);

  HANDLE mmap (caddr_t *addr, size_t len, int prot, int flags, off_t off);
  int munmap (HANDLE h, caddr_t addr, size_t len);
  int msync (HANDLE h, caddr_t addr, size_t len, int flags);
  bool fixup_mmap_after_fork (HANDLE h, int prot, int flags,
			      off_t offset, SIZE_T size, void *address);
  int mkdir (mode_t mode);
  int rmdir ();
  DIR __reg2 *opendir (int fd);
  int __reg3 readdir (DIR *, dirent *);
  long telldir (DIR *);
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);

  ssize_t __reg3 pread (void *, size_t, off_t, void *aio = NULL);
  ssize_t __reg3 pwrite (void *, size_t, off_t, void *aio = NULL);

  fhandler_disk_file (void *) {}
  dev_t get_dev () { return pc.fs_serial_number (); }

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_disk_file *> (x) = *this;
    x->reset (this);
  }

  fhandler_disk_file *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_disk_file));
    fhandler_disk_file *fh = new (ptr) fhandler_disk_file (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_dev: public fhandler_disk_file
{
  const struct _device *devidx;
  bool dir_exists;
  int drive, part;
public:
  fhandler_dev ();
  int open (int flags, mode_t mode);
  int close ();
  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  DIR __reg2 *opendir (int fd);
  int __reg3 readdir (DIR *, dirent *);
  void rewinddir (DIR *);

  fhandler_dev (void *) {}
  dev_t get_dev () { return dir_exists ? pc.fs_serial_number ()
				       : get_device (); }

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev));
    fhandler_dev *fh = new (ptr) fhandler_dev (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_cygdrive: public fhandler_disk_file
{
 public:
  fhandler_cygdrive ();
  int open (int flags, mode_t mode);
  DIR __reg2 *opendir (int fd);
  int __reg3 readdir (DIR *, dirent *);
  void rewinddir (DIR *);
  int closedir (DIR *);
  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);

  fhandler_cygdrive (void *) {}
  dev_t get_dev () { return get_device (); }

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_cygdrive *> (x) = *this;
    x->reset (this);
  }

  fhandler_cygdrive *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_cygdrive));
    fhandler_cygdrive *fh = new (ptr) fhandler_cygdrive (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_serial: public fhandler_base
{
 private:
  size_t vmin_;				/* from termios */
  unsigned int vtime_;			/* from termios */
  pid_t pgrp_;
  int rts;				/* for Windows 9x purposes only */
  int dtr;				/* for Windows 9x purposes only */

 public:
  int overlapped_armed;
  OVERLAPPED io_status;
  DWORD ev;

  /* Constructor */
  fhandler_serial ();

  int open (int flags, mode_t mode);
  int close ();
  int init (HANDLE h, DWORD a, mode_t flags);
  void overlapped_setup ();
  int dup (fhandler_base *child, int);
  void __reg3 raw_read (void *ptr, size_t& ulen);
  ssize_t __reg3 raw_write (const void *ptr, size_t ulen);
  int tcsendbreak (int);
  int tcdrain ();
  int tcflow (int);
  int ioctl (unsigned int cmd, void *);
  int switch_modem_lines (int set, int clr);
  int tcsetattr (int a, const struct termios *t);
  int tcgetattr (struct termios *t);
  off_t lseek (off_t, int)
  { 
    set_errno (ESPIPE);
    return -1;
  }
  int tcflush (int);
  bool is_tty () const { return true; }
  void fixup_after_fork (HANDLE parent);
  void fixup_after_exec ();

  /* We maintain a pgrp so that tcsetpgrp and tcgetpgrp work, but we
     don't use it for permissions checking.  fhandler_pty_slave does
     permission checking on pgrps.  */
  virtual int tcgetpgrp () { return pgrp_; }
  virtual int tcsetpgrp (const pid_t pid) { pgrp_ = pid; return 0; }
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_serial (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_serial *> (x) = *this;
    x->reset (this);
  }

  fhandler_serial *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_serial));
    fhandler_serial *fh = new (ptr) fhandler_serial (ptr);
    copyto (fh);
    return fh;
  }
};

#define acquire_input_mutex(ms) \
  __acquire_input_mutex (__PRETTY_FUNCTION__, __LINE__, ms)

#define release_input_mutex() \
  __release_input_mutex (__PRETTY_FUNCTION__, __LINE__)

#define acquire_output_mutex(ms) \
  __acquire_output_mutex (__PRETTY_FUNCTION__, __LINE__, ms)

#define release_output_mutex() \
  __release_output_mutex (__PRETTY_FUNCTION__, __LINE__)

class tty;
class tty_min;
class fhandler_termios: public fhandler_base
{
 private:
  HANDLE output_handle;
 protected:
  virtual void doecho (const void *, DWORD) {};
  virtual int accept_input () {return 1;};
  int ioctl (int, void *);
  tty_min *_tc;
  tty *get_ttyp () {return (tty *) tc ();}
  int eat_readahead (int n);

 public:
  tty_min*& tc () {return _tc;}
  fhandler_termios () :
  fhandler_base ()
  {
    need_fork_fixup (true);
  }
  HANDLE& get_output_handle () { return output_handle; }
  HANDLE& get_output_handle_cyg () { return output_handle; }
  line_edit_status line_edit (const char *rptr, size_t nread, termios&,
			      ssize_t *bytes_read = NULL);
  void set_output_handle (HANDLE h) { output_handle = h; }
  void tcinit (bool force);
  bool is_tty () const { return true; }
  void sigflush ();
  int tcgetpgrp ();
  int tcsetpgrp (int pid);
  bg_check_types bg_check (int sig, bool dontsignal = false);
  virtual DWORD __acquire_output_mutex (const char *fn, int ln, DWORD ms) {return 1;}
  virtual void __release_output_mutex (const char *fn, int ln) {}
  void echo_erase (int force = 0);
  virtual off_t lseek (off_t, int);
  pid_t tcgetsid ();

  fhandler_termios (void *) {}

  virtual void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_termios *> (x) = *this;
    x->reset (this);
  }

  virtual fhandler_termios *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_termios));
    fhandler_termios *fh = new (ptr) fhandler_termios (ptr);
    copyto (fh);
    return fh;
  }
};

enum ansi_intensity
{
  INTENSITY_INVISIBLE,
  INTENSITY_DIM,
  INTENSITY_NORMAL,
  INTENSITY_BOLD
};

#define normal 0
#define gotesc 1
#define gotsquare 2
#define gotarg1 3
#define gotrsquare 4
#define gotcommand 5
#define gettitle 6
#define eattitle 7
#define gotparen 8
#define gotrparen 9
#define eatpalette 10
#define endpalette 11
#define MAXARGS 10

enum cltype
{
  cl_curr_pos = 1,
  cl_disp_beg,
  cl_disp_end,
  cl_buf_beg,
  cl_buf_end
};

class dev_console
{
  pid_t owner;
  bool is_legacy;

  WORD default_color, underline_color, dim_color;

  /* Used to determine if an input keystroke should be modified with META. */
  int meta_mask;

/* Output state */
  int state;
  int args[MAXARGS];
  int nargs;
  unsigned rarg;
  bool saw_question_mark;
  bool saw_greater_than_sign;
  bool saw_space;
  bool vt100_graphics_mode_G0;
  bool vt100_graphics_mode_G1;
  bool iso_2022_G1;
  bool alternate_charset_active;
  bool metabit;
  char backspace_keycode;

  char my_title_buf [TITLESIZE + 1];

  WORD current_win32_attr;
  ansi_intensity intensity;
  bool underline, blink, reverse;
  WORD fg, bg;

  /* saved cursor coordinates */
  int savex, savey;


  struct
    {
      short Top;
      short Bottom;
    } scroll_region;

  CONSOLE_SCREEN_BUFFER_INFO b;
  COORD dwWinSize;
  COORD dwEnd;

  /* saved screen */
  COORD save_bufsize;
  PCHAR_INFO save_buf;
  COORD save_cursor;
  SHORT save_top;

  COORD dwLastCursorPosition;
  COORD dwMousePosition;	/* scroll-adjusted coord of mouse event */
  COORD dwLastMousePosition;	/* scroll-adjusted coord of previous mouse event */
  DWORD dwLastButtonState;	/* (not noting mouse wheel events) */
  int last_button_code;		/* transformed mouse report button code */
  int nModifiers;

  bool insert_mode;
  int use_mouse;
  bool ext_mouse_mode5;
  bool ext_mouse_mode6;
  bool ext_mouse_mode15;
  bool use_focus;
  bool raw_win32_keyboard_mode;
  char cons_rabuf[40];  // cannot get longer than char buf[40] in char_command
  char *cons_rapoi;

  inline UINT get_console_cp ();
  DWORD con_to_str (char *d, int dlen, WCHAR w);
  DWORD str_to_con (mbtowc_p, PWCHAR d, const char *s, DWORD sz);
  void set_color (HANDLE);
  void set_default_attr ();
  int set_cl_x (cltype);
  int set_cl_y (cltype);
  bool fillin (HANDLE);
  bool __reg3 scroll_window (HANDLE, int, int, int, int);
  void __reg3 scroll_buffer (HANDLE, int, int, int, int, int, int);
  void __reg3 clear_screen (HANDLE, int, int, int, int);
  void __reg3 save_restore (HANDLE, char);

  friend class fhandler_console;
};

/* This is a input and output console handle */
class fhandler_console: public fhandler_termios
{
public:
  struct console_state
  {
    tty_min tty_min_state;
    dev_console con;
  };
  bool input_ready;
  enum input_states
  {
    input_error = -1,
    input_processing = 0,
    input_ok = 1,
    input_signalled = 2,
    input_winch = 3
  };
private:
  static const unsigned MAX_WRITE_CHARS;
  static console_state *shared_console_info;
  static bool invisible_console;
  HANDLE input_mutex, output_mutex;

  /* Used when we encounter a truncated multi-byte sequence.  The
     lead bytes are stored here and revisited in the next write call. */
  struct {
    int len;
    unsigned char buf[4]; /* Max len of valid UTF-8 sequence. */
  } trunc_buf;
  PWCHAR write_buf;

/* Output calls */
  void set_default_attr ();

  void scroll_buffer (int, int, int, int, int, int);
  void scroll_buffer_screen (int, int, int, int, int, int);
  void __reg3 clear_screen (cltype, cltype, cltype, cltype);
  void __reg3 cursor_set (bool, int, int);
  void __reg3 cursor_get (int *, int *);
  void __reg3 cursor_rel (int, int);
  inline void write_replacement_char ();
  inline bool write_console (PWCHAR, DWORD, DWORD&);
  const unsigned char *write_normal (unsigned const char*, unsigned const char *);
  void char_command (char);
  bool set_raw_win32_keyboard_mode (bool);
  int output_tcsetattr (int a, const struct termios *t);

/* Input calls */
  int igncr_enabled ();
  int input_tcsetattr (int a, const struct termios *t);
  void set_cursor_maybe ();
  static bool create_invisible_console (HWINSTA);
  static bool create_invisible_console_workaround ();
  static console_state *open_shared_console (HWND, HANDLE&, bool&);

 public:
  static pid_t tc_getpgid ()
  {
    return shared_console_info ? shared_console_info->tty_min_state.getpgid () : myself->pgid;
  }
  fhandler_console (fh_devices);
  static console_state *open_shared_console (HWND hw, HANDLE& h)
  {
    bool createit = false;
    return open_shared_console (hw, h, createit);
  }

  fhandler_console* is_console () { return this; }

  bool use_archetype () const {return true;}

  int open (int flags, mode_t mode);
  void open_setup (int flags);
  int dup (fhandler_base *, int);

  void __reg3 read (void *ptr, size_t& len);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void doecho (const void *str, DWORD len) { (void) write (str, len); }
  int close ();
  static bool exists () {return !!GetConsoleCP ();}

  int tcflush (int);
  int tcsetattr (int a, const struct termios *t);
  int tcgetattr (struct termios *t);

  int ioctl (unsigned int cmd, void *);
  int init (HANDLE, DWORD, mode_t);
  bool mouse_aware (MOUSE_EVENT_RECORD& mouse_event);
  bool focus_aware () {return shared_console_info->con.use_focus;}
  bool get_cons_readahead_valid ()
  {
    acquire_input_mutex (INFINITE);
    bool ret = shared_console_info->con.cons_rapoi != NULL &&
      *shared_console_info->con.cons_rapoi;
    release_input_mutex ();
    return ret;
  }

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
  void fixup_after_fork_exec (bool);
  void fixup_after_exec () {fixup_after_fork_exec (true);}
  void fixup_after_fork (HANDLE) {fixup_after_fork_exec (false);}
  void set_close_on_exec (bool val);
  void set_input_state ();
  bool send_winch_maybe ();
  void setup ();
  bool set_unit ();
  static bool need_invisible ();
  static void free_console ();
  static const char *get_nonascii_key (INPUT_RECORD& input_rec, char *);
  static DWORD get_console_process_id (DWORD pid, bool match);

  fhandler_console (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_console *> (x) = *this;
    x->reset (this);
  }

  fhandler_console *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_console));
    fhandler_console *fh = new (ptr) fhandler_console (ptr);
    copyto (fh);
    return fh;
  }
  input_states process_input_message ();
  void setup_io_mutex (void);
  DWORD __acquire_input_mutex (const char *fn, int ln, DWORD ms);
  void __release_input_mutex (const char *fn, int ln);
  DWORD __acquire_output_mutex (const char *fn, int ln, DWORD ms);
  void __release_output_mutex (const char *fn, int ln);

  friend tty_min * tty_list::get_cttyp ();
};

class fhandler_pty_common: public fhandler_termios
{
 public:
  fhandler_pty_common ()
    : fhandler_termios (),
    output_mutex (NULL), input_mutex (NULL),
    input_available_event (NULL)
  {
    pc.file_attributes (FILE_ATTRIBUTE_NORMAL);
  }
  static const unsigned pipesize = 128 * 1024;
  HANDLE output_mutex, input_mutex;
  HANDLE input_available_event;

  bool use_archetype () const {return true;}
  DWORD __acquire_output_mutex (const char *fn, int ln, DWORD ms);
  void __release_output_mutex (const char *fn, int ln);

  int close ();
  off_t lseek (off_t, int);
  bool bytes_available (DWORD& n);
  void set_close_on_exec (bool val);
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_pty_common (void *) {}

  virtual void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_pty_common *> (x) = *this;
    x->reset (this);
  }

  virtual fhandler_pty_common *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pty_common));
    fhandler_pty_common *fh = new (ptr) fhandler_pty_common (ptr);
    copyto (fh);
    return fh;
  }

  bool attach_pcon_in_fork (void)
  {
    return get_ttyp ()->attach_pcon_in_fork;
  }
  DWORD get_helper_process_id (void)
  {
    return get_ttyp ()->helper_process_id;
  }
  HPCON get_pseudo_console (void)
  {
    return get_ttyp ()->h_pseudo_console;
  }
  bool to_be_read_from_pcon (void);

 protected:
  BOOL process_opost_output (HANDLE h,
			     const void *ptr, ssize_t& len, bool is_echo);
};

class fhandler_pty_slave: public fhandler_pty_common
{
  HANDLE inuse;			// used to indicate that a tty is in use
  HANDLE output_handle_cyg, io_handle_cyg;
  DWORD pid_restore;
  int fd;

  /* Helper functions for fchmod and fchown. */
  bool fch_open_handles (bool chown);
  int fch_set_sd (security_descriptor &sd, bool chown);
  void fch_close_handles ();

  bool try_reattach_pcon ();
  void restore_reattach_pcon ();

 public:
  /* Constructor */
  fhandler_pty_slave (int);
  /* Destructor */
  ~fhandler_pty_slave ();

  void set_output_handle_cyg (HANDLE h) { output_handle_cyg = h; }
  HANDLE& get_output_handle_cyg () { return output_handle_cyg; }
  void set_handle_cyg (HANDLE h) { io_handle_cyg = h; }
  HANDLE& get_handle_cyg () { return io_handle_cyg; }

  int open (int flags, mode_t mode = 0);
  void open_setup (int flags);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __reg3 read (void *ptr, size_t& len);
  int init (HANDLE, DWORD, mode_t);

  int tcsetattr (int a, const struct termios *t);
  int tcgetattr (struct termios *t);
  int tcflush (int);
  int ioctl (unsigned int cmd, void *);
  int close ();
  void cleanup ();
  int dup (fhandler_base *child, int);
  void fixup_after_fork (HANDLE parent);
  void fixup_after_exec ();

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
  virtual char const *ttyname () { return pc.dev.name (); }
  int __reg2 fstat (struct stat *buf);
  int __reg3 facl (int, int, struct acl *);
  int __reg1 fchmod (mode_t mode);
  int __reg2 fchown (uid_t uid, gid_t gid);

  fhandler_pty_slave (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_pty_slave *> (x) = *this;
    x->reset (this);
  }

  fhandler_pty_slave *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pty_slave));
    fhandler_pty_slave *fh = new (ptr) fhandler_pty_slave (ptr);
    copyto (fh);
    return fh;
  }
  void set_switch_to_pcon (int fd);
  void reset_switch_to_pcon (void);
  void push_to_pcon_screenbuffer (const char *ptr, size_t len);
  void mask_switch_to_pcon_in (bool mask)
  {
    if (!mask && get_ttyp ()->pcon_pid &&
	get_ttyp ()->pcon_pid != myself->pid &&
	kill (get_ttyp ()->pcon_pid, 0) == 0)
      return;
    get_ttyp ()->mask_switch_to_pcon_in = mask;
  }
  void fixup_after_attach (bool native_maybe, int fd);
  bool is_line_input (void)
  {
    return get_ttyp ()->ti.c_lflag & ICANON;
  }
  void setup_locale (void);
  void set_freeconsole_on_close (bool val);
};

#define __ptsname(buf, unit) __small_sprintf ((buf), "/dev/pty%d", (unit))
class fhandler_pty_master: public fhandler_pty_common
{
  int pktmode;			// non-zero if pty in a packet mode.
  HANDLE master_ctl;		// Control socket for handle duplication
  cygthread *master_thread;	// Master control thread
  HANDLE from_master, to_master, from_slave, to_slave;
  HANDLE echo_r, echo_w;
  DWORD dwProcessId;		// Owner of master handles
  HANDLE to_master_cyg, from_master_cyg;
  cygthread *master_fwd_thread;	// Master forwarding thread

public:
  HANDLE get_echo_handle () const { return echo_r; }
  /* Constructor */
  fhandler_pty_master (int);

  DWORD pty_master_thread ();
  DWORD pty_master_fwd_thread ();
  int process_slave_output (char *buf, size_t len, int pktmode_on);
  void doecho (const void *str, DWORD len);
  int accept_input ();
  int open (int flags, mode_t mode = 0);
  void open_setup (int flags);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __reg3 read (void *ptr, size_t& len);
  int close ();
  void cleanup ();

  int tcsetattr (int a, const struct termios *t);
  int tcgetattr (struct termios *t);
  int tcflush (int);
  int ioctl (unsigned int cmd, void *);

  int ptsname_r (char *, size_t);

  bool hit_eof ();
  bool setup ();
  int dup (fhandler_base *, int);
  void fixup_after_fork (HANDLE parent);
  void fixup_after_exec ();
  int tcgetpgrp ();
  void flush_to_slave ();

  fhandler_pty_master (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_pty_master *> (x) = *this;
    x->reset (this);
  }

  fhandler_pty_master *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pty_master));
    fhandler_pty_master *fh = new (ptr) fhandler_pty_master (ptr);
    copyto (fh);
    return fh;
  }

  bool setup_pseudoconsole (void);
};

class fhandler_dev_null: public fhandler_base
{
 public:
  fhandler_dev_null ();

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_dev_null (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_null *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_null *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_null));
    fhandler_dev_null *fh = new (ptr) fhandler_dev_null (ptr);
    copyto (fh);
    return fh;
  }

  ssize_t __stdcall write (const void *ptr, size_t len);
};

class fhandler_dev_zero: public fhandler_base
{
 public:
  fhandler_dev_zero ();
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __reg3 read (void *ptr, size_t& len);
  off_t lseek (off_t, int) { return 0; }

  virtual HANDLE mmap (caddr_t *addr, size_t len, int prot,
		       int flags, off_t off);
  virtual int munmap (HANDLE h, caddr_t addr, size_t len);
  virtual int msync (HANDLE h, caddr_t addr, size_t len, int flags);
  virtual bool fixup_mmap_after_fork (HANDLE h, int prot, int flags,
				      off_t offset, SIZE_T size,
				      void *address);

  fhandler_dev_zero (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_zero *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_zero *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_zero));
    fhandler_dev_zero *fh = new (ptr) fhandler_dev_zero (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_dev_random: public fhandler_base
{
 protected:
  uint32_t pseudo;

  int pseudo_write (const void *ptr, size_t len);
  int pseudo_read (void *ptr, size_t len);

 public:
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __reg3 read (void *ptr, size_t& len);
  off_t lseek (off_t, int) { return 0; }

  fhandler_dev_random () : fhandler_base () {}
  fhandler_dev_random (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_random *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_random *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_random));
    fhandler_dev_random *fh = new (ptr) fhandler_dev_random (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_dev_clipboard: public fhandler_base
{
  UINT cygnativeformat;
  off_t pos;
  void *membuffer;
  size_t msize;
  int set_clipboard (const void *buf, size_t len);

 public:
  fhandler_dev_clipboard ();
  int is_windows () { return 1; }
  int __reg2 fstat (struct stat *buf);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __reg3 read (void *ptr, size_t& len);
  off_t lseek (off_t offset, int whence);
  int close ();

  int dup (fhandler_base *child, int);
  void fixup_after_exec ();

  fhandler_dev_clipboard (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_clipboard *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_clipboard *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_clipboard));
    fhandler_dev_clipboard *fh = new (ptr) fhandler_dev_clipboard (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_windows: public fhandler_base
{
 private:
  HWND hWnd_;	// the window whose messages are to be retrieved by read() call
  int method_;  // write method (Post or Send)
 public:
  fhandler_windows ();
  int is_windows () { return 1; }
  HWND get_hwnd () { return hWnd_; }
  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __reg3 read (void *ptr, size_t& len);
  int ioctl (unsigned int cmd, void *);
  off_t lseek (off_t, int) { return 0; }
  int close () { return 0; }

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_windows (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_windows *> (x) = *this;
    x->reset (this);
  }

  fhandler_windows *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_windows));
    fhandler_windows *fh = new (ptr) fhandler_windows (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_dev_dsp: public fhandler_base
{
 public:
  class Audio;
  class Audio_out;
  class Audio_in;
 private:
  int audioformat_;
  int audiofreq_;
  int audiobits_;
  int audiochannels_;
  Audio_out *audio_out_;
  Audio_in  *audio_in_;
 public:
  fhandler_dev_dsp ();
  fhandler_dev_dsp *base () const {return (fhandler_dev_dsp *)archetype;}

  int open (int, mode_t mode = 0);
  ssize_t __stdcall write (const void *, size_t);
  void __reg3 read (void *, size_t&);
  int ioctl (unsigned int, void *);
  int close ();
  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();

 private:
  ssize_t __stdcall _write (const void *, size_t);
  void __reg3 _read (void *, size_t&);
  int _ioctl (unsigned int, void *);
  void _fixup_after_fork (HANDLE);
  void _fixup_after_exec ();

  void __reg1 close_audio_in ();
  void __reg2 close_audio_out (bool = false);
  bool use_archetype () const {return true;}

  fhandler_dev_dsp (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_dsp *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_dsp *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_dsp));
    fhandler_dev_dsp *fh = new (ptr) fhandler_dev_dsp (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_virtual : public fhandler_base
{
 protected:
  char *filebuf;
  off_t filesize;
  off_t position;
  int fileid; // unique within each class
 public:

  fhandler_virtual ();
  virtual ~fhandler_virtual();

  virtual virtual_ftype_t exists();
  DIR __reg2 *opendir (int fd);
  long telldir (DIR *);
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __reg3 read (void *ptr, size_t& len);
  off_t lseek (off_t, int);
  int dup (fhandler_base *child, int);
  int open (int flags, mode_t mode = 0);
  int close ();
  int __reg2 fstatvfs (struct statvfs *buf);
  int __reg1 fchmod (mode_t mode);
  int __reg2 fchown (uid_t uid, gid_t gid);
  int __reg3 facl (int, int, struct acl *);
  virtual bool fill_filebuf ();
  char *get_filebuf () { return filebuf; }
  void fixup_after_exec ();

  fhandler_virtual (void *) {}

  virtual void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_virtual *> (x) = *this;
    x->reset (this);
  }

  virtual fhandler_virtual *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_virtual));
    fhandler_virtual *fh = new (ptr) fhandler_virtual (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_proc: public fhandler_virtual
{
 public:
  fhandler_proc ();
  virtual_ftype_t exists();
  DIR __reg2 *opendir (int fd);
  int closedir (DIR *);
  int __reg3 readdir (DIR *, dirent *);
  static fh_devices get_proc_fhandler (const char *path);

  int open (int flags, mode_t mode = 0);
  int __reg2 fstat (struct stat *buf);
  bool fill_filebuf ();

  fhandler_proc (void *) {}

  virtual void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_proc *> (x) = *this;
    x->reset (this);
  }

  virtual fhandler_proc *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_proc));
    fhandler_proc *fh = new (ptr) fhandler_proc (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_procsys: public fhandler_virtual
{
 public:
  fhandler_procsys ();
  virtual_ftype_t __reg2 exists(struct stat *buf);
  virtual_ftype_t exists();
  DIR __reg2 *opendir (int fd);
  int __reg3 readdir (DIR *, dirent *);
  long telldir (DIR *);
  void seekdir (DIR *, long);
  int closedir (DIR *);
  int open (int flags, mode_t mode = 0);
  int close ();
  void __reg3 read (void *ptr, size_t& len);
  ssize_t __stdcall write (const void *ptr, size_t len);
  int __reg2 fstat (struct stat *buf);
  bool fill_filebuf ();

  fhandler_procsys (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_procsys *> (x) = *this;
    x->reset (this);
  }

  fhandler_procsys *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_procsys));
    fhandler_procsys *fh = new (ptr) fhandler_procsys (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_procsysvipc: public fhandler_proc
{
  pid_t pid;
 public:
  fhandler_procsysvipc ();
  virtual_ftype_t exists();
  int __reg3 readdir (DIR *, dirent *);
  int open (int flags, mode_t mode = 0);
  int __reg2 fstat (struct stat *buf);
  bool fill_filebuf ();

  fhandler_procsysvipc (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_procsysvipc *> (x) = *this;
    x->reset (this);
  }

  fhandler_procsysvipc *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_procsysvipc));
    fhandler_procsysvipc *fh = new (ptr) fhandler_procsysvipc (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_netdrive: public fhandler_virtual
{
 public:
  fhandler_netdrive ();
  virtual_ftype_t exists();
  int __reg3 readdir (DIR *, dirent *);
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);
  int open (int flags, mode_t mode = 0);
  int close ();
  int __reg2 fstat (struct stat *buf);

  fhandler_netdrive (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_netdrive *> (x) = *this;
    x->reset (this);
  }

  fhandler_netdrive *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_netdrive));
    fhandler_netdrive *fh = new (ptr) fhandler_netdrive (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_registry: public fhandler_proc
{
 private:
  wchar_t *value_name;
  DWORD wow64;
  int prefix_len;
 public:
  fhandler_registry ();
  void set_name (path_conv &pc);
  virtual_ftype_t exists();
  DIR __reg2 *opendir (int fd);
  int __reg3 readdir (DIR *, dirent *);
  long telldir (DIR *);
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);

  int open (int flags, mode_t mode = 0);
  int __reg2 fstat (struct stat *buf);
  bool fill_filebuf ();
  int close ();
  int dup (fhandler_base *child, int);

  fhandler_registry (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_registry *> (x) = *this;
    x->reset (this);
  }

  fhandler_registry *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_registry));
    fhandler_registry *fh = new (ptr) fhandler_registry (ptr);
    copyto (fh);
    return fh;
  }
};

class pinfo;
class fhandler_process: public fhandler_proc
{
 protected:
  pid_t pid;
  virtual_ftype_t fd_type;
 public:
  fhandler_process ();
  virtual_ftype_t exists();
  DIR __reg2 *opendir (int fd);
  int closedir (DIR *);
  int __reg3 readdir (DIR *, dirent *);
  int open (int flags, mode_t mode = 0);
  int __reg2 fstat (struct stat *buf);
  bool fill_filebuf ();

  fhandler_process (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_process *> (x) = *this;
    x->reset (this);
  }

  fhandler_process *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_process));
    fhandler_process *fh = new (ptr) fhandler_process (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_process_fd : public fhandler_process
{
  fhandler_base *fetch_fh (HANDLE &, uint32_t);

 public:
  fhandler_process_fd () : fhandler_process () {}
  fhandler_process_fd (void *) {}

  virtual fhandler_base *fd_reopen (int, mode_t);
  int __reg2 fstat (struct stat *buf);
  virtual int __reg2 link (const char *);

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_process_fd *> (x) = *this;
    x->reset (this);
  }

  fhandler_process_fd *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_process_fd));
    fhandler_process_fd *fh = new (ptr) fhandler_process_fd (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_procnet: public fhandler_proc
{
  pid_t pid;
 public:
  fhandler_procnet ();
  fhandler_procnet (void *) {}
  virtual_ftype_t exists();
  int __reg3 readdir (DIR *, dirent *);
  int open (int flags, mode_t mode = 0);
  int __reg2 fstat (struct stat *buf);
  bool fill_filebuf ();

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_procnet *> (x) = *this;
    x->reset (this);
  }

  fhandler_procnet *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_procnet));
    fhandler_procnet *fh = new (ptr) fhandler_procnet (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_signalfd : public fhandler_base
{
  sigset_t sigset;

 public:
  fhandler_signalfd ();
  fhandler_signalfd (void *) {}

  fhandler_signalfd *is_signalfd () { return this; }

  char *get_proc_fd_name (char *buf);

  int signalfd (const sigset_t *mask, int flags);
  int __reg2 fstat (struct stat *buf);
  void __reg3 read (void *ptr, size_t& len);
  ssize_t __stdcall write (const void *, size_t);

  int poll ();
  inline sigset_t get_sigset () const { return sigset; }

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_signalfd *> (x) = *this;
    x->reset (this);
  }

  fhandler_signalfd *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_signalfd));
    fhandler_signalfd *fh = new (ptr) fhandler_signalfd (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_timerfd : public fhandler_base
{
  timer_t timerid;

 public:
  fhandler_timerfd ();
  fhandler_timerfd (void *) {}
  ~fhandler_timerfd () {}

  fhandler_timerfd *is_timerfd () { return this; }

  char *get_proc_fd_name (char *buf);

  int timerfd (clockid_t clock_id, int flags);
  int settime (int flags, const struct itimerspec *value,
	       struct itimerspec *ovalue);
  int gettime (struct itimerspec *ovalue);

  int __reg2 fstat (struct stat *buf);
  void __reg3 read (void *ptr, size_t& len);
  ssize_t __stdcall write (const void *, size_t);
  int dup (fhandler_base *child, int);
  int ioctl (unsigned int, void *);
  int close ();

  HANDLE get_timerfd_handle ();

  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_timerfd *> (x) = *this;
    x->reset (this);
  }

  fhandler_timerfd *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_timerfd));
    fhandler_timerfd *fh = new (ptr) fhandler_timerfd (ptr);
    copyto (fh);
    return fh;
  }
};

struct fhandler_nodevice: public fhandler_base
{
  fhandler_nodevice ();
  int open (int flags, mode_t mode = 0);
};

#define report_tty_counts(fh, call, use_op) \
  termios_printf ("%s %s, %susecount %d",\
		  fh->ttyname (), call,\
		  use_op, ((fhandler_pty_slave *) (fh->archetype ?: fh))->usecount);

typedef union
{
  char __base[sizeof (fhandler_base)];
  char __console[sizeof (fhandler_console)];
  char __dev[sizeof (fhandler_dev)];
  char __cygdrive[sizeof (fhandler_cygdrive)];
  char __dev_clipboard[sizeof (fhandler_dev_clipboard)];
  char __dev_dsp[sizeof (fhandler_dev_dsp)];
  char __dev_floppy[sizeof (fhandler_dev_floppy)];
  char __dev_null[sizeof (fhandler_dev_null)];
  char __dev_random[sizeof (fhandler_dev_random)];
  char __dev_raw[sizeof (fhandler_dev_raw)];
  char __dev_tape[sizeof (fhandler_dev_tape)];
  char __dev_zero[sizeof (fhandler_dev_zero)];
  char __disk_file[sizeof (fhandler_disk_file)];
  char __fifo[sizeof (fhandler_fifo)];
  char __netdrive[sizeof (fhandler_netdrive)];
  char __nodevice[sizeof (fhandler_nodevice)];
  char __pipe[sizeof (fhandler_pipe)];
  char __proc[sizeof (fhandler_proc)];
  char __process[sizeof (fhandler_process)];
  char __process_fd[sizeof (fhandler_process_fd)];
  char __procnet[sizeof (fhandler_procnet)];
  char __procsys[sizeof (fhandler_procsys)];
  char __procsysvipc[sizeof (fhandler_procsysvipc)];
  char __pty_master[sizeof (fhandler_pty_master)];
  char __registry[sizeof (fhandler_registry)];
  char __serial[sizeof (fhandler_serial)];
  char __signalfd[sizeof (fhandler_signalfd)];
  char __timerfd[sizeof (fhandler_timerfd)];
  char __socket_inet[sizeof (fhandler_socket_inet)];
  char __socket_local[sizeof (fhandler_socket_local)];
#ifdef __WITH_AF_UNIX
  char __socket_unix[sizeof (fhandler_socket_unix)];
#endif /* __WITH_AF_UNIX */
  char __termios[sizeof (fhandler_termios)];
  char __pty_common[sizeof (fhandler_pty_common)];
  char __pty_slave[sizeof (fhandler_pty_slave)];
  char __virtual[sizeof (fhandler_virtual)];
  char __windows[sizeof (fhandler_windows)];
} fhandler_union;
