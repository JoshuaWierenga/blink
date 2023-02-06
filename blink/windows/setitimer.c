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
#include <math.h>
#include <stdbool.h>

#include "blink/errno.h"
#include "blink/windows/macros.h"
#include "blink/windows/setitimer.h"

static bool __hastimer;
static bool __singleshot;
static long double __lastalrm;
static long double __interval;

// Copied from tui.c
static long double nowl(void) {
  long double secs;
  struct timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);
  secs = tv.tv_nsec;
  secs *= 1 / 1e9;
  secs += tv.tv_sec;
  return secs;
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874ceb9c9b826bc98e1f20f7b3b0cf63462bf/libc/calls/setitimer-nt.c#L80-L132
static int sys_setitimer_nt(int which, const struct itimerval *newvalue,
                                 struct itimerval *out_opt_oldvalue) {
    long double elapsed, untilnext;

    if (which != ITIMER_REAL ||
        (newvalue && (!(0 <= newvalue->it_value.tv_usec &&
            newvalue->it_value.tv_usec < 1000000) ||
            !(0 <= newvalue->it_interval.tv_usec &&
            newvalue->it_interval.tv_usec < 1000000)))) {
        return einval();
    }

    if (out_opt_oldvalue) {
        if (__hastimer) {
            elapsed = nowl() - __lastalrm;
            if (elapsed > __interval) {
                untilnext = 0;
            } else {
                untilnext = __interval - elapsed;
            }
            out_opt_oldvalue->it_interval.tv_sec = __interval;
            out_opt_oldvalue->it_interval.tv_usec = 1 / 1e6 * fmodl(__interval, 1);
            out_opt_oldvalue->it_value.tv_sec = untilnext;
            out_opt_oldvalue->it_value.tv_usec = 1 / 1e6 * fmodl(untilnext, 1);
        } else {
            out_opt_oldvalue->it_interval.tv_sec = 0;
            out_opt_oldvalue->it_interval.tv_usec = 0;
            out_opt_oldvalue->it_value.tv_sec = 0;
            out_opt_oldvalue->it_value.tv_usec = 0;
        }
    }

    if (newvalue) {
        if (newvalue->it_interval.tv_sec || newvalue->it_interval.tv_usec ||
            newvalue->it_value.tv_sec || newvalue->it_value.tv_usec) {
                __hastimer = true;
                if (newvalue->it_interval.tv_sec || newvalue->it_interval.tv_usec) {
                    __singleshot = false;
                    __interval = newvalue->it_interval.tv_sec +
                                 1 / 1e6 * newvalue->it_interval.tv_usec;
                } else {
                    __singleshot = true;
                    __interval =
                        newvalue->it_value.tv_sec + 1 / 
                            1e6 * newvalue->it_value.tv_usec;
                }
                __lastalrm = nowl();
            } else {
                __hastimer = false;
            }
        }

    return 0;
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/setitimer.c#L66-L107
int setitimer(int which, const struct itimerval *newvalue,
              struct itimerval *oldvalue) {
    int rc;

    rc = sys_setitimer_nt(which, newvalue, oldvalue);

#if 1
  if (newvalue && oldvalue) {
    STRACE("setitimer(%d, "
           "{{%'ld, %'ld}, {%'ld, %'ld}}, "
           "[{{%'ld, %'ld}, {%'ld, %'ld}}]) -> %d %m",
           which, newvalue->it_interval.tv_sec, newvalue->it_interval.tv_usec,
           newvalue->it_value.tv_sec, newvalue->it_value.tv_usec,
           oldvalue->it_interval.tv_sec, oldvalue->it_interval.tv_usec,
           oldvalue->it_value.tv_sec, oldvalue->it_value.tv_usec, rc);
  } else if (newvalue) {
    STRACE("setitimer(%d, {{%'ld, %'ld}, {%'ld, %'ld}}, NULL) -> %d %m", which,
           newvalue->it_interval.tv_sec, newvalue->it_interval.tv_usec,
           newvalue->it_value.tv_sec, newvalue->it_value.tv_usec, rc);
  } else if (oldvalue) {
    STRACE("setitimer(%d, NULL, [{{%'ld, %'ld}, {%'ld, %'ld}}]) -> %d %m", which,
           oldvalue->it_interval.tv_sec, oldvalue->it_interval.tv_usec,
           oldvalue->it_value.tv_sec, oldvalue->it_value.tv_usec, rc);
  } else {
    STRACE("setitimer(%d, NULL, NULL) -> %d %m", which, rc);
  }
#endif

  return rc;
}
