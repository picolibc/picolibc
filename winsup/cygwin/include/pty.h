#ifndef __PTY_H__
#define __PTY_H__

#include <_ansi.h>
#include <sys/termios.h>

#ifdef __cplusplus
extern "C" {
#endif

int _EXFUN(openpty ,(int *, int *, char *, struct termios *, struct winsize *));
int _EXFUN(forkpty ,(int *, char *, struct termios *, struct winsize *));

#ifdef __cplusplus
}
#endif

#endif /* __PTY_H__ */
