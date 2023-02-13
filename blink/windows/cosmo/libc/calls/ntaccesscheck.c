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
#include <windef.h>
#include <winbase.h>
#include <stdlib.h>
#include <stdint.h>
#include <uchar.h>

#include "blink/errno.h"
#include "blink/windows/macros.h"
#include "blink/windows/cosmo/libc/calls/syscall_support-nt.internal.h"
#include "blink/windows/cosmo/libc/sysv/errfuns.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/ntaccesscheck.c

/**
 * Asks Microsoft if we're authorized to use a folder or file.
 *
 * Implementation Details: MSDN documentation imposes no limit on the
 * internal size of SECURITY_DESCRIPTOR, which we are responsible for
 * allocating. We've selected 1024 which shall hopefully be adequate.
 *
 * @param flags can have R_OK, W_OK, X_OK, etc.
 * @return 0 if authorized, or -1 w/ errno
 * @kudos Aaron Ballman for teaching this
 * @see libc/sysv/consts.sh
 */
int ntaccesscheck(const char16_t *pathname, DWORD flags) {
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
  hImpersonatedToken = hToken = (HANDLE)-1;
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
            STRACE("ntaccesscheck finale failed %d %d", result, flags);
            rc = eacces();
          }
        } else {
          rc = __winerr();
          STRACE("%s(%#hs) failed: %m", "AccessCheck", pathname);
        }
      } else {
        rc = __winerr();
        STRACE("%s(%#hs) failed: %m", "DuplicateToken", pathname);
      }
    } else {
      rc = __winerr();
      STRACE("%s(%#hs) failed: %m", "OpenProcessToken", pathname);
    }
  } else {
    e = GetLastError();
    if (e == ERROR_INSUFFICIENT_BUFFER) {
      if (!freeme && (freeme = malloc(secsize))) {
        s = freeme;
        goto TryAgain;
      } else {
        rc = enomem();
        STRACE("%s(%#hs) failed: %m", "GetFileSecurity", pathname);
      }
    } else {
      errno = e;
      STRACE("%s(%#hs) failed: %m", "GetFileSecurity", pathname);
      rc = -1;
    }
  }
  if (freeme) free(freeme);
  if (hImpersonatedToken != (HANDLE)-1) CloseHandle(hImpersonatedToken);
  if (hToken != (HANDLE)-1) CloseHandle(hToken);
  return rc;
}
