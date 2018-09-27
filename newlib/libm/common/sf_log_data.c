/* Data definition for logf.
   Copyright (c) 2017 Arm Ltd.  All rights reserved.

   SPDX-License-Identifier: BSD-3-Clause

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the company may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "fdlibm.h"
#if !__OBSOLETE_MATH

#include "math_config.h"

const struct logf_data __logf_data = {
  .tab = {
  { 0x1.661ec79f8f3bep+0, -0x1.57bf7808caadep-2 },
  { 0x1.571ed4aaf883dp+0, -0x1.2bef0a7c06ddbp-2 },
  { 0x1.49539f0f010bp+0, -0x1.01eae7f513a67p-2 },
  { 0x1.3c995b0b80385p+0, -0x1.b31d8a68224e9p-3 },
  { 0x1.30d190c8864a5p+0, -0x1.6574f0ac07758p-3 },
  { 0x1.25e227b0b8eap+0, -0x1.1aa2bc79c81p-3 },
  { 0x1.1bb4a4a1a343fp+0, -0x1.a4e76ce8c0e5ep-4 },
  { 0x1.12358f08ae5bap+0, -0x1.1973c5a611cccp-4 },
  { 0x1.0953f419900a7p+0, -0x1.252f438e10c1ep-5 },
  { 0x1p+0, 0x0p+0 },
  { 0x1.e608cfd9a47acp-1, 0x1.aa5aa5df25984p-5 },
  { 0x1.ca4b31f026aap-1, 0x1.c5e53aa362eb4p-4 },
  { 0x1.b2036576afce6p-1, 0x1.526e57720db08p-3 },
  { 0x1.9c2d163a1aa2dp-1, 0x1.bc2860d22477p-3 },
  { 0x1.886e6037841edp-1, 0x1.1058bc8a07ee1p-2 },
  { 0x1.767dcf5534862p-1, 0x1.4043057b6ee09p-2 },
  },
  .ln2 = 0x1.62e42fefa39efp-1,
  .poly = {
  -0x1.00ea348b88334p-2, 0x1.5575b0be00b6ap-2, -0x1.ffffef20a4123p-2,
  }
};
#endif /* !__OBSOLETE_MATH */
