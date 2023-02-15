/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include <minwindef.h>
#include <stddef.h>
#include <sys/uio.h>

#include "blink/errno.h"
#include "blink/windows/macros.h"
#include "blink/windows/cosmo/libc/calls/struct/iovec.internal.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/writev.c

/**
 * Writes data from multiple buffers.
 *
 * This is the same thing as write() except it has multiple buffers.
 * This yields a performance boost in situations where it'd be expensive
 * to stitch data together using memcpy() or issuing multiple syscalls.
 * This wrapper is implemented so that writev() calls where iovlen<2 may
 * be passed to the kernel as write() instead. This yields a 100 cycle
 * performance boost in the case of a single small iovec.
 *
 * Please note that it's not an error for a short write to happen. This
 * can happen in the kernel if EINTR happens after some of the write has
 * been committed. It can also happen if we need to polyfill this system
 * call using write().
 *
 * @return number of bytes actually handed off, or -1 w/ errno
 * @cancellationpoint
 * @restartable
 */
ssize_t writev(int fd, const struct iovec *iov, int iovlen) {
  ssize_t rc;

  if (fd >= 0 && iovlen >= 0) {
    // The cosmo version of this code checks if fd < fds count but blink
    // doesn't appear to keep track of real fds since the host os can do
    // it but windows of course can't. Its possible that a open handle
    // count would match but I am not sure. Its also possible that gnulib
    // might polyfill a method based on a posix one but I can only find
    // info on a brute force method and a definitely unavailable /proc
    // one. https://stackoverflow.com/questions/7976769
    rc = sys_writev_nt(fd, iov, iovlen);
  } else if (fd < 0) {
    rc = ebadf();
  } else {
    rc = einval();
  }

  STRACE("writev(%d, %s, %d) → %'ld% m", fd,
         DescribeIovec(rc != -1 ? rc : -2, iov, iovlen), iovlen, rc);
  return rc;
}
