#ifndef BLINK_WIN_STRING_H_
#define BLINK_WIN_STRING_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/mem/mem.h

#include_next <string.h>

#ifdef _WIN32
#include "third_party/gnulib_build/lib/string.h"

char *strndup(const char *, size_t);
#endif

#endif /* BLINK_WIN_STRING_H_ */
