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
#include <stdarg.h>
#include <stdint.h>

#include "blink/errno.h"
#include "blink/win.h"

#ifdef __MINGW64_VERSION_MAJOR
#include <io.h>
#include <windef.h>
#include <winbase.h>

// Based on https://github.com/jart/cosmopolitan/blob/e1b8339/libc/calls/fcntl-nt.c#L309-L350
static int sys_fcntl_nt(int fd, int cmd, uintptr_t arg) {
  int rc;
  uint32_t flags;
  HANDLE fdHandle = (HANDLE)_get_osfhandle(fd);
  // Cosmo version had kFdFile ≈ FILE_TYPE_DISK and kFdSocket ≈ FILE_TYPE_PIPE but then g_fds.c marks
  // the windows standard handles as kFdFile while windows marks them as FILE_TYPE_CHAR ≈ kFdConsole
  if (GetFileType(fdHandle) == FILE_TYPE_DISK || GetFileType(fdHandle) == FILE_TYPE_PIPE ||
      GetFileType(fdHandle) == FILE_TYPE_CHAR) {
    /*if (cmd == F_GETFL) {
      //TODO exit or otherwise report the lack of a getfl implementation
      rc = enosys();
    } else if (cmd == F_SETFL) {
      //TODO exit or otherwise report the lack of a setfl implementation
      rc = enosys();
    } else if (cmd == F_GETFD) {
      //TODO exit or otherwise report the lack of a getfd implementation
      rc = enosys();
    } else if (cmd == F_SETFD) {
      //TODO exit or otherwise report the lack of a setfd implementation
      rc = enosys();
    } else if (cmd == F_SETLK || cmd == F_SETLKW || cmd == F_GETLK) {
      //TODO exit or otherwise report the lack of a setlk/setlkw/getlk implementation
    } else*/ if (cmd == F_DUPFD || cmd == F_DUPFD_CLOEXEC) {
      rc = sys_dup_nt(fd, -1, (cmd == F_DUPFD_CLOEXEC ? O_CLOEXEC : 0));
    } else {
      rc = einval();
    }
  } else {
    rc = ebadf();
  }
  return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/e1b8339/libc/calls/fcntl.c#L102-L166
int fcntl(int fd, int cmd, ...) {
  int rc;
  va_list va;
  uintptr_t arg;

  va_start(va, cmd);
  arg = va_arg(va, uintptr_t);
  va_end(va);

  if (fd >= 0) {
    if (cmd >= 0) {
      rc = sys_fcntl_nt(fd, cmd, arg);
    } else {
      rc = einval();
    }
  } else {
    rc = ebadf();
  }

  return rc;
}
#endif
