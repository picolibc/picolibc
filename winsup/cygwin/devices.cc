

#include "winsup.h"
#include "devices.h"
#include "sys/cygwin.h"
#include "tty.h"
#include "pinfo.h"
#include "shared_info.h"
#include "path.h"
#include "fhandler.h"
#include "ntdll.h"

typedef const _device *KR_device_t;


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

  sys_mbstowcs (wpath, MAX_PATH, dev.native ());
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

const _device dev_cygdrive_storage =
  {"/cygdrive", {FH_CYGDRIVE}, "", exists};

const _device dev_fs_storage =
  {"", {FH_FS}, "", exists};

const _device dev_proc_storage =
  {"", {FH_PROC}, "", exists};

const _device dev_procnet_storage =
  {"", {FH_PROCNET}, "", exists};

const _device dev_procsys_storage =
  {"", {FH_PROCSYS}, "", exists};

const _device dev_procsysvipc_storage =
  {"", {FH_PROCSYSVIPC}, "", exists};

const _device dev_netdrive_storage =
  {"", {FH_NETDRIVE}, "", exists};

const _device dev_registry_storage =
  {"", {FH_REGISTRY}, "", exists_internal};

const _device dev_piper_storage =
  {"", {FH_PIPER}, "", exists_internal};

const _device dev_pipew_storage =
  {"", {FH_PIPEW}, "", exists_internal};

const _device dev_socket_storage =
  {"", {FH_SOCKET}, "", exists_internal};

const _device dev_af_inet_storage =
  {"", {FH_INET}, "", exists_internal};

const _device dev_af_unix_storage =
  {"", {FH_UNIX}, "", exists_internal};

const _device dev_af_local_storage =
  {"", {FH_LOCAL}, "", exists_internal};

const _device dev_bad_storage =
  {"", {FH_NADA}, "", exists_internal};

const _device dev_error_storage =
  {"", {FH_ERROR}, "", exists_internal};

#define BRACK(x) {devn: x}
const _RDATA _device dev_storage[] =
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
  {"/dev/cons64", BRACK(FHDEV(DEV_CONS_MAJOR, 64)), "/dev/cons64", exists_console, S_IFCHR, true},
  {"/dev/cons65", BRACK(FHDEV(DEV_CONS_MAJOR, 65)), "/dev/cons65", exists_console, S_IFCHR, true},
  {"/dev/cons66", BRACK(FHDEV(DEV_CONS_MAJOR, 66)), "/dev/cons66", exists_console, S_IFCHR, true},
  {"/dev/cons67", BRACK(FHDEV(DEV_CONS_MAJOR, 67)), "/dev/cons67", exists_console, S_IFCHR, true},
  {"/dev/cons68", BRACK(FHDEV(DEV_CONS_MAJOR, 68)), "/dev/cons68", exists_console, S_IFCHR, true},
  {"/dev/cons69", BRACK(FHDEV(DEV_CONS_MAJOR, 69)), "/dev/cons69", exists_console, S_IFCHR, true},
  {"/dev/cons70", BRACK(FHDEV(DEV_CONS_MAJOR, 70)), "/dev/cons70", exists_console, S_IFCHR, true},
  {"/dev/cons71", BRACK(FHDEV(DEV_CONS_MAJOR, 71)), "/dev/cons71", exists_console, S_IFCHR, true},
  {"/dev/cons72", BRACK(FHDEV(DEV_CONS_MAJOR, 72)), "/dev/cons72", exists_console, S_IFCHR, true},
  {"/dev/cons73", BRACK(FHDEV(DEV_CONS_MAJOR, 73)), "/dev/cons73", exists_console, S_IFCHR, true},
  {"/dev/cons74", BRACK(FHDEV(DEV_CONS_MAJOR, 74)), "/dev/cons74", exists_console, S_IFCHR, true},
  {"/dev/cons75", BRACK(FHDEV(DEV_CONS_MAJOR, 75)), "/dev/cons75", exists_console, S_IFCHR, true},
  {"/dev/cons76", BRACK(FHDEV(DEV_CONS_MAJOR, 76)), "/dev/cons76", exists_console, S_IFCHR, true},
  {"/dev/cons77", BRACK(FHDEV(DEV_CONS_MAJOR, 77)), "/dev/cons77", exists_console, S_IFCHR, true},
  {"/dev/cons78", BRACK(FHDEV(DEV_CONS_MAJOR, 78)), "/dev/cons78", exists_console, S_IFCHR, true},
  {"/dev/cons79", BRACK(FHDEV(DEV_CONS_MAJOR, 79)), "/dev/cons79", exists_console, S_IFCHR, true},
  {"/dev/cons80", BRACK(FHDEV(DEV_CONS_MAJOR, 80)), "/dev/cons80", exists_console, S_IFCHR, true},
  {"/dev/cons81", BRACK(FHDEV(DEV_CONS_MAJOR, 81)), "/dev/cons81", exists_console, S_IFCHR, true},
  {"/dev/cons82", BRACK(FHDEV(DEV_CONS_MAJOR, 82)), "/dev/cons82", exists_console, S_IFCHR, true},
  {"/dev/cons83", BRACK(FHDEV(DEV_CONS_MAJOR, 83)), "/dev/cons83", exists_console, S_IFCHR, true},
  {"/dev/cons84", BRACK(FHDEV(DEV_CONS_MAJOR, 84)), "/dev/cons84", exists_console, S_IFCHR, true},
  {"/dev/cons85", BRACK(FHDEV(DEV_CONS_MAJOR, 85)), "/dev/cons85", exists_console, S_IFCHR, true},
  {"/dev/cons86", BRACK(FHDEV(DEV_CONS_MAJOR, 86)), "/dev/cons86", exists_console, S_IFCHR, true},
  {"/dev/cons87", BRACK(FHDEV(DEV_CONS_MAJOR, 87)), "/dev/cons87", exists_console, S_IFCHR, true},
  {"/dev/cons88", BRACK(FHDEV(DEV_CONS_MAJOR, 88)), "/dev/cons88", exists_console, S_IFCHR, true},
  {"/dev/cons89", BRACK(FHDEV(DEV_CONS_MAJOR, 89)), "/dev/cons89", exists_console, S_IFCHR, true},
  {"/dev/cons90", BRACK(FHDEV(DEV_CONS_MAJOR, 90)), "/dev/cons90", exists_console, S_IFCHR, true},
  {"/dev/cons91", BRACK(FHDEV(DEV_CONS_MAJOR, 91)), "/dev/cons91", exists_console, S_IFCHR, true},
  {"/dev/cons92", BRACK(FHDEV(DEV_CONS_MAJOR, 92)), "/dev/cons92", exists_console, S_IFCHR, true},
  {"/dev/cons93", BRACK(FHDEV(DEV_CONS_MAJOR, 93)), "/dev/cons93", exists_console, S_IFCHR, true},
  {"/dev/cons94", BRACK(FHDEV(DEV_CONS_MAJOR, 94)), "/dev/cons94", exists_console, S_IFCHR, true},
  {"/dev/cons95", BRACK(FHDEV(DEV_CONS_MAJOR, 95)), "/dev/cons95", exists_console, S_IFCHR, true},
  {"/dev/cons96", BRACK(FHDEV(DEV_CONS_MAJOR, 96)), "/dev/cons96", exists_console, S_IFCHR, true},
  {"/dev/cons97", BRACK(FHDEV(DEV_CONS_MAJOR, 97)), "/dev/cons97", exists_console, S_IFCHR, true},
  {"/dev/cons98", BRACK(FHDEV(DEV_CONS_MAJOR, 98)), "/dev/cons98", exists_console, S_IFCHR, true},
  {"/dev/cons99", BRACK(FHDEV(DEV_CONS_MAJOR, 99)), "/dev/cons99", exists_console, S_IFCHR, true},
  {"/dev/cons100", BRACK(FHDEV(DEV_CONS_MAJOR, 100)), "/dev/cons100", exists_console, S_IFCHR, true},
  {"/dev/cons101", BRACK(FHDEV(DEV_CONS_MAJOR, 101)), "/dev/cons101", exists_console, S_IFCHR, true},
  {"/dev/cons102", BRACK(FHDEV(DEV_CONS_MAJOR, 102)), "/dev/cons102", exists_console, S_IFCHR, true},
  {"/dev/cons103", BRACK(FHDEV(DEV_CONS_MAJOR, 103)), "/dev/cons103", exists_console, S_IFCHR, true},
  {"/dev/cons104", BRACK(FHDEV(DEV_CONS_MAJOR, 104)), "/dev/cons104", exists_console, S_IFCHR, true},
  {"/dev/cons105", BRACK(FHDEV(DEV_CONS_MAJOR, 105)), "/dev/cons105", exists_console, S_IFCHR, true},
  {"/dev/cons106", BRACK(FHDEV(DEV_CONS_MAJOR, 106)), "/dev/cons106", exists_console, S_IFCHR, true},
  {"/dev/cons107", BRACK(FHDEV(DEV_CONS_MAJOR, 107)), "/dev/cons107", exists_console, S_IFCHR, true},
  {"/dev/cons108", BRACK(FHDEV(DEV_CONS_MAJOR, 108)), "/dev/cons108", exists_console, S_IFCHR, true},
  {"/dev/cons109", BRACK(FHDEV(DEV_CONS_MAJOR, 109)), "/dev/cons109", exists_console, S_IFCHR, true},
  {"/dev/cons110", BRACK(FHDEV(DEV_CONS_MAJOR, 110)), "/dev/cons110", exists_console, S_IFCHR, true},
  {"/dev/cons111", BRACK(FHDEV(DEV_CONS_MAJOR, 111)), "/dev/cons111", exists_console, S_IFCHR, true},
  {"/dev/cons112", BRACK(FHDEV(DEV_CONS_MAJOR, 112)), "/dev/cons112", exists_console, S_IFCHR, true},
  {"/dev/cons113", BRACK(FHDEV(DEV_CONS_MAJOR, 113)), "/dev/cons113", exists_console, S_IFCHR, true},
  {"/dev/cons114", BRACK(FHDEV(DEV_CONS_MAJOR, 114)), "/dev/cons114", exists_console, S_IFCHR, true},
  {"/dev/cons115", BRACK(FHDEV(DEV_CONS_MAJOR, 115)), "/dev/cons115", exists_console, S_IFCHR, true},
  {"/dev/cons116", BRACK(FHDEV(DEV_CONS_MAJOR, 116)), "/dev/cons116", exists_console, S_IFCHR, true},
  {"/dev/cons117", BRACK(FHDEV(DEV_CONS_MAJOR, 117)), "/dev/cons117", exists_console, S_IFCHR, true},
  {"/dev/cons118", BRACK(FHDEV(DEV_CONS_MAJOR, 118)), "/dev/cons118", exists_console, S_IFCHR, true},
  {"/dev/cons119", BRACK(FHDEV(DEV_CONS_MAJOR, 119)), "/dev/cons119", exists_console, S_IFCHR, true},
  {"/dev/cons120", BRACK(FHDEV(DEV_CONS_MAJOR, 120)), "/dev/cons120", exists_console, S_IFCHR, true},
  {"/dev/cons121", BRACK(FHDEV(DEV_CONS_MAJOR, 121)), "/dev/cons121", exists_console, S_IFCHR, true},
  {"/dev/cons122", BRACK(FHDEV(DEV_CONS_MAJOR, 122)), "/dev/cons122", exists_console, S_IFCHR, true},
  {"/dev/cons123", BRACK(FHDEV(DEV_CONS_MAJOR, 123)), "/dev/cons123", exists_console, S_IFCHR, true},
  {"/dev/cons124", BRACK(FHDEV(DEV_CONS_MAJOR, 124)), "/dev/cons124", exists_console, S_IFCHR, true},
  {"/dev/cons125", BRACK(FHDEV(DEV_CONS_MAJOR, 125)), "/dev/cons125", exists_console, S_IFCHR, true},
  {"/dev/cons126", BRACK(FHDEV(DEV_CONS_MAJOR, 126)), "/dev/cons126", exists_console, S_IFCHR, true},
  {"/dev/cons127", BRACK(FHDEV(DEV_CONS_MAJOR, 127)), "/dev/cons127", exists_console, S_IFCHR, true},
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
  {"/dev/pty64", BRACK(FHDEV(DEV_PTYS_MAJOR, 64)), "/dev/pty64", exists_pty, S_IFCHR, true},
  {"/dev/pty65", BRACK(FHDEV(DEV_PTYS_MAJOR, 65)), "/dev/pty65", exists_pty, S_IFCHR, true},
  {"/dev/pty66", BRACK(FHDEV(DEV_PTYS_MAJOR, 66)), "/dev/pty66", exists_pty, S_IFCHR, true},
  {"/dev/pty67", BRACK(FHDEV(DEV_PTYS_MAJOR, 67)), "/dev/pty67", exists_pty, S_IFCHR, true},
  {"/dev/pty68", BRACK(FHDEV(DEV_PTYS_MAJOR, 68)), "/dev/pty68", exists_pty, S_IFCHR, true},
  {"/dev/pty69", BRACK(FHDEV(DEV_PTYS_MAJOR, 69)), "/dev/pty69", exists_pty, S_IFCHR, true},
  {"/dev/pty70", BRACK(FHDEV(DEV_PTYS_MAJOR, 70)), "/dev/pty70", exists_pty, S_IFCHR, true},
  {"/dev/pty71", BRACK(FHDEV(DEV_PTYS_MAJOR, 71)), "/dev/pty71", exists_pty, S_IFCHR, true},
  {"/dev/pty72", BRACK(FHDEV(DEV_PTYS_MAJOR, 72)), "/dev/pty72", exists_pty, S_IFCHR, true},
  {"/dev/pty73", BRACK(FHDEV(DEV_PTYS_MAJOR, 73)), "/dev/pty73", exists_pty, S_IFCHR, true},
  {"/dev/pty74", BRACK(FHDEV(DEV_PTYS_MAJOR, 74)), "/dev/pty74", exists_pty, S_IFCHR, true},
  {"/dev/pty75", BRACK(FHDEV(DEV_PTYS_MAJOR, 75)), "/dev/pty75", exists_pty, S_IFCHR, true},
  {"/dev/pty76", BRACK(FHDEV(DEV_PTYS_MAJOR, 76)), "/dev/pty76", exists_pty, S_IFCHR, true},
  {"/dev/pty77", BRACK(FHDEV(DEV_PTYS_MAJOR, 77)), "/dev/pty77", exists_pty, S_IFCHR, true},
  {"/dev/pty78", BRACK(FHDEV(DEV_PTYS_MAJOR, 78)), "/dev/pty78", exists_pty, S_IFCHR, true},
  {"/dev/pty79", BRACK(FHDEV(DEV_PTYS_MAJOR, 79)), "/dev/pty79", exists_pty, S_IFCHR, true},
  {"/dev/pty80", BRACK(FHDEV(DEV_PTYS_MAJOR, 80)), "/dev/pty80", exists_pty, S_IFCHR, true},
  {"/dev/pty81", BRACK(FHDEV(DEV_PTYS_MAJOR, 81)), "/dev/pty81", exists_pty, S_IFCHR, true},
  {"/dev/pty82", BRACK(FHDEV(DEV_PTYS_MAJOR, 82)), "/dev/pty82", exists_pty, S_IFCHR, true},
  {"/dev/pty83", BRACK(FHDEV(DEV_PTYS_MAJOR, 83)), "/dev/pty83", exists_pty, S_IFCHR, true},
  {"/dev/pty84", BRACK(FHDEV(DEV_PTYS_MAJOR, 84)), "/dev/pty84", exists_pty, S_IFCHR, true},
  {"/dev/pty85", BRACK(FHDEV(DEV_PTYS_MAJOR, 85)), "/dev/pty85", exists_pty, S_IFCHR, true},
  {"/dev/pty86", BRACK(FHDEV(DEV_PTYS_MAJOR, 86)), "/dev/pty86", exists_pty, S_IFCHR, true},
  {"/dev/pty87", BRACK(FHDEV(DEV_PTYS_MAJOR, 87)), "/dev/pty87", exists_pty, S_IFCHR, true},
  {"/dev/pty88", BRACK(FHDEV(DEV_PTYS_MAJOR, 88)), "/dev/pty88", exists_pty, S_IFCHR, true},
  {"/dev/pty89", BRACK(FHDEV(DEV_PTYS_MAJOR, 89)), "/dev/pty89", exists_pty, S_IFCHR, true},
  {"/dev/pty90", BRACK(FHDEV(DEV_PTYS_MAJOR, 90)), "/dev/pty90", exists_pty, S_IFCHR, true},
  {"/dev/pty91", BRACK(FHDEV(DEV_PTYS_MAJOR, 91)), "/dev/pty91", exists_pty, S_IFCHR, true},
  {"/dev/pty92", BRACK(FHDEV(DEV_PTYS_MAJOR, 92)), "/dev/pty92", exists_pty, S_IFCHR, true},
  {"/dev/pty93", BRACK(FHDEV(DEV_PTYS_MAJOR, 93)), "/dev/pty93", exists_pty, S_IFCHR, true},
  {"/dev/pty94", BRACK(FHDEV(DEV_PTYS_MAJOR, 94)), "/dev/pty94", exists_pty, S_IFCHR, true},
  {"/dev/pty95", BRACK(FHDEV(DEV_PTYS_MAJOR, 95)), "/dev/pty95", exists_pty, S_IFCHR, true},
  {"/dev/pty96", BRACK(FHDEV(DEV_PTYS_MAJOR, 96)), "/dev/pty96", exists_pty, S_IFCHR, true},
  {"/dev/pty97", BRACK(FHDEV(DEV_PTYS_MAJOR, 97)), "/dev/pty97", exists_pty, S_IFCHR, true},
  {"/dev/pty98", BRACK(FHDEV(DEV_PTYS_MAJOR, 98)), "/dev/pty98", exists_pty, S_IFCHR, true},
  {"/dev/pty99", BRACK(FHDEV(DEV_PTYS_MAJOR, 99)), "/dev/pty99", exists_pty, S_IFCHR, true},
  {"/dev/pty100", BRACK(FHDEV(DEV_PTYS_MAJOR, 100)), "/dev/pty100", exists_pty, S_IFCHR, true},
  {"/dev/pty101", BRACK(FHDEV(DEV_PTYS_MAJOR, 101)), "/dev/pty101", exists_pty, S_IFCHR, true},
  {"/dev/pty102", BRACK(FHDEV(DEV_PTYS_MAJOR, 102)), "/dev/pty102", exists_pty, S_IFCHR, true},
  {"/dev/pty103", BRACK(FHDEV(DEV_PTYS_MAJOR, 103)), "/dev/pty103", exists_pty, S_IFCHR, true},
  {"/dev/pty104", BRACK(FHDEV(DEV_PTYS_MAJOR, 104)), "/dev/pty104", exists_pty, S_IFCHR, true},
  {"/dev/pty105", BRACK(FHDEV(DEV_PTYS_MAJOR, 105)), "/dev/pty105", exists_pty, S_IFCHR, true},
  {"/dev/pty106", BRACK(FHDEV(DEV_PTYS_MAJOR, 106)), "/dev/pty106", exists_pty, S_IFCHR, true},
  {"/dev/pty107", BRACK(FHDEV(DEV_PTYS_MAJOR, 107)), "/dev/pty107", exists_pty, S_IFCHR, true},
  {"/dev/pty108", BRACK(FHDEV(DEV_PTYS_MAJOR, 108)), "/dev/pty108", exists_pty, S_IFCHR, true},
  {"/dev/pty109", BRACK(FHDEV(DEV_PTYS_MAJOR, 109)), "/dev/pty109", exists_pty, S_IFCHR, true},
  {"/dev/pty110", BRACK(FHDEV(DEV_PTYS_MAJOR, 110)), "/dev/pty110", exists_pty, S_IFCHR, true},
  {"/dev/pty111", BRACK(FHDEV(DEV_PTYS_MAJOR, 111)), "/dev/pty111", exists_pty, S_IFCHR, true},
  {"/dev/pty112", BRACK(FHDEV(DEV_PTYS_MAJOR, 112)), "/dev/pty112", exists_pty, S_IFCHR, true},
  {"/dev/pty113", BRACK(FHDEV(DEV_PTYS_MAJOR, 113)), "/dev/pty113", exists_pty, S_IFCHR, true},
  {"/dev/pty114", BRACK(FHDEV(DEV_PTYS_MAJOR, 114)), "/dev/pty114", exists_pty, S_IFCHR, true},
  {"/dev/pty115", BRACK(FHDEV(DEV_PTYS_MAJOR, 115)), "/dev/pty115", exists_pty, S_IFCHR, true},
  {"/dev/pty116", BRACK(FHDEV(DEV_PTYS_MAJOR, 116)), "/dev/pty116", exists_pty, S_IFCHR, true},
  {"/dev/pty117", BRACK(FHDEV(DEV_PTYS_MAJOR, 117)), "/dev/pty117", exists_pty, S_IFCHR, true},
  {"/dev/pty118", BRACK(FHDEV(DEV_PTYS_MAJOR, 118)), "/dev/pty118", exists_pty, S_IFCHR, true},
  {"/dev/pty119", BRACK(FHDEV(DEV_PTYS_MAJOR, 119)), "/dev/pty119", exists_pty, S_IFCHR, true},
  {"/dev/pty120", BRACK(FHDEV(DEV_PTYS_MAJOR, 120)), "/dev/pty120", exists_pty, S_IFCHR, true},
  {"/dev/pty121", BRACK(FHDEV(DEV_PTYS_MAJOR, 121)), "/dev/pty121", exists_pty, S_IFCHR, true},
  {"/dev/pty122", BRACK(FHDEV(DEV_PTYS_MAJOR, 122)), "/dev/pty122", exists_pty, S_IFCHR, true},
  {"/dev/pty123", BRACK(FHDEV(DEV_PTYS_MAJOR, 123)), "/dev/pty123", exists_pty, S_IFCHR, true},
  {"/dev/pty124", BRACK(FHDEV(DEV_PTYS_MAJOR, 124)), "/dev/pty124", exists_pty, S_IFCHR, true},
  {"/dev/pty125", BRACK(FHDEV(DEV_PTYS_MAJOR, 125)), "/dev/pty125", exists_pty, S_IFCHR, true},
  {"/dev/pty126", BRACK(FHDEV(DEV_PTYS_MAJOR, 126)), "/dev/pty126", exists_pty, S_IFCHR, true},
  {"/dev/pty127", BRACK(FHDEV(DEV_PTYS_MAJOR, 127)), "/dev/pty127", exists_pty, S_IFCHR, true},
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
  {":ptym63", BRACK(FHDEV(DEV_PTYM_MAJOR, 63)), "/dev/ptym63", exists_internal, S_IFCHR, false},
  {":ptym64", BRACK(FHDEV(DEV_PTYM_MAJOR, 64)), "/dev/ptym64", exists_internal, S_IFCHR, false},
  {":ptym65", BRACK(FHDEV(DEV_PTYM_MAJOR, 65)), "/dev/ptym65", exists_internal, S_IFCHR, false},
  {":ptym66", BRACK(FHDEV(DEV_PTYM_MAJOR, 66)), "/dev/ptym66", exists_internal, S_IFCHR, false},
  {":ptym67", BRACK(FHDEV(DEV_PTYM_MAJOR, 67)), "/dev/ptym67", exists_internal, S_IFCHR, false},
  {":ptym68", BRACK(FHDEV(DEV_PTYM_MAJOR, 68)), "/dev/ptym68", exists_internal, S_IFCHR, false},
  {":ptym69", BRACK(FHDEV(DEV_PTYM_MAJOR, 69)), "/dev/ptym69", exists_internal, S_IFCHR, false},
  {":ptym70", BRACK(FHDEV(DEV_PTYM_MAJOR, 70)), "/dev/ptym70", exists_internal, S_IFCHR, false},
  {":ptym71", BRACK(FHDEV(DEV_PTYM_MAJOR, 71)), "/dev/ptym71", exists_internal, S_IFCHR, false},
  {":ptym72", BRACK(FHDEV(DEV_PTYM_MAJOR, 72)), "/dev/ptym72", exists_internal, S_IFCHR, false},
  {":ptym73", BRACK(FHDEV(DEV_PTYM_MAJOR, 73)), "/dev/ptym73", exists_internal, S_IFCHR, false},
  {":ptym74", BRACK(FHDEV(DEV_PTYM_MAJOR, 74)), "/dev/ptym74", exists_internal, S_IFCHR, false},
  {":ptym75", BRACK(FHDEV(DEV_PTYM_MAJOR, 75)), "/dev/ptym75", exists_internal, S_IFCHR, false},
  {":ptym76", BRACK(FHDEV(DEV_PTYM_MAJOR, 76)), "/dev/ptym76", exists_internal, S_IFCHR, false},
  {":ptym77", BRACK(FHDEV(DEV_PTYM_MAJOR, 77)), "/dev/ptym77", exists_internal, S_IFCHR, false},
  {":ptym78", BRACK(FHDEV(DEV_PTYM_MAJOR, 78)), "/dev/ptym78", exists_internal, S_IFCHR, false},
  {":ptym79", BRACK(FHDEV(DEV_PTYM_MAJOR, 79)), "/dev/ptym79", exists_internal, S_IFCHR, false},
  {":ptym80", BRACK(FHDEV(DEV_PTYM_MAJOR, 80)), "/dev/ptym80", exists_internal, S_IFCHR, false},
  {":ptym81", BRACK(FHDEV(DEV_PTYM_MAJOR, 81)), "/dev/ptym81", exists_internal, S_IFCHR, false},
  {":ptym82", BRACK(FHDEV(DEV_PTYM_MAJOR, 82)), "/dev/ptym82", exists_internal, S_IFCHR, false},
  {":ptym83", BRACK(FHDEV(DEV_PTYM_MAJOR, 83)), "/dev/ptym83", exists_internal, S_IFCHR, false},
  {":ptym84", BRACK(FHDEV(DEV_PTYM_MAJOR, 84)), "/dev/ptym84", exists_internal, S_IFCHR, false},
  {":ptym85", BRACK(FHDEV(DEV_PTYM_MAJOR, 85)), "/dev/ptym85", exists_internal, S_IFCHR, false},
  {":ptym86", BRACK(FHDEV(DEV_PTYM_MAJOR, 86)), "/dev/ptym86", exists_internal, S_IFCHR, false},
  {":ptym87", BRACK(FHDEV(DEV_PTYM_MAJOR, 87)), "/dev/ptym87", exists_internal, S_IFCHR, false},
  {":ptym88", BRACK(FHDEV(DEV_PTYM_MAJOR, 88)), "/dev/ptym88", exists_internal, S_IFCHR, false},
  {":ptym89", BRACK(FHDEV(DEV_PTYM_MAJOR, 89)), "/dev/ptym89", exists_internal, S_IFCHR, false},
  {":ptym90", BRACK(FHDEV(DEV_PTYM_MAJOR, 90)), "/dev/ptym90", exists_internal, S_IFCHR, false},
  {":ptym91", BRACK(FHDEV(DEV_PTYM_MAJOR, 91)), "/dev/ptym91", exists_internal, S_IFCHR, false},
  {":ptym92", BRACK(FHDEV(DEV_PTYM_MAJOR, 92)), "/dev/ptym92", exists_internal, S_IFCHR, false},
  {":ptym93", BRACK(FHDEV(DEV_PTYM_MAJOR, 93)), "/dev/ptym93", exists_internal, S_IFCHR, false},
  {":ptym94", BRACK(FHDEV(DEV_PTYM_MAJOR, 94)), "/dev/ptym94", exists_internal, S_IFCHR, false},
  {":ptym95", BRACK(FHDEV(DEV_PTYM_MAJOR, 95)), "/dev/ptym95", exists_internal, S_IFCHR, false},
  {":ptym96", BRACK(FHDEV(DEV_PTYM_MAJOR, 96)), "/dev/ptym96", exists_internal, S_IFCHR, false},
  {":ptym97", BRACK(FHDEV(DEV_PTYM_MAJOR, 97)), "/dev/ptym97", exists_internal, S_IFCHR, false},
  {":ptym98", BRACK(FHDEV(DEV_PTYM_MAJOR, 98)), "/dev/ptym98", exists_internal, S_IFCHR, false},
  {":ptym99", BRACK(FHDEV(DEV_PTYM_MAJOR, 99)), "/dev/ptym99", exists_internal, S_IFCHR, false},
  {":ptym100", BRACK(FHDEV(DEV_PTYM_MAJOR, 100)), "/dev/ptym100", exists_internal, S_IFCHR, false},
  {":ptym101", BRACK(FHDEV(DEV_PTYM_MAJOR, 101)), "/dev/ptym101", exists_internal, S_IFCHR, false},
  {":ptym102", BRACK(FHDEV(DEV_PTYM_MAJOR, 102)), "/dev/ptym102", exists_internal, S_IFCHR, false},
  {":ptym103", BRACK(FHDEV(DEV_PTYM_MAJOR, 103)), "/dev/ptym103", exists_internal, S_IFCHR, false},
  {":ptym104", BRACK(FHDEV(DEV_PTYM_MAJOR, 104)), "/dev/ptym104", exists_internal, S_IFCHR, false},
  {":ptym105", BRACK(FHDEV(DEV_PTYM_MAJOR, 105)), "/dev/ptym105", exists_internal, S_IFCHR, false},
  {":ptym106", BRACK(FHDEV(DEV_PTYM_MAJOR, 106)), "/dev/ptym106", exists_internal, S_IFCHR, false},
  {":ptym107", BRACK(FHDEV(DEV_PTYM_MAJOR, 107)), "/dev/ptym107", exists_internal, S_IFCHR, false},
  {":ptym108", BRACK(FHDEV(DEV_PTYM_MAJOR, 108)), "/dev/ptym108", exists_internal, S_IFCHR, false},
  {":ptym109", BRACK(FHDEV(DEV_PTYM_MAJOR, 109)), "/dev/ptym109", exists_internal, S_IFCHR, false},
  {":ptym110", BRACK(FHDEV(DEV_PTYM_MAJOR, 110)), "/dev/ptym110", exists_internal, S_IFCHR, false},
  {":ptym111", BRACK(FHDEV(DEV_PTYM_MAJOR, 111)), "/dev/ptym111", exists_internal, S_IFCHR, false},
  {":ptym112", BRACK(FHDEV(DEV_PTYM_MAJOR, 112)), "/dev/ptym112", exists_internal, S_IFCHR, false},
  {":ptym113", BRACK(FHDEV(DEV_PTYM_MAJOR, 113)), "/dev/ptym113", exists_internal, S_IFCHR, false},
  {":ptym114", BRACK(FHDEV(DEV_PTYM_MAJOR, 114)), "/dev/ptym114", exists_internal, S_IFCHR, false},
  {":ptym115", BRACK(FHDEV(DEV_PTYM_MAJOR, 115)), "/dev/ptym115", exists_internal, S_IFCHR, false},
  {":ptym116", BRACK(FHDEV(DEV_PTYM_MAJOR, 116)), "/dev/ptym116", exists_internal, S_IFCHR, false},
  {":ptym117", BRACK(FHDEV(DEV_PTYM_MAJOR, 117)), "/dev/ptym117", exists_internal, S_IFCHR, false},
  {":ptym118", BRACK(FHDEV(DEV_PTYM_MAJOR, 118)), "/dev/ptym118", exists_internal, S_IFCHR, false},
  {":ptym119", BRACK(FHDEV(DEV_PTYM_MAJOR, 119)), "/dev/ptym119", exists_internal, S_IFCHR, false},
  {":ptym120", BRACK(FHDEV(DEV_PTYM_MAJOR, 120)), "/dev/ptym120", exists_internal, S_IFCHR, false},
  {":ptym121", BRACK(FHDEV(DEV_PTYM_MAJOR, 121)), "/dev/ptym121", exists_internal, S_IFCHR, false},
  {":ptym122", BRACK(FHDEV(DEV_PTYM_MAJOR, 122)), "/dev/ptym122", exists_internal, S_IFCHR, false},
  {":ptym123", BRACK(FHDEV(DEV_PTYM_MAJOR, 123)), "/dev/ptym123", exists_internal, S_IFCHR, false},
  {":ptym124", BRACK(FHDEV(DEV_PTYM_MAJOR, 124)), "/dev/ptym124", exists_internal, S_IFCHR, false},
  {":ptym125", BRACK(FHDEV(DEV_PTYM_MAJOR, 125)), "/dev/ptym125", exists_internal, S_IFCHR, false},
  {":ptym126", BRACK(FHDEV(DEV_PTYM_MAJOR, 126)), "/dev/ptym126", exists_internal, S_IFCHR, false},
  {":ptym127", BRACK(FHDEV(DEV_PTYM_MAJOR, 127)), "/dev/ptym127", exists_internal, S_IFCHR, false}
};

const _device *cons_dev = dev_storage + 20;
const _device *console_dev = dev_storage + 148;
const _device *ptym_dev = dev_storage + 656;
const _device *ptys_dev = dev_storage + 297;
const _device *urandom_dev = dev_storage + 651;


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
return dev_storage + 655;

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
return dev_storage + 654;

}
            }
          else
            {
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
return dev_storage + 665;

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
return dev_storage + 664;

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
return dev_storage + 663;

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
return dev_storage + 662;

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
return dev_storage + 661;

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
return dev_storage + 660;

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
          if (strncmp (KR_keyword, ":ptym2", 6) == 0)
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
        case '1':
          if (strncmp (KR_keyword, ":ptym1", 6) == 0)
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
        case '0':
          if (strncmp (KR_keyword, ":ptym0", 6) == 0)
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
    case 7:
      switch (KR_keyword [5])
        {
        case '9':
          switch (KR_keyword [6])
            {
            case '9':
              if (strncmp (KR_keyword, ":ptym99", 7) == 0)
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
            case '8':
              if (strncmp (KR_keyword, ":ptym98", 7) == 0)
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
            case '7':
              if (strncmp (KR_keyword, ":ptym97", 7) == 0)
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
            case '6':
              if (strncmp (KR_keyword, ":ptym96", 7) == 0)
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
            case '5':
              if (strncmp (KR_keyword, ":ptym95", 7) == 0)
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
            case '4':
              if (strncmp (KR_keyword, ":ptym94", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym93", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym92", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym91", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym90", 7) == 0)
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
            default:
{
return	NULL;

}
            }
        case '8':
          switch (KR_keyword [6])
            {
            case '9':
              if (strncmp (KR_keyword, ":ptym89", 7) == 0)
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
            case '8':
              if (strncmp (KR_keyword, ":ptym88", 7) == 0)
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
            case '7':
              if (strncmp (KR_keyword, ":ptym87", 7) == 0)
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
            case '6':
              if (strncmp (KR_keyword, ":ptym86", 7) == 0)
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
            case '5':
              if (strncmp (KR_keyword, ":ptym85", 7) == 0)
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
            case '4':
              if (strncmp (KR_keyword, ":ptym84", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym83", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym82", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym81", 7) == 0)
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
              if (strncmp (KR_keyword, ":ptym80", 7) == 0)
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
            default:
{
return	NULL;

}
            }
        case '7':
          switch (KR_keyword [6])
            {
            case '9':
              if (strncmp (KR_keyword, ":ptym79", 7) == 0)
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
            case '8':
              if (strncmp (KR_keyword, ":ptym78", 7) == 0)
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
            case '7':
              if (strncmp (KR_keyword, ":ptym77", 7) == 0)
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
            case '6':
              if (strncmp (KR_keyword, ":ptym76", 7) == 0)
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
            case '5':
              if (strncmp (KR_keyword, ":ptym75", 7) == 0)
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
            case '4':
              if (strncmp (KR_keyword, ":ptym74", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym73", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym72", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym71", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym70", 7) == 0)
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
        case '6':
          switch (KR_keyword [6])
            {
            case '9':
              if (strncmp (KR_keyword, ":ptym69", 7) == 0)
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
            case '8':
              if (strncmp (KR_keyword, ":ptym68", 7) == 0)
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
            case '7':
              if (strncmp (KR_keyword, ":ptym67", 7) == 0)
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
            case '6':
              if (strncmp (KR_keyword, ":ptym66", 7) == 0)
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
            case '5':
              if (strncmp (KR_keyword, ":ptym65", 7) == 0)
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
            case '4':
              if (strncmp (KR_keyword, ":ptym64", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym63", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym62", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym61", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym60", 7) == 0)
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
        case '5':
          switch (KR_keyword [6])
            {
            case '9':
              if (strncmp (KR_keyword, ":ptym59", 7) == 0)
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
              if (strncmp (KR_keyword, ":ptym58", 7) == 0)
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
            case '7':
              if (strncmp (KR_keyword, ":ptym57", 7) == 0)
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
            case '6':
              if (strncmp (KR_keyword, ":ptym56", 7) == 0)
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
            case '5':
              if (strncmp (KR_keyword, ":ptym55", 7) == 0)
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
            case '4':
              if (strncmp (KR_keyword, ":ptym54", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym53", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym52", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym51", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym50", 7) == 0)
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
return dev_storage + 705;

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
              if (strncmp (KR_keyword, ":ptym47", 7) == 0)
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
            case '6':
              if (strncmp (KR_keyword, ":ptym46", 7) == 0)
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
            case '5':
              if (strncmp (KR_keyword, ":ptym45", 7) == 0)
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
            case '4':
              if (strncmp (KR_keyword, ":ptym44", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym43", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym42", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym41", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym40", 7) == 0)
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
return dev_storage + 695;

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
return dev_storage + 694;

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
              if (strncmp (KR_keyword, ":ptym36", 7) == 0)
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
            case '5':
              if (strncmp (KR_keyword, ":ptym35", 7) == 0)
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
            case '4':
              if (strncmp (KR_keyword, ":ptym34", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym33", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym32", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym31", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym30", 7) == 0)
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
        case '2':
          switch (KR_keyword [6])
            {
            case '9':
              if (strncmp (KR_keyword, ":ptym29", 7) == 0)
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
            case '8':
              if (strncmp (KR_keyword, ":ptym28", 7) == 0)
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
            case '7':
              if (strncmp (KR_keyword, ":ptym27", 7) == 0)
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
            case '6':
              if (strncmp (KR_keyword, ":ptym26", 7) == 0)
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
              if (strncmp (KR_keyword, ":ptym25", 7) == 0)
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
              if (strncmp (KR_keyword, ":ptym24", 7) == 0)
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
            case '3':
              if (strncmp (KR_keyword, ":ptym23", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym22", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym21", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym20", 7) == 0)
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
return dev_storage + 675;

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
return dev_storage + 674;

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
return dev_storage + 673;

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
return dev_storage + 672;

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
return dev_storage + 671;

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
              if (strncmp (KR_keyword, ":ptym13", 7) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym12", 7) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym11", 7) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym10", 7) == 0)
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
        case 'y':
          if (strncmp (KR_keyword, "/dev/tty", 8) == 0)
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
        case 'p':
          if (strncmp (KR_keyword, "/dev/dsp", 8) == 0)
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
        case '9':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st9", 8) == 0)
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
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr9", 8) == 0)
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd9", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym119", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym109", 8) == 0)
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
        case '8':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st8", 8) == 0)
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
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr8", 8) == 0)
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd8", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym118", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym108", 8) == 0)
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
        case '7':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st7", 8) == 0)
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
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr7", 8) == 0)
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd7", 8) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym127", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym117", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym107", 8) == 0)
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
        case '6':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st6", 8) == 0)
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
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr6", 8) == 0)
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd6", 8) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym126", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym116", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym106", 8) == 0)
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
        case '5':
          switch (KR_keyword [6])
            {
            case 't':
              if (strncmp (KR_keyword, "/dev/st5", 8) == 0)
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
            case 'r':
              if (strncmp (KR_keyword, "/dev/sr5", 8) == 0)
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/fd5", 8) == 0)
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
            case '2':
              if (strncmp (KR_keyword, ":ptym125", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym115", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym105", 8) == 0)
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
return dev_storage + 462;

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
return dev_storage + 446;

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
return dev_storage + 154;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym124", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym114", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym104", 8) == 0)
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
return dev_storage + 461;

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
return dev_storage + 445;

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
return dev_storage + 153;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym123", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym113", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym103", 8) == 0)
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
return dev_storage + 460;

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
return dev_storage + 444;

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
return dev_storage + 152;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym122", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym112", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym102", 8) == 0)
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
return dev_storage + 459;

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
return dev_storage + 443;

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
return dev_storage + 151;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym121", 8) == 0)
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
            case '1':
              if (strncmp (KR_keyword, ":ptym111", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym101", 8) == 0)
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
return dev_storage + 458;

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
return dev_storage + 442;

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
return dev_storage + 150;

}
                }
              else
                {
{
return	NULL;

}
                }
            case '2':
              if (strncmp (KR_keyword, ":ptym120", 8) == 0)
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
              if (strncmp (KR_keyword, ":ptym110", 8) == 0)
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
            case '0':
              if (strncmp (KR_keyword, ":ptym100", 8) == 0)
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
return dev_storage + 296;

}
            }
          else
            {
{
return	NULL;

}
            }
        case 'o':
          if (strncmp (KR_keyword, "/dev/zero", 9) == 0)
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
        case 'l':
          switch (KR_keyword [5])
            {
            case 'n':
              if (strncmp (KR_keyword, "/dev/null", 9) == 0)
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
            case 'f':
              if (strncmp (KR_keyword, "/dev/full", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '9':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty9", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst9", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd9", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st99", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st89", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st79", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st69", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st59", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st49", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st39", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st29", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/st19", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '8':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty8", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst8", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd8", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st98", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st88", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st78", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st68", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/st58", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st48", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st38", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st28", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/st18", 9) == 0)
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
        case '7':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty7", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst7", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd7", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st97", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st87", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st77", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st67", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st57", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st47", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st37", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st27", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/st17", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '6':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty6", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst6", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd6", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st96", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st86", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st76", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st66", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st56", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st46", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st36", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st26", 9) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/st16", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '5':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty5", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst5", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd5", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st95", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st85", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/st75", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st65", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st55", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st45", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st35", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/st25", 9) == 0)
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
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st15", 9) == 0)
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
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr15", 9) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd15", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/pty4", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst4", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd4", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st94", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st84", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st74", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st64", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st54", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st44", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st34", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st24", 9) == 0)
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
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st14", 9) == 0)
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
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr14", 9) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd14", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/pty3", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst3", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd3", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st93", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st83", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st73", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st63", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st53", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st43", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st33", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st23", 9) == 0)
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
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st13", 9) == 0)
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
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr13", 9) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd13", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '2':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty2", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst2", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd2", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st92", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st82", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st72", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st62", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st52", 9) == 0)
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
              if (strncmp (KR_keyword, "/dev/st42", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st32", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st22", 9) == 0)
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
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st12", 9) == 0)
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
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr12", 9) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd12", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '1':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty1", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst1", 9) == 0)
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
            case 'm':
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd1", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st91", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st81", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st71", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st61", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st51", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st41", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st31", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st21", 9) == 0)
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
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st11", 9) == 0)
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
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr11", 9) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd11", 9) == 0)
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
            default:
{
return	NULL;

}
            }
        case '0':
          switch (KR_keyword [7])
            {
            case 'y':
              if (strncmp (KR_keyword, "/dev/pty0", 9) == 0)
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
            case 't':
              if (strncmp (KR_keyword, "/dev/nst0", 9) == 0)
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
            case 'd':
              if (strncmp (KR_keyword, "/dev/scd0", 9) == 0)
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
            case '9':
              if (strncmp (KR_keyword, "/dev/st90", 9) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st80", 9) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st70", 9) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st60", 9) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st50", 9) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st40", 9) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st30", 9) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st20", 9) == 0)
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
            case '1':
              switch (KR_keyword [6])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/st10", 9) == 0)
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
                case 'r':
                  if (strncmp (KR_keyword, "/dev/sr10", 9) == 0)
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
                case 'd':
                  if (strncmp (KR_keyword, "/dev/fd10", 9) == 0)
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
      switch (KR_keyword [8])
        {
        case 's':
          switch (KR_keyword [9])
            {
            case '9':
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
            case '8':
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
            case '7':
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
            case '6':
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
            case '5':
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
            case '4':
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
            case '3':
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
            case '2':
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
            case '1':
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
            case '0':
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
        case 'i':
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
        case 'S':
          switch (KR_keyword [9])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/ttyS9", 10) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/ttyS8", 10) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/ttyS7", 10) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/ttyS6", 10) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/ttyS5", 10) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/ttyS4", 10) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/ttyS3", 10) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/ttyS2", 10) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/ttyS1", 10) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/ttyS0", 10) == 0)
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
        case '9':
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty99", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty98", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty97", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty96", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty95", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty94", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty93", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty92", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty91", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty90", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst99", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst98", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst97", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst96", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst95", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst94", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst93", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst92", 10) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst91", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst90", 10) == 0)
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
        case '8':
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty89", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty88", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty87", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty86", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty85", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty84", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty83", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty82", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty81", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty80", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst89", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst88", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst87", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst86", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst85", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst84", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst83", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst82", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst81", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst80", 10) == 0)
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
        case '7':
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty79", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty78", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty77", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty76", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty75", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty74", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty73", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty72", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty71", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty70", 10) == 0)
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
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst79", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst78", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst77", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst76", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst75", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst74", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst73", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst72", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst71", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst70", 10) == 0)
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
            default:
{
return	NULL;

}
            }
        case '6':
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty69", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty68", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty67", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty66", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty65", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty64", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty63", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty62", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty61", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty60", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst69", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst68", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst67", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst66", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst65", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst64", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst63", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst62", 10) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst61", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst60", 10) == 0)
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
        case '5':
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty59", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty58", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty57", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty56", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty55", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty54", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty53", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty52", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty51", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty50", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst59", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst58", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst57", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst56", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst55", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst54", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst53", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst52", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst51", 10) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst50", 10) == 0)
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
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty49", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty48", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty47", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty46", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty45", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty44", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty43", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty42", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty41", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty40", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst49", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst48", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst47", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst46", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst45", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst44", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst43", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst42", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst41", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst40", 10) == 0)
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
            default:
{
return	NULL;

}
            }
        case '3':
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty39", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty38", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty37", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty36", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty35", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty34", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty33", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty32", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty31", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty30", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst39", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst38", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst37", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst36", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst35", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst34", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst33", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst32", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst31", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst30", 10) == 0)
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
          switch (KR_keyword [5])
            {
            case 's':
              switch (KR_keyword [9])
                {
                case '7':
                  if (strncmp (KR_keyword, "/dev/st127", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/st126", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/st125", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/st124", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/st123", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/st122", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/st121", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/st120", 10) == 0)
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
            case 'p':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty29", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty28", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty27", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty26", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty25", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty24", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty23", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty22", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty21", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty20", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst29", 10) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst28", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst27", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst26", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst25", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst24", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst23", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst22", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst21", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst20", 10) == 0)
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
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty19", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty18", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty17", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty16", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty15", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty14", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty13", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty12", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty11", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty10", 10) == 0)
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
            case 't':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst19", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst18", 10) == 0)
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
                  if (strncmp (KR_keyword, "/dev/nst17", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst16", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst15", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst14", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst13", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst12", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst11", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst10", 10) == 0)
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
            case 'm':
              switch (KR_keyword [9])
                {
                case '6':
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
                case '5':
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
                case '4':
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
                case '3':
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
                case '2':
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
                case '1':
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
                case '0':
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
            case 'd':
              switch (KR_keyword [9])
                {
                case '5':
                  if (strncmp (KR_keyword, "/dev/scd15", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/scd14", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/scd13", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/scd12", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/scd11", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/scd10", 10) == 0)
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [9])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/st119", 10) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/st118", 10) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/st117", 10) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/st116", 10) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/st115", 10) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/st114", 10) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/st113", 10) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/st112", 10) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/st111", 10) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/st110", 10) == 0)
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
          switch (KR_keyword [9])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/st109", 10) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/st108", 10) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/st107", 10) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/st106", 10) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/st105", 10) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/st104", 10) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/st103", 10) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/st102", 10) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/st101", 10) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/st100", 10) == 0)
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
return dev_storage + 425;

}
            }
          else
            {
{
return	NULL;

}
            }
        case '9':
          switch (KR_keyword [10])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/cons99", 11) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/cons98", 11) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/cons97", 11) == 0)
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
              if (strncmp (KR_keyword, "/dev/cons96", 11) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/cons95", 11) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/cons94", 11) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/cons93", 11) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/cons92", 11) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/cons91", 11) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/cons90", 11) == 0)
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
        case '8':
          switch (KR_keyword [10])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/cons89", 11) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/cons88", 11) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/cons87", 11) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/cons86", 11) == 0)
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
              if (strncmp (KR_keyword, "/dev/cons85", 11) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/cons84", 11) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/cons83", 11) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/cons82", 11) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/cons81", 11) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/cons80", 11) == 0)
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
        case '7':
          switch (KR_keyword [10])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/cons79", 11) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/cons78", 11) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/cons77", 11) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/cons76", 11) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/cons75", 11) == 0)
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
              if (strncmp (KR_keyword, "/dev/cons74", 11) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/cons73", 11) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/cons72", 11) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/cons71", 11) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/cons70", 11) == 0)
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
        case '6':
          switch (KR_keyword [10])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/cons69", 11) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/cons68", 11) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/cons67", 11) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/cons66", 11) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/cons65", 11) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/cons64", 11) == 0)
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
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS63", 11) == 0)
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
                case 'c':
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
                default:
{
return	NULL;

}
                }
            case '2':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS62", 11) == 0)
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
                case 'c':
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
                default:
{
return	NULL;

}
                }
            case '1':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS61", 11) == 0)
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
                case 'c':
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
                default:
{
return	NULL;

}
                }
            case '0':
              switch (KR_keyword [5])
                {
                case 't':
                  if (strncmp (KR_keyword, "/dev/ttyS60", 11) == 0)
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
                case 'c':
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
        case '5':
          switch (KR_keyword [5])
            {
            case 't':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/ttyS59", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/ttyS58", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/ttyS57", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS56", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS55", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS54", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS53", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS52", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS51", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/ttyS50", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [10])
                {
                case '9':
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
                case '8':
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
                case '7':
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
                case '6':
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
                case '3':
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
                case '2':
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
                case '1':
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
                case '0':
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
          switch (KR_keyword [5])
            {
            case 't':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/ttyS49", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/ttyS48", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/ttyS47", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS46", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS45", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS44", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS43", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS42", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS41", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/ttyS40", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [10])
                {
                case '9':
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
                case '8':
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
                case '7':
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
                case '6':
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
                case '5':
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
                case '2':
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
                case '1':
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
                case '0':
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
          switch (KR_keyword [5])
            {
            case 't':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/ttyS39", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/ttyS38", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/ttyS37", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS36", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS35", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS34", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS33", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS32", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS31", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/ttyS30", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [10])
                {
                case '9':
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
                case '8':
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
                case '7':
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
                case '6':
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
                case '5':
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
                case '4':
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
                case '1':
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
                case '0':
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
          switch (KR_keyword [5])
            {
            case 't':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/ttyS29", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/ttyS28", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/ttyS27", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS26", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS25", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS24", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS23", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS22", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS21", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/ttyS20", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'p':
              switch (KR_keyword [10])
                {
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty127", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty126", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty125", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty124", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty123", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty122", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty121", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty120", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [10])
                {
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst127", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst126", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst125", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst124", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst123", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst122", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst121", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst120", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [10])
                {
                case '9':
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
                case '8':
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
                case '7':
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
                case '6':
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
                case '5':
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
                case '4':
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
                case '3':
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
                case '0':
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
          switch (KR_keyword [5])
            {
            case 't':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/ttyS19", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/ttyS18", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/ttyS17", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/ttyS16", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/ttyS15", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/ttyS14", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/ttyS13", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/ttyS12", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/ttyS11", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/ttyS10", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'p':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty119", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty118", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty117", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty116", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty115", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty114", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty113", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty112", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty111", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty110", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst119", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst118", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst117", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst116", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst115", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst114", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst113", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst112", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst111", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst110", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'c':
              switch (KR_keyword [10])
                {
                case '9':
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
                case '8':
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
                case '7':
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
                case '6':
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
                case '5':
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
                case '4':
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
                case '3':
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
                case '2':
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
                case '0':
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
            default:
{
return	NULL;

}
            }
        case '0':
          switch (KR_keyword [5])
            {
            case 'p':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/pty109", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/pty108", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/pty107", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/pty106", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/pty105", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/pty104", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/pty103", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/pty102", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/pty101", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/pty100", 11) == 0)
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
                default:
{
return	NULL;

}
                }
            case 'n':
              switch (KR_keyword [10])
                {
                case '9':
                  if (strncmp (KR_keyword, "/dev/nst109", 11) == 0)
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
                case '8':
                  if (strncmp (KR_keyword, "/dev/nst108", 11) == 0)
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
                case '7':
                  if (strncmp (KR_keyword, "/dev/nst107", 11) == 0)
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
                case '6':
                  if (strncmp (KR_keyword, "/dev/nst106", 11) == 0)
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
                case '5':
                  if (strncmp (KR_keyword, "/dev/nst105", 11) == 0)
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
                case '4':
                  if (strncmp (KR_keyword, "/dev/nst104", 11) == 0)
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
                case '3':
                  if (strncmp (KR_keyword, "/dev/nst103", 11) == 0)
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
                case '2':
                  if (strncmp (KR_keyword, "/dev/nst102", 11) == 0)
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
                case '1':
                  if (strncmp (KR_keyword, "/dev/nst101", 11) == 0)
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
                case '0':
                  if (strncmp (KR_keyword, "/dev/nst100", 11) == 0)
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
      switch (KR_keyword [10])
        {
        case 'w':
          if (strncmp (KR_keyword, "/dev/windows", 12) == 0)
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
        case 'o':
          if (strncmp (KR_keyword, "/dev/urandom", 12) == 0)
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
        case 'l':
          if (strncmp (KR_keyword, "/dev/console", 12) == 0)
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
        case '2':
          switch (KR_keyword [11])
            {
            case '7':
              if (strncmp (KR_keyword, "/dev/cons127", 12) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/cons126", 12) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/cons125", 12) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/cons124", 12) == 0)
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
              if (strncmp (KR_keyword, "/dev/cons123", 12) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/cons122", 12) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/cons121", 12) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/cons120", 12) == 0)
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
        case '1':
          switch (KR_keyword [11])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/cons119", 12) == 0)
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
              if (strncmp (KR_keyword, "/dev/cons118", 12) == 0)
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
            case '7':
              if (strncmp (KR_keyword, "/dev/cons117", 12) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/cons116", 12) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/cons115", 12) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/cons114", 12) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/cons113", 12) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/cons112", 12) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/cons111", 12) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/cons110", 12) == 0)
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
        case '0':
          switch (KR_keyword [11])
            {
            case '9':
              if (strncmp (KR_keyword, "/dev/cons109", 12) == 0)
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
            case '8':
              if (strncmp (KR_keyword, "/dev/cons108", 12) == 0)
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
              if (strncmp (KR_keyword, "/dev/cons107", 12) == 0)
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
            case '6':
              if (strncmp (KR_keyword, "/dev/cons106", 12) == 0)
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
            case '5':
              if (strncmp (KR_keyword, "/dev/cons105", 12) == 0)
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
            case '4':
              if (strncmp (KR_keyword, "/dev/cons104", 12) == 0)
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
            case '3':
              if (strncmp (KR_keyword, "/dev/cons103", 12) == 0)
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
            case '2':
              if (strncmp (KR_keyword, "/dev/cons102", 12) == 0)
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
            case '1':
              if (strncmp (KR_keyword, "/dev/cons101", 12) == 0)
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
            case '0':
              if (strncmp (KR_keyword, "/dev/cons100", 12) == 0)
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

const _device *dev_storage_end = dev_storage
				+ (sizeof dev_storage / sizeof dev_storage[0]);

/* Convert disk/partition to major/minor */
static void
conv_dp_to_mm (int drive, int part, _major_t &major, _minor_t &minor)
{
  if (part >= 16)
    {
      major = DEV_SD_HIGHPART_START + drive / 5;
      drive %= 5;
      minor = (part - 16) + 48 * drive;
      return;
    }
  if (drive < ('q' - 'a'))      /* /dev/sda -to- /dev/sdp */
    major = DEV_SD_MAJOR;
  else if (drive < 32)		/* /dev/sdq -to- /dev/sdaf */
    {
      major = DEV_SD1_MAJOR;
      drive -= 'q' - 'a';
    }
  else if (drive < 48)		/* /dev/sdag -to- /dev/sdav */
    {
      major = DEV_SD2_MAJOR;
      drive -= 32;
    }
  else if (drive < 64)		/* /dev/sdaw -to- /dev/sdbl */
    {
      major = DEV_SD3_MAJOR;
      drive -= 48;
    }
  else if (drive < 80)		/* /dev/sdbm -to- /dev/sdcb */
    {
      major = DEV_SD4_MAJOR;
      drive -= 64;
    }
  else if (drive < 96)		/* /dev/sdcc -to- /dev/sdcr */
    {
      major = DEV_SD5_MAJOR;
      drive -= 80;
    }
  else if (drive < 112)		/* /dev/sdcs -to- /dev/sddh */
    {
      major = DEV_SD6_MAJOR;
      drive -= 96;
    }
  /* NOTE: This will cause multiple /dev/sddx entries in
	   /proc/partitions if there are more than 128 devices */
  else				/* /dev/sddi -to- /dev/sddx */
    {
      major = DEV_SD7_MAJOR;
      drive -= 112;
    }
  minor = part + (drive * 16);
}

#define DISK_PREFIX	"/dev/sd"
#define DP_LEN		(sizeof (DISK_PREFIX) - 1)

static const char *hd_pattern = "\\Device\\Harddisk%u\\Partition%u";

void
device::parse (const char *s)
{
  size_t len = strlen (s);
  const _device *dev = KR_find_keyword (s, len);

  if (!dev)
    {
      /* /dev/sd* devices have 8192 entries, given that we support 128 disks
	 with up to 64 partitions.  Handling them with shilka raises the size
	 of devices.o from ~250K to ~2 Megs.  So we handle them here manually
	 to save this space. */
      int drive = 0, part = 0;
      const char *pos = s + DP_LEN;

      /* Generic check for /dev/sd[a-z] prefix */
      if (len <= DP_LEN || strncmp (s, DISK_PREFIX, DP_LEN)
	  || pos[0] < 'a' || pos[0] > 'z')
	goto no_disk;

      /* /dev/sdd[a-x]? */
      if (pos[0] == 'd' && pos[1] >= 'a' && pos[1] <= 'x')
	{
	  drive = 104 + (pos[1] - 'a');
	  ++pos;
	}
      /* /dev/sd[a-c][a-z]? */
      else if (pos[0] <= 'c' && pos[1] >= 'a' && pos[1] <= 'z')
	{
	  drive = 26 + (pos[0] - 'a') * 26 + (pos[1] - 'a');
	  ++pos;
	}
      else
	drive = (pos[0] - 'a');
      /* Check next position in string for partition number. */
      ++pos;
      /* No partition number, equivalent to Windows partition 0. */
      if (!pos[0])
	;
      /* First digit must not be 0. */
      else if (pos[0] < '1' || pos[0] > '9')
	goto no_disk;
      else if (!pos[1])
	part = (pos[0] - '0');
      else if (pos[1] < '0' || pos[1] > '9' || pos[2] != '\0')
	goto no_disk;
      else
	{
	  part = (pos[0] - '0') * 10 + (pos[1] - '0');
	  if (part > 63)
	    goto no_disk;
	}
      char buf[sizeof *hd_pattern + 32];
      __small_sprintf (buf, hd_pattern, drive, part);
      native (buf, false);
      if (exists_ntdev (*this))
	{
	  name (s, true);
	  conv_dp_to_mm (drive, part, d.major, d.minor);
	  native (buf, true);
	  exists_func = exists_ntdev;
	  _mode = S_IFBLK;
	  lives_in_dev = dev_on_fs = false;
	  return;
	}
no_disk:
      *this = *fs_dev;
    }
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

  for (const _device *devidx = dev_storage; devidx < dev_storage_end; devidx++)
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
  char buf[sizeof ("/dev/sddx63")], *bp;

  conv_dp_to_mm (drive, part, d.major, d.minor);
  bp = stpcpy (buf, "/dev/sd");
  if (drive >= 26)
    {
      drive -= 26;
      *bp++ = drive / 26 + 'a';
      drive %= 26;
    }
  *bp++ = drive + 'a';
  if (part)
    {
      if (part >= 10)
	{
	  *bp++ = part / 10 + '0';
	  part %= 10;
	}
      *bp++ = part + '0';
    }
  *bp = '\0';
  name (buf, true);
}


