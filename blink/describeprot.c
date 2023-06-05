/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2023 Justine Alexandra Roberts Tunney                              │
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
#include <stdlib.h>
#ifndef __MINGW64_VERSION_MAJOR
#include <string.h>
#include <sys/mman.h>

#include "blink/debug.h"
#include "blink/thread.h"
#include "blink/util.h"

const char *DescribeProt(int prot) {
  char *p;
  bool gotsome;
  _Thread_local static char buf[64];
  if (!prot) return "PROT_NONE";
  p = buf;
  gotsome = false;
  if (prot & PROT_READ) {
    if (gotsome) *p++ = '|';
    p = stpcpy(p, "PROT_READ");
    prot &= ~PROT_READ;
    gotsome = true;
  }
  if (prot & PROT_WRITE) {
    if (gotsome) *p++ = '|';
    p = stpcpy(p, "PROT_WRITE");
    prot &= ~PROT_WRITE;
    gotsome = true;
  }
  if (prot & PROT_EXEC) {
    if (gotsome) *p++ = '|';
    p = stpcpy(p, "PROT_EXEC");
    prot &= ~PROT_EXEC;
    gotsome = true;
  }
  if (prot) {
    if (gotsome) *p++ = '|';
    p = FormatInt64(p, prot);
  }
  return buf;
}
#endif