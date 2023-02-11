#ifndef BLINK_WIN_STRING_H_
#define BLINK_WIN_STRING_H_

#ifdef _WIN32
#include "third_party/gnulib_build/lib/string.h"

char *strndup(const char *, size_t);
#endif

#include_next <string.h>

#endif /* BLINK_WIN_UNISTD_H_ */