/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is mostly a header file test to make sure picolibc headers
 * can be used with a C++ compiler. This file mentions all POSIX headers,
 * but those not provided with picolibc are commented out
 */

//#include <aio.h>

#include <arpa/inet.h>
#include <assert.h>
#include <complex.h>
#include <cpio.h>
#include <ctype.h>

//#include <dirent.h>
//#include <dlfcn.h>

#include <errno.h>
#include <fcntl.h>
#include <fenv.h>
#include <float.h>
//#include <fmtmsg.h>
#include <fnmatch.h>
#include <glob.h>
#include <grp.h>
#include <iconv.h>
#include <inttypes.h>
#include <iso646.h>
#include <langinfo.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <math.h>

//#include <monetary.h>
//#include <mqueue.h>
//#include <ndbm.h>
//#include <net/if.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <nl_types.h>
//#include <poll.h>
//#include <pthread.h>

#include <pwd.h>
#include <regex.h>
#include <sched.h>
#include <search.h>

//#include <semaphore.h>

#include <setjmp.h>
#include <signal.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

//#include <stropts.h>
//#include <sys/ipc.h>
//#include <sys/mman.h>
//#include <sys/msg.h>

#include <sys/resource.h>
#include <sys/select.h>

//#include <sys/sem.h>
//#include <sys/shm.h>
//#include <sys/socket.h>

#include <sys/stat.h>

//#include <sys/statvfs.h>

#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

//#include <sys/uio.h>
//#include <sys/un.h>
//#include <sys/utsname.h>

#include <sys/wait.h>

//#include <syslog.h>

#include <tar.h>

//#include <termios.h>
//#include <tgmath.h>

#include <time.h>

//#include <trace.h>
//#include <ulimit.h>

#include <unistd.h>
#include <utime.h>

//#include <utmpx.h>

#include <wchar.h>
#include <wctype.h>
#include <wordexp.h>

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wmain"
#endif

extern "C" {

int main(void);

int main(void) {
	printf("Hello, world!\n");
	return 0;
}

}
