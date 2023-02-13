/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
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
#include <limits.h>
#include <uchar.h>
#include <windef.h>
#include <winbase.h>

#include "blink/errno.h"
#include "blink/macros.h"
#include "blink/windows/macros.h"
#include "blink/windows/cosmo/libc/calls/syscall_support-nt.internal.h"
#include "blink/windows/cosmo/libc/sysv/errfuns.h"
#include "third_party/gnulib_build/lib/fcntl.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/mkntpathat.c

int __mkntpathat(int dirfd, const char *path, int flags,
                 char16_t file[PATH_MAX]) {
  char16_t dir[PATH_MAX];
  uint32_t dirlen, filelen;
  HANDLE handle;
  if ((filelen = __mkntpath2(path, file, flags)) == -1) return -1;
  if (!filelen) return enoent();
  if (file[0] != u'\\' && dirfd != AT_FDCWD) { /* ProTip: \\?\C:\foo */
    handle = (HANDLE)_get_osfhandle(dirfd);
    if (dirfd < 0 || GetFileType(handle) != FILE_TYPE_DISK) return ebadf();
    dirlen = GetFinalPathNameByHandleW(handle, dir, ARRAYLEN(dir),
                                       FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (!dirlen) return __winerr();
    if (dirlen + 1 + filelen + 1 > ARRAYLEN(dir)) {
      STRACE("path too long: %#.*hs\\%#.*hs", dirlen, dir, filelen, file);
      return enametoolong();
    }
    dir[dirlen] = u'\\';
    memcpy(dir + dirlen + 1, file, (filelen + 1) * sizeof(char16_t));
    memcpy(file, dir, (dirlen + 1 + filelen + 1) * sizeof(char16_t));
    return dirlen + 1 + filelen;
  } else {
    return filelen;
  }
}
