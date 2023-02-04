#ifndef BLINK_WIN_IOCTL_H_
#define BLINK_WIN_IOCTL_H_

#ifdef _WIN32
#include <stdlib.h>
#include <stdint.h>
#include "blink/windows/macros.h"
#include "blink/windows/termios.h"

int ioctl(int, uint64_t, ...);

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/ioctl.h#L26-L63
#define __IOCTL_DISPATCH(CMP, DEFAULT, FD, REQUEST, ...)    \
({                                                          \
    int ReZ;                                                \
    if (CMP(REQUEST, TIOCGWINSZ)) {                         \
        ReZ = ioctl_tiocgwinsz(FD, ##__VA_ARGS__);          \
    } else if (CMP(REQUEST, TCGETS)) {                      \
        ReZ = ioctl_tcgets(FD, ##__VA_ARGS__);              \
	} else if (CMP(REQUEST, TCSETS)) {                      \
	    ReZ = ioctl_tcsets(FD, REQUEST, ##__VA_ARGS__);     \
    } else {                                                \
        STRACE("ioctl(%d, %d, ...) -> ERROR: "              \
            "Requested ioctl is not supported on windows",  \
            (fd), (REQUEST));                               \
        abort();                                            \
    }                                                       \
    STRACE("ioctl(%d, %d, ...) -> %'ld %m",                 \
        (fd), (REQUEST), ReZ);                              \
    ReZ;                                                    \
})
#endif

#endif /* BLINK_WIN_IOCTL_H_ */
