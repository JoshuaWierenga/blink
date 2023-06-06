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

#include "blink/assert.h"
#include "blink/errno.h"
#include "blink/win.h"

#ifdef __MINGW64_VERSION_MAJOR
#include <io.h>
#include <windef.h>
#include <winbase.h>

// Based on https://github.com/jart/cosmopolitan/blob/e1b8339/libc/calls/dup-nt.c#L32-L84
int sys_dup_nt(int oldfd, int newfd, int flags) {
  int64_t rc;
  HANDLE proc, handle, newHandle;
  handle = (HANDLE)_get_osfhandle(oldfd);
  unassert(!(flags & ~O_CLOEXEC));

  // Cosmo version only required newfd >= -1 as it picked newfd if not provided
  // but now windows must pick it and so speicifying one manually is not supported
  if (oldfd < 0 || newfd != -1 ||
      (GetFileType(handle) != FILE_TYPE_DISK && GetFileType(handle) != FILE_TYPE_PIPE &&
       GetFileType(handle) != FILE_TYPE_CHAR)) {
    return ebadf();
  }

  proc = GetCurrentProcess();

  if (DuplicateHandle(proc, handle, proc, &newHandle, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
    // Have to make sure CloseHandle is never called on newHandle from now on,
    // since it isn't returned from this function that isn't an issue.
    // close/_close on rc is fine though
    // TODO Find a way to get the flags on oldfd to apply to the new handle, probably {low_, }info or
    // fully wrapping fd management like cosmo does to keep track fd info would work, both would also
    // solve the next todo as well, additionally some of this might be handled by DUPLICATE_SAME_ACCESS
    // TODO Find a way to keep track of FD_CLOEXEC so it actually works
    rc = _open_osfhandle((intptr_t)newHandle, 0);
  } else {
    CloseHandle(newHandle);
    // TODO Add _winerr and dos error to errno mapping
    rc = enosys();
  }

  return rc;
}
#endif
