#ifndef BLINK_FDS_H_
#define BLINK_FDS_H_

#include <stdint.h>

#ifdef _WIN32
#include "third_party/gnulib_build/lib/poll.h"
#include "third_party/gnulib_build/lib/sys/uio.h"
#else
#include <poll.h>
#include <sys/uio.h>
#endif

struct MachineFdClosed {
  int fd;
  struct MachineFdClosed *next;
};

struct MachineFdCb {
  int (*close)(int);
  ssize_t (*readv)(int, const struct iovec *, int);
  ssize_t (*writev)(int, const struct iovec *, int);
  int (*ioctl)(int, int, ...);
  int (*poll)(struct pollfd *, nfds_t, int);
};

struct MachineFd {
  int fd;
  const struct MachineFdCb *cb;
};

struct MachineFds {
  int i, n;
  struct MachineFd *p;
  struct MachineFdClosed *closed;
};

int MachineFdAdd(struct MachineFds *);
void MachineFdRemove(struct MachineFds *, int);

#endif /* BLINK_FDS_H_ */
