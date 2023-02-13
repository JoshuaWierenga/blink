#ifndef BLINK_WIN_COSMO_LIBC_CALLS_SYSCALL_SUPPORT_NT_INTERNAL_H_
#define BLINK_WIN_COSMO_LIBC_CALLS_SYSCALL_SUPPORT_NT_INTERNAL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/syscall_support-nt.internal.h

int __mkntpath2(const char *, char16_t[PATH_MAX], int);
int __mkntpathat(int, const char *, int, char16_t[PATH_MAX]);
int ntaccesscheck(const char16_t *, DWORD);
int64_t __fix_enotdir(int64_t, wchar_t *);
int64_t __winerr(void);

#endif /* BLINK_WIN_COSMO_LIBC_CALLS_SYSCALL_SUPPORT_NT_INTERNAL_H_ */
