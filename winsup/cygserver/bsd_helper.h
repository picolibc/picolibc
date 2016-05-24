/* bsd_helper.h: Helps integrating BSD kernel code

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
#ifndef _BSD_HELPER_H
#define _BSD_HELPER_H

#include <sys/types.h>
#include <sys/syslog.h>

enum tun_bool_t {
  TUN_UNDEF = 0,
  TUN_FALSE = 1,
  TUN_TRUE  = 2
};

#define TUNABLE_INT_FETCH(a,b)	tunable_int_fetch((a),(b))
#define TUNABLE_BOOL_FETCH(a,b)	tunable_bool_fetch((a),(b))

#define sys_malloc(a,b,c) (malloc(a)?:(panic("malloc failed in %s, line %d"),(void*)NULL))
#define sys_free(a,b) free(a)

#define jail_sysvipc_allowed true
#define jailed(a) false

extern const char *__progname;

/* Global vars, determining whether the IPC stuff should be started or not. */
extern tun_bool_t support_sharedmem;
extern tun_bool_t support_msgqueues;
extern tun_bool_t support_semaphores;

extern SECURITY_ATTRIBUTES sec_all_nih;

void securityinit (void);

int win_copyin (class thread *, const void *, void *, size_t);
int win_copyout (class thread *, const void *, void *, size_t);
#define copyin(a,b,c) win_copyin((td),(a),(b),(c))
#define copyout(a,b,c) win_copyout((td),(a),(b),(c))

void *get_token_info (HANDLE, TOKEN_INFORMATION_CLASS);
int ipcperm (class thread *, struct ipc_perm *, unsigned int);
int suser (class thread *);
bool adjust_identity_info (struct proc *p);

struct vmspace *ipc_p_vmspace (struct proc *);
int ipcexit_creat_hookthread(class thread *);
void ipcinit (void);
int ipcunload (void);

vm_object_t _vm_pager_allocate (int, int);
#define vm_pager_allocate(a,b,s,c,d) _vm_pager_allocate((s),(mode))
vm_object_t vm_object_duplicate (class thread *td, vm_object_t object);
void vm_object_deallocate (vm_object_t object);

void tunable_param_init (const char *, bool);
void tunable_int_fetch (const char *, int32_t *);
void tunable_bool_fetch (const char *, tun_bool_t *);

#endif /* _BSD_HELPER_H */
