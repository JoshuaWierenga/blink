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
#include <stdlib.h>
#include <string.h>

#include "blink/lines.h"
#include "blink/windows/strndup.h"

struct Lines *NewLines(void) {
  return calloc(1, sizeof(struct Lines));
}

void FreeLines(struct Lines *lines) {
  size_t i;
  for (i = 0; i < lines->n; ++i) {
    free(lines->p[i]);
  }
  free(lines);
}

void AppendLine(struct Lines *lines, const char *s, unsigned n) {
  lines->p = realloc(lines->p, ++lines->n * sizeof(*lines->p));
  lines->p[lines->n - 1] = strndup(s, n);
}

void AppendLines(struct Lines *lines, const char *s) {
  const char *p;
  for (;;) {
    p = strchr(s, '\n');
    if (p) {
      AppendLine(lines, s, p - s);
      s = p + 1;
    } else {
      if (*s) {
        AppendLine(lines, s, -1);
      }
      break;
    }
  }
}
