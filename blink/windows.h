#ifndef WINBLINK_WINDOWS_H_
#define WINBLINK_WINDOWS_H_

#include "blink/tunables.h"

// Can extend this to old icc, mingw and mingw-w64 as required, some have more
// posix support than others so might need to be handled differently
#if defined(_WIN32) && defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define WINBLINK

#define BLINK_COMMITS_VALUE 589
// TODO Fetch from environment as total git commits - BLINK_COMMITS_VALUE
#define WINBLINK_COMMITS_VALUE 4

// TODO Add windows version of ./configure?
#if defined(DEBUG)
#define BUILD_MODE "dbg"
#elif defined(NDEBUG)
#define BUILD_MODE "rel"
#endif
// TODO Find a way display version as major.minor.patch(.rev? this part does
// exist as its own macro), fetch from cl.exe somehow, it doesn't have a pretty
// printed version, would need to extract from regular output.
#define BUILD_TOOLCHAIN  "cl.exe (MSVC) " STRINGISE(_MSC_FULL_VER)
#define BLINK_COMMITS    STRINGISE(BLINK_COMMITS_VALUE)
#define BLINK_GITSHA     "671aa0b8fd2d87bf1ce8df8ff11f0ccf3e1d8993"
#define CONFIG_ARGUMENTS "--disable-all"
#define WINBLINK_COMMITS STRINGISE(WINBLINK_COMMITS_VALUE)
// TODO Fetch from environment
// #define WINBLINK_GITSHA
#endif

#define WINBLINK_MAJOR 0
#define WINBLINK_MINOR 1
#define WINBLINK_PATCH 0
#define WINBLINK_VERSION \
  MKVERSION(WINBLINK_MAJOR, WINBLINK_MINOR, WINBLINK_PATCH)

#define PATH_MAX _MAX_PATH
#define getcwd   _getcwd
#define strdup   _strdup

#endif /* WINBLINK_WINDOWS_H_ */