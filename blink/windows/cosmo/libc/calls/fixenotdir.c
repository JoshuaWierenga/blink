/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2022 Justine Alexandra Roberts Tunney                              │
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
#include <error.h>
#include <fileapi.h>
#include <inttypes.h>
#include <stdbool.h>

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/fixenotdir.c

static bool SubpathExistsThatsNotDirectory(wchar_t *path) {
  int e;
  wchar_t *p;
  uint32_t attrs;
  e = errno;
  while ((p = wcsrchr(path, '\\'))) {
    *p = u'\0';
    if ((attrs = GetFileAttributesW(path)) != -1u) {
      if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        return false;
      } else {
        return true;
      }
    } else {
      errno = e;
    }
  }
  return false;
}

int64_t __fix_enotdir3(int64_t rc, wchar_t *path1, wchar_t *path2) {
  if (rc == -1 && errno == ERROR_PATH_NOT_FOUND) {
    if ((!path1 || !SubpathExistsThatsNotDirectory(path1)) &&
        (!path2 || !SubpathExistsThatsNotDirectory(path2))) {
      errno = ERROR_FILE_NOT_FOUND;
    }
  }
  return rc;
}

// WIN32 doesn't distinguish between ENOTDIR and ENOENT. UNIX strictly
// requires that a directory component *exists* but is not a directory
// whereas WIN32 will return ENOTDIR if a dir label simply isn't found
//
// - ENOTDIR: A component used as a directory in pathname is not, in
//   fact, a directory. -or- pathname is relative and dirfd is a file
//   descriptor referring to a file other than a directory.
//
// - ENOENT: A directory component in pathname does not exist or is a
//   dangling symbolic link.
//
int64_t __fix_enotdir(int64_t rc, wchar_t *path) {
  return __fix_enotdir3(rc, path, 0);
}
