#ifndef BLINK_WIN_STRNDUP_H_
#define BLINK_WIN_STRNDUP_H_

#ifdef _WIN32
// Where is this coming from? I was about to copy the 
// cosmo version but suddenly lines.c started building.
char *strndup(const char *s, size_t n);
#endif

#endif /* BLINK_WIN_STRNDUP_H_ */
