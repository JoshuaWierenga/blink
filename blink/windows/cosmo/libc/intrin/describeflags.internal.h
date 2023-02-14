#ifndef BLINK_WIN_COSMO_LIBC_INTRIN_DESCRIBEFLAGS_INTERNAL_H_
#define BLINK_WIN_COSMO_LIBC_INTRIN_DESCRIBEFLAGS_INTERNAL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/intrin/describeflags.internal.h
struct __attribute__((__packed__)) DescribeFlags {
  unsigned flag;
  const char *name;
};

const char *DescribeFlags(char *, size_t, struct DescribeFlags *, size_t,
                          const char *, unsigned);

const char *DescribeDirfd(char[12], int);
const char *DescribeOpenFlags(char[128], int);

#define DescribeDirfd(x)             DescribeDirfd(alloca(12), x)
#define DescribeOpenFlags(x)         DescribeOpenFlags(alloca(128), x)

#endif /* BLINK_WIN_COSMO_LIBC_INTRIN_DESCRIBEFLAGS_INTERNAL_H_ */
