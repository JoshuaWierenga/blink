/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
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
#include <minwindef.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#include "blink/windows/cosmo/libc/calls/struct/stat.internal.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/intrin/describestat.c

#ifdef DescribeStat
#undef DescribeStat
#endif

#define N 300


//Safely is not relevant, that is mingw's stdlib(mostly window's stdlib of course) job.
#define append(...) o += /*k*/snprintf(buf + o, N - o, __VA_ARGS__)

const char *DescribeStat(char buf[N], int rc, const struct stat *st) {
  int o = 0;

  if (rc == -1) return "n/a";
  if (!st) return "NULL";
  // Again not relevant.
  /*if (kisdangerous(st)) {
    ksnprintf(buf, N, "%p", st);
    return buf;
  }*/

  append("{.st_%s=%'lld", "size", st->st_size);

  if (st->st_blocks) {
    append(", .st_blocks=%'lld/512", st->st_blocks * (uint64_t)512);
  }

  if (st->st_mode) {
    append(", .st_%s=%#o", "mode", st->st_mode);
  }

  if (st->st_nlink != 1) {
    append(", .st_%s=%'llu", "nlink", st->st_nlink);
  }

  if (st->st_uid) {
    append(", .st_%s=%u", "uid", st->st_uid);
  }

  if (st->st_gid) {
    append(", .st_%s=%u", "gid", st->st_gid);
  }

  if (st->st_dev) {
    append(", .st_%s=%llu", "dev", st->st_dev);
  }

  if (st->st_ino) {
    append(", .st_%s=%llu", "ino", st->st_ino);
  }

  if (st->st_gen) {
    append(", .st_%s=%'llu", "gen", st->st_gen);
  }

  if (st->st_flags) {
    append(", .st_%s=%x", "flags", st->st_flags);
  }

  if (st->st_rdev) {
    append(", .st_%s=%'llu", "rdev", st->st_rdev);
  }

  if (st->st_blksize != PAGESIZE) {
    append(", .st_%s=%'lld", "blksize", st->st_blksize);
  }

  append("}");

  return buf;
}
