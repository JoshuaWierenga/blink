#ifndef BLINK_WIN_COSMO_LIBC_INTRIN_PUSHPOP_H_
#define BLINK_WIN_COSMO_LIBC_INTRIN_PUSHPOP_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/intrin/pushpop.h

#if !defined(__GNUC__) || defined(__STRICT_ANSI__)
#define pushpop(x) (x)
#else
/**
 * PushPop
 * An elegant weapon for a more civilized age.
 */
#define pushpop(x)                                          \
  ({                                                        \
    typeof(x) Popped;                                       \
    if (__builtin_constant_p(x) &&                          \
        (TYPE_SIGNED(typeof(x)) ? (intptr_t)(x) + 128 < 256 \
                                : (intptr_t)(x) < 128)) {   \
      if (x) {                                              \
        asm("push\t%1\n\t"                                  \
            "pop\t%q0"                                      \
            : "=r"(Popped)                                  \
            : "ir"(x));                                     \
      } else {                                              \
        asm("xor\t%k0,%k0" : "=r"(Popped));                 \
      }                                                     \
    } else {                                                \
      asm("" : "=r"(Popped) : "0"(x));                      \
    }                                                       \
    Popped;                                                 \
  })
#endif

#if !defined(__GNUC__) || defined(__STRICT_ANSI__)
#define pushmov(d, x) (*(d) = (x))
#else
#define pushmov(d, x)                                       \
  ({                                                        \
    typeof(*(d)) Popped = (x);                              \
    if (__builtin_constant_p(x) &&                          \
        (TYPE_SIGNED(typeof(x)) ? (intptr_t)(x) + 128 < 256 \
                                : (intptr_t)(x) < 128)) {   \
      asm("pushq\t%1\n\t"                                   \
          "popq\t%0"                                        \
          : "=m"(*(d))                                      \
          : "ir"(Popped));                                  \
    } else {                                                \
      *(d) = Popped;                                        \
    }                                                       \
    Popped;                                                 \
  })
#endif

#endif /* COSMOPOLITAN_LIBC_INTRIN_PUSHPOP_H_ */
