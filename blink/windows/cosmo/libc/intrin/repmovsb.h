#ifndef BLINK_WIN_COSMO_LIBC_INTRIN_REPMOVSB_H_
#define BLINK_WIN_COSMO_LIBC_INTRIN_REPMOVSB_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/intrin/repmovsb.h

inline void repmovsb(void **dest, const void **src, size_t cx) {
  char *di = (char *)*dest;
  const char *si = (const char *)*src;
  while (cx) *di++ = *si++, cx--;
  *dest = di, *src = si;
}

#endif /* BLINK_WIN_COSMO_LIBC_INTRIN_REPMOVSB_H_ */
