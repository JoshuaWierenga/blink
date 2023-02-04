#ifndef BLINK_IOVS_H_
#define BLINK_IOVS_H_
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include "third_party/gnulib_build/lib/sys/uio.h"
#else
#include <sys/uio.h>
#endif

struct Iovs {
  unsigned i, n;
  struct iovec *p;
  struct iovec init[2];
};

int AppendIovs(struct Iovs *, void *, size_t);
void FreeIovs(struct Iovs *);
void InitIovs(struct Iovs *);

#endif /* BLINK_IOVS_H_ */
