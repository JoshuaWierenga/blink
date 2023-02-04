#ifndef BLINK_WIN_TERMIOS_H_
#define BLINK_WIN_TERMIOS_H_

#ifdef _WIN32
#define TIOCGWINSZ  0
#define TCGETS      1
#define TCSETS      2
#define TCSETSW     3
#define TCSETSF     4

#define VINTR       0
#define VQUIT       1
#define VERASE      2
#define VKILL       3
#define VEOF        4
#define VMIN        5
#define VSTART      6
#define VSTOP       7
#define VSUSP       8
#define VREPRINT    9
#define VDISCARD   10
#define VWERASE    11
#define VLNEXT     13
#define NCCS       VLNEXT + 1

#define CS5         0x00
#define CS6         0x01
#define CS7         0x02
#define CS8         0x03
#define CSIZE       CS8

#define ISIG        0x01
#define ICANON      0x02
#define IEXTEN      0x04
#define ECHO        0x08
#define ECHOE       0x10

#define OPOST       0x01
#define ONLCR       0x02

struct termios {
    uint32_t c_iflag;
    uint32_t c_oflag;
    uint32_t c_cflag;
    uint32_t c_lflag;
    uint8_t c_cc[NCCS];
};

struct winsize {
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
	uint16_t ws_ypixel;
};
#endif

#endif /* BLINK_WIN_TERMIOS_H_ */
