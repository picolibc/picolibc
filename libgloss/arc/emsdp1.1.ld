/* Memory map for ARC EM Software Development Platform 1.0 and 1.1 */

MEMORY
{
    PSRAM : ORIGIN = 0x10000000, LENGTH =  16M
    ICCM0 : ORIGIN = 0x60000000, LENGTH = 128K
    DCCM  : ORIGIN = 0x80000000, LENGTH = 128K
}

REGION_ALIAS("startup", ICCM0)
REGION_ALIAS("text", ICCM0)
REGION_ALIAS("data", DCCM)
REGION_ALIAS("sdata", DCCM)

INCLUDE arcv2elf-common.ld
