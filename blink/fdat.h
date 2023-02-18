#ifndef BLINK_FDAT_H_
#define BLINK_FDAT_H_
#include <unistd.h>

#include "blink/builtin.h"

int faccessat_(int fd, const char *path, int amode, int flag);

#if !defined(HAVE_FACCESSAT) && defined(_WIN32)
#ifdef faccessat
#undef faccessat
#endif
#ifdef AT_FDCWD
#undef AT_FDCWD
#endif
#define faccessat faccessat_
#define AT_FDCWD -100
#endif

#endif /* BLINK_FDAT_H_ */
