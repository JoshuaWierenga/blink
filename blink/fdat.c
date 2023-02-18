/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2023 Justine Alexandra Roberts Tunney                              │
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
#define _WIN32_WINNT 0x0600 // Vista, required for MinGW to define GetFinalPathNameByHandleW
#include <limits.h>
#include <windef.h>
#include <winbase.h>
#include <stdbool.h>
#include <stringapiset.h>

#include "blink/assert.h"
#include "blink/errno.h"
#include "blink/fdat.h"
#include "blink/macros.h"
#include "blink/types.h"
#include "blink/util.h"

int64_t __winerr(void) {
  int i;
  errno_t e = GetLastError();
  if (e) {
    for (i = 0; kDos2Errno[i].doscode; ++i) {
      if (e == kDos2Errno[i].doscode) {
        return kDos2Errno[i].errnocode;
      }
    }
  }
  unassert(e > 0);
  errno = e;
  return -1;
}


static inline bool IsSlash(char c) {
  return c == '/' || c == '\\';
}

static inline int IsAlpha(int c) {
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

const struct NtMagicPaths {
#define TAB(NAME, STRING) char NAME[sizeof(STRING)];
TAB(devtty, "/dev/tty")
TAB(devnull, "/dev/null")
TAB(devstdin, "/dev/stdin")
TAB(devstdout, "/dev/stdout")
TAB(nul, "NUL")
TAB(conin, "CONIN$")
TAB(conout, "CONOUT$")
#undef TAB
} kNtMagicPaths = {
#define TAB(NAME, STRING) STRING,
TAB(devtty, "/dev/tty")
TAB(devnull, "/dev/null")
TAB(devstdin, "/dev/stdin")
TAB(devstdout, "/dev/stdout")
TAB(nul, "NUL")
TAB(conin, "CONIN$")
TAB(conout, "CONOUT$")
#undef TAB
};

static const char *FixNtMagicPath(const char *path, unsigned flags) {
  if (!IsSlash(path[0])) return path;
  if (strcmp(path, kNtMagicPaths.devtty) == 0) {
    if ((flags & O_ACCMODE) == O_RDONLY) {
      return kNtMagicPaths.conin;
    } else if ((flags & O_ACCMODE) == O_WRONLY) {
      return kNtMagicPaths.conout;
    }
  }
  if (strcmp(path, kNtMagicPaths.devnull) == 0) return kNtMagicPaths.nul;
  if (strcmp(path, kNtMagicPaths.devstdin) == 0) return kNtMagicPaths.conin;
  if (strcmp(path, kNtMagicPaths.devstdout) == 0) return kNtMagicPaths.conout;
  return path;
}

int __mkntpath2(const char *path, wchar_t path16[static PATH_MAX], int flags) {
  const char *q;
  bool isdospath;
  wchar_t c, *p;
  size_t i, j, n, m, x, z;
  if (!path) return efault();
  path = FixNtMagicPath(path, flags);
  p = path16;
  q = path;

  if ((IsSlash(q[0]) && IsAlpha(q[1]) && (!q[2] || IsSlash(q[2]))) ||
      (IsAlpha(q[0]) && q[1] == ':' && IsSlash(q[2]))) {
    z = MIN(32767, PATH_MAX);
    // turn "\c\foo" into "\\?\c:\foo" and "\c" into "\\?\c:\"
    wcsncpy(p, L"\\\\?\\X:\\", 7);
    p[4] = q[1] == ':' ? q[0] : q[1];
    p += 7;
    q += !q[2] ? 2 : 3;
    z -= 7;
    x = 7;
  } else if (IsSlash(q[0]) && IsSlash(q[1]) && q[2] == '?' && IsSlash(q[3])) {
    z = MIN(32767, PATH_MAX);
    x = 0;
  } else {
    z = MIN(260, PATH_MAX);
    x = 0;
  }

  // turn /tmp into GetTempPath()
  if (!x && IsSlash(q[0]) && q[1] == 't' && q[2] == 'm' && q[3] == 'p' &&
      (IsSlash(q[4]) || !q[4])) {
    m = GetTempPathW(z, p);
    if (!q[4]) return m;
    q += 5;
    p += m;
    z -= m;
  } else {
    m = 0;
  }

  // turn utf-8 into utf-16
  n = MultiByteToWideChar(CP_UTF8, 0, q, strlen(q), NULL, 0);
  if (n >= z - 1) { 
    return enametoolong();
  }
  MultiByteToWideChar(CP_UTF8, 0, q, strlen(q), p, z);
  

  // 1. turn `/` into `\`
  // 2. turn `\\` into `\` if not at beginning
  for (j = i = 0; i < n; ++i) {
    c = p[i];
    if (c == '/') {
      c = '\\';
    }
    if (j > 1 && c == '\\' && p[j - 1] == '\\') {
      continue;
    }
    p[j++] = c;
  }
  p[j] = 0;
  n = j;

  // our path is now stored at `path16` with length `n`
  n = x + m + n;

  // To avoid toil like this:
  //
  //     CMD.EXE was started with the above path as the current directory.
  //     UNC paths are not supported.  Defaulting to Windows directory.
  //     Access is denied.
  //
  // Remove \\?\ prefix if we're within 260 character limit.
  if (n > 4 && n < 260 &&   //
      path16[0] == '\\' &&  //
      path16[1] == '\\' &&  //
      path16[2] == '?' &&   //
      path16[3] == '\\') {
    memmove(path16, path16 + 4, (n - 4 + 1) * sizeof(*path16));
    n -= 4;
  }

  return n;
}

// Based on https://github.com/jart/cosmopolitan/blob/2b6261a/libc/calls/mkntpathat.c
int __mkntpathat(int dirfd, const char *path, int flags, wchar_t file[static PATH_MAX]) {
  wchar_t dir[PATH_MAX];
  uint32_t dirlen, filelen;
  HANDLE h;
  if ((filelen = __mkntpath2(path, file, flags)) == -1) return -1;
  if (!filelen) return enoent();
  if (file[0] != L'\\' && dirfd != AT_FDCWD) {
    if ((h = (HANDLE)_get_osfhandle(dirfd)) == INVALID_HANDLE_VALUE ||
        GetFileType(h) != FILE_TYPE_DISK) {
      return ebadf();
    }
    dirlen = GetFinalPathNameByHandleW(h, dir, ARRAYLEN(dir),
                                       FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (!dirlen) return __winerr();
    if (dirlen + 1 + filelen + 1 > ARRAYLEN(dir)) {
      return enametoolong();
    }
    dir[dirlen] = L'\\';
    memcpy(dir + dirlen + 1, file, (filelen + 1) * sizeof(*file));
    memcpy(file, dir, (dirlen + 1 + filelen + 1) * sizeof(*file));
    return dirlen + 1 + filelen;
  } else {
    return filelen;
  }
}


// faccessat is not supported on windows

int faccessat_(int dirfd, const char *path, int mode, int flags) {
  wchar_t path16[PATH_MAX];
  
  if (!path) {
    return efault();
  }
  
  if (__mkntpathat(dirfd, path, 0, path16) == -1) return -1;
  return _waccess(path16, mode);
}

// TODO: Add openat
