#ifndef _SYS_CYGWIN_H
#define _SYS_CYGWIN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern pid_t cygwin32_winpid_to_pid (int);
extern void cygwin32_win32_to_posix_path_list (const char *, char *);
extern int cygwin32_win32_to_posix_path_list_buf_size (const char *);
extern void cygwin32_posix_to_win32_path_list (const char *, char *);
extern int cygwin32_posix_to_win32_path_list_buf_size (const char *);
extern int cygwin32_conv_to_win32_path (const char *, char *);
extern int cygwin32_conv_to_full_win32_path (const char *, char *);
extern void cygwin32_conv_to_posix_path (const char *, char *);
extern void cygwin32_conv_to_full_posix_path (const char *, char *);
extern int cygwin32_posix_path_list_p (const char *);
extern void cygwin32_split_path (const char *, char *, char *);

extern pid_t cygwin_winpid_to_pid (int);
extern int cygwin_win32_to_posix_path_list (const char *, char *);
extern int cygwin_win32_to_posix_path_list_buf_size (const char *);
extern int cygwin_posix_to_win32_path_list (const char *, char *);
extern int cygwin_posix_to_win32_path_list_buf_size (const char *);
extern int cygwin_conv_to_win32_path (const char *, char *);
extern int cygwin_conv_to_full_win32_path (const char *, char *);
extern int cygwin_conv_to_posix_path (const char *, char *);
extern int cygwin_conv_to_full_posix_path (const char *, char *);
extern int cygwin_posix_path_list_p (const char *);
extern void cygwin_split_path (const char *, char *, char *);

#ifdef _GNU_H_WINDOWS32_BASE
/* included if <windows.h> is included */
extern int cygwin32_attach_handle_to_fd (char *, int, HANDLE, int, int);
extern int cygwin_attach_handle_to_fd (char *, int, HANDLE, mode_t, unsigned);
#endif

#ifdef __cplusplus
};
#endif

#endif /* _SYS_CYGWIN_H */
