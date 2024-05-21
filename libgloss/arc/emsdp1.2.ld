/* Memory map for ARC EM Software Development Platform 1.2 */

MEMORY
{
    ICCM0 : ORIGIN = 0x10000000, LENGTH = 128K
    PSRAM : ORIGIN = 0x40000000, LENGTH =  16M
    DCCM  : ORIGIN = 0x80000000, LENGTH = 128K
}

REGION_ALIAS("startup", ICCM0)
REGION_ALIAS("text", ICCM0)
REGION_ALIAS("data", DCCM)
REGION_ALIAS("sdata", DCCM)

INCLUDE arcv2elf-common.ld
