/* Memory map for ARC EM Starter Kit v1 - EM6GP */

MEMORY
{
    ICCM : ORIGIN = 0x00000000, LENGTH =  32K
    DRAM : ORIGIN = 0x10000000, LENGTH = 128M
}

REGION_ALIAS("startup", ICCM)
REGION_ALIAS("text", DRAM)
REGION_ALIAS("data", DRAM)
REGION_ALIAS("sdata", DRAM)

INCLUDE arcv2elf-common.ld
