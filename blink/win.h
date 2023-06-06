#ifndef BLINK_WIN_H_
#define BLINK_WIN_H_

#ifdef __MINGW64_VERSION_MAJOR

// dup support

#define O_CLOEXEC 0x00080000

int sys_dup_nt(int oldfd, int newfd, int flags);

// fnctl support

#define F_DUPFD 0
//#define F_GETFD 1
//#define F_SETFD 2
//#define F_GETFL 3
//#define F_SETFL 4

//#define F_GETLK 5
//#define F_SETLK 6
//#define F_SETLKW 7

//#define FD_CLOEXEC 1
#define F_DUPFD_CLOEXEC 0x0406 // Currently the same as F_DUPFD

int fcntl(int fd, int cmd, ...);

#endif /* __MINGW64_VERSION_MAJOR */
#endif /* BLINK_WIN_H_ */
