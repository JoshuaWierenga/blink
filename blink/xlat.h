#ifndef BLINK_XLAT_H_
#define BLINK_XLAT_H_
#include "blink/windows.h"

#ifndef WINBLINK
#include <netinet/in.h>
#endif
#include <signal.h>
#ifndef WINBLINK
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#endif
#include <sys/stat.h>
#ifndef WINBLINK
#include <sys/statvfs.h>
#include <sys/time.h>
#include <termios.h>

#include "blink/linux.h"

int UnXlatSiCode(int, int);
int UnXlatOpenFlags(int);
int UnXlatAccMode(int);
#endif
int UnXlatSignal(int);
#ifndef WINBLINK
int UnXlatItimer(int);
int XlatAccess(int);
int XlatClock(int, clock_t *);
int XlatErrno(int);
int XlatOpenFlags(int);
int XlatAccMode(int);
int XlatResource(int);
int XlatRusage(int);
int XlatShutdown(int);
int XlatSignal(int);
int XlatSocketFamily(int);
int XlatSocketLevel(int, int *);
int XlatSocketOptname(int, int);
int XlatSocketProtocol(int);
int XlatSocketType(int);
int XlatWait(int);
int XlatWhence(int);

int XlatSockaddrToHost(struct sockaddr_storage *, const struct sockaddr_linux *,
                       u32);
int XlatSockaddrToLinux(struct sockaddr_storage_linux *,
                        const struct sockaddr *, socklen_t);
void XlatStatToLinux(struct stat_linux *, const struct stat *);
void XlatRusageToLinux(struct rusage_linux *, const struct rusage *);
void XlatItimervalToLinux(struct itimerval_linux *, const struct itimerval *);
void XlatLinuxToItimerval(struct itimerval *, const struct itimerval_linux *);
void XlatLinuxToTermios(struct termios *, const struct termios_linux *);
void XlatTermiosToLinux(struct termios_linux *, const struct termios *);
void XlatWinsizeToLinux(struct winsize_linux *, const struct winsize *);
void XlatWinsizeToHost(struct winsize *, const struct winsize_linux *);
void XlatSigsetToLinux(u8[8], const sigset_t *);
void XlatLinuxToSigset(sigset_t *, u64);
void XlatRlimitToLinux(struct rlimit_linux *, const struct rlimit *);
void XlatLinuxToRlimit(int, struct rlimit *, const struct rlimit_linux *);

#endif

#endif /* BLINK_XLAT_H_ */
