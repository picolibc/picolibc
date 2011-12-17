/* fhandler.h

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _FHANDLER_H_
#define _FHANDLER_H_

#include "tty.h"
/* fcntl flags used only internaly. */
#define O_NOSYMLINK 0x080000
#define O_DIROPEN   0x100000

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
struct __acl32;

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

enum conn_state
{
  unconnected = 0,
  connect_pending = 1,
  connected = 2,
  connect_failed = 3
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
  virt_blk = -7,	/* Block special */
  virt_chr = -6,	/* Character special */
  virt_fsfile = -5,	/* FS-based file via /proc/sys */
  virt_socket = -4,	/* Socket */
  virt_pipe = -3,	/* Pipe */
  virt_symlink = -2,	/* Symlink */
  virt_file = -1,	/* Regular file */
  virt_none = 0,	/* Invalid, Error */
  virt_directory = 1,	/* Directory */
  virt_rootdir = 2,	/* Root directory of virtual FS */
  virt_fsdir = 3,	/* FS-based directory via /proc/sys */
};

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

   public:
    status_flags () :
      rbinary (0), rbinset (0), wbinary (0), wbinset (0), nohandle (0),
      did_lseek (0), query_open (no_query), close_on_exec (0),
      need_fork_fixup (0), isclosed (0)
      {}
  } status, open_status;

 private:
  ACCESS_MASK access;
  ULONG options;

  HANDLE io_handle;

  __ino64_t ino;	/* file ID or hashed filename, depends on FS. */
  long _refcnt;

 protected:
  /* File open flags from open () and fcntl () calls */
  int openflags;

  char *rabuf;		/* used for crlf conversion in text files */
  size_t ralen;
  size_t raixget;
  size_t raixput;
  size_t rabuflen;

  /* Used for advisory file locking.  See flock.cc.  */
  long long unique_id;
  void del_my_locks (del_lock_called_from);

  HANDLE read_state;

 public:
  long refcnt(long i = 0) {return _refcnt += i;}
  class fhandler_base *archetype;
  int usecount;

  path_conv pc;

  void reset (const fhandler_base *);
  virtual bool use_archetype () const {return false;}
  virtual void set_name (path_conv &pc);
  virtual void set_name (const char *s)
  {
    pc.set_normalized_path (s);
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
  void set_io_handle (HANDLE x) { io_handle = x; }

  DWORD& get_device () { return dev (); }
  DWORD get_major () { return dev ().get_major (); }
  DWORD get_minor () { return dev ().get_minor (); }
  virtual int get_unit () { return dev ().get_minor (); }

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

  int eat_readahead (int n);

  void set_readahead_valid (int val, int ch = -1);

  int get_readahead_into_buffer (char *buf, size_t buflen);

  bool has_acls () const { return pc.has_acls (); }

  bool isremote () { return pc.isremote (); }

  bool has_attribute (DWORD x) const {return pc.has_attribute (x);}
  const char *get_name () const { return pc.normalized_path; }
  const char *get_win32_name () { return pc.get_win32 (); }
  __dev32_t get_dev () { return pc.fs_serial_number (); }
  __ino64_t get_ino () { return ino ?: ino = hash_path_name (0, pc.get_nt_native_path ()); }
  long long get_unique_id () const { return unique_id; }
  /* Returns name used for /proc/<pid>/fd in buf. */
  virtual char *get_proc_fd_name (char *buf);

  virtual void hclose (HANDLE h) {CloseHandle (h);}
  virtual void set_no_inheritance (HANDLE &, bool);

  /* fixup fd possibly non-inherited handles after fork */
  bool fork_fixup (HANDLE, HANDLE &, const char *);
  virtual bool need_fixup_before () const {return false;}

  int open_with_arch (int, mode_t = 0);
  virtual int open (int, mode_t);
  virtual void open_setup (int flags) { return; }

  int close_with_arch ();
  virtual int close ();
  virtual void cleanup () { return; }
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
  virtual int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
  int __stdcall fstat_fs (struct __stat64 *buf) __attribute__ ((regparm (2)));
private:
  int __stdcall fstat_helper (struct __stat64 *buf,
			      DWORD nNumberOfLinks)
		__attribute__ ((regparm (3)));
  int __stdcall fstat_by_nfs_ea (struct __stat64 *buf) __attribute__ ((regparm (2)));
  int __stdcall fstat_by_handle (struct __stat64 *buf) __attribute__ ((regparm (2)));
  int __stdcall fstat_by_name (struct __stat64 *buf) __attribute__ ((regparm (2)));
public:
  virtual int __stdcall fstatvfs (struct statvfs *buf) __attribute__ ((regparm (2)));
  int utimens_fs (const struct timespec *) __attribute__ ((regparm (2)));
  virtual int __stdcall fchmod (mode_t mode) __attribute__ ((regparm (1)));
  virtual int __stdcall fchown (__uid32_t uid, __gid32_t gid) __attribute__ ((regparm (2)));
  virtual int __stdcall facl (int, int, __acl32 *) __attribute__ ((regparm (3)));
  virtual ssize_t __stdcall fgetxattr (const char *, void *, size_t) __attribute__ ((regparm (3)));
  virtual int __stdcall fsetxattr (const char *, const void *, size_t, int) __attribute__ ((regparm (3)));
  virtual int __stdcall fadvise (_off64_t, _off64_t, int) __attribute__ ((regparm (3)));
  virtual int __stdcall ftruncate (_off64_t, bool) __attribute__ ((regparm (3)));
  virtual int __stdcall link (const char *) __attribute__ ((regparm (2)));
  virtual int __stdcall utimens (const struct timespec *) __attribute__ ((regparm (2)));
  virtual int __stdcall fsync () __attribute__ ((regparm (1)));
  virtual int ioctl (unsigned int cmd, void *);
  virtual int fcntl (int cmd, void *);
  virtual char const *ttyname () { return get_name (); }
  virtual void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  virtual ssize_t __stdcall write (const void *ptr, size_t len);
  virtual ssize_t __stdcall readv (const struct iovec *, int iovcnt, ssize_t tot = -1);
  virtual ssize_t __stdcall writev (const struct iovec *, int iovcnt, ssize_t tot = -1);
  virtual ssize_t __stdcall pread (void *, size_t, _off64_t) __attribute__ ((regparm (3)));
  virtual ssize_t __stdcall pwrite (void *, size_t, _off64_t) __attribute__ ((regparm (3)));
  virtual _off64_t lseek (_off64_t offset, int whence);
  virtual int lock (int, struct __flock64 *);
  virtual int dup (fhandler_base *child, int flags);
  virtual int fpathconf (int);

  virtual HANDLE mmap (caddr_t *addr, size_t len, int prot,
		       int flags, _off64_t off);
  virtual int munmap (HANDLE h, caddr_t addr, size_t len);
  virtual int msync (HANDLE h, caddr_t addr, size_t len, int flags);
  virtual bool fixup_mmap_after_fork (HANDLE h, int prot, int flags,
				      _off64_t offset, DWORD size,
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
  virtual int tcgetsid ();
  virtual bool is_tty () const { return false; }
  virtual bool ispipe () const { return false; }
  virtual pid_t get_popen_pid () const {return 0;}
  virtual bool isdevice () const { return true; }
  virtual bool isfifo () const { return false; }
  virtual int ptsname_r (char *, size_t);
  virtual class fhandler_socket *is_socket () { return NULL; }
  virtual class fhandler_console *is_console () { return 0; }
  virtual int is_windows () {return 0; }

  virtual void __stdcall raw_read (void *ptr, size_t& ulen) __attribute__ ((regparm (3)));
  virtual ssize_t __stdcall raw_write (const void *ptr, size_t ulen) __attribute__ ((regparm (3)));

  /* Virtual accessor functions to hide the fact
     that some fd's have two handles. */
  virtual HANDLE& get_handle () { return io_handle; }
  virtual HANDLE& get_io_handle () { return io_handle; }
  virtual HANDLE& get_output_handle () { return io_handle; }
  virtual HANDLE get_stat_handle () { return pc.handle () ?: io_handle; }
  virtual bool hit_eof () {return false;}
  virtual select_record *select_read (select_stuff *);
  virtual select_record *select_write (select_stuff *);
  virtual select_record *select_except (select_stuff *);
  virtual const char *get_native_name ()
  {
    return dev ().native;
  }
  virtual bg_check_types bg_check (int) {return bg_ok;}
  void clear_readahead ()
  {
    raixput = raixget = ralen = rabuflen = 0;
    rabuf = NULL;
  }
  void operator delete (void *p) {cfree (p);}
  virtual void set_eof () {}
  virtual int mkdir (mode_t mode);
  virtual int rmdir ();
  virtual DIR *opendir (int fd) __attribute__ ((regparm (2)));
  virtual int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  virtual long telldir (DIR *);
  virtual void seekdir (DIR *, long);
  virtual void rewinddir (DIR *);
  virtual int closedir (DIR *);
  bool is_auto_device () {return isdevice () && !dev ().isfs ();}
  bool is_fs_special () {return pc.is_fs_special ();}
  bool issymlink () {return pc.issymlink ();}
  bool device_access_denied (int) __attribute__ ((regparm (2)));
  int fhaccess (int flags, bool) __attribute__ ((regparm (3)));
  virtual bool __stdcall has_ongoing_io () __attribute__ ((regparm (1))) {return false;}

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
  int addr_family;
  int type;
  int connect_secret[4];

  wsa_event *wsock_events;
  HANDLE wsock_mtx;
  HANDLE wsock_evt;
 public:
  bool init_events ();
  int evaluate_events (const long event_mask, long &events, const bool erase);
  const HANDLE wsock_event () const { return wsock_evt; }
  const LONG serial_number () const { return wsock_events->serial_number; }
 private:
  int wait_for_events (const long event_mask, const DWORD flags);
  void release_events ();

  pid_t     sec_pid;
  __uid32_t sec_uid;
  __gid32_t sec_gid;
  pid_t     sec_peer_pid;
  __uid32_t sec_peer_uid;
  __gid32_t sec_peer_gid;
  void af_local_set_secret (char *);
  void af_local_setblocking (bool &, bool &);
  void af_local_unsetblocking (bool, bool);
  void af_local_set_cred ();
  void af_local_copy (fhandler_socket *);
  bool af_local_recv_secret ();
  bool af_local_send_secret ();
  bool af_local_recv_cred ();
  bool af_local_send_cred ();
  int af_local_accept ();
 public:
  int af_local_connect ();
  void af_local_set_sockpair_cred ();

 private:
  int	    _rmem;
  int	    _wmem;
 public:
  int &rmem () { return _rmem; }
  int &wmem () { return _wmem; }
  void rmem (int nrmem) { _rmem = nrmem; }
  void wmem (int nwmem) { _wmem = nwmem; }

 private:
  struct _WSAPROTOCOL_INFOW *prot_info_ptr;
 public:
  void init_fixup_before ();
  bool need_fixup_before () const {return prot_info_ptr != NULL;}

 private:
  char *sun_path;
  char *peer_sun_path;
  struct status_flags
  {
    unsigned async_io		   : 1; /* async I/O */
    unsigned saw_shutdown_read     : 1; /* Socket saw a SHUT_RD */
    unsigned saw_shutdown_write    : 1; /* Socket saw a SHUT_WR */
    unsigned saw_reuseaddr	   : 1; /* Socket saw SO_REUSEADDR call */
    unsigned listener		   : 1; /* listen called */
    unsigned connect_state	   : 2;
   public:
    status_flags () :
      async_io (0), saw_shutdown_read (0), saw_shutdown_write (0),
      listener (0), connect_state (unconnected)
      {}
  } status;

 public:
  fhandler_socket ();
  ~fhandler_socket ();
  int get_socket () { return (int) get_handle(); }
  fhandler_socket *is_socket () { return this; }

  IMPLEMENT_STATUS_FLAG (bool, async_io)
  IMPLEMENT_STATUS_FLAG (bool, saw_shutdown_read)
  IMPLEMENT_STATUS_FLAG (bool, saw_shutdown_write)
  IMPLEMENT_STATUS_FLAG (bool, saw_reuseaddr)
  IMPLEMENT_STATUS_FLAG (bool, listener)
  IMPLEMENT_STATUS_FLAG (conn_state, connect_state)

  int bind (const struct sockaddr *name, int namelen);
  int connect (const struct sockaddr *name, int namelen);
  int listen (int backlog);
  int accept4 (struct sockaddr *peer, int *len, int flags);
  int getsockname (struct sockaddr *name, int *namelen);
  int getpeername (struct sockaddr *name, int *namelen);
  int getpeereid (pid_t *pid, __uid32_t *euid, __gid32_t *egid);

  int open (int flags, mode_t mode = 0);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  ssize_t __stdcall readv (const struct iovec *, int iovcnt, ssize_t tot = -1);
  inline ssize_t recv_internal (struct _WSAMSG *wsamsg);
  ssize_t recvfrom (void *ptr, size_t len, int flags,
		    struct sockaddr *from, int *fromlen);
  ssize_t recvmsg (struct msghdr *msg, int flags);

  ssize_t __stdcall write (const void *ptr, size_t len);
  ssize_t __stdcall writev (const struct iovec *, int iovcnt, ssize_t tot = -1);
  inline ssize_t send_internal (struct _WSAMSG *wsamsg, int flags);
  ssize_t sendto (const void *ptr, size_t len, int flags,
	      const struct sockaddr *to, int tolen);
  ssize_t sendmsg (const struct msghdr *msg, int flags);

  int ioctl (unsigned int cmd, void *);
  int fcntl (int cmd, void *);
  _off64_t lseek (_off64_t, int) { return 0; }
  int shutdown (int how);
  int close ();
  void hclose (HANDLE) {close ();}
  int dup (fhandler_base *child, int);

  void set_close_on_exec (bool val);
  int fixup_before_fork_exec (DWORD);
  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();
  char *get_proc_fd_name (char *buf);

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
  void set_addr_family (int af) {addr_family = af;}
  int get_addr_family () {return addr_family;}
  void set_socket_type (int st) { type = st;}
  int get_socket_type () {return type;}
  void set_sun_path (const char *path);
  char *get_sun_path () {return sun_path;}
  void set_peer_sun_path (const char *path);
  char *get_peer_sun_path () {return peer_sun_path;}

  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
  int __stdcall fstatvfs (struct statvfs *buf) __attribute__ ((regparm (2)));
  int __stdcall fchmod (mode_t mode) __attribute__ ((regparm (1)));
  int __stdcall fchown (__uid32_t uid, __gid32_t gid) __attribute__ ((regparm (2)));
  int __stdcall facl (int, int, __acl32 *) __attribute__ ((regparm (3)));
  int __stdcall link (const char *) __attribute__ ((regparm (2)));

  fhandler_socket (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_socket *> (x) = *this;
    x->reset (this);
  }

  fhandler_socket *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_socket));
    fhandler_socket *fh = new (ptr) fhandler_socket (ptr);
    copyto (fh);
    return fh;
  }
};

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
    overlapped_error
  };
  bool io_pending;
  OVERLAPPED io_status;
  OVERLAPPED *overlapped;
  size_t max_atomic_write;
public:
  wait_return __stdcall wait_overlapped (bool, bool, DWORD *, bool, DWORD = 0) __attribute__ ((regparm (3)));
  int __stdcall setup_overlapped () __attribute__ ((regparm (1)));
  void __stdcall destroy_overlapped () __attribute__ ((regparm (1)));
  virtual void __stdcall raw_read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  virtual ssize_t __stdcall raw_write (const void *ptr, size_t len) __attribute__ ((regparm (3)));
  OVERLAPPED *&get_overlapped () {return overlapped;}
  OVERLAPPED *get_overlapped_buffer () {return &io_status;}
  void set_overlapped (OVERLAPPED *ov) {overlapped = ov;}
  fhandler_base_overlapped (): io_pending (false), overlapped (NULL), max_atomic_write (0)
  {
    memset (&io_status, 0, sizeof io_status);
  }
  bool __stdcall has_ongoing_io () __attribute__ ((regparm (1)));

  void fixup_after_fork (HANDLE);
  void fixup_after_exec ();

  int close ();
  int dup (fhandler_base *child, int);

  void check_later ();
  static void flush_all_async_io () __attribute__ ((regparm (1)));;

  fhandler_base_overlapped (void *) {}

  virtual void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_base_overlapped *> (x) = *this;
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
  _off64_t lseek (_off64_t offset, int whence);
  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
  char *get_proc_fd_name (char *buf);
  int open (int flags, mode_t mode = 0);
  int dup (fhandler_base *child, int);
  int ioctl (unsigned int cmd, void *);
  int __stdcall fstatvfs (struct statvfs *buf) __attribute__ ((regparm (2)));
  int __stdcall fadvise (_off64_t, _off64_t, int) __attribute__ ((regparm (3)));
  int __stdcall ftruncate (_off64_t, bool) __attribute__ ((regparm (3)));
  int init (HANDLE, DWORD, mode_t);
  static int create (fhandler_pipe *[2], unsigned, int);
  static DWORD create (LPSECURITY_ATTRIBUTES, HANDLE *, HANDLE *, DWORD,
		       const char *, DWORD);
  fhandler_pipe (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_pipe *> (x) = *this;
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

class fhandler_fifo: public fhandler_base_overlapped
{
  HANDLE read_ready;
  HANDLE write_ready;
  bool wait (HANDLE) __attribute__ ((regparm (2)));
  char *fifo_name (char *, const char *) __attribute__ ((regparm (2)));
public:
  fhandler_fifo ();
  int open (int, mode_t);
  int close ();
  int dup (fhandler_base *child, int);
  bool isfifo () const { return true; }
  void set_close_on_exec (bool val);
  void __stdcall raw_read (void *ptr, size_t& ulen) __attribute__ ((regparm (3)));
  void fixup_after_fork (HANDLE);
  int __stdcall fstatvfs (struct statvfs *buf) __attribute__ ((regparm (2)));
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
    fhandler_fifo *fh = new (ptr) fhandler_fifo (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_mailslot : public fhandler_base_overlapped
{
  POBJECT_ATTRIBUTES get_object_attr (OBJECT_ATTRIBUTES &, PUNICODE_STRING, int);
 public:
  fhandler_mailslot ();
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall raw_write (const void *, size_t) __attribute__ ((regparm (3)));
  int ioctl (unsigned int cmd, void *);
  select_record *select_read (select_stuff *);

  fhandler_mailslot (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_mailslot *> (x) = *this;
    x->reset (this);
  }

  fhandler_mailslot *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_mailslot));
    fhandler_mailslot *fh = new (ptr) fhandler_mailslot (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_dev_raw: public fhandler_base
{
 protected:
  char *devbuf;
  size_t devbufsiz;
  size_t devbufstart;
  size_t devbufend;
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

  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));

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
  _off64_t drive_size;
  unsigned long bytes_per_sector;
  part_t *partitions;
  struct status_flags
  {
    unsigned eom_detected    : 1;
   public:
    status_flags () : eom_detected (0) {}
  } status;

  IMPLEMENT_STATUS_FLAG (bool, eom_detected)

  inline _off64_t get_current_position ();
  int get_drive_info (struct hd_geometry *geo);

  int lock_partition (DWORD to_write);

  BOOL write_file (const void *buf, DWORD to_write, DWORD *written, int *err);
  BOOL read_file (void *buf, DWORD to_read, DWORD *read, int *err);

 public:
  fhandler_dev_floppy ();

  int open (int flags, mode_t mode = 0);
  int close ();
  int dup (fhandler_base *child, int);
  void __stdcall raw_read (void *ptr, size_t& ulen) __attribute__ ((regparm (3)));
  ssize_t __stdcall raw_write (const void *ptr, size_t ulen) __attribute__ ((regparm (3)));
  _off64_t lseek (_off64_t offset, int whence);
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
  HANDLE mt_evt;

  bool is_rewind_device () { return get_minor () < 128; }
  unsigned int driveno () { return (unsigned int) get_minor () & 0x7f; }
  void drive_init ();

  inline bool _lock (bool);
  inline int unlock (int ret = 0);

 public:
  fhandler_dev_tape ();

  int open (int flags, mode_t mode = 0);
  virtual int close ();

  void __stdcall raw_read (void *ptr, size_t& ulen) __attribute__ ((regparm (3)));
  ssize_t __stdcall raw_write (const void *ptr, size_t ulen) __attribute__ ((regparm (3)));

  virtual _off64_t lseek (_off64_t offset, int whence);

  virtual int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));

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
  int readdir_helper (DIR *, dirent *, DWORD, DWORD, PUNICODE_STRING fname) __attribute__ ((regparm (3)));

  int prw_open (bool);

 public:
  fhandler_disk_file ();
  fhandler_disk_file (path_conv &pc);

  int open (int flags, mode_t mode);
  int close ();
  int dup (fhandler_base *child, int);
  void fixup_after_fork (HANDLE parent);
  int lock (int, struct __flock64 *);
  bool isdevice () const { return false; }
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
  int __stdcall fchmod (mode_t mode) __attribute__ ((regparm (1)));
  int __stdcall fchown (__uid32_t uid, __gid32_t gid) __attribute__ ((regparm (2)));
  int __stdcall facl (int, int, __acl32 *) __attribute__ ((regparm (3)));
  ssize_t __stdcall fgetxattr (const char *, void *, size_t) __attribute__ ((regparm (3)));
  int __stdcall fsetxattr (const char *, const void *, size_t, int) __attribute__ ((regparm (3)));
  int __stdcall fadvise (_off64_t, _off64_t, int) __attribute__ ((regparm (3)));
  int __stdcall ftruncate (_off64_t, bool) __attribute__ ((regparm (3)));
  int __stdcall link (const char *) __attribute__ ((regparm (2)));
  int __stdcall utimens (const struct timespec *) __attribute__ ((regparm (2)));
  int __stdcall fstatvfs (struct statvfs *buf) __attribute__ ((regparm (2)));

  HANDLE mmap (caddr_t *addr, size_t len, int prot, int flags, _off64_t off);
  int munmap (HANDLE h, caddr_t addr, size_t len);
  int msync (HANDLE h, caddr_t addr, size_t len, int flags);
  bool fixup_mmap_after_fork (HANDLE h, int prot, int flags,
			      _off64_t offset, DWORD size, void *address);
  int mkdir (mode_t mode);
  int rmdir ();
  DIR *opendir (int fd) __attribute__ ((regparm (2)));
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  long telldir (DIR *);
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);

  ssize_t __stdcall pread (void *, size_t, _off64_t) __attribute__ ((regparm (3)));
  ssize_t __stdcall pwrite (void *, size_t, _off64_t) __attribute__ ((regparm (3)));

  fhandler_disk_file (void *) {}

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

class fhandler_cygdrive: public fhandler_disk_file
{
  enum
  {
    DRVSZ = sizeof ("x:\\")
  };
  int ndrives;
  const char *pdrive;
  char pdrive_buf[1 + (2 * 26 * DRVSZ)];
  void set_drives ();
 public:
  fhandler_cygdrive ();
  int open (int flags, mode_t mode);
  int close ();
  DIR *opendir (int fd) __attribute__ ((regparm (2)));
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  void rewinddir (DIR *);
  int closedir (DIR *);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));

  fhandler_cygdrive (void *) {}

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
  void __stdcall raw_read (void *ptr, size_t& ulen) __attribute__ ((regparm (3)));
  ssize_t __stdcall raw_write (const void *ptr, size_t ulen) __attribute__ ((regparm (3)));
  int tcsendbreak (int);
  int tcdrain ();
  int tcflow (int);
  int ioctl (unsigned int cmd, void *);
  int switch_modem_lines (int set, int clr);
  int tcsetattr (int a, const struct termios *t);
  int tcgetattr (struct termios *t);
  _off64_t lseek (_off64_t, int) { return 0; }
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
 public:
  tty_min*& tc () {return _tc;}
  fhandler_termios () :
  fhandler_base ()
  {
    need_fork_fixup (true);
  }
  HANDLE& get_output_handle () { return output_handle; }
  line_edit_status line_edit (const char *rptr, int nread, termios&);
  void set_output_handle (HANDLE h) { output_handle = h; }
  void tcinit (bool force);
  bool is_tty () const { return true; }
  void sigflush ();
  int tcgetpgrp ();
  int tcsetpgrp (int pid);
  bg_check_types bg_check (int sig);
  virtual DWORD __acquire_output_mutex (const char *fn, int ln, DWORD ms) {return 1;}
  virtual void __release_output_mutex (const char *fn, int ln) {}
  void echo_erase (int force = 0);
  virtual _off64_t lseek (_off64_t, int);
  int tcgetsid ();

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
#define MAXARGS 10

class dev_console
{
  WORD default_color, underline_color, dim_color;

  /* Used to determine if an input keystroke should be modified with META. */
  int meta_mask;

/* Output state */
  int state_;
  int args_[MAXARGS];
  int nargs_;
  unsigned rarg;
  bool saw_question_mark;
  bool saw_greater_than_sign;
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

  /* saved screen */
  COORD savebufsiz;
  PCHAR_INFO savebuf;

  struct
    {
      short Top, Bottom;
    } scroll_region;
  struct
    {
      SHORT winTop;
      SHORT winBottom;
      COORD dwWinSize;
      COORD dwBufferSize;
      COORD dwCursorPosition;
      WORD wAttributes;
    } info;

  COORD dwLastCursorPosition;
  COORD dwMousePosition;	/* scroll-adjusted coord of mouse event */
  COORD dwLastMousePosition;	/* scroll-adjusted coord of previous mouse event */
  DWORD dwLastButtonState;	/* (not noting mouse wheel events) */
  int last_button_code;		/* transformed mouse report button code */
  int nModifiers;

  bool insert_mode;
  int use_mouse;
  bool use_focus;
  bool raw_win32_keyboard_mode;

  inline UINT get_console_cp ();
  DWORD con_to_str (char *d, int dlen, WCHAR w);
  DWORD str_to_con (mbtowc_p, const char *, PWCHAR d, const char *s, DWORD sz);
  void set_color (HANDLE);
  bool fillin_info (HANDLE);
  void set_default_attr ();

  friend class fhandler_console;
};

/* This is a input and output console handle */
class fhandler_console: public fhandler_termios
{
public:
  struct console_state
  {
    tty_min tty_min_state;
    dev_console dev_state;
  };
private:
  static const unsigned MAX_WRITE_CHARS;
  static console_state *shared_console_info;
  static bool invisible_console;

  /* Used when we encounter a truncated multi-byte sequence.  The
     lead bytes are stored here and revisited in the next write call. */
  struct {
    int len;
    unsigned char buf[4]; /* Max len of valid UTF-8 sequence. */
  } trunc_buf;
  PWCHAR write_buf;

/* Output calls */
  void set_default_attr ();

  void clear_screen (int, int, int, int);
  void scroll_screen (int, int, int, int, int, int);
  void cursor_set (bool, int, int);
  void cursor_get (int *, int *);
  void cursor_rel (int, int);
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
  static pid_t tc_getpgid () {return shared_console_info->tty_min_state.getpgid ();}
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

  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  ssize_t __stdcall write (const void *ptr, size_t len);
  void doecho (const void *str, DWORD len) { (void) write (str, len); }
  int close ();

  int tcflush (int);
  int tcsetattr (int a, const struct termios *t);
  int tcgetattr (struct termios *t);

  int ioctl (unsigned int cmd, void *);
  int init (HANDLE, DWORD, mode_t);
  bool mouse_aware (MOUSE_EVENT_RECORD& mouse_event);
  bool focus_aware () {return shared_console_info->dev_state.use_focus;}

  select_record *select_read (select_stuff *);
  select_record *select_write (select_stuff *);
  select_record *select_except (select_stuff *);
  void fixup_after_fork_exec (bool);
  void fixup_after_exec () {fixup_after_fork_exec (true);}
  void fixup_after_fork (HANDLE) {fixup_after_fork_exec (false);}
  void set_close_on_exec (bool val);
  void set_input_state ();
  void send_winch_maybe ();
  void setup ();
  bool set_unit ();
  static bool need_invisible ();
  static bool has_a () {return !invisible_console;}

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
  friend tty_min * tty_list::get_cttyp ();
};

class fhandler_pty_common: public fhandler_termios
{
 public:
  fhandler_pty_common ()
    : fhandler_termios (),
      output_mutex (NULL),
    input_mutex (NULL), input_available_event (NULL)
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
  _off64_t lseek (_off64_t, int);
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
};

class fhandler_pty_slave: public fhandler_pty_common
{
  HANDLE inuse;			// used to indicate that a tty is in use

  /* Helper functions for fchmod and fchown. */
  bool fch_open_handles ();
  int fch_set_sd (security_descriptor &sd, bool chown);
  void fch_close_handles ();

 public:
  /* Constructor */
  fhandler_pty_slave (int);

  int open (int flags, mode_t mode = 0);
  void open_setup (int flags);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
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
  int get_unit ();
  virtual char const *ttyname () { return pc.dev.name; }
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
  int __stdcall fchmod (mode_t mode) __attribute__ ((regparm (1)));
  int __stdcall fchown (__uid32_t uid, __gid32_t gid) __attribute__ ((regparm (2)));

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
};

#define __ptsname(buf, unit) __small_sprintf ((buf), "/dev/pty%d", (unit))
class fhandler_pty_master: public fhandler_pty_common
{
  int pktmode;			// non-zero if pty in a packet mode.
  HANDLE master_ctl;		// Control socket for handle duplication
  cygthread *master_thread;	// Master control thread
  HANDLE from_master, to_master;
  DWORD dwProcessId;		// Owner of master handles

public:
  int need_nl;			// Next read should start with \n

  /* Constructor */
  fhandler_pty_master (int);

  DWORD pty_master_thread ();
  int process_slave_output (char *buf, size_t len, int pktmode_on);
  void doecho (const void *str, DWORD len);
  int accept_input ();
  int open (int flags, mode_t mode = 0);
  void open_setup (int flags);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
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

  fhandler_pty_master (void *) {}
  ~fhandler_pty_master ();

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
};

class fhandler_dev_zero: public fhandler_base
{
 public:
  fhandler_dev_zero ();
  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  _off64_t lseek (_off64_t offset, int whence);

  virtual HANDLE mmap (caddr_t *addr, size_t len, int prot,
		       int flags, _off64_t off);
  virtual int munmap (HANDLE h, caddr_t addr, size_t len);
  virtual int msync (HANDLE h, caddr_t addr, size_t len, int flags);
  virtual bool fixup_mmap_after_fork (HANDLE h, int prot, int flags,
				      _off64_t offset, DWORD size,
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
  HCRYPTPROV crypt_prov;
  long pseudo;
  _off64_t dummy_offset;

  bool crypt_gen_random (void *ptr, size_t len);
  int pseudo_write (const void *ptr, size_t len);
  int pseudo_read (void *ptr, size_t len);

 public:
  fhandler_dev_random ();
  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  _off64_t lseek (_off64_t offset, int whence);
  int close ();
  int dup (fhandler_base *child, int);

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

class fhandler_dev_mem: public fhandler_base
{
 protected:
  DWORD mem_size;
  _off64_t pos;

 public:
  fhandler_dev_mem ();
  ~fhandler_dev_mem ();

  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall write (const void *ptr, size_t ulen);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  _off64_t lseek (_off64_t offset, int whence);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));

  HANDLE mmap (caddr_t *addr, size_t len, int prot, int flags, _off64_t off);
  int munmap (HANDLE h, caddr_t addr, size_t len);
  int msync (HANDLE h, caddr_t addr, size_t len, int flags);
  bool fixup_mmap_after_fork (HANDLE h, int prot, int flags,
			      _off64_t offset, DWORD size, void *address);

  fhandler_dev_mem (void *) {}

  void copyto (fhandler_base *x)
  {
    x->pc.free_strings ();
    *reinterpret_cast<fhandler_dev_mem *> (x) = *this;
    x->reset (this);
  }

  fhandler_dev_mem *clone (cygheap_types malloc_type = HEAP_FHANDLER)
  {
    void *ptr = (void *) ccalloc (malloc_type, 1, sizeof (fhandler_dev_mem));
    fhandler_dev_mem *fh = new (ptr) fhandler_dev_mem (ptr);
    copyto (fh);
    return fh;
  }
};

class fhandler_dev_clipboard: public fhandler_base
{
  _off64_t pos;
  void *membuffer;
  size_t msize;
  bool eof;
 public:
  fhandler_dev_clipboard ();
  int is_windows () { return 1; }
  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  _off64_t lseek (_off64_t offset, int whence);
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
  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  int ioctl (unsigned int cmd, void *);
  _off64_t lseek (_off64_t, int) { return 0; }
  int close () { return 0; }

  void set_close_on_exec (bool val);
  void fixup_after_fork (HANDLE parent);
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

  int open (int flags, mode_t mode = 0);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  int ioctl (unsigned int cmd, void *);
  _off64_t lseek (_off64_t, int);
  int close ();
  void fixup_after_fork (HANDLE parent);
  void fixup_after_exec ();
 private:
  void close_audio_in ();
  void close_audio_out (bool immediately = false);
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
  _off64_t filesize;
  _off64_t position;
  int fileid; // unique within each class
 public:

  fhandler_virtual ();
  virtual ~fhandler_virtual();

  virtual virtual_ftype_t exists();
  DIR *opendir (int fd) __attribute__ ((regparm (2)));
  long telldir (DIR *);
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);
  ssize_t __stdcall write (const void *ptr, size_t len);
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  _off64_t lseek (_off64_t, int);
  int dup (fhandler_base *child, int);
  int open (int flags, mode_t mode = 0);
  int close ();
  int __stdcall fstat (struct stat *buf) __attribute__ ((regparm (2)));
  int __stdcall fstatvfs (struct statvfs *buf) __attribute__ ((regparm (2)));
  int __stdcall fchmod (mode_t mode) __attribute__ ((regparm (1)));
  int __stdcall fchown (__uid32_t uid, __gid32_t gid) __attribute__ ((regparm (2)));
  int __stdcall facl (int, int, __acl32 *) __attribute__ ((regparm (3)));
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
  DIR *opendir (int fd) __attribute__ ((regparm (2)));
  int closedir (DIR *);
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  static fh_devices get_proc_fhandler (const char *path);

  int open (int flags, mode_t mode = 0);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
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
  virtual_ftype_t exists(struct __stat64 *buf) __attribute__ ((regparm (2)));
  virtual_ftype_t exists();
  DIR *opendir (int fd) __attribute__ ((regparm (2)));
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  long telldir (DIR *);
  void seekdir (DIR *, long);
  int closedir (DIR *);
  int open (int flags, mode_t mode = 0);
  int close ();
  void __stdcall read (void *ptr, size_t& len) __attribute__ ((regparm (3)));
  ssize_t __stdcall write (const void *ptr, size_t len);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
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
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  int open (int flags, mode_t mode = 0);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
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
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);
  int open (int flags, mode_t mode = 0);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));

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
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  long telldir (DIR *);
  void seekdir (DIR *, long);
  void rewinddir (DIR *);
  int closedir (DIR *);

  int open (int flags, mode_t mode = 0);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
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
  pid_t pid;
 public:
  fhandler_process ();
  virtual_ftype_t exists();
  DIR *opendir (int fd) __attribute__ ((regparm (2)));
  int closedir (DIR *);
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  int open (int flags, mode_t mode = 0);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
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

class fhandler_procnet: public fhandler_proc
{
  pid_t pid;
 public:
  fhandler_procnet ();
  virtual_ftype_t exists();
  int readdir (DIR *, dirent *) __attribute__ ((regparm (3)));
  int open (int flags, mode_t mode = 0);
  int __stdcall fstat (struct __stat64 *buf) __attribute__ ((regparm (2)));
  bool fill_filebuf ();

  fhandler_procnet (void *) {}

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
  char __cygdrive[sizeof (fhandler_cygdrive)];
  char __dev_clipboard[sizeof (fhandler_dev_clipboard)];
  char __dev_dsp[sizeof (fhandler_dev_dsp)];
  char __dev_floppy[sizeof (fhandler_dev_floppy)];
  char __dev_mem[sizeof (fhandler_dev_mem)];
  char __dev_null[sizeof (fhandler_dev_null)];
  char __dev_random[sizeof (fhandler_dev_random)];
  char __dev_raw[sizeof (fhandler_dev_raw)];
  char __dev_tape[sizeof (fhandler_dev_tape)];
  char __dev_zero[sizeof (fhandler_dev_zero)];
  char __disk_file[sizeof (fhandler_disk_file)];
  char __fifo[sizeof (fhandler_fifo)];
  char __mailslot[sizeof (fhandler_mailslot)];
  char __netdrive[sizeof (fhandler_netdrive)];
  char __nodevice[sizeof (fhandler_nodevice)];
  char __pipe[sizeof (fhandler_pipe)];
  char __proc[sizeof (fhandler_proc)];
  char __process[sizeof (fhandler_process)];
  char __procnet[sizeof (fhandler_procnet)];
  char __procsys[sizeof (fhandler_procsys)];
  char __procsysvipc[sizeof (fhandler_procsysvipc)];
  char __pty_master[sizeof (fhandler_pty_master)];
  char __registry[sizeof (fhandler_registry)];
  char __serial[sizeof (fhandler_serial)];
  char __socket[sizeof (fhandler_socket)];
  char __termios[sizeof (fhandler_termios)];
  char __pty_common[sizeof (fhandler_pty_common)];
  char __pty_slave[sizeof (fhandler_pty_slave)];
  char __virtual[sizeof (fhandler_virtual)];
  char __windows[sizeof (fhandler_windows)];
} fhandler_union;
#endif /* _FHANDLER_H_ */
