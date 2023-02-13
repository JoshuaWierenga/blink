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
#include <uchar.h>
#include <windef.h>
#include <winbase.h>
#include <wincon.h>

#include "blink/errno.h"
#include "blink/macros.h"
#include "blink/windows/ioctl.h"
#include "blink/windows/cosmo/libc/calls/syscall_support-nt.internal.h"
#include "blink/windows/cosmo/libc/sysv/errfuns.h"

#define EQUAL(X, Y) ((X) == (Y))
#define CTRL(x)  ((x) ^ 0100)

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl_tiocgwinsz-nt.c#L36-L84
static int ioctl_tiocgwinsz_nt(int fd, struct winsize *ws) {
    int i, e, rc;
    DWORD mode;
    HANDLE handles[3];
    STARTUPINFO startinfo;
    CONSOLE_SCREEN_BUFFER_INFOEX sbinfo;
    rc = -1;
    e = errno;
    if (ws) {
        handles[0] = (HANDLE)_get_osfhandle(fd);
        GetStartupInfo(&startinfo);
        // Is this required? ioctl_tcgets_nt for example just hardcodes the indices.
        if (startinfo.dwFlags & STARTF_USESTDHANDLES) {
            handles[1] = startinfo.hStdOutput;
            handles[2] = startinfo.hStdInput;
        } else {
            __winerr();
        }
        for (i = 0; i < ARRAYLEN(handles); ++i) {
            if (GetFileType(handles[i]) == FILE_TYPE_DISK 
                || GetFileType(handles[i]) == FILE_TYPE_CHAR) {
                if (GetConsoleMode(handles[i], &mode)) {
                    memset(&sbinfo, 0, sizeof(sbinfo));
                    sbinfo.cbSize = sizeof(sbinfo);
                    if (GetConsoleScreenBufferInfoEx(handles[i], &sbinfo)) {
                        ws->ws_col = sbinfo.srWindow.Right - sbinfo.srWindow.Left + 1;
                        ws->ws_row = sbinfo.srWindow.Bottom - sbinfo.srWindow.Top + 1;
                        ws->ws_xpixel = 0;
                        ws->ws_ypixel = 0;
                        errno = e;
                        rc = 0;
                        break;
                    } else if (startinfo.dwFlags & STARTF_USECOUNTCHARS) {
                        ws->ws_col = startinfo.dwXCountChars;
                        ws->ws_row = startinfo.dwYCountChars;
                        ws->ws_xpixel = 0;
                        ws->ws_ypixel = 0;
                        errno = e;
                        rc = 0;
                        break;
                    } else {
                        __winerr();
                    }
                } else {
                    enotty();
                }
            } else {
                ebadf();
            }
        }
    } else {
        efault();
    }
    return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl_tiocgwinsz.c#L35-L57
static int ioctl_tiocgwinsz(int fd, ...) {
    int rc;
    va_list va;
    struct winsize *ws;
    va_start(va, fd);
    ws = va_arg(va, struct winsize *);
    va_end(va);
    if (fd >= 0) {
        rc = ioctl_tiocgwinsz_nt(fd, ws);
    } else {
        rc = einval();
    }
    STRACE("%s(%d) -> %d %m", "ioctl_tiocgwinsz", fd, rc);
    return rc;
}


// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl_tcgets-nt.c#L31-L84
static int ioctl_tcgets_nt(int fd, struct termios *tio) {
    HANDLE in, out;
    BOOL inok, outok;
    DWORD inmode, outmode;
    // Should any of this use fd?
    inok = GetConsoleMode((in = (HANDLE)_get_osfhandle(0)), &inmode);
    outok = GetConsoleMode((out = (HANDLE)_get_osfhandle(1)), &outmode);
    if (inok | outok) {
        memset(tio, 0, sizeof(*tio));

        tio->c_cflag |= CS8;

        tio->c_cc[VINTR] = CTRL('C');
        tio->c_cc[VQUIT] = CTRL('\\');
        tio->c_cc[VERASE] = CTRL('?');
        tio->c_cc[VKILL] = CTRL('U');
        tio->c_cc[VEOF] = CTRL('D');
        tio->c_cc[VMIN] = CTRL('A');
        tio->c_cc[VSTART] = CTRL('Q');
        tio->c_cc[VSTOP] = CTRL('S');
        tio->c_cc[VSUSP] = CTRL('Z');
        tio->c_cc[VREPRINT] = CTRL('R');
        tio->c_cc[VDISCARD] = CTRL('O');
        tio->c_cc[VWERASE] = CTRL('W');
        tio->c_cc[VLNEXT] = CTRL('V');

        if (inok) {
          if (inmode & ENABLE_LINE_INPUT) {
            tio->c_lflag |= ICANON;
          }
          if (inmode & ENABLE_ECHO_INPUT) {
            tio->c_lflag |= ECHO;
          }
          if (inmode & ENABLE_PROCESSED_INPUT) {
            tio->c_lflag |= IEXTEN | ISIG;
            if (tio->c_lflag | ECHO) {
              tio->c_lflag |= ECHOE;
            }
          }
        }

        if (outok) {
          if (outmode & ENABLE_PROCESSED_INPUT) {
            tio->c_oflag |= OPOST;
          }
          if (!(outmode & DISABLE_NEWLINE_AUTO_RETURN)) {
            tio->c_oflag |= OPOST | ONLCR;
          }
        }

        return 0;
    } else {
        return enotty();
    }
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl_tcgets.c#L70-L94
static int ioctl_tcgets(int fd, ...) {
    int rc;
    va_list va;
    struct termios *tio;
    va_start(va, fd);
    tio = va_arg(va, struct termios *);
    va_end(va);
    if (fd >= 0) {
        if (!tio) {
            rc = efault();
        } else {
            rc = ioctl_tcgets_nt(fd, tio);
        }
    } else {
        rc = einval();
    }
    STRACE("ioctl_tcgets(%d, %p) -> %d %m", fd, tio, rc);
    return rc;
}


// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl_tcsets-nt.c#L32-L87
static int ioctl_tcsets_nt(int ignored, uint64_t request,
    const struct termios *tio) {
    HANDLE in, out;
    BOOL ok, inok, outok;
    DWORD inmode, outmode;
    // Should any of this use fd?
    inok = GetConsoleMode((in = (HANDLE)_get_osfhandle(0)), &inmode);
    outok = GetConsoleMode((out = (HANDLE)_get_osfhandle(1)), &outmode);
    if (inok | outok) {

        if (inok) {
            if (request == TCSETSF) {
                FlushConsoleInputBuffer(in);
            }
            inmode &=
                ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
            inmode |= ENABLE_WINDOW_INPUT;
            if (tio->c_lflag & ICANON) {
                inmode |= ENABLE_LINE_INPUT;
            }
            if (tio->c_lflag & ECHO) {
                /*
                 * kNtEnableEchoInput can be used only if the ENABLE_LINE_INPUT mode
                 * is also enabled. --Quoth MSDN
                 */
                inmode |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT;
            }
            if (tio->c_lflag & (IEXTEN | ISIG)) {
                inmode |= ENABLE_PROCESSED_INPUT;
            }
            // TODO Add IsAtLeastWindows10, can use peb but first need to check if 32 or 64 bit
            /*if (IsAtLeastWindows10()) {
                inmode |= kNtEnableVirtualTerminalInput;
            }*/
            ok = SetConsoleMode(in, inmode);
            STRACE("SetConsoleMode(%p, 0x%X) -> %hhhd", in, inmode, ok);
        }

        if (outok) {
            outmode &= ~(DISABLE_NEWLINE_AUTO_RETURN);
            outmode |= ENABLE_PROCESSED_INPUT;
            if (!(tio->c_oflag & ONLCR)) {
                outmode |= DISABLE_NEWLINE_AUTO_RETURN;
            }
            // TODO Add IsAtLeastWindows10, can use peb but first need to check if 32 or 64 bit
            /*if (IsAtLeastWindows10()) {
                outmode |= kNtEnableVirtualTerminalProcessing;
            }*/
            ok = SetConsoleMode(out, outmode);
            STRACE("SetConsoleMode(%p, 0x%X) -> %hhhd", out, outmode, ok);
        }

        return 0;
    } else {
        return enotty();
    }
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl_tcsets.c#L54-L85
static int ioctl_tcsets(int fd, uint64_t request, ...) {
    int rc;
    va_list va;
    const struct termios *tio;
    va_start(va, request);
    tio = va_arg(va, const struct termios *);
    va_end(va);
    // TODO Port over terminal restore code?
    if (!tio) {
        rc = efault();
    } else if (fd >= 0) {
        rc = ioctl_tcsets_nt(fd, request, tio);
    } else {
        rc = einval();
    }
    STRACE("ioctl_tcsets(%d, %p, %p) -> %d %m", fd, request, tio, rc);
    return rc;
}


// TODO Replace with macro version from cosmo ioctl.h
// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl.c#L31-L38
int ioctl(int fd, uint64_t request, ...) {
    void *arg;
    va_list va;
    va_start(va, request);
    arg = va_arg(va, void *);
    va_end(va);
    // Need to clear this to prevent the existing value getting through if
    // everything works.
    errno = 0;
    return __IOCTL_DISPATCH(EQUAL, -1, fd, request, arg);
}
