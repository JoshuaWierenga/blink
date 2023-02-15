#ifndef BLINK_WIN_WCHAR_H_
#define BLINK_WIN_WCHAR_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/str/unicode.h

#include_next <wchar.h>

#ifdef _WIN32
int wcwidth(wchar_t);
#endif

#endif /* BLINK_WIN_WCHAR_H_ */
