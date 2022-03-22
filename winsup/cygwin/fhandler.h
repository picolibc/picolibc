/* fhandler.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once
#include "pinfo.h"

#include "tty.h"
#include "mqueue_types.h"
#include <cygwin/_socketflags.h>
#include <cygwin/_ucred.h>
#include <sys/un.h>

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
    unsigned rbinset		: 1; /* binary read mode explicitly set */
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

   public:
    status_flags () :
      rbinary (0), rbinset (0), wbinary (0), wbinset (0), nohandle (0),
      did_lseek (0), query_open (no_query), close_on_exec (0),
      need_fork_fixup (0), isclosed (0), mandatory_locking (0)
      {}
  } status, open_status;

 private:
  ACCESS_MASK access;
  ULONG options;

  HANDLE io_handle;

  ino_t ino;	/* file ID or hashed filename, depends on FS. */
  LONG _refcnt;
 public:
  struct rabuf_t
  {
    char *rabuf;		/* used for crlf conversion in text files */
    size_t ralen;
    size_t raixget;
    size_t raixput;
    size_t rabuflen;
  };

 protected:
  /* File open flags from open () and fcntl () calls */
  int openflags;

  struct rabuf_t ra;

  /* Used for advisory file locking.  See flock.cc.  */
  int64_t unique_id;
  void del_my_locks (del_lock_called_from);
  void set_ino (ino_t i) { ino = i; }

  HANDLE read_state;
  HANDLE select_sem;

 public:
  LONG inc_refcnt () {return InterlockedIncrement (&_refcnt);}
  LONG dec_refcnt () {return InterlockedDecrement (&_refcnt);}
  class fhandler_base *archetype;
  int usecount;

  path_conv pc;

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
	       | (((open_status.wbinset ? open_status.wbinary : 1)
		   || (open_status.rbinset ? open_status.rbinary : 1))
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

  virtual char *&rabuf () { return ra.rabuf; };
  virtual size_t &ralen () { return ra.ralen; };
  virtual size_t &raixget () { return ra.raixget; };
  virtual size_t &raixput () { return ra.raixput; };
  virtual size_t &rabuflen () { return ra.rabuflen; };

  bool get_readahead_valid () { return raixget () < ralen (); }
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
  virtual bool open_setup (int flags);
  virtual void post_open_setup (int fd) {}
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
  int __reg2 fstatvfs_by_handle (HANDLE h, struct statvfs *buf);
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

  /* Get a handle to the named pipe file system directory.  Used by
     fhandler_pipe, fhandler_fifo, and fhandler_socket_unix. */
  static NTSTATUS npfs_handle (HANDLE &);

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
  virtual class fhandler_mqueue *is_mqueue () { return NULL; }
  virtual int is_windows () {return 0; }

  virtual void __reg3 raw_read (void *ptr, size_t& ulen);
  virtual ssize_t __reg3 raw_write (const void *ptr, size_t ulen);

  /* Virtual accessor functions to hide the fact
     that some fd's have two handles. */
  virtual HANDLE& get_handle () { return io_handle; }
  virtual HANDLE& get_handle_nat () { return io_handle; }
  virtual HANDLE& get_output_handle () { return io_handle; }
  virtual HANDLE& get_output_handle_nat () { return io_handle; }
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
    raixput () = raixget () = ralen () = rabuflen () = 0;
    rabuf () = NULL;
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

  fhandler_base (void *) {}

 protected:
  void _copy_from_reset_helper ()
  {
    ra.rabuf = NULL;
    ra.ralen = 0;
    ra.raixget = 0;
    ra.raixput = 0;
    ra.rabuflen = 0;
    _refcnt = 0;
  }

 public:
  virtual void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_base *> (x);
    _copy_from_reset_helper ();
  }

  virtual fhandler_base *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_base));
    fhandler_base *fh = new (ptr) fhandler_base (ptr);
    fh->copy_from (this);
    return fh;
  }

  HANDLE get_select_sem () { return select_sem; }
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
  bool oobinline;	/* True if option SO_OOBINLINE is set */
  bool tcp_quickack;	/* True if quickack is enabled */
  bool tcp_fastopen;	/* True if TCP_FASTOPEN is set on older systems */
  int  tcp_keepidle;	/* TCP_KEEPIDLE value in secs on older systems */
  int  tcp_keepcnt;	/* TCP_KEEPCNT value on older systems */
  int  tcp_keepintvl;	/* TCP_KEEPINTVL value in secs on older systems */

  int set_keepalive (int keepidle, int keepcnt, int keepintvl);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_socket_inet *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_socket_inet *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_socket_inet));
    fhandler_socket_inet *fh = new (ptr) fhandler_socket_inet (ptr);
    fh->copy_from (this);
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

  int open (int flags, mode_t mode = 0);
  int close ();
  int fcntl (int cmd, intptr_t);
  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  int __reg1 fchmod (mode_t newmode);
  int __reg2 fchown (uid_t newuid, gid_t newgid);
  int __reg3 facl (int, int, struct acl *);
  int __reg2 link (const char *);

  /* from here on: CLONING */
  fhandler_socket_local (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_socket_local *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_socket_local *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_socket_local));
    fhandler_socket_local *fh = new (ptr) fhandler_socket_local (ptr);
    fh->copy_from (this);
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
  void fixup_helper ();

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
  int open (int flags, mode_t mode = 0);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_socket_unix *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_socket_unix *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_socket_unix));
    fhandler_socket_unix *fh = new (ptr) fhandler_socket_unix (ptr);
    fh->copy_from (this);
    return fh;
  }
};

#endif /* __WITH_AF_UNIX */

/* A parent of fhandler_pipe and fhandler_fifo. */
class fhandler_pipe_fifo: public fhandler_base
{
 protected:
  size_t pipe_buf_size;
  virtual void release_select_sem (const char *) {};

 public:
  fhandler_pipe_fifo ();

  virtual bool reader_closed () { return false; };
  ssize_t __reg3 raw_write (const void *ptr, size_t len);
};

class fhandler_pipe: public fhandler_pipe_fifo
{
private:
  HANDLE read_mtx;
  pid_t popen_pid;
  HANDLE query_hdl;
  HANDLE hdl_cnt_mtx;
  HANDLE query_hdl_proc;
  HANDLE query_hdl_value;
  HANDLE query_hdl_close_req_evt;
  uint64_t pipename_key;
  DWORD pipename_pid;
  LONG pipename_id;
  void release_select_sem (const char *);
  HANDLE get_query_hdl_per_process (WCHAR *, OBJECT_NAME_INFORMATION *);
  HANDLE get_query_hdl_per_system (WCHAR *, OBJECT_NAME_INFORMATION *);
public:
  fhandler_pipe ();

  bool ispipe() const { return true; }
  void set_pipe_buf_size ();

  void set_popen_pid (pid_t pid) {popen_pid = pid;}
  pid_t get_popen_pid () const {return popen_pid;}
  off_t lseek (off_t offset, int whence);
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
  char *get_proc_fd_name (char *buf);
  int open (int flags, mode_t mode = 0);
  bool open_setup (int flags);
  void fixup_after_fork (HANDLE);
  int dup (fhandler_base *child, int);
  void set_close_on_exec (bool val);
  int close ();
  void __reg3 raw_read (void *ptr, size_t& len);
  int ioctl (unsigned int cmd, void *);
  int fcntl (int cmd, intptr_t);
  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  int __reg3 fadvise (off_t, off_t, int);
  int __reg3 ftruncate (off_t, bool);
  int init (HANDLE, DWORD, mode_t, int64_t);
  static int create (fhandler_pipe *[2], unsigned, int);
  static DWORD create (LPSECURITY_ATTRIBUTES, HANDLE *, HANDLE *, DWORD,
		       const char *, DWORD, int64_t *unique_id = NULL);
  fhandler_pipe (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_pipe *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_pipe *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pipe));
    fhandler_pipe *fh = new (ptr) fhandler_pipe (ptr);
    fh->copy_from (this);
    return fh;
  }
  void set_pipe_non_blocking (bool nonblocking);
  HANDLE get_query_handle () const { return query_hdl; }
  void close_query_handle ()
  {
    if (query_hdl)
      {
	CloseHandle (query_hdl);
	query_hdl = NULL;
      }
    if (query_hdl_close_req_evt)
      {
	CloseHandle (query_hdl_close_req_evt);
	query_hdl_close_req_evt = NULL;
      }
  }
  bool reader_closed ();
  HANDLE temporary_query_hdl ();
  bool need_close_query_hdl ()
    {
      return query_hdl_close_req_evt ?
	IsEventSignalled (query_hdl_close_req_evt) : false;
    }
  bool request_close_query_hdl ()
    {
      if (query_hdl_close_req_evt)
	{
	  SetEvent (query_hdl_close_req_evt);
	  return true;
	}
      return false;
    }
};

#define CYGWIN_FIFO_PIPE_NAME_LEN     47

enum fifo_client_connect_state
{
  fc_unknown,
  fc_error,
  fc_disconnected,
  fc_closing,
  fc_listening,
  fc_connected,
  fc_input_avail,
};

struct fifo_client_handler
{
  HANDLE h;
  fifo_client_connect_state _state;
  bool last_read;  /* true if our last successful read was from this client. */
  fifo_client_handler () : h (NULL), _state (fc_unknown), last_read (false) {}
  void close () { NtClose (h); }
  fifo_client_connect_state get_state () const { return _state; }
  void set_state (fifo_client_connect_state s) { _state = s; }
  /* Query O/S.  Return previous state. */
  fifo_client_connect_state query_and_set_state ();
};

class fhandler_fifo;

struct fifo_reader_id_t
{
  DWORD winpid;
  fhandler_fifo *fh;

  operator bool () const { return winpid != 0 || fh != NULL; }

  friend bool operator == (const fifo_reader_id_t &l, const fifo_reader_id_t &r)
  {
    return l.winpid == r.winpid && l.fh == r.fh;
  }

  friend bool operator != (const fifo_reader_id_t &l, const fifo_reader_id_t &r)
  {
    return l.winpid != r.winpid || l.fh != r.fh;
  }
};

/* Info needed by all fhandlers for a given FIFO, stored in named
   shared memory.  This is mostly for readers, but writers need access
   in order to update the count of open writers. */
class fifo_shmem_t
{
  LONG _nreaders, _nwriters;
  fifo_reader_id_t _owner, _prev_owner, _pending_owner;
  af_unix_spinlock_t _owner_lock, _reading_lock, _nreaders_lock, _nwriters_lock;

  /* Info about shared memory block used for temporary storage of the
     owner's fc_handler list. */
  LONG _sh_nhandlers, _sh_shandlers, _sh_fc_handler_committed,
    _sh_fc_handler_updated;

public:
  int nreaders () const { return (int) _nreaders; }
  int inc_nreaders () { return (int) InterlockedIncrement (&_nreaders); }
  int dec_nreaders () { return (int) InterlockedDecrement (&_nreaders); }
  int nwriters () const { return (int) _nwriters; }
  int inc_nwriters () { return (int) InterlockedIncrement (&_nwriters); }
  int dec_nwriters () { return (int) InterlockedDecrement (&_nwriters); }

  fifo_reader_id_t get_owner () const { return _owner; }
  void set_owner (fifo_reader_id_t fr_id) { _owner = fr_id; }
  fifo_reader_id_t get_prev_owner () const { return _prev_owner; }
  void set_prev_owner (fifo_reader_id_t fr_id) { _prev_owner = fr_id; }
  fifo_reader_id_t get_pending_owner () const { return _pending_owner; }
  void set_pending_owner (fifo_reader_id_t fr_id) { _pending_owner = fr_id; }

  void owner_lock () { _owner_lock.lock (); }
  void owner_unlock () { _owner_lock.unlock (); }
  void reading_lock () { _reading_lock.lock (); }
  void reading_unlock () { _reading_lock.unlock (); }
  void nreaders_lock () { _nreaders_lock.lock (); }
  void nreaders_unlock () { _nreaders_lock.unlock (); }
  void nwriters_lock () { _nwriters_lock.lock (); }
  void nwriters_unlock () { _nwriters_lock.unlock (); }

  int get_shared_nhandlers () const { return (int) _sh_nhandlers; }
  void set_shared_nhandlers (int n) { InterlockedExchange (&_sh_nhandlers, n); }
  int get_shared_shandlers () const { return (int) _sh_shandlers; }
  void set_shared_shandlers (int n) { InterlockedExchange (&_sh_shandlers, n); }
  size_t get_shared_fc_handler_committed () const
  { return (size_t) _sh_fc_handler_committed; }
  void set_shared_fc_handler_committed (size_t n)
  { InterlockedExchange (&_sh_fc_handler_committed, (LONG) n); }
  bool shared_fc_handler_updated () const { return _sh_fc_handler_updated; }
  void shared_fc_handler_updated (bool val)
  { InterlockedExchange (&_sh_fc_handler_updated, val); }
};

class fhandler_fifo: public fhandler_pipe_fifo
{
  /* Handles to named events shared by all fhandlers for a given FIFO. */
  HANDLE read_ready;            /* A reader is open; OK for a writer to open. */
  HANDLE write_ready;           /* A writer is open; OK for a reader to open. */
  HANDLE writer_opening;        /* A writer is opening; no EOF. */

  /* Handles to named events needed by all readers of a given FIFO. */
  HANDLE owner_needed_evt;      /* The owner is closing. */
  HANDLE owner_found_evt;       /* A new owner has taken over. */
  HANDLE update_needed_evt;     /* shared_fc_handler needs updating. */

  /* Handles to non-shared events needed for fifo_reader_threads. */
  HANDLE cancel_evt;            /* Signal thread to terminate. */
  HANDLE thr_sync_evt;          /* The thread has terminated. */

  UNICODE_STRING pipe_name;
  PWCHAR pipe_name_buf;
  fifo_client_handler *fc_handler;     /* Dynamically growing array. */
  int shandlers;                       /* Size (capacity) of the array. */
  int nhandlers;                       /* Number of elements in the array. */
  af_unix_spinlock_t _fifo_client_lock;
  bool reader, writer, duplexer;
  fifo_reader_id_t me;

  HANDLE shmem_handle;
  fifo_shmem_t *shmem;
  HANDLE shared_fc_hdl;
  /* Dynamically growing array in shared memory. */
  fifo_client_handler *shared_fc_handler;

  bool __reg2 wait (HANDLE);
  HANDLE create_pipe_instance ();
  NTSTATUS open_pipe (HANDLE&);
  NTSTATUS wait_open_pipe (HANDLE&);
  int add_client_handler (bool new_pipe_instance = true);
  void delete_client_handler (int);
  void cleanup_handlers ();
  void close_all_handlers ();
  void cancel_reader_thread ();

  int create_shmem (bool only_open = false);
  int reopen_shmem ();
  int create_shared_fc_handler ();
  int reopen_shared_fc_handler ();
  int remap_shared_fc_handler (size_t);

  int nreaders () const { return shmem->nreaders (); }
  int inc_nreaders () { return shmem->inc_nreaders (); }
  int dec_nreaders () { return shmem->dec_nreaders (); }
  int nwriters () const { return shmem->nwriters (); }
  int inc_nwriters () { return shmem->inc_nwriters (); }
  int dec_nwriters () { return shmem->dec_nwriters (); }
  void nreaders_lock () { shmem->nreaders_lock (); }
  void nreaders_unlock () { shmem->nreaders_unlock (); }
  void nwriters_lock () { shmem->nwriters_lock (); }
  void nwriters_unlock () { shmem->nwriters_unlock (); }

  fifo_reader_id_t get_owner () const { return shmem->get_owner (); }
  void set_owner (fifo_reader_id_t fr_id) { shmem->set_owner (fr_id); }
  fifo_reader_id_t get_prev_owner () const { return shmem->get_prev_owner (); }
  void set_prev_owner (fifo_reader_id_t fr_id)
  { shmem->set_prev_owner (fr_id); }
  fifo_reader_id_t get_pending_owner () const
  { return shmem->get_pending_owner (); }
  void set_pending_owner (fifo_reader_id_t fr_id)
  { shmem->set_pending_owner (fr_id); }
  void owner_lock () { shmem->owner_lock (); }
  void owner_unlock () { shmem->owner_unlock (); }

  void owner_needed ()
  {
    ResetEvent (owner_found_evt);
    SetEvent (owner_needed_evt);
  }
  void owner_found ()
  {
    ResetEvent (owner_needed_evt);
    SetEvent (owner_found_evt);
  }

  int get_shared_nhandlers () { return shmem->get_shared_nhandlers (); }
  void set_shared_nhandlers (int n) { shmem->set_shared_nhandlers (n); }
  int get_shared_shandlers () { return shmem->get_shared_shandlers (); }
  void set_shared_shandlers (int n) { shmem->set_shared_shandlers (n); }
  size_t get_shared_fc_handler_committed () const
  { return shmem->get_shared_fc_handler_committed (); }
  void set_shared_fc_handler_committed (size_t n)
  { shmem->set_shared_fc_handler_committed (n); }
  int update_my_handlers ();
  int update_shared_handlers ();
  bool shared_fc_handler_updated () const
  { return shmem->shared_fc_handler_updated (); }
  void shared_fc_handler_updated (bool val)
  { shmem->shared_fc_handler_updated (val); }

  void release_select_sem (const char *);

public:
  fhandler_fifo ();
  ~fhandler_fifo ()
  {
    if (pipe_name_buf)
      cfree (pipe_name_buf);
  }
  /* Called if we appear to be at EOF after polling fc_handlers. */
  bool hit_eof () const
  { return !nwriters () && !IsEventSignalled (writer_opening); }
  int get_nhandlers () const { return nhandlers; }
  fifo_client_handler &get_fc_handler (int i) { return fc_handler[i]; }
  PUNICODE_STRING get_pipe_name ();
  DWORD fifo_reader_thread_func ();
  void fifo_client_lock () { _fifo_client_lock.lock (); }
  void fifo_client_unlock () { _fifo_client_lock.unlock (); }
  void record_connection (fifo_client_handler&, bool = true,
			  fifo_client_connect_state = fc_connected);

  int take_ownership (DWORD timeout = INFINITE);
  void reading_lock () { shmem->reading_lock (); }
  void reading_unlock () { shmem->reading_unlock (); }

  int open (int, mode_t);
  off_t lseek (off_t offset, int whence);
  int close ();
  int fcntl (int cmd, intptr_t);
  int dup (fhandler_base *child, int);
  bool isfifo () const { return true; }
  void set_close_on_exec (bool val);
  void __reg3 raw_read (void *ptr, size_t& ulen);
  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();
  int __reg2 fstat (struct stat *buf);
  int __reg2 fstatvfs (struct statvfs *buf);
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_fifo (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_fifo *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_fifo *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_fifo));
    fhandler_fifo *fhf = new (ptr) fhandler_fifo (ptr);
    fhf->copy_from (this);
    fhf->pipe_name_buf = NULL;
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_raw *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_raw *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_raw));
    fhandler_dev_raw *fh = new (ptr) fhandler_dev_raw (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_floppy *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_floppy *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_floppy));
    fhandler_dev_floppy *fh = new (ptr) fhandler_dev_floppy (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_tape *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_tape *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_tape));
    fhandler_dev_tape *fh = new (ptr) fhandler_dev_tape (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_disk_file *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_disk_file *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_disk_file));
    fhandler_disk_file *fh = new (ptr) fhandler_disk_file (ptr);
    fh->copy_from (this);
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
  int rmdir ();
  DIR __reg2 *opendir (int fd);
  int __reg3 readdir (DIR *, dirent *);
  void rewinddir (DIR *);

  fhandler_dev (void *) {}
  dev_t get_dev () { return dir_exists ? pc.fs_serial_number ()
				       : get_device (); }

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev));
    fhandler_dev *fh = new (ptr) fhandler_dev (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_cygdrive *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_cygdrive *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_cygdrive));
    fhandler_cygdrive *fh = new (ptr) fhandler_cygdrive (ptr);
    fh->copy_from (this);
    return fh;
  }
};

class fhandler_serial: public fhandler_base
{
 private:
  cc_t vmin_;				/* from termios */
  cc_t vtime_;				/* from termios */
  pid_t pgrp_;
  int rts;				/* for Windows 9x purposes only */
  int dtr;				/* for Windows 9x purposes only */

 public:
  /* Constructor */
  fhandler_serial ();

  int open (int flags, mode_t mode);
  int init (HANDLE h, DWORD a, mode_t flags);
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

  /* We maintain a pgrp so that tcsetpgrp and tcgetpgrp work, but we
     don't use it for permissions checking.  fhandler_pty_slave does
     permission checking on pgrps.  */
  virtual int tcgetpgrp () { return pgrp_; }
  virtual int tcsetpgrp (const pid_t pid) { pgrp_ = pid; return 0; }
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_serial (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_serial *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_serial *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_serial));
    fhandler_serial *fh = new (ptr) fhandler_serial (ptr);
    fh->copy_from (this);
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

extern DWORD mutex_timeout;
DWORD acquire_attach_mutex (DWORD t);
void release_attach_mutex (void);

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
  virtual void acquire_input_mutex_if_necessary (DWORD ms) {};
  virtual void release_input_mutex_if_necessary (void) {};
  virtual void discard_input () {};

  /* Result status of processing keys in process_sigs(). */
  enum process_sig_state {
    signalled, /* Signalled normally */
    not_signalled, /* Not signalled at all */
    not_signalled_but_done, /* Not signalled, but CTRL_C_EVENT was sent. */
    not_signalled_with_nat_reader, /* Not signalled, but detected non-cygwin
				      process may be reading the tty. */
    done_with_debugger /* The key was processed (CTRL_C_EVENT was sent)
			  for inferior of GDB. */
  };

 public:
  virtual pid_t tc_getpgid () { return 0; };
  tty_min*& tc () {return _tc;}
  fhandler_termios () :
  fhandler_base ()
  {
    need_fork_fixup (true);
  }
  HANDLE& get_output_handle () { return output_handle; }
  HANDLE& get_output_handle_nat () { return output_handle; }
  static process_sig_state process_sigs (char c, tty *ttyp,
					 fhandler_termios *fh);
  static bool process_stop_start (char c, tty *ttyp);
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

  virtual void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_termios *> (x);
    _copy_from_reset_helper ();
  }

  virtual fhandler_termios *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_termios));
    fhandler_termios *fh = new (ptr) fhandler_termios (ptr);
    fh->copy_from (this);
    return fh;
  }
  static bool path_iscygexec_a (LPCSTR n, LPSTR c);
  static bool path_iscygexec_w (LPCWSTR n, LPWSTR c);
  virtual void cleanup_before_exit () {}
  virtual void setpgid_aux (pid_t pid) {}
  virtual bool need_console_handler () { return false; }
  virtual bool need_send_ctrl_c_event () { return true; }
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
#define MAXARGS 16

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
  bool saw_exclamation_mark;
  bool vt100_graphics_mode_G0;
  bool vt100_graphics_mode_G1;
  bool iso_2022_G1;
  bool alternate_charset_active;
  bool metabit;
  char backspace_keycode;
  bool screen_alternated; /* For xterm compatible mode only */

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
  bool cursor_key_app_mode;
  bool disable_master_thread;
  int num_processed; /* Number of input events in the current input buffer
			already processed by cons_master_thread(). */

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
  struct handle_set_t
  {
    HANDLE input_handle;
    HANDLE output_handle;
    HANDLE input_mutex;
    HANDLE output_mutex;
  };
  HANDLE thread_sync_event;
private:
  static const unsigned MAX_WRITE_CHARS;
  static console_state *shared_console_info;
  static bool invisible_console;
  HANDLE input_mutex, output_mutex;
  handle_set_t handle_set;

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

/* Input calls */
  int igncr_enabled ();
  void set_cursor_maybe ();
  static bool create_invisible_console_workaround (bool force);
  static console_state *open_shared_console (HWND, HANDLE&, bool&);
  static void fix_tab_position (HANDLE h);

/* console mode calls */
  const handle_set_t *get_handle_set (void) {return &handle_set;}
  static void set_input_mode (tty::cons_mode m, const termios *t,
			      const handle_set_t *p);
  static void set_output_mode (tty::cons_mode m, const termios *t,
			       const handle_set_t *p);

 public:
  pid_t tc_getpgid ()
  {
    return shared_console_info ?
      shared_console_info->tty_min_state.getpgid () : 0;
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
  bool open_setup (int flags);
  void post_open_setup (int fd);
  int dup (fhandler_base *, int);

  void __reg3 read (void *ptr, size_t& len);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void doecho (const void *str, DWORD len);
  int close ();
  static bool exists ()
    {
      acquire_attach_mutex (mutex_timeout);
      UINT cp = GetConsoleCP ();
      release_attach_mutex ();
      return !!cp;
    }

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
  bool send_winch_maybe ();
  void setup ();
  bool set_unit ();
  static bool need_invisible (bool force = false);
  static void free_console ();
  static const char *get_nonascii_key (INPUT_RECORD& input_rec, char *);

  fhandler_console (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_console *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_console *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_console));
    fhandler_console *fh = new (ptr) fhandler_console (ptr);
    fh->copy_from (this);
    return fh;
  }
  input_states process_input_message ();
  bg_check_types bg_check (int sig, bool dontsignal = false);
  void setup_io_mutex (void);
  DWORD __acquire_input_mutex (const char *fn, int ln, DWORD ms);
  void __release_input_mutex (const char *fn, int ln);
  DWORD __acquire_output_mutex (const char *fn, int ln, DWORD ms);
  void __release_output_mutex (const char *fn, int ln);
  void acquire_input_mutex_if_necessary (DWORD ms)
  {
    acquire_input_mutex (ms);
    acquire_attach_mutex (ms);
  }
  void release_input_mutex_if_necessary (void)
  {
    release_attach_mutex ();
    release_input_mutex ();
  }

  char *&rabuf ();
  size_t &ralen ();
  size_t &raixget ();
  size_t &raixput ();
  size_t &rabuflen ();

  void get_duplicated_handle_set (handle_set_t *p);
  static void close_handle_set (handle_set_t *p);

  static void cons_master_thread (handle_set_t *p, tty *ttyp);
  void setup_for_non_cygwin_app ();
  static void cleanup_for_non_cygwin_app (handle_set_t *p);
  static void set_console_mode_to_native ();
  bool need_console_handler ();

  friend tty_min * tty_list::get_cttyp ();
};

class fhandler_pty_common: public fhandler_termios
{
 public:
  fhandler_pty_common ()
    : fhandler_termios (),
    output_mutex (NULL), input_mutex (NULL), pipe_sw_mutex (NULL),
    input_available_event (NULL)
  {
    pc.file_attributes (FILE_ATTRIBUTE_NORMAL);
  }
  static const unsigned pipesize = 128 * 1024;
  HANDLE output_mutex, input_mutex, pipe_sw_mutex;
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

  virtual void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_pty_common *> (x);
    _copy_from_reset_helper ();
  }

  virtual fhandler_pty_common *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pty_common));
    fhandler_pty_common *fh = new (ptr) fhandler_pty_common (ptr);
    fh->copy_from (this);
    return fh;
  }

  void resize_pseudo_console (struct winsize *);
  static DWORD get_console_process_id (DWORD pid, bool match,
				       bool cygwin = false,
				       bool stub_only = false);
  bool to_be_read_from_nat_pipe (void);
  static DWORD attach_console_temporarily (DWORD target_pid);
  static void resume_from_temporarily_attach (DWORD resume_pid);

 protected:
  static BOOL process_opost_output (HANDLE h, const void *ptr, ssize_t& len,
				    bool is_echo, tty *ttyp,
				    bool is_nonblocking);
};

class fhandler_pty_slave: public fhandler_pty_common
{
  HANDLE inuse;			// used to indicate that a tty is in use
  HANDLE output_handle_nat, io_handle_nat;
  HANDLE slave_reading;
  LONG num_reader;

  /* Helper functions for fchmod and fchown. */
  bool fch_open_handles (bool chown);
  int fch_set_sd (security_descriptor &sd, bool chown);
  void fch_close_handles ();

 public:
  pid_t tc_getpgid () { return _tc ? _tc->pgid : 0; }

  struct handle_set_t
  {
    HANDLE from_master_nat;
    HANDLE input_available_event;
    HANDLE input_mutex;
    HANDLE pipe_sw_mutex;
  };

  /* Constructor */
  fhandler_pty_slave (int);

  void set_output_handle_nat (HANDLE h) { output_handle_nat = h; }
  HANDLE& get_output_handle_nat () { return output_handle_nat; }
  void set_handle_nat (HANDLE h) { io_handle_nat = h; }
  HANDLE& get_handle_nat () { return io_handle_nat; }

  int open (int flags, mode_t mode = 0);
  bool open_setup (int flags);
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
  bg_check_types bg_check (int sig, bool dontsignal = false);
  virtual char const *ttyname () { return pc.dev.name (); }
  int __reg2 fstat (struct stat *buf);
  int __reg3 facl (int, int, struct acl *);
  int __reg1 fchmod (mode_t mode);
  int __reg2 fchown (uid_t uid, gid_t gid);

  fhandler_pty_slave (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_pty_slave *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_pty_slave *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pty_slave));
    fhandler_pty_slave *fh = new (ptr) fhandler_pty_slave (ptr);
    fh->copy_from (this);
    return fh;
  }
  bool setup_pseudoconsole ();
  static DWORD get_winpid_to_hand_over (tty *ttyp, DWORD force_switch_to);
  static void close_pseudoconsole (tty *ttyp, DWORD force_switch_to = 0);
  static void hand_over_only (tty *ttyp, DWORD force_switch_to = 0);
  bool term_has_pcon_cap (const WCHAR *env);
  void set_switch_to_nat_pipe (void);
  void reset_switch_to_nat_pipe (void);
  void mask_switch_to_nat_pipe (bool mask, bool xfer);
  void setup_locale (void);
  void create_invisible_console (void);
  static void transfer_input (tty::xfer_dir dir, HANDLE from, tty *ttyp,
			      HANDLE input_available_event);
  HANDLE get_input_available_event (void) { return input_available_event; }
  bool pcon_activated (void) { return get_ttyp ()->pcon_activated; }
  void cleanup_before_exit ();
  void get_duplicated_handle_set (handle_set_t *p);
  static void close_handle_set (handle_set_t *p);
  void setup_for_non_cygwin_app (bool nopcon, PWCHAR envblock,
				 bool stdin_is_ptys);
  static void cleanup_for_non_cygwin_app (handle_set_t *p, tty *ttyp,
					  bool stdin_is_ptys,
					  DWORD force_switch_to = 0);
  void setpgid_aux (pid_t pid);
  static void release_ownership_of_nat_pipe (tty *ttyp, fhandler_termios *fh);
};

#define __ptsname(buf, unit) __small_sprintf ((buf), "/dev/pty%d", (unit))
class fhandler_pty_master: public fhandler_pty_common
{
public:
  /* Parameter set for the static function pty_master_thread() */
  struct master_thread_param_t {
    HANDLE from_master_nat;
    HANDLE from_master;
    HANDLE to_master_nat;
    HANDLE to_master;
    HANDLE to_slave_nat;
    HANDLE to_slave;
    HANDLE master_ctl;
    HANDLE input_available_event;
  };
  /* Parameter set for the static function pty_master_fwd_thread() */
  struct master_fwd_thread_param_t {
    HANDLE to_master;
    HANDLE from_slave_nat;
    HANDLE output_mutex;
    tty *ttyp;
  };
private:
  int pktmode;			// non-zero if pty in a packet mode.
  HANDLE master_ctl;		// Control socket for handle duplication
  cygthread *master_thread;	// Master control thread
  HANDLE from_master_nat, to_master_nat, from_slave_nat, to_slave_nat;
  HANDLE echo_r, echo_w;
  DWORD dwProcessId;		// Owner of master handles
  HANDLE to_master, from_master;
  cygthread *master_fwd_thread;	// Master forwarding thread
  HANDLE thread_param_copied_event;

public:
  HANDLE get_echo_handle () const { return echo_r; }
  /* Constructor */
  fhandler_pty_master (int);

  static DWORD pty_master_thread (const master_thread_param_t *p);
  static DWORD pty_master_fwd_thread (const master_fwd_thread_param_t *p);
  int process_slave_output (char *buf, size_t len, int pktmode_on);
  void doecho (const void *str, DWORD len);
  int accept_input ();
  int open (int flags, mode_t mode = 0);
  bool open_setup (int flags);
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
  void discard_input ();

  fhandler_pty_master (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_pty_master *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_pty_master *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_pty_master));
    fhandler_pty_master *fh = new (ptr) fhandler_pty_master (ptr);
    fh->copy_from (this);
    return fh;
  }
  void get_master_thread_param (master_thread_param_t *p);
  void get_master_fwd_thread_param (master_fwd_thread_param_t *p);
  void set_mask_flusho (bool m) { get_ttyp ()->mask_flusho = m; }
  bool need_send_ctrl_c_event ();
};

class fhandler_dev_null: public fhandler_base
{
 public:
  fhandler_dev_null ();

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);

  fhandler_dev_null (void *) {}

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_null *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_null *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_null));
    fhandler_dev_null *fh = new (ptr) fhandler_dev_null (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_zero *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_zero *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_zero));
    fhandler_dev_zero *fh = new (ptr) fhandler_dev_zero (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_random *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_random *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_random));
    fhandler_dev_random *fh = new (ptr) fhandler_dev_random (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_clipboard *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_clipboard *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_clipboard));
    fhandler_dev_clipboard *fh = new (ptr) fhandler_dev_clipboard (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_windows *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_windows *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_windows));
    fhandler_windows *fh = new (ptr) fhandler_windows (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_dsp *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_dev_dsp *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_dsp));
    fhandler_dev_dsp *fh = new (ptr) fhandler_dev_dsp (ptr);
    fh->copy_from (this);
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
  bool diropen;
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

  virtual void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_virtual *> (x);
    _copy_from_reset_helper ();
  }

  virtual fhandler_virtual *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_virtual));
    fhandler_virtual *fh = new (ptr) fhandler_virtual (ptr);
    fh->copy_from (this);
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

  virtual void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_proc *> (x);
    _copy_from_reset_helper ();
  }

  virtual fhandler_proc *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_proc));
    fhandler_proc *fh = new (ptr) fhandler_proc (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_procsys *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_procsys *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_procsys));
    fhandler_procsys *fh = new (ptr) fhandler_procsys (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_procsysvipc *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_procsysvipc *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_procsysvipc));
    fhandler_procsysvipc *fh = new (ptr) fhandler_procsysvipc (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_netdrive *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_netdrive *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_netdrive));
    fhandler_netdrive *fh = new (ptr) fhandler_netdrive (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_registry *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_registry *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_registry));
    fhandler_registry *fh = new (ptr) fhandler_registry (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_process *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_process *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_process));
    fhandler_process *fh = new (ptr) fhandler_process (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_process_fd *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_process_fd *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_process_fd));
    fhandler_process_fd *fh = new (ptr) fhandler_process_fd (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_procnet *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_procnet *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_procnet));
    fhandler_procnet *fh = new (ptr) fhandler_procnet (ptr);
    fh->copy_from (this);
    return fh;
  }
};

class fhandler_dev_fd: public fhandler_virtual
{
 public:
  fhandler_dev_fd ();
  virtual_ftype_t exists();

  int __reg2 fstat (struct stat *buf);
  bool fill_filebuf ();

  fhandler_dev_fd (void *) {}

  virtual void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_dev_fd *> (x);
    _copy_from_reset_helper ();
  }

  virtual fhandler_dev_fd *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_fd));
    fhandler_dev_fd *fh = new (ptr) fhandler_dev_fd (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_signalfd *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_signalfd *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_signalfd));
    fhandler_signalfd *fh = new (ptr) fhandler_signalfd (ptr);
    fh->copy_from (this);
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

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_timerfd *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_timerfd *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_timerfd));
    fhandler_timerfd *fh = new (ptr) fhandler_timerfd (ptr);
    fh->copy_from (this);
    return fh;
  }
};

class fhandler_mqueue: public fhandler_disk_file
{
  struct mq_info mqi;
  /* Duplicate filebuf usage of fhandler_virtual */
  char *filebuf;
  off_t filesize;
  off_t position;

  bool valid_path ();

  struct mq_info *_mqinfo (SIZE_T, mode_t, int, bool);

  struct mq_info *mqinfo_create (struct mq_attr *, mode_t, int);
  struct mq_info *mqinfo_open (int);
  void mq_open_finish (bool, bool);

  int _dup (HANDLE, fhandler_mqueue *);

  bool fill_filebuf ();

  int mutex_lock (HANDLE, bool);
  int mutex_unlock (HANDLE);
  int cond_timedwait (HANDLE, HANDLE, const struct timespec *);
  void cond_signal (HANDLE);

public:
  fhandler_mqueue ();
  fhandler_mqueue (void *) {}
  ~fhandler_mqueue ();

  fhandler_mqueue *is_mqueue () { return this; }

  char *get_proc_fd_name (char *);

  int open (int, mode_t);
  int mq_open (int, mode_t, struct mq_attr *);
  int mq_getattr (struct mq_attr *);
  int mq_setattr (const struct mq_attr *, struct mq_attr *);
  int mq_notify (const struct sigevent *);
  int mq_timedsend (const char *, size_t, unsigned int,
		    const struct timespec *);
  ssize_t mq_timedrecv (char *, size_t, unsigned int *,
			const struct timespec *);

  struct mq_info *mqinfo () { return &mqi; }

  void fixup_after_fork (HANDLE);

#define NO_IMPL		{ set_errno (EPERM); return -1; }

  ssize_t __reg3 fgetxattr (const char *, void *, size_t) NO_IMPL;
  int __reg3 fsetxattr (const char *, const void *, size_t, int) NO_IMPL;
  int __reg3 fadvise (off_t, off_t, int) NO_IMPL;
  int __reg3 ftruncate (off_t, bool) NO_IMPL;
  int __reg2 link (const char *) NO_IMPL;
  int mkdir (mode_t) NO_IMPL;
  ssize_t __reg3 pread (void *, size_t, off_t, void *aio = NULL) NO_IMPL;
  ssize_t __reg3 pwrite (void *, size_t, off_t, void *aio = NULL) NO_IMPL;
  int lock (int, struct flock *) NO_IMPL;
  int mand_lock (int, struct flock *) NO_IMPL;
  void __reg3 read (void *, size_t&);
  off_t lseek (off_t, int);
  int __reg2 fstat (struct stat *);
  int dup (fhandler_base *, int);
  int fcntl (int cmd, intptr_t);
  int ioctl (unsigned int, void *);
  int close ();

  void copy_from (fhandler_base *x)
  {
    pc.free_strings ();
    *this = *reinterpret_cast<fhandler_mqueue *> (x);
    _copy_from_reset_helper ();
  }

  fhandler_mqueue *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_mqueue));
    fhandler_mqueue *fh = new (ptr) fhandler_mqueue (ptr);
    fh->copy_from (this);
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
  char __dev_fd[sizeof (fhandler_dev_fd)];
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
  char __mqueue[sizeof (fhandler_mqueue)];
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
