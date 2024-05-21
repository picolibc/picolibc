/* Memory map fo ARC EM Starter Kit v1 - EM4 */

MEMORY
{
    ICCM : ORIGIN = 0x00000000, LENGTH = 128K
    DCCM : ORIGIN = 0x80000000, LENGTH =  64K
}

REGION_ALIAS("startup", ICCM)
REGION_ALIAS("text", ICCM)
REGION_ALIAS("data", DCCM)
REGION_ALIAS("sdata", DCCM)

INCLUDE arcv2elf-common.ld
