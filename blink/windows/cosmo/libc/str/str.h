#ifndef BLINK_WIN_COSMO_STR_H_
#define BLINK_WIN_COSMO_STR_H_
#include <stdint.h>
#include <uchar.h>

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/integral/c.inc

typedef struct {
  intptr_t ax, dx;
} axdx_t;

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/str/str.h

axdx_t tprecode8to16(char16_t *, size_t, const char *);
axdx_t tprecode16to8(char *, size_t, const char16_t *);

#endif /* BLINK_WIN_COSMO_STR_H_ */
