/*
 * IROM/DRAM definition in QEMU:
 *     [ESP32S3_MEMREGION_IROM] = { 0x40000000, 0x60000 },
 *     [ESP32S3_MEMREGION_DRAM] = { 0x3FC80000, 0x70000 },
 *
 * In theory we could use whole DRAM section, but I had some faults when using
 * memory in range 0x3ffae000 - 0x3ffb0000
 *
 * But used memory range for data such as esp-idf for ESP32S3 to satisfy user's
 * expectation on chip emulation
 *
 * Pass '--defsym=entire_dram_seg=1' to linker script to use whole DRAM
 *
 */

MEMORY
{
  iram_seg  (X) : org = 0x40378000, len = 0x70000
  dram_seg (RW) : org = DEFINED(entire_dram_seg) ? 0x3FC80000 : 0x3FCF0000,
                  len = DEFINED(entire_dram_seg) ? 0x70000 : 0x10000
}
