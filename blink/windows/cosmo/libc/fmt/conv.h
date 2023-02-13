#ifndef BLINK_WIN_COSMO_LIBC_FMT_CONV_H_
#define BLINK_WIN_COSMO_LIBC_FMT_CONV_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/fmt/conv.h

/*───────────────────────────────────────────────────────────────────────────│─╗
│ cosmopolitan § conversion                                                ─╬─│┼
╚────────────────────────────────────────────────────────────────────────────│*/

#define MODERNITYSECONDS 11644473600ull
#define HECTONANOSECONDS 10000000ull

/*───────────────────────────────────────────────────────────────────────────│─╗
│ cosmopolitan § conversion » time                                         ─╬─│┼
╚────────────────────────────────────────────────────────────────────────────│*/

struct timespec WindowsTimeToTimeSpec(int64_t);
struct timespec WindowsDurationToTimeSpec(int64_t);

#define ReadFileTime(t)                     \
  ({                                        \
    FILETIME __t = t;              \
    uint64_t x = __t.dwHighDateTime;        \
    (int64_t)(x << 32 | __t.dwLowDateTime); \
  })

#define FileTimeToTimeSpec(x) WindowsTimeToTimeSpec(ReadFileTime(x))

#endif /* BLINK_WIN_COSMO_LIBC_FMT_CONV_H_ */
