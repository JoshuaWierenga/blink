#ifndef BLINK_FLAG_H_
#define BLINK_FLAG_H_
#include <stdbool.h>

#include "blink/types.h"
#include "blink/windows.h"

#ifndef WINBLINK

extern bool FLAG_zero;
extern bool FLAG_wantjit;
#endif
extern bool FLAG_nolinear;
#ifndef WINBLINK
extern bool FLAG_noconnect;
extern bool FLAG_nologstderr;
extern bool FLAG_alsologtostderr;

extern int FLAG_strace;
#endif
extern int FLAG_vabits;

extern long FLAG_pagesize;

#ifndef WINBLINK
extern u64 FLAG_skew;
#endif
extern u64 FLAG_vaspace;
extern u64 FLAG_stacktop;
extern u64 FLAG_aslrmask;
extern u64 FLAG_imagestart;
extern i64 FLAG_automapend;
extern i64 FLAG_automapstart;
extern u64 FLAG_dyninterpaddr;

extern char *FLAG_logpath;
#ifndef WINBLINK
extern const char *FLAG_overlays;
extern const char *FLAG_prefix;
extern const char *FLAG_bios;
#endif

#endif /* BLINK_FLAG_H_ */
