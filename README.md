# Windows blink

blink is an emulator for running x86-64-linux programs on different
operating systems and hardware architectures. It's designed to do the
same thing as the `qemu-x86_64` command, except rather than being a 10mb
binary, blink only has a ~200kb footprint. For further details on the
motivations for this tool, please read <https://justine.lol/ape.html>.

Note that only blink builds currently as blinkenlights/tui has been
removed.

Currently, this branch is focused on adding support for Windows x86-64
to blink for cosmopolitan binaries and enabling compilation with th
MinGW-w64 compiler. This is somewhat redundant since I have already made
changes to cosmo to ensure that Windows Vista, 7 and 8 x86-64 remain
supported and have made some experimental changes to enable binaries to
run on Windows XP Professional x64 Edition and Windows Server 2003 x86-64
but there is a chance that blink can do it better. There is also Cygwin
support now that should work as far back as Windows XP SP3, even on
x86-32 but that has so far proven to be unstable.

To do this I have reverted all the way to blink's first commit(at least
in its own repo), removed all code and then slowly started readding it
while ensuring everything works on Windows. This has required polyfilling
lots of POSIX functionality that is missing on windows. This has been
done using a mix of cosmopolitan, gnulib(which I plan to remove due to
licencing differences) and an mmap implementation I found that was
specifically designed for use with MinGW-w64.

In the future, I plan to support Windows x86-32 and at least match Cygwin
in working as far back as Windows XP SP3 if not further back. Compiling
with MSVC is something I am considering but have not decided on yet.

Some basic information about what works and what doesn't can be found [here](https://docs.google.com/spreadsheets/d/1861-zsZLEvvcHWlU3aC9PD9QkzspXeXB/edit?usp=sharing&ouid=109492778709853331117&rtpof=true&sd=true).

## Getting Started

You can compile blink on x86-64 Linux, Darwin, FreeBSD, NetBSD, OpenBSD,
or Apple Silicon with Rosetta installed, using your platform toolchain.

```sh
$ build/bootstrap/make.com -j8 o///blink/blink
$ o///blink/blink third_party/cosmo/hello.com
hello world
$ o///blink/blink third_party/cosmo/tinyhello.elf
hello world
```

To compile blink for x86-64 Windows, mingw-w64 is required, I have
tested `x86_64-w64-mingw32-gcc-posix (GCC) 10-posix 20220113` on x86-64
WSL2 Ubuntu running on Windows 10 which works.

```sh
$ make o//x86_64-mingw-w64/blink/blink
$ o//x86_64-mingw-w64/blink/blink.exe third_party/cosmo/hello.com
hello world
$ o//x86_64-mingw-w64/blink/blink.exe third_party/cosmo/tinyhello.elf
hello world
```

There's a terminal interface for debugging(currently unavailable):

```
$ build/bootstrap/make.com -j8 o///blink/tui
$ o///blink/tui -t third_party/cosmo/tinyhello.elf
```

On x86-64 Linux you can cross-compile blink for Linux systems with x86,
arm, m68k, riscv, mips, s390x, powerpc, or microblaze cpus. This happens
using vendored musl-cross-make toolchains and static qemu for testing.

```sh
$ build/bootstrap/make.com -j8 test
$ o/third_party/qemu/qemu-aarch64 o//aarch64/blink/blink third_party/cosmo/hello.com
hello world
```

## Technical Details

blink is an x86-64 interpreter written in straightforward POSIX ANSI C.
Similar to Bochs, there's no JIT or code generation currently in blink.
Therefore you're trading away performance for a tinier emulator that'll
just work, is ISC (rather than GPL) licensed and it won't let untrusted
code get too close to your hardware. Instruction decoding is done using
our trimmed-down version of Intel's disassembler Xed.
