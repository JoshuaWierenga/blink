#ifndef BLINK_DEBUG_H_
#define BLINK_DEBUG_H_
#include <stddef.h>

#ifndef _WIN32
#include "blink/fds.h"
#endif
#include "blink/types.h"

void PrintBacktrace(void);
#ifndef _WIN32
void DumpHex(u8 *, size_t);
void PrintFds(struct Fds *);
const char *DescribeProt(int);
const char *DescribeMopcode(int);
const char *DescribeCpuFlags(int);
#endif

#endif /* BLINK_DEBUG_H_ */
