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
#include <winerror.h>

#include "blink/builtin.h"
#include "blink/errno.h"

static dontinline long ReturnErrno(int e) {
  errno = e;
  return -1;
}

long ebadf(void) {
    return ReturnErrno(EBADF);
}

long einval(void) {
  return ReturnErrno(EINVAL);
}

long eagain(void) {
  return ReturnErrno(EAGAIN);
}

long enomem(void) {
  return ReturnErrno(ENOMEM);
}

long enosys(void) {
  return ReturnErrno(ENOSYS);
}

long efault(void) {
  return ReturnErrno(EFAULT);
}

void *efault0(void) {
  efault();
  return 0;
}

long eintr(void) {
  return ReturnErrno(EINTR);
}

long eoverflow(void) {
  return ReturnErrno(EOVERFLOW);
}

long enfile(void) {
  return ReturnErrno(ENFILE);
}

long esrch(void) {
  return ReturnErrno(ESRCH);
}

long eperm(void) {
  return ReturnErrno(EPERM);
}

long enotsup(void) {
  return ReturnErrno(ENOTSUP);
}

long enoent(void) {
  return ReturnErrno(ENOENT);
}

long enotdir(void) {
  return ReturnErrno(ENOTDIR);
}

long erange(void) {
  return ReturnErrno(ERANGE);
}

long eopnotsupp(void) {
  return ReturnErrno(EOPNOTSUPP);
}

long enametoolong(void) {
  return ReturnErrno(ENAMETOOLONG);
}


// This is potentially useless since MinGW's errno.h uses different
// values for many errnos than linux.
struct Dos2Errno kDos2Errno[] = {
  { ERROR_INVALID_TARGET_HANDLE, EBADF },
  { ERROR_DIRECT_ACCESS_HANDLE, EBADF },
  { ERROR_MOD_NOT_FOUND, ENOSYS },
  { ERROR_BAD_COMMAND, EACCES },
  { ERROR_BAD_LENGTH, EACCES },
  { ERROR_BAD_NETPATH, ENOENT },
  { ERROR_BAD_NET_NAME, ENOENT },
  { ERROR_BAD_PATHNAME, ENOENT },
  { ERROR_BAD_NET_RESP, ENETDOWN },
  { ERROR_FILE_EXISTS, EEXIST },
  { ERROR_CANNOT_MAKE, EACCES },
  { ERROR_COMMITMENT_LIMIT, ENOMEM },
  { ERROR_CONNECTION_ABORTED, ECONNABORTED },
  { ERROR_CONNECTION_ACTIVE, EISCONN },
  { ERROR_CONNECTION_REFUSED, ECONNREFUSED },
  { ERROR_CRC, EACCES },
  { ERROR_DIR_NOT_EMPTY, ENOTEMPTY },
  { ERROR_DUP_NAME, EADDRINUSE },
  { ERROR_FILENAME_EXCED_RANGE, ENAMETOOLONG },
  { ERROR_GEN_FAILURE, EACCES },
  { ERROR_GRACEFUL_DISCONNECT, EPIPE },
  { ERROR_HOST_DOWN, EHOSTUNREACH },
  { ERROR_HOST_UNREACHABLE, EHOSTUNREACH },
  { ERROR_INSUFFICIENT_BUFFER, EFAULT },
  { ERROR_NOACCESS, EFAULT },
  { ERROR_INVALID_ADDRESS, EADDRNOTAVAIL },
  { ERROR_NOT_A_REPARSE_POINT, EINVAL },
  { ERROR_INVALID_FUNCTION, EINVAL },
  { ERROR_NEGATIVE_SEEK, EINVAL },
  { ERROR_INVALID_NETNAME, EADDRNOTAVAIL },
  { ERROR_INVALID_USER_BUFFER, EMSGSIZE },
  { ERROR_IO_PENDING, EINPROGRESS },
  { ERROR_LOCK_VIOLATION, EAGAIN },
  { ERROR_MORE_DATA, EMSGSIZE },
  { ERROR_NETNAME_DELETED, ECONNABORTED },
  { ERROR_NETWORK_ACCESS_DENIED, EACCES },
  { ERROR_NETWORK_BUSY, ENETDOWN },
  { ERROR_NETWORK_UNREACHABLE, ENETUNREACH },
  { ERROR_NONPAGED_SYSTEM_RESOURCES, ENOMEM },
  { ERROR_NOT_ENOUGH_MEMORY, ENOMEM },
  { ERROR_NOT_ENOUGH_QUOTA, ENOMEM },
  { ERROR_NOT_FOUND, ENOENT },
  { ERROR_NOT_READY, EACCES },
  { ERROR_NOT_SUPPORTED, ENOTSUP },
  { ERROR_NO_MORE_FILES, ENOENT },
  { ERROR_NO_SYSTEM_RESOURCES, ENOMEM },
  { ERROR_OPERATION_ABORTED, EINTR },
  { ERROR_OUT_OF_PAPER, EACCES },
  { ERROR_PAGED_SYSTEM_RESOURCES, ENOMEM },
  { ERROR_PAGEFILE_QUOTA, ENOMEM },
  { ERROR_PIPE_NOT_CONNECTED, EPIPE },
  { ERROR_PORT_UNREACHABLE, ECONNRESET },
  { ERROR_PROTOCOL_UNREACHABLE, ENETUNREACH },
  { ERROR_REM_NOT_LIST, ECONNREFUSED },
  { ERROR_REQUEST_ABORTED, EINTR },
  { ERROR_REQ_NOT_ACCEP, EWOULDBLOCK },
  { ERROR_SECTOR_NOT_FOUND, EACCES },
  { ERROR_SEM_TIMEOUT, ETIMEDOUT },
  { ERROR_SHARING_VIOLATION, EACCES },
  { ERROR_TOO_MANY_NAMES, ENOMEM },
  { ERROR_UNEXP_NET_ERR, ECONNABORTED },
  { ERROR_WORKING_SET_QUOTA, ENOMEM },
  { ERROR_WRITE_PROTECT, EACCES },
  { ERROR_WRONG_DISK, EACCES },
  { WSAEACCES, EACCES },
  { WSAEDISCON, EPIPE },
  { WSAEFAULT, EFAULT },
  { WSAEINVAL, EINVAL },
  // EDQUOT isn't in mingw errno.h so just using linux value.
  { WSAEDQUOT, /*EDQUOT*/ 122 },
  { WSAEPROCLIM, ENOMEM },
  { WSANOTINITIALISED, ENETDOWN },
  { WSASYSNOTREADY, ENETDOWN },
  { WSAVERNOTSUPPORTED, ENOSYS },
  { WSAETIMEDOUT, ETIMEDOUT },
};