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
#include <dirent.h>
#include <inttypes.h>
#include <io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <windef.h>
#include <winbase.h>

#include "blink/errno.h"
#include "blink/windows/cosmo/libc/calls/syscall_support-nt.internal.h"
#include "blink/windows/cosmo/libc/str/str.h"

static long enametoolong() {
  errno = 36; // From linux errno list
  return -1;
}

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/sysv/consts/dt.h

#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK     10

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/stdio/dirstream.c

/**
 * Directory stream object.
 */
struct dirstream {
  HANDLE fd;
  int64_t tell;
  struct dirent ent;
  struct {
    bool isdone;
    wchar_t *name;
    WIN32_FIND_DATAW windata;
  };
};

static WDIR *opendir_nt_impl(wchar_t *name, size_t len) {
  WDIR *res;
  if (len + 2 + 1 <= PATH_MAX) {
    if (len == 1 && name[0] == '.') {
      name[0] = '*';
    } else {
      if (len > 1 && name[len - 1] != u'\\') {
        name[len++] = u'\\';
      }
      name[len++] = u'*';
    }
    name[len] = u'\0';
    if ((res = calloc(1, sizeof(WDIR)))) {
      if ((res->fd = FindFirstFileW(name, &res->windata)) != (HANDLE)-1) {
        return res;
      }
      __fix_enotdir(-1, name);
      free(res);
    }
  } else {
    enametoolong();
  }
  return NULL;
}

static WDIR *fdopendir_nt(int fd) {
  WDIR *res;
  wchar_t *name;
  HANDLE handle;
  handle = (HANDLE)_get_osfhandle(fd);
  if (GetFileType(handle) == FILE_TYPE_DISK) {
    if ((name = malloc(PATH_MAX * sizeof(*name)))) {
      if ((res = opendir_nt_impl(
               name, GetFinalPathNameByHandleW(
                         handle, name, PATH_MAX,
                         FILE_NAME_NORMALIZED | VOLUME_NAME_DOS)))) {
        res->name = name;
        _close(fd);
        return res;
      }
      free(name);
    }
  } else {
    ebadf();
  }
  return NULL;
}

static uint8_t GetNtDirentType(WIN32_FIND_DATAW *w) {
  // Not a fan of this but microsoft deprecated the final three fields
  // and so mingw-w64 dropped them from the struct.
  switch (*(DWORD*)(w + sizeof(WIN32_FIND_DATAW))) {
    case FILE_TYPE_DISK:
      return DT_BLK;
    case FILE_TYPE_CHAR:
      return DT_CHR;
    case FILE_TYPE_PIPE:
      return DT_FIFO;
    default:
      if (w->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return DT_DIR;
      } else if (w->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        return DT_LNK;
      } else {
        return DT_REG;
      }
  }
}

static struct dirent *readdir_nt(WDIR *dir) {
  size_t i;
  if (!dir->isdone) {
    memset(&dir->ent, 0, sizeof(dir->ent));
    dir->ent.d_ino++;
    dir->ent.d_off = dir->tell++;
    dir->ent.d_reclen =
        tprecode16to8(dir->ent.d_name, sizeof(dir->ent.d_name) - 2,
                      dir->windata.cFileName)
            .ax;
    for (i = 0; i < dir->ent.d_reclen; ++i) {
      if (dir->ent.d_name[i] == '\\') {
        dir->ent.d_name[i] = '/';
      }
    }
    dir->ent.d_type = GetNtDirentType(&dir->windata);
    dir->isdone = !FindNextFileW(dir->fd, &dir->windata);
    return &dir->ent;
  } else {
    return NULL;
  }
}

/**
 * Creates directory object for file descriptor.
 *
 * @param fd gets owned by this function, if it succeeds
 * @return new directory object, which must be freed by closedir(),
 *     or NULL w/ errno
 * @errors ENOMEM and fd is closed
 */
WDIR *fdopendirw(int fd) {
  WDIR *dir;
  dir = fdopendir_nt(fd);
  return dir;
}

/**
 * Reads next entry from directory stream.
 *
 * This API doesn't define any particular ordering.
 *
 * @param dir is the object opendir() or fdopendir() returned
 * @return next entry or NULL on end or error, which can be
 *     differentiated by setting errno to 0 beforehand
 */
struct dirent *readdirw(WDIR *dir) {
  struct dirent *e;
  e = readdir_nt(dir);
  return e;
}
