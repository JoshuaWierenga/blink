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

#include "third_party/gnulib_build/config.h"
#include "third_party/gnulib_build/lib/signal.h"
#include "third_party/gnulib_build/lib/sys/resource.h"

#include "blink/windows/macros.h"
#include "blink/windows/wait4.h"

static int __sig_mask(int how, const sigset_t *neu, sigset_t *old) {
    uint64_t x, y, *mask;
    if (how == SIG_BLOCK || how == SIG_UNBLOCK || how == SIG_SETMASK) {
        mask = &__sig.sigmask;
        if (old) {
            old->__bits[0] = *mask;
            old->__bits[1] = 0;
        }
        if (neu) {
            x = *mask;
            y = neu->__bits[0];
            if (how == SIG_BLOCK) {
                x |= y;
            } else if (how == SIG_UNBLOCK) {
                x &= ~y;
            } else {
                x = y;
            }
            x &= ~(0
#define M(x) | GetSigBit(x)
#include "libc/intrin/sigisprecious.inc"
            );
            *mask = x;
        }
        return 0;
    } else {
        return einval();
    }
}

static int sys_wait4_nt(int pid, int *opt_out_wstatus, int options,
                        struct rusage *opt_out_rusage) {
    int rc;
    sigset_t oldmask, mask = {0};
    sigaddset(&mask, SIGCHLD);
    __sig_mask(SIG_BLOCK, &mask, &oldmask);
    rc = sys_wait4_nt_impl(pid, opt_out_wstatus, options, opt_out_rusage);
    __sig_mask(SIG_SETMASK, &oldmask, 0);
    return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/wait4.c#L42-L64
int wait4(int pid, int *opt_out_wstatus, int options,
          struct rusage *opt_out_rusage) {
    int rc, ws = 0;

    rc = sys_wait4_nt(pid, &ws, options, opt_out_rusage);
    if (rc != -1 && opt_out_wstatus) *opt_out_wstatus = ws;

    STRACE("wait4(%d, [%#x], %d, %p) → %d% m", pid, ws, options, opt_out_rusage,
           rc);
    return rc;
}*/
