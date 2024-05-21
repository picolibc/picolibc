/*
 * arc-timer.h -- provide API for the default (number 0) ARC timer.
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#ifndef _ARC_TIMER_H
#define _ARC_TIMER_H

void _arc_timer_default_reset (void);
int _arc_timer_default_present (void);
unsigned int _arc_timer_default_read (void);

#endif /* !_ARC_TIMER_H  */
