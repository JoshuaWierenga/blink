#ifndef BLINK_WIN_COSMO_LIBC_STR_UTF16_H_
#define BLINK_WIN_COSMO_LIBC_STR_UTF16_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/str/utf16.h

#define UTF16_MASK 0xfc00
#define UTF16_CONT 0xdc00 /* 0xDC00..0xDFFF */

#define IsLowSurrogate(wc)  ((UTF16_MASK & (wc)) == UTF16_CONT)
#define IsUcs2(wc)          (((65535 & (wc)) >> 11) != 27)
#define IsUtf16Cont(wc)     IsLowSurrogate(wc) /* TODO: DELETE */
#define MergeUtf16(hi, lo)  ((((hi)-0xD800) << 10) + ((lo)-0xDC00) + 0x10000)
#define EncodeUtf16(wc)                                       \
  ((0x0000 <= (wc) && (wc) <= 0xFFFF) ||                      \
   (0xE000 <= (wc) && (wc) <= 0xFFFF))                        \
       ? (wc)                                                 \
   : 0x10000 <= (wc) && (wc) <= 0x10FFFF                      \
       ? (((((wc)-0x10000) >> 10) + 0xD800) |                 \
          (unsigned)((((wc)-0x10000) & 1023) + 0xDC00) << 16) \
       : 0xFFFD

#endif /* BLINK_WIN_COSMO_LIBC_STR_UTF16_H_ */
