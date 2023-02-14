#ifndef BLINK_WIN_FCNTL_H_
#define BLINK_WIN_FCNTL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/calls.h

#include_next <fcntl.h>

#ifdef _WIN32
#include "third_party/gnulib_build/lib/fcntl.h"

// Not required until gnulib's fcntl module is removed.
//int fcntl(int, int, ...);
// Not sure if this is needed, windows does provide a version of this,
// will be required to I want/need to switch to cosmo's open flags.
//int open(const char *, int, ...);
int openat(int, const char *, int, ...);
#endif

#endif /* BLINK_WIN_FCNTL_H_ */
