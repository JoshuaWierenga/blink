#ifndef BLINK_WIN_COSMO_LIBC_CALLS_STRUCT_STAT_INTERNAL_H_
#define BLINK_WIN_COSMO_LIBC_CALLS_STRUCT_STAT_INTERNAL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/integral/normalize.inc

#define PAGESIZE    0x1000  /* i386+ */

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/struct/stat.h

int sys_fstat_nt(HANDLE, struct stat *);
int sys_fstatat_nt(int, const char *, struct stat *, int);

const char *DescribeStat(char[300], int, const struct stat *);
#define DescribeStat(rc, st) DescribeStat(alloca(300), rc, st)

#endif /* BLINK_WIN_COSMO_LIBC_CALLS_STRUCT_STAT_INTERNAL_H_ */
