#ifndef BLINK_WIN_GETEIDS_H_
#define BLINK_WIN_GETEIDS_H_

#ifdef _WIN32
#include <stdint.h>

uint32_t getuid(void);
uint32_t geteuid(void);

uint32_t getgid(void);
uint32_t getegid(void);
#endif

#endif /* BLINK_WIN_GETEIDS_H_ */
