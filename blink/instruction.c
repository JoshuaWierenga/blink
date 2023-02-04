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
#include <string.h>

#include "blink/address.h"
#include "blink/bitscan.h"
#include "blink/endian.h"
#include "blink/machine.h"
#include "blink/macros.h"
#include "blink/memory.h"
#include "blink/modrm.h"
#include "blink/throw.h"
#include "blink/x86.h"

static bool IsOpcodeEqual(struct XedDecodedInst *xedd, uint8_t *a) {
  uint64_t w;
  unsigned n;
  if ((n = xedd->length)) {
    if (n <= 7) {
      w = Read64(a) ^ Read64(xedd->bytes);
      return !w || (bsf(w) >> 3) >= n;
    } else {
      return !memcmp(a, xedd->bytes, n);
    }
  } else {
    return false;
  }
}

static void ReadInstruction(struct Machine *m, uint8_t *p, unsigned n) {
  struct XedDecodedInst xedd[1];
  InitializeInstruction(xedd, m->mode);
  if (!DecodeInstruction(xedd, p, n)) {
    memcpy(m->xedd, xedd, sizeof(m->icache[0]));
  } else {
    HaltMachine(m, kMachineDecodeError);
  }
}

static void LoadInstructionSlow(struct Machine *m, uint64_t ip) {
  unsigned i;
  uint8_t *addr;
  uint8_t copy[15], *toil;
  i = 0x1000 - (ip & 0xfff);
  addr = ResolveAddress(m, ip);
  if ((toil = FindReal(m, ip + i))) {
    memcpy(copy, addr, i);
    memcpy(copy + i, toil, 15 - i);
    ReadInstruction(m, copy, 15);
  } else {
    ReadInstruction(m, addr, i);
  }
}

void LoadInstruction(struct Machine *m) {
  uint64_t ip;
  unsigned key;
  uint8_t *addr;
  ip = Read64(m->cs) + MaskAddress(m->mode & 3, m->ip);
  key = ip & (ARRAYLEN(m->icache) - 1);
  m->xedd = (struct XedDecodedInst *)m->icache[key];
  if ((ip & 0xfff) < 0x1000 - 15) {
    if (ip - (ip & 0xfff) == m->codevirt && m->codehost) {
      addr = m->codehost + (ip & 0xfff);
    } else {
      m->codevirt = ip - (ip & 0xfff);
      m->codehost = ResolveAddress(m, m->codevirt);
      addr = m->codehost + (ip & 0xfff);
    }
    if (!IsOpcodeEqual(m->xedd, addr)) {
      ReadInstruction(m, addr, 15);
    }
  } else {
    LoadInstructionSlow(m, ip);
  }
}
