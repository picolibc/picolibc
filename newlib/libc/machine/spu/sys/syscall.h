#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H
#ifdef __cplusplus
extern "C" {
#endif
int __send_to_ppe(unsigned int signalcode, unsigned int opcode, void *data);
#ifdef __cplusplus
}
#endif
#endif
