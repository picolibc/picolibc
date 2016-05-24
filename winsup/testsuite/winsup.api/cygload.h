// cygload.h                                      -*- C++ -*-
//
//
// Written by Max Kaehn <slothman@electric-cloud.com>
//
// This software is a copyrighted work licensed under the terms of the
// Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details.
//
// Note that dynamically linking to cygwin1.dll automatically places your code
// under the GPL unless you purchase a Cygwin Contract with Red Hat, Inc.
// See http://www.redhat.com/software/cygwin/ for more information.

// This program has large numbers of progress messages so as to provide
// maximum information about crash locations for anyone without access to
// a Microsoft debugger.


// This file contains the basic infrastructure for connecting an MSVC
// process to Cygwin.

#ifndef __CYGLOAD_H__
#define __CYGLOAD_H__

#include <windows.h>            // for GetProcAddress()
#include <functional>           // for pointer_to_unary_function
#include <stdexcept>            // for runtime_error
#include <string>
#include <map>
#include <vector>

// Convert GetLastError() to a human-readable STL exception.
class windows_error : public std::runtime_error
{
public:
  windows_error (const char *message, const char *detail = NULL)
    : runtime_error (format (GetLastError (), message, detail)) { }
  windows_error(DWORD error, const char *message, const char *detail = NULL)
    : runtime_error (format (error, message, detail)) { }

  static std::string format (DWORD error, const char *message,
                             const char *detail);
};

namespace cygwin
{

  // Cygwin keeps important thread-local information at the top of the
  // stack.  Its DllMain-equivalent will do the right thing for any threads
  // you spawn, but you need to declare one of these as the very first thing
  // in your main() function so horrible things won't happen when cygwin
  // overwrites your stack.  This will back up the data that will be
  // overwritten and restore it when the destructor is called.
  class padding {
  public:
    padding ();
    ~padding ();

    // Verifies that padding has been declared.
    static void check ();

  private:
    std::vector< char > _backup;
    char *_stackbase, *_end;

    // gdb reports sizeof(_cygtls) == 3964 at the time of this writing.
    // This is at the end of the object so it'll be toward the bottom
    // of the stack when it gets declared.
    char _padding[32768];

    static padding *_main;
    static DWORD _mainTID;
  };

  // This hooks your application up to cygwin:  it loads cygwin1.dll,
  // initializes it properly, grabs some important symbols, and
  // spawns a thread to let you receive signals from cygwin.
  class connector {
  public:
    connector (const char *dll = "cygwin1.dll");
    ~connector ();

    // A wrapper around GetProcAddress() for fetching symbols from the
    // cygwin DLL.  Can throw windows_error.
    template < class T > void get_symbol (const char *name, T &fn) const;

    // Wrappers for errno() and strerror().
    int err_no () const;
    std::string str_error (int) const;

    // Converting between the worlds of Windows and Cygwin.
    std::string unix_path (const std::string &windows) const;
    std::string win_path (const std::string &unix) const;

  private:
    HMODULE _library;

    int *(*_errno) ();
    const char *(*_strerror) (int);
    void (*_conv_to_full_posix_path) (const char *, char *);
    void (*_conv_to_full_win32_path) (const char *, char *);

  public:
    // The constructor will automatically hook you up for receiving
    // cygwin signals.  Just specify a signal and pass in a signal_handler.
    typedef std::pointer_to_unary_function<int,void> signal_handler;
    signal_handler *set_handler (int signal, signal_handler *);

  private:
    // Cygwin signals can only be received in threads that are calling
    // interruptible functions or otherwise ready to intercept signals, so
    // we spawn a thread that does nothing but call sigwait().

    // This is the entry point:
    static DWORD WINAPI signal_thread (void *);
    // It runs this:
    void await_signal ();
    // And will execute this on receipt of any signal for which it's
    // registered:
    void handle_signals (int);

    HANDLE _signal_thread;
    bool _waiting_for_signals, _signal_thread_done;
    CRITICAL_SECTION _thread_mutex;

    typedef std::map< int, signal_handler * > callback_list;
    callback_list _signal_handlers;
  };

  template <class T> void connector::get_symbol (const char *name,
                                                 T &symbol) const
  {
    FARPROC retval = NULL;

    retval = GetProcAddress (_library, name);

    if (retval == NULL)
      throw windows_error ("GetProcAddress", name);

    symbol = reinterpret_cast < T > (retval);
  }

  // cygwin::error converts errno to a human-readable exception.
  class error : public std::runtime_error
  {
  public:
    error (connector *c, const char *function, const char *detail = NULL)
      : runtime_error (format (c, c->err_no (), function, detail)) { }
    error (connector *c, int err_no, const char *function,
           const char *detail = NULL)
      : runtime_error (format (c, err_no, function, detail)) { }

    static std::string format(connector *c, int err_no,
                              const char *message, const char *detail);
  };
}

#endif // __CYGLOAD_H__
