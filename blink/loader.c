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
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "blink/windows/macros.h"
#include WINDOWSHEADER(mman/mman.h,sys/mman.h)

#include "blink/argv.h"
#include "blink/endian.h"
#include "blink/loader.h"
#include "blink/machine.h"
#include "blink/macros.h"
#include "blink/memory.h"
#include "blink/util.h"

#define READ64(p) Read64((const uint8_t *)(p))
#define READ32(p) Read32((const uint8_t *)(p))

static void LoadElfLoadSegment(struct Machine *m, void *code, size_t codesize,
                               Elf64_Phdr *phdr) {
  int64_t align, bsssize;
  int64_t felf, fstart, fend, vstart, vbss, vend;
  align = MAX(Read64(phdr->p_align), 4096);
  assert(1 == popcount(align));
  assert(0 == (Read64(phdr->p_vaddr) - Read64(phdr->p_offset)) % align);
  felf = (int64_t)(intptr_t)code;
  vstart = ROUNDDOWN(Read64(phdr->p_vaddr), align);
  vbss = ROUNDUP(Read64(phdr->p_vaddr) + Read64(phdr->p_filesz), align);
  vend = ROUNDUP(Read64(phdr->p_vaddr) + Read64(phdr->p_memsz), align);
  fstart = felf + ROUNDDOWN(Read64(phdr->p_offset), align);
  fend = felf + Read64(phdr->p_offset) + Read64(phdr->p_filesz);
  bsssize = vend - vbss;
  m->brk = MAX(m->brk, vend);
  assert(vend >= vstart);
  assert(fend >= fstart);
  assert(felf <= fstart);
  assert(vstart >= -0x800000000000);
  assert(vend <= 0x800000000000);
  assert(vend - vstart >= fstart - fend);
  assert(Read64(phdr->p_filesz) <= Read64(phdr->p_memsz));
  assert(felf + Read64(phdr->p_offset) - fstart ==
         Read64(phdr->p_vaddr) - vstart);
  if (ReserveVirtual(m, vstart, fend - fstart, 0x0207) == -1) {
    fprintf(stderr, "ReserveVirtual failed\r\n");
    exit(1);
  }
  VirtualRecv(m, vstart, (void *)(uintptr_t)fstart, fend - fstart);
  if (bsssize) {
    if (ReserveVirtual(m, vbss, bsssize, 0x0207) == -1) {
      fprintf(stderr, "ReserveVirtual failed\r\n");
      exit(1);
    }
  }
  if (Read64(phdr->p_memsz) - Read64(phdr->p_filesz) > bsssize) {
    VirtualSet(m, Read64(phdr->p_vaddr) + Read64(phdr->p_filesz), 0,
               Read64(phdr->p_memsz) - Read64(phdr->p_filesz) - bsssize);
  }
}

static void LoadElf(struct Machine *m, struct Elf *elf) {
  unsigned i;
  Elf64_Phdr *phdr;
  m->ip = elf->base = Read64(elf->ehdr->e_entry);
  for (i = 0; i < Read16(elf->ehdr->e_phnum); ++i) {
    phdr = GetElfSegmentHeaderAddress(elf->ehdr, elf->size, i);
    switch (Read32(phdr->p_type)) {
      case PT_LOAD:
        elf->base = MIN(elf->base, Read64(phdr->p_vaddr));
        LoadElfLoadSegment(m, elf->ehdr, elf->size, phdr);
        break;
      default:
        break;
    }
  }
}

static void LoadBin(struct Machine *m, intptr_t base, const char *prog,
                    void *code, size_t codesize) {
  Elf64_Phdr phdr;
  Write32(phdr.p_type, PT_LOAD);
  Write32(phdr.p_flags, PF_X | PF_R | PF_W);
  Write64(phdr.p_offset, 0);
  Write64(phdr.p_vaddr, base);
  Write64(phdr.p_paddr, base);
  Write64(phdr.p_filesz, codesize);
  Write64(phdr.p_memsz, ROUNDUP(codesize + 4 * 1024 * 1024, 4 * 1024 * 1024));
  Write64(phdr.p_align, 4096);
  LoadElfLoadSegment(m, code, codesize, &phdr);
  m->ip = base;
}

static void BootProgram(struct Machine *m, struct Elf *elf, size_t codesize) {
  m->ip = 0x7c00;
  elf->base = 0x7c00;
  if (ReserveReal(m, 0x00f00000) == -1) {
    fprintf(stderr, "ReserveReal failed\r\n");
    exit(1);
  }
  memset(m->real.p, 0, 0x00f00000);
  Write16(m->real.p + 0x400, 0x3F8);
  Write16(m->real.p + 0x40E, 0xb0000 >> 4);
  Write16(m->real.p + 0x413, 0xb0000 / 1024);
  Write16(m->real.p + 0x44A, 80);
  Write64(m->cs, 0);
  Write64(m->dx, 0);
  memcpy(m->real.p + 0x7c00, elf->map, 512);
  if (memcmp(elf->map, "\177ELF", 4) == 0) {
    elf->ehdr = (void *)elf->map;
    elf->size = codesize;
    elf->base = Read64(elf->ehdr->e_entry);
  } else {
    elf->base = 0x7c00;
    elf->ehdr = NULL;
    elf->size = 0;
  }
}

static int GetElfHeader(char ehdr[64], const char *prog, const char *image) {
  int c, i;
  const char *p;
  for (p = image; p < image + 4096; ++p) {
    if (READ64(p) != READ64("printf '")) {
      continue;
    }
    for (i = 0, p += 8; p + 3 < image + 4096 && (c = *p++) != '\'';) {
      if (c == '\\') {
        if ('0' <= *p && *p <= '7') {
          c = *p++ - '0';
          if ('0' <= *p && *p <= '7') {
            c *= 8;
            c += *p++ - '0';
            if ('0' <= *p && *p <= '7') {
              c *= 8;
              c += *p++ - '0';
            }
          }
        }
      }
      if (i < 64) {
        ehdr[i++] = c;
      } else {
        fprintf(stderr, "%s: ape printf elf header too long\n", prog);
        return -1;
      }
    }
    if (i != 64) {
      fprintf(stderr, "%s: ape printf elf header too short\n", prog);
      return -1;
    }
    if (READ32(ehdr) != READ32("\177ELF")) {
      fprintf(stderr, "%s: ape printf elf header didn't have elf magic\n",
              prog);
      return -1;
    }
    return 0;
  }
  fprintf(stderr, "%s: printf statement not found in first 4096 bytes\n", prog);
  return -1;
}

void LoadProgram(struct Machine *m, char *prog, char **args, char **vars,
                 struct Elf *elf) {
  int fd;
  int64_t sp;
  char ehdr[64];
  struct stat st;
  assert(prog);
  elf->prog = prog;
  if ((fd = open(prog,
#ifdef _WIN32
      O_RDWR
#else
      O_RDONLY
#endif
)) == -1 ||
      (fstat(fd, &st) == -1 || !st.st_size)) {
    fputs(prog, stderr);
    fputs(": not found\r\n", stderr);
    exit(1);
  }
  elf->mapsize = st.st_size;
  elf->map =
      mmap(NULL, elf->mapsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (elf->map == MAP_FAILED) {
    fprintf(stderr, "mmap failed: %s\r\n", strerror(errno));
    exit(1);
  }
  close(fd);
  ResetCpu(m);
  if ((m->mode & 3) == XED_MODE_REAL) {
    BootProgram(m, elf, elf->mapsize);
  } else {
    sp = 0x800000000000;
    Write64(m->sp, sp);
    m->cr3 = AllocateLinearPage(m);
    if (ReserveVirtual(m, sp - 0x100000, 0x100000, 0x0207) == -1) {
      fprintf(stderr, "ReserveVirtual failed\r\n");
      exit(1);
    }
    LoadArgv(m, prog, args, vars);
    if (memcmp(elf->map, "\177ELF", 4) == 0) {
      elf->ehdr = (void *)elf->map;
      elf->size = elf->mapsize;
      LoadElf(m, elf);
    } else if (READ64(elf->map) == READ64("MZqFpD='") &&
               !GetElfHeader(ehdr, prog, elf->map)) {
      memcpy(elf->map, ehdr, 64);
      elf->ehdr = (void *)elf->map;
      elf->size = elf->mapsize;
      LoadElf(m, elf);
    } else {
      elf->base = 0x400000;
      elf->ehdr = NULL;
      elf->size = 0;
      LoadBin(m, elf->base, prog, elf->map, elf->mapsize);
    }
  }
}
