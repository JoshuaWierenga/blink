/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2022 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include <errno.h>
/*#include <netinet/tcp.h>
#include <netinet/udp.h>*/
#include <signal.h>
#include <string.h>
/*#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>*/
#include <time.h>
//#include <unistd.h>

#ifdef _WIN32
#include <winsock.h>

#define EPFNOSUPPORT WSAEPFNOSUPPORT
#else
#include <sys/socket.h>
#endif

#include "blink/windows/macros.h"
#include WINDOWSGNULIBHEADER(fcntl.h)
#include WINDOWSGNULIBHEADER(signal.h)
#include WINDOWSHEADER(mman/mman.h,sys/mman.h)

//#include "blink/builtin.h"
#include "blink/case.h"
#include "blink/endian.h"
#include "blink/xlat.h"

#include <stdio.h>

static long einval(void) {
  errno = EINVAL;
  return -1;
}

int XlatErrno(int x) {
  switch (x) {
    XLAT(EPERM, 1);
    XLAT(ENOENT, 2);
    XLAT(ESRCH, 3);
    XLAT(EINTR, 4);
    XLAT(EIO, 5);
    XLAT(ENXIO, 6);
    XLAT(E2BIG, 7);
    XLAT(ENOEXEC, 8);
    XLAT(EBADF, 9);
    XLAT(ECHILD, 10);
    XLAT(EAGAIN, 11);
#if EWOULDBLOCK != EAGAIN
    XLAT(EWOULDBLOCK, 11);
#endif
    XLAT(ENOMEM, 12);
    XLAT(EACCES, 13);
    XLAT(EFAULT, 14);
    XLAT(EBUSY, 16);
    XLAT(EEXIST, 17);
    XLAT(EXDEV, 18);
    XLAT(ENODEV, 19);
    XLAT(ENOTDIR, 20);
    XLAT(EISDIR, 21);
    XLAT(EINVAL, 22);
    XLAT(ENFILE, 23);
    XLAT(EMFILE, 24);
    XLAT(ENOTTY, 25);
    XLAT(ETXTBSY, 26);
    XLAT(EFBIG, 27);
    XLAT(ENOSPC, 28);
    XLAT(ESPIPE, 29);
    XLAT(EROFS, 30);
    XLAT(EMLINK, 31);
    XLAT(EPIPE, 32);
    XLAT(EDOM, 33);
    XLAT(ERANGE, 34);
    XLAT(EDEADLK, 35);
    XLAT(ENAMETOOLONG, 36);
    XLAT(ENOLCK, 37);
    XLAT(ENOSYS, 38);
    XLAT(ENOTEMPTY, 39);
    XLAT(ELOOP, 40);
    XLAT(ENOMSG, 42);
    XLAT(EIDRM, 43);
    XLAT(EPROTO, 71);
    XLAT(EBADMSG, 74);
    XLAT(EOVERFLOW, 75);
    XLAT(EILSEQ, 84);
    XLAT(ENOTSOCK, 88);
    XLAT(EDESTADDRREQ, 89);
    XLAT(EMSGSIZE, 90);
    XLAT(EPROTOTYPE, 91);
    XLAT(ENOPROTOOPT, 92);
    XLAT(EPROTONOSUPPORT, 93);
    XLAT(ENOTSUP, 95);
#if EOPNOTSUPP != ENOTSUP
    XLAT(EOPNOTSUPP, 95);
#endif
    XLAT(EAFNOSUPPORT, 97);
    XLAT(EADDRINUSE, 98);
    XLAT(EADDRNOTAVAIL, 99);
    XLAT(ENETDOWN, 100);
    XLAT(ENETUNREACH, 101);
    XLAT(ENETRESET, 102);
    XLAT(ECONNABORTED, 103);
    XLAT(ECONNRESET, 104);
    XLAT(ENOBUFS, 105);
    XLAT(EISCONN, 106);
    XLAT(ENOTCONN, 107);
    XLAT(ETIMEDOUT, 110);
    XLAT(ECONNREFUSED, 111);
    XLAT(EHOSTUNREACH, 113);
    XLAT(EALREADY, 114);
    XLAT(EINPROGRESS, 115);
    XLAT(ECANCELED, 125);
    XLAT(EOWNERDEAD, 130);
    XLAT(ENOTRECOVERABLE, 131);
#ifdef ENOTBLK
    XLAT(ENOTBLK, 15);
#endif
#ifdef ETIME
    XLAT(ETIME, 62);
#endif
#ifdef ENONET
    XLAT(ENONET, 64);
#endif
#ifdef EREMOTE
    XLAT(EREMOTE, 66);
#endif
#ifdef ERESTART
    XLAT(ERESTART, 85);
#endif
#ifdef EUSERS
    XLAT(EUSERS, 87);
#endif
#ifdef ENOSR
    XLAT(ENOSR, 63);
#endif
#ifdef ENOSTR
    XLAT(ENOSTR, 60);
#endif
#ifdef ENODATA
    XLAT(ENODATA, 61);
#endif
#ifdef ENOLINK
    XLAT(ENOLINK, 67);
#endif
#ifdef EMULTIHOP
    XLAT(EMULTIHOP, 72);
#endif
#ifdef ESOCKTNOSUPPORT
    XLAT(ESOCKTNOSUPPORT, 94);
#endif
#ifdef EPFNOSUPPORT
    XLAT(EPFNOSUPPORT, 96);
#endif
#ifdef ESHUTDOWN
    XLAT(ESHUTDOWN, 108);
#endif
#ifdef ETOOMANYREFS
    XLAT(ETOOMANYREFS, 109);
#endif
#ifdef EHOSTDOWN
    XLAT(EHOSTDOWN, 112);
#endif
#ifdef ESTALE
    XLAT(ESTALE, 116);
#endif
#ifdef EDQUOT
    XLAT(EDQUOT, 122);
#endif
#ifdef ENOMEDIUM
    XLAT(ENOMEDIUM, 123);
#endif
#ifdef EMEDIUMTYPE
    XLAT(EMEDIUMTYPE, 124);
#endif
    default:
      return x;
  }
}

int XlatSignal(int x) {
  switch (x) {
    XLAT(2, SIGINT);
    XLAT(4, SIGILL);
    XLAT(6, SIGABRT);
    XLAT(8, SIGFPE);
    XLAT(11, SIGSEGV);
    XLAT(15, SIGTERM);
#ifdef SIGHUP
    XLAT(1, SIGHUP);
#endif
#ifdef SIGQUIT
    XLAT(3, SIGQUIT);
#endif
#ifdef SIGTRAP
    XLAT(5, SIGTRAP);
#endif
#ifdef SIGBUS
    XLAT(7, SIGBUS);
#endif
#ifdef SIGKILL
    XLAT(9, SIGKILL);
#endif
#ifdef SIGUSR1
    XLAT(10, SIGUSR1);
#endif
#ifdef SIGUSR2
    XLAT(12, SIGUSR2);
#endif
#ifdef SIGPIPE
    XLAT(13, SIGPIPE);
#endif
#ifdef SIGALRM
    XLAT(14, SIGALRM);
#endif
#ifdef SIGCHLD
    XLAT(17, SIGCHLD);
#endif
#ifdef SIGCONT
    XLAT(18, SIGCONT);
#endif
#ifdef SIGTTIN
    XLAT(21, SIGTTIN);
#endif
#ifdef SIGTTOU
    XLAT(22, SIGTTOU);
#endif
#ifdef SIGXCPU
    XLAT(24, SIGXCPU);
#endif
#ifdef SIGXFSZ
    XLAT(25, SIGXFSZ);
#endif
#ifdef SIGVTALRM
    XLAT(26, SIGVTALRM);
#endif
#ifdef SIGPROF
    XLAT(27, SIGPROF);
#endif
#ifdef SIGWINCH
    XLAT(28, SIGWINCH);
#endif
#ifdef SIGIO
    XLAT(29, SIGIO);
#endif
#ifdef SIGSTOP
    XLAT(19, SIGSTOP);
#endif
#ifdef SIGSYS
    XLAT(31, SIGSYS);
#endif
#ifdef SIGTSTP
    XLAT(20, SIGTSTP);
#endif
#ifdef SIGURG
    XLAT(23, SIGURG);
#endif
    default:
      printf("Unsupported signal %i detected.\n", x);
      return einval();
  }
}

int UnXlatSignal(int x) {
  switch (x) {
    XLAT(SIGINT, 2);
    XLAT(SIGILL, 4);
    XLAT(SIGABRT, 6);
    XLAT(SIGFPE, 8);
    XLAT(SIGSEGV, 11);
    XLAT(SIGTERM, 15);
#ifdef SIGHUP
    XLAT(SIGHUP, 1);
#endif
#ifdef SIGQUIT
    XLAT(SIGQUIT, 3);
#endif
#ifdef SIGTRAP
    XLAT(SIGTRAP, 5);
#endif
#ifdef SIGBUS
    XLAT(SIGBUS, 7);
#endif
#ifdef SIGKILL
    XLAT(SIGKILL, 9);
#endif
#ifdef SIGUSR1
    XLAT(SIGUSR1, 10);
#endif
#ifdef SIGUSR2
    XLAT(SIGUSR2, 12);
#endif
#ifdef SIGPIPE
    XLAT(SIGPIPE, 13);
#endif
#ifdef SIGALRM
    XLAT(SIGALRM, 14);
#endif
#ifdef SIGCHLD
    XLAT(SIGCHLD, 17);
#endif
#ifdef SIGCONT
    XLAT(SIGCONT, 18);
#endif
#ifdef SIGTTIN
    XLAT(SIGTTIN, 21);
#endif
#ifdef SIGTTOU
    XLAT(SIGTTOU, 22);
#endif
#ifdef SIGXCPU
    XLAT(SIGXCPU, 24);
#endif
#ifdef SIGXFSZ
    XLAT(SIGXFSZ, 25);
#endif
#ifdef SIGVTALRM
    XLAT(SIGVTALRM, 26);
#endif
#ifdef SIGPROF
    XLAT(SIGPROF, 27);
#endif
#ifdef SIGWINCH
    XLAT(SIGWINCH, 28);
#endif
#ifdef SIGIO
    XLAT(SIGIO, 29);
#endif
#ifdef SIGSTOP
    XLAT(SIGSTOP, 19);
#endif
#ifdef SIGSYS
    XLAT(SIGSYS, 31);
#endif
#ifdef SIGTSTP
    XLAT(SIGTSTP, 20);
#endif
#ifdef SIGURG
    XLAT(SIGURG, 23);
#endif
    default:
      return 15;
  }
}

int UnXlatSicode(int sig, int code) {
  switch (code) {
#ifdef SI_USER
    XLAT(SI_USER, 0);
#endif
#ifdef SI_QUEUE
    XLAT(SI_QUEUE, -1);
#endif
#ifdef SI_TIMER
    XLAT(SI_TIMER, -2);
#endif
#ifdef SI_TKILL
    XLAT(SI_TKILL, -6);
#endif
#ifdef SI_MESGQ
    XLAT(SI_MESGQ, -3);
#endif
#ifdef SI_ASYNCIO
    XLAT(SI_ASYNCIO, -4);
#endif
#ifdef SI_ASYNCNL
    XLAT(SI_ASYNCNL, -60);
#endif
#ifdef SI_KERNEL
    XLAT(SI_KERNEL, 0x80);
#endif
    default:
      switch (sig) {
#ifdef SIGCHLD
        case SIGCHLD:
          switch (code) {
            XLAT(CLD_EXITED, 1);
            XLAT(CLD_KILLED, 2);
            XLAT(CLD_DUMPED, 3);
            XLAT(CLD_TRAPPED, 4);
            XLAT(CLD_STOPPED, 5);
            XLAT(CLD_CONTINUED, 6);
            default:
              return -1;
          }
#endif
#ifdef SIGBUS
        case SIGTRAP:
          switch (code) {
            XLAT(TRAP_BRKPT, 1);
            XLAT(TRAP_TRACE, 2);
            default:
              return -1;
          }
#endif
        case SIGSEGV:
          switch (code) {
#ifdef SEGV_MAPERR
            XLAT(SEGV_MAPERR, 1);
#endif
#ifdef SEGV_ACCERR
            XLAT(SEGV_ACCERR, 2);
#endif
            default:
              return -1;
          }
        case SIGFPE:
          switch (code) {
#ifdef FPE_INTDIV
            XLAT(FPE_INTDIV, 1);
#endif
#ifdef FPE_INTOVF
            XLAT(FPE_INTOVF, 2);
#endif
#ifdef FPE_FLTDIV
            XLAT(FPE_FLTDIV, 3);
#endif
#ifdef FPE_FLTOVF
            XLAT(FPE_FLTOVF, 4);
#endif
#ifdef FPE_FLTUND
            XLAT(FPE_FLTUND, 5);
#endif
#ifdef FPE_FLTRES
            XLAT(FPE_FLTRES, 6);
#endif
#ifdef FPE_FLTINV
            XLAT(FPE_FLTINV, 7);
#endif
#ifdef FPE_FLTSUB
            XLAT(FPE_FLTSUB, 8);
#endif
            default:
              return -1;
          }
        case SIGILL:
          switch (code) {
#ifdef ILL_ILLOPC
            XLAT(ILL_ILLOPC, 1);
#endif
#ifdef ILL_ILLOPN
            XLAT(ILL_ILLOPN, 2);
#endif
#ifdef ILL_ILLADR
            XLAT(ILL_ILLADR, 3);
#endif
#ifdef ILL_ILLTRP
            XLAT(ILL_ILLTRP, 4);
#endif
#ifdef ILL_PRVOPC
            XLAT(ILL_PRVOPC, 5);
#endif
#ifdef ILL_PRVREG
            XLAT(ILL_PRVREG, 6);
#endif
#ifdef ILL_COPROC
            XLAT(ILL_COPROC, 7);
#endif
#ifdef ILL_BADSTK
            XLAT(ILL_BADSTK, 8);
#endif
            default:
              return -1;
          }
#ifdef SIGBUS
        case SIGBUS:
          switch (code) {
            XLAT(BUS_ADRALN, 1);
            XLAT(BUS_ADRERR, 2);
            XLAT(BUS_OBJERR, 3);
#ifdef BUS_MCEERR_AR
            XLAT(BUS_MCEERR_AR, 4);
#endif
#ifdef BUS_MCEERR_AO
            XLAT(BUS_MCEERR_AO, 5);
#endif
            default:
              return -1;
          }
#endif
#ifdef SIGIO
        case SIGIO:
          switch (code) {
            XLAT(POLL_IN, 1);
            XLAT(POLL_OUT, 2);
            XLAT(POLL_MSG, 3);
            XLAT(POLL_ERR, 4);
            XLAT(POLL_PRI, 5);
            XLAT(POLL_HUP, 6);
            default:
              return -1;
          }
#endif
        default:
          return -1;
      }
  }
}

int XlatSig(int x) {
  switch (x) {
    XLAT(0, SIG_BLOCK);
    XLAT(1, SIG_UNBLOCK);
    XLAT(2, SIG_SETMASK);
    default:
      return einval();
  }
}

int XlatRusage(int x) {
  switch (x) {
    XLAT(0, RUSAGE_SELF);
    XLAT(-1, RUSAGE_CHILDREN);
    default:
      return einval();
  }
}

int XlatSocketFamily(int x) {
  switch (x) {
    XLAT(0, AF_UNSPEC);
    XLAT(1, AF_UNIX);
    XLAT(2, AF_INET);
    default:
      errno = EPFNOSUPPORT;
      return -1;
  }
}

/*int UnXlatSocketFamily(int x) {
  switch (x) {
    XLAT(AF_UNSPEC, 0);
    XLAT(AF_UNIX, 1);
    XLAT(AF_INET, 2);
    default:
      return x;
  }
}*/

int XlatSocketType(int x) {
  switch (x) {
    XLAT(1, SOCK_STREAM);
    XLAT(2, SOCK_DGRAM);
    default:
      return einval();
  }
}

int XlatSocketProtocol(int x) {
  switch (x) {
    XLAT(6, IPPROTO_TCP);
    XLAT(17, IPPROTO_UDP);
    default:
      return einval();
  }
}

/*int XlatSocketLevel(int level) {
  switch (level) {
    XLAT(1, SOL_SOCKET);
    XLAT(6, IPPROTO_TCP);
    XLAT(17, IPPROTO_UDP);
    default:
      return einval();
  }
}

int XlatSocketOptname(int level, int optname) {
  switch (level) {
    case SOL_SOCKET:
      switch (optname) {
        XLAT(2, SO_REUSEADDR);
        XLAT(5, SO_DONTROUTE);
        XLAT(7, SO_SNDBUF);
        XLAT(8, SO_RCVBUF);
        XLAT(9, SO_KEEPALIVE);
        XLAT(13, SO_LINGER);
        XLAT(15, SO_REUSEPORT);
        default:
          return einval();
      }
    case IPPROTO_TCP:
      switch (optname) {
        XLAT(1, TCP_NODELAY);
#if defined(TCP_CORK)
        XLAT(3, TCP_CORK);
#elif defined(TCP_NOPUSH)
        XLAT(3, TCP_NOPUSH);
#endif
#ifdef TCP_FASTOPEN
        XLAT(23, TCP_FASTOPEN);
#endif
#ifdef TCP_QUICKACK
        XLAT(12, TCP_QUICKACK);
#endif
        default:
          return einval();
      }
    default:
      return einval();
  }
}*/

int XlatAccess(int x) {
  int r = F_OK;
  if (x & 1) r |= X_OK;
  if (x & 2) r |= W_OK;
  if (x & 4) r |= R_OK;
  return r;
}

/*int XlatLock(int x) {
  int r = 0;
  if (x & 1) r |= LOCK_SH;
  if (x & 2) r |= LOCK_EX;
  if (x & 4) r |= LOCK_NB;
  if (x & 8) r |= LOCK_UN;
  return r;
}

int XlatWait(int x) {
  int r = 0;
  if (x & 1) r |= WNOHANG;
  if (x & 2) r |= WUNTRACED;
  if (x & 8) r |= WCONTINUED;
  return r;
}*/

int XlatMapFlags(int x) {
  int r = 0;
  if (x & 1) r |= MAP_SHARED;
  if (x & 2) r |= MAP_PRIVATE;
  if (x & 16) r |= MAP_FIXED;
  if (x & 32) r |= MAP_ANONYMOUS;
  return r;
}

int XlatClock(int x) {
  switch (x) {
    XLAT(0, CLOCK_REALTIME);
    XLAT(1, CLOCK_MONOTONIC);
    XLAT(2, CLOCK_PROCESS_CPUTIME_ID);
#ifdef CLOCK_MONOTONIC_RAW
    XLAT(4, CLOCK_MONOTONIC_RAW);
#endif
    default:
      return x;
  }
}

int XlatAtf(int x) {
  int res = 0;
  if (x & 0x0100) res |= AT_SYMLINK_NOFOLLOW;
  if (x & 0x0200) res |= AT_REMOVEDIR;
  if (x & 0x0400) res |= AT_SYMLINK_FOLLOW;
  return res;
}

int XlatOpenMode(int flags) {
  switch (flags & 3) {
    case 0:
      return O_RDONLY;
    case 1:
      return O_WRONLY;
    case 2:
      return O_RDWR;
    default:
      __builtin_unreachable();
  }
}

int XlatOpenFlags(int flags) {
  int res;
  res = XlatOpenMode(flags);
  if (flags & 0x00400) res |= O_APPEND;
  if (flags & 0x00040) res |= O_CREAT;
  if (flags & 0x00080) res |= O_EXCL;
  if (flags & 0x00200) res |= O_TRUNC;
  if (flags & 0x00800) res |= O_NDELAY;
#ifdef O_DIRECT
  if (flags & 0x04000) res |= O_DIRECT;
#endif
  if (flags & 0x10000) res |= O_DIRECTORY;
  if (flags & 0x20000) res |= O_NOFOLLOW;
  if (flags & 0x80000) res |= O_CLOEXEC;
  if (flags & 0x00100) res |= O_NOCTTY;
#ifdef O_ASYNC
  if (flags & 0x02000) res |= O_ASYNC;
#endif
#ifdef O_NOATIME
  if (flags & 0x40000) res |= O_NOATIME;
#endif
#ifdef O_DSYNC
  if (flags & 0x000001000) res |= O_DSYNC;
#endif
#ifdef O_SYNC
  if ((flags & 0x00101000) == 0x00101000) res |= O_SYNC;
#endif
  return res;
}

int XlatFcntlCmd(int x) {
  switch (x) {
    XLAT(1, F_GETFD);
#ifdef F_SETFD
    XLAT(2, F_SETFD);
#endif
#ifdef F_GETFL
    XLAT(3, F_GETFL);
#endif
#ifdef F_SETFL
    XLAT(4, F_SETFL);
#endif
    default:
      return einval();
  }
}

int XlatFcntlArg(int x) {
  switch (x) {
    XLAT(0, 0);
    XLAT(1, FD_CLOEXEC);
    XLAT(0x0800, O_NONBLOCK);
    default:
      return einval();
  }
}

/*void XlatSockaddrToHost(struct sockaddr_in *dst,
                        const struct sockaddr_in_linux *src) {
  memset(dst, 0, sizeof(*dst));
  dst->sin_family = XlatSocketFamily(Read16(src->sin_family));
  dst->sin_port = src->sin_port;
  dst->sin_addr.s_addr = src->sin_addr;
}

void XlatSockaddrToLinux(struct sockaddr_in_linux *dst,
                         const struct sockaddr_in *src) {
  memset(dst, 0, sizeof(*dst));
  Write16(dst->sin_family, UnXlatSocketFamily(src->sin_family));
  dst->sin_port = src->sin_port;
  dst->sin_addr = src->sin_addr.s_addr;
}*/

void XlatStatToLinux(struct stat_linux *dst, const struct stat *src) {
  Write64(dst->st_dev, src->st_dev);
  Write64(dst->st_ino, src->st_ino);
  Write64(dst->st_nlink, src->st_nlink);
  Write32(dst->st_mode, src->st_mode);
  Write32(dst->st_uid, src->st_uid);
  Write32(dst->st_gid, src->st_gid);
  Write32(dst->__pad, 0);
  Write64(dst->st_rdev, src->st_rdev);
  Write64(dst->st_size, src->st_size);
  Write64(dst->st_blksize, src->st_blksize);
  Write64(dst->st_blocks, src->st_blocks);
  Write64(dst->st_dev, src->st_dev);
  // TODO Decide if ns accuracy is required, if not then just add macros like lunux.
#if _WIN32
  Write64(dst->st_atim.tv_sec, (uint64_t)src->st_atim.tv_sec);
  Write64(dst->st_atim.tv_nsec, (uint64_t)src->st_atim.tv_nsec);
  Write64(dst->st_mtim.tv_sec, (uint64_t)src->st_mtim.tv_sec);
  Write64(dst->st_mtim.tv_nsec, (uint64_t)src->st_mtim.tv_nsec);
  Write64(dst->st_ctim.tv_sec, (uint64_t)src->st_ctim.tv_sec);
  Write64(dst->st_ctim.tv_nsec, (uint64_t)src->st_ctim.tv_nsec);
#else
  Write64(dst->st_atim.tv_sec, src->st_atime);
  Write64(dst->st_atim.tv_nsec, 0);
  Write64(dst->st_mtim.tv_sec, src->st_mtime);
  Write64(dst->st_mtim.tv_nsec, 0);
  Write64(dst->st_ctim.tv_sec, src->st_ctime);
  Write64(dst->st_ctim.tv_nsec, 0);
#endif
}

void XlatRusageToLinux(struct rusage_linux *dst, const struct rusage *src) {
  Write64(dst->ru_utime.tv_sec, src->ru_utime.tv_sec);
  Write64(dst->ru_utime.tv_usec, src->ru_utime.tv_usec);
  Write64(dst->ru_stime.tv_sec, src->ru_stime.tv_sec);
  Write64(dst->ru_stime.tv_usec, src->ru_stime.tv_usec);
  Write64(dst->ru_maxrss, src->ru_maxrss);
  Write64(dst->ru_ixrss, src->ru_ixrss);
  Write64(dst->ru_idrss, src->ru_idrss);
  Write64(dst->ru_isrss, src->ru_isrss);
  Write64(dst->ru_minflt, src->ru_minflt);
  Write64(dst->ru_majflt, src->ru_majflt);
  Write64(dst->ru_nswap, src->ru_nswap);
  Write64(dst->ru_inblock, src->ru_inblock);
  Write64(dst->ru_oublock, src->ru_oublock);
  Write64(dst->ru_msgsnd, src->ru_msgsnd);
  Write64(dst->ru_msgrcv, src->ru_msgrcv);
  Write64(dst->ru_nsignals, src->ru_nsignals);
  Write64(dst->ru_nvcsw, src->ru_nvcsw);
  Write64(dst->ru_nivcsw, src->ru_nivcsw);
}

/*void XlatItimervalToLinux(struct itimerval_linux *dst,
                          const struct itimerval *src) {
  Write64(dst->it_interval.tv_sec, src->it_interval.tv_sec);
  Write64(dst->it_interval.tv_usec, src->it_interval.tv_usec);
  Write64(dst->it_value.tv_sec, src->it_value.tv_sec);
  Write64(dst->it_value.tv_usec, src->it_value.tv_usec);
}

void XlatLinuxToItimerval(struct itimerval *dst,
                          const struct itimerval_linux *src) {
  dst->it_interval.tv_sec = Read64(src->it_interval.tv_sec);
  dst->it_interval.tv_usec = Read64(src->it_interval.tv_usec);
  dst->it_value.tv_sec = Read64(src->it_value.tv_sec);
  dst->it_value.tv_usec = Read64(src->it_value.tv_usec);
}*/

void XlatWinsizeToLinux(struct winsize_linux *dst, const struct winsize *src) {
  memset(dst, 0, sizeof(*dst));
  Write16(dst->ws_row, src->ws_row);
  Write16(dst->ws_col, src->ws_col);
}

/*void XlatSigsetToLinux(uint8_t dst[8], const sigset_t *src) {
  int i;
  uint64_t x;
  for (x = i = 0; i < 64; ++i) {
    if (sigismember(src, i + 1)) {
      x |= 1ull << i;
    }
  }
  Write64(dst, x);
}

void XlatLinuxToSigset(sigset_t *dst, const uint8_t src[8]) {
  int i;
  uint64_t x;
  x = Read64(src);
  sigemptyset(dst);
  for (i = 0; i < 64; ++i) {
    if ((1ull << i) & x) {
      sigaddset(dst, i + 1);
    }
  }
}*/

static int XlatTermiosCflag(int x) {
  int r = 0;
  if (x & 0x0001) r |= ISIG;
#ifdef CSTOPB
  if (x & 0x0040) r |= CSTOPB;
#endif
#ifdef CREAD
  if (x & 0x0080) r |= CREAD;
#endif
#ifdef PARENB
  if (x & 0x0100) r |= PARENB;
#endif
#ifdef PARODD
  if (x & 0x0200) r |= PARODD;
#endif
#ifdef HUPCL
  if (x & 0x0400) r |= HUPCL;
#endif
#ifdef CLOCAL
  if (x & 0x0800) r |= CLOCAL;
#endif
  if ((x & 0x0030) == 0x0010) {
    r |= CS6;
  } else if ((x & 0x0030) == 0x0020) {
    r |= CS7;
  } else if ((x & 0x0030) == 0x0030) {
    r |= CS8;
  }
  return r;
}

static int UnXlatTermiosCflag(int x) {
  int r = 0;
  if (x & ISIG) r |= 0x0001;
#ifdef CSTOPB
  if (x & CSTOPB) r |= 0x0040;
#endif
#ifdef CREAD
  if (x & CREAD) r |= 0x0080;
#endif
#ifdef PARENB
  if (x & PARENB) r |= 0x0100;
#endif
#ifdef PARODD
  if (x & PARODD) r |= 0x0200;
#endif
#ifdef HUPCL
  if (x & HUPCL) r |= 0x0400;
#endif
#ifdef CLOCAL
  if (x & CLOCAL) r |= 0x0800;
#endif
  if ((x & CSIZE) == CS5) {
    r |= 0x0000;
  } else if ((x & CSIZE) == CS6) {
    r |= 0x0010;
  } else if ((x & CSIZE) == CS7) {
    r |= 0x0020;
  } else if ((x & CSIZE) == CS8) {
    r |= 0x0030;
  }
  return r;
}

static int XlatTermiosLflag(int x) {
  int r = 0;
  if (x & 0x0001) r |= ISIG;
  if (x & 0x0002) r |= ICANON;
  if (x & 0x0008) r |= ECHO;
  if (x & 0x0010) r |= ECHOE;
#ifdef ECHOK
  if (x & 0x0020) r |= ECHOK;
#endif
#ifdef ECHONL
  if (x & 0x0040) r |= ECHONL;
#endif
#ifdef NOFLSH
  if (x & 0x0080) r |= NOFLSH;
#endif
#ifdef TOSTOP
  if (x & 0x0100) r |= TOSTOP;
#endif
#ifdef IEXTEN
  if (x & 0x8000) r |= IEXTEN;
#endif
#ifdef ECHOCTL
  if (x & 0x0200) r |= ECHOCTL;
#endif
#ifdef ECHOPRT
  if (x & 0x0400) r |= ECHOPRT;
#endif
#ifdef ECHOKE
  if (x & 0x0800) r |= ECHOKE;
#endif
#ifdef FLUSHO
  if (x & 0x1000) r |= FLUSHO;
#endif
#ifdef PENDIN
  if (x & 0x4000) r |= PENDIN;
#endif
#ifdef XCASE
  if (x & 0x0004) r |= XCASE;
#endif
  return r;
}

static int UnXlatTermiosLflag(int x) {
  int r = 0;
  if (x & ISIG) r |= 0x0001;
  if (x & ICANON) r |= 0x0002;
  if (x & ECHO) r |= 0x0008;
  if (x & ECHOE) r |= 0x0010;
#ifdef ECHOK
  if (x & ECHOK) r |= 0x0020;
#endif
#ifdef ECHONL
  if (x & ECHONL) r |= 0x0040;
#endif
#ifdef NOFLSH
  if (x & NOFLSH) r |= 0x0080;
#endif
#ifdef TOSTOP
  if (x & TOSTOP) r |= 0x0100;
#endif
#ifdef IEXTEN
  if (x & IEXTEN) r |= 0x8000;
#endif
#ifdef ECHOCTL
  if (x & ECHOCTL) r |= 0x0200;
#endif
#ifdef ECHOPRT
  if (x & ECHOPRT) r |= 0x0400;
#endif
#ifdef ECHOKE
  if (x & ECHOKE) r |= 0x0800;
#endif
#ifdef FLUSHO
  if (x & FLUSHO) r |= 0x1000;
#endif
#ifdef PENDIN
  if (x & PENDIN) r |= 0x4000;
#endif
#ifdef XCASE
  if (x & XCASE) r |= 0x0004;
#endif
  return r;
}

static int XlatTermiosIflag(int x) {
  int r = 0;
#ifdef IGNBLK
  if (x & 0x0001) r |= IGNBRK;
#endif
#ifdef BRKINT
  if (x & 0x0002) r |= BRKINT;
#endif
#ifdef IGNPAR
  if (x & 0x0004) r |= IGNPAR;
#endif
#ifdef PARMRK
  if (x & 0x0008) r |= PARMRK;
#endif
#ifdef INPCK
  if (x & 0x0010) r |= INPCK;
#endif
#ifdef ISTRIP
  if (x & 0x0020) r |= ISTRIP;
#endif
#ifdef INLCR
  if (x & 0x0040) r |= INLCR;
#endif
#ifdef IGNCR
  if (x & 0x0080) r |= IGNCR;
#endif
#ifdef ICRNL
  if (x & 0x0100) r |= ICRNL;
#endif
#ifdef IXON
  if (x & 0x0400) r |= IXON;
#endif
#ifdef IXANY
  if (x & 0x0800) r |= IXANY;
#endif
#ifdef IXOFF
  if (x & 0x1000) r |= IXOFF;
#endif
#ifdef IMAXBEL
  if (x & 0x2000) r |= IMAXBEL;
#endif
#ifdef IUTF8
  if (x & 0x4000) r |= IUTF8;
#endif
#ifdef IUCLC
  if (x & 0x0200) r |= IUCLC;
#endif
  return r;
}

static int UnXlatTermiosIflag(int x) {
  int r = 0;
#ifdef IGNBRK
  if (x & IGNBRK) r |= 0x0001;
#endif
#ifdef BRKINT
  if (x & BRKINT) r |= 0x0002;
#endif
#ifdef IGNPAR
  if (x & IGNPAR) r |= 0x0004;
#endif
#ifdef PARMRK
  if (x & PARMRK) r |= 0x0008;
#endif
#ifdef INPCK
  if (x & INPCK) r |= 0x0010;
#endif
#ifdef ISTRIP
  if (x & ISTRIP) r |= 0x0020;
#endif
#ifdef INLCR
  if (x & INLCR) r |= 0x0040;
#endif
#ifdef IGNCR
  if (x & IGNCR) r |= 0x0080;
#endif
#ifdef ICRNL
  if (x & ICRNL) r |= 0x0100;
#endif
#ifdef IXON
  if (x & IXON) r |= 0x0400;
#endif
#ifdef IXANY
  if (x & IXANY) r |= 0x0800;
#endif
#ifdef IXOFF
  if (x & IXOFF) r |= 0x1000;
#endif
#ifdef IMAXBEL
  if (x & IMAXBEL) r |= 0x2000;
#endif
#ifdef IUTF8
  if (x & IUTF8) r |= 0x4000;
#endif
#ifdef IUCLC
  if (x & IUCLC) r |= 0x0200;
#endif
  return r;
}

static int XlatTermiosOflag(int x) {
  int r = 0;
  if (x & 0x0001) r |= OPOST;
#ifdef ONLCR
  if (x & 0x0004) r |= ONLCR;
#endif
#ifdef OCRNL
  if (x & 0x0008) r |= OCRNL;
#endif
#ifdef ONOCR
  if (x & 0x0010) r |= ONOCR;
#endif
#ifdef ONLRET
  if (x & 0x0020) r |= ONLRET;
#endif
#ifdef OFILL
  if (x & 0x0040) r |= OFILL;
#endif
#ifdef OFDEL
  if (x & 0x0080) r |= OFDEL;
#endif
#ifdef NLDLY
  if ((x & 0x0100) == 0x0000) {
    r |= NL0;
  } else if ((x & 0x0100) == 0x0100) {
    r |= NL1;
#ifdef NL2
  } else if ((x & 0x0100) == 0x0000) {
    r |= NL2;
#endif
#ifdef NL3
  } else if ((x & 0x0100) == 0x0000) {
    r |= NL3;
#endif
  }
#endif
#ifdef CRDLY
  if ((x & 0x0600) == 0x0000) {
    r |= CR0;
  } else if ((x & 0x0600) == 0x0200) {
    r |= CR1;
  } else if ((x & 0x0600) == 0x0400) {
    r |= CR2;
  } else if ((x & 0x0600) == 0x0600) {
    r |= CR3;
  }
#endif
#ifdef TABDLY
  if ((x & 0x1800) == 0x0000) {
    r |= TAB0;
#ifdef TAB1
  } else if ((x & 0x1800) == 0x0800) {
    r |= TAB1;
#endif
#ifdef TAB1
  } else if ((x & 0x1800) == 0x1000) {
    r |= TAB2;
#endif
  } else if ((x & 0x1800) == 0x1800) {
    r |= TAB3;
  }
#endif
#ifdef BSDLY
  if ((x & 0x2000) == 0x0000) {
    r |= BS0;
  } else if ((x & 0x2000) == 0x2000) {
    r |= BS1;
  }
#endif
#ifdef VTBDLY
  if ((x & 0x4000) == 0x0000) {
    r |= VT0;
  } else if ((x & 0x4000) == 0x4000) {
    r |= VT1;
  }
#endif
#ifdef FFBDLY
  if ((x & 0x8000) == 0x0000) {
    r |= FF0;
  } else if ((x & 0x8000) == 0x8000) {
    r |= FF1;
  }
#endif
#ifdef OLCUC
  if (x & 0x0002) r |= OLCUC;
#endif
  return r;
}

static int UnXlatTermiosOflag(int x) {
  int r = 0;
  if (x & OPOST) r |= 0x0001;
#ifdef ONLCR
  if (x & ONLCR) r |= 0x0004;
#endif
#ifdef OCRNL
  if (x & OCRNL) r |= 0x0008;
#endif
#ifdef ONOCR
  if (x & ONOCR) r |= 0x0010;
#endif
#ifdef ONLRET
  if (x & ONLRET) r |= 0x0020;
#endif
#ifdef OFILL
  if (x & OFILL) r |= 0x0040;
#endif
#ifdef OFDEL
  if (x & OFDEL) r |= 0x0080;
#endif
#ifdef NLDLY
  if ((x & NLDLY) == NL0) {
    r |= 0x0000;
  } else if ((x & NLDLY) == NL1) {
    r |= 0x0100;
#ifdef NL2
  } else if ((x & NLDLY) == NL2) {
    r |= 0x0000;
#endif
#ifdef NL3
  } else if ((x & NLDLY) == NL3) {
    r |= 0x0000;
#endif
  }
#endif
#ifdef CRDLY
  if ((x & CRDLY) == CR0) {
    r |= 0x0000;
  } else if ((x & CRDLY) == CR1) {
    r |= 0x0200;
  } else if ((x & CRDLY) == CR2) {
    r |= 0x0400;
  } else if ((x & CRDLY) == CR3) {
    r |= 0x0600;
  }
#endif
#ifdef TABDLY
  if ((x & TABDLY) == TAB0) {
    r |= 0x0000;
#ifdef TAB1
  } else if ((x & TABDLY) == TAB1) {
    r |= 0x0800;
#endif
#ifdef TAB2
  } else if ((x & TABDLY) == TAB2) {
    r |= 0x1000;
#endif
  } else if ((x & TABDLY) == TAB3) {
    r |= 0x1800;
  }
#endif
#ifdef BSDLY
  if ((x & BSDLY) == BS0) {
    r |= 0x0000;
  } else if ((x & BSDLY) == BS1) {
    r |= 0x2000;
  }
#endif
#ifdef VTDLY
  if ((x & VTDLY) == VT0) {
    r |= 0x0000;
  } else if ((x & VTDLY) == VT1) {
    r |= 0x4000;
  }
#endif
#ifdef FFDLY
  if ((x & FFDLY) == FF0) {
    r |= 0x0000;
  } else if ((x & FFDLY) == FF1) {
    r |= 0x8000;
  }
#endif
#ifdef OLCUC
  if (x & OLCUC) r |= 0x0002;
#endif
  return r;
}

static void XlatTermiosCc(struct termios *dst,
                          const struct termios_linux *src) {
  dst->c_cc[VINTR] = src->c_cc[0];
  dst->c_cc[VQUIT] = src->c_cc[1];
  dst->c_cc[VERASE] = src->c_cc[2];
  dst->c_cc[VKILL] = src->c_cc[3];
  dst->c_cc[VEOF] = src->c_cc[4];
#ifdef VTIME
  dst->c_cc[VTIME] = src->c_cc[5];
#endif
  dst->c_cc[VMIN] = src->c_cc[6];
  dst->c_cc[VSTART] = src->c_cc[8];
  dst->c_cc[VSTOP] = src->c_cc[9];
  dst->c_cc[VSUSP] = src->c_cc[10];
#ifdef VTIME
  dst->c_cc[VEOL] = src->c_cc[11];
#endif
#ifdef VSWTC
  dst->c_cc[VSWTC] = src->c_cc[7];
#endif
#ifdef VREPRINT
  dst->c_cc[VREPRINT] = src->c_cc[12];
#endif
#ifdef VDISCARD
  dst->c_cc[VDISCARD] = src->c_cc[13];
#endif
#ifdef VWERASE
  dst->c_cc[VWERASE] = src->c_cc[14];
#endif
#ifdef VLNEXT
  dst->c_cc[VLNEXT] = src->c_cc[15];
#endif
#ifdef VEOL2
  dst->c_cc[VEOL2] = src->c_cc[16];
#endif
}

static void UnXlatTermiosCc(struct termios_linux *dst,
                            const struct termios *src) {
  dst->c_cc[0] = src->c_cc[VINTR];
  dst->c_cc[1] = src->c_cc[VQUIT];
  dst->c_cc[2] = src->c_cc[VERASE];
  dst->c_cc[3] = src->c_cc[VKILL];
  dst->c_cc[4] = src->c_cc[VEOF];
#ifdef VTIME
  dst->c_cc[5] = src->c_cc[VTIME];
#endif
  dst->c_cc[6] = src->c_cc[VMIN];
  dst->c_cc[8] = src->c_cc[VSTART];
  dst->c_cc[9] = src->c_cc[VSTOP];
  dst->c_cc[10] = src->c_cc[VSUSP];
#ifdef VEOL
  dst->c_cc[11] = src->c_cc[VEOL];
#endif
#ifdef VSWTC
  dst->c_cc[7] = src->c_cc[VSWTC];
#endif
#ifdef VREPRINT
  dst->c_cc[12] = src->c_cc[VREPRINT];
#endif
#ifdef VDISCARD
  dst->c_cc[13] = src->c_cc[VDISCARD];
#endif
#ifdef VWERASE
  dst->c_cc[14] = src->c_cc[VWERASE];
#endif
#ifdef VLNEXT
  dst->c_cc[15] = src->c_cc[VLNEXT];
#endif
#ifdef VEOL2
  dst->c_cc[16] = src->c_cc[VEOL2];
#endif
}

void XlatLinuxToTermios(struct termios *dst, const struct termios_linux *src) {
  memset(dst, 0, sizeof(*dst));
  dst->c_iflag = XlatTermiosIflag(Read32(src->c_iflag));
  dst->c_oflag = XlatTermiosOflag(Read32(src->c_oflag));
  dst->c_cflag = XlatTermiosCflag(Read32(src->c_cflag));
  dst->c_lflag = XlatTermiosLflag(Read32(src->c_lflag));
  XlatTermiosCc(dst, src);
}

void XlatTermiosToLinux(struct termios_linux *dst, const struct termios *src) {
  memset(dst, 0, sizeof(*dst));
  Write32(dst->c_iflag, UnXlatTermiosIflag(src->c_iflag));
  Write32(dst->c_oflag, UnXlatTermiosOflag(src->c_oflag));
  Write32(dst->c_cflag, UnXlatTermiosCflag(src->c_cflag));
  Write32(dst->c_lflag, UnXlatTermiosLflag(src->c_lflag));
  UnXlatTermiosCc(dst, src);
}
