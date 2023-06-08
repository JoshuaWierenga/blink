#ifndef BLINK_FDS_H_
#define BLINK_FDS_H_
#include <dirent.h>
#include <limits.h>
#ifndef __MINGW64_VERSION_MAJOR
#include <netinet/in.h>
#include <poll.h>
#endif
#include <stdbool.h>
#ifndef __MINGW64_VERSION_MAJOR
#include <sys/socket.h>
#include <sys/uio.h>
#include <termios.h>
#endif

#include "blink/dll.h"
#include "blink/thread.h"
#include "blink/types.h"

#ifndef __MINGW64_VERSION_MAJOR
#define FD_CONTAINER(e) DLL_CONTAINER(struct Fd, elem, e)

struct winsize;

struct FdCb {
  int (*close)(int);
  ssize_t (*readv)(int, const struct iovec *, int);
  ssize_t (*writev)(int, const struct iovec *, int);
  int (*poll)(struct pollfd *, nfds_t, int);
  int (*tcgetattr)(int, struct termios *);
  int (*tcsetattr)(int, int, const struct termios *);
  int (*tcgetwinsize)(int, struct winsize *);
  int (*tcsetwinsize)(int, const struct winsize *);
};

struct Fd {
  int fildes;      // file descriptor
  int oflags;      // host O_XXX constants
  int socktype;    // host SOCK_XXX constants
  bool norestart;  // is SO_RCVTIMEO in play?
  DIR *dirstream;  // for getdents() lazilly
  struct Dll elem;
  pthread_mutex_t_ lock;
  const struct FdCb *cb;
  char *path;
  union {
    struct sockaddr sa;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
  } saddr;
};
#endif

struct Fds {
  struct Dll *list;
  pthread_mutex_t_ lock;
};

#ifndef __MINGW64_VERSION_MAJOR
extern const struct FdCb kFdCbHost;
#endif

void InitFds(struct Fds *);
#ifndef __MINGW64_VERSION_MAJOR
struct Fd *AddFd(struct Fds *, int, int);
struct Fd *ForkFd(struct Fds *, struct Fd *, int, int);
struct Fd *GetFd(struct Fds *, int);
void LockFd(struct Fd *);
void UnlockFd(struct Fd *);
int CountFds(struct Fds *);
void FreeFd(struct Fd *);
void DestroyFds(struct Fds *);
void InheritFd(struct Fd *);

#endif
#endif /* BLINK_FDS_H_ */
