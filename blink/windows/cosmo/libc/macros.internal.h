#ifndef BLINK_WIN_COSMO_LIBC_MACROS_H_
#define BLINK_WIN_COSMO_LIBC_MACROS_H_

// Based on https://github.com/jart/cosmopolitan/blob/9634227/libc/macros.internal.h

#if 0
/*───────────────────────────────────────────────────────────────────────────│─╗
│ cosmopolitan § macros                                                    ─╬─│┼
╚────────────────────────────────────────────────────────────────────────────│*/
#endif

/**
 * @fileoverview Common C preprocessor, assembler, and linker macros.
 * Ported to work under mingw-w64 which produces pe32(+) binaries and
 * supports different gas pseudo-ops than elf.
 */

#ifdef __ASSEMBLER__
// clang-format off

//	Shorthand notation for widely-acknowledged sections.
.macro	.rodata
	.section .rodata,"a"//,@progbits
.endm

//	Mergeable numeric constant sections.
//
//	@note	linker de-dupes item/values across whole compile
//	@note	therefore item/values are reordered w.r.t. link order
//	@note	therefore no section relative addressing
.macro	.rodata.cst4
	.section .rodata.cst4,"a"//M",@progbits,4
	.align	4
.endm

//	Helpers for Cosmopolitan _init() amalgamation magic.
//	@param	name should be consistent across macros for a module
//	@see	libc/runtime/_init.S
.macro	.initro number:req name:req
	.section ".initro.\number\().\name","a"//,@progbits
	.align	8
.endm
.macro	.initbss number:req name:req
	.section ".piro.bss.init.2.\number\().\name","aw"//,@nobits
	.align	8
.endm
.macro	.init.start number:req name:req
	.section ".init.\number\().\name","ax"//,@progbits
"\name":
.endm
.macro	.init.end number:req name:req bnd=globl vis prevsec:req
	.endfn	"\name",\bnd,\vis
    .\prevsec //.previous
.endm

//  These two still gives "Warning: unexpected storage class 0" despite my attempt 
//  at porting them to coff/pe.
//	Ends function definition.
//	@cost	saves 1-3 lines of code
.macro	.endfn	name:req bnd vis
  .def "\name"
    .size	.-"\name"
    //.type	@function I can't find decent documentation on what to put here for pe
    // Microsoft has documentation for it but I can't find the gas names for the same
    // https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#type-representation
    // using the values directly causes an error. Default of unknown literal is fine.
  .endef
 .ifnb	\bnd
  .\bnd	"\name"
 .endif
 .ifnb	\vis
  .\vis	"\name"
 .endif
.endm

//	Ends variable definition.
//	@cost	saves 1-3 lines of code
.macro	.endobj	name:req bnd vis
 .def "\name"
   .size	.-"\name"
   //.type   @object
 .endef
 .ifnb	\bnd
  .\bnd	"\name"
 .endif
 .ifnb	\vis
  .\vis	"\name"
 .endif
.endm

// clang-format on
#endif /* __ASSEMBLER__ */
#endif /* COSMOPOLITAN_LIBC_MACROS_H_ */
