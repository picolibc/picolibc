

#include "winsup.h"
#include "devices.h"
#include "sys/cygwin.h"
#include "tty.h"
#include "pinfo.h"
#include "shared_info.h"
#include "path.h"
#include "fhandler.h"
#include "ntdll.h"

typedef const device *KR_device_t;


static KR_device_t KR_find_keyword (const char *KR_keyword, int KR_length);




static int
exists_internal (const device&)
{
  return false;
}

static int
exists (const device&)
{
  return true;
}

/* Check existence of POSIX devices backed by real NT devices. */
static int
exists_ntdev (const device& dev)
{
  WCHAR wpath[MAX_PATH];
  UNICODE_STRING upath;
  OBJECT_ATTRIBUTES attr;
  HANDLE h;
  NTSTATUS status;

  sys_mbstowcs (wpath, MAX_PATH, dev.native);
  RtlInitUnicodeString (&upath, wpath);
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  /* Except for the serial IO devices, the native paths are
     direct device paths, not symlinks, so every status code
     except for "NOT_FOUND" means the device exists. */
  status = NtOpenSymbolicLinkObject (&h, SYMBOLIC_LINK_QUERY, &attr);
  switch (status)
    {
    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_OBJECT_PATH_NOT_FOUND:
      return false;
    case STATUS_SUCCESS:
      NtClose (h);
    default:
      break;
    }
  return true;
}

/* Don't list via readdir but allow as a direct reference. */
static int
exists_ntdev_silent (const device& dev)
{
  return exists_ntdev (dev) ? -1 : false;
}

static int
exists_console (const device& dev)
{
  fh_devices devn = *const_cast<device *> (&dev);
  switch (devn)
    {
    case FH_CONSOLE:
    case FH_CONIN:
    case FH_CONOUT:
      return fhandler_console::exists ();
    default:
      /* Only show my own console device (for now?) */
      return iscons_dev (myself->ctty) && myself->ctty == devn;
    }
}

static int
exists_pty (const device& dev)
{
  /* Only existing slave ptys. */
  return cygwin_shared->tty.connect (dev.get_minor ()) != -1;
}

const device dev_cygdrive_storage =
  {"/cygdrive", {FH_CYGDRIVE}, "", exists};

const device dev_fs_storage =
  {"", {FH_FS}, "", exists};

const device dev_proc_storage =
  {"", {FH_PROC}, "", exists};

const device dev_procnet_storage =
  {"", {FH_PROCNET}, "", exists};

const device dev_procsys_storage =
  {"", {FH_PROCSYS}, "", exists};

const device dev_procsysvipc_storage =
  {"", {FH_PROCSYSVIPC}, "", exists};

const device dev_netdrive_storage =
  {"", {FH_NETDRIVE}, "", exists};

const device dev_registry_storage =
  {"", {FH_REGISTRY}, "", exists_internal};

const device dev_piper_storage =
  {"", {FH_PIPER}, "", exists_internal};

const device dev_pipew_storage =
  {"", {FH_PIPEW}, "", exists_internal};

const device dev_tcp_storage =
  {"", {FH_TCP}, "", exists_internal};

const device dev_udp_storage =
  {"", {FH_UDP}, "", exists_internal};

const device dev_stream_storage =
  {"", {FH_STREAM}, "", exists_internal};

const device dev_dgram_storage =
  {"", {FH_DGRAM}, "", exists_internal};

const device dev_bad_storage =
  {"", {FH_NADA}, "", exists_internal};

const device dev_error_storage =
  {"", {FH_ERROR}, "", exists_internal};

#define BRACK(x) {devn_int: x}
const _RDATA device dev_storage[] =
{
  {"/dev", BRACK(FH_DEV), "", exists, S_IFDIR, false},
  {"/dev/clipboard", BRACK(FH_CLIPBOARD), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {"/dev/com1", BRACK(FHDEV(DEV_SERIAL_MAJOR, 0)), "\\??\\COM1", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com2", BRACK(FHDEV(DEV_SERIAL_MAJOR, 1)), "\\??\\COM2", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com3", BRACK(FHDEV(DEV_SERIAL_MAJOR, 2)), "\\??\\COM3", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com4", BRACK(FHDEV(DEV_SERIAL_MAJOR, 3)), "\\??\\COM4", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com5", BRACK(FHDEV(DEV_SERIAL_MAJOR, 4)), "\\??\\COM5", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com6", BRACK(FHDEV(DEV_SERIAL_MAJOR, 5)), "\\??\\COM6", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com7", BRACK(FHDEV(DEV_SERIAL_MAJOR, 6)), "\\??\\COM7", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com8", BRACK(FHDEV(DEV_SERIAL_MAJOR, 7)), "\\??\\COM8", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com9", BRACK(FHDEV(DEV_SERIAL_MAJOR, 8)), "\\??\\COM9", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com10", BRACK(FHDEV(DEV_SERIAL_MAJOR, 9)), "\\??\\COM10", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com11", BRACK(FHDEV(DEV_SERIAL_MAJOR, 10)), "\\??\\COM11", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com12", BRACK(FHDEV(DEV_SERIAL_MAJOR, 11)), "\\??\\COM12", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com13", BRACK(FHDEV(DEV_SERIAL_MAJOR, 12)), "\\??\\COM13", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com14", BRACK(FHDEV(DEV_SERIAL_MAJOR, 13)), "\\??\\COM14", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com15", BRACK(FHDEV(DEV_SERIAL_MAJOR, 14)), "\\??\\COM15", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/com16", BRACK(FHDEV(DEV_SERIAL_MAJOR, 15)), "\\??\\COM16", exists_ntdev_silent, S_IFCHR, true},
  {"/dev/conin", BRACK(FH_CONIN), "/dev/conin", exists_console, S_IFCHR, true},
  {"/dev/conout", BRACK(FH_CONOUT), "/dev/conout", exists_console, S_IFCHR, true},
  {"/dev/cons0", BRACK(FHDEV(DEV_CONS_MAJOR, 0)), "/dev/cons0", exists_console, S_IFCHR, true},
  {"/dev/cons1", BRACK(FHDEV(DEV_CONS_MAJOR, 1)), "/dev/cons1", exists_console, S_IFCHR, true},
  {"/dev/cons2", BRACK(FHDEV(DEV_CONS_MAJOR, 2)), "/dev/cons2", exists_console, S_IFCHR, true},
  {"/dev/cons3", BRACK(FHDEV(DEV_CONS_MAJOR, 3)), "/dev/cons3", exists_console, S_IFCHR, true},
  {"/dev/cons4", BRACK(FHDEV(DEV_CONS_MAJOR, 4)), "/dev/cons4", exists_console, S_IFCHR, true},
  {"/dev/cons5", BRACK(FHDEV(DEV_CONS_MAJOR, 5)), "/dev/cons5", exists_console, S_IFCHR, true},
  {"/dev/cons6", BRACK(FHDEV(DEV_CONS_MAJOR, 6)), "/dev/cons6", exists_console, S_IFCHR, true},
  {"/dev/cons7", BRACK(FHDEV(DEV_CONS_MAJOR, 7)), "/dev/cons7", exists_console, S_IFCHR, true},
  {"/dev/cons8", BRACK(FHDEV(DEV_CONS_MAJOR, 8)), "/dev/cons8", exists_console, S_IFCHR, true},
  {"/dev/cons9", BRACK(FHDEV(DEV_CONS_MAJOR, 9)), "/dev/cons9", exists_console, S_IFCHR, true},
  {"/dev/cons10", BRACK(FHDEV(DEV_CONS_MAJOR, 10)), "/dev/cons10", exists_console, S_IFCHR, true},
  {"/dev/cons11", BRACK(FHDEV(DEV_CONS_MAJOR, 11)), "/dev/cons11", exists_console, S_IFCHR, true},
  {"/dev/cons12", BRACK(FHDEV(DEV_CONS_MAJOR, 12)), "/dev/cons12", exists_console, S_IFCHR, true},
  {"/dev/cons13", BRACK(FHDEV(DEV_CONS_MAJOR, 13)), "/dev/cons13", exists_console, S_IFCHR, true},
  {"/dev/cons14", BRACK(FHDEV(DEV_CONS_MAJOR, 14)), "/dev/cons14", exists_console, S_IFCHR, true},
  {"/dev/cons15", BRACK(FHDEV(DEV_CONS_MAJOR, 15)), "/dev/cons15", exists_console, S_IFCHR, true},
  {"/dev/cons16", BRACK(FHDEV(DEV_CONS_MAJOR, 16)), "/dev/cons16", exists_console, S_IFCHR, true},
  {"/dev/cons17", BRACK(FHDEV(DEV_CONS_MAJOR, 17)), "/dev/cons17", exists_console, S_IFCHR, true},
  {"/dev/cons18", BRACK(FHDEV(DEV_CONS_MAJOR, 18)), "/dev/cons18", exists_console, S_IFCHR, true},
  {"/dev/cons19", BRACK(FHDEV(DEV_CONS_MAJOR, 19)), "/dev/cons19", exists_console, S_IFCHR, true},
  {"/dev/cons20", BRACK(FHDEV(DEV_CONS_MAJOR, 20)), "/dev/cons20", exists_console, S_IFCHR, true},
  {"/dev/cons21", BRACK(FHDEV(DEV_CONS_MAJOR, 21)), "/dev/cons21", exists_console, S_IFCHR, true},
  {"/dev/cons22", BRACK(FHDEV(DEV_CONS_MAJOR, 22)), "/dev/cons22", exists_console, S_IFCHR, true},
  {"/dev/cons23", BRACK(FHDEV(DEV_CONS_MAJOR, 23)), "/dev/cons23", exists_console, S_IFCHR, true},
  {"/dev/cons24", BRACK(FHDEV(DEV_CONS_MAJOR, 24)), "/dev/cons24", exists_console, S_IFCHR, true},
  {"/dev/cons25", BRACK(FHDEV(DEV_CONS_MAJOR, 25)), "/dev/cons25", exists_console, S_IFCHR, true},
  {"/dev/cons26", BRACK(FHDEV(DEV_CONS_MAJOR, 26)), "/dev/cons26", exists_console, S_IFCHR, true},
  {"/dev/cons27", BRACK(FHDEV(DEV_CONS_MAJOR, 27)), "/dev/cons27", exists_console, S_IFCHR, true},
  {"/dev/cons28", BRACK(FHDEV(DEV_CONS_MAJOR, 28)), "/dev/cons28", exists_console, S_IFCHR, true},
  {"/dev/cons29", BRACK(FHDEV(DEV_CONS_MAJOR, 29)), "/dev/cons29", exists_console, S_IFCHR, true},
  {"/dev/cons30", BRACK(FHDEV(DEV_CONS_MAJOR, 30)), "/dev/cons30", exists_console, S_IFCHR, true},
  {"/dev/cons31", BRACK(FHDEV(DEV_CONS_MAJOR, 31)), "/dev/cons31", exists_console, S_IFCHR, true},
  {"/dev/cons32", BRACK(FHDEV(DEV_CONS_MAJOR, 32)), "/dev/cons32", exists_console, S_IFCHR, true},
  {"/dev/cons33", BRACK(FHDEV(DEV_CONS_MAJOR, 33)), "/dev/cons33", exists_console, S_IFCHR, true},
  {"/dev/cons34", BRACK(FHDEV(DEV_CONS_MAJOR, 34)), "/dev/cons34", exists_console, S_IFCHR, true},
  {"/dev/cons35", BRACK(FHDEV(DEV_CONS_MAJOR, 35)), "/dev/cons35", exists_console, S_IFCHR, true},
  {"/dev/cons36", BRACK(FHDEV(DEV_CONS_MAJOR, 36)), "/dev/cons36", exists_console, S_IFCHR, true},
  {"/dev/cons37", BRACK(FHDEV(DEV_CONS_MAJOR, 37)), "/dev/cons37", exists_console, S_IFCHR, true},
  {"/dev/cons38", BRACK(FHDEV(DEV_CONS_MAJOR, 38)), "/dev/cons38", exists_console, S_IFCHR, true},
  {"/dev/cons39", BRACK(FHDEV(DEV_CONS_MAJOR, 39)), "/dev/cons39", exists_console, S_IFCHR, true},
  {"/dev/cons40", BRACK(FHDEV(DEV_CONS_MAJOR, 40)), "/dev/cons40", exists_console, S_IFCHR, true},
  {"/dev/cons41", BRACK(FHDEV(DEV_CONS_MAJOR, 41)), "/dev/cons41", exists_console, S_IFCHR, true},
  {"/dev/cons42", BRACK(FHDEV(DEV_CONS_MAJOR, 42)), "/dev/cons42", exists_console, S_IFCHR, true},
  {"/dev/cons43", BRACK(FHDEV(DEV_CONS_MAJOR, 43)), "/dev/cons43", exists_console, S_IFCHR, true},
  {"/dev/cons44", BRACK(FHDEV(DEV_CONS_MAJOR, 44)), "/dev/cons44", exists_console, S_IFCHR, true},
  {"/dev/cons45", BRACK(FHDEV(DEV_CONS_MAJOR, 45)), "/dev/cons45", exists_console, S_IFCHR, true},
  {"/dev/cons46", BRACK(FHDEV(DEV_CONS_MAJOR, 46)), "/dev/cons46", exists_console, S_IFCHR, true},
  {"/dev/cons47", BRACK(FHDEV(DEV_CONS_MAJOR, 47)), "/dev/cons47", exists_console, S_IFCHR, true},
  {"/dev/cons48", BRACK(FHDEV(DEV_CONS_MAJOR, 48)), "/dev/cons48", exists_console, S_IFCHR, true},
  {"/dev/cons49", BRACK(FHDEV(DEV_CONS_MAJOR, 49)), "/dev/cons49", exists_console, S_IFCHR, true},
  {"/dev/cons50", BRACK(FHDEV(DEV_CONS_MAJOR, 50)), "/dev/cons50", exists_console, S_IFCHR, true},
  {"/dev/cons51", BRACK(FHDEV(DEV_CONS_MAJOR, 51)), "/dev/cons51", exists_console, S_IFCHR, true},
  {"/dev/cons52", BRACK(FHDEV(DEV_CONS_MAJOR, 52)), "/dev/cons52", exists_console, S_IFCHR, true},
  {"/dev/cons53", BRACK(FHDEV(DEV_CONS_MAJOR, 53)), "/dev/cons53", exists_console, S_IFCHR, true},
  {"/dev/cons54", BRACK(FHDEV(DEV_CONS_MAJOR, 54)), "/dev/cons54", exists_console, S_IFCHR, true},
  {"/dev/cons55", BRACK(FHDEV(DEV_CONS_MAJOR, 55)), "/dev/cons55", exists_console, S_IFCHR, true},
  {"/dev/cons56", BRACK(FHDEV(DEV_CONS_MAJOR, 56)), "/dev/cons56", exists_console, S_IFCHR, true},
  {"/dev/cons57", BRACK(FHDEV(DEV_CONS_MAJOR, 57)), "/dev/cons57", exists_console, S_IFCHR, true},
  {"/dev/cons58", BRACK(FHDEV(DEV_CONS_MAJOR, 58)), "/dev/cons58", exists_console, S_IFCHR, true},
  {"/dev/cons59", BRACK(FHDEV(DEV_CONS_MAJOR, 59)), "/dev/cons59", exists_console, S_IFCHR, true},
  {"/dev/cons60", BRACK(FHDEV(DEV_CONS_MAJOR, 60)), "/dev/cons60", exists_console, S_IFCHR, true},
  {"/dev/cons61", BRACK(FHDEV(DEV_CONS_MAJOR, 61)), "/dev/cons61", exists_console, S_IFCHR, true},
  {"/dev/cons62", BRACK(FHDEV(DEV_CONS_MAJOR, 62)), "/dev/cons62", exists_console, S_IFCHR, true},
  {"/dev/cons63", BRACK(FHDEV(DEV_CONS_MAJOR, 63)), "/dev/cons63", exists_console, S_IFCHR, true},
  {"/dev/console", BRACK(FH_CONSOLE), "/dev/console", exists_console, S_IFCHR, true},
  {"/dev/dsp", BRACK(FH_OSS_DSP), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {"/dev/fd0", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 0)), "\\Device\\Floppy0", exists_ntdev, S_IFBLK, true},
  {"/dev/fd1", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 1)), "\\Device\\Floppy1", exists_ntdev, S_IFBLK, true},
  {"/dev/fd2", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 2)), "\\Device\\Floppy2", exists_ntdev, S_IFBLK, true},
  {"/dev/fd3", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 3)), "\\Device\\Floppy3", exists_ntdev, S_IFBLK, true},
  {"/dev/fd4", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 4)), "\\Device\\Floppy4", exists_ntdev, S_IFBLK, true},
  {"/dev/fd5", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 5)), "\\Device\\Floppy5", exists_ntdev, S_IFBLK, true},
  {"/dev/fd6", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 6)), "\\Device\\Floppy6", exists_ntdev, S_IFBLK, true},
  {"/dev/fd7", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 7)), "\\Device\\Floppy7", exists_ntdev, S_IFBLK, true},
  {"/dev/fd8", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 8)), "\\Device\\Floppy8", exists_ntdev, S_IFBLK, true},
  {"/dev/fd9", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 9)), "\\Device\\Floppy9", exists_ntdev, S_IFBLK, true},
  {"/dev/fd10", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 10)), "\\Device\\Floppy10", exists_ntdev, S_IFBLK, true},
  {"/dev/fd11", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 11)), "\\Device\\Floppy11", exists_ntdev, S_IFBLK, true},
  {"/dev/fd12", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 12)), "\\Device\\Floppy12", exists_ntdev, S_IFBLK, true},
  {"/dev/fd13", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 13)), "\\Device\\Floppy13", exists_ntdev, S_IFBLK, true},
  {"/dev/fd14", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 14)), "\\Device\\Floppy14", exists_ntdev, S_IFBLK, true},
  {"/dev/fd15", BRACK(FHDEV(DEV_FLOPPY_MAJOR, 15)), "\\Device\\Floppy15", exists_ntdev, S_IFBLK, true},
  {"/dev/full", BRACK(FH_FULL), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {"/dev/kmsg", BRACK(FH_KMSG), "\\Device\\MailSlot\\cygwin\\dev\\kmsg", exists_ntdev, S_IFCHR, true},
  {"/dev/nst0", BRACK(FHDEV(DEV_TAPE_MAJOR, 128)), "\\Device\\Tape0", exists_ntdev, S_IFBLK, true},
  {"/dev/nst1", BRACK(FHDEV(DEV_TAPE_MAJOR, 129)), "\\Device\\Tape1", exists_ntdev, S_IFBLK, true},
  {"/dev/nst2", BRACK(FHDEV(DEV_TAPE_MAJOR, 130)), "\\Device\\Tape2", exists_ntdev, S_IFBLK, true},
  {"/dev/nst3", BRACK(FHDEV(DEV_TAPE_MAJOR, 131)), "\\Device\\Tape3", exists_ntdev, S_IFBLK, true},
  {"/dev/nst4", BRACK(FHDEV(DEV_TAPE_MAJOR, 132)), "\\Device\\Tape4", exists_ntdev, S_IFBLK, true},
  {"/dev/nst5", BRACK(FHDEV(DEV_TAPE_MAJOR, 133)), "\\Device\\Tape5", exists_ntdev, S_IFBLK, true},
  {"/dev/nst6", BRACK(FHDEV(DEV_TAPE_MAJOR, 134)), "\\Device\\Tape6", exists_ntdev, S_IFBLK, true},
  {"/dev/nst7", BRACK(FHDEV(DEV_TAPE_MAJOR, 135)), "\\Device\\Tape7", exists_ntdev, S_IFBLK, true},
  {"/dev/nst8", BRACK(FHDEV(DEV_TAPE_MAJOR, 136)), "\\Device\\Tape8", exists_ntdev, S_IFBLK, true},
  {"/dev/nst9", BRACK(FHDEV(DEV_TAPE_MAJOR, 137)), "\\Device\\Tape9", exists_ntdev, S_IFBLK, true},
  {"/dev/nst10", BRACK(FHDEV(DEV_TAPE_MAJOR, 138)), "\\Device\\Tape10", exists_ntdev, S_IFBLK, true},
  {"/dev/nst11", BRACK(FHDEV(DEV_TAPE_MAJOR, 139)), "\\Device\\Tape11", exists_ntdev, S_IFBLK, true},
  {"/dev/nst12", BRACK(FHDEV(DEV_TAPE_MAJOR, 140)), "\\Device\\Tape12", exists_ntdev, S_IFBLK, true},
  {"/dev/nst13", BRACK(FHDEV(DEV_TAPE_MAJOR, 141)), "\\Device\\Tape13", exists_ntdev, S_IFBLK, true},
  {"/dev/nst14", BRACK(FHDEV(DEV_TAPE_MAJOR, 142)), "\\Device\\Tape14", exists_ntdev, S_IFBLK, true},
  {"/dev/nst15", BRACK(FHDEV(DEV_TAPE_MAJOR, 143)), "\\Device\\Tape15", exists_ntdev, S_IFBLK, true},
  {"/dev/nst16", BRACK(FHDEV(DEV_TAPE_MAJOR, 144)), "\\Device\\Tape16", exists_ntdev, S_IFBLK, true},
  {"/dev/nst17", BRACK(FHDEV(DEV_TAPE_MAJOR, 145)), "\\Device\\Tape17", exists_ntdev, S_IFBLK, true},
  {"/dev/nst18", BRACK(FHDEV(DEV_TAPE_MAJOR, 146)), "\\Device\\Tape18", exists_ntdev, S_IFBLK, true},
  {"/dev/nst19", BRACK(FHDEV(DEV_TAPE_MAJOR, 147)), "\\Device\\Tape19", exists_ntdev, S_IFBLK, true},
  {"/dev/nst20", BRACK(FHDEV(DEV_TAPE_MAJOR, 148)), "\\Device\\Tape20", exists_ntdev, S_IFBLK, true},
  {"/dev/nst21", BRACK(FHDEV(DEV_TAPE_MAJOR, 149)), "\\Device\\Tape21", exists_ntdev, S_IFBLK, true},
  {"/dev/nst22", BRACK(FHDEV(DEV_TAPE_MAJOR, 150)), "\\Device\\Tape22", exists_ntdev, S_IFBLK, true},
  {"/dev/nst23", BRACK(FHDEV(DEV_TAPE_MAJOR, 151)), "\\Device\\Tape23", exists_ntdev, S_IFBLK, true},
  {"/dev/nst24", BRACK(FHDEV(DEV_TAPE_MAJOR, 152)), "\\Device\\Tape24", exists_ntdev, S_IFBLK, true},
  {"/dev/nst25", BRACK(FHDEV(DEV_TAPE_MAJOR, 153)), "\\Device\\Tape25", exists_ntdev, S_IFBLK, true},
  {"/dev/nst26", BRACK(FHDEV(DEV_TAPE_MAJOR, 154)), "\\Device\\Tape26", exists_ntdev, S_IFBLK, true},
  {"/dev/nst27", BRACK(FHDEV(DEV_TAPE_MAJOR, 155)), "\\Device\\Tape27", exists_ntdev, S_IFBLK, true},
  {"/dev/nst28", BRACK(FHDEV(DEV_TAPE_MAJOR, 156)), "\\Device\\Tape28", exists_ntdev, S_IFBLK, true},
  {"/dev/nst29", BRACK(FHDEV(DEV_TAPE_MAJOR, 157)), "\\Device\\Tape29", exists_ntdev, S_IFBLK, true},
  {"/dev/nst30", BRACK(FHDEV(DEV_TAPE_MAJOR, 158)), "\\Device\\Tape30", exists_ntdev, S_IFBLK, true},
  {"/dev/nst31", BRACK(FHDEV(DEV_TAPE_MAJOR, 159)), "\\Device\\Tape31", exists_ntdev, S_IFBLK, true},
  {"/dev/nst32", BRACK(FHDEV(DEV_TAPE_MAJOR, 160)), "\\Device\\Tape32", exists_ntdev, S_IFBLK, true},
  {"/dev/nst33", BRACK(FHDEV(DEV_TAPE_MAJOR, 161)), "\\Device\\Tape33", exists_ntdev, S_IFBLK, true},
  {"/dev/nst34", BRACK(FHDEV(DEV_TAPE_MAJOR, 162)), "\\Device\\Tape34", exists_ntdev, S_IFBLK, true},
  {"/dev/nst35", BRACK(FHDEV(DEV_TAPE_MAJOR, 163)), "\\Device\\Tape35", exists_ntdev, S_IFBLK, true},
  {"/dev/nst36", BRACK(FHDEV(DEV_TAPE_MAJOR, 164)), "\\Device\\Tape36", exists_ntdev, S_IFBLK, true},
  {"/dev/nst37", BRACK(FHDEV(DEV_TAPE_MAJOR, 165)), "\\Device\\Tape37", exists_ntdev, S_IFBLK, true},
  {"/dev/nst38", BRACK(FHDEV(DEV_TAPE_MAJOR, 166)), "\\Device\\Tape38", exists_ntdev, S_IFBLK, true},
  {"/dev/nst39", BRACK(FHDEV(DEV_TAPE_MAJOR, 167)), "\\Device\\Tape39", exists_ntdev, S_IFBLK, true},
  {"/dev/nst40", BRACK(FHDEV(DEV_TAPE_MAJOR, 168)), "\\Device\\Tape40", exists_ntdev, S_IFBLK, true},
  {"/dev/nst41", BRACK(FHDEV(DEV_TAPE_MAJOR, 169)), "\\Device\\Tape41", exists_ntdev, S_IFBLK, true},
  {"/dev/nst42", BRACK(FHDEV(DEV_TAPE_MAJOR, 170)), "\\Device\\Tape42", exists_ntdev, S_IFBLK, true},
  {"/dev/nst43", BRACK(FHDEV(DEV_TAPE_MAJOR, 171)), "\\Device\\Tape43", exists_ntdev, S_IFBLK, true},
  {"/dev/nst44", BRACK(FHDEV(DEV_TAPE_MAJOR, 172)), "\\Device\\Tape44", exists_ntdev, S_IFBLK, true},
  {"/dev/nst45", BRACK(FHDEV(DEV_TAPE_MAJOR, 173)), "\\Device\\Tape45", exists_ntdev, S_IFBLK, true},
  {"/dev/nst46", BRACK(FHDEV(DEV_TAPE_MAJOR, 174)), "\\Device\\Tape46", exists_ntdev, S_IFBLK, true},
  {"/dev/nst47", BRACK(FHDEV(DEV_TAPE_MAJOR, 175)), "\\Device\\Tape47", exists_ntdev, S_IFBLK, true},
  {"/dev/nst48", BRACK(FHDEV(DEV_TAPE_MAJOR, 176)), "\\Device\\Tape48", exists_ntdev, S_IFBLK, true},
  {"/dev/nst49", BRACK(FHDEV(DEV_TAPE_MAJOR, 177)), "\\Device\\Tape49", exists_ntdev, S_IFBLK, true},
  {"/dev/nst50", BRACK(FHDEV(DEV_TAPE_MAJOR, 178)), "\\Device\\Tape50", exists_ntdev, S_IFBLK, true},
  {"/dev/nst51", BRACK(FHDEV(DEV_TAPE_MAJOR, 179)), "\\Device\\Tape51", exists_ntdev, S_IFBLK, true},
  {"/dev/nst52", BRACK(FHDEV(DEV_TAPE_MAJOR, 180)), "\\Device\\Tape52", exists_ntdev, S_IFBLK, true},
  {"/dev/nst53", BRACK(FHDEV(DEV_TAPE_MAJOR, 181)), "\\Device\\Tape53", exists_ntdev, S_IFBLK, true},
  {"/dev/nst54", BRACK(FHDEV(DEV_TAPE_MAJOR, 182)), "\\Device\\Tape54", exists_ntdev, S_IFBLK, true},
  {"/dev/nst55", BRACK(FHDEV(DEV_TAPE_MAJOR, 183)), "\\Device\\Tape55", exists_ntdev, S_IFBLK, true},
  {"/dev/nst56", BRACK(FHDEV(DEV_TAPE_MAJOR, 184)), "\\Device\\Tape56", exists_ntdev, S_IFBLK, true},
  {"/dev/nst57", BRACK(FHDEV(DEV_TAPE_MAJOR, 185)), "\\Device\\Tape57", exists_ntdev, S_IFBLK, true},
  {"/dev/nst58", BRACK(FHDEV(DEV_TAPE_MAJOR, 186)), "\\Device\\Tape58", exists_ntdev, S_IFBLK, true},
  {"/dev/nst59", BRACK(FHDEV(DEV_TAPE_MAJOR, 187)), "\\Device\\Tape59", exists_ntdev, S_IFBLK, true},
  {"/dev/nst60", BRACK(FHDEV(DEV_TAPE_MAJOR, 188)), "\\Device\\Tape60", exists_ntdev, S_IFBLK, true},
  {"/dev/nst61", BRACK(FHDEV(DEV_TAPE_MAJOR, 189)), "\\Device\\Tape61", exists_ntdev, S_IFBLK, true},
  {"/dev/nst62", BRACK(FHDEV(DEV_TAPE_MAJOR, 190)), "\\Device\\Tape62", exists_ntdev, S_IFBLK, true},
  {"/dev/nst63", BRACK(FHDEV(DEV_TAPE_MAJOR, 191)), "\\Device\\Tape63", exists_ntdev, S_IFBLK, true},
  {"/dev/nst64", BRACK(FHDEV(DEV_TAPE_MAJOR, 192)), "\\Device\\Tape64", exists_ntdev, S_IFBLK, true},
  {"/dev/nst65", BRACK(FHDEV(DEV_TAPE_MAJOR, 193)), "\\Device\\Tape65", exists_ntdev, S_IFBLK, true},
  {"/dev/nst66", BRACK(FHDEV(DEV_TAPE_MAJOR, 194)), "\\Device\\Tape66", exists_ntdev, S_IFBLK, true},
  {"/dev/nst67", BRACK(FHDEV(DEV_TAPE_MAJOR, 195)), "\\Device\\Tape67", exists_ntdev, S_IFBLK, true},
  {"/dev/nst68", BRACK(FHDEV(DEV_TAPE_MAJOR, 196)), "\\Device\\Tape68", exists_ntdev, S_IFBLK, true},
  {"/dev/nst69", BRACK(FHDEV(DEV_TAPE_MAJOR, 197)), "\\Device\\Tape69", exists_ntdev, S_IFBLK, true},
  {"/dev/nst70", BRACK(FHDEV(DEV_TAPE_MAJOR, 198)), "\\Device\\Tape70", exists_ntdev, S_IFBLK, true},
  {"/dev/nst71", BRACK(FHDEV(DEV_TAPE_MAJOR, 199)), "\\Device\\Tape71", exists_ntdev, S_IFBLK, true},
  {"/dev/nst72", BRACK(FHDEV(DEV_TAPE_MAJOR, 200)), "\\Device\\Tape72", exists_ntdev, S_IFBLK, true},
  {"/dev/nst73", BRACK(FHDEV(DEV_TAPE_MAJOR, 201)), "\\Device\\Tape73", exists_ntdev, S_IFBLK, true},
  {"/dev/nst74", BRACK(FHDEV(DEV_TAPE_MAJOR, 202)), "\\Device\\Tape74", exists_ntdev, S_IFBLK, true},
  {"/dev/nst75", BRACK(FHDEV(DEV_TAPE_MAJOR, 203)), "\\Device\\Tape75", exists_ntdev, S_IFBLK, true},
  {"/dev/nst76", BRACK(FHDEV(DEV_TAPE_MAJOR, 204)), "\\Device\\Tape76", exists_ntdev, S_IFBLK, true},
  {"/dev/nst77", BRACK(FHDEV(DEV_TAPE_MAJOR, 205)), "\\Device\\Tape77", exists_ntdev, S_IFBLK, true},
  {"/dev/nst78", BRACK(FHDEV(DEV_TAPE_MAJOR, 206)), "\\Device\\Tape78", exists_ntdev, S_IFBLK, true},
  {"/dev/nst79", BRACK(FHDEV(DEV_TAPE_MAJOR, 207)), "\\Device\\Tape79", exists_ntdev, S_IFBLK, true},
  {"/dev/nst80", BRACK(FHDEV(DEV_TAPE_MAJOR, 208)), "\\Device\\Tape80", exists_ntdev, S_IFBLK, true},
  {"/dev/nst81", BRACK(FHDEV(DEV_TAPE_MAJOR, 209)), "\\Device\\Tape81", exists_ntdev, S_IFBLK, true},
  {"/dev/nst82", BRACK(FHDEV(DEV_TAPE_MAJOR, 210)), "\\Device\\Tape82", exists_ntdev, S_IFBLK, true},
  {"/dev/nst83", BRACK(FHDEV(DEV_TAPE_MAJOR, 211)), "\\Device\\Tape83", exists_ntdev, S_IFBLK, true},
  {"/dev/nst84", BRACK(FHDEV(DEV_TAPE_MAJOR, 212)), "\\Device\\Tape84", exists_ntdev, S_IFBLK, true},
  {"/dev/nst85", BRACK(FHDEV(DEV_TAPE_MAJOR, 213)), "\\Device\\Tape85", exists_ntdev, S_IFBLK, true},
  {"/dev/nst86", BRACK(FHDEV(DEV_TAPE_MAJOR, 214)), "\\Device\\Tape86", exists_ntdev, S_IFBLK, true},
  {"/dev/nst87", BRACK(FHDEV(DEV_TAPE_MAJOR, 215)), "\\Device\\Tape87", exists_ntdev, S_IFBLK, true},
  {"/dev/nst88", BRACK(FHDEV(DEV_TAPE_MAJOR, 216)), "\\Device\\Tape88", exists_ntdev, S_IFBLK, true},
  {"/dev/nst89", BRACK(FHDEV(DEV_TAPE_MAJOR, 217)), "\\Device\\Tape89", exists_ntdev, S_IFBLK, true},
  {"/dev/nst90", BRACK(FHDEV(DEV_TAPE_MAJOR, 218)), "\\Device\\Tape90", exists_ntdev, S_IFBLK, true},
  {"/dev/nst91", BRACK(FHDEV(DEV_TAPE_MAJOR, 219)), "\\Device\\Tape91", exists_ntdev, S_IFBLK, true},
  {"/dev/nst92", BRACK(FHDEV(DEV_TAPE_MAJOR, 220)), "\\Device\\Tape92", exists_ntdev, S_IFBLK, true},
  {"/dev/nst93", BRACK(FHDEV(DEV_TAPE_MAJOR, 221)), "\\Device\\Tape93", exists_ntdev, S_IFBLK, true},
  {"/dev/nst94", BRACK(FHDEV(DEV_TAPE_MAJOR, 222)), "\\Device\\Tape94", exists_ntdev, S_IFBLK, true},
  {"/dev/nst95", BRACK(FHDEV(DEV_TAPE_MAJOR, 223)), "\\Device\\Tape95", exists_ntdev, S_IFBLK, true},
  {"/dev/nst96", BRACK(FHDEV(DEV_TAPE_MAJOR, 224)), "\\Device\\Tape96", exists_ntdev, S_IFBLK, true},
  {"/dev/nst97", BRACK(FHDEV(DEV_TAPE_MAJOR, 225)), "\\Device\\Tape97", exists_ntdev, S_IFBLK, true},
  {"/dev/nst98", BRACK(FHDEV(DEV_TAPE_MAJOR, 226)), "\\Device\\Tape98", exists_ntdev, S_IFBLK, true},
  {"/dev/nst99", BRACK(FHDEV(DEV_TAPE_MAJOR, 227)), "\\Device\\Tape99", exists_ntdev, S_IFBLK, true},
  {"/dev/nst100", BRACK(FHDEV(DEV_TAPE_MAJOR, 228)), "\\Device\\Tape100", exists_ntdev, S_IFBLK, true},
  {"/dev/nst101", BRACK(FHDEV(DEV_TAPE_MAJOR, 229)), "\\Device\\Tape101", exists_ntdev, S_IFBLK, true},
  {"/dev/nst102", BRACK(FHDEV(DEV_TAPE_MAJOR, 230)), "\\Device\\Tape102", exists_ntdev, S_IFBLK, true},
  {"/dev/nst103", BRACK(FHDEV(DEV_TAPE_MAJOR, 231)), "\\Device\\Tape103", exists_ntdev, S_IFBLK, true},
  {"/dev/nst104", BRACK(FHDEV(DEV_TAPE_MAJOR, 232)), "\\Device\\Tape104", exists_ntdev, S_IFBLK, true},
  {"/dev/nst105", BRACK(FHDEV(DEV_TAPE_MAJOR, 233)), "\\Device\\Tape105", exists_ntdev, S_IFBLK, true},
  {"/dev/nst106", BRACK(FHDEV(DEV_TAPE_MAJOR, 234)), "\\Device\\Tape106", exists_ntdev, S_IFBLK, true},
  {"/dev/nst107", BRACK(FHDEV(DEV_TAPE_MAJOR, 235)), "\\Device\\Tape107", exists_ntdev, S_IFBLK, true},
  {"/dev/nst108", BRACK(FHDEV(DEV_TAPE_MAJOR, 236)), "\\Device\\Tape108", exists_ntdev, S_IFBLK, true},
  {"/dev/nst109", BRACK(FHDEV(DEV_TAPE_MAJOR, 237)), "\\Device\\Tape109", exists_ntdev, S_IFBLK, true},
  {"/dev/nst110", BRACK(FHDEV(DEV_TAPE_MAJOR, 238)), "\\Device\\Tape110", exists_ntdev, S_IFBLK, true},
  {"/dev/nst111", BRACK(FHDEV(DEV_TAPE_MAJOR, 239)), "\\Device\\Tape111", exists_ntdev, S_IFBLK, true},
  {"/dev/nst112", BRACK(FHDEV(DEV_TAPE_MAJOR, 240)), "\\Device\\Tape112", exists_ntdev, S_IFBLK, true},
  {"/dev/nst113", BRACK(FHDEV(DEV_TAPE_MAJOR, 241)), "\\Device\\Tape113", exists_ntdev, S_IFBLK, true},
  {"/dev/nst114", BRACK(FHDEV(DEV_TAPE_MAJOR, 242)), "\\Device\\Tape114", exists_ntdev, S_IFBLK, true},
  {"/dev/nst115", BRACK(FHDEV(DEV_TAPE_MAJOR, 243)), "\\Device\\Tape115", exists_ntdev, S_IFBLK, true},
  {"/dev/nst116", BRACK(FHDEV(DEV_TAPE_MAJOR, 244)), "\\Device\\Tape116", exists_ntdev, S_IFBLK, true},
  {"/dev/nst117", BRACK(FHDEV(DEV_TAPE_MAJOR, 245)), "\\Device\\Tape117", exists_ntdev, S_IFBLK, true},
  {"/dev/nst118", BRACK(FHDEV(DEV_TAPE_MAJOR, 246)), "\\Device\\Tape118", exists_ntdev, S_IFBLK, true},
  {"/dev/nst119", BRACK(FHDEV(DEV_TAPE_MAJOR, 247)), "\\Device\\Tape119", exists_ntdev, S_IFBLK, true},
  {"/dev/nst120", BRACK(FHDEV(DEV_TAPE_MAJOR, 248)), "\\Device\\Tape120", exists_ntdev, S_IFBLK, true},
  {"/dev/nst121", BRACK(FHDEV(DEV_TAPE_MAJOR, 249)), "\\Device\\Tape121", exists_ntdev, S_IFBLK, true},
  {"/dev/nst122", BRACK(FHDEV(DEV_TAPE_MAJOR, 250)), "\\Device\\Tape122", exists_ntdev, S_IFBLK, true},
  {"/dev/nst123", BRACK(FHDEV(DEV_TAPE_MAJOR, 251)), "\\Device\\Tape123", exists_ntdev, S_IFBLK, true},
  {"/dev/nst124", BRACK(FHDEV(DEV_TAPE_MAJOR, 252)), "\\Device\\Tape124", exists_ntdev, S_IFBLK, true},
  {"/dev/nst125", BRACK(FHDEV(DEV_TAPE_MAJOR, 253)), "\\Device\\Tape125", exists_ntdev, S_IFBLK, true},
  {"/dev/nst126", BRACK(FHDEV(DEV_TAPE_MAJOR, 254)), "\\Device\\Tape126", exists_ntdev, S_IFBLK, true},
  {"/dev/nst127", BRACK(FHDEV(DEV_TAPE_MAJOR, 255)), "\\Device\\Tape127", exists_ntdev, S_IFBLK, true},
  {"/dev/null", BRACK(FH_NULL), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {"/dev/ptmx", BRACK(FH_PTMX), "/dev/ptmx", exists, S_IFCHR, true},
  {"/dev/pty0", BRACK(FHDEV(DEV_PTYS_MAJOR, 0)), "/dev/pty0", exists_pty, S_IFCHR, true},
  {"/dev/pty1", BRACK(FHDEV(DEV_PTYS_MAJOR, 1)), "/dev/pty1", exists_pty, S_IFCHR, true},
  {"/dev/pty2", BRACK(FHDEV(DEV_PTYS_MAJOR, 2)), "/dev/pty2", exists_pty, S_IFCHR, true},
  {"/dev/pty3", BRACK(FHDEV(DEV_PTYS_MAJOR, 3)), "/dev/pty3", exists_pty, S_IFCHR, true},
  {"/dev/pty4", BRACK(FHDEV(DEV_PTYS_MAJOR, 4)), "/dev/pty4", exists_pty, S_IFCHR, true},
  {"/dev/pty5", BRACK(FHDEV(DEV_PTYS_MAJOR, 5)), "/dev/pty5", exists_pty, S_IFCHR, true},
  {"/dev/pty6", BRACK(FHDEV(DEV_PTYS_MAJOR, 6)), "/dev/pty6", exists_pty, S_IFCHR, true},
  {"/dev/pty7", BRACK(FHDEV(DEV_PTYS_MAJOR, 7)), "/dev/pty7", exists_pty, S_IFCHR, true},
  {"/dev/pty8", BRACK(FHDEV(DEV_PTYS_MAJOR, 8)), "/dev/pty8", exists_pty, S_IFCHR, true},
  {"/dev/pty9", BRACK(FHDEV(DEV_PTYS_MAJOR, 9)), "/dev/pty9", exists_pty, S_IFCHR, true},
  {"/dev/pty10", BRACK(FHDEV(DEV_PTYS_MAJOR, 10)), "/dev/pty10", exists_pty, S_IFCHR, true},
  {"/dev/pty11", BRACK(FHDEV(DEV_PTYS_MAJOR, 11)), "/dev/pty11", exists_pty, S_IFCHR, true},
  {"/dev/pty12", BRACK(FHDEV(DEV_PTYS_MAJOR, 12)), "/dev/pty12", exists_pty, S_IFCHR, true},
  {"/dev/pty13", BRACK(FHDEV(DEV_PTYS_MAJOR, 13)), "/dev/pty13", exists_pty, S_IFCHR, true},
  {"/dev/pty14", BRACK(FHDEV(DEV_PTYS_MAJOR, 14)), "/dev/pty14", exists_pty, S_IFCHR, true},
  {"/dev/pty15", BRACK(FHDEV(DEV_PTYS_MAJOR, 15)), "/dev/pty15", exists_pty, S_IFCHR, true},
  {"/dev/pty16", BRACK(FHDEV(DEV_PTYS_MAJOR, 16)), "/dev/pty16", exists_pty, S_IFCHR, true},
  {"/dev/pty17", BRACK(FHDEV(DEV_PTYS_MAJOR, 17)), "/dev/pty17", exists_pty, S_IFCHR, true},
  {"/dev/pty18", BRACK(FHDEV(DEV_PTYS_MAJOR, 18)), "/dev/pty18", exists_pty, S_IFCHR, true},
  {"/dev/pty19", BRACK(FHDEV(DEV_PTYS_MAJOR, 19)), "/dev/pty19", exists_pty, S_IFCHR, true},
  {"/dev/pty20", BRACK(FHDEV(DEV_PTYS_MAJOR, 20)), "/dev/pty20", exists_pty, S_IFCHR, true},
  {"/dev/pty21", BRACK(FHDEV(DEV_PTYS_MAJOR, 21)), "/dev/pty21", exists_pty, S_IFCHR, true},
  {"/dev/pty22", BRACK(FHDEV(DEV_PTYS_MAJOR, 22)), "/dev/pty22", exists_pty, S_IFCHR, true},
  {"/dev/pty23", BRACK(FHDEV(DEV_PTYS_MAJOR, 23)), "/dev/pty23", exists_pty, S_IFCHR, true},
  {"/dev/pty24", BRACK(FHDEV(DEV_PTYS_MAJOR, 24)), "/dev/pty24", exists_pty, S_IFCHR, true},
  {"/dev/pty25", BRACK(FHDEV(DEV_PTYS_MAJOR, 25)), "/dev/pty25", exists_pty, S_IFCHR, true},
  {"/dev/pty26", BRACK(FHDEV(DEV_PTYS_MAJOR, 26)), "/dev/pty26", exists_pty, S_IFCHR, true},
  {"/dev/pty27", BRACK(FHDEV(DEV_PTYS_MAJOR, 27)), "/dev/pty27", exists_pty, S_IFCHR, true},
  {"/dev/pty28", BRACK(FHDEV(DEV_PTYS_MAJOR, 28)), "/dev/pty28", exists_pty, S_IFCHR, true},
  {"/dev/pty29", BRACK(FHDEV(DEV_PTYS_MAJOR, 29)), "/dev/pty29", exists_pty, S_IFCHR, true},
  {"/dev/pty30", BRACK(FHDEV(DEV_PTYS_MAJOR, 30)), "/dev/pty30", exists_pty, S_IFCHR, true},
  {"/dev/pty31", BRACK(FHDEV(DEV_PTYS_MAJOR, 31)), "/dev/pty31", exists_pty, S_IFCHR, true},
  {"/dev/pty32", BRACK(FHDEV(DEV_PTYS_MAJOR, 32)), "/dev/pty32", exists_pty, S_IFCHR, true},
  {"/dev/pty33", BRACK(FHDEV(DEV_PTYS_MAJOR, 33)), "/dev/pty33", exists_pty, S_IFCHR, true},
  {"/dev/pty34", BRACK(FHDEV(DEV_PTYS_MAJOR, 34)), "/dev/pty34", exists_pty, S_IFCHR, true},
  {"/dev/pty35", BRACK(FHDEV(DEV_PTYS_MAJOR, 35)), "/dev/pty35", exists_pty, S_IFCHR, true},
  {"/dev/pty36", BRACK(FHDEV(DEV_PTYS_MAJOR, 36)), "/dev/pty36", exists_pty, S_IFCHR, true},
  {"/dev/pty37", BRACK(FHDEV(DEV_PTYS_MAJOR, 37)), "/dev/pty37", exists_pty, S_IFCHR, true},
  {"/dev/pty38", BRACK(FHDEV(DEV_PTYS_MAJOR, 38)), "/dev/pty38", exists_pty, S_IFCHR, true},
  {"/dev/pty39", BRACK(FHDEV(DEV_PTYS_MAJOR, 39)), "/dev/pty39", exists_pty, S_IFCHR, true},
  {"/dev/pty40", BRACK(FHDEV(DEV_PTYS_MAJOR, 40)), "/dev/pty40", exists_pty, S_IFCHR, true},
  {"/dev/pty41", BRACK(FHDEV(DEV_PTYS_MAJOR, 41)), "/dev/pty41", exists_pty, S_IFCHR, true},
  {"/dev/pty42", BRACK(FHDEV(DEV_PTYS_MAJOR, 42)), "/dev/pty42", exists_pty, S_IFCHR, true},
  {"/dev/pty43", BRACK(FHDEV(DEV_PTYS_MAJOR, 43)), "/dev/pty43", exists_pty, S_IFCHR, true},
  {"/dev/pty44", BRACK(FHDEV(DEV_PTYS_MAJOR, 44)), "/dev/pty44", exists_pty, S_IFCHR, true},
  {"/dev/pty45", BRACK(FHDEV(DEV_PTYS_MAJOR, 45)), "/dev/pty45", exists_pty, S_IFCHR, true},
  {"/dev/pty46", BRACK(FHDEV(DEV_PTYS_MAJOR, 46)), "/dev/pty46", exists_pty, S_IFCHR, true},
  {"/dev/pty47", BRACK(FHDEV(DEV_PTYS_MAJOR, 47)), "/dev/pty47", exists_pty, S_IFCHR, true},
  {"/dev/pty48", BRACK(FHDEV(DEV_PTYS_MAJOR, 48)), "/dev/pty48", exists_pty, S_IFCHR, true},
  {"/dev/pty49", BRACK(FHDEV(DEV_PTYS_MAJOR, 49)), "/dev/pty49", exists_pty, S_IFCHR, true},
  {"/dev/pty50", BRACK(FHDEV(DEV_PTYS_MAJOR, 50)), "/dev/pty50", exists_pty, S_IFCHR, true},
  {"/dev/pty51", BRACK(FHDEV(DEV_PTYS_MAJOR, 51)), "/dev/pty51", exists_pty, S_IFCHR, true},
  {"/dev/pty52", BRACK(FHDEV(DEV_PTYS_MAJOR, 52)), "/dev/pty52", exists_pty, S_IFCHR, true},
  {"/dev/pty53", BRACK(FHDEV(DEV_PTYS_MAJOR, 53)), "/dev/pty53", exists_pty, S_IFCHR, true},
  {"/dev/pty54", BRACK(FHDEV(DEV_PTYS_MAJOR, 54)), "/dev/pty54", exists_pty, S_IFCHR, true},
  {"/dev/pty55", BRACK(FHDEV(DEV_PTYS_MAJOR, 55)), "/dev/pty55", exists_pty, S_IFCHR, true},
  {"/dev/pty56", BRACK(FHDEV(DEV_PTYS_MAJOR, 56)), "/dev/pty56", exists_pty, S_IFCHR, true},
  {"/dev/pty57", BRACK(FHDEV(DEV_PTYS_MAJOR, 57)), "/dev/pty57", exists_pty, S_IFCHR, true},
  {"/dev/pty58", BRACK(FHDEV(DEV_PTYS_MAJOR, 58)), "/dev/pty58", exists_pty, S_IFCHR, true},
  {"/dev/pty59", BRACK(FHDEV(DEV_PTYS_MAJOR, 59)), "/dev/pty59", exists_pty, S_IFCHR, true},
  {"/dev/pty60", BRACK(FHDEV(DEV_PTYS_MAJOR, 60)), "/dev/pty60", exists_pty, S_IFCHR, true},
  {"/dev/pty61", BRACK(FHDEV(DEV_PTYS_MAJOR, 61)), "/dev/pty61", exists_pty, S_IFCHR, true},
  {"/dev/pty62", BRACK(FHDEV(DEV_PTYS_MAJOR, 62)), "/dev/pty62", exists_pty, S_IFCHR, true},
  {"/dev/pty63", BRACK(FHDEV(DEV_PTYS_MAJOR, 63)), "/dev/pty63", exists_pty, S_IFCHR, true},
  {"/dev/random", BRACK(FH_RANDOM), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {"/dev/scd0", BRACK(FHDEV(DEV_CDROM_MAJOR, 0)), "\\Device\\CdRom0", exists_ntdev, S_IFBLK, true},
  {"/dev/scd1", BRACK(FHDEV(DEV_CDROM_MAJOR, 1)), "\\Device\\CdRom1", exists_ntdev, S_IFBLK, true},
  {"/dev/scd2", BRACK(FHDEV(DEV_CDROM_MAJOR, 2)), "\\Device\\CdRom2", exists_ntdev, S_IFBLK, true},
  {"/dev/scd3", BRACK(FHDEV(DEV_CDROM_MAJOR, 3)), "\\Device\\CdRom3", exists_ntdev, S_IFBLK, true},
  {"/dev/scd4", BRACK(FHDEV(DEV_CDROM_MAJOR, 4)), "\\Device\\CdRom4", exists_ntdev, S_IFBLK, true},
  {"/dev/scd5", BRACK(FHDEV(DEV_CDROM_MAJOR, 5)), "\\Device\\CdRom5", exists_ntdev, S_IFBLK, true},
  {"/dev/scd6", BRACK(FHDEV(DEV_CDROM_MAJOR, 6)), "\\Device\\CdRom6", exists_ntdev, S_IFBLK, true},
  {"/dev/scd7", BRACK(FHDEV(DEV_CDROM_MAJOR, 7)), "\\Device\\CdRom7", exists_ntdev, S_IFBLK, true},
  {"/dev/scd8", BRACK(FHDEV(DEV_CDROM_MAJOR, 8)), "\\Device\\CdRom8", exists_ntdev, S_IFBLK, true},
  {"/dev/scd9", BRACK(FHDEV(DEV_CDROM_MAJOR, 9)), "\\Device\\CdRom9", exists_ntdev, S_IFBLK, true},
  {"/dev/scd10", BRACK(FHDEV(DEV_CDROM_MAJOR, 10)), "\\Device\\CdRom10", exists_ntdev, S_IFBLK, true},
  {"/dev/scd11", BRACK(FHDEV(DEV_CDROM_MAJOR, 11)), "\\Device\\CdRom11", exists_ntdev, S_IFBLK, true},
  {"/dev/scd12", BRACK(FHDEV(DEV_CDROM_MAJOR, 12)), "\\Device\\CdRom12", exists_ntdev, S_IFBLK, true},
  {"/dev/scd13", BRACK(FHDEV(DEV_CDROM_MAJOR, 13)), "\\Device\\CdRom13", exists_ntdev, S_IFBLK, true},
  {"/dev/scd14", BRACK(FHDEV(DEV_CDROM_MAJOR, 14)), "\\Device\\CdRom14", exists_ntdev, S_IFBLK, true},
  {"/dev/scd15", BRACK(FHDEV(DEV_CDROM_MAJOR, 15)), "\\Device\\CdRom15", exists_ntdev, S_IFBLK, true},
  {"/dev/sda", BRACK(FH_SDA), "\\Device\\Harddisk0\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb", BRACK(FH_SDB), "\\Device\\Harddisk1\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc", BRACK(FH_SDC), "\\Device\\Harddisk2\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd", BRACK(FH_SDD), "\\Device\\Harddisk3\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sde", BRACK(FH_SDE), "\\Device\\Harddisk4\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf", BRACK(FH_SDF), "\\Device\\Harddisk5\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg", BRACK(FH_SDG), "\\Device\\Harddisk6\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh", BRACK(FH_SDH), "\\Device\\Harddisk7\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi", BRACK(FH_SDI), "\\Device\\Harddisk8\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj", BRACK(FH_SDJ), "\\Device\\Harddisk9\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk", BRACK(FH_SDK), "\\Device\\Harddisk10\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl", BRACK(FH_SDL), "\\Device\\Harddisk11\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm", BRACK(FH_SDM), "\\Device\\Harddisk12\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn", BRACK(FH_SDN), "\\Device\\Harddisk13\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo", BRACK(FH_SDO), "\\Device\\Harddisk14\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp", BRACK(FH_SDP), "\\Device\\Harddisk15\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq", BRACK(FH_SDQ), "\\Device\\Harddisk16\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr", BRACK(FH_SDR), "\\Device\\Harddisk17\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sds", BRACK(FH_SDS), "\\Device\\Harddisk18\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt", BRACK(FH_SDT), "\\Device\\Harddisk19\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu", BRACK(FH_SDU), "\\Device\\Harddisk20\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv", BRACK(FH_SDV), "\\Device\\Harddisk21\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw", BRACK(FH_SDW), "\\Device\\Harddisk22\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx", BRACK(FH_SDX), "\\Device\\Harddisk23\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy", BRACK(FH_SDY), "\\Device\\Harddisk24\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz", BRACK(FH_SDZ), "\\Device\\Harddisk25\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sda1", BRACK(FH_SDA | 1), "\\Device\\Harddisk0\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sda2", BRACK(FH_SDA | 2), "\\Device\\Harddisk0\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sda3", BRACK(FH_SDA | 3), "\\Device\\Harddisk0\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sda4", BRACK(FH_SDA | 4), "\\Device\\Harddisk0\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sda5", BRACK(FH_SDA | 5), "\\Device\\Harddisk0\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sda6", BRACK(FH_SDA | 6), "\\Device\\Harddisk0\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sda7", BRACK(FH_SDA | 7), "\\Device\\Harddisk0\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sda8", BRACK(FH_SDA | 8), "\\Device\\Harddisk0\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sda9", BRACK(FH_SDA | 9), "\\Device\\Harddisk0\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sda10", BRACK(FH_SDA | 10), "\\Device\\Harddisk0\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sda11", BRACK(FH_SDA | 11), "\\Device\\Harddisk0\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sda12", BRACK(FH_SDA | 12), "\\Device\\Harddisk0\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sda13", BRACK(FH_SDA | 13), "\\Device\\Harddisk0\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sda14", BRACK(FH_SDA | 14), "\\Device\\Harddisk0\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sda15", BRACK(FH_SDA | 15), "\\Device\\Harddisk0\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb1", BRACK(FH_SDB | 1), "\\Device\\Harddisk1\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb2", BRACK(FH_SDB | 2), "\\Device\\Harddisk1\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb3", BRACK(FH_SDB | 3), "\\Device\\Harddisk1\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb4", BRACK(FH_SDB | 4), "\\Device\\Harddisk1\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb5", BRACK(FH_SDB | 5), "\\Device\\Harddisk1\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb6", BRACK(FH_SDB | 6), "\\Device\\Harddisk1\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb7", BRACK(FH_SDB | 7), "\\Device\\Harddisk1\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb8", BRACK(FH_SDB | 8), "\\Device\\Harddisk1\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb9", BRACK(FH_SDB | 9), "\\Device\\Harddisk1\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb10", BRACK(FH_SDB | 10), "\\Device\\Harddisk1\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb11", BRACK(FH_SDB | 11), "\\Device\\Harddisk1\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb12", BRACK(FH_SDB | 12), "\\Device\\Harddisk1\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb13", BRACK(FH_SDB | 13), "\\Device\\Harddisk1\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb14", BRACK(FH_SDB | 14), "\\Device\\Harddisk1\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdb15", BRACK(FH_SDB | 15), "\\Device\\Harddisk1\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc1", BRACK(FH_SDC | 1), "\\Device\\Harddisk2\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc2", BRACK(FH_SDC | 2), "\\Device\\Harddisk2\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc3", BRACK(FH_SDC | 3), "\\Device\\Harddisk2\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc4", BRACK(FH_SDC | 4), "\\Device\\Harddisk2\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc5", BRACK(FH_SDC | 5), "\\Device\\Harddisk2\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc6", BRACK(FH_SDC | 6), "\\Device\\Harddisk2\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc7", BRACK(FH_SDC | 7), "\\Device\\Harddisk2\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc8", BRACK(FH_SDC | 8), "\\Device\\Harddisk2\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc9", BRACK(FH_SDC | 9), "\\Device\\Harddisk2\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc10", BRACK(FH_SDC | 10), "\\Device\\Harddisk2\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc11", BRACK(FH_SDC | 11), "\\Device\\Harddisk2\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc12", BRACK(FH_SDC | 12), "\\Device\\Harddisk2\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc13", BRACK(FH_SDC | 13), "\\Device\\Harddisk2\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc14", BRACK(FH_SDC | 14), "\\Device\\Harddisk2\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdc15", BRACK(FH_SDC | 15), "\\Device\\Harddisk2\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd1", BRACK(FH_SDD | 1), "\\Device\\Harddisk3\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd2", BRACK(FH_SDD | 2), "\\Device\\Harddisk3\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd3", BRACK(FH_SDD | 3), "\\Device\\Harddisk3\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd4", BRACK(FH_SDD | 4), "\\Device\\Harddisk3\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd5", BRACK(FH_SDD | 5), "\\Device\\Harddisk3\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd6", BRACK(FH_SDD | 6), "\\Device\\Harddisk3\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd7", BRACK(FH_SDD | 7), "\\Device\\Harddisk3\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd8", BRACK(FH_SDD | 8), "\\Device\\Harddisk3\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd9", BRACK(FH_SDD | 9), "\\Device\\Harddisk3\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd10", BRACK(FH_SDD | 10), "\\Device\\Harddisk3\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd11", BRACK(FH_SDD | 11), "\\Device\\Harddisk3\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd12", BRACK(FH_SDD | 12), "\\Device\\Harddisk3\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd13", BRACK(FH_SDD | 13), "\\Device\\Harddisk3\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd14", BRACK(FH_SDD | 14), "\\Device\\Harddisk3\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdd15", BRACK(FH_SDD | 15), "\\Device\\Harddisk3\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sde1", BRACK(FH_SDE | 1), "\\Device\\Harddisk4\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sde2", BRACK(FH_SDE | 2), "\\Device\\Harddisk4\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sde3", BRACK(FH_SDE | 3), "\\Device\\Harddisk4\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sde4", BRACK(FH_SDE | 4), "\\Device\\Harddisk4\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sde5", BRACK(FH_SDE | 5), "\\Device\\Harddisk4\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sde6", BRACK(FH_SDE | 6), "\\Device\\Harddisk4\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sde7", BRACK(FH_SDE | 7), "\\Device\\Harddisk4\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sde8", BRACK(FH_SDE | 8), "\\Device\\Harddisk4\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sde9", BRACK(FH_SDE | 9), "\\Device\\Harddisk4\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sde10", BRACK(FH_SDE | 10), "\\Device\\Harddisk4\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sde11", BRACK(FH_SDE | 11), "\\Device\\Harddisk4\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sde12", BRACK(FH_SDE | 12), "\\Device\\Harddisk4\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sde13", BRACK(FH_SDE | 13), "\\Device\\Harddisk4\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sde14", BRACK(FH_SDE | 14), "\\Device\\Harddisk4\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sde15", BRACK(FH_SDE | 15), "\\Device\\Harddisk4\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf1", BRACK(FH_SDF | 1), "\\Device\\Harddisk5\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf2", BRACK(FH_SDF | 2), "\\Device\\Harddisk5\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf3", BRACK(FH_SDF | 3), "\\Device\\Harddisk5\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf4", BRACK(FH_SDF | 4), "\\Device\\Harddisk5\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf5", BRACK(FH_SDF | 5), "\\Device\\Harddisk5\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf6", BRACK(FH_SDF | 6), "\\Device\\Harddisk5\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf7", BRACK(FH_SDF | 7), "\\Device\\Harddisk5\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf8", BRACK(FH_SDF | 8), "\\Device\\Harddisk5\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf9", BRACK(FH_SDF | 9), "\\Device\\Harddisk5\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf10", BRACK(FH_SDF | 10), "\\Device\\Harddisk5\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf11", BRACK(FH_SDF | 11), "\\Device\\Harddisk5\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf12", BRACK(FH_SDF | 12), "\\Device\\Harddisk5\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf13", BRACK(FH_SDF | 13), "\\Device\\Harddisk5\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf14", BRACK(FH_SDF | 14), "\\Device\\Harddisk5\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdf15", BRACK(FH_SDF | 15), "\\Device\\Harddisk5\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg1", BRACK(FH_SDG | 1), "\\Device\\Harddisk6\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg2", BRACK(FH_SDG | 2), "\\Device\\Harddisk6\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg3", BRACK(FH_SDG | 3), "\\Device\\Harddisk6\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg4", BRACK(FH_SDG | 4), "\\Device\\Harddisk6\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg5", BRACK(FH_SDG | 5), "\\Device\\Harddisk6\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg6", BRACK(FH_SDG | 6), "\\Device\\Harddisk6\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg7", BRACK(FH_SDG | 7), "\\Device\\Harddisk6\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg8", BRACK(FH_SDG | 8), "\\Device\\Harddisk6\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg9", BRACK(FH_SDG | 9), "\\Device\\Harddisk6\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg10", BRACK(FH_SDG | 10), "\\Device\\Harddisk6\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg11", BRACK(FH_SDG | 11), "\\Device\\Harddisk6\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg12", BRACK(FH_SDG | 12), "\\Device\\Harddisk6\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg13", BRACK(FH_SDG | 13), "\\Device\\Harddisk6\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg14", BRACK(FH_SDG | 14), "\\Device\\Harddisk6\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdg15", BRACK(FH_SDG | 15), "\\Device\\Harddisk6\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh1", BRACK(FH_SDH | 1), "\\Device\\Harddisk7\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh2", BRACK(FH_SDH | 2), "\\Device\\Harddisk7\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh3", BRACK(FH_SDH | 3), "\\Device\\Harddisk7\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh4", BRACK(FH_SDH | 4), "\\Device\\Harddisk7\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh5", BRACK(FH_SDH | 5), "\\Device\\Harddisk7\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh6", BRACK(FH_SDH | 6), "\\Device\\Harddisk7\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh7", BRACK(FH_SDH | 7), "\\Device\\Harddisk7\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh8", BRACK(FH_SDH | 8), "\\Device\\Harddisk7\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh9", BRACK(FH_SDH | 9), "\\Device\\Harddisk7\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh10", BRACK(FH_SDH | 10), "\\Device\\Harddisk7\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh11", BRACK(FH_SDH | 11), "\\Device\\Harddisk7\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh12", BRACK(FH_SDH | 12), "\\Device\\Harddisk7\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh13", BRACK(FH_SDH | 13), "\\Device\\Harddisk7\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh14", BRACK(FH_SDH | 14), "\\Device\\Harddisk7\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdh15", BRACK(FH_SDH | 15), "\\Device\\Harddisk7\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi1", BRACK(FH_SDI | 1), "\\Device\\Harddisk8\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi2", BRACK(FH_SDI | 2), "\\Device\\Harddisk8\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi3", BRACK(FH_SDI | 3), "\\Device\\Harddisk8\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi4", BRACK(FH_SDI | 4), "\\Device\\Harddisk8\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi5", BRACK(FH_SDI | 5), "\\Device\\Harddisk8\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi6", BRACK(FH_SDI | 6), "\\Device\\Harddisk8\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi7", BRACK(FH_SDI | 7), "\\Device\\Harddisk8\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi8", BRACK(FH_SDI | 8), "\\Device\\Harddisk8\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi9", BRACK(FH_SDI | 9), "\\Device\\Harddisk8\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi10", BRACK(FH_SDI | 10), "\\Device\\Harddisk8\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi11", BRACK(FH_SDI | 11), "\\Device\\Harddisk8\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi12", BRACK(FH_SDI | 12), "\\Device\\Harddisk8\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi13", BRACK(FH_SDI | 13), "\\Device\\Harddisk8\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi14", BRACK(FH_SDI | 14), "\\Device\\Harddisk8\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdi15", BRACK(FH_SDI | 15), "\\Device\\Harddisk8\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj1", BRACK(FH_SDJ | 1), "\\Device\\Harddisk9\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj2", BRACK(FH_SDJ | 2), "\\Device\\Harddisk9\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj3", BRACK(FH_SDJ | 3), "\\Device\\Harddisk9\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj4", BRACK(FH_SDJ | 4), "\\Device\\Harddisk9\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj5", BRACK(FH_SDJ | 5), "\\Device\\Harddisk9\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj6", BRACK(FH_SDJ | 6), "\\Device\\Harddisk9\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj7", BRACK(FH_SDJ | 7), "\\Device\\Harddisk9\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj8", BRACK(FH_SDJ | 8), "\\Device\\Harddisk9\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj9", BRACK(FH_SDJ | 9), "\\Device\\Harddisk9\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj10", BRACK(FH_SDJ | 10), "\\Device\\Harddisk9\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj11", BRACK(FH_SDJ | 11), "\\Device\\Harddisk9\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj12", BRACK(FH_SDJ | 12), "\\Device\\Harddisk9\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj13", BRACK(FH_SDJ | 13), "\\Device\\Harddisk9\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj14", BRACK(FH_SDJ | 14), "\\Device\\Harddisk9\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdj15", BRACK(FH_SDJ | 15), "\\Device\\Harddisk9\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk1", BRACK(FH_SDK | 1), "\\Device\\Harddisk10\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk2", BRACK(FH_SDK | 2), "\\Device\\Harddisk10\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk3", BRACK(FH_SDK | 3), "\\Device\\Harddisk10\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk4", BRACK(FH_SDK | 4), "\\Device\\Harddisk10\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk5", BRACK(FH_SDK | 5), "\\Device\\Harddisk10\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk6", BRACK(FH_SDK | 6), "\\Device\\Harddisk10\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk7", BRACK(FH_SDK | 7), "\\Device\\Harddisk10\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk8", BRACK(FH_SDK | 8), "\\Device\\Harddisk10\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk9", BRACK(FH_SDK | 9), "\\Device\\Harddisk10\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk10", BRACK(FH_SDK | 10), "\\Device\\Harddisk10\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk11", BRACK(FH_SDK | 11), "\\Device\\Harddisk10\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk12", BRACK(FH_SDK | 12), "\\Device\\Harddisk10\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk13", BRACK(FH_SDK | 13), "\\Device\\Harddisk10\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk14", BRACK(FH_SDK | 14), "\\Device\\Harddisk10\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdk15", BRACK(FH_SDK | 15), "\\Device\\Harddisk10\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl1", BRACK(FH_SDL | 1), "\\Device\\Harddisk11\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl2", BRACK(FH_SDL | 2), "\\Device\\Harddisk11\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl3", BRACK(FH_SDL | 3), "\\Device\\Harddisk11\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl4", BRACK(FH_SDL | 4), "\\Device\\Harddisk11\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl5", BRACK(FH_SDL | 5), "\\Device\\Harddisk11\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl6", BRACK(FH_SDL | 6), "\\Device\\Harddisk11\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl7", BRACK(FH_SDL | 7), "\\Device\\Harddisk11\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl8", BRACK(FH_SDL | 8), "\\Device\\Harddisk11\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl9", BRACK(FH_SDL | 9), "\\Device\\Harddisk11\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl10", BRACK(FH_SDL | 10), "\\Device\\Harddisk11\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl11", BRACK(FH_SDL | 11), "\\Device\\Harddisk11\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl12", BRACK(FH_SDL | 12), "\\Device\\Harddisk11\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl13", BRACK(FH_SDL | 13), "\\Device\\Harddisk11\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl14", BRACK(FH_SDL | 14), "\\Device\\Harddisk11\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdl15", BRACK(FH_SDL | 15), "\\Device\\Harddisk11\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm1", BRACK(FH_SDM | 1), "\\Device\\Harddisk12\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm2", BRACK(FH_SDM | 2), "\\Device\\Harddisk12\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm3", BRACK(FH_SDM | 3), "\\Device\\Harddisk12\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm4", BRACK(FH_SDM | 4), "\\Device\\Harddisk12\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm5", BRACK(FH_SDM | 5), "\\Device\\Harddisk12\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm6", BRACK(FH_SDM | 6), "\\Device\\Harddisk12\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm7", BRACK(FH_SDM | 7), "\\Device\\Harddisk12\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm8", BRACK(FH_SDM | 8), "\\Device\\Harddisk12\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm9", BRACK(FH_SDM | 9), "\\Device\\Harddisk12\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm10", BRACK(FH_SDM | 10), "\\Device\\Harddisk12\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm11", BRACK(FH_SDM | 11), "\\Device\\Harddisk12\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm12", BRACK(FH_SDM | 12), "\\Device\\Harddisk12\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm13", BRACK(FH_SDM | 13), "\\Device\\Harddisk12\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm14", BRACK(FH_SDM | 14), "\\Device\\Harddisk12\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdm15", BRACK(FH_SDM | 15), "\\Device\\Harddisk12\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn1", BRACK(FH_SDN | 1), "\\Device\\Harddisk13\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn2", BRACK(FH_SDN | 2), "\\Device\\Harddisk13\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn3", BRACK(FH_SDN | 3), "\\Device\\Harddisk13\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn4", BRACK(FH_SDN | 4), "\\Device\\Harddisk13\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn5", BRACK(FH_SDN | 5), "\\Device\\Harddisk13\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn6", BRACK(FH_SDN | 6), "\\Device\\Harddisk13\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn7", BRACK(FH_SDN | 7), "\\Device\\Harddisk13\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn8", BRACK(FH_SDN | 8), "\\Device\\Harddisk13\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn9", BRACK(FH_SDN | 9), "\\Device\\Harddisk13\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn10", BRACK(FH_SDN | 10), "\\Device\\Harddisk13\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn11", BRACK(FH_SDN | 11), "\\Device\\Harddisk13\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn12", BRACK(FH_SDN | 12), "\\Device\\Harddisk13\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn13", BRACK(FH_SDN | 13), "\\Device\\Harddisk13\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn14", BRACK(FH_SDN | 14), "\\Device\\Harddisk13\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdn15", BRACK(FH_SDN | 15), "\\Device\\Harddisk13\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo1", BRACK(FH_SDO | 1), "\\Device\\Harddisk14\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo2", BRACK(FH_SDO | 2), "\\Device\\Harddisk14\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo3", BRACK(FH_SDO | 3), "\\Device\\Harddisk14\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo4", BRACK(FH_SDO | 4), "\\Device\\Harddisk14\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo5", BRACK(FH_SDO | 5), "\\Device\\Harddisk14\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo6", BRACK(FH_SDO | 6), "\\Device\\Harddisk14\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo7", BRACK(FH_SDO | 7), "\\Device\\Harddisk14\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo8", BRACK(FH_SDO | 8), "\\Device\\Harddisk14\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo9", BRACK(FH_SDO | 9), "\\Device\\Harddisk14\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo10", BRACK(FH_SDO | 10), "\\Device\\Harddisk14\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo11", BRACK(FH_SDO | 11), "\\Device\\Harddisk14\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo12", BRACK(FH_SDO | 12), "\\Device\\Harddisk14\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo13", BRACK(FH_SDO | 13), "\\Device\\Harddisk14\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo14", BRACK(FH_SDO | 14), "\\Device\\Harddisk14\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdo15", BRACK(FH_SDO | 15), "\\Device\\Harddisk14\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp1", BRACK(FH_SDP | 1), "\\Device\\Harddisk15\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp2", BRACK(FH_SDP | 2), "\\Device\\Harddisk15\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp3", BRACK(FH_SDP | 3), "\\Device\\Harddisk15\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp4", BRACK(FH_SDP | 4), "\\Device\\Harddisk15\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp5", BRACK(FH_SDP | 5), "\\Device\\Harddisk15\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp6", BRACK(FH_SDP | 6), "\\Device\\Harddisk15\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp7", BRACK(FH_SDP | 7), "\\Device\\Harddisk15\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp8", BRACK(FH_SDP | 8), "\\Device\\Harddisk15\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp9", BRACK(FH_SDP | 9), "\\Device\\Harddisk15\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp10", BRACK(FH_SDP | 10), "\\Device\\Harddisk15\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp11", BRACK(FH_SDP | 11), "\\Device\\Harddisk15\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp12", BRACK(FH_SDP | 12), "\\Device\\Harddisk15\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp13", BRACK(FH_SDP | 13), "\\Device\\Harddisk15\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp14", BRACK(FH_SDP | 14), "\\Device\\Harddisk15\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdp15", BRACK(FH_SDP | 15), "\\Device\\Harddisk15\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq1", BRACK(FH_SDQ | 1), "\\Device\\Harddisk16\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq2", BRACK(FH_SDQ | 2), "\\Device\\Harddisk16\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq3", BRACK(FH_SDQ | 3), "\\Device\\Harddisk16\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq4", BRACK(FH_SDQ | 4), "\\Device\\Harddisk16\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq5", BRACK(FH_SDQ | 5), "\\Device\\Harddisk16\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq6", BRACK(FH_SDQ | 6), "\\Device\\Harddisk16\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq7", BRACK(FH_SDQ | 7), "\\Device\\Harddisk16\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq8", BRACK(FH_SDQ | 8), "\\Device\\Harddisk16\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq9", BRACK(FH_SDQ | 9), "\\Device\\Harddisk16\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq10", BRACK(FH_SDQ | 10), "\\Device\\Harddisk16\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq11", BRACK(FH_SDQ | 11), "\\Device\\Harddisk16\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq12", BRACK(FH_SDQ | 12), "\\Device\\Harddisk16\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq13", BRACK(FH_SDQ | 13), "\\Device\\Harddisk16\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq14", BRACK(FH_SDQ | 14), "\\Device\\Harddisk16\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdq15", BRACK(FH_SDQ | 15), "\\Device\\Harddisk16\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr1", BRACK(FH_SDR | 1), "\\Device\\Harddisk17\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr2", BRACK(FH_SDR | 2), "\\Device\\Harddisk17\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr3", BRACK(FH_SDR | 3), "\\Device\\Harddisk17\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr4", BRACK(FH_SDR | 4), "\\Device\\Harddisk17\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr5", BRACK(FH_SDR | 5), "\\Device\\Harddisk17\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr6", BRACK(FH_SDR | 6), "\\Device\\Harddisk17\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr7", BRACK(FH_SDR | 7), "\\Device\\Harddisk17\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr8", BRACK(FH_SDR | 8), "\\Device\\Harddisk17\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr9", BRACK(FH_SDR | 9), "\\Device\\Harddisk17\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr10", BRACK(FH_SDR | 10), "\\Device\\Harddisk17\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr11", BRACK(FH_SDR | 11), "\\Device\\Harddisk17\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr12", BRACK(FH_SDR | 12), "\\Device\\Harddisk17\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr13", BRACK(FH_SDR | 13), "\\Device\\Harddisk17\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr14", BRACK(FH_SDR | 14), "\\Device\\Harddisk17\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdr15", BRACK(FH_SDR | 15), "\\Device\\Harddisk17\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sds1", BRACK(FH_SDS | 1), "\\Device\\Harddisk18\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sds2", BRACK(FH_SDS | 2), "\\Device\\Harddisk18\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sds3", BRACK(FH_SDS | 3), "\\Device\\Harddisk18\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sds4", BRACK(FH_SDS | 4), "\\Device\\Harddisk18\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sds5", BRACK(FH_SDS | 5), "\\Device\\Harddisk18\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sds6", BRACK(FH_SDS | 6), "\\Device\\Harddisk18\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sds7", BRACK(FH_SDS | 7), "\\Device\\Harddisk18\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sds8", BRACK(FH_SDS | 8), "\\Device\\Harddisk18\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sds9", BRACK(FH_SDS | 9), "\\Device\\Harddisk18\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sds10", BRACK(FH_SDS | 10), "\\Device\\Harddisk18\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sds11", BRACK(FH_SDS | 11), "\\Device\\Harddisk18\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sds12", BRACK(FH_SDS | 12), "\\Device\\Harddisk18\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sds13", BRACK(FH_SDS | 13), "\\Device\\Harddisk18\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sds14", BRACK(FH_SDS | 14), "\\Device\\Harddisk18\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sds15", BRACK(FH_SDS | 15), "\\Device\\Harddisk18\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt1", BRACK(FH_SDT | 1), "\\Device\\Harddisk19\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt2", BRACK(FH_SDT | 2), "\\Device\\Harddisk19\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt3", BRACK(FH_SDT | 3), "\\Device\\Harddisk19\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt4", BRACK(FH_SDT | 4), "\\Device\\Harddisk19\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt5", BRACK(FH_SDT | 5), "\\Device\\Harddisk19\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt6", BRACK(FH_SDT | 6), "\\Device\\Harddisk19\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt7", BRACK(FH_SDT | 7), "\\Device\\Harddisk19\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt8", BRACK(FH_SDT | 8), "\\Device\\Harddisk19\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt9", BRACK(FH_SDT | 9), "\\Device\\Harddisk19\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt10", BRACK(FH_SDT | 10), "\\Device\\Harddisk19\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt11", BRACK(FH_SDT | 11), "\\Device\\Harddisk19\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt12", BRACK(FH_SDT | 12), "\\Device\\Harddisk19\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt13", BRACK(FH_SDT | 13), "\\Device\\Harddisk19\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt14", BRACK(FH_SDT | 14), "\\Device\\Harddisk19\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdt15", BRACK(FH_SDT | 15), "\\Device\\Harddisk19\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu1", BRACK(FH_SDU | 1), "\\Device\\Harddisk20\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu2", BRACK(FH_SDU | 2), "\\Device\\Harddisk20\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu3", BRACK(FH_SDU | 3), "\\Device\\Harddisk20\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu4", BRACK(FH_SDU | 4), "\\Device\\Harddisk20\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu5", BRACK(FH_SDU | 5), "\\Device\\Harddisk20\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu6", BRACK(FH_SDU | 6), "\\Device\\Harddisk20\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu7", BRACK(FH_SDU | 7), "\\Device\\Harddisk20\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu8", BRACK(FH_SDU | 8), "\\Device\\Harddisk20\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu9", BRACK(FH_SDU | 9), "\\Device\\Harddisk20\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu10", BRACK(FH_SDU | 10), "\\Device\\Harddisk20\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu11", BRACK(FH_SDU | 11), "\\Device\\Harddisk20\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu12", BRACK(FH_SDU | 12), "\\Device\\Harddisk20\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu13", BRACK(FH_SDU | 13), "\\Device\\Harddisk20\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu14", BRACK(FH_SDU | 14), "\\Device\\Harddisk20\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdu15", BRACK(FH_SDU | 15), "\\Device\\Harddisk20\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv1", BRACK(FH_SDV | 1), "\\Device\\Harddisk21\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv2", BRACK(FH_SDV | 2), "\\Device\\Harddisk21\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv3", BRACK(FH_SDV | 3), "\\Device\\Harddisk21\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv4", BRACK(FH_SDV | 4), "\\Device\\Harddisk21\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv5", BRACK(FH_SDV | 5), "\\Device\\Harddisk21\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv6", BRACK(FH_SDV | 6), "\\Device\\Harddisk21\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv7", BRACK(FH_SDV | 7), "\\Device\\Harddisk21\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv8", BRACK(FH_SDV | 8), "\\Device\\Harddisk21\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv9", BRACK(FH_SDV | 9), "\\Device\\Harddisk21\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv10", BRACK(FH_SDV | 10), "\\Device\\Harddisk21\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv11", BRACK(FH_SDV | 11), "\\Device\\Harddisk21\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv12", BRACK(FH_SDV | 12), "\\Device\\Harddisk21\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv13", BRACK(FH_SDV | 13), "\\Device\\Harddisk21\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv14", BRACK(FH_SDV | 14), "\\Device\\Harddisk21\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdv15", BRACK(FH_SDV | 15), "\\Device\\Harddisk21\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw1", BRACK(FH_SDW | 1), "\\Device\\Harddisk22\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw2", BRACK(FH_SDW | 2), "\\Device\\Harddisk22\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw3", BRACK(FH_SDW | 3), "\\Device\\Harddisk22\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw4", BRACK(FH_SDW | 4), "\\Device\\Harddisk22\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw5", BRACK(FH_SDW | 5), "\\Device\\Harddisk22\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw6", BRACK(FH_SDW | 6), "\\Device\\Harddisk22\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw7", BRACK(FH_SDW | 7), "\\Device\\Harddisk22\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw8", BRACK(FH_SDW | 8), "\\Device\\Harddisk22\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw9", BRACK(FH_SDW | 9), "\\Device\\Harddisk22\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw10", BRACK(FH_SDW | 10), "\\Device\\Harddisk22\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw11", BRACK(FH_SDW | 11), "\\Device\\Harddisk22\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw12", BRACK(FH_SDW | 12), "\\Device\\Harddisk22\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw13", BRACK(FH_SDW | 13), "\\Device\\Harddisk22\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw14", BRACK(FH_SDW | 14), "\\Device\\Harddisk22\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdw15", BRACK(FH_SDW | 15), "\\Device\\Harddisk22\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx1", BRACK(FH_SDX | 1), "\\Device\\Harddisk23\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx2", BRACK(FH_SDX | 2), "\\Device\\Harddisk23\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx3", BRACK(FH_SDX | 3), "\\Device\\Harddisk23\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx4", BRACK(FH_SDX | 4), "\\Device\\Harddisk23\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx5", BRACK(FH_SDX | 5), "\\Device\\Harddisk23\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx6", BRACK(FH_SDX | 6), "\\Device\\Harddisk23\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx7", BRACK(FH_SDX | 7), "\\Device\\Harddisk23\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx8", BRACK(FH_SDX | 8), "\\Device\\Harddisk23\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx9", BRACK(FH_SDX | 9), "\\Device\\Harddisk23\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx10", BRACK(FH_SDX | 10), "\\Device\\Harddisk23\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx11", BRACK(FH_SDX | 11), "\\Device\\Harddisk23\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx12", BRACK(FH_SDX | 12), "\\Device\\Harddisk23\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx13", BRACK(FH_SDX | 13), "\\Device\\Harddisk23\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx14", BRACK(FH_SDX | 14), "\\Device\\Harddisk23\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdx15", BRACK(FH_SDX | 15), "\\Device\\Harddisk23\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy1", BRACK(FH_SDY | 1), "\\Device\\Harddisk24\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy2", BRACK(FH_SDY | 2), "\\Device\\Harddisk24\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy3", BRACK(FH_SDY | 3), "\\Device\\Harddisk24\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy4", BRACK(FH_SDY | 4), "\\Device\\Harddisk24\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy5", BRACK(FH_SDY | 5), "\\Device\\Harddisk24\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy6", BRACK(FH_SDY | 6), "\\Device\\Harddisk24\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy7", BRACK(FH_SDY | 7), "\\Device\\Harddisk24\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy8", BRACK(FH_SDY | 8), "\\Device\\Harddisk24\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy9", BRACK(FH_SDY | 9), "\\Device\\Harddisk24\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy10", BRACK(FH_SDY | 10), "\\Device\\Harddisk24\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy11", BRACK(FH_SDY | 11), "\\Device\\Harddisk24\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy12", BRACK(FH_SDY | 12), "\\Device\\Harddisk24\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy13", BRACK(FH_SDY | 13), "\\Device\\Harddisk24\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy14", BRACK(FH_SDY | 14), "\\Device\\Harddisk24\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdy15", BRACK(FH_SDY | 15), "\\Device\\Harddisk24\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz1", BRACK(FH_SDZ | 1), "\\Device\\Harddisk25\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz2", BRACK(FH_SDZ | 2), "\\Device\\Harddisk25\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz3", BRACK(FH_SDZ | 3), "\\Device\\Harddisk25\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz4", BRACK(FH_SDZ | 4), "\\Device\\Harddisk25\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz5", BRACK(FH_SDZ | 5), "\\Device\\Harddisk25\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz6", BRACK(FH_SDZ | 6), "\\Device\\Harddisk25\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz7", BRACK(FH_SDZ | 7), "\\Device\\Harddisk25\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz8", BRACK(FH_SDZ | 8), "\\Device\\Harddisk25\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz9", BRACK(FH_SDZ | 9), "\\Device\\Harddisk25\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz10", BRACK(FH_SDZ | 10), "\\Device\\Harddisk25\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz11", BRACK(FH_SDZ | 11), "\\Device\\Harddisk25\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz12", BRACK(FH_SDZ | 12), "\\Device\\Harddisk25\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz13", BRACK(FH_SDZ | 13), "\\Device\\Harddisk25\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz14", BRACK(FH_SDZ | 14), "\\Device\\Harddisk25\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdz15", BRACK(FH_SDZ | 15), "\\Device\\Harddisk25\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa", BRACK(FH_SDAA), "\\Device\\Harddisk26\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab", BRACK(FH_SDAB), "\\Device\\Harddisk27\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac", BRACK(FH_SDAC), "\\Device\\Harddisk28\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad", BRACK(FH_SDAD), "\\Device\\Harddisk29\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae", BRACK(FH_SDAE), "\\Device\\Harddisk30\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf", BRACK(FH_SDAF), "\\Device\\Harddisk31\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag", BRACK(FH_SDAG), "\\Device\\Harddisk32\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah", BRACK(FH_SDAH), "\\Device\\Harddisk33\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai", BRACK(FH_SDAI), "\\Device\\Harddisk34\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj", BRACK(FH_SDAJ), "\\Device\\Harddisk35\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak", BRACK(FH_SDAK), "\\Device\\Harddisk36\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal", BRACK(FH_SDAL), "\\Device\\Harddisk37\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam", BRACK(FH_SDAM), "\\Device\\Harddisk38\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan", BRACK(FH_SDAN), "\\Device\\Harddisk39\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao", BRACK(FH_SDAO), "\\Device\\Harddisk40\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap", BRACK(FH_SDAP), "\\Device\\Harddisk41\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq", BRACK(FH_SDAQ), "\\Device\\Harddisk42\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar", BRACK(FH_SDAR), "\\Device\\Harddisk43\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas", BRACK(FH_SDAS), "\\Device\\Harddisk44\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat", BRACK(FH_SDAT), "\\Device\\Harddisk45\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau", BRACK(FH_SDAU), "\\Device\\Harddisk46\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav", BRACK(FH_SDAV), "\\Device\\Harddisk47\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw", BRACK(FH_SDAW), "\\Device\\Harddisk48\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax", BRACK(FH_SDAX), "\\Device\\Harddisk49\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sday", BRACK(FH_SDAY), "\\Device\\Harddisk50\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz", BRACK(FH_SDAZ), "\\Device\\Harddisk51\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa1", BRACK(FH_SDAA | 1), "\\Device\\Harddisk26\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa2", BRACK(FH_SDAA | 2), "\\Device\\Harddisk26\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa3", BRACK(FH_SDAA | 3), "\\Device\\Harddisk26\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa4", BRACK(FH_SDAA | 4), "\\Device\\Harddisk26\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa5", BRACK(FH_SDAA | 5), "\\Device\\Harddisk26\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa6", BRACK(FH_SDAA | 6), "\\Device\\Harddisk26\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa7", BRACK(FH_SDAA | 7), "\\Device\\Harddisk26\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa8", BRACK(FH_SDAA | 8), "\\Device\\Harddisk26\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa9", BRACK(FH_SDAA | 9), "\\Device\\Harddisk26\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa10", BRACK(FH_SDAA | 10), "\\Device\\Harddisk26\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa11", BRACK(FH_SDAA | 11), "\\Device\\Harddisk26\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa12", BRACK(FH_SDAA | 12), "\\Device\\Harddisk26\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa13", BRACK(FH_SDAA | 13), "\\Device\\Harddisk26\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa14", BRACK(FH_SDAA | 14), "\\Device\\Harddisk26\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaa15", BRACK(FH_SDAA | 15), "\\Device\\Harddisk26\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab1", BRACK(FH_SDAB | 1), "\\Device\\Harddisk27\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab2", BRACK(FH_SDAB | 2), "\\Device\\Harddisk27\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab3", BRACK(FH_SDAB | 3), "\\Device\\Harddisk27\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab4", BRACK(FH_SDAB | 4), "\\Device\\Harddisk27\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab5", BRACK(FH_SDAB | 5), "\\Device\\Harddisk27\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab6", BRACK(FH_SDAB | 6), "\\Device\\Harddisk27\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab7", BRACK(FH_SDAB | 7), "\\Device\\Harddisk27\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab8", BRACK(FH_SDAB | 8), "\\Device\\Harddisk27\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab9", BRACK(FH_SDAB | 9), "\\Device\\Harddisk27\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab10", BRACK(FH_SDAB | 10), "\\Device\\Harddisk27\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab11", BRACK(FH_SDAB | 11), "\\Device\\Harddisk27\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab12", BRACK(FH_SDAB | 12), "\\Device\\Harddisk27\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab13", BRACK(FH_SDAB | 13), "\\Device\\Harddisk27\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab14", BRACK(FH_SDAB | 14), "\\Device\\Harddisk27\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdab15", BRACK(FH_SDAB | 15), "\\Device\\Harddisk27\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac1", BRACK(FH_SDAC | 1), "\\Device\\Harddisk28\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac2", BRACK(FH_SDAC | 2), "\\Device\\Harddisk28\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac3", BRACK(FH_SDAC | 3), "\\Device\\Harddisk28\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac4", BRACK(FH_SDAC | 4), "\\Device\\Harddisk28\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac5", BRACK(FH_SDAC | 5), "\\Device\\Harddisk28\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac6", BRACK(FH_SDAC | 6), "\\Device\\Harddisk28\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac7", BRACK(FH_SDAC | 7), "\\Device\\Harddisk28\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac8", BRACK(FH_SDAC | 8), "\\Device\\Harddisk28\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac9", BRACK(FH_SDAC | 9), "\\Device\\Harddisk28\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac10", BRACK(FH_SDAC | 10), "\\Device\\Harddisk28\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac11", BRACK(FH_SDAC | 11), "\\Device\\Harddisk28\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac12", BRACK(FH_SDAC | 12), "\\Device\\Harddisk28\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac13", BRACK(FH_SDAC | 13), "\\Device\\Harddisk28\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac14", BRACK(FH_SDAC | 14), "\\Device\\Harddisk28\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdac15", BRACK(FH_SDAC | 15), "\\Device\\Harddisk28\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad1", BRACK(FH_SDAD | 1), "\\Device\\Harddisk29\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad2", BRACK(FH_SDAD | 2), "\\Device\\Harddisk29\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad3", BRACK(FH_SDAD | 3), "\\Device\\Harddisk29\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad4", BRACK(FH_SDAD | 4), "\\Device\\Harddisk29\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad5", BRACK(FH_SDAD | 5), "\\Device\\Harddisk29\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad6", BRACK(FH_SDAD | 6), "\\Device\\Harddisk29\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad7", BRACK(FH_SDAD | 7), "\\Device\\Harddisk29\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad8", BRACK(FH_SDAD | 8), "\\Device\\Harddisk29\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad9", BRACK(FH_SDAD | 9), "\\Device\\Harddisk29\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad10", BRACK(FH_SDAD | 10), "\\Device\\Harddisk29\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad11", BRACK(FH_SDAD | 11), "\\Device\\Harddisk29\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad12", BRACK(FH_SDAD | 12), "\\Device\\Harddisk29\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad13", BRACK(FH_SDAD | 13), "\\Device\\Harddisk29\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad14", BRACK(FH_SDAD | 14), "\\Device\\Harddisk29\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdad15", BRACK(FH_SDAD | 15), "\\Device\\Harddisk29\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae1", BRACK(FH_SDAE | 1), "\\Device\\Harddisk30\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae2", BRACK(FH_SDAE | 2), "\\Device\\Harddisk30\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae3", BRACK(FH_SDAE | 3), "\\Device\\Harddisk30\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae4", BRACK(FH_SDAE | 4), "\\Device\\Harddisk30\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae5", BRACK(FH_SDAE | 5), "\\Device\\Harddisk30\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae6", BRACK(FH_SDAE | 6), "\\Device\\Harddisk30\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae7", BRACK(FH_SDAE | 7), "\\Device\\Harddisk30\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae8", BRACK(FH_SDAE | 8), "\\Device\\Harddisk30\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae9", BRACK(FH_SDAE | 9), "\\Device\\Harddisk30\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae10", BRACK(FH_SDAE | 10), "\\Device\\Harddisk30\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae11", BRACK(FH_SDAE | 11), "\\Device\\Harddisk30\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae12", BRACK(FH_SDAE | 12), "\\Device\\Harddisk30\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae13", BRACK(FH_SDAE | 13), "\\Device\\Harddisk30\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae14", BRACK(FH_SDAE | 14), "\\Device\\Harddisk30\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdae15", BRACK(FH_SDAE | 15), "\\Device\\Harddisk30\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf1", BRACK(FH_SDAF | 1), "\\Device\\Harddisk31\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf2", BRACK(FH_SDAF | 2), "\\Device\\Harddisk31\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf3", BRACK(FH_SDAF | 3), "\\Device\\Harddisk31\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf4", BRACK(FH_SDAF | 4), "\\Device\\Harddisk31\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf5", BRACK(FH_SDAF | 5), "\\Device\\Harddisk31\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf6", BRACK(FH_SDAF | 6), "\\Device\\Harddisk31\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf7", BRACK(FH_SDAF | 7), "\\Device\\Harddisk31\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf8", BRACK(FH_SDAF | 8), "\\Device\\Harddisk31\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf9", BRACK(FH_SDAF | 9), "\\Device\\Harddisk31\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf10", BRACK(FH_SDAF | 10), "\\Device\\Harddisk31\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf11", BRACK(FH_SDAF | 11), "\\Device\\Harddisk31\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf12", BRACK(FH_SDAF | 12), "\\Device\\Harddisk31\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf13", BRACK(FH_SDAF | 13), "\\Device\\Harddisk31\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf14", BRACK(FH_SDAF | 14), "\\Device\\Harddisk31\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaf15", BRACK(FH_SDAF | 15), "\\Device\\Harddisk31\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag1", BRACK(FH_SDAG | 1), "\\Device\\Harddisk32\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag2", BRACK(FH_SDAG | 2), "\\Device\\Harddisk32\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag3", BRACK(FH_SDAG | 3), "\\Device\\Harddisk32\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag4", BRACK(FH_SDAG | 4), "\\Device\\Harddisk32\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag5", BRACK(FH_SDAG | 5), "\\Device\\Harddisk32\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag6", BRACK(FH_SDAG | 6), "\\Device\\Harddisk32\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag7", BRACK(FH_SDAG | 7), "\\Device\\Harddisk32\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag8", BRACK(FH_SDAG | 8), "\\Device\\Harddisk32\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag9", BRACK(FH_SDAG | 9), "\\Device\\Harddisk32\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag10", BRACK(FH_SDAG | 10), "\\Device\\Harddisk32\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag11", BRACK(FH_SDAG | 11), "\\Device\\Harddisk32\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag12", BRACK(FH_SDAG | 12), "\\Device\\Harddisk32\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag13", BRACK(FH_SDAG | 13), "\\Device\\Harddisk32\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag14", BRACK(FH_SDAG | 14), "\\Device\\Harddisk32\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdag15", BRACK(FH_SDAG | 15), "\\Device\\Harddisk32\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah1", BRACK(FH_SDAH | 1), "\\Device\\Harddisk33\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah2", BRACK(FH_SDAH | 2), "\\Device\\Harddisk33\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah3", BRACK(FH_SDAH | 3), "\\Device\\Harddisk33\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah4", BRACK(FH_SDAH | 4), "\\Device\\Harddisk33\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah5", BRACK(FH_SDAH | 5), "\\Device\\Harddisk33\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah6", BRACK(FH_SDAH | 6), "\\Device\\Harddisk33\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah7", BRACK(FH_SDAH | 7), "\\Device\\Harddisk33\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah8", BRACK(FH_SDAH | 8), "\\Device\\Harddisk33\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah9", BRACK(FH_SDAH | 9), "\\Device\\Harddisk33\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah10", BRACK(FH_SDAH | 10), "\\Device\\Harddisk33\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah11", BRACK(FH_SDAH | 11), "\\Device\\Harddisk33\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah12", BRACK(FH_SDAH | 12), "\\Device\\Harddisk33\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah13", BRACK(FH_SDAH | 13), "\\Device\\Harddisk33\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah14", BRACK(FH_SDAH | 14), "\\Device\\Harddisk33\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdah15", BRACK(FH_SDAH | 15), "\\Device\\Harddisk33\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai1", BRACK(FH_SDAI | 1), "\\Device\\Harddisk34\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai2", BRACK(FH_SDAI | 2), "\\Device\\Harddisk34\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai3", BRACK(FH_SDAI | 3), "\\Device\\Harddisk34\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai4", BRACK(FH_SDAI | 4), "\\Device\\Harddisk34\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai5", BRACK(FH_SDAI | 5), "\\Device\\Harddisk34\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai6", BRACK(FH_SDAI | 6), "\\Device\\Harddisk34\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai7", BRACK(FH_SDAI | 7), "\\Device\\Harddisk34\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai8", BRACK(FH_SDAI | 8), "\\Device\\Harddisk34\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai9", BRACK(FH_SDAI | 9), "\\Device\\Harddisk34\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai10", BRACK(FH_SDAI | 10), "\\Device\\Harddisk34\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai11", BRACK(FH_SDAI | 11), "\\Device\\Harddisk34\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai12", BRACK(FH_SDAI | 12), "\\Device\\Harddisk34\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai13", BRACK(FH_SDAI | 13), "\\Device\\Harddisk34\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai14", BRACK(FH_SDAI | 14), "\\Device\\Harddisk34\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdai15", BRACK(FH_SDAI | 15), "\\Device\\Harddisk34\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj1", BRACK(FH_SDAJ | 1), "\\Device\\Harddisk35\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj2", BRACK(FH_SDAJ | 2), "\\Device\\Harddisk35\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj3", BRACK(FH_SDAJ | 3), "\\Device\\Harddisk35\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj4", BRACK(FH_SDAJ | 4), "\\Device\\Harddisk35\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj5", BRACK(FH_SDAJ | 5), "\\Device\\Harddisk35\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj6", BRACK(FH_SDAJ | 6), "\\Device\\Harddisk35\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj7", BRACK(FH_SDAJ | 7), "\\Device\\Harddisk35\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj8", BRACK(FH_SDAJ | 8), "\\Device\\Harddisk35\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj9", BRACK(FH_SDAJ | 9), "\\Device\\Harddisk35\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj10", BRACK(FH_SDAJ | 10), "\\Device\\Harddisk35\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj11", BRACK(FH_SDAJ | 11), "\\Device\\Harddisk35\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj12", BRACK(FH_SDAJ | 12), "\\Device\\Harddisk35\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj13", BRACK(FH_SDAJ | 13), "\\Device\\Harddisk35\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj14", BRACK(FH_SDAJ | 14), "\\Device\\Harddisk35\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaj15", BRACK(FH_SDAJ | 15), "\\Device\\Harddisk35\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak1", BRACK(FH_SDAK | 1), "\\Device\\Harddisk36\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak2", BRACK(FH_SDAK | 2), "\\Device\\Harddisk36\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak3", BRACK(FH_SDAK | 3), "\\Device\\Harddisk36\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak4", BRACK(FH_SDAK | 4), "\\Device\\Harddisk36\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak5", BRACK(FH_SDAK | 5), "\\Device\\Harddisk36\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak6", BRACK(FH_SDAK | 6), "\\Device\\Harddisk36\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak7", BRACK(FH_SDAK | 7), "\\Device\\Harddisk36\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak8", BRACK(FH_SDAK | 8), "\\Device\\Harddisk36\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak9", BRACK(FH_SDAK | 9), "\\Device\\Harddisk36\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak10", BRACK(FH_SDAK | 10), "\\Device\\Harddisk36\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak11", BRACK(FH_SDAK | 11), "\\Device\\Harddisk36\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak12", BRACK(FH_SDAK | 12), "\\Device\\Harddisk36\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak13", BRACK(FH_SDAK | 13), "\\Device\\Harddisk36\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak14", BRACK(FH_SDAK | 14), "\\Device\\Harddisk36\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdak15", BRACK(FH_SDAK | 15), "\\Device\\Harddisk36\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal1", BRACK(FH_SDAL | 1), "\\Device\\Harddisk37\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal2", BRACK(FH_SDAL | 2), "\\Device\\Harddisk37\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal3", BRACK(FH_SDAL | 3), "\\Device\\Harddisk37\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal4", BRACK(FH_SDAL | 4), "\\Device\\Harddisk37\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal5", BRACK(FH_SDAL | 5), "\\Device\\Harddisk37\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal6", BRACK(FH_SDAL | 6), "\\Device\\Harddisk37\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal7", BRACK(FH_SDAL | 7), "\\Device\\Harddisk37\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal8", BRACK(FH_SDAL | 8), "\\Device\\Harddisk37\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal9", BRACK(FH_SDAL | 9), "\\Device\\Harddisk37\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal10", BRACK(FH_SDAL | 10), "\\Device\\Harddisk37\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal11", BRACK(FH_SDAL | 11), "\\Device\\Harddisk37\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal12", BRACK(FH_SDAL | 12), "\\Device\\Harddisk37\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal13", BRACK(FH_SDAL | 13), "\\Device\\Harddisk37\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal14", BRACK(FH_SDAL | 14), "\\Device\\Harddisk37\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdal15", BRACK(FH_SDAL | 15), "\\Device\\Harddisk37\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam1", BRACK(FH_SDAM | 1), "\\Device\\Harddisk38\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam2", BRACK(FH_SDAM | 2), "\\Device\\Harddisk38\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam3", BRACK(FH_SDAM | 3), "\\Device\\Harddisk38\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam4", BRACK(FH_SDAM | 4), "\\Device\\Harddisk38\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam5", BRACK(FH_SDAM | 5), "\\Device\\Harddisk38\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam6", BRACK(FH_SDAM | 6), "\\Device\\Harddisk38\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam7", BRACK(FH_SDAM | 7), "\\Device\\Harddisk38\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam8", BRACK(FH_SDAM | 8), "\\Device\\Harddisk38\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam9", BRACK(FH_SDAM | 9), "\\Device\\Harddisk38\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam10", BRACK(FH_SDAM | 10), "\\Device\\Harddisk38\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam11", BRACK(FH_SDAM | 11), "\\Device\\Harddisk38\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam12", BRACK(FH_SDAM | 12), "\\Device\\Harddisk38\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam13", BRACK(FH_SDAM | 13), "\\Device\\Harddisk38\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam14", BRACK(FH_SDAM | 14), "\\Device\\Harddisk38\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdam15", BRACK(FH_SDAM | 15), "\\Device\\Harddisk38\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan1", BRACK(FH_SDAN | 1), "\\Device\\Harddisk39\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan2", BRACK(FH_SDAN | 2), "\\Device\\Harddisk39\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan3", BRACK(FH_SDAN | 3), "\\Device\\Harddisk39\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan4", BRACK(FH_SDAN | 4), "\\Device\\Harddisk39\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan5", BRACK(FH_SDAN | 5), "\\Device\\Harddisk39\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan6", BRACK(FH_SDAN | 6), "\\Device\\Harddisk39\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan7", BRACK(FH_SDAN | 7), "\\Device\\Harddisk39\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan8", BRACK(FH_SDAN | 8), "\\Device\\Harddisk39\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan9", BRACK(FH_SDAN | 9), "\\Device\\Harddisk39\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan10", BRACK(FH_SDAN | 10), "\\Device\\Harddisk39\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan11", BRACK(FH_SDAN | 11), "\\Device\\Harddisk39\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan12", BRACK(FH_SDAN | 12), "\\Device\\Harddisk39\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan13", BRACK(FH_SDAN | 13), "\\Device\\Harddisk39\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan14", BRACK(FH_SDAN | 14), "\\Device\\Harddisk39\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdan15", BRACK(FH_SDAN | 15), "\\Device\\Harddisk39\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao1", BRACK(FH_SDAO | 1), "\\Device\\Harddisk40\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao2", BRACK(FH_SDAO | 2), "\\Device\\Harddisk40\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao3", BRACK(FH_SDAO | 3), "\\Device\\Harddisk40\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao4", BRACK(FH_SDAO | 4), "\\Device\\Harddisk40\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao5", BRACK(FH_SDAO | 5), "\\Device\\Harddisk40\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao6", BRACK(FH_SDAO | 6), "\\Device\\Harddisk40\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao7", BRACK(FH_SDAO | 7), "\\Device\\Harddisk40\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao8", BRACK(FH_SDAO | 8), "\\Device\\Harddisk40\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao9", BRACK(FH_SDAO | 9), "\\Device\\Harddisk40\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao10", BRACK(FH_SDAO | 10), "\\Device\\Harddisk40\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao11", BRACK(FH_SDAO | 11), "\\Device\\Harddisk40\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao12", BRACK(FH_SDAO | 12), "\\Device\\Harddisk40\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao13", BRACK(FH_SDAO | 13), "\\Device\\Harddisk40\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao14", BRACK(FH_SDAO | 14), "\\Device\\Harddisk40\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdao15", BRACK(FH_SDAO | 15), "\\Device\\Harddisk40\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap1", BRACK(FH_SDAP | 1), "\\Device\\Harddisk41\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap2", BRACK(FH_SDAP | 2), "\\Device\\Harddisk41\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap3", BRACK(FH_SDAP | 3), "\\Device\\Harddisk41\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap4", BRACK(FH_SDAP | 4), "\\Device\\Harddisk41\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap5", BRACK(FH_SDAP | 5), "\\Device\\Harddisk41\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap6", BRACK(FH_SDAP | 6), "\\Device\\Harddisk41\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap7", BRACK(FH_SDAP | 7), "\\Device\\Harddisk41\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap8", BRACK(FH_SDAP | 8), "\\Device\\Harddisk41\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap9", BRACK(FH_SDAP | 9), "\\Device\\Harddisk41\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap10", BRACK(FH_SDAP | 10), "\\Device\\Harddisk41\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap11", BRACK(FH_SDAP | 11), "\\Device\\Harddisk41\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap12", BRACK(FH_SDAP | 12), "\\Device\\Harddisk41\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap13", BRACK(FH_SDAP | 13), "\\Device\\Harddisk41\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap14", BRACK(FH_SDAP | 14), "\\Device\\Harddisk41\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdap15", BRACK(FH_SDAP | 15), "\\Device\\Harddisk41\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq1", BRACK(FH_SDAQ | 1), "\\Device\\Harddisk42\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq2", BRACK(FH_SDAQ | 2), "\\Device\\Harddisk42\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq3", BRACK(FH_SDAQ | 3), "\\Device\\Harddisk42\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq4", BRACK(FH_SDAQ | 4), "\\Device\\Harddisk42\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq5", BRACK(FH_SDAQ | 5), "\\Device\\Harddisk42\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq6", BRACK(FH_SDAQ | 6), "\\Device\\Harddisk42\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq7", BRACK(FH_SDAQ | 7), "\\Device\\Harddisk42\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq8", BRACK(FH_SDAQ | 8), "\\Device\\Harddisk42\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq9", BRACK(FH_SDAQ | 9), "\\Device\\Harddisk42\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq10", BRACK(FH_SDAQ | 10), "\\Device\\Harddisk42\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq11", BRACK(FH_SDAQ | 11), "\\Device\\Harddisk42\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq12", BRACK(FH_SDAQ | 12), "\\Device\\Harddisk42\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq13", BRACK(FH_SDAQ | 13), "\\Device\\Harddisk42\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq14", BRACK(FH_SDAQ | 14), "\\Device\\Harddisk42\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaq15", BRACK(FH_SDAQ | 15), "\\Device\\Harddisk42\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar1", BRACK(FH_SDAR | 1), "\\Device\\Harddisk43\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar2", BRACK(FH_SDAR | 2), "\\Device\\Harddisk43\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar3", BRACK(FH_SDAR | 3), "\\Device\\Harddisk43\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar4", BRACK(FH_SDAR | 4), "\\Device\\Harddisk43\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar5", BRACK(FH_SDAR | 5), "\\Device\\Harddisk43\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar6", BRACK(FH_SDAR | 6), "\\Device\\Harddisk43\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar7", BRACK(FH_SDAR | 7), "\\Device\\Harddisk43\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar8", BRACK(FH_SDAR | 8), "\\Device\\Harddisk43\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar9", BRACK(FH_SDAR | 9), "\\Device\\Harddisk43\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar10", BRACK(FH_SDAR | 10), "\\Device\\Harddisk43\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar11", BRACK(FH_SDAR | 11), "\\Device\\Harddisk43\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar12", BRACK(FH_SDAR | 12), "\\Device\\Harddisk43\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar13", BRACK(FH_SDAR | 13), "\\Device\\Harddisk43\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar14", BRACK(FH_SDAR | 14), "\\Device\\Harddisk43\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdar15", BRACK(FH_SDAR | 15), "\\Device\\Harddisk43\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas1", BRACK(FH_SDAS | 1), "\\Device\\Harddisk44\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas2", BRACK(FH_SDAS | 2), "\\Device\\Harddisk44\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas3", BRACK(FH_SDAS | 3), "\\Device\\Harddisk44\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas4", BRACK(FH_SDAS | 4), "\\Device\\Harddisk44\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas5", BRACK(FH_SDAS | 5), "\\Device\\Harddisk44\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas6", BRACK(FH_SDAS | 6), "\\Device\\Harddisk44\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas7", BRACK(FH_SDAS | 7), "\\Device\\Harddisk44\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas8", BRACK(FH_SDAS | 8), "\\Device\\Harddisk44\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas9", BRACK(FH_SDAS | 9), "\\Device\\Harddisk44\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas10", BRACK(FH_SDAS | 10), "\\Device\\Harddisk44\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas11", BRACK(FH_SDAS | 11), "\\Device\\Harddisk44\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas12", BRACK(FH_SDAS | 12), "\\Device\\Harddisk44\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas13", BRACK(FH_SDAS | 13), "\\Device\\Harddisk44\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas14", BRACK(FH_SDAS | 14), "\\Device\\Harddisk44\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdas15", BRACK(FH_SDAS | 15), "\\Device\\Harddisk44\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat1", BRACK(FH_SDAT | 1), "\\Device\\Harddisk45\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat2", BRACK(FH_SDAT | 2), "\\Device\\Harddisk45\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat3", BRACK(FH_SDAT | 3), "\\Device\\Harddisk45\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat4", BRACK(FH_SDAT | 4), "\\Device\\Harddisk45\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat5", BRACK(FH_SDAT | 5), "\\Device\\Harddisk45\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat6", BRACK(FH_SDAT | 6), "\\Device\\Harddisk45\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat7", BRACK(FH_SDAT | 7), "\\Device\\Harddisk45\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat8", BRACK(FH_SDAT | 8), "\\Device\\Harddisk45\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat9", BRACK(FH_SDAT | 9), "\\Device\\Harddisk45\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat10", BRACK(FH_SDAT | 10), "\\Device\\Harddisk45\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat11", BRACK(FH_SDAT | 11), "\\Device\\Harddisk45\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat12", BRACK(FH_SDAT | 12), "\\Device\\Harddisk45\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat13", BRACK(FH_SDAT | 13), "\\Device\\Harddisk45\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat14", BRACK(FH_SDAT | 14), "\\Device\\Harddisk45\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdat15", BRACK(FH_SDAT | 15), "\\Device\\Harddisk45\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau1", BRACK(FH_SDAU | 1), "\\Device\\Harddisk46\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau2", BRACK(FH_SDAU | 2), "\\Device\\Harddisk46\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau3", BRACK(FH_SDAU | 3), "\\Device\\Harddisk46\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau4", BRACK(FH_SDAU | 4), "\\Device\\Harddisk46\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau5", BRACK(FH_SDAU | 5), "\\Device\\Harddisk46\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau6", BRACK(FH_SDAU | 6), "\\Device\\Harddisk46\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau7", BRACK(FH_SDAU | 7), "\\Device\\Harddisk46\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau8", BRACK(FH_SDAU | 8), "\\Device\\Harddisk46\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau9", BRACK(FH_SDAU | 9), "\\Device\\Harddisk46\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau10", BRACK(FH_SDAU | 10), "\\Device\\Harddisk46\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau11", BRACK(FH_SDAU | 11), "\\Device\\Harddisk46\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau12", BRACK(FH_SDAU | 12), "\\Device\\Harddisk46\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau13", BRACK(FH_SDAU | 13), "\\Device\\Harddisk46\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau14", BRACK(FH_SDAU | 14), "\\Device\\Harddisk46\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdau15", BRACK(FH_SDAU | 15), "\\Device\\Harddisk46\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav1", BRACK(FH_SDAV | 1), "\\Device\\Harddisk47\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav2", BRACK(FH_SDAV | 2), "\\Device\\Harddisk47\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav3", BRACK(FH_SDAV | 3), "\\Device\\Harddisk47\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav4", BRACK(FH_SDAV | 4), "\\Device\\Harddisk47\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav5", BRACK(FH_SDAV | 5), "\\Device\\Harddisk47\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav6", BRACK(FH_SDAV | 6), "\\Device\\Harddisk47\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav7", BRACK(FH_SDAV | 7), "\\Device\\Harddisk47\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav8", BRACK(FH_SDAV | 8), "\\Device\\Harddisk47\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav9", BRACK(FH_SDAV | 9), "\\Device\\Harddisk47\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav10", BRACK(FH_SDAV | 10), "\\Device\\Harddisk47\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav11", BRACK(FH_SDAV | 11), "\\Device\\Harddisk47\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav12", BRACK(FH_SDAV | 12), "\\Device\\Harddisk47\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav13", BRACK(FH_SDAV | 13), "\\Device\\Harddisk47\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav14", BRACK(FH_SDAV | 14), "\\Device\\Harddisk47\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdav15", BRACK(FH_SDAV | 15), "\\Device\\Harddisk47\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw1", BRACK(FH_SDAW | 1), "\\Device\\Harddisk48\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw2", BRACK(FH_SDAW | 2), "\\Device\\Harddisk48\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw3", BRACK(FH_SDAW | 3), "\\Device\\Harddisk48\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw4", BRACK(FH_SDAW | 4), "\\Device\\Harddisk48\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw5", BRACK(FH_SDAW | 5), "\\Device\\Harddisk48\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw6", BRACK(FH_SDAW | 6), "\\Device\\Harddisk48\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw7", BRACK(FH_SDAW | 7), "\\Device\\Harddisk48\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw8", BRACK(FH_SDAW | 8), "\\Device\\Harddisk48\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw9", BRACK(FH_SDAW | 9), "\\Device\\Harddisk48\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw10", BRACK(FH_SDAW | 10), "\\Device\\Harddisk48\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw11", BRACK(FH_SDAW | 11), "\\Device\\Harddisk48\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw12", BRACK(FH_SDAW | 12), "\\Device\\Harddisk48\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw13", BRACK(FH_SDAW | 13), "\\Device\\Harddisk48\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw14", BRACK(FH_SDAW | 14), "\\Device\\Harddisk48\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaw15", BRACK(FH_SDAW | 15), "\\Device\\Harddisk48\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax1", BRACK(FH_SDAX | 1), "\\Device\\Harddisk49\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax2", BRACK(FH_SDAX | 2), "\\Device\\Harddisk49\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax3", BRACK(FH_SDAX | 3), "\\Device\\Harddisk49\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax4", BRACK(FH_SDAX | 4), "\\Device\\Harddisk49\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax5", BRACK(FH_SDAX | 5), "\\Device\\Harddisk49\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax6", BRACK(FH_SDAX | 6), "\\Device\\Harddisk49\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax7", BRACK(FH_SDAX | 7), "\\Device\\Harddisk49\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax8", BRACK(FH_SDAX | 8), "\\Device\\Harddisk49\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax9", BRACK(FH_SDAX | 9), "\\Device\\Harddisk49\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax10", BRACK(FH_SDAX | 10), "\\Device\\Harddisk49\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax11", BRACK(FH_SDAX | 11), "\\Device\\Harddisk49\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax12", BRACK(FH_SDAX | 12), "\\Device\\Harddisk49\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax13", BRACK(FH_SDAX | 13), "\\Device\\Harddisk49\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax14", BRACK(FH_SDAX | 14), "\\Device\\Harddisk49\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdax15", BRACK(FH_SDAX | 15), "\\Device\\Harddisk49\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sday1", BRACK(FH_SDAY | 1), "\\Device\\Harddisk50\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sday2", BRACK(FH_SDAY | 2), "\\Device\\Harddisk50\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sday3", BRACK(FH_SDAY | 3), "\\Device\\Harddisk50\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sday4", BRACK(FH_SDAY | 4), "\\Device\\Harddisk50\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sday5", BRACK(FH_SDAY | 5), "\\Device\\Harddisk50\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sday6", BRACK(FH_SDAY | 6), "\\Device\\Harddisk50\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sday7", BRACK(FH_SDAY | 7), "\\Device\\Harddisk50\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sday8", BRACK(FH_SDAY | 8), "\\Device\\Harddisk50\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sday9", BRACK(FH_SDAY | 9), "\\Device\\Harddisk50\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sday10", BRACK(FH_SDAY | 10), "\\Device\\Harddisk50\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sday11", BRACK(FH_SDAY | 11), "\\Device\\Harddisk50\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sday12", BRACK(FH_SDAY | 12), "\\Device\\Harddisk50\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sday13", BRACK(FH_SDAY | 13), "\\Device\\Harddisk50\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sday14", BRACK(FH_SDAY | 14), "\\Device\\Harddisk50\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sday15", BRACK(FH_SDAY | 15), "\\Device\\Harddisk50\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz1", BRACK(FH_SDAZ | 1), "\\Device\\Harddisk51\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz2", BRACK(FH_SDAZ | 2), "\\Device\\Harddisk51\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz3", BRACK(FH_SDAZ | 3), "\\Device\\Harddisk51\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz4", BRACK(FH_SDAZ | 4), "\\Device\\Harddisk51\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz5", BRACK(FH_SDAZ | 5), "\\Device\\Harddisk51\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz6", BRACK(FH_SDAZ | 6), "\\Device\\Harddisk51\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz7", BRACK(FH_SDAZ | 7), "\\Device\\Harddisk51\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz8", BRACK(FH_SDAZ | 8), "\\Device\\Harddisk51\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz9", BRACK(FH_SDAZ | 9), "\\Device\\Harddisk51\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz10", BRACK(FH_SDAZ | 10), "\\Device\\Harddisk51\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz11", BRACK(FH_SDAZ | 11), "\\Device\\Harddisk51\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz12", BRACK(FH_SDAZ | 12), "\\Device\\Harddisk51\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz13", BRACK(FH_SDAZ | 13), "\\Device\\Harddisk51\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz14", BRACK(FH_SDAZ | 14), "\\Device\\Harddisk51\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdaz15", BRACK(FH_SDAZ | 15), "\\Device\\Harddisk51\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba", BRACK(FH_SDBA), "\\Device\\Harddisk52\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb", BRACK(FH_SDBB), "\\Device\\Harddisk53\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc", BRACK(FH_SDBC), "\\Device\\Harddisk54\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd", BRACK(FH_SDBD), "\\Device\\Harddisk55\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe", BRACK(FH_SDBE), "\\Device\\Harddisk56\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf", BRACK(FH_SDBF), "\\Device\\Harddisk57\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg", BRACK(FH_SDBG), "\\Device\\Harddisk58\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh", BRACK(FH_SDBH), "\\Device\\Harddisk59\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi", BRACK(FH_SDBI), "\\Device\\Harddisk60\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj", BRACK(FH_SDBJ), "\\Device\\Harddisk61\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk", BRACK(FH_SDBK), "\\Device\\Harddisk62\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl", BRACK(FH_SDBL), "\\Device\\Harddisk63\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm", BRACK(FH_SDBM), "\\Device\\Harddisk64\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn", BRACK(FH_SDBN), "\\Device\\Harddisk65\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo", BRACK(FH_SDBO), "\\Device\\Harddisk66\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp", BRACK(FH_SDBP), "\\Device\\Harddisk67\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq", BRACK(FH_SDBQ), "\\Device\\Harddisk68\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr", BRACK(FH_SDBR), "\\Device\\Harddisk69\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs", BRACK(FH_SDBS), "\\Device\\Harddisk70\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt", BRACK(FH_SDBT), "\\Device\\Harddisk71\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu", BRACK(FH_SDBU), "\\Device\\Harddisk72\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv", BRACK(FH_SDBV), "\\Device\\Harddisk73\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw", BRACK(FH_SDBW), "\\Device\\Harddisk74\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx", BRACK(FH_SDBX), "\\Device\\Harddisk75\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby", BRACK(FH_SDBY), "\\Device\\Harddisk76\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz", BRACK(FH_SDBZ), "\\Device\\Harddisk77\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba1", BRACK(FH_SDBA | 1), "\\Device\\Harddisk52\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba2", BRACK(FH_SDBA | 2), "\\Device\\Harddisk52\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba3", BRACK(FH_SDBA | 3), "\\Device\\Harddisk52\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba4", BRACK(FH_SDBA | 4), "\\Device\\Harddisk52\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba5", BRACK(FH_SDBA | 5), "\\Device\\Harddisk52\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba6", BRACK(FH_SDBA | 6), "\\Device\\Harddisk52\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba7", BRACK(FH_SDBA | 7), "\\Device\\Harddisk52\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba8", BRACK(FH_SDBA | 8), "\\Device\\Harddisk52\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba9", BRACK(FH_SDBA | 9), "\\Device\\Harddisk52\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba10", BRACK(FH_SDBA | 10), "\\Device\\Harddisk52\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba11", BRACK(FH_SDBA | 11), "\\Device\\Harddisk52\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba12", BRACK(FH_SDBA | 12), "\\Device\\Harddisk52\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba13", BRACK(FH_SDBA | 13), "\\Device\\Harddisk52\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba14", BRACK(FH_SDBA | 14), "\\Device\\Harddisk52\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdba15", BRACK(FH_SDBA | 15), "\\Device\\Harddisk52\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb1", BRACK(FH_SDBB | 1), "\\Device\\Harddisk53\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb2", BRACK(FH_SDBB | 2), "\\Device\\Harddisk53\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb3", BRACK(FH_SDBB | 3), "\\Device\\Harddisk53\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb4", BRACK(FH_SDBB | 4), "\\Device\\Harddisk53\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb5", BRACK(FH_SDBB | 5), "\\Device\\Harddisk53\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb6", BRACK(FH_SDBB | 6), "\\Device\\Harddisk53\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb7", BRACK(FH_SDBB | 7), "\\Device\\Harddisk53\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb8", BRACK(FH_SDBB | 8), "\\Device\\Harddisk53\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb9", BRACK(FH_SDBB | 9), "\\Device\\Harddisk53\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb10", BRACK(FH_SDBB | 10), "\\Device\\Harddisk53\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb11", BRACK(FH_SDBB | 11), "\\Device\\Harddisk53\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb12", BRACK(FH_SDBB | 12), "\\Device\\Harddisk53\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb13", BRACK(FH_SDBB | 13), "\\Device\\Harddisk53\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb14", BRACK(FH_SDBB | 14), "\\Device\\Harddisk53\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbb15", BRACK(FH_SDBB | 15), "\\Device\\Harddisk53\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc1", BRACK(FH_SDBC | 1), "\\Device\\Harddisk54\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc2", BRACK(FH_SDBC | 2), "\\Device\\Harddisk54\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc3", BRACK(FH_SDBC | 3), "\\Device\\Harddisk54\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc4", BRACK(FH_SDBC | 4), "\\Device\\Harddisk54\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc5", BRACK(FH_SDBC | 5), "\\Device\\Harddisk54\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc6", BRACK(FH_SDBC | 6), "\\Device\\Harddisk54\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc7", BRACK(FH_SDBC | 7), "\\Device\\Harddisk54\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc8", BRACK(FH_SDBC | 8), "\\Device\\Harddisk54\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc9", BRACK(FH_SDBC | 9), "\\Device\\Harddisk54\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc10", BRACK(FH_SDBC | 10), "\\Device\\Harddisk54\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc11", BRACK(FH_SDBC | 11), "\\Device\\Harddisk54\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc12", BRACK(FH_SDBC | 12), "\\Device\\Harddisk54\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc13", BRACK(FH_SDBC | 13), "\\Device\\Harddisk54\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc14", BRACK(FH_SDBC | 14), "\\Device\\Harddisk54\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbc15", BRACK(FH_SDBC | 15), "\\Device\\Harddisk54\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd1", BRACK(FH_SDBD | 1), "\\Device\\Harddisk55\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd2", BRACK(FH_SDBD | 2), "\\Device\\Harddisk55\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd3", BRACK(FH_SDBD | 3), "\\Device\\Harddisk55\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd4", BRACK(FH_SDBD | 4), "\\Device\\Harddisk55\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd5", BRACK(FH_SDBD | 5), "\\Device\\Harddisk55\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd6", BRACK(FH_SDBD | 6), "\\Device\\Harddisk55\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd7", BRACK(FH_SDBD | 7), "\\Device\\Harddisk55\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd8", BRACK(FH_SDBD | 8), "\\Device\\Harddisk55\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd9", BRACK(FH_SDBD | 9), "\\Device\\Harddisk55\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd10", BRACK(FH_SDBD | 10), "\\Device\\Harddisk55\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd11", BRACK(FH_SDBD | 11), "\\Device\\Harddisk55\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd12", BRACK(FH_SDBD | 12), "\\Device\\Harddisk55\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd13", BRACK(FH_SDBD | 13), "\\Device\\Harddisk55\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd14", BRACK(FH_SDBD | 14), "\\Device\\Harddisk55\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbd15", BRACK(FH_SDBD | 15), "\\Device\\Harddisk55\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe1", BRACK(FH_SDBE | 1), "\\Device\\Harddisk56\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe2", BRACK(FH_SDBE | 2), "\\Device\\Harddisk56\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe3", BRACK(FH_SDBE | 3), "\\Device\\Harddisk56\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe4", BRACK(FH_SDBE | 4), "\\Device\\Harddisk56\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe5", BRACK(FH_SDBE | 5), "\\Device\\Harddisk56\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe6", BRACK(FH_SDBE | 6), "\\Device\\Harddisk56\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe7", BRACK(FH_SDBE | 7), "\\Device\\Harddisk56\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe8", BRACK(FH_SDBE | 8), "\\Device\\Harddisk56\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe9", BRACK(FH_SDBE | 9), "\\Device\\Harddisk56\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe10", BRACK(FH_SDBE | 10), "\\Device\\Harddisk56\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe11", BRACK(FH_SDBE | 11), "\\Device\\Harddisk56\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe12", BRACK(FH_SDBE | 12), "\\Device\\Harddisk56\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe13", BRACK(FH_SDBE | 13), "\\Device\\Harddisk56\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe14", BRACK(FH_SDBE | 14), "\\Device\\Harddisk56\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbe15", BRACK(FH_SDBE | 15), "\\Device\\Harddisk56\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf1", BRACK(FH_SDBF | 1), "\\Device\\Harddisk57\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf2", BRACK(FH_SDBF | 2), "\\Device\\Harddisk57\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf3", BRACK(FH_SDBF | 3), "\\Device\\Harddisk57\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf4", BRACK(FH_SDBF | 4), "\\Device\\Harddisk57\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf5", BRACK(FH_SDBF | 5), "\\Device\\Harddisk57\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf6", BRACK(FH_SDBF | 6), "\\Device\\Harddisk57\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf7", BRACK(FH_SDBF | 7), "\\Device\\Harddisk57\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf8", BRACK(FH_SDBF | 8), "\\Device\\Harddisk57\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf9", BRACK(FH_SDBF | 9), "\\Device\\Harddisk57\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf10", BRACK(FH_SDBF | 10), "\\Device\\Harddisk57\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf11", BRACK(FH_SDBF | 11), "\\Device\\Harddisk57\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf12", BRACK(FH_SDBF | 12), "\\Device\\Harddisk57\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf13", BRACK(FH_SDBF | 13), "\\Device\\Harddisk57\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf14", BRACK(FH_SDBF | 14), "\\Device\\Harddisk57\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbf15", BRACK(FH_SDBF | 15), "\\Device\\Harddisk57\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg1", BRACK(FH_SDBG | 1), "\\Device\\Harddisk58\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg2", BRACK(FH_SDBG | 2), "\\Device\\Harddisk58\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg3", BRACK(FH_SDBG | 3), "\\Device\\Harddisk58\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg4", BRACK(FH_SDBG | 4), "\\Device\\Harddisk58\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg5", BRACK(FH_SDBG | 5), "\\Device\\Harddisk58\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg6", BRACK(FH_SDBG | 6), "\\Device\\Harddisk58\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg7", BRACK(FH_SDBG | 7), "\\Device\\Harddisk58\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg8", BRACK(FH_SDBG | 8), "\\Device\\Harddisk58\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg9", BRACK(FH_SDBG | 9), "\\Device\\Harddisk58\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg10", BRACK(FH_SDBG | 10), "\\Device\\Harddisk58\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg11", BRACK(FH_SDBG | 11), "\\Device\\Harddisk58\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg12", BRACK(FH_SDBG | 12), "\\Device\\Harddisk58\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg13", BRACK(FH_SDBG | 13), "\\Device\\Harddisk58\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg14", BRACK(FH_SDBG | 14), "\\Device\\Harddisk58\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbg15", BRACK(FH_SDBG | 15), "\\Device\\Harddisk58\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh1", BRACK(FH_SDBH | 1), "\\Device\\Harddisk59\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh2", BRACK(FH_SDBH | 2), "\\Device\\Harddisk59\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh3", BRACK(FH_SDBH | 3), "\\Device\\Harddisk59\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh4", BRACK(FH_SDBH | 4), "\\Device\\Harddisk59\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh5", BRACK(FH_SDBH | 5), "\\Device\\Harddisk59\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh6", BRACK(FH_SDBH | 6), "\\Device\\Harddisk59\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh7", BRACK(FH_SDBH | 7), "\\Device\\Harddisk59\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh8", BRACK(FH_SDBH | 8), "\\Device\\Harddisk59\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh9", BRACK(FH_SDBH | 9), "\\Device\\Harddisk59\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh10", BRACK(FH_SDBH | 10), "\\Device\\Harddisk59\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh11", BRACK(FH_SDBH | 11), "\\Device\\Harddisk59\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh12", BRACK(FH_SDBH | 12), "\\Device\\Harddisk59\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh13", BRACK(FH_SDBH | 13), "\\Device\\Harddisk59\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh14", BRACK(FH_SDBH | 14), "\\Device\\Harddisk59\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbh15", BRACK(FH_SDBH | 15), "\\Device\\Harddisk59\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi1", BRACK(FH_SDBI | 1), "\\Device\\Harddisk60\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi2", BRACK(FH_SDBI | 2), "\\Device\\Harddisk60\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi3", BRACK(FH_SDBI | 3), "\\Device\\Harddisk60\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi4", BRACK(FH_SDBI | 4), "\\Device\\Harddisk60\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi5", BRACK(FH_SDBI | 5), "\\Device\\Harddisk60\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi6", BRACK(FH_SDBI | 6), "\\Device\\Harddisk60\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi7", BRACK(FH_SDBI | 7), "\\Device\\Harddisk60\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi8", BRACK(FH_SDBI | 8), "\\Device\\Harddisk60\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi9", BRACK(FH_SDBI | 9), "\\Device\\Harddisk60\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi10", BRACK(FH_SDBI | 10), "\\Device\\Harddisk60\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi11", BRACK(FH_SDBI | 11), "\\Device\\Harddisk60\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi12", BRACK(FH_SDBI | 12), "\\Device\\Harddisk60\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi13", BRACK(FH_SDBI | 13), "\\Device\\Harddisk60\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi14", BRACK(FH_SDBI | 14), "\\Device\\Harddisk60\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbi15", BRACK(FH_SDBI | 15), "\\Device\\Harddisk60\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj1", BRACK(FH_SDBJ | 1), "\\Device\\Harddisk61\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj2", BRACK(FH_SDBJ | 2), "\\Device\\Harddisk61\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj3", BRACK(FH_SDBJ | 3), "\\Device\\Harddisk61\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj4", BRACK(FH_SDBJ | 4), "\\Device\\Harddisk61\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj5", BRACK(FH_SDBJ | 5), "\\Device\\Harddisk61\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj6", BRACK(FH_SDBJ | 6), "\\Device\\Harddisk61\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj7", BRACK(FH_SDBJ | 7), "\\Device\\Harddisk61\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj8", BRACK(FH_SDBJ | 8), "\\Device\\Harddisk61\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj9", BRACK(FH_SDBJ | 9), "\\Device\\Harddisk61\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj10", BRACK(FH_SDBJ | 10), "\\Device\\Harddisk61\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj11", BRACK(FH_SDBJ | 11), "\\Device\\Harddisk61\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj12", BRACK(FH_SDBJ | 12), "\\Device\\Harddisk61\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj13", BRACK(FH_SDBJ | 13), "\\Device\\Harddisk61\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj14", BRACK(FH_SDBJ | 14), "\\Device\\Harddisk61\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbj15", BRACK(FH_SDBJ | 15), "\\Device\\Harddisk61\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk1", BRACK(FH_SDBK | 1), "\\Device\\Harddisk62\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk2", BRACK(FH_SDBK | 2), "\\Device\\Harddisk62\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk3", BRACK(FH_SDBK | 3), "\\Device\\Harddisk62\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk4", BRACK(FH_SDBK | 4), "\\Device\\Harddisk62\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk5", BRACK(FH_SDBK | 5), "\\Device\\Harddisk62\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk6", BRACK(FH_SDBK | 6), "\\Device\\Harddisk62\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk7", BRACK(FH_SDBK | 7), "\\Device\\Harddisk62\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk8", BRACK(FH_SDBK | 8), "\\Device\\Harddisk62\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk9", BRACK(FH_SDBK | 9), "\\Device\\Harddisk62\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk10", BRACK(FH_SDBK | 10), "\\Device\\Harddisk62\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk11", BRACK(FH_SDBK | 11), "\\Device\\Harddisk62\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk12", BRACK(FH_SDBK | 12), "\\Device\\Harddisk62\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk13", BRACK(FH_SDBK | 13), "\\Device\\Harddisk62\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk14", BRACK(FH_SDBK | 14), "\\Device\\Harddisk62\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbk15", BRACK(FH_SDBK | 15), "\\Device\\Harddisk62\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl1", BRACK(FH_SDBL | 1), "\\Device\\Harddisk63\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl2", BRACK(FH_SDBL | 2), "\\Device\\Harddisk63\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl3", BRACK(FH_SDBL | 3), "\\Device\\Harddisk63\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl4", BRACK(FH_SDBL | 4), "\\Device\\Harddisk63\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl5", BRACK(FH_SDBL | 5), "\\Device\\Harddisk63\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl6", BRACK(FH_SDBL | 6), "\\Device\\Harddisk63\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl7", BRACK(FH_SDBL | 7), "\\Device\\Harddisk63\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl8", BRACK(FH_SDBL | 8), "\\Device\\Harddisk63\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl9", BRACK(FH_SDBL | 9), "\\Device\\Harddisk63\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl10", BRACK(FH_SDBL | 10), "\\Device\\Harddisk63\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl11", BRACK(FH_SDBL | 11), "\\Device\\Harddisk63\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl12", BRACK(FH_SDBL | 12), "\\Device\\Harddisk63\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl13", BRACK(FH_SDBL | 13), "\\Device\\Harddisk63\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl14", BRACK(FH_SDBL | 14), "\\Device\\Harddisk63\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbl15", BRACK(FH_SDBL | 15), "\\Device\\Harddisk63\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm1", BRACK(FH_SDBM | 1), "\\Device\\Harddisk64\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm2", BRACK(FH_SDBM | 2), "\\Device\\Harddisk64\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm3", BRACK(FH_SDBM | 3), "\\Device\\Harddisk64\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm4", BRACK(FH_SDBM | 4), "\\Device\\Harddisk64\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm5", BRACK(FH_SDBM | 5), "\\Device\\Harddisk64\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm6", BRACK(FH_SDBM | 6), "\\Device\\Harddisk64\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm7", BRACK(FH_SDBM | 7), "\\Device\\Harddisk64\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm8", BRACK(FH_SDBM | 8), "\\Device\\Harddisk64\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm9", BRACK(FH_SDBM | 9), "\\Device\\Harddisk64\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm10", BRACK(FH_SDBM | 10), "\\Device\\Harddisk64\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm11", BRACK(FH_SDBM | 11), "\\Device\\Harddisk64\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm12", BRACK(FH_SDBM | 12), "\\Device\\Harddisk64\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm13", BRACK(FH_SDBM | 13), "\\Device\\Harddisk64\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm14", BRACK(FH_SDBM | 14), "\\Device\\Harddisk64\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbm15", BRACK(FH_SDBM | 15), "\\Device\\Harddisk64\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn1", BRACK(FH_SDBN | 1), "\\Device\\Harddisk65\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn2", BRACK(FH_SDBN | 2), "\\Device\\Harddisk65\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn3", BRACK(FH_SDBN | 3), "\\Device\\Harddisk65\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn4", BRACK(FH_SDBN | 4), "\\Device\\Harddisk65\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn5", BRACK(FH_SDBN | 5), "\\Device\\Harddisk65\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn6", BRACK(FH_SDBN | 6), "\\Device\\Harddisk65\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn7", BRACK(FH_SDBN | 7), "\\Device\\Harddisk65\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn8", BRACK(FH_SDBN | 8), "\\Device\\Harddisk65\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn9", BRACK(FH_SDBN | 9), "\\Device\\Harddisk65\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn10", BRACK(FH_SDBN | 10), "\\Device\\Harddisk65\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn11", BRACK(FH_SDBN | 11), "\\Device\\Harddisk65\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn12", BRACK(FH_SDBN | 12), "\\Device\\Harddisk65\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn13", BRACK(FH_SDBN | 13), "\\Device\\Harddisk65\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn14", BRACK(FH_SDBN | 14), "\\Device\\Harddisk65\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbn15", BRACK(FH_SDBN | 15), "\\Device\\Harddisk65\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo1", BRACK(FH_SDBO | 1), "\\Device\\Harddisk66\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo2", BRACK(FH_SDBO | 2), "\\Device\\Harddisk66\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo3", BRACK(FH_SDBO | 3), "\\Device\\Harddisk66\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo4", BRACK(FH_SDBO | 4), "\\Device\\Harddisk66\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo5", BRACK(FH_SDBO | 5), "\\Device\\Harddisk66\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo6", BRACK(FH_SDBO | 6), "\\Device\\Harddisk66\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo7", BRACK(FH_SDBO | 7), "\\Device\\Harddisk66\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo8", BRACK(FH_SDBO | 8), "\\Device\\Harddisk66\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo9", BRACK(FH_SDBO | 9), "\\Device\\Harddisk66\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo10", BRACK(FH_SDBO | 10), "\\Device\\Harddisk66\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo11", BRACK(FH_SDBO | 11), "\\Device\\Harddisk66\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo12", BRACK(FH_SDBO | 12), "\\Device\\Harddisk66\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo13", BRACK(FH_SDBO | 13), "\\Device\\Harddisk66\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo14", BRACK(FH_SDBO | 14), "\\Device\\Harddisk66\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbo15", BRACK(FH_SDBO | 15), "\\Device\\Harddisk66\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp1", BRACK(FH_SDBP | 1), "\\Device\\Harddisk67\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp2", BRACK(FH_SDBP | 2), "\\Device\\Harddisk67\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp3", BRACK(FH_SDBP | 3), "\\Device\\Harddisk67\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp4", BRACK(FH_SDBP | 4), "\\Device\\Harddisk67\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp5", BRACK(FH_SDBP | 5), "\\Device\\Harddisk67\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp6", BRACK(FH_SDBP | 6), "\\Device\\Harddisk67\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp7", BRACK(FH_SDBP | 7), "\\Device\\Harddisk67\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp8", BRACK(FH_SDBP | 8), "\\Device\\Harddisk67\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp9", BRACK(FH_SDBP | 9), "\\Device\\Harddisk67\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp10", BRACK(FH_SDBP | 10), "\\Device\\Harddisk67\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp11", BRACK(FH_SDBP | 11), "\\Device\\Harddisk67\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp12", BRACK(FH_SDBP | 12), "\\Device\\Harddisk67\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp13", BRACK(FH_SDBP | 13), "\\Device\\Harddisk67\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp14", BRACK(FH_SDBP | 14), "\\Device\\Harddisk67\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbp15", BRACK(FH_SDBP | 15), "\\Device\\Harddisk67\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq1", BRACK(FH_SDBQ | 1), "\\Device\\Harddisk68\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq2", BRACK(FH_SDBQ | 2), "\\Device\\Harddisk68\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq3", BRACK(FH_SDBQ | 3), "\\Device\\Harddisk68\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq4", BRACK(FH_SDBQ | 4), "\\Device\\Harddisk68\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq5", BRACK(FH_SDBQ | 5), "\\Device\\Harddisk68\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq6", BRACK(FH_SDBQ | 6), "\\Device\\Harddisk68\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq7", BRACK(FH_SDBQ | 7), "\\Device\\Harddisk68\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq8", BRACK(FH_SDBQ | 8), "\\Device\\Harddisk68\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq9", BRACK(FH_SDBQ | 9), "\\Device\\Harddisk68\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq10", BRACK(FH_SDBQ | 10), "\\Device\\Harddisk68\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq11", BRACK(FH_SDBQ | 11), "\\Device\\Harddisk68\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq12", BRACK(FH_SDBQ | 12), "\\Device\\Harddisk68\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq13", BRACK(FH_SDBQ | 13), "\\Device\\Harddisk68\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq14", BRACK(FH_SDBQ | 14), "\\Device\\Harddisk68\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbq15", BRACK(FH_SDBQ | 15), "\\Device\\Harddisk68\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr1", BRACK(FH_SDBR | 1), "\\Device\\Harddisk69\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr2", BRACK(FH_SDBR | 2), "\\Device\\Harddisk69\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr3", BRACK(FH_SDBR | 3), "\\Device\\Harddisk69\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr4", BRACK(FH_SDBR | 4), "\\Device\\Harddisk69\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr5", BRACK(FH_SDBR | 5), "\\Device\\Harddisk69\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr6", BRACK(FH_SDBR | 6), "\\Device\\Harddisk69\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr7", BRACK(FH_SDBR | 7), "\\Device\\Harddisk69\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr8", BRACK(FH_SDBR | 8), "\\Device\\Harddisk69\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr9", BRACK(FH_SDBR | 9), "\\Device\\Harddisk69\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr10", BRACK(FH_SDBR | 10), "\\Device\\Harddisk69\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr11", BRACK(FH_SDBR | 11), "\\Device\\Harddisk69\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr12", BRACK(FH_SDBR | 12), "\\Device\\Harddisk69\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr13", BRACK(FH_SDBR | 13), "\\Device\\Harddisk69\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr14", BRACK(FH_SDBR | 14), "\\Device\\Harddisk69\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbr15", BRACK(FH_SDBR | 15), "\\Device\\Harddisk69\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs1", BRACK(FH_SDBS | 1), "\\Device\\Harddisk70\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs2", BRACK(FH_SDBS | 2), "\\Device\\Harddisk70\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs3", BRACK(FH_SDBS | 3), "\\Device\\Harddisk70\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs4", BRACK(FH_SDBS | 4), "\\Device\\Harddisk70\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs5", BRACK(FH_SDBS | 5), "\\Device\\Harddisk70\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs6", BRACK(FH_SDBS | 6), "\\Device\\Harddisk70\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs7", BRACK(FH_SDBS | 7), "\\Device\\Harddisk70\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs8", BRACK(FH_SDBS | 8), "\\Device\\Harddisk70\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs9", BRACK(FH_SDBS | 9), "\\Device\\Harddisk70\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs10", BRACK(FH_SDBS | 10), "\\Device\\Harddisk70\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs11", BRACK(FH_SDBS | 11), "\\Device\\Harddisk70\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs12", BRACK(FH_SDBS | 12), "\\Device\\Harddisk70\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs13", BRACK(FH_SDBS | 13), "\\Device\\Harddisk70\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs14", BRACK(FH_SDBS | 14), "\\Device\\Harddisk70\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbs15", BRACK(FH_SDBS | 15), "\\Device\\Harddisk70\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt1", BRACK(FH_SDBT | 1), "\\Device\\Harddisk71\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt2", BRACK(FH_SDBT | 2), "\\Device\\Harddisk71\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt3", BRACK(FH_SDBT | 3), "\\Device\\Harddisk71\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt4", BRACK(FH_SDBT | 4), "\\Device\\Harddisk71\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt5", BRACK(FH_SDBT | 5), "\\Device\\Harddisk71\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt6", BRACK(FH_SDBT | 6), "\\Device\\Harddisk71\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt7", BRACK(FH_SDBT | 7), "\\Device\\Harddisk71\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt8", BRACK(FH_SDBT | 8), "\\Device\\Harddisk71\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt9", BRACK(FH_SDBT | 9), "\\Device\\Harddisk71\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt10", BRACK(FH_SDBT | 10), "\\Device\\Harddisk71\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt11", BRACK(FH_SDBT | 11), "\\Device\\Harddisk71\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt12", BRACK(FH_SDBT | 12), "\\Device\\Harddisk71\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt13", BRACK(FH_SDBT | 13), "\\Device\\Harddisk71\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt14", BRACK(FH_SDBT | 14), "\\Device\\Harddisk71\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbt15", BRACK(FH_SDBT | 15), "\\Device\\Harddisk71\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu1", BRACK(FH_SDBU | 1), "\\Device\\Harddisk72\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu2", BRACK(FH_SDBU | 2), "\\Device\\Harddisk72\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu3", BRACK(FH_SDBU | 3), "\\Device\\Harddisk72\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu4", BRACK(FH_SDBU | 4), "\\Device\\Harddisk72\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu5", BRACK(FH_SDBU | 5), "\\Device\\Harddisk72\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu6", BRACK(FH_SDBU | 6), "\\Device\\Harddisk72\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu7", BRACK(FH_SDBU | 7), "\\Device\\Harddisk72\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu8", BRACK(FH_SDBU | 8), "\\Device\\Harddisk72\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu9", BRACK(FH_SDBU | 9), "\\Device\\Harddisk72\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu10", BRACK(FH_SDBU | 10), "\\Device\\Harddisk72\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu11", BRACK(FH_SDBU | 11), "\\Device\\Harddisk72\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu12", BRACK(FH_SDBU | 12), "\\Device\\Harddisk72\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu13", BRACK(FH_SDBU | 13), "\\Device\\Harddisk72\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu14", BRACK(FH_SDBU | 14), "\\Device\\Harddisk72\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbu15", BRACK(FH_SDBU | 15), "\\Device\\Harddisk72\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv1", BRACK(FH_SDBV | 1), "\\Device\\Harddisk73\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv2", BRACK(FH_SDBV | 2), "\\Device\\Harddisk73\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv3", BRACK(FH_SDBV | 3), "\\Device\\Harddisk73\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv4", BRACK(FH_SDBV | 4), "\\Device\\Harddisk73\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv5", BRACK(FH_SDBV | 5), "\\Device\\Harddisk73\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv6", BRACK(FH_SDBV | 6), "\\Device\\Harddisk73\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv7", BRACK(FH_SDBV | 7), "\\Device\\Harddisk73\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv8", BRACK(FH_SDBV | 8), "\\Device\\Harddisk73\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv9", BRACK(FH_SDBV | 9), "\\Device\\Harddisk73\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv10", BRACK(FH_SDBV | 10), "\\Device\\Harddisk73\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv11", BRACK(FH_SDBV | 11), "\\Device\\Harddisk73\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv12", BRACK(FH_SDBV | 12), "\\Device\\Harddisk73\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv13", BRACK(FH_SDBV | 13), "\\Device\\Harddisk73\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv14", BRACK(FH_SDBV | 14), "\\Device\\Harddisk73\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbv15", BRACK(FH_SDBV | 15), "\\Device\\Harddisk73\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw1", BRACK(FH_SDBW | 1), "\\Device\\Harddisk74\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw2", BRACK(FH_SDBW | 2), "\\Device\\Harddisk74\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw3", BRACK(FH_SDBW | 3), "\\Device\\Harddisk74\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw4", BRACK(FH_SDBW | 4), "\\Device\\Harddisk74\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw5", BRACK(FH_SDBW | 5), "\\Device\\Harddisk74\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw6", BRACK(FH_SDBW | 6), "\\Device\\Harddisk74\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw7", BRACK(FH_SDBW | 7), "\\Device\\Harddisk74\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw8", BRACK(FH_SDBW | 8), "\\Device\\Harddisk74\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw9", BRACK(FH_SDBW | 9), "\\Device\\Harddisk74\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw10", BRACK(FH_SDBW | 10), "\\Device\\Harddisk74\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw11", BRACK(FH_SDBW | 11), "\\Device\\Harddisk74\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw12", BRACK(FH_SDBW | 12), "\\Device\\Harddisk74\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw13", BRACK(FH_SDBW | 13), "\\Device\\Harddisk74\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw14", BRACK(FH_SDBW | 14), "\\Device\\Harddisk74\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbw15", BRACK(FH_SDBW | 15), "\\Device\\Harddisk74\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx1", BRACK(FH_SDBX | 1), "\\Device\\Harddisk75\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx2", BRACK(FH_SDBX | 2), "\\Device\\Harddisk75\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx3", BRACK(FH_SDBX | 3), "\\Device\\Harddisk75\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx4", BRACK(FH_SDBX | 4), "\\Device\\Harddisk75\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx5", BRACK(FH_SDBX | 5), "\\Device\\Harddisk75\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx6", BRACK(FH_SDBX | 6), "\\Device\\Harddisk75\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx7", BRACK(FH_SDBX | 7), "\\Device\\Harddisk75\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx8", BRACK(FH_SDBX | 8), "\\Device\\Harddisk75\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx9", BRACK(FH_SDBX | 9), "\\Device\\Harddisk75\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx10", BRACK(FH_SDBX | 10), "\\Device\\Harddisk75\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx11", BRACK(FH_SDBX | 11), "\\Device\\Harddisk75\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx12", BRACK(FH_SDBX | 12), "\\Device\\Harddisk75\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx13", BRACK(FH_SDBX | 13), "\\Device\\Harddisk75\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx14", BRACK(FH_SDBX | 14), "\\Device\\Harddisk75\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbx15", BRACK(FH_SDBX | 15), "\\Device\\Harddisk75\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby1", BRACK(FH_SDBY | 1), "\\Device\\Harddisk76\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby2", BRACK(FH_SDBY | 2), "\\Device\\Harddisk76\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby3", BRACK(FH_SDBY | 3), "\\Device\\Harddisk76\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby4", BRACK(FH_SDBY | 4), "\\Device\\Harddisk76\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby5", BRACK(FH_SDBY | 5), "\\Device\\Harddisk76\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby6", BRACK(FH_SDBY | 6), "\\Device\\Harddisk76\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby7", BRACK(FH_SDBY | 7), "\\Device\\Harddisk76\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby8", BRACK(FH_SDBY | 8), "\\Device\\Harddisk76\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby9", BRACK(FH_SDBY | 9), "\\Device\\Harddisk76\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby10", BRACK(FH_SDBY | 10), "\\Device\\Harddisk76\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby11", BRACK(FH_SDBY | 11), "\\Device\\Harddisk76\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby12", BRACK(FH_SDBY | 12), "\\Device\\Harddisk76\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby13", BRACK(FH_SDBY | 13), "\\Device\\Harddisk76\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby14", BRACK(FH_SDBY | 14), "\\Device\\Harddisk76\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdby15", BRACK(FH_SDBY | 15), "\\Device\\Harddisk76\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz1", BRACK(FH_SDBZ | 1), "\\Device\\Harddisk77\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz2", BRACK(FH_SDBZ | 2), "\\Device\\Harddisk77\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz3", BRACK(FH_SDBZ | 3), "\\Device\\Harddisk77\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz4", BRACK(FH_SDBZ | 4), "\\Device\\Harddisk77\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz5", BRACK(FH_SDBZ | 5), "\\Device\\Harddisk77\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz6", BRACK(FH_SDBZ | 6), "\\Device\\Harddisk77\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz7", BRACK(FH_SDBZ | 7), "\\Device\\Harddisk77\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz8", BRACK(FH_SDBZ | 8), "\\Device\\Harddisk77\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz9", BRACK(FH_SDBZ | 9), "\\Device\\Harddisk77\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz10", BRACK(FH_SDBZ | 10), "\\Device\\Harddisk77\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz11", BRACK(FH_SDBZ | 11), "\\Device\\Harddisk77\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz12", BRACK(FH_SDBZ | 12), "\\Device\\Harddisk77\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz13", BRACK(FH_SDBZ | 13), "\\Device\\Harddisk77\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz14", BRACK(FH_SDBZ | 14), "\\Device\\Harddisk77\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdbz15", BRACK(FH_SDBZ | 15), "\\Device\\Harddisk77\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca", BRACK(FH_SDCA), "\\Device\\Harddisk78\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb", BRACK(FH_SDCB), "\\Device\\Harddisk79\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc", BRACK(FH_SDCC), "\\Device\\Harddisk80\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd", BRACK(FH_SDCD), "\\Device\\Harddisk81\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce", BRACK(FH_SDCE), "\\Device\\Harddisk82\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf", BRACK(FH_SDCF), "\\Device\\Harddisk83\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg", BRACK(FH_SDCG), "\\Device\\Harddisk84\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch", BRACK(FH_SDCH), "\\Device\\Harddisk85\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci", BRACK(FH_SDCI), "\\Device\\Harddisk86\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj", BRACK(FH_SDCJ), "\\Device\\Harddisk87\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck", BRACK(FH_SDCK), "\\Device\\Harddisk88\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl", BRACK(FH_SDCL), "\\Device\\Harddisk89\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm", BRACK(FH_SDCM), "\\Device\\Harddisk90\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn", BRACK(FH_SDCN), "\\Device\\Harddisk91\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco", BRACK(FH_SDCO), "\\Device\\Harddisk92\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp", BRACK(FH_SDCP), "\\Device\\Harddisk93\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq", BRACK(FH_SDCQ), "\\Device\\Harddisk94\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr", BRACK(FH_SDCR), "\\Device\\Harddisk95\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs", BRACK(FH_SDCS), "\\Device\\Harddisk96\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct", BRACK(FH_SDCT), "\\Device\\Harddisk97\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu", BRACK(FH_SDCU), "\\Device\\Harddisk98\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv", BRACK(FH_SDCV), "\\Device\\Harddisk99\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw", BRACK(FH_SDCW), "\\Device\\Harddisk100\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx", BRACK(FH_SDCX), "\\Device\\Harddisk101\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy", BRACK(FH_SDCY), "\\Device\\Harddisk102\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz", BRACK(FH_SDCZ), "\\Device\\Harddisk103\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca1", BRACK(FH_SDCA | 1), "\\Device\\Harddisk78\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca2", BRACK(FH_SDCA | 2), "\\Device\\Harddisk78\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca3", BRACK(FH_SDCA | 3), "\\Device\\Harddisk78\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca4", BRACK(FH_SDCA | 4), "\\Device\\Harddisk78\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca5", BRACK(FH_SDCA | 5), "\\Device\\Harddisk78\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca6", BRACK(FH_SDCA | 6), "\\Device\\Harddisk78\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca7", BRACK(FH_SDCA | 7), "\\Device\\Harddisk78\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca8", BRACK(FH_SDCA | 8), "\\Device\\Harddisk78\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca9", BRACK(FH_SDCA | 9), "\\Device\\Harddisk78\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca10", BRACK(FH_SDCA | 10), "\\Device\\Harddisk78\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca11", BRACK(FH_SDCA | 11), "\\Device\\Harddisk78\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca12", BRACK(FH_SDCA | 12), "\\Device\\Harddisk78\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca13", BRACK(FH_SDCA | 13), "\\Device\\Harddisk78\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca14", BRACK(FH_SDCA | 14), "\\Device\\Harddisk78\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdca15", BRACK(FH_SDCA | 15), "\\Device\\Harddisk78\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb1", BRACK(FH_SDCB | 1), "\\Device\\Harddisk79\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb2", BRACK(FH_SDCB | 2), "\\Device\\Harddisk79\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb3", BRACK(FH_SDCB | 3), "\\Device\\Harddisk79\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb4", BRACK(FH_SDCB | 4), "\\Device\\Harddisk79\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb5", BRACK(FH_SDCB | 5), "\\Device\\Harddisk79\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb6", BRACK(FH_SDCB | 6), "\\Device\\Harddisk79\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb7", BRACK(FH_SDCB | 7), "\\Device\\Harddisk79\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb8", BRACK(FH_SDCB | 8), "\\Device\\Harddisk79\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb9", BRACK(FH_SDCB | 9), "\\Device\\Harddisk79\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb10", BRACK(FH_SDCB | 10), "\\Device\\Harddisk79\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb11", BRACK(FH_SDCB | 11), "\\Device\\Harddisk79\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb12", BRACK(FH_SDCB | 12), "\\Device\\Harddisk79\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb13", BRACK(FH_SDCB | 13), "\\Device\\Harddisk79\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb14", BRACK(FH_SDCB | 14), "\\Device\\Harddisk79\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcb15", BRACK(FH_SDCB | 15), "\\Device\\Harddisk79\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc1", BRACK(FH_SDCC | 1), "\\Device\\Harddisk80\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc2", BRACK(FH_SDCC | 2), "\\Device\\Harddisk80\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc3", BRACK(FH_SDCC | 3), "\\Device\\Harddisk80\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc4", BRACK(FH_SDCC | 4), "\\Device\\Harddisk80\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc5", BRACK(FH_SDCC | 5), "\\Device\\Harddisk80\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc6", BRACK(FH_SDCC | 6), "\\Device\\Harddisk80\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc7", BRACK(FH_SDCC | 7), "\\Device\\Harddisk80\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc8", BRACK(FH_SDCC | 8), "\\Device\\Harddisk80\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc9", BRACK(FH_SDCC | 9), "\\Device\\Harddisk80\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc10", BRACK(FH_SDCC | 10), "\\Device\\Harddisk80\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc11", BRACK(FH_SDCC | 11), "\\Device\\Harddisk80\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc12", BRACK(FH_SDCC | 12), "\\Device\\Harddisk80\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc13", BRACK(FH_SDCC | 13), "\\Device\\Harddisk80\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc14", BRACK(FH_SDCC | 14), "\\Device\\Harddisk80\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcc15", BRACK(FH_SDCC | 15), "\\Device\\Harddisk80\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd1", BRACK(FH_SDCD | 1), "\\Device\\Harddisk81\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd2", BRACK(FH_SDCD | 2), "\\Device\\Harddisk81\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd3", BRACK(FH_SDCD | 3), "\\Device\\Harddisk81\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd4", BRACK(FH_SDCD | 4), "\\Device\\Harddisk81\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd5", BRACK(FH_SDCD | 5), "\\Device\\Harddisk81\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd6", BRACK(FH_SDCD | 6), "\\Device\\Harddisk81\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd7", BRACK(FH_SDCD | 7), "\\Device\\Harddisk81\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd8", BRACK(FH_SDCD | 8), "\\Device\\Harddisk81\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd9", BRACK(FH_SDCD | 9), "\\Device\\Harddisk81\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd10", BRACK(FH_SDCD | 10), "\\Device\\Harddisk81\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd11", BRACK(FH_SDCD | 11), "\\Device\\Harddisk81\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd12", BRACK(FH_SDCD | 12), "\\Device\\Harddisk81\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd13", BRACK(FH_SDCD | 13), "\\Device\\Harddisk81\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd14", BRACK(FH_SDCD | 14), "\\Device\\Harddisk81\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcd15", BRACK(FH_SDCD | 15), "\\Device\\Harddisk81\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce1", BRACK(FH_SDCE | 1), "\\Device\\Harddisk82\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce2", BRACK(FH_SDCE | 2), "\\Device\\Harddisk82\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce3", BRACK(FH_SDCE | 3), "\\Device\\Harddisk82\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce4", BRACK(FH_SDCE | 4), "\\Device\\Harddisk82\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce5", BRACK(FH_SDCE | 5), "\\Device\\Harddisk82\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce6", BRACK(FH_SDCE | 6), "\\Device\\Harddisk82\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce7", BRACK(FH_SDCE | 7), "\\Device\\Harddisk82\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce8", BRACK(FH_SDCE | 8), "\\Device\\Harddisk82\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce9", BRACK(FH_SDCE | 9), "\\Device\\Harddisk82\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce10", BRACK(FH_SDCE | 10), "\\Device\\Harddisk82\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce11", BRACK(FH_SDCE | 11), "\\Device\\Harddisk82\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce12", BRACK(FH_SDCE | 12), "\\Device\\Harddisk82\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce13", BRACK(FH_SDCE | 13), "\\Device\\Harddisk82\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce14", BRACK(FH_SDCE | 14), "\\Device\\Harddisk82\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdce15", BRACK(FH_SDCE | 15), "\\Device\\Harddisk82\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf1", BRACK(FH_SDCF | 1), "\\Device\\Harddisk83\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf2", BRACK(FH_SDCF | 2), "\\Device\\Harddisk83\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf3", BRACK(FH_SDCF | 3), "\\Device\\Harddisk83\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf4", BRACK(FH_SDCF | 4), "\\Device\\Harddisk83\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf5", BRACK(FH_SDCF | 5), "\\Device\\Harddisk83\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf6", BRACK(FH_SDCF | 6), "\\Device\\Harddisk83\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf7", BRACK(FH_SDCF | 7), "\\Device\\Harddisk83\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf8", BRACK(FH_SDCF | 8), "\\Device\\Harddisk83\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf9", BRACK(FH_SDCF | 9), "\\Device\\Harddisk83\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf10", BRACK(FH_SDCF | 10), "\\Device\\Harddisk83\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf11", BRACK(FH_SDCF | 11), "\\Device\\Harddisk83\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf12", BRACK(FH_SDCF | 12), "\\Device\\Harddisk83\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf13", BRACK(FH_SDCF | 13), "\\Device\\Harddisk83\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf14", BRACK(FH_SDCF | 14), "\\Device\\Harddisk83\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcf15", BRACK(FH_SDCF | 15), "\\Device\\Harddisk83\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg1", BRACK(FH_SDCG | 1), "\\Device\\Harddisk84\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg2", BRACK(FH_SDCG | 2), "\\Device\\Harddisk84\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg3", BRACK(FH_SDCG | 3), "\\Device\\Harddisk84\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg4", BRACK(FH_SDCG | 4), "\\Device\\Harddisk84\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg5", BRACK(FH_SDCG | 5), "\\Device\\Harddisk84\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg6", BRACK(FH_SDCG | 6), "\\Device\\Harddisk84\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg7", BRACK(FH_SDCG | 7), "\\Device\\Harddisk84\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg8", BRACK(FH_SDCG | 8), "\\Device\\Harddisk84\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg9", BRACK(FH_SDCG | 9), "\\Device\\Harddisk84\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg10", BRACK(FH_SDCG | 10), "\\Device\\Harddisk84\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg11", BRACK(FH_SDCG | 11), "\\Device\\Harddisk84\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg12", BRACK(FH_SDCG | 12), "\\Device\\Harddisk84\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg13", BRACK(FH_SDCG | 13), "\\Device\\Harddisk84\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg14", BRACK(FH_SDCG | 14), "\\Device\\Harddisk84\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcg15", BRACK(FH_SDCG | 15), "\\Device\\Harddisk84\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch1", BRACK(FH_SDCH | 1), "\\Device\\Harddisk85\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch2", BRACK(FH_SDCH | 2), "\\Device\\Harddisk85\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch3", BRACK(FH_SDCH | 3), "\\Device\\Harddisk85\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch4", BRACK(FH_SDCH | 4), "\\Device\\Harddisk85\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch5", BRACK(FH_SDCH | 5), "\\Device\\Harddisk85\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch6", BRACK(FH_SDCH | 6), "\\Device\\Harddisk85\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch7", BRACK(FH_SDCH | 7), "\\Device\\Harddisk85\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch8", BRACK(FH_SDCH | 8), "\\Device\\Harddisk85\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch9", BRACK(FH_SDCH | 9), "\\Device\\Harddisk85\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch10", BRACK(FH_SDCH | 10), "\\Device\\Harddisk85\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch11", BRACK(FH_SDCH | 11), "\\Device\\Harddisk85\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch12", BRACK(FH_SDCH | 12), "\\Device\\Harddisk85\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch13", BRACK(FH_SDCH | 13), "\\Device\\Harddisk85\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch14", BRACK(FH_SDCH | 14), "\\Device\\Harddisk85\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdch15", BRACK(FH_SDCH | 15), "\\Device\\Harddisk85\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci1", BRACK(FH_SDCI | 1), "\\Device\\Harddisk86\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci2", BRACK(FH_SDCI | 2), "\\Device\\Harddisk86\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci3", BRACK(FH_SDCI | 3), "\\Device\\Harddisk86\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci4", BRACK(FH_SDCI | 4), "\\Device\\Harddisk86\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci5", BRACK(FH_SDCI | 5), "\\Device\\Harddisk86\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci6", BRACK(FH_SDCI | 6), "\\Device\\Harddisk86\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci7", BRACK(FH_SDCI | 7), "\\Device\\Harddisk86\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci8", BRACK(FH_SDCI | 8), "\\Device\\Harddisk86\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci9", BRACK(FH_SDCI | 9), "\\Device\\Harddisk86\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci10", BRACK(FH_SDCI | 10), "\\Device\\Harddisk86\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci11", BRACK(FH_SDCI | 11), "\\Device\\Harddisk86\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci12", BRACK(FH_SDCI | 12), "\\Device\\Harddisk86\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci13", BRACK(FH_SDCI | 13), "\\Device\\Harddisk86\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci14", BRACK(FH_SDCI | 14), "\\Device\\Harddisk86\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdci15", BRACK(FH_SDCI | 15), "\\Device\\Harddisk86\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj1", BRACK(FH_SDCJ | 1), "\\Device\\Harddisk87\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj2", BRACK(FH_SDCJ | 2), "\\Device\\Harddisk87\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj3", BRACK(FH_SDCJ | 3), "\\Device\\Harddisk87\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj4", BRACK(FH_SDCJ | 4), "\\Device\\Harddisk87\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj5", BRACK(FH_SDCJ | 5), "\\Device\\Harddisk87\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj6", BRACK(FH_SDCJ | 6), "\\Device\\Harddisk87\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj7", BRACK(FH_SDCJ | 7), "\\Device\\Harddisk87\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj8", BRACK(FH_SDCJ | 8), "\\Device\\Harddisk87\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj9", BRACK(FH_SDCJ | 9), "\\Device\\Harddisk87\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj10", BRACK(FH_SDCJ | 10), "\\Device\\Harddisk87\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj11", BRACK(FH_SDCJ | 11), "\\Device\\Harddisk87\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj12", BRACK(FH_SDCJ | 12), "\\Device\\Harddisk87\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj13", BRACK(FH_SDCJ | 13), "\\Device\\Harddisk87\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj14", BRACK(FH_SDCJ | 14), "\\Device\\Harddisk87\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcj15", BRACK(FH_SDCJ | 15), "\\Device\\Harddisk87\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck1", BRACK(FH_SDCK | 1), "\\Device\\Harddisk88\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck2", BRACK(FH_SDCK | 2), "\\Device\\Harddisk88\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck3", BRACK(FH_SDCK | 3), "\\Device\\Harddisk88\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck4", BRACK(FH_SDCK | 4), "\\Device\\Harddisk88\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck5", BRACK(FH_SDCK | 5), "\\Device\\Harddisk88\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck6", BRACK(FH_SDCK | 6), "\\Device\\Harddisk88\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck7", BRACK(FH_SDCK | 7), "\\Device\\Harddisk88\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck8", BRACK(FH_SDCK | 8), "\\Device\\Harddisk88\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck9", BRACK(FH_SDCK | 9), "\\Device\\Harddisk88\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck10", BRACK(FH_SDCK | 10), "\\Device\\Harddisk88\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck11", BRACK(FH_SDCK | 11), "\\Device\\Harddisk88\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck12", BRACK(FH_SDCK | 12), "\\Device\\Harddisk88\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck13", BRACK(FH_SDCK | 13), "\\Device\\Harddisk88\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck14", BRACK(FH_SDCK | 14), "\\Device\\Harddisk88\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdck15", BRACK(FH_SDCK | 15), "\\Device\\Harddisk88\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl1", BRACK(FH_SDCL | 1), "\\Device\\Harddisk89\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl2", BRACK(FH_SDCL | 2), "\\Device\\Harddisk89\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl3", BRACK(FH_SDCL | 3), "\\Device\\Harddisk89\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl4", BRACK(FH_SDCL | 4), "\\Device\\Harddisk89\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl5", BRACK(FH_SDCL | 5), "\\Device\\Harddisk89\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl6", BRACK(FH_SDCL | 6), "\\Device\\Harddisk89\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl7", BRACK(FH_SDCL | 7), "\\Device\\Harddisk89\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl8", BRACK(FH_SDCL | 8), "\\Device\\Harddisk89\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl9", BRACK(FH_SDCL | 9), "\\Device\\Harddisk89\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl10", BRACK(FH_SDCL | 10), "\\Device\\Harddisk89\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl11", BRACK(FH_SDCL | 11), "\\Device\\Harddisk89\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl12", BRACK(FH_SDCL | 12), "\\Device\\Harddisk89\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl13", BRACK(FH_SDCL | 13), "\\Device\\Harddisk89\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl14", BRACK(FH_SDCL | 14), "\\Device\\Harddisk89\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcl15", BRACK(FH_SDCL | 15), "\\Device\\Harddisk89\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm1", BRACK(FH_SDCM | 1), "\\Device\\Harddisk90\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm2", BRACK(FH_SDCM | 2), "\\Device\\Harddisk90\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm3", BRACK(FH_SDCM | 3), "\\Device\\Harddisk90\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm4", BRACK(FH_SDCM | 4), "\\Device\\Harddisk90\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm5", BRACK(FH_SDCM | 5), "\\Device\\Harddisk90\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm6", BRACK(FH_SDCM | 6), "\\Device\\Harddisk90\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm7", BRACK(FH_SDCM | 7), "\\Device\\Harddisk90\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm8", BRACK(FH_SDCM | 8), "\\Device\\Harddisk90\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm9", BRACK(FH_SDCM | 9), "\\Device\\Harddisk90\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm10", BRACK(FH_SDCM | 10), "\\Device\\Harddisk90\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm11", BRACK(FH_SDCM | 11), "\\Device\\Harddisk90\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm12", BRACK(FH_SDCM | 12), "\\Device\\Harddisk90\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm13", BRACK(FH_SDCM | 13), "\\Device\\Harddisk90\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm14", BRACK(FH_SDCM | 14), "\\Device\\Harddisk90\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcm15", BRACK(FH_SDCM | 15), "\\Device\\Harddisk90\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn1", BRACK(FH_SDCN | 1), "\\Device\\Harddisk91\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn2", BRACK(FH_SDCN | 2), "\\Device\\Harddisk91\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn3", BRACK(FH_SDCN | 3), "\\Device\\Harddisk91\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn4", BRACK(FH_SDCN | 4), "\\Device\\Harddisk91\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn5", BRACK(FH_SDCN | 5), "\\Device\\Harddisk91\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn6", BRACK(FH_SDCN | 6), "\\Device\\Harddisk91\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn7", BRACK(FH_SDCN | 7), "\\Device\\Harddisk91\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn8", BRACK(FH_SDCN | 8), "\\Device\\Harddisk91\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn9", BRACK(FH_SDCN | 9), "\\Device\\Harddisk91\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn10", BRACK(FH_SDCN | 10), "\\Device\\Harddisk91\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn11", BRACK(FH_SDCN | 11), "\\Device\\Harddisk91\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn12", BRACK(FH_SDCN | 12), "\\Device\\Harddisk91\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn13", BRACK(FH_SDCN | 13), "\\Device\\Harddisk91\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn14", BRACK(FH_SDCN | 14), "\\Device\\Harddisk91\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcn15", BRACK(FH_SDCN | 15), "\\Device\\Harddisk91\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco1", BRACK(FH_SDCO | 1), "\\Device\\Harddisk92\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco2", BRACK(FH_SDCO | 2), "\\Device\\Harddisk92\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco3", BRACK(FH_SDCO | 3), "\\Device\\Harddisk92\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco4", BRACK(FH_SDCO | 4), "\\Device\\Harddisk92\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco5", BRACK(FH_SDCO | 5), "\\Device\\Harddisk92\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco6", BRACK(FH_SDCO | 6), "\\Device\\Harddisk92\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco7", BRACK(FH_SDCO | 7), "\\Device\\Harddisk92\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco8", BRACK(FH_SDCO | 8), "\\Device\\Harddisk92\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco9", BRACK(FH_SDCO | 9), "\\Device\\Harddisk92\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco10", BRACK(FH_SDCO | 10), "\\Device\\Harddisk92\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco11", BRACK(FH_SDCO | 11), "\\Device\\Harddisk92\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco12", BRACK(FH_SDCO | 12), "\\Device\\Harddisk92\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco13", BRACK(FH_SDCO | 13), "\\Device\\Harddisk92\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco14", BRACK(FH_SDCO | 14), "\\Device\\Harddisk92\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdco15", BRACK(FH_SDCO | 15), "\\Device\\Harddisk92\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp1", BRACK(FH_SDCP | 1), "\\Device\\Harddisk93\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp2", BRACK(FH_SDCP | 2), "\\Device\\Harddisk93\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp3", BRACK(FH_SDCP | 3), "\\Device\\Harddisk93\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp4", BRACK(FH_SDCP | 4), "\\Device\\Harddisk93\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp5", BRACK(FH_SDCP | 5), "\\Device\\Harddisk93\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp6", BRACK(FH_SDCP | 6), "\\Device\\Harddisk93\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp7", BRACK(FH_SDCP | 7), "\\Device\\Harddisk93\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp8", BRACK(FH_SDCP | 8), "\\Device\\Harddisk93\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp9", BRACK(FH_SDCP | 9), "\\Device\\Harddisk93\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp10", BRACK(FH_SDCP | 10), "\\Device\\Harddisk93\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp11", BRACK(FH_SDCP | 11), "\\Device\\Harddisk93\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp12", BRACK(FH_SDCP | 12), "\\Device\\Harddisk93\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp13", BRACK(FH_SDCP | 13), "\\Device\\Harddisk93\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp14", BRACK(FH_SDCP | 14), "\\Device\\Harddisk93\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcp15", BRACK(FH_SDCP | 15), "\\Device\\Harddisk93\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq1", BRACK(FH_SDCQ | 1), "\\Device\\Harddisk94\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq2", BRACK(FH_SDCQ | 2), "\\Device\\Harddisk94\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq3", BRACK(FH_SDCQ | 3), "\\Device\\Harddisk94\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq4", BRACK(FH_SDCQ | 4), "\\Device\\Harddisk94\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq5", BRACK(FH_SDCQ | 5), "\\Device\\Harddisk94\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq6", BRACK(FH_SDCQ | 6), "\\Device\\Harddisk94\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq7", BRACK(FH_SDCQ | 7), "\\Device\\Harddisk94\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq8", BRACK(FH_SDCQ | 8), "\\Device\\Harddisk94\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq9", BRACK(FH_SDCQ | 9), "\\Device\\Harddisk94\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq10", BRACK(FH_SDCQ | 10), "\\Device\\Harddisk94\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq11", BRACK(FH_SDCQ | 11), "\\Device\\Harddisk94\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq12", BRACK(FH_SDCQ | 12), "\\Device\\Harddisk94\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq13", BRACK(FH_SDCQ | 13), "\\Device\\Harddisk94\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq14", BRACK(FH_SDCQ | 14), "\\Device\\Harddisk94\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcq15", BRACK(FH_SDCQ | 15), "\\Device\\Harddisk94\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr1", BRACK(FH_SDCR | 1), "\\Device\\Harddisk95\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr2", BRACK(FH_SDCR | 2), "\\Device\\Harddisk95\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr3", BRACK(FH_SDCR | 3), "\\Device\\Harddisk95\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr4", BRACK(FH_SDCR | 4), "\\Device\\Harddisk95\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr5", BRACK(FH_SDCR | 5), "\\Device\\Harddisk95\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr6", BRACK(FH_SDCR | 6), "\\Device\\Harddisk95\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr7", BRACK(FH_SDCR | 7), "\\Device\\Harddisk95\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr8", BRACK(FH_SDCR | 8), "\\Device\\Harddisk95\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr9", BRACK(FH_SDCR | 9), "\\Device\\Harddisk95\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr10", BRACK(FH_SDCR | 10), "\\Device\\Harddisk95\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr11", BRACK(FH_SDCR | 11), "\\Device\\Harddisk95\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr12", BRACK(FH_SDCR | 12), "\\Device\\Harddisk95\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr13", BRACK(FH_SDCR | 13), "\\Device\\Harddisk95\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr14", BRACK(FH_SDCR | 14), "\\Device\\Harddisk95\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcr15", BRACK(FH_SDCR | 15), "\\Device\\Harddisk95\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs1", BRACK(FH_SDCS | 1), "\\Device\\Harddisk96\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs2", BRACK(FH_SDCS | 2), "\\Device\\Harddisk96\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs3", BRACK(FH_SDCS | 3), "\\Device\\Harddisk96\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs4", BRACK(FH_SDCS | 4), "\\Device\\Harddisk96\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs5", BRACK(FH_SDCS | 5), "\\Device\\Harddisk96\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs6", BRACK(FH_SDCS | 6), "\\Device\\Harddisk96\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs7", BRACK(FH_SDCS | 7), "\\Device\\Harddisk96\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs8", BRACK(FH_SDCS | 8), "\\Device\\Harddisk96\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs9", BRACK(FH_SDCS | 9), "\\Device\\Harddisk96\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs10", BRACK(FH_SDCS | 10), "\\Device\\Harddisk96\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs11", BRACK(FH_SDCS | 11), "\\Device\\Harddisk96\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs12", BRACK(FH_SDCS | 12), "\\Device\\Harddisk96\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs13", BRACK(FH_SDCS | 13), "\\Device\\Harddisk96\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs14", BRACK(FH_SDCS | 14), "\\Device\\Harddisk96\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcs15", BRACK(FH_SDCS | 15), "\\Device\\Harddisk96\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct1", BRACK(FH_SDCT | 1), "\\Device\\Harddisk97\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct2", BRACK(FH_SDCT | 2), "\\Device\\Harddisk97\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct3", BRACK(FH_SDCT | 3), "\\Device\\Harddisk97\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct4", BRACK(FH_SDCT | 4), "\\Device\\Harddisk97\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct5", BRACK(FH_SDCT | 5), "\\Device\\Harddisk97\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct6", BRACK(FH_SDCT | 6), "\\Device\\Harddisk97\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct7", BRACK(FH_SDCT | 7), "\\Device\\Harddisk97\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct8", BRACK(FH_SDCT | 8), "\\Device\\Harddisk97\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct9", BRACK(FH_SDCT | 9), "\\Device\\Harddisk97\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct10", BRACK(FH_SDCT | 10), "\\Device\\Harddisk97\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct11", BRACK(FH_SDCT | 11), "\\Device\\Harddisk97\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct12", BRACK(FH_SDCT | 12), "\\Device\\Harddisk97\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct13", BRACK(FH_SDCT | 13), "\\Device\\Harddisk97\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct14", BRACK(FH_SDCT | 14), "\\Device\\Harddisk97\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdct15", BRACK(FH_SDCT | 15), "\\Device\\Harddisk97\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu1", BRACK(FH_SDCU | 1), "\\Device\\Harddisk98\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu2", BRACK(FH_SDCU | 2), "\\Device\\Harddisk98\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu3", BRACK(FH_SDCU | 3), "\\Device\\Harddisk98\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu4", BRACK(FH_SDCU | 4), "\\Device\\Harddisk98\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu5", BRACK(FH_SDCU | 5), "\\Device\\Harddisk98\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu6", BRACK(FH_SDCU | 6), "\\Device\\Harddisk98\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu7", BRACK(FH_SDCU | 7), "\\Device\\Harddisk98\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu8", BRACK(FH_SDCU | 8), "\\Device\\Harddisk98\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu9", BRACK(FH_SDCU | 9), "\\Device\\Harddisk98\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu10", BRACK(FH_SDCU | 10), "\\Device\\Harddisk98\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu11", BRACK(FH_SDCU | 11), "\\Device\\Harddisk98\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu12", BRACK(FH_SDCU | 12), "\\Device\\Harddisk98\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu13", BRACK(FH_SDCU | 13), "\\Device\\Harddisk98\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu14", BRACK(FH_SDCU | 14), "\\Device\\Harddisk98\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcu15", BRACK(FH_SDCU | 15), "\\Device\\Harddisk98\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv1", BRACK(FH_SDCV | 1), "\\Device\\Harddisk99\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv2", BRACK(FH_SDCV | 2), "\\Device\\Harddisk99\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv3", BRACK(FH_SDCV | 3), "\\Device\\Harddisk99\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv4", BRACK(FH_SDCV | 4), "\\Device\\Harddisk99\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv5", BRACK(FH_SDCV | 5), "\\Device\\Harddisk99\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv6", BRACK(FH_SDCV | 6), "\\Device\\Harddisk99\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv7", BRACK(FH_SDCV | 7), "\\Device\\Harddisk99\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv8", BRACK(FH_SDCV | 8), "\\Device\\Harddisk99\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv9", BRACK(FH_SDCV | 9), "\\Device\\Harddisk99\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv10", BRACK(FH_SDCV | 10), "\\Device\\Harddisk99\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv11", BRACK(FH_SDCV | 11), "\\Device\\Harddisk99\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv12", BRACK(FH_SDCV | 12), "\\Device\\Harddisk99\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv13", BRACK(FH_SDCV | 13), "\\Device\\Harddisk99\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv14", BRACK(FH_SDCV | 14), "\\Device\\Harddisk99\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcv15", BRACK(FH_SDCV | 15), "\\Device\\Harddisk99\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw1", BRACK(FH_SDCW | 1), "\\Device\\Harddisk100\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw2", BRACK(FH_SDCW | 2), "\\Device\\Harddisk100\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw3", BRACK(FH_SDCW | 3), "\\Device\\Harddisk100\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw4", BRACK(FH_SDCW | 4), "\\Device\\Harddisk100\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw5", BRACK(FH_SDCW | 5), "\\Device\\Harddisk100\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw6", BRACK(FH_SDCW | 6), "\\Device\\Harddisk100\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw7", BRACK(FH_SDCW | 7), "\\Device\\Harddisk100\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw8", BRACK(FH_SDCW | 8), "\\Device\\Harddisk100\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw9", BRACK(FH_SDCW | 9), "\\Device\\Harddisk100\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw10", BRACK(FH_SDCW | 10), "\\Device\\Harddisk100\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw11", BRACK(FH_SDCW | 11), "\\Device\\Harddisk100\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw12", BRACK(FH_SDCW | 12), "\\Device\\Harddisk100\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw13", BRACK(FH_SDCW | 13), "\\Device\\Harddisk100\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw14", BRACK(FH_SDCW | 14), "\\Device\\Harddisk100\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcw15", BRACK(FH_SDCW | 15), "\\Device\\Harddisk100\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx1", BRACK(FH_SDCX | 1), "\\Device\\Harddisk101\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx2", BRACK(FH_SDCX | 2), "\\Device\\Harddisk101\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx3", BRACK(FH_SDCX | 3), "\\Device\\Harddisk101\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx4", BRACK(FH_SDCX | 4), "\\Device\\Harddisk101\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx5", BRACK(FH_SDCX | 5), "\\Device\\Harddisk101\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx6", BRACK(FH_SDCX | 6), "\\Device\\Harddisk101\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx7", BRACK(FH_SDCX | 7), "\\Device\\Harddisk101\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx8", BRACK(FH_SDCX | 8), "\\Device\\Harddisk101\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx9", BRACK(FH_SDCX | 9), "\\Device\\Harddisk101\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx10", BRACK(FH_SDCX | 10), "\\Device\\Harddisk101\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx11", BRACK(FH_SDCX | 11), "\\Device\\Harddisk101\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx12", BRACK(FH_SDCX | 12), "\\Device\\Harddisk101\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx13", BRACK(FH_SDCX | 13), "\\Device\\Harddisk101\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx14", BRACK(FH_SDCX | 14), "\\Device\\Harddisk101\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcx15", BRACK(FH_SDCX | 15), "\\Device\\Harddisk101\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy1", BRACK(FH_SDCY | 1), "\\Device\\Harddisk102\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy2", BRACK(FH_SDCY | 2), "\\Device\\Harddisk102\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy3", BRACK(FH_SDCY | 3), "\\Device\\Harddisk102\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy4", BRACK(FH_SDCY | 4), "\\Device\\Harddisk102\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy5", BRACK(FH_SDCY | 5), "\\Device\\Harddisk102\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy6", BRACK(FH_SDCY | 6), "\\Device\\Harddisk102\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy7", BRACK(FH_SDCY | 7), "\\Device\\Harddisk102\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy8", BRACK(FH_SDCY | 8), "\\Device\\Harddisk102\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy9", BRACK(FH_SDCY | 9), "\\Device\\Harddisk102\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy10", BRACK(FH_SDCY | 10), "\\Device\\Harddisk102\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy11", BRACK(FH_SDCY | 11), "\\Device\\Harddisk102\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy12", BRACK(FH_SDCY | 12), "\\Device\\Harddisk102\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy13", BRACK(FH_SDCY | 13), "\\Device\\Harddisk102\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy14", BRACK(FH_SDCY | 14), "\\Device\\Harddisk102\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcy15", BRACK(FH_SDCY | 15), "\\Device\\Harddisk102\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz1", BRACK(FH_SDCZ | 1), "\\Device\\Harddisk103\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz2", BRACK(FH_SDCZ | 2), "\\Device\\Harddisk103\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz3", BRACK(FH_SDCZ | 3), "\\Device\\Harddisk103\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz4", BRACK(FH_SDCZ | 4), "\\Device\\Harddisk103\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz5", BRACK(FH_SDCZ | 5), "\\Device\\Harddisk103\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz6", BRACK(FH_SDCZ | 6), "\\Device\\Harddisk103\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz7", BRACK(FH_SDCZ | 7), "\\Device\\Harddisk103\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz8", BRACK(FH_SDCZ | 8), "\\Device\\Harddisk103\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz9", BRACK(FH_SDCZ | 9), "\\Device\\Harddisk103\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz10", BRACK(FH_SDCZ | 10), "\\Device\\Harddisk103\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz11", BRACK(FH_SDCZ | 11), "\\Device\\Harddisk103\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz12", BRACK(FH_SDCZ | 12), "\\Device\\Harddisk103\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz13", BRACK(FH_SDCZ | 13), "\\Device\\Harddisk103\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz14", BRACK(FH_SDCZ | 14), "\\Device\\Harddisk103\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdcz15", BRACK(FH_SDCZ | 15), "\\Device\\Harddisk103\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda", BRACK(FH_SDDA), "\\Device\\Harddisk104\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb", BRACK(FH_SDDB), "\\Device\\Harddisk105\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc", BRACK(FH_SDDC), "\\Device\\Harddisk106\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd", BRACK(FH_SDDD), "\\Device\\Harddisk107\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde", BRACK(FH_SDDE), "\\Device\\Harddisk108\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf", BRACK(FH_SDDF), "\\Device\\Harddisk109\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg", BRACK(FH_SDDG), "\\Device\\Harddisk110\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh", BRACK(FH_SDDH), "\\Device\\Harddisk111\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi", BRACK(FH_SDDI), "\\Device\\Harddisk112\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj", BRACK(FH_SDDJ), "\\Device\\Harddisk113\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk", BRACK(FH_SDDK), "\\Device\\Harddisk114\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl", BRACK(FH_SDDL), "\\Device\\Harddisk115\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm", BRACK(FH_SDDM), "\\Device\\Harddisk116\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn", BRACK(FH_SDDN), "\\Device\\Harddisk117\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo", BRACK(FH_SDDO), "\\Device\\Harddisk118\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp", BRACK(FH_SDDP), "\\Device\\Harddisk119\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq", BRACK(FH_SDDQ), "\\Device\\Harddisk120\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr", BRACK(FH_SDDR), "\\Device\\Harddisk121\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds", BRACK(FH_SDDS), "\\Device\\Harddisk122\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt", BRACK(FH_SDDT), "\\Device\\Harddisk123\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu", BRACK(FH_SDDU), "\\Device\\Harddisk124\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv", BRACK(FH_SDDV), "\\Device\\Harddisk125\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw", BRACK(FH_SDDW), "\\Device\\Harddisk126\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx", BRACK(FH_SDDX), "\\Device\\Harddisk127\\Partition0", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda1", BRACK(FH_SDDA | 1), "\\Device\\Harddisk104\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda2", BRACK(FH_SDDA | 2), "\\Device\\Harddisk104\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda3", BRACK(FH_SDDA | 3), "\\Device\\Harddisk104\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda4", BRACK(FH_SDDA | 4), "\\Device\\Harddisk104\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda5", BRACK(FH_SDDA | 5), "\\Device\\Harddisk104\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda6", BRACK(FH_SDDA | 6), "\\Device\\Harddisk104\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda7", BRACK(FH_SDDA | 7), "\\Device\\Harddisk104\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda8", BRACK(FH_SDDA | 8), "\\Device\\Harddisk104\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda9", BRACK(FH_SDDA | 9), "\\Device\\Harddisk104\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda10", BRACK(FH_SDDA | 10), "\\Device\\Harddisk104\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda11", BRACK(FH_SDDA | 11), "\\Device\\Harddisk104\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda12", BRACK(FH_SDDA | 12), "\\Device\\Harddisk104\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda13", BRACK(FH_SDDA | 13), "\\Device\\Harddisk104\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda14", BRACK(FH_SDDA | 14), "\\Device\\Harddisk104\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdda15", BRACK(FH_SDDA | 15), "\\Device\\Harddisk104\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb1", BRACK(FH_SDDB | 1), "\\Device\\Harddisk105\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb2", BRACK(FH_SDDB | 2), "\\Device\\Harddisk105\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb3", BRACK(FH_SDDB | 3), "\\Device\\Harddisk105\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb4", BRACK(FH_SDDB | 4), "\\Device\\Harddisk105\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb5", BRACK(FH_SDDB | 5), "\\Device\\Harddisk105\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb6", BRACK(FH_SDDB | 6), "\\Device\\Harddisk105\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb7", BRACK(FH_SDDB | 7), "\\Device\\Harddisk105\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb8", BRACK(FH_SDDB | 8), "\\Device\\Harddisk105\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb9", BRACK(FH_SDDB | 9), "\\Device\\Harddisk105\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb10", BRACK(FH_SDDB | 10), "\\Device\\Harddisk105\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb11", BRACK(FH_SDDB | 11), "\\Device\\Harddisk105\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb12", BRACK(FH_SDDB | 12), "\\Device\\Harddisk105\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb13", BRACK(FH_SDDB | 13), "\\Device\\Harddisk105\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb14", BRACK(FH_SDDB | 14), "\\Device\\Harddisk105\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddb15", BRACK(FH_SDDB | 15), "\\Device\\Harddisk105\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc1", BRACK(FH_SDDC | 1), "\\Device\\Harddisk106\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc2", BRACK(FH_SDDC | 2), "\\Device\\Harddisk106\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc3", BRACK(FH_SDDC | 3), "\\Device\\Harddisk106\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc4", BRACK(FH_SDDC | 4), "\\Device\\Harddisk106\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc5", BRACK(FH_SDDC | 5), "\\Device\\Harddisk106\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc6", BRACK(FH_SDDC | 6), "\\Device\\Harddisk106\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc7", BRACK(FH_SDDC | 7), "\\Device\\Harddisk106\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc8", BRACK(FH_SDDC | 8), "\\Device\\Harddisk106\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc9", BRACK(FH_SDDC | 9), "\\Device\\Harddisk106\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc10", BRACK(FH_SDDC | 10), "\\Device\\Harddisk106\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc11", BRACK(FH_SDDC | 11), "\\Device\\Harddisk106\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc12", BRACK(FH_SDDC | 12), "\\Device\\Harddisk106\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc13", BRACK(FH_SDDC | 13), "\\Device\\Harddisk106\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc14", BRACK(FH_SDDC | 14), "\\Device\\Harddisk106\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddc15", BRACK(FH_SDDC | 15), "\\Device\\Harddisk106\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd1", BRACK(FH_SDDD | 1), "\\Device\\Harddisk107\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd2", BRACK(FH_SDDD | 2), "\\Device\\Harddisk107\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd3", BRACK(FH_SDDD | 3), "\\Device\\Harddisk107\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd4", BRACK(FH_SDDD | 4), "\\Device\\Harddisk107\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd5", BRACK(FH_SDDD | 5), "\\Device\\Harddisk107\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd6", BRACK(FH_SDDD | 6), "\\Device\\Harddisk107\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd7", BRACK(FH_SDDD | 7), "\\Device\\Harddisk107\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd8", BRACK(FH_SDDD | 8), "\\Device\\Harddisk107\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd9", BRACK(FH_SDDD | 9), "\\Device\\Harddisk107\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd10", BRACK(FH_SDDD | 10), "\\Device\\Harddisk107\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd11", BRACK(FH_SDDD | 11), "\\Device\\Harddisk107\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd12", BRACK(FH_SDDD | 12), "\\Device\\Harddisk107\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd13", BRACK(FH_SDDD | 13), "\\Device\\Harddisk107\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd14", BRACK(FH_SDDD | 14), "\\Device\\Harddisk107\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddd15", BRACK(FH_SDDD | 15), "\\Device\\Harddisk107\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde1", BRACK(FH_SDDE | 1), "\\Device\\Harddisk108\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde2", BRACK(FH_SDDE | 2), "\\Device\\Harddisk108\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde3", BRACK(FH_SDDE | 3), "\\Device\\Harddisk108\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde4", BRACK(FH_SDDE | 4), "\\Device\\Harddisk108\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde5", BRACK(FH_SDDE | 5), "\\Device\\Harddisk108\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde6", BRACK(FH_SDDE | 6), "\\Device\\Harddisk108\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde7", BRACK(FH_SDDE | 7), "\\Device\\Harddisk108\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde8", BRACK(FH_SDDE | 8), "\\Device\\Harddisk108\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde9", BRACK(FH_SDDE | 9), "\\Device\\Harddisk108\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde10", BRACK(FH_SDDE | 10), "\\Device\\Harddisk108\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde11", BRACK(FH_SDDE | 11), "\\Device\\Harddisk108\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde12", BRACK(FH_SDDE | 12), "\\Device\\Harddisk108\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde13", BRACK(FH_SDDE | 13), "\\Device\\Harddisk108\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde14", BRACK(FH_SDDE | 14), "\\Device\\Harddisk108\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdde15", BRACK(FH_SDDE | 15), "\\Device\\Harddisk108\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf1", BRACK(FH_SDDF | 1), "\\Device\\Harddisk109\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf2", BRACK(FH_SDDF | 2), "\\Device\\Harddisk109\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf3", BRACK(FH_SDDF | 3), "\\Device\\Harddisk109\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf4", BRACK(FH_SDDF | 4), "\\Device\\Harddisk109\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf5", BRACK(FH_SDDF | 5), "\\Device\\Harddisk109\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf6", BRACK(FH_SDDF | 6), "\\Device\\Harddisk109\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf7", BRACK(FH_SDDF | 7), "\\Device\\Harddisk109\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf8", BRACK(FH_SDDF | 8), "\\Device\\Harddisk109\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf9", BRACK(FH_SDDF | 9), "\\Device\\Harddisk109\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf10", BRACK(FH_SDDF | 10), "\\Device\\Harddisk109\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf11", BRACK(FH_SDDF | 11), "\\Device\\Harddisk109\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf12", BRACK(FH_SDDF | 12), "\\Device\\Harddisk109\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf13", BRACK(FH_SDDF | 13), "\\Device\\Harddisk109\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf14", BRACK(FH_SDDF | 14), "\\Device\\Harddisk109\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddf15", BRACK(FH_SDDF | 15), "\\Device\\Harddisk109\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg1", BRACK(FH_SDDG | 1), "\\Device\\Harddisk110\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg2", BRACK(FH_SDDG | 2), "\\Device\\Harddisk110\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg3", BRACK(FH_SDDG | 3), "\\Device\\Harddisk110\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg4", BRACK(FH_SDDG | 4), "\\Device\\Harddisk110\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg5", BRACK(FH_SDDG | 5), "\\Device\\Harddisk110\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg6", BRACK(FH_SDDG | 6), "\\Device\\Harddisk110\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg7", BRACK(FH_SDDG | 7), "\\Device\\Harddisk110\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg8", BRACK(FH_SDDG | 8), "\\Device\\Harddisk110\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg9", BRACK(FH_SDDG | 9), "\\Device\\Harddisk110\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg10", BRACK(FH_SDDG | 10), "\\Device\\Harddisk110\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg11", BRACK(FH_SDDG | 11), "\\Device\\Harddisk110\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg12", BRACK(FH_SDDG | 12), "\\Device\\Harddisk110\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg13", BRACK(FH_SDDG | 13), "\\Device\\Harddisk110\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg14", BRACK(FH_SDDG | 14), "\\Device\\Harddisk110\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddg15", BRACK(FH_SDDG | 15), "\\Device\\Harddisk110\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh1", BRACK(FH_SDDH | 1), "\\Device\\Harddisk111\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh2", BRACK(FH_SDDH | 2), "\\Device\\Harddisk111\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh3", BRACK(FH_SDDH | 3), "\\Device\\Harddisk111\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh4", BRACK(FH_SDDH | 4), "\\Device\\Harddisk111\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh5", BRACK(FH_SDDH | 5), "\\Device\\Harddisk111\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh6", BRACK(FH_SDDH | 6), "\\Device\\Harddisk111\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh7", BRACK(FH_SDDH | 7), "\\Device\\Harddisk111\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh8", BRACK(FH_SDDH | 8), "\\Device\\Harddisk111\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh9", BRACK(FH_SDDH | 9), "\\Device\\Harddisk111\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh10", BRACK(FH_SDDH | 10), "\\Device\\Harddisk111\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh11", BRACK(FH_SDDH | 11), "\\Device\\Harddisk111\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh12", BRACK(FH_SDDH | 12), "\\Device\\Harddisk111\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh13", BRACK(FH_SDDH | 13), "\\Device\\Harddisk111\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh14", BRACK(FH_SDDH | 14), "\\Device\\Harddisk111\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddh15", BRACK(FH_SDDH | 15), "\\Device\\Harddisk111\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi1", BRACK(FH_SDDI | 1), "\\Device\\Harddisk112\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi2", BRACK(FH_SDDI | 2), "\\Device\\Harddisk112\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi3", BRACK(FH_SDDI | 3), "\\Device\\Harddisk112\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi4", BRACK(FH_SDDI | 4), "\\Device\\Harddisk112\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi5", BRACK(FH_SDDI | 5), "\\Device\\Harddisk112\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi6", BRACK(FH_SDDI | 6), "\\Device\\Harddisk112\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi7", BRACK(FH_SDDI | 7), "\\Device\\Harddisk112\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi8", BRACK(FH_SDDI | 8), "\\Device\\Harddisk112\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi9", BRACK(FH_SDDI | 9), "\\Device\\Harddisk112\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi10", BRACK(FH_SDDI | 10), "\\Device\\Harddisk112\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi11", BRACK(FH_SDDI | 11), "\\Device\\Harddisk112\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi12", BRACK(FH_SDDI | 12), "\\Device\\Harddisk112\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi13", BRACK(FH_SDDI | 13), "\\Device\\Harddisk112\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi14", BRACK(FH_SDDI | 14), "\\Device\\Harddisk112\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddi15", BRACK(FH_SDDI | 15), "\\Device\\Harddisk112\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj1", BRACK(FH_SDDJ | 1), "\\Device\\Harddisk113\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj2", BRACK(FH_SDDJ | 2), "\\Device\\Harddisk113\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj3", BRACK(FH_SDDJ | 3), "\\Device\\Harddisk113\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj4", BRACK(FH_SDDJ | 4), "\\Device\\Harddisk113\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj5", BRACK(FH_SDDJ | 5), "\\Device\\Harddisk113\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj6", BRACK(FH_SDDJ | 6), "\\Device\\Harddisk113\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj7", BRACK(FH_SDDJ | 7), "\\Device\\Harddisk113\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj8", BRACK(FH_SDDJ | 8), "\\Device\\Harddisk113\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj9", BRACK(FH_SDDJ | 9), "\\Device\\Harddisk113\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj10", BRACK(FH_SDDJ | 10), "\\Device\\Harddisk113\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj11", BRACK(FH_SDDJ | 11), "\\Device\\Harddisk113\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj12", BRACK(FH_SDDJ | 12), "\\Device\\Harddisk113\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj13", BRACK(FH_SDDJ | 13), "\\Device\\Harddisk113\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj14", BRACK(FH_SDDJ | 14), "\\Device\\Harddisk113\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddj15", BRACK(FH_SDDJ | 15), "\\Device\\Harddisk113\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk1", BRACK(FH_SDDK | 1), "\\Device\\Harddisk114\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk2", BRACK(FH_SDDK | 2), "\\Device\\Harddisk114\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk3", BRACK(FH_SDDK | 3), "\\Device\\Harddisk114\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk4", BRACK(FH_SDDK | 4), "\\Device\\Harddisk114\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk5", BRACK(FH_SDDK | 5), "\\Device\\Harddisk114\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk6", BRACK(FH_SDDK | 6), "\\Device\\Harddisk114\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk7", BRACK(FH_SDDK | 7), "\\Device\\Harddisk114\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk8", BRACK(FH_SDDK | 8), "\\Device\\Harddisk114\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk9", BRACK(FH_SDDK | 9), "\\Device\\Harddisk114\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk10", BRACK(FH_SDDK | 10), "\\Device\\Harddisk114\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk11", BRACK(FH_SDDK | 11), "\\Device\\Harddisk114\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk12", BRACK(FH_SDDK | 12), "\\Device\\Harddisk114\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk13", BRACK(FH_SDDK | 13), "\\Device\\Harddisk114\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk14", BRACK(FH_SDDK | 14), "\\Device\\Harddisk114\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddk15", BRACK(FH_SDDK | 15), "\\Device\\Harddisk114\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl1", BRACK(FH_SDDL | 1), "\\Device\\Harddisk115\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl2", BRACK(FH_SDDL | 2), "\\Device\\Harddisk115\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl3", BRACK(FH_SDDL | 3), "\\Device\\Harddisk115\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl4", BRACK(FH_SDDL | 4), "\\Device\\Harddisk115\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl5", BRACK(FH_SDDL | 5), "\\Device\\Harddisk115\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl6", BRACK(FH_SDDL | 6), "\\Device\\Harddisk115\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl7", BRACK(FH_SDDL | 7), "\\Device\\Harddisk115\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl8", BRACK(FH_SDDL | 8), "\\Device\\Harddisk115\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl9", BRACK(FH_SDDL | 9), "\\Device\\Harddisk115\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl10", BRACK(FH_SDDL | 10), "\\Device\\Harddisk115\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl11", BRACK(FH_SDDL | 11), "\\Device\\Harddisk115\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl12", BRACK(FH_SDDL | 12), "\\Device\\Harddisk115\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl13", BRACK(FH_SDDL | 13), "\\Device\\Harddisk115\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl14", BRACK(FH_SDDL | 14), "\\Device\\Harddisk115\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddl15", BRACK(FH_SDDL | 15), "\\Device\\Harddisk115\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm1", BRACK(FH_SDDM | 1), "\\Device\\Harddisk116\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm2", BRACK(FH_SDDM | 2), "\\Device\\Harddisk116\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm3", BRACK(FH_SDDM | 3), "\\Device\\Harddisk116\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm4", BRACK(FH_SDDM | 4), "\\Device\\Harddisk116\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm5", BRACK(FH_SDDM | 5), "\\Device\\Harddisk116\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm6", BRACK(FH_SDDM | 6), "\\Device\\Harddisk116\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm7", BRACK(FH_SDDM | 7), "\\Device\\Harddisk116\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm8", BRACK(FH_SDDM | 8), "\\Device\\Harddisk116\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm9", BRACK(FH_SDDM | 9), "\\Device\\Harddisk116\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm10", BRACK(FH_SDDM | 10), "\\Device\\Harddisk116\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm11", BRACK(FH_SDDM | 11), "\\Device\\Harddisk116\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm12", BRACK(FH_SDDM | 12), "\\Device\\Harddisk116\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm13", BRACK(FH_SDDM | 13), "\\Device\\Harddisk116\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm14", BRACK(FH_SDDM | 14), "\\Device\\Harddisk116\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddm15", BRACK(FH_SDDM | 15), "\\Device\\Harddisk116\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn1", BRACK(FH_SDDN | 1), "\\Device\\Harddisk117\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn2", BRACK(FH_SDDN | 2), "\\Device\\Harddisk117\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn3", BRACK(FH_SDDN | 3), "\\Device\\Harddisk117\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn4", BRACK(FH_SDDN | 4), "\\Device\\Harddisk117\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn5", BRACK(FH_SDDN | 5), "\\Device\\Harddisk117\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn6", BRACK(FH_SDDN | 6), "\\Device\\Harddisk117\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn7", BRACK(FH_SDDN | 7), "\\Device\\Harddisk117\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn8", BRACK(FH_SDDN | 8), "\\Device\\Harddisk117\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn9", BRACK(FH_SDDN | 9), "\\Device\\Harddisk117\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn10", BRACK(FH_SDDN | 10), "\\Device\\Harddisk117\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn11", BRACK(FH_SDDN | 11), "\\Device\\Harddisk117\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn12", BRACK(FH_SDDN | 12), "\\Device\\Harddisk117\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn13", BRACK(FH_SDDN | 13), "\\Device\\Harddisk117\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn14", BRACK(FH_SDDN | 14), "\\Device\\Harddisk117\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddn15", BRACK(FH_SDDN | 15), "\\Device\\Harddisk117\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo1", BRACK(FH_SDDO | 1), "\\Device\\Harddisk118\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo2", BRACK(FH_SDDO | 2), "\\Device\\Harddisk118\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo3", BRACK(FH_SDDO | 3), "\\Device\\Harddisk118\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo4", BRACK(FH_SDDO | 4), "\\Device\\Harddisk118\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo5", BRACK(FH_SDDO | 5), "\\Device\\Harddisk118\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo6", BRACK(FH_SDDO | 6), "\\Device\\Harddisk118\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo7", BRACK(FH_SDDO | 7), "\\Device\\Harddisk118\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo8", BRACK(FH_SDDO | 8), "\\Device\\Harddisk118\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo9", BRACK(FH_SDDO | 9), "\\Device\\Harddisk118\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo10", BRACK(FH_SDDO | 10), "\\Device\\Harddisk118\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo11", BRACK(FH_SDDO | 11), "\\Device\\Harddisk118\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo12", BRACK(FH_SDDO | 12), "\\Device\\Harddisk118\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo13", BRACK(FH_SDDO | 13), "\\Device\\Harddisk118\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo14", BRACK(FH_SDDO | 14), "\\Device\\Harddisk118\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddo15", BRACK(FH_SDDO | 15), "\\Device\\Harddisk118\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp1", BRACK(FH_SDDP | 1), "\\Device\\Harddisk119\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp2", BRACK(FH_SDDP | 2), "\\Device\\Harddisk119\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp3", BRACK(FH_SDDP | 3), "\\Device\\Harddisk119\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp4", BRACK(FH_SDDP | 4), "\\Device\\Harddisk119\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp5", BRACK(FH_SDDP | 5), "\\Device\\Harddisk119\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp6", BRACK(FH_SDDP | 6), "\\Device\\Harddisk119\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp7", BRACK(FH_SDDP | 7), "\\Device\\Harddisk119\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp8", BRACK(FH_SDDP | 8), "\\Device\\Harddisk119\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp9", BRACK(FH_SDDP | 9), "\\Device\\Harddisk119\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp10", BRACK(FH_SDDP | 10), "\\Device\\Harddisk119\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp11", BRACK(FH_SDDP | 11), "\\Device\\Harddisk119\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp12", BRACK(FH_SDDP | 12), "\\Device\\Harddisk119\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp13", BRACK(FH_SDDP | 13), "\\Device\\Harddisk119\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp14", BRACK(FH_SDDP | 14), "\\Device\\Harddisk119\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddp15", BRACK(FH_SDDP | 15), "\\Device\\Harddisk119\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq1", BRACK(FH_SDDQ | 1), "\\Device\\Harddisk120\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq2", BRACK(FH_SDDQ | 2), "\\Device\\Harddisk120\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq3", BRACK(FH_SDDQ | 3), "\\Device\\Harddisk120\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq4", BRACK(FH_SDDQ | 4), "\\Device\\Harddisk120\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq5", BRACK(FH_SDDQ | 5), "\\Device\\Harddisk120\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq6", BRACK(FH_SDDQ | 6), "\\Device\\Harddisk120\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq7", BRACK(FH_SDDQ | 7), "\\Device\\Harddisk120\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq8", BRACK(FH_SDDQ | 8), "\\Device\\Harddisk120\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq9", BRACK(FH_SDDQ | 9), "\\Device\\Harddisk120\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq10", BRACK(FH_SDDQ | 10), "\\Device\\Harddisk120\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq11", BRACK(FH_SDDQ | 11), "\\Device\\Harddisk120\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq12", BRACK(FH_SDDQ | 12), "\\Device\\Harddisk120\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq13", BRACK(FH_SDDQ | 13), "\\Device\\Harddisk120\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq14", BRACK(FH_SDDQ | 14), "\\Device\\Harddisk120\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddq15", BRACK(FH_SDDQ | 15), "\\Device\\Harddisk120\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr1", BRACK(FH_SDDR | 1), "\\Device\\Harddisk121\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr2", BRACK(FH_SDDR | 2), "\\Device\\Harddisk121\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr3", BRACK(FH_SDDR | 3), "\\Device\\Harddisk121\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr4", BRACK(FH_SDDR | 4), "\\Device\\Harddisk121\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr5", BRACK(FH_SDDR | 5), "\\Device\\Harddisk121\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr6", BRACK(FH_SDDR | 6), "\\Device\\Harddisk121\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr7", BRACK(FH_SDDR | 7), "\\Device\\Harddisk121\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr8", BRACK(FH_SDDR | 8), "\\Device\\Harddisk121\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr9", BRACK(FH_SDDR | 9), "\\Device\\Harddisk121\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr10", BRACK(FH_SDDR | 10), "\\Device\\Harddisk121\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr11", BRACK(FH_SDDR | 11), "\\Device\\Harddisk121\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr12", BRACK(FH_SDDR | 12), "\\Device\\Harddisk121\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr13", BRACK(FH_SDDR | 13), "\\Device\\Harddisk121\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr14", BRACK(FH_SDDR | 14), "\\Device\\Harddisk121\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddr15", BRACK(FH_SDDR | 15), "\\Device\\Harddisk121\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds1", BRACK(FH_SDDS | 1), "\\Device\\Harddisk122\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds2", BRACK(FH_SDDS | 2), "\\Device\\Harddisk122\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds3", BRACK(FH_SDDS | 3), "\\Device\\Harddisk122\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds4", BRACK(FH_SDDS | 4), "\\Device\\Harddisk122\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds5", BRACK(FH_SDDS | 5), "\\Device\\Harddisk122\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds6", BRACK(FH_SDDS | 6), "\\Device\\Harddisk122\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds7", BRACK(FH_SDDS | 7), "\\Device\\Harddisk122\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds8", BRACK(FH_SDDS | 8), "\\Device\\Harddisk122\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds9", BRACK(FH_SDDS | 9), "\\Device\\Harddisk122\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds10", BRACK(FH_SDDS | 10), "\\Device\\Harddisk122\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds11", BRACK(FH_SDDS | 11), "\\Device\\Harddisk122\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds12", BRACK(FH_SDDS | 12), "\\Device\\Harddisk122\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds13", BRACK(FH_SDDS | 13), "\\Device\\Harddisk122\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds14", BRACK(FH_SDDS | 14), "\\Device\\Harddisk122\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sdds15", BRACK(FH_SDDS | 15), "\\Device\\Harddisk122\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt1", BRACK(FH_SDDT | 1), "\\Device\\Harddisk123\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt2", BRACK(FH_SDDT | 2), "\\Device\\Harddisk123\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt3", BRACK(FH_SDDT | 3), "\\Device\\Harddisk123\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt4", BRACK(FH_SDDT | 4), "\\Device\\Harddisk123\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt5", BRACK(FH_SDDT | 5), "\\Device\\Harddisk123\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt6", BRACK(FH_SDDT | 6), "\\Device\\Harddisk123\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt7", BRACK(FH_SDDT | 7), "\\Device\\Harddisk123\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt8", BRACK(FH_SDDT | 8), "\\Device\\Harddisk123\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt9", BRACK(FH_SDDT | 9), "\\Device\\Harddisk123\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt10", BRACK(FH_SDDT | 10), "\\Device\\Harddisk123\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt11", BRACK(FH_SDDT | 11), "\\Device\\Harddisk123\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt12", BRACK(FH_SDDT | 12), "\\Device\\Harddisk123\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt13", BRACK(FH_SDDT | 13), "\\Device\\Harddisk123\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt14", BRACK(FH_SDDT | 14), "\\Device\\Harddisk123\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddt15", BRACK(FH_SDDT | 15), "\\Device\\Harddisk123\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu1", BRACK(FH_SDDU | 1), "\\Device\\Harddisk124\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu2", BRACK(FH_SDDU | 2), "\\Device\\Harddisk124\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu3", BRACK(FH_SDDU | 3), "\\Device\\Harddisk124\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu4", BRACK(FH_SDDU | 4), "\\Device\\Harddisk124\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu5", BRACK(FH_SDDU | 5), "\\Device\\Harddisk124\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu6", BRACK(FH_SDDU | 6), "\\Device\\Harddisk124\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu7", BRACK(FH_SDDU | 7), "\\Device\\Harddisk124\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu8", BRACK(FH_SDDU | 8), "\\Device\\Harddisk124\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu9", BRACK(FH_SDDU | 9), "\\Device\\Harddisk124\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu10", BRACK(FH_SDDU | 10), "\\Device\\Harddisk124\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu11", BRACK(FH_SDDU | 11), "\\Device\\Harddisk124\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu12", BRACK(FH_SDDU | 12), "\\Device\\Harddisk124\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu13", BRACK(FH_SDDU | 13), "\\Device\\Harddisk124\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu14", BRACK(FH_SDDU | 14), "\\Device\\Harddisk124\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddu15", BRACK(FH_SDDU | 15), "\\Device\\Harddisk124\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv1", BRACK(FH_SDDV | 1), "\\Device\\Harddisk125\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv2", BRACK(FH_SDDV | 2), "\\Device\\Harddisk125\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv3", BRACK(FH_SDDV | 3), "\\Device\\Harddisk125\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv4", BRACK(FH_SDDV | 4), "\\Device\\Harddisk125\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv5", BRACK(FH_SDDV | 5), "\\Device\\Harddisk125\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv6", BRACK(FH_SDDV | 6), "\\Device\\Harddisk125\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv7", BRACK(FH_SDDV | 7), "\\Device\\Harddisk125\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv8", BRACK(FH_SDDV | 8), "\\Device\\Harddisk125\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv9", BRACK(FH_SDDV | 9), "\\Device\\Harddisk125\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv10", BRACK(FH_SDDV | 10), "\\Device\\Harddisk125\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv11", BRACK(FH_SDDV | 11), "\\Device\\Harddisk125\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv12", BRACK(FH_SDDV | 12), "\\Device\\Harddisk125\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv13", BRACK(FH_SDDV | 13), "\\Device\\Harddisk125\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv14", BRACK(FH_SDDV | 14), "\\Device\\Harddisk125\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddv15", BRACK(FH_SDDV | 15), "\\Device\\Harddisk125\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw1", BRACK(FH_SDDW | 1), "\\Device\\Harddisk126\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw2", BRACK(FH_SDDW | 2), "\\Device\\Harddisk126\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw3", BRACK(FH_SDDW | 3), "\\Device\\Harddisk126\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw4", BRACK(FH_SDDW | 4), "\\Device\\Harddisk126\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw5", BRACK(FH_SDDW | 5), "\\Device\\Harddisk126\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw6", BRACK(FH_SDDW | 6), "\\Device\\Harddisk126\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw7", BRACK(FH_SDDW | 7), "\\Device\\Harddisk126\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw8", BRACK(FH_SDDW | 8), "\\Device\\Harddisk126\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw9", BRACK(FH_SDDW | 9), "\\Device\\Harddisk126\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw10", BRACK(FH_SDDW | 10), "\\Device\\Harddisk126\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw11", BRACK(FH_SDDW | 11), "\\Device\\Harddisk126\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw12", BRACK(FH_SDDW | 12), "\\Device\\Harddisk126\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw13", BRACK(FH_SDDW | 13), "\\Device\\Harddisk126\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw14", BRACK(FH_SDDW | 14), "\\Device\\Harddisk126\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddw15", BRACK(FH_SDDW | 15), "\\Device\\Harddisk126\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx1", BRACK(FH_SDDX | 1), "\\Device\\Harddisk127\\Partition1", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx2", BRACK(FH_SDDX | 2), "\\Device\\Harddisk127\\Partition2", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx3", BRACK(FH_SDDX | 3), "\\Device\\Harddisk127\\Partition3", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx4", BRACK(FH_SDDX | 4), "\\Device\\Harddisk127\\Partition4", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx5", BRACK(FH_SDDX | 5), "\\Device\\Harddisk127\\Partition5", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx6", BRACK(FH_SDDX | 6), "\\Device\\Harddisk127\\Partition6", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx7", BRACK(FH_SDDX | 7), "\\Device\\Harddisk127\\Partition7", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx8", BRACK(FH_SDDX | 8), "\\Device\\Harddisk127\\Partition8", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx9", BRACK(FH_SDDX | 9), "\\Device\\Harddisk127\\Partition9", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx10", BRACK(FH_SDDX | 10), "\\Device\\Harddisk127\\Partition10", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx11", BRACK(FH_SDDX | 11), "\\Device\\Harddisk127\\Partition11", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx12", BRACK(FH_SDDX | 12), "\\Device\\Harddisk127\\Partition12", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx13", BRACK(FH_SDDX | 13), "\\Device\\Harddisk127\\Partition13", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx14", BRACK(FH_SDDX | 14), "\\Device\\Harddisk127\\Partition14", exists_ntdev, S_IFBLK, true},
  {"/dev/sddx15", BRACK(FH_SDDX | 15), "\\Device\\Harddisk127\\Partition15", exists_ntdev, S_IFBLK, true},
  {"/dev/sr0", BRACK(FHDEV(DEV_CDROM_MAJOR, 0)), "\\Device\\CdRom0", exists_ntdev, S_IFBLK, true},
  {"/dev/sr1", BRACK(FHDEV(DEV_CDROM_MAJOR, 1)), "\\Device\\CdRom1", exists_ntdev, S_IFBLK, true},
  {"/dev/sr2", BRACK(FHDEV(DEV_CDROM_MAJOR, 2)), "\\Device\\CdRom2", exists_ntdev, S_IFBLK, true},
  {"/dev/sr3", BRACK(FHDEV(DEV_CDROM_MAJOR, 3)), "\\Device\\CdRom3", exists_ntdev, S_IFBLK, true},
  {"/dev/sr4", BRACK(FHDEV(DEV_CDROM_MAJOR, 4)), "\\Device\\CdRom4", exists_ntdev, S_IFBLK, true},
  {"/dev/sr5", BRACK(FHDEV(DEV_CDROM_MAJOR, 5)), "\\Device\\CdRom5", exists_ntdev, S_IFBLK, true},
  {"/dev/sr6", BRACK(FHDEV(DEV_CDROM_MAJOR, 6)), "\\Device\\CdRom6", exists_ntdev, S_IFBLK, true},
  {"/dev/sr7", BRACK(FHDEV(DEV_CDROM_MAJOR, 7)), "\\Device\\CdRom7", exists_ntdev, S_IFBLK, true},
  {"/dev/sr8", BRACK(FHDEV(DEV_CDROM_MAJOR, 8)), "\\Device\\CdRom8", exists_ntdev, S_IFBLK, true},
  {"/dev/sr9", BRACK(FHDEV(DEV_CDROM_MAJOR, 9)), "\\Device\\CdRom9", exists_ntdev, S_IFBLK, true},
  {"/dev/sr10", BRACK(FHDEV(DEV_CDROM_MAJOR, 10)), "\\Device\\CdRom10", exists_ntdev, S_IFBLK, true},
  {"/dev/sr11", BRACK(FHDEV(DEV_CDROM_MAJOR, 11)), "\\Device\\CdRom11", exists_ntdev, S_IFBLK, true},
  {"/dev/sr12", BRACK(FHDEV(DEV_CDROM_MAJOR, 12)), "\\Device\\CdRom12", exists_ntdev, S_IFBLK, true},
  {"/dev/sr13", BRACK(FHDEV(DEV_CDROM_MAJOR, 13)), "\\Device\\CdRom13", exists_ntdev, S_IFBLK, true},
  {"/dev/sr14", BRACK(FHDEV(DEV_CDROM_MAJOR, 14)), "\\Device\\CdRom14", exists_ntdev, S_IFBLK, true},
  {"/dev/sr15", BRACK(FHDEV(DEV_CDROM_MAJOR, 15)), "\\Device\\CdRom15", exists_ntdev, S_IFBLK, true},
  {"/dev/st0", BRACK(FHDEV(DEV_TAPE_MAJOR, 0)), "\\Device\\Tape0", exists_ntdev, S_IFBLK, true},
  {"/dev/st1", BRACK(FHDEV(DEV_TAPE_MAJOR, 1)), "\\Device\\Tape1", exists_ntdev, S_IFBLK, true},
  {"/dev/st2", BRACK(FHDEV(DEV_TAPE_MAJOR, 2)), "\\Device\\Tape2", exists_ntdev, S_IFBLK, true},
  {"/dev/st3", BRACK(FHDEV(DEV_TAPE_MAJOR, 3)), "\\Device\\Tape3", exists_ntdev, S_IFBLK, true},
  {"/dev/st4", BRACK(FHDEV(DEV_TAPE_MAJOR, 4)), "\\Device\\Tape4", exists_ntdev, S_IFBLK, true},
  {"/dev/st5", BRACK(FHDEV(DEV_TAPE_MAJOR, 5)), "\\Device\\Tape5", exists_ntdev, S_IFBLK, true},
  {"/dev/st6", BRACK(FHDEV(DEV_TAPE_MAJOR, 6)), "\\Device\\Tape6", exists_ntdev, S_IFBLK, true},
  {"/dev/st7", BRACK(FHDEV(DEV_TAPE_MAJOR, 7)), "\\Device\\Tape7", exists_ntdev, S_IFBLK, true},
  {"/dev/st8", BRACK(FHDEV(DEV_TAPE_MAJOR, 8)), "\\Device\\Tape8", exists_ntdev, S_IFBLK, true},
  {"/dev/st9", BRACK(FHDEV(DEV_TAPE_MAJOR, 9)), "\\Device\\Tape9", exists_ntdev, S_IFBLK, true},
  {"/dev/st10", BRACK(FHDEV(DEV_TAPE_MAJOR, 10)), "\\Device\\Tape10", exists_ntdev, S_IFBLK, true},
  {"/dev/st11", BRACK(FHDEV(DEV_TAPE_MAJOR, 11)), "\\Device\\Tape11", exists_ntdev, S_IFBLK, true},
  {"/dev/st12", BRACK(FHDEV(DEV_TAPE_MAJOR, 12)), "\\Device\\Tape12", exists_ntdev, S_IFBLK, true},
  {"/dev/st13", BRACK(FHDEV(DEV_TAPE_MAJOR, 13)), "\\Device\\Tape13", exists_ntdev, S_IFBLK, true},
  {"/dev/st14", BRACK(FHDEV(DEV_TAPE_MAJOR, 14)), "\\Device\\Tape14", exists_ntdev, S_IFBLK, true},
  {"/dev/st15", BRACK(FHDEV(DEV_TAPE_MAJOR, 15)), "\\Device\\Tape15", exists_ntdev, S_IFBLK, true},
  {"/dev/st16", BRACK(FHDEV(DEV_TAPE_MAJOR, 16)), "\\Device\\Tape16", exists_ntdev, S_IFBLK, true},
  {"/dev/st17", BRACK(FHDEV(DEV_TAPE_MAJOR, 17)), "\\Device\\Tape17", exists_ntdev, S_IFBLK, true},
  {"/dev/st18", BRACK(FHDEV(DEV_TAPE_MAJOR, 18)), "\\Device\\Tape18", exists_ntdev, S_IFBLK, true},
  {"/dev/st19", BRACK(FHDEV(DEV_TAPE_MAJOR, 19)), "\\Device\\Tape19", exists_ntdev, S_IFBLK, true},
  {"/dev/st20", BRACK(FHDEV(DEV_TAPE_MAJOR, 20)), "\\Device\\Tape20", exists_ntdev, S_IFBLK, true},
  {"/dev/st21", BRACK(FHDEV(DEV_TAPE_MAJOR, 21)), "\\Device\\Tape21", exists_ntdev, S_IFBLK, true},
  {"/dev/st22", BRACK(FHDEV(DEV_TAPE_MAJOR, 22)), "\\Device\\Tape22", exists_ntdev, S_IFBLK, true},
  {"/dev/st23", BRACK(FHDEV(DEV_TAPE_MAJOR, 23)), "\\Device\\Tape23", exists_ntdev, S_IFBLK, true},
  {"/dev/st24", BRACK(FHDEV(DEV_TAPE_MAJOR, 24)), "\\Device\\Tape24", exists_ntdev, S_IFBLK, true},
  {"/dev/st25", BRACK(FHDEV(DEV_TAPE_MAJOR, 25)), "\\Device\\Tape25", exists_ntdev, S_IFBLK, true},
  {"/dev/st26", BRACK(FHDEV(DEV_TAPE_MAJOR, 26)), "\\Device\\Tape26", exists_ntdev, S_IFBLK, true},
  {"/dev/st27", BRACK(FHDEV(DEV_TAPE_MAJOR, 27)), "\\Device\\Tape27", exists_ntdev, S_IFBLK, true},
  {"/dev/st28", BRACK(FHDEV(DEV_TAPE_MAJOR, 28)), "\\Device\\Tape28", exists_ntdev, S_IFBLK, true},
  {"/dev/st29", BRACK(FHDEV(DEV_TAPE_MAJOR, 29)), "\\Device\\Tape29", exists_ntdev, S_IFBLK, true},
  {"/dev/st30", BRACK(FHDEV(DEV_TAPE_MAJOR, 30)), "\\Device\\Tape30", exists_ntdev, S_IFBLK, true},
  {"/dev/st31", BRACK(FHDEV(DEV_TAPE_MAJOR, 31)), "\\Device\\Tape31", exists_ntdev, S_IFBLK, true},
  {"/dev/st32", BRACK(FHDEV(DEV_TAPE_MAJOR, 32)), "\\Device\\Tape32", exists_ntdev, S_IFBLK, true},
  {"/dev/st33", BRACK(FHDEV(DEV_TAPE_MAJOR, 33)), "\\Device\\Tape33", exists_ntdev, S_IFBLK, true},
  {"/dev/st34", BRACK(FHDEV(DEV_TAPE_MAJOR, 34)), "\\Device\\Tape34", exists_ntdev, S_IFBLK, true},
  {"/dev/st35", BRACK(FHDEV(DEV_TAPE_MAJOR, 35)), "\\Device\\Tape35", exists_ntdev, S_IFBLK, true},
  {"/dev/st36", BRACK(FHDEV(DEV_TAPE_MAJOR, 36)), "\\Device\\Tape36", exists_ntdev, S_IFBLK, true},
  {"/dev/st37", BRACK(FHDEV(DEV_TAPE_MAJOR, 37)), "\\Device\\Tape37", exists_ntdev, S_IFBLK, true},
  {"/dev/st38", BRACK(FHDEV(DEV_TAPE_MAJOR, 38)), "\\Device\\Tape38", exists_ntdev, S_IFBLK, true},
  {"/dev/st39", BRACK(FHDEV(DEV_TAPE_MAJOR, 39)), "\\Device\\Tape39", exists_ntdev, S_IFBLK, true},
  {"/dev/st40", BRACK(FHDEV(DEV_TAPE_MAJOR, 40)), "\\Device\\Tape40", exists_ntdev, S_IFBLK, true},
  {"/dev/st41", BRACK(FHDEV(DEV_TAPE_MAJOR, 41)), "\\Device\\Tape41", exists_ntdev, S_IFBLK, true},
  {"/dev/st42", BRACK(FHDEV(DEV_TAPE_MAJOR, 42)), "\\Device\\Tape42", exists_ntdev, S_IFBLK, true},
  {"/dev/st43", BRACK(FHDEV(DEV_TAPE_MAJOR, 43)), "\\Device\\Tape43", exists_ntdev, S_IFBLK, true},
  {"/dev/st44", BRACK(FHDEV(DEV_TAPE_MAJOR, 44)), "\\Device\\Tape44", exists_ntdev, S_IFBLK, true},
  {"/dev/st45", BRACK(FHDEV(DEV_TAPE_MAJOR, 45)), "\\Device\\Tape45", exists_ntdev, S_IFBLK, true},
  {"/dev/st46", BRACK(FHDEV(DEV_TAPE_MAJOR, 46)), "\\Device\\Tape46", exists_ntdev, S_IFBLK, true},
  {"/dev/st47", BRACK(FHDEV(DEV_TAPE_MAJOR, 47)), "\\Device\\Tape47", exists_ntdev, S_IFBLK, true},
  {"/dev/st48", BRACK(FHDEV(DEV_TAPE_MAJOR, 48)), "\\Device\\Tape48", exists_ntdev, S_IFBLK, true},
  {"/dev/st49", BRACK(FHDEV(DEV_TAPE_MAJOR, 49)), "\\Device\\Tape49", exists_ntdev, S_IFBLK, true},
  {"/dev/st50", BRACK(FHDEV(DEV_TAPE_MAJOR, 50)), "\\Device\\Tape50", exists_ntdev, S_IFBLK, true},
  {"/dev/st51", BRACK(FHDEV(DEV_TAPE_MAJOR, 51)), "\\Device\\Tape51", exists_ntdev, S_IFBLK, true},
  {"/dev/st52", BRACK(FHDEV(DEV_TAPE_MAJOR, 52)), "\\Device\\Tape52", exists_ntdev, S_IFBLK, true},
  {"/dev/st53", BRACK(FHDEV(DEV_TAPE_MAJOR, 53)), "\\Device\\Tape53", exists_ntdev, S_IFBLK, true},
  {"/dev/st54", BRACK(FHDEV(DEV_TAPE_MAJOR, 54)), "\\Device\\Tape54", exists_ntdev, S_IFBLK, true},
  {"/dev/st55", BRACK(FHDEV(DEV_TAPE_MAJOR, 55)), "\\Device\\Tape55", exists_ntdev, S_IFBLK, true},
  {"/dev/st56", BRACK(FHDEV(DEV_TAPE_MAJOR, 56)), "\\Device\\Tape56", exists_ntdev, S_IFBLK, true},
  {"/dev/st57", BRACK(FHDEV(DEV_TAPE_MAJOR, 57)), "\\Device\\Tape57", exists_ntdev, S_IFBLK, true},
  {"/dev/st58", BRACK(FHDEV(DEV_TAPE_MAJOR, 58)), "\\Device\\Tape58", exists_ntdev, S_IFBLK, true},
  {"/dev/st59", BRACK(FHDEV(DEV_TAPE_MAJOR, 59)), "\\Device\\Tape59", exists_ntdev, S_IFBLK, true},
  {"/dev/st60", BRACK(FHDEV(DEV_TAPE_MAJOR, 60)), "\\Device\\Tape60", exists_ntdev, S_IFBLK, true},
  {"/dev/st61", BRACK(FHDEV(DEV_TAPE_MAJOR, 61)), "\\Device\\Tape61", exists_ntdev, S_IFBLK, true},
  {"/dev/st62", BRACK(FHDEV(DEV_TAPE_MAJOR, 62)), "\\Device\\Tape62", exists_ntdev, S_IFBLK, true},
  {"/dev/st63", BRACK(FHDEV(DEV_TAPE_MAJOR, 63)), "\\Device\\Tape63", exists_ntdev, S_IFBLK, true},
  {"/dev/st64", BRACK(FHDEV(DEV_TAPE_MAJOR, 64)), "\\Device\\Tape64", exists_ntdev, S_IFBLK, true},
  {"/dev/st65", BRACK(FHDEV(DEV_TAPE_MAJOR, 65)), "\\Device\\Tape65", exists_ntdev, S_IFBLK, true},
  {"/dev/st66", BRACK(FHDEV(DEV_TAPE_MAJOR, 66)), "\\Device\\Tape66", exists_ntdev, S_IFBLK, true},
  {"/dev/st67", BRACK(FHDEV(DEV_TAPE_MAJOR, 67)), "\\Device\\Tape67", exists_ntdev, S_IFBLK, true},
  {"/dev/st68", BRACK(FHDEV(DEV_TAPE_MAJOR, 68)), "\\Device\\Tape68", exists_ntdev, S_IFBLK, true},
  {"/dev/st69", BRACK(FHDEV(DEV_TAPE_MAJOR, 69)), "\\Device\\Tape69", exists_ntdev, S_IFBLK, true},
  {"/dev/st70", BRACK(FHDEV(DEV_TAPE_MAJOR, 70)), "\\Device\\Tape70", exists_ntdev, S_IFBLK, true},
  {"/dev/st71", BRACK(FHDEV(DEV_TAPE_MAJOR, 71)), "\\Device\\Tape71", exists_ntdev, S_IFBLK, true},
  {"/dev/st72", BRACK(FHDEV(DEV_TAPE_MAJOR, 72)), "\\Device\\Tape72", exists_ntdev, S_IFBLK, true},
  {"/dev/st73", BRACK(FHDEV(DEV_TAPE_MAJOR, 73)), "\\Device\\Tape73", exists_ntdev, S_IFBLK, true},
  {"/dev/st74", BRACK(FHDEV(DEV_TAPE_MAJOR, 74)), "\\Device\\Tape74", exists_ntdev, S_IFBLK, true},
  {"/dev/st75", BRACK(FHDEV(DEV_TAPE_MAJOR, 75)), "\\Device\\Tape75", exists_ntdev, S_IFBLK, true},
  {"/dev/st76", BRACK(FHDEV(DEV_TAPE_MAJOR, 76)), "\\Device\\Tape76", exists_ntdev, S_IFBLK, true},
  {"/dev/st77", BRACK(FHDEV(DEV_TAPE_MAJOR, 77)), "\\Device\\Tape77", exists_ntdev, S_IFBLK, true},
  {"/dev/st78", BRACK(FHDEV(DEV_TAPE_MAJOR, 78)), "\\Device\\Tape78", exists_ntdev, S_IFBLK, true},
  {"/dev/st79", BRACK(FHDEV(DEV_TAPE_MAJOR, 79)), "\\Device\\Tape79", exists_ntdev, S_IFBLK, true},
  {"/dev/st80", BRACK(FHDEV(DEV_TAPE_MAJOR, 80)), "\\Device\\Tape80", exists_ntdev, S_IFBLK, true},
  {"/dev/st81", BRACK(FHDEV(DEV_TAPE_MAJOR, 81)), "\\Device\\Tape81", exists_ntdev, S_IFBLK, true},
  {"/dev/st82", BRACK(FHDEV(DEV_TAPE_MAJOR, 82)), "\\Device\\Tape82", exists_ntdev, S_IFBLK, true},
  {"/dev/st83", BRACK(FHDEV(DEV_TAPE_MAJOR, 83)), "\\Device\\Tape83", exists_ntdev, S_IFBLK, true},
  {"/dev/st84", BRACK(FHDEV(DEV_TAPE_MAJOR, 84)), "\\Device\\Tape84", exists_ntdev, S_IFBLK, true},
  {"/dev/st85", BRACK(FHDEV(DEV_TAPE_MAJOR, 85)), "\\Device\\Tape85", exists_ntdev, S_IFBLK, true},
  {"/dev/st86", BRACK(FHDEV(DEV_TAPE_MAJOR, 86)), "\\Device\\Tape86", exists_ntdev, S_IFBLK, true},
  {"/dev/st87", BRACK(FHDEV(DEV_TAPE_MAJOR, 87)), "\\Device\\Tape87", exists_ntdev, S_IFBLK, true},
  {"/dev/st88", BRACK(FHDEV(DEV_TAPE_MAJOR, 88)), "\\Device\\Tape88", exists_ntdev, S_IFBLK, true},
  {"/dev/st89", BRACK(FHDEV(DEV_TAPE_MAJOR, 89)), "\\Device\\Tape89", exists_ntdev, S_IFBLK, true},
  {"/dev/st90", BRACK(FHDEV(DEV_TAPE_MAJOR, 90)), "\\Device\\Tape90", exists_ntdev, S_IFBLK, true},
  {"/dev/st91", BRACK(FHDEV(DEV_TAPE_MAJOR, 91)), "\\Device\\Tape91", exists_ntdev, S_IFBLK, true},
  {"/dev/st92", BRACK(FHDEV(DEV_TAPE_MAJOR, 92)), "\\Device\\Tape92", exists_ntdev, S_IFBLK, true},
  {"/dev/st93", BRACK(FHDEV(DEV_TAPE_MAJOR, 93)), "\\Device\\Tape93", exists_ntdev, S_IFBLK, true},
  {"/dev/st94", BRACK(FHDEV(DEV_TAPE_MAJOR, 94)), "\\Device\\Tape94", exists_ntdev, S_IFBLK, true},
  {"/dev/st95", BRACK(FHDEV(DEV_TAPE_MAJOR, 95)), "\\Device\\Tape95", exists_ntdev, S_IFBLK, true},
  {"/dev/st96", BRACK(FHDEV(DEV_TAPE_MAJOR, 96)), "\\Device\\Tape96", exists_ntdev, S_IFBLK, true},
  {"/dev/st97", BRACK(FHDEV(DEV_TAPE_MAJOR, 97)), "\\Device\\Tape97", exists_ntdev, S_IFBLK, true},
  {"/dev/st98", BRACK(FHDEV(DEV_TAPE_MAJOR, 98)), "\\Device\\Tape98", exists_ntdev, S_IFBLK, true},
  {"/dev/st99", BRACK(FHDEV(DEV_TAPE_MAJOR, 99)), "\\Device\\Tape99", exists_ntdev, S_IFBLK, true},
  {"/dev/st100", BRACK(FHDEV(DEV_TAPE_MAJOR, 100)), "\\Device\\Tape100", exists_ntdev, S_IFBLK, true},
  {"/dev/st101", BRACK(FHDEV(DEV_TAPE_MAJOR, 101)), "\\Device\\Tape101", exists_ntdev, S_IFBLK, true},
  {"/dev/st102", BRACK(FHDEV(DEV_TAPE_MAJOR, 102)), "\\Device\\Tape102", exists_ntdev, S_IFBLK, true},
  {"/dev/st103", BRACK(FHDEV(DEV_TAPE_MAJOR, 103)), "\\Device\\Tape103", exists_ntdev, S_IFBLK, true},
  {"/dev/st104", BRACK(FHDEV(DEV_TAPE_MAJOR, 104)), "\\Device\\Tape104", exists_ntdev, S_IFBLK, true},
  {"/dev/st105", BRACK(FHDEV(DEV_TAPE_MAJOR, 105)), "\\Device\\Tape105", exists_ntdev, S_IFBLK, true},
  {"/dev/st106", BRACK(FHDEV(DEV_TAPE_MAJOR, 106)), "\\Device\\Tape106", exists_ntdev, S_IFBLK, true},
  {"/dev/st107", BRACK(FHDEV(DEV_TAPE_MAJOR, 107)), "\\Device\\Tape107", exists_ntdev, S_IFBLK, true},
  {"/dev/st108", BRACK(FHDEV(DEV_TAPE_MAJOR, 108)), "\\Device\\Tape108", exists_ntdev, S_IFBLK, true},
  {"/dev/st109", BRACK(FHDEV(DEV_TAPE_MAJOR, 109)), "\\Device\\Tape109", exists_ntdev, S_IFBLK, true},
  {"/dev/st110", BRACK(FHDEV(DEV_TAPE_MAJOR, 110)), "\\Device\\Tape110", exists_ntdev, S_IFBLK, true},
  {"/dev/st111", BRACK(FHDEV(DEV_TAPE_MAJOR, 111)), "\\Device\\Tape111", exists_ntdev, S_IFBLK, true},
  {"/dev/st112", BRACK(FHDEV(DEV_TAPE_MAJOR, 112)), "\\Device\\Tape112", exists_ntdev, S_IFBLK, true},
  {"/dev/st113", BRACK(FHDEV(DEV_TAPE_MAJOR, 113)), "\\Device\\Tape113", exists_ntdev, S_IFBLK, true},
  {"/dev/st114", BRACK(FHDEV(DEV_TAPE_MAJOR, 114)), "\\Device\\Tape114", exists_ntdev, S_IFBLK, true},
  {"/dev/st115", BRACK(FHDEV(DEV_TAPE_MAJOR, 115)), "\\Device\\Tape115", exists_ntdev, S_IFBLK, true},
  {"/dev/st116", BRACK(FHDEV(DEV_TAPE_MAJOR, 116)), "\\Device\\Tape116", exists_ntdev, S_IFBLK, true},
  {"/dev/st117", BRACK(FHDEV(DEV_TAPE_MAJOR, 117)), "\\Device\\Tape117", exists_ntdev, S_IFBLK, true},
  {"/dev/st118", BRACK(FHDEV(DEV_TAPE_MAJOR, 118)), "\\Device\\Tape118", exists_ntdev, S_IFBLK, true},
  {"/dev/st119", BRACK(FHDEV(DEV_TAPE_MAJOR, 119)), "\\Device\\Tape119", exists_ntdev, S_IFBLK, true},
  {"/dev/st120", BRACK(FHDEV(DEV_TAPE_MAJOR, 120)), "\\Device\\Tape120", exists_ntdev, S_IFBLK, true},
  {"/dev/st121", BRACK(FHDEV(DEV_TAPE_MAJOR, 121)), "\\Device\\Tape121", exists_ntdev, S_IFBLK, true},
  {"/dev/st122", BRACK(FHDEV(DEV_TAPE_MAJOR, 122)), "\\Device\\Tape122", exists_ntdev, S_IFBLK, true},
  {"/dev/st123", BRACK(FHDEV(DEV_TAPE_MAJOR, 123)), "\\Device\\Tape123", exists_ntdev, S_IFBLK, true},
  {"/dev/st124", BRACK(FHDEV(DEV_TAPE_MAJOR, 124)), "\\Device\\Tape124", exists_ntdev, S_IFBLK, true},
  {"/dev/st125", BRACK(FHDEV(DEV_TAPE_MAJOR, 125)), "\\Device\\Tape125", exists_ntdev, S_IFBLK, true},
  {"/dev/st126", BRACK(FHDEV(DEV_TAPE_MAJOR, 126)), "\\Device\\Tape126", exists_ntdev, S_IFBLK, true},
  {"/dev/st127", BRACK(FHDEV(DEV_TAPE_MAJOR, 127)), "\\Device\\Tape127", exists_ntdev, S_IFBLK, true},
  {"/dev/tty", BRACK(FH_TTY), "/dev/tty", exists, S_IFCHR, true},
  {"/dev/ttyS0", BRACK(FHDEV(DEV_SERIAL_MAJOR, 0)), "\\??\\COM1", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS1", BRACK(FHDEV(DEV_SERIAL_MAJOR, 1)), "\\??\\COM2", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS2", BRACK(FHDEV(DEV_SERIAL_MAJOR, 2)), "\\??\\COM3", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS3", BRACK(FHDEV(DEV_SERIAL_MAJOR, 3)), "\\??\\COM4", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS4", BRACK(FHDEV(DEV_SERIAL_MAJOR, 4)), "\\??\\COM5", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS5", BRACK(FHDEV(DEV_SERIAL_MAJOR, 5)), "\\??\\COM6", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS6", BRACK(FHDEV(DEV_SERIAL_MAJOR, 6)), "\\??\\COM7", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS7", BRACK(FHDEV(DEV_SERIAL_MAJOR, 7)), "\\??\\COM8", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS8", BRACK(FHDEV(DEV_SERIAL_MAJOR, 8)), "\\??\\COM9", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS9", BRACK(FHDEV(DEV_SERIAL_MAJOR, 9)), "\\??\\COM10", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS10", BRACK(FHDEV(DEV_SERIAL_MAJOR, 10)), "\\??\\COM11", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS11", BRACK(FHDEV(DEV_SERIAL_MAJOR, 11)), "\\??\\COM12", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS12", BRACK(FHDEV(DEV_SERIAL_MAJOR, 12)), "\\??\\COM13", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS13", BRACK(FHDEV(DEV_SERIAL_MAJOR, 13)), "\\??\\COM14", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS14", BRACK(FHDEV(DEV_SERIAL_MAJOR, 14)), "\\??\\COM15", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS15", BRACK(FHDEV(DEV_SERIAL_MAJOR, 15)), "\\??\\COM16", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS16", BRACK(FHDEV(DEV_SERIAL_MAJOR, 16)), "\\??\\COM17", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS17", BRACK(FHDEV(DEV_SERIAL_MAJOR, 17)), "\\??\\COM18", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS18", BRACK(FHDEV(DEV_SERIAL_MAJOR, 18)), "\\??\\COM19", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS19", BRACK(FHDEV(DEV_SERIAL_MAJOR, 19)), "\\??\\COM20", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS20", BRACK(FHDEV(DEV_SERIAL_MAJOR, 20)), "\\??\\COM21", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS21", BRACK(FHDEV(DEV_SERIAL_MAJOR, 21)), "\\??\\COM22", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS22", BRACK(FHDEV(DEV_SERIAL_MAJOR, 22)), "\\??\\COM23", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS23", BRACK(FHDEV(DEV_SERIAL_MAJOR, 23)), "\\??\\COM24", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS24", BRACK(FHDEV(DEV_SERIAL_MAJOR, 24)), "\\??\\COM25", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS25", BRACK(FHDEV(DEV_SERIAL_MAJOR, 25)), "\\??\\COM26", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS26", BRACK(FHDEV(DEV_SERIAL_MAJOR, 26)), "\\??\\COM27", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS27", BRACK(FHDEV(DEV_SERIAL_MAJOR, 27)), "\\??\\COM28", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS28", BRACK(FHDEV(DEV_SERIAL_MAJOR, 28)), "\\??\\COM29", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS29", BRACK(FHDEV(DEV_SERIAL_MAJOR, 29)), "\\??\\COM30", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS30", BRACK(FHDEV(DEV_SERIAL_MAJOR, 30)), "\\??\\COM31", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS31", BRACK(FHDEV(DEV_SERIAL_MAJOR, 31)), "\\??\\COM32", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS32", BRACK(FHDEV(DEV_SERIAL_MAJOR, 32)), "\\??\\COM33", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS33", BRACK(FHDEV(DEV_SERIAL_MAJOR, 33)), "\\??\\COM34", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS34", BRACK(FHDEV(DEV_SERIAL_MAJOR, 34)), "\\??\\COM35", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS35", BRACK(FHDEV(DEV_SERIAL_MAJOR, 35)), "\\??\\COM36", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS36", BRACK(FHDEV(DEV_SERIAL_MAJOR, 36)), "\\??\\COM37", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS37", BRACK(FHDEV(DEV_SERIAL_MAJOR, 37)), "\\??\\COM38", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS38", BRACK(FHDEV(DEV_SERIAL_MAJOR, 38)), "\\??\\COM39", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS39", BRACK(FHDEV(DEV_SERIAL_MAJOR, 39)), "\\??\\COM40", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS40", BRACK(FHDEV(DEV_SERIAL_MAJOR, 40)), "\\??\\COM41", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS41", BRACK(FHDEV(DEV_SERIAL_MAJOR, 41)), "\\??\\COM42", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS42", BRACK(FHDEV(DEV_SERIAL_MAJOR, 42)), "\\??\\COM43", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS43", BRACK(FHDEV(DEV_SERIAL_MAJOR, 43)), "\\??\\COM44", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS44", BRACK(FHDEV(DEV_SERIAL_MAJOR, 44)), "\\??\\COM45", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS45", BRACK(FHDEV(DEV_SERIAL_MAJOR, 45)), "\\??\\COM46", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS46", BRACK(FHDEV(DEV_SERIAL_MAJOR, 46)), "\\??\\COM47", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS47", BRACK(FHDEV(DEV_SERIAL_MAJOR, 47)), "\\??\\COM48", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS48", BRACK(FHDEV(DEV_SERIAL_MAJOR, 48)), "\\??\\COM49", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS49", BRACK(FHDEV(DEV_SERIAL_MAJOR, 49)), "\\??\\COM50", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS50", BRACK(FHDEV(DEV_SERIAL_MAJOR, 50)), "\\??\\COM51", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS51", BRACK(FHDEV(DEV_SERIAL_MAJOR, 51)), "\\??\\COM52", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS52", BRACK(FHDEV(DEV_SERIAL_MAJOR, 52)), "\\??\\COM53", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS53", BRACK(FHDEV(DEV_SERIAL_MAJOR, 53)), "\\??\\COM54", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS54", BRACK(FHDEV(DEV_SERIAL_MAJOR, 54)), "\\??\\COM55", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS55", BRACK(FHDEV(DEV_SERIAL_MAJOR, 55)), "\\??\\COM56", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS56", BRACK(FHDEV(DEV_SERIAL_MAJOR, 56)), "\\??\\COM57", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS57", BRACK(FHDEV(DEV_SERIAL_MAJOR, 57)), "\\??\\COM58", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS58", BRACK(FHDEV(DEV_SERIAL_MAJOR, 58)), "\\??\\COM59", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS59", BRACK(FHDEV(DEV_SERIAL_MAJOR, 59)), "\\??\\COM60", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS60", BRACK(FHDEV(DEV_SERIAL_MAJOR, 60)), "\\??\\COM61", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS61", BRACK(FHDEV(DEV_SERIAL_MAJOR, 61)), "\\??\\COM62", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS62", BRACK(FHDEV(DEV_SERIAL_MAJOR, 62)), "\\??\\COM63", exists_ntdev, S_IFCHR, true},
  {"/dev/ttyS63", BRACK(FHDEV(DEV_SERIAL_MAJOR, 63)), "\\??\\COM64", exists_ntdev, S_IFCHR, true},
  {"/dev/urandom", BRACK(FH_URANDOM), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {"/dev/windows", BRACK(FH_WINDOWS), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {"/dev/zero", BRACK(FH_ZERO), "\\Device\\Null", exists_ntdev, S_IFCHR, true},
  {":fifo", BRACK(FH_FIFO), "/dev/fifo", exists_internal, S_IFCHR, false},
  {":pipe", BRACK(FH_PIPE), "/dev/pipe", exists_internal, S_IFCHR, false},
  {":ptym0", BRACK(FHDEV(DEV_PTYM_MAJOR, 0)), "/dev/ptym0", exists_internal, S_IFCHR, false},
  {":ptym1", BRACK(FHDEV(DEV_PTYM_MAJOR, 1)), "/dev/ptym1", exists_internal, S_IFCHR, false},
  {":ptym2", BRACK(FHDEV(DEV_PTYM_MAJOR, 2)), "/dev/ptym2", exists_internal, S_IFCHR, false},
  {":ptym3", BRACK(FHDEV(DEV_PTYM_MAJOR, 3)), "/dev/ptym3", exists_internal, S_IFCHR, false},
  {":ptym4", BRACK(FHDEV(DEV_PTYM_MAJOR, 4)), "/dev/ptym4", exists_internal, S_IFCHR, false},
  {":ptym5", BRACK(FHDEV(DEV_PTYM_MAJOR, 5)), "/dev/ptym5", exists_internal, S_IFCHR, false},
  {":ptym6", BRACK(FHDEV(DEV_PTYM_MAJOR, 6)), "/dev/ptym6", exists_internal, S_IFCHR, false},
  {":ptym7", BRACK(FHDEV(DEV_PTYM_MAJOR, 7)), "/dev/ptym7", exists_internal, S_IFCHR, false},
  {":ptym8", BRACK(FHDEV(DEV_PTYM_MAJOR, 8)), "/dev/ptym8", exists_internal, S_IFCHR, false},
  {":ptym9", BRACK(FHDEV(DEV_PTYM_MAJOR, 9)), "/dev/ptym9", exists_internal, S_IFCHR, false},
  {":ptym10", BRACK(FHDEV(DEV_PTYM_MAJOR, 10)), "/dev/ptym10", exists_internal, S_IFCHR, false},
  {":ptym11", BRACK(FHDEV(DEV_PTYM_MAJOR, 11)), "/dev/ptym11", exists_internal, S_IFCHR, false},
  {":ptym12", BRACK(FHDEV(DEV_PTYM_MAJOR, 12)), "/dev/ptym12", exists_internal, S_IFCHR, false},
  {":ptym13", BRACK(FHDEV(DEV_PTYM_MAJOR, 13)), "/dev/ptym13", exists_internal, S_IFCHR, false},
  {":ptym14", BRACK(FHDEV(DEV_PTYM_MAJOR, 14)), "/dev/ptym14", exists_internal, S_IFCHR, false},
  {":ptym15", BRACK(FHDEV(DEV_PTYM_MAJOR, 15)), "/dev/ptym15", exists_internal, S_IFCHR, false},
  {":ptym16", BRACK(FHDEV(DEV_PTYM_MAJOR, 16)), "/dev/ptym16", exists_internal, S_IFCHR, false},
  {":ptym17", BRACK(FHDEV(DEV_PTYM_MAJOR, 17)), "/dev/ptym17", exists_internal, S_IFCHR, false},
  {":ptym18", BRACK(FHDEV(DEV_PTYM_MAJOR, 18)), "/dev/ptym18", exists_internal, S_IFCHR, false},
  {":ptym19", BRACK(FHDEV(DEV_PTYM_MAJOR, 19)), "/dev/ptym19", exists_internal, S_IFCHR, false},
  {":ptym20", BRACK(FHDEV(DEV_PTYM_MAJOR, 20)), "/dev/ptym20", exists_internal, S_IFCHR, false},
  {":ptym21", BRACK(FHDEV(DEV_PTYM_MAJOR, 21)), "/dev/ptym21", exists_internal, S_IFCHR, false},
  {":ptym22", BRACK(FHDEV(DEV_PTYM_MAJOR, 22)), "/dev/ptym22", exists_internal, S_IFCHR, false},
  {":ptym23", BRACK(FHDEV(DEV_PTYM_MAJOR, 23)), "/dev/ptym23", exists_internal, S_IFCHR, false},
  {":ptym24", BRACK(FHDEV(DEV_PTYM_MAJOR, 24)), "/dev/ptym24", exists_internal, S_IFCHR, false},
  {":ptym25", BRACK(FHDEV(DEV_PTYM_MAJOR, 25)), "/dev/ptym25", exists_internal, S_IFCHR, false},
  {":ptym26", BRACK(FHDEV(DEV_PTYM_MAJOR, 26)), "/dev/ptym26", exists_internal, S_IFCHR, false},
  {":ptym27", BRACK(FHDEV(DEV_PTYM_MAJOR, 27)), "/dev/ptym27", exists_internal, S_IFCHR, false},
  {":ptym28", BRACK(FHDEV(DEV_PTYM_MAJOR, 28)), "/dev/ptym28", exists_internal, S_IFCHR, false},
  {":ptym29", BRACK(FHDEV(DEV_PTYM_MAJOR, 29)), "/dev/ptym29", exists_internal, S_IFCHR, false},
  {":ptym30", BRACK(FHDEV(DEV_PTYM_MAJOR, 30)), "/dev/ptym30", exists_internal, S_IFCHR, false},
  {":ptym31", BRACK(FHDEV(DEV_PTYM_MAJOR, 31)), "/dev/ptym31", exists_internal, S_IFCHR, false},
  {":ptym32", BRACK(FHDEV(DEV_PTYM_MAJOR, 32)), "/dev/ptym32", exists_internal, S_IFCHR, false},
  {":ptym33", BRACK(FHDEV(DEV_PTYM_MAJOR, 33)), "/dev/ptym33", exists_internal, S_IFCHR, false},
  {":ptym34", BRACK(FHDEV(DEV_PTYM_MAJOR, 34)), "/dev/ptym34", exists_internal, S_IFCHR, false},
  {":ptym35", BRACK(FHDEV(DEV_PTYM_MAJOR, 35)), "/dev/ptym35", exists_internal, S_IFCHR, false},
  {":ptym36", BRACK(FHDEV(DEV_PTYM_MAJOR, 36)), "/dev/ptym36", exists_internal, S_IFCHR, false},
  {":ptym37", BRACK(FHDEV(DEV_PTYM_MAJOR, 37)), "/dev/ptym37", exists_internal, S_IFCHR, false},
  {":ptym38", BRACK(FHDEV(DEV_PTYM_MAJOR, 38)), "/dev/ptym38", exists_internal, S_IFCHR, false},
  {":ptym39", BRACK(FHDEV(DEV_PTYM_MAJOR, 39)), "/dev/ptym39", exists_internal, S_IFCHR, false},
  {":ptym40", BRACK(FHDEV(DEV_PTYM_MAJOR, 40)), "/dev/ptym40", exists_internal, S_IFCHR, false},
  {":ptym41", BRACK(FHDEV(DEV_PTYM_MAJOR, 41)), "/dev/ptym41", exists_internal, S_IFCHR, false},
  {":ptym42", BRACK(FHDEV(DEV_PTYM_MAJOR, 42)), "/dev/ptym42", exists_internal, S_IFCHR, false},
  {":ptym43", BRACK(FHDEV(DEV_PTYM_MAJOR, 43)), "/dev/ptym43", exists_internal, S_IFCHR, false},
  {":ptym44", BRACK(FHDEV(DEV_PTYM_MAJOR, 44)), "/dev/ptym44", exists_internal, S_IFCHR, false},
  {":ptym45", BRACK(FHDEV(DEV_PTYM_MAJOR, 45)), "/dev/ptym45", exists_internal, S_IFCHR, false},
  {":ptym46", BRACK(FHDEV(DEV_PTYM_MAJOR, 46)), "/dev/ptym46", exists_internal, S_IFCHR, false},
  {":ptym47", BRACK(FHDEV(DEV_PTYM_MAJOR, 47)), "/dev/ptym47", exists_internal, S_IFCHR, false},
  {":ptym48", BRACK(FHDEV(DEV_PTYM_MAJOR, 48)), "/dev/ptym48", exists_internal, S_IFCHR, false},
  {":ptym49", BRACK(FHDEV(DEV_PTYM_MAJOR, 49)), "/dev/ptym49", exists_internal, S_IFCHR, false},
  {":ptym50", BRACK(FHDEV(DEV_PTYM_MAJOR, 50)), "/dev/ptym50", exists_internal, S_IFCHR, false},
  {":ptym51", BRACK(FHDEV(DEV_PTYM_MAJOR, 51)), "/dev/ptym51", exists_internal, S_IFCHR, false},
  {":ptym52", BRACK(FHDEV(DEV_PTYM_MAJOR, 52)), "/dev/ptym52", exists_internal, S_IFCHR, false},
  {":ptym53", BRACK(FHDEV(DEV_PTYM_MAJOR, 53)), "/dev/ptym53", exists_internal, S_IFCHR, false},
  {":ptym54", BRACK(FHDEV(DEV_PTYM_MAJOR, 54)), "/dev/ptym54", exists_internal, S_IFCHR, false},
  {":ptym55", BRACK(FHDEV(DEV_PTYM_MAJOR, 55)), "/dev/ptym55", exists_internal, S_IFCHR, false},
  {":ptym56", BRACK(FHDEV(DEV_PTYM_MAJOR, 56)), "/dev/ptym56", exists_internal, S_IFCHR, false},
  {":ptym57", BRACK(FHDEV(DEV_PTYM_MAJOR, 57)), "/dev/ptym57", exists_internal, S_IFCHR, false},
  {":ptym58", BRACK(FHDEV(DEV_PTYM_MAJOR, 58)), "/dev/ptym58", exists_internal, S_IFCHR, false},
  {":ptym59", BRACK(FHDEV(DEV_PTYM_MAJOR, 59)), "/dev/ptym59", exists_internal, S_IFCHR, false},
  {":ptym60", BRACK(FHDEV(DEV_PTYM_MAJOR, 60)), "/dev/ptym60", exists_internal, S_IFCHR, false},
  {":ptym61", BRACK(FHDEV(DEV_PTYM_MAJOR, 61)), "/dev/ptym61", exists_internal, S_IFCHR, false},
  {":ptym62", BRACK(FHDEV(DEV_PTYM_MAJOR, 62)), "/dev/ptym62", exists_internal, S_IFCHR, false},
  {":ptym63", BRACK(FHDEV(DEV_PTYM_MAJOR, 63)), "/dev/ptym63", exists_internal, S_IFCHR, false}
};

const device *cons_dev = dev_storage + 20;
const device *console_dev = dev_storage + 84;
const device *ptym_dev = dev_storage + 2577;
const device *ptys_dev = dev_storage + 234;
const device *urandom_dev = dev_storage + 2572;


static KR_device_t KR_find_keyword (const char *KR_keyword, int KR_length)
{

  switch (KR_length)
    {
    case 4:
          if (strncmp (KR_keyword, "/dev", 4) == 0)
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
    case 5:
      switch (KR_keyword [1])
        {
        case 'p':
          if (strncmp (KR_keyword, ":pipe", 5) == 0)
            {
{
return dev_storage + 2576;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'f':
          if (strncmp (KR_keyword, ":fifo", 5) == 0)
            {
{
return dev_storage + 2575;

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
    case 6:
      switch (KR_keyword [5])
        {
        case '9':
          if (strncmp (KR_keyword, ":ptym9", 6) == 0)
            {
{
return dev_storage + 2586;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '8':
          if (strncmp (KR_keyword, ":ptym8", 6) == 0)
            {
{
return dev_storage + 2585;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '7':
          if (strncmp (KR_keyword, ":ptym7", 6) == 0)
            {
{
return dev_storage + 2584;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '6':
          if (strncmp (KR_keyword, ":ptym6", 6) == 0)
            {
{
return dev_storage + 2583;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '5':
          if (strncmp (KR_keyword, ":ptym5", 6) == 0)
            {
{
return dev_storage + 2582;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '4':
          if (strncmp (KR_keyword, ":ptym4", 6) == 0)
            {
{
return dev_storage + 2581;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '3':
          if (strncmp (KR_keyword, ":ptym3", 6) == 0)
            {
{
return dev_storage + 2580;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '2':
          if (strncmp (KR_keyword, ":ptym2", 6) == 0)
            {
{
return dev_storage + 2579;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '1':
          if (strncmp (KR_keyword, ":ptym1", 6) == 0)
            {
{
return dev_storage + 2578;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '0':
          if (strncmp (KR_keyword, ":ptym0", 6) == 0)
            {
{
return dev_storage + 2577;

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
    case 7:
      switch (KR_keyword [5])
        {
        case '6':
          switch (KR_keyword [6])
            {
            case '3':
              if (strncmp (KR_keyword, ":ptym63", 7) == 0)
                {
{
return dev_storage + 2640;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym62", 7) == 0)
                {
{
return dev_storage + 2639;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, ":ptym61", 7) == 0)
                {
{
return dev_storage + 2638;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, ":ptym60", 7) == 0)
                {
{
return dev_storage + 2637;

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
            case '9':
              if (strncmp (KR_keyword, ":ptym59", 7) == 0)
                {
{
return dev_storage + 2636;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, ":ptym58", 7) == 0)
                {
{
return dev_storage + 2635;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, ":ptym57", 7) == 0)
                {
{
return dev_storage + 2634;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, ":ptym56", 7) == 0)
                {
{
return dev_storage + 2633;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, ":ptym55", 7) == 0)
                {
{
return dev_storage + 2632;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, ":ptym54", 7) == 0)
                {
{
return dev_storage + 2631;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, ":ptym53", 7) == 0)
                {
{
return dev_storage + 2630;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym52", 7) == 0)
                {
{
return dev_storage + 2629;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, ":ptym51", 7) == 0)
                {
{
return dev_storage + 2628;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, ":ptym50", 7) == 0)
                {
{
return dev_storage + 2627;

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
            case '9':
              if (strncmp (KR_keyword, ":ptym49", 7) == 0)
                {
{
return dev_storage + 2626;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, ":ptym48", 7) == 0)
                {
{
return dev_storage + 2625;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, ":ptym47", 7) == 0)
                {
{
return dev_storage + 2624;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, ":ptym46", 7) == 0)
                {
{
return dev_storage + 2623;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, ":ptym45", 7) == 0)
                {
{
return dev_storage + 2622;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, ":ptym44", 7) == 0)
                {
{
return dev_storage + 2621;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, ":ptym43", 7) == 0)
                {
{
return dev_storage + 2620;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym42", 7) == 0)
                {
{
return dev_storage + 2619;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, ":ptym41", 7) == 0)
                {
{
return dev_storage + 2618;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, ":ptym40", 7) == 0)
                {
{
return dev_storage + 2617;

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
            case '9':
              if (strncmp (KR_keyword, ":ptym39", 7) == 0)
                {
{
return dev_storage + 2616;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, ":ptym38", 7) == 0)
                {
{
return dev_storage + 2615;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, ":ptym37", 7) == 0)
                {
{
return dev_storage + 2614;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, ":ptym36", 7) == 0)
                {
{
return dev_storage + 2613;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, ":ptym35", 7) == 0)
                {
{
return dev_storage + 2612;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, ":ptym34", 7) == 0)
                {
{
return dev_storage + 2611;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, ":ptym33", 7) == 0)
                {
{
return dev_storage + 2610;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym32", 7) == 0)
                {
{
return dev_storage + 2609;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, ":ptym31", 7) == 0)
                {
{
return dev_storage + 2608;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, ":ptym30", 7) == 0)
                {
{
return dev_storage + 2607;

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
            case '9':
              if (strncmp (KR_keyword, ":ptym29", 7) == 0)
                {
{
return dev_storage + 2606;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, ":ptym28", 7) == 0)
                {
{
return dev_storage + 2605;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, ":ptym27", 7) == 0)
                {
{
return dev_storage + 2604;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, ":ptym26", 7) == 0)
                {
{
return dev_storage + 2603;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, ":ptym25", 7) == 0)
                {
{
return dev_storage + 2602;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, ":ptym24", 7) == 0)
                {
{
return dev_storage + 2601;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, ":ptym23", 7) == 0)
                {
{
return dev_storage + 2600;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym22", 7) == 0)
                {
{
return dev_storage + 2599;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, ":ptym21", 7) == 0)
                {
{
return dev_storage + 2598;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, ":ptym20", 7) == 0)
                {
{
return dev_storage + 2597;

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
            case '9':
              if (strncmp (KR_keyword, ":ptym19", 7) == 0)
                {
{
return dev_storage + 2596;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, ":ptym18", 7) == 0)
                {
{
return dev_storage + 2595;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, ":ptym17", 7) == 0)
                {
{
return dev_storage + 2594;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, ":ptym16", 7) == 0)
                {
{
return dev_storage + 2593;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, ":ptym15", 7) == 0)
                {
{
return dev_storage + 2592;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, ":ptym14", 7) == 0)
                {
{
return dev_storage + 2591;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, ":ptym13", 7) == 0)
                {
{
return dev_storage + 2590;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym12", 7) == 0)
                {
{
return dev_storage + 2589;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, ":ptym11", 7) == 0)
                {
{
return dev_storage + 2588;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, ":ptym10", 7) == 0)
                {
{
return dev_storage + 2587;

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
    case 8:
      switch (KR_keyword [7])
        {
        case 'z':
          if (strncmp (KR_keyword, "/dev/sdz", 8) == 0)
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
        case 'y':
          switch (KR_keyword [5])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/tty", 8) == 0)
                {
{
return dev_storage + 2507;

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
return dev_storage + 339;

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
return dev_storage + 338;

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
return dev_storage + 337;

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
return dev_storage + 336;

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
return dev_storage + 335;

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
return dev_storage + 334;

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
return dev_storage + 333;

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
return dev_storage + 332;

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
return dev_storage + 331;

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
return dev_storage + 330;

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
return dev_storage + 85;

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
return dev_storage + 329;

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
return dev_storage + 328;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'm':
          if (strncmp (KR_keyword, "/dev/sdm", 8) == 0)
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
        case 'l':
          if (strncmp (KR_keyword, "/dev/sdl", 8) == 0)
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
        case 'k':
          if (strncmp (KR_keyword, "/dev/sdk", 8) == 0)
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
        case 'j':
          if (strncmp (KR_keyword, "/dev/sdj", 8) == 0)
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
        case 'i':
          if (strncmp (KR_keyword, "/dev/sdi", 8) == 0)
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
        case 'h':
          if (strncmp (KR_keyword, "/dev/sdh", 8) == 0)
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
          if (strncmp (KR_keyword, "/dev/sdg", 8) == 0)
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
        case 'f':
          if (strncmp (KR_keyword, "/dev/sdf", 8) == 0)
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
        case 'e':
          if (strncmp (KR_keyword, "/dev/sde", 8) == 0)
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
        case 'd':
          if (strncmp (KR_keyword, "/dev/sdd", 8) == 0)
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
        case 'c':
          if (strncmp (KR_keyword, "/dev/sdc", 8) == 0)
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
        case 'b':
          if (strncmp (KR_keyword, "/dev/sdb", 8) == 0)
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
        case 'a':
          if (strncmp (KR_keyword, "/dev/sda", 8) == 0)
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
        case '9':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st9", 8) == 0)
                {
{
return dev_storage + 2388;

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
return dev_storage + 2372;

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
return dev_storage + 95;

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
return dev_storage + 2387;

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
return dev_storage + 2371;

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
return dev_storage + 94;

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
return dev_storage + 2386;

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
return dev_storage + 2370;

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
return dev_storage + 93;

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
return dev_storage + 2385;

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
return dev_storage + 2369;

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
return dev_storage + 92;

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
return dev_storage + 2384;

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
return dev_storage + 2368;

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
return dev_storage + 91;

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
return dev_storage + 2383;

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
return dev_storage + 2367;

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
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st3", 8) == 0)
                {
{
return dev_storage + 2382;

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
return dev_storage + 2366;

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
        case '2':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st2", 8) == 0)
                {
{
return dev_storage + 2381;

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
return dev_storage + 2365;

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
        case '1':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st1", 8) == 0)
                {
{
return dev_storage + 2380;

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
return dev_storage + 2364;

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
        case '0':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st0", 8) == 0)
                {
{
return dev_storage + 2379;

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
return dev_storage + 2363;

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
return dev_storage + 86;

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
      switch (KR_keyword [7])
        {
        case 'z':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdz9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdz8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdz7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdz6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdz5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdz4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdz3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdz2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdz1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'y':
          switch (KR_keyword [8])
            {
            case '9':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy9", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty9", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '8':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy8", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty8", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '7':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy7", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty7", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '6':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy6", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty6", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy5", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty5", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy4", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty4", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy3", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty3", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy2", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty2", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdy1", 9) == 0)
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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty1", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/pty0", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'x':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdx9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdx8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdx7", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdx6", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdx5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdx4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdx3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdx2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdx1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'w':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdw9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdw8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdw7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdw6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdw5", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdw4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdw3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdw2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdw1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'v':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdv9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdv8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdv7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdv6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdv5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdv4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdv3", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdv2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdv1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'u':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdu9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdu8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdu7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdu6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdu5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdu4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdu3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdu2", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdu1", 9) == 0)
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
        case 't':
          switch (KR_keyword [8])
            {
            case '9':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt9", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst9", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '8':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt8", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst8", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '7':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt7", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst7", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '6':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt6", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst6", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt5", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst5", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt4", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst4", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt3", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst3", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt2", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst2", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdt1", 9) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst1", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/nst0", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 's':
          switch (KR_keyword [8])
            {
            case 'g':
              if (strncmp (KR_keyword, "/dev/kmsg", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/sds9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sds8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sds7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sds6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sds5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sds4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sds3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sds2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sds1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'r':
          switch (KR_keyword [8])
            {
            case 'o':
              if (strncmp (KR_keyword, "/dev/zero", 9) == 0)
                {
{
return dev_storage + 2574;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/sdr9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdr8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdr7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdr6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdr5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdr4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdr3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdr2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdr1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'q':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdq9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdq8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdq7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdq6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdq5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdq4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdq3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdq2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdq1", 9) == 0)
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
        case 'p':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdp9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdp8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdp7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdp6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdp5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdp4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdp3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdp2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdp1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'o':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdo9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdo8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdo7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdo6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdo5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdo4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdo3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdo2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdo1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'n':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdn9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdn8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdn7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdn6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdn5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdn4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdn3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdn2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdn1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'm':
          switch (KR_keyword [5])
            {
            case 's':
              switch (KR_keyword [8])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/sdm9", 9) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/sdm8", 9) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/sdm7", 9) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/sdm6", 9) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/sdm5", 9) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/sdm4", 9) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/sdm3", 9) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/sdm2", 9) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdm1", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/ptmx", 9) == 0)
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
            case 'c':
              switch (KR_keyword [8])
                {
                case '9':
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
                case '8':
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
                case '7':
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
                case '6':
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
                case '5':
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
                case '4':
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
                case '3':
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
                case '2':
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
                case '1':
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
            default:
{
return	NULL;

}
            }
        case 'l':
          switch (KR_keyword [8])
            {
            case 'l':
              switch (KR_keyword [5])
                {
                case 'n':
                  if (strncmp (KR_keyword, "/dev/null", 9) == 0)
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
                case 'f':
                  if (strncmp (KR_keyword, "/dev/full", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/sdl9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdl8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdl7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdl6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdl5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdl4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdl3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdl2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdl1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'k':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdk9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdk8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdk7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdk6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdk5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdk4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdk3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdk2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdk1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'j':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdj9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdj8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdj7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdj6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdj5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdj4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdj3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdj2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdj1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'i':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdi9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdi8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdi7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdi6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdi5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdi4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdi3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdi2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdi1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'h':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdh9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdh8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdh7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdh6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdh5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdh4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdh3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdh2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdh1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'g':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdg9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdg8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdg7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdg6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdg5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdg4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdg3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdg2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdg1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'f':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sdf9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdf8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdf7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdf6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdf5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdf4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdf3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdf2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdf1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'e':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/sde9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sde8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sde7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sde6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sde5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sde4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sde3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sde2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sde1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'd':
          switch (KR_keyword [8])
            {
            case 'x':
              if (strncmp (KR_keyword, "/dev/sddx", 9) == 0)
                {
{
return dev_storage + 2002;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sddw", 9) == 0)
                {
{
return dev_storage + 2001;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sddv", 9) == 0)
                {
{
return dev_storage + 2000;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sddu", 9) == 0)
                {
{
return dev_storage + 1999;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              if (strncmp (KR_keyword, "/dev/sddt", 9) == 0)
                {
{
return dev_storage + 1998;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sdds", 9) == 0)
                {
{
return dev_storage + 1997;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sddr", 9) == 0)
                {
{
return dev_storage + 1996;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sddq", 9) == 0)
                {
{
return dev_storage + 1995;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sddp", 9) == 0)
                {
{
return dev_storage + 1994;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sddo", 9) == 0)
                {
{
return dev_storage + 1993;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sddn", 9) == 0)
                {
{
return dev_storage + 1992;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              if (strncmp (KR_keyword, "/dev/sddm", 9) == 0)
                {
{
return dev_storage + 1991;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sddl", 9) == 0)
                {
{
return dev_storage + 1990;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sddk", 9) == 0)
                {
{
return dev_storage + 1989;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sddj", 9) == 0)
                {
{
return dev_storage + 1988;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sddi", 9) == 0)
                {
{
return dev_storage + 1987;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sddh", 9) == 0)
                {
{
return dev_storage + 1986;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sddg", 9) == 0)
                {
{
return dev_storage + 1985;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sddf", 9) == 0)
                {
{
return dev_storage + 1984;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sdde", 9) == 0)
                {
{
return dev_storage + 1983;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/sddd", 9) == 0)
                {
{
return dev_storage + 1982;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sddc", 9) == 0)
                {
{
return dev_storage + 1981;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sddb", 9) == 0)
                {
{
return dev_storage + 1980;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sdda", 9) == 0)
                {
{
return dev_storage + 1979;

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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd9", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd9", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '8':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd8", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd8", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '7':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd7", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd7", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '6':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd6", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd6", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd5", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd5", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd4", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd4", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd3", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd3", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd2", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd2", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [6])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdd1", 9) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/scd1", 9) == 0)
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
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/scd0", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'c':
          switch (KR_keyword [8])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdcz", 9) == 0)
                {
{
return dev_storage + 1588;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              if (strncmp (KR_keyword, "/dev/sdcy", 9) == 0)
                {
{
return dev_storage + 1587;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdcx", 9) == 0)
                {
{
return dev_storage + 1586;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdcw", 9) == 0)
                {
{
return dev_storage + 1585;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdcv", 9) == 0)
                {
{
return dev_storage + 1584;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdcu", 9) == 0)
                {
{
return dev_storage + 1583;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              if (strncmp (KR_keyword, "/dev/sdct", 9) == 0)
                {
{
return dev_storage + 1582;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sdcs", 9) == 0)
                {
{
return dev_storage + 1581;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdcr", 9) == 0)
                {
{
return dev_storage + 1580;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdcq", 9) == 0)
                {
{
return dev_storage + 1579;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdcp", 9) == 0)
                {
{
return dev_storage + 1578;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdco", 9) == 0)
                {
{
return dev_storage + 1577;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdcn", 9) == 0)
                {
{
return dev_storage + 1576;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              if (strncmp (KR_keyword, "/dev/sdcm", 9) == 0)
                {
{
return dev_storage + 1575;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdcl", 9) == 0)
                {
{
return dev_storage + 1574;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdck", 9) == 0)
                {
{
return dev_storage + 1573;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdcj", 9) == 0)
                {
{
return dev_storage + 1572;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdci", 9) == 0)
                {
{
return dev_storage + 1571;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdch", 9) == 0)
                {
{
return dev_storage + 1570;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdcg", 9) == 0)
                {
{
return dev_storage + 1569;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdcf", 9) == 0)
                {
{
return dev_storage + 1568;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sdce", 9) == 0)
                {
{
return dev_storage + 1567;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/sdcd", 9) == 0)
                {
{
return dev_storage + 1566;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdcc", 9) == 0)
                {
{
return dev_storage + 1565;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdcb", 9) == 0)
                {
{
return dev_storage + 1564;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sdca", 9) == 0)
                {
{
return dev_storage + 1563;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/sdc9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdc8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdc7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdc6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdc5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdc4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdc3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdc2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdc1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'b':
          switch (KR_keyword [8])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdbz", 9) == 0)
                {
{
return dev_storage + 1172;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'y':
              if (strncmp (KR_keyword, "/dev/sdby", 9) == 0)
                {
{
return dev_storage + 1171;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdbx", 9) == 0)
                {
{
return dev_storage + 1170;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdbw", 9) == 0)
                {
{
return dev_storage + 1169;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdbv", 9) == 0)
                {
{
return dev_storage + 1168;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdbu", 9) == 0)
                {
{
return dev_storage + 1167;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 't':
              if (strncmp (KR_keyword, "/dev/sdbt", 9) == 0)
                {
{
return dev_storage + 1166;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 's':
              if (strncmp (KR_keyword, "/dev/sdbs", 9) == 0)
                {
{
return dev_storage + 1165;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdbr", 9) == 0)
                {
{
return dev_storage + 1164;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdbq", 9) == 0)
                {
{
return dev_storage + 1163;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdbp", 9) == 0)
                {
{
return dev_storage + 1162;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdbo", 9) == 0)
                {
{
return dev_storage + 1161;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdbn", 9) == 0)
                {
{
return dev_storage + 1160;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'm':
              if (strncmp (KR_keyword, "/dev/sdbm", 9) == 0)
                {
{
return dev_storage + 1159;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdbl", 9) == 0)
                {
{
return dev_storage + 1158;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdbk", 9) == 0)
                {
{
return dev_storage + 1157;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdbj", 9) == 0)
                {
{
return dev_storage + 1156;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdbi", 9) == 0)
                {
{
return dev_storage + 1155;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdbh", 9) == 0)
                {
{
return dev_storage + 1154;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdbg", 9) == 0)
                {
{
return dev_storage + 1153;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdbf", 9) == 0)
                {
{
return dev_storage + 1152;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'e':
              if (strncmp (KR_keyword, "/dev/sdbe", 9) == 0)
                {
{
return dev_storage + 1151;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              if (strncmp (KR_keyword, "/dev/sdbd", 9) == 0)
                {
{
return dev_storage + 1150;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdbc", 9) == 0)
                {
{
return dev_storage + 1149;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdbb", 9) == 0)
                {
{
return dev_storage + 1148;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'a':
              if (strncmp (KR_keyword, "/dev/sdba", 9) == 0)
                {
{
return dev_storage + 1147;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '9':
              if (strncmp (KR_keyword, "/dev/sdb9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sdb8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sdb7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sdb6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sdb5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sdb4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sdb3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sdb2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sdb1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case 'a':
          switch (KR_keyword [8])
            {
            case 'z':
              if (strncmp (KR_keyword, "/dev/sdaz", 9) == 0)
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
            case 'y':
              if (strncmp (KR_keyword, "/dev/sday", 9) == 0)
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
            case 'x':
              if (strncmp (KR_keyword, "/dev/sdax", 9) == 0)
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
            case 'w':
              if (strncmp (KR_keyword, "/dev/sdaw", 9) == 0)
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
            case 'v':
              if (strncmp (KR_keyword, "/dev/sdav", 9) == 0)
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
            case 'u':
              if (strncmp (KR_keyword, "/dev/sdau", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdat", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdas", 9) == 0)
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
            case 'r':
              if (strncmp (KR_keyword, "/dev/sdar", 9) == 0)
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
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdaq", 9) == 0)
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
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdap", 9) == 0)
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
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdao", 9) == 0)
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
            case 'n':
              if (strncmp (KR_keyword, "/dev/sdan", 9) == 0)
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
            case 'm':
              if (strncmp (KR_keyword, "/dev/sdam", 9) == 0)
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
            case 'l':
              if (strncmp (KR_keyword, "/dev/sdal", 9) == 0)
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
            case 'k':
              if (strncmp (KR_keyword, "/dev/sdak", 9) == 0)
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
            case 'j':
              if (strncmp (KR_keyword, "/dev/sdaj", 9) == 0)
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
            case 'i':
              if (strncmp (KR_keyword, "/dev/sdai", 9) == 0)
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
            case 'h':
              if (strncmp (KR_keyword, "/dev/sdah", 9) == 0)
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
            case 'g':
              if (strncmp (KR_keyword, "/dev/sdag", 9) == 0)
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
            case 'f':
              if (strncmp (KR_keyword, "/dev/sdaf", 9) == 0)
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
            case 'e':
              if (strncmp (KR_keyword, "/dev/sdae", 9) == 0)
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/sdad", 9) == 0)
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
            case 'c':
              if (strncmp (KR_keyword, "/dev/sdac", 9) == 0)
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
            case 'b':
              if (strncmp (KR_keyword, "/dev/sdab", 9) == 0)
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
            case 'a':
              if (strncmp (KR_keyword, "/dev/sdaa", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/sda9", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/sda8", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/sda7", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/sda6", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/sda5", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/sda4", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/sda3", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/sda2", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/sda1", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '9':
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/st99", 9) == 0)
                {
{
return dev_storage + 2478;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st98", 9) == 0)
                {
{
return dev_storage + 2477;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st97", 9) == 0)
                {
{
return dev_storage + 2476;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st96", 9) == 0)
                {
{
return dev_storage + 2475;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st95", 9) == 0)
                {
{
return dev_storage + 2474;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st94", 9) == 0)
                {
{
return dev_storage + 2473;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st93", 9) == 0)
                {
{
return dev_storage + 2472;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st92", 9) == 0)
                {
{
return dev_storage + 2471;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st91", 9) == 0)
                {
{
return dev_storage + 2470;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st90", 9) == 0)
                {
{
return dev_storage + 2469;

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
            case '9':
              if (strncmp (KR_keyword, "/dev/st89", 9) == 0)
                {
{
return dev_storage + 2468;

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
return dev_storage + 2467;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st87", 9) == 0)
                {
{
return dev_storage + 2466;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st86", 9) == 0)
                {
{
return dev_storage + 2465;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st85", 9) == 0)
                {
{
return dev_storage + 2464;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st84", 9) == 0)
                {
{
return dev_storage + 2463;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st83", 9) == 0)
                {
{
return dev_storage + 2462;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st82", 9) == 0)
                {
{
return dev_storage + 2461;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st81", 9) == 0)
                {
{
return dev_storage + 2460;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st80", 9) == 0)
                {
{
return dev_storage + 2459;

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
            case '9':
              if (strncmp (KR_keyword, "/dev/st79", 9) == 0)
                {
{
return dev_storage + 2458;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st78", 9) == 0)
                {
{
return dev_storage + 2457;

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
return dev_storage + 2456;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st76", 9) == 0)
                {
{
return dev_storage + 2455;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st75", 9) == 0)
                {
{
return dev_storage + 2454;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st74", 9) == 0)
                {
{
return dev_storage + 2453;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st73", 9) == 0)
                {
{
return dev_storage + 2452;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st72", 9) == 0)
                {
{
return dev_storage + 2451;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st71", 9) == 0)
                {
{
return dev_storage + 2450;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st70", 9) == 0)
                {
{
return dev_storage + 2449;

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
            case '9':
              if (strncmp (KR_keyword, "/dev/st69", 9) == 0)
                {
{
return dev_storage + 2448;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st68", 9) == 0)
                {
{
return dev_storage + 2447;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st67", 9) == 0)
                {
{
return dev_storage + 2446;

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
return dev_storage + 2445;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st65", 9) == 0)
                {
{
return dev_storage + 2444;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st64", 9) == 0)
                {
{
return dev_storage + 2443;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st63", 9) == 0)
                {
{
return dev_storage + 2442;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st62", 9) == 0)
                {
{
return dev_storage + 2441;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st61", 9) == 0)
                {
{
return dev_storage + 2440;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st60", 9) == 0)
                {
{
return dev_storage + 2439;

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
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/st59", 9) == 0)
                {
{
return dev_storage + 2438;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st58", 9) == 0)
                {
{
return dev_storage + 2437;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st57", 9) == 0)
                {
{
return dev_storage + 2436;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st56", 9) == 0)
                {
{
return dev_storage + 2435;

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
return dev_storage + 2434;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st54", 9) == 0)
                {
{
return dev_storage + 2433;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st53", 9) == 0)
                {
{
return dev_storage + 2432;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st52", 9) == 0)
                {
{
return dev_storage + 2431;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st51", 9) == 0)
                {
{
return dev_storage + 2430;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st50", 9) == 0)
                {
{
return dev_storage + 2429;

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
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/st49", 9) == 0)
                {
{
return dev_storage + 2428;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st48", 9) == 0)
                {
{
return dev_storage + 2427;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st47", 9) == 0)
                {
{
return dev_storage + 2426;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st46", 9) == 0)
                {
{
return dev_storage + 2425;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st45", 9) == 0)
                {
{
return dev_storage + 2424;

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
return dev_storage + 2423;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st43", 9) == 0)
                {
{
return dev_storage + 2422;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st42", 9) == 0)
                {
{
return dev_storage + 2421;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st41", 9) == 0)
                {
{
return dev_storage + 2420;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st40", 9) == 0)
                {
{
return dev_storage + 2419;

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
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/st39", 9) == 0)
                {
{
return dev_storage + 2418;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st38", 9) == 0)
                {
{
return dev_storage + 2417;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st37", 9) == 0)
                {
{
return dev_storage + 2416;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st36", 9) == 0)
                {
{
return dev_storage + 2415;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st35", 9) == 0)
                {
{
return dev_storage + 2414;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st34", 9) == 0)
                {
{
return dev_storage + 2413;

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
return dev_storage + 2412;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, "/dev/st32", 9) == 0)
                {
{
return dev_storage + 2411;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st31", 9) == 0)
                {
{
return dev_storage + 2410;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st30", 9) == 0)
                {
{
return dev_storage + 2409;

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
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/st29", 9) == 0)
                {
{
return dev_storage + 2408;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st28", 9) == 0)
                {
{
return dev_storage + 2407;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st27", 9) == 0)
                {
{
return dev_storage + 2406;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st26", 9) == 0)
                {
{
return dev_storage + 2405;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              if (strncmp (KR_keyword, "/dev/st25", 9) == 0)
                {
{
return dev_storage + 2404;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '4':
              if (strncmp (KR_keyword, "/dev/st24", 9) == 0)
                {
{
return dev_storage + 2403;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '3':
              if (strncmp (KR_keyword, "/dev/st23", 9) == 0)
                {
{
return dev_storage + 2402;

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
return dev_storage + 2401;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '1':
              if (strncmp (KR_keyword, "/dev/st21", 9) == 0)
                {
{
return dev_storage + 2400;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st20", 9) == 0)
                {
{
return dev_storage + 2399;

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
          switch (KR_keyword [8])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/st19", 9) == 0)
                {
{
return dev_storage + 2398;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '8':
              if (strncmp (KR_keyword, "/dev/st18", 9) == 0)
                {
{
return dev_storage + 2397;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '7':
              if (strncmp (KR_keyword, "/dev/st17", 9) == 0)
                {
{
return dev_storage + 2396;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '6':
              if (strncmp (KR_keyword, "/dev/st16", 9) == 0)
                {
{
return dev_storage + 2395;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '5':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st15", 9) == 0)
                    {
{
return dev_storage + 2394;

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
return dev_storage + 2378;

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
return dev_storage + 101;

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
                  if (strncmp (KR_keyword, "/dev/st14", 9) == 0)
                    {
{
return dev_storage + 2393;

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
return dev_storage + 2377;

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
            case '3':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st13", 9) == 0)
                    {
{
return dev_storage + 2392;

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
return dev_storage + 2376;

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
            case '2':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st12", 9) == 0)
                    {
{
return dev_storage + 2391;

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
return dev_storage + 2375;

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
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st11", 9) == 0)
                    {
{
return dev_storage + 2390;

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
return dev_storage + 2374;

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
            case '0':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st10", 9) == 0)
                    {
{
return dev_storage + 2389;

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
return dev_storage + 2373;

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
return dev_storage + 96;

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
            case 'z':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcz9", 10) == 0)
                    {
{
return dev_storage + 1972;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbz9", 10) == 0)
                    {
{
return dev_storage + 1556;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaz9", 10) == 0)
                    {
{
return dev_storage + 1140;

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
            case 'y':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcy9", 10) == 0)
                    {
{
return dev_storage + 1957;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdby9", 10) == 0)
                    {
{
return dev_storage + 1541;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sday9", 10) == 0)
                    {
{
return dev_storage + 1125;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddx9", 10) == 0)
                    {
{
return dev_storage + 2356;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcx9", 10) == 0)
                    {
{
return dev_storage + 1942;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbx9", 10) == 0)
                    {
{
return dev_storage + 1526;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdax9", 10) == 0)
                    {
{
return dev_storage + 1110;

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
            case 'w':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddw9", 10) == 0)
                    {
{
return dev_storage + 2341;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcw9", 10) == 0)
                    {
{
return dev_storage + 1927;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbw9", 10) == 0)
                    {
{
return dev_storage + 1511;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaw9", 10) == 0)
                    {
{
return dev_storage + 1095;

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
            case 'v':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddv9", 10) == 0)
                    {
{
return dev_storage + 2326;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcv9", 10) == 0)
                    {
{
return dev_storage + 1912;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbv9", 10) == 0)
                    {
{
return dev_storage + 1496;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdav9", 10) == 0)
                    {
{
return dev_storage + 1080;

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
            case 'u':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddu9", 10) == 0)
                    {
{
return dev_storage + 2311;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcu9", 10) == 0)
                    {
{
return dev_storage + 1897;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbu9", 10) == 0)
                    {
{
return dev_storage + 1481;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdau9", 10) == 0)
                    {
{
return dev_storage + 1065;

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
            case 't':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddt9", 10) == 0)
                    {
{
return dev_storage + 2296;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdct9", 10) == 0)
                    {
{
return dev_storage + 1882;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbt9", 10) == 0)
                    {
{
return dev_storage + 1466;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdat9", 10) == 0)
                    {
{
return dev_storage + 1050;

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
              switch (KR_keyword [7])
                {
                case 'n':
                  if (strncmp (KR_keyword, "/dev/cons9", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdds9", 10) == 0)
                    {
{
return dev_storage + 2281;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcs9", 10) == 0)
                    {
{
return dev_storage + 1867;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbs9", 10) == 0)
                    {
{
return dev_storage + 1451;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdas9", 10) == 0)
                    {
{
return dev_storage + 1035;

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
            case 'r':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddr9", 10) == 0)
                    {
{
return dev_storage + 2266;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcr9", 10) == 0)
                    {
{
return dev_storage + 1852;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbr9", 10) == 0)
                    {
{
return dev_storage + 1436;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdar9", 10) == 0)
                    {
{
return dev_storage + 1020;

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
            case 'q':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddq9", 10) == 0)
                    {
{
return dev_storage + 2251;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcq9", 10) == 0)
                    {
{
return dev_storage + 1837;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbq9", 10) == 0)
                    {
{
return dev_storage + 1421;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaq9", 10) == 0)
                    {
{
return dev_storage + 1005;

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
            case 'p':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddp9", 10) == 0)
                    {
{
return dev_storage + 2236;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcp9", 10) == 0)
                    {
{
return dev_storage + 1822;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbp9", 10) == 0)
                    {
{
return dev_storage + 1406;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdap9", 10) == 0)
                    {
{
return dev_storage + 990;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddo9", 10) == 0)
                    {
{
return dev_storage + 2221;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdco9", 10) == 0)
                    {
{
return dev_storage + 1807;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbo9", 10) == 0)
                    {
{
return dev_storage + 1391;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdao9", 10) == 0)
                    {
{
return dev_storage + 975;

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
            case 'n':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddn9", 10) == 0)
                    {
{
return dev_storage + 2206;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcn9", 10) == 0)
                    {
{
return dev_storage + 1792;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbn9", 10) == 0)
                    {
{
return dev_storage + 1376;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdan9", 10) == 0)
                    {
{
return dev_storage + 960;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddm9", 10) == 0)
                    {
{
return dev_storage + 2191;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcm9", 10) == 0)
                    {
{
return dev_storage + 1777;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbm9", 10) == 0)
                    {
{
return dev_storage + 1361;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdam9", 10) == 0)
                    {
{
return dev_storage + 945;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddl9", 10) == 0)
                    {
{
return dev_storage + 2176;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcl9", 10) == 0)
                    {
{
return dev_storage + 1762;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbl9", 10) == 0)
                    {
{
return dev_storage + 1346;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdal9", 10) == 0)
                    {
{
return dev_storage + 930;

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
            case 'k':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddk9", 10) == 0)
                    {
{
return dev_storage + 2161;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdck9", 10) == 0)
                    {
{
return dev_storage + 1747;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbk9", 10) == 0)
                    {
{
return dev_storage + 1331;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdak9", 10) == 0)
                    {
{
return dev_storage + 915;

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
            case 'j':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddj9", 10) == 0)
                    {
{
return dev_storage + 2146;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcj9", 10) == 0)
                    {
{
return dev_storage + 1732;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbj9", 10) == 0)
                    {
{
return dev_storage + 1316;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaj9", 10) == 0)
                    {
{
return dev_storage + 900;

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
            case 'i':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddi9", 10) == 0)
                    {
{
return dev_storage + 2131;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdci9", 10) == 0)
                    {
{
return dev_storage + 1717;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbi9", 10) == 0)
                    {
{
return dev_storage + 1301;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdai9", 10) == 0)
                    {
{
return dev_storage + 885;

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
            case 'h':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddh9", 10) == 0)
                    {
{
return dev_storage + 2116;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdch9", 10) == 0)
                    {
{
return dev_storage + 1702;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbh9", 10) == 0)
                    {
{
return dev_storage + 1286;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdah9", 10) == 0)
                    {
{
return dev_storage + 870;

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
            case 'g':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddg9", 10) == 0)
                    {
{
return dev_storage + 2101;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcg9", 10) == 0)
                    {
{
return dev_storage + 1687;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbg9", 10) == 0)
                    {
{
return dev_storage + 1271;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdag9", 10) == 0)
                    {
{
return dev_storage + 855;

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
            case 'f':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddf9", 10) == 0)
                    {
{
return dev_storage + 2086;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcf9", 10) == 0)
                    {
{
return dev_storage + 1672;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbf9", 10) == 0)
                    {
{
return dev_storage + 1256;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaf9", 10) == 0)
                    {
{
return dev_storage + 840;

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
            case 'e':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdde9", 10) == 0)
                    {
{
return dev_storage + 2071;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdce9", 10) == 0)
                    {
{
return dev_storage + 1657;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbe9", 10) == 0)
                    {
{
return dev_storage + 1241;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdae9", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd9", 10) == 0)
                    {
{
return dev_storage + 2056;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcd9", 10) == 0)
                    {
{
return dev_storage + 1642;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbd9", 10) == 0)
                    {
{
return dev_storage + 1226;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdad9", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddc9", 10) == 0)
                    {
{
return dev_storage + 2041;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc9", 10) == 0)
                    {
{
return dev_storage + 1627;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbc9", 10) == 0)
                    {
{
return dev_storage + 1211;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdac9", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddb9", 10) == 0)
                    {
{
return dev_storage + 2026;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcb9", 10) == 0)
                    {
{
return dev_storage + 1612;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb9", 10) == 0)
                    {
{
return dev_storage + 1196;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdab9", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdda9", 10) == 0)
                    {
{
return dev_storage + 2011;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdca9", 10) == 0)
                    {
{
return dev_storage + 1597;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdba9", 10) == 0)
                    {
{
return dev_storage + 1181;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa9", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS9", 10) == 0)
                {
{
return dev_storage + 2517;

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
return dev_storage + 203;

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
return dev_storage + 193;

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
return dev_storage + 183;

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
return dev_storage + 173;

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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty59", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst59", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty49", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst49", 10) == 0)
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
            case '3':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty39", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst39", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty29", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst29", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/st119", 10) == 0)
                    {
{
return dev_storage + 2498;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty19", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst19", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st109", 10) == 0)
                {
{
return dev_storage + 2488;

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
            case 'z':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcz8", 10) == 0)
                    {
{
return dev_storage + 1971;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbz8", 10) == 0)
                    {
{
return dev_storage + 1555;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaz8", 10) == 0)
                    {
{
return dev_storage + 1139;

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
            case 'y':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcy8", 10) == 0)
                    {
{
return dev_storage + 1956;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdby8", 10) == 0)
                    {
{
return dev_storage + 1540;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sday8", 10) == 0)
                    {
{
return dev_storage + 1124;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddx8", 10) == 0)
                    {
{
return dev_storage + 2355;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcx8", 10) == 0)
                    {
{
return dev_storage + 1941;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbx8", 10) == 0)
                    {
{
return dev_storage + 1525;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdax8", 10) == 0)
                    {
{
return dev_storage + 1109;

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
            case 'w':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddw8", 10) == 0)
                    {
{
return dev_storage + 2340;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcw8", 10) == 0)
                    {
{
return dev_storage + 1926;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbw8", 10) == 0)
                    {
{
return dev_storage + 1510;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaw8", 10) == 0)
                    {
{
return dev_storage + 1094;

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
            case 'v':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddv8", 10) == 0)
                    {
{
return dev_storage + 2325;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcv8", 10) == 0)
                    {
{
return dev_storage + 1911;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbv8", 10) == 0)
                    {
{
return dev_storage + 1495;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdav8", 10) == 0)
                    {
{
return dev_storage + 1079;

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
            case 'u':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddu8", 10) == 0)
                    {
{
return dev_storage + 2310;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcu8", 10) == 0)
                    {
{
return dev_storage + 1896;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbu8", 10) == 0)
                    {
{
return dev_storage + 1480;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdau8", 10) == 0)
                    {
{
return dev_storage + 1064;

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
            case 't':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddt8", 10) == 0)
                    {
{
return dev_storage + 2295;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdct8", 10) == 0)
                    {
{
return dev_storage + 1881;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbt8", 10) == 0)
                    {
{
return dev_storage + 1465;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdat8", 10) == 0)
                    {
{
return dev_storage + 1049;

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
              switch (KR_keyword [7])
                {
                case 'n':
                  if (strncmp (KR_keyword, "/dev/cons8", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdds8", 10) == 0)
                    {
{
return dev_storage + 2280;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcs8", 10) == 0)
                    {
{
return dev_storage + 1866;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbs8", 10) == 0)
                    {
{
return dev_storage + 1450;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdas8", 10) == 0)
                    {
{
return dev_storage + 1034;

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
            case 'r':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddr8", 10) == 0)
                    {
{
return dev_storage + 2265;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcr8", 10) == 0)
                    {
{
return dev_storage + 1851;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbr8", 10) == 0)
                    {
{
return dev_storage + 1435;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdar8", 10) == 0)
                    {
{
return dev_storage + 1019;

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
            case 'q':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddq8", 10) == 0)
                    {
{
return dev_storage + 2250;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcq8", 10) == 0)
                    {
{
return dev_storage + 1836;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbq8", 10) == 0)
                    {
{
return dev_storage + 1420;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaq8", 10) == 0)
                    {
{
return dev_storage + 1004;

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
            case 'p':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddp8", 10) == 0)
                    {
{
return dev_storage + 2235;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcp8", 10) == 0)
                    {
{
return dev_storage + 1821;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbp8", 10) == 0)
                    {
{
return dev_storage + 1405;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdap8", 10) == 0)
                    {
{
return dev_storage + 989;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddo8", 10) == 0)
                    {
{
return dev_storage + 2220;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdco8", 10) == 0)
                    {
{
return dev_storage + 1806;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbo8", 10) == 0)
                    {
{
return dev_storage + 1390;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdao8", 10) == 0)
                    {
{
return dev_storage + 974;

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
            case 'n':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddn8", 10) == 0)
                    {
{
return dev_storage + 2205;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcn8", 10) == 0)
                    {
{
return dev_storage + 1791;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbn8", 10) == 0)
                    {
{
return dev_storage + 1375;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdan8", 10) == 0)
                    {
{
return dev_storage + 959;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddm8", 10) == 0)
                    {
{
return dev_storage + 2190;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcm8", 10) == 0)
                    {
{
return dev_storage + 1776;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbm8", 10) == 0)
                    {
{
return dev_storage + 1360;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdam8", 10) == 0)
                    {
{
return dev_storage + 944;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddl8", 10) == 0)
                    {
{
return dev_storage + 2175;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcl8", 10) == 0)
                    {
{
return dev_storage + 1761;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbl8", 10) == 0)
                    {
{
return dev_storage + 1345;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdal8", 10) == 0)
                    {
{
return dev_storage + 929;

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
            case 'k':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddk8", 10) == 0)
                    {
{
return dev_storage + 2160;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdck8", 10) == 0)
                    {
{
return dev_storage + 1746;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbk8", 10) == 0)
                    {
{
return dev_storage + 1330;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdak8", 10) == 0)
                    {
{
return dev_storage + 914;

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
            case 'j':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddj8", 10) == 0)
                    {
{
return dev_storage + 2145;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcj8", 10) == 0)
                    {
{
return dev_storage + 1731;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbj8", 10) == 0)
                    {
{
return dev_storage + 1315;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaj8", 10) == 0)
                    {
{
return dev_storage + 899;

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
            case 'i':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddi8", 10) == 0)
                    {
{
return dev_storage + 2130;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdci8", 10) == 0)
                    {
{
return dev_storage + 1716;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbi8", 10) == 0)
                    {
{
return dev_storage + 1300;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdai8", 10) == 0)
                    {
{
return dev_storage + 884;

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
            case 'h':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddh8", 10) == 0)
                    {
{
return dev_storage + 2115;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdch8", 10) == 0)
                    {
{
return dev_storage + 1701;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbh8", 10) == 0)
                    {
{
return dev_storage + 1285;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdah8", 10) == 0)
                    {
{
return dev_storage + 869;

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
            case 'g':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddg8", 10) == 0)
                    {
{
return dev_storage + 2100;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcg8", 10) == 0)
                    {
{
return dev_storage + 1686;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbg8", 10) == 0)
                    {
{
return dev_storage + 1270;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdag8", 10) == 0)
                    {
{
return dev_storage + 854;

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
            case 'f':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddf8", 10) == 0)
                    {
{
return dev_storage + 2085;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcf8", 10) == 0)
                    {
{
return dev_storage + 1671;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbf8", 10) == 0)
                    {
{
return dev_storage + 1255;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaf8", 10) == 0)
                    {
{
return dev_storage + 839;

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
            case 'e':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdde8", 10) == 0)
                    {
{
return dev_storage + 2070;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdce8", 10) == 0)
                    {
{
return dev_storage + 1656;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbe8", 10) == 0)
                    {
{
return dev_storage + 1240;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdae8", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd8", 10) == 0)
                    {
{
return dev_storage + 2055;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcd8", 10) == 0)
                    {
{
return dev_storage + 1641;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbd8", 10) == 0)
                    {
{
return dev_storage + 1225;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdad8", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddc8", 10) == 0)
                    {
{
return dev_storage + 2040;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc8", 10) == 0)
                    {
{
return dev_storage + 1626;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbc8", 10) == 0)
                    {
{
return dev_storage + 1210;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdac8", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddb8", 10) == 0)
                    {
{
return dev_storage + 2025;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcb8", 10) == 0)
                    {
{
return dev_storage + 1611;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb8", 10) == 0)
                    {
{
return dev_storage + 1195;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdab8", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdda8", 10) == 0)
                    {
{
return dev_storage + 2010;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdca8", 10) == 0)
                    {
{
return dev_storage + 1596;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdba8", 10) == 0)
                    {
{
return dev_storage + 1180;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa8", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS8", 10) == 0)
                {
{
return dev_storage + 2516;

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
return dev_storage + 202;

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
return dev_storage + 192;

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
return dev_storage + 182;

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
return dev_storage + 172;

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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty58", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst58", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty48", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst48", 10) == 0)
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
            case '3':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty38", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst38", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty28", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst28", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/st118", 10) == 0)
                    {
{
return dev_storage + 2497;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty18", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst18", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st108", 10) == 0)
                {
{
return dev_storage + 2487;

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
            case 'z':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcz7", 10) == 0)
                    {
{
return dev_storage + 1970;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbz7", 10) == 0)
                    {
{
return dev_storage + 1554;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaz7", 10) == 0)
                    {
{
return dev_storage + 1138;

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
            case 'y':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcy7", 10) == 0)
                    {
{
return dev_storage + 1955;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdby7", 10) == 0)
                    {
{
return dev_storage + 1539;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sday7", 10) == 0)
                    {
{
return dev_storage + 1123;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddx7", 10) == 0)
                    {
{
return dev_storage + 2354;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcx7", 10) == 0)
                    {
{
return dev_storage + 1940;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbx7", 10) == 0)
                    {
{
return dev_storage + 1524;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdax7", 10) == 0)
                    {
{
return dev_storage + 1108;

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
            case 'w':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddw7", 10) == 0)
                    {
{
return dev_storage + 2339;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcw7", 10) == 0)
                    {
{
return dev_storage + 1925;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbw7", 10) == 0)
                    {
{
return dev_storage + 1509;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaw7", 10) == 0)
                    {
{
return dev_storage + 1093;

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
            case 'v':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddv7", 10) == 0)
                    {
{
return dev_storage + 2324;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcv7", 10) == 0)
                    {
{
return dev_storage + 1910;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbv7", 10) == 0)
                    {
{
return dev_storage + 1494;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdav7", 10) == 0)
                    {
{
return dev_storage + 1078;

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
            case 'u':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddu7", 10) == 0)
                    {
{
return dev_storage + 2309;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcu7", 10) == 0)
                    {
{
return dev_storage + 1895;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbu7", 10) == 0)
                    {
{
return dev_storage + 1479;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdau7", 10) == 0)
                    {
{
return dev_storage + 1063;

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
            case 't':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddt7", 10) == 0)
                    {
{
return dev_storage + 2294;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdct7", 10) == 0)
                    {
{
return dev_storage + 1880;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbt7", 10) == 0)
                    {
{
return dev_storage + 1464;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdat7", 10) == 0)
                    {
{
return dev_storage + 1048;

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
              switch (KR_keyword [7])
                {
                case 'n':
                  if (strncmp (KR_keyword, "/dev/cons7", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdds7", 10) == 0)
                    {
{
return dev_storage + 2279;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcs7", 10) == 0)
                    {
{
return dev_storage + 1865;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbs7", 10) == 0)
                    {
{
return dev_storage + 1449;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdas7", 10) == 0)
                    {
{
return dev_storage + 1033;

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
            case 'r':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddr7", 10) == 0)
                    {
{
return dev_storage + 2264;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcr7", 10) == 0)
                    {
{
return dev_storage + 1850;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbr7", 10) == 0)
                    {
{
return dev_storage + 1434;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdar7", 10) == 0)
                    {
{
return dev_storage + 1018;

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
            case 'q':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddq7", 10) == 0)
                    {
{
return dev_storage + 2249;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcq7", 10) == 0)
                    {
{
return dev_storage + 1835;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbq7", 10) == 0)
                    {
{
return dev_storage + 1419;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaq7", 10) == 0)
                    {
{
return dev_storage + 1003;

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
            case 'p':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddp7", 10) == 0)
                    {
{
return dev_storage + 2234;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcp7", 10) == 0)
                    {
{
return dev_storage + 1820;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbp7", 10) == 0)
                    {
{
return dev_storage + 1404;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdap7", 10) == 0)
                    {
{
return dev_storage + 988;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddo7", 10) == 0)
                    {
{
return dev_storage + 2219;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdco7", 10) == 0)
                    {
{
return dev_storage + 1805;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbo7", 10) == 0)
                    {
{
return dev_storage + 1389;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdao7", 10) == 0)
                    {
{
return dev_storage + 973;

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
            case 'n':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddn7", 10) == 0)
                    {
{
return dev_storage + 2204;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcn7", 10) == 0)
                    {
{
return dev_storage + 1790;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbn7", 10) == 0)
                    {
{
return dev_storage + 1374;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdan7", 10) == 0)
                    {
{
return dev_storage + 958;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddm7", 10) == 0)
                    {
{
return dev_storage + 2189;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcm7", 10) == 0)
                    {
{
return dev_storage + 1775;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbm7", 10) == 0)
                    {
{
return dev_storage + 1359;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdam7", 10) == 0)
                    {
{
return dev_storage + 943;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddl7", 10) == 0)
                    {
{
return dev_storage + 2174;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcl7", 10) == 0)
                    {
{
return dev_storage + 1760;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbl7", 10) == 0)
                    {
{
return dev_storage + 1344;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdal7", 10) == 0)
                    {
{
return dev_storage + 928;

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
            case 'k':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddk7", 10) == 0)
                    {
{
return dev_storage + 2159;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdck7", 10) == 0)
                    {
{
return dev_storage + 1745;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbk7", 10) == 0)
                    {
{
return dev_storage + 1329;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdak7", 10) == 0)
                    {
{
return dev_storage + 913;

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
            case 'j':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddj7", 10) == 0)
                    {
{
return dev_storage + 2144;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcj7", 10) == 0)
                    {
{
return dev_storage + 1730;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbj7", 10) == 0)
                    {
{
return dev_storage + 1314;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaj7", 10) == 0)
                    {
{
return dev_storage + 898;

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
            case 'i':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddi7", 10) == 0)
                    {
{
return dev_storage + 2129;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdci7", 10) == 0)
                    {
{
return dev_storage + 1715;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbi7", 10) == 0)
                    {
{
return dev_storage + 1299;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdai7", 10) == 0)
                    {
{
return dev_storage + 883;

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
            case 'h':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddh7", 10) == 0)
                    {
{
return dev_storage + 2114;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdch7", 10) == 0)
                    {
{
return dev_storage + 1700;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbh7", 10) == 0)
                    {
{
return dev_storage + 1284;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdah7", 10) == 0)
                    {
{
return dev_storage + 868;

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
            case 'g':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddg7", 10) == 0)
                    {
{
return dev_storage + 2099;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcg7", 10) == 0)
                    {
{
return dev_storage + 1685;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbg7", 10) == 0)
                    {
{
return dev_storage + 1269;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdag7", 10) == 0)
                    {
{
return dev_storage + 853;

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
            case 'f':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddf7", 10) == 0)
                    {
{
return dev_storage + 2084;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcf7", 10) == 0)
                    {
{
return dev_storage + 1670;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbf7", 10) == 0)
                    {
{
return dev_storage + 1254;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaf7", 10) == 0)
                    {
{
return dev_storage + 838;

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
            case 'e':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdde7", 10) == 0)
                    {
{
return dev_storage + 2069;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdce7", 10) == 0)
                    {
{
return dev_storage + 1655;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbe7", 10) == 0)
                    {
{
return dev_storage + 1239;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdae7", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd7", 10) == 0)
                    {
{
return dev_storage + 2054;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcd7", 10) == 0)
                    {
{
return dev_storage + 1640;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbd7", 10) == 0)
                    {
{
return dev_storage + 1224;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdad7", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddc7", 10) == 0)
                    {
{
return dev_storage + 2039;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc7", 10) == 0)
                    {
{
return dev_storage + 1625;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbc7", 10) == 0)
                    {
{
return dev_storage + 1209;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdac7", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddb7", 10) == 0)
                    {
{
return dev_storage + 2024;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcb7", 10) == 0)
                    {
{
return dev_storage + 1610;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb7", 10) == 0)
                    {
{
return dev_storage + 1194;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdab7", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdda7", 10) == 0)
                    {
{
return dev_storage + 2009;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdca7", 10) == 0)
                    {
{
return dev_storage + 1595;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdba7", 10) == 0)
                    {
{
return dev_storage + 1179;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa7", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS7", 10) == 0)
                {
{
return dev_storage + 2515;

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
return dev_storage + 201;

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
return dev_storage + 191;

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
return dev_storage + 181;

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
return dev_storage + 171;

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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty57", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst57", 10) == 0)
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
            case '4':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty47", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst47", 10) == 0)
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
            case '3':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty37", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst37", 10) == 0)
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
            case '2':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/st127", 10) == 0)
                    {
{
return dev_storage + 2506;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty27", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst27", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/st117", 10) == 0)
                    {
{
return dev_storage + 2496;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty17", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst17", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '0':
              if (strncmp (KR_keyword, "/dev/st107", 10) == 0)
                {
{
return dev_storage + 2486;

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
            case 'z':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcz6", 10) == 0)
                    {
{
return dev_storage + 1969;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbz6", 10) == 0)
                    {
{
return dev_storage + 1553;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaz6", 10) == 0)
                    {
{
return dev_storage + 1137;

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
            case 'y':
              switch (KR_keyword [7])
                {
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcy6", 10) == 0)
                    {
{
return dev_storage + 1954;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdby6", 10) == 0)
                    {
{
return dev_storage + 1538;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sday6", 10) == 0)
                    {
{
return dev_storage + 1122;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddx6", 10) == 0)
                    {
{
return dev_storage + 2353;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcx6", 10) == 0)
                    {
{
return dev_storage + 1939;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbx6", 10) == 0)
                    {
{
return dev_storage + 1523;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdax6", 10) == 0)
                    {
{
return dev_storage + 1107;

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
            case 'w':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddw6", 10) == 0)
                    {
{
return dev_storage + 2338;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcw6", 10) == 0)
                    {
{
return dev_storage + 1924;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbw6", 10) == 0)
                    {
{
return dev_storage + 1508;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaw6", 10) == 0)
                    {
{
return dev_storage + 1092;

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
            case 'v':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddv6", 10) == 0)
                    {
{
return dev_storage + 2323;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcv6", 10) == 0)
                    {
{
return dev_storage + 1909;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbv6", 10) == 0)
                    {
{
return dev_storage + 1493;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdav6", 10) == 0)
                    {
{
return dev_storage + 1077;

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
            case 'u':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddu6", 10) == 0)
                    {
{
return dev_storage + 2308;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcu6", 10) == 0)
                    {
{
return dev_storage + 1894;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbu6", 10) == 0)
                    {
{
return dev_storage + 1478;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdau6", 10) == 0)
                    {
{
return dev_storage + 1062;

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
            case 't':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddt6", 10) == 0)
                    {
{
return dev_storage + 2293;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdct6", 10) == 0)
                    {
{
return dev_storage + 1879;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbt6", 10) == 0)
                    {
{
return dev_storage + 1463;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdat6", 10) == 0)
                    {
{
return dev_storage + 1047;

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
              switch (KR_keyword [7])
                {
                case 'n':
                  if (strncmp (KR_keyword, "/dev/cons6", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdds6", 10) == 0)
                    {
{
return dev_storage + 2278;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcs6", 10) == 0)
                    {
{
return dev_storage + 1864;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbs6", 10) == 0)
                    {
{
return dev_storage + 1448;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdas6", 10) == 0)
                    {
{
return dev_storage + 1032;

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
            case 'r':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddr6", 10) == 0)
                    {
{
return dev_storage + 2263;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcr6", 10) == 0)
                    {
{
return dev_storage + 1849;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbr6", 10) == 0)
                    {
{
return dev_storage + 1433;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdar6", 10) == 0)
                    {
{
return dev_storage + 1017;

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
            case 'q':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddq6", 10) == 0)
                    {
{
return dev_storage + 2248;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcq6", 10) == 0)
                    {
{
return dev_storage + 1834;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbq6", 10) == 0)
                    {
{
return dev_storage + 1418;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaq6", 10) == 0)
                    {
{
return dev_storage + 1002;

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
            case 'p':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddp6", 10) == 0)
                    {
{
return dev_storage + 2233;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcp6", 10) == 0)
                    {
{
return dev_storage + 1819;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbp6", 10) == 0)
                    {
{
return dev_storage + 1403;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdap6", 10) == 0)
                    {
{
return dev_storage + 987;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddo6", 10) == 0)
                    {
{
return dev_storage + 2218;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdco6", 10) == 0)
                    {
{
return dev_storage + 1804;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbo6", 10) == 0)
                    {
{
return dev_storage + 1388;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdao6", 10) == 0)
                    {
{
return dev_storage + 972;

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
            case 'n':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddn6", 10) == 0)
                    {
{
return dev_storage + 2203;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcn6", 10) == 0)
                    {
{
return dev_storage + 1789;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbn6", 10) == 0)
                    {
{
return dev_storage + 1373;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdan6", 10) == 0)
                    {
{
return dev_storage + 957;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddm6", 10) == 0)
                    {
{
return dev_storage + 2188;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcm6", 10) == 0)
                    {
{
return dev_storage + 1774;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbm6", 10) == 0)
                    {
{
return dev_storage + 1358;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdam6", 10) == 0)
                    {
{
return dev_storage + 942;

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
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddl6", 10) == 0)
                    {
{
return dev_storage + 2173;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcl6", 10) == 0)
                    {
{
return dev_storage + 1759;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbl6", 10) == 0)
                    {
{
return dev_storage + 1343;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdal6", 10) == 0)
                    {
{
return dev_storage + 927;

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
            case 'k':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddk6", 10) == 0)
                    {
{
return dev_storage + 2158;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdck6", 10) == 0)
                    {
{
return dev_storage + 1744;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbk6", 10) == 0)
                    {
{
return dev_storage + 1328;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdak6", 10) == 0)
                    {
{
return dev_storage + 912;

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
            case 'j':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddj6", 10) == 0)
                    {
{
return dev_storage + 2143;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcj6", 10) == 0)
                    {
{
return dev_storage + 1729;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbj6", 10) == 0)
                    {
{
return dev_storage + 1313;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaj6", 10) == 0)
                    {
{
return dev_storage + 897;

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
            case 'i':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddi6", 10) == 0)
                    {
{
return dev_storage + 2128;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdci6", 10) == 0)
                    {
{
return dev_storage + 1714;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbi6", 10) == 0)
                    {
{
return dev_storage + 1298;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdai6", 10) == 0)
                    {
{
return dev_storage + 882;

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
            case 'h':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddh6", 10) == 0)
                    {
{
return dev_storage + 2113;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdch6", 10) == 0)
                    {
{
return dev_storage + 1699;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbh6", 10) == 0)
                    {
{
return dev_storage + 1283;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdah6", 10) == 0)
                    {
{
return dev_storage + 867;

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
            case 'g':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddg6", 10) == 0)
                    {
{
return dev_storage + 2098;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcg6", 10) == 0)
                    {
{
return dev_storage + 1684;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbg6", 10) == 0)
                    {
{
return dev_storage + 1268;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdag6", 10) == 0)
                    {
{
return dev_storage + 852;

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
            case 'f':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddf6", 10) == 0)
                    {
{
return dev_storage + 2083;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcf6", 10) == 0)
                    {
{
return dev_storage + 1669;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbf6", 10) == 0)
                    {
{
return dev_storage + 1253;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaf6", 10) == 0)
                    {
{
return dev_storage + 837;

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
            case 'e':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdde6", 10) == 0)
                    {
{
return dev_storage + 2068;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdce6", 10) == 0)
                    {
{
return dev_storage + 1654;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbe6", 10) == 0)
                    {
{
return dev_storage + 1238;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdae6", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd6", 10) == 0)
                    {
{
return dev_storage + 2053;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcd6", 10) == 0)
                    {
{
return dev_storage + 1639;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbd6", 10) == 0)
                    {
{
return dev_storage + 1223;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdad6", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddc6", 10) == 0)
                    {
{
return dev_storage + 2038;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc6", 10) == 0)
                    {
{
return dev_storage + 1624;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbc6", 10) == 0)
                    {
{
return dev_storage + 1208;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdac6", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddb6", 10) == 0)
                    {
{
return dev_storage + 2023;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcb6", 10) == 0)
                    {
{
return dev_storage + 1609;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb6", 10) == 0)
                    {
{
return dev_storage + 1193;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdab6", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [7])
                {
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdda6", 10) == 0)
                    {
{
return dev_storage + 2008;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdca6", 10) == 0)
                    {
{
return dev_storage + 1594;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdba6", 10) == 0)
                    {
{
return dev_storage + 1178;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa6", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'S':
              if (strncmp (KR_keyword, "/dev/ttyS6", 10) == 0)
                {
{
return dev_storage + 2514;

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
return dev_storage + 200;

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
return dev_storage + 190;

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
return dev_storage + 180;

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
return dev_storage + 170;

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
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty56", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst56", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty46", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst46", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty36", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst36", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/st126", 10) == 0)
                    {
{
return dev_storage + 2505;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty26", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst26", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/st116", 10) == 0)
                    {
{
return dev_storage + 2495;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/pty16", 10) == 0)
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
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst16", 10) == 0)
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
return dev_storage + 2485;

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
return dev_storage + 730;

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
return dev_storage + 2513;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty55", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty45", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty35", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty25", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy15", 10) == 0)
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
                    case 'p':
                      if (strncmp (KR_keyword, "/dev/pty15", 10) == 0)
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
return dev_storage + 700;

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
return dev_storage + 685;

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
return dev_storage + 670;

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
return dev_storage + 655;

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
return dev_storage + 199;

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
return dev_storage + 189;

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
return dev_storage + 179;

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
return dev_storage + 169;

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
return dev_storage + 159;

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
return dev_storage + 149;

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
return dev_storage + 139;

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
return dev_storage + 129;

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
return dev_storage + 640;

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
return dev_storage + 119;

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
              if (strncmp (KR_keyword, "/dev/sdr15", 10) == 0)
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
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq15", 10) == 0)
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
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp15", 10) == 0)
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
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo15", 10) == 0)
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
            case 'n':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdn15", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons5", 10) == 0)
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
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm15", 10) == 0)
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
return dev_storage + 520;

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
return dev_storage + 505;

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
return dev_storage + 490;

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
return dev_storage + 475;

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
return dev_storage + 460;

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
return dev_storage + 445;

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
return dev_storage + 430;

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
return dev_storage + 415;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx5", 10) == 0)
                    {
{
return dev_storage + 2352;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw5", 10) == 0)
                    {
{
return dev_storage + 2337;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv5", 10) == 0)
                    {
{
return dev_storage + 2322;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu5", 10) == 0)
                    {
{
return dev_storage + 2307;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt5", 10) == 0)
                    {
{
return dev_storage + 2292;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds5", 10) == 0)
                    {
{
return dev_storage + 2277;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr5", 10) == 0)
                    {
{
return dev_storage + 2262;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq5", 10) == 0)
                    {
{
return dev_storage + 2247;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp5", 10) == 0)
                    {
{
return dev_storage + 2232;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo5", 10) == 0)
                    {
{
return dev_storage + 2217;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn5", 10) == 0)
                    {
{
return dev_storage + 2202;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm5", 10) == 0)
                    {
{
return dev_storage + 2187;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl5", 10) == 0)
                    {
{
return dev_storage + 2172;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk5", 10) == 0)
                    {
{
return dev_storage + 2157;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj5", 10) == 0)
                    {
{
return dev_storage + 2142;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi5", 10) == 0)
                    {
{
return dev_storage + 2127;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh5", 10) == 0)
                    {
{
return dev_storage + 2112;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg5", 10) == 0)
                    {
{
return dev_storage + 2097;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf5", 10) == 0)
                    {
{
return dev_storage + 2082;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde5", 10) == 0)
                    {
{
return dev_storage + 2067;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd5", 10) == 0)
                    {
{
return dev_storage + 2052;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc5", 10) == 0)
                    {
{
return dev_storage + 2037;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb5", 10) == 0)
                    {
{
return dev_storage + 2022;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda5", 10) == 0)
                    {
{
return dev_storage + 2007;

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
                    case 'd':
                      if (strncmp (KR_keyword, "/dev/sdd15", 10) == 0)
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
                      if (strncmp (KR_keyword, "/dev/scd15", 10) == 0)
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
            case 'c':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz5", 10) == 0)
                    {
{
return dev_storage + 1968;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy5", 10) == 0)
                    {
{
return dev_storage + 1953;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx5", 10) == 0)
                    {
{
return dev_storage + 1938;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw5", 10) == 0)
                    {
{
return dev_storage + 1923;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv5", 10) == 0)
                    {
{
return dev_storage + 1908;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu5", 10) == 0)
                    {
{
return dev_storage + 1893;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct5", 10) == 0)
                    {
{
return dev_storage + 1878;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs5", 10) == 0)
                    {
{
return dev_storage + 1863;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr5", 10) == 0)
                    {
{
return dev_storage + 1848;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq5", 10) == 0)
                    {
{
return dev_storage + 1833;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp5", 10) == 0)
                    {
{
return dev_storage + 1818;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco5", 10) == 0)
                    {
{
return dev_storage + 1803;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn5", 10) == 0)
                    {
{
return dev_storage + 1788;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm5", 10) == 0)
                    {
{
return dev_storage + 1773;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl5", 10) == 0)
                    {
{
return dev_storage + 1758;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck5", 10) == 0)
                    {
{
return dev_storage + 1743;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj5", 10) == 0)
                    {
{
return dev_storage + 1728;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci5", 10) == 0)
                    {
{
return dev_storage + 1713;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch5", 10) == 0)
                    {
{
return dev_storage + 1698;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg5", 10) == 0)
                    {
{
return dev_storage + 1683;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf5", 10) == 0)
                    {
{
return dev_storage + 1668;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce5", 10) == 0)
                    {
{
return dev_storage + 1653;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd5", 10) == 0)
                    {
{
return dev_storage + 1638;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc5", 10) == 0)
                    {
{
return dev_storage + 1623;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb5", 10) == 0)
                    {
{
return dev_storage + 1608;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca5", 10) == 0)
                    {
{
return dev_storage + 1593;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdc15", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz5", 10) == 0)
                    {
{
return dev_storage + 1552;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby5", 10) == 0)
                    {
{
return dev_storage + 1537;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx5", 10) == 0)
                    {
{
return dev_storage + 1522;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw5", 10) == 0)
                    {
{
return dev_storage + 1507;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv5", 10) == 0)
                    {
{
return dev_storage + 1492;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu5", 10) == 0)
                    {
{
return dev_storage + 1477;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt5", 10) == 0)
                    {
{
return dev_storage + 1462;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs5", 10) == 0)
                    {
{
return dev_storage + 1447;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr5", 10) == 0)
                    {
{
return dev_storage + 1432;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq5", 10) == 0)
                    {
{
return dev_storage + 1417;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp5", 10) == 0)
                    {
{
return dev_storage + 1402;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo5", 10) == 0)
                    {
{
return dev_storage + 1387;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn5", 10) == 0)
                    {
{
return dev_storage + 1372;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm5", 10) == 0)
                    {
{
return dev_storage + 1357;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl5", 10) == 0)
                    {
{
return dev_storage + 1342;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk5", 10) == 0)
                    {
{
return dev_storage + 1327;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj5", 10) == 0)
                    {
{
return dev_storage + 1312;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi5", 10) == 0)
                    {
{
return dev_storage + 1297;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh5", 10) == 0)
                    {
{
return dev_storage + 1282;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg5", 10) == 0)
                    {
{
return dev_storage + 1267;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf5", 10) == 0)
                    {
{
return dev_storage + 1252;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe5", 10) == 0)
                    {
{
return dev_storage + 1237;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd5", 10) == 0)
                    {
{
return dev_storage + 1222;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc5", 10) == 0)
                    {
{
return dev_storage + 1207;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb5", 10) == 0)
                    {
{
return dev_storage + 1192;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba5", 10) == 0)
                    {
{
return dev_storage + 1177;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdb15", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz5", 10) == 0)
                    {
{
return dev_storage + 1136;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday5", 10) == 0)
                    {
{
return dev_storage + 1121;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax5", 10) == 0)
                    {
{
return dev_storage + 1106;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw5", 10) == 0)
                    {
{
return dev_storage + 1091;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav5", 10) == 0)
                    {
{
return dev_storage + 1076;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau5", 10) == 0)
                    {
{
return dev_storage + 1061;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat5", 10) == 0)
                    {
{
return dev_storage + 1046;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas5", 10) == 0)
                    {
{
return dev_storage + 1031;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar5", 10) == 0)
                    {
{
return dev_storage + 1016;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq5", 10) == 0)
                    {
{
return dev_storage + 1001;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap5", 10) == 0)
                    {
{
return dev_storage + 986;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao5", 10) == 0)
                    {
{
return dev_storage + 971;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan5", 10) == 0)
                    {
{
return dev_storage + 956;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam5", 10) == 0)
                    {
{
return dev_storage + 941;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal5", 10) == 0)
                    {
{
return dev_storage + 926;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak5", 10) == 0)
                    {
{
return dev_storage + 911;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj5", 10) == 0)
                    {
{
return dev_storage + 896;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai5", 10) == 0)
                    {
{
return dev_storage + 881;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah5", 10) == 0)
                    {
{
return dev_storage + 866;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag5", 10) == 0)
                    {
{
return dev_storage + 851;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf5", 10) == 0)
                    {
{
return dev_storage + 836;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae5", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad5", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac5", 10) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab5", 10) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa5", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/sda15", 10) == 0)
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
                default:
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
return dev_storage + 2504;

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
return dev_storage + 2494;

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
return dev_storage + 2484;

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
return dev_storage + 729;

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
return dev_storage + 2512;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty54", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty44", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty34", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty24", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy14", 10) == 0)
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
                    case 'p':
                      if (strncmp (KR_keyword, "/dev/pty14", 10) == 0)
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
return dev_storage + 699;

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
return dev_storage + 684;

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
return dev_storage + 669;

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
return dev_storage + 654;

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
return dev_storage + 198;

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
return dev_storage + 188;

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
return dev_storage + 178;

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
return dev_storage + 168;

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
return dev_storage + 158;

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
return dev_storage + 148;

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
return dev_storage + 138;

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
return dev_storage + 128;

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
return dev_storage + 639;

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
return dev_storage + 118;

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
              if (strncmp (KR_keyword, "/dev/sdr14", 10) == 0)
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
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq14", 10) == 0)
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
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp14", 10) == 0)
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
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo14", 10) == 0)
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
            case 'n':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdn14", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons4", 10) == 0)
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
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm14", 10) == 0)
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
return dev_storage + 519;

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
return dev_storage + 504;

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
return dev_storage + 489;

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
return dev_storage + 474;

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
return dev_storage + 459;

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
return dev_storage + 444;

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
return dev_storage + 429;

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
return dev_storage + 414;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx4", 10) == 0)
                    {
{
return dev_storage + 2351;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw4", 10) == 0)
                    {
{
return dev_storage + 2336;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv4", 10) == 0)
                    {
{
return dev_storage + 2321;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu4", 10) == 0)
                    {
{
return dev_storage + 2306;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt4", 10) == 0)
                    {
{
return dev_storage + 2291;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds4", 10) == 0)
                    {
{
return dev_storage + 2276;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr4", 10) == 0)
                    {
{
return dev_storage + 2261;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq4", 10) == 0)
                    {
{
return dev_storage + 2246;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp4", 10) == 0)
                    {
{
return dev_storage + 2231;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo4", 10) == 0)
                    {
{
return dev_storage + 2216;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn4", 10) == 0)
                    {
{
return dev_storage + 2201;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm4", 10) == 0)
                    {
{
return dev_storage + 2186;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl4", 10) == 0)
                    {
{
return dev_storage + 2171;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk4", 10) == 0)
                    {
{
return dev_storage + 2156;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj4", 10) == 0)
                    {
{
return dev_storage + 2141;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi4", 10) == 0)
                    {
{
return dev_storage + 2126;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh4", 10) == 0)
                    {
{
return dev_storage + 2111;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg4", 10) == 0)
                    {
{
return dev_storage + 2096;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf4", 10) == 0)
                    {
{
return dev_storage + 2081;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde4", 10) == 0)
                    {
{
return dev_storage + 2066;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd4", 10) == 0)
                    {
{
return dev_storage + 2051;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc4", 10) == 0)
                    {
{
return dev_storage + 2036;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb4", 10) == 0)
                    {
{
return dev_storage + 2021;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda4", 10) == 0)
                    {
{
return dev_storage + 2006;

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
                    case 'd':
                      if (strncmp (KR_keyword, "/dev/sdd14", 10) == 0)
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
                      if (strncmp (KR_keyword, "/dev/scd14", 10) == 0)
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
            case 'c':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz4", 10) == 0)
                    {
{
return dev_storage + 1967;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy4", 10) == 0)
                    {
{
return dev_storage + 1952;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx4", 10) == 0)
                    {
{
return dev_storage + 1937;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw4", 10) == 0)
                    {
{
return dev_storage + 1922;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv4", 10) == 0)
                    {
{
return dev_storage + 1907;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu4", 10) == 0)
                    {
{
return dev_storage + 1892;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct4", 10) == 0)
                    {
{
return dev_storage + 1877;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs4", 10) == 0)
                    {
{
return dev_storage + 1862;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr4", 10) == 0)
                    {
{
return dev_storage + 1847;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq4", 10) == 0)
                    {
{
return dev_storage + 1832;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp4", 10) == 0)
                    {
{
return dev_storage + 1817;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco4", 10) == 0)
                    {
{
return dev_storage + 1802;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn4", 10) == 0)
                    {
{
return dev_storage + 1787;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm4", 10) == 0)
                    {
{
return dev_storage + 1772;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl4", 10) == 0)
                    {
{
return dev_storage + 1757;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck4", 10) == 0)
                    {
{
return dev_storage + 1742;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj4", 10) == 0)
                    {
{
return dev_storage + 1727;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci4", 10) == 0)
                    {
{
return dev_storage + 1712;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch4", 10) == 0)
                    {
{
return dev_storage + 1697;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg4", 10) == 0)
                    {
{
return dev_storage + 1682;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf4", 10) == 0)
                    {
{
return dev_storage + 1667;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce4", 10) == 0)
                    {
{
return dev_storage + 1652;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd4", 10) == 0)
                    {
{
return dev_storage + 1637;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc4", 10) == 0)
                    {
{
return dev_storage + 1622;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb4", 10) == 0)
                    {
{
return dev_storage + 1607;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca4", 10) == 0)
                    {
{
return dev_storage + 1592;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdc14", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz4", 10) == 0)
                    {
{
return dev_storage + 1551;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby4", 10) == 0)
                    {
{
return dev_storage + 1536;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx4", 10) == 0)
                    {
{
return dev_storage + 1521;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw4", 10) == 0)
                    {
{
return dev_storage + 1506;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv4", 10) == 0)
                    {
{
return dev_storage + 1491;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu4", 10) == 0)
                    {
{
return dev_storage + 1476;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt4", 10) == 0)
                    {
{
return dev_storage + 1461;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs4", 10) == 0)
                    {
{
return dev_storage + 1446;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr4", 10) == 0)
                    {
{
return dev_storage + 1431;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq4", 10) == 0)
                    {
{
return dev_storage + 1416;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp4", 10) == 0)
                    {
{
return dev_storage + 1401;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo4", 10) == 0)
                    {
{
return dev_storage + 1386;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn4", 10) == 0)
                    {
{
return dev_storage + 1371;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm4", 10) == 0)
                    {
{
return dev_storage + 1356;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl4", 10) == 0)
                    {
{
return dev_storage + 1341;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk4", 10) == 0)
                    {
{
return dev_storage + 1326;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj4", 10) == 0)
                    {
{
return dev_storage + 1311;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi4", 10) == 0)
                    {
{
return dev_storage + 1296;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh4", 10) == 0)
                    {
{
return dev_storage + 1281;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg4", 10) == 0)
                    {
{
return dev_storage + 1266;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf4", 10) == 0)
                    {
{
return dev_storage + 1251;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe4", 10) == 0)
                    {
{
return dev_storage + 1236;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd4", 10) == 0)
                    {
{
return dev_storage + 1221;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc4", 10) == 0)
                    {
{
return dev_storage + 1206;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb4", 10) == 0)
                    {
{
return dev_storage + 1191;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba4", 10) == 0)
                    {
{
return dev_storage + 1176;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdb14", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz4", 10) == 0)
                    {
{
return dev_storage + 1135;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday4", 10) == 0)
                    {
{
return dev_storage + 1120;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax4", 10) == 0)
                    {
{
return dev_storage + 1105;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw4", 10) == 0)
                    {
{
return dev_storage + 1090;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav4", 10) == 0)
                    {
{
return dev_storage + 1075;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau4", 10) == 0)
                    {
{
return dev_storage + 1060;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat4", 10) == 0)
                    {
{
return dev_storage + 1045;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas4", 10) == 0)
                    {
{
return dev_storage + 1030;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar4", 10) == 0)
                    {
{
return dev_storage + 1015;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq4", 10) == 0)
                    {
{
return dev_storage + 1000;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap4", 10) == 0)
                    {
{
return dev_storage + 985;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao4", 10) == 0)
                    {
{
return dev_storage + 970;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan4", 10) == 0)
                    {
{
return dev_storage + 955;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam4", 10) == 0)
                    {
{
return dev_storage + 940;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal4", 10) == 0)
                    {
{
return dev_storage + 925;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak4", 10) == 0)
                    {
{
return dev_storage + 910;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj4", 10) == 0)
                    {
{
return dev_storage + 895;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai4", 10) == 0)
                    {
{
return dev_storage + 880;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah4", 10) == 0)
                    {
{
return dev_storage + 865;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag4", 10) == 0)
                    {
{
return dev_storage + 850;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf4", 10) == 0)
                    {
{
return dev_storage + 835;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae4", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad4", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac4", 10) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab4", 10) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa4", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/sda14", 10) == 0)
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
                default:
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
return dev_storage + 2503;

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
return dev_storage + 2493;

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
return dev_storage + 2483;

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
return dev_storage + 728;

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
return dev_storage + 2511;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty63", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty53", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty43", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty33", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty23", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy13", 10) == 0)
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
                    case 'p':
                      if (strncmp (KR_keyword, "/dev/pty13", 10) == 0)
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
return dev_storage + 698;

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
return dev_storage + 683;

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
return dev_storage + 668;

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
return dev_storage + 653;

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
return dev_storage + 197;

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
return dev_storage + 187;

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
return dev_storage + 177;

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
                  if (strncmp (KR_keyword, "/dev/nst53", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst43", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst33", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst23", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt13", 10) == 0)
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
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst13", 10) == 0)
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
              if (strncmp (KR_keyword, "/dev/sdr13", 10) == 0)
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
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq13", 10) == 0)
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
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp13", 10) == 0)
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
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo13", 10) == 0)
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
            case 'n':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdn13", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons3", 10) == 0)
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
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm13", 10) == 0)
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
return dev_storage + 518;

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
return dev_storage + 503;

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
return dev_storage + 488;

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
return dev_storage + 473;

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
return dev_storage + 458;

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
return dev_storage + 443;

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
return dev_storage + 428;

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
return dev_storage + 413;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx3", 10) == 0)
                    {
{
return dev_storage + 2350;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw3", 10) == 0)
                    {
{
return dev_storage + 2335;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv3", 10) == 0)
                    {
{
return dev_storage + 2320;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu3", 10) == 0)
                    {
{
return dev_storage + 2305;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt3", 10) == 0)
                    {
{
return dev_storage + 2290;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds3", 10) == 0)
                    {
{
return dev_storage + 2275;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr3", 10) == 0)
                    {
{
return dev_storage + 2260;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq3", 10) == 0)
                    {
{
return dev_storage + 2245;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp3", 10) == 0)
                    {
{
return dev_storage + 2230;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo3", 10) == 0)
                    {
{
return dev_storage + 2215;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn3", 10) == 0)
                    {
{
return dev_storage + 2200;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm3", 10) == 0)
                    {
{
return dev_storage + 2185;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl3", 10) == 0)
                    {
{
return dev_storage + 2170;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk3", 10) == 0)
                    {
{
return dev_storage + 2155;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj3", 10) == 0)
                    {
{
return dev_storage + 2140;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi3", 10) == 0)
                    {
{
return dev_storage + 2125;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh3", 10) == 0)
                    {
{
return dev_storage + 2110;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg3", 10) == 0)
                    {
{
return dev_storage + 2095;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf3", 10) == 0)
                    {
{
return dev_storage + 2080;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde3", 10) == 0)
                    {
{
return dev_storage + 2065;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd3", 10) == 0)
                    {
{
return dev_storage + 2050;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc3", 10) == 0)
                    {
{
return dev_storage + 2035;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb3", 10) == 0)
                    {
{
return dev_storage + 2020;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda3", 10) == 0)
                    {
{
return dev_storage + 2005;

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
                    case 'd':
                      if (strncmp (KR_keyword, "/dev/sdd13", 10) == 0)
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
                      if (strncmp (KR_keyword, "/dev/scd13", 10) == 0)
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
            case 'c':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz3", 10) == 0)
                    {
{
return dev_storage + 1966;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy3", 10) == 0)
                    {
{
return dev_storage + 1951;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx3", 10) == 0)
                    {
{
return dev_storage + 1936;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw3", 10) == 0)
                    {
{
return dev_storage + 1921;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv3", 10) == 0)
                    {
{
return dev_storage + 1906;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu3", 10) == 0)
                    {
{
return dev_storage + 1891;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct3", 10) == 0)
                    {
{
return dev_storage + 1876;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs3", 10) == 0)
                    {
{
return dev_storage + 1861;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr3", 10) == 0)
                    {
{
return dev_storage + 1846;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq3", 10) == 0)
                    {
{
return dev_storage + 1831;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp3", 10) == 0)
                    {
{
return dev_storage + 1816;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco3", 10) == 0)
                    {
{
return dev_storage + 1801;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn3", 10) == 0)
                    {
{
return dev_storage + 1786;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm3", 10) == 0)
                    {
{
return dev_storage + 1771;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl3", 10) == 0)
                    {
{
return dev_storage + 1756;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck3", 10) == 0)
                    {
{
return dev_storage + 1741;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj3", 10) == 0)
                    {
{
return dev_storage + 1726;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci3", 10) == 0)
                    {
{
return dev_storage + 1711;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch3", 10) == 0)
                    {
{
return dev_storage + 1696;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg3", 10) == 0)
                    {
{
return dev_storage + 1681;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf3", 10) == 0)
                    {
{
return dev_storage + 1666;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce3", 10) == 0)
                    {
{
return dev_storage + 1651;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd3", 10) == 0)
                    {
{
return dev_storage + 1636;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc3", 10) == 0)
                    {
{
return dev_storage + 1621;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb3", 10) == 0)
                    {
{
return dev_storage + 1606;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca3", 10) == 0)
                    {
{
return dev_storage + 1591;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdc13", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz3", 10) == 0)
                    {
{
return dev_storage + 1550;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby3", 10) == 0)
                    {
{
return dev_storage + 1535;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx3", 10) == 0)
                    {
{
return dev_storage + 1520;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw3", 10) == 0)
                    {
{
return dev_storage + 1505;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv3", 10) == 0)
                    {
{
return dev_storage + 1490;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu3", 10) == 0)
                    {
{
return dev_storage + 1475;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt3", 10) == 0)
                    {
{
return dev_storage + 1460;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs3", 10) == 0)
                    {
{
return dev_storage + 1445;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr3", 10) == 0)
                    {
{
return dev_storage + 1430;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq3", 10) == 0)
                    {
{
return dev_storage + 1415;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp3", 10) == 0)
                    {
{
return dev_storage + 1400;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo3", 10) == 0)
                    {
{
return dev_storage + 1385;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn3", 10) == 0)
                    {
{
return dev_storage + 1370;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm3", 10) == 0)
                    {
{
return dev_storage + 1355;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl3", 10) == 0)
                    {
{
return dev_storage + 1340;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk3", 10) == 0)
                    {
{
return dev_storage + 1325;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj3", 10) == 0)
                    {
{
return dev_storage + 1310;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi3", 10) == 0)
                    {
{
return dev_storage + 1295;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh3", 10) == 0)
                    {
{
return dev_storage + 1280;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg3", 10) == 0)
                    {
{
return dev_storage + 1265;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf3", 10) == 0)
                    {
{
return dev_storage + 1250;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe3", 10) == 0)
                    {
{
return dev_storage + 1235;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd3", 10) == 0)
                    {
{
return dev_storage + 1220;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc3", 10) == 0)
                    {
{
return dev_storage + 1205;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb3", 10) == 0)
                    {
{
return dev_storage + 1190;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba3", 10) == 0)
                    {
{
return dev_storage + 1175;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdb13", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz3", 10) == 0)
                    {
{
return dev_storage + 1134;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday3", 10) == 0)
                    {
{
return dev_storage + 1119;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax3", 10) == 0)
                    {
{
return dev_storage + 1104;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw3", 10) == 0)
                    {
{
return dev_storage + 1089;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav3", 10) == 0)
                    {
{
return dev_storage + 1074;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau3", 10) == 0)
                    {
{
return dev_storage + 1059;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat3", 10) == 0)
                    {
{
return dev_storage + 1044;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas3", 10) == 0)
                    {
{
return dev_storage + 1029;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar3", 10) == 0)
                    {
{
return dev_storage + 1014;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq3", 10) == 0)
                    {
{
return dev_storage + 999;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap3", 10) == 0)
                    {
{
return dev_storage + 984;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao3", 10) == 0)
                    {
{
return dev_storage + 969;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan3", 10) == 0)
                    {
{
return dev_storage + 954;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam3", 10) == 0)
                    {
{
return dev_storage + 939;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal3", 10) == 0)
                    {
{
return dev_storage + 924;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak3", 10) == 0)
                    {
{
return dev_storage + 909;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj3", 10) == 0)
                    {
{
return dev_storage + 894;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai3", 10) == 0)
                    {
{
return dev_storage + 879;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah3", 10) == 0)
                    {
{
return dev_storage + 864;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag3", 10) == 0)
                    {
{
return dev_storage + 849;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf3", 10) == 0)
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
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae3", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad3", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac3", 10) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab3", 10) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa3", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/sda13", 10) == 0)
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
                default:
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
return dev_storage + 2502;

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
return dev_storage + 2492;

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
return dev_storage + 2482;

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
return dev_storage + 727;

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
return dev_storage + 2510;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty62", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty52", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty42", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty32", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty22", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy12", 10) == 0)
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
                    case 'p':
                      if (strncmp (KR_keyword, "/dev/pty12", 10) == 0)
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
return dev_storage + 697;

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
return dev_storage + 682;

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
return dev_storage + 667;

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
return dev_storage + 652;

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
return dev_storage + 196;

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
return dev_storage + 186;

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
return dev_storage + 176;

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
return dev_storage + 166;

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
return dev_storage + 156;

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
return dev_storage + 146;

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
return dev_storage + 136;

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
return dev_storage + 126;

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
return dev_storage + 637;

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
return dev_storage + 116;

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
              if (strncmp (KR_keyword, "/dev/sdr12", 10) == 0)
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
            case 'q':
              if (strncmp (KR_keyword, "/dev/sdq12", 10) == 0)
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
            case 'p':
              if (strncmp (KR_keyword, "/dev/sdp12", 10) == 0)
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
            case 'o':
              if (strncmp (KR_keyword, "/dev/sdo12", 10) == 0)
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
            case 'n':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdn12", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons2", 10) == 0)
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
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm12", 10) == 0)
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
return dev_storage + 517;

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
return dev_storage + 502;

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
return dev_storage + 487;

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
return dev_storage + 472;

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
return dev_storage + 457;

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
return dev_storage + 442;

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
return dev_storage + 427;

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
return dev_storage + 412;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx2", 10) == 0)
                    {
{
return dev_storage + 2349;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw2", 10) == 0)
                    {
{
return dev_storage + 2334;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv2", 10) == 0)
                    {
{
return dev_storage + 2319;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu2", 10) == 0)
                    {
{
return dev_storage + 2304;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt2", 10) == 0)
                    {
{
return dev_storage + 2289;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds2", 10) == 0)
                    {
{
return dev_storage + 2274;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr2", 10) == 0)
                    {
{
return dev_storage + 2259;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq2", 10) == 0)
                    {
{
return dev_storage + 2244;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp2", 10) == 0)
                    {
{
return dev_storage + 2229;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo2", 10) == 0)
                    {
{
return dev_storage + 2214;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn2", 10) == 0)
                    {
{
return dev_storage + 2199;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm2", 10) == 0)
                    {
{
return dev_storage + 2184;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl2", 10) == 0)
                    {
{
return dev_storage + 2169;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk2", 10) == 0)
                    {
{
return dev_storage + 2154;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj2", 10) == 0)
                    {
{
return dev_storage + 2139;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi2", 10) == 0)
                    {
{
return dev_storage + 2124;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh2", 10) == 0)
                    {
{
return dev_storage + 2109;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg2", 10) == 0)
                    {
{
return dev_storage + 2094;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf2", 10) == 0)
                    {
{
return dev_storage + 2079;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde2", 10) == 0)
                    {
{
return dev_storage + 2064;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd2", 10) == 0)
                    {
{
return dev_storage + 2049;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc2", 10) == 0)
                    {
{
return dev_storage + 2034;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb2", 10) == 0)
                    {
{
return dev_storage + 2019;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda2", 10) == 0)
                    {
{
return dev_storage + 2004;

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
                    case 'd':
                      if (strncmp (KR_keyword, "/dev/sdd12", 10) == 0)
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
                      if (strncmp (KR_keyword, "/dev/scd12", 10) == 0)
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
            case 'c':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz2", 10) == 0)
                    {
{
return dev_storage + 1965;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy2", 10) == 0)
                    {
{
return dev_storage + 1950;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx2", 10) == 0)
                    {
{
return dev_storage + 1935;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw2", 10) == 0)
                    {
{
return dev_storage + 1920;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv2", 10) == 0)
                    {
{
return dev_storage + 1905;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu2", 10) == 0)
                    {
{
return dev_storage + 1890;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct2", 10) == 0)
                    {
{
return dev_storage + 1875;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs2", 10) == 0)
                    {
{
return dev_storage + 1860;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr2", 10) == 0)
                    {
{
return dev_storage + 1845;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq2", 10) == 0)
                    {
{
return dev_storage + 1830;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp2", 10) == 0)
                    {
{
return dev_storage + 1815;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco2", 10) == 0)
                    {
{
return dev_storage + 1800;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn2", 10) == 0)
                    {
{
return dev_storage + 1785;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm2", 10) == 0)
                    {
{
return dev_storage + 1770;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl2", 10) == 0)
                    {
{
return dev_storage + 1755;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck2", 10) == 0)
                    {
{
return dev_storage + 1740;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj2", 10) == 0)
                    {
{
return dev_storage + 1725;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci2", 10) == 0)
                    {
{
return dev_storage + 1710;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch2", 10) == 0)
                    {
{
return dev_storage + 1695;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg2", 10) == 0)
                    {
{
return dev_storage + 1680;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf2", 10) == 0)
                    {
{
return dev_storage + 1665;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce2", 10) == 0)
                    {
{
return dev_storage + 1650;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd2", 10) == 0)
                    {
{
return dev_storage + 1635;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc2", 10) == 0)
                    {
{
return dev_storage + 1620;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb2", 10) == 0)
                    {
{
return dev_storage + 1605;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca2", 10) == 0)
                    {
{
return dev_storage + 1590;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdc12", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz2", 10) == 0)
                    {
{
return dev_storage + 1549;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby2", 10) == 0)
                    {
{
return dev_storage + 1534;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx2", 10) == 0)
                    {
{
return dev_storage + 1519;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw2", 10) == 0)
                    {
{
return dev_storage + 1504;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv2", 10) == 0)
                    {
{
return dev_storage + 1489;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu2", 10) == 0)
                    {
{
return dev_storage + 1474;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt2", 10) == 0)
                    {
{
return dev_storage + 1459;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs2", 10) == 0)
                    {
{
return dev_storage + 1444;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr2", 10) == 0)
                    {
{
return dev_storage + 1429;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq2", 10) == 0)
                    {
{
return dev_storage + 1414;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp2", 10) == 0)
                    {
{
return dev_storage + 1399;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo2", 10) == 0)
                    {
{
return dev_storage + 1384;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn2", 10) == 0)
                    {
{
return dev_storage + 1369;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm2", 10) == 0)
                    {
{
return dev_storage + 1354;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl2", 10) == 0)
                    {
{
return dev_storage + 1339;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk2", 10) == 0)
                    {
{
return dev_storage + 1324;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj2", 10) == 0)
                    {
{
return dev_storage + 1309;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi2", 10) == 0)
                    {
{
return dev_storage + 1294;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh2", 10) == 0)
                    {
{
return dev_storage + 1279;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg2", 10) == 0)
                    {
{
return dev_storage + 1264;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf2", 10) == 0)
                    {
{
return dev_storage + 1249;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe2", 10) == 0)
                    {
{
return dev_storage + 1234;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd2", 10) == 0)
                    {
{
return dev_storage + 1219;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc2", 10) == 0)
                    {
{
return dev_storage + 1204;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb2", 10) == 0)
                    {
{
return dev_storage + 1189;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba2", 10) == 0)
                    {
{
return dev_storage + 1174;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdb12", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz2", 10) == 0)
                    {
{
return dev_storage + 1133;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday2", 10) == 0)
                    {
{
return dev_storage + 1118;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax2", 10) == 0)
                    {
{
return dev_storage + 1103;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw2", 10) == 0)
                    {
{
return dev_storage + 1088;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav2", 10) == 0)
                    {
{
return dev_storage + 1073;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau2", 10) == 0)
                    {
{
return dev_storage + 1058;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat2", 10) == 0)
                    {
{
return dev_storage + 1043;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas2", 10) == 0)
                    {
{
return dev_storage + 1028;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar2", 10) == 0)
                    {
{
return dev_storage + 1013;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq2", 10) == 0)
                    {
{
return dev_storage + 998;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap2", 10) == 0)
                    {
{
return dev_storage + 983;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao2", 10) == 0)
                    {
{
return dev_storage + 968;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan2", 10) == 0)
                    {
{
return dev_storage + 953;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam2", 10) == 0)
                    {
{
return dev_storage + 938;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal2", 10) == 0)
                    {
{
return dev_storage + 923;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak2", 10) == 0)
                    {
{
return dev_storage + 908;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj2", 10) == 0)
                    {
{
return dev_storage + 893;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai2", 10) == 0)
                    {
{
return dev_storage + 878;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah2", 10) == 0)
                    {
{
return dev_storage + 863;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag2", 10) == 0)
                    {
{
return dev_storage + 848;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf2", 10) == 0)
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
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae2", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad2", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac2", 10) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab2", 10) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa2", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/sda12", 10) == 0)
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
                default:
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
return dev_storage + 2501;

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
return dev_storage + 2491;

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
return dev_storage + 2481;

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
return dev_storage + 726;

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
return dev_storage + 2509;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty61", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty51", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty41", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty31", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty21", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy11", 10) == 0)
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
                    case 'p':
                      if (strncmp (KR_keyword, "/dev/pty11", 10) == 0)
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
return dev_storage + 696;

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
return dev_storage + 681;

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
return dev_storage + 666;

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
return dev_storage + 651;

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
return dev_storage + 195;

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
return dev_storage + 185;

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
return dev_storage + 175;

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
return dev_storage + 165;

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
return dev_storage + 155;

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
                  if (strncmp (KR_keyword, "/dev/nst31", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst21", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdt11", 10) == 0)
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
                    case 'n':
                      if (strncmp (KR_keyword, "/dev/nst11", 10) == 0)
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
return dev_storage + 621;

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
return dev_storage + 606;

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
return dev_storage + 591;

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
return dev_storage + 576;

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
return dev_storage + 561;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdn11", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons1", 10) == 0)
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
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm11", 10) == 0)
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
return dev_storage + 516;

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
return dev_storage + 501;

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
return dev_storage + 486;

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
return dev_storage + 471;

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
return dev_storage + 456;

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
return dev_storage + 441;

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
return dev_storage + 426;

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
return dev_storage + 411;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx1", 10) == 0)
                    {
{
return dev_storage + 2348;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw1", 10) == 0)
                    {
{
return dev_storage + 2333;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv1", 10) == 0)
                    {
{
return dev_storage + 2318;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu1", 10) == 0)
                    {
{
return dev_storage + 2303;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt1", 10) == 0)
                    {
{
return dev_storage + 2288;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds1", 10) == 0)
                    {
{
return dev_storage + 2273;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr1", 10) == 0)
                    {
{
return dev_storage + 2258;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq1", 10) == 0)
                    {
{
return dev_storage + 2243;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp1", 10) == 0)
                    {
{
return dev_storage + 2228;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo1", 10) == 0)
                    {
{
return dev_storage + 2213;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn1", 10) == 0)
                    {
{
return dev_storage + 2198;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm1", 10) == 0)
                    {
{
return dev_storage + 2183;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl1", 10) == 0)
                    {
{
return dev_storage + 2168;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk1", 10) == 0)
                    {
{
return dev_storage + 2153;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj1", 10) == 0)
                    {
{
return dev_storage + 2138;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi1", 10) == 0)
                    {
{
return dev_storage + 2123;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh1", 10) == 0)
                    {
{
return dev_storage + 2108;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg1", 10) == 0)
                    {
{
return dev_storage + 2093;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf1", 10) == 0)
                    {
{
return dev_storage + 2078;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde1", 10) == 0)
                    {
{
return dev_storage + 2063;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd1", 10) == 0)
                    {
{
return dev_storage + 2048;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc1", 10) == 0)
                    {
{
return dev_storage + 2033;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb1", 10) == 0)
                    {
{
return dev_storage + 2018;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda1", 10) == 0)
                    {
{
return dev_storage + 2003;

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
                    case 'd':
                      if (strncmp (KR_keyword, "/dev/sdd11", 10) == 0)
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
                      if (strncmp (KR_keyword, "/dev/scd11", 10) == 0)
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
            case 'c':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz1", 10) == 0)
                    {
{
return dev_storage + 1964;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy1", 10) == 0)
                    {
{
return dev_storage + 1949;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx1", 10) == 0)
                    {
{
return dev_storage + 1934;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw1", 10) == 0)
                    {
{
return dev_storage + 1919;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv1", 10) == 0)
                    {
{
return dev_storage + 1904;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu1", 10) == 0)
                    {
{
return dev_storage + 1889;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct1", 10) == 0)
                    {
{
return dev_storage + 1874;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs1", 10) == 0)
                    {
{
return dev_storage + 1859;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr1", 10) == 0)
                    {
{
return dev_storage + 1844;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq1", 10) == 0)
                    {
{
return dev_storage + 1829;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp1", 10) == 0)
                    {
{
return dev_storage + 1814;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco1", 10) == 0)
                    {
{
return dev_storage + 1799;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn1", 10) == 0)
                    {
{
return dev_storage + 1784;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm1", 10) == 0)
                    {
{
return dev_storage + 1769;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl1", 10) == 0)
                    {
{
return dev_storage + 1754;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck1", 10) == 0)
                    {
{
return dev_storage + 1739;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj1", 10) == 0)
                    {
{
return dev_storage + 1724;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci1", 10) == 0)
                    {
{
return dev_storage + 1709;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch1", 10) == 0)
                    {
{
return dev_storage + 1694;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg1", 10) == 0)
                    {
{
return dev_storage + 1679;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf1", 10) == 0)
                    {
{
return dev_storage + 1664;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce1", 10) == 0)
                    {
{
return dev_storage + 1649;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd1", 10) == 0)
                    {
{
return dev_storage + 1634;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc1", 10) == 0)
                    {
{
return dev_storage + 1619;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb1", 10) == 0)
                    {
{
return dev_storage + 1604;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca1", 10) == 0)
                    {
{
return dev_storage + 1589;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdc11", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz1", 10) == 0)
                    {
{
return dev_storage + 1548;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby1", 10) == 0)
                    {
{
return dev_storage + 1533;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx1", 10) == 0)
                    {
{
return dev_storage + 1518;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw1", 10) == 0)
                    {
{
return dev_storage + 1503;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv1", 10) == 0)
                    {
{
return dev_storage + 1488;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu1", 10) == 0)
                    {
{
return dev_storage + 1473;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt1", 10) == 0)
                    {
{
return dev_storage + 1458;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs1", 10) == 0)
                    {
{
return dev_storage + 1443;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr1", 10) == 0)
                    {
{
return dev_storage + 1428;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq1", 10) == 0)
                    {
{
return dev_storage + 1413;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp1", 10) == 0)
                    {
{
return dev_storage + 1398;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo1", 10) == 0)
                    {
{
return dev_storage + 1383;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn1", 10) == 0)
                    {
{
return dev_storage + 1368;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm1", 10) == 0)
                    {
{
return dev_storage + 1353;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl1", 10) == 0)
                    {
{
return dev_storage + 1338;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk1", 10) == 0)
                    {
{
return dev_storage + 1323;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj1", 10) == 0)
                    {
{
return dev_storage + 1308;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi1", 10) == 0)
                    {
{
return dev_storage + 1293;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh1", 10) == 0)
                    {
{
return dev_storage + 1278;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg1", 10) == 0)
                    {
{
return dev_storage + 1263;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf1", 10) == 0)
                    {
{
return dev_storage + 1248;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe1", 10) == 0)
                    {
{
return dev_storage + 1233;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd1", 10) == 0)
                    {
{
return dev_storage + 1218;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc1", 10) == 0)
                    {
{
return dev_storage + 1203;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb1", 10) == 0)
                    {
{
return dev_storage + 1188;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba1", 10) == 0)
                    {
{
return dev_storage + 1173;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/sdb11", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz1", 10) == 0)
                    {
{
return dev_storage + 1132;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday1", 10) == 0)
                    {
{
return dev_storage + 1117;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax1", 10) == 0)
                    {
{
return dev_storage + 1102;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw1", 10) == 0)
                    {
{
return dev_storage + 1087;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav1", 10) == 0)
                    {
{
return dev_storage + 1072;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau1", 10) == 0)
                    {
{
return dev_storage + 1057;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat1", 10) == 0)
                    {
{
return dev_storage + 1042;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas1", 10) == 0)
                    {
{
return dev_storage + 1027;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar1", 10) == 0)
                    {
{
return dev_storage + 1012;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq1", 10) == 0)
                    {
{
return dev_storage + 997;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap1", 10) == 0)
                    {
{
return dev_storage + 982;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao1", 10) == 0)
                    {
{
return dev_storage + 967;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan1", 10) == 0)
                    {
{
return dev_storage + 952;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam1", 10) == 0)
                    {
{
return dev_storage + 937;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal1", 10) == 0)
                    {
{
return dev_storage + 922;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak1", 10) == 0)
                    {
{
return dev_storage + 907;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj1", 10) == 0)
                    {
{
return dev_storage + 892;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai1", 10) == 0)
                    {
{
return dev_storage + 877;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah1", 10) == 0)
                    {
{
return dev_storage + 862;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag1", 10) == 0)
                    {
{
return dev_storage + 847;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf1", 10) == 0)
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
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae1", 10) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad1", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac1", 10) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab1", 10) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa1", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/sda11", 10) == 0)
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
                default:
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
return dev_storage + 2500;

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
return dev_storage + 2490;

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
return dev_storage + 2480;

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
return dev_storage + 725;

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
return dev_storage + 2508;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty60", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty50", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty40", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty30", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty20", 10) == 0)
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
                case '1':
                  switch (KR_keyword [5])
                    {
                    case 's':
                      if (strncmp (KR_keyword, "/dev/sdy10", 10) == 0)
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
                    case 'p':
                      if (strncmp (KR_keyword, "/dev/pty10", 10) == 0)
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
return dev_storage + 695;

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
return dev_storage + 680;

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
return dev_storage + 665;

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
return dev_storage + 650;

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
return dev_storage + 194;

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
return dev_storage + 184;

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
return dev_storage + 174;

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
return dev_storage + 164;

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
return dev_storage + 154;

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
return dev_storage + 144;

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
return dev_storage + 134;

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
return dev_storage + 124;

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
return dev_storage + 635;

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
return dev_storage + 114;

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
return dev_storage + 620;

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
return dev_storage + 605;

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
return dev_storage + 590;

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
return dev_storage + 575;

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
return dev_storage + 560;

}
                }
              else
                {
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdn10", 10) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons0", 10) == 0)
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
            case 'm':
              switch (KR_keyword [5])
                {
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdm10", 10) == 0)
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
return dev_storage + 515;

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
return dev_storage + 500;

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
return dev_storage + 485;

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
return dev_storage + 470;

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
return dev_storage + 455;

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
return dev_storage + 440;

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
return dev_storage + 425;

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
return dev_storage + 410;

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
return dev_storage + 395;

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
return dev_storage + 309;

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
return dev_storage + 380;

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
return dev_storage + 365;

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
return dev_storage + 350;

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
return dev_storage + 2499;

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
return dev_storage + 2489;

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
return dev_storage + 2479;

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
      switch (KR_keyword [10])
        {
        case 't':
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
        case 'm':
          if (strncmp (KR_keyword, "/dev/random", 11) == 0)
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
        case '9':
          switch (KR_keyword [9])
            {
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS59", 11) == 0)
                    {
{
return dev_storage + 2567;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons59", 11) == 0)
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
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS49", 11) == 0)
                    {
{
return dev_storage + 2557;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons49", 11) == 0)
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
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS39", 11) == 0)
                    {
{
return dev_storage + 2547;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons39", 11) == 0)
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
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS29", 11) == 0)
                    {
{
return dev_storage + 2537;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons29", 11) == 0)
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
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS19", 11) == 0)
                    {
{
return dev_storage + 2527;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst119", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons19", 11) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/nst109", 11) == 0)
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
            default:
{
return	NULL;

}
            }
        case '8':
          switch (KR_keyword [9])
            {
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS58", 11) == 0)
                    {
{
return dev_storage + 2566;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons58", 11) == 0)
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
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS48", 11) == 0)
                    {
{
return dev_storage + 2556;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons48", 11) == 0)
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
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS38", 11) == 0)
                    {
{
return dev_storage + 2546;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons38", 11) == 0)
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
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS28", 11) == 0)
                    {
{
return dev_storage + 2536;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons28", 11) == 0)
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
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS18", 11) == 0)
                    {
{
return dev_storage + 2526;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst118", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons18", 11) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/nst108", 11) == 0)
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
            default:
{
return	NULL;

}
            }
        case '7':
          switch (KR_keyword [9])
            {
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS57", 11) == 0)
                    {
{
return dev_storage + 2565;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons57", 11) == 0)
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
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS47", 11) == 0)
                    {
{
return dev_storage + 2555;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons47", 11) == 0)
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
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS37", 11) == 0)
                    {
{
return dev_storage + 2545;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons37", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS27", 11) == 0)
                    {
{
return dev_storage + 2535;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst127", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons27", 11) == 0)
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
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS17", 11) == 0)
                    {
{
return dev_storage + 2525;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst117", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons17", 11) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/nst107", 11) == 0)
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
            default:
{
return	NULL;

}
            }
        case '6':
          switch (KR_keyword [9])
            {
            case '5':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS56", 11) == 0)
                    {
{
return dev_storage + 2564;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons56", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case '4':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS46", 11) == 0)
                    {
{
return dev_storage + 2554;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons46", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case '3':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS36", 11) == 0)
                    {
{
return dev_storage + 2544;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons36", 11) == 0)
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
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS26", 11) == 0)
                    {
{
return dev_storage + 2534;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst126", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons26", 11) == 0)
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
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS16", 11) == 0)
                    {
{
return dev_storage + 2524;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/nst116", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/cons16", 11) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/nst106", 11) == 0)
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
            default:
{
return	NULL;

}
            }
        case '5':
          switch (KR_keyword [7])
            {
            case 'y':
              switch (KR_keyword [9])
                {
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS55", 11) == 0)
                    {
{
return dev_storage + 2563;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS45", 11) == 0)
                    {
{
return dev_storage + 2553;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS35", 11) == 0)
                    {
{
return dev_storage + 2543;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS25", 11) == 0)
                    {
{
return dev_storage + 2533;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS15", 11) == 0)
                    {
{
return dev_storage + 2523;

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
            case 't':
              switch (KR_keyword [9])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst125", 11) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst115", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst105", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '5':
                  if (strncmp (KR_keyword, "/dev/cons55", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/cons45", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/cons35", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/cons25", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/cons15", 11) == 0)
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
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx15", 11) == 0)
                    {
{
return dev_storage + 2362;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw15", 11) == 0)
                    {
{
return dev_storage + 2347;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv15", 11) == 0)
                    {
{
return dev_storage + 2332;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu15", 11) == 0)
                    {
{
return dev_storage + 2317;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt15", 11) == 0)
                    {
{
return dev_storage + 2302;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds15", 11) == 0)
                    {
{
return dev_storage + 2287;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr15", 11) == 0)
                    {
{
return dev_storage + 2272;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq15", 11) == 0)
                    {
{
return dev_storage + 2257;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp15", 11) == 0)
                    {
{
return dev_storage + 2242;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo15", 11) == 0)
                    {
{
return dev_storage + 2227;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn15", 11) == 0)
                    {
{
return dev_storage + 2212;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm15", 11) == 0)
                    {
{
return dev_storage + 2197;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl15", 11) == 0)
                    {
{
return dev_storage + 2182;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk15", 11) == 0)
                    {
{
return dev_storage + 2167;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj15", 11) == 0)
                    {
{
return dev_storage + 2152;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi15", 11) == 0)
                    {
{
return dev_storage + 2137;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh15", 11) == 0)
                    {
{
return dev_storage + 2122;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg15", 11) == 0)
                    {
{
return dev_storage + 2107;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf15", 11) == 0)
                    {
{
return dev_storage + 2092;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde15", 11) == 0)
                    {
{
return dev_storage + 2077;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd15", 11) == 0)
                    {
{
return dev_storage + 2062;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc15", 11) == 0)
                    {
{
return dev_storage + 2047;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb15", 11) == 0)
                    {
{
return dev_storage + 2032;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda15", 11) == 0)
                    {
{
return dev_storage + 2017;

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
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz15", 11) == 0)
                    {
{
return dev_storage + 1978;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy15", 11) == 0)
                    {
{
return dev_storage + 1963;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx15", 11) == 0)
                    {
{
return dev_storage + 1948;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw15", 11) == 0)
                    {
{
return dev_storage + 1933;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv15", 11) == 0)
                    {
{
return dev_storage + 1918;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu15", 11) == 0)
                    {
{
return dev_storage + 1903;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct15", 11) == 0)
                    {
{
return dev_storage + 1888;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs15", 11) == 0)
                    {
{
return dev_storage + 1873;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr15", 11) == 0)
                    {
{
return dev_storage + 1858;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq15", 11) == 0)
                    {
{
return dev_storage + 1843;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp15", 11) == 0)
                    {
{
return dev_storage + 1828;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco15", 11) == 0)
                    {
{
return dev_storage + 1813;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn15", 11) == 0)
                    {
{
return dev_storage + 1798;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm15", 11) == 0)
                    {
{
return dev_storage + 1783;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl15", 11) == 0)
                    {
{
return dev_storage + 1768;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck15", 11) == 0)
                    {
{
return dev_storage + 1753;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj15", 11) == 0)
                    {
{
return dev_storage + 1738;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci15", 11) == 0)
                    {
{
return dev_storage + 1723;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch15", 11) == 0)
                    {
{
return dev_storage + 1708;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg15", 11) == 0)
                    {
{
return dev_storage + 1693;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf15", 11) == 0)
                    {
{
return dev_storage + 1678;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce15", 11) == 0)
                    {
{
return dev_storage + 1663;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd15", 11) == 0)
                    {
{
return dev_storage + 1648;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc15", 11) == 0)
                    {
{
return dev_storage + 1633;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb15", 11) == 0)
                    {
{
return dev_storage + 1618;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca15", 11) == 0)
                    {
{
return dev_storage + 1603;

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
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz15", 11) == 0)
                    {
{
return dev_storage + 1562;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby15", 11) == 0)
                    {
{
return dev_storage + 1547;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx15", 11) == 0)
                    {
{
return dev_storage + 1532;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw15", 11) == 0)
                    {
{
return dev_storage + 1517;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv15", 11) == 0)
                    {
{
return dev_storage + 1502;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu15", 11) == 0)
                    {
{
return dev_storage + 1487;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt15", 11) == 0)
                    {
{
return dev_storage + 1472;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs15", 11) == 0)
                    {
{
return dev_storage + 1457;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr15", 11) == 0)
                    {
{
return dev_storage + 1442;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq15", 11) == 0)
                    {
{
return dev_storage + 1427;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp15", 11) == 0)
                    {
{
return dev_storage + 1412;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo15", 11) == 0)
                    {
{
return dev_storage + 1397;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn15", 11) == 0)
                    {
{
return dev_storage + 1382;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm15", 11) == 0)
                    {
{
return dev_storage + 1367;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl15", 11) == 0)
                    {
{
return dev_storage + 1352;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk15", 11) == 0)
                    {
{
return dev_storage + 1337;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj15", 11) == 0)
                    {
{
return dev_storage + 1322;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi15", 11) == 0)
                    {
{
return dev_storage + 1307;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh15", 11) == 0)
                    {
{
return dev_storage + 1292;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg15", 11) == 0)
                    {
{
return dev_storage + 1277;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf15", 11) == 0)
                    {
{
return dev_storage + 1262;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe15", 11) == 0)
                    {
{
return dev_storage + 1247;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd15", 11) == 0)
                    {
{
return dev_storage + 1232;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc15", 11) == 0)
                    {
{
return dev_storage + 1217;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb15", 11) == 0)
                    {
{
return dev_storage + 1202;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba15", 11) == 0)
                    {
{
return dev_storage + 1187;

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
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz15", 11) == 0)
                    {
{
return dev_storage + 1146;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday15", 11) == 0)
                    {
{
return dev_storage + 1131;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax15", 11) == 0)
                    {
{
return dev_storage + 1116;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw15", 11) == 0)
                    {
{
return dev_storage + 1101;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav15", 11) == 0)
                    {
{
return dev_storage + 1086;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau15", 11) == 0)
                    {
{
return dev_storage + 1071;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat15", 11) == 0)
                    {
{
return dev_storage + 1056;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas15", 11) == 0)
                    {
{
return dev_storage + 1041;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar15", 11) == 0)
                    {
{
return dev_storage + 1026;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq15", 11) == 0)
                    {
{
return dev_storage + 1011;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap15", 11) == 0)
                    {
{
return dev_storage + 996;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao15", 11) == 0)
                    {
{
return dev_storage + 981;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan15", 11) == 0)
                    {
{
return dev_storage + 966;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam15", 11) == 0)
                    {
{
return dev_storage + 951;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal15", 11) == 0)
                    {
{
return dev_storage + 936;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak15", 11) == 0)
                    {
{
return dev_storage + 921;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj15", 11) == 0)
                    {
{
return dev_storage + 906;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai15", 11) == 0)
                    {
{
return dev_storage + 891;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah15", 11) == 0)
                    {
{
return dev_storage + 876;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag15", 11) == 0)
                    {
{
return dev_storage + 861;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf15", 11) == 0)
                    {
{
return dev_storage + 846;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae15", 11) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad15", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac15", 11) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab15", 11) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa15", 11) == 0)
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
            case 'y':
              switch (KR_keyword [9])
                {
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS54", 11) == 0)
                    {
{
return dev_storage + 2562;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS44", 11) == 0)
                    {
{
return dev_storage + 2552;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS34", 11) == 0)
                    {
{
return dev_storage + 2542;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS24", 11) == 0)
                    {
{
return dev_storage + 2532;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS14", 11) == 0)
                    {
{
return dev_storage + 2522;

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
            case 't':
              switch (KR_keyword [9])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst124", 11) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst114", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst104", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '5':
                  if (strncmp (KR_keyword, "/dev/cons54", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/cons44", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/cons34", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/cons24", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/cons14", 11) == 0)
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
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx14", 11) == 0)
                    {
{
return dev_storage + 2361;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw14", 11) == 0)
                    {
{
return dev_storage + 2346;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv14", 11) == 0)
                    {
{
return dev_storage + 2331;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu14", 11) == 0)
                    {
{
return dev_storage + 2316;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt14", 11) == 0)
                    {
{
return dev_storage + 2301;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds14", 11) == 0)
                    {
{
return dev_storage + 2286;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr14", 11) == 0)
                    {
{
return dev_storage + 2271;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq14", 11) == 0)
                    {
{
return dev_storage + 2256;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp14", 11) == 0)
                    {
{
return dev_storage + 2241;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo14", 11) == 0)
                    {
{
return dev_storage + 2226;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn14", 11) == 0)
                    {
{
return dev_storage + 2211;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm14", 11) == 0)
                    {
{
return dev_storage + 2196;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl14", 11) == 0)
                    {
{
return dev_storage + 2181;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk14", 11) == 0)
                    {
{
return dev_storage + 2166;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj14", 11) == 0)
                    {
{
return dev_storage + 2151;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi14", 11) == 0)
                    {
{
return dev_storage + 2136;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh14", 11) == 0)
                    {
{
return dev_storage + 2121;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg14", 11) == 0)
                    {
{
return dev_storage + 2106;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf14", 11) == 0)
                    {
{
return dev_storage + 2091;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde14", 11) == 0)
                    {
{
return dev_storage + 2076;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd14", 11) == 0)
                    {
{
return dev_storage + 2061;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc14", 11) == 0)
                    {
{
return dev_storage + 2046;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb14", 11) == 0)
                    {
{
return dev_storage + 2031;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda14", 11) == 0)
                    {
{
return dev_storage + 2016;

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
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz14", 11) == 0)
                    {
{
return dev_storage + 1977;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy14", 11) == 0)
                    {
{
return dev_storage + 1962;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx14", 11) == 0)
                    {
{
return dev_storage + 1947;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw14", 11) == 0)
                    {
{
return dev_storage + 1932;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv14", 11) == 0)
                    {
{
return dev_storage + 1917;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu14", 11) == 0)
                    {
{
return dev_storage + 1902;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct14", 11) == 0)
                    {
{
return dev_storage + 1887;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs14", 11) == 0)
                    {
{
return dev_storage + 1872;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr14", 11) == 0)
                    {
{
return dev_storage + 1857;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq14", 11) == 0)
                    {
{
return dev_storage + 1842;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp14", 11) == 0)
                    {
{
return dev_storage + 1827;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco14", 11) == 0)
                    {
{
return dev_storage + 1812;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn14", 11) == 0)
                    {
{
return dev_storage + 1797;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm14", 11) == 0)
                    {
{
return dev_storage + 1782;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl14", 11) == 0)
                    {
{
return dev_storage + 1767;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck14", 11) == 0)
                    {
{
return dev_storage + 1752;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj14", 11) == 0)
                    {
{
return dev_storage + 1737;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci14", 11) == 0)
                    {
{
return dev_storage + 1722;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch14", 11) == 0)
                    {
{
return dev_storage + 1707;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg14", 11) == 0)
                    {
{
return dev_storage + 1692;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf14", 11) == 0)
                    {
{
return dev_storage + 1677;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce14", 11) == 0)
                    {
{
return dev_storage + 1662;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd14", 11) == 0)
                    {
{
return dev_storage + 1647;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc14", 11) == 0)
                    {
{
return dev_storage + 1632;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb14", 11) == 0)
                    {
{
return dev_storage + 1617;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca14", 11) == 0)
                    {
{
return dev_storage + 1602;

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
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz14", 11) == 0)
                    {
{
return dev_storage + 1561;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby14", 11) == 0)
                    {
{
return dev_storage + 1546;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx14", 11) == 0)
                    {
{
return dev_storage + 1531;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw14", 11) == 0)
                    {
{
return dev_storage + 1516;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv14", 11) == 0)
                    {
{
return dev_storage + 1501;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu14", 11) == 0)
                    {
{
return dev_storage + 1486;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt14", 11) == 0)
                    {
{
return dev_storage + 1471;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs14", 11) == 0)
                    {
{
return dev_storage + 1456;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr14", 11) == 0)
                    {
{
return dev_storage + 1441;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq14", 11) == 0)
                    {
{
return dev_storage + 1426;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp14", 11) == 0)
                    {
{
return dev_storage + 1411;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo14", 11) == 0)
                    {
{
return dev_storage + 1396;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn14", 11) == 0)
                    {
{
return dev_storage + 1381;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm14", 11) == 0)
                    {
{
return dev_storage + 1366;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl14", 11) == 0)
                    {
{
return dev_storage + 1351;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk14", 11) == 0)
                    {
{
return dev_storage + 1336;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj14", 11) == 0)
                    {
{
return dev_storage + 1321;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi14", 11) == 0)
                    {
{
return dev_storage + 1306;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh14", 11) == 0)
                    {
{
return dev_storage + 1291;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg14", 11) == 0)
                    {
{
return dev_storage + 1276;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf14", 11) == 0)
                    {
{
return dev_storage + 1261;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe14", 11) == 0)
                    {
{
return dev_storage + 1246;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd14", 11) == 0)
                    {
{
return dev_storage + 1231;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc14", 11) == 0)
                    {
{
return dev_storage + 1216;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb14", 11) == 0)
                    {
{
return dev_storage + 1201;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba14", 11) == 0)
                    {
{
return dev_storage + 1186;

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
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz14", 11) == 0)
                    {
{
return dev_storage + 1145;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday14", 11) == 0)
                    {
{
return dev_storage + 1130;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax14", 11) == 0)
                    {
{
return dev_storage + 1115;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw14", 11) == 0)
                    {
{
return dev_storage + 1100;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav14", 11) == 0)
                    {
{
return dev_storage + 1085;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau14", 11) == 0)
                    {
{
return dev_storage + 1070;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat14", 11) == 0)
                    {
{
return dev_storage + 1055;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas14", 11) == 0)
                    {
{
return dev_storage + 1040;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar14", 11) == 0)
                    {
{
return dev_storage + 1025;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq14", 11) == 0)
                    {
{
return dev_storage + 1010;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap14", 11) == 0)
                    {
{
return dev_storage + 995;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao14", 11) == 0)
                    {
{
return dev_storage + 980;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan14", 11) == 0)
                    {
{
return dev_storage + 965;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam14", 11) == 0)
                    {
{
return dev_storage + 950;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal14", 11) == 0)
                    {
{
return dev_storage + 935;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak14", 11) == 0)
                    {
{
return dev_storage + 920;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj14", 11) == 0)
                    {
{
return dev_storage + 905;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai14", 11) == 0)
                    {
{
return dev_storage + 890;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah14", 11) == 0)
                    {
{
return dev_storage + 875;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag14", 11) == 0)
                    {
{
return dev_storage + 860;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf14", 11) == 0)
                    {
{
return dev_storage + 845;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae14", 11) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad14", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac14", 11) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab14", 11) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa14", 11) == 0)
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
            case 'y':
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS63", 11) == 0)
                    {
{
return dev_storage + 2571;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS53", 11) == 0)
                    {
{
return dev_storage + 2561;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS43", 11) == 0)
                    {
{
return dev_storage + 2551;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS33", 11) == 0)
                    {
{
return dev_storage + 2541;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS23", 11) == 0)
                    {
{
return dev_storage + 2531;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS13", 11) == 0)
                    {
{
return dev_storage + 2521;

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
            case 't':
              switch (KR_keyword [9])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst123", 11) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst113", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst103", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/cons63", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/cons53", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/cons43", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/cons33", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/cons23", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/cons13", 11) == 0)
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
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx13", 11) == 0)
                    {
{
return dev_storage + 2360;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw13", 11) == 0)
                    {
{
return dev_storage + 2345;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv13", 11) == 0)
                    {
{
return dev_storage + 2330;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu13", 11) == 0)
                    {
{
return dev_storage + 2315;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt13", 11) == 0)
                    {
{
return dev_storage + 2300;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds13", 11) == 0)
                    {
{
return dev_storage + 2285;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr13", 11) == 0)
                    {
{
return dev_storage + 2270;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq13", 11) == 0)
                    {
{
return dev_storage + 2255;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp13", 11) == 0)
                    {
{
return dev_storage + 2240;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo13", 11) == 0)
                    {
{
return dev_storage + 2225;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn13", 11) == 0)
                    {
{
return dev_storage + 2210;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm13", 11) == 0)
                    {
{
return dev_storage + 2195;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl13", 11) == 0)
                    {
{
return dev_storage + 2180;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk13", 11) == 0)
                    {
{
return dev_storage + 2165;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj13", 11) == 0)
                    {
{
return dev_storage + 2150;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi13", 11) == 0)
                    {
{
return dev_storage + 2135;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh13", 11) == 0)
                    {
{
return dev_storage + 2120;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg13", 11) == 0)
                    {
{
return dev_storage + 2105;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf13", 11) == 0)
                    {
{
return dev_storage + 2090;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde13", 11) == 0)
                    {
{
return dev_storage + 2075;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd13", 11) == 0)
                    {
{
return dev_storage + 2060;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc13", 11) == 0)
                    {
{
return dev_storage + 2045;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb13", 11) == 0)
                    {
{
return dev_storage + 2030;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda13", 11) == 0)
                    {
{
return dev_storage + 2015;

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
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz13", 11) == 0)
                    {
{
return dev_storage + 1976;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy13", 11) == 0)
                    {
{
return dev_storage + 1961;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx13", 11) == 0)
                    {
{
return dev_storage + 1946;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw13", 11) == 0)
                    {
{
return dev_storage + 1931;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv13", 11) == 0)
                    {
{
return dev_storage + 1916;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu13", 11) == 0)
                    {
{
return dev_storage + 1901;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct13", 11) == 0)
                    {
{
return dev_storage + 1886;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs13", 11) == 0)
                    {
{
return dev_storage + 1871;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr13", 11) == 0)
                    {
{
return dev_storage + 1856;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq13", 11) == 0)
                    {
{
return dev_storage + 1841;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp13", 11) == 0)
                    {
{
return dev_storage + 1826;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco13", 11) == 0)
                    {
{
return dev_storage + 1811;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn13", 11) == 0)
                    {
{
return dev_storage + 1796;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm13", 11) == 0)
                    {
{
return dev_storage + 1781;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl13", 11) == 0)
                    {
{
return dev_storage + 1766;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck13", 11) == 0)
                    {
{
return dev_storage + 1751;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj13", 11) == 0)
                    {
{
return dev_storage + 1736;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci13", 11) == 0)
                    {
{
return dev_storage + 1721;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch13", 11) == 0)
                    {
{
return dev_storage + 1706;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg13", 11) == 0)
                    {
{
return dev_storage + 1691;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf13", 11) == 0)
                    {
{
return dev_storage + 1676;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce13", 11) == 0)
                    {
{
return dev_storage + 1661;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd13", 11) == 0)
                    {
{
return dev_storage + 1646;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc13", 11) == 0)
                    {
{
return dev_storage + 1631;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb13", 11) == 0)
                    {
{
return dev_storage + 1616;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca13", 11) == 0)
                    {
{
return dev_storage + 1601;

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
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz13", 11) == 0)
                    {
{
return dev_storage + 1560;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby13", 11) == 0)
                    {
{
return dev_storage + 1545;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx13", 11) == 0)
                    {
{
return dev_storage + 1530;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw13", 11) == 0)
                    {
{
return dev_storage + 1515;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv13", 11) == 0)
                    {
{
return dev_storage + 1500;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu13", 11) == 0)
                    {
{
return dev_storage + 1485;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt13", 11) == 0)
                    {
{
return dev_storage + 1470;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs13", 11) == 0)
                    {
{
return dev_storage + 1455;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr13", 11) == 0)
                    {
{
return dev_storage + 1440;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq13", 11) == 0)
                    {
{
return dev_storage + 1425;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp13", 11) == 0)
                    {
{
return dev_storage + 1410;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo13", 11) == 0)
                    {
{
return dev_storage + 1395;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn13", 11) == 0)
                    {
{
return dev_storage + 1380;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm13", 11) == 0)
                    {
{
return dev_storage + 1365;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl13", 11) == 0)
                    {
{
return dev_storage + 1350;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk13", 11) == 0)
                    {
{
return dev_storage + 1335;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj13", 11) == 0)
                    {
{
return dev_storage + 1320;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi13", 11) == 0)
                    {
{
return dev_storage + 1305;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh13", 11) == 0)
                    {
{
return dev_storage + 1290;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg13", 11) == 0)
                    {
{
return dev_storage + 1275;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf13", 11) == 0)
                    {
{
return dev_storage + 1260;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe13", 11) == 0)
                    {
{
return dev_storage + 1245;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd13", 11) == 0)
                    {
{
return dev_storage + 1230;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc13", 11) == 0)
                    {
{
return dev_storage + 1215;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb13", 11) == 0)
                    {
{
return dev_storage + 1200;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba13", 11) == 0)
                    {
{
return dev_storage + 1185;

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
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz13", 11) == 0)
                    {
{
return dev_storage + 1144;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday13", 11) == 0)
                    {
{
return dev_storage + 1129;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax13", 11) == 0)
                    {
{
return dev_storage + 1114;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw13", 11) == 0)
                    {
{
return dev_storage + 1099;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav13", 11) == 0)
                    {
{
return dev_storage + 1084;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau13", 11) == 0)
                    {
{
return dev_storage + 1069;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat13", 11) == 0)
                    {
{
return dev_storage + 1054;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas13", 11) == 0)
                    {
{
return dev_storage + 1039;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar13", 11) == 0)
                    {
{
return dev_storage + 1024;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq13", 11) == 0)
                    {
{
return dev_storage + 1009;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap13", 11) == 0)
                    {
{
return dev_storage + 994;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao13", 11) == 0)
                    {
{
return dev_storage + 979;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan13", 11) == 0)
                    {
{
return dev_storage + 964;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam13", 11) == 0)
                    {
{
return dev_storage + 949;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal13", 11) == 0)
                    {
{
return dev_storage + 934;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak13", 11) == 0)
                    {
{
return dev_storage + 919;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj13", 11) == 0)
                    {
{
return dev_storage + 904;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai13", 11) == 0)
                    {
{
return dev_storage + 889;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah13", 11) == 0)
                    {
{
return dev_storage + 874;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag13", 11) == 0)
                    {
{
return dev_storage + 859;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf13", 11) == 0)
                    {
{
return dev_storage + 844;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae13", 11) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad13", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac13", 11) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab13", 11) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa13", 11) == 0)
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
            case 'y':
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS62", 11) == 0)
                    {
{
return dev_storage + 2570;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS52", 11) == 0)
                    {
{
return dev_storage + 2560;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS42", 11) == 0)
                    {
{
return dev_storage + 2550;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS32", 11) == 0)
                    {
{
return dev_storage + 2540;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS22", 11) == 0)
                    {
{
return dev_storage + 2530;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS12", 11) == 0)
                    {
{
return dev_storage + 2520;

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
            case 't':
              switch (KR_keyword [9])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst122", 11) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst112", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst102", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/cons62", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/cons52", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/cons42", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/cons32", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/cons22", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/cons12", 11) == 0)
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
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx12", 11) == 0)
                    {
{
return dev_storage + 2359;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw12", 11) == 0)
                    {
{
return dev_storage + 2344;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv12", 11) == 0)
                    {
{
return dev_storage + 2329;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu12", 11) == 0)
                    {
{
return dev_storage + 2314;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt12", 11) == 0)
                    {
{
return dev_storage + 2299;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds12", 11) == 0)
                    {
{
return dev_storage + 2284;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr12", 11) == 0)
                    {
{
return dev_storage + 2269;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq12", 11) == 0)
                    {
{
return dev_storage + 2254;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp12", 11) == 0)
                    {
{
return dev_storage + 2239;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo12", 11) == 0)
                    {
{
return dev_storage + 2224;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn12", 11) == 0)
                    {
{
return dev_storage + 2209;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm12", 11) == 0)
                    {
{
return dev_storage + 2194;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl12", 11) == 0)
                    {
{
return dev_storage + 2179;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk12", 11) == 0)
                    {
{
return dev_storage + 2164;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj12", 11) == 0)
                    {
{
return dev_storage + 2149;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi12", 11) == 0)
                    {
{
return dev_storage + 2134;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh12", 11) == 0)
                    {
{
return dev_storage + 2119;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg12", 11) == 0)
                    {
{
return dev_storage + 2104;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf12", 11) == 0)
                    {
{
return dev_storage + 2089;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde12", 11) == 0)
                    {
{
return dev_storage + 2074;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd12", 11) == 0)
                    {
{
return dev_storage + 2059;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc12", 11) == 0)
                    {
{
return dev_storage + 2044;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb12", 11) == 0)
                    {
{
return dev_storage + 2029;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda12", 11) == 0)
                    {
{
return dev_storage + 2014;

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
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz12", 11) == 0)
                    {
{
return dev_storage + 1975;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy12", 11) == 0)
                    {
{
return dev_storage + 1960;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx12", 11) == 0)
                    {
{
return dev_storage + 1945;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw12", 11) == 0)
                    {
{
return dev_storage + 1930;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv12", 11) == 0)
                    {
{
return dev_storage + 1915;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu12", 11) == 0)
                    {
{
return dev_storage + 1900;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct12", 11) == 0)
                    {
{
return dev_storage + 1885;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs12", 11) == 0)
                    {
{
return dev_storage + 1870;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr12", 11) == 0)
                    {
{
return dev_storage + 1855;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq12", 11) == 0)
                    {
{
return dev_storage + 1840;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp12", 11) == 0)
                    {
{
return dev_storage + 1825;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco12", 11) == 0)
                    {
{
return dev_storage + 1810;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn12", 11) == 0)
                    {
{
return dev_storage + 1795;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm12", 11) == 0)
                    {
{
return dev_storage + 1780;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl12", 11) == 0)
                    {
{
return dev_storage + 1765;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck12", 11) == 0)
                    {
{
return dev_storage + 1750;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj12", 11) == 0)
                    {
{
return dev_storage + 1735;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci12", 11) == 0)
                    {
{
return dev_storage + 1720;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch12", 11) == 0)
                    {
{
return dev_storage + 1705;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg12", 11) == 0)
                    {
{
return dev_storage + 1690;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf12", 11) == 0)
                    {
{
return dev_storage + 1675;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce12", 11) == 0)
                    {
{
return dev_storage + 1660;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd12", 11) == 0)
                    {
{
return dev_storage + 1645;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc12", 11) == 0)
                    {
{
return dev_storage + 1630;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb12", 11) == 0)
                    {
{
return dev_storage + 1615;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca12", 11) == 0)
                    {
{
return dev_storage + 1600;

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
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz12", 11) == 0)
                    {
{
return dev_storage + 1559;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby12", 11) == 0)
                    {
{
return dev_storage + 1544;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx12", 11) == 0)
                    {
{
return dev_storage + 1529;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw12", 11) == 0)
                    {
{
return dev_storage + 1514;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv12", 11) == 0)
                    {
{
return dev_storage + 1499;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu12", 11) == 0)
                    {
{
return dev_storage + 1484;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt12", 11) == 0)
                    {
{
return dev_storage + 1469;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs12", 11) == 0)
                    {
{
return dev_storage + 1454;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr12", 11) == 0)
                    {
{
return dev_storage + 1439;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq12", 11) == 0)
                    {
{
return dev_storage + 1424;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp12", 11) == 0)
                    {
{
return dev_storage + 1409;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo12", 11) == 0)
                    {
{
return dev_storage + 1394;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn12", 11) == 0)
                    {
{
return dev_storage + 1379;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm12", 11) == 0)
                    {
{
return dev_storage + 1364;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl12", 11) == 0)
                    {
{
return dev_storage + 1349;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk12", 11) == 0)
                    {
{
return dev_storage + 1334;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj12", 11) == 0)
                    {
{
return dev_storage + 1319;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi12", 11) == 0)
                    {
{
return dev_storage + 1304;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh12", 11) == 0)
                    {
{
return dev_storage + 1289;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg12", 11) == 0)
                    {
{
return dev_storage + 1274;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf12", 11) == 0)
                    {
{
return dev_storage + 1259;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe12", 11) == 0)
                    {
{
return dev_storage + 1244;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd12", 11) == 0)
                    {
{
return dev_storage + 1229;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc12", 11) == 0)
                    {
{
return dev_storage + 1214;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb12", 11) == 0)
                    {
{
return dev_storage + 1199;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba12", 11) == 0)
                    {
{
return dev_storage + 1184;

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
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz12", 11) == 0)
                    {
{
return dev_storage + 1143;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday12", 11) == 0)
                    {
{
return dev_storage + 1128;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax12", 11) == 0)
                    {
{
return dev_storage + 1113;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw12", 11) == 0)
                    {
{
return dev_storage + 1098;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav12", 11) == 0)
                    {
{
return dev_storage + 1083;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau12", 11) == 0)
                    {
{
return dev_storage + 1068;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat12", 11) == 0)
                    {
{
return dev_storage + 1053;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas12", 11) == 0)
                    {
{
return dev_storage + 1038;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar12", 11) == 0)
                    {
{
return dev_storage + 1023;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq12", 11) == 0)
                    {
{
return dev_storage + 1008;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap12", 11) == 0)
                    {
{
return dev_storage + 993;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao12", 11) == 0)
                    {
{
return dev_storage + 978;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan12", 11) == 0)
                    {
{
return dev_storage + 963;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam12", 11) == 0)
                    {
{
return dev_storage + 948;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal12", 11) == 0)
                    {
{
return dev_storage + 933;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak12", 11) == 0)
                    {
{
return dev_storage + 918;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj12", 11) == 0)
                    {
{
return dev_storage + 903;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai12", 11) == 0)
                    {
{
return dev_storage + 888;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah12", 11) == 0)
                    {
{
return dev_storage + 873;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag12", 11) == 0)
                    {
{
return dev_storage + 858;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf12", 11) == 0)
                    {
{
return dev_storage + 843;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae12", 11) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad12", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac12", 11) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab12", 11) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa12", 11) == 0)
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
            case 'y':
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS61", 11) == 0)
                    {
{
return dev_storage + 2569;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS51", 11) == 0)
                    {
{
return dev_storage + 2559;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS41", 11) == 0)
                    {
{
return dev_storage + 2549;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS31", 11) == 0)
                    {
{
return dev_storage + 2539;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS21", 11) == 0)
                    {
{
return dev_storage + 2529;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS11", 11) == 0)
                    {
{
return dev_storage + 2519;

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
            case 't':
              switch (KR_keyword [9])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst121", 11) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst111", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst101", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/cons61", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/cons51", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/cons41", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/cons31", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/cons21", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/cons11", 11) == 0)
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
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx11", 11) == 0)
                    {
{
return dev_storage + 2358;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw11", 11) == 0)
                    {
{
return dev_storage + 2343;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv11", 11) == 0)
                    {
{
return dev_storage + 2328;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu11", 11) == 0)
                    {
{
return dev_storage + 2313;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt11", 11) == 0)
                    {
{
return dev_storage + 2298;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds11", 11) == 0)
                    {
{
return dev_storage + 2283;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr11", 11) == 0)
                    {
{
return dev_storage + 2268;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq11", 11) == 0)
                    {
{
return dev_storage + 2253;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp11", 11) == 0)
                    {
{
return dev_storage + 2238;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo11", 11) == 0)
                    {
{
return dev_storage + 2223;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn11", 11) == 0)
                    {
{
return dev_storage + 2208;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm11", 11) == 0)
                    {
{
return dev_storage + 2193;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl11", 11) == 0)
                    {
{
return dev_storage + 2178;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk11", 11) == 0)
                    {
{
return dev_storage + 2163;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj11", 11) == 0)
                    {
{
return dev_storage + 2148;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi11", 11) == 0)
                    {
{
return dev_storage + 2133;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh11", 11) == 0)
                    {
{
return dev_storage + 2118;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg11", 11) == 0)
                    {
{
return dev_storage + 2103;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf11", 11) == 0)
                    {
{
return dev_storage + 2088;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde11", 11) == 0)
                    {
{
return dev_storage + 2073;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd11", 11) == 0)
                    {
{
return dev_storage + 2058;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc11", 11) == 0)
                    {
{
return dev_storage + 2043;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb11", 11) == 0)
                    {
{
return dev_storage + 2028;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda11", 11) == 0)
                    {
{
return dev_storage + 2013;

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
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz11", 11) == 0)
                    {
{
return dev_storage + 1974;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy11", 11) == 0)
                    {
{
return dev_storage + 1959;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx11", 11) == 0)
                    {
{
return dev_storage + 1944;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw11", 11) == 0)
                    {
{
return dev_storage + 1929;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv11", 11) == 0)
                    {
{
return dev_storage + 1914;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu11", 11) == 0)
                    {
{
return dev_storage + 1899;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct11", 11) == 0)
                    {
{
return dev_storage + 1884;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs11", 11) == 0)
                    {
{
return dev_storage + 1869;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr11", 11) == 0)
                    {
{
return dev_storage + 1854;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq11", 11) == 0)
                    {
{
return dev_storage + 1839;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp11", 11) == 0)
                    {
{
return dev_storage + 1824;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco11", 11) == 0)
                    {
{
return dev_storage + 1809;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn11", 11) == 0)
                    {
{
return dev_storage + 1794;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm11", 11) == 0)
                    {
{
return dev_storage + 1779;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl11", 11) == 0)
                    {
{
return dev_storage + 1764;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck11", 11) == 0)
                    {
{
return dev_storage + 1749;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj11", 11) == 0)
                    {
{
return dev_storage + 1734;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci11", 11) == 0)
                    {
{
return dev_storage + 1719;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch11", 11) == 0)
                    {
{
return dev_storage + 1704;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg11", 11) == 0)
                    {
{
return dev_storage + 1689;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf11", 11) == 0)
                    {
{
return dev_storage + 1674;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce11", 11) == 0)
                    {
{
return dev_storage + 1659;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd11", 11) == 0)
                    {
{
return dev_storage + 1644;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc11", 11) == 0)
                    {
{
return dev_storage + 1629;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb11", 11) == 0)
                    {
{
return dev_storage + 1614;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca11", 11) == 0)
                    {
{
return dev_storage + 1599;

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
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz11", 11) == 0)
                    {
{
return dev_storage + 1558;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby11", 11) == 0)
                    {
{
return dev_storage + 1543;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx11", 11) == 0)
                    {
{
return dev_storage + 1528;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw11", 11) == 0)
                    {
{
return dev_storage + 1513;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv11", 11) == 0)
                    {
{
return dev_storage + 1498;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu11", 11) == 0)
                    {
{
return dev_storage + 1483;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt11", 11) == 0)
                    {
{
return dev_storage + 1468;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs11", 11) == 0)
                    {
{
return dev_storage + 1453;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr11", 11) == 0)
                    {
{
return dev_storage + 1438;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq11", 11) == 0)
                    {
{
return dev_storage + 1423;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp11", 11) == 0)
                    {
{
return dev_storage + 1408;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo11", 11) == 0)
                    {
{
return dev_storage + 1393;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn11", 11) == 0)
                    {
{
return dev_storage + 1378;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm11", 11) == 0)
                    {
{
return dev_storage + 1363;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl11", 11) == 0)
                    {
{
return dev_storage + 1348;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk11", 11) == 0)
                    {
{
return dev_storage + 1333;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj11", 11) == 0)
                    {
{
return dev_storage + 1318;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi11", 11) == 0)
                    {
{
return dev_storage + 1303;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh11", 11) == 0)
                    {
{
return dev_storage + 1288;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg11", 11) == 0)
                    {
{
return dev_storage + 1273;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf11", 11) == 0)
                    {
{
return dev_storage + 1258;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe11", 11) == 0)
                    {
{
return dev_storage + 1243;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd11", 11) == 0)
                    {
{
return dev_storage + 1228;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc11", 11) == 0)
                    {
{
return dev_storage + 1213;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb11", 11) == 0)
                    {
{
return dev_storage + 1198;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba11", 11) == 0)
                    {
{
return dev_storage + 1183;

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
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz11", 11) == 0)
                    {
{
return dev_storage + 1142;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday11", 11) == 0)
                    {
{
return dev_storage + 1127;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax11", 11) == 0)
                    {
{
return dev_storage + 1112;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw11", 11) == 0)
                    {
{
return dev_storage + 1097;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav11", 11) == 0)
                    {
{
return dev_storage + 1082;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau11", 11) == 0)
                    {
{
return dev_storage + 1067;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat11", 11) == 0)
                    {
{
return dev_storage + 1052;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas11", 11) == 0)
                    {
{
return dev_storage + 1037;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar11", 11) == 0)
                    {
{
return dev_storage + 1022;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq11", 11) == 0)
                    {
{
return dev_storage + 1007;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap11", 11) == 0)
                    {
{
return dev_storage + 992;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao11", 11) == 0)
                    {
{
return dev_storage + 977;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan11", 11) == 0)
                    {
{
return dev_storage + 962;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam11", 11) == 0)
                    {
{
return dev_storage + 947;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal11", 11) == 0)
                    {
{
return dev_storage + 932;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak11", 11) == 0)
                    {
{
return dev_storage + 917;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj11", 11) == 0)
                    {
{
return dev_storage + 902;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai11", 11) == 0)
                    {
{
return dev_storage + 887;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah11", 11) == 0)
                    {
{
return dev_storage + 872;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag11", 11) == 0)
                    {
{
return dev_storage + 857;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf11", 11) == 0)
                    {
{
return dev_storage + 842;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae11", 11) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad11", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac11", 11) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab11", 11) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa11", 11) == 0)
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
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS60", 11) == 0)
                    {
{
return dev_storage + 2568;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS50", 11) == 0)
                    {
{
return dev_storage + 2558;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS40", 11) == 0)
                    {
{
return dev_storage + 2548;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS30", 11) == 0)
                    {
{
return dev_storage + 2538;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS20", 11) == 0)
                    {
{
return dev_storage + 2528;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS10", 11) == 0)
                    {
{
return dev_storage + 2518;

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
            case 't':
              switch (KR_keyword [9])
                {
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst120", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst110", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst100", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '6':
                  if (strncmp (KR_keyword, "/dev/cons60", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/cons50", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/cons40", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/cons30", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/cons20", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/cons10", 11) == 0)
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
            case 'd':
              switch (KR_keyword [8])
                {
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sddx10", 11) == 0)
                    {
{
return dev_storage + 2357;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sddw10", 11) == 0)
                    {
{
return dev_storage + 2342;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sddv10", 11) == 0)
                    {
{
return dev_storage + 2327;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sddu10", 11) == 0)
                    {
{
return dev_storage + 2312;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sddt10", 11) == 0)
                    {
{
return dev_storage + 2297;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdds10", 11) == 0)
                    {
{
return dev_storage + 2282;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sddr10", 11) == 0)
                    {
{
return dev_storage + 2267;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sddq10", 11) == 0)
                    {
{
return dev_storage + 2252;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sddp10", 11) == 0)
                    {
{
return dev_storage + 2237;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sddo10", 11) == 0)
                    {
{
return dev_storage + 2222;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sddn10", 11) == 0)
                    {
{
return dev_storage + 2207;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sddm10", 11) == 0)
                    {
{
return dev_storage + 2192;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sddl10", 11) == 0)
                    {
{
return dev_storage + 2177;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sddk10", 11) == 0)
                    {
{
return dev_storage + 2162;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sddj10", 11) == 0)
                    {
{
return dev_storage + 2147;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sddi10", 11) == 0)
                    {
{
return dev_storage + 2132;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sddh10", 11) == 0)
                    {
{
return dev_storage + 2117;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sddg10", 11) == 0)
                    {
{
return dev_storage + 2102;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sddf10", 11) == 0)
                    {
{
return dev_storage + 2087;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdde10", 11) == 0)
                    {
{
return dev_storage + 2072;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sddd10", 11) == 0)
                    {
{
return dev_storage + 2057;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sddc10", 11) == 0)
                    {
{
return dev_storage + 2042;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sddb10", 11) == 0)
                    {
{
return dev_storage + 2027;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdda10", 11) == 0)
                    {
{
return dev_storage + 2012;

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
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdcz10", 11) == 0)
                    {
{
return dev_storage + 1973;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdcy10", 11) == 0)
                    {
{
return dev_storage + 1958;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdcx10", 11) == 0)
                    {
{
return dev_storage + 1943;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdcw10", 11) == 0)
                    {
{
return dev_storage + 1928;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdcv10", 11) == 0)
                    {
{
return dev_storage + 1913;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdcu10", 11) == 0)
                    {
{
return dev_storage + 1898;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdct10", 11) == 0)
                    {
{
return dev_storage + 1883;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdcs10", 11) == 0)
                    {
{
return dev_storage + 1868;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdcr10", 11) == 0)
                    {
{
return dev_storage + 1853;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdcq10", 11) == 0)
                    {
{
return dev_storage + 1838;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdcp10", 11) == 0)
                    {
{
return dev_storage + 1823;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdco10", 11) == 0)
                    {
{
return dev_storage + 1808;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdcn10", 11) == 0)
                    {
{
return dev_storage + 1793;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdcm10", 11) == 0)
                    {
{
return dev_storage + 1778;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdcl10", 11) == 0)
                    {
{
return dev_storage + 1763;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdck10", 11) == 0)
                    {
{
return dev_storage + 1748;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdcj10", 11) == 0)
                    {
{
return dev_storage + 1733;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdci10", 11) == 0)
                    {
{
return dev_storage + 1718;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdch10", 11) == 0)
                    {
{
return dev_storage + 1703;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdcg10", 11) == 0)
                    {
{
return dev_storage + 1688;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdcf10", 11) == 0)
                    {
{
return dev_storage + 1673;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdce10", 11) == 0)
                    {
{
return dev_storage + 1658;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdcd10", 11) == 0)
                    {
{
return dev_storage + 1643;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdcc10", 11) == 0)
                    {
{
return dev_storage + 1628;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdcb10", 11) == 0)
                    {
{
return dev_storage + 1613;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdca10", 11) == 0)
                    {
{
return dev_storage + 1598;

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
            case 'b':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdbz10", 11) == 0)
                    {
{
return dev_storage + 1557;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sdby10", 11) == 0)
                    {
{
return dev_storage + 1542;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdbx10", 11) == 0)
                    {
{
return dev_storage + 1527;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdbw10", 11) == 0)
                    {
{
return dev_storage + 1512;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdbv10", 11) == 0)
                    {
{
return dev_storage + 1497;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdbu10", 11) == 0)
                    {
{
return dev_storage + 1482;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdbt10", 11) == 0)
                    {
{
return dev_storage + 1467;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdbs10", 11) == 0)
                    {
{
return dev_storage + 1452;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdbr10", 11) == 0)
                    {
{
return dev_storage + 1437;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdbq10", 11) == 0)
                    {
{
return dev_storage + 1422;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdbp10", 11) == 0)
                    {
{
return dev_storage + 1407;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdbo10", 11) == 0)
                    {
{
return dev_storage + 1392;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdbn10", 11) == 0)
                    {
{
return dev_storage + 1377;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdbm10", 11) == 0)
                    {
{
return dev_storage + 1362;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdbl10", 11) == 0)
                    {
{
return dev_storage + 1347;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdbk10", 11) == 0)
                    {
{
return dev_storage + 1332;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdbj10", 11) == 0)
                    {
{
return dev_storage + 1317;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdbi10", 11) == 0)
                    {
{
return dev_storage + 1302;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdbh10", 11) == 0)
                    {
{
return dev_storage + 1287;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdbg10", 11) == 0)
                    {
{
return dev_storage + 1272;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdbf10", 11) == 0)
                    {
{
return dev_storage + 1257;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdbe10", 11) == 0)
                    {
{
return dev_storage + 1242;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdbd10", 11) == 0)
                    {
{
return dev_storage + 1227;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdbc10", 11) == 0)
                    {
{
return dev_storage + 1212;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdbb10", 11) == 0)
                    {
{
return dev_storage + 1197;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdba10", 11) == 0)
                    {
{
return dev_storage + 1182;

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
            case 'a':
              switch (KR_keyword [8])
                {
                case 'z':
                  if (strncmp (KR_keyword, "/dev/sdaz10", 11) == 0)
                    {
{
return dev_storage + 1141;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'y':
                  if (strncmp (KR_keyword, "/dev/sday10", 11) == 0)
                    {
{
return dev_storage + 1126;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'x':
                  if (strncmp (KR_keyword, "/dev/sdax10", 11) == 0)
                    {
{
return dev_storage + 1111;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'w':
                  if (strncmp (KR_keyword, "/dev/sdaw10", 11) == 0)
                    {
{
return dev_storage + 1096;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'v':
                  if (strncmp (KR_keyword, "/dev/sdav10", 11) == 0)
                    {
{
return dev_storage + 1081;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'u':
                  if (strncmp (KR_keyword, "/dev/sdau10", 11) == 0)
                    {
{
return dev_storage + 1066;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 't':
                  if (strncmp (KR_keyword, "/dev/sdat10", 11) == 0)
                    {
{
return dev_storage + 1051;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 's':
                  if (strncmp (KR_keyword, "/dev/sdas10", 11) == 0)
                    {
{
return dev_storage + 1036;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sdar10", 11) == 0)
                    {
{
return dev_storage + 1021;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'q':
                  if (strncmp (KR_keyword, "/dev/sdaq10", 11) == 0)
                    {
{
return dev_storage + 1006;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'p':
                  if (strncmp (KR_keyword, "/dev/sdap10", 11) == 0)
                    {
{
return dev_storage + 991;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'o':
                  if (strncmp (KR_keyword, "/dev/sdao10", 11) == 0)
                    {
{
return dev_storage + 976;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'n':
                  if (strncmp (KR_keyword, "/dev/sdan10", 11) == 0)
                    {
{
return dev_storage + 961;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'm':
                  if (strncmp (KR_keyword, "/dev/sdam10", 11) == 0)
                    {
{
return dev_storage + 946;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'l':
                  if (strncmp (KR_keyword, "/dev/sdal10", 11) == 0)
                    {
{
return dev_storage + 931;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'k':
                  if (strncmp (KR_keyword, "/dev/sdak10", 11) == 0)
                    {
{
return dev_storage + 916;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'j':
                  if (strncmp (KR_keyword, "/dev/sdaj10", 11) == 0)
                    {
{
return dev_storage + 901;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'i':
                  if (strncmp (KR_keyword, "/dev/sdai10", 11) == 0)
                    {
{
return dev_storage + 886;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'h':
                  if (strncmp (KR_keyword, "/dev/sdah10", 11) == 0)
                    {
{
return dev_storage + 871;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'g':
                  if (strncmp (KR_keyword, "/dev/sdag10", 11) == 0)
                    {
{
return dev_storage + 856;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'f':
                  if (strncmp (KR_keyword, "/dev/sdaf10", 11) == 0)
                    {
{
return dev_storage + 841;

}
                    }
                  else
                    {
{
return	NULL;

}
                    }
                case 'e':
                  if (strncmp (KR_keyword, "/dev/sdae10", 11) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/sdad10", 11) == 0)
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
                case 'c':
                  if (strncmp (KR_keyword, "/dev/sdac10", 11) == 0)
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
                case 'b':
                  if (strncmp (KR_keyword, "/dev/sdab10", 11) == 0)
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
                case 'a':
                  if (strncmp (KR_keyword, "/dev/sdaa10", 11) == 0)
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
    case 12:
      switch (KR_keyword [5])
        {
        case 'w':
          if (strncmp (KR_keyword, "/dev/windows", 12) == 0)
            {
{
return dev_storage + 2573;

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
return dev_storage + 2572;

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
return dev_storage + 84;

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
return dev_storage + 1;

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






#undef BRACK

const device *dev_storage_end = dev_storage + (sizeof dev_storage / sizeof dev_storage[0]);

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
  dev_t devn = FHDEV (major, minor);

  d.devn = 0;

  for (const device *devidx = dev_storage; devidx < dev_storage_end; devidx++)
    if (devidx->d.devn == devn)
      {
	*this = *devidx;
	break;
      }

  if (!*this)
    d.devn = FHDEV (major, minor);
}

void
device::parse (dev_t dev)
{
  parse (_major (dev), _minor (dev));
}

void
device::parsedisk (int drive, int part)
{
  int base;
  if (drive < ('q' - 'a'))      /* /dev/sda -to- /dev/sdp */
    base = DEV_SD_MAJOR;
  else if (drive < 32)		/* /dev/sdq -to- /dev/sdaf */
    {
      base = DEV_SD1_MAJOR;
      drive -= 'q' - 'a';
    }
  else if (drive < 48)		/* /dev/sdag -to- /dev/sdav */
    {
      base = DEV_SD2_MAJOR;
      drive -= 32;
    }
  else if (drive < 64)		/* /dev/sdaw -to- /dev/sdbl */
    {
      base = DEV_SD3_MAJOR;
      drive -= 48;
    }
  else if (drive < 80)		/* /dev/sdbm -to- /dev/sdcb */
    {
      base = DEV_SD4_MAJOR;
      drive -= 64;
    }
  else if (drive < 96)		/* /dev/sdcc -to- /dev/sdcr */
    {
      base = DEV_SD5_MAJOR;
      drive -= 80;
    }
  else if (drive < 112)		/* /dev/sdcs -to- /dev/sddh */
    {
      base = DEV_SD6_MAJOR;
      drive -= 96;
    }
  /* NOTE: This will cause multiple /dev/sddx entries in
	   /proc/partitions if there are more than 128 devices */
  else				/* /dev/sddi -to- /dev/sddx */
    {
      base = DEV_SD7_MAJOR;
      drive -= 112;
    }
  parse (base, part + (drive * 16));
}


