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
#include <sys/stat.h>
#include <uchar.h>
#include <windef.h>
#include <winbase.h>

#include "blink/windows/cosmo/libc/calls/struct/stat.internal.h"
#include "blink/windows/cosmo/libc/calls/syscall_support-nt.internal.h"
#include "third_party/gnulib_build/lib/fcntl.h"

/*#include "libc/calls/struct/stat.h"
#include "libc/calls/struct/stat.internal.h"
#include "libc/calls/syscall_support-nt.internal.h"
#include "libc/nt/createfile.h"
#include "libc/nt/enum/accessmask.h"
#include "libc/nt/enum/creationdisposition.h"
#include "libc/nt/enum/fileflagandattributes.h"
#include "libc/nt/enum/filesharemode.h"
#include "libc/nt/runtime.h"
#include "libc/sysv/consts/at.h"*/

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/fstatat-nt.c

int sys_fstatat_nt(int dirfd, const char *path, struct stat *st,
                               int flags) {
  int rc;
  HANDLE fh;
  uint16_t path16[PATH_MAX];
  if (__mkntpathat(dirfd, path, 0, path16) == -1) return -1;
  if ((fh = CreateFileW(
           path16, FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING,
           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS |
               ((flags & AT_SYMLINK_NOFOLLOW) ? FILE_FLAG_OPEN_REPARSE_POINT
                                              : 0),
           0)) != (HANDLE)-1) {
    rc = st ? sys_fstat_nt(fh, st) : 0;
    CloseHandle(fh);
  } else {
    rc = __winerr();
  }
  return __fix_enotdir(rc, path16);
}
