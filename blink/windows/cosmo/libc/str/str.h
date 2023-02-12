#ifndef BLINK_WIN_COSMO_STR_H_
#define BLINK_WIN_COSMO_SRE_H_
#include <stdint.h>
#include <uchar.h>

typedef struct {
  intptr_t ax, dx;
} axdx_t;

axdx_t tprecode16to8(char *, size_t, const char16_t *);

#endif /* BLINK_WIN_COSMO_STR_H_ */
