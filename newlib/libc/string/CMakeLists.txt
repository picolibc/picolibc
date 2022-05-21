#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2022 Keith Packard
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
picolibc_sources(
  bcmp.c
  bcopy.c
  bzero.c
  explicit_bzero.c
  ffsl.c
  ffsll.c
  fls.c
  flsl.c
  flsll.c
  gnu_basename.c
  index.c
  memccpy.c
  memchr.c
  memcmp.c
  memcpy.c
  memmem.c
  memmove.c
  mempcpy.c
  memrchr.c
  memset.c
  rawmemchr.c
  rindex.c
  stpcpy.c
  stpncpy.c
  strcasecmp.c
  strcasecmp_l.c
  strcasestr.c
  strcat.c
  strchr.c
  strchrnul.c
  strcoll.c
  strcoll_l.c
  strcmp.c
  strcpy.c
  strcspn.c
  strdup.c
  strerror.c
  strerror_r.c
  strlcat.c
  strlcpy.c
  strlen.c
  strlwr.c
  strncasecmp.c
  strncasecmp_l.c
  strncat.c
  strncmp.c
  strncpy.c
  strndup.c
  strnlen.c
  strnstr.c
  strpbrk.c
  strrchr.c
  strsep.c
  strsignal.c
  strspn.c
  strstr.c
  strtok.c
  strtok_r.c
  strupr.c
  strverscmp.c
  strxfrm.c
  strxfrm_l.c
  swab.c
  timingsafe_bcmp.c
  timingsafe_memcmp.c
  u_strerr.c
  wcpcpy.c
  wcpncpy.c
  wcscasecmp.c
  wcscasecmp_l.c
  wcscat.c
  wcschr.c
  wcscmp.c
  wcscoll.c
  wcscoll_l.c
  wcscpy.c
  wcscspn.c
  wcsdup.c
  wcslcat.c
  wcslcpy.c
  wcslen.c
  wcsncasecmp.c
  wcsncasecmp_l.c
  wcsncat.c
  wcsncmp.c
  wcsncpy.c
  wcsnlen.c
  wcspbrk.c
  wcsrchr.c
  wcsspn.c
  wcsstr.c
  wcstok.c
  wcswidth.c
  wcsxfrm.c
  wcsxfrm_l.c
  wcwidth.c
  wmemchr.c
  wmemcmp.c
  wmemcpy.c
  wmemmove.c
  wmempcpy.c
  wmemset.c
  xpg_strerror_r.c
  )
