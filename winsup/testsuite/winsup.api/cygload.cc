/* cygload.cc

   Written by Max Kaehn <slothman@electric-cloud.com>
  
   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details.
  
   Note that dynamically linking to cygwin1.dll automatically places your code
   under the GPL unless you purchase a Cygwin Contract with Red Hat, Inc.
   See http://www.redhat.com/software/cygwin/ for more information.

   cygload demonstrates how to dynamically load cygwin1.dll.  The default
   build uses MinGW to compile it; the Makefile also shows how to build
   it using the Microsoft compiler.

   By default, the program will silently test basic functionality:
     * Making space on the stack for cygtls
     * Loading and initializing cygwin1.dll
     * Path translation
     * Error handling
     * Signal handling

   Options for this program:
     -v              Verbose output.  Normal operation is entirely silent,
		     save for errors.
     -testinterrupts Pauses the program for 30 seconds so you can demonstrate
		     that it handles ^C properly.
     -cygwin         Name of DLL to load.  Defaults to "cygwin1.dll". */

#include "cygload.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <errno.h>              // for ENOENT
#include <sys/types.h>
#include <sys/stat.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

cygwin::padding *cygwin::padding::_main = NULL;
DWORD cygwin::padding::_mainTID = 0;

// Cygwin signal constants
#undef SIGINT
#undef SIGTERM

static const int SIGHUP = 1;
static const int SIGINT = 2;
static const int SIGTERM = 15;  // Cygwin won't deliver this one to us;
                                // expect unadorned "kill" to just kill
                                // your process.
static const int SIGSTOP = 17;  // Cygwin insists on delivering SIGSTOP to
                                // the main thread.  If your main thread
                                // is not interruptible, you'll miss the
                                // signal and ignore the request to suspend.
static const int SIGTSTP = 18;  // ^Z on a tty.
static const int SIGCONT = 19;  // Resume a stopped process.
static const int SIGUSR1 = 30;
static const int SIGUSR2 = 31;

// Using *out instead of cout.  In verbose mode, out == &cout.
static std::ostream *out = NULL;

cygwin::padding::padding ()
{
  _main = this;
  _mainTID = GetCurrentThreadId ();

  _end = _padding + sizeof (_padding);
  char *stackbase;
#ifdef __GNUC__ /* GCC */
# ifdef __x86_64__
    __asm__ (
    "mov %%gs:8, %0"
    :"=r"(stackbase)
    );
# elif __X86__
  __asm__ (
    "movl %%fs:4, %0"
    :"=r"(stackbase)
    );
# else
#  error Unknown architecture
# endif
#else /* !GCC assumed to be MSVC */
# ifdef __X86__
  __asm
      {
        mov eax, fs:[4];
        mov stackbase, eax;
      }
#else
#  error Unknown architecture
# endif
#endif
  _stackbase = stackbase;

  // We've gotten as close as we can to the top of the stack.  Even
  // subverting the entry point, though, still doesn't get us there-- I'm
  // getting 64 bytes in use before the entry point.  So we back up the data
  // there and restore it when the destructor is called:
  if ((_stackbase - _end) != 0)
    {
      size_t delta = (_stackbase - _end);

      _backup.resize (delta);

      memcpy (&(_backup[0]), _end, delta);
    }
}

cygwin::padding::~padding ()
{
  _main = NULL;

  if (_backup.size ())
    {
      memcpy (_end, &(_backup[0]), _backup.size ());
    }
}

void
cygwin::padding::check ()
{
  if (_main == NULL)
    throw std::runtime_error ("No padding declared!");
  if (_mainTID != GetCurrentThreadId ())
    throw std::runtime_error ("You need to initialize cygwin::connector "
                              "in the same thread in which you declared the "
                              "padding.");

  if (_main->_backup.size ())
    *out << "Warning!  Stack base is "
         << static_cast<void *>(_main->_stackbase)
         << ".  padding ends at " << static_cast<void *>(_main->_end)
         << ".  Delta is " << (_main->_stackbase - _main->_end)
         << ".  Stack variables could be overwritten!" << endl;
}

cygwin::connector::connector (const char *dll)
{
  // This will throw if padding is not in place.
  padding::check ();

  *out << "Loading " << dll << "..." << endl;

  // This should call init.cc:dll_entry() with DLL_PROCESS_ATTACH,
  // which calls dll_crt0_0().
  if ((_library = LoadLibrary (dll)) == NULL)
    throw windows_error ("LoadLibrary", dll);

  *out << "Initializing cygwin..." << endl;

  // This calls dcrt0.cc:cygwin_dll_init(), which calls dll_crt0_1(),
  // which will, among other things:
  // * spawn the cygwin signal handling thread from sigproc_init()
  // * initialize the thread-local storage for this thread and overwrite
  //   the first 4K of the stack
  void (*cyginit) ();
  get_symbol ("cygwin_dll_init", cyginit);
  (*cyginit) ();

  *out << "Loading symbols..." << endl;

  // Pick up the function pointers for the basic infrastructure.
  get_symbol ("__errno", _errno);
  get_symbol ("strerror", _strerror);

  // Note that you need to be running an interruptible cygwin function if
  // you want to receive signals.  You can use the standard signal()
  // mechanism if you're willing to have your main thread spend all its time
  // in interruptible cygwin functions like sleep().  Christopher Faylor
  // cautions that this solution "could be slightly racy":  if a second
  // signal comes in before the first one is done processing, the thread
  // won't be back in sigwait() to catch it.
  *out << "Spawning signal handling thread..." << endl;

  _waiting_for_signals = true;
  _signal_thread_done = false;
  InitializeCriticalSection (&_thread_mutex);

  DWORD tid;

  _signal_thread = CreateThread (NULL,   // Default security.
                                 32768,  // Adjust the stack size as
                                         // appropriate for what your signal
                                         // handler needs in order to run, and
                                         // then add 4K for cygtls.
                                 &signal_thread, // Function to call
                                 this,   // Context
                                 0,      // Flags
                                 &tid);  // Thread ID

  if (_signal_thread == NULL)
    throw windows_error ("CreateThread", "signal_thread");
}

cygwin::connector::~connector ()
{
  try
  {
    // First, shut down signal handling.
    int (*raze) (int);
    int (*pthread_join) (void *, void **);

    get_symbol ("raise", raze);
    get_symbol ("pthread_join", pthread_join);

    // Tell the listener to shut down...
    _waiting_for_signals = false;
    int err = 0;
    EnterCriticalSection (&_thread_mutex);
    if (!_signal_thread_done)
      err = raze (SIGUSR2);
    LeaveCriticalSection (&_thread_mutex);
    if (err)
      cerr << error (this, "raise", "SIGUSR2").what () << endl;
    // ...and get the thread to join.
    if (!CloseHandle (_signal_thread))
      throw windows_error ("CloseHandle", "signal_thread");

    // This should call init.cc:dll_entry() with DLL_PROCESS_DETACH.
    if (!FreeLibrary (_library))
      throw windows_error ("FreeLibrary", "cygwin1.dll");
  }
  catch (std::exception &x)
  {
    cerr << x.what () << endl;
  }
}

DWORD WINAPI
cygwin::connector::signal_thread (void *param)
{
  connector *that = reinterpret_cast < connector * > (param);

  try
  {
    that->await_signal ();
  }
  catch (std::exception &x)
  {
    cerr << "signal_thread caught " << x.what () << endl;
    return 0;
  }
  return 0;
}

void
cygwin::connector::await_signal ()
{
  // Wait for signals.
  unsigned long sigset[32];
  int sig;
  int (*empty) (void *);
  int (*add) (void *, int);
  int (*wait) (void *, int *);

  get_symbol ("sigemptyset", empty);
  get_symbol ("sigaddset", add);
  get_symbol ("sigwait", wait);

  empty (sigset);
  add (sigset, SIGHUP);
  add (sigset, SIGINT);
//  add (sigset, SIGSTOP);
//  add (sigset, SIGTSTP);      // I can't get this to suspend properly, so
                                // I'll leave it up to chance that the main
                                // thread is interruptible.
  add (sigset, SIGUSR1);
  add (sigset, SIGUSR2);

  while (_waiting_for_signals)
    {
      int err = wait (sigset, &sig);
      if (err)
        cerr << error (this, "sigwait").what () << endl;
      else
        *out << "Received signal " << sig << "." << endl;
      switch (sig)
        {
          case SIGUSR2:
            if (!_waiting_for_signals)
              {
                // SIGUSR2 is how ~connector wakes this thread
                goto done;
              }
            break;
          default:
            break;
        }
      handle_signals (sig);
    }
done:
  EnterCriticalSection (&_thread_mutex);
  _signal_thread_done = true;
  LeaveCriticalSection (&_thread_mutex);

  *out << "await_signal done." << endl;
}

cygwin::connector::signal_handler *
cygwin::connector::set_handler (int signal, signal_handler *handler)
{
  signal_handler *retval = _signal_handlers[signal];

  if (handler == NULL)
    _signal_handlers.erase (signal);
  else
    _signal_handlers[signal] = handler;

  return retval;
}

void
cygwin::connector::handle_signals (int sig)
{
  callback_list::iterator h = _signal_handlers.find (sig);

  if (h != _signal_handlers.end ())
    {
      try
      {
        signal_handler *handler = h->second;
        (*handler) (sig);
        return;
      }
      catch (std::exception &x)
      {
        cerr << "cygwin::connector::handle_signals caught "
             << x.what () << "!" << endl;
        return;
      }
    }

  cerr << "No handler for signal " << sig << "!" << endl;
}

int
cygwin::connector::err_no () const
{
  int *e = (*_errno) ();
  if (e == NULL)
    {
      return -1;
    }
  return *e;
}

string
cygwin::connector::str_error (int err_no) const
{
  string retval;

  const char *s = (*_strerror) (err_no);
  if (s != NULL)
    {
      retval = s;
    }
  else
    {
      std::ostringstream o;
      o << "Unexpected errno " << err_no;
      retval = o.str ();
    }

  return retval;
}

string
cygwin::connector::unix_path (const string &windows) const
{
  char buf[MAX_PATH];

  _conv_to_full_posix_path (windows.c_str (), buf);

  return string (buf);
}

string
cygwin::connector::win_path (const string &unix) const
{
  char buf[MAX_PATH];

  _conv_to_full_win32_path (unix.c_str (), buf);

  return string (buf);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

string
cygwin::error::format (cygwin::connector *c,
                       int err_no, const char *message, const char *detail)
{
  std::ostringstream ret;

  ret << message;
  if (detail)
    {
      ret << "(" << detail << ")";
    }
  ret << ":  " << c->str_error (err_no);

  return ret.str ();
}

string
windows_error::format (DWORD error, const char *message, const char *detail)
{
  std::ostringstream ret;
  char buf[512];
  DWORD bytes;

  ret << message;
  if (detail)
    ret << "(" << detail << ")";
  ret << ":  ";

  bytes = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
                         MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                         buf, sizeof (buf), 0);

  if (bytes == 0)
    ret << "Unexpected Windows error " << error;
  else
    {
      // Remove trailing whitespace
      char *p = buf + bytes - 1;
      while (isspace (*p))
        *p-- = '\0';
      ret << buf << " (" << error << ")";
    }

  return ret.str ();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C" int mainCRTStartup ();

// This just pushes 4K onto the stack, backs up the original stack, and
// jumps into the regular startup code.  This avoids having to worry about
// backing up argc and argv.
extern "C" int __stdcall
cygloadCRTStartup ()
{
  cygwin::padding padding;
  return mainCRTStartup ();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void
hangup (int sig)
{
  cout << "Hangup (" << sig << ")." << endl;
}

static void
interrupt (int sig)
{
  cerr << "Interrupt (" << sig << ")!" << endl;
  exit (0);
}

static int caught = false;

static void
catch_signal (int)
{
  *out << "Signals are working." << endl;
  caught = true;
}

int
main (int argc, char *argv[])
{
  // If you do not want to use cygloadCRTStartup() as an entry point,
  // uncomment this line, but be sure to have *everything* you want
  // from the stack below it backed up before you call the
  // constructor for cygwin::connector.
  //cygwin::padding padding;

  std::ostringstream output;
  bool verbose = false, testinterrupts = false;
  const char *dll = "cygwin1.dll";

  out = &output;

  for (int i = 1; i < argc; ++i)
    {
      string arg = string (argv[i]);

      if (arg == "-v")
        {
          verbose = true;
          out = &cout;
        }
      else if (arg == "-testinterrupts")
        testinterrupts = true;
      else if (arg == "-cygwin")
        {
          if (i+1 >= argc)
            {
              cerr << "Need to supply an argument with -cygwin." << endl;
              return 255;
            }
          dll = argv[++i];
        }
    }


  try
  {
    *out << "Connecting to cygwin..." << endl;
    cygwin::connector cygwin (dll);
    *out << "Successfully connected." << endl;

    string result = cygwin.str_error (ENOENT);

    if (result != "No such file or directory")
      {
        cerr << "strerror(ENOENT) returned \""
             << result
             << "\" instead of \"No such file or directory\"!"
             << endl;
        return 1;
      }
    else if (verbose)
      {
        *out << "strerror(ENOENT) = " << result << endl;
      }

    // Path conversion:  from cygwin to Windows...
    result = cygwin.win_path ("/usr");
    struct _stat statbuf;
    if (::_stat (result.c_str (), &statbuf) < 0)
      {
        cerr << "stat(\"" << result << "\") failed!" << endl;
        return 2;
      }
    else if (verbose)
      {
        *out << "/usr == " << result << endl;
      }

    // ...and back:
    char buf[MAX_PATH], scratch[256];
    GetSystemDirectory (buf, sizeof(buf));
    int (*cygstat) (const char *, void *);
    cygwin.get_symbol ("stat", cygstat);

    if (cygstat (buf, scratch) < 0)
      {
        cerr << "cygwin stat(\"" << buf << "\") failed!" << endl;
        return 3;
      }
    else if (verbose)
      {
        *out << buf << " == " << cygwin.unix_path(buf) << endl;
      }

    // Test error handling.  This should output
    // "open(/potrzebie/furshlugginer):  No such file or directory"
    {
      int (*cygopen) (const char *, int);
      cygwin.get_symbol ("open", cygopen);

      if (cygopen ("/potrzebie/furshlugginer", 0 /* O_RDONLY */ ) < 0)
        {
          int err = cygwin.err_no ();
          if (err != ENOENT)
            {
              cerr << "cygwin open(\"/potrzebie/furshlugginer\", "
                  "O_RDONLY):  expected to fail with ENOENT, got "
                   << err << "!" << endl;
              return 4;
            }
          if (verbose)
            *out << cygwin::error (&cygwin, "open",
                                   "/potrzebie/furshlugginer").what ()
                 << endl;
        }
      else
        {
          cerr << "/potrzebie/furshlugginer should not exist!"
               << endl;
          return 5;
        }
    }

    // And signal handling:
    std::pointer_to_unary_function < int , void > h1 (&hangup);
    std::pointer_to_unary_function < int , void > h2 (&interrupt);
    std::pointer_to_unary_function < int , void > h3 (&catch_signal);
    cygwin.set_handler (SIGHUP, &h1);
    cygwin.set_handler (SIGINT, &h2);
    cygwin.set_handler (SIGUSR1, &h3);

    // Make sure the signal handler thread has had time to start...
    Sleep (100);
    // Send a test signal to set "caught" to true...
    int (*raze) (int);
    cygwin.get_symbol ("raise", raze);
    raze (SIGUSR1);
    // And give the thread time to wait for the shutdown signal.
    Sleep (100);

    if (testinterrupts)
      {
        // This is a worst case scenario for testing interrupts:  the
        // main thread is in a long-duration Windows API call.  This
        // makes the main thread uninterruptible; cygwin will retry
        // 20 times, with a low_priority_sleep(0) between each try.
        cout << "Sleeping for 30 seconds, waiting for a signal..." << endl;
        Sleep (30000);
        cout << "Done waiting." << endl;
      }
  }
  catch (std::exception &x)
  {
    cerr << x.what () << endl;
    return 2;
  }

  if (caught)
    return 0;
  else
    {
      cerr << "Never received SIGUSR1." << endl;
      return 1;
    }
}
