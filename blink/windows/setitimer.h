#ifndef BLINK_WIN_setitimer_H_
#define BLINK_WIN_setitimer_H_

#ifdef _WIN32
#define ITIMER_REAL    0

struct itimerval {
    struct timeval it_interval;
    struct timeval it_value;
};

int setitimer(int which, const struct itimerval *newvalue,
              struct itimerval *oldvalue);
#endif

#endif /* BLINK_WIN_setitimer_H_ */
