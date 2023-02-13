#ifndef BLINK_WIN_COSMO_LIBC_CALLS_SYSCALL_NT_INTERNAL_H_
#define BLINK_WIN_COSMO_LIBC_CALLS_SYSCALL_NT_INTERNAL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/syscall-nt.internal.h

int sys_faccessat_nt(int, const char *, int, uint32_t);

#endif /* BLINK_WIN_COSMO_LIBC_CALLS_SYSCALL_NT_INTERNAL_H_ */
