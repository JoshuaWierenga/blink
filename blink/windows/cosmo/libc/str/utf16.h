#ifndef BLINK_WIN_COSMO_UTF16_H_
#define BLINK_WIN_COSMO_UTF16_H_

#define UTF16_MASK 0xfc00
#define UTF16_CONT 0xdc00 /* 0xDC00..0xDFFF */

#define IsLowSurrogate(wc)  ((UTF16_MASK & (wc)) == UTF16_CONT)
#define IsUcs2(wc)          (((65535 & (wc)) >> 11) != 27)
#define IsUtf16Cont(wc)     IsLowSurrogate(wc) /* TODO: DELETE */
#define MergeUtf16(hi, lo)  ((((hi)-0xD800) << 10) + ((lo)-0xDC00) + 0x10000)

#endif /* BLINK_WIN_COSMO_UTF16_H_ */
