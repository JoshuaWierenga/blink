#include <limits.h>
#include <stdint.h>
#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include "blink/macros.h"
#include "blink/windows/getids.h"
#include "blink/windows/macros.h"

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/getuid.c#L30-L37
static uint32_t KnuthMultiplicativeHash32(const void *buf, size_t size) {
    size_t i;
    uint32_t h;
    const uint32_t kPhiPrime = 0x9e3779b1;
    const unsigned char *p = (const unsigned char *)buf;
    for (h = i = 0; i < size; i++) h = (p[i] + h) * kPhiPrime;
    return h;
}

// GetUserNameEx might be better? Or perhaps GetTokenInformation
// since it can give an id.
// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/getuid.c#L39-L44
static uint32_t GetUserNameHash(void) {
    char buf[UNLEN + 1];
    DWORD size = ARRAYLEN(buf);
    GetUserName((char*)&buf, &size);
    return KnuthMultiplicativeHash32(buf, size >> 1) & INT_MAX;
}


// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/getuid.c#L57-L69
uint32_t getuid(void) {
    int rc;
    // Need to clear this to prevent the existing value getting through if
    // everything works.
    errno = 0;
    rc = GetUserNameHash();
    _npassert(rc >= 0);
    STRACE("%s() -> %d", "getuid", rc);
    return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/geteuid.c#L28-L37
uint32_t geteuid(void) {
    uint32_t rc;
    rc = getuid();
    STRACE("%s() -> %u %m", "geteuid", rc);
    return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/getuid.c#L82-L94
uint32_t getgid(void) {
    int rc;
    // Need to clear this to prevent the existing value getting through if
    // everything works.
    errno = 0;
    rc = GetUserNameHash();
    _npassert(rc >= 0);
    STRACE("%s() -> %d", "getgid", rc);
    return rc;
}

// Based on https://github.com/jart/cosmopolitan/blob/4b8874c/libc/calls/getegid.c#L29-L38
uint32_t getegid(void) {
    uint32_t rc;
    rc = getgid();
    STRACE("%s() -> %u %m", "getegid", rc);
    return rc;
}
