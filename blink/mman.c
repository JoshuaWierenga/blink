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
#include <stdbool.h>

#include "blink/assert.h"
#include "blink/bitscan.h"
#include "blink/errno.h"
#include "blink/likely.h"
#include "blink/macros.h"
#include "blink/map.h"
#include "blink/thread.h"
#include "blink/win.h"

#ifdef __MINGW64_VERSION_MAJOR
#include <handleapi.h>
#include <io.h>
#include <libloaderapi.h>
#include <memoryapi.h>
#include <ntstatus.h>
#include <psapi.h>
#include <processthreadsapi.h>
#include <winternl.h>

// NtQueryInformationFile is not exported so needs to be manually accessed
typedef NTSTATUS (NTAPI* PNtQueryInformationFile)(HANDLE, PIO_STATUS_BLOCK, PVOID,
                                                  ULONG, FILE_INFORMATION_CLASS);

NTSTATUS NtQueryInformationFile(HANDLE handle, PIO_STATUS_BLOCK io, PVOID ptr,
                                 ULONG length, FILE_INFORMATION_CLASS informationClass) {
  HMODULE ntdllModule = GetModuleHandle(TEXT("ntdll.dll"));
  PNtQueryInformationFile pNtQueryInformationFile =
    (PNtQueryInformationFile)GetProcAddress(ntdllModule, "NtQueryInformationFile");
  // Supported in all windows nt versions so should not fail but check anyway
  if (NULL != pNtQueryInformationFile) {
    pNtQueryInformationFile(handle, io, ptr, length, informationClass);
  } else {
    exit(1);
  }
}

// Attempt at getting start and end point of the binary
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

// Hardcoded currently but guest setrlimit can set this
#define __virtualmax -1

// Based on https://github.com/jart/cosmopolitan/blob/b94b29d/libc/intrin/mmi_lock.c
static struct MemoryIntervals _mmi;
static pthread_mutex_t_ __mmi_lock_obj;

static void(__mmi_lock)(void) {
  LOCK(&__mmi_lock_obj);
}

static void(__mmi_unlock)(void) {
  UNLOCK(&__mmi_lock_obj);
}

// Based on https://github.com/jart/cosmopolitan/blob/b94b29d/libc/intrin/directmap.internal.h#L11-L14
struct DirectMap {
  void *addr;
  HANDLE maphandle;
};

// From https://github.com/jart/cosmopolitan/blob/b94b29d/libc/integral/normalize.inc
#define FRAMESIZE   0x10000
#define GUARDSIZE   0x4000
#define OPEN_MAX    16

// From https://github.com/jart/cosmopolitan/blob/b6182db/libc/dce.h#L63-L67
// TODO: Can if asan work as is or does it require lots of other asan code
// from cosmo, if so its likely easier to just remove asan code from here
#ifdef __SANITIZE_ADDRESS__
#define IsAsan() 1
#else
#define IsAsan() 0
#endif

// From https://github.com/jart/cosmopolitan/blob/fea68b1/libc/runtime/memtrack.internal.h
#define kAutomapStart       _kMemVista(0x100080040000, 0x010080040000)
#define kAutomapSize        (kMemtrackStart - kAutomapStart)
#define kMemtrackStart      _kMemVista(0x1fe7fffc0000, 0x01e7fffc0000)
#define _kMemVista(NORMAL, WINVISTA) (IsAtLeastWindows8p1() ? NORMAL : WINVISTA)

// Based on https://github.com/jart/cosmopolitan/blob/b94b29d7/libc/runtime/memtrack.internal.h
#define kMemtrackGran       (!IsAsan() ? FRAMESIZE : FRAMESIZE * 8)

struct MemoryInterval {
  int x;
  int y;
  HANDLE h;
  long size;
  long offset;
  int flags;
  char prot;
  bool iscow;
  bool readonlyfile;
};

struct MemoryIntervals {
  size_t i, n;
  struct MemoryInterval *p;
  struct MemoryInterval s[OPEN_MAX];
};

#define IsLegalPointer(p) \
  (-0x800000000000 <= (intptr_t)(p) && (intptr_t)(p) <= 0x7fffffffffff)
#define ADDR_32_TO_48(x) (intptr_t)((uint64_t)(int)(x) << 16)

forceinline pureconst bool IsLegalSize(uint64_t n) {
  /* subtract frame size so roundup is safe */
  return n <= 0x800000000000 - FRAMESIZE;
}

forceinline pureconst bool OverlapsImageSpace(const void *p, size_t n) {
  const unsigned char *BegA, *EndA, *BegB, *EndB;
  HMODULE selfModule;
  MODULEINFO selfInfo;
  if (n) {
    BegA = p;
    EndA = BegA + (n - 1);
    selfModule = (HMODULE)&__ImageBase;
    if (!GetModuleInformation(GetCurrentProcess(), selfModule, &selfInfo, sizeof(selfInfo))) {
        return 1;
    }
    BegB = (char*)selfInfo.lpBaseOfDll;
    EndB = BegB + selfInfo.SizeOfImage - 1;
    return MAX(BegA, BegB) < MIN(EndA, EndB);
  } else {
    return 0;
  }
}

forceinline pureconst bool OverlapsArenaSpace(const void *p, size_t n) {
  intptr_t BegA, EndA, BegB, EndB;
  if (n) {
    BegA = (intptr_t)p;
    EndA = BegA + (n - 1);
    BegB = 0x50000000;
    EndB = 0x7ffdffff;
    return MAX(BegA, BegB) < MIN(EndA, EndB);
  } else {
    return 0;
  }
}

forceinline pureconst bool OverlapsShadowSpace(const void *p, size_t n) {
  intptr_t BegA, EndA, BegB, EndB;
  if (n) {
    BegA = (intptr_t)p;
    EndA = BegA + (n - 1);
    BegB = 0x7fff0000;
    EndB = 0x10007fffffff;
    return MAX(BegA, BegB) < MIN(EndA, EndB);
  } else {
    return 0;
  }
}

// From https://github.com/jart/cosmopolitan/blob/b94b29d/libc/runtime/getmemtracksize.c
static noasan size_t GetMemtrackSize(struct MemoryIntervals *mm) {
  size_t i, n;
  for (n = i = 0; i < mm->i; ++i) {
    n += ((size_t)(mm->p[i].y - mm->p[i].x) + 1) << 16;
  }
  return n;
}

// From https://github.com/jart/cosmopolitan/blob/b6182db/libc/intrin/midpoint.h#L28
#define _midpoint(a, b) (((a) & (b)) + ((a) ^ (b)) / 2)

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/intrin/findmemoryinterval.c
static noasan unsigned FindMemoryInterval(const struct MemoryIntervals *mm, int x) {
  unsigned l, m, r;
  l = 0;
  r = mm->i;
  while (l < r) {
    m = _midpoint(l, r);
    if (mm->p[m].y < x) {
      l = m + 1;
    } else {
      r = m;
    }
  }
  unassert(l == mm->i || x <= mm->p[l].y);
  return l;
}

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/intrin/kntisinheritable.greg.c
static SECURITY_ATTRIBUTES kNtIsInheritable = {
    sizeof(SECURITY_ATTRIBUTES),
    NULL,
    true,
};

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/intrin/prot2nt.greg.c
static int __prot2nt(int prot, int iscow) {
  switch (prot & (PROT_READ | PROT_WRITE | PROT_EXEC)) {
    case PROT_READ:
      return PAGE_READONLY;
    case PROT_EXEC:
    case PROT_EXEC | PROT_READ:
      return PAGE_EXECUTE_READ;
    case PROT_WRITE:
    case PROT_READ | PROT_WRITE:
      if (iscow) {
        return PAGE_WRITECOPY;
      } else {
        return PAGE_READWRITE;
      }
    case PROT_WRITE | PROT_EXEC:
    case PROT_READ | PROT_WRITE | PROT_EXEC:
      if (iscow) {
        return PAGE_EXECUTE_WRITECOPY;
      } else {
        return PAGE_EXECUTE_READWRITE;
      }
    default:
      return 0x1; //kNtPageNoaccess
  }
}

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/intrin/directmap-nt.c
// TODO Replace windows's open function so that all handles have GENERIC_EXECUTE set
static struct DirectMap sys_mmap_nt(void *addr, size_t size, int prot,
                                    int flags, int fd, int64_t off) {
  int iscow;
  HANDLE handle;
  DWORD oldprot, flprotect, desiredaccess;
  struct DirectMap dm;
  SECURITY_ATTRIBUTES *sec;
  IO_STATUS_BLOCK fdstatus;
  FILE_ACCESS_INFORMATION fdaccess;
  NTSTATUS fdquerystatus;

  if (flags & MAP_ANONYMOUS_) {
    handle = INVALID_HANDLE_VALUE;
  } else {
    handle = (HANDLE)_get_osfhandle(fd);
  }

  if ((flags & MAP_TYPE) != MAP_SHARED) {
    sec = 0;  // MAP_PRIVATE isn't inherited across fork()
  } else {
    sec = &kNtIsInheritable;  // MAP_SHARED gives us zero-copy fork()
  }

  // nt will whine under many circumstances if we change the execute bit
  // later using mprotect(). the workaround is to always request execute
  // and then virtualprotect() it away until we actually need it. please
  // note that open-nt.c always requests an kNtGenericExecute accessmask
  iscow = false;
  if (handle != INVALID_HANDLE_VALUE) {
    if ((flags & MAP_TYPE) != MAP_SHARED) {
      // windows has cow pages but they can't propagate across fork()
      // that means we only get copy-on-write for the root process :(
      flprotect = PAGE_WRITECOPY; //PAGE_EXECUTE_WRITECOPY
      desiredaccess = FILE_MAP_COPY; //|| FILE_MAP_EXECUTE
      iscow = true;
    } else {
      fdquerystatus = NtQueryInformationFile(handle, &fdstatus, &fdaccess,
                        sizeof(fdaccess), FileAccessInformation);
      // Assume not read only if NtQueryInformationFile failed, should this be the other way?
      // TODO Confirm that posix O_RDONLY is equivalent to GENERIC_READ & ~GENERIC_WRITE and
      // has nothing to do with GENERIC_EXECUTE
      if (fdquerystatus == STATUS_SUCCESS &&
          (fdaccess.AccessFlags & GENERIC_READ) != 0 &&
          (fdaccess.AccessFlags & GENERIC_WRITE) == 0) {
        flprotect = PAGE_READONLY; //PAGE_EXECUTE_READ
        desiredaccess = FILE_MAP_READ; //|| FILE_MAP_EXECUTE
      } else {
        flprotect = PAGE_READWRITE; //PAGE_EXECUTE_READWRITE
        desiredaccess = FILE_MAP_WRITE; //|| FILE_MAP_EXECUTE
      }
    }
  } else {
    unassert(flags & MAP_ANONYMOUS_);
    flprotect = PAGE_EXECUTE_READWRITE;
    desiredaccess = FILE_MAP_WRITE | FILE_MAP_EXECUTE;
  }

  if ((dm.maphandle = CreateFileMappingW(handle, sec, flprotect,
                                         (size + off) >> 32, (size + off), 0))) {
    if ((dm.addr = MapViewOfFileEx(dm.maphandle, desiredaccess, off >> 32, off,
                                   size, addr))) {
      if (VirtualProtect(addr, size, __prot2nt(prot, iscow), &oldprot)) {
        return dm;
      }
      UnmapViewOfFile(dm.addr);
    }
    CloseHandle(dm.maphandle);
  }

  dm.maphandle = INVALID_HANDLE_VALUE;
  dm.addr = (void *)(intptr_t)-1;
  return dm;
}

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/intrin/memtrack.greg.c
static void *MoveMemoryIntervals(struct MemoryInterval *d,
                                 const struct MemoryInterval *s, int n) {
  int i;
  unassert(n >= 0);
  if (d > s) {
    for (i = n; i--;) {
      d[i] = s[i];
    }
  } else {
    for (i = 0; i < n; ++i) {
      d[i] = s[i];
    }
  }
  return d;
}

static void RemoveMemoryIntervals(struct MemoryIntervals *mm, int i, int n) {
  unassert(i >= 0);
  unassert(i + n <= mm->i);
  MoveMemoryIntervals(mm->p + i, mm->p + i + n, mm->i - (i + n));
  mm->i -= n;
}

static bool ExtendMemoryIntervals(struct MemoryIntervals *mm) {
  int prot, flags;
  char *base, *shad;
  size_t gran, size;
  struct DirectMap dm;
  gran = kMemtrackGran;
  base = (char *)kMemtrackStart;
  prot = PROT_READ | PROT_WRITE;
  flags = MAP_ANONYMOUS_ | MAP_PRIVATE | MAP_FIXED;
  // TODO(jart): These map handles should not leak across NT fork()
  if (mm->p == mm->s) {
    // TODO(jart): How can we detect ASAN mode under GREG?
    if (1 || IsAsan()) {
      shad = (char *)(((intptr_t)base >> 3) + 0x7fff8000);
      dm = sys_mmap_nt(shad, gran >> 3, prot, flags, -1, 0);
      if (!dm.addr) return false;
    }
    dm = sys_mmap_nt(base, gran, prot, flags, -1, 0);
    if (!dm.addr) return false;
    MoveMemoryIntervals(dm.addr, mm->p, mm->i);
    mm->p = dm.addr;
    mm->n = gran / sizeof(*mm->p);
  } else {
    size = ROUNDUP(mm->n * sizeof(*mm->p), gran);
    base += size;
    if (IsAsan()) {
      shad = (char *)(((intptr_t)base >> 3) + 0x7fff8000);
      dm = sys_mmap_nt(shad, gran >> 3, prot, flags, -1, 0);
      if (!dm.addr) return false;
    }
    dm = sys_mmap_nt(base, gran, prot, flags, -1, 0);
    if (!dm.addr) return false;
    mm->n = (size + gran) / sizeof(*mm->p);
  }
  return true;
}

static int CreateMemoryInterval(struct MemoryIntervals *mm, int i) {
  unassert(i >= 0);
  unassert(i <= mm->i);
  unassert(mm->n >= 0);
  if (UNLIKELY(mm->i == mm->n) && !ExtendMemoryIntervals(mm)) return enomem();
  MoveMemoryIntervals(mm->p + i + 1, mm->p + i, mm->i++ - i);
  return 0;
}

static int PunchHole(struct MemoryIntervals *mm, int x, int y, int i) {
  if (CreateMemoryInterval(mm, i) == -1) return -1;
  mm->p[i + 0].size -= (size_t)(mm->p[i + 0].y - (x - 1)) * FRAMESIZE;
  mm->p[i + 0].y = x - 1;
  mm->p[i + 1].size -= (size_t)((y + 1) - mm->p[i + 1].x) * FRAMESIZE;
  mm->p[i + 1].x = y + 1;
  return 0;
}

static int ReleaseMemoryIntervals(struct MemoryIntervals *mm, int x, int y,
                           void wf(struct MemoryIntervals *, int, int)) {
  unsigned l, r;
  unassert(y >= x);
  if (!mm->i) return 0;
  // binary search for the lefthand side
  l = FindMemoryInterval(mm, x);
  if (l == mm->i) return 0;
  if (y < mm->p[l].x) return 0;

  // binary search for the righthand side
  r = FindMemoryInterval(mm, y);
  if (r == mm->i || (r > l && y < mm->p[r].x)) --r;
  unassert(r >= l);
  unassert(x <= mm->p[r].y);

  // remove the middle of an existing map
  //
  // ----|mmmmmmmmmmmmmmmm|--------- before
  //           xxxxx
  // ----|mmmm|-----|mmmmm|--------- after
  //
  // this isn't possible on windows because we track each
  // 64kb segment on that platform using a separate entry
  if (l == r && x > mm->p[l].x && y < mm->p[l].y) {
    return PunchHole(mm, x, y, l);
  }

  // trim the right side of the lefthand map
  //
  // ----|mmmmmmm|-------------- before
  //           xxxxx
  // ----|mmmm|----------------- after
  //
  if (x > mm->p[l].x && x <= mm->p[l].y) {
    unassert(y >= mm->p[l].y);
    return einval();
  }

  // trim the left side of the righthand map
  //
  // ------------|mmmmm|-------- before
  //           xxxxx
  // ---------------|mm|-------- after
  //
  if (y >= mm->p[r].x && y < mm->p[r].y) {
    unassert(x <= mm->p[r].x);
    return einval();
  }

  if (l <= r) {
    if (wf) {
      wf(mm, l, r);
    }
    RemoveMemoryIntervals(mm, l, r - l + 1);
  }
  return 0;
}

static int TrackMemoryInterval(struct MemoryIntervals *mm, int x, int y, HANDLE h,
                               int prot, int flags, bool readonlyfile, bool iscow,
                               long offset, long size) {
  unsigned i;
  unassert(y >= x);
  i = FindMemoryInterval(mm, x);
  // try to extend the righthand side of the lefthand entry
  // we can't do that if we're tracking independent handles
  // we can't do that if it's a file map with a small size!
  if (i && x == mm->p[i - 1].y + 1 && h == mm->p[i - 1].h &&
      prot == mm->p[i - 1].prot && flags == mm->p[i - 1].flags &&
      mm->p[i - 1].size ==
          (size_t)(mm->p[i - 1].y - mm->p[i - 1].x) * FRAMESIZE + FRAMESIZE) {
    mm->p[i - 1].size += (size_t)(y - mm->p[i - 1].y) * FRAMESIZE;
    mm->p[i - 1].y = y;
    // if we filled the hole then merge the two mappings
    if (i < mm->i && y + 1 == mm->p[i].x && h == mm->p[i].h &&
        prot == mm->p[i].prot && flags == mm->p[i].flags) {
      mm->p[i - 1].y = mm->p[i].y;
      mm->p[i - 1].size += mm->p[i].size;
      RemoveMemoryIntervals(mm, i, 1);
    }
  }

  // try to extend the lefthand side of the righthand entry
  // we can't do that if we're creating a smaller file map!
  else if (i < mm->i && y + 1 == mm->p[i].x && h == mm->p[i].h &&
           prot == mm->p[i].prot && flags == mm->p[i].flags &&
           size == (size_t)(y - x) * FRAMESIZE + FRAMESIZE) {
    mm->p[i].size += (size_t)(mm->p[i].x - x) * FRAMESIZE;
    mm->p[i].x = x;
  }

  // otherwise, create a new entry and memmove the items
  else {
    if (CreateMemoryInterval(mm, i) == -1) return -1;
    mm->p[i].x = x;
    mm->p[i].y = y;
    mm->p[i].h = h;
    mm->p[i].prot = prot;
    mm->p[i].flags = flags;
    mm->p[i].offset = offset;
    mm->p[i].size = size;
    mm->p[i].iscow = iscow;
    mm->p[i].readonlyfile = readonlyfile;
  }

  return 0;
}

// From https://github.com/jart/cosmopolitan/blob/b6182d/libc/runtime/memtracknt.c
static inline noasan void *GetFrameAddr(int f) {
  intptr_t a;
  a = f;
  a *= FRAMESIZE;
  return (void *)a;
}

static noasan void ReleaseMemoryNt(struct MemoryIntervals *mm, int l, int r) {
  int i, ok;
  size_t size;
  char *addr, *last;
  for (i = l; i <= r; ++i) {
    addr = GetFrameAddr(mm->p[i].x);
    last = GetFrameAddr(mm->p[i].y);
    UnmapViewOfFile(addr);
    CloseHandle(mm->p[i].h);
  }
}

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/runtime/untrackmemoryintervals.c
static int UntrackMemoryIntervals(void *addr, size_t size) {
  int a, b;
  unassert(size > 0);
  a = ROUNDDOWN((intptr_t)addr, FRAMESIZE) >> 16;
  b = ROUNDDOWN((intptr_t)addr + size - 1, FRAMESIZE) >> 16;
  return ReleaseMemoryIntervals(&_mmi, a, b, ReleaseMemoryNt);
}

// From https://github.com/jart/cosmopolitan/blob/b6182db/libc/log/libfatal.internal.h#L29-L40
forceinline void *__repstosb(void *di, char al, size_t cx) {
  asm("rep stosb"
      : "=D"(di), "=c"(cx), "=m"(*(char(*)[cx])di)
      : "0"(di), "1"(cx), "a"(al));
  return di;
}

// From https://github.com/jart/cosmopolitan/blob/b6182db/libc/runtime/ismemtracked.greg.c
static inline bool IsMemtrackedImpl(int x, int y) {
  unsigned i;
  i = FindMemoryInterval(&_mmi, x);
  if (i == _mmi.i) return false;
  if (x < _mmi.p[i].x) return false;
  for (;;) {
    if (y <= _mmi.p[i].y) return true;
    if (++i == _mmi.i) return false;
    if (_mmi.p[i].x != _mmi.p[i - 1].y + 1) return false;
  }
}

static bool IsMemtracked(int x, int y) {
  /* assumes __mmi_lock() is held */
  bool res;
  res = IsMemtrackedImpl(x, y);
  return res;
}

// From https://github.com/jart/cosmopolitan/blob/b6182db/libc/intrin/asancodes.h
#define kAsanStackOverflow  -9
#define kAsanUnmapped       -17

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/runtime/mmap.c
#define IP(X)      (intptr_t)(X)
#define VIP(X)     (void *)IP(X)
#define ALIGNED(p) (!(IP(p) & (FRAMESIZE - 1)))
#define FRAME(x)   ((int)((intptr_t)(x) >> 16))

static pureconst uint64_t RoundDownTwoPow(uint64_t x) {
  return x ? 1ul << bsr(x) : 0;
}

_Noreturn static void OnUnrecoverableMmapError(const char *s) {
  //STRACE("%s %m", s);
  // TODO Reset windows console? blinkenlights already has an atexit handler
  exit(199);
}

static noasan inline bool OverlapsExistingMapping(char *p, size_t n) {
  int a, b, i;
  unassert(n > 0);
  a = FRAME(p);
  b = FRAME(p + (n - 1));
  i = FindMemoryInterval(&_mmi, a);
  if (i < _mmi.i) {
    if (a <= _mmi.p[i].x && _mmi.p[i].x <= b) return true;
    if (a <= _mmi.p[i].y && _mmi.p[i].y <= b) return true;
    if (_mmi.p[i].x <= a && b <= _mmi.p[i].y) return true;
  }
  return false;
}

static noasan bool ChooseMemoryInterval(int x, int n, int align, int *res) {
  // TODO: improve performance
  int i, start, end;
  unassert(align > 0);
  if (_mmi.i) {

    // find the start of the automap memory region
    i = FindMemoryInterval(&_mmi, x);
    if (i < _mmi.i) {

      // check to see if there's space available before the first entry
      if (!__builtin_add_overflow(x, align - 1, &start)) {
        start &= -align;
        if (!__builtin_add_overflow(start, n - 1, &end)) {
          if (end < _mmi.p[i].x) {
            *res = start;
            return true;
          }
        }
      }

      // check to see if there's space available between two entries
      while (++i < _mmi.i) {
        if (!__builtin_add_overflow(_mmi.p[i - 1].y, 1, &start) &&
            !__builtin_add_overflow(start, align - 1, &start)) {
          start &= -align;
          if (!__builtin_add_overflow(start, n - 1, &end)) {
            if (end < _mmi.p[i].x) {
              *res = start;
              return true;
            }
          }
        }
      }
    }

    // otherwise append after the last entry if space is available
    if (!__builtin_add_overflow(_mmi.p[i - 1].y, 1, &start) &&
        !__builtin_add_overflow(start, align - 1, &start)) {
      start &= -align;
      if (!__builtin_add_overflow(start, n - 1, &end)) {
        *res = start;
        return true;
      }
    }

  } else {
    // if memtrack is empty, then just assign the requested address
    // assuming it doesn't overflow
    if (!__builtin_add_overflow(x, align - 1, &start)) {
      start &= -align;
      if (!__builtin_add_overflow(start, n - 1, &end)) {
        *res = start;
        return true;
      }
    }
  }

  return false;
}

static noasan bool Automap(int count, int align, int *res) {
  return ChooseMemoryInterval(FRAME(kAutomapStart), count, align, res) &&
         *res + count <= FRAME(kAutomapStart + (kAutomapSize - 1));
}

static dontinline noasan void *MapMemories(char *addr, size_t size,
                                           int prot, int flags,
                                           int fd, int64_t off,
                                           int f, int x, int n) {
  size_t i, m;
  int64_t oi, sz;
  struct DirectMap dm;
  bool iscow, readonlyfile;
  HANDLE handle;
  IO_STATUS_BLOCK fdstatus;
  FILE_ACCESS_INFORMATION fdaccess;
  NTSTATUS fdquerystatus;
  m = (size_t)(n - 1) << 16;
  unassert(m < size);
  unassert(m + FRAMESIZE >= size);
  oi = fd == -1 ? 0 : off + m;
  sz = size - m;
  dm = sys_mmap_nt(addr + m, sz, prot, f, fd, oi);
  if (dm.addr == MAP_FAILED) return MAP_FAILED;
  iscow = (flags & MAP_TYPE) != MAP_SHARED && fd != -1;
  handle = (HANDLE)_get_osfhandle(fd);
  fdquerystatus = NtQueryInformationFile(handle, &fdstatus, &fdaccess,
                    sizeof(fdaccess), FileAccessInformation);
  // Assume not read only if NtQueryInformationFile failed, should this be the other way?
  // TODO Confirm that posix O_RDONLY is equivalent to GENERIC_READ & ~GENERIC_WRITE and
  // has nothing to do with GENERIC_EXECUTE
  readonlyfile = (flags & MAP_TYPE) == MAP_SHARED && fd != -1 &&
                  fdquerystatus == STATUS_SUCCESS &&
                  (fdaccess.AccessFlags & GENERIC_READ) != 0 &&
                  (fdaccess.AccessFlags & GENERIC_WRITE) == 0;
  if (TrackMemoryInterval(&_mmi, x + (n - 1), x + (n - 1), dm.maphandle, prot,
                          flags, readonlyfile, iscow, oi, sz) == -1) {
    OnUnrecoverableMmapError("MapMemories unrecoverable #1");
  }
  for (i = 0; i < m; i += FRAMESIZE) {
    oi = fd == -1 ? 0 : off + i;
    sz = FRAMESIZE;
    dm = sys_mmap_nt(addr + i, sz, prot, f, fd, oi);
    if (dm.addr == MAP_FAILED ||
        TrackMemoryInterval(&_mmi, x + i / FRAMESIZE, x + i / FRAMESIZE,
                            dm.maphandle, prot, flags, readonlyfile, iscow, oi,
                            sz) == -1) {
      OnUnrecoverableMmapError("MapMemories unrecoverable #2");
    }
  }
  return addr;
}

static noasan inline void *_Mmap(void *addr, size_t size, int prot, int flags, int fd,
    int64_t off) {
  char *p = addr;
  struct DirectMap dm;
  int a, b, i, f, m, n, x;
  bool needguard, clashes;
  size_t virtualused, virtualneed;

  if (VERY_UNLIKELY(!size)) {
    //STRACE("can't mmap zero bytes");
    return VIP(einval());
  }

  if (VERY_UNLIKELY(!ALIGNED(p))) {
    //STRACE("cosmo mmap is 64kb aligned");
    return VIP(einval());
  }

  if (VERY_UNLIKELY(!IsLegalSize(size))) {
    //STRACE("mmap size isn't legal");
    return VIP(einval());
  }

  if (VERY_UNLIKELY(!IsLegalPointer(p))) {
    //STRACE("mmap addr isn't 48-bit");
    return VIP(einval());
  }

  if ((flags & (MAP_SHARED | MAP_PRIVATE)) == (MAP_SHARED | MAP_PRIVATE)) {
    flags = MAP_SHARED;  // cf. MAP_SHARED_VALIDATE
  }

  if (flags & MAP_ANONYMOUS_) {
    fd = -1;
    off = 0;
    size = ROUNDUP(size, FRAMESIZE);
    prot |= PROT_WRITE;  // kludge
    if ((flags & MAP_TYPE) == MAP_FILE) {
      //STRACE("need MAP_PRIVATE or MAP_SHARED");
      return VIP(einval());
    }
  } else if (VERY_UNLIKELY(off < 0)) {
    //STRACE("mmap negative offset");
    return VIP(einval());
  } else if (VERY_UNLIKELY(!ALIGNED(off))) {
    //STRACE("mmap off isn't 64kb aligned");
    return VIP(einval());
  } else if (VERY_UNLIKELY(INT64_MAX - size < off)) {
    //STRACE("mmap too large");
    return VIP(einval());
  }

  if (__virtualmax < LONG_MAX &&
      (__builtin_add_overflow((virtualused = GetMemtrackSize(&_mmi)), size,
                              &virtualneed) ||
       virtualneed > __virtualmax)) {
    /*STRACE("mmap %'zu size + %'zu inuse exceeds virtual memory limit %'zu",
           size, virtualused, __virtualmax);*/
    return VIP(enomem());
  }

  clashes = OverlapsImageSpace(p, size) || OverlapsExistingMapping(p, size);

  if ((flags & MAP_FIXED_NOREPLACE) == MAP_FIXED_NOREPLACE && clashes) {
    //STRACE("mmap noreplace overlaps existing");
    return VIP(eexist());
  }

  if (__builtin_add_overflow((int)(size >> 16), (int)!!(size & (FRAMESIZE - 1)),
                             &n)) {
    //STRACE("mmap range overflows");
    return VIP(einval());
  }

  a = MAX(1, RoundDownTwoPow(size) >> 16);
  f = (flags & ~MAP_FIXED_NOREPLACE) | MAP_FIXED;
  if (flags & MAP_FIXED) {
    x = FRAME(p);
    if (UntrackMemoryIntervals(p, size)) {
      OnUnrecoverableMmapError("FIXED UNTRACK FAILED");
    }
  } else if (p && !clashes && !OverlapsArenaSpace(p, size) &&
             !OverlapsShadowSpace(p, size)) {
    x = FRAME(p);
  } else if (!Automap(n, a, &x)) {
    //STRACE("automap has no room for %d frames with %d alignment", n, a);
    return VIP(enomem());
  }

  needguard = false;
  p = (char *)ADDR_32_TO_48(x);
  if ((f & MAP_TYPE) == MAP_STACK) {
    if (~f & MAP_ANONYMOUS_) {
      //STRACE("MAP_STACK must be anonymous");
      return VIP(einval());
    }
    f &= ~MAP_TYPE;
    f |= MAP_PRIVATE;
    needguard = true;
  }

  p = MapMemories(p, size, prot, flags, fd, off, f, x, n);

  if (p != MAP_FAILED) {
    if (needguard) {
      if (IsAsan()) {
        __repstosb((void *)(((intptr_t)p >> 3) + 0x7fff8000),
                   kAsanStackOverflow, GUARDSIZE / 8);
      }
    }
  }

  return p;
}

void *mmap(void *addr, size_t size, int prot, int flags, int fd, int64_t off) {
  void *res;
  size_t toto = 0;
  __mmi_lock();
  res = _Mmap(addr, size, prot, flags, fd, off);
  __mmi_unlock();
  return res;
}

// Based on https://github.com/jart/cosmopolitan/blob/b6182db/libc/runtime/munmap.c
static noasan int _Munmap(char *p, size_t n);

static noasan void MunmapShadow(char *p, size_t n) {
  intptr_t a, b, x, y;
  //KERNTRACE("MunmapShadow(%p, %'zu)", p, n);
  a = ((intptr_t)p >> 3) + 0x7fff8000;
  b = a + (n >> 3);
  if (IsMemtracked(FRAME(a), FRAME(b - 1))) {
    x = ROUNDUP(a, FRAMESIZE);
    y = ROUNDDOWN(b, FRAMESIZE);
    if (0 && x < y) {
      // delete shadowspace if unmapping ≥512kb. in practice it has
      // to be >1mb since we can only unmap it if it's aligned, and
      // as such we poison the edges if there are any.
      __repstosb((void *)a, kAsanUnmapped, x - a);
      _Munmap((void *)x, y - x);
      __repstosb((void *)y, kAsanUnmapped, b - y);
    } else {
      // otherwise just poison and assume reuse
      __repstosb((void *)a, kAsanUnmapped, b - a);
    }
  } else {
    //STRACE("unshadow(%.12p, %p) EFAULT", a, b - a);
  }
}

static noasan void MunmapImpl(char *p, size_t n) {
  char *q;
  size_t m;
  intptr_t a, b, c;
  int i, l, r, rc, beg, end;
  //KERNTRACE("MunmapImpl(%p, %'zu)", p, n);
  l = FRAME(p);
  r = FRAME(p + n - 1);
  i = FindMemoryInterval(&_mmi, l);
  for (; i < _mmi.i && r >= _mmi.p[i].x; ++i) {
    if (l >= _mmi.p[i].x && r <= _mmi.p[i].y) {

      // it's contained within the entry
      beg = l;
      end = r;
    } else if (l <= _mmi.p[i].x && r >= _mmi.p[i].x) {

      // it overlaps with the lefthand side of the entry
      beg = _mmi.p[i].x;
      end = MIN(r, _mmi.p[i].y);
    } else if (l <= _mmi.p[i].y && r >= _mmi.p[i].y) {

      // it overlaps with the righthand side of the entry
      beg = MAX(_mmi.p[i].x, l);
      end = _mmi.p[i].y;
    } else {
      __builtin_unreachable();
    }
    // openbsd even requires that if we mapped, for instance a 5 byte
    // file, that we be sure to call munmap(file, 5). let's abstract!
    a = ADDR_32_TO_48(beg);
    b = ADDR_32_TO_48(end) + FRAMESIZE;
    c = ADDR_32_TO_48(_mmi.p[i].x) + _mmi.p[i].size;
    q = (char *)a;
    m = MIN(b, c) - a;
    if (IsAsan() && !OverlapsShadowSpace(p, n)) {
      MunmapShadow(q, m);
    }
  }
}

static noasan int _Munmap(char *p, size_t n) {
  unsigned i;
  char poison;
  intptr_t a, b, x, y;
  //unassert(!__vforked);
  if (UNLIKELY(!n)) {
    //STRACE("munmap n is 0");
    return einval();
  }
  if (UNLIKELY(!IsLegalSize(n))) {
    //STRACE("munmap n isn't 48-bit");
    return einval();
  }
  if (UNLIKELY(!IsLegalPointer(p))) {
    //STRACE("munmap p isn't 48-bit");
    return einval();
  }
  if (UNLIKELY(!IsLegalPointer(p + (n - 1)))) {
    //STRACE("munmap p+(n-1) isn't 48-bit");
    return einval();
  }
  if (UNLIKELY(!ALIGNED(p))) {
    //STRACE("munmap(%p) isn't 64kb aligned", p);
    return einval();
  }
  MunmapImpl(p, n);
  return UntrackMemoryIntervals(p, n);
}

int munmap(void *p, size_t n) {
  int rc;
  size_t toto;
  __mmi_lock();
  rc = _Munmap(p, n);
  __mmi_unlock();
  //STRACE("munmap(%.12p, %'zu) → %d% m (%'zu bytes total)", p, n, rc, toto);
  return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/01fd655/libc/intrin/mmi.init.S
void InitMemory() {
    _mmi.n = OPEN_MAX;
    _mmi.p = _mmi.s;
    __mmi_lock_obj = 1; // PTHREAD_MUTEX_RECURSIVE
}

#endif // __MINGW64_VERSION_MAJOR
