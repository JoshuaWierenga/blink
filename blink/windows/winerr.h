#ifndef BLINK_WIN_WINERR_H_
#define BLINK_WIN_WINERR_H_

#ifdef _WIN32
#include <stdint.h>

int64_t __winerr(void);
#endif

#endif /* BLINK_WIN_WINERR_H_ */
