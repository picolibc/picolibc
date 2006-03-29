#include <sys/types.h>
#include "go32.h"
#include "dpmi.h"
#include "dos.h"

u_short _go32_my_cs()
{
  asm("movw %cs,%ax");
}

u_short _go32_my_ds()
{
  asm("movw %ds,%ax");
}

u_short _go32_my_ss()
{
  asm("movw %ss,%ax");
}

u_short _go32_conventional_mem_selector()
{
  return _go32_info_block.selector_for_linear_memory;
}

static _go32_dpmi_registers regs;
static volatile u_long ctrl_break_count = 0;
static int ctrl_break_hooked = 0;
static _go32_dpmi_seginfo old_vector;
static _go32_dpmi_seginfo new_vector;

static ctrl_break_isr(_go32_dpmi_registers *regs)
{
  ctrl_break_count ++;
}

u_long _go32_was_ctrl_break_hit()
{
  u_long cnt;
  _go32_want_ctrl_break(1);
  cnt = ctrl_break_count;
  ctrl_break_count = 0;
  return cnt;
}

void _go32_want_ctrl_break(int yes)
{
  if (yes)
  {
    if (ctrl_break_hooked)
      return;
    _go32_dpmi_get_real_mode_interrupt_vector(0x1b, &old_vector);

    new_vector.pm_offset = (int)ctrl_break_isr;
    _go32_dpmi_allocate_real_mode_callback_iret(&new_vector, &regs);
    _go32_dpmi_set_real_mode_interrupt_vector(0x1b, &new_vector);
    ctrl_break_count = 0;
    ctrl_break_hooked = 1;
  }
  else
  {
    if (!ctrl_break_hooked)
      return;
    _go32_dpmi_set_real_mode_interrupt_vector(0x1b, &old_vector);
    _go32_dpmi_free_real_mode_callback(&new_vector);
    ctrl_break_count = 0;
    ctrl_break_hooked = 0;
  }
}
