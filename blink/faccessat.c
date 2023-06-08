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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "blink/errno.h"
#include "blink/likely.h"
#include "blink/macros.h"
#include "blink/win.h"

#ifdef __MINGW64_VERSION_MAJOR
#include <esent.h>
#include <fcntl.h>
#include <fileapi.h>
#include <windef.h>
#include <winbase.h>

// From https://github.com/jart/cosmopolitan/blob/01fd655/libc/integral/normalize.inc#L84
#undef PATH_MAX
#define PATH_MAX 1024

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/ntmagicpaths.c
// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/ntmagicpaths.inc
// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/ntmagicpaths.internal.h
struct NtMagicPaths {
  char devtty[sizeof("/dev/tty")];
  char devnull[sizeof("/dev/null")];
  char devstdin[sizeof("/dev/stdin")];
  char devstdout[sizeof("/dev/stdout")];
  char nul[sizeof("NUL")];
  char conin[sizeof("CONIN$")];
  char conout[sizeof("CONOUT$")];
};

const struct NtMagicPaths kNtMagicPaths = {
  "/dev/tty",
  "/dev/null",
  "/dev/stdin",
  "/dev/stdout",
  "NUL",
  "CONIN$",
  "CONOUT$",
};

// From https://github.com/jart/cosmopolitan/blob/01fd655/libc/integral/c.inc#L138-L140
typedef struct {
  intptr_t ax, dx;
} axdx_t;

// From https://github.com/jart/cosmopolitan/blob/01fd655/libc/intrin/bsr.h
#define _bsr(x)   (__builtin_clz(x) ^ (sizeof(int) * CHAR_BIT - 1))

// From https://github.com/jart/cosmopolitan/blob/01fd655/libc/str/thompike.h
#define ThomPikeCont(x)     (0200 == (0300 & (x)))
#define ThomPikeByte(x)     ((x) & (((1 << ThomPikeMsb(x)) - 1) | 3))
#define ThomPikeLen(x)      (7 - ThomPikeMsb(x))
#define ThomPikeMsb(x)      ((255 & (x)) < 252 ? _bsr(255 & ~(x)) : 1)
#define ThomPikeMerge(x, y) ((x) << 6 | 077 & (y))

// From https://github.com/jart/cosmopolitan/blob/01fd655/libc/str/utf16.h#L18-L25
#define EncodeUtf16(wc)                                       \
  (LIKELY((0x0000 <= (wc) && (wc) <= 0xFFFF) ||               \
          (0xE000 <= (wc) && (wc) <= 0xFFFF))                 \
       ? (wc)                                                 \
   : 0x10000 <= (wc) && (wc) <= 0x10FFFF                      \
       ? (((((wc)-0x10000) >> 10) + 0xD800) |                 \
          (unsigned)((((wc)-0x10000) & 1023) + 0xDC00) << 16) \
       : 0xFFFD)

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/str/tprecode8to16.c
// TODO Add sse2 version?
static axdx_t tprecode8to16(WCHAR *dst, size_t dstsize, const char *src) {
  axdx_t r;
  unsigned w;
  int x, y, a, b, i, n;
  r.ax = 0;
  r.dx = 0;
  for (;;) {
    x = src[r.dx++] & 0377;
    if (x >= 0300) {
      a = ThomPikeByte(x);
      n = ThomPikeLen(x) - 1;
      for (i = 0;;) {
        if (!(b = src[r.dx + i] & 0377)) break;
        if (!ThomPikeCont(b)) break;
        a = ThomPikeMerge(a, b);
        if (++i == n) {
          r.dx += i;
          x = a;
          break;
        }
      }
    }
    if (!x) break;
    w = EncodeUtf16(x);
    while (w && r.ax + 1 < dstsize) {
      dst[r.ax++] = w;
      w >>= 16;
    }
  }
  if (r.ax < dstsize) {
    dst[r.ax] = 0;
  }
  return r;
}

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/mkntpath.c
static inline bool IsSlash(char c) {
  return c == '/' || c == '\\';
}

static inline int IsAlpha(int c) {
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

static const char *FixNtMagicPath(const char *path,
                                  unsigned flags) {
  const struct NtMagicPaths *mp = &kNtMagicPaths;
  __asm__("" : "+r"(mp));
  if (!IsSlash(path[0])) return path;
  if (strcmp(path, mp->devtty) == 0) {
    if ((flags & O_ACCMODE) == O_RDONLY) {
      return mp->conin;
    } else if ((flags & O_ACCMODE) == O_WRONLY) {
      return mp->conout;
    }
  }
  if (strcmp(path, mp->devnull) == 0) return mp->nul;
  if (strcmp(path, mp->devstdin) == 0) return mp->conin;
  if (strcmp(path, mp->devstdout) == 0) return mp->conout;
  return path;
}

static int __mkntpath2(const char *path,
                       WCHAR path16[PATH_MAX], int flags) {
  /*
   * 1. Need +1 for NUL-terminator
   * 2. Need +1 for UTF-16 overflow
   * 3. Need ≥2 for SetCurrentDirectory trailing slash requirement
   * 4. Need ≥13 for mkdir() i.e. 1+8+3+1, e.g. "\\ffffffff.xxx\0"
   *    which is an "8.3 filename" from the DOS days
   */
  const char *q;
  bool isdospath;
  WCHAR c, *p;
  size_t i, j, n, m, x, z;
  if (!path) return efault();
  path = FixNtMagicPath(path, flags);
  p = path16;
  q = path;

  if (IsSlash(q[0]) && IsAlpha(q[1]) && IsSlash(q[2])) {
    z = MIN(32767, PATH_MAX);
    // turn "\c\foo" into "\\?\c:\foo"
    p[0] = '\\';
    p[1] = '\\';
    p[2] = '?';
    p[3] = '\\';
    p[4] = q[1];
    p[5] = ':';
    p[6] = '\\';
    p += 7;
    q += 3;
    z -= 7;
    x = 7;
  } else if (IsSlash(q[0]) && IsAlpha(q[1]) && !q[2]) {
    z = MIN(32767, PATH_MAX);
    // turn "\c" into "\\?\c:\"
    p[0] = '\\';
    p[1] = '\\';
    p[2] = '?';
    p[3] = '\\';
    p[4] = q[1];
    p[5] = ':';
    p[6] = '\\';
    p += 7;
    q += 2;
    z -= 7;
    x = 7;
  } else if (IsAlpha(q[0]) && q[1] == ':' && IsSlash(q[2])) {
    z = MIN(32767, PATH_MAX);
    // turn "c:\foo" into "\\?\c:\foo"
    p[0] = '\\';
    p[1] = '\\';
    p[2] = '?';
    p[3] = '\\';
    p[4] = q[0];
    p[5] = ':';
    p[6] = '\\';
    p += 7;
    q += 3;
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
  n = tprecode8to16(p, z, q).ax;
  if (n >= z - 1) {
    //STRACE("path too long for windows: %#s", path);
    return enametoolong();
  }

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
    memmove(path16, path16 + 4, (n - 4 + 1) * sizeof(WCHAR));
    n -= 4;
  }

  return n;
}

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/mkntpathat.c
static int __mkntpathat(int dirfd, const char *path, int flags,
                        WCHAR file[PATH_MAX]) {
  WCHAR dir[PATH_MAX];
  uint32_t dirlen, filelen;
  HANDLE fdHandle;
  if ((filelen = __mkntpath2(path, file, flags)) == -1) return -1;
  if (!filelen) return enoent();
  if (file[0] != u'\\' && dirfd != AT_FDCWD) { /* ProTip: \\?\C:\foo */
    fdHandle = (HANDLE)_get_osfhandle(dirfd);
    if (GetFileType(fdHandle) != FILE_TYPE_DISK
        && GetFileType(fdHandle) != FILE_TYPE_CHAR) return ebadf();
    dirlen = GetFinalPathNameByHandleW(fdHandle, dir, ARRAYLEN(dir),
                                       FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    // TODO Add _winerr and dos error to errno mapping
    if (!dirlen) return enosys(); // __winerr();
    if (dirlen + 1 + filelen + 1 > ARRAYLEN(dir)) {
      //STRACE("path too long: %#.*hs\\%#.*hs", dirlen, dir, filelen, file);
      return enametoolong();
    }
    dir[dirlen] = u'\\';
    memcpy(dir + dirlen + 1, file, (filelen + 1) * sizeof(WCHAR));
    memcpy(file, dir, (dirlen + 1 + filelen + 1) * sizeof(WCHAR));
    return dirlen + 1 + filelen;
  } else {
    return filelen;
  }
}

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/ntaccesscheck.c
static int ntaccesscheck(const WCHAR *pathname, DWORD flags) {
  int rc, e;
  void *freeme;
  BOOL result;
  SECURITY_DESCRIPTOR *s;
  GENERIC_MAPPING mapping;
  PRIVILEGE_SET privileges;
  HANDLE hToken, hImpersonatedToken;
  DWORD secsize, granted, privsize;
  intptr_t buffer[1024 / sizeof(intptr_t)];
  freeme = 0;
  granted = 0;
  result = false;
  s = (void *)buffer;
  secsize = sizeof(buffer);
  privsize = sizeof(privileges);
  memset(&privileges, 0, sizeof(privileges));
  mapping.GenericRead = FILE_GENERIC_READ;
  mapping.GenericWrite = FILE_GENERIC_WRITE;
  mapping.GenericExecute = FILE_GENERIC_EXECUTE;
  mapping.GenericAll = FILE_ALL_ACCESS;
  MapGenericMask(&flags, &mapping);
  hImpersonatedToken = hToken = INVALID_HANDLE_VALUE;
TryAgain:
  if (GetFileSecurityW(pathname,
                       OWNER_SECURITY_INFORMATION |
                          GROUP_SECURITY_INFORMATION |
                          DACL_SECURITY_INFORMATION,
                       s, secsize, &secsize)) {
    if (OpenProcessToken(GetCurrentProcess(),
                         TOKEN_IMPERSONATE | TOKEN_QUERY |
                             TOKEN_DUPLICATE | STANDARD_RIGHTS_READ,
                         &hToken)) {
      if (DuplicateToken(hToken, SecurityImpersonation,
                         &hImpersonatedToken)) {
        if (AccessCheck(s, hImpersonatedToken, flags, &mapping, &privileges,
                        &privsize, &granted, &result)) {
          if (result || flags == F_OK) {
            rc = 0;
          } else {
            //STRACE("ntaccesscheck finale failed %d %d", result, flags);
            rc = eacces();
          }
        } else {
          // TODO Add _winerr and dos error to errno mapping
          rc = enosys(); //__winerr();
          //STRACE("%s(%#hs) failed: %m", "AccessCheck", pathname);
        }
      } else {
        // TODO Add _winerr and dos error to errno mapping
        rc = enosys(); //__winerr();
        //STRACE("%s(%#hs) failed: %m", "DuplicateToken", pathname);
      }
    } else {
      // TODO Add _winerr and dos error to errno mapping
      rc = enosys(); //__winerr();
      //STRACE("%s(%#hs) failed: %m", "OpenProcessToken", pathname);
    }
  } else {
    e = GetLastError();
    if (e == ERROR_INSUFFICIENT_BUFFER) {
      if (!freeme && (freeme = malloc(secsize))) {
        s = freeme;
        goto TryAgain;
      } else {
        rc = enomem();
        //STRACE("%s(%#hs) failed: %m", "GetFileSecurity", pathname);
      }
    } else {
      errno = e;
      //STRACE("%s(%#hs) failed: %m", "GetFileSecurity", pathname);
      rc = -1;
    }
  }
  if (freeme) free(freeme);
  if (hImpersonatedToken != INVALID_HANDLE_VALUE) CloseHandle(hImpersonatedToken);
  if (hToken != INVALID_HANDLE_VALUE) CloseHandle(hToken);
  return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/fixenotdir.c
static bool SubpathExistsThatsNotDirectory(WCHAR *path) {
  int e;
  WCHAR *p;
  uint32_t attrs;
  e = errno;
  while ((p = wcschr(path, '\\'))) {
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

static dontinline int64_t __fix_enotdir(int64_t rc, WCHAR *path) {
  if (rc == -1 && errno == ERROR_PATH_NOT_FOUND) {
    if (!path || !SubpathExistsThatsNotDirectory(path)) {
      errno = ERROR_FILE_NOT_FOUND;
    }
  }
  return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/faccessat-nt.c
static int sys_faccessat_nt(int dirfd, const char *path, int mode,
                            uint32_t flags) {
  WCHAR path16[PATH_MAX];
  if (__mkntpathat(dirfd, path, 0, path16) == -1) return -1;
  return __fix_enotdir(ntaccesscheck(path16, mode), path16);
}

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/calls/faccessat.c
int faccessat(int dirfd, const char *path, int amode, int flags) {
  int rc;
  if (!path) {
    rc = efault();
  } else {
    rc = sys_faccessat_nt(dirfd, path, amode, flags);
  }
  /*STRACE("faccessat(%s, %#s, %#o, %#x) → %d% m", DescribeDirfd(dirfd), path,
         amode, flags, rc);*/
  return rc;
}

#endif // __MINGW64_VERSION_MAJOR
