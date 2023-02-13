#include <errno.h>
#include <stdint.h>

#include "blink/windows/cosmo/libc/sysv/errfuns.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/sysv/errfuns.sh

// These are meant to generated assembly but I couldn't get that to work for pe32
// as all the files were filled with efi only pseudo-ops and the gas documentation
// only talks about coff and elf, not pe32. I know coff and pe32 are related but
// even with coff pseudo-ops, gas kept compaining.
intptr_t enoent(void) {
    errno = ENOENT;
    return -1;
}

intptr_t eacces(void) {
    errno = ENOENT;
    return -1;
}

intptr_t enotty(void) {
    errno = ENOTTY;
    return -1;
}

intptr_t enametoolong(void) {
    errno = ENOENT;
    return -1;
}
