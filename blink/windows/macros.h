#ifndef BLINK_WIN_MACROS_H_
#define BLINK_WIN_MACROS_H_

// Based on https://stackoverflow.com/a/32077478
#ifdef _WIN32
#define STR(x) #x

#define WINDOWSHEADER(winpath, defpath) STR(third_party/winpath)
#define WINDOWSGNULIBHEADER(path) WINDOWSHEADER(gnulib_build/lib/path,)
#else
#define WINDOWSHEADER(winpath, defpath) <defpath>
#define WINDOWSGNULIBHEADER(path) WINDOWSHEADER(,path)
#endif

#ifdef _WIN32
#include <time.h>
#include <unistd.h>
#include "blink/log.h"

// Based on https://github.com/jart/cosmopolitan/blob/6d36584/libc/assert.h#L38-L48
#define _npassert(x)                    \
  ({                                    \
    if (__builtin_expect(!(x), 0)) {    \
        __builtin_unreachable();        \
	}                                   \
    (void)0;                            \
  })

// Sligtly cut down with respect to cosmo's STRACE but kind of close.
#define STRACE(FMT, ...)                                \
    LOGF("SYS %6p %'18lu " FMT "\n", getpid(),          \
         (unsigned long)time(NULL),  ##__VA_ARGS__);
#endif

#endif /* BLINK_WIN_MACROS_H_ */
