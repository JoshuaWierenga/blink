#ifndef BLINK_WIN_COSMO_LIBC_FMT_MAGNUMSTRS_H_
#define BLINK_WIN_COSMO_LIBC_FMT_MAGNUMSTRS_H_

#include <fcntl.h>

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/fmt/magnumstrs.internal.h

#define MAGNUM_TERMINATOR -123

#define MAGNUM_NUMBER(TABLE, INDEX) \
  (const int)(TABLE[INDEX].x)

#define MAGNUM_STRING(TABLE, INDEX) \
  (const char *)(TABLE[INDEX].s)

struct MagnumStr {
  int x;
  char* s;
};

// I had some major issues getting the assembly version working so just copied
// the array into c.
// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/sysv/consts.sh

// Using mingw/microsoft's open flags for now some are missing.
// I think I do now know how to kind of get the cosmo ones working even in asm 
// but its a pain and may not work correctly.
const struct MagnumStr kOpenFlags[] = {
  { O_RDWR,       "RDWR"	     },  // order matters
  { O_RDONLY,     "RDONLY"	     },  //
  { O_WRONLY,     "WRONLY"	     },  //
  { O_ACCMODE,    "ACCMODE"	     },  // mask of prev three
  { O_CREAT,      "CREAT"	     },  //
  { O_EXCL,       "EXCL"	     },  //
  { O_TRUNC,      "TRUNC"	     },  //
  { O_CLOEXEC,    "CLOEXEC"	     },  //
  { O_NONBLOCK,   "NONBLOCK"     },  //
//{ O_TMPFILE,    "TMPFILE"      },  // linux, windows
  { O_DIRECTORY,  "DIRECTORY"    },  // order matters
  { O_DIRECT,     "DIRECT"	     },  // no-op on xnu/openbsd
  { O_APPEND,     "APPEND"	     },  // weird on nt
  { O_NOFOLLOW,   "NOFOLLOW"     },  // unix
  { O_SYNC,       "SYNC"	     },  // unix
//{ O_ASYNC,      "ASYNC"        },  // unix
  { O_NOCTTY,     "NOCTTY"	     },  // unix
  { O_NOATIME,    "NOATIME"	     },  // linux
  { O_EXEC,       "EXEC"	     },  // free/openbsd
  { O_SEARCH,     "SEARCH"	     },  // free/netbsd
  { O_DSYNC,      "DSYNC"	     },  // linux/xnu/open/netbsd
  { O_RSYNC,      "RSYNC"	     },  // linux/open/netbsd
//{ O_PATH,       "PATH"	     },  // linux
//{ O_VERIFY,     "VERIFY"       },  // freebsd
//{ O_SHLOCK,     "SHLOCK"       },  // bsd
//{ O_EXLOCK,     "EXLOCK"       },  // bsd
  { O_RANDOM,     "RANDOM"	     },  // windows
  { O_SEQUENTIAL, "SEQUENTIAL"   },  // windows
//{ O_COMPRESSED, "COMPRESSED"   },  // windows
//{ O_INDEXED,    "INDEXED"      },  // windows
//{ O_LARGEFILE,  "LARGEFILE"    },  //
  { MAGNUM_TERMINATOR, NULL      }
};

#endif /* BLINK_WIN_COSMO_LIBC_FMT_MAGNUMSTRS_H_ */
