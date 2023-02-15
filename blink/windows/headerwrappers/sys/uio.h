#ifndef BLINK_WIN_UIO_H_
#define BLINK_WIN_UIO_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/struct/iovec.h

#ifdef _WIN32
struct iovec {
  void *iov_base;
  size_t iov_len;
};

ssize_t readv(int, const struct iovec *, int);
ssize_t writev(int, const struct iovec *, int);
#else
// Header doesn't exist on windows.
#include_next <sys/stat.h>
#endif

#endif /* BLINK_WIN_UIO_H_ */
