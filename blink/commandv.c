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
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blink/windows.h"
#ifdef WINBLINK
#include <Windows.h>
#else
#include <unistd.h>

#include "blink/builtin.h"
#include "blink/overlays.h"
#endif
#include "blink/util.h"
#ifndef WINBLINK
#include "blink/vfs.h"
#endif

struct PathSearcher {
  char *path;
  size_t pathlen;
  size_t namelen;
  const char *name;
  char *syspath;
};

static char EndsWithIgnoreCase(const char *p, size_t n, const char *s) {
  size_t i, m;
  if (n >= (m = strlen(s))) {
    for (i = n - m; i < n; ++i) {
      if (tolower(p[i]) != *s++) {
        return 0;
      }
    }
    return 1;
  } else {
    return 0;
  }
}

static char IsComPath(struct PathSearcher *ps) {
  return EndsWithIgnoreCase(ps->name, ps->namelen, ".com") ||
         EndsWithIgnoreCase(ps->name, ps->namelen, ".exe") ||
         EndsWithIgnoreCase(ps->name, ps->namelen, ".com.dbg");
}

static char AccessCommand(struct PathSearcher *ps, const char *suffix,
                          size_t pathlen) {
  size_t suffixlen;
#ifdef WINBLINK
  char fullpath[PATH_MAX] = {0};
  DWORD filesecuritysize, privilegeslength, useraccessrequest, useraccessactual;
  PSECURITY_DESCRIPTOR filesecurity;
  HANDLE usertoken, impersonatedusertoken;
  GENERIC_MAPPING accessmapping;
  PRIVILEGE_SET privileges;
  BOOL accessstatus;
  bool fileexecutable;
#endif
  suffixlen = strlen(suffix);
  if (pathlen + 1 + ps->namelen + suffixlen + 1 > ps->pathlen) return 0;
  if (pathlen && ps->path[pathlen - 1] != '/') ps->path[pathlen++] = '/';
  memcpy(ps->path + pathlen, ps->name, ps->namelen);
  memcpy(ps->path + pathlen + ps->namelen, suffix, suffixlen + 1);
#ifdef WINBLINK
  fileexecutable = 0;
  fullpath[0] = 0;
  if (GetFullPathNameA(ps->path, sizeof fullpath, fullpath, NULL) == 0 ||
      fullpath[0] == 0) {
    return 0;
  }
  // Based on https://blog.aaronballman.com/2011/08/how-to-check-access-rights/
  GetFileSecurityA(fullpath,
                   OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                       DACL_SECURITY_INFORMATION,
                   NULL, 0, &filesecuritysize);
  if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) return 0;
  if (!(filesecurity = malloc(filesecuritysize))) return 0;
  if (GetFileSecurityA(fullpath,
                       OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                           DACL_SECURITY_INFORMATION,
                       filesecurity, filesecuritysize, &filesecuritysize)) {
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &usertoken)) {
      if (DuplicateToken(usertoken, SecurityImpersonation,
                         &impersonatedusertoken)) {
        // TODO Check if execute is useful here, the binary is a elf binary so
        // can't run directly on windows. Should this be FILE_GENERIC_READ?
        useraccessrequest = FILE_GENERIC_EXECUTE;
        accessmapping =
            (GENERIC_MAPPING){FILE_GENERIC_READ, FILE_GENERIC_WRITE,
                              FILE_GENERIC_EXECUTE, FILE_ALL_ACCESS};
        privilegeslength = sizeof privileges;
        if (AccessCheck(filesecurity, impersonatedusertoken, useraccessrequest,
                        &accessmapping, &privileges, &privilegeslength,
                        &useraccessactual, &accessstatus)) {
          fileexecutable = accessstatus == TRUE;
        }
        CloseHandle(impersonatedusertoken);
      }
      CloseHandle(usertoken);
    }
    free(filesecurity);
  }

  return (char)fileexecutable;

#else
  return !VfsAccess(AT_FDCWD, ps->path, X_OK, 0);
#endif
}

static char BlinkSearchPath(struct PathSearcher *ps, const char *suffix) {
  const char *p;
  size_t i;
  for (p = ps->syspath;;) {
#ifdef WINBLINK
    for (i = 0; p[i] && p[i] != ';'; ++i) {
#else
    for (i = 0; p[i] && p[i] != ':'; ++i) {
#endif
      if (i < ps->pathlen) {
        ps->path[i] = p[i];
      }
    }
    if (AccessCommand(ps, suffix, i)) {
      return 1;
#ifdef WINBLINK
    } else if (p[i] == ';') {
#else
    } else if (p[i] == ':') {
#endif
      p += i + 1;
    } else {
      return 0;
    }
  }
}

static char FindCommand(struct PathSearcher *ps, const char *suffix) {
  if (memchr(ps->name, '/', ps->namelen) ||
      memchr(ps->name, '\\', ps->namelen)) {
    ps->path[0] = 0;
    return AccessCommand(ps, suffix, 0);
  } else {
    if (AccessCommand(ps, suffix, 0)) return 1;
  }
  return BlinkSearchPath(ps, suffix);
}

/**
 * Searches for command `name` on system `$PATH`.
 */
char *Commandv(const char *name, char *buf, size_t size) {
  struct PathSearcher ps;
#ifdef WINBLINK
  char sysroot[_MAX_PATH];
  size_t sysrootlen, syspathlen;
#endif
  bool res;
  ps.path = buf;
  ps.pathlen = size;
  if (!(ps.namelen = strlen((ps.name = name)))) return 0;
  if (ps.namelen + 1 > ps.pathlen) return 0;
#ifdef WINBLINK
  if (_dupenv_s(&ps.syspath, NULL, "PATH")) {
    if (getenv_s(&sysrootlen, sysroot, sizeof sysroot, "SystemRoot")) return 0;
    if (sysrootlen == 0) return 0;
    syspathlen = snprintf(NULL, 0, "%s\\System32;%s;%s\\System32\\wbem",
                          sysroot, sysroot, sysroot);
    if (!(ps.syspath = malloc((syspathlen + 1) * sizeof *ps.syspath))) return 0;
    snprintf(ps.syspath, syspathlen + 1, "%s\\System32;%s;%s\\System32\\wbem",
             sysroot, sysroot, sysroot);
  }
#else
  ps.syspath = getenv("PATH");
  if (!ps.syspath) ps.syspath = "/usr/local/bin:/bin:/usr/bin";
#endif
  res = FindCommand(&ps, "") || (!IsComPath(&ps) && FindCommand(&ps, ".com"));
#ifdef WINBLINK
  free(ps.syspath);
#endif
  if (res) {
    return ps.path;
  } else {
    return 0;
  }
}
