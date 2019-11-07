/* get thread-specific reentrant pointer */

#include <reent.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/* Copied from the HSA documentation.  */
typedef struct hsa_signal_s {
  uint64_t handle;
} hsa_signal_t;
typedef struct hsa_kernel_dispatch_packet_s {
  uint16_t header ;
  uint16_t setup;
  uint16_t workgroup_size_x ;
  uint16_t workgroup_size_y ;
  uint16_t workgroup_size_z;
  uint16_t reserved0;
  uint32_t grid_size_x ;
  uint32_t grid_size_y ;
  uint32_t grid_size_z;
  uint32_t private_segment_size;
  uint32_t group_segment_size;
  uint64_t kernel_object;
  uint64_t reserved2;
  hsa_signal_t completion_signal;
} hsa_kernel_dispatch_packet_t;

struct _reent *
__getreent (void)
{
  /* Place the reent data at the top of the stack allocation.
     s[0:1] contains a 48-bit private segment base address.
     s11 contains the offset to the base of the stack.
     s[4:5] contains the dispatch pointer.
     
     WARNING: this code will break if s[0:1] is ever used for anything!  */
  const register unsigned long buffer_descriptor asm("s0");
  unsigned long private_segment = buffer_descriptor & 0x0000ffffffffffff;
  const register unsigned int stack_offset asm("s11");
  const register hsa_kernel_dispatch_packet_t *dispatch_ptr asm("s4");

  struct data {
    int marker;
    struct _reent reent;
  } *data;

  unsigned long stack_base = private_segment + stack_offset;
  unsigned long stack_end = stack_base + dispatch_ptr->private_segment_size * 64;
  unsigned long addr = (stack_end - sizeof(struct data)) & ~7;
  data = (struct data *)addr;

  register long sp asm("s16");
  if (sp >= addr)
    goto stackoverflow;

  /* Stash a marker in the unused upper 16 bits of s[0:1] to indicate that
     the reent data is initialized.  */
  const register unsigned int s1 asm("s1");
  unsigned int marker = s1 >> 16;
  if (marker != 12345)
    {
      asm("s_and_b32\ts1, s1, 0xffff");
      asm("s_or_b32\ts1, s1, (12345 << 16)");
      data->marker = 12345;

      __builtin_memset (&data->reent, 0, sizeof(struct _reent));
      _REENT_INIT_PTR_ZEROED (&data->reent);
    }
  else if (data->marker != 12345)
    goto stackoverflow;


  return &data->reent;

stackoverflow:
    write (2, "GCN Stack Overflow!\n", 20);
    abort ();
}

