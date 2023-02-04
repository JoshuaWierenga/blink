#ifndef BLINK_TERMIOS_H_
#define BLINK_TERMIOS_H_

#ifdef _WIN32
#include "third_party/gnulib_build/lib/termios.h"
#include "blink/windows/termios.h"
#else
#include <termios.h>
#endif

/*#ifndef TCGETS
#define TCGETS TIOCGETA
#endif
#ifndef TCSETS
#define TCSETS TIOCSETA
#endif
#ifndef TCSETSW
#define TCSETSW TIOCSETAW
#endif
#ifndef TCSETSF
#define TCSETSF TIOCSETAF
#endif*/

#endif /* BLINK_TERMIOS_H_ */
