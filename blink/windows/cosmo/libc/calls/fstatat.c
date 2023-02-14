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
#include <sys/stat.h>

#include "blink/windows/macros.h"
#include "blink/windows/cosmo/libc/calls/struct/stat.internal.h"
#include "blink/windows/cosmo/libc/fmt/itoa.h"
#include "blink/windows/cosmo/libc/intrin/describeflags.internal.h"
#include "third_party/gnulib_build/lib/fcntl.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/fstatat.c

static inline const char *__strace_fstatat_flags(char buf[12], int flags) {
  if (flags == AT_SYMLINK_NOFOLLOW) return "AT_SYMLINK_NOFOLLOW";
  FormatInt32(buf, flags);
  return buf;
}

/**
 * Returns information about thing.
 *
 * @param dirfd is normally AT_FDCWD but if it's an open directory and
 *     file is a relative path, then file becomes relative to dirfd
 * @param st is where result is stored
 * @param flags can have AT_SYMLINK_NOFOLLOW
 * @return 0 on success, or -1 w/ errno
 * @see S_ISDIR(st.st_mode), S_ISREG()
 * @asyncsignalsafe
 * @vforksafe
 */
int fstatat(int dirfd, const char *path, struct stat *st, int flags) {
  /* execve() depends on this */
  int rc;
  rc = sys_fstatat_nt(dirfd, path, st, flags);
  STRACE("fstatat(%s, %#s, [%s], %s) -> %d %m", DescribeDirfd(dirfd), path,
         DescribeStat(rc, st), __strace_fstatat_flags(alloca(12), flags), rc);
  return rc;
}
