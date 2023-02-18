#ifndef BLINK_ERRNO_H_
#define BLINK_ERRNO_H_

#include "blink/types.h"

long eagain(void);
long ebadf(void);
long efault(void);
void *efault0(void);
long eintr(void);
long einval(void);
long enfile(void);
long enoent(void);
long enomem(void);
long enosys(void);
long enotdir(void);
long enotsup(void);
long eoverflow(void);
long eopnotsupp(void);
long erange(void);
long eperm(void);
long esrch(void);
long enametoolong(void);

struct __attribute__((__packed__)) Dos2Errno {
 u16 doscode;
 errno_t errnocode;
};

extern struct Dos2Errno kDos2Errno[];

#endif /* BLINK_ERRNO_H_ */
