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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "blink/assert.h"
#include "blink/log.h"
#include "blink/thread.h"
#include "blink/tunables.h"

// TODO: Decide on if to use windows's limited fd support
#ifdef _WIN32
static HANDLE g_errh = INVALID_HANDLE_VALUE;
#else
static int g_errfd;
#endif

int WriteErrorString(const char *buf) {
#ifdef _WIN32
  return WriteError(INVALID_HANDLE_VALUE, buf, (DWORD)strlen(buf));
#else
  return WriteError(0, buf, strlen(buf));
#endif
}

#ifdef _WIN32
int WriteError(HANDLE h, const char *buf, DWORD len) {
#else
int WriteError(int fd, const char *buf, int len) {
#endif
  int rc, cs;
#ifdef HAVE_PTHREAD_SETCANCELSTATE
  unassert(!pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs));
#endif
#ifdef _WIN32
  // TODO: Are there any reasons this could fail for which its worth trying again?
  // TODO: Handle unicode
  rc = WriteConsole(h != INVALID_HANDLE_VALUE ? h : g_errh, buf, len, NULL, NULL);
#else
  do rc = write(fd > 0 ? fd : g_errfd, buf, len);
  while (rc == -1 && errno == EINTR);
#endif
#ifdef HAVE_PTHREAD_SETCANCELSTATE
      unassert(!pthread_setcancelstate(cs, 0));
#endif
  return rc;
}

void WriteErrorInit(void) {
#ifdef _WIN32
  if (g_errh != INVALID_HANDLE_VALUE) return;
  if (DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_ERROR_HANDLE),
                      GetCurrentProcess(), &g_errh, 0, FALSE,
                      DUPLICATE_SAME_ACCESS) == FALSE) {
    exit(200);
  }
#else
  if (g_errfd) return;
  g_errfd = fcntl(2, F_DUPFD_CLOEXEC, kMinBlinkFd);
  if (g_errfd == -1) exit(200);
#endif
}
