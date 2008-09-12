/* version.h -- Cygwin version numbers and accompanying documentation.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Cygwin versioning is relatively complicated because of its status
   as a shared library.  Let's start with how versioning used to be done.

   Historical versioning in Cygwin 16.0 to 19.5:

   In the olden days of Cygwin, we had a dll major and minor version
   and a registry version.  The major number started at 16 because the
   "b15" GNU-Win32 release of the compiler tools was out when this
   scheme was started.  We incremented the DLL name frequently (for
   every official release) and towards the end of this period every
   release used a different shared memory area to prevent DLLs from
   interfering with each other (embedding a build timestamp into the
   name of the shared memory area).  This turned out to be a Bad Idea
   (tm) because people needed to mingle separate releases and have
   them work together more than we thought they would.  This was
   especially problematic when tty info needed to be retained when an
   old Cygwin executable executed a newer one.

   In the old scheme, we incremented the major number whenever a
   change to the dll invalidated existing executables.  This can
   happen for a number of reasons, including when functions are
   removed from the export list of the dll.  The minor number was
   incremented when a change was made that we wanted to record, but
   that didn't invalidate existing executables.  Both numbers were
   recorded in the executable and in the dll.

   In October 1998 (starting with Cygwin 19.6), we started a new method
   of Cygwin versioning: */

      /* The DLL major and minor numbers correspond to the "version of
	 the Cygwin shared library".  This version is used to track important
	 changes to the DLL and is mainly informative in nature. */

#define CYGWIN_VERSION_DLL_MAJOR 1007
#define CYGWIN_VERSION_DLL_MINOR 0

      /* Major numbers before CYGWIN_VERSION_DLL_EPOCH are
	 incompatible. */

#define CYGWIN_VERSION_DLL_EPOCH 19

      /* CYGWIN_VERSION_DLL_COMBINED gives us a single number
	 representing the combined DLL major and minor numbers. */

      /* WATCH OUT FOR OCTAL!  Don't use, say, "00020" for 0.20 */

#define CYGWIN_VERSION_DLL_MAKE_COMBINED(maj, min) (((maj) * 1000) + min)
#define CYGWIN_VERSION_DLL_COMBINED \
  CYGWIN_VERSION_DLL_MAKE_COMBINED (CYGWIN_VERSION_DLL_MAJOR, CYGWIN_VERSION_DLL_MINOR)

     /* Every version of cygwin <= this uses an old, incorrect method
	to determine signal masks. */

#define CYGWIN_VERSION_USER_API_VERSION_COMBINED \
  CYGWIN_VERSION_DLL_MAKE_COMBINED (user_data->api_major, user_data->api_minor)

    /* API versions <= this had a termios structure whose members were
       too small to accomodate modern settings. */
#define CYGWIN_VERSION_DLL_OLD_TERMIOS		5
#define CYGWIN_VERSION_DLL_IS_OLD_TERMIOS \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED <= CYGWIN_VERSION_DLL_OLD_TERMIOS)

#define CYGWIN_VERSION_DLL_MALLOC_ENV		28
     /* Old APIs had getc/putc macros that conflict with new CR/LF
	handling in the stdio buffers */
#define CYGWIN_VERSION_OLD_STDIO_CRLF_HANDLING \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED <= 20)

#define CYGWIN_VERSION_CHECK_FOR_S_IEXEC \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED >= 36)

#define CYGWIN_VERSION_CHECK_FOR_OLD_O_NONBLOCK \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED <= 28)

#define CYGWIN_VERSION_CHECK_FOR_USING_BIG_TYPES \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED >= 79)

#define CYGWIN_VERSION_CHECK_FOR_USING_ANCIENT_MSGHDR \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED <= 138)

#define CYGWIN_VERSION_CHECK_FOR_USING_WINSOCK1_VALUES \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED <= 138)

#define CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ \
  (CYGWIN_VERSION_USER_API_VERSION_COMBINED <= 161)

     /* API_MAJOR 0.0: Initial version.  API_MINOR changes:
	1: Export cygwin32_ calls as cygwin_ as well.
	2: Export j1, jn, y1, yn.
	3: Export dll_noncygwin_dllcrt0.
	4: New socket ioctls, revamped ifconf support.
	5: Thread support/exports.
	6: Change in termios handling.
	7: Export scandir and alphasort.
	8: Export _ctype_, _sys_errlist, _sys_nerr.
	9: Mount-related changes, new cygwin_umount export.
	   Raw device support (tape, floppies).
       10: Fast math routine support added.
       11: Export seekdir, telldir.
       12: Export pthread_join, pthread_detach.
       13: Export math funcs gamma and friends, also _j0, _j1, etc.
       14: Export snprintf and vnsprintf.
       15: Export glob
       16: Export cygwin_stackdump
       17: Export fast math stuff
       18: Stop exporting _strace_wm
       19: Export fchown, lchown, lacl
       20: regsub, inet_network
       21: incompatible change to stdio cr/lf and buffering
       22: Export cygwin_logon_user, cygwin_set_impersonation_token.
	   geteuid, getegid return effective uid/gid.
	   getuid, getgid return real uid/gid.
	   seteuid, setegid set only effective uid/gid.
	   setuid, setgid set effective and real uid/gid.
       23: Export new dll_crt0 interface and cygwin_user_data for use
	   with crt0 startup code.
       24: Export poll and _poll.
       25: Export getmode and _getmode.
       26: CW_GET_CYGDRIVE_PREFIXES addition to external.cc
       27: CW_GETPINFO_FULL addition to external.cc
       28: Accidentally bumped by cgf
       29: Export hstrerror
       30: CW_GET_CYGDRIVE_INFO addition to external.cc
       31: Export inet_aton
       32: Export getrlimit/setrlimit
       33: Export setlogmask
       34: Separated out mount table
       35: Export drand48, erand48, jrand48, lcong48, lrand48,
	   mrand48, nrand48, seed48, and srand48.
       36: Added _cygwin_S_IEXEC, et al
       37: [f]pathconv support _PC_POSIX_PERMISSIONS and _PC_POSIX_SECURITY
       38: vscanf, vscanf_r, and random pthread functions
       39: asctime_r, ctime_r, gmtime_r, localtime_r
       40: fchdir
       41: __signgam
       42: sys_errlist, sys_nerr
       43: sigsetjmp, siglongjmp fixed
       44: Export dirfd
       45: perprocess change, gamma_r, gammaf_r, lgamma_r, lgammaf_r
       46: Remove cygwin_getshared
       47: Report EOTWarningZoneSize in struct mtget.
       48: Export "posix" regex functions
       49: Export setutent, endutent, utmpname, getutent, getutid, getutline.
       50: Export fnmatch.
       51: Export recvmsg, sendmsg.
       52: Export strptime
       53: Export strlcat, strlcpy.
       54: Export __fpclassifyd, __fpclassifyf, __signbitd, __signbitf.
       55: Export fcloseall, fcloseall_r.
       56: Make ntsec on by default.
       57: Export setgroups.
       58: Export memalign, valloc, malloc_trim, malloc_usable_size, mallopt,
	   malloc_stats
       59: getsid
       60: MSG_NOSIGNAL
       61: Export getc_unlocked, getchar_unlocked, putc_unlocked,
	   putchar_unlocked
       62: Erroneously bumped
       63: Export pututline
       64: Export fseeko, ftello
       65: Export siginterrupt
       66: Export nl_langinfo
       67: Export pthread_getsequence_np
       68: Export netdb stuff
       69: Export strtof
       70: Export asprintf, _asprintf_r, vasprintf, _vasprintf_r
       71: Export strerror_r
       72: Export nanosleep
       73: Export setreuid32, setreuid, setregid32, setregid
       74: Export _strtold a64l hcreate hcreate_r hdestroy hdestroy_r hsearch
		  hsearch_r isblank iswalnum iswalpha iswblank iswcntrl iswctype
		  iswdigit iswgraph iswlower iswprint iswpunct iswspace iswupper
		  iswxdigit l64a mbrlen mbrtowc mbsinit mbsrtowcs mempcpy
		  on_exit setbuffer setlinebuf strndup strnlen tdelete tdestroy
		  tfind towctrans towlower towupper tsearch twalk wcrtomb wcscat
		  wcschr wcscpy wcscspn wcslcat wcslcpy wcsncat wcsncmp wcsncpy
		  wcspbrk wcsrchr wcsrtombs wcsspn wcsstr wctob wctob wctrans
		  wctype wmemchr wmemcmp wmemcpy wmemmove wmemset
       75: Export exp2 exp2f fdim fdimf fma fmaf fmax fmaxf fmin fminf lrint
		  lrintf lround lroundf nearbyint nearbyintf remquo remquof
		  round roundf scalbln scalblnf sincos sincosf tgamma tgammaf
		  truncf
       76: mallinfo
       77: thread-safe exit/at_exit
       78: Use stat and fstat rather than _stat, and _fstat.
	   Export btowc and trunc.
       79: Export acl32 aclcheck32 aclfrommode32 aclfrompbits32 aclfromtext32
		  aclsort32 acltomode32 acltopbits32 acltotext32 facl32
		  fgetpos64 fopen64 freopen64 fseeko64 fsetpos64 ftello64
		  _open64 _lseek64 _fstat64 _stat64 mknod32
       80: Export pthread_rwlock stuff
       81: CW_CHECK_NTSEC addition to external.cc
       82: Export wcscoll wcswidth wcwidth
       83: Export gethostid
       84: Pty open allocates invisible console.  64 bit interface
       85: Export new 32/64 functions from API 0.79 only with leading
	   underscore.  No problems with backward compatibility since no
	   official release has been made so far.  This change removes
	   exported symbols like fopen64, which might confuse configure.
       86: Export ftok
       87: Export vsyslog
       88: Export _getreent
       89: Export __mempcpy
       90: Export _fopen64
       91: Export argz_add argz_add_sep argz_append argz_count argz_create
	   argz_create_sep argz_delete argz_extract argz_insert
	   argz_next argz_replace argz_stringify envz_add envz_entry
	   envz_get envz_merge envz_remove envz_strip
       92: Export getusershell, setusershell, endusershell
       93: Export daemon, forkpty, openpty, iruserok, ruserok, login_tty,
	   openpty, forkpty, revoke, logwtmp, updwtmp
       94: Export getopt, getopt_long, optarg, opterr, optind, optopt,
	   optreset, __check_rhosts_file, __rcmd_errstr.
       95: Export shmat, shmctl, shmdt, shmget.
       96: CW_GET_ERRNO_FROM_WINERROR addition to external.cc
       97: Export sem_open, sem_close, sem_timedwait, sem_getvalue.
       98: Export _tmpfile64.
       99: CW_GET_POSIX_SECURITY_ATTRIBUTE addition to external.cc.
      100: CW_GET_SHMLBA addition to external.cc.
      101: Export err, errx, verr, verrx, warn, warnx, vwarn, vwarnx.
      102: CW_GET_UID_FROM_SID and CW_GET_GID_FROM_SID addition to external.cc.
      103: Export getprogname, setprogname.
      104: Export msgctl, msgget, msgrcv, msgsnd, semctl, semget, semop.
      105: Export sigwait.
      106: Export flock.
      107: Export fcntl64.
      108: Remove unused (hopefully) reent_data export.
      109: Oh well.  Someone uses reent_data.
      110: Export clock_gettime, sigwaitinfo, timer_create, timer_delete,
	   timer_settime
      111: Export sigqueue, sighold.
      112: Redefine some mtget fields.
      113: Again redefine some mtget fields.  Use mt_fileno and mt_blkno as
	   on Linux.
      114: Export rand_r, ttyname_r.
      115: Export flockfile, ftrylockfile, funlockfile, getgrgid_r, getgrnam_r,
	   getlogin_r.
      116: Export atoll.
      117: Export utmpx functions, Return utmp * from pututent.
      118: Export getpriority, setpriority.
      119: Export fdatasync.
      120: Export basename, dirname.
      122: Export statvfs, fstatvfs.
      123: Export utmpxname.
      124: Add MAP_AUTOGROW flag to mmap.
      125: LD_PRELOAD/CW_HOOK available.
      126: Export lsearch, lfind, timer_gettime.
      127: Export sigrelese.
      128: Export pselect.
      129: Export mkdtemp.
      130: Export strtoimax, strtoumax, llabs, imaxabs, lldiv, imaxdiv.
      131: Export inet_ntop, inet_pton.
      132: Add GLOB_LIMIT flag to glob.
      133: Export __getline, __getdelim.
      134: Export getline, getdelim.
      135: Export pread, pwrite
      136: Add TIOCMBIS/TIOCMBIC ioctl codes.
      137: fts_children, fts_close, fts_get_clientptr, fts_get_stream,
	   fts_open, fts_read, fts_set, fts_set_clientptr, ftw, nftw.
      138: Export readdir_r.
      139: Start using POSIX definition of struct msghdr and WinSock2
	   IPPROTO_IP values.
      140: Export mlock, munlock.
      141: Export futimes, lutimes.
      142: Export memmem
      143: Export clock_getres, clock_setres
      144: Export timelocal, timegm.
      145: Add MAP_NORESERVE flag to mmap.
      146: Change SI_USER definition.  FIXME: Need to develop compatibility
	   macro for this?
      147: Eliminate problematic d_ino from dirent structure.  unsetenv now
	   returns int, as per linux.
      148: Add open(2) flags O_SYNC, O_RSYNC, O_DSYNC and O_DIRECT.
      149: Add open(2) flag O_NOFOLLOW.
      150: Export getsubopt.
      151: Export __opendir_with_d_ino
      152: Revert to having d_ino in dirent unconditionally.
      153: Export updwtmpx, Implement CW_SETUP_WINENV.
      154: Export sigset, sigignore.
      155: Export __isinff, __isinfd, __isnanf, __isnand.
      156: Export __srbuf_r, __swget_r.
      157: Export gai_strerror, getaddrinfo, getnameinfo, freeaddrinfo,
	   in6addr_any, in6addr_loopback.
      158: Export bindresvport, bindresvport_sa, iruserok_sa, rcmd_af,
	   rresvport_af.
      159: Export posix_openpt.
      160: Export posix_fadvise, posix_fallocate.
      161: Export resolver functions.
      162: New struct ifreq.  Export if_nametoindex, if_indextoname,
	   if_nameindex, if_freenameindex.
      163: Export posix_madvise, posix_memalign.
      164: Export shm_open, shm_unlink.
      165: Export mq_close, mq_getattr, mq_notify, mq_open, mq_receive,
	   mq_send, mq_setattr, mq_timedreceive, mq_timedsend, mq_unlink.
      166: Export sem_unlink.
      167: Add st_birthtim to struct stat.
      168: Export asnprintf, dprintf, _Exit, vasnprintf, vdprintf.
      169: Export confstr.
      170: Export insque, remque.
      171: Export exp10, exp10f, pow10, pow10f, strcasestr, funopen,
	   fopencookie.
      172: Export getifaddrs, freeifaddrs.
      173: Export __assert_func.
      174: Export stpcpy, stpncpy.
      175: Export fdopendir.
      176: Export wcstol, wcstoll, wcstoul, wcstoull, wcsxfrm.
      177: Export sys_sigabbrev
      178: Export wcpcpy, wcpncpy.
      179: Export _f_llrint, _f_llrintf, _f_llrintl, _f_lrint, _f_lrintf,
	   _f_lrintl, _f_rint, _f_rintf, _f_rintl, llrint, llrintf, llrintl,
	   rintl, lrintl, and redirect exports of lrint, lrintf, rint, rintf.
      180: Export getxattr, lgetxattr, fgetxattr, listxattr, llistxattr,
	   flistxattr, setxattr, lsetxattr, fsetxattr, removexattr,
	   lremovexattr, fremovexattr.
      181: Export cygwin_conv_path, cygwin_create_path, cygwin_conv_path_list.
      182: Export lockf.
      FIXME: Removed 12 year old and entirely wrong wprintf function at
           this point.  We need a working implementation soon.
      183: Export open_memstream, fmemopen.
      184: Export openat, faccessat, fchmodat, fchownat, fstatat, futimesat,
	   linkat, mkdirat, mkfifoat, mknodat, readlinkat, renameat, symlinkat, 
	   unlinkat.
      185: Export futimens, utimensat.
      186: Remove ancient V8 regexp functions.  Also eliminate old crt0 interface
           which provided its own user_data structure.
      187: Export cfmakeraw.
     */

     /* Note that we forgot to bump the api for ualarm, strtoll, strtoull */

#define CYGWIN_VERSION_API_MAJOR 0
#define CYGWIN_VERSION_API_MINOR 187

     /* There is also a compatibity version number associated with the
	shared memory regions.  It is incremented when incompatible
	changes are made to the shared memory region *or* to any named
	shared mutexes, semaphores, etc.   The arbitrary starting
	version was 0 (cygwin release 98r2).
	Bump to 4 since this hasn't been rigorously updated in a
	while.  */

#define CYGWIN_VERSION_SHARED_DATA 5

     /* An identifier used in the names used to create shared objects.
	The full names include the CYGWIN_VERSION_SHARED_DATA version
	as well as this identifier. */

#define CYGWIN_VERSION_DLL_IDENTIFIER	"cygwin1"

     /* The Cygwin mount table interface in the Win32 registry also
	has a version number associated with it in case that is
	changed in a non-backwards compatible fashion.  Increment this
	version number whenever incompatible changes in mount table
	registry usage are made.

	1: Original number version.
	2: New mount registry layout, system-wide mount accessibility.
	3: The mount table is not in the registry anymore, but in /etc/fstab.
     */

#define CYGWIN_VERSION_MOUNT_REGISTRY 3

     /* Identifiers used in the Win32 registry. */

#define CYGWIN_INFO_CYGWIN_REGISTRY_NAME "Cygwin"
#define CYGWIN_INFO_PROGRAM_OPTIONS_NAME "Program Options"

     /* The default cygdrive prefix. */

#define CYGWIN_INFO_CYGDRIVE_DEFAULT_PREFIX "/cygdrive"

     /* In addition to the above version number strings, the build
	process adds some strings that may be useful in
	debugging/identifying a particular Cygwin DLL:

	The mkvers.sh script at the top level produces a .cc file
	which initializes a cygwin_version structure based on the
	above version information and creates a string table for
	grepping via "fgrep '%%%' cygwinwhatever.dll" if you are
	using GNU grep.  Otherwise you may want to do a
	"strings cygwinwhatever.dll | fgrep '%%%'" instead.

	This will produce output such as:

	%%% Cygwin dll_identifier: cygwin
	%%% Cygwin api_major: 0
	%%% Cygwin api_minor: 0
	%%% Cygwin dll_major: 19
	%%% Cygwin dll_minor: 6
	%%% Cygwin shared_data: 1
	%%% Cygwin registry: b15
	%%% Cygwin build date: Wed Oct 14 16:26:51 EDT 1998
	%%% Cygwin shared id: cygwinS1

	This information can also be obtained through a call to
	cygwin_internal (CW_GETVERSIONINFO).
     */

#define CYGWIN_VERSION_MAGIC(a, b) ((unsigned) ((((unsigned short) a) << 16) | (unsigned short) b))
#define CYGWIN_VERSION_MAGIC_VERSION(a) ((unsigned) ((unsigned)a & 0xffff))
