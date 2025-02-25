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
#include "blink/windows.h"

#include <stdlib.h>
#include <string.h>
#ifdef WINBLINK
#include <direct.h>
#else
#include <unistd.h>
#define strcpy_s(d, l, s) strcpy(d, s)
#endif

#include "blink/util.h"

static char *g_startdir;

static void FreeStartDir(void) {
  free(g_startdir);
  g_startdir = 0;
}

char *GetStartDir(void) {
  if (!g_startdir) {
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) strcpy_s(cwd, PATH_MAX, ".");
    g_startdir = strdup(cwd);
    atexit(FreeStartDir);
  }
  return g_startdir;
}
