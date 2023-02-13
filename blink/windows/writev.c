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
#include <errno.h>
#include <uchar.h>
#include <windef.h>
#include <winbase.h>

#include "third_party/gnulib_build/lib/sys/uio.h"

#include "blink/errno.h"
#include "blink/macros.h"
#include "blink/windows/macros.h"
#include "blink/windows/writev.h"
#include "blink/windows/cosmo/libc/calls/syscall_support-nt.internal.h"

// Based on https://github.com/jart/cosmopolitan/blob/6d36584/libc/calls/write-nt.c#L39-L84
static ssize_t sys_write_nt_impl(int fd, void *data, size_t size,
                                             ssize_t offset) {
    BOOL ok;
    HANDLE h;
    LARGE_INTEGER i, p;
    DWORD sent;
    OVERLAPPED overlap;

    h = (HANDLE)_get_osfhandle(fd);

    if (offset != -1) {
        // windows changes the file pointer even if overlapped is passed
        i.QuadPart = 0;
        _npassert(SetFilePointerEx(h, i, &p, SEEK_CUR));
    }

    if (offset == -1) {
        ok = WriteFile(h, data, MIN(size, 0x7ffff000), &sent, NULL);
    } else {
        memset(&overlap, 0, sizeof overlap);
        overlap.Pointer = (void *)(uintptr_t)offset;
        ok = WriteFile(h, data, MIN(size, 0x7ffff000), &sent, &overlap);
    }

    if (offset != -1) {
        // windows clobbers file pointer even on error
        _npassert(SetFilePointerEx(h, p, 0, SEEK_SET));
    }

    if (ok) {
        return sent;
    }

    switch (GetLastError()) {
        case ERROR_INVALID_HANDLE:
        case ERROR_ACCESS_DENIED:   // write doesn't return EACCESS
            return ebadf();
        case ERROR_NOT_ENOUGH_QUOTA:
            errno = WSAEDQUOT;
            return -1;
        case ERROR_BROKEN_PIPE:     // broken pipe
        case ERROR_NO_DATA:         // closing named pipe
	        STRACE("broken pipe");
            _Exit(128 + EPIPE);
        default:
            return __winerr();
    }
}

// Based on https://github.com/jart/cosmopolitan/blob/6d36584/libc/calls/write-nt.c#L86-L107
static ssize_t sys_write_nt(int fd, const struct iovec *iov, size_t iovlen,
                            ssize_t opt_offset) {
    ssize_t rc;
    size_t i, total;
    if (opt_offset < -1) return einval();
    while (iovlen && !iov[0].iov_len) iov++, iovlen--;
    if (iovlen) {
        for (total = i = 0; i < iovlen; ++i) {
            if (!iov[i].iov_len) continue;
            rc = sys_write_nt_impl(fd, iov[i].iov_base, iov[i].iov_len, opt_offset);
            if (rc == -1) return -1;
            total += rc;
            if (opt_offset != -1) opt_offset += rc;
            if (rc < iov[i].iov_len) break;
        }
        return total;
    } else {
        return sys_write_nt_impl(fd, NULL, 0, opt_offset);
    }
}

// Based on https://github.com/jart/cosmopolitan/blob/6d36584/libc/calls/writev-nt.c#L25-L35
static ssize_t sys_writev_nt(int fd, const struct iovec *iov, int iovlen) {  
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    switch (GetFileType(handle)) {
        case FILE_TYPE_DISK:
        case FILE_TYPE_CHAR:
            return sys_write_nt(fd, iov, iovlen, -1);
        // TODO: Check if socket fds are used in blink
        //case FILE_TYPE_PIPE:
            //return sys_send_nt(fd, iov, iovlen, 0);
        default:
        return ebadf();
    }
}

// Based on https://github.com/jart/cosmopolitan/blob/6d36584/libc/calls/writev.c#L54-L87
ssize_t writev(int fd, const struct iovec *iov, int iovlen) {
    ssize_t rc;
    errno = 0;

    if (fd >= 0 && iovlen >= 0) {
        // The cosmo version of this code checks if fd < fds count but blink,
        // doesn't appear to keep track of real fds since the host os can do
        // it but windows of course can't. Its possible that a open handle
        // count would match but I am not sure. Its also possible that gnulib
        // might polyfill a method based on a posix one but I can only find
        // info on a brute force method and a definitely unavailable /proc
        // one. https://stackoverflow.com/questions/7976769
        rc = sys_writev_nt(fd, iov, iovlen);
    } else if (fd < 0) {
        rc = ebadf();
    } else {
        rc = einval();
    }

	STRACE("writev(%d, ioctl, %d) -> %'ld %m", fd, iovlen, rc);
    return rc;
}
