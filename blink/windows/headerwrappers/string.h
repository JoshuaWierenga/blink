#ifndef BLINK_WIN_STRING_H_
#define BLINK_WIN_STRING_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/mem/mem.h
// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/str/str.h

#include_next <string.h>

#ifdef _WIN32
char *stpcpy(char *, const char *);
char *strndup(const char *, size_t);
#endif

#endif /* BLINK_WIN_STRING_H_ */
