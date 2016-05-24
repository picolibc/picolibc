/* testsuite.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This file implements a test harness for the MinGW implementation of
   POSIX path translation in utils/path.cc.  This code is used by strace
   and cygcheck which cannot depend on the Cygwin DLL.  The tests below
   are a basic set of sanity checks for translating relative and
   absolute paths from POSIX form to Win32 form based on the contents of
   a mount table.  */

/* Including this file should be a no-op if TESTSUITE is not defined.  */
#ifdef TESTSUITE

/* These definitions are common to both the testsuite mount table
   as well as the testsuite definitions themselves, so define them
   here so that they are only defined in one location.  */
#define TESTSUITE_ROOT "X:\\xyzroot"
#define TESTSUITE_CYGDRIVE "/testcygdrive"

/* Define a mount table in the form that read_mounts() would populate.
   This is used in place of actually reading the host mount
   table from the registry for the duration of the testsuite.  This
   table should match the battery of tests below.  */

#if defined(TESTSUITE_MOUNT_TABLE)
static mnt_t mount_table[] = {
/* native                 posix               flags                        issys */
 { TESTSUITE_ROOT,        (char*)"/",                MOUNT_BINARY | MOUNT_SYSTEM, 1 },
 { "O:\\other",           (char*)"/otherdir",        MOUNT_BINARY | MOUNT_SYSTEM, 1 },
 { "S:\\some\\dir",       (char*)"/somedir",         MOUNT_BINARY | MOUNT_SYSTEM, 1 },
 { TESTSUITE_ROOT"\\bin", (char*)"/usr/bin",         MOUNT_BINARY | MOUNT_SYSTEM, 1 },
 { TESTSUITE_ROOT"\\lib", (char*)"/usr/lib",         MOUNT_BINARY | MOUNT_SYSTEM, 1 },
 { ".",                   (char*)TESTSUITE_CYGDRIVE, MOUNT_BINARY | MOUNT_SYSTEM | MOUNT_CYGDRIVE, 1 },
 { NULL,                  (char*)NULL,               0,                           0 }
};


/* Define the main set of tests.  This is defined here instead of in
   testsuite.cc so that all test harness data is in one place and not
   spread over several files.  */

#elif defined(TESTSUITE_TESTS)
#define NO_CWD "N/A"
static test_t testsuite_tests[] = {
 { NO_CWD,                     "/file.ext",              TESTSUITE_ROOT"\\file.ext" },
 { NO_CWD,                     "/dir/file.ext",          TESTSUITE_ROOT"\\dir\\file.ext" },
 { NO_CWD,                     "/foo/dir/file.ext",      TESTSUITE_ROOT"\\foo\\dir\\file.ext" },
 { NO_CWD,                     "/bin/file.ext",          TESTSUITE_ROOT"\\bin\\file.ext" },
 { NO_CWD,                     "/bin/dir/file.ext",      TESTSUITE_ROOT"\\bin\\dir\\file.ext" },
 { NO_CWD,                     "/lib/file.ext",          TESTSUITE_ROOT"\\lib\\file.ext" },
 { NO_CWD,                     "/lib/dir/file.ext",      TESTSUITE_ROOT"\\lib\\dir\\file.ext" },
 { NO_CWD,                     "/usr/bin/file.ext",      TESTSUITE_ROOT"\\bin\\file.ext" },
 { NO_CWD,                     "/usr/bin/dir/file.ext",  TESTSUITE_ROOT"\\bin\\dir\\file.ext" },
 { NO_CWD,                     "/usr/lib/file.ext",      TESTSUITE_ROOT"\\lib\\file.ext" },
 { NO_CWD,                     "/usr/lib/dir/file.ext",  TESTSUITE_ROOT"\\lib\\dir\\file.ext" },
 { NO_CWD,                     "/home/file.ext",         TESTSUITE_ROOT"\\home\\file.ext" },
 { NO_CWD,                     "/home/foo/file.ext",     TESTSUITE_ROOT"\\home\\foo\\file.ext" },
 { NO_CWD,                     "/home/foo/dir/file.ext", TESTSUITE_ROOT"\\home\\foo\\dir\\file.ext" },
 { NO_CWD,                     "/usr/file.ext",          TESTSUITE_ROOT"\\usr\\file.ext" },
 { NO_CWD,                     "/usr/share/file.ext",    TESTSUITE_ROOT"\\usr\\share\\file.ext" },
 { TESTSUITE_ROOT,             "foo",                    TESTSUITE_ROOT"\\foo" },
 { TESTSUITE_ROOT,             "./foo",                  TESTSUITE_ROOT"\\foo" },
 { TESTSUITE_ROOT,             "foo/bar",                TESTSUITE_ROOT"\\foo\\bar" },
 { TESTSUITE_ROOT,             "./foo/bar",              TESTSUITE_ROOT"\\foo\\bar" },
 { TESTSUITE_ROOT,             "foo/./bar",              TESTSUITE_ROOT"\\foo\\bar" },
 { TESTSUITE_ROOT,             "./foo/./bar",            TESTSUITE_ROOT"\\foo\\bar" },
 { TESTSUITE_ROOT,             "bin/file.ext",           TESTSUITE_ROOT"\\bin\\file.ext" },
 { TESTSUITE_ROOT,             "lib/file.ext",           TESTSUITE_ROOT"\\lib\\file.ext" },
 { TESTSUITE_ROOT,             "usr/bin/file.ext",       TESTSUITE_ROOT"\\bin\\file.ext" },
 { TESTSUITE_ROOT,             "usr/lib/file.ext",       TESTSUITE_ROOT"\\lib\\file.ext" },
 { TESTSUITE_ROOT,             "etc/file.ext",           TESTSUITE_ROOT"\\etc\\file.ext" },
 { TESTSUITE_ROOT,             "etc/foo/file.ext",       TESTSUITE_ROOT"\\etc\\foo\\file.ext" },
 { TESTSUITE_ROOT"\\bin",      "foo",                    TESTSUITE_ROOT"\\bin\\foo" },
 { TESTSUITE_ROOT"\\bin",      "./foo",                  TESTSUITE_ROOT"\\bin\\foo" },
 { TESTSUITE_ROOT"\\bin",      "foo/bar",                TESTSUITE_ROOT"\\bin\\foo\\bar" },
 { TESTSUITE_ROOT"\\bin",      "./foo/bar",              TESTSUITE_ROOT"\\bin\\foo\\bar" },
 { TESTSUITE_ROOT"\\bin",      "foo/./bar",              TESTSUITE_ROOT"\\bin\\foo\\bar" },
 { TESTSUITE_ROOT"\\bin",      "./foo/./bar",            TESTSUITE_ROOT"\\bin\\foo\\bar" },
 { TESTSUITE_ROOT"\\bin\\foo", "bar",                    TESTSUITE_ROOT"\\bin\\foo\\bar" },
 { TESTSUITE_ROOT"\\bin\\foo", "./bar",                  TESTSUITE_ROOT"\\bin\\foo\\bar" },
 { TESTSUITE_ROOT"\\bin\\foo", "bar/baz",                TESTSUITE_ROOT"\\bin\\foo\\bar\\baz" },
 { TESTSUITE_ROOT"\\bin\\foo", "./bar/baz",              TESTSUITE_ROOT"\\bin\\foo\\bar\\baz" },
 { TESTSUITE_ROOT"\\bin\\foo", "bar/./baz",              TESTSUITE_ROOT"\\bin\\foo\\bar\\baz" },
 { TESTSUITE_ROOT"\\bin\\foo", "./bar/./baz",            TESTSUITE_ROOT"\\bin\\foo\\bar\\baz" },
 { TESTSUITE_ROOT"\\tmp",      "foo",                    TESTSUITE_ROOT"\\tmp\\foo" },
 { TESTSUITE_ROOT"\\tmp",      "./foo",                  TESTSUITE_ROOT"\\tmp\\foo" },
 { TESTSUITE_ROOT"\\tmp",      "foo/bar",                TESTSUITE_ROOT"\\tmp\\foo\\bar" },
 { TESTSUITE_ROOT"\\tmp",      "./foo/bar",              TESTSUITE_ROOT"\\tmp\\foo\\bar" },
 { NO_CWD,                     "/otherdir/file.ext",     "O:\\other\\file.ext" },
 { NO_CWD,                     "/otherdir/./file.ext",   "O:\\other\\file.ext" },
 { NO_CWD,                     "/otherdir/foo/file.ext", "O:\\other\\foo\\file.ext" },
 { "O:\\other",                "file.ext",               "O:\\other\\file.ext" },
 { "O:\\other",                "./file.ext",             "O:\\other\\file.ext" },
 { "O:\\other",                "foo/file.ext",           "O:\\other\\foo\\file.ext" },
 { "O:\\other\\foo",           "file.ext",               "O:\\other\\foo\\file.ext" },
 { "O:\\other\\foo",           "./file.ext",             "O:\\other\\foo\\file.ext" },
 { "O:\\other\\foo",           "bar/file.ext",           "O:\\other\\foo\\bar\\file.ext" },
 { NO_CWD,                     "/somedir/file.ext",      "S:\\some\\dir\\file.ext" },
 { NO_CWD,                     "/somedir/./file.ext",    "S:\\some\\dir\\file.ext" },
 { NO_CWD,                     "/somedir/foo/file.ext",  "S:\\some\\dir\\foo\\file.ext" },
 { "S:\\some\\dir",            "file.ext",               "S:\\some\\dir\\file.ext" },
 { "S:\\some\\dir",            "./file.ext",             "S:\\some\\dir\\file.ext" },
 { "S:\\some\\dir",            "foo/file.ext",           "S:\\some\\dir\\foo\\file.ext" },
 { "S:\\some\\dir\\foo",       "file.ext",               "S:\\some\\dir\\foo\\file.ext" },
 { "S:\\some\\dir\\foo",       "./file.ext",             "S:\\some\\dir\\foo\\file.ext" },
 { "S:\\some\\dir\\foo",       "bar/file.ext",           "S:\\some\\dir\\foo\\bar\\file.ext" },
 { NO_CWD,                     "//server/share/foo/bar", "\\\\server\\share\\foo\\bar" },
 { NO_CWD,                     NULL,                     NULL }
};

#else

/* Redirect calls to GetCurrentDirectory() to the testsuite instead.  */
#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif
#define GetCurrentDirectory testsuite_getcwd

DWORD testsuite_getcwd (DWORD, LPSTR);

#endif

#endif /* TESTSUITE */

