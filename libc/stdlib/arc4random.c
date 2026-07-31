/*	$OpenBSD: arc4random.c,v 1.58 2022/07/31 13:41:45 tb Exp $	*/

/*
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 * Copyright (c) 2013, Markus Friedl <markus@openbsd.org>
 * Copyright (c) 2014, Theo de Raadt <deraadt@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * ChaCha based random number generator for OpenBSD.
 */

#define _DEFAULT_SOURCE
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/lock.h>

#define KEYSTREAM_ONLY
#include "chacha_private.h"

#define minimum(a, b) ((a) < (b) ? (a) : (b))

#if defined(__GNUC__) || defined(_MSC_VER)
#define inline __inline
#else /* __GNUC__ || _MSC_VER */
#define inline
#endif /* !__GNUC__ && !_MSC_VER */

#define KEYSZ   32
#define IVSZ    8
#define BLOCKSZ 64
#define RSBUFSZ (16 * BLOCKSZ)

#if SIZE_MAX <= 65535
#define REKEY_BASE ((size_t)32 * 1024) /* NB. should be a power of 2 */
#elif SIZE_MAX <= 1048575
#define REKEY_BASE ((size_t)256 * 1024) /* NB. should be a power of 2 */
#else
#define REKEY_BASE ((size_t)1024 * 1024) /* NB. should be a power of 2 */
#endif

/* Marked MAP_INHERIT_ZERO, so zero'd out in fork children. */
struct _rs {
    size_t rs_have;  /* valid bytes at end of rs_buf */
    size_t rs_count; /* bytes till reseed */
};

/* Maybe be preserved in fork children, if _rs_allocate() decides. */
struct _rsx {
    chacha_ctx    rs_chacha;       /* chacha context for random keystream */
    unsigned char rs_buf[RSBUFSZ]; /* keystream blocks */
};

static bool        arc4random_init;
static struct _rs  arc4random_rs;
static struct _rsx arc4random_rsx;

static inline void _rs_rekey(unsigned char *dat, size_t datlen);

int                arc4random_fork_detect(void) __weak;
void               arc4random_abort(void) __weak;

static inline void
_rs_init(unsigned char *buf, size_t n)
{
    if (n < KEYSZ + IVSZ)
        return;

    chacha_keysetup(&arc4random_rsx.rs_chacha, buf, KEYSZ * 8);
    chacha_ivsetup(&arc4random_rsx.rs_chacha, buf + KEYSZ);
}

static void
_rs_stir(void)
{
    uint8_t  rnd[KEYSZ + IVSZ];
    uint32_t rekey_fuzz = 0;

    memset(rnd, 0, (KEYSZ + IVSZ) * sizeof(u_char));

    if (getentropy(rnd, sizeof rnd) == -1) {
        if (arc4random_abort)
            arc4random_abort();
    }

    if (!arc4random_init) {
        arc4random_init = true;
        _rs_init(rnd, sizeof(rnd));
    } else {
        _rs_rekey(rnd, sizeof(rnd));
    }
    explicit_bzero(rnd, sizeof(rnd)); /* discard source seed */

    /* invalidate rs_buf */
    arc4random_rs.rs_have = 0;
    memset(arc4random_rsx.rs_buf, 0, sizeof(arc4random_rsx.rs_buf));

    /* rekey interval should not be predictable */
    chacha_encrypt_bytes(&arc4random_rsx.rs_chacha, (uint8_t *)&rekey_fuzz, (uint8_t *)&rekey_fuzz,
                         sizeof(rekey_fuzz));
    arc4random_rs.rs_count = REKEY_BASE + (rekey_fuzz % REKEY_BASE);
}

static inline void
_rs_stir_if_needed(size_t len)
{
    if (arc4random_rs.rs_count <= len || (arc4random_fork_detect && arc4random_fork_detect()))
        _rs_stir();
    if (arc4random_rs.rs_count <= len)
        arc4random_rs.rs_count = 0;
    else
        arc4random_rs.rs_count -= len;
}

static inline void
_rs_rekey(unsigned char *dat, size_t datlen)
{
#ifndef KEYSTREAM_ONLY
    memset(arc4random_rsx.rs_buf, 0, sizeof(arc4random_rsx.rs_buf));
#endif
    /* fill rs_buf with the keystream */
    chacha_encrypt_bytes(&arc4random_rsx.rs_chacha, arc4random_rsx.rs_buf, arc4random_rsx.rs_buf,
                         sizeof(arc4random_rsx.rs_buf));
    /* mix in optional user provided data */
    if (dat) {
        size_t i, m;

        m = minimum(datlen, KEYSZ + IVSZ);
        for (i = 0; i < m; i++)
            arc4random_rsx.rs_buf[i] ^= dat[i];
    }
    /* immediately reinit for backtracking resistance */
    _rs_init(arc4random_rsx.rs_buf, KEYSZ + IVSZ);
    memset(arc4random_rsx.rs_buf, 0, KEYSZ + IVSZ);
    arc4random_rs.rs_have = sizeof(arc4random_rsx.rs_buf) - KEYSZ - IVSZ;
}

static inline void
_rs_random_buf(void *_buf, size_t n)
{
    unsigned char *buf = (unsigned char *)_buf;
    unsigned char *keystream;
    size_t         m;

    _rs_stir_if_needed(n);
    while (n > 0) {
        if (arc4random_rs.rs_have > 0) {
            m = minimum(n, arc4random_rs.rs_have);
            keystream
                = arc4random_rsx.rs_buf + sizeof(arc4random_rsx.rs_buf) - arc4random_rs.rs_have;
            memcpy(buf, keystream, m);
            memset(keystream, 0, m);
            buf += m;
            n -= m;
            arc4random_rs.rs_have -= m;
        }
        if (arc4random_rs.rs_have == 0)
            _rs_rekey(NULL, 0);
    }
}

static inline void
_rs_random_u32(uint32_t *val)
{
    unsigned char *keystream;

    _rs_stir_if_needed(sizeof(*val));
    if (arc4random_rs.rs_have < sizeof(*val))
        _rs_rekey(NULL, 0);
    keystream = arc4random_rsx.rs_buf + sizeof(arc4random_rsx.rs_buf) - arc4random_rs.rs_have;
    memcpy(val, keystream, sizeof(*val));
    memset(keystream, 0, sizeof(*val));
    arc4random_rs.rs_have -= sizeof(*val);
}

uint32_t
arc4random(void)
{
    uint32_t val;

    __LIBC_LOCK();
    _rs_random_u32(&val);
    __LIBC_UNLOCK();
    return val;
}

void
arc4random_buf(void *buf, size_t n)
{
    __LIBC_LOCK();
    _rs_random_buf(buf, n);
    __LIBC_UNLOCK();
}
