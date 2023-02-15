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
//#include <sys/uio.h>

#include "blink/windows/macros.h"
#include WINDOWSGNULIBHEADER(poll.h)

#include "blink/ioports.h"
#include "blink/uart.h"

static int OpE9Read(struct Machine *m) {
  int fd;
  uint8_t b;
  fd = 0;
  if (fd >= m->fds.i) return -1;
  if (!m->fds.p[fd].cb) return -1;
  if (m->fds.p[fd].cb->readv(m->fds.p[fd].fd, &(struct iovec){&b, 1}, 1) == 1) {
    return b;
  } else {
    return -1;
  }
}

static void OpE9Write(struct Machine *m, uint8_t b) {
  int fd;
  fd = 1;
  if (fd >= m->fds.i) return;
  if (!m->fds.p[fd].cb) return;
  m->fds.p[fd].cb->writev(m->fds.p[fd].fd, &(struct iovec){&b, 1}, 1);
}

static int OpE9Poll(struct Machine *m) {
  int rc, fd = 0;
  struct pollfd pf;
  if (fd >= m->fds.i) return -1;
  if (!m->fds.p[fd].cb) return -1;
  pf.fd = m->fds.p[fd].fd;
  pf.events = POLLIN | POLLOUT;
  rc = m->fds.p[fd].cb->poll(&pf, 1, 20);
  if (rc <= 0) return rc;
  return pf.revents;
}

static int OpSerialIn(struct Machine *m, int r) {
  int p, s;
  switch (r) {
    case UART_DLL:
      if (!m->dlab) {
        return OpE9Read(m);
      } else {
        return 0x01;
      }
    case UART_LSR:
      if ((p = OpE9Poll(m)) == -1) return -1;
      s = UART_TTYIDL;
      if (p & POLLIN) s |= UART_TTYDA;
      if (p & POLLOUT) s |= UART_TTYTXR;
      return s;
    default:
      return 0;
  }
}

static void OpSerialOut(struct Machine *m, int r, uint32_t x) {
  switch (r) {
    case UART_DLL:
      if (!m->dlab) {
        return OpE9Write(m, x);
      }
      break;
    case UART_LCR:
      m->dlab = !!(x & UART_DLAB);
      break;
    default:
      break;
  }
}

uint64_t OpIn(struct Machine *m, uint16_t p) {
  switch (p) {
    case 0xE9:
      return OpE9Read(m);
    case 0x3F8 ... 0x3FF:
      return OpSerialIn(m, p - 0x3F8);
    default:
      return -1;
  }
}

void OpOut(struct Machine *m, uint16_t p, uint32_t x) {
  switch (p) {
    case 0xE9:
      OpE9Write(m, x);
      break;
    case 0x3F8 ... 0x3FF:
      OpSerialOut(m, p - 0x3F8, x);
      break;
    default:
      break;
  }
}
