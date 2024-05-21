/* Memory map for ARC HS Development Kit and ARC HS Development Kit 4xD */

MEMORY
{
    DRAM : ORIGIN = 0x90000000, LENGTH = 0x50000000
}

REGION_ALIAS("startup", DRAM)
REGION_ALIAS("text", DRAM)
REGION_ALIAS("data", DRAM)
REGION_ALIAS("sdata", DRAM)

INCLUDE arcv2elf-common.ld
