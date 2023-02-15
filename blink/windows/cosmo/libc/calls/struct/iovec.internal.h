#ifndef BLINK_WIN_COSMO_LIBC_CALLS_STRUCT_IOVEC_INTERNAL_H_
#define BLINK_WIN_COSMO_LIBC_CALLS_STRUCT_IOVEC_INTERNAL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/struct/iovec.internal.h

ssize_t sys_read_nt(HANDLE, const struct iovec *, size_t, ssize_t);
ssize_t sys_readv_nt(int, const struct iovec *, int);
ssize_t sys_write_nt(HANDLE, const struct iovec *, size_t, ssize_t);
ssize_t sys_writev_nt(int, const struct iovec *, int);

const char *DescribeIovec(char[300], ssize_t, const struct iovec *, int);
#define DescribeIovec(x, y, z) DescribeIovec(alloca(300), x, y, z)

#endif /* BLINK_WIN_COSMO_LIBC_CALLS_STRUCT_IOVEC_INTERNAL_H_ */
