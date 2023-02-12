#ifndef BLINK_WIN_DIRENT_H_
#define BLINK_WIN_DIRENT_H_

#ifdef _WIN32
#include <stdint.h>

struct dirent {      /* linux getdents64 abi */
  uint64_t d_ino;    /* inode number */
  int64_t d_off;     /* implementation-dependent location number */
  uint16_t d_reclen; /* byte length of this whole struct and string */
  uint8_t d_type;    /* DT_REG, DT_DIR, DT_UNKNOWN, DT_BLK, etc. */
  char d_name[256];  /* NUL-terminated basename */
};

// The naming might not stick but its designed to make it clear that
// any strings included in WDIR are using utf-16 wide chars.
struct dirstream;
typedef struct dirstream WDIR;

WDIR *fdopendirw(int);
struct dirent *readdirw(WDIR *);
#endif

#endif /* BLINK_WIN_UNISTD_H_ */