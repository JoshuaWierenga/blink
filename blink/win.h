#ifndef BLINK_WIN_H_
#define BLINK_WIN_H_

#include <stddef.h>

#ifdef __MINGW64_VERSION_MAJOR

#include <stdint.h>

// Ensure GetFinalPathNameByHandleW is defined, requires vista or newer
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600

// version detection

#define IsAtLeastWindows8p1() \
  (GetNtMajorVersion() > 6 || (GetNtMajorVersion() == 6 && GetNtMinorVersion() == 3))
#define GetNtMajorVersion()    \
  ({                           \
    uintptr_t __x;             \
    asm("mov\t%%gs:96,%q0\r\n" \
        "mov\t280(%q0),%b0"    \
        : "=q"(__x));          \
    (unsigned char)__x;        \
  })
#define GetNtMinorVersion()    \
  ({                           \
    uintptr_t __x;             \
    asm("mov\t%%gs:96,%q0\r\n" \
        "mov\t284(%q0),%b0"    \
        : "=q"(__x));          \
    (unsigned char)__x;        \
  })

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

// mman support

#define MAP_FAILED         ((void *)-1)

#define MAP_FILE                     0
#define MAP_SHARED                   1
#define MAP_PRIVATE                  2
#define MAP_STACK                    6
#define MAP_TYPE                    15
#define MAP_FIXED           0x00000010
#define MAP_FIXED_NOREPLACE 0x08000000

#define PROT_READ  1
#define PROT_WRITE 2
#define PROT_EXEC  4

void *mmap(void *addr, size_t size, int prot, int flags, int fd, int64_t off);
int munmap(void *p, size_t n);

void InitMemory();

// fd support

#define AT_FDCWD -100

int faccessat(int dirfd, const char *path, int amode, int flags);

// miscellaneous

#ifdef _PID_T_
typedef _pid_t	pid_t;
#endif

#ifdef _OFF_T_
#if defined(__x86_64__) && defined(_OFF64_T_DEFINED)
typedef _off64_t off_t;
#elif defined(__i386__)
typedef _off32_t off_t;
#else
#error off_t is not available despite the relevent\
header being included, different arch used?
#endif
#endif

#define environ _environ

// I disabled emitting of posix macros and functions
// from windows in preparation for replacing open,
// need to temporarily redefine the macros that are
// being used via their internal names
#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#define O_APPEND _O_APPEND
#define O_CREAT _O_CREAT
#define O_ACCMODE _O_ACCMODE

#define close _close
#define getcwd _getcwd
#define open _open
#define strdup _strdup
#define unlink _unlink
#define write _write

#else

#define InitMemory()

#endif/* __MINGW64_VERSION_MAJOR */
#endif /* BLINK_WIN_H_ */
