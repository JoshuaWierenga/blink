#ifndef BLINK_WIN_COSMO_LIBC_CALLS_NTMAGICPATHS_H_
#define BLINK_WIN_COSMO_LIBC_CALLS_NTMAGICPATHS_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/ntmagicpaths.internal.h

struct NtMagicPaths {
#define TAB(NAME, STRING) char NAME[sizeof(STRING)];
#include "blink/windows/cosmo/libc/calls/ntmagicpaths.inc"
#undef TAB
};

extern const struct NtMagicPaths kNtMagicPaths;

#endif /* BLINK_WIN_COSMO_LIBC_CALLS_NTMAGICPATHS_H_ */
