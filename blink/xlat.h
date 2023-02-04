#ifndef BLINK_XLAT_H_
#define BLINK_XLAT_H_
//#include <signal.h>
#include <sys/stat.h>
//#include <sys/time.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif

#include "blink/windows/macros.h"
#include WINDOWSGNULIBHEADER(sys/resource.h)

#include "blink/linux.h"
#include "blink/termios.h"

int XlatAccess(int);
int XlatAtf(int);
int XlatClock(int);
int XlatErrno(int);
int XlatFcntlArg(int);
int XlatFcntlCmd(int);
//int XlatLock(int);*/
int XlatMapFlags(int);
int XlatOpenFlags(int);
//int XlatOpenMode(int);
int XlatRusage(int);
int XlatSig(int);
int XlatSignal(int);
int UnXlatSignal(int);
int UnXlatSicode(int, int);
int XlatSocketFamily(int);
/*int XlatSocketLevel(int);
int XlatSocketOptname(int, int);*/
int XlatSocketProtocol(int);
int XlatSocketType(int);
int XlatWait(int);

/*void XlatSockaddrToHost(struct sockaddr_in *, const struct sockaddr_in_linux *);
void XlatSockaddrToLinux(struct sockaddr_in_linux *,
                         const struct sockaddr_in *);*/
void XlatStatToLinux(struct stat_linux *, const struct stat *);
void XlatRusageToLinux(struct rusage_linux *, const struct rusage *);
/*void XlatItimervalToLinux(struct itimerval_linux *, const struct itimerval *);
void XlatLinuxToItimerval(struct itimerval *, const struct itimerval_linux *);*/
void XlatLinuxToTermios(struct termios *, const struct termios_linux *);
void XlatTermiosToLinux(struct termios_linux *, const struct termios *);
void XlatWinsizeToLinux(struct winsize_linux *, const struct winsize *);
/*void XlatSigsetToLinux(uint8_t[8], const sigset_t *);
void XlatLinuxToSigset(sigset_t *, const uint8_t[8]);*/

#endif /* BLINK_XLAT_H_ */
