#ifndef BLINK_WIN_COSMO_LIBC_INTRIN_DESCRIBEFLAGS_INTERNAL_H_
#define BLINK_WIN_COSMO_LIBC_INTRIN_DESCRIBEFLAGS_INTERNAL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/intrin/describeflags.internal.h

const char *DescribeDirfd(char[12], int);

#define DescribeDirfd(x)             DescribeDirfd(alloca(12), x)

#endif /* BLINK_WIN_COSMO_LIBC_INTRIN_DESCRIBEFLAGS_INTERNAL_H_ */
