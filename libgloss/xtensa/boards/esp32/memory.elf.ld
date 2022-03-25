/*
 * IROM/DRAM definition in QEMU:
 *     [ESP32_MEMREGION_IROM] = { 0x40000000, 0x70000 },
 *     [ESP32_MEMREGION_DRAM] = { 0x3ffae000, 0x52000 },
 *
 * In theory we could use whole DRAM section, but I had some faults when using
 * memory in range 0x3ffae000 - 0x3ffb0000
 *
 * But used memory range for data such as esp-idf for ESP32 to satisfy user's
 * expectation on chip emulation
 *
 * Pass '--defsym=entire_dram_seg=1' to linker script to use whole DRAM
 *
 */

MEMORY
{
  iram_seg  (X) : org = 0x40078000, len = 0x28000
/* 64k at the end of DRAM, after ROM bootloader stack
 * or entire DRAM (for QEMU only)
 */
  dram_seg (RW) : org = DEFINED(entire_dram_seg) ? 0x3FFB0000 : 0x3FFF0000,
                  len = DEFINED(entire_dram_seg) ? 0x50000 : 0x10000
}


