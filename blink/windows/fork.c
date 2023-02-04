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
/*#include <stdint.h>
#include <processthreadsapi.h>

#include "third_party/gnulib_build/config.h"
#include "third_party/gnulib_build/lib/signal.h"

#include "blink/windows/fork.h"
#include "blink/windows/macros.h"

sigset_t _sigsetmask(sigset_t neu) {
    sigset_t res;
    __sig_mask(SIG_SETMASK, &neu, &res);
    return res;
}

sigset_t _sigblockall(void) {
    sigset_t ss;
    memset(&ss, -1, sizeof(ss));
    return _sigsetmask(ss);
}

#define BLOCK_SIGNALS               \
    do {                            \
        sigset_t _SigMask;          \
        _SigMask = _sigblockall()
	
#define ALLOW_SIGNALS               \
        _sigsetmask(_SigMask);      \
    }                               \
    while (0)
	

static int _fork() {
    int ax, dx, parent;
    BLOCK_SIGNALS;
    ax = sys_fork_nt();
    if (!ax) {
        dx = GetCurrentProcessId();
        parent = __pid;
        __pid = dx;
        STRACE("fork() -> 0 (child of %d)", parent);
    } else {
        STRACE("fork() -> %d %m", ax);
    }
    ALLOW_SIGNALS;
    return ax;
}

int fork(void) {
  return _fork(0);
}*/
