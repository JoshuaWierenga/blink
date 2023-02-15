#ifndef BLINK_WIN_COSMO_LIBC_CALLS_INTERNAL_H_
#define BLINK_WIN_COSMO_LIBC_CALLS_INTERNAL_H_
#include "blink/macros.h"

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/internal.h

inline size_t _clampio(size_t size) {
  //if (!IsTrustworthy()) {
    return MIN(size, 0x7ffff000);
  /*} else {
    return size;
  }*/
}

#endif /* BLINK_WIN_COSMO_LIBC_CALLS_INTERNAL_H_ */
