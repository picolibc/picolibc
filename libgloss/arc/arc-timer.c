/*
 * arc-timer.c -- provide API for ARC timers.
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

#include "arc-specific.h"

#define ARC_TIM_BUILD		0x75
#define ARC_TIM_BUILD_VER_MASK	0x00FF
#define ARC_TIM_BUILD_TIM0_FL	0x0100
#define ARC_TIM_BUILD_TIM1_FL	0x0200

#define ARC_TIM_COUNT0		0x21
#define ARC_TIM_CONTROL0	0x22
#define ARC_TIM_LIMIT0		0x23

#define ARC_TIM_COUNT1		0x100
#define ARC_TIM_CONTROL1	0x101
#define ARC_TIM_LIMIT1		0x102

#define ARC_TIM_CONTROL_NH_FL	0x0002

/* Timer used by '_default' functions.  */
const unsigned int arc_timer_default = 0;

/* Check if given timer exists.  */
static int
_arc_timer_present (unsigned int tim)
{
  unsigned int bcr = read_aux_reg (ARC_TIM_BUILD);
  unsigned int ver = bcr & ARC_TIM_BUILD_VER_MASK;

  if (ver == 0)
    return 0;
  else if (ver == 1)
    return 1;
  else if (tim == 0)
    return ((bcr & ARC_TIM_BUILD_TIM0_FL) != 0);
  else if (tim == 1)
    return ((bcr & ARC_TIM_BUILD_TIM1_FL) != 0);
  else
    return 0;
}

/* Get raw value of a given timer.  */
static unsigned int
_arc_timer_read (unsigned int tim)
{
  if (_arc_timer_present (tim))
    {
      if (tim == 0)
	return read_aux_reg (ARC_TIM_COUNT0);
      else if (tim == 1)
	return read_aux_reg (ARC_TIM_COUNT1);
    }

  return 0;
}

/*
 * Set default values to a given timer.
 * Defaults: Not Halted bit is set, limit is 0xFFFFFFFF, count set to 0.
 */
static void
_arc_timer_reset (unsigned int tim)
{
  unsigned int ctrl, tim_control, tim_count, tim_limit;

  if (_arc_timer_present (tim))
    {
      if (tim == 0)
	{
	  tim_control = ARC_TIM_CONTROL0;
	  tim_count = ARC_TIM_COUNT0;
	  tim_limit = ARC_TIM_LIMIT0;
	}
      else if (tim == 1)
	{
	  tim_control = ARC_TIM_CONTROL1;
	  tim_count = ARC_TIM_COUNT1;
	  tim_limit = ARC_TIM_LIMIT1;
	}
      else
	{
	  return;
	}

      ctrl = read_aux_reg (tim_control);
      /* Disable timer interrupt when programming.  */
      write_aux_reg (0, tim_control);
      /* Default limit is 24-bit, increase it to 32-bit.  */
      write_aux_reg (0xFFFFFFFF, tim_limit);
      /* Set NH bit to count only when processor is running.  */
      write_aux_reg (ctrl | ARC_TIM_CONTROL_NH_FL, tim_control);
      write_aux_reg (0, tim_count);
    }
}


/* Check if arc_timer_default exists.  */
int
_arc_timer_default_present (void)
{
  return _arc_timer_present (arc_timer_default);
}

/* Read arc_timer_default value.  */
unsigned int
_arc_timer_default_read (void)
{
  return _arc_timer_read (arc_timer_default);
}

/* Reset arc_timer_default.  */
void
_arc_timer_default_reset (void)
{
  _arc_timer_reset (arc_timer_default);
}
