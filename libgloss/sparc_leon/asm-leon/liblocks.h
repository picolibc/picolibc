/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef __LEONBARE_LIBLOCKS_H
#define __LEONBARE_LIBLOCKS_H

extern int (*__lbst_pthread_mutex_init) (pthread_mutex_t * __mutex,
					 pthread_mutexattr_t * __mutex_attr);
extern int (*__lbst_pthread_mutex_destroy) (pthread_mutex_t * __mutex);
extern int (*__lbst_pthread_mutex_trylock) (pthread_mutex_t * __mutex);
extern int (*__lbst_pthread_mutex_lock) (pthread_mutex_t * __mutex);
extern int (*__lbst_pthread_mutex_unlock) (pthread_mutex_t * __mutex);
extern int (*__lbst_pthread_mutexattr_init) (pthread_mutexattr_t * __attr);
extern int (*__lbst_pthread_mutexattr_destroy) (pthread_mutexattr_t * __attr);
extern int (*__lbst_pthread_mutexattr_settype) (pthread_mutexattr_t * __attr,
						int __kind);

#endif /* __LEONBARE_LIBLOCKS_H */
