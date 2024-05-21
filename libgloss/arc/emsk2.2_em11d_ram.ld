/* Memory map for ARC EM Starter Kit v2.2 and 2.3 - EM11D */

MEMORY
{
    ICCM : ORIGIN = 0x00000000, LENGTH =  64K
    DRAM : ORIGIN = 0x10000000, LENGTH = 128M
    DCCM : ORIGIN = 0x80000000, LENGTH =  64K
}

REGION_ALIAS("startup", ICCM)
REGION_ALIAS("text", DRAM)
REGION_ALIAS("data", DRAM)
REGION_ALIAS("sdata", DRAM)

INCLUDE arcv2elf-common.ld
