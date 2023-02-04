#ifndef BLINK_WIN_WRITE_H_
#define BLINK_WIN_WRITE_H_

#ifdef _WIN32
#include <stddef.h>
#include "third_party/gnulib_build/lib/sys/uio.h"

ssize_t writev(int fd, const struct iovec *iov, int iovlen);
#endif

#endif /* BLINK_WIN_WRITE_H_ */
