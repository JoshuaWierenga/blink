#ifndef BLINK_WIN_COSMO_LIBC_ASSERT_H_
#define BLINK_WIN_COSMO_LIBC_ASSERT_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/assert.h

#ifdef __GNUC__
/**
 * Specifies expression can't possibly be false.
 *
 * This macro uses the `notpossible` keyword and is intended for things
 * like systems integration, where we know for a fact that something is
 * never going to happen, but there's no proof. So we don't want to add
 * extra bloat for filenames and line numbers, since `ShowCrashReports`
 * can print a backtrace if we just embed the UD2 instruction to crash.
 * That's useful for systems code, for the following reason. Invariants
 * make sense with _unassert() since they usually get placed at the top
 * of functions. But if you used _unassert() to test a system call does
 * not fail, then check happens at the end of your function usually and
 * is therefore likely to result in a failure condition where execution
 * falls through into a different function, which is shocking to debug.
 */
#define _npassert(x)                 \
  ({                                 \
    if (__builtin_expect(!(x), 0)) { \
      __builtin_unreachable();       \
    }                                \
    (void)0;                         \
  })
#else
#define _npassert(x) _unassert(x)
#endif

#endif /* BLINK_WIN_COSMO_LIBC_ASSERT_H_ */
