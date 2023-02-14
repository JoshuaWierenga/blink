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
#include <ntdef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <uchar.h>
#include <windef.h>
#include <winbase.h>
#include <winioctl.h>

#include "blink/bitscan.h"
#include "blink/errno.h"
#include "blink/tpenc.h"
#include "blink/windows/macros.h"
#include "blink/windows/cosmo/libc/calls/syscall_support-nt.internal.h"
#include "blink/windows/cosmo/libc/calls/struct/stat.internal.h"
#include "blink/windows/cosmo/libc/fmt/conv.h"
#include "blink/windows/cosmo/libc/integral/normalize.inc"
#include "blink/windows/cosmo/libc/str/utf16.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/macros.internal.h

#define ROUNDUP(X, K)       (((X) + (K)-1) & -(K))

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/sysv/consts/s.h

#define S_IFIFO  0010000 /* pipe */
#define S_IFCHR  0020000 /* character device */
#define S_IFDIR  0040000 /* directory */
#define S_IFREG  0100000 /* regular file */
#define S_IFLNK  0120000 /* symbolic link */
#define S_IFMT   0170000 /* mask of file types above */

#define S_ISDIR(mode)  (((mode)&S_IFMT) == S_IFDIR)
#define S_ISREG(mode)  (((mode)&S_IFMT) == S_IFREG)
#define S_ISLNK(mode)  (((mode)&S_IFMT) == S_IFLNK)

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/fstat-nt.h

static uint32_t GetSizeOfReparsePoint(HANDLE fh) {
  wint_t x, y;
  const char16_t *p;
  uint32_t mem, i, z = 0;
  DWORD n;
  REPARSE_DATA_BUFFER *rdb;
  long buf[(sizeof(*rdb) + PATH_MAX * sizeof(char16_t)) / sizeof(long)];
  mem = sizeof(buf);
  rdb = (REPARSE_DATA_BUFFER *)buf;
  if (DeviceIoControl(fh, FSCTL_GET_REPARSE_POINT, 0, 0, rdb, mem, &n, 0)) {
    i = 0;
    n = rdb->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(char16_t);
    p = (char16_t *)((char *)rdb->SymbolicLinkReparseBuffer.PathBuffer +
                     rdb->SymbolicLinkReparseBuffer.PrintNameOffset);
    while (i < n) {
      x = p[i++] & 0xffff;
      if (!IsUcs2(x)) {
        if (i < n) {
          y = p[i++] & 0xffff;
          x = MergeUtf16(x, y);
        } else {
          x = 0xfffd;
        }
      }
      z += x < 0200 ? 1 : bsr(tpenc(x)) >> 3;
    }
  } else {
    STRACE("%s failed %m", "GetSizeOfReparsePoint");
  }
  return z;
}

int sys_fstat_nt(HANDLE handle, struct stat *st) {
  int filetype;
  LARGE_INTEGER actualsize;
  FILE_COMPRESSION_INFO fci;
  BY_HANDLE_FILE_INFORMATION wst;
  if (!st) return efault();
  if ((filetype = GetFileType(handle))) {
    memset(st, 0, sizeof(*st));
    switch (filetype) {
      case FILE_TYPE_CHAR:
        st->st_mode = S_IFCHR | 0644;
        break;
      case FILE_TYPE_PIPE:
        st->st_mode = S_IFIFO | 0644;
        break;
      case FILE_TYPE_DISK:
        if (GetFileInformationByHandle(handle, &wst)) {
          st->st_mode = 0555;
          st->st_flags = wst.dwFileAttributes;
          if (!(wst.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
            st->st_mode |= 0200;
          }
          if (wst.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            st->st_mode |= S_IFDIR;
          } else if (wst.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            st->st_mode |= S_IFLNK;
          } else {
            st->st_mode |= S_IFREG;
          }
          st->st_atim = FileTimeToTimeSpec(wst.ftLastAccessTime);
          st->st_mtim = FileTimeToTimeSpec(wst.ftLastWriteTime);
          st->st_ctim = FileTimeToTimeSpec(wst.ftCreationTime);
          st->st_birthtim = st->st_ctim;
          st->st_size = (uint64_t)wst.nFileSizeHigh << 32 | wst.nFileSizeLow;
          st->st_blksize = PAGESIZE;
          st->st_dev = wst.dwVolumeSerialNumber;
          st->st_rdev = 0;
          st->st_ino = (uint64_t)wst.nFileIndexHigh << 32 | wst.nFileIndexLow;
          st->st_nlink = wst.nNumberOfLinks;
          if (S_ISLNK(st->st_mode)) {
            if (!st->st_size) {
              st->st_size = GetSizeOfReparsePoint(handle);
            }
          } else {
            actualsize.QuadPart = st->st_size;
            if (S_ISREG(st->st_mode) &&
                GetFileInformationByHandleEx(handle, FileCompressionInfo,
                                             &fci, sizeof(fci))) {
              actualsize = fci.CompressedFileSize;
            }
            st->st_blocks = ROUNDUP(actualsize.QuadPart, PAGESIZE) / 512;
          }
        } else {
          STRACE("%s failed %m", "GetFileInformationByHandle");
        }
        break;
      default:
        break;
    }
    return 0;
  } else {
    STRACE("%s failed %m", "GetFileType");
    return __winerr();
  }
}
