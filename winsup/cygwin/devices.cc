

#include "winsup.h"
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "devices.h"
#include "sys/cygwin.h"
#include "tty.h"
#include "pinfo.h"
typedef const device *KR_device_t;


static KR_device_t KR_find_keyword (const char *KR_keyword, int KR_length);



const device dev_cygdrive_storage =
  {"/cygdrive", FH_CYGDRIVE, "/cygdrive"};

const device dev_fs_storage =
  {"", FH_FS, ""};

const device dev_proc_storage =
  {"", FH_PROC, ""};

const device dev_registry_storage =
  {"", FH_REGISTRY, ""};

const device dev_piper_storage =
  {"", FH_PIPER, ""};

const device dev_pipew_storage =
  {"", FH_PIPEW, ""};

const device dev_tcp_storage =
  {"", FH_TCP, ""};

const device dev_udp_storage =
  {"", FH_UDP, ""};

const device dev_stream_storage =
  {"", FH_STREAM, ""};

const device dev_dgram_storage =
  {"", FH_DGRAM, ""};

const device dev_bad_storage =
  {"", FH_BAD, ""};

static const device dev_storage[] =
{
  {"/dev/clipboard", FH_CLIPBOARD, "\\dev\\clipboard"},
  {"/dev/com0", FHDEV(DEV_SERIAL_MAJOR, 0), "\\\\.\\com0"},
  {"/dev/com1", FHDEV(DEV_SERIAL_MAJOR, 1), "\\\\.\\com1"},
  {"/dev/com2", FHDEV(DEV_SERIAL_MAJOR, 2), "\\\\.\\com2"},
  {"/dev/com3", FHDEV(DEV_SERIAL_MAJOR, 3), "\\\\.\\com3"},
  {"/dev/com4", FHDEV(DEV_SERIAL_MAJOR, 4), "\\\\.\\com4"},
  {"/dev/com5", FHDEV(DEV_SERIAL_MAJOR, 5), "\\\\.\\com5"},
  {"/dev/com6", FHDEV(DEV_SERIAL_MAJOR, 6), "\\\\.\\com6"},
  {"/dev/com7", FHDEV(DEV_SERIAL_MAJOR, 7), "\\\\.\\com7"},
  {"/dev/com8", FHDEV(DEV_SERIAL_MAJOR, 8), "\\\\.\\com8"},
  {"/dev/com9", FHDEV(DEV_SERIAL_MAJOR, 9), "\\\\.\\com9"},
  {"/dev/com10", FHDEV(DEV_SERIAL_MAJOR, 10), "\\\\.\\com10"},
  {"/dev/com11", FHDEV(DEV_SERIAL_MAJOR, 11), "\\\\.\\com11"},
  {"/dev/com12", FHDEV(DEV_SERIAL_MAJOR, 12), "\\\\.\\com12"},
  {"/dev/com13", FHDEV(DEV_SERIAL_MAJOR, 13), "\\\\.\\com13"},
  {"/dev/com14", FHDEV(DEV_SERIAL_MAJOR, 14), "\\\\.\\com14"},
  {"/dev/com15", FHDEV(DEV_SERIAL_MAJOR, 15), "\\\\.\\com15"},
  {"/dev/com16", FHDEV(DEV_SERIAL_MAJOR, 16), "\\\\.\\com16"},
  {"/dev/conin", FH_CONIN, "conin"},
  {"/dev/conout", FH_CONOUT, "conout"},
  {"/dev/console", FH_CONSOLE, "\\dev\\console"},
  {"/dev/dsp", FH_OSS_DSP, "\\dev\\dsp"},
  {"/dev/fd0", FHDEV(DEV_FLOPPY_MAJOR, 0), "\\Device\\Floppy0"},
  {"/dev/fd1", FHDEV(DEV_FLOPPY_MAJOR, 1), "\\Device\\Floppy1"},
  {"/dev/fd2", FHDEV(DEV_FLOPPY_MAJOR, 2), "\\Device\\Floppy2"},
  {"/dev/fd3", FHDEV(DEV_FLOPPY_MAJOR, 3), "\\Device\\Floppy3"},
  {"/dev/fd4", FHDEV(DEV_FLOPPY_MAJOR, 4), "\\Device\\Floppy4"},
  {"/dev/fd5", FHDEV(DEV_FLOPPY_MAJOR, 5), "\\Device\\Floppy5"},
  {"/dev/fd6", FHDEV(DEV_FLOPPY_MAJOR, 6), "\\Device\\Floppy6"},
  {"/dev/fd7", FHDEV(DEV_FLOPPY_MAJOR, 7), "\\Device\\Floppy7"},
  {"/dev/fd8", FHDEV(DEV_FLOPPY_MAJOR, 8), "\\Device\\Floppy8"},
  {"/dev/fd9", FHDEV(DEV_FLOPPY_MAJOR, 9), "\\Device\\Floppy9"},
  {"/dev/fd10", FHDEV(DEV_FLOPPY_MAJOR, 10), "\\Device\\Floppy10"},
  {"/dev/fd11", FHDEV(DEV_FLOPPY_MAJOR, 11), "\\Device\\Floppy11"},
  {"/dev/fd12", FHDEV(DEV_FLOPPY_MAJOR, 12), "\\Device\\Floppy12"},
  {"/dev/fd13", FHDEV(DEV_FLOPPY_MAJOR, 13), "\\Device\\Floppy13"},
  {"/dev/fd14", FHDEV(DEV_FLOPPY_MAJOR, 14), "\\Device\\Floppy14"},
  {"/dev/fd15", FHDEV(DEV_FLOPPY_MAJOR, 15), "\\Device\\Floppy15"},
  {"/dev/fifo", FH_FIFO, "\\dev\\fifo"},
  {"/dev/kmem", FH_KMEM, "\\dev\\mem"},
  {"/dev/mem", FH_MEM, "\\dev\\mem"},
  {"/dev/nst0", FHDEV(DEV_TAPE_MAJOR, 128), "\\Device\\Tape0"},
  {"/dev/nst1", FHDEV(DEV_TAPE_MAJOR, 129), "\\Device\\Tape1"},
  {"/dev/nst2", FHDEV(DEV_TAPE_MAJOR, 130), "\\Device\\Tape2"},
  {"/dev/nst3", FHDEV(DEV_TAPE_MAJOR, 131), "\\Device\\Tape3"},
  {"/dev/nst4", FHDEV(DEV_TAPE_MAJOR, 132), "\\Device\\Tape4"},
  {"/dev/nst5", FHDEV(DEV_TAPE_MAJOR, 133), "\\Device\\Tape5"},
  {"/dev/nst6", FHDEV(DEV_TAPE_MAJOR, 134), "\\Device\\Tape6"},
  {"/dev/nst7", FHDEV(DEV_TAPE_MAJOR, 135), "\\Device\\Tape7"},
  {"/dev/nst8", FHDEV(DEV_TAPE_MAJOR, 136), "\\Device\\Tape8"},
  {"/dev/nst9", FHDEV(DEV_TAPE_MAJOR, 137), "\\Device\\Tape9"},
  {"/dev/nst10", FHDEV(DEV_TAPE_MAJOR, 138), "\\Device\\Tape10"},
  {"/dev/nst11", FHDEV(DEV_TAPE_MAJOR, 139), "\\Device\\Tape11"},
  {"/dev/nst12", FHDEV(DEV_TAPE_MAJOR, 140), "\\Device\\Tape12"},
  {"/dev/nst13", FHDEV(DEV_TAPE_MAJOR, 141), "\\Device\\Tape13"},
  {"/dev/nst14", FHDEV(DEV_TAPE_MAJOR, 142), "\\Device\\Tape14"},
  {"/dev/nst15", FHDEV(DEV_TAPE_MAJOR, 143), "\\Device\\Tape15"},
  {"/dev/nst16", FHDEV(DEV_TAPE_MAJOR, 144), "\\Device\\Tape16"},
  {"/dev/nst17", FHDEV(DEV_TAPE_MAJOR, 145), "\\Device\\Tape17"},
  {"/dev/nst18", FHDEV(DEV_TAPE_MAJOR, 146), "\\Device\\Tape18"},
  {"/dev/nst19", FHDEV(DEV_TAPE_MAJOR, 147), "\\Device\\Tape19"},
  {"/dev/nst20", FHDEV(DEV_TAPE_MAJOR, 148), "\\Device\\Tape20"},
  {"/dev/nst21", FHDEV(DEV_TAPE_MAJOR, 149), "\\Device\\Tape21"},
  {"/dev/nst22", FHDEV(DEV_TAPE_MAJOR, 150), "\\Device\\Tape22"},
  {"/dev/nst23", FHDEV(DEV_TAPE_MAJOR, 151), "\\Device\\Tape23"},
  {"/dev/nst24", FHDEV(DEV_TAPE_MAJOR, 152), "\\Device\\Tape24"},
  {"/dev/nst25", FHDEV(DEV_TAPE_MAJOR, 153), "\\Device\\Tape25"},
  {"/dev/nst26", FHDEV(DEV_TAPE_MAJOR, 154), "\\Device\\Tape26"},
  {"/dev/nst27", FHDEV(DEV_TAPE_MAJOR, 155), "\\Device\\Tape27"},
  {"/dev/nst28", FHDEV(DEV_TAPE_MAJOR, 156), "\\Device\\Tape28"},
  {"/dev/nst29", FHDEV(DEV_TAPE_MAJOR, 157), "\\Device\\Tape29"},
  {"/dev/nst30", FHDEV(DEV_TAPE_MAJOR, 158), "\\Device\\Tape30"},
  {"/dev/nst31", FHDEV(DEV_TAPE_MAJOR, 159), "\\Device\\Tape31"},
  {"/dev/nst32", FHDEV(DEV_TAPE_MAJOR, 160), "\\Device\\Tape32"},
  {"/dev/nst33", FHDEV(DEV_TAPE_MAJOR, 161), "\\Device\\Tape33"},
  {"/dev/nst34", FHDEV(DEV_TAPE_MAJOR, 162), "\\Device\\Tape34"},
  {"/dev/nst35", FHDEV(DEV_TAPE_MAJOR, 163), "\\Device\\Tape35"},
  {"/dev/nst36", FHDEV(DEV_TAPE_MAJOR, 164), "\\Device\\Tape36"},
  {"/dev/nst37", FHDEV(DEV_TAPE_MAJOR, 165), "\\Device\\Tape37"},
  {"/dev/nst38", FHDEV(DEV_TAPE_MAJOR, 166), "\\Device\\Tape38"},
  {"/dev/nst39", FHDEV(DEV_TAPE_MAJOR, 167), "\\Device\\Tape39"},
  {"/dev/nst40", FHDEV(DEV_TAPE_MAJOR, 168), "\\Device\\Tape40"},
  {"/dev/nst41", FHDEV(DEV_TAPE_MAJOR, 169), "\\Device\\Tape41"},
  {"/dev/nst42", FHDEV(DEV_TAPE_MAJOR, 170), "\\Device\\Tape42"},
  {"/dev/nst43", FHDEV(DEV_TAPE_MAJOR, 171), "\\Device\\Tape43"},
  {"/dev/nst44", FHDEV(DEV_TAPE_MAJOR, 172), "\\Device\\Tape44"},
  {"/dev/nst45", FHDEV(DEV_TAPE_MAJOR, 173), "\\Device\\Tape45"},
  {"/dev/nst46", FHDEV(DEV_TAPE_MAJOR, 174), "\\Device\\Tape46"},
  {"/dev/nst47", FHDEV(DEV_TAPE_MAJOR, 175), "\\Device\\Tape47"},
  {"/dev/nst48", FHDEV(DEV_TAPE_MAJOR, 176), "\\Device\\Tape48"},
  {"/dev/nst49", FHDEV(DEV_TAPE_MAJOR, 177), "\\Device\\Tape49"},
  {"/dev/nst50", FHDEV(DEV_TAPE_MAJOR, 178), "\\Device\\Tape50"},
  {"/dev/nst51", FHDEV(DEV_TAPE_MAJOR, 179), "\\Device\\Tape51"},
  {"/dev/nst52", FHDEV(DEV_TAPE_MAJOR, 180), "\\Device\\Tape52"},
  {"/dev/nst53", FHDEV(DEV_TAPE_MAJOR, 181), "\\Device\\Tape53"},
  {"/dev/nst54", FHDEV(DEV_TAPE_MAJOR, 182), "\\Device\\Tape54"},
  {"/dev/nst55", FHDEV(DEV_TAPE_MAJOR, 183), "\\Device\\Tape55"},
  {"/dev/nst56", FHDEV(DEV_TAPE_MAJOR, 184), "\\Device\\Tape56"},
  {"/dev/nst57", FHDEV(DEV_TAPE_MAJOR, 185), "\\Device\\Tape57"},
  {"/dev/nst58", FHDEV(DEV_TAPE_MAJOR, 186), "\\Device\\Tape58"},
  {"/dev/nst59", FHDEV(DEV_TAPE_MAJOR, 187), "\\Device\\Tape59"},
  {"/dev/nst60", FHDEV(DEV_TAPE_MAJOR, 188), "\\Device\\Tape60"},
  {"/dev/nst61", FHDEV(DEV_TAPE_MAJOR, 189), "\\Device\\Tape61"},
  {"/dev/nst62", FHDEV(DEV_TAPE_MAJOR, 190), "\\Device\\Tape62"},
  {"/dev/nst63", FHDEV(DEV_TAPE_MAJOR, 191), "\\Device\\Tape63"},
  {"/dev/nst64", FHDEV(DEV_TAPE_MAJOR, 192), "\\Device\\Tape64"},
  {"/dev/nst65", FHDEV(DEV_TAPE_MAJOR, 193), "\\Device\\Tape65"},
  {"/dev/nst66", FHDEV(DEV_TAPE_MAJOR, 194), "\\Device\\Tape66"},
  {"/dev/nst67", FHDEV(DEV_TAPE_MAJOR, 195), "\\Device\\Tape67"},
  {"/dev/nst68", FHDEV(DEV_TAPE_MAJOR, 196), "\\Device\\Tape68"},
  {"/dev/nst69", FHDEV(DEV_TAPE_MAJOR, 197), "\\Device\\Tape69"},
  {"/dev/nst70", FHDEV(DEV_TAPE_MAJOR, 198), "\\Device\\Tape70"},
  {"/dev/nst71", FHDEV(DEV_TAPE_MAJOR, 199), "\\Device\\Tape71"},
  {"/dev/nst72", FHDEV(DEV_TAPE_MAJOR, 200), "\\Device\\Tape72"},
  {"/dev/nst73", FHDEV(DEV_TAPE_MAJOR, 201), "\\Device\\Tape73"},
  {"/dev/nst74", FHDEV(DEV_TAPE_MAJOR, 202), "\\Device\\Tape74"},
  {"/dev/nst75", FHDEV(DEV_TAPE_MAJOR, 203), "\\Device\\Tape75"},
  {"/dev/nst76", FHDEV(DEV_TAPE_MAJOR, 204), "\\Device\\Tape76"},
  {"/dev/nst77", FHDEV(DEV_TAPE_MAJOR, 205), "\\Device\\Tape77"},
  {"/dev/nst78", FHDEV(DEV_TAPE_MAJOR, 206), "\\Device\\Tape78"},
  {"/dev/nst79", FHDEV(DEV_TAPE_MAJOR, 207), "\\Device\\Tape79"},
  {"/dev/nst80", FHDEV(DEV_TAPE_MAJOR, 208), "\\Device\\Tape80"},
  {"/dev/nst81", FHDEV(DEV_TAPE_MAJOR, 209), "\\Device\\Tape81"},
  {"/dev/nst82", FHDEV(DEV_TAPE_MAJOR, 210), "\\Device\\Tape82"},
  {"/dev/nst83", FHDEV(DEV_TAPE_MAJOR, 211), "\\Device\\Tape83"},
  {"/dev/nst84", FHDEV(DEV_TAPE_MAJOR, 212), "\\Device\\Tape84"},
  {"/dev/nst85", FHDEV(DEV_TAPE_MAJOR, 213), "\\Device\\Tape85"},
  {"/dev/nst86", FHDEV(DEV_TAPE_MAJOR, 214), "\\Device\\Tape86"},
  {"/dev/nst87", FHDEV(DEV_TAPE_MAJOR, 215), "\\Device\\Tape87"},
  {"/dev/nst88", FHDEV(DEV_TAPE_MAJOR, 216), "\\Device\\Tape88"},
  {"/dev/nst89", FHDEV(DEV_TAPE_MAJOR, 217), "\\Device\\Tape89"},
  {"/dev/nst90", FHDEV(DEV_TAPE_MAJOR, 218), "\\Device\\Tape90"},
  {"/dev/nst91", FHDEV(DEV_TAPE_MAJOR, 219), "\\Device\\Tape91"},
  {"/dev/nst92", FHDEV(DEV_TAPE_MAJOR, 220), "\\Device\\Tape92"},
  {"/dev/nst93", FHDEV(DEV_TAPE_MAJOR, 221), "\\Device\\Tape93"},
  {"/dev/nst94", FHDEV(DEV_TAPE_MAJOR, 222), "\\Device\\Tape94"},
  {"/dev/nst95", FHDEV(DEV_TAPE_MAJOR, 223), "\\Device\\Tape95"},
  {"/dev/nst96", FHDEV(DEV_TAPE_MAJOR, 224), "\\Device\\Tape96"},
  {"/dev/nst97", FHDEV(DEV_TAPE_MAJOR, 225), "\\Device\\Tape97"},
  {"/dev/nst98", FHDEV(DEV_TAPE_MAJOR, 226), "\\Device\\Tape98"},
  {"/dev/nst99", FHDEV(DEV_TAPE_MAJOR, 227), "\\Device\\Tape99"},
  {"/dev/nst100", FHDEV(DEV_TAPE_MAJOR, 228), "\\Device\\Tape100"},
  {"/dev/nst101", FHDEV(DEV_TAPE_MAJOR, 229), "\\Device\\Tape101"},
  {"/dev/nst102", FHDEV(DEV_TAPE_MAJOR, 230), "\\Device\\Tape102"},
  {"/dev/nst103", FHDEV(DEV_TAPE_MAJOR, 231), "\\Device\\Tape103"},
  {"/dev/nst104", FHDEV(DEV_TAPE_MAJOR, 232), "\\Device\\Tape104"},
  {"/dev/nst105", FHDEV(DEV_TAPE_MAJOR, 233), "\\Device\\Tape105"},
  {"/dev/nst106", FHDEV(DEV_TAPE_MAJOR, 234), "\\Device\\Tape106"},
  {"/dev/nst107", FHDEV(DEV_TAPE_MAJOR, 235), "\\Device\\Tape107"},
  {"/dev/nst108", FHDEV(DEV_TAPE_MAJOR, 236), "\\Device\\Tape108"},
  {"/dev/nst109", FHDEV(DEV_TAPE_MAJOR, 237), "\\Device\\Tape109"},
  {"/dev/nst110", FHDEV(DEV_TAPE_MAJOR, 238), "\\Device\\Tape110"},
  {"/dev/nst111", FHDEV(DEV_TAPE_MAJOR, 239), "\\Device\\Tape111"},
  {"/dev/nst112", FHDEV(DEV_TAPE_MAJOR, 240), "\\Device\\Tape112"},
  {"/dev/nst113", FHDEV(DEV_TAPE_MAJOR, 241), "\\Device\\Tape113"},
  {"/dev/nst114", FHDEV(DEV_TAPE_MAJOR, 242), "\\Device\\Tape114"},
  {"/dev/nst115", FHDEV(DEV_TAPE_MAJOR, 243), "\\Device\\Tape115"},
  {"/dev/nst116", FHDEV(DEV_TAPE_MAJOR, 244), "\\Device\\Tape116"},
  {"/dev/nst117", FHDEV(DEV_TAPE_MAJOR, 245), "\\Device\\Tape117"},
  {"/dev/nst118", FHDEV(DEV_TAPE_MAJOR, 246), "\\Device\\Tape118"},
  {"/dev/nst119", FHDEV(DEV_TAPE_MAJOR, 247), "\\Device\\Tape119"},
  {"/dev/nst120", FHDEV(DEV_TAPE_MAJOR, 248), "\\Device\\Tape120"},
  {"/dev/nst121", FHDEV(DEV_TAPE_MAJOR, 249), "\\Device\\Tape121"},
  {"/dev/nst122", FHDEV(DEV_TAPE_MAJOR, 250), "\\Device\\Tape122"},
  {"/dev/nst123", FHDEV(DEV_TAPE_MAJOR, 251), "\\Device\\Tape123"},
  {"/dev/nst124", FHDEV(DEV_TAPE_MAJOR, 252), "\\Device\\Tape124"},
  {"/dev/nst125", FHDEV(DEV_TAPE_MAJOR, 253), "\\Device\\Tape125"},
  {"/dev/nst126", FHDEV(DEV_TAPE_MAJOR, 254), "\\Device\\Tape126"},
  {"/dev/nst127", FHDEV(DEV_TAPE_MAJOR, 255), "\\Device\\Tape127"},
  {"/dev/null", FH_NULL, "nul"},
  {"/dev/pipe", FH_PIPE, "\\dev\\pipe"},
  {"/dev/port", FH_PORT, "\\dev\\port"},
  {"/dev/ptmx", FH_PTYM, "\\dev\\ptmx"},
  {"/dev/random", FH_RANDOM, "\\dev\\random"},
  {"/dev/scd0", FHDEV(DEV_CDROM_MAJOR, 0), "\\Device\\CdRom0"},
  {"/dev/scd1", FHDEV(DEV_CDROM_MAJOR, 1), "\\Device\\CdRom1"},
  {"/dev/scd2", FHDEV(DEV_CDROM_MAJOR, 2), "\\Device\\CdRom2"},
  {"/dev/scd3", FHDEV(DEV_CDROM_MAJOR, 3), "\\Device\\CdRom3"},
  {"/dev/scd4", FHDEV(DEV_CDROM_MAJOR, 4), "\\Device\\CdRom4"},
  {"/dev/scd5", FHDEV(DEV_CDROM_MAJOR, 5), "\\Device\\CdRom5"},
  {"/dev/scd6", FHDEV(DEV_CDROM_MAJOR, 6), "\\Device\\CdRom6"},
  {"/dev/scd7", FHDEV(DEV_CDROM_MAJOR, 7), "\\Device\\CdRom7"},
  {"/dev/scd8", FHDEV(DEV_CDROM_MAJOR, 8), "\\Device\\CdRom8"},
  {"/dev/scd9", FHDEV(DEV_CDROM_MAJOR, 9), "\\Device\\CdRom9"},
  {"/dev/scd10", FHDEV(DEV_CDROM_MAJOR, 10), "\\Device\\CdRom10"},
  {"/dev/scd11", FHDEV(DEV_CDROM_MAJOR, 11), "\\Device\\CdRom11"},
  {"/dev/scd12", FHDEV(DEV_CDROM_MAJOR, 12), "\\Device\\CdRom12"},
  {"/dev/scd13", FHDEV(DEV_CDROM_MAJOR, 13), "\\Device\\CdRom13"},
  {"/dev/scd14", FHDEV(DEV_CDROM_MAJOR, 14), "\\Device\\CdRom14"},
  {"/dev/scd15", FHDEV(DEV_CDROM_MAJOR, 15), "\\Device\\CdRom15"},
  {"/dev/sda", FH_SDA, "\\Device\\Harddisk0\\Partition0"},
  {"/dev/sdb", FH_SDB, "\\Device\\Harddisk1\\Partition0"},
  {"/dev/sdc", FH_SDC, "\\Device\\Harddisk2\\Partition0"},
  {"/dev/sdd", FH_SDD, "\\Device\\Harddisk3\\Partition0"},
  {"/dev/sde", FH_SDE, "\\Device\\Harddisk4\\Partition0"},
  {"/dev/sdf", FH_SDF, "\\Device\\Harddisk5\\Partition0"},
  {"/dev/sdg", FH_SDG, "\\Device\\Harddisk6\\Partition0"},
  {"/dev/sdh", FH_SDH, "\\Device\\Harddisk7\\Partition0"},
  {"/dev/sdi", FH_SDI, "\\Device\\Harddisk8\\Partition0"},
  {"/dev/sdj", FH_SDJ, "\\Device\\Harddisk9\\Partition0"},
  {"/dev/sdk", FH_SDK, "\\Device\\Harddisk10\\Partition0"},
  {"/dev/sdl", FH_SDL, "\\Device\\Harddisk11\\Partition0"},
  {"/dev/sdm", FH_SDM, "\\Device\\Harddisk12\\Partition0"},
  {"/dev/sdn", FH_SDN, "\\Device\\Harddisk13\\Partition0"},
  {"/dev/sdo", FH_SDO, "\\Device\\Harddisk14\\Partition0"},
  {"/dev/sdp", FH_SDP, "\\Device\\Harddisk15\\Partition0"},
  {"/dev/sdq", FH_SDQ, "\\Device\\Harddisk16\\Partition0"},
  {"/dev/sdr", FH_SDR, "\\Device\\Harddisk17\\Partition0"},
  {"/dev/sds", FH_SDS, "\\Device\\Harddisk18\\Partition0"},
  {"/dev/sdt", FH_SDT, "\\Device\\Harddisk19\\Partition0"},
  {"/dev/sdu", FH_SDU, "\\Device\\Harddisk20\\Partition0"},
  {"/dev/sdv", FH_SDV, "\\Device\\Harddisk21\\Partition0"},
  {"/dev/sdw", FH_SDW, "\\Device\\Harddisk22\\Partition0"},
  {"/dev/sdx", FH_SDX, "\\Device\\Harddisk23\\Partition0"},
  {"/dev/sdy", FH_SDY, "\\Device\\Harddisk24\\Partition0"},
  {"/dev/sdz", FH_SDZ, "\\Device\\Harddisk25\\Partition0"},
  {"/dev/sda1", FH_SDA | 1, "\\Device\\Harddisk0\\Partition1"},
  {"/dev/sda2", FH_SDA | 2, "\\Device\\Harddisk0\\Partition2"},
  {"/dev/sda3", FH_SDA | 3, "\\Device\\Harddisk0\\Partition3"},
  {"/dev/sda4", FH_SDA | 4, "\\Device\\Harddisk0\\Partition4"},
  {"/dev/sda5", FH_SDA | 5, "\\Device\\Harddisk0\\Partition5"},
  {"/dev/sda6", FH_SDA | 6, "\\Device\\Harddisk0\\Partition6"},
  {"/dev/sda7", FH_SDA | 7, "\\Device\\Harddisk0\\Partition7"},
  {"/dev/sda8", FH_SDA | 8, "\\Device\\Harddisk0\\Partition8"},
  {"/dev/sda9", FH_SDA | 9, "\\Device\\Harddisk0\\Partition9"},
  {"/dev/sda10", FH_SDA | 10, "\\Device\\Harddisk0\\Partition10"},
  {"/dev/sda11", FH_SDA | 11, "\\Device\\Harddisk0\\Partition11"},
  {"/dev/sda12", FH_SDA | 12, "\\Device\\Harddisk0\\Partition12"},
  {"/dev/sda13", FH_SDA | 13, "\\Device\\Harddisk0\\Partition13"},
  {"/dev/sda14", FH_SDA | 14, "\\Device\\Harddisk0\\Partition14"},
  {"/dev/sda15", FH_SDA | 15, "\\Device\\Harddisk0\\Partition15"},
  {"/dev/sdb1", FH_SDB | 1, "\\Device\\Harddisk1\\Partition1"},
  {"/dev/sdb2", FH_SDB | 2, "\\Device\\Harddisk1\\Partition2"},
  {"/dev/sdb3", FH_SDB | 3, "\\Device\\Harddisk1\\Partition3"},
  {"/dev/sdb4", FH_SDB | 4, "\\Device\\Harddisk1\\Partition4"},
  {"/dev/sdb5", FH_SDB | 5, "\\Device\\Harddisk1\\Partition5"},
  {"/dev/sdb6", FH_SDB | 6, "\\Device\\Harddisk1\\Partition6"},
  {"/dev/sdb7", FH_SDB | 7, "\\Device\\Harddisk1\\Partition7"},
  {"/dev/sdb8", FH_SDB | 8, "\\Device\\Harddisk1\\Partition8"},
  {"/dev/sdb9", FH_SDB | 9, "\\Device\\Harddisk1\\Partition9"},
  {"/dev/sdb10", FH_SDB | 10, "\\Device\\Harddisk1\\Partition10"},
  {"/dev/sdb11", FH_SDB | 11, "\\Device\\Harddisk1\\Partition11"},
  {"/dev/sdb12", FH_SDB | 12, "\\Device\\Harddisk1\\Partition12"},
  {"/dev/sdb13", FH_SDB | 13, "\\Device\\Harddisk1\\Partition13"},
  {"/dev/sdb14", FH_SDB | 14, "\\Device\\Harddisk1\\Partition14"},
  {"/dev/sdb15", FH_SDB | 15, "\\Device\\Harddisk1\\Partition15"},
  {"/dev/sdc1", FH_SDC | 1, "\\Device\\Harddisk2\\Partition1"},
  {"/dev/sdc2", FH_SDC | 2, "\\Device\\Harddisk2\\Partition2"},
  {"/dev/sdc3", FH_SDC | 3, "\\Device\\Harddisk2\\Partition3"},
  {"/dev/sdc4", FH_SDC | 4, "\\Device\\Harddisk2\\Partition4"},
  {"/dev/sdc5", FH_SDC | 5, "\\Device\\Harddisk2\\Partition5"},
  {"/dev/sdc6", FH_SDC | 6, "\\Device\\Harddisk2\\Partition6"},
  {"/dev/sdc7", FH_SDC | 7, "\\Device\\Harddisk2\\Partition7"},
  {"/dev/sdc8", FH_SDC | 8, "\\Device\\Harddisk2\\Partition8"},
  {"/dev/sdc9", FH_SDC | 9, "\\Device\\Harddisk2\\Partition9"},
  {"/dev/sdc10", FH_SDC | 10, "\\Device\\Harddisk2\\Partition10"},
  {"/dev/sdc11", FH_SDC | 11, "\\Device\\Harddisk2\\Partition11"},
  {"/dev/sdc12", FH_SDC | 12, "\\Device\\Harddisk2\\Partition12"},
  {"/dev/sdc13", FH_SDC | 13, "\\Device\\Harddisk2\\Partition13"},
  {"/dev/sdc14", FH_SDC | 14, "\\Device\\Harddisk2\\Partition14"},
  {"/dev/sdc15", FH_SDC | 15, "\\Device\\Harddisk2\\Partition15"},
  {"/dev/sdd1", FH_SDD | 1, "\\Device\\Harddisk3\\Partition1"},
  {"/dev/sdd2", FH_SDD | 2, "\\Device\\Harddisk3\\Partition2"},
  {"/dev/sdd3", FH_SDD | 3, "\\Device\\Harddisk3\\Partition3"},
  {"/dev/sdd4", FH_SDD | 4, "\\Device\\Harddisk3\\Partition4"},
  {"/dev/sdd5", FH_SDD | 5, "\\Device\\Harddisk3\\Partition5"},
  {"/dev/sdd6", FH_SDD | 6, "\\Device\\Harddisk3\\Partition6"},
  {"/dev/sdd7", FH_SDD | 7, "\\Device\\Harddisk3\\Partition7"},
  {"/dev/sdd8", FH_SDD | 8, "\\Device\\Harddisk3\\Partition8"},
  {"/dev/sdd9", FH_SDD | 9, "\\Device\\Harddisk3\\Partition9"},
  {"/dev/sdd10", FH_SDD | 10, "\\Device\\Harddisk3\\Partition10"},
  {"/dev/sdd11", FH_SDD | 11, "\\Device\\Harddisk3\\Partition11"},
  {"/dev/sdd12", FH_SDD | 12, "\\Device\\Harddisk3\\Partition12"},
  {"/dev/sdd13", FH_SDD | 13, "\\Device\\Harddisk3\\Partition13"},
  {"/dev/sdd14", FH_SDD | 14, "\\Device\\Harddisk3\\Partition14"},
  {"/dev/sdd15", FH_SDD | 15, "\\Device\\Harddisk3\\Partition15"},
  {"/dev/sde1", FH_SDE | 1, "\\Device\\Harddisk4\\Partition1"},
  {"/dev/sde2", FH_SDE | 2, "\\Device\\Harddisk4\\Partition2"},
  {"/dev/sde3", FH_SDE | 3, "\\Device\\Harddisk4\\Partition3"},
  {"/dev/sde4", FH_SDE | 4, "\\Device\\Harddisk4\\Partition4"},
  {"/dev/sde5", FH_SDE | 5, "\\Device\\Harddisk4\\Partition5"},
  {"/dev/sde6", FH_SDE | 6, "\\Device\\Harddisk4\\Partition6"},
  {"/dev/sde7", FH_SDE | 7, "\\Device\\Harddisk4\\Partition7"},
  {"/dev/sde8", FH_SDE | 8, "\\Device\\Harddisk4\\Partition8"},
  {"/dev/sde9", FH_SDE | 9, "\\Device\\Harddisk4\\Partition9"},
  {"/dev/sde10", FH_SDE | 10, "\\Device\\Harddisk4\\Partition10"},
  {"/dev/sde11", FH_SDE | 11, "\\Device\\Harddisk4\\Partition11"},
  {"/dev/sde12", FH_SDE | 12, "\\Device\\Harddisk4\\Partition12"},
  {"/dev/sde13", FH_SDE | 13, "\\Device\\Harddisk4\\Partition13"},
  {"/dev/sde14", FH_SDE | 14, "\\Device\\Harddisk4\\Partition14"},
  {"/dev/sde15", FH_SDE | 15, "\\Device\\Harddisk4\\Partition15"},
  {"/dev/sdf1", FH_SDF | 1, "\\Device\\Harddisk5\\Partition1"},
  {"/dev/sdf2", FH_SDF | 2, "\\Device\\Harddisk5\\Partition2"},
  {"/dev/sdf3", FH_SDF | 3, "\\Device\\Harddisk5\\Partition3"},
  {"/dev/sdf4", FH_SDF | 4, "\\Device\\Harddisk5\\Partition4"},
  {"/dev/sdf5", FH_SDF | 5, "\\Device\\Harddisk5\\Partition5"},
  {"/dev/sdf6", FH_SDF | 6, "\\Device\\Harddisk5\\Partition6"},
  {"/dev/sdf7", FH_SDF | 7, "\\Device\\Harddisk5\\Partition7"},
  {"/dev/sdf8", FH_SDF | 8, "\\Device\\Harddisk5\\Partition8"},
  {"/dev/sdf9", FH_SDF | 9, "\\Device\\Harddisk5\\Partition9"},
  {"/dev/sdf10", FH_SDF | 10, "\\Device\\Harddisk5\\Partition10"},
  {"/dev/sdf11", FH_SDF | 11, "\\Device\\Harddisk5\\Partition11"},
  {"/dev/sdf12", FH_SDF | 12, "\\Device\\Harddisk5\\Partition12"},
  {"/dev/sdf13", FH_SDF | 13, "\\Device\\Harddisk5\\Partition13"},
  {"/dev/sdf14", FH_SDF | 14, "\\Device\\Harddisk5\\Partition14"},
  {"/dev/sdf15", FH_SDF | 15, "\\Device\\Harddisk5\\Partition15"},
  {"/dev/sdg1", FH_SDG | 1, "\\Device\\Harddisk6\\Partition1"},
  {"/dev/sdg2", FH_SDG | 2, "\\Device\\Harddisk6\\Partition2"},
  {"/dev/sdg3", FH_SDG | 3, "\\Device\\Harddisk6\\Partition3"},
  {"/dev/sdg4", FH_SDG | 4, "\\Device\\Harddisk6\\Partition4"},
  {"/dev/sdg5", FH_SDG | 5, "\\Device\\Harddisk6\\Partition5"},
  {"/dev/sdg6", FH_SDG | 6, "\\Device\\Harddisk6\\Partition6"},
  {"/dev/sdg7", FH_SDG | 7, "\\Device\\Harddisk6\\Partition7"},
  {"/dev/sdg8", FH_SDG | 8, "\\Device\\Harddisk6\\Partition8"},
  {"/dev/sdg9", FH_SDG | 9, "\\Device\\Harddisk6\\Partition9"},
  {"/dev/sdg10", FH_SDG | 10, "\\Device\\Harddisk6\\Partition10"},
  {"/dev/sdg11", FH_SDG | 11, "\\Device\\Harddisk6\\Partition11"},
  {"/dev/sdg12", FH_SDG | 12, "\\Device\\Harddisk6\\Partition12"},
  {"/dev/sdg13", FH_SDG | 13, "\\Device\\Harddisk6\\Partition13"},
  {"/dev/sdg14", FH_SDG | 14, "\\Device\\Harddisk6\\Partition14"},
  {"/dev/sdg15", FH_SDG | 15, "\\Device\\Harddisk6\\Partition15"},
  {"/dev/sdh1", FH_SDH | 1, "\\Device\\Harddisk7\\Partition1"},
  {"/dev/sdh2", FH_SDH | 2, "\\Device\\Harddisk7\\Partition2"},
  {"/dev/sdh3", FH_SDH | 3, "\\Device\\Harddisk7\\Partition3"},
  {"/dev/sdh4", FH_SDH | 4, "\\Device\\Harddisk7\\Partition4"},
  {"/dev/sdh5", FH_SDH | 5, "\\Device\\Harddisk7\\Partition5"},
  {"/dev/sdh6", FH_SDH | 6, "\\Device\\Harddisk7\\Partition6"},
  {"/dev/sdh7", FH_SDH | 7, "\\Device\\Harddisk7\\Partition7"},
  {"/dev/sdh8", FH_SDH | 8, "\\Device\\Harddisk7\\Partition8"},
  {"/dev/sdh9", FH_SDH | 9, "\\Device\\Harddisk7\\Partition9"},
  {"/dev/sdh10", FH_SDH | 10, "\\Device\\Harddisk7\\Partition10"},
  {"/dev/sdh11", FH_SDH | 11, "\\Device\\Harddisk7\\Partition11"},
  {"/dev/sdh12", FH_SDH | 12, "\\Device\\Harddisk7\\Partition12"},
  {"/dev/sdh13", FH_SDH | 13, "\\Device\\Harddisk7\\Partition13"},
  {"/dev/sdh14", FH_SDH | 14, "\\Device\\Harddisk7\\Partition14"},
  {"/dev/sdh15", FH_SDH | 15, "\\Device\\Harddisk7\\Partition15"},
  {"/dev/sdi1", FH_SDI | 1, "\\Device\\Harddisk8\\Partition1"},
  {"/dev/sdi2", FH_SDI | 2, "\\Device\\Harddisk8\\Partition2"},
  {"/dev/sdi3", FH_SDI | 3, "\\Device\\Harddisk8\\Partition3"},
  {"/dev/sdi4", FH_SDI | 4, "\\Device\\Harddisk8\\Partition4"},
  {"/dev/sdi5", FH_SDI | 5, "\\Device\\Harddisk8\\Partition5"},
  {"/dev/sdi6", FH_SDI | 6, "\\Device\\Harddisk8\\Partition6"},
  {"/dev/sdi7", FH_SDI | 7, "\\Device\\Harddisk8\\Partition7"},
  {"/dev/sdi8", FH_SDI | 8, "\\Device\\Harddisk8\\Partition8"},
  {"/dev/sdi9", FH_SDI | 9, "\\Device\\Harddisk8\\Partition9"},
  {"/dev/sdi10", FH_SDI | 10, "\\Device\\Harddisk8\\Partition10"},
  {"/dev/sdi11", FH_SDI | 11, "\\Device\\Harddisk8\\Partition11"},
  {"/dev/sdi12", FH_SDI | 12, "\\Device\\Harddisk8\\Partition12"},
  {"/dev/sdi13", FH_SDI | 13, "\\Device\\Harddisk8\\Partition13"},
  {"/dev/sdi14", FH_SDI | 14, "\\Device\\Harddisk8\\Partition14"},
  {"/dev/sdi15", FH_SDI | 15, "\\Device\\Harddisk8\\Partition15"},
  {"/dev/sdj1", FH_SDJ | 1, "\\Device\\Harddisk9\\Partition1"},
  {"/dev/sdj2", FH_SDJ | 2, "\\Device\\Harddisk9\\Partition2"},
  {"/dev/sdj3", FH_SDJ | 3, "\\Device\\Harddisk9\\Partition3"},
  {"/dev/sdj4", FH_SDJ | 4, "\\Device\\Harddisk9\\Partition4"},
  {"/dev/sdj5", FH_SDJ | 5, "\\Device\\Harddisk9\\Partition5"},
  {"/dev/sdj6", FH_SDJ | 6, "\\Device\\Harddisk9\\Partition6"},
  {"/dev/sdj7", FH_SDJ | 7, "\\Device\\Harddisk9\\Partition7"},
  {"/dev/sdj8", FH_SDJ | 8, "\\Device\\Harddisk9\\Partition8"},
  {"/dev/sdj9", FH_SDJ | 9, "\\Device\\Harddisk9\\Partition9"},
  {"/dev/sdj10", FH_SDJ | 10, "\\Device\\Harddisk9\\Partition10"},
  {"/dev/sdj11", FH_SDJ | 11, "\\Device\\Harddisk9\\Partition11"},
  {"/dev/sdj12", FH_SDJ | 12, "\\Device\\Harddisk9\\Partition12"},
  {"/dev/sdj13", FH_SDJ | 13, "\\Device\\Harddisk9\\Partition13"},
  {"/dev/sdj14", FH_SDJ | 14, "\\Device\\Harddisk9\\Partition14"},
  {"/dev/sdj15", FH_SDJ | 15, "\\Device\\Harddisk9\\Partition15"},
  {"/dev/sdk1", FH_SDK | 1, "\\Device\\Harddisk10\\Partition1"},
  {"/dev/sdk2", FH_SDK | 2, "\\Device\\Harddisk10\\Partition2"},
  {"/dev/sdk3", FH_SDK | 3, "\\Device\\Harddisk10\\Partition3"},
  {"/dev/sdk4", FH_SDK | 4, "\\Device\\Harddisk10\\Partition4"},
  {"/dev/sdk5", FH_SDK | 5, "\\Device\\Harddisk10\\Partition5"},
  {"/dev/sdk6", FH_SDK | 6, "\\Device\\Harddisk10\\Partition6"},
  {"/dev/sdk7", FH_SDK | 7, "\\Device\\Harddisk10\\Partition7"},
  {"/dev/sdk8", FH_SDK | 8, "\\Device\\Harddisk10\\Partition8"},
  {"/dev/sdk9", FH_SDK | 9, "\\Device\\Harddisk10\\Partition9"},
  {"/dev/sdk10", FH_SDK | 10, "\\Device\\Harddisk10\\Partition10"},
  {"/dev/sdk11", FH_SDK | 11, "\\Device\\Harddisk10\\Partition11"},
  {"/dev/sdk12", FH_SDK | 12, "\\Device\\Harddisk10\\Partition12"},
  {"/dev/sdk13", FH_SDK | 13, "\\Device\\Harddisk10\\Partition13"},
  {"/dev/sdk14", FH_SDK | 14, "\\Device\\Harddisk10\\Partition14"},
  {"/dev/sdk15", FH_SDK | 15, "\\Device\\Harddisk10\\Partition15"},
  {"/dev/sdl1", FH_SDL | 1, "\\Device\\Harddisk11\\Partition1"},
  {"/dev/sdl2", FH_SDL | 2, "\\Device\\Harddisk11\\Partition2"},
  {"/dev/sdl3", FH_SDL | 3, "\\Device\\Harddisk11\\Partition3"},
  {"/dev/sdl4", FH_SDL | 4, "\\Device\\Harddisk11\\Partition4"},
  {"/dev/sdl5", FH_SDL | 5, "\\Device\\Harddisk11\\Partition5"},
  {"/dev/sdl6", FH_SDL | 6, "\\Device\\Harddisk11\\Partition6"},
  {"/dev/sdl7", FH_SDL | 7, "\\Device\\Harddisk11\\Partition7"},
  {"/dev/sdl8", FH_SDL | 8, "\\Device\\Harddisk11\\Partition8"},
  {"/dev/sdl9", FH_SDL | 9, "\\Device\\Harddisk11\\Partition9"},
  {"/dev/sdl10", FH_SDL | 10, "\\Device\\Harddisk11\\Partition10"},
  {"/dev/sdl11", FH_SDL | 11, "\\Device\\Harddisk11\\Partition11"},
  {"/dev/sdl12", FH_SDL | 12, "\\Device\\Harddisk11\\Partition12"},
  {"/dev/sdl13", FH_SDL | 13, "\\Device\\Harddisk11\\Partition13"},
  {"/dev/sdl14", FH_SDL | 14, "\\Device\\Harddisk11\\Partition14"},
  {"/dev/sdl15", FH_SDL | 15, "\\Device\\Harddisk11\\Partition15"},
  {"/dev/sdm1", FH_SDM | 1, "\\Device\\Harddisk12\\Partition1"},
  {"/dev/sdm2", FH_SDM | 2, "\\Device\\Harddisk12\\Partition2"},
  {"/dev/sdm3", FH_SDM | 3, "\\Device\\Harddisk12\\Partition3"},
  {"/dev/sdm4", FH_SDM | 4, "\\Device\\Harddisk12\\Partition4"},
  {"/dev/sdm5", FH_SDM | 5, "\\Device\\Harddisk12\\Partition5"},
  {"/dev/sdm6", FH_SDM | 6, "\\Device\\Harddisk12\\Partition6"},
  {"/dev/sdm7", FH_SDM | 7, "\\Device\\Harddisk12\\Partition7"},
  {"/dev/sdm8", FH_SDM | 8, "\\Device\\Harddisk12\\Partition8"},
  {"/dev/sdm9", FH_SDM | 9, "\\Device\\Harddisk12\\Partition9"},
  {"/dev/sdm10", FH_SDM | 10, "\\Device\\Harddisk12\\Partition10"},
  {"/dev/sdm11", FH_SDM | 11, "\\Device\\Harddisk12\\Partition11"},
  {"/dev/sdm12", FH_SDM | 12, "\\Device\\Harddisk12\\Partition12"},
  {"/dev/sdm13", FH_SDM | 13, "\\Device\\Harddisk12\\Partition13"},
  {"/dev/sdm14", FH_SDM | 14, "\\Device\\Harddisk12\\Partition14"},
  {"/dev/sdm15", FH_SDM | 15, "\\Device\\Harddisk12\\Partition15"},
  {"/dev/sdn1", FH_SDN | 1, "\\Device\\Harddisk13\\Partition1"},
  {"/dev/sdn2", FH_SDN | 2, "\\Device\\Harddisk13\\Partition2"},
  {"/dev/sdn3", FH_SDN | 3, "\\Device\\Harddisk13\\Partition3"},
  {"/dev/sdn4", FH_SDN | 4, "\\Device\\Harddisk13\\Partition4"},
  {"/dev/sdn5", FH_SDN | 5, "\\Device\\Harddisk13\\Partition5"},
  {"/dev/sdn6", FH_SDN | 6, "\\Device\\Harddisk13\\Partition6"},
  {"/dev/sdn7", FH_SDN | 7, "\\Device\\Harddisk13\\Partition7"},
  {"/dev/sdn8", FH_SDN | 8, "\\Device\\Harddisk13\\Partition8"},
  {"/dev/sdn9", FH_SDN | 9, "\\Device\\Harddisk13\\Partition9"},
  {"/dev/sdn10", FH_SDN | 10, "\\Device\\Harddisk13\\Partition10"},
  {"/dev/sdn11", FH_SDN | 11, "\\Device\\Harddisk13\\Partition11"},
  {"/dev/sdn12", FH_SDN | 12, "\\Device\\Harddisk13\\Partition12"},
  {"/dev/sdn13", FH_SDN | 13, "\\Device\\Harddisk13\\Partition13"},
  {"/dev/sdn14", FH_SDN | 14, "\\Device\\Harddisk13\\Partition14"},
  {"/dev/sdn15", FH_SDN | 15, "\\Device\\Harddisk13\\Partition15"},
  {"/dev/sdo1", FH_SDO | 1, "\\Device\\Harddisk14\\Partition1"},
  {"/dev/sdo2", FH_SDO | 2, "\\Device\\Harddisk14\\Partition2"},
  {"/dev/sdo3", FH_SDO | 3, "\\Device\\Harddisk14\\Partition3"},
  {"/dev/sdo4", FH_SDO | 4, "\\Device\\Harddisk14\\Partition4"},
  {"/dev/sdo5", FH_SDO | 5, "\\Device\\Harddisk14\\Partition5"},
  {"/dev/sdo6", FH_SDO | 6, "\\Device\\Harddisk14\\Partition6"},
  {"/dev/sdo7", FH_SDO | 7, "\\Device\\Harddisk14\\Partition7"},
  {"/dev/sdo8", FH_SDO | 8, "\\Device\\Harddisk14\\Partition8"},
  {"/dev/sdo9", FH_SDO | 9, "\\Device\\Harddisk14\\Partition9"},
  {"/dev/sdo10", FH_SDO | 10, "\\Device\\Harddisk14\\Partition10"},
  {"/dev/sdo11", FH_SDO | 11, "\\Device\\Harddisk14\\Partition11"},
  {"/dev/sdo12", FH_SDO | 12, "\\Device\\Harddisk14\\Partition12"},
  {"/dev/sdo13", FH_SDO | 13, "\\Device\\Harddisk14\\Partition13"},
  {"/dev/sdo14", FH_SDO | 14, "\\Device\\Harddisk14\\Partition14"},
  {"/dev/sdo15", FH_SDO | 15, "\\Device\\Harddisk14\\Partition15"},
  {"/dev/sdp1", FH_SDP | 1, "\\Device\\Harddisk15\\Partition1"},
  {"/dev/sdp2", FH_SDP | 2, "\\Device\\Harddisk15\\Partition2"},
  {"/dev/sdp3", FH_SDP | 3, "\\Device\\Harddisk15\\Partition3"},
  {"/dev/sdp4", FH_SDP | 4, "\\Device\\Harddisk15\\Partition4"},
  {"/dev/sdp5", FH_SDP | 5, "\\Device\\Harddisk15\\Partition5"},
  {"/dev/sdp6", FH_SDP | 6, "\\Device\\Harddisk15\\Partition6"},
  {"/dev/sdp7", FH_SDP | 7, "\\Device\\Harddisk15\\Partition7"},
  {"/dev/sdp8", FH_SDP | 8, "\\Device\\Harddisk15\\Partition8"},
  {"/dev/sdp9", FH_SDP | 9, "\\Device\\Harddisk15\\Partition9"},
  {"/dev/sdp10", FH_SDP | 10, "\\Device\\Harddisk15\\Partition10"},
  {"/dev/sdp11", FH_SDP | 11, "\\Device\\Harddisk15\\Partition11"},
  {"/dev/sdp12", FH_SDP | 12, "\\Device\\Harddisk15\\Partition12"},
  {"/dev/sdp13", FH_SDP | 13, "\\Device\\Harddisk15\\Partition13"},
  {"/dev/sdp14", FH_SDP | 14, "\\Device\\Harddisk15\\Partition14"},
  {"/dev/sdp15", FH_SDP | 15, "\\Device\\Harddisk15\\Partition15"},
  {"/dev/sdq1", FH_SDQ | 1, "\\Device\\Harddisk16\\Partition1"},
  {"/dev/sdq2", FH_SDQ | 2, "\\Device\\Harddisk16\\Partition2"},
  {"/dev/sdq3", FH_SDQ | 3, "\\Device\\Harddisk16\\Partition3"},
  {"/dev/sdq4", FH_SDQ | 4, "\\Device\\Harddisk16\\Partition4"},
  {"/dev/sdq5", FH_SDQ | 5, "\\Device\\Harddisk16\\Partition5"},
  {"/dev/sdq6", FH_SDQ | 6, "\\Device\\Harddisk16\\Partition6"},
  {"/dev/sdq7", FH_SDQ | 7, "\\Device\\Harddisk16\\Partition7"},
  {"/dev/sdq8", FH_SDQ | 8, "\\Device\\Harddisk16\\Partition8"},
  {"/dev/sdq9", FH_SDQ | 9, "\\Device\\Harddisk16\\Partition9"},
  {"/dev/sdq10", FH_SDQ | 10, "\\Device\\Harddisk16\\Partition10"},
  {"/dev/sdq11", FH_SDQ | 11, "\\Device\\Harddisk16\\Partition11"},
  {"/dev/sdq12", FH_SDQ | 12, "\\Device\\Harddisk16\\Partition12"},
  {"/dev/sdq13", FH_SDQ | 13, "\\Device\\Harddisk16\\Partition13"},
  {"/dev/sdq14", FH_SDQ | 14, "\\Device\\Harddisk16\\Partition14"},
  {"/dev/sdq15", FH_SDQ | 15, "\\Device\\Harddisk16\\Partition15"},
  {"/dev/sdr1", FH_SDR | 1, "\\Device\\Harddisk17\\Partition1"},
  {"/dev/sdr2", FH_SDR | 2, "\\Device\\Harddisk17\\Partition2"},
  {"/dev/sdr3", FH_SDR | 3, "\\Device\\Harddisk17\\Partition3"},
  {"/dev/sdr4", FH_SDR | 4, "\\Device\\Harddisk17\\Partition4"},
  {"/dev/sdr5", FH_SDR | 5, "\\Device\\Harddisk17\\Partition5"},
  {"/dev/sdr6", FH_SDR | 6, "\\Device\\Harddisk17\\Partition6"},
  {"/dev/sdr7", FH_SDR | 7, "\\Device\\Harddisk17\\Partition7"},
  {"/dev/sdr8", FH_SDR | 8, "\\Device\\Harddisk17\\Partition8"},
  {"/dev/sdr9", FH_SDR | 9, "\\Device\\Harddisk17\\Partition9"},
  {"/dev/sdr10", FH_SDR | 10, "\\Device\\Harddisk17\\Partition10"},
  {"/dev/sdr11", FH_SDR | 11, "\\Device\\Harddisk17\\Partition11"},
  {"/dev/sdr12", FH_SDR | 12, "\\Device\\Harddisk17\\Partition12"},
  {"/dev/sdr13", FH_SDR | 13, "\\Device\\Harddisk17\\Partition13"},
  {"/dev/sdr14", FH_SDR | 14, "\\Device\\Harddisk17\\Partition14"},
  {"/dev/sdr15", FH_SDR | 15, "\\Device\\Harddisk17\\Partition15"},
  {"/dev/sds1", FH_SDS | 1, "\\Device\\Harddisk18\\Partition1"},
  {"/dev/sds2", FH_SDS | 2, "\\Device\\Harddisk18\\Partition2"},
  {"/dev/sds3", FH_SDS | 3, "\\Device\\Harddisk18\\Partition3"},
  {"/dev/sds4", FH_SDS | 4, "\\Device\\Harddisk18\\Partition4"},
  {"/dev/sds5", FH_SDS | 5, "\\Device\\Harddisk18\\Partition5"},
  {"/dev/sds6", FH_SDS | 6, "\\Device\\Harddisk18\\Partition6"},
  {"/dev/sds7", FH_SDS | 7, "\\Device\\Harddisk18\\Partition7"},
  {"/dev/sds8", FH_SDS | 8, "\\Device\\Harddisk18\\Partition8"},
  {"/dev/sds9", FH_SDS | 9, "\\Device\\Harddisk18\\Partition9"},
  {"/dev/sds10", FH_SDS | 10, "\\Device\\Harddisk18\\Partition10"},
  {"/dev/sds11", FH_SDS | 11, "\\Device\\Harddisk18\\Partition11"},
  {"/dev/sds12", FH_SDS | 12, "\\Device\\Harddisk18\\Partition12"},
  {"/dev/sds13", FH_SDS | 13, "\\Device\\Harddisk18\\Partition13"},
  {"/dev/sds14", FH_SDS | 14, "\\Device\\Harddisk18\\Partition14"},
  {"/dev/sds15", FH_SDS | 15, "\\Device\\Harddisk18\\Partition15"},
  {"/dev/sdt1", FH_SDT | 1, "\\Device\\Harddisk19\\Partition1"},
  {"/dev/sdt2", FH_SDT | 2, "\\Device\\Harddisk19\\Partition2"},
  {"/dev/sdt3", FH_SDT | 3, "\\Device\\Harddisk19\\Partition3"},
  {"/dev/sdt4", FH_SDT | 4, "\\Device\\Harddisk19\\Partition4"},
  {"/dev/sdt5", FH_SDT | 5, "\\Device\\Harddisk19\\Partition5"},
  {"/dev/sdt6", FH_SDT | 6, "\\Device\\Harddisk19\\Partition6"},
  {"/dev/sdt7", FH_SDT | 7, "\\Device\\Harddisk19\\Partition7"},
  {"/dev/sdt8", FH_SDT | 8, "\\Device\\Harddisk19\\Partition8"},
  {"/dev/sdt9", FH_SDT | 9, "\\Device\\Harddisk19\\Partition9"},
  {"/dev/sdt10", FH_SDT | 10, "\\Device\\Harddisk19\\Partition10"},
  {"/dev/sdt11", FH_SDT | 11, "\\Device\\Harddisk19\\Partition11"},
  {"/dev/sdt12", FH_SDT | 12, "\\Device\\Harddisk19\\Partition12"},
  {"/dev/sdt13", FH_SDT | 13, "\\Device\\Harddisk19\\Partition13"},
  {"/dev/sdt14", FH_SDT | 14, "\\Device\\Harddisk19\\Partition14"},
  {"/dev/sdt15", FH_SDT | 15, "\\Device\\Harddisk19\\Partition15"},
  {"/dev/sdu1", FH_SDU | 1, "\\Device\\Harddisk20\\Partition1"},
  {"/dev/sdu2", FH_SDU | 2, "\\Device\\Harddisk20\\Partition2"},
  {"/dev/sdu3", FH_SDU | 3, "\\Device\\Harddisk20\\Partition3"},
  {"/dev/sdu4", FH_SDU | 4, "\\Device\\Harddisk20\\Partition4"},
  {"/dev/sdu5", FH_SDU | 5, "\\Device\\Harddisk20\\Partition5"},
  {"/dev/sdu6", FH_SDU | 6, "\\Device\\Harddisk20\\Partition6"},
  {"/dev/sdu7", FH_SDU | 7, "\\Device\\Harddisk20\\Partition7"},
  {"/dev/sdu8", FH_SDU | 8, "\\Device\\Harddisk20\\Partition8"},
  {"/dev/sdu9", FH_SDU | 9, "\\Device\\Harddisk20\\Partition9"},
  {"/dev/sdu10", FH_SDU | 10, "\\Device\\Harddisk20\\Partition10"},
  {"/dev/sdu11", FH_SDU | 11, "\\Device\\Harddisk20\\Partition11"},
  {"/dev/sdu12", FH_SDU | 12, "\\Device\\Harddisk20\\Partition12"},
  {"/dev/sdu13", FH_SDU | 13, "\\Device\\Harddisk20\\Partition13"},
  {"/dev/sdu14", FH_SDU | 14, "\\Device\\Harddisk20\\Partition14"},
  {"/dev/sdu15", FH_SDU | 15, "\\Device\\Harddisk20\\Partition15"},
  {"/dev/sdv1", FH_SDV | 1, "\\Device\\Harddisk21\\Partition1"},
  {"/dev/sdv2", FH_SDV | 2, "\\Device\\Harddisk21\\Partition2"},
  {"/dev/sdv3", FH_SDV | 3, "\\Device\\Harddisk21\\Partition3"},
  {"/dev/sdv4", FH_SDV | 4, "\\Device\\Harddisk21\\Partition4"},
  {"/dev/sdv5", FH_SDV | 5, "\\Device\\Harddisk21\\Partition5"},
  {"/dev/sdv6", FH_SDV | 6, "\\Device\\Harddisk21\\Partition6"},
  {"/dev/sdv7", FH_SDV | 7, "\\Device\\Harddisk21\\Partition7"},
  {"/dev/sdv8", FH_SDV | 8, "\\Device\\Harddisk21\\Partition8"},
  {"/dev/sdv9", FH_SDV | 9, "\\Device\\Harddisk21\\Partition9"},
  {"/dev/sdv10", FH_SDV | 10, "\\Device\\Harddisk21\\Partition10"},
  {"/dev/sdv11", FH_SDV | 11, "\\Device\\Harddisk21\\Partition11"},
  {"/dev/sdv12", FH_SDV | 12, "\\Device\\Harddisk21\\Partition12"},
  {"/dev/sdv13", FH_SDV | 13, "\\Device\\Harddisk21\\Partition13"},
  {"/dev/sdv14", FH_SDV | 14, "\\Device\\Harddisk21\\Partition14"},
  {"/dev/sdv15", FH_SDV | 15, "\\Device\\Harddisk21\\Partition15"},
  {"/dev/sdw1", FH_SDW | 1, "\\Device\\Harddisk22\\Partition1"},
  {"/dev/sdw2", FH_SDW | 2, "\\Device\\Harddisk22\\Partition2"},
  {"/dev/sdw3", FH_SDW | 3, "\\Device\\Harddisk22\\Partition3"},
  {"/dev/sdw4", FH_SDW | 4, "\\Device\\Harddisk22\\Partition4"},
  {"/dev/sdw5", FH_SDW | 5, "\\Device\\Harddisk22\\Partition5"},
  {"/dev/sdw6", FH_SDW | 6, "\\Device\\Harddisk22\\Partition6"},
  {"/dev/sdw7", FH_SDW | 7, "\\Device\\Harddisk22\\Partition7"},
  {"/dev/sdw8", FH_SDW | 8, "\\Device\\Harddisk22\\Partition8"},
  {"/dev/sdw9", FH_SDW | 9, "\\Device\\Harddisk22\\Partition9"},
  {"/dev/sdw10", FH_SDW | 10, "\\Device\\Harddisk22\\Partition10"},
  {"/dev/sdw11", FH_SDW | 11, "\\Device\\Harddisk22\\Partition11"},
  {"/dev/sdw12", FH_SDW | 12, "\\Device\\Harddisk22\\Partition12"},
  {"/dev/sdw13", FH_SDW | 13, "\\Device\\Harddisk22\\Partition13"},
  {"/dev/sdw14", FH_SDW | 14, "\\Device\\Harddisk22\\Partition14"},
  {"/dev/sdw15", FH_SDW | 15, "\\Device\\Harddisk22\\Partition15"},
  {"/dev/sdx1", FH_SDX | 1, "\\Device\\Harddisk23\\Partition1"},
  {"/dev/sdx2", FH_SDX | 2, "\\Device\\Harddisk23\\Partition2"},
  {"/dev/sdx3", FH_SDX | 3, "\\Device\\Harddisk23\\Partition3"},
  {"/dev/sdx4", FH_SDX | 4, "\\Device\\Harddisk23\\Partition4"},
  {"/dev/sdx5", FH_SDX | 5, "\\Device\\Harddisk23\\Partition5"},
  {"/dev/sdx6", FH_SDX | 6, "\\Device\\Harddisk23\\Partition6"},
  {"/dev/sdx7", FH_SDX | 7, "\\Device\\Harddisk23\\Partition7"},
  {"/dev/sdx8", FH_SDX | 8, "\\Device\\Harddisk23\\Partition8"},
  {"/dev/sdx9", FH_SDX | 9, "\\Device\\Harddisk23\\Partition9"},
  {"/dev/sdx10", FH_SDX | 10, "\\Device\\Harddisk23\\Partition10"},
  {"/dev/sdx11", FH_SDX | 11, "\\Device\\Harddisk23\\Partition11"},
  {"/dev/sdx12", FH_SDX | 12, "\\Device\\Harddisk23\\Partition12"},
  {"/dev/sdx13", FH_SDX | 13, "\\Device\\Harddisk23\\Partition13"},
  {"/dev/sdx14", FH_SDX | 14, "\\Device\\Harddisk23\\Partition14"},
  {"/dev/sdx15", FH_SDX | 15, "\\Device\\Harddisk23\\Partition15"},
  {"/dev/sdy1", FH_SDY | 1, "\\Device\\Harddisk24\\Partition1"},
  {"/dev/sdy2", FH_SDY | 2, "\\Device\\Harddisk24\\Partition2"},
  {"/dev/sdy3", FH_SDY | 3, "\\Device\\Harddisk24\\Partition3"},
  {"/dev/sdy4", FH_SDY | 4, "\\Device\\Harddisk24\\Partition4"},
  {"/dev/sdy5", FH_SDY | 5, "\\Device\\Harddisk24\\Partition5"},
  {"/dev/sdy6", FH_SDY | 6, "\\Device\\Harddisk24\\Partition6"},
  {"/dev/sdy7", FH_SDY | 7, "\\Device\\Harddisk24\\Partition7"},
  {"/dev/sdy8", FH_SDY | 8, "\\Device\\Harddisk24\\Partition8"},
  {"/dev/sdy9", FH_SDY | 9, "\\Device\\Harddisk24\\Partition9"},
  {"/dev/sdy10", FH_SDY | 10, "\\Device\\Harddisk24\\Partition10"},
  {"/dev/sdy11", FH_SDY | 11, "\\Device\\Harddisk24\\Partition11"},
  {"/dev/sdy12", FH_SDY | 12, "\\Device\\Harddisk24\\Partition12"},
  {"/dev/sdy13", FH_SDY | 13, "\\Device\\Harddisk24\\Partition13"},
  {"/dev/sdy14", FH_SDY | 14, "\\Device\\Harddisk24\\Partition14"},
  {"/dev/sdy15", FH_SDY | 15, "\\Device\\Harddisk24\\Partition15"},
  {"/dev/sdz1", FH_SDZ | 1, "\\Device\\Harddisk25\\Partition1"},
  {"/dev/sdz2", FH_SDZ | 2, "\\Device\\Harddisk25\\Partition2"},
  {"/dev/sdz3", FH_SDZ | 3, "\\Device\\Harddisk25\\Partition3"},
  {"/dev/sdz4", FH_SDZ | 4, "\\Device\\Harddisk25\\Partition4"},
  {"/dev/sdz5", FH_SDZ | 5, "\\Device\\Harddisk25\\Partition5"},
  {"/dev/sdz6", FH_SDZ | 6, "\\Device\\Harddisk25\\Partition6"},
  {"/dev/sdz7", FH_SDZ | 7, "\\Device\\Harddisk25\\Partition7"},
  {"/dev/sdz8", FH_SDZ | 8, "\\Device\\Harddisk25\\Partition8"},
  {"/dev/sdz9", FH_SDZ | 9, "\\Device\\Harddisk25\\Partition9"},
  {"/dev/sdz10", FH_SDZ | 10, "\\Device\\Harddisk25\\Partition10"},
  {"/dev/sdz11", FH_SDZ | 11, "\\Device\\Harddisk25\\Partition11"},
  {"/dev/sdz12", FH_SDZ | 12, "\\Device\\Harddisk25\\Partition12"},
  {"/dev/sdz13", FH_SDZ | 13, "\\Device\\Harddisk25\\Partition13"},
  {"/dev/sdz14", FH_SDZ | 14, "\\Device\\Harddisk25\\Partition14"},
  {"/dev/sdz15", FH_SDZ | 15, "\\Device\\Harddisk25\\Partition15"},
  {"/dev/sr0", FHDEV(DEV_CDROM_MAJOR, 0), "\\Device\\CdRom0"},
  {"/dev/sr1", FHDEV(DEV_CDROM_MAJOR, 1), "\\Device\\CdRom1"},
  {"/dev/sr2", FHDEV(DEV_CDROM_MAJOR, 2), "\\Device\\CdRom2"},
  {"/dev/sr3", FHDEV(DEV_CDROM_MAJOR, 3), "\\Device\\CdRom3"},
  {"/dev/sr4", FHDEV(DEV_CDROM_MAJOR, 4), "\\Device\\CdRom4"},
  {"/dev/sr5", FHDEV(DEV_CDROM_MAJOR, 5), "\\Device\\CdRom5"},
  {"/dev/sr6", FHDEV(DEV_CDROM_MAJOR, 6), "\\Device\\CdRom6"},
  {"/dev/sr7", FHDEV(DEV_CDROM_MAJOR, 7), "\\Device\\CdRom7"},
  {"/dev/sr8", FHDEV(DEV_CDROM_MAJOR, 8), "\\Device\\CdRom8"},
  {"/dev/sr9", FHDEV(DEV_CDROM_MAJOR, 9), "\\Device\\CdRom9"},
  {"/dev/sr10", FHDEV(DEV_CDROM_MAJOR, 10), "\\Device\\CdRom10"},
  {"/dev/sr11", FHDEV(DEV_CDROM_MAJOR, 11), "\\Device\\CdRom11"},
  {"/dev/sr12", FHDEV(DEV_CDROM_MAJOR, 12), "\\Device\\CdRom12"},
  {"/dev/sr13", FHDEV(DEV_CDROM_MAJOR, 13), "\\Device\\CdRom13"},
  {"/dev/sr14", FHDEV(DEV_CDROM_MAJOR, 14), "\\Device\\CdRom14"},
  {"/dev/sr15", FHDEV(DEV_CDROM_MAJOR, 15), "\\Device\\CdRom15"},
  {"/dev/st0", FHDEV(DEV_TAPE_MAJOR, 0), "\\Device\\Tape0"},
  {"/dev/st1", FHDEV(DEV_TAPE_MAJOR, 1), "\\Device\\Tape1"},
  {"/dev/st2", FHDEV(DEV_TAPE_MAJOR, 2), "\\Device\\Tape2"},
  {"/dev/st3", FHDEV(DEV_TAPE_MAJOR, 3), "\\Device\\Tape3"},
  {"/dev/st4", FHDEV(DEV_TAPE_MAJOR, 4), "\\Device\\Tape4"},
  {"/dev/st5", FHDEV(DEV_TAPE_MAJOR, 5), "\\Device\\Tape5"},
  {"/dev/st6", FHDEV(DEV_TAPE_MAJOR, 6), "\\Device\\Tape6"},
  {"/dev/st7", FHDEV(DEV_TAPE_MAJOR, 7), "\\Device\\Tape7"},
  {"/dev/st8", FHDEV(DEV_TAPE_MAJOR, 8), "\\Device\\Tape8"},
  {"/dev/st9", FHDEV(DEV_TAPE_MAJOR, 9), "\\Device\\Tape9"},
  {"/dev/st10", FHDEV(DEV_TAPE_MAJOR, 10), "\\Device\\Tape10"},
  {"/dev/st11", FHDEV(DEV_TAPE_MAJOR, 11), "\\Device\\Tape11"},
  {"/dev/st12", FHDEV(DEV_TAPE_MAJOR, 12), "\\Device\\Tape12"},
  {"/dev/st13", FHDEV(DEV_TAPE_MAJOR, 13), "\\Device\\Tape13"},
  {"/dev/st14", FHDEV(DEV_TAPE_MAJOR, 14), "\\Device\\Tape14"},
  {"/dev/st15", FHDEV(DEV_TAPE_MAJOR, 15), "\\Device\\Tape15"},
  {"/dev/st16", FHDEV(DEV_TAPE_MAJOR, 16), "\\Device\\Tape16"},
  {"/dev/st17", FHDEV(DEV_TAPE_MAJOR, 17), "\\Device\\Tape17"},
  {"/dev/st18", FHDEV(DEV_TAPE_MAJOR, 18), "\\Device\\Tape18"},
  {"/dev/st19", FHDEV(DEV_TAPE_MAJOR, 19), "\\Device\\Tape19"},
  {"/dev/st20", FHDEV(DEV_TAPE_MAJOR, 20), "\\Device\\Tape20"},
  {"/dev/st21", FHDEV(DEV_TAPE_MAJOR, 21), "\\Device\\Tape21"},
  {"/dev/st22", FHDEV(DEV_TAPE_MAJOR, 22), "\\Device\\Tape22"},
  {"/dev/st23", FHDEV(DEV_TAPE_MAJOR, 23), "\\Device\\Tape23"},
  {"/dev/st24", FHDEV(DEV_TAPE_MAJOR, 24), "\\Device\\Tape24"},
  {"/dev/st25", FHDEV(DEV_TAPE_MAJOR, 25), "\\Device\\Tape25"},
  {"/dev/st26", FHDEV(DEV_TAPE_MAJOR, 26), "\\Device\\Tape26"},
  {"/dev/st27", FHDEV(DEV_TAPE_MAJOR, 27), "\\Device\\Tape27"},
  {"/dev/st28", FHDEV(DEV_TAPE_MAJOR, 28), "\\Device\\Tape28"},
  {"/dev/st29", FHDEV(DEV_TAPE_MAJOR, 29), "\\Device\\Tape29"},
  {"/dev/st30", FHDEV(DEV_TAPE_MAJOR, 30), "\\Device\\Tape30"},
  {"/dev/st31", FHDEV(DEV_TAPE_MAJOR, 31), "\\Device\\Tape31"},
  {"/dev/st32", FHDEV(DEV_TAPE_MAJOR, 32), "\\Device\\Tape32"},
  {"/dev/st33", FHDEV(DEV_TAPE_MAJOR, 33), "\\Device\\Tape33"},
  {"/dev/st34", FHDEV(DEV_TAPE_MAJOR, 34), "\\Device\\Tape34"},
  {"/dev/st35", FHDEV(DEV_TAPE_MAJOR, 35), "\\Device\\Tape35"},
  {"/dev/st36", FHDEV(DEV_TAPE_MAJOR, 36), "\\Device\\Tape36"},
  {"/dev/st37", FHDEV(DEV_TAPE_MAJOR, 37), "\\Device\\Tape37"},
  {"/dev/st38", FHDEV(DEV_TAPE_MAJOR, 38), "\\Device\\Tape38"},
  {"/dev/st39", FHDEV(DEV_TAPE_MAJOR, 39), "\\Device\\Tape39"},
  {"/dev/st40", FHDEV(DEV_TAPE_MAJOR, 40), "\\Device\\Tape40"},
  {"/dev/st41", FHDEV(DEV_TAPE_MAJOR, 41), "\\Device\\Tape41"},
  {"/dev/st42", FHDEV(DEV_TAPE_MAJOR, 42), "\\Device\\Tape42"},
  {"/dev/st43", FHDEV(DEV_TAPE_MAJOR, 43), "\\Device\\Tape43"},
  {"/dev/st44", FHDEV(DEV_TAPE_MAJOR, 44), "\\Device\\Tape44"},
  {"/dev/st45", FHDEV(DEV_TAPE_MAJOR, 45), "\\Device\\Tape45"},
  {"/dev/st46", FHDEV(DEV_TAPE_MAJOR, 46), "\\Device\\Tape46"},
  {"/dev/st47", FHDEV(DEV_TAPE_MAJOR, 47), "\\Device\\Tape47"},
  {"/dev/st48", FHDEV(DEV_TAPE_MAJOR, 48), "\\Device\\Tape48"},
  {"/dev/st49", FHDEV(DEV_TAPE_MAJOR, 49), "\\Device\\Tape49"},
  {"/dev/st50", FHDEV(DEV_TAPE_MAJOR, 50), "\\Device\\Tape50"},
  {"/dev/st51", FHDEV(DEV_TAPE_MAJOR, 51), "\\Device\\Tape51"},
  {"/dev/st52", FHDEV(DEV_TAPE_MAJOR, 52), "\\Device\\Tape52"},
  {"/dev/st53", FHDEV(DEV_TAPE_MAJOR, 53), "\\Device\\Tape53"},
  {"/dev/st54", FHDEV(DEV_TAPE_MAJOR, 54), "\\Device\\Tape54"},
  {"/dev/st55", FHDEV(DEV_TAPE_MAJOR, 55), "\\Device\\Tape55"},
  {"/dev/st56", FHDEV(DEV_TAPE_MAJOR, 56), "\\Device\\Tape56"},
  {"/dev/st57", FHDEV(DEV_TAPE_MAJOR, 57), "\\Device\\Tape57"},
  {"/dev/st58", FHDEV(DEV_TAPE_MAJOR, 58), "\\Device\\Tape58"},
  {"/dev/st59", FHDEV(DEV_TAPE_MAJOR, 59), "\\Device\\Tape59"},
  {"/dev/st60", FHDEV(DEV_TAPE_MAJOR, 60), "\\Device\\Tape60"},
  {"/dev/st61", FHDEV(DEV_TAPE_MAJOR, 61), "\\Device\\Tape61"},
  {"/dev/st62", FHDEV(DEV_TAPE_MAJOR, 62), "\\Device\\Tape62"},
  {"/dev/st63", FHDEV(DEV_TAPE_MAJOR, 63), "\\Device\\Tape63"},
  {"/dev/st64", FHDEV(DEV_TAPE_MAJOR, 64), "\\Device\\Tape64"},
  {"/dev/st65", FHDEV(DEV_TAPE_MAJOR, 65), "\\Device\\Tape65"},
  {"/dev/st66", FHDEV(DEV_TAPE_MAJOR, 66), "\\Device\\Tape66"},
  {"/dev/st67", FHDEV(DEV_TAPE_MAJOR, 67), "\\Device\\Tape67"},
  {"/dev/st68", FHDEV(DEV_TAPE_MAJOR, 68), "\\Device\\Tape68"},
  {"/dev/st69", FHDEV(DEV_TAPE_MAJOR, 69), "\\Device\\Tape69"},
  {"/dev/st70", FHDEV(DEV_TAPE_MAJOR, 70), "\\Device\\Tape70"},
  {"/dev/st71", FHDEV(DEV_TAPE_MAJOR, 71), "\\Device\\Tape71"},
  {"/dev/st72", FHDEV(DEV_TAPE_MAJOR, 72), "\\Device\\Tape72"},
  {"/dev/st73", FHDEV(DEV_TAPE_MAJOR, 73), "\\Device\\Tape73"},
  {"/dev/st74", FHDEV(DEV_TAPE_MAJOR, 74), "\\Device\\Tape74"},
  {"/dev/st75", FHDEV(DEV_TAPE_MAJOR, 75), "\\Device\\Tape75"},
  {"/dev/st76", FHDEV(DEV_TAPE_MAJOR, 76), "\\Device\\Tape76"},
  {"/dev/st77", FHDEV(DEV_TAPE_MAJOR, 77), "\\Device\\Tape77"},
  {"/dev/st78", FHDEV(DEV_TAPE_MAJOR, 78), "\\Device\\Tape78"},
  {"/dev/st79", FHDEV(DEV_TAPE_MAJOR, 79), "\\Device\\Tape79"},
  {"/dev/st80", FHDEV(DEV_TAPE_MAJOR, 80), "\\Device\\Tape80"},
  {"/dev/st81", FHDEV(DEV_TAPE_MAJOR, 81), "\\Device\\Tape81"},
  {"/dev/st82", FHDEV(DEV_TAPE_MAJOR, 82), "\\Device\\Tape82"},
  {"/dev/st83", FHDEV(DEV_TAPE_MAJOR, 83), "\\Device\\Tape83"},
  {"/dev/st84", FHDEV(DEV_TAPE_MAJOR, 84), "\\Device\\Tape84"},
  {"/dev/st85", FHDEV(DEV_TAPE_MAJOR, 85), "\\Device\\Tape85"},
  {"/dev/st86", FHDEV(DEV_TAPE_MAJOR, 86), "\\Device\\Tape86"},
  {"/dev/st87", FHDEV(DEV_TAPE_MAJOR, 87), "\\Device\\Tape87"},
  {"/dev/st88", FHDEV(DEV_TAPE_MAJOR, 88), "\\Device\\Tape88"},
  {"/dev/st89", FHDEV(DEV_TAPE_MAJOR, 89), "\\Device\\Tape89"},
  {"/dev/st90", FHDEV(DEV_TAPE_MAJOR, 90), "\\Device\\Tape90"},
  {"/dev/st91", FHDEV(DEV_TAPE_MAJOR, 91), "\\Device\\Tape91"},
  {"/dev/st92", FHDEV(DEV_TAPE_MAJOR, 92), "\\Device\\Tape92"},
  {"/dev/st93", FHDEV(DEV_TAPE_MAJOR, 93), "\\Device\\Tape93"},
  {"/dev/st94", FHDEV(DEV_TAPE_MAJOR, 94), "\\Device\\Tape94"},
  {"/dev/st95", FHDEV(DEV_TAPE_MAJOR, 95), "\\Device\\Tape95"},
  {"/dev/st96", FHDEV(DEV_TAPE_MAJOR, 96), "\\Device\\Tape96"},
  {"/dev/st97", FHDEV(DEV_TAPE_MAJOR, 97), "\\Device\\Tape97"},
  {"/dev/st98", FHDEV(DEV_TAPE_MAJOR, 98), "\\Device\\Tape98"},
  {"/dev/st99", FHDEV(DEV_TAPE_MAJOR, 99), "\\Device\\Tape99"},
  {"/dev/st100", FHDEV(DEV_TAPE_MAJOR, 100), "\\Device\\Tape100"},
  {"/dev/st101", FHDEV(DEV_TAPE_MAJOR, 101), "\\Device\\Tape101"},
  {"/dev/st102", FHDEV(DEV_TAPE_MAJOR, 102), "\\Device\\Tape102"},
  {"/dev/st103", FHDEV(DEV_TAPE_MAJOR, 103), "\\Device\\Tape103"},
  {"/dev/st104", FHDEV(DEV_TAPE_MAJOR, 104), "\\Device\\Tape104"},
  {"/dev/st105", FHDEV(DEV_TAPE_MAJOR, 105), "\\Device\\Tape105"},
  {"/dev/st106", FHDEV(DEV_TAPE_MAJOR, 106), "\\Device\\Tape106"},
  {"/dev/st107", FHDEV(DEV_TAPE_MAJOR, 107), "\\Device\\Tape107"},
  {"/dev/st108", FHDEV(DEV_TAPE_MAJOR, 108), "\\Device\\Tape108"},
  {"/dev/st109", FHDEV(DEV_TAPE_MAJOR, 109), "\\Device\\Tape109"},
  {"/dev/st110", FHDEV(DEV_TAPE_MAJOR, 110), "\\Device\\Tape110"},
  {"/dev/st111", FHDEV(DEV_TAPE_MAJOR, 111), "\\Device\\Tape111"},
  {"/dev/st112", FHDEV(DEV_TAPE_MAJOR, 112), "\\Device\\Tape112"},
  {"/dev/st113", FHDEV(DEV_TAPE_MAJOR, 113), "\\Device\\Tape113"},
  {"/dev/st114", FHDEV(DEV_TAPE_MAJOR, 114), "\\Device\\Tape114"},
  {"/dev/st115", FHDEV(DEV_TAPE_MAJOR, 115), "\\Device\\Tape115"},
  {"/dev/st116", FHDEV(DEV_TAPE_MAJOR, 116), "\\Device\\Tape116"},
  {"/dev/st117", FHDEV(DEV_TAPE_MAJOR, 117), "\\Device\\Tape117"},
  {"/dev/st118", FHDEV(DEV_TAPE_MAJOR, 118), "\\Device\\Tape118"},
  {"/dev/st119", FHDEV(DEV_TAPE_MAJOR, 119), "\\Device\\Tape119"},
  {"/dev/st120", FHDEV(DEV_TAPE_MAJOR, 120), "\\Device\\Tape120"},
  {"/dev/st121", FHDEV(DEV_TAPE_MAJOR, 121), "\\Device\\Tape121"},
  {"/dev/st122", FHDEV(DEV_TAPE_MAJOR, 122), "\\Device\\Tape122"},
  {"/dev/st123", FHDEV(DEV_TAPE_MAJOR, 123), "\\Device\\Tape123"},
  {"/dev/st124", FHDEV(DEV_TAPE_MAJOR, 124), "\\Device\\Tape124"},
  {"/dev/st125", FHDEV(DEV_TAPE_MAJOR, 125), "\\Device\\Tape125"},
  {"/dev/st126", FHDEV(DEV_TAPE_MAJOR, 126), "\\Device\\Tape126"},
  {"/dev/st127", FHDEV(DEV_TAPE_MAJOR, 127), "\\Device\\Tape127"},
  {"/dev/tty", FH_TTY, "\\dev\\tty"},
  {"/dev/tty0", FHDEV(DEV_TTYS_MAJOR, 0), "\\dev\\tty0"},
  {"/dev/tty1", FHDEV(DEV_TTYS_MAJOR, 1), "\\dev\\tty1"},
  {"/dev/tty2", FHDEV(DEV_TTYS_MAJOR, 2), "\\dev\\tty2"},
  {"/dev/tty3", FHDEV(DEV_TTYS_MAJOR, 3), "\\dev\\tty3"},
  {"/dev/tty4", FHDEV(DEV_TTYS_MAJOR, 4), "\\dev\\tty4"},
  {"/dev/tty5", FHDEV(DEV_TTYS_MAJOR, 5), "\\dev\\tty5"},
  {"/dev/tty6", FHDEV(DEV_TTYS_MAJOR, 6), "\\dev\\tty6"},
  {"/dev/tty7", FHDEV(DEV_TTYS_MAJOR, 7), "\\dev\\tty7"},
  {"/dev/tty8", FHDEV(DEV_TTYS_MAJOR, 8), "\\dev\\tty8"},
  {"/dev/tty9", FHDEV(DEV_TTYS_MAJOR, 9), "\\dev\\tty9"},
  {"/dev/tty10", FHDEV(DEV_TTYS_MAJOR, 10), "\\dev\\tty10"},
  {"/dev/tty11", FHDEV(DEV_TTYS_MAJOR, 11), "\\dev\\tty11"},
  {"/dev/tty12", FHDEV(DEV_TTYS_MAJOR, 12), "\\dev\\tty12"},
  {"/dev/tty13", FHDEV(DEV_TTYS_MAJOR, 13), "\\dev\\tty13"},
  {"/dev/tty14", FHDEV(DEV_TTYS_MAJOR, 14), "\\dev\\tty14"},
  {"/dev/tty15", FHDEV(DEV_TTYS_MAJOR, 15), "\\dev\\tty15"},
  {"/dev/tty16", FHDEV(DEV_TTYS_MAJOR, 16), "\\dev\\tty16"},
  {"/dev/tty17", FHDEV(DEV_TTYS_MAJOR, 17), "\\dev\\tty17"},
  {"/dev/tty18", FHDEV(DEV_TTYS_MAJOR, 18), "\\dev\\tty18"},
  {"/dev/tty19", FHDEV(DEV_TTYS_MAJOR, 19), "\\dev\\tty19"},
  {"/dev/tty20", FHDEV(DEV_TTYS_MAJOR, 20), "\\dev\\tty20"},
  {"/dev/tty21", FHDEV(DEV_TTYS_MAJOR, 21), "\\dev\\tty21"},
  {"/dev/tty22", FHDEV(DEV_TTYS_MAJOR, 22), "\\dev\\tty22"},
  {"/dev/tty23", FHDEV(DEV_TTYS_MAJOR, 23), "\\dev\\tty23"},
  {"/dev/tty24", FHDEV(DEV_TTYS_MAJOR, 24), "\\dev\\tty24"},
  {"/dev/tty25", FHDEV(DEV_TTYS_MAJOR, 25), "\\dev\\tty25"},
  {"/dev/tty26", FHDEV(DEV_TTYS_MAJOR, 26), "\\dev\\tty26"},
  {"/dev/tty27", FHDEV(DEV_TTYS_MAJOR, 27), "\\dev\\tty27"},
  {"/dev/tty28", FHDEV(DEV_TTYS_MAJOR, 28), "\\dev\\tty28"},
  {"/dev/tty29", FHDEV(DEV_TTYS_MAJOR, 29), "\\dev\\tty29"},
  {"/dev/tty30", FHDEV(DEV_TTYS_MAJOR, 30), "\\dev\\tty30"},
  {"/dev/tty31", FHDEV(DEV_TTYS_MAJOR, 31), "\\dev\\tty31"},
  {"/dev/tty32", FHDEV(DEV_TTYS_MAJOR, 32), "\\dev\\tty32"},
  {"/dev/tty33", FHDEV(DEV_TTYS_MAJOR, 33), "\\dev\\tty33"},
  {"/dev/tty34", FHDEV(DEV_TTYS_MAJOR, 34), "\\dev\\tty34"},
  {"/dev/tty35", FHDEV(DEV_TTYS_MAJOR, 35), "\\dev\\tty35"},
  {"/dev/tty36", FHDEV(DEV_TTYS_MAJOR, 36), "\\dev\\tty36"},
  {"/dev/tty37", FHDEV(DEV_TTYS_MAJOR, 37), "\\dev\\tty37"},
  {"/dev/tty38", FHDEV(DEV_TTYS_MAJOR, 38), "\\dev\\tty38"},
  {"/dev/tty39", FHDEV(DEV_TTYS_MAJOR, 39), "\\dev\\tty39"},
  {"/dev/tty40", FHDEV(DEV_TTYS_MAJOR, 40), "\\dev\\tty40"},
  {"/dev/tty41", FHDEV(DEV_TTYS_MAJOR, 41), "\\dev\\tty41"},
  {"/dev/tty42", FHDEV(DEV_TTYS_MAJOR, 42), "\\dev\\tty42"},
  {"/dev/tty43", FHDEV(DEV_TTYS_MAJOR, 43), "\\dev\\tty43"},
  {"/dev/tty44", FHDEV(DEV_TTYS_MAJOR, 44), "\\dev\\tty44"},
  {"/dev/tty45", FHDEV(DEV_TTYS_MAJOR, 45), "\\dev\\tty45"},
  {"/dev/tty46", FHDEV(DEV_TTYS_MAJOR, 46), "\\dev\\tty46"},
  {"/dev/tty47", FHDEV(DEV_TTYS_MAJOR, 47), "\\dev\\tty47"},
  {"/dev/tty48", FHDEV(DEV_TTYS_MAJOR, 48), "\\dev\\tty48"},
  {"/dev/tty49", FHDEV(DEV_TTYS_MAJOR, 49), "\\dev\\tty49"},
  {"/dev/tty50", FHDEV(DEV_TTYS_MAJOR, 50), "\\dev\\tty50"},
  {"/dev/tty51", FHDEV(DEV_TTYS_MAJOR, 51), "\\dev\\tty51"},
  {"/dev/tty52", FHDEV(DEV_TTYS_MAJOR, 52), "\\dev\\tty52"},
  {"/dev/tty53", FHDEV(DEV_TTYS_MAJOR, 53), "\\dev\\tty53"},
  {"/dev/tty54", FHDEV(DEV_TTYS_MAJOR, 54), "\\dev\\tty54"},
  {"/dev/tty55", FHDEV(DEV_TTYS_MAJOR, 55), "\\dev\\tty55"},
  {"/dev/tty56", FHDEV(DEV_TTYS_MAJOR, 56), "\\dev\\tty56"},
  {"/dev/tty57", FHDEV(DEV_TTYS_MAJOR, 57), "\\dev\\tty57"},
  {"/dev/tty58", FHDEV(DEV_TTYS_MAJOR, 58), "\\dev\\tty58"},
  {"/dev/tty59", FHDEV(DEV_TTYS_MAJOR, 59), "\\dev\\tty59"},
  {"/dev/tty60", FHDEV(DEV_TTYS_MAJOR, 60), "\\dev\\tty60"},
  {"/dev/tty61", FHDEV(DEV_TTYS_MAJOR, 61), "\\dev\\tty61"},
  {"/dev/tty62", FHDEV(DEV_TTYS_MAJOR, 62), "\\dev\\tty62"},
  {"/dev/tty63", FHDEV(DEV_TTYS_MAJOR, 63), "\\dev\\tty63"},
  {"/dev/ttyS0", FHDEV(DEV_SERIAL_MAJOR, 1), "\\\\.\\com1"},
  {"/dev/ttyS1", FHDEV(DEV_SERIAL_MAJOR, 2), "\\\\.\\com2"},
  {"/dev/ttyS2", FHDEV(DEV_SERIAL_MAJOR, 3), "\\\\.\\com3"},
  {"/dev/ttyS3", FHDEV(DEV_SERIAL_MAJOR, 4), "\\\\.\\com4"},
  {"/dev/ttyS4", FHDEV(DEV_SERIAL_MAJOR, 5), "\\\\.\\com5"},
  {"/dev/ttyS5", FHDEV(DEV_SERIAL_MAJOR, 6), "\\\\.\\com6"},
  {"/dev/ttyS6", FHDEV(DEV_SERIAL_MAJOR, 7), "\\\\.\\com7"},
  {"/dev/ttyS7", FHDEV(DEV_SERIAL_MAJOR, 8), "\\\\.\\com8"},
  {"/dev/ttyS8", FHDEV(DEV_SERIAL_MAJOR, 9), "\\\\.\\com9"},
  {"/dev/ttyS9", FHDEV(DEV_SERIAL_MAJOR, 10), "\\\\.\\com10"},
  {"/dev/ttyS10", FHDEV(DEV_SERIAL_MAJOR, 11), "\\\\.\\com11"},
  {"/dev/ttyS11", FHDEV(DEV_SERIAL_MAJOR, 12), "\\\\.\\com12"},
  {"/dev/ttyS12", FHDEV(DEV_SERIAL_MAJOR, 13), "\\\\.\\com13"},
  {"/dev/ttyS13", FHDEV(DEV_SERIAL_MAJOR, 14), "\\\\.\\com14"},
  {"/dev/ttyS14", FHDEV(DEV_SERIAL_MAJOR, 15), "\\\\.\\com15"},
  {"/dev/ttyS15", FHDEV(DEV_SERIAL_MAJOR, 16), "\\\\.\\com16"},
  {"/dev/ttym", FH_TTYM, "\\dev\\ttym"},
  {"/dev/urandom", FH_URANDOM, "\\dev\\urandom"},
  {"/dev/windows", FH_WINDOWS, "\\dev\\windows"},
  {"/dev/zero", FH_ZERO, "\\dev\\zero"}
};

const device *console_dev = dev_storage + 20;
const device *ttym_dev = dev_storage + 831;
const device *ttys_dev = dev_storage + 751;
const device *urandom_dev = dev_storage + 832;


static KR_device_t KR_find_keyword (const char *KR_keyword, int KR_length)
{

  switch (KR_length)
    {
    case 8:
      switch (KR_keyword [7])
        {
        case 'z':
          if (strncmp (KR_keyword, "/dev/sdz", 8) == 0)
            {
{
return dev_storage + 215;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'y':
          switch (KR_keyword [5])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/tty", 8) == 0)
                {
{
return dev_storage + 750;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sdy", 8) == 0)
                {
{
return dev_storage + 214;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'x':
          if (strncmp (KR_keyword, "/dev/sdx", 8) == 0)
            {
{
return dev_storage + 213;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'w':
          if (strncmp (KR_keyword, "/dev/sdw", 8) == 0)
            {
{
return dev_storage + 212;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'v':
          if (strncmp (KR_keyword, "/dev/sdv", 8) == 0)
            {
{
return dev_storage + 211;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'u':
          if (strncmp (KR_keyword, "/dev/sdu", 8) == 0)
            {
{
return dev_storage + 210;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 't':
          if (strncmp (KR_keyword, "/dev/sdt", 8) == 0)
            {
{
return dev_storage + 209;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 's':
          if (strncmp (KR_keyword, "/dev/sds", 8) == 0)
            {
{
return dev_storage + 208;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'r':
          if (strncmp (KR_keyword, "/dev/sdr", 8) == 0)
            {
{
return dev_storage + 207;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'q':
          if (strncmp (KR_keyword, "/dev/sdq", 8) == 0)
            {
{
return dev_storage + 206;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'p':
          switch (KR_keyword [5])
            {
            case 's':
              if (strncmp (KR_keyword, "/dev/sdp", 8) == 0)
                {
{
return dev_storage + 205;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/dsp", 8) == 0)
                {
{
return dev_storage + 21;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'o':
          if (strncmp (KR_keyword, "/dev/sdo", 8) == 0)
            {
{
return dev_storage + 204;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'n':
          if (strncmp (KR_keyword, "/dev/sdn", 8) == 0)
            {
{
return dev_storage + 203;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'm':
          switch (KR_keyword [5])
            {
            case 's':
              if (strncmp (KR_keyword, "/dev/sdm", 8) == 0)
                {
{
return dev_storage + 202;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              if (strncmp (KR_keyword, "/dev/mem", 8) == 0)
                {
{
return dev_storage + 40;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'l':
          if (strncmp (KR_keyword, "/dev/sdl", 8) == 0)
            {
{
return dev_storage + 201;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'k':
          if (strncmp (KR_keyword, "/dev/sdk", 8) == 0)
            {
{
return dev_storage + 200;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'j':
          if (strncmp (KR_keyword, "/dev/sdj", 8) == 0)
            {
{
return dev_storage + 199;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'i':
          if (strncmp (KR_keyword, "/dev/sdi", 8) == 0)
            {
{
return dev_storage + 198;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'h':
          if (strncmp (KR_keyword, "/dev/sdh", 8) == 0)
            {
{
return dev_storage + 197;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'g':
          if (strncmp (KR_keyword, "/dev/sdg", 8) == 0)
            {
{
return dev_storage + 196;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'f':
          if (strncmp (KR_keyword, "/dev/sdf", 8) == 0)
            {
{
return dev_storage + 195;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'e':
          if (strncmp (KR_keyword, "/dev/sde", 8) == 0)
            {
{
return dev_storage + 194;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'd':
          if (strncmp (KR_keyword, "/dev/sdd", 8) == 0)
            {
{
return dev_storage + 193;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'c':
          if (strncmp (KR_keyword, "/dev/sdc", 8) == 0)
            {
{
return dev_storage + 192;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'b':
          if (strncmp (KR_keyword, "/dev/sdb", 8) == 0)
            {
{
return dev_storage + 191;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'a':
          if (strncmp (KR_keyword, "/dev/sda", 8) == 0)
            {
{
return dev_storage + 190;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '9':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st9", 8) == 0)
                {
{
return dev_storage + 631;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr9", 8) == 0)
                {
{
return dev_storage + 615;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd9", 8) == 0)
                {
{
return dev_storage + 31;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '8':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st8", 8) == 0)
                {
{
return dev_storage + 630;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr8", 8) == 0)
                {
{
return dev_storage + 614;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd8", 8) == 0)
                {
{
return dev_storage + 30;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '7':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st7", 8) == 0)
                {
{
return dev_storage + 629;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr7", 8) == 0)
                {
{
return dev_storage + 613;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd7", 8) == 0)
                {
{
return dev_storage + 29;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '6':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st6", 8) == 0)
                {
{
return dev_storage + 628;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr6", 8) == 0)
                {
{
return dev_storage + 612;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd6", 8) == 0)
                {
{
return dev_storage + 28;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '5':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st5", 8) == 0)
                {
{
return dev_storage + 627;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr5", 8) == 0)
                {
{
return dev_storage + 611;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd5", 8) == 0)
                {
{
return dev_storage + 27;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '4':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st4", 8) == 0)
                {
{
return dev_storage + 626;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr4", 8) == 0)
                {
{
return dev_storage + 610;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd4", 8) == 0)
                {
{
return dev_storage + 26;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '3':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st3", 8) == 0)
                {
{
return dev_storage + 625;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr3", 8) == 0)
                {
{
return dev_storage + 609;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd3", 8) == 0)
                {
{
return dev_storage + 25;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '2':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st2", 8) == 0)
                {
{
return dev_storage + 624;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr2", 8) == 0)
                {
{
return dev_storage + 608;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd2", 8) == 0)
                {
{
return dev_storage + 24;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '1':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st1", 8) == 0)
                {
{
return dev_storage + 623;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr1", 8) == 0)
                {
{
return dev_storage + 607;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd1", 8) == 0)
                {
{
return dev_storage + 23;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '0':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st0", 8) == 0)
                {
{
return dev_storage + 622;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr0", 8) == 0)
                {
{
return dev_storage + 606;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd0", 8) == 0)
                {
{
return dev_storage + 22;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 9:
      switch (KR_keyword [8])
        {
        case 'x':
          if (strncmp (KR_keyword, "/dev/ptmx", 9) == 0)
            {
{
return dev_storage + 172;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 't':
          if (strncmp (KR_keyword, "/dev/port", 9) == 0)
            {
{
return dev_storage + 171;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'o':
          switch (KR_keyword [5])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/zero", 9) == 0)
                {
{
return dev_storage + 834;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/fifo", 9) == 0)
                {
{
return dev_storage + 38;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'm':
          switch (KR_keyword [5])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/ttym", 9) == 0)
                {
{
return dev_storage + 831;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/kmem", 9) == 0)
                {
{
return dev_storage + 39;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case 'l':
          if (strncmp (KR_keyword, "/dev/null", 9) == 0)
            {
{
return dev_storage + 169;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'e':
          if (strncmp (KR_keyword, "/dev/pipe", 9) == 0)
            {
{
return dev_storage + 170;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '9':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz9", 9) == 0)
                {
{
return dev_storage + 599;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty9", 9) == 0)
                    {
{
return dev_storage + 760;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy9", 9) == 0)
                    {
{
return dev_storage + 584;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx9", 9) == 0)
                {
{
return dev_storage + 569;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw9", 9) == 0)
                {
{
return dev_storage + 554;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv9", 9) == 0)
                {
{
return dev_storage + 539;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu9", 9) == 0)
                {
{
return dev_storage + 524;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt9", 9) == 0)
                    {
{
return dev_storage + 509;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst9", 9) == 0)
                    {
{
return dev_storage + 50;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds9", 9) == 0)
                {
{
return dev_storage + 494;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr9", 9) == 0)
                {
{
return dev_storage + 479;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq9", 9) == 0)
                {
{
return dev_storage + 464;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp9", 9) == 0)
                {
{
return dev_storage + 449;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo9", 9) == 0)
                {
{
return dev_storage + 434;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn9", 9) == 0)
                {
{
return dev_storage + 419;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm9", 9) == 0)
                    {
{
return dev_storage + 404;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com9", 9) == 0)
                    {
{
return dev_storage + 10;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl9", 9) == 0)
                {
{
return dev_storage + 389;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk9", 9) == 0)
                {
{
return dev_storage + 374;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj9", 9) == 0)
                {
{
return dev_storage + 359;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi9", 9) == 0)
                {
{
return dev_storage + 344;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh9", 9) == 0)
                {
{
return dev_storage + 329;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg9", 9) == 0)
                {
{
return dev_storage + 314;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf9", 9) == 0)
                {
{
return dev_storage + 299;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde9", 9) == 0)
                {
{
return dev_storage + 284;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd9", 9) == 0)
                    {
{
return dev_storage + 269;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd9", 9) == 0)
                    {
{
return dev_storage + 183;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc9", 9) == 0)
                {
{
return dev_storage + 254;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb9", 9) == 0)
                {
{
return dev_storage + 239;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda9", 9) == 0)
                {
{
return dev_storage + 224;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st99", 9) == 0)
                {
{
return dev_storage + 721;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st89", 9) == 0)
                {
{
return dev_storage + 711;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st79", 9) == 0)
                {
{
return dev_storage + 701;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st69", 9) == 0)
                {
{
return dev_storage + 691;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st59", 9) == 0)
                {
{
return dev_storage + 681;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st49", 9) == 0)
                {
{
return dev_storage + 671;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st39", 9) == 0)
                {
{
return dev_storage + 661;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st29", 9) == 0)
                {
{
return dev_storage + 651;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st19", 9) == 0)
                {
{
return dev_storage + 641;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '8':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz8", 9) == 0)
                {
{
return dev_storage + 598;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty8", 9) == 0)
                    {
{
return dev_storage + 759;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy8", 9) == 0)
                    {
{
return dev_storage + 583;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx8", 9) == 0)
                {
{
return dev_storage + 568;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw8", 9) == 0)
                {
{
return dev_storage + 553;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv8", 9) == 0)
                {
{
return dev_storage + 538;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu8", 9) == 0)
                {
{
return dev_storage + 523;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt8", 9) == 0)
                    {
{
return dev_storage + 508;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst8", 9) == 0)
                    {
{
return dev_storage + 49;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds8", 9) == 0)
                {
{
return dev_storage + 493;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr8", 9) == 0)
                {
{
return dev_storage + 478;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq8", 9) == 0)
                {
{
return dev_storage + 463;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp8", 9) == 0)
                {
{
return dev_storage + 448;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo8", 9) == 0)
                {
{
return dev_storage + 433;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn8", 9) == 0)
                {
{
return dev_storage + 418;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm8", 9) == 0)
                    {
{
return dev_storage + 403;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com8", 9) == 0)
                    {
{
return dev_storage + 9;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl8", 9) == 0)
                {
{
return dev_storage + 388;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk8", 9) == 0)
                {
{
return dev_storage + 373;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj8", 9) == 0)
                {
{
return dev_storage + 358;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi8", 9) == 0)
                {
{
return dev_storage + 343;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh8", 9) == 0)
                {
{
return dev_storage + 328;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg8", 9) == 0)
                {
{
return dev_storage + 313;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf8", 9) == 0)
                {
{
return dev_storage + 298;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde8", 9) == 0)
                {
{
return dev_storage + 283;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd8", 9) == 0)
                    {
{
return dev_storage + 268;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd8", 9) == 0)
                    {
{
return dev_storage + 182;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc8", 9) == 0)
                {
{
return dev_storage + 253;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb8", 9) == 0)
                {
{
return dev_storage + 238;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda8", 9) == 0)
                {
{
return dev_storage + 223;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st98", 9) == 0)
                {
{
return dev_storage + 720;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st88", 9) == 0)
                {
{
return dev_storage + 710;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st78", 9) == 0)
                {
{
return dev_storage + 700;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st68", 9) == 0)
                {
{
return dev_storage + 690;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st58", 9) == 0)
                {
{
return dev_storage + 680;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st48", 9) == 0)
                {
{
return dev_storage + 670;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st38", 9) == 0)
                {
{
return dev_storage + 660;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st28", 9) == 0)
                {
{
return dev_storage + 650;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st18", 9) == 0)
                {
{
return dev_storage + 640;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '7':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz7", 9) == 0)
                {
{
return dev_storage + 597;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty7", 9) == 0)
                    {
{
return dev_storage + 758;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy7", 9) == 0)
                    {
{
return dev_storage + 582;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx7", 9) == 0)
                {
{
return dev_storage + 567;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw7", 9) == 0)
                {
{
return dev_storage + 552;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv7", 9) == 0)
                {
{
return dev_storage + 537;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu7", 9) == 0)
                {
{
return dev_storage + 522;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt7", 9) == 0)
                    {
{
return dev_storage + 507;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst7", 9) == 0)
                    {
{
return dev_storage + 48;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds7", 9) == 0)
                {
{
return dev_storage + 492;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr7", 9) == 0)
                {
{
return dev_storage + 477;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq7", 9) == 0)
                {
{
return dev_storage + 462;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp7", 9) == 0)
                {
{
return dev_storage + 447;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo7", 9) == 0)
                {
{
return dev_storage + 432;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn7", 9) == 0)
                {
{
return dev_storage + 417;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm7", 9) == 0)
                    {
{
return dev_storage + 402;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com7", 9) == 0)
                    {
{
return dev_storage + 8;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl7", 9) == 0)
                {
{
return dev_storage + 387;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk7", 9) == 0)
                {
{
return dev_storage + 372;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj7", 9) == 0)
                {
{
return dev_storage + 357;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi7", 9) == 0)
                {
{
return dev_storage + 342;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh7", 9) == 0)
                {
{
return dev_storage + 327;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg7", 9) == 0)
                {
{
return dev_storage + 312;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf7", 9) == 0)
                {
{
return dev_storage + 297;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde7", 9) == 0)
                {
{
return dev_storage + 282;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd7", 9) == 0)
                    {
{
return dev_storage + 267;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd7", 9) == 0)
                    {
{
return dev_storage + 181;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc7", 9) == 0)
                {
{
return dev_storage + 252;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb7", 9) == 0)
                {
{
return dev_storage + 237;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda7", 9) == 0)
                {
{
return dev_storage + 222;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st97", 9) == 0)
                {
{
return dev_storage + 719;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st87", 9) == 0)
                {
{
return dev_storage + 709;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st77", 9) == 0)
                {
{
return dev_storage + 699;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st67", 9) == 0)
                {
{
return dev_storage + 689;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st57", 9) == 0)
                {
{
return dev_storage + 679;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st47", 9) == 0)
                {
{
return dev_storage + 669;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st37", 9) == 0)
                {
{
return dev_storage + 659;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st27", 9) == 0)
                {
{
return dev_storage + 649;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st17", 9) == 0)
                {
{
return dev_storage + 639;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '6':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz6", 9) == 0)
                {
{
return dev_storage + 596;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty6", 9) == 0)
                    {
{
return dev_storage + 757;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy6", 9) == 0)
                    {
{
return dev_storage + 581;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx6", 9) == 0)
                {
{
return dev_storage + 566;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw6", 9) == 0)
                {
{
return dev_storage + 551;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv6", 9) == 0)
                {
{
return dev_storage + 536;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu6", 9) == 0)
                {
{
return dev_storage + 521;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt6", 9) == 0)
                    {
{
return dev_storage + 506;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst6", 9) == 0)
                    {
{
return dev_storage + 47;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds6", 9) == 0)
                {
{
return dev_storage + 491;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr6", 9) == 0)
                {
{
return dev_storage + 476;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq6", 9) == 0)
                {
{
return dev_storage + 461;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp6", 9) == 0)
                {
{
return dev_storage + 446;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo6", 9) == 0)
                {
{
return dev_storage + 431;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn6", 9) == 0)
                {
{
return dev_storage + 416;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm6", 9) == 0)
                    {
{
return dev_storage + 401;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com6", 9) == 0)
                    {
{
return dev_storage + 7;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl6", 9) == 0)
                {
{
return dev_storage + 386;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk6", 9) == 0)
                {
{
return dev_storage + 371;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj6", 9) == 0)
                {
{
return dev_storage + 356;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi6", 9) == 0)
                {
{
return dev_storage + 341;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh6", 9) == 0)
                {
{
return dev_storage + 326;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg6", 9) == 0)
                {
{
return dev_storage + 311;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf6", 9) == 0)
                {
{
return dev_storage + 296;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde6", 9) == 0)
                {
{
return dev_storage + 281;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd6", 9) == 0)
                    {
{
return dev_storage + 266;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd6", 9) == 0)
                    {
{
return dev_storage + 180;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc6", 9) == 0)
                {
{
return dev_storage + 251;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb6", 9) == 0)
                {
{
return dev_storage + 236;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda6", 9) == 0)
                {
{
return dev_storage + 221;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st96", 9) == 0)
                {
{
return dev_storage + 718;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st86", 9) == 0)
                {
{
return dev_storage + 708;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st76", 9) == 0)
                {
{
return dev_storage + 698;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st66", 9) == 0)
                {
{
return dev_storage + 688;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st56", 9) == 0)
                {
{
return dev_storage + 678;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st46", 9) == 0)
                {
{
return dev_storage + 668;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st36", 9) == 0)
                {
{
return dev_storage + 658;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st26", 9) == 0)
                {
{
return dev_storage + 648;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st16", 9) == 0)
                {
{
return dev_storage + 638;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '5':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz5", 9) == 0)
                {
{
return dev_storage + 595;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty5", 9) == 0)
                    {
{
return dev_storage + 756;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy5", 9) == 0)
                    {
{
return dev_storage + 580;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx5", 9) == 0)
                {
{
return dev_storage + 565;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw5", 9) == 0)
                {
{
return dev_storage + 550;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv5", 9) == 0)
                {
{
return dev_storage + 535;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu5", 9) == 0)
                {
{
return dev_storage + 520;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt5", 9) == 0)
                    {
{
return dev_storage + 505;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst5", 9) == 0)
                    {
{
return dev_storage + 46;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds5", 9) == 0)
                {
{
return dev_storage + 490;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr5", 9) == 0)
                {
{
return dev_storage + 475;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq5", 9) == 0)
                {
{
return dev_storage + 460;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp5", 9) == 0)
                {
{
return dev_storage + 445;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo5", 9) == 0)
                {
{
return dev_storage + 430;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn5", 9) == 0)
                {
{
return dev_storage + 415;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm5", 9) == 0)
                    {
{
return dev_storage + 400;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com5", 9) == 0)
                    {
{
return dev_storage + 6;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl5", 9) == 0)
                {
{
return dev_storage + 385;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk5", 9) == 0)
                {
{
return dev_storage + 370;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj5", 9) == 0)
                {
{
return dev_storage + 355;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi5", 9) == 0)
                {
{
return dev_storage + 340;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh5", 9) == 0)
                {
{
return dev_storage + 325;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg5", 9) == 0)
                {
{
return dev_storage + 310;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf5", 9) == 0)
                {
{
return dev_storage + 295;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde5", 9) == 0)
                {
{
return dev_storage + 280;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd5", 9) == 0)
                    {
{
return dev_storage + 265;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd5", 9) == 0)
                    {
{
return dev_storage + 179;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc5", 9) == 0)
                {
{
return dev_storage + 250;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb5", 9) == 0)
                {
{
return dev_storage + 235;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda5", 9) == 0)
                {
{
return dev_storage + 220;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st95", 9) == 0)
                {
{
return dev_storage + 717;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st85", 9) == 0)
                {
{
return dev_storage + 707;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st75", 9) == 0)
                {
{
return dev_storage + 697;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st65", 9) == 0)
                {
{
return dev_storage + 687;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st55", 9) == 0)
                {
{
return dev_storage + 677;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st45", 9) == 0)
                {
{
return dev_storage + 667;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st35", 9) == 0)
                {
{
return dev_storage + 657;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st25", 9) == 0)
                {
{
return dev_storage + 647;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st15", 9) == 0)
                    {
{
return dev_storage + 637;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr15", 9) == 0)
                    {
{
return dev_storage + 621;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd15", 9) == 0)
                    {
{
return dev_storage + 37;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '4':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz4", 9) == 0)
                {
{
return dev_storage + 594;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty4", 9) == 0)
                    {
{
return dev_storage + 755;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy4", 9) == 0)
                    {
{
return dev_storage + 579;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx4", 9) == 0)
                {
{
return dev_storage + 564;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw4", 9) == 0)
                {
{
return dev_storage + 549;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv4", 9) == 0)
                {
{
return dev_storage + 534;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu4", 9) == 0)
                {
{
return dev_storage + 519;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt4", 9) == 0)
                    {
{
return dev_storage + 504;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst4", 9) == 0)
                    {
{
return dev_storage + 45;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds4", 9) == 0)
                {
{
return dev_storage + 489;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr4", 9) == 0)
                {
{
return dev_storage + 474;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq4", 9) == 0)
                {
{
return dev_storage + 459;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp4", 9) == 0)
                {
{
return dev_storage + 444;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo4", 9) == 0)
                {
{
return dev_storage + 429;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn4", 9) == 0)
                {
{
return dev_storage + 414;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm4", 9) == 0)
                    {
{
return dev_storage + 399;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com4", 9) == 0)
                    {
{
return dev_storage + 5;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl4", 9) == 0)
                {
{
return dev_storage + 384;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk4", 9) == 0)
                {
{
return dev_storage + 369;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj4", 9) == 0)
                {
{
return dev_storage + 354;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi4", 9) == 0)
                {
{
return dev_storage + 339;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh4", 9) == 0)
                {
{
return dev_storage + 324;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg4", 9) == 0)
                {
{
return dev_storage + 309;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf4", 9) == 0)
                {
{
return dev_storage + 294;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde4", 9) == 0)
                {
{
return dev_storage + 279;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd4", 9) == 0)
                    {
{
return dev_storage + 264;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd4", 9) == 0)
                    {
{
return dev_storage + 178;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc4", 9) == 0)
                {
{
return dev_storage + 249;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb4", 9) == 0)
                {
{
return dev_storage + 234;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda4", 9) == 0)
                {
{
return dev_storage + 219;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st94", 9) == 0)
                {
{
return dev_storage + 716;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st84", 9) == 0)
                {
{
return dev_storage + 706;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st74", 9) == 0)
                {
{
return dev_storage + 696;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st64", 9) == 0)
                {
{
return dev_storage + 686;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st54", 9) == 0)
                {
{
return dev_storage + 676;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st44", 9) == 0)
                {
{
return dev_storage + 666;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st34", 9) == 0)
                {
{
return dev_storage + 656;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st24", 9) == 0)
                {
{
return dev_storage + 646;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st14", 9) == 0)
                    {
{
return dev_storage + 636;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr14", 9) == 0)
                    {
{
return dev_storage + 620;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd14", 9) == 0)
                    {
{
return dev_storage + 36;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '3':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz3", 9) == 0)
                {
{
return dev_storage + 593;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty3", 9) == 0)
                    {
{
return dev_storage + 754;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy3", 9) == 0)
                    {
{
return dev_storage + 578;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx3", 9) == 0)
                {
{
return dev_storage + 563;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw3", 9) == 0)
                {
{
return dev_storage + 548;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv3", 9) == 0)
                {
{
return dev_storage + 533;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu3", 9) == 0)
                {
{
return dev_storage + 518;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt3", 9) == 0)
                    {
{
return dev_storage + 503;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst3", 9) == 0)
                    {
{
return dev_storage + 44;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds3", 9) == 0)
                {
{
return dev_storage + 488;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr3", 9) == 0)
                {
{
return dev_storage + 473;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq3", 9) == 0)
                {
{
return dev_storage + 458;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp3", 9) == 0)
                {
{
return dev_storage + 443;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo3", 9) == 0)
                {
{
return dev_storage + 428;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn3", 9) == 0)
                {
{
return dev_storage + 413;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm3", 9) == 0)
                    {
{
return dev_storage + 398;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com3", 9) == 0)
                    {
{
return dev_storage + 4;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl3", 9) == 0)
                {
{
return dev_storage + 383;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk3", 9) == 0)
                {
{
return dev_storage + 368;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj3", 9) == 0)
                {
{
return dev_storage + 353;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi3", 9) == 0)
                {
{
return dev_storage + 338;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh3", 9) == 0)
                {
{
return dev_storage + 323;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg3", 9) == 0)
                {
{
return dev_storage + 308;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf3", 9) == 0)
                {
{
return dev_storage + 293;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde3", 9) == 0)
                {
{
return dev_storage + 278;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd3", 9) == 0)
                    {
{
return dev_storage + 263;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd3", 9) == 0)
                    {
{
return dev_storage + 177;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc3", 9) == 0)
                {
{
return dev_storage + 248;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb3", 9) == 0)
                {
{
return dev_storage + 233;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda3", 9) == 0)
                {
{
return dev_storage + 218;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st93", 9) == 0)
                {
{
return dev_storage + 715;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st83", 9) == 0)
                {
{
return dev_storage + 705;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st73", 9) == 0)
                {
{
return dev_storage + 695;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st63", 9) == 0)
                {
{
return dev_storage + 685;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st53", 9) == 0)
                {
{
return dev_storage + 675;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st43", 9) == 0)
                {
{
return dev_storage + 665;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st33", 9) == 0)
                {
{
return dev_storage + 655;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st23", 9) == 0)
                {
{
return dev_storage + 645;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st13", 9) == 0)
                    {
{
return dev_storage + 635;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr13", 9) == 0)
                    {
{
return dev_storage + 619;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd13", 9) == 0)
                    {
{
return dev_storage + 35;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '2':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz2", 9) == 0)
                {
{
return dev_storage + 592;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty2", 9) == 0)
                    {
{
return dev_storage + 753;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy2", 9) == 0)
                    {
{
return dev_storage + 577;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx2", 9) == 0)
                {
{
return dev_storage + 562;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw2", 9) == 0)
                {
{
return dev_storage + 547;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv2", 9) == 0)
                {
{
return dev_storage + 532;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu2", 9) == 0)
                {
{
return dev_storage + 517;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt2", 9) == 0)
                    {
{
return dev_storage + 502;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst2", 9) == 0)
                    {
{
return dev_storage + 43;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds2", 9) == 0)
                {
{
return dev_storage + 487;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr2", 9) == 0)
                {
{
return dev_storage + 472;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq2", 9) == 0)
                {
{
return dev_storage + 457;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp2", 9) == 0)
                {
{
return dev_storage + 442;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo2", 9) == 0)
                {
{
return dev_storage + 427;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn2", 9) == 0)
                {
{
return dev_storage + 412;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm2", 9) == 0)
                    {
{
return dev_storage + 397;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com2", 9) == 0)
                    {
{
return dev_storage + 3;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl2", 9) == 0)
                {
{
return dev_storage + 382;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk2", 9) == 0)
                {
{
return dev_storage + 367;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj2", 9) == 0)
                {
{
return dev_storage + 352;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi2", 9) == 0)
                {
{
return dev_storage + 337;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh2", 9) == 0)
                {
{
return dev_storage + 322;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg2", 9) == 0)
                {
{
return dev_storage + 307;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf2", 9) == 0)
                {
{
return dev_storage + 292;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde2", 9) == 0)
                {
{
return dev_storage + 277;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd2", 9) == 0)
                    {
{
return dev_storage + 262;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd2", 9) == 0)
                    {
{
return dev_storage + 176;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc2", 9) == 0)
                {
{
return dev_storage + 247;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb2", 9) == 0)
                {
{
return dev_storage + 232;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda2", 9) == 0)
                {
{
return dev_storage + 217;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st92", 9) == 0)
                {
{
return dev_storage + 714;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st82", 9) == 0)
                {
{
return dev_storage + 704;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st72", 9) == 0)
                {
{
return dev_storage + 694;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st62", 9) == 0)
                {
{
return dev_storage + 684;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st52", 9) == 0)
                {
{
return dev_storage + 674;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st42", 9) == 0)
                {
{
return dev_storage + 664;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st32", 9) == 0)
                {
{
return dev_storage + 654;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st22", 9) == 0)
                {
{
return dev_storage + 644;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st12", 9) == 0)
                    {
{
return dev_storage + 634;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr12", 9) == 0)
                    {
{
return dev_storage + 618;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd12", 9) == 0)
                    {
{
return dev_storage + 34;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '1':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz1", 9) == 0)
                {
{
return dev_storage + 591;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty1", 9) == 0)
                    {
{
return dev_storage + 752;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy1", 9) == 0)
                    {
{
return dev_storage + 576;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx1", 9) == 0)
                {
{
return dev_storage + 561;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw1", 9) == 0)
                {
{
return dev_storage + 546;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv1", 9) == 0)
                {
{
return dev_storage + 531;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu1", 9) == 0)
                {
{
return dev_storage + 516;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt1", 9) == 0)
                    {
{
return dev_storage + 501;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst1", 9) == 0)
                    {
{
return dev_storage + 42;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds1", 9) == 0)
                {
{
return dev_storage + 486;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr1", 9) == 0)
                {
{
return dev_storage + 471;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq1", 9) == 0)
                {
{
return dev_storage + 456;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp1", 9) == 0)
                {
{
return dev_storage + 441;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo1", 9) == 0)
                {
{
return dev_storage + 426;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn1", 9) == 0)
                {
{
return dev_storage + 411;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm1", 9) == 0)
                    {
{
return dev_storage + 396;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com1", 9) == 0)
                    {
{
return dev_storage + 2;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl1", 9) == 0)
                {
{
return dev_storage + 381;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk1", 9) == 0)
                {
{
return dev_storage + 366;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj1", 9) == 0)
                {
{
return dev_storage + 351;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi1", 9) == 0)
                {
{
return dev_storage + 336;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh1", 9) == 0)
                {
{
return dev_storage + 321;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg1", 9) == 0)
                {
{
return dev_storage + 306;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf1", 9) == 0)
                {
{
return dev_storage + 291;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde1", 9) == 0)
                {
{
return dev_storage + 276;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd1", 9) == 0)
                    {
{
return dev_storage + 261;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd1", 9) == 0)
                    {
{
return dev_storage + 175;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc1", 9) == 0)
                {
{
return dev_storage + 246;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb1", 9) == 0)
                {
{
return dev_storage + 231;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda1", 9) == 0)
                {
{
return dev_storage + 216;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st91", 9) == 0)
                {
{
return dev_storage + 713;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st81", 9) == 0)
                {
{
return dev_storage + 703;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st71", 9) == 0)
                {
{
return dev_storage + 693;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st61", 9) == 0)
                {
{
return dev_storage + 683;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st51", 9) == 0)
                {
{
return dev_storage + 673;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st41", 9) == 0)
                {
{
return dev_storage + 663;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st31", 9) == 0)
                {
{
return dev_storage + 653;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st21", 9) == 0)
                {
{
return dev_storage + 643;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st11", 9) == 0)
                    {
{
return dev_storage + 633;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr11", 9) == 0)
                    {
{
return dev_storage + 617;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd11", 9) == 0)
                    {
{
return dev_storage + 33;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '0':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/tty0", 9) == 0)
                {
{
return dev_storage + 751;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              if (strncmp (KR_keyword, "/dev/nst0", 9) == 0)
                {
{
return dev_storage + 41;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              if (strncmp (KR_keyword, "/dev/com0", 9) == 0)
                {
{
return dev_storage + 1;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd0", 9) == 0)
                {
{
return dev_storage + 174;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/st90", 9) == 0)
                {
{
return dev_storage + 712;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st80", 9) == 0)
                {
{
return dev_storage + 702;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st70", 9) == 0)
                {
{
return dev_storage + 692;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st60", 9) == 0)
                {
{
return dev_storage + 682;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st50", 9) == 0)
                {
{
return dev_storage + 672;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st40", 9) == 0)
                {
{
return dev_storage + 662;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st30", 9) == 0)
                {
{
return dev_storage + 652;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st20", 9) == 0)
                {
{
return dev_storage + 642;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st10", 9) == 0)
                    {
{
return dev_storage + 632;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr10", 9) == 0)
                    {
{
return dev_storage + 616;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd10", 9) == 0)
                    {
{
return dev_storage + 32;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 10:
      switch (KR_keyword [9])
        {
        case 'n':
          if (strncmp (KR_keyword, "/dev/conin", 10) == 0)
            {
{
return dev_storage + 18;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '9':
          switch (KR_keyword [8])
            {
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS9", 10) == 0)
                {
{
return dev_storage + 824;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/nst99", 10) == 0)
                {
{
return dev_storage + 140;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/nst89", 10) == 0)
                {
{
return dev_storage + 130;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/nst79", 10) == 0)
                {
{
return dev_storage + 120;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/nst69", 10) == 0)
                {
{
return dev_storage + 110;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty59", 10) == 0)
                    {
{
return dev_storage + 810;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst59", 10) == 0)
                    {
{
return dev_storage + 100;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty49", 10) == 0)
                    {
{
return dev_storage + 800;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst49", 10) == 0)
                    {
{
return dev_storage + 90;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty39", 10) == 0)
                    {
{
return dev_storage + 790;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst39", 10) == 0)
                    {
{
return dev_storage + 80;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty29", 10) == 0)
                    {
{
return dev_storage + 780;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst29", 10) == 0)
                    {
{
return dev_storage + 70;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty19", 10) == 0)
                    {
{
return dev_storage + 770;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/st119", 10) == 0)
                    {
{
return dev_storage + 741;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst19", 10) == 0)
                    {
{
return dev_storage + 60;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st109", 10) == 0)
                {
{
return dev_storage + 731;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '8':
          switch (KR_keyword [8])
            {
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS8", 10) == 0)
                {
{
return dev_storage + 823;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/nst98", 10) == 0)
                {
{
return dev_storage + 139;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/nst88", 10) == 0)
                {
{
return dev_storage + 129;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/nst78", 10) == 0)
                {
{
return dev_storage + 119;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/nst68", 10) == 0)
                {
{
return dev_storage + 109;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty58", 10) == 0)
                    {
{
return dev_storage + 809;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst58", 10) == 0)
                    {
{
return dev_storage + 99;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty48", 10) == 0)
                    {
{
return dev_storage + 799;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst48", 10) == 0)
                    {
{
return dev_storage + 89;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty38", 10) == 0)
                    {
{
return dev_storage + 789;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst38", 10) == 0)
                    {
{
return dev_storage + 79;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty28", 10) == 0)
                    {
{
return dev_storage + 779;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst28", 10) == 0)
                    {
{
return dev_storage + 69;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty18", 10) == 0)
                    {
{
return dev_storage + 769;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/st118", 10) == 0)
                    {
{
return dev_storage + 740;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst18", 10) == 0)
                    {
{
return dev_storage + 59;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st108", 10) == 0)
                {
{
return dev_storage + 730;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '7':
          switch (KR_keyword [8])
            {
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS7", 10) == 0)
                {
{
return dev_storage + 822;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/nst97", 10) == 0)
                {
{
return dev_storage + 138;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/nst87", 10) == 0)
                {
{
return dev_storage + 128;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/nst77", 10) == 0)
                {
{
return dev_storage + 118;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/nst67", 10) == 0)
                {
{
return dev_storage + 108;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty57", 10) == 0)
                    {
{
return dev_storage + 808;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst57", 10) == 0)
                    {
{
return dev_storage + 98;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty47", 10) == 0)
                    {
{
return dev_storage + 798;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst47", 10) == 0)
                    {
{
return dev_storage + 88;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty37", 10) == 0)
                    {
{
return dev_storage + 788;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst37", 10) == 0)
                    {
{
return dev_storage + 78;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty27", 10) == 0)
                    {
{
return dev_storage + 778;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/st127", 10) == 0)
                    {
{
return dev_storage + 749;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst27", 10) == 0)
                    {
{
return dev_storage + 68;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty17", 10) == 0)
                    {
{
return dev_storage + 768;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/st117", 10) == 0)
                    {
{
return dev_storage + 739;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst17", 10) == 0)
                    {
{
return dev_storage + 58;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st107", 10) == 0)
                {
{
return dev_storage + 729;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '6':
          switch (KR_keyword [8])
            {
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS6", 10) == 0)
                {
{
return dev_storage + 821;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/nst96", 10) == 0)
                {
{
return dev_storage + 137;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/nst86", 10) == 0)
                {
{
return dev_storage + 127;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/nst76", 10) == 0)
                {
{
return dev_storage + 117;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/nst66", 10) == 0)
                {
{
return dev_storage + 107;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty56", 10) == 0)
                    {
{
return dev_storage + 807;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst56", 10) == 0)
                    {
{
return dev_storage + 97;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty46", 10) == 0)
                    {
{
return dev_storage + 797;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst46", 10) == 0)
                    {
{
return dev_storage + 87;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty36", 10) == 0)
                    {
{
return dev_storage + 787;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst36", 10) == 0)
                    {
{
return dev_storage + 77;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty26", 10) == 0)
                    {
{
return dev_storage + 777;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/st126", 10) == 0)
                    {
{
return dev_storage + 748;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst26", 10) == 0)
                    {
{
return dev_storage + 67;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/tty16", 10) == 0)
                    {
{
return dev_storage + 767;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/st116", 10) == 0)
                    {
{
return dev_storage + 738;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst16", 10) == 0)
                    {
{
return dev_storage + 57;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com16", 10) == 0)
                    {
{
return dev_storage + 17;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st106", 10) == 0)
                {
{
return dev_storage + 728;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '5':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz15", 10) == 0)
                {
{
return dev_storage + 605;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [8])
                {
                case 'S':
                  if (strncmp (KR_keyword, "/dev/ttyS5", 10) == 0)
                    {
{
return dev_storage + 820;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/tty55", 10) == 0)
                    {
{
return dev_storage + 806;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/tty45", 10) == 0)
                    {
{
return dev_storage + 796;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/tty35", 10) == 0)
                    {
{
return dev_storage + 786;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/tty25", 10) == 0)
                    {
{
return dev_storage + 776;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 't':
                      if (strncmp (KR_keyword, "/dev/tty15", 10) == 0)
                        {
{
return dev_storage + 766;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy15", 10) == 0)
                        {
{
return dev_storage + 590;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx15", 10) == 0)
                {
{
return dev_storage + 575;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw15", 10) == 0)
                {
{
return dev_storage + 560;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv15", 10) == 0)
                {
{
return dev_storage + 545;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu15", 10) == 0)
                {
{
return dev_storage + 530;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [8])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst95", 10) == 0)
                    {
{
return dev_storage + 136;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst85", 10) == 0)
                    {
{
return dev_storage + 126;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst75", 10) == 0)
                    {
{
return dev_storage + 116;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst65", 10) == 0)
                    {
{
return dev_storage + 106;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst55", 10) == 0)
                    {
{
return dev_storage + 96;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst45", 10) == 0)
                    {
{
return dev_storage + 86;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst35", 10) == 0)
                    {
{
return dev_storage + 76;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst25", 10) == 0)
                    {
{
return dev_storage + 66;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt15", 10) == 0)
                        {
{
return dev_storage + 515;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst15", 10) == 0)
                        {
{
return dev_storage + 56;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds15", 10) == 0)
                {
{
return dev_storage + 500;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr15", 10) == 0)
                {
{
return dev_storage + 485;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq15", 10) == 0)
                {
{
return dev_storage + 470;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp15", 10) == 0)
                {
{
return dev_storage + 455;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo15", 10) == 0)
                {
{
return dev_storage + 440;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn15", 10) == 0)
                {
{
return dev_storage + 425;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm15", 10) == 0)
                    {
{
return dev_storage + 410;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com15", 10) == 0)
                    {
{
return dev_storage + 16;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl15", 10) == 0)
                {
{
return dev_storage + 395;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk15", 10) == 0)
                {
{
return dev_storage + 380;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj15", 10) == 0)
                {
{
return dev_storage + 365;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi15", 10) == 0)
                {
{
return dev_storage + 350;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh15", 10) == 0)
                {
{
return dev_storage + 335;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg15", 10) == 0)
                {
{
return dev_storage + 320;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf15", 10) == 0)
                {
{
return dev_storage + 305;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde15", 10) == 0)
                {
{
return dev_storage + 290;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd15", 10) == 0)
                    {
{
return dev_storage + 275;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd15", 10) == 0)
                    {
{
return dev_storage + 189;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc15", 10) == 0)
                {
{
return dev_storage + 260;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb15", 10) == 0)
                {
{
return dev_storage + 245;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda15", 10) == 0)
                {
{
return dev_storage + 230;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [8])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/st125", 10) == 0)
                    {
{
return dev_storage + 747;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/st115", 10) == 0)
                    {
{
return dev_storage + 737;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '0':
                  if (strncmp (KR_keyword, "/dev/st105", 10) == 0)
                    {
{
return dev_storage + 727;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '4':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz14", 10) == 0)
                {
{
return dev_storage + 604;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [8])
                {
                case 'S':
                  if (strncmp (KR_keyword, "/dev/ttyS4", 10) == 0)
                    {
{
return dev_storage + 819;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/tty54", 10) == 0)
                    {
{
return dev_storage + 805;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/tty44", 10) == 0)
                    {
{
return dev_storage + 795;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/tty34", 10) == 0)
                    {
{
return dev_storage + 785;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/tty24", 10) == 0)
                    {
{
return dev_storage + 775;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 't':
                      if (strncmp (KR_keyword, "/dev/tty14", 10) == 0)
                        {
{
return dev_storage + 765;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy14", 10) == 0)
                        {
{
return dev_storage + 589;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx14", 10) == 0)
                {
{
return dev_storage + 574;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw14", 10) == 0)
                {
{
return dev_storage + 559;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv14", 10) == 0)
                {
{
return dev_storage + 544;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu14", 10) == 0)
                {
{
return dev_storage + 529;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [8])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst94", 10) == 0)
                    {
{
return dev_storage + 135;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst84", 10) == 0)
                    {
{
return dev_storage + 125;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst74", 10) == 0)
                    {
{
return dev_storage + 115;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst64", 10) == 0)
                    {
{
return dev_storage + 105;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst54", 10) == 0)
                    {
{
return dev_storage + 95;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst44", 10) == 0)
                    {
{
return dev_storage + 85;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst34", 10) == 0)
                    {
{
return dev_storage + 75;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst24", 10) == 0)
                    {
{
return dev_storage + 65;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt14", 10) == 0)
                        {
{
return dev_storage + 514;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst14", 10) == 0)
                        {
{
return dev_storage + 55;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds14", 10) == 0)
                {
{
return dev_storage + 499;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr14", 10) == 0)
                {
{
return dev_storage + 484;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq14", 10) == 0)
                {
{
return dev_storage + 469;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp14", 10) == 0)
                {
{
return dev_storage + 454;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo14", 10) == 0)
                {
{
return dev_storage + 439;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn14", 10) == 0)
                {
{
return dev_storage + 424;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm14", 10) == 0)
                    {
{
return dev_storage + 409;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com14", 10) == 0)
                    {
{
return dev_storage + 15;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl14", 10) == 0)
                {
{
return dev_storage + 394;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk14", 10) == 0)
                {
{
return dev_storage + 379;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj14", 10) == 0)
                {
{
return dev_storage + 364;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi14", 10) == 0)
                {
{
return dev_storage + 349;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh14", 10) == 0)
                {
{
return dev_storage + 334;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg14", 10) == 0)
                {
{
return dev_storage + 319;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf14", 10) == 0)
                {
{
return dev_storage + 304;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde14", 10) == 0)
                {
{
return dev_storage + 289;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd14", 10) == 0)
                    {
{
return dev_storage + 274;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd14", 10) == 0)
                    {
{
return dev_storage + 188;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc14", 10) == 0)
                {
{
return dev_storage + 259;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb14", 10) == 0)
                {
{
return dev_storage + 244;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda14", 10) == 0)
                {
{
return dev_storage + 229;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [8])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/st124", 10) == 0)
                    {
{
return dev_storage + 746;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/st114", 10) == 0)
                    {
{
return dev_storage + 736;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '0':
                  if (strncmp (KR_keyword, "/dev/st104", 10) == 0)
                    {
{
return dev_storage + 726;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '3':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz13", 10) == 0)
                {
{
return dev_storage + 603;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [8])
                {
                case 'S':
                  if (strncmp (KR_keyword, "/dev/ttyS3", 10) == 0)
                    {
{
return dev_storage + 818;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/tty63", 10) == 0)
                    {
{
return dev_storage + 814;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/tty53", 10) == 0)
                    {
{
return dev_storage + 804;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/tty43", 10) == 0)
                    {
{
return dev_storage + 794;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/tty33", 10) == 0)
                    {
{
return dev_storage + 784;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/tty23", 10) == 0)
                    {
{
return dev_storage + 774;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 't':
                      if (strncmp (KR_keyword, "/dev/tty13", 10) == 0)
                        {
{
return dev_storage + 764;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy13", 10) == 0)
                        {
{
return dev_storage + 588;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx13", 10) == 0)
                {
{
return dev_storage + 573;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw13", 10) == 0)
                {
{
return dev_storage + 558;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv13", 10) == 0)
                {
{
return dev_storage + 543;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu13", 10) == 0)
                {
{
return dev_storage + 528;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [8])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst93", 10) == 0)
                    {
{
return dev_storage + 134;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst83", 10) == 0)
                    {
{
return dev_storage + 124;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst73", 10) == 0)
                    {
{
return dev_storage + 114;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst63", 10) == 0)
                    {
{
return dev_storage + 104;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst53", 10) == 0)
                    {
{
return dev_storage + 94;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst43", 10) == 0)
                    {
{
return dev_storage + 84;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst33", 10) == 0)
                    {
{
return dev_storage + 74;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst23", 10) == 0)
                    {
{
return dev_storage + 64;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt13", 10) == 0)
                        {
{
return dev_storage + 513;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst13", 10) == 0)
                        {
{
return dev_storage + 54;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds13", 10) == 0)
                {
{
return dev_storage + 498;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr13", 10) == 0)
                {
{
return dev_storage + 483;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq13", 10) == 0)
                {
{
return dev_storage + 468;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp13", 10) == 0)
                {
{
return dev_storage + 453;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo13", 10) == 0)
                {
{
return dev_storage + 438;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn13", 10) == 0)
                {
{
return dev_storage + 423;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm13", 10) == 0)
                    {
{
return dev_storage + 408;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com13", 10) == 0)
                    {
{
return dev_storage + 14;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl13", 10) == 0)
                {
{
return dev_storage + 393;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk13", 10) == 0)
                {
{
return dev_storage + 378;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj13", 10) == 0)
                {
{
return dev_storage + 363;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi13", 10) == 0)
                {
{
return dev_storage + 348;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh13", 10) == 0)
                {
{
return dev_storage + 333;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg13", 10) == 0)
                {
{
return dev_storage + 318;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf13", 10) == 0)
                {
{
return dev_storage + 303;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde13", 10) == 0)
                {
{
return dev_storage + 288;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd13", 10) == 0)
                    {
{
return dev_storage + 273;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd13", 10) == 0)
                    {
{
return dev_storage + 187;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc13", 10) == 0)
                {
{
return dev_storage + 258;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb13", 10) == 0)
                {
{
return dev_storage + 243;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda13", 10) == 0)
                {
{
return dev_storage + 228;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [8])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/st123", 10) == 0)
                    {
{
return dev_storage + 745;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/st113", 10) == 0)
                    {
{
return dev_storage + 735;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '0':
                  if (strncmp (KR_keyword, "/dev/st103", 10) == 0)
                    {
{
return dev_storage + 725;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '2':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz12", 10) == 0)
                {
{
return dev_storage + 602;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [8])
                {
                case 'S':
                  if (strncmp (KR_keyword, "/dev/ttyS2", 10) == 0)
                    {
{
return dev_storage + 817;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/tty62", 10) == 0)
                    {
{
return dev_storage + 813;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/tty52", 10) == 0)
                    {
{
return dev_storage + 803;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/tty42", 10) == 0)
                    {
{
return dev_storage + 793;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/tty32", 10) == 0)
                    {
{
return dev_storage + 783;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/tty22", 10) == 0)
                    {
{
return dev_storage + 773;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 't':
                      if (strncmp (KR_keyword, "/dev/tty12", 10) == 0)
                        {
{
return dev_storage + 763;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy12", 10) == 0)
                        {
{
return dev_storage + 587;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx12", 10) == 0)
                {
{
return dev_storage + 572;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw12", 10) == 0)
                {
{
return dev_storage + 557;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv12", 10) == 0)
                {
{
return dev_storage + 542;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu12", 10) == 0)
                {
{
return dev_storage + 527;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [8])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst92", 10) == 0)
                    {
{
return dev_storage + 133;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst82", 10) == 0)
                    {
{
return dev_storage + 123;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst72", 10) == 0)
                    {
{
return dev_storage + 113;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst62", 10) == 0)
                    {
{
return dev_storage + 103;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst52", 10) == 0)
                    {
{
return dev_storage + 93;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst42", 10) == 0)
                    {
{
return dev_storage + 83;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst32", 10) == 0)
                    {
{
return dev_storage + 73;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst22", 10) == 0)
                    {
{
return dev_storage + 63;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt12", 10) == 0)
                        {
{
return dev_storage + 512;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst12", 10) == 0)
                        {
{
return dev_storage + 53;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds12", 10) == 0)
                {
{
return dev_storage + 497;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr12", 10) == 0)
                {
{
return dev_storage + 482;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq12", 10) == 0)
                {
{
return dev_storage + 467;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp12", 10) == 0)
                {
{
return dev_storage + 452;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo12", 10) == 0)
                {
{
return dev_storage + 437;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn12", 10) == 0)
                {
{
return dev_storage + 422;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm12", 10) == 0)
                    {
{
return dev_storage + 407;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com12", 10) == 0)
                    {
{
return dev_storage + 13;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl12", 10) == 0)
                {
{
return dev_storage + 392;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk12", 10) == 0)
                {
{
return dev_storage + 377;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj12", 10) == 0)
                {
{
return dev_storage + 362;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi12", 10) == 0)
                {
{
return dev_storage + 347;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh12", 10) == 0)
                {
{
return dev_storage + 332;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg12", 10) == 0)
                {
{
return dev_storage + 317;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf12", 10) == 0)
                {
{
return dev_storage + 302;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde12", 10) == 0)
                {
{
return dev_storage + 287;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd12", 10) == 0)
                    {
{
return dev_storage + 272;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd12", 10) == 0)
                    {
{
return dev_storage + 186;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc12", 10) == 0)
                {
{
return dev_storage + 257;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb12", 10) == 0)
                {
{
return dev_storage + 242;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda12", 10) == 0)
                {
{
return dev_storage + 227;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [8])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/st122", 10) == 0)
                    {
{
return dev_storage + 744;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/st112", 10) == 0)
                    {
{
return dev_storage + 734;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '0':
                  if (strncmp (KR_keyword, "/dev/st102", 10) == 0)
                    {
{
return dev_storage + 724;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '1':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz11", 10) == 0)
                {
{
return dev_storage + 601;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [8])
                {
                case 'S':
                  if (strncmp (KR_keyword, "/dev/ttyS1", 10) == 0)
                    {
{
return dev_storage + 816;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/tty61", 10) == 0)
                    {
{
return dev_storage + 812;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/tty51", 10) == 0)
                    {
{
return dev_storage + 802;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/tty41", 10) == 0)
                    {
{
return dev_storage + 792;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/tty31", 10) == 0)
                    {
{
return dev_storage + 782;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/tty21", 10) == 0)
                    {
{
return dev_storage + 772;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 't':
                      if (strncmp (KR_keyword, "/dev/tty11", 10) == 0)
                        {
{
return dev_storage + 762;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy11", 10) == 0)
                        {
{
return dev_storage + 586;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx11", 10) == 0)
                {
{
return dev_storage + 571;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw11", 10) == 0)
                {
{
return dev_storage + 556;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv11", 10) == 0)
                {
{
return dev_storage + 541;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu11", 10) == 0)
                {
{
return dev_storage + 526;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [8])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst91", 10) == 0)
                    {
{
return dev_storage + 132;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst81", 10) == 0)
                    {
{
return dev_storage + 122;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst71", 10) == 0)
                    {
{
return dev_storage + 112;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst61", 10) == 0)
                    {
{
return dev_storage + 102;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst51", 10) == 0)
                    {
{
return dev_storage + 92;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst41", 10) == 0)
                    {
{
return dev_storage + 82;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst31", 10) == 0)
                    {
{
return dev_storage + 72;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst21", 10) == 0)
                    {
{
return dev_storage + 62;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt11", 10) == 0)
                        {
{
return dev_storage + 511;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst11", 10) == 0)
                        {
{
return dev_storage + 52;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds11", 10) == 0)
                {
{
return dev_storage + 496;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr11", 10) == 0)
                {
{
return dev_storage + 481;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq11", 10) == 0)
                {
{
return dev_storage + 466;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp11", 10) == 0)
                {
{
return dev_storage + 451;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo11", 10) == 0)
                {
{
return dev_storage + 436;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn11", 10) == 0)
                {
{
return dev_storage + 421;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm11", 10) == 0)
                    {
{
return dev_storage + 406;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com11", 10) == 0)
                    {
{
return dev_storage + 12;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl11", 10) == 0)
                {
{
return dev_storage + 391;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk11", 10) == 0)
                {
{
return dev_storage + 376;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj11", 10) == 0)
                {
{
return dev_storage + 361;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi11", 10) == 0)
                {
{
return dev_storage + 346;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh11", 10) == 0)
                {
{
return dev_storage + 331;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg11", 10) == 0)
                {
{
return dev_storage + 316;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf11", 10) == 0)
                {
{
return dev_storage + 301;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde11", 10) == 0)
                {
{
return dev_storage + 286;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd11", 10) == 0)
                    {
{
return dev_storage + 271;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd11", 10) == 0)
                    {
{
return dev_storage + 185;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc11", 10) == 0)
                {
{
return dev_storage + 256;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb11", 10) == 0)
                {
{
return dev_storage + 241;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda11", 10) == 0)
                {
{
return dev_storage + 226;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [8])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/st121", 10) == 0)
                    {
{
return dev_storage + 743;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/st111", 10) == 0)
                    {
{
return dev_storage + 733;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '0':
                  if (strncmp (KR_keyword, "/dev/st101", 10) == 0)
                    {
{
return dev_storage + 723;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '0':
          switch (KR_keyword [7])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdz10", 10) == 0)
                {
{
return dev_storage + 600;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              switch (KR_keyword [8])
                {
                case 'S':
                  if (strncmp (KR_keyword, "/dev/ttyS0", 10) == 0)
                    {
{
return dev_storage + 815;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/tty60", 10) == 0)
                    {
{
return dev_storage + 811;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/tty50", 10) == 0)
                    {
{
return dev_storage + 801;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/tty40", 10) == 0)
                    {
{
return dev_storage + 791;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/tty30", 10) == 0)
                    {
{
return dev_storage + 781;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/tty20", 10) == 0)
                    {
{
return dev_storage + 771;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 't':
                      if (strncmp (KR_keyword, "/dev/tty10", 10) == 0)
                        {
{
return dev_storage + 761;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy10", 10) == 0)
                        {
{
return dev_storage + 585;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdx10", 10) == 0)
                {
{
return dev_storage + 570;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdw10", 10) == 0)
                {
{
return dev_storage + 555;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdv10", 10) == 0)
                {
{
return dev_storage + 540;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdu10", 10) == 0)
                {
{
return dev_storage + 525;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              switch (KR_keyword [8])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst90", 10) == 0)
                    {
{
return dev_storage + 131;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst80", 10) == 0)
                    {
{
return dev_storage + 121;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst70", 10) == 0)
                    {
{
return dev_storage + 111;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst60", 10) == 0)
                    {
{
return dev_storage + 101;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst50", 10) == 0)
                    {
{
return dev_storage + 91;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst40", 10) == 0)
                    {
{
return dev_storage + 81;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst30", 10) == 0)
                    {
{
return dev_storage + 71;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst20", 10) == 0)
                    {
{
return dev_storage + 61;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt10", 10) == 0)
                        {
{
return dev_storage + 510;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst10", 10) == 0)
                        {
{
return dev_storage + 51;

}
                        }
                      else
                        {
{
return	NULL;

}
                        }
                    default:
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sds10", 10) == 0)
                {
{
return dev_storage + 495;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdr10", 10) == 0)
                {
{
return dev_storage + 480;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq10", 10) == 0)
                {
{
return dev_storage + 465;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp10", 10) == 0)
                {
{
return dev_storage + 450;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo10", 10) == 0)
                {
{
return dev_storage + 435;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdn10", 10) == 0)
                {
{
return dev_storage + 420;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm10", 10) == 0)
                    {
{
return dev_storage + 405;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/com10", 10) == 0)
                    {
{
return dev_storage + 11;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdl10", 10) == 0)
                {
{
return dev_storage + 390;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdk10", 10) == 0)
                {
{
return dev_storage + 375;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdj10", 10) == 0)
                {
{
return dev_storage + 360;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdi10", 10) == 0)
                {
{
return dev_storage + 345;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdh10", 10) == 0)
                {
{
return dev_storage + 330;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdg10", 10) == 0)
                {
{
return dev_storage + 315;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdf10", 10) == 0)
                {
{
return dev_storage + 300;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sde10", 10) == 0)
                {
{
return dev_storage + 285;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd10", 10) == 0)
                    {
{
return dev_storage + 270;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd10", 10) == 0)
                    {
{
return dev_storage + 184;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdc10", 10) == 0)
                {
{
return dev_storage + 255;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdb10", 10) == 0)
                {
{
return dev_storage + 240;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sda10", 10) == 0)
                {
{
return dev_storage + 225;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [8])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/st120", 10) == 0)
                    {
{
return dev_storage + 742;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/st110", 10) == 0)
                    {
{
return dev_storage + 732;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '0':
                  if (strncmp (KR_keyword, "/dev/st100", 10) == 0)
                    {
{
return dev_storage + 722;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 11:
      switch (KR_keyword [9])
        {
        case 'u':
          if (strncmp (KR_keyword, "/dev/conout", 11) == 0)
            {
{
return dev_storage + 19;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'o':
          if (strncmp (KR_keyword, "/dev/random", 11) == 0)
            {
{
return dev_storage + 173;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '2':
          switch (KR_keyword [10])
            {
            case '7':
              if (strncmp (KR_keyword, "/dev/nst127", 11) == 0)
                {
{
return dev_storage + 168;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/nst126", 11) == 0)
                {
{
return dev_storage + 167;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/nst125", 11) == 0)
                {
{
return dev_storage + 166;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/nst124", 11) == 0)
                {
{
return dev_storage + 165;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/nst123", 11) == 0)
                {
{
return dev_storage + 164;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/nst122", 11) == 0)
                {
{
return dev_storage + 163;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/nst121", 11) == 0)
                {
{
return dev_storage + 162;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/nst120", 11) == 0)
                {
{
return dev_storage + 161;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '1':
          switch (KR_keyword [10])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/nst119", 11) == 0)
                {
{
return dev_storage + 160;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/nst118", 11) == 0)
                {
{
return dev_storage + 159;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/nst117", 11) == 0)
                {
{
return dev_storage + 158;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/nst116", 11) == 0)
                {
{
return dev_storage + 157;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS15", 11) == 0)
                    {
{
return dev_storage + 830;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst115", 11) == 0)
                    {
{
return dev_storage + 156;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS14", 11) == 0)
                    {
{
return dev_storage + 829;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst114", 11) == 0)
                    {
{
return dev_storage + 155;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS13", 11) == 0)
                    {
{
return dev_storage + 828;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst113", 11) == 0)
                    {
{
return dev_storage + 154;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS12", 11) == 0)
                    {
{
return dev_storage + 827;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst112", 11) == 0)
                    {
{
return dev_storage + 153;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS11", 11) == 0)
                    {
{
return dev_storage + 826;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst111", 11) == 0)
                    {
{
return dev_storage + 152;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            case '0':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS10", 11) == 0)
                    {
{
return dev_storage + 825;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst110", 11) == 0)
                    {
{
return dev_storage + 151;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                default:
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        case '0':
          switch (KR_keyword [10])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/nst109", 11) == 0)
                {
{
return dev_storage + 150;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/nst108", 11) == 0)
                {
{
return dev_storage + 149;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/nst107", 11) == 0)
                {
{
return dev_storage + 148;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/nst106", 11) == 0)
                {
{
return dev_storage + 147;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/nst105", 11) == 0)
                {
{
return dev_storage + 146;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/nst104", 11) == 0)
                {
{
return dev_storage + 145;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/nst103", 11) == 0)
                {
{
return dev_storage + 144;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/nst102", 11) == 0)
                {
{
return dev_storage + 143;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/nst101", 11) == 0)
                {
{
return dev_storage + 142;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/nst100", 11) == 0)
                {
{
return dev_storage + 141;

}
                }
              else
                {
{
return	NULL;

}
                }
            default:
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 12:
      switch (KR_keyword [5])
        {
        case 'w':
          if (strncmp (KR_keyword, "/dev/windows", 12) == 0)
            {
{
return dev_storage + 833;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'u':
          if (strncmp (KR_keyword, "/dev/urandom", 12) == 0)
            {
{
return dev_storage + 832;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'c':
          if (strncmp (KR_keyword, "/dev/console", 12) == 0)
            {
{
return dev_storage + 20;

}
            }
          else
            {
{
return	NULL;

}
            }
        default:
{
return	NULL;

}
        }
    case 14:
          if (strncmp (KR_keyword, "/dev/clipboard", 14) == 0)
            {
{
return dev_storage + 0;

}
            }
          else
            {
{
return	NULL;

}
            }
    default:
{
return	NULL;

}
    }
}






void
device::parse (const char *s)
{
  size_t len = strlen (s);
  const device *dev = KR_find_keyword (s, len);

  if (!dev)
    *this = *fs_dev;
  else
    *this = *dev;
}

void
device::init ()
{
  /* nothing to do... yet */
}

void
device::parse (_major_t major, _minor_t minor)
{
  _dev_t dev = FHDEV (major, minor);

  devn = 0;

  for (unsigned i = 0; i < (sizeof (dev_storage) / sizeof (dev_storage[0])); i++)
    if (dev_storage[i].devn == dev)
      {
	*this = dev_storage[i];
	break;
      }

  if (!*this)
    devn = FHDEV (major, minor);
  return;
}

void
device::parse (_dev_t dev)
{
  parse (_major (dev), _minor (dev));
}

void
device::tty_to_real_device ()
{
  if (!real_tty_attached (myself))
    *this = myself->ctty < 0 ? dev_bad_storage : *console_dev;
  else
    parse (DEV_TTYS_MAJOR, myself->ctty);
}



