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
#include <unistd.h>
#include <stdint.h>

#include "blink/errno.h"
#include "blink/windows/macros.h"
#include "blink/windows/cosmo/libc/calls/syscall-nt.internal.h"
#include "blink/windows/cosmo/libc/intrin/describeflags.internal.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/faccessat.c

/**
 * Checks if effective user can access path in particular ways.
 *
 * @param dirfd is normally AT_FDCWD but if it's an open directory and
 *     file is a relative path, then file is opened relative to dirfd
 * @param path is a filename or directory
 * @param amode can be `R_OK`, `W_OK`, `X_OK`, or `F_OK`
 * @param flags can have `AT_EACCESS` and/or `AT_SYMLINK_NOFOLLOW`
 * @return 0 if ok, or -1 and sets errno
 * @raise EINVAL if `mode` has bad value
 * @raise EPERM if pledge() is in play without rpath promise
 * @raise EACCES if access for requested `mode` would be denied
 * @raise ENOTDIR if a directory component in `path` exists as non-directory
 * @raise ENOENT if component of `path` doesn't exist or `path` is empty
 * @raise ENOTSUP if `path` is a zip file and `dirfd` isn't `AT_FDCWD`
 * @note on Linux `flags` is only supported on Linux 5.8+
 * @asyncsignalsafe
 */
int faccessat(int dirfd, const char *path, int amode, int flags) {
  int rc;
  if (!path) {
    rc = efault();
  } else {
    rc = sys_faccessat_nt(dirfd, path, amode, flags);
  }
  STRACE("faccessat(%s, %#s, %#o, %#x) → %d% m", DescribeDirfd(dirfd), path,
         amode, flags, rc);
  return rc;
}
