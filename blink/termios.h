#ifndef BLINK_TERMIOS_H_
#define BLINK_TERMIOS_H_

#include "blink/windows/macros.h"
#include WINDOWSGNULIBHEADER(termios.h)

#ifdef _WIN32
#include "blink/windows/termios.h"
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
