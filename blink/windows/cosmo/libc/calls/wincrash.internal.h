#ifndef COSMOPOLITAN_LIBC_CALLS_WINCRASH_INTERNAL_H_
#define COSMOPOLITAN_LIBC_CALLS_WINCRASH_INTERNAL_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/calls/wincrash.internal.h

OVERLAPPED *_offset2overlap(HANDLE, int64_t, OVERLAPPED *);

#endif /* COSMOPOLITAN_LIBC_CALLS_WINCRASH_INTERNAL_H_ */
