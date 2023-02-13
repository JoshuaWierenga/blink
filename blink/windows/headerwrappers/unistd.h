#ifndef BLINK_WIN_UNISTD_H_
#define BLINK_WIN_UNISTD_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/calls.h

#include_next <unistd.h>

#ifdef _WIN32
#include "third_party/gnulib_build/config.h"
#include "third_party/gnulib_build/lib/unistd.h"

int faccessat(int, const char *, int, int);
#endif

#endif /* BLINK_WIN_UNISTD_H_ */
